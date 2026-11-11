// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"
#include "Siege/HOLDeployRules.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLDeployProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// Palier3InProgress.md G6 d: the Mount / Dismount button. The player's entry is HOLPlayerOrders::ToggleDeploy (what the controller's RPC calls),
// through the REAL queue, order processor, deploy processor and combat: pressed on a packed engine it starts the deploying, deployed it fires,
// pressed again it packs up and does not deploy by itself again; a press by another slot does nothing (D288). Effects measured: the phase of the
// fragment, then the target's health.
namespace UE::HeroesOfLiberty::Tests::DeployButton
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployPlayerButtonTest, "HeroesOfLiberty.Siege.Deploy.PlayerButton", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployPlayerButtonTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DeployButton;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	IConsoleVariable* Deployable = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Siege.Deployable"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled) || !TestNotNull(TEXT("hol.Siege.Deployable"), Deployable))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	const int32 FormerDeployable = Deployable->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);
	Deployable->Set(1, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem)
		&& TestNotNull(TEXT("commands"), Commands))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLUnitOrderProcessor* Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
		Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLDeployProcessor* Deploy = NewObject<UHOLDeployProcessor>(Simulation);
		Deploy->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 StepMs = FMath::RoundToInt(StepSeconds * 1000.f);
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds, &Step]
		{
			Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Orders, ProcessingContext);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [Units](const FName Type, const int32 Owner, const FVector& Location)
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				TArray<uint32> Before;
				Units->GetAllUnitIds(Before);
				FHOLUnitSpawnRequest Request;
				Request.TypeName = Type;
				Request.Count = 1;
				Request.OwnerPlayer = static_cast<uint8>(Owner);
				Request.Center = Location;
				Units->RequestUnits(Request);
				Units->ServeBirthRequests();
				TArray<uint32> After;
				Units->GetAllUnitIds(After);
				uint32 Born = 0;
				for (const uint32 Id : After)
				{
					Born = Before.Contains(Id) ? Born : Id;
				}
				if (Born == 0 || Born % 4 == 0)
				{
					return Born;
				}
				const uint32 Again[] = { Born };
				Units->RequestDestroyUnits(Again);
				Units->FlushDestructions();
			}
			return 0u;
		};
		const auto HealthOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			const FHOLUnitStateFragment* State = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
			return State ? State->CurrentHealth : -1.f;
		};
		const HOLDeploy::FProfile Profile = HOLDeploy::DefaultProfile();
		const int32 DeploySteps = Profile.DeployMs / StepMs;
		const int32 PackSteps = Profile.PackMs / StepMs;
		const int32 TenSeconds = FMath::CeilToInt(10.f / StepSeconds);

		const uint32 Engine = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
		if (TestTrue(TEXT("the deployable Mangonel born"), Engine != 0))
		{
			const FMassEntityHandle Entity = Units->FindEntity(Engine);
			const auto PhaseOf = [&EntityManager, Entity]
			{
				const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
				return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
			};
			const TArray<uint32> Mine = { Engine };
			TestEqual(TEXT("born packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));

			// 1. A press by another slot (D288): nothing happens.
			HOLPlayerOrders::ToggleDeploy(*World, 2, Mine);
			for (int32 Index = 0; Index < 3; ++Index)
			{
				RunStep();
			}
			TestEqual(TEXT("another slot's press leaves the engine packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));

			// 2. The owner's press on a packed engine: it starts deploying, then it is deployed.
			TestTrue(TEXT("the owner's press enters the queue"), HOLPlayerOrders::ToggleDeploy(*World, 0, Mine));
			RunStep();
			RunStep();
			TestEqual(TEXT("the press starts the deploying"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Deploying));
			for (int32 Index = 0; Index < DeploySteps; ++Index)
			{
				RunStep();
			}
			TestEqual(TEXT("the engine ends deployed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Deployed));

			// 3. Deployed, it fires at the target it is ordered on (the effect: the target's health moves).
			const uint32 Foe = Place(FName(TEXT("Spearman")), 3, FVector(2000.0, 0.0, 0.0));
			if (TestTrue(TEXT("the target born"), Foe != 0))
			{
				FHOLCombatFragment* EngineCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Entity);
				FHOLCombatFragment* FoeCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Foe));
				if (TestNotNull(TEXT("combat fragments"), EngineCombat) && TestNotNull(TEXT("target combat"), FoeCombat))
				{
					FoeCombat->Mode = EHOLCombatMode::Hold;
					EngineCombat->Target = Units->FindEntity(Foe);
					EngineCombat->bOrderedTarget = true;
					const float Start = HealthOf(Foe);
					bool bHit = false;
					for (int32 Index = 0; Index < TenSeconds && !bHit; ++Index)
					{
						RunStep();
						bHit = HealthOf(Foe) < Start;
					}
					TestTrue(TEXT("deployed by the button, the engine fires"), bHit);

					// 4. A second press: it packs up and does not deploy again by itself, though its target stands.
					TestTrue(TEXT("the second press enters the queue"), HOLPlayerOrders::ToggleDeploy(*World, 0, Mine));
					RunStep();
					RunStep();
					TestEqual(TEXT("the second press starts the packing"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packing));
					for (int32 Index = 0; Index < PackSteps + 2; ++Index)
					{
						RunStep();
					}
					TestEqual(TEXT("the engine ends packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
					for (int32 Index = 0; Index < DeploySteps / 2; ++Index)
					{
						RunStep();
					}
					TestEqual(TEXT("packed by the player, it does not deploy again on its own"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
				}
			}
			const TArray<uint32> Gone = { Engine, Foe };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	Deployable->Set(FormerDeployable, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
