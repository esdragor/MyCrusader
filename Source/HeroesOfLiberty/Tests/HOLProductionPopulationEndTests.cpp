// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLProductionCommand.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// D829 (the user's BUG on the trial alert_population, 2026-11-16): the population is checked at the END of the production, not when a
// unit is queued. On the real chain: the production orders go through the command queue and the production subsystem (server), the
// population is the one the player state shows, the units are real (served births), one of them dies by MarkUnitDying.
namespace UE::HeroesOfLiberty::Tests::PopulationEnd
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));

	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		AHOLPlayerState* Player = nullptr;
		AHOLBuilding* Centre = nullptr;
		bool bReady = false;

		FWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
			UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
			UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			if (!State || !Units || !Commands || !Production || !Construction || !Sources || !World->GetSubsystem<UHOLSimulationSubsystem>())
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::AI : EHOLSlotState::Closed;
				Config.Slots[Index].ColorIndex = Index;
				Config.Slots[Index].Faction = EHOLFaction::DawnOrder;
			}
			State->InitVictory(Config);
			AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			Sources->OnWorldBeginPlay(*World);
			Construction->OnWorldBeginPlay(*World);
			Production->OnWorldBeginPlay(*World);
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->GetPlayerIndex() == 0)
				{
					Player = HOLEach;
				}
			}
			// The forum (Main centre: 10 places, D283), finished.
			Centre = World->SpawnActor<AHOLBuilding>(FVector(4000.0, 0.0, 0.0), FRotator::ZeroRotator);
			if (Centre)
			{
				Centre->Setup(TEXT("MainCentre"), 0, 0);
			}
			bReady = Player && Centre;
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		void RunStep(const int64 StepIndex) { World->GetSubsystem<UHOLSimulationSubsystem>()->GetOnStepStarted().Broadcast(StepIndex); }

		void OrderPeasant()
		{
			FHOLProductionCommand Command;
			Command.PlayerIndex = 0;
			Command.Building = Centre;
			Command.TypeName = Peasant;
			Commands->IssueProductionOrder(Command);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLProductionPopulationEndTest, "HeroesOfLiberty.Production.PopulationCheckedAtEnd", UE::HeroesOfLiberty::Tests::PopulationEnd::Flags)
bool FHOLProductionPopulationEndTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::PopulationEnd;
	FWorld Test;
	if (!TestTrue(TEXT("the test world stands"), Test.bReady))
	{
		return false;
	}
	const FHOLUnitTypeRow* Row = Test.Units->FindTypeRow(Peasant);
	if (!TestNotNull(TEXT("the Peasant's row"), Row))
	{
		return false;
	}
	const int32 BuildSteps = UHOLProductionSubsystem::GetBuildSteps(*Row, GetDefault<UHOLSimulationSettings>()->GetStepSeconds());
	Test.Player->SetResources(FHOLResources(5000, 5000, 5000, 5000));

	// 9 Peasants of slot 0, real units (the scenario alert_population offers the same): 9 / 10.
	FHOLUnitSpawnRequest Nine{ Peasant, 9, FVector(2000.0, 0.0, 0.0), 600.f, 0 };
	Test.Units->RequestUnits(Nine);
	Test.Units->ServeBirthRequests();

	// Two Peasants queued at 9 / 10: both queued, the first produced; a unit in production holds no place (D829).
	Test.OrderPeasant();
	Test.OrderPeasant();
	int64 Step = 1;
	Test.RunStep(Step);
	TestEqual(TEXT("two Peasants queued at 9 / 10"), Test.Centre->GetProductionQueue().Num(), 2);
	TestEqual(TEXT("in production, the first holds no place: still 9 / 10"), Test.Player->GetPopulationUsed(), 9);
	TestEqual(TEXT("cap of the forum"), Test.Player->GetPopulationCap(), 10);

	// The first comes out (10 / 10), the second is produced to 100 % and does NOT come out.
	for (int32 Each = 0; Each < 2 * BuildSteps + 5; ++Each)
	{
		Test.RunStep(++Step);
	}
	const TArray<FHOLProductionItem>& Queue = Test.Centre->GetProductionQueue();
	if (!TestEqual(TEXT("the second Peasant is still in the queue"), Queue.Num(), 1))
	{
		return false;
	}
	TestTrue(TEXT("the second Peasant was produced (started)"), Queue[0].IsStarted());
	TestEqual(TEXT("the second Peasant is at 100 %"), HOLProduction::GetProgress(Queue[0], static_cast<double>(Step), BuildSteps), 1.f);
	TestTrue(TEXT("it waits at 100 % for a place"), HOLProduction::IsWaitingForRoom(Queue[0], static_cast<double>(Step), BuildSteps));
	TestEqual(TEXT("the first came out: 10 / 10"), Test.Player->GetPopulationUsed(), 10);

	// A Peasant dies: a place is free, the second comes out at the next step (10 / 10 again).
	TArray<uint32> Ids;
	Test.Units->GetUnitIdsOfType(Peasant, Ids);
	if (!TestTrue(TEXT("living Peasants"), Ids.Num() > 0))
	{
		return false;
	}
	Test.Units->MarkUnitDying(Ids[0]);
	Test.RunStep(++Step);
	TestEqual(TEXT("a place freed: the second Peasant came out"), Test.Centre->GetProductionQueue().Num(), 0);
	TestEqual(TEXT("10 / 10 again: the dead one out, the new one in"), Test.Player->GetPopulationUsed(), 10);
	return true;
}

#endif
