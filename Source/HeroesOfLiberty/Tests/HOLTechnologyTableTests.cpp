// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Buildings/HOLProductionRules.h"
#include "Technologies/HOLTechnologyRules.h"
#include "Simulation/HOLUnitTypes.h"
#include "Technologies/HOLTechnologyTypes.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

// T1 (Technologies, GDD 11.1): the table Tools/Data/Technologies.csv, read through the same import as the editor's
// (UDataTable::CreateTableFromCSVString, what make_technologies.py does into /Game/Technologies/Technologies). The expected
// content below is the GDD's grid 11.1 (names, tier, building when the GDD places it), not a copy of the CSV: the test fails
// when a technology of the grid is missing, wrongly tiered or put in a building the GDD names otherwise.
namespace UE::HeroesOfLiberty::Tests::TechnologyTable
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** One line of GDD 11.1: the row name, its tier, and its building when the GDD names it (empty: left to T-Q3). */
	struct FGridLine
	{
		const TCHAR* Name;
		int32 Tier;
		const TCHAR* Building;
	};

	constexpr FGridLine Grid[] = {
		// Forge: the four lines, I at tier 1 and II at tier 2.
		{ TEXT("MeleeAttackI"), 1, TEXT("Forge") }, { TEXT("MeleeAttackII"), 2, TEXT("Forge") },
		{ TEXT("RangedAttackI"), 1, TEXT("Forge") }, { TEXT("RangedAttackII"), 2, TEXT("Forge") },
		{ TEXT("InfantryArmorI"), 1, TEXT("Forge") }, { TEXT("InfantryArmorII"), 2, TEXT("Forge") },
		{ TEXT("CavalryArmorI"), 1, TEXT("Forge") }, { TEXT("CavalryArmorII"), 2, TEXT("Forge") },
		// Camps and farm (T-Q3 = A, D583): the Wheelbarrow is researched at the Farm, with the Plough, not at the main centre.
		{ TEXT("DoubleAxesI"), 1, TEXT("LumberCamp") }, { TEXT("DoubleAxesII"), 2, TEXT("LumberCamp") },
		{ TEXT("MinerPicksI"), 1, TEXT("MiningCamp") }, { TEXT("MinerPicksII"), 2, TEXT("MiningCamp") },
		{ TEXT("PloughI"), 1, TEXT("Farm") }, { TEXT("PloughII"), 2, TEXT("Farm") },
		{ TEXT("WheelbarrowI"), 1, TEXT("Farm") }, { TEXT("WheelbarrowII"), 2, TEXT("Farm") },
		// Main centre: the Defence Militia is dropped (T-Q9, "On oublie", D600); only the alarm bell stays.
		{ TEXT("ImprovedAlarmBell"), 2, TEXT("MainCentre") },
		// Academy (tier 2 only).
		{ TEXT("Masonry"), 2, TEXT("Academy") }, { TEXT("Veterans"), 2, TEXT("Academy") }, { TEXT("Logistics"), 2, TEXT("Academy") },
		// Siege workshop.
		{ TEXT("ReinforcedRam"), 1, TEXT("SiegeWorkshop") }, { TEXT("HeavyMunitions"), 2, TEXT("SiegeWorkshop") },
		// The Dawn's own (D27, 11.1; T-Q3 = A, D583): Liturgy and Blessing of Arms at the Sanctuary, Tempered Halberds at the
		// Barracks, Vow of Poverty at the Chapel.
		{ TEXT("Liturgy"), 1, TEXT("Sanctuary") }, { TEXT("TemperedHalberds"), 1, TEXT("Barracks") },
		{ TEXT("BlessingOfArms"), 2, TEXT("Sanctuary") }, { TEXT("VowOfPoverty"), 2, TEXT("Chapel") },
	};

	/** The Dawn's own technologies of the grid (the others are common, D27). */
	constexpr const TCHAR* DawnOwn[] = { TEXT("Liturgy"), TEXT("TemperedHalberds"), TEXT("BlessingOfArms"), TEXT("VowOfPoverty") };

	/** Buildings of GDD 11.1 not yet in BuildingTypes.csv (their chantiers to come, T-Q4): a row may name them. The Sanctuary is a type since SC-Q1 = B (D602), the siege workshop since S2. */
	constexpr const TCHAR* PlannedBuildings[] = { TEXT("Forge"), TEXT("Academy"), TEXT("Chapel") };

	/** The statistics some code applies today (T5 gathering and workers, T6 melee and ranged attack): a row whose statistics are all here is Implemented, any other is "Soon" (D601). */
	constexpr const TCHAR* CodedStats[] = {
		TEXT("MeleeAttack"), TEXT("RangedAttack"),
		TEXT("GatherWood"), TEXT("GatherStone"), TEXT("GatherGold"), TEXT("GatherFood"), TEXT("WorkerSpeed"), TEXT("WorkerCarry"),
	};

	/** The statistics the effects code (T5) will read; a new one is added here with its code. */
	constexpr const TCHAR* KnownStats[] = {
		TEXT("MeleeAttack"), TEXT("RangedAttack"), TEXT("InfantryArmor"), TEXT("CavalryArmor"),
		TEXT("GatherWood"), TEXT("GatherStone"), TEXT("GatherGold"), TEXT("GatherFood"), TEXT("WorkerSpeed"), TEXT("WorkerCarry"),
		TEXT("GarrisonSpeed"), TEXT("WallTowerHealth"), TEXT("CoreUnitHealth"), TEXT("ArmyMarchSpeed"),
		TEXT("RamHealth"), TEXT("MangonelDamage"), TEXT("HonourGain"), TEXT("HalberdierBonusVsHeavyInfantry"),
		TEXT("KnightAndHalberdierBonusVsProfane"), TEXT("MonkCostDiscount"),
	};

	template <typename T, int32 N>
	bool Contains(const T* const (&List)[N], const FString& Name)
	{
		for (const T* Item : List)
		{
			if (Name == Item)
			{
				return true;
			}
		}
		return false;
	}

	/** Imports a CSV of Tools/Data into a transient table of a row type, the way the editor script does; false (and the problems reported) if it fails. */
	bool Import(FAutomationTestBase& Test, const TCHAR* File, UScriptStruct* RowStruct, UDataTable*& OutTable)
	{
		const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), File);
		FString Text;
		if (!Test.TestTrue(FString::Printf(TEXT("%s is there (Tools/Data/%s)"), *Path, File), FFileHelper::LoadFileToString(Text, *Path)))
		{
			return false;
		}
		OutTable = NewObject<UDataTable>(GetTransientPackage());
		OutTable->RowStruct = RowStruct;
		const TArray<FString> Problems = OutTable->CreateTableFromCSVString(Text);
		for (const FString& Problem : Problems)
		{
			Test.AddError(FString::Printf(TEXT("%s: %s"), File, *Problem));
		}
		return Problems.IsEmpty();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTechnologyTableTest, "HeroesOfLiberty.Technologies.Table", UE::HeroesOfLiberty::Tests::TechnologyTable::Flags)
bool FHOLTechnologyTableTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::TechnologyTable;

	UDataTable* Techs = nullptr;
	if (!Import(*this, TEXT("Technologies.csv"), FHOLTechnologyRow::StaticStruct(), Techs))
	{
		return false;
	}
	UDataTable* Buildings = nullptr;
	if (!Import(*this, TEXT("BuildingTypes.csv"), FHOLBuildingTypeRow::StaticStruct(), Buildings))
	{
		return false;
	}

	// 1. Every line of the grid 11.1 is there, at its tier and (when the GDD says it) in its building; nothing else.
	TestEqual(TEXT("as many rows as the grid 11.1 has lines"), Techs->GetRowNames().Num(), static_cast<int32>(UE_ARRAY_COUNT(Grid)));
	for (const FGridLine& Line : Grid)
	{
		const FHOLTechnologyRow* Row = Techs->FindRow<FHOLTechnologyRow>(Line.Name, TEXT("technology"), false);
		if (!TestNotNull(FString::Printf(TEXT("%s of the grid 11.1 is in the table"), Line.Name), Row))
		{
			continue;
		}
		TestEqual(FString::Printf(TEXT("%s: tier of the grid"), Line.Name), Row->MinTier, Line.Tier);
		if (*Line.Building)
		{
			TestEqual(FString::Printf(TEXT("%s: building of the grid"), Line.Name), Row->Building, FName(Line.Building));
		}
		// 2. The faction: the Dawn's own four, common for all the others (D27).
		TestEqual(FString::Printf(TEXT("%s: faction"), Line.Name), Row->Faction, Contains(DawnOwn, Line.Name) ? FName(TEXT("Dawn")) : FName(TEXT("Common")));
	}

	// 3. The three conditions of GDD 11 hold on every row, and the effect is readable by the code to come.
	for (const TPair<FName, uint8*>& Pair : Techs->GetRowMap())
	{
		const FString Name = Pair.Key.ToString();
		const FHOLTechnologyRow& Row = *reinterpret_cast<const FHOLTechnologyRow*>(Pair.Value);
		TestFalse(FString::Printf(TEXT("%s: has a name to show"), *Name), Row.DisplayName.IsEmpty());

		// Condition 1: the hero (a tier of the prototype, 1 or 2; a level only as the stricter rule).
		TestTrue(FString::Printf(TEXT("%s: tier 1 or 2 (the tier 3 of GDD 11.2 is not in the table, T-Q5)"), *Name), Row.MinTier >= 1 && Row.MinTier <= 2);
		TestEqual(FString::Printf(TEXT("%s: the tier alone decides (T-Q1 = A, D583): hero level 0"), *Name), Row.MinHeroLevel, 0);
		// Condition 2: a building that is in BuildingTypes.csv or planned.
		const bool bKnownBuilding = Buildings->FindRow<FHOLBuildingTypeRow>(Row.Building, TEXT("building"), false) != nullptr
			|| Contains(PlannedBuildings, Row.Building.ToString());
		TestTrue(FString::Printf(TEXT("%s: building %s is a type of BuildingTypes.csv or a planned one"), *Name, *Row.Building.ToString()), bKnownBuilding);
		// Condition 3: a cost, and a time.
		TestTrue(FString::Printf(TEXT("%s: costs something"), *Name), Row.Cost.Food + Row.Cost.Wood + Row.Cost.Stone + Row.Cost.Gold > 0);
		TestTrue(FString::Printf(TEXT("%s: costs no negative amount"), *Name), Row.Cost.Food >= 0 && Row.Cost.Wood >= 0 && Row.Cost.Stone >= 0 && Row.Cost.Gold >= 0);
		TestTrue(FString::Printf(TEXT("%s: takes time"), *Name), Row.ResearchTime > 0.f);

		// The effect: known statistics, a positive change.
		TestFalse(FString::Printf(TEXT("%s: changes a statistic"), *Name), Row.Stats.IsEmpty());
		for (const FName Stat : Row.Stats)
		{
			TestTrue(FString::Printf(TEXT("%s: statistic %s is one the effects code knows"), *Name, *Stat.ToString()), Contains(KnownStats, Stat.ToString()));
		}
		TestTrue(FString::Printf(TEXT("%s: changes by a positive percent"), *Name), Row.Percent > 0);
		// T-Q10/T-Q11 = B (D601): the column Implemented says whether some code applies the effect; "Soon" otherwise.
		bool bAllCoded = true;
		for (const FName Stat : Row.Stats)
		{
			bAllCoded &= Contains(CodedStats, Stat.ToString());
		}
		TestEqual(FString::Printf(TEXT("%s: Implemented tells whether its statistics are applied by code"), *Name), Row.Implemented, bAllCoded);

		// The chain: a level II asks for the level I of its line (same building and statistics, one tier below, a smaller effect).
		if (Name.EndsWith(TEXT("II")))
		{
			const FString First = Name.LeftChop(1);
			TestEqual(FString::Printf(TEXT("%s: asks for %s"), *Name, *First), Row.Requires, FName(*First));
		}
		if (!Row.Requires.IsNone())
		{
			const FHOLTechnologyRow* Before = Techs->FindRow<FHOLTechnologyRow>(Row.Requires, TEXT("required technology"), false);
			if (TestNotNull(FString::Printf(TEXT("%s: the technology it requires (%s) is in the table"), *Name, *Row.Requires.ToString()), Before))
			{
				TestTrue(FString::Printf(TEXT("%s: asks a lower tier than itself"), *Name), Before->MinTier < Row.MinTier);
				TestEqual(FString::Printf(TEXT("%s: researched in the same building as %s"), *Name, *Row.Requires.ToString()), Before->Building, Row.Building);
				TestTrue(FString::Printf(TEXT("%s: stronger than %s"), *Name, *Row.Requires.ToString()), Row.Percent > Before->Percent);
				TestTrue(FString::Printf(TEXT("%s: same statistics as %s"), *Name, *Row.Requires.ToString()), Row.Stats == Before->Stats);
			}
		}
	}
	return true;
}

// T3/T4 (T-Q4 = A, D583; T-Q10/T-Q11 = B, D601): of the 26 technologies of the table (the Militia is dropped, D600), the ones
// the game can play now are those whose building is a type of BuildingTypes.csv AND whose effect is coded (Implemented): the
// camps and the farm (8). The Improved Alarm Bell (main centre) and the Tempered Halberds (Barracks) have their building but no
// effect yet: greyed "Soon". The Reinforced Ram and Heavy Munitions (siege workshop, S2) are Soon too (RamHealth and MangonelDamage are read at S5). The others (Forge, Academy, Chapel) wait for their buildings.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTechnologyPlayableTest, "HeroesOfLiberty.Technologies.Playable", UE::HeroesOfLiberty::Tests::TechnologyTable::Flags)
bool FHOLTechnologyPlayableTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::TechnologyTable;

	UDataTable* Techs = nullptr;
	UDataTable* Buildings = nullptr;
	if (!Import(*this, TEXT("Technologies.csv"), FHOLTechnologyRow::StaticStruct(), Techs) || !Import(*this, TEXT("BuildingTypes.csv"), FHOLBuildingTypeRow::StaticStruct(), Buildings))
	{
		return false;
	}
	const auto BuildingExists = [Buildings](const FName Building) { return Buildings->FindRow<FHOLBuildingTypeRow>(Building, TEXT("building"), false) != nullptr; };

	const TSet<FName> Expected = {
		TEXT("DoubleAxesI"), TEXT("DoubleAxesII"), TEXT("MinerPicksI"), TEXT("MinerPicksII"), TEXT("PloughI"), TEXT("PloughII"),
		TEXT("WheelbarrowI"), TEXT("WheelbarrowII"),
	};
	TSet<FName> Playable;
	for (const TPair<FName, uint8*>& Pair : Techs->GetRowMap())
	{
		const FHOLTechnologyRow& Row = *reinterpret_cast<const FHOLTechnologyRow*>(Pair.Value);
		if (HOLTechnology::IsPlayable(Row, BuildingExists))
		{
			Playable.Add(Pair.Key);
		}
	}
	TestEqual(TEXT("8 technologies are playable"), Playable.Num(), 8);
	for (const TCHAR* Soon : { TEXT("ImprovedAlarmBell"), TEXT("TemperedHalberds"), TEXT("ReinforcedRam"), TEXT("HeavyMunitions") })
	{
		const FHOLTechnologyRow* Row = Techs->FindRow<FHOLTechnologyRow>(Soon, TEXT("technology"), false);
		if (TestNotNull(FString::Printf(TEXT("%s is in the table"), Soon), Row))
		{
			TestTrue(FString::Printf(TEXT("%s: its building exists (it is greyed Soon, not missing)"), Soon), BuildingExists(Row->Building));
			TestFalse(FString::Printf(TEXT("%s is not playable: no effect coded (D601)"), Soon), HOLTechnology::IsPlayable(*Row, BuildingExists));
		}
	}
	for (const FName Name : Expected)
	{
		TestTrue(FString::Printf(TEXT("%s is playable"), *Name.ToString()), Playable.Contains(Name));
	}
	for (const FName Name : Playable)
	{
		TestTrue(FString::Printf(TEXT("%s is expected to be playable"), *Name.ToString()), Expected.Contains(Name));
	}

	// T-Q6 = A: the file of 5 (D283) of every building that researches one is the production queue's.
	for (const FName Name : Expected)
	{
		const FHOLTechnologyRow* Row = Techs->FindRow<FHOLTechnologyRow>(Name, TEXT("technology"), false);
		const FHOLBuildingTypeRow* Building = Row ? Buildings->FindRow<FHOLBuildingTypeRow>(Row->Building, TEXT("building"), false) : nullptr;
		if (TestNotNull(FString::Printf(TEXT("%s: its building is there"), *Name.ToString()), Building))
		{
			TestEqual(FString::Printf(TEXT("%s: %s researches in a queue of 5"), *Name.ToString(), *Row->Building.ToString()), Building->MaxQueue, 5);
		}
	}
	return true;
}

// T3/T4: the rules (order of the reasons, as the script's), the cost of a research as a queue item, the state of the player.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTechnologyRulesTest, "HeroesOfLiberty.Technologies.Rules", UE::HeroesOfLiberty::Tests::TechnologyTable::Flags)
bool FHOLTechnologyRulesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::TechnologyTable;
	using HOLTechnology::EBlock;

	UDataTable* Techs = nullptr;
	if (!Import(*this, TEXT("Technologies.csv"), FHOLTechnologyRow::StaticStruct(), Techs))
	{
		return false;
	}
	const FHOLTechnologyRow* Plough1 = Techs->FindRow<FHOLTechnologyRow>(TEXT("PloughI"), TEXT("technology"), false);
	const FHOLTechnologyRow* Plough2 = Techs->FindRow<FHOLTechnologyRow>(TEXT("PloughII"), TEXT("technology"), false);
	if (!TestNotNull(TEXT("PloughI"), Plough1) || !TestNotNull(TEXT("PloughII"), Plough2))
	{
		return false;
	}

	HOLTechnology::FFacts Ready;
	Ready.HeroTier = 1;
	Ready.HeroLevel = 3;
	Ready.bBuildingFinished = true;
	Ready.bCanPay = true;
	TestEqual(TEXT("level I, tier 1, farm, paid: available"), int32(HOLTechnology::Check(*Plough1, Ready)), int32(EBlock::None));

	HOLTechnology::FFacts Facts = Ready;
	Facts.HeroTier = 0;
	TestEqual(TEXT("no tier yet"), int32(HOLTechnology::Check(*Plough1, Facts)), int32(EBlock::TierTooLow));
	Facts = Ready;
	TestEqual(TEXT("level II at tier 1"), int32(HOLTechnology::Check(*Plough2, Facts)), int32(EBlock::TierTooLow));
	Facts.HeroTier = 2;
	Facts.bRequiresDone = false;
	TestEqual(TEXT("level II without level I"), int32(HOLTechnology::Check(*Plough2, Facts)), int32(EBlock::RequiresMissing));
	Facts.bRequiresDone = true;
	TestEqual(TEXT("level II at tier 2 with level I"), int32(HOLTechnology::Check(*Plough2, Facts)), int32(EBlock::None));
	Facts = Ready;
	Facts.bBuildingFinished = false;
	TestEqual(TEXT("no finished building"), int32(HOLTechnology::Check(*Plough1, Facts)), int32(EBlock::NoBuilding));
	Facts = Ready;
	Facts.bCanPay = false;
	TestEqual(TEXT("cannot pay"), int32(HOLTechnology::Check(*Plough1, Facts)), int32(EBlock::CannotPay));
	Facts = Ready;
	Facts.bQueued = true;
	TestEqual(TEXT("already queued"), int32(HOLTechnology::Check(*Plough1, Facts)), int32(EBlock::AlreadyQueued));
	Facts.bResearched = true;
	TestEqual(TEXT("researched wins over queued"), int32(HOLTechnology::Check(*Plough1, Facts)), int32(EBlock::AlreadyResearched));

	// T-Q11 = B (D601): a technology with no coded effect is refused "Soon" before every other reason, and the log says so.
	FHOLTechnologyRow Soon = *Plough1;
	Soon.Implemented = false;
	TestEqual(TEXT("not implemented, everything else ready"), int32(HOLTechnology::Check(Soon, Ready)), int32(EBlock::NotImplemented));
	Facts = Ready;
	Facts.HeroTier = 0;
	Facts.bResearched = true;
	TestEqual(TEXT("not implemented wins over the tier and the rest"), int32(HOLTechnology::Check(Soon, Facts)), int32(EBlock::NotImplemented));
	TestEqual(TEXT("the log says soon"), FString(HOLTechnology::DescribeBlock(EBlock::NotImplemented)), FString(TEXT("soon")));

	// T-Q1 = A: the tier alone decides while MinHeroLevel is 0; a row that asks a level too (the stricter holds) is refused below it.
	FHOLTechnologyRow Strict = *Plough1;
	Strict.MinHeroLevel = 5;
	Facts = Ready;
	TestEqual(TEXT("tier open, level 3 < 5"), int32(HOLTechnology::Check(Strict, Facts)), int32(EBlock::HeroLevelTooLow));
	Facts.HeroLevel = 5;
	TestEqual(TEXT("level 5"), int32(HOLTechnology::Check(Strict, Facts)), int32(EBlock::None));
	Facts = Ready;
	Facts.HeroLevel = 0;
	TestEqual(TEXT("MinHeroLevel 0: the tier alone, whatever the level"), int32(HOLTechnology::Check(*Plough1, Facts)), int32(EBlock::None));

	// A research is an item of the production queue (T-Q6 = A): cost taken at the order, given back in full at the cancel, no population.
	const float StepSeconds = 0.05f;
	const FHOLProductionCost Cost = HOLTechnology::MakeCost(*Plough1, StepSeconds);
	TestTrue(TEXT("the cost is the row's"), Cost.Resources == Plough1->Cost);
	TestEqual(TEXT("30 s = 600 steps of 0.05 s"), Cost.BuildSteps, 600);
	TestEqual(TEXT("no population place"), Cost.Population, 0);

	TArray<FHOLProductionItem> Queue;
	FHOLResources Stock(1000, 1000, 1000, 1000);
	TestFalse(TEXT("not queued yet"), HOLTechnology::IsQueued(Queue, TEXT("PloughI")));
	for (int32 Index = 0; Index < 5; ++Index)
	{
		const FName Name = Index == 0 ? FName(TEXT("PloughI")) : FName(*FString::Printf(TEXT("Other%d"), Index));
		TestEqual(FString::Printf(TEXT("research %d queued"), Index), int32(HOLProduction::TryEnqueue(Queue, Name, Cost, 5, Stock)), int32(EHOLProductionRefusal::None));
	}
	TestEqual(TEXT("the sixth is refused: a file of 5"), int32(HOLProduction::TryEnqueue(Queue, TEXT("PloughII"), Cost, 5, Stock)), int32(EHOLProductionRefusal::QueueFull));
	TestTrue(TEXT("queued"), HOLTechnology::IsQueued(Queue, TEXT("PloughI")));
	TestFalse(TEXT("another one is not"), HOLTechnology::IsQueued(Queue, TEXT("PloughII")));
	TestEqual(TEXT("five costs taken"), Stock.Food, 1000 - 5 * Plough1->Cost.Food);
	TestTrue(TEXT("cancel gives it back in full"), HOLProduction::Cancel(Queue, 0, Cost, Stock));
	TestEqual(TEXT("four costs taken"), Stock.Food, 1000 - 4 * Plough1->Cost.Food);
	TestFalse(TEXT("no longer queued"), HOLTechnology::IsQueued(Queue, TEXT("PloughI")));

	// A research needs no population place: a player over his cap (a house lost) still finishes it.
	TArray<FHOLProductionItem> Solo;
	FHOLResources Rich(1000, 1000, 1000, 1000);
	HOLProduction::TryEnqueue(Solo, TEXT("PloughI"), Cost, 5, Rich);
	int32 FreePopulation = -3;
	TArray<FName> Done;
	const auto GetCost = [&Cost](FName) { return Cost; };
	HOLProduction::Advance(Solo, 0, GetCost, FreePopulation, Done);
	TestTrue(TEXT("started although the population is over its cap"), Solo.Num() == 1 && Solo[0].IsStarted());
	TestEqual(TEXT("the population is left alone"), FreePopulation, -3);
	HOLProduction::Advance(Solo, 600, GetCost, FreePopulation, Done);
	TestTrue(TEXT("done after 600 steps"), Done.Num() == 1 && Done[0] == FName(TEXT("PloughI")) && Solo.IsEmpty());

	// T5: the effects. A line's percent is its TOTAL (T-Q2): I then II gives II's, not I + II. Other lines on a statistic add up.
	const auto Find = [Techs](const FName Name) { return Techs->FindRow<FHOLTechnologyRow>(Name, TEXT("technology"), false); };
	TArray<FName> Researched;
	TestEqual(TEXT("nothing researched: no bonus"), HOLTechnology::GetBonusPercent(Researched, TEXT("GatherFood"), Find), 0);
	Researched.Add(TEXT("PloughI"));
	TestEqual(TEXT("Plough I: +10 % food"), HOLTechnology::GetBonusPercent(Researched, TEXT("GatherFood"), Find), Plough1->Percent);
	TestEqual(TEXT("Plough I: nothing for wood"), HOLTechnology::GetBonusPercent(Researched, TEXT("GatherWood"), Find), 0);
	Researched.Add(TEXT("PloughII"));
	TestEqual(TEXT("Plough I and II: II's total, not the sum"), HOLTechnology::GetBonusPercent(Researched, TEXT("GatherFood"), Find), Plough2->Percent);
	TestTrue(TEXT("II gives more than I"), Plough2->Percent > Plough1->Percent);
	Researched.Add(TEXT("MinerPicksI"));
	const FHOLTechnologyRow* Picks = Find(TEXT("MinerPicksI"));
	TestEqual(TEXT("Miner picks: stone"), HOLTechnology::GetBonusPercent(Researched, TEXT("GatherStone"), Find), Picks ? Picks->Percent : -1);
	TestEqual(TEXT("Miner picks: gold"), HOLTechnology::GetBonusPercent(Researched, TEXT("GatherGold"), Find), Picks ? Picks->Percent : -1);
	TestEqual(TEXT("the plough still gives food only"), HOLTechnology::GetBonusPercent(Researched, TEXT("GatherFood"), Find), Plough2->Percent);
	TestEqual(TEXT("an unknown technology gives nothing"), HOLTechnology::GetBonusPercent(TArray<FName>{ TEXT("Nothing") }, TEXT("GatherFood"), Find), 0);

	// T5: the Wheelbarrow raises what a worker carries (a whole quantity, 10 at the Peasant): both of its stats are the line's.
	Researched = { TEXT("WheelbarrowI") };
	const FHOLTechnologyRow* Barrow1 = Find(TEXT("WheelbarrowI"));
	const FHOLTechnologyRow* Barrow2 = Find(TEXT("WheelbarrowII"));
	TestEqual(TEXT("Wheelbarrow I: carry +10 %"), HOLTechnology::GetBonusPercent(Researched, HOLTechnology::WorkerCarryStat, Find), Barrow1 ? Barrow1->Percent : -1);
	Researched.Add(TEXT("WheelbarrowII"));
	TestEqual(TEXT("Wheelbarrow I and II: speed +20 % (the line's total)"), HOLTechnology::GetBonusPercent(Researched, HOLTechnology::WorkerSpeedStat, Find), Barrow2 ? Barrow2->Percent : -1);
	TestEqual(TEXT("10 carried, +0 %: 10"), HOLTechnology::ScaleByPercent(10, 0), 10);
	TestEqual(TEXT("10 carried, +10 %: 11"), HOLTechnology::ScaleByPercent(10, 10), 11);
	TestEqual(TEXT("10 carried, +20 %: 12"), HOLTechnology::ScaleByPercent(10, 20), 12);
	TestEqual(TEXT("3 carried, +10 %: still 3 (a third of a unit is not one)"), HOLTechnology::ScaleByPercent(3, 10), 3);
	TestEqual(TEXT("nothing carried: nothing"), HOLTechnology::ScaleByPercent(0, 20), 0);
	TestEqual(TEXT("a negative percent never lowers it"), HOLTechnology::ScaleByPercent(10, -50), 10);

	// T6: the Forge's attack lines raise a hit's base attack by the attack's type; melee and ranged are separate lines.
	TestEqual(TEXT("melee hits are raised by MeleeAttack"), HOLTechnology::GetAttackStat(EHOLAttackType::Melee), HOLTechnology::MeleeAttackStat);
	TestEqual(TEXT("ranged hits are raised by RangedAttack"), HOLTechnology::GetAttackStat(EHOLAttackType::Ranged), HOLTechnology::RangedAttackStat);
	TestEqual(TEXT("siege hits: no technology yet"), HOLTechnology::GetAttackStat(EHOLAttackType::Siege), FName(NAME_None));
	TestEqual(TEXT("hero hits: no technology"), HOLTechnology::GetAttackStat(EHOLAttackType::Hero), FName(NAME_None));
	const FHOLTechnologyRow* Blades1 = Find(TEXT("MeleeAttackI"));
	const FHOLTechnologyRow* Blades2 = Find(TEXT("MeleeAttackII"));
	TestTrue(TEXT("the table has Lames affutees I and II"), Blades1 && Blades2);
	Researched = { TEXT("MeleeAttackI") };
	TestEqual(TEXT("Lames affutees I: melee +10 %"), HOLTechnology::GetBonusPercent(Researched, HOLTechnology::GetAttackStat(EHOLAttackType::Melee), Find), Blades1 ? Blades1->Percent : -1);
	TestEqual(TEXT("Lames affutees I: nothing for the ranged"), HOLTechnology::GetBonusPercent(Researched, HOLTechnology::GetAttackStat(EHOLAttackType::Ranged), Find), 0);
	Researched.Add(TEXT("MeleeAttackII"));
	TestEqual(TEXT("Lames affutees I and II: II's total"), HOLTechnology::GetBonusPercent(Researched, HOLTechnology::GetAttackStat(EHOLAttackType::Melee), Find), Blades2 ? Blades2->Percent : -1);
	Researched = { TEXT("RangedAttackI") };
	const FHOLTechnologyRow* Fletching1 = Find(TEXT("RangedAttackI"));
	TestEqual(TEXT("Empennage I: ranged +10 %"), HOLTechnology::GetBonusPercent(Researched, HOLTechnology::GetAttackStat(EHOLAttackType::Ranged), Find), Fletching1 ? Fletching1->Percent : -1);
	TestEqual(TEXT("Empennage I: nothing for the melee"), HOLTechnology::GetBonusPercent(Researched, HOLTechnology::GetAttackStat(EHOLAttackType::Melee), Find), 0);

	// The player's state: a technology is researched once.
	FHOLTechnologyState State;
	TestFalse(TEXT("none yet"), State.Has(TEXT("PloughI")));
	TestTrue(TEXT("added"), State.Add(TEXT("PloughI")));
	TestTrue(TEXT("has it"), State.Has(TEXT("PloughI")));
	TestFalse(TEXT("added once"), State.Add(TEXT("PloughI")));
	TestEqual(TEXT("one entry"), State.Done.Num(), 1);
	return true;
}

#endif
