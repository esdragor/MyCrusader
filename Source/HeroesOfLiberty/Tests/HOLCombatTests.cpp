// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J7b.1 checks (J7-Q4): the pure rules of the melee (damage, cadence, reach, target choice), on the table's numbers.
namespace UE::HeroesOfLiberty::Tests::Combat
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A type of the unit table as the simulation sees it (its shared fragment); false when the row is missing. */
	bool GetType(FAutomationTestBase& Test, const FName Name, FHOLUnitTypeConstants& OutConstants)
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		const FHOLUnitTypeRow* Row = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(Name, TEXT("Combat test")) : nullptr;
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s in the unit type table"), *Name.ToString()), Row))
		{
			return false;
		}
		OutConstants = Row->ToConstants(Name);
		return true;
	}

	int32 StepMs()
	{
		return HOLCombat::ToMilliseconds(GetDefault<UHOLSimulationSettings>()->GetStepSeconds());
	}

	/** Steps from the start of an attack to each of the first HitCount hits. */
	TArray<int32> HitSteps(const int32 IntervalMs, const int32 HitCount)
	{
		TArray<int32> Steps;
		int32 Cooldown = HOLCombat::StartAttack(0, IntervalMs);
		for (int32 Step = 1; Steps.Num() < HitCount && Step < 100000; ++Step)
		{
			if (HOLCombat::TickAttack(Cooldown, StepMs(), IntervalMs))
			{
				Steps.Add(Step);
			}
		}
		return Steps;
	}

	HOLCombat::FCandidate Enemy(const uint64 Id, const double X, const double Y = 0.)
	{
		HOLCombat::FCandidate Candidate;
		Candidate.Id = Id;
		Candidate.Position = FVector2D(X, Y);
		return Candidate;
	}

	/** A soldier at the origin: body 60 cm, reach 50 cm, aggression 12 m (the table's Spearman). */
	HOLCombat::FSeeker Soldier(const HOLCombat::ESeekMode Mode = HOLCombat::ESeekMode::Auto, const float Aggression = 1200.f)
	{
		HOLCombat::FSeeker Seeker;
		Seeker.AggressionRadius = Aggression;
		Seeker.Mode = Mode;
		return Seeker;
	}

	/** The Id chosen, 0 for none. */
	uint64 Chosen(const HOLCombat::FSeeker& Seeker, TConstArrayView<HOLCombat::FCandidate> Candidates)
	{
		const int32 Index = HOLCombat::ChooseTarget(Seeker, Candidates);
		return Index == INDEX_NONE ? 0 : Candidates[Index].Id;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatDamageTest, "HeroesOfLiberty.Combat.Damage", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatDamageTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	FHOLUnitTypeConstants Spearman, ManAtArms, Peasant, Archer;
	if (!GetType(*this, TEXT("Spearman"), Spearman) || !GetType(*this, TEXT("ManAtArms"), ManAtArms)
		|| !GetType(*this, TEXT("Peasant"), Peasant) || !GetType(*this, TEXT("Archer"), Archer))
	{
		return false;
	}

	// The table reaches the shared fragment (J7a.1 numbers).
	TestEqual(TEXT("Spearman: 80 HP"), Spearman.MaxHealth, 80);
	TestEqual(TEXT("Spearman: attack 8"), Spearman.Attack, 8);
	TestEqual(TEXT("Spearman: every 1.875 s"), Spearman.AttackInterval, 1.875f);
	TestEqual(TEXT("Spearman: +17"), Spearman.BonusDamage, 17);
	TestEqual(TEXT("Spearman: against cavalry"), int32(Spearman.BonusCategory), int32(EHOLArmorCategory::Cavalry));
	TestEqual(TEXT("Spearman: armours 0 / 0"), Spearman.MeleeArmor + Spearman.RangedArmor, 0);
	TestEqual(TEXT("Spearman: aggression 12 m"), Spearman.AggressionRadius, 1200.f);
	TestEqual(TEXT("Man-at-arms: 155 HP"), ManAtArms.MaxHealth, 155);
	TestEqual(TEXT("Man-at-arms: every 1.375 s"), ManAtArms.AttackInterval, 1.375f);
	TestEqual(TEXT("Man-at-arms: melee armour 4"), ManAtArms.MeleeArmor, 4);
	TestEqual(TEXT("Man-at-arms: ranged armour 4"), ManAtArms.RangedArmor, 4);
	TestEqual(TEXT("Man-at-arms: heavy infantry"), int32(ManAtArms.ArmorCategory), int32(EHOLArmorCategory::HeavyInfantry));
	TestEqual(TEXT("Peasant: attack 3"), Peasant.Attack, 3);
	TestEqual(TEXT("Peasant: every 2 s"), Peasant.AttackInterval, 2.f);
	TestEqual(TEXT("Peasant: no aggression"), Peasant.AggressionRadius, 0.f);
	TestEqual(TEXT("Archer: reach 15 m"), Archer.AttackRange, 1500.f);

	// Damage = attack + bonus - armour, 1 at least (D21).
	TestEqual(TEXT("Spearman on Man-at-arms: 8 - 4"), HOLCombat::GetDamage(Spearman, ManAtArms, false), 4);
	TestEqual(TEXT("Man-at-arms on Spearman: 8 - 0"), HOLCombat::GetDamage(ManAtArms, Spearman, false), 8);
	TestEqual(TEXT("Spearman on Spearman: no bonus"), HOLCombat::GetDamage(Spearman, Spearman, false), 8);
	FHOLUnitTypeConstants Horseman;
	Horseman.ArmorCategory = EHOLArmorCategory::Cavalry;
	Horseman.MeleeArmor = 2;
	Horseman.RangedArmor = 5;
	TestEqual(TEXT("Spearman on cavalry: 8 + 17 - 2"), HOLCombat::GetDamage(Spearman, Horseman, false), 23);
	TestEqual(TEXT("Peasant on Man-at-arms: 3 - 4, 1 at least"), HOLCombat::GetDamage(Peasant, ManAtArms, false), 1);
	TestEqual(TEXT("Archer on Spearman: 5 + 5 (light infantry), ranged armour 0"), HOLCombat::GetDamage(Archer, Spearman, true), 10);
	TestEqual(TEXT("Archer on Man-at-arms: 5 - 4 (ranged armour)"), HOLCombat::GetDamage(Archer, ManAtArms, true), 1);
	TestEqual(TEXT("ranged hit takes the ranged armour"), HOLCombat::GetDamage(Spearman, Horseman, true), 20);
	FHOLUnitTypeConstants NoBonus = Spearman;
	NoBonus.BonusDamage = 0;
	TestEqual(TEXT("no bonus damage: no bonus"), HOLCombat::GetDamage(NoBonus, Horseman, false), 6);

	// Hits to kill (the matchup the user will see first).
	TestEqual(TEXT("Spearman kills a Man-at-arms in 39 hits"), HOLCombat::GetHitsToKill(ManAtArms.MaxHealth, HOLCombat::GetDamage(Spearman, ManAtArms, false)), 39);
	TestEqual(TEXT("Man-at-arms kills a Spearman in 10 hits"), HOLCombat::GetHitsToKill(Spearman.MaxHealth, HOLCombat::GetDamage(ManAtArms, Spearman, false)), 10);
	TestEqual(TEXT("Spearman kills a Spearman in 10 hits"), HOLCombat::GetHitsToKill(80, 8), 10);
	TestEqual(TEXT("one more HP, one more hit"), HOLCombat::GetHitsToKill(81, 8), 11);
	TestEqual(TEXT("dead: no hit"), HOLCombat::GetHitsToKill(0, 8), 0);
	TestEqual(TEXT("zero damage counts as 1"), HOLCombat::GetHitsToKill(3, 0), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatCadenceTest, "HeroesOfLiberty.Combat.Cadence", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatCadenceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	TestEqual(TEXT("fixed step of 50 ms"), StepMs(), 50);
	TestEqual(TEXT("1.875 s"), HOLCombat::ToMilliseconds(1.875f), 1875);
	TestEqual(TEXT("1.375 s"), HOLCombat::ToMilliseconds(1.375f), 1375);
	TestEqual(TEXT("wind-up: half the interval"), HOLCombat::GetWindUpMilliseconds(1875), 937);

	// Spearman: first hit after the wind-up (937 ms -> 19 steps), then 37 or 38 steps apart, exactly 1.875 s on average.
	const TArray<int32> Spear = HitSteps(1875, 9);
	if (TestEqual(TEXT("9 hits"), Spear.Num(), 9))
	{
		TestEqual(TEXT("Spearman: first hit at step 19 (0.95 s)"), Spear[0], 19);
		for (int32 Hit = 1; Hit < Spear.Num(); ++Hit)
		{
			const int32 Gap = Spear[Hit] - Spear[Hit - 1];
			TestTrue(FString::Printf(TEXT("Spearman: gap %d is 37 or 38 steps (%d)"), Hit, Gap), Gap == 37 || Gap == 38);
		}
		TestEqual(TEXT("Spearman: 8 intervals = 15 s = 300 steps, no drift"), Spear[8] - Spear[0], 300);
	}

	// Man-at-arms kills a Spearman: 10 hits, 0.6875 + 9 x 1.375 = 13.06 s, within one step.
	const TArray<int32> Arms = HitSteps(1375, 10);
	if (TestEqual(TEXT("10 hits"), Arms.Num(), 10))
	{
		const int32 Ms = Arms.Last() * StepMs();
		TestTrue(FString::Printf(TEXT("Man-at-arms kills a Spearman in 13.06 s (%d ms)"), Ms), Ms >= 13062 && Ms < 13062 + 50);
	}

	// A switch of target keeps the rhythm, never a free hit; idle runs the cool-down down to 0.
	TestEqual(TEXT("start: wind-up at least"), HOLCombat::StartAttack(100, 1375), 687);
	TestEqual(TEXT("start: keeps a longer wait"), HOLCombat::StartAttack(1000, 1375), 1000);
	int32 Cooldown = 120;
	HOLCombat::TickIdle(Cooldown, 50);
	TestEqual(TEXT("idle: runs down"), Cooldown, 70);
	HOLCombat::TickIdle(Cooldown, 50);
	HOLCombat::TickIdle(Cooldown, 50);
	TestEqual(TEXT("idle: stays at 0"), Cooldown, 0);

	// One hit per step at most, even with an interval shorter than the step.
	Cooldown = 0;
	int32 Hits = 0;
	for (int32 Step = 0; Step < 10; ++Step)
	{
		Hits += HOLCombat::TickAttack(Cooldown, 50, 10) ? 1 : 0;
	}
	TestEqual(TEXT("short interval: one hit per step"), Hits, 10);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatRangeTest, "HeroesOfLiberty.Combat.Range", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatRangeTest::RunTest(const FString& Parameters)
{
	const FVector2D Origin(0., 0.);
	TestEqual(TEXT("edge distance: 200 - 60 - 60"), HOLCombat::GetEdgeDistance(Origin, 60.f, FVector2D(200., 0.), 60.f), 80., 1e-9);
	TestTrue(TEXT("reach 50: edges 50 apart, in"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(170., 0.), 60.f, 50.f));
	TestFalse(TEXT("reach 50: edges 50.1 apart, out"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(170.1, 0.), 60.f, 50.f));
	TestTrue(TEXT("pushed into each other: in"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(100., 0.), 60.f, 50.f));
	TestTrue(TEXT("diagonal: in"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(120., 120.), 60.f, 50.f));
	TestFalse(TEXT("diagonal: out"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(121., 121.), 60.f, 50.f));
	TestTrue(TEXT("larger body reached from further"), HOLCombat::IsInRange(Origin, 60.f, FVector2D(400., 0.), 300.f, 50.f));
	TestTrue(TEXT("Archer: 15 m edge to edge"), HOLCombat::IsInRange(Origin, 52.f, FVector2D(1612., 0.), 60.f, 1500.f));
	TestFalse(TEXT("Archer: beyond 15 m"), HOLCombat::IsInRange(Origin, 52.f, FVector2D(1613., 0.), 60.f, 1500.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCombatTargetTest, "HeroesOfLiberty.Combat.Target", UE::HeroesOfLiberty::Tests::Combat::Flags)

bool FHOLCombatTargetTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Combat;
	using HOLCombat::ESeekMode;
	using HOLCombat::FCandidate;

	// Search radius.
	TestEqual(TEXT("search: aggression radius"), HOLCombat::GetSearchRadius(Soldier()), 1200.f);
	TestEqual(TEXT("search: none without aggression"), HOLCombat::GetSearchRadius(Soldier(ESeekMode::Auto, 0.f)), 0.f);
	TestEqual(TEXT("search: attack-move gives 12 m"), HOLCombat::GetSearchRadius(Soldier(ESeekMode::AttackMove, 0.f)), HOLCombat::AttackMoveRadius);
	TestEqual(TEXT("search: hold, the reach only"), HOLCombat::GetSearchRadius(Soldier(ESeekMode::Hold)), 50.f);

	TestEqual(TEXT("nobody: none"), Chosen(Soldier(), {}), uint64(0));
	TestEqual(TEXT("nearest edge to edge"), Chosen(Soldier(), { Enemy(1, 400.), Enemy(2, 300.) }), uint64(2));
	{
		TArray<FCandidate> Set = { Enemy(1, 160.), Enemy(2, 250.) };
		Set[1].bAttacksMe = true;
		TestEqual(TEXT("in reach before one attacking me out of reach"), Chosen(Soldier(), Set), uint64(1));
		Set[0].Position = FVector2D(300., 0.);
		Set[1].Position = FVector2D(600., 0.);
		TestEqual(TEXT("out of reach: one attacking me before a nearer one"), Chosen(Soldier(), Set), uint64(2));
	}
	{
		// Tie-break: same edge distance, smallest id, in any order.
		const TArray<FCandidate> Set = { Enemy(7, 300.), Enemy(3, -300.), Enemy(5, 0., 300.) };
		const TArray<FCandidate> Reversed = { Set[2], Set[1], Set[0] };
		TestEqual(TEXT("tie: smallest id"), Chosen(Soldier(), Set), uint64(3));
		TestEqual(TEXT("tie: same in reverse order"), Chosen(Soldier(), Reversed), uint64(3));
	}
	{
		// Aggression radius: 1380 cm edge to edge is beyond 12 m, unless it attacks me (riposte).
		TArray<FCandidate> Set = { Enemy(1, 1500.) };
		TestEqual(TEXT("beyond the aggression radius: none"), Chosen(Soldier(), Set), uint64(0));
		Set[0].bAttacksMe = true;
		TestEqual(TEXT("beyond, but attacking me: riposte"), Chosen(Soldier(), Set), uint64(1));
		Set[0].Position = FVector2D(150., 0.);
		TestEqual(TEXT("Peasant (no aggression): even attacked, none"), Chosen(Soldier(ESeekMode::Auto, 0.f), Set), uint64(0));
		TestEqual(TEXT("Peasant on an attack-move: takes it on"), Chosen(Soldier(ESeekMode::AttackMove, 0.f), Set), uint64(1));
	}
	{
		// Hold position: in reach only, never a chase, even when attacked from further.
		TArray<FCandidate> Set = { Enemy(1, 300.) };
		Set[0].bAttacksMe = true;
		TestEqual(TEXT("hold: out of reach, attacking me, none"), Chosen(Soldier(ESeekMode::Hold), Set), uint64(0));
		Set.Add(Enemy(2, 165.));
		TestEqual(TEXT("hold: the one in reach"), Chosen(Soldier(ESeekMode::Hold), Set), uint64(2));
		TestEqual(TEXT("hold: Peasant hits back in reach"), Chosen(Soldier(ESeekMode::Hold, 0.f), Set), uint64(2));
	}
	{
		// The current target in reach is kept; out of reach, the choice is made again.
		TArray<FCandidate> Set = { Enemy(1, 130.), Enemy(2, 165.) };
		Set[1].bIsCurrentTarget = true;
		TestEqual(TEXT("current target in reach: kept"), Chosen(Soldier(), Set), uint64(2));
		Set[1].Position = FVector2D(400., 0.);
		TestEqual(TEXT("current target out of reach: the one in reach"), Chosen(Soldier(), Set), uint64(1));
	}
	{
		// A unit before a building (hook), even a building in reach.
		TArray<FCandidate> Set = { Enemy(1, 150.), Enemy(2, 600.) };
		Set[0].bIsBuilding = true;
		TestEqual(TEXT("unit before building"), Chosen(Soldier(), Set), uint64(2));
		Set.RemoveAt(1);
		TestEqual(TEXT("a building alone is taken on"), Chosen(Soldier(), Set), uint64(1));
	}
	{
		// Spreading: out of reach, a crowded enemy comes after a free one further away; in reach, never passed over.
		TArray<FCandidate> Set = { Enemy(1, 300.), Enemy(2, 500.) };
		Set[0].AttackerCount = HOLCombat::MaxAttackers;
		TestEqual(TEXT("spreading: the free one"), Chosen(Soldier(), Set), uint64(2));
		Set[0].AttackerCount = HOLCombat::MaxAttackers - 1;
		TestEqual(TEXT("spreading: room left, the nearest"), Chosen(Soldier(), Set), uint64(1));
		Set[0].AttackerCount = HOLCombat::MaxAttackers + 2;
		Set[0].Position = FVector2D(160., 0.);
		TestEqual(TEXT("spreading: crowded but in reach, taken"), Chosen(Soldier(), Set), uint64(1));
	}
	{
		// Determinism: 40 enemies (ties included), shuffled 20 times, always the same one.
		FRandomStream Random(7);
		TArray<FCandidate> Set;
		for (int32 Index = 0; Index < 40; ++Index)
		{
			FCandidate Candidate = Enemy(100 + Index, 200. + 100. * Random.RandRange(0, 9), 100. * Random.RandRange(-5, 5));
			Candidate.bAttacksMe = Random.RandRange(0, 3) == 0;
			Candidate.AttackerCount = Random.RandRange(0, 8);
			Set.Add(Candidate);
		}
		const uint64 First = Chosen(Soldier(), Set);
		TestTrue(TEXT("determinism: one is chosen"), First != 0);
		bool bSame = true;
		for (int32 Shuffle = 0; Shuffle < 20; ++Shuffle)
		{
			for (int32 Index = Set.Num() - 1; Index > 0; --Index)
			{
				Set.Swap(Index, Random.RandRange(0, Index));
			}
			bSame &= Chosen(Soldier(), Set) == First;
		}
		TestTrue(TEXT("determinism: same choice in any order"), bSame);
	}
	return true;
}

#endif
