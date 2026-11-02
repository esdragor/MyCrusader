// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLHealRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "Technologies/HOLTechnologyRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// Q-chapelle-moine = A (D636): the Monk, the common healer (GDD 7.1, D134, D212). Its row of the real unit table
// (/Game/Units/UnitTypes, imported from Tools/Data/UnitTypes.csv). The numbers are the user's (100 food + 100 gold, 80 health, the
// Spearman's speed, a heal of 5 health per second within 8 m, light infantry); the production time is provisional (to balance at the trial).
namespace UE::HeroesOfLiberty::Tests::Monk
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMonkTableTest, "HeroesOfLiberty.Monk.Table", UE::HeroesOfLiberty::Tests::Monk::Flags)
bool FHOLMonkTableTest::RunTest(const FString& Parameters)
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		return false;
	}
	const FHOLUnitTypeRow* Monk = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Monk"), TEXT("Monk test"), false);
	const FHOLUnitTypeRow* Spearman = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Spearman"), TEXT("Monk test"), false);
	if (!TestNotNull(TEXT("Monk in the unit type table"), Monk) || !TestNotNull(TEXT("Spearman in the unit type table"), Spearman))
	{
		return false;
	}
	TestTrue(TEXT("Monk: 100 food + 100 gold"), Monk->Cost.Food == 100 && Monk->Cost.Gold == 100 && Monk->Cost.Wood == 0 && Monk->Cost.Stone == 0);
	TestEqual(TEXT("Monk: 80 health"), Monk->MaxHealth, 80);
	TestEqual(TEXT("Monk: the Spearman's speed"), Monk->Speed, Spearman->Speed);
	TestEqual(TEXT("Monk: one place"), Monk->Population, 1);
	TestTrue(TEXT("Monk: light infantry"), Monk->ArmorCategories == TArray<EHOLArmorCategory>({ EHOLArmorCategory::LightInfantry }));
	TestEqual(TEXT("Monk: heals 5 health per second"), Monk->HealRate, 5.f);
	TestEqual(TEXT("Monk: within 8 m"), Monk->HealRange, 800.f);
	TestEqual(TEXT("Monk: no attack on units (a healer)"), Monk->Attack, 0);
	TestEqual(TEXT("Monk: no attack on buildings"), Monk->BuildingAttack, 0);
	TestEqual(TEXT("Monk: no aggression (it takes nobody on)"), Monk->AggressionRadius, 0.f);
	TestFalse(TEXT("Monk: no riposte"), Monk->Ripostes);
	TestTrue(TEXT("Monk: a production time"), Monk->BuildTime > 0.f);
	TestTrue(TEXT("Monk: sees at least as far as the Spearman"), Monk->SightRadius >= Spearman->SightRadius);

	// No other type heals (the column is new): the Monk alone.
	UnitTypes->ForeachRow<FHOLUnitTypeRow>(TEXT("Monk test"), [this](const FName& Name, const FHOLUnitTypeRow& Row)
	{
		if (Name != FName(TEXT("Monk")))
		{
			TestEqual(*FString::Printf(TEXT("%s: heals nobody"), *Name.ToString()), Row.HealRate, 0.f);
		}
	});
	return true;
}

// The heal's rules (HOLHeal), pure: one ally at a time, 8 m, never itself, never a King or a siege engine, 5 health per second.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHealRulesTest, "HeroesOfLiberty.Monk.HealRules", UE::HeroesOfLiberty::Tests::Monk::Flags)
bool FHOLHealRulesTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeal;
	const auto Make = [](const uint32 Id, const double X, const float Health, const uint8 Camp = 1)
	{
		FCandidate Candidate;
		Candidate.UnitId = Id;
		Candidate.Location = FVector2D(X, 0.);
		Candidate.Health = Health;
		Candidate.MaxHealth = 100.f;
		Candidate.Camp = Camp;
		return Candidate;
	};
	constexpr uint32 Monk = 1;
	constexpr float Range = 800.f;
	const FVector2D Here(0., 0.);

	// The Monk itself is wounded and alone: it heals nobody (no self-heal).
	TArray<FCandidate> Alone = { Make(Monk, 0., 40.f) };
	TestEqual(TEXT("a wounded Monk alone heals nobody: no self-heal"), ChooseTarget(Monk, 1, Here, Range, 0u, 0u, Alone), int32(INDEX_NONE));
	TestEqual(TEXT("... even if it kept itself as its target"), ChooseTarget(Monk, 1, Here, Range, 0u, Monk, Alone), int32(INDEX_NONE));

	// An ally in reach and wounded is the target; a healthy one, an enemy, a King, a siege engine or a dying unit is not.
	TArray<FCandidate> Mixed = { Make(Monk, 0., 40.f), Make(2, 300., 100.f), Make(3, 300., 50.f, 2), Make(4, 300., 50.f), Make(5, 300., 50.f), Make(6, 300., 0.f), Make(7, 300., 60.f) };
	Mixed[3].bUnhealable = true; // the King (HOLCanBeHealed) or an engine
	Mixed[4].bUnhealable = true;
	TestEqual(TEXT("the wounded ally is the target (not a healthy one, an enemy, a King, an engine, a dead unit)"), ChooseTarget(Monk, 1, Here, Range, 0u, 0u, Mixed), 6);
	TestFalse(TEXT("a healthy ally is not healable"), IsHealable(Monk, 1, Mixed[1]));
	TestFalse(TEXT("an enemy is not healable"), IsHealable(Monk, 1, Mixed[2]));
	TestFalse(TEXT("an unhealable unit is not healable"), IsHealable(Monk, 1, Mixed[3]));
	TestFalse(TEXT("a dead unit is not healable"), IsHealable(Monk, 1, Mixed[5]));
	TestTrue(TEXT("a wounded ally is healable"), IsHealable(Monk, 1, Mixed[6]));
	TestFalse(TEXT("the healer is not healable by itself"), IsHealable(Monk, 1, Mixed[0]));

	// 8 m, centre to centre: 800 is in, 801 out.
	TArray<FCandidate> Edge = { Make(Monk, 0., 100.f), Make(2, 800., 50.f), Make(3, 801., 50.f) };
	TestEqual(TEXT("an ally at 8 m is healed"), ChooseTarget(Monk, 1, Here, Range, 0u, 0u, TConstArrayView<FCandidate>(&Edge[0], 2)), 1);
	TestEqual(TEXT("an ally past 8 m is not"), ChooseTarget(Monk, 1, Here, Range, 0u, 0u, TConstArrayView<FCandidate>(&Edge[2], 1)), int32(INDEX_NONE));

	// The nearest, the lowest identifier on a tie, and the kept target (one at a time) before a nearer one.
	TArray<FCandidate> Two = { Make(Monk, 0., 100.f), Make(8, 500., 50.f), Make(9, 200., 50.f), Make(10, -200., 50.f) };
	TestEqual(TEXT("the nearest wounded ally, the lowest identifier on a tie"), ChooseTarget(Monk, 1, Here, Range, 0u, 0u, Two), 2);
	TestEqual(TEXT("the target it healed before is kept while healable and in reach, though another is nearer"), ChooseTarget(Monk, 1, Here, Range, 0u, 8u, Two), 1);
	Two[1].Health = 100.f;
	TestEqual(TEXT("the kept target healed to full: the nearest wounded one, the next"), ChooseTarget(Monk, 1, Here, Range, 0u, 8u, Two), 2);
	Two[2].Location = FVector2D(5000., 0.);
	TestEqual(TEXT("the kept target out of reach: another"), ChooseTarget(Monk, 1, Here, Range, 0u, 9u, Two), 3);

	// An order (a right click on an ally): the ordered unit first, though another is nearer or kept; ignored out of reach or healed.
	Two[1].Health = 50.f;
	Two[2].Location = FVector2D(200., 0.);
	TestEqual(TEXT("the ordered ally, though another is nearer"), ChooseTarget(Monk, 1, Here, Range, 8u, 0u, Two), 1);
	TestEqual(TEXT("the ordered ally, though the kept one is another"), ChooseTarget(Monk, 1, Here, Range, 8u, 9u, Two), 1);
	TestEqual(TEXT("the kept one without an order"), ChooseTarget(Monk, 1, Here, Range, 0u, 8u, Two), 1);
	Two[1].Location = FVector2D(5000., 0.);
	TestEqual(TEXT("the ordered ally out of reach: the nearest, the order waits"), ChooseTarget(Monk, 1, Here, Range, 8u, 0u, Two), 2);
	Two[1].Location = FVector2D(500., 0.);
	Two[1].Health = 100.f;
	TestEqual(TEXT("the ordered ally already healed: the nearest"), ChooseTarget(Monk, 1, Here, Range, 8u, 0u, Two), 2);
	Two[1].Health = 50.f;
	Two[1].Camp = 2;
	TestEqual(TEXT("an enemy cannot be ordered healed"), ChooseTarget(Monk, 1, Here, Range, 8u, 0u, Two), 2);
	// 5 health per second on 50 ms steps: 0.25 a step, 5 after 20 steps, held at the maximum.
	float Health = 40.f;
	for (int32 Step = 0; Step < 20; ++Step)
	{
		Health = GetHealedHealth(Health, 100.f, 5.f, 50);
	}
	TestEqual(TEXT("5 health per second: one second of steps heals 5"), Health, 45.f, 0.001f);
	TestEqual(TEXT("held at the maximum"), GetHealedHealth(99.9f, 100.f, 5.f, 50), 100.f);
	return true;
}

// A client's guess of a heal (HOLHeal::FRiseTracker): rises noticed, several expiring in the same frame, units gone, no entry lost or kept by error.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRiseTrackerTest, "HeroesOfLiberty.Monk.RiseTracker", UE::HeroesOfLiberty::Tests::Monk::Flags)
bool FHOLRiseTrackerTest::RunTest(const FString& Parameters)
{
	using FUnits = TArray<TTuple<uint32, int32, bool>>;
	HOLHeal::FRiseTracker Tracker;
	FUnits Units;
	for (uint32 Id = 1; Id <= 40; ++Id)
	{
		Units.Add(MakeTuple(Id, 10, true));
	}
	Tracker.Update(Units, 0.0, 5.0);
	TestTrue(TEXT("nothing rose at the first look"), Tracker.IsEmpty());
	// Forty units rise at once at t = 1.
	for (TTuple<uint32, int32, bool>& Unit : Units)
	{
		Unit.Get<1>() += 1;
	}
	Tracker.Update(Units, 1.0, 5.0);
	TestEqual(TEXT("forty rises noticed"), Tracker.NumRises(), 40);
	TestTrue(TEXT("rising within the window"), Tracker.IsRising(7, 1.4, 0.6));
	TestFalse(TEXT("not rising past the window"), Tracker.IsRising(7, 1.7, 0.6));
	TestFalse(TEXT("an unknown unit does not rise"), Tracker.IsRising(999, 1.0, 0.6));
	// Half of them rise again at t = 3, the others stay: all the 40 expire together at t = 6.5 except those risen again.
	for (TTuple<uint32, int32, bool>& Unit : Units)
	{
		Unit.Get<1>() += Unit.Get<0>() % 2 == 0 ? 1 : 0;
	}
	Tracker.Update(Units, 3.0, 5.0);
	TestEqual(TEXT("still forty remembered"), Tracker.NumRises(), 40);
	Tracker.Update(Units, 6.5, 5.0);
	TestEqual(TEXT("the twenty that rose at t = 1 and not again are forgotten in one frame, the twenty that rose at t = 3 stay"), Tracker.NumRises(), 20);
	TestTrue(TEXT("a unit that rose at t = 3 is still known"), !Tracker.IsRising(2, 6.5, 0.6) && Tracker.NumRises() == 20);
	// Units gone (dead, out of sight) are forgotten with their rise.
	Units.RemoveAll([](const TTuple<uint32, int32, bool>& Unit) { return Unit.Get<0>() <= 30; });
	Tracker.Update(Units, 6.6, 5.0);
	TestEqual(TEXT("only the units still there keep a rise (even ids from 32 to 40: five)"), Tracker.NumRises(), 5);
	// A health that falls or an unknown maximum is no rise.
	Units.Empty();
	Units.Add(MakeTuple(100u, 50, true));
	Units.Add(MakeTuple(101u, 50, false));
	Tracker.Update(Units, 7.0, 5.0);
	Units[0].Get<1>() = 40;
	Units[1].Get<1>() = 60;
	Tracker.Update(Units, 7.1, 5.0);
	TestFalse(TEXT("a falling health is not healed"), Tracker.IsRising(100, 7.1, 0.6));
	TestFalse(TEXT("an unknown maximum is not a rise"), Tracker.IsRising(101, 7.1, 0.6));
	return true;
}

// The Vow of Poverty (GDD 11.1, table: MonkCostDiscount 20 %): the Monk costs less, and only the Monk.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMonkDiscountTest, "HeroesOfLiberty.Monk.CostDiscount", UE::HeroesOfLiberty::Tests::Monk::Flags)
bool FHOLMonkDiscountTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("the Monk's cost is lowered by MonkCostDiscount"), HOLTechnology::GetCostDiscountStat(TEXT("Monk")), FName(TEXT("MonkCostDiscount")));
	TestTrue(TEXT("no other unit's is"), HOLTechnology::GetCostDiscountStat(TEXT("Spearman")).IsNone() && HOLTechnology::GetCostDiscountStat(TEXT("Peasant")).IsNone());
	const FHOLResources Cost(100, 0, 0, 100);
	TestTrue(TEXT("20 %: 80 food + 80 gold"), HOLTechnology::GetDiscountedCost(Cost, 20) == FHOLResources(80, 0, 0, 80));
	TestTrue(TEXT("0 %: unchanged"), HOLTechnology::GetDiscountedCost(Cost, 0) == Cost);
	TestTrue(TEXT("a free resource stays free"), HOLTechnology::GetDiscountedCost(FHOLResources(0, 0, 0, 50), 20).Food == 0);
	TestTrue(TEXT("rounded to the nearest unit: 15 % of 50 = 42.5 -> 43"), HOLTechnology::GetDiscountedCost(FHOLResources(50, 0, 0, 0), 15).Food == 43);
	TestTrue(TEXT("never below 10 % of the cost"), HOLTechnology::GetDiscountedCost(Cost, 400).Food == 10);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
