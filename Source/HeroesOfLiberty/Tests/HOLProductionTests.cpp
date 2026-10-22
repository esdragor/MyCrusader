// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionRules.h"
#include "Engine/DataTable.h"
#include "Match/HOLMatchSettings.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J3 checks (D284): the production rules (D282) and the provisional values of the tables (D283).
namespace UE::HeroesOfLiberty::Tests::Production
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));
	/** The soldiers of the Barracks (J7-Q2; the Archer, J7c.1). */
	const FName SpearmanType(TEXT("Spearman"));
	const FName ManAtArmsType(TEXT("ManAtArms"));
	const FName ArcherType(TEXT("Archer"));

	/** 50 food, 4 steps, 1 place. */
	FHOLProductionCost MakeCost()
	{
		FHOLProductionCost Cost;
		Cost.Resources = FHOLResources(50, 0, 0, 0);
		Cost.BuildSteps = 4;
		Cost.Population = 1;
		return Cost;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLProductionQueueTest, "HeroesOfLiberty.Production.Queue", UE::HeroesOfLiberty::Tests::Production::Flags)
bool FHOLProductionQueueTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Production;
	const FHOLProductionCost Cost = MakeCost();
	TArray<FHOLProductionItem> Queue;
	FHOLResources Stock(120, 0, 0, 0);

	TestEqual(TEXT("first queued"), int32(HOLProduction::TryEnqueue(Queue, Peasant, Cost, 5, Stock)), int32(EHOLProductionRefusal::None));
	TestEqual(TEXT("second queued"), int32(HOLProduction::TryEnqueue(Queue, Peasant, Cost, 5, Stock)), int32(EHOLProductionRefusal::None));
	TestTrue(TEXT("cost taken when queued"), Stock == FHOLResources(20, 0, 0, 0));
	TestFalse(TEXT("a queued item waits"), Queue[0].IsStarted());
	TestEqual(TEXT("third refused: 20 food left"), int32(HOLProduction::TryEnqueue(Queue, Peasant, Cost, 5, Stock)), int32(EHOLProductionRefusal::NotEnoughResources));
	TestEqual(TEXT("a refusal changes nothing"), Queue.Num(), 2);
	TestTrue(TEXT("stock unchanged by a refusal"), Stock == FHOLResources(20, 0, 0, 0));

	FHOLResources Plenty(1000, 0, 0, 0);
	TestEqual(TEXT("queue of 2 at most"), int32(HOLProduction::TryEnqueue(Queue, Peasant, Cost, 2, Plenty)), int32(EHOLProductionRefusal::QueueFull));
	TestEqual(TEXT("nothing taken from a full queue"), Plenty.Food, 1000);

	TestTrue(TEXT("cancel the first"), HOLProduction::Cancel(Queue, 0, Cost, Stock));
	TestTrue(TEXT("cancel gives the cost back in full"), Stock == FHOLResources(70, 0, 0, 0));
	TestEqual(TEXT("one item left"), Queue.Num(), 1);
	TestFalse(TEXT("nothing to cancel past the queue"), HOLProduction::Cancel(Queue, 3, Cost, Stock));
	TestEqual(TEXT("stock unchanged by a wrong cancel"), Stock.Food, 70);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLProductionAdvanceTest, "HeroesOfLiberty.Production.Advance", UE::HeroesOfLiberty::Tests::Production::Flags)
bool FHOLProductionAdvanceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Production;
	const FHOLProductionCost Cost = MakeCost();
	const auto GetCost = [&Cost](FName) { return Cost; };
	TArray<FHOLProductionItem> Queue;
	FHOLResources Stock(1000, 0, 0, 0);
	HOLProduction::TryEnqueue(Queue, Peasant, Cost, 5, Stock);
	HOLProduction::TryEnqueue(Queue, Peasant, Cost, 5, Stock);
	TArray<FName> Done;

	// Population full: the head waits, as in AoE.
	int32 FreePopulation = 0;
	HOLProduction::Advance(Queue, 10, GetCost, FreePopulation, Done);
	TestFalse(TEXT("waits while the population is full"), Queue[0].IsStarted());
	TestEqual(TEXT("no progress while waiting"), HOLProduction::GetProgress(Queue[0], 10.0, Cost.BuildSteps), 0.f);

	// Room for one: the head starts and holds its place; the next one does not start.
	FreePopulation = 1;
	HOLProduction::Advance(Queue, 11, GetCost, FreePopulation, Done);
	TestEqual(TEXT("head started at step 11"), Queue[0].StartStep, int64(11));
	TestEqual(TEXT("its place is taken"), FreePopulation, 0);
	TestFalse(TEXT("only the head is produced"), Queue[1].IsStarted());
	TestEqual(TEXT("started items hold their places"), HOLProduction::GetStartedPopulation(Queue, GetCost), 1);
	TestEqual(TEXT("half done two steps later"), HOLProduction::GetProgress(Queue[0], 13.0, Cost.BuildSteps), 0.5f);

	// Not done before BuildSteps steps.
	FreePopulation = 5;
	HOLProduction::Advance(Queue, 14, GetCost, FreePopulation, Done);
	TestEqual(TEXT("not done after 3 steps"), Done.Num(), 0);

	// Done after BuildSteps steps; the next one starts at the same step.
	HOLProduction::Advance(Queue, 15, GetCost, FreePopulation, Done);
	TestEqual(TEXT("one unit done"), Done.Num(), 1);
	TestEqual(TEXT("one item left"), Queue.Num(), 1);
	TestEqual(TEXT("the next one starts at once"), Queue[0].StartStep, int64(15));
	TestEqual(TEXT("its place is taken"), FreePopulation, 4);
	TestEqual(TEXT("progress is capped"), HOLProduction::GetProgress(Queue[0], 100.0, Cost.BuildSteps), 1.f);

	HOLProduction::Advance(Queue, 19, GetCost, FreePopulation, Done);
	TestEqual(TEXT("both units done"), Done.Num(), 2);
	TestEqual(TEXT("queue empty"), Queue.Num(), 0);

	TestEqual(TEXT("population cap: what the buildings give"), HOLProduction::GetPopulationCap(10), 10);
	TestEqual(TEXT("population cap: 175 at most (D03)"), HOLProduction::GetPopulationCap(400), 175);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLProductionTablesTest, "HeroesOfLiberty.Production.Tables", UE::HeroesOfLiberty::Tests::Production::Flags)
bool FHOLProductionTablesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Production;
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		const FHOLUnitTypeRow* Row = UnitTypes->FindRow<FHOLUnitTypeRow>(Peasant, TEXT("Production test"));
		if (TestNotNull(TEXT("Peasant in the unit type table"), Row))
		{
			TestTrue(TEXT("50 food (D283)"), Row->Cost == FHOLResources(50, 0, 0, 0));
			TestEqual(TEXT("20 s (D283)"), Row->BuildTime, 20.f);
			TestEqual(TEXT("one place"), Row->Population, 1);
			TestTrue(TEXT("Peasant: its own armour category only (J7-Q4)"), Row->ArmorCategories.Num() == 1 && Row->HasArmorCategory(EHOLArmorCategory::Peasant));
			TestTrue(TEXT("Peasant: a weak attack (J7-Q4)"), Row->Attack > 0 && Row->Attack < 8);
			TestEqual(TEXT("Peasant: takes on nobody by itself"), Row->AggressionRadius, 0.f);
		}

		// J7a.1 (J7-Q2, J7-Q4): the two soldiers of the Barracks and their combat values (AoE4-like, indicative).
		const FHOLUnitTypeRow* Spearman = UnitTypes->FindRow<FHOLUnitTypeRow>(SpearmanType, TEXT("Production test"));
		if (TestNotNull(TEXT("Spearman in the unit type table"), Spearman))
		{
			TestTrue(TEXT("Spearman: 60 food, 20 wood"), Spearman->Cost == FHOLResources(60, 20, 0, 0));
			TestEqual(TEXT("Spearman: one place"), Spearman->Population, 1);
			TestEqual(TEXT("Spearman: 80 health"), Spearman->MaxHealth, 80);
			TestTrue(TEXT("Spearman: light infantry only"), Spearman->ArmorCategories.Num() == 1 && Spearman->HasArmorCategory(EHOLArmorCategory::LightInfantry));
			TestEqual(TEXT("Spearman: attack 8"), Spearman->Attack, 8);
			TestTrue(TEXT("Spearman: +17 against cavalry"), Spearman->Bonuses.Num() == 1 && Spearman->Bonuses[0].Category == EHOLArmorCategory::Cavalry
				&& Spearman->Bonuses[0].Damage == 17);
			TestTrue(TEXT("Spearman: a cadence, a melee reach, an aggression radius"),
				Spearman->AttackInterval > 0.f && Spearman->AttackRange > 0.f && Spearman->AttackRange < 200.f && Spearman->AggressionRadius > 0.f);
		}
		const FHOLUnitTypeRow* ManAtArms = UnitTypes->FindRow<FHOLUnitTypeRow>(ManAtArmsType, TEXT("Production test"));
		if (TestNotNull(TEXT("ManAtArms in the unit type table"), ManAtArms))
		{
			TestTrue(TEXT("Man-at-arms: 100 food, 20 gold"), ManAtArms->Cost == FHOLResources(100, 0, 0, 20));
			TestEqual(TEXT("Man-at-arms: one place"), ManAtArms->Population, 1);
			TestEqual(TEXT("Man-at-arms: 155 health"), ManAtArms->MaxHealth, 155);
			TestTrue(TEXT("Man-at-arms: heavy infantry only"), ManAtArms->ArmorCategories.Num() == 1 && ManAtArms->HasArmorCategory(EHOLArmorCategory::HeavyInfantry));
			TestEqual(TEXT("Man-at-arms: attack 12"), ManAtArms->Attack, 12);
			TestEqual(TEXT("Man-at-arms: melee armour 4"), ManAtArms->MeleeArmor, 4);
			TestEqual(TEXT("Man-at-arms: ranged armour 4"), ManAtArms->RangedArmor, 4);
			TestEqual(TEXT("Man-at-arms: no bonus"), ManAtArms->Bonuses.Num(), 0);
			TestEqual(TEXT("Man-at-arms: hero armour 2 (D345)"), ManAtArms->HeroArmor, 2);
			TestTrue(TEXT("Man-at-arms: costs more than the Spearman"), Spearman && ManAtArms->Cost.Food > Spearman->Cost.Food);
		}
		// J7c.1: the Archer, third soldier of the Barracks (AoE4-like, indicative).
		const FHOLUnitTypeRow* Archer = UnitTypes->FindRow<FHOLUnitTypeRow>(ArcherType, TEXT("Production test"));
		if (TestNotNull(TEXT("Archer in the unit type table"), Archer))
		{
			TestTrue(TEXT("Archer: 30 food, 50 wood"), Archer->Cost == FHOLResources(30, 50, 0, 0));
			TestEqual(TEXT("Archer: one place"), Archer->Population, 1);
			TestEqual(TEXT("Archer: 70 health"), Archer->MaxHealth, 70);
			TestTrue(TEXT("Archer: Ranged category only"), Archer->ArmorCategories.Num() == 1 && Archer->HasArmorCategory(EHOLArmorCategory::Ranged));
			TestEqual(TEXT("Archer: ranged attack (D345)"), int32(Archer->AttackType), int32(EHOLAttackType::Ranged));
			const bool bArcherBonuses = Archer->Bonuses.Num() == 2
				&& Archer->Bonuses[0].Category == EHOLArmorCategory::LightInfantry && Archer->Bonuses[0].Damage == 5
				&& Archer->Bonuses[1].Category == EHOLArmorCategory::GunpowderInfantry && Archer->Bonuses[1].Damage == 5;
			TestTrue(TEXT("Archer: +5 against light infantry and +5 against gunpowder infantry (D345)"), bArcherBonuses);
			TestEqual(TEXT("Archer: 779 cm/s"), Archer->Speed, 779.f);
		}
	}

	const FHOLBuildingTypeRow* Barracks = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Barracks);
	if (TestNotNull(TEXT("Barracks in the building type table"), Barracks))
	{
		TestEqual(TEXT("Barracks: military (F1, D30)"), int32(Barracks->Category), int32(EHOLBuildingCategory::Military));
		TestTrue(TEXT("Barracks: produces the Spearman, the Man-at-arms then the Archer"),
			Barracks->Produces == TArray<FName>({ SpearmanType, ManAtArmsType, ArcherType }));
		TestEqual(TEXT("Barracks: gives no population"), Barracks->PopulationProvided, 0);
		TestTrue(TEXT("Barracks: not a deposit"), Barracks->Accepts.IsEmpty());
	}

	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("main centre in the building type table"), MainCentre))
	{
		TestEqual(TEXT("base population 10 (D283)"), MainCentre->PopulationProvided, 10);
		TestEqual(TEXT("queue of 5 (D283)"), MainCentre->MaxQueue, 5);
		TestTrue(TEXT("produces the Peasant"), MainCentre->Produces.Contains(Peasant));
	}

	TestTrue(TEXT("200 of each at the start (D283)"), GetDefault<UHOLMatchSettings>()->StartingResources == FHOLResources(200, 200, 200, 200));
	return true;
}

#endif
