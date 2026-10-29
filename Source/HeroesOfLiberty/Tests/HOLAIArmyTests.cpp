// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA2.1 checks (IA2-Q1 A): the slot's army and the enemies in the AI summary, the rule alone (Make fed by hand), then read
// from a test world through the real spawn path (HOLAISummary::Read -> GatherEnemies, the one place the others' units are read).
namespace UE::HeroesOfLiberty::Tests::AIArmy
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));
	const FName Spearman(TEXT("Spearman"));
	const FName ManAtArms(TEXT("ManAtArms"));
	const FName Archer(TEXT("Archer"));

	FHOLUnitView MakeUnit(const uint32 Id, const uint8 Owner, const FName Type, const EHOLUnitActivity Activity = EHOLUnitActivity::Idle,
		const FVector& Location = FVector::ZeroVector, const int32 Health = 100)
	{
		FHOLUnitView Unit;
		Unit.UnitId = Id;
		Unit.OwnerPlayer = Owner;
		Unit.TypeName = Type;
		Unit.CarryCapacity = Type == Peasant ? 10 : 0;
		Unit.Activity = Activity;
		Unit.Location = Location;
		Unit.Health = Health;
		return Unit;
	}

	FHOLAIBuilding MakeCentre(const int32 Owner, const double X, const double Y)
	{
		FHOLAIBuilding Centre;
		Centre.OwnerPlayer = Owner;
		Centre.TypeName = HOLBuildingTypeNames::MainCentre;
		Centre.Location = FVector(X, Y, 0.0);
		return Centre;
	}

	/** "3 7": the identifiers of a unit list, in order. */
	FString JoinUnits(const TArray<FHOLAIUnit>& List)
	{
		return FString::JoinBy(List, TEXT(" "), [](const FHOLAIUnit& Unit) { return FString::FromInt(Unit.Id); });
	}

	/** "Spearman Archer": the types of a unit list, in order. */
	FString JoinTypes(const TArray<FHOLAIUnit>& List)
	{
		return FString::JoinBy(List, TEXT(" "), [](const FHOLAIUnit& Unit) { return Unit.Type.ToString(); });
	}

	/** Units of Type born for Owner around Location (no walk: the test world runs no movement); their identifiers. */
	TArray<uint32> PlaceUnits(UHOLUnitSubsystem& Units, const FName Type, const int32 Owner, const FVector& Location, const int32 Count)
	{
		TArray<uint32> Before;
		Units.GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Type;
		Request.Count = Count;
		Request.OwnerPlayer = static_cast<uint8>(Owner);
		Request.Center = Location;
		Units.RequestUnits(Request);
		Units.ServeBirthRequests();
		TArray<uint32> After;
		Units.GetAllUnitIds(After);
		After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
		After.Sort();
		return After;
	}
}

// The army: the slot's soldiers by identifier (ascending), with their type, place, health, idleness and target; not its
// Peasants, its King, its hero, a dying soldier, nor another slot's soldiers.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIArmyRuleTest, "HeroesOfLiberty.AI.Summary.ArmyRule", UE::HeroesOfLiberty::Tests::AIArmy::Flags)
bool FHOLAIArmyRuleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIArmy;
	FHOLUnitView King = MakeUnit(20, 0, TEXT("King"));
	King.bKing = true;
	FHOLUnitView Hero = MakeUnit(21, 0, TEXT("Hero"));
	Hero.bHero = true;
	FHOLUnitView Fighter = MakeUnit(9, 0, ManAtArms, EHOLUnitActivity::Attacking, FVector(500.0, 600.0, 0.0), 40);
	Fighter.TargetUnitId = 31;
	const TArray<FHOLUnitView> Units = {
		MakeUnit(12, 0, Archer, EHOLUnitActivity::Moving),
		MakeUnit(4, 0, Spearman, EHOLUnitActivity::Idle, FVector(100.0, 200.0, 0.0), 75),
		MakeUnit(3, 0, Peasant, EHOLUnitActivity::Idle),
		Fighter,
		MakeUnit(10, 0, Spearman, EHOLUnitActivity::Dying),
		King,
		Hero,
		MakeUnit(30, 1, Spearman, EHOLUnitActivity::Idle),
	};
	const FHOLAISummary Slot0 = HOLAISummary::Make(0, Units, {}, {}, {});
	TestEqual(TEXT("slot 0's army: its three soldiers, ascending, no Peasant, King, hero, dying nor slot 1's"), JoinUnits(Slot0.Army), FString(TEXT("4 9 12")));
	TestEqual(TEXT("slot 0's army: the types"), JoinTypes(Slot0.Army), FString(TEXT("Spearman ManAtArms Archer")));
	if (Slot0.Army.Num() == 3)
	{
		TestEqual(TEXT("the Spearman: its owner"), Slot0.Army[0].OwnerSlot, 0);
		TestEqual(TEXT("the Spearman: its health"), Slot0.Army[0].Health, 75);
		TestTrue(TEXT("the Spearman: its place"), Slot0.Army[0].Location.Equals(FVector(100.0, 200.0, 0.0)));
		TestTrue(TEXT("the Spearman stands: idle"), Slot0.Army[0].bIdle);
		TestEqual(TEXT("the Spearman: no target"), Slot0.Army[0].TargetId, static_cast<int32>(INDEX_NONE));
		TestFalse(TEXT("the Man-at-arms fights: not idle"), Slot0.Army[1].bIdle);
		TestEqual(TEXT("the Man-at-arms: its target"), Slot0.Army[1].TargetId, 31);
		TestFalse(TEXT("the Archer walks: not idle"), Slot0.Army[2].bIdle);
	}
	const FHOLAISummary Slot1 = HOLAISummary::Make(1, Units, {}, {}, {});
	TestEqual(TEXT("slot 1's army: its own Spearman"), JoinUnits(Slot1.Army), FString(TEXT("30")));
	TestEqual(TEXT("without enemies given, none are read from the units"), Slot0.Enemies.Num(), 0);
	TestFalse(TEXT("without buildings, no enemy centre"), Slot0.bHasEnemyCentre);
	TestTrue(TEXT("ToString tells the army and the enemies"), Slot0.ToString().Contains(TEXT("army 3, enemies 0")));
	return true;
}

// The enemies and the enemy centre, the rule alone: the units given by GatherEnemies, kept for another slot, not defeated
// (DefeatedMask), not dying, ascending, Peasants included; the nearest enemy centre to the slot's own (a tie: the lower slot).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIEnemiesRuleTest, "HeroesOfLiberty.AI.Summary.EnemiesRule", UE::HeroesOfLiberty::Tests::AIArmy::Flags)
bool FHOLAIEnemiesRuleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIArmy;
	const TArray<FHOLUnitView> Enemies = {
		MakeUnit(15, 2, Spearman, EHOLUnitActivity::Idle),
		MakeUnit(7, 1, Peasant, EHOLUnitActivity::Gathering, FVector(-900.0, 0.0, 0.0), 30),
		MakeUnit(8, 1, Archer, EHOLUnitActivity::Attacking, FVector(-800.0, 50.0, 0.0), 55),
		MakeUnit(9, 1, Spearman, EHOLUnitActivity::Dying),
		MakeUnit(11, 0, Spearman, EHOLUnitActivity::Idle),
		MakeUnit(20, 3, ManAtArms, EHOLUnitActivity::Idle),
	};
	const TArray<FHOLAIBuilding> Buildings = {
		MakeCentre(0, 0.0, 0.0),
		MakeCentre(1, -10000.0, 0.0),
		MakeCentre(2, 6000.0, 0.0),
		MakeCentre(3, 2000.0, 0.0),
	};
	const uint8 Slot3Defeated = 1 << 3;
	const FHOLAISummary Slot0 = HOLAISummary::Make(0, {}, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, Buildings, {}, {}, Enemies, Slot3Defeated);
	TestEqual(TEXT("enemies: slots 1 and 2, ascending; not its own, not the dying, not the defeated slot 3's"), JoinUnits(Slot0.Enemies), FString(TEXT("7 8 15")));
	TestEqual(TEXT("enemies: the types, the Peasant included"), JoinTypes(Slot0.Enemies), FString(TEXT("Peasant Archer Spearman")));
	if (Slot0.Enemies.Num() == 3)
	{
		TestEqual(TEXT("the Peasant: its owner"), Slot0.Enemies[0].OwnerSlot, 1);
		TestEqual(TEXT("the Archer: its health"), Slot0.Enemies[1].Health, 55);
		TestTrue(TEXT("the Archer: its place"), Slot0.Enemies[1].Location.Equals(FVector(-800.0, 50.0, 0.0)));
		TestFalse(TEXT("the Archer fights: not idle"), Slot0.Enemies[1].bIdle);
		TestTrue(TEXT("the Spearman of slot 2 stands: idle"), Slot0.Enemies[2].bIdle);
		TestEqual(TEXT("the Spearman: its owner"), Slot0.Enemies[2].OwnerSlot, 2);
	}
	TestTrue(TEXT("the enemy centre is known"), Slot0.bHasEnemyCentre);
	// Slot 3's centre is nearer (2 000 cm) but slot 3 is defeated: slot 2's (6 000 cm) is the nearest left, slot 1's is farther.
	TestTrue(TEXT("the nearest centre of a slot not defeated: slot 2's"), Slot0.EnemyCentreLocation.Equals(FVector(6000.0, 0.0, 0.0)));
	TestTrue(TEXT("ToString tells the enemies"), Slot0.ToString().Contains(TEXT("enemies 3")));

	// A tie: the lower slot's centre.
	const TArray<FHOLAIBuilding> Tied = { MakeCentre(0, 0.0, 0.0), MakeCentre(2, 0.0, 5000.0), MakeCentre(1, 0.0, -5000.0) };
	const FHOLAISummary TiedSummary = HOLAISummary::Make(0, {}, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, Tied, {}, {}, {}, 0);
	TestTrue(TEXT("a tie: the lower slot's centre"), TiedSummary.bHasEnemyCentre && TiedSummary.EnemyCentreLocation.Equals(FVector(0.0, -5000.0, 0.0)));

	// Without a centre of its own, the lowest slot's; without any enemy centre (a slot's own only), none.
	const TArray<FHOLAIBuilding> NoOwn = { MakeCentre(2, 100.0, 0.0), MakeCentre(1, 9000.0, 0.0) };
	const FHOLAISummary NoOwnSummary = HOLAISummary::Make(0, {}, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, NoOwn, {}, {}, {}, 0);
	TestTrue(TEXT("no centre of its own: the lowest slot's"), NoOwnSummary.bHasEnemyCentre && NoOwnSummary.EnemyCentreLocation.Equals(FVector(9000.0, 0.0, 0.0)));
	const TArray<FHOLAIBuilding> OnlyOwn = { MakeCentre(0, 0.0, 0.0) };
	TestFalse(TEXT("only its own centre: no enemy centre"), HOLAISummary::Make(0, {}, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, OnlyOwn, {}, {}, {}, 0).bHasEnemyCentre);
	return true;
}

// The army read from a test world through the real spawn path: three Peasants, two Spearmen and an Archer of slot 0
// (and a Spearman of slot 1): slot 0's army is its three soldiers, with the right types.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIArmyTest, "HeroesOfLiberty.AI.Summary.Army", UE::HeroesOfLiberty::Tests::AIArmy::Flags)
bool FHOLAIArmyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIArmy;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	if (TestNotNull(TEXT("units"), Units))
	{
		const TArray<uint32> Peasants = PlaceUnits(*Units, Peasant, 0, FVector(1000.0, 1000.0, 0.0), 3);
		const TArray<uint32> Spearmen = PlaceUnits(*Units, Spearman, 0, FVector(-1000.0, 1000.0, 0.0), 2);
		const TArray<uint32> Archers = PlaceUnits(*Units, Archer, 0, FVector(-1000.0, -1000.0, 0.0), 1);
		const TArray<uint32> Other = PlaceUnits(*Units, Spearman, 1, FVector(-9000.0, 0.0, 0.0), 1);
		TestEqual(TEXT("three Peasants born"), Peasants.Num(), 3);
		TestEqual(TEXT("two Spearmen born"), Spearmen.Num(), 2);
		TestEqual(TEXT("an Archer born"), Archers.Num(), 1);
		TestEqual(TEXT("a Spearman of slot 1 born"), Other.Num(), 1);

		const FHOLAISummary Slot0 = HOLAISummary::Read(*World, 0);
		TestEqual(TEXT("slot 0's army: its two Spearmen then its Archer (by identifier), no Peasant, not slot 1's"),
			JoinTypes(Slot0.Army), FString(TEXT("Spearman Spearman Archer")));
		TestEqual(TEXT("slot 0's army: the identifiers born"), JoinUnits(Slot0.Army),
			FString::JoinBy(Spearmen, TEXT(" "), [](const uint32 Id) { return FString::FromInt(static_cast<int32>(Id)); }) + TEXT(" ")
			+ (Archers.IsEmpty() ? FString() : FString::FromInt(static_cast<int32>(Archers[0]))));
		for (const FHOLAIUnit& Soldier : Slot0.Army)
		{
			TestTrue(TEXT("a soldier born: alive with health points, standing"), Soldier.Health > 0 && Soldier.bIdle && Soldier.OwnerSlot == 0
				&& Soldier.TargetId == INDEX_NONE);
		}
		TestEqual(TEXT("the Peasants stay in PeasantIds"), Slot0.PeasantIds.Num(), 3);
		AddInfo(Slot0.ToString());
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// The enemies read from a test world (HOLAISummary::GatherEnemies): slots 0 (human), 1 and 2 (AI), each a finished centre
// and units; slot 2 defeated. Slot 0 sees slot 1's units (the Peasant too) and its centre, not its own, not slot 2's;
// slot 1 sees slot 0's. Slot 2's units and centre are left out while it is defeated.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIEnemiesTest, "HeroesOfLiberty.AI.Summary.Enemies", UE::HeroesOfLiberty::Tests::AIArmy::Flags)
bool FHOLAIEnemiesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIArmy;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	AHOLBuilding* Centre0 = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Centre1 = World->SpawnActor<AHOLBuilding>(FVector(-10000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Centre2 = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("centre 0"), Centre0)
		&& TestNotNull(TEXT("centre 1"), Centre1) && TestNotNull(TEXT("centre 2"), Centre2))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index <= 2 ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
		Centre0->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Centre1->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		Centre2->Setup(HOLBuildingTypeNames::MainCentre, 2, 2);

		const TArray<uint32> Own = PlaceUnits(*Units, Spearman, 0, FVector(1000.0, 0.0, 0.0), 2);
		const TArray<uint32> Peasants1 = PlaceUnits(*Units, Peasant, 1, FVector(-9000.0, 1000.0, 0.0), 1);
		const TArray<uint32> Archers1 = PlaceUnits(*Units, Archer, 1, FVector(-9000.0, -1000.0, 0.0), 2);
		const TArray<uint32> Slot2Units = PlaceUnits(*Units, ManAtArms, 2, FVector(2500.0, 0.0, 0.0), 1);
		TestEqual(TEXT("units born"), Own.Num() + Peasants1.Num() + Archers1.Num() + Slot2Units.Num(), 6);

		// Slot 2 still fights: slot 0 sees all the others' units and slot 2's centre, the nearest.
		FHOLAISummary Slot0 = HOLAISummary::Read(*World, 0);
		TestEqual(TEXT("slot 0 sees the others' units, ascending, not its own"), Slot0.Enemies.Num(), 4);
		for (const FHOLAIUnit& Enemy : Slot0.Enemies)
		{
			TestTrue(TEXT("an enemy of another slot"), Enemy.OwnerSlot == 1 || Enemy.OwnerSlot == 2);
			TestTrue(TEXT("an enemy: alive, a place and a type"), Enemy.Health > 0 && !Enemy.Type.IsNone());
		}
		TestTrue(TEXT("the nearest enemy centre: slot 2's"), Slot0.bHasEnemyCentre && Slot0.EnemyCentreLocation.Equals(FVector(3000.0, 0.0, 0.0), 1.0));

		// Slot 2 defeated: its units and centre are left out.
		TestTrue(TEXT("slot 2 defeated"), State->DefeatSlot(2, EHOLDefeatReason::Surrender));
		Slot0 = HOLAISummary::Read(*World, 0);
		TArray<FString> Seen;
		for (const FHOLAIUnit& Enemy : Slot0.Enemies)
		{
			TestEqual(TEXT("only slot 1 left"), Enemy.OwnerSlot, 1);
			Seen.Add(Enemy.Type.ToString());
		}
		TestEqual(TEXT("slot 0 sees slot 1's three units (the Peasant too)"), Slot0.Enemies.Num(), 3);
		TestEqual(TEXT("the types, by identifier"), FString::Join(Seen, TEXT(" ")), FString(TEXT("Peasant Archer Archer")));
		TestTrue(TEXT("the enemy centre: slot 1's"), Slot0.bHasEnemyCentre && Slot0.EnemyCentreLocation.Equals(FVector(-10000.0, 0.0, 0.0), 1.0));

		const FHOLAISummary Slot1 = HOLAISummary::Read(*World, 1);
		TestEqual(TEXT("slot 1 sees slot 0's two Spearmen, not its own"), JoinTypes(Slot1.Enemies), FString(TEXT("Spearman Spearman")));
		TestTrue(TEXT("slot 1: the enemy centre is slot 0's"), Slot1.bHasEnemyCentre && Slot1.EnemyCentreLocation.Equals(FVector(0.0, 0.0, 0.0), 1.0));
		TestEqual(TEXT("slot 1's own army: its two Archers"), JoinTypes(Slot1.Army), FString(TEXT("Archer Archer")));
		AddInfo(Slot0.ToString());
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
