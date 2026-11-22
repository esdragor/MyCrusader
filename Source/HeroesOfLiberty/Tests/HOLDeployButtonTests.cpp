// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassNavigationFragments.h"
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

// Retours/E-deploy.md (preflight deploy FAIL 2026-11-13): in a real match the press of D left the engine packed. An engine that still has a
// walk of its own when the player presses D (a spawn spread, a march not over, a walk stalled) must stand and deploy: the walk is not a new
// march order that cancels the deploying. Through the REAL queue, order processor, deploy processor and combat; effect: the phase reaches
// Deployed (the cell D then says Dismount) and the walk is over.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployPressWhileWalkingTest, "HeroesOfLiberty.Siege.Deploy.PressWhileWalking", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployPressWhileWalkingTest::RunTest(const FString& Parameters)
{
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
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
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
		const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Orders, ProcessingContext);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("Mangonel"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector::ZeroVector;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		uint32 Engine = 0;
		for (const uint32 Id : After)
		{
			Engine = Before.Contains(Id) ? Engine : Id;
		}
		if (TestTrue(TEXT("the deployable Mangonel born"), Engine != 0))
		{
			const FMassEntityHandle Entity = Units->FindEntity(Engine);
			const auto PhaseOf = [&EntityManager, Entity]
			{
				const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
				return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
			};
			const auto Walking = [&EntityManager, Entity]
			{
				const FHOLMoveOrderFragment* Order = EntityManager->GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
				return Order && Order->Phase != EHOLMovePhase::None;
			};
			const TArray<uint32> Mine = { Engine };
			TestEqual(TEXT("born packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
			// A walk of its own, not over (no movement runs here: it stays a walk under way, as a stalled one in a match).
			TestTrue(TEXT("the march order enters the queue"), HOLPlayerOrders::Move(*World, 0, Mine, FVector(4000.0, 0.0, 0.0)));
			RunStep();
			RunStep();
			TestTrue(TEXT("precondition: the packed engine walks"), Walking());

			TestTrue(TEXT("the press enters the queue"), HOLPlayerOrders::ToggleDeploy(*World, 0, Mine));
			const int32 DeploySteps = HOLDeploy::DefaultProfile().DeployMs / StepMs;
			for (int32 Index = 0; Index < DeploySteps + 4; ++Index)
			{
				RunStep();
			}
			TestEqual(TEXT("pressed while walking, the engine ends deployed (cell D: Dismount)"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Deployed));
			TestFalse(TEXT("pressed while walking, the engine stands (its walk is over)"), Walking());
			const TArray<uint32> Gone = { Engine };
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

// D861 (b): a DEPLOYED engine that gets a move order must pack up, then LEAVE without a second click. UHOLDeployProcessor zeroes the
// speed of an engine that is not packed; the order processor re-sets a walk's speed only while it is above 0, so the zero outlived the
// Packing and the engine stood packed with its order still open (or closed as stalled near its goal). Through the REAL queue, order
// processor, deploy processor and combat; effect: once Packed (+ 2 steps) the march order is still open and its speed is above 0 (when
// the same march had a speed before the engine deployed: no movement runs in this harness, a walk waiting for its path has none).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployMoveWhileDeployedKeepsOrderTest, "HeroesOfLiberty.Siege.Deploy.MoveWhileDeployedKeepsOrder", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployMoveWhileDeployedKeepsOrderTest::RunTest(const FString& Parameters)
{
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
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
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
		const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Orders, ProcessingContext);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("Mangonel"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector::ZeroVector;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		uint32 Engine = 0;
		for (const uint32 Id : After)
		{
			Engine = Before.Contains(Id) ? Engine : Id;
		}
		if (TestTrue(TEXT("the deployable Mangonel born"), Engine != 0))
		{
			const FMassEntityHandle Entity = Units->FindEntity(Engine);
			const auto PhaseOf = [&EntityManager, Entity]
			{
				const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
				return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
			};
			const auto Walking = [&EntityManager, Entity]
			{
				const FHOLMoveOrderFragment* Order = EntityManager->GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
				return Order && Order->Phase != EHOLMovePhase::None;
			};
			const auto WalkSpeed = [&EntityManager, Entity]
			{
				const FMassMoveTargetFragment* MoveTarget = EntityManager->GetFragmentDataPtr<FMassMoveTargetFragment>(Entity);
				return MoveTarget ? MoveTarget->DesiredSpeed.Get() : -1.f;
			};
			const TArray<uint32> Mine = { Engine };
			// A goal near enough (< 500 cm) for the order processor's stalled-walk count to run while the engine does not move (it packs up).
			const FVector Goal(400.0, 0.0, 0.0);

			// Baseline: the same march of the packed engine, and the speed it has then (0 when the harness leaves it waiting for a path).
			TestTrue(TEXT("the first march order enters the queue"), HOLPlayerOrders::Move(*World, 0, Mine, Goal));
			RunStep();
			RunStep();
			TestTrue(TEXT("precondition: the packed engine walks"), Walking());
			const float SpeedPacked = WalkSpeed();

			// It deploys where it stands (the press drops its walk), then gets the march order.
			TestTrue(TEXT("the press enters the queue"), HOLPlayerOrders::ToggleDeploy(*World, 0, Mine));
			const int32 DeploySteps = HOLDeploy::DefaultProfile().DeployMs / StepMs;
			for (int32 Index = 0; Index < DeploySteps + 4; ++Index)
			{
				RunStep();
			}
			TestEqual(TEXT("precondition: the engine is deployed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Deployed));
			TestTrue(TEXT("the march order of the deployed engine enters the queue"), HOLPlayerOrders::Move(*World, 0, Mine, Goal));
			RunStep();
			TestEqual(TEXT("the march order packs the engine up"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packing));
			const int32 PackSteps = HOLDeploy::DefaultProfile().PackMs / StepMs;
			for (int32 Index = 0; Index < PackSteps + 4; ++Index)
			{
				RunStep();
			}
			TestEqual(TEXT("packed again"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
			RunStep();
			RunStep();
			// The effect (never the phase alone): the engine leaves by itself.
			TestTrue(TEXT("packed up, the engine keeps its march order (no second click)"), Walking());
			if (SpeedPacked > 0.f)
			{
				TestTrue(TEXT("packed up, the engine walks again (speed above 0)"), WalkSpeed() > 0.f);
			}
			const TArray<uint32> Gone = { Engine };
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

// D861 (a): a PACKED engine given an attack order on a target in reach must deploy, then fire (it only walked). Through the REAL queue, order
// processor, deploy processor and combat. No movement runs in this harness: a target beyond the Mangonel's own reach (3000 cm, UnitTypes.csv
// AttackRange / BuildingAttackRange) is never walked to, so the real chain is driven at 2500 cm (inside both the reach and HOLDeploy's aim range).
// The band between HOLDeploy's MaxRangeCm (5000) and the combat's reach is EMPTY for the Mangonel (3000 < 5000): the case "the combat says in
// reach but the aim range says no" is driven with the combat's own note written by hand (PendingOrder 1 + a distance), the one input the combat
// would give. Effects measured: the phase of the fragment, then the target's health.
namespace UE::HeroesOfLiberty::Tests::DeployButton
{
	enum class EAttackWhilePackedCase : uint8
	{
		/** The real chain: an Attack / AttackBuilding order on a target 2500 cm off, building then unit. */
		InCombatReach,
		/** The combat's note at a distance between MaxRangeCm and farther (the aim range must not contradict the combat), and a too-near note. */
		BeyondDeployRange,
	};

	bool RunAttackWhilePacked(FAutomationTestBase& Test, const EAttackWhilePackedCase Case)
	{
		IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
		IConsoleVariable* Deployable = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Siege.Deployable"));
		if (!Test.TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled) || !Test.TestNotNull(TEXT("hol.Siege.Deployable"), Deployable))
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
		if (Test.TestNotNull(TEXT("units"), Units) && Test.TestNotNull(TEXT("simulation"), Simulation) && Test.TestNotNull(TEXT("entities"), EntitySubsystem))
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
			const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds]
			{
				const int64 Step = Simulation->AdvanceStepIndexForTests();
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
			const int32 TenSeconds = FMath::CeilToInt(10.f / StepSeconds);
			// Some steps of slack: the order is applied, the combat notes it, the deploy processor reads it, in this order, over a few steps.
			const int32 Slack = 20;
			const auto PhaseOfUnit = [Units, &EntityManager](const uint32 Id)
			{
				const FHOLDeployFragment* Fragment = Id != 0 ? EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Units->FindEntity(Id)) : nullptr;
				return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
			};
			const int32 Packed = static_cast<int32>(HOLDeploy::EPhase::Packed);
			const int32 Deployed = static_cast<int32>(HOLDeploy::EPhase::Deployed);

			if (Case == EAttackWhilePackedCase::InCombatReach)
			{
				const FVector Spot(2500.0, 0.0, 0.0);

				// 1. A building (the enemy House of slot 3, team 3: the preflight's layout).
				AHOLBuilding* House = World->SpawnActor<AHOLBuilding>(Spot, FRotator::ZeroRotator);
				const uint32 EngineA = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
				if (Test.TestNotNull(TEXT("house"), House) && Test.TestTrue(TEXT("the deployable Mangonel born (building)"), EngineA != 0))
				{
					House->Setup(HOLBuildingTypeNames::House, 3, 3);
					const TArray<uint32> Mine = { EngineA };
					Test.TestEqual(TEXT("born packed (building)"), PhaseOfUnit(EngineA), Packed);
					const float Start = House->GetHealth();
					Test.TestTrue(TEXT("the attack order on the building enters the queue"), HOLPlayerOrders::AttackBuilding(*World, 0, Mine, House));
					for (int32 Index = 0; Index < DeploySteps + Slack; ++Index)
					{
						RunStep();
					}
					Test.TestEqual(TEXT("packed, ordered on a House in reach, the engine ends deployed"), PhaseOfUnit(EngineA), Deployed);
					bool bHit = House->GetHealth() < Start;
					for (int32 Index = 0; Index < TenSeconds && !bHit; ++Index)
					{
						RunStep();
						bHit = House->GetHealth() < Start;
					}
					Test.TestTrue(TEXT("deployed by the attack order, the engine fires: the House's health goes down"), bHit);
				}
				if (House)
				{
					House->Destroy();
				}
				const TArray<uint32> GoneA = { EngineA };
				Units->RequestDestroyUnits(GoneA);
				Units->FlushDestructions();

				// 2. A unit (a Spearman of slot 3 on Hold, so that it stays where it stands).
				const uint32 EngineB = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
				const uint32 Foe = Place(FName(TEXT("Spearman")), 3, Spot);
				if (Test.TestTrue(TEXT("the deployable Mangonel born (unit)"), EngineB != 0) && Test.TestTrue(TEXT("the target born"), Foe != 0))
				{
					FHOLCombatFragment* FoeCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Foe));
					if (Test.TestNotNull(TEXT("target combat"), FoeCombat))
					{
						FoeCombat->Mode = EHOLCombatMode::Hold;
						const TArray<uint32> Mine = { EngineB };
						const float Start = HealthOf(Foe);
						Test.TestTrue(TEXT("the attack order on the unit enters the queue"), HOLPlayerOrders::Attack(*World, 0, Mine, Foe));
						for (int32 Index = 0; Index < DeploySteps + Slack; ++Index)
						{
							RunStep();
						}
						Test.TestEqual(TEXT("packed, ordered on a unit in reach, the engine ends deployed"), PhaseOfUnit(EngineB), Deployed);
						bool bHit = HealthOf(Foe) < Start;
						for (int32 Index = 0; Index < TenSeconds && !bHit; ++Index)
						{
							RunStep();
							bHit = HealthOf(Foe) < Start;
						}
						Test.TestTrue(TEXT("deployed by the attack order, the engine fires: the unit's health goes down"), bHit);
					}
				}
				const TArray<uint32> GoneB = { EngineB, Foe };
				Units->RequestDestroyUnits(GoneB);
				Units->FlushDestructions();
			}
			else
			{
				// The combat's note by hand: its in-reach verdict is the one input; the aim range of HOLDeploy must not turn it into a march.
				const auto NoteAndRun = [&](const uint32 Engine, const int32 DistanceCm)
				{
					FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Units->FindEntity(Engine));
					if (Fragment)
					{
						Fragment->PendingOrder = 1;
						Fragment->PendingDistanceCm = DistanceCm;
					}
					for (int32 Index = 0; Index < DeploySteps + 4; ++Index)
					{
						RunStep();
					}
				};
				const uint32 Far = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
				if (Test.TestTrue(TEXT("the deployable Mangonel born (far note)"), Far != 0))
				{
					NoteAndRun(Far, Profile.MaxRangeCm + 1000);
					Test.TestEqual(TEXT("the combat says in reach beyond MaxRangeCm: the engine deploys (it does not just stop)"), PhaseOfUnit(Far), Deployed);
				}
				const uint32 Near = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
				if (Test.TestTrue(TEXT("the deployable Mangonel born (near note)"), Near != 0))
				{
					NoteAndRun(Near, Profile.MinRangeCm - 100);
					Test.TestEqual(TEXT("the combat says in reach nearer than MinRangeCm: the engine stays packed (target too close)"), PhaseOfUnit(Near), Packed);
				}
				const TArray<uint32> Gone = { Far, Near };
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
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployAttackWhilePackedTest, "HeroesOfLiberty.Siege.Deploy.AttackWhilePackedDeploysThenFires", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployAttackWhilePackedTest::RunTest(const FString& Parameters)
{
	return UE::HeroesOfLiberty::Tests::DeployButton::RunAttackWhilePacked(*this, UE::HeroesOfLiberty::Tests::DeployButton::EAttackWhilePackedCase::InCombatReach);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployAttackBeyondDeployRangeTest, "HeroesOfLiberty.Siege.Deploy.AtTheCombatReachButBeyondDeployRange", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployAttackBeyondDeployRangeTest::RunTest(const FString& Parameters)
{
	return UE::HeroesOfLiberty::Tests::DeployButton::RunAttackWhilePacked(*this, UE::HeroesOfLiberty::Tests::DeployButton::EAttackWhilePackedCase::BeyondDeployRange);
}

// D877 (user answer 2026-11-21, Auto_Journey_File line 69): a packed ("demonte") engine that FINISHES a march mounts by itself, every engine.
// RED until the fix: today the engine stays packed at the goal. Real chain (queue, order processor, deploy processor, combat); no movement runs in
// this harness, so the goal is 10 cm from where the engine stands (the order processor's arrival closes the order). Effects measured: the march order
// is over (not walking), then the phase of the fragment reaches Deployed with no press on the button.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployArrivalMountsByItselfTest, "HeroesOfLiberty.Siege.Deploy.PackedEngineMountsByItselfAtTheEndOfAMarch", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployArrivalMountsByItselfTest::RunTest(const FString& Parameters)
{
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
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
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
		const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Orders, ProcessingContext);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		TArray<uint32> Before;
		Units->GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = FName(TEXT("Mangonel"));
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector::ZeroVector;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> After;
		Units->GetAllUnitIds(After);
		uint32 Engine = 0;
		for (const uint32 Id : After)
		{
			Engine = Before.Contains(Id) ? Engine : Id;
		}
		if (TestTrue(TEXT("the deployable Mangonel born"), Engine != 0))
		{
			const FMassEntityHandle Entity = Units->FindEntity(Engine);
			const auto PhaseOf = [&EntityManager, Entity]
			{
				const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
				return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
			};
			const auto Walking = [&EntityManager, Entity]
			{
				const FHOLMoveOrderFragment* Order = EntityManager->GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
				return Order && Order->Phase != EHOLMovePhase::None;
			};
			const TArray<uint32> Mine = { Engine };
			TestEqual(TEXT("precondition: the engine is born packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
			TestTrue(TEXT("the march order enters the queue"), HOLPlayerOrders::Move(*World, 0, Mine, FVector(10.0, 0.0, 0.0)));
			int32 Steps = 0;
			RunStep();
			RunStep();
			while (Walking() && Steps < 400)
			{
				RunStep();
				++Steps;
			}
			TestFalse(TEXT("precondition: the march is over (the engine arrived)"), Walking());
			const int32 DeploySteps = HOLDeploy::DefaultProfile().DeployMs / StepMs;
			for (int32 Index = 0; Index < DeploySteps + 20; ++Index)
			{
				RunStep();
			}
			TestEqual(TEXT("packed engine, march finished: it mounts by itself (Deployed, no press on the button)"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Deployed));
			const TArray<uint32> Gone = { Engine };
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
// D869b (ELROND-1120sa, recette 1120pb): the mount bar stayed at 0 dashes 0.3 s after the order and the second cycle stayed on "Montage...".
// The client reads only the replicated phase, so its clock (HOLDeploy::FWatch) must start at the ORDER, not at the first drawing of the face.
// The bar is computed by the real HOLDeploy::ProgressPermille (1.5 s to mount, one dash = 125 permille).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployMountBarAdvancesTest, "HeroesOfLiberty.Siege.Deploy.MountBarAdvancesFromServerPhase", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployMountBarAdvancesTest::RunTest(const FString& Parameters)
{
	const auto PermilleAt = [](HOLDeploy::FWatch& Watch, const int32 ObservedPhase, const double NowSeconds)
	{
		HOLDeploy::FState State;
		State.Phase = HOLDeploy::PhaseAt(ObservedPhase < 0 ? 2 : ObservedPhase); // as AHOLPlayerController::GetDeployProgress: no changing phase watched = settled, no bar (PhaseAt(-1) would be Packing)
		State.ElapsedMs = HOLDeploy::WatchElapsedMs(Watch, ObservedPhase, NowSeconds);
		return HOLDeploy::ProgressPermille(State, HOLDeploy::DefaultProfile(), 0);
	};
	HOLDeploy::FWatch Watch;
	HOLDeploy::NoteOrder(Watch, 0.0);
	// The first drawing that sees the phase comes 0.3 s after the order (the refresh is four times a second): at least one dash already.
	TestTrue(TEXT("0.3 s after the order the mount bar shows at least one dash (>= 125 permille)"), PermilleAt(Watch, 1, 0.3) >= 125);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeploySecondMountReturnsToPackTest, "HeroesOfLiberty.Siege.Deploy.SecondMountReturnsToPack", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeploySecondMountReturnsToPackTest::RunTest(const FString& Parameters)
{
	const auto PermilleAt = [](HOLDeploy::FWatch& Watch, const int32 ObservedPhase, const double NowSeconds)
	{
		HOLDeploy::FState State;
		State.Phase = HOLDeploy::PhaseAt(ObservedPhase < 0 ? 2 : ObservedPhase); // as AHOLPlayerController::GetDeployProgress: no changing phase watched = settled, no bar (PhaseAt(-1) would be Packing)
		State.ElapsedMs = HOLDeploy::WatchElapsedMs(Watch, ObservedPhase, NowSeconds);
		return HOLDeploy::ProgressPermille(State, HOLDeploy::DefaultProfile(), 0);
	};
	HOLDeploy::FWatch Watch;
	HOLDeploy::NoteOrder(Watch, 0.0);
	PermilleAt(Watch, 1, 0.3);
	const int32 DuringFirst = PermilleAt(Watch, 1, 1.4);
	TestTrue(TEXT("first cycle: the bar is nearly full at 1.4 s"), DuringFirst >= 875);
	// The engine is deployed (phase 2): the face leaves the bar for the settled key (Pack); nothing is read until the next order.
	TestEqual(TEXT("deployed: no bar, the face goes back to Pack"), PermilleAt(Watch, -1, 1.6), -1);
	// Second cycle with the clock untouched in between (phase 2 and 3 never drawn): a new order restarts the bar from 0.
	HOLDeploy::NoteOrder(Watch, 10.0);
	const int32 SecondMount = PermilleAt(Watch, 1, 10.3);
	TestTrue(TEXT("second cycle: 0.3 s after its order the bar is at one dash, not full (125..400 permille)"), SecondMount >= 125 && SecondMount <= 400);
	// A mount ordered while a stale phase-1 clock was still held (the phase was never read as settled in between).
	HOLDeploy::FWatch Stale;
	HOLDeploy::NoteOrder(Stale, 0.0);
	PermilleAt(Stale, 1, 0.3);
	HOLDeploy::NoteOrder(Stale, 10.0);
	const int32 SecondStale = PermilleAt(Stale, 1, 10.3);
	TestTrue(TEXT("mount after a mount whose end was never read: the bar restarts (125..400 permille)"), SecondStale >= 125 && SecondStale <= 400);
	return true;
}
// D877 half 2 (user answer 2026-11-21, Auto_Journey_File line 72): a right click on a target while the engine is packed ("demonte") = it comes into
// reach if need be, mounts, then fires. Real chain (queue, order processor, deploy processor, combat). No movement runs in this harness, so a stand-in
// mover (below) advances the engine's transform 50 cm a step while its walk is open (it comes from the combat's approach: the engine stays packed
// while it walks). The foe (a Spearman of slot 3 on Hold) stands 2000 cm beyond the deploy range, so the engine must come closer first.
// Effects measured: the engine's phase reaches Deployed (no press on the button), then the foe's health goes down.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployClickOnFarTargetWhilePackedTest, "HeroesOfLiberty.Siege.Deploy.ClickOnFarTargetWhilePackedComesInMountsAndFires", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployClickOnFarTargetWhilePackedTest::RunTest(const FString& Parameters)
{
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
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
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
		const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Orders, ProcessingContext);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [&](const FName Type, const int32 Owner, const FVector& At)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Type;
			Request.Count = 1;
			Request.OwnerPlayer = Owner;
			Request.Center = At;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Born = 0;
			for (const uint32 Id : After)
			{
				Born = Before.Contains(Id) ? Born : Id;
			}
			return Born;
		};
		const HOLDeploy::FProfile Profile = HOLDeploy::DefaultProfile();
		const double FoeX = static_cast<double>(Profile.MaxRangeCm) + 2000.0;
		const uint32 Engine = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
		const uint32 Foe = Place(FName(TEXT("Spearman")), 3, FVector(FoeX, 0.0, 0.0));
		if (TestTrue(TEXT("the deployable Mangonel born"), Engine != 0) && TestTrue(TEXT("the target born"), Foe != 0))
		{
			const FMassEntityHandle EngineEntity = Units->FindEntity(Engine);
			const FMassEntityHandle FoeEntity = Units->FindEntity(Foe);
			FHOLCombatFragment* FoeCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(FoeEntity);
			if (TestNotNull(TEXT("target combat"), FoeCombat))
			{
				FoeCombat->Mode = EHOLCombatMode::Hold;
				const auto PhaseOf = [&EntityManager, EngineEntity]
				{
					const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
				};
				const auto FoeHealth = [&EntityManager, FoeEntity]
				{
					const FHOLUnitStateFragment* State = EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(FoeEntity);
					return State ? State->CurrentHealth : 0.f;
				};
				// The stand-in mover: the engine walks to the foe at 50 cm a step while its (combat's) walk is open and it is packed.
				const auto MoveIfWalking = [&EntityManager, EngineEntity, FoeX]
				{
					const FHOLMoveOrderFragment* Order = EntityManager->GetFragmentDataPtr<FHOLMoveOrderFragment>(EngineEntity);
					FTransformFragment* Transform = EntityManager->GetFragmentDataPtr<FTransformFragment>(EngineEntity);
					const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					if (Order && Transform && Fragment && Order->Phase != EHOLMovePhase::None && Fragment->Phase == static_cast<uint8>(HOLDeploy::EPhase::Packed))
					{
						FVector Location = Transform->GetTransform().GetLocation();
						Location.X = FMath::Min(Location.X + 50.0, FoeX);
						Transform->GetMutableTransform().SetLocation(Location);
					}
				};
				const TArray<uint32> Mine = { Engine };
				TestEqual(TEXT("precondition: the engine is born packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
				const float Start = FoeHealth();
				TestTrue(TEXT("the attack order on the far unit enters the queue"), HOLPlayerOrders::Attack(*World, 0, Mine, Foe));
				const int32 DeploySteps = Profile.DeployMs / StepMs;
				for (int32 Index = 0; Index < 800 && PhaseOf() != static_cast<int32>(HOLDeploy::EPhase::Deployed); ++Index)
				{
					MoveIfWalking();
					RunStep();
				}
				for (int32 Index = 0; Index < DeploySteps + 20; ++Index)
				{
					RunStep();
				}
				TestEqual(TEXT("packed, clicked on a far unit: it comes into reach and ends deployed (no press on the button)"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Deployed));
				bool bHit = FoeHealth() < Start;
				for (int32 Index = 0; Index < 600 && !bHit; ++Index)
				{
					RunStep();
					bHit = FoeHealth() < Start;
				}
				TestTrue(TEXT("deployed after the click, the engine fires: the far unit's health goes down"), bHit);
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

// D877 half 2, cause of ELROND-1121sd: a packed engine MARCHING (a player Move open, bMarching = 1) gets a right click attack on a far unit. The attack order cuts the
// march (ApplyCombatOrder: Phase None) and must also drop the engine's march note: otherwise UHOLDeployProcessor reads "march ended" and mounts it on the spot (Toggle)
// while the foe is still out of reach. Expected: it stays Packed while far, comes in, mounts in reach, and the foe loses health.
// D877 half 2 (user answer 2026-11-21, Auto_Journey_File line 72): a right click on a target while the engine is packed ("demonte") = it comes into
// reach if need be, mounts, then fires. Real chain (queue, order processor, deploy processor, combat). No movement runs in this harness, so a stand-in
// mover (below) advances the engine's transform 50 cm a step while its walk is open (it comes from the combat's approach: the engine stays packed
// while it walks). The foe (a Spearman of slot 3 on Hold) stands 2000 cm beyond the deploy range, so the engine must come closer first.
// Effects measured: the engine's phase reaches Deployed (no press on the button), then the foe's health goes down.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployAttackCutsOpenMarchTest, "HeroesOfLiberty.Siege.Deploy.AttackCutsAnOpenMarchAndComesInMountsAndFires", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployAttackCutsOpenMarchTest::RunTest(const FString& Parameters)
{
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
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem))
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
		const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Orders, ProcessingContext);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Place = [&](const FName Type, const int32 Owner, const FVector& At)
		{
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = Type;
			Request.Count = 1;
			Request.OwnerPlayer = Owner;
			Request.Center = At;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Born = 0;
			for (const uint32 Id : After)
			{
				Born = Before.Contains(Id) ? Born : Id;
			}
			return Born;
		};
		const HOLDeploy::FProfile Profile = HOLDeploy::DefaultProfile();
		const double FoeX = static_cast<double>(Profile.MaxRangeCm) + 2000.0;
		const uint32 Engine = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
		const uint32 Foe = Place(FName(TEXT("Spearman")), 3, FVector(FoeX, 0.0, 0.0));
		if (TestTrue(TEXT("the deployable Mangonel born"), Engine != 0) && TestTrue(TEXT("the target born"), Foe != 0))
		{
			const FMassEntityHandle EngineEntity = Units->FindEntity(Engine);
			const FMassEntityHandle FoeEntity = Units->FindEntity(Foe);
			FHOLCombatFragment* FoeCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(FoeEntity);
			if (TestNotNull(TEXT("target combat"), FoeCombat))
			{
				FoeCombat->Mode = EHOLCombatMode::Hold;
				const auto PhaseOf = [&EntityManager, EngineEntity]
				{
					const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
				};
				const auto FoeHealth = [&EntityManager, FoeEntity]
				{
					const FHOLUnitStateFragment* State = EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(FoeEntity);
					return State ? State->CurrentHealth : 0.f;
				};
				// The stand-in mover: the engine walks to the foe at 50 cm a step while its (combat's) walk is open and it is packed.
				const auto MoveIfWalking = [&EntityManager, EngineEntity, FoeX]
				{
					const FHOLMoveOrderFragment* Order = EntityManager->GetFragmentDataPtr<FHOLMoveOrderFragment>(EngineEntity);
					FTransformFragment* Transform = EntityManager->GetFragmentDataPtr<FTransformFragment>(EngineEntity);
					const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					if (Order && Transform && Fragment && Order->Phase != EHOLMovePhase::None && Fragment->Phase == static_cast<uint8>(HOLDeploy::EPhase::Packed))
					{
						FVector Location = Transform->GetTransform().GetLocation();
						Location.X = FMath::Min(Location.X + 50.0, FoeX);
						Transform->GetMutableTransform().SetLocation(Location);
					}
				};
				const TArray<uint32> Mine = { Engine };
				TestTrue(TEXT("a player march order enters the queue"), HOLPlayerOrders::Move(*World, 0, Mine, FVector(1000.0, 0.0, 0.0)));
				RunStep();
				{
					const FHOLDeployFragment* Marching = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					TestTrue(TEXT("precondition: the march is open and noted (bMarching = 1)"), Marching && Marching->bMarching == 1);
				}
				TestEqual(TEXT("precondition: the engine is born packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
				const float Start = FoeHealth();
				TestTrue(TEXT("the attack order on the far unit enters the queue"), HOLPlayerOrders::Attack(*World, 0, Mine, Foe));
				const int32 DeploySteps = Profile.DeployMs / StepMs;
				bool bMountedFar = false;
				for (int32 Index = 0; Index < 800 && PhaseOf() != static_cast<int32>(HOLDeploy::EPhase::Deployed); ++Index)
				{
					const FTransformFragment* At = EntityManager->GetFragmentDataPtr<FTransformFragment>(EngineEntity);
					bMountedFar |= At && PhaseOf() != static_cast<int32>(HOLDeploy::EPhase::Packed) && FoeX - At->GetTransform().GetLocation().X > static_cast<double>(Profile.MaxRangeCm) + 100.0;
					MoveIfWalking();
					RunStep();
				}
				for (int32 Index = 0; Index < DeploySteps + 20; ++Index)
				{
					RunStep();
				}
				TestFalse(TEXT("a march cut by the attack does not mount the engine on the spot (it stays packed while the foe is out of reach)"), bMountedFar);
				TestEqual(TEXT("packed, clicked on a far unit: it comes into reach and ends deployed (no press on the button)"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Deployed));
				bool bHit = FoeHealth() < Start;
				for (int32 Index = 0; Index < 600 && !bHit; ++Index)
				{
					RunStep();
					bHit = FoeHealth() < Start;
				}
				TestTrue(TEXT("deployed after the click, the engine fires: the far unit's health goes down"), bHit);
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

// ELROND-1122e (D882, option B; D877 unchanged): a Stop during the player's march CUTS it and the engine STAYS packed ("demonte"); it mounts only by
// the button D, an attack, or the NORMAL end of a walk. Real chain (queue, order processor, deploy processor, combat); no movement system runs in
// this harness, so a stand-in mover advances the engine 50 cm a step while its walk is open, and closes the walk (Phase None) on arrival as the
// Mass movement does. Effect measured: the engine's phase (Packed all along after the Stop; Deploying or Deployed after a normal arrival).
namespace HOLDeployStopMarch
{
	void Run(FAutomationTestBase& Test, const bool bStop)
	{
		IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
		IConsoleVariable* Deployable = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Siege.Deployable"));
		if (!Test.TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled) || !Test.TestNotNull(TEXT("hol.Siege.Deployable"), Deployable))
		{
			return;
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
		if (Test.TestNotNull(TEXT("units"), Units) && Test.TestNotNull(TEXT("simulation"), Simulation) && Test.TestNotNull(TEXT("entities"), EntitySubsystem))
		{
			TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			UHOLUnitOrderProcessor* Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
			UHOLDeployProcessor* Deploy = NewObject<UHOLDeployProcessor>(Simulation);
			Deploy->CallInitialize(Simulation, EntityManager.ToSharedRef());
			UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
			Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			const auto RunStep = [Simulation, Units, Orders, Deploy, Combat, &EntityManager, StepSeconds]
			{
				const int64 Step = Simulation->AdvanceStepIndexForTests();
				Simulation->GetOnStepStarted().Broadcast(Step);
				Units->ServeBirthRequests();
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
				UE::Mass::Executor::Run(*Deploy, ProcessingContext);
				UE::Mass::Executor::Run(*Combat, ProcessingContext);
				Units->FlushDestructions();
				Simulation->GetOnStepEnded().Broadcast(Step);
			};
			TArray<uint32> Before;
			Units->GetAllUnitIds(Before);
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FName(TEXT("Mangonel"));
			Request.Count = 1;
			Request.OwnerPlayer = 0;
			Request.Center = FVector::ZeroVector;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			TArray<uint32> After;
			Units->GetAllUnitIds(After);
			uint32 Engine = 0;
			for (const uint32 Id : After)
			{
				Engine = Before.Contains(Id) ? Engine : Id;
			}
			if (Test.TestTrue(TEXT("the deployable Mangonel born"), Engine != 0))
			{
				const FMassEntityHandle EngineEntity = Units->FindEntity(Engine);
				const double GoalX = 1000.0;
				const auto PhaseOf = [&EntityManager, EngineEntity]
				{
					const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					return Fragment ? static_cast<int32>(Fragment->Phase) : -1;
				};
				const auto XOf = [&EntityManager, EngineEntity]
				{
					const FTransformFragment* At = EntityManager->GetFragmentDataPtr<FTransformFragment>(EngineEntity);
					return At ? At->GetTransform().GetLocation().X : -1.0;
				};
				// The stand-in mover: 50 cm a step while the walk is open and the engine packed; the walk closes on arrival (as Mass does).
				const auto MoveIfWalking = [&EntityManager, EngineEntity, GoalX]
				{
					FHOLMoveOrderFragment* Order = EntityManager->GetFragmentDataPtr<FHOLMoveOrderFragment>(EngineEntity);
					FTransformFragment* Transform = EntityManager->GetFragmentDataPtr<FTransformFragment>(EngineEntity);
					const FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					if (Order && Transform && Fragment && Order->Phase != EHOLMovePhase::None && Fragment->Phase == static_cast<uint8>(HOLDeploy::EPhase::Packed))
					{
						FVector Location = Transform->GetTransform().GetLocation();
						Location.X = FMath::Min(Location.X + 50.0, GoalX);
						Transform->GetMutableTransform().SetLocation(Location);
						if (Location.X >= GoalX)
						{
							Order->Phase = EHOLMovePhase::None;
						}
					}
				};
				const TArray<uint32> Mine = { Engine };
				Test.TestTrue(TEXT("a player march order enters the queue"), HOLPlayerOrders::Move(*World, 0, Mine, FVector(GoalX, 0.0, 0.0)));
				RunStep();
				{
					const FHOLDeployFragment* Marching = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					Test.TestTrue(TEXT("precondition: the march is open and noted (bMarching = 1)"), Marching && Marching->bMarching == 1);
				}
				Test.TestEqual(TEXT("precondition: the engine is born packed"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
				for (int32 Index = 0; Index < 5; ++Index)
				{
					MoveIfWalking();
					RunStep();
				}
				Test.TestTrue(TEXT("precondition: the march is under way, not over (250 cm of 1000)"), XOf() > 0.0 && XOf() < GoalX);
				if (bStop)
				{
					const double StopX = XOf();
					Test.TestTrue(TEXT("the Stop order enters the queue"), HOLPlayerOrders::Stop(*World, 0, Mine, false));
					RunStep();
					const FHOLDeployFragment* Cut = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(EngineEntity);
					const FHOLMoveOrderFragment* CutOrder = EntityManager->GetFragmentDataPtr<FHOLMoveOrderFragment>(EngineEntity);
					Test.TestTrue(TEXT("the Stop cut the march note (bMarching = 0)"), Cut && Cut->bMarching == 0);
					Test.TestTrue(TEXT("the Stop closed the walk"), CutOrder && CutOrder->Phase == EHOLMovePhase::None);
					// N seconds: the engine must not mount by itself (no "march ended" read from the cut walk).
					const int32 HoldSteps = FMath::CeilToInt(10.f / StepSeconds);
					bool bLeftPacked = false;
					for (int32 Index = 0; Index < HoldSteps; ++Index)
					{
						MoveIfWalking();
						RunStep();
						bLeftPacked |= PhaseOf() != static_cast<int32>(HOLDeploy::EPhase::Packed);
					}
					Test.TestFalse(TEXT("after a Stop during the march the engine never leaves Packed by itself (D882 B)"), bLeftPacked);
					Test.TestEqual(TEXT("after a Stop during the march the engine is still packed 10 s later"), PhaseOf(), static_cast<int32>(HOLDeploy::EPhase::Packed));
					Test.TestTrue(TEXT("the engine did not walk on after the Stop"), XOf() <= StopX + 1.0);
				}
				else
				{
					// D877 counter-case: the march ends NORMALLY (the walk closes on arrival) while packed: the engine mounts by itself.
					bool bMounted = false;
					for (int32 Index = 0; Index < 200 && !bMounted; ++Index)
					{
						MoveIfWalking();
						RunStep();
						bMounted = PhaseOf() != static_cast<int32>(HOLDeploy::EPhase::Packed);
					}
					Test.TestTrue(TEXT("the engine reached the end of its march"), XOf() >= GoalX);
					Test.TestTrue(TEXT("D877: a march that ends normally while packed mounts the engine (Deploying or Deployed)"), bMounted);
				}
				const TArray<uint32> Gone = { Engine };
				Units->RequestDestroyUnits(Gone);
				Units->FlushDestructions();
			}
		}
		CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
		Deployable->Set(FormerDeployable, ECVF_SetByCode);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployStopCutsMarchStaysPackedTest, "HeroesOfLiberty.Siege.Deploy.StopDuringMarchStaysPackedAndDoesNotMountByItself", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployStopCutsMarchStaysPackedTest::RunTest(const FString& Parameters)
{
	HOLDeployStopMarch::Run(*this, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployNormalMarchEndMountsTest, "HeroesOfLiberty.Siege.Deploy.NormalMarchEndWhilePackedMountsTheEngine", UE::HeroesOfLiberty::Tests::DeployButton::Flags)

bool FHOLDeployNormalMarchEndMountsTest::RunTest(const FString& Parameters)
{
	HOLDeployStopMarch::Run(*this, false);
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
