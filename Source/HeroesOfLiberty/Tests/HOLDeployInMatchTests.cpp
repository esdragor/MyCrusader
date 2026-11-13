// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
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

// Palier3InProgress.md G6, steps b and c: the Mangonel stands in for the deployable engine (cvar hol.Siege.Deployable, default 0: no unit of the
// game carries the deploy fragment). Through the REAL combat and deploy processors: without the cvar the Mangonel fires at once at its ordered
// target; with it, it is born packed with the fragment, does not fire while it deploys (the effect: the target's health does not move), and
// fires once deployed. Its speed is nil while it is not packed.
namespace UE::HeroesOfLiberty::Tests::DeployInMatch
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployInMatchTest, "HeroesOfLiberty.Siege.Deploy.InMatch", UE::HeroesOfLiberty::Tests::DeployInMatch::Flags)

bool FHOLDeployInMatchTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DeployInMatch;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	IConsoleVariable* Deployable = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Siege.Deployable"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled) || !TestNotNull(TEXT("hol.Siege.Deployable"), Deployable))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	const int32 FormerDeployable = Deployable->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);

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
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLDeployProcessor* Deploy = NewObject<UHOLDeployProcessor>(Simulation);
		Deploy->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 StepMs = FMath::RoundToInt(StepSeconds * 1000.f);
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, Deploy, &EntityManager, StepSeconds, &Step]
		{
			// The simulation's own step index must move: an arrow in flight (the Mangonel's blow) lands when it reaches its ArrivalStep.
			Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		// Born again until its identifier is a multiple of 4 (one search step of 4), as HeroesOfLiberty.Duel.ProtectedInMatch.
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
		// The Mangonel of slot 0 at the origin, ordered on a Spearman of slot 3 (slot 1 is on slot 0's team by default: the first run of this
		// test landed no blow on it, 2026-11-11) holding its ground 20 m away.
		const auto Setup = [&](uint32& OutEngine, uint32& OutTarget)
		{
			OutEngine = Place(FName(TEXT("Mangonel")), 0, FVector::ZeroVector);
			OutTarget = Place(FName(TEXT("Spearman")), 3, FVector(2000.0, 0.0, 0.0)); // beyond the proposed minimum range (1200 cm)
			if (OutEngine == 0 || OutTarget == 0)
			{
				return false;
			}
			const FMassEntityHandle Engine = Units->FindEntity(OutEngine);
			const FMassEntityHandle Target = Units->FindEntity(OutTarget);
			FHOLCombatFragment* EngineCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Engine);
			FHOLCombatFragment* TargetCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Target);
			if (EngineCombat == nullptr || TargetCombat == nullptr)
			{
				return false;
			}
			TargetCombat->Mode = EHOLCombatMode::Hold;
			EngineCombat->Target = Target;
			EngineCombat->bOrderedTarget = true;
			return true;
		};
		const auto Remove = [Units](const uint32 A, const uint32 B)
		{
			const TArray<uint32> Gone = { A, B };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
		};
		const int32 TenSeconds = FMath::CeilToInt(10.f / StepSeconds);
		const HOLDeploy::FProfile Profile = HOLDeploy::DefaultProfile();
		const int32 DeploySteps = Profile.DeployMs / StepMs;

		// 1. Without the cvar, no deploy fragment: the Mangonel fires at its target.
		Deployable->Set(0, ECVF_SetByCode);
		uint32 Engine = 0;
		uint32 Foe = 0;
		if (TestTrue(TEXT("control: the Mangonel and its target born"), Setup(Engine, Foe)))
		{
			TestNull(TEXT("control: no deploy fragment without the cvar"), EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Units->FindEntity(Engine)));
			const float Start = HealthOf(Foe);
			bool bHit = false;
			for (int32 Index = 0; Index < TenSeconds && !bHit; ++Index)
			{
				RunStep();
				bHit = HealthOf(Foe) < Start;
			}
			TestTrue(TEXT("control: the Mangonel hurts its target"), bHit);
			if (!bHit)
			{
				const FHOLCombatFragment* Seen = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(Engine));
				AddInfo(FString::Printf(TEXT("diag: engine hits %d attacking %d approaching %d cooldown %d target set %d, foe health %.1f (start %.1f)"),
					Seen ? Seen->Hits : -1, Seen ? Seen->bAttacking : -1, Seen ? Seen->bApproaching : -1, Seen ? Seen->CooldownMs : -1,
					Seen ? Seen->Target.IsSet() : -1, HealthOf(Foe), Start));
			}
			Remove(Engine, Foe);
		}

		// 2. With the cvar: born packed with the fragment, silent while it deploys, then it fires.
		Deployable->Set(1, ECVF_SetByCode);
		if (TestTrue(TEXT("the deployable Mangonel and its target born"), Setup(Engine, Foe)))
		{
			const FMassEntityHandle Entity = Units->FindEntity(Engine);
			FHOLDeployFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
			if (TestNotNull(TEXT("the Mangonel carries the deploy fragment with the cvar"), Fragment))
			{
				TestEqual(TEXT("born packed"), static_cast<int32>(Fragment->Phase), static_cast<int32>(HOLDeploy::EPhase::Packed));
				const float Start = HealthOf(Foe);
				bool bHitEarly = false;
				bool bDeploying = false;
				for (int32 Index = 0; Index < DeploySteps / 2; ++Index)
				{
					RunStep();
					Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
					bHitEarly |= HealthOf(Foe) < Start;
					bDeploying |= Fragment && Fragment->Phase == static_cast<uint8>(HOLDeploy::EPhase::Deploying);
				}
				TestTrue(TEXT("its ordered attack starts the deploying"), bDeploying);
				TestFalse(TEXT("it does not fire while it deploys"), bHitEarly);
				bool bHit = false;
				for (int32 Index = 0; Index < DeploySteps + TenSeconds && !bHit; ++Index)
				{
					RunStep();
					bHit = HealthOf(Foe) < Start;
				}
				Fragment = EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Entity);
				TestTrue(TEXT("deployed, it fires"), bHit && Fragment && Fragment->Phase == static_cast<uint8>(HOLDeploy::EPhase::Deployed));
			}
			Remove(Engine, Foe);
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	Deployable->Set(FormerDeployable, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// Palier3InProgress.md G6 f (preflight deploy 16:45, step 38 'waitlog Mangonel hits a building for 45' never came): a DEPLOYED engine at rest,
// with no order, near an enemy House within its building reach. Its AggressionRadius is 0 (UnitTypes.csv: 'only on order'), so it never takes a
// building on by itself. Retours/Q-deployed-engine-autofire.md: cvar hol.Siege.DeployedAutoFire, 0 (default, A) only on order; 1 (B) a deployed
// engine at rest takes on an enemy building within its building reach. Through the REAL combat and deploy processors; the effect: the House's health.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDeployedAutoFireTest, "HeroesOfLiberty.Siege.Deploy.DeployedAutoFire", UE::HeroesOfLiberty::Tests::DeployInMatch::Flags)

bool FHOLDeployedAutoFireTest::RunTest(const FString& Parameters)
{
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	IConsoleVariable* Deployable = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Siege.Deployable"));
	IConsoleVariable* AutoFire = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Siege.DeployedAutoFire"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled) || !TestNotNull(TEXT("hol.Siege.Deployable"), Deployable))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	const int32 FormerDeployable = Deployable->GetInt();
	const int32 FormerAutoFire = AutoFire ? AutoFire->GetInt() : 0;
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
	// The enemy House of slot 3 (slot 1 is on slot 0's team by default), its centre 2200 cm off: the preflight's layout, within the reach (3000).
	AHOLBuilding* House = World->SpawnActor<AHOLBuilding>(FVector(2200.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("entities"), EntitySubsystem)
		&& TestNotNull(TEXT("house"), House))
	{
		House->Setup(HOLBuildingTypeNames::House, 3, 3);
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLDeployProcessor* Deploy = NewObject<UHOLDeployProcessor>(Simulation);
		Deploy->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const auto RunStep = [Simulation, Units, Combat, Deploy, &EntityManager, StepSeconds]
		{
			const int64 Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Deploy, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		// A Mangonel of slot 0 at the origin, deployed at once (the deploying itself is HeroesOfLiberty.Siege.Deploy.InMatch's), no order.
		const auto PlaceDeployed = [Units, &EntityManager]
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
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
				uint32 Born = 0;
				for (const uint32 Id : After)
				{
					Born = Before.Contains(Id) ? Born : Id;
				}
				if (Born == 0 || Born % 4 == 0)
				{
					FHOLDeployFragment* Fragment = Born != 0 ? EntityManager->GetFragmentDataPtr<FHOLDeployFragment>(Units->FindEntity(Born)) : nullptr;
					if (Fragment)
					{
						Fragment->Phase = static_cast<uint8>(HOLDeploy::EPhase::Deployed);
					}
					return Fragment ? Born : 0u;
				}
				const uint32 Again[] = { Born };
				Units->RequestDestroyUnits(Again);
				Units->FlushDestructions();
			}
			return 0u;
		};
		const int32 TenSeconds = FMath::CeilToInt(10.f / StepSeconds);
		const auto Fires = [&](const int32 Value, const TCHAR* What)
		{
			if (AutoFire)
			{
				AutoFire->Set(Value, ECVF_SetByCode);
			}
			const uint32 Engine = PlaceDeployed();
			if (!TestTrue(FString::Printf(TEXT("%s: the deployed Mangonel born"), What), Engine != 0))
			{
				return false;
			}
			const float Start = House->GetHealth();
			bool bHit = false;
			for (int32 Index = 0; Index < TenSeconds && !bHit; ++Index)
			{
				RunStep();
				bHit = House->GetHealth() < Start;
			}
			const TArray<uint32> Gone = { Engine };
			Units->RequestDestroyUnits(Gone);
			Units->FlushDestructions();
			return bHit;
		};
		// 0 (A, the default): only on order, the House untouched.
		TestFalse(TEXT("hol.Siege.DeployedAutoFire 0: the deployed engine at rest does not fire at the House by itself"), Fires(0, TEXT("A")));
		// 1 (B): the deployed engine at rest fires at the enemy House within its building reach.
		TestTrue(TEXT("hol.Siege.DeployedAutoFire 1: the deployed engine at rest fires at the enemy House within its reach"), Fires(1, TEXT("B")));
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	Deployable->Set(FormerDeployable, ECVF_SetByCode);
	if (AutoFire)
	{
		AutoFire->Set(FormerAutoFire, ECVF_SetByCode);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
