// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAISummary.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA3.1 checks (IA3-Q1 A): the main forums (lives, phase, ruin, hold), the King and the Regicide flag in the AI summary, the
// rule alone (Make fed by hand), then read from a test world where a forum really falls (HOLAISummary::Read -> GatherForums).
namespace UE::HeroesOfLiberty::Tests::AIForum
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FHOLAIForum MakeForum(const int32 Slot, const int32 Lives)
	{
		FHOLAIForum Forum;
		Forum.Slot = Slot;
		Forum.Lives = Lives;
		return Forum;
	}

	FHOLAIBuilding MakeCentre(const int32 Owner, const double X, const double Y)
	{
		FHOLAIBuilding Centre;
		Centre.OwnerPlayer = Owner;
		Centre.TypeName = HOLBuildingTypeNames::MainCentre;
		Centre.Location = FVector(X, Y, 0.0);
		return Centre;
	}

	FHOLUnitView MakeKing(const uint32 Id, const uint8 Owner, const EHOLUnitActivity Activity = EHOLUnitActivity::Idle)
	{
		FHOLUnitView King;
		King.UnitId = Id;
		King.OwnerPlayer = Owner;
		King.TypeName = TEXT("King");
		King.bKing = true;
		King.Activity = Activity;
		King.Health = 300;
		return King;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIForumRuleTest, "HeroesOfLiberty.AI.Summary.ForumRule", UE::HeroesOfLiberty::Tests::AIForum::Flags)
bool FHOLAIForumRuleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIForum;
	FHOLAIForum Ruin = MakeForum(1, 1);
	Ruin.bDown = true;
	Ruin.bRuin = true;
	Ruin.RuinSecondsLeft = 12.f;
	Ruin.RebuildProgress = 0.f;
	Ruin.HoldProgress = 0.25f;
	Ruin.bContested = true;
	const TArray<FHOLAIForum> Forums = { MakeForum(2, 2), Ruin, MakeForum(0, 2), MakeForum(3, 1) };
	const TArray<FHOLAIBuilding> Buildings = { MakeCentre(0, 100.0, 0.0), MakeCentre(1, 5000.0, 0.0), MakeCentre(3, 9000.0, 0.0) };
	const TArray<FHOLUnitView> Units = { MakeKing(20, 0), MakeKing(21, 1), MakeKing(22, 2, EHOLUnitActivity::Dying) };
	const uint8 DefeatedMask = 1 << 3;

	const FHOLAISummary Slot0 = HOLAISummary::Make(0, Units, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, Buildings, {}, {}, {}, DefeatedMask, Forums, true);
	if (TestTrue(TEXT("slot 0 finds its own forum"), Slot0.bHasOwnForum))
	{
		TestEqual(TEXT("its lives"), Slot0.OwnForum.Lives, 2);
		TestFalse(TEXT("it stands"), Slot0.OwnForum.bDown);
		TestTrue(TEXT("its place: its main centre's"), Slot0.OwnForum.bHasLocation && Slot0.OwnForum.Location.Equals(FVector(100.0, 0.0, 0.0)));
	}
	TestEqual(TEXT("the other forums: slots 1 and 2 (3 is defeated), by slot"), Slot0.EnemyForums.Num(), 2);
	if (Slot0.EnemyForums.Num() == 2)
	{
		const FHOLAIForum& First = Slot0.EnemyForums[0];
		TestEqual(TEXT("first: slot 1"), First.Slot, 1);
		TestTrue(TEXT("slot 1 lies in ruin"), First.bDown && First.bRuin && !First.bRebuilding);
		TestEqual(TEXT("its ruin seconds"), First.RuinSecondsLeft, 12.f);
		TestEqual(TEXT("its hold"), First.HoldProgress, 0.25f);
		TestTrue(TEXT("contested"), First.bContested);
		TestTrue(TEXT("its place"), First.bHasLocation && First.Location.Equals(FVector(5000.0, 0.0, 0.0)));
		TestEqual(TEXT("second: slot 2"), Slot0.EnemyForums[1].Slot, 2);
		TestFalse(TEXT("slot 2 has no centre: no place"), Slot0.EnemyForums[1].bHasLocation);
	}
	TestTrue(TEXT("slot 0 has its King, id 20"), Slot0.bHasKing && Slot0.King.Id == 20 && Slot0.King.OwnerSlot == 0);
	TestTrue(TEXT("the Regicide flag follows"), Slot0.bRegicide);
	TestEqual(TEXT("the King is not in the army"), Slot0.Army.Num(), 0);

	const FHOLAISummary Slot2 = HOLAISummary::Make(2, Units, {}, {}, {}, HOLAISummary::DefaultMaxSourcesPerResource, Buildings, {}, {}, {}, DefeatedMask, Forums);
	TestFalse(TEXT("slot 2's King is dying: none"), Slot2.bHasKing);
	TestFalse(TEXT("no Regicide by default"), Slot2.bRegicide);
	TestEqual(TEXT("slot 2 sees slots 0 and 1 as the others"), Slot2.EnemyForums.Num(), 2);

	const FHOLAISummary Nothing = HOLAISummary::Make(0, Units, {}, {}, {});
	TestFalse(TEXT("without forums given: none"), Nothing.bHasOwnForum);
	TestEqual(TEXT("no enemy forum"), Nothing.EnemyForums.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIForumReadTest, "HeroesOfLiberty.AI.Summary.ForumRead", UE::HeroesOfLiberty::Tests::AIForum::Flags)
bool FHOLAIForumReadTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLBuilding* Centre0 = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Centre1 = World->SpawnActor<AHOLBuilding>(FVector(-10000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("centre 0"), Centre0) && TestNotNull(TEXT("centre 1"), Centre1))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
		Centre0->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Centre1->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		Centre0->SetPrimaryForum(true);
		Centre1->SetPrimaryForum(true);

		FHOLAISummary Slot0 = HOLAISummary::Read(*World, 0);
		if (TestTrue(TEXT("slot 0 finds its forum"), Slot0.bHasOwnForum))
		{
			TestEqual(TEXT("two lives at the start"), Slot0.OwnForum.Lives, 2);
			TestFalse(TEXT("standing"), Slot0.OwnForum.bDown);
		}
		TestEqual(TEXT("slot 0 knows the other forum only (closed slots left out)"), Slot0.EnemyForums.Num(), 1);

		// Slot 1's forum falls: in ruin, one life left.
		TestTrue(TEXT("slot 1's forum falls"), State->HandleForumFall(*Centre1));
		Slot0 = HOLAISummary::Read(*World, 0);
		if (Slot0.EnemyForums.Num() == 1)
		{
			const FHOLAIForum& Fallen = Slot0.EnemyForums[0];
			TestEqual(TEXT("slot 1"), Fallen.Slot, 1);
			TestEqual(TEXT("one life left"), Fallen.Lives, 1);
			TestTrue(TEXT("in ruin"), Fallen.bDown && Fallen.bRuin && !Fallen.bRebuilding);
			TestTrue(TEXT("ruin seconds left"), Fallen.RuinSecondsLeft > 0.f);
			TestTrue(TEXT("its place"), Fallen.bHasLocation && Fallen.Location.Equals(FVector(-10000.0, 0.0, 0.0), 1.0));
		}
		const FHOLAISummary Slot1 = HOLAISummary::Read(*World, 1);
		TestTrue(TEXT("slot 1 reads its own forum down"), Slot1.bHasOwnForum && Slot1.OwnForum.bRuin && Slot1.OwnForum.Lives == 1);

		// The rebuilding after the ruin.
		TestTrue(TEXT("the ruin is skipped"), State->SkipForumPhase(1));
		const FHOLAISummary Rebuilding = HOLAISummary::Read(*World, 1);
		TestTrue(TEXT("rebuilding: down, not in ruin"), Rebuilding.OwnForum.bDown && !Rebuilding.OwnForum.bRuin && Rebuilding.OwnForum.bRebuilding);
		TestTrue(TEXT("rebuilding: not done yet"), Rebuilding.OwnForum.RebuildProgress < 1.f);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// IA4: a teammate is not an enemy. The rule alone (Make with the camp table of a configuration: team 1 = slots 0 and 1, slot 2 alone),
// then read from a test world with the same configuration (forums and centre).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAITeammatesRuleTest, "HeroesOfLiberty.AI.Summary.TeammatesRule", UE::HeroesOfLiberty::Tests::AIForum::Flags)
bool FHOLAITeammatesRuleTest::RunTest(const FString& Parameters)
{
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
	{
		Config.Slots[Index].State = Index < 3 ? EHOLSlotState::AI : EHOLSlotState::Closed;
		Config.Slots[Index].Team = Index < 2 ? 1 : 0;
	}
	const FHOLCampTable Camps(Config);

	TArray<FHOLUnitView> Enemies;
	for (uint32 Id = 1; Id <= 3; ++Id)
	{
		FHOLUnitView Unit;
		Unit.UnitId = Id;
		Unit.OwnerPlayer = static_cast<uint8>(Id);	// Ids 1 and 2: owners 1 (teammate) and 2 (enemy); id 3: owner 3 (closed slot, alone)
		Unit.Health = 50;
		Enemies.Add(Unit);
	}
	TArray<FHOLAIBuilding> Buildings = { UE::HeroesOfLiberty::Tests::AIForum::MakeCentre(1, 1000.0, 0.0), UE::HeroesOfLiberty::Tests::AIForum::MakeCentre(2, -9000.0, 0.0) };
	TArray<FHOLAIForum> Forums = { UE::HeroesOfLiberty::Tests::AIForum::MakeForum(0, 2), UE::HeroesOfLiberty::Tests::AIForum::MakeForum(1, 2), UE::HeroesOfLiberty::Tests::AIForum::MakeForum(2, 2) };

	const FHOLAISummary Summary = HOLAISummary::Make(0, {}, {}, {}, TOptional<FHOLResources>(), HOLAISummary::DefaultMaxSourcesPerResource, Buildings, {}, {},
		Enemies, 0, Forums, false, Camps);
	TestEqual(TEXT("the teammate's unit is not an enemy"), Summary.Enemies.Num(), 2);
	for (const FHOLAIUnit& Enemy : Summary.Enemies)
	{
		TestTrue(TEXT("no unit of slot 1 among the enemies"), Enemy.Id != 1);
	}
	TestEqual(TEXT("the teammate's forum is not an enemy forum"), Summary.EnemyForums.Num(), 1);
	if (Summary.EnemyForums.Num() == 1)
	{
		TestEqual(TEXT("slot 2's forum"), Summary.EnemyForums[0].Slot, 2);
	}
	TestTrue(TEXT("the enemy centre is slot 2's, not the nearer teammate's"), Summary.bHasEnemyCentre && Summary.EnemyCentreLocation.Equals(FVector(-9000.0, 0.0, 0.0), 1.0));
	TestTrue(TEXT("its own forum is still read"), Summary.bHasOwnForum);

	// Without a camp table (each owner alone): everyone else is an enemy, as before.
	const FHOLAISummary Alone = HOLAISummary::Make(0, {}, {}, {}, TOptional<FHOLResources>(), HOLAISummary::DefaultMaxSourcesPerResource, Buildings, {}, {},
		Enemies, 0, Forums, false);
	TestEqual(TEXT("alone: three enemy units"), Alone.Enemies.Num(), 3);
	TestEqual(TEXT("alone: two enemy forums"), Alone.EnemyForums.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAITeammatesReadTest, "HeroesOfLiberty.AI.Summary.TeammatesRead", UE::HeroesOfLiberty::Tests::AIForum::Flags)
bool FHOLAITeammatesReadTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLBuilding* Centre0 = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Centre1 = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Centre2 = World->SpawnActor<AHOLBuilding>(FVector(-10000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("centre 0"), Centre0) && TestNotNull(TEXT("centre 1"), Centre1) && TestNotNull(TEXT("centre 2"), Centre2))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index < 3 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			Config.Slots[Index].Team = Index < 2 ? 1 : 2;
		}
		State->InitVictory(Config);
		Centre0->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Centre1->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		Centre2->Setup(HOLBuildingTypeNames::MainCentre, 2, 2);
		Centre0->SetPrimaryForum(true);
		Centre1->SetPrimaryForum(true);
		Centre2->SetPrimaryForum(true);

		const FHOLAISummary Slot0 = HOLAISummary::Read(*World, 0);
		TestEqual(TEXT("slot 0 sees slot 2's forum only (slot 1 is its teammate)"), Slot0.EnemyForums.Num(), 1);
		if (Slot0.EnemyForums.Num() == 1)
		{
			TestEqual(TEXT("it is slot 2's"), Slot0.EnemyForums[0].Slot, 2);
		}
		TestTrue(TEXT("the enemy centre is slot 2's"), Slot0.bHasEnemyCentre && Slot0.EnemyCentreLocation.Equals(FVector(-10000.0, 0.0, 0.0), 1.0));
		const FHOLAISummary Slot2 = HOLAISummary::Read(*World, 2);
		TestEqual(TEXT("slot 2 sees both forums of the other team"), Slot2.EnemyForums.Num(), 2);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
