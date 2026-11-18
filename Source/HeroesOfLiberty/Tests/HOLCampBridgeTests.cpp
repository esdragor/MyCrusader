// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCampBoard.h"

#if WITH_DEV_AUTOMATION_TESTS

// Bascule Camp (D830/D836 point 3, AngelscriptInProgress.md): the server asks the script's rule through UHOLCampBoard
// (Script/Neutrals/HOLCampBoardScript.as -> HOLCamp::Step / HOLCamp::Rewards of HOLCampRules.as). The C++ rule is gone (script is the only source):
// the board's answers are checked against the cases of the script rule, and without a board the camp is inert.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCampServerBoardTest, "HeroesOfLiberty.Camp.ServerBoard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLCampServerBoardTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLCampBoard* Board = UHOLCampBoard::Find(World);
	if (TestNotNull(TEXT("the script board of the camps"), Board))
	{
		const FVector2D Post(10000.0, 0.0);
		const FHOLCampBoardSettings Settings;
		TArray<FHOLCampBoardMember> Members;
		for (int32 Index = 0; Index < 3; ++Index)
		{
			Members.Add(FHOLCampBoardMember{ 500 + Index, Post });
		}
		const TArray<FHOLCampBoardIntruder> AtAggro = { FHOLCampBoardIntruder{ 1, 0, Post + FVector2D(Settings.AggroRadius, 0.0) } };
		const TArray<FHOLCampBoardIntruder> Beyond = { FHOLCampBoardIntruder{ 1, 0, Post + FVector2D(Settings.AggroRadius + 1.f, 0.0) } };

		const FHOLCampBoardStep Drawn = Board->StepNow(EHOLCampBoardState::Guarding, Settings, Post, Members, AtAggro, false);
		TestTrue(TEXT("An intruder at the aggro radius draws the camp: fighting, changed"), Drawn.State == EHOLCampBoardState::Fighting && Drawn.bChanged);
		const FHOLCampBoardStep Far = Board->StepNow(EHOLCampBoardState::Guarding, Settings, Post, Members, Beyond, false);
		TestTrue(TEXT("An intruder beyond the aggro radius: still guarding"), Far.State == EHOLCampBoardState::Guarding && !Far.bChanged);
		const FHOLCampBoardStep Gone = Board->StepNow(EHOLCampBoardState::Fighting, Settings, Post, Members, {}, false);
		TestTrue(TEXT("Nobody within the leash: returning, all three members sent back"), Gone.State == EHOLCampBoardState::Returning && Gone.ReturnIds == TArray<int32>({500, 501, 502}));
		const FHOLCampBoardStep Dead = Board->StepNow(EHOLCampBoardState::Fighting, Settings, Post, {}, AtAggro, false);
		TestTrue(TEXT("No member left: cleared, the reward due once"), Dead.State == EHOLCampBoardState::Cleared && Dead.bJustCleared);
		TestFalse(TEXT("A cleared camp stays cleared, no second reward"), Board->StepNow(EHOLCampBoardState::Cleared, Settings, Post, {}, AtAggro, false).bJustCleared);

		// The rewards: shared 60/40 of 100 XP; the last blow takes it all.
		const TArray<FHOLCampBoardHit> Hits = { FHOLCampBoardHit{ 0, 60 }, FHOLCampBoardHit{ 1, 40 } };
		const TArray<FHOLCampBoardShare> Shared = Board->RewardsNow(Settings, Hits);
		TestTrue(TEXT("Shared: 60 and 40 XP by owner"), Shared.Num() == 2 && Shared[0].Owner == 0 && Shared[0].XP == 60 && Shared[1].Owner == 1 && Shared[1].XP == 40);
		FHOLCampBoardSettings LastBlow = Settings;
		LastBlow.Credit = EHOLCampBoardCredit::LastBlow;
		const TArray<FHOLCampBoardShare> Whole = Board->RewardsNow(LastBlow, Hits);
		TestTrue(TEXT("LastBlow: owner 1 takes the whole reward"), Whole.Num() == 1 && Whole[0].Owner == 1 && Whole[0].XP == Settings.RewardXP);
		TestEqual(TEXT("No hit, no share"), Board->RewardsNow(Settings, {}).Num(), 0);

		// The path of the server (UHOLCampSubsystem): the C++ types in and out through the board.
		const HOLCampRules::FSettings RuleSettings;
		const TArray<HOLCampRules::FMember> RuleMembers = { HOLCampRules::FMember{ 500, Post }, HOLCampRules::FMember{ 501, Post } };
		const TArray<HOLCampRules::FIntruder> RuleIntruders = { HOLCampRules::FIntruder{ 1, 0, Post + FVector2D(RuleSettings.AggroRadius, 0.0) } };
		HOLCampRules::EState ViaBoard = HOLCampRules::EState::Guarding;
		const HOLCampRules::FStep StepBoard = UHOLCampBoard::StepForCamp(World, ViaBoard, RuleSettings, Post, RuleMembers, RuleIntruders, false);
		TestTrue(TEXT("StepForCamp goes through the board: fighting, changed, nobody sent back"), ViaBoard == HOLCampRules::EState::Fighting && StepBoard.bChanged && StepBoard.ReturnIds.Num() == 0);
		const TArray<HOLCampRules::FHit> RuleHits = { HOLCampRules::FHit{ 0, 60 }, HOLCampRules::FHit{ 1, 40 } };
		const TArray<HOLCampRules::FShare> SharesBoard = UHOLCampBoard::RewardsForCamp(World, RuleSettings, RuleHits);
		TestTrue(TEXT("RewardsForCamp goes through the board: 60 and 40 XP by owner"), SharesBoard.Num() == 2 && SharesBoard[0].Owner == 0 && SharesBoard[0].XP == 60
			&& SharesBoard[1].Owner == 1 && SharesBoard[1].XP == 40);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// D836 point 3: no script board = the camp does not move (state kept, no one sent back) and no reward, an error is logged. A world that is
// neither Game nor PIE has no board (DoesSupportWorldType), which is the same case as a missing script.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCampInertBoardTest, "HeroesOfLiberty.Camp.ServerBoardInert",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLCampInertBoardTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Inactive, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Inactive);
	Context.SetCurrentWorld(World);
	TestNull(TEXT("no camp board in this world"), UHOLCampBoard::Find(World));

	AddExpectedError(TEXT("Camp: no script board, the camp keeps its state"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("Camp: no script board, no reward"), EAutomationExpectedErrorFlags::Contains, 1);
	const FVector2D Post(10000.0, 0.0);
	const HOLCampRules::FSettings Settings;
	const TArray<HOLCampRules::FMember> Members = { HOLCampRules::FMember{ 500, Post } };
	const TArray<HOLCampRules::FIntruder> Intruders = { HOLCampRules::FIntruder{ 1, 0, Post } };
	HOLCampRules::EState State = HOLCampRules::EState::Guarding;
	const HOLCampRules::FStep Step = UHOLCampBoard::StepForCamp(World, State, Settings, Post, Members, Intruders, true);
	TestTrue(TEXT("No board: the state is unchanged, nothing changed, nobody sent back, no clear"), State == HOLCampRules::EState::Guarding && !Step.bChanged
		&& !Step.bJustCleared && Step.ReturnIds.Num() == 0);
	const TArray<HOLCampRules::FHit> Hits = { HOLCampRules::FHit{ 0, 60 } };
	TestEqual(TEXT("No board: no reward"), UHOLCampBoard::RewardsForCamp(World, Settings, Hits).Num(), 0);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif

