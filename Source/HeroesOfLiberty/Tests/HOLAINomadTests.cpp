// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// N3 (NomadInProgress.md, Retours/Q-nomad-ia.md default A): the script brain of a Nomad AI seat founds its first forum on the real
// chain: C++ summary (bNomadBeforeFirstForum, ForumPrice) -> HOLAINomad::Plan (Script/AI/HOLAINomadRules.as) -> the AI's place order
// -> the construction subsystem's foundation, paid from the seat's stock.
namespace UE::HeroesOfLiberty::Tests::AINomad
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FVector BigGroup(3000.0, 3000.0, 0.0);
	const FVector SmallGroup(-3000.0, -3000.0, 0.0);

	/**
	 * A match's server: slot 0 human (its centre far away), slot 1 an AI Nomad seat (AHOLGameState::SetSeatNomad) without any
	 * forum, 3 Peasants at BigGroup and 2 at SmallGroup; its stock is the price of its MainCentre computed by the game
	 * (UHOLBuildingSettings::GetBuildCost, the first forum's price) minus WoodShort.
	 */
	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		AHOLPlayerState* AIState = nullptr;
		UHOLAIBrainSubsystem* Brain = nullptr;
		FHOLResources Price;
		int32 Peasants = 0;

		explicit FWorld(const int32 WoodShort)
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			AHOLBuilding* EnemyCentre = World->SpawnActor<AHOLBuilding>(FVector(-20000.0, 0.0, 0.0), FRotator::ZeroRotator);
			UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
			UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
			UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Brain = UHOLAIBrainSubsystem::Find(World);
			if (!State || !EnemyCentre || !Construction || !Production || !Sources || !Units || !Brain || !World->GetSubsystem<UHOLSimulationSubsystem>())
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->GetPlayerIndex() == 1)
				{
					AIState = HOLEach;
				}
			}
			State->SetSeatNomad(1, true);
			EnemyCentre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
			Sources->OnWorldBeginPlay(*World);
			Construction->OnWorldBeginPlay(*World);
			Production->OnWorldBeginPlay(*World);
			for (const TPair<FVector, int32>& Group : { TPair<FVector, int32>(BigGroup, 3), TPair<FVector, int32>(SmallGroup, 2) })
			{
				FHOLUnitSpawnRequest Request;
				Request.TypeName = FName(TEXT("Peasant"));
				Request.Count = Group.Value;
				Request.OwnerPlayer = 1;
				Request.Center = Group.Key;
				Units->RequestUnits(Request);
				Units->ServeBirthRequests();
			}
			TArray<uint32> All;
			Units->GetAllUnitIds(All);
			Peasants = All.Num();
			Price = UHOLBuildingSettings::GetBuildCost(World, HOLBuildingTypeNames::MainCentre, 1);
			if (AIState)
			{
				AIState->SetPopulation(5, 10);
				FHOLResources Stock = Price;
				Stock.Wood -= WoodShort;
				AIState->SetResources(Stock);
			}
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return AIState && Brain && Peasants == 5 && Price.Wood > 0; }

		void RunStep() { World->GetSubsystem<UHOLSimulationSubsystem>()->GetOnStepStarted().Broadcast(1); }

		TArray<FVector> Forums() const
		{
			TArray<FVector> Places;
			for (TActorIterator<AHOLBuilding> It(World); It; ++It)
			{
				if (It->GetTypeName() == HOLBuildingTypeNames::MainCentre && It->GetOwnerPlayer() == 1)
				{
					Places.Add(It->GetActorLocation());
				}
			}
			return Places;
		}
	};
}

/**
 * N3: the stock covers the first forum's price: at its first thought the Nomad AI places a MainCentre foundation at its largest
 * group (3 Peasants, not the 2), and the price is taken from its stock. Red while the brain does not call HOLAINomad::Plan.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAINomadFoundsForumTest, "HeroesOfLiberty.AI.Nomad.FoundsFirstForum", UE::HeroesOfLiberty::Tests::AINomad::Flags)
bool FHOLAINomadFoundsForumTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AINomad;
	FWorld Test(0);
	if (!TestTrue(TEXT("test world ready (script brain, Nomad slot 1, 5 Peasants, a first forum price)"), Test.IsReady()))
	{
		return false;
	}
	TestTrue(TEXT("slot 1 thinks"), Test.Brain->ThinkNow(1));
	Test.RunStep();
	const TArray<FVector> Forums = Test.Forums();
	if (TestEqual(TEXT("one MainCentre foundation of slot 1"), Forums.Num(), 1))
	{
		const double ToBig = FVector::Dist2D(Forums[0], BigGroup);
		const double ToSmall = FVector::Dist2D(Forums[0], SmallGroup);
		TestTrue(*FString::Printf(TEXT("at the largest group (%.0f cm from the 3, %.0f from the 2)"), ToBig, ToSmall), ToBig < ToSmall);
	}
	const FHOLResources After = Test.AIState->GetResources();
	TestEqual(TEXT("the forum's wood paid"), After.Wood, 0);
	TestEqual(TEXT("the forum's stone paid"), After.Stone, 0);
	return true;
}

/** N3: one wood short of the first forum's price: the Nomad AI places no forum and its stock stays whole. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAINomadWaitsForumTest, "HeroesOfLiberty.AI.Nomad.WaitsWithoutPrice", UE::HeroesOfLiberty::Tests::AINomad::Flags)
bool FHOLAINomadWaitsForumTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AINomad;
	FWorld Test(1);
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	Test.Brain->ThinkNow(1);
	Test.RunStep();
	TestEqual(TEXT("no MainCentre of slot 1"), Test.Forums().Num(), 0);
	TestEqual(TEXT("its wood kept"), Test.AIState->GetResources().Wood, Test.Price.Wood - 1);
	return true;
}

#endif
