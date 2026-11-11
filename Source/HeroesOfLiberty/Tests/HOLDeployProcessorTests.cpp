// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"
#include "Siege/HOLDeployRules.h"
#include "Simulation/HOLDeployProcessor.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// Palier3InProgress.md G6, first sub-step: the deployment state of a unit goes through the REAL deploy processor and the real unit table.
// A Spearman stands in for the engine (no unit type is deployable yet). An attack order far away deploys it (it fires only once deployed),
// the same order near the minimum range is refused, a walk order packs it, and a walk order during the deploying cancels it.
namespace UE::HeroesOfLiberty::Tests::DeployProcessor
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployProcessorTest, "HeroesOfLiberty.Siege.Deploy.Processor", UE::HeroesOfLiberty::Tests::DeployProcessor::Flags)

bool FHOLDeployProcessorTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DeployProcessor;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLDeployProcessor* Deploy = NewObject<UHOLDeployProcessor>(Simulation);
		Deploy->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 StepMs = FMath::RoundToInt(StepSeconds * 1000.f);
		int64 Step = 0;
		const auto RunSteps = [Simulation, Deploy, &EntityManager, StepSeconds, &Step](const int32 Count)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				++Step;
				Simulation->GetOnStepStarted().Broadcast(Step);
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Deploy, ProcessingContext);
				Simulation->GetOnStepEnded().Broadcast(Step);
			}
		};

		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("Spearman"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector::ZeroVector;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
		const FMassEntityHandle Entity = After.Num() == 1 ? Units->FindEntity(After[0]) : FMassEntityHandle();
		if (TestTrue(TEXT("a stand-in unit born"), Entity.IsSet()))
		{
			EntityManager->AddFragmentToEntity(Entity, FHOLDeployFragment::StaticStruct());
			FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
			if (TestNotNull(TEXT("the unit carries the deploy fragment"), Fragment))
			{
				const HOLDeploy::FProfile Profile = HOLDeploy::DefaultProfile();
				const int32 DeploySteps = Profile.DeployMs / StepMs + 2;
				const int32 PackSteps = Profile.PackMs / StepMs + 2;
				const auto Order = [&Fragment](const HOLDeploy::EOrder Kind, const int32 DistanceCm)
				{
					Fragment->PendingOrder = static_cast<uint8>(Kind) + 1;
					Fragment->PendingDistanceCm = DistanceCm;
				};
				const auto PhaseIs = [&Fragment](const HOLDeploy::EPhase Phase) { return Fragment->Phase == static_cast<uint8>(Phase); };
				const auto ActionIs = [&Fragment](const HOLDeploy::EAction Action) { return Fragment->LastAction == static_cast<uint8>(Action); };
				const int32 InRangeCm = (Profile.MinRangeCm + Profile.MaxRangeCm) / 2;

				// 1. Packed, an attack on a target in range: it starts to deploy, and is not deployed before its delay.
				TestTrue(TEXT("born packed"), PhaseIs(HOLDeploy::EPhase::Packed));
				Order(HOLDeploy::EOrder::Attack, InRangeCm);
				RunSteps(1);
				TestTrue(TEXT("the attack order starts the deploying"), ActionIs(HOLDeploy::EAction::Deploy) && PhaseIs(HOLDeploy::EPhase::Deploying));
				TestEqual(TEXT("the order is read once"), static_cast<int32>(Fragment->PendingOrder), 0);
				RunSteps(DeploySteps / 2);
				TestTrue(TEXT("half the delay: still deploying"), PhaseIs(HOLDeploy::EPhase::Deploying));
				RunSteps(DeploySteps);
				TestTrue(TEXT("after the delay: deployed"), PhaseIs(HOLDeploy::EPhase::Deployed));

				// 2. Deployed: the same attack fires, a target under the minimum range is refused, a walk order starts the packing.
				Order(HOLDeploy::EOrder::Attack, InRangeCm);
				RunSteps(1);
				TestTrue(TEXT("deployed, a target in range: fire"), ActionIs(HOLDeploy::EAction::Fire) && PhaseIs(HOLDeploy::EPhase::Deployed));
				Order(HOLDeploy::EOrder::Attack, Profile.MinRangeCm / 2);
				RunSteps(1);
				TestTrue(TEXT("a target nearer than the minimum range is refused"),
					ActionIs(HOLDeploy::EAction::Refuse) && Fragment->LastReason == HOLDeploy::TooClose && PhaseIs(HOLDeploy::EPhase::Deployed));
				Order(HOLDeploy::EOrder::Move, 0);
				RunSteps(1);
				TestTrue(TEXT("a walk order starts the packing"), ActionIs(HOLDeploy::EAction::Pack) && PhaseIs(HOLDeploy::EPhase::Packing));
				RunSteps(PackSteps);
				TestTrue(TEXT("after the delay: packed"), PhaseIs(HOLDeploy::EPhase::Packed));

				// 3. A walk order while deploying cancels it at once.
				Order(HOLDeploy::EOrder::Attack, InRangeCm);
				RunSteps(3);
				TestTrue(TEXT("deploying again"), PhaseIs(HOLDeploy::EPhase::Deploying));
				Order(HOLDeploy::EOrder::Move, 0);
				RunSteps(1);
				TestTrue(TEXT("a walk order cancels the deploying"), ActionIs(HOLDeploy::EAction::CancelDeploy) && PhaseIs(HOLDeploy::EPhase::Packed));
			}
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
