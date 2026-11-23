// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Match/HOLGameState.h"
#include "Simulation/HOLHealProcessor.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLZoneEffectRules.h"

namespace
{
	// AHOLGameState::GetFactionResource is private in C++ (the script reads it by reflection): the test reads the gauge the same way, no change of visibility.
	int32 ReadFactionGauge(AHOLGameState* State, const int32 Slot)
	{
		UFunction* Getter = State ? State->FindFunction(TEXT("GetFactionResource")) : nullptr;
		if (!Getter)
		{
			return -1;
		}
		struct { int32 Slot; int32 ReturnValue; } Params{ Slot, 0 };
		State->ProcessEvent(Getter, &Params);
		return Params.ReturnValue;
	}
}

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::ZoneEffect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	const FName Spearman(TEXT("Spearman"));
}

// D859, Seal of Light: the pure rules of a zone on the ground. Geometry chosen by hand (a 3-4-5 triangle: the point (300, 400) is exactly
// 500 cm from the centre), so the expected answers do not come from the code under test.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectRulesTest, "HeroesOfLiberty.Zone.Rules", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectRulesTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("the centre is in the zone"), HOLZoneEffect::IsInZone(1000, 2000, 600, 1000, 2000));
	TestTrue(TEXT("exactly on the edge (a 3-4-5 triangle, 500 cm): in"), HOLZoneEffect::IsInZone(0, 0, 500, 300, 400));
	TestFalse(TEXT("one centimetre beyond the edge: out"), HOLZoneEffect::IsInZone(0, 0, 499, 300, 400));
	TestFalse(TEXT("far away: out"), HOLZoneEffect::IsInZone(0, 0, 600, 5000, 0));

	TestFalse(TEXT("before its start: not active"), HOLZoneEffect::ZoneActive(5000, 10000, 4999));
	TestTrue(TEXT("at its start: active"), HOLZoneEffect::ZoneActive(5000, 10000, 5000));
	TestTrue(TEXT("one millisecond before its end: active"), HOLZoneEffect::ZoneActive(5000, 10000, 14999));
	TestFalse(TEXT("at its end (start + duration): over"), HOLZoneEffect::ZoneActive(5000, 10000, 15000));

	TestEqual(TEXT("an enemy in an active zone, -40 %: 60"), HOLZoneEffect::GetSpeedPercent(true, true, true, 40), 60);
	TestEqual(TEXT("not an enemy: 100"), HOLZoneEffect::GetSpeedPercent(false, true, true, 40), 100);
	TestEqual(TEXT("out of the zone: 100"), HOLZoneEffect::GetSpeedPercent(true, false, true, 40), 100);
	TestEqual(TEXT("the zone over: 100"), HOLZoneEffect::GetSpeedPercent(true, true, false, 40), 100);
	TestEqual(TEXT("a slow of more than 100 %: held in place (0), never negative"), HOLZoneEffect::GetSpeedPercent(true, true, true, 250), 0);
	return true;
}

// D859, Seal of Light, the EFFECT: the slot puts a Seal at a point (UHOLHeroSubsystem::StartLightSeal); through the real combat processor an
// ENEMY unit in the zone takes 60 % of its walk (FHOLCombatFragment::ZoneSpeedPercent, applied by the movement processor), the slot's own unit
// in the zone and an enemy out of it keep 100 %, and once the 10 s are over the enemy walks at 100 % again. Nobody fights (hol.Combat.Enabled 0).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectSlowTest, "HeroesOfLiberty.Zone.Slow", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectSlowTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ZoneEffect;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(0, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("heroes"), Heroes)
		&& TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		Heroes->BindSteps();
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
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
		const auto SpeedOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			const FHOLCombatFragment* Fragment = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
			return Fragment ? static_cast<int32>(Fragment->ZoneSpeedPercent) : -1;
		};

		// The zone of slot 0 at the origin (6 m): an enemy (slot 1) 3 m from the centre, another 9 m from it, a unit of slot 0 3 m from it.
		const uint32 EnemyIn = Place(Spearman, 1, FVector(300.0, 0.0, 0.0));
		const uint32 EnemyOut = Place(Spearman, 1, FVector(900.0, 0.0, 0.0));
		const uint32 Own = Place(Spearman, 0, FVector(0.0, 300.0, 0.0));
		if (TestTrue(TEXT("the three units born"), EnemyIn != 0 && EnemyOut != 0 && Own != 0))
		{
			RunStep();
			TestEqual(TEXT("before the Seal: the enemy in the future zone walks at 100"), SpeedOf(EnemyIn), 100);
			TestTrue(TEXT("slot 0 puts the Seal of Light at the origin"), Heroes->StartLightSeal(0, FVector(0.0, 0.0, 0.0)));
			RunStep();
			TestEqual(TEXT("the enemy 3 m from the centre: -40 %, 60"), SpeedOf(EnemyIn), 60);
			TestEqual(TEXT("the enemy 9 m from the centre, out of the 6 m zone: 100"), SpeedOf(EnemyOut), 100);
			TestEqual(TEXT("the slot's own unit in the zone: 100"), SpeedOf(Own), 100);
			// Its 10 s over: the whole speed again.
			for (int32 Steps = 0; Steps < FMath::CeilToInt(HOLZoneEffect::LightSealDurationMs / 1000.f / StepSeconds) + 2; ++Steps)
			{
				RunStep();
			}
			TestEqual(TEXT("the Seal over: the enemy walks at 100 again"), SpeedOf(EnemyIn), 100);
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// D689, Sacred Light, the EFFECT (S1-a): the slot puts a Sacred Light at a point (UHOLHeroSubsystem::StartSacredLight); through the real heal
// processor (no healer on the field: it must wake for the zone) a WOUNDED ALLY in the zone gains health over the steps at the zone's rate, a
// wounded ally out of the zone and a wounded ENEMY in it gain nothing, and once the ~5 s are over the heal stops. The health read is the
// fragment's, after the steps the processor really ran (not a constant of the rules).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectSacredLightTest, "HeroesOfLiberty.Zone.SacredLight", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectSacredLightTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ZoneEffect;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(0, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("heroes"), Heroes)
		&& TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		Heroes->BindSteps();
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, Heal, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			UE::Mass::Executor::Run(*Heal, ProcessingContext);
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
		const auto StateOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};

		// Sacred Light of slot 0 at the origin (6 m): its own unit 3 m from the centre, another 9 m from it, an enemy (slot 1) 3 m from it.
		const uint32 AllyIn = Place(Spearman, 0, FVector(300.0, 0.0, 0.0));
		const uint32 AllyOut = Place(Spearman, 0, FVector(900.0, 0.0, 0.0));
		const uint32 EnemyIn = Place(Spearman, 1, FVector(0.0, 300.0, 0.0));
		if (TestTrue(TEXT("the three units born"), AllyIn != 0 && AllyOut != 0 && EnemyIn != 0))
		{
			RunStep();
			FHOLUnitStateFragment* InState = StateOf(AllyIn);
			FHOLUnitStateFragment* OutState = StateOf(AllyOut);
			FHOLUnitStateFragment* EnemyState = StateOf(EnemyIn);
			if (TestTrue(TEXT("the three states"), InState && OutState && EnemyState))
			{
				// Wound the three units to 20 (their full health is much higher): the heal has room to show.
				InState->CurrentHealth = 20.f;
				OutState->CurrentHealth = 20.f;
				EnemyState->CurrentHealth = 20.f;
				TestTrue(TEXT("slot 0 puts the Sacred Light at the origin"), Heroes->StartSacredLight(0, FVector(0.0, 0.0, 0.0)));
				const int32 HealSteps = FMath::CeilToInt(1.f / StepSeconds);
				for (int32 Steps = 0; Steps < HealSteps; ++Steps)
				{
					RunStep();
				}
				const float SecondGain = InState->CurrentHealth - 20.f;
				const float Expected = static_cast<float>(HOLZoneEffect::SacredLightHealPerSecond);
				TestTrue(*FString::Printf(TEXT("the ally in the zone gained about one second of heal (%.2f, expected %.2f)"), SecondGain, Expected),
					SecondGain > Expected * 0.8f && SecondGain < Expected * 1.3f);
				TestEqual(TEXT("the ally 9 m from the centre, out of the 6 m zone: not healed"), OutState->CurrentHealth, 20.f);
				TestEqual(TEXT("the enemy in the zone: not healed"), EnemyState->CurrentHealth, 20.f);
				// Its ~5 s over: the heal stops.
				for (int32 Steps = 0; Steps < FMath::CeilToInt(HOLZoneEffect::SacredLightDurationMs / 1000.f / StepSeconds) + 2; ++Steps)
				{
					RunStep();
				}
				const float AfterZone = InState->CurrentHealth;
				TestTrue(TEXT("the ally in the zone gained more over the whole zone"), AfterZone > 20.f + SecondGain);
				for (int32 Steps = 0; Steps < HealSteps; ++Steps)
				{
					RunStep();
				}
				TestEqual(TEXT("the Sacred Light over: the heal has stopped"), InState->CurrentHealth, AfterZone);
			}
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}


// D689, Honor of the Dawn from the heal (S1-e): the Sacred Light of slot 0 heals a wounded ally; the processor credits slot 0's faction gauge
// (AHOLGameState::FactionResource) one Honor per HpPerPoint hit points healed. The expected Honor is CALCULATED from the health the fragment
// really gained (floor(gain / HpPerPoint)), never a constant; a slot that healed nothing (slot 1) gets nothing; the gauge stops at the cap.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectHealHonorTest, "HeroesOfLiberty.Zone.HealHonor", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectHealHonorTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ZoneEffect;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(0, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	World->SetGameState(State);

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("heroes"), Heroes) && TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		Heroes->BindSteps();
		constexpr int32 HpPerPoint = 20;
		constexpr int32 Cap = 100;
		UHOLHealProcessor::ConfigureHonor(HpPerPoint, Cap);
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, Heal, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			UE::Mass::Executor::Run(*Heal, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Spearman;
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = FVector(300.0, 0.0, 0.0);
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> Ids;
		Units->GetAllUnitIds(Ids);
		const FMassEntityHandle Entity = Ids.Num() == 1 ? Units->FindEntity(Ids[0]) : FMassEntityHandle();
		FHOLUnitStateFragment* Ally = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		if (TestNotNull(TEXT("the ally born"), Ally))
		{
			RunStep();
			Ally->CurrentHealth = 20.f;
			TestEqual(TEXT("the gauge of slot 0 starts empty"), ReadFactionGauge(State, 0), 0);
			TestTrue(TEXT("slot 0 puts the Sacred Light at the origin"), Heroes->StartSacredLight(0, FVector(0.0, 0.0, 0.0)));
			const int32 ZoneSteps = FMath::CeilToInt(HOLZoneEffect::SacredLightDurationMs / 1000.f / StepSeconds) + 2;
			for (int32 Steps = 0; Steps < ZoneSteps; ++Steps)
			{
				RunStep();
			}
			const int32 Gained = FMath::FloorToInt(Ally->CurrentHealth - 20.f);
			const int32 Expected = FMath::Min(Gained / HpPerPoint, Cap);
			TestTrue(*FString::Printf(TEXT("the zone healed something (%d hit points)"), Gained), Gained >= HpPerPoint);
			// The remainder under HpPerPoint waits for the next heal, so the gauge is within one point of the whole division.
			const int32 Honor = ReadFactionGauge(State, 0);
			TestTrue(*FString::Printf(TEXT("slot 0 gauge %d = about %d (%d hit points healed, one Honor per %d)"), Honor, Expected, Gained, HpPerPoint),
				Honor >= Expected - 1 && Honor <= Expected && Honor > 0);
			TestEqual(TEXT("slot 1 healed nothing: no Honor"), ReadFactionGauge(State, 1), 0);
		}
		UHOLHealProcessor::ConfigureHonor(0, 0);
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// REVUE-1123pm G3 (part b): the Honor settings and the pending hit points are process-wide statics; the end of a game world must turn the Honor off.
// A world configured with the Honor (heal, kill, hit points waiting) is destroyed; the next one, NOT configured, finds the Honor off: no setting,
// an empty table, no hit point kept, no gain from a death.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectHonorWorldEndTest, "HeroesOfLiberty.Zone.HonorWorldEnd", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectHonorWorldEndTest::RunTest(const FString& Parameters)
{
	const auto OpenWorld = [](AHOLGameState*& OutState) -> UWorld*
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		OutState = World->SpawnActor<AHOLGameState>();
		World->SetGameState(OutState);
		return World;
	};
	const auto CloseWorld = [](UWorld* World)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	AHOLGameState* FirstState = nullptr;
	UWorld* First = OpenWorld(FirstState);
	UHOLHealProcessor::ConfigureHonor(20, 100, 5, 1200);
	UHOLHealProcessor::AddHonorHp(0, 35.);
	TestEqual(TEXT("first world: Honor on (one per 20 hit points)"), UHOLHealProcessor::GetHonorHpPerPoint(), 20);
	TestTrue(TEXT("first world: 35 hit points wait for slot 0"), UHOLHealProcessor::GetHonorPendingHp().IsValidIndex(0) && FMath::IsNearlyEqual(UHOLHealProcessor::GetHonorPendingHp()[0], 35.));
	CloseWorld(First);

	AHOLGameState* SecondState = nullptr;
	UWorld* Second = OpenWorld(SecondState);
	TestEqual(TEXT("second world (not configured): no hit points per Honor"), UHOLHealProcessor::GetHonorHpPerPoint(), 0);
	TestEqual(TEXT("second world: no cap"), UHOLHealProcessor::GetHonorCap(), 0);
	TestEqual(TEXT("second world: no Honor per kill"), UHOLHealProcessor::GetHonorKillGain(), 0);
	TestEqual(TEXT("second world: no kill radius"), UHOLHealProcessor::GetHonorKillRadiusCm(), 0);
	TestTrue(TEXT("second world: no hit point waits (the table is empty)"), UHOLHealProcessor::GetHonorPendingHp().IsEmpty());
	UHOLHealProcessor::AddHonorHp(0, 50.);
	TestTrue(TEXT("second world: a heal gains nothing while the Honor is off"), UHOLHealProcessor::GetHonorPendingHp().IsEmpty());
	UHOLHealProcessor::CreditKillHonor(Second, 0);
	TestEqual(TEXT("second world: a death earns no Honor"), ReadFactionGauge(SecondState, 0), 0);
	CloseWorld(Second);
	return true;
}

// REVUE-1123pm G2 (part a): the gauge is read and written by reflection with local structs; the credit must first check the size of the
// parameter block of the function it found. The check (UHOLHealProcessor::ParmsSizeMatches) is tested on the real getter and setter of the
// game state, with the right size and wrong ones; the credit itself keeps being tested by Zone.HealHonor / Zone.KillHonor.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectHonorGuardTest, "HeroesOfLiberty.Zone.HonorGuard", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectHonorGuardTest::RunTest(const FString& Parameters)
{
	const UFunction* Getter = AHOLGameState::StaticClass()->FindFunctionByName(TEXT("GetFactionResource"));
	const UFunction* Setter = AHOLGameState::StaticClass()->FindFunctionByName(TEXT("SetFactionResource"));
	if (!TestNotNull(TEXT("GetFactionResource reflected"), Getter) || !TestNotNull(TEXT("SetFactionResource reflected"), Setter))
	{
		return false;
	}
	// Slot (int32) + return value (int32), as in CreditHonorPoints; Slot + Value (int32 each).
	TestTrue(TEXT("getter: 8 bytes, 2 parameters is the signature the credit expects"), UHOLHealProcessor::ParmsSizeMatches(Getter, 8, 2));
	TestTrue(TEXT("setter: 8 bytes, 2 parameters is the signature the credit expects"), UHOLHealProcessor::ParmsSizeMatches(Setter, 8, 2));
	TestFalse(TEXT("getter: a struct of 12 bytes is refused"), UHOLHealProcessor::ParmsSizeMatches(Getter, 12, 2));
	TestFalse(TEXT("getter: a struct of 4 bytes is refused"), UHOLHealProcessor::ParmsSizeMatches(Getter, 4, 2));
	TestFalse(TEXT("getter: 3 parameters instead of 2 is refused"), UHOLHealProcessor::ParmsSizeMatches(Getter, 8, 3));
	TestFalse(TEXT("setter: a struct of 16 bytes is refused"), UHOLHealProcessor::ParmsSizeMatches(Setter, 16, 2));
	TestFalse(TEXT("no function: refused"), UHOLHealProcessor::ParmsSizeMatches(nullptr, 8, 2));
	return true;
}

// D689/D859, Honor of the Dawn from a kill (S1-e): a Dawn spearman (slot 0) kills a 1 hp enemy (slot 1) within 12 m of a Dawn building, by the real combat processor:
// the Dawn's gauge gains HonorKillGain per enemy that died (counted from the units really gone, never a constant); the enemy slot earns nothing.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLZoneEffectKillHonorTest, "HeroesOfLiberty.Zone.KillHonor", UE::HeroesOfLiberty::Tests::ZoneEffect::Flags)

bool FHOLZoneEffectKillHonorTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::ZoneEffect;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	World->SetGameState(State);

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("entities"), EntitySubsystem))
	{
		constexpr int32 KillGain = 3;
		constexpr int32 Cap = 100;
		UHOLHealProcessor::ConfigureHonor(20, Cap, KillGain, 1200);
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Combat, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		// Born one by one (as HeroesOfLiberty.HeroXP.KillInMatch), so each unit of a fight is known by its own identifier, and born again until
		// its identifier is a multiple of 4: the combat looks for a target one step of SearchSteps (4) staggered by (UnitId + StepIndex), and the
		// test's steps do not advance the simulation's step index, so a unit whose id is not a multiple of 4 never looks for an enemy
		// (KillHonor 15:26: no blow struck in 10 s, enemy hp 1, Dawn spearman hp 80).
		const auto Place = [Units](const int32 Owner, const FVector& Location)
		{
			for (int32 Attempt = 0; Attempt < 8; ++Attempt)
			{
				TArray<uint32> Before;
				Units->GetAllUnitIds(Before);
				FHOLUnitSpawnRequest Request;
				Request.TypeName = Spearman;
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
		// The unit's hit points (-1 once it is gone), for the message of a failure and for IsGone.
		const auto HealthOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Handle = Units->FindEntity(Id);
			const FHOLUnitStateFragment* Fragment = Handle.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Handle) : nullptr;
			return Fragment ? Fragment->CurrentHealth : -1.f;
		};
		// Dead = no hit point left, as HeroesOfLiberty.HeroXP.KillInMatch (IsDown): the combat marks the death (hp -7, the kill and its Honor counted,
		// KillHonor 15:46: gauge right) and the unit leaves the subsystem later, so its type name is still there at the end of the step.
		const auto IsGone = [&HealthOf](const uint32 Id)
		{
			return HealthOf(Id) <= 0.f;
		};
		const auto SetHealthToOne = [Units, &EntityManager](const uint32 Id)
		{
			FHOLUnitStateFragment* Fragment = EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(Id));
			if (Fragment)
			{
				Fragment->CurrentHealth = 1.f;
			}
			return Fragment != nullptr;
		};
		// Steps until the victim is gone (10 s at most), the loop ends at its death; true if it is gone. The deaths are the units really gone, not a count of the test's own.
		const auto RunUntilGone = [&RunStep, &IsGone, StepSeconds](const uint32 Victim)
		{
			for (int32 Steps = 0; Steps < FMath::CeilToInt(10.f / StepSeconds) && !IsGone(Victim); ++Steps)
			{
				RunStep();
			}
			RunStep();
			return IsGone(Victim);
		};
		// D859: the Honor comes from a kill near a BUILDING of the Dawn: slot 0's centre stands 10 m west of the first fight.
		AHOLBuilding* DawnCentre = World->SpawnActor<AHOLBuilding>(FVector(-1000.0, 0.0, 0.0), FRotator::ZeroRotator);
		if (DawnCentre)
		{
			DawnCentre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		}
		const uint32 DawnNear = Place(0, FVector(0.0, 0.0, 0.0));
		const uint32 EnemyNear = Place(1, FVector(150.0, 0.0, 0.0));
		if (TestNotNull(TEXT("slot 0 centre spawned"), DawnCentre) && TestTrue(TEXT("a Dawn spearman and an enemy spearman born near the centre"), DawnNear != 0 && EnemyNear != 0))
		{
			RunStep();
			TestTrue(TEXT("the near enemy is down to 1 hit point"), SetHealthToOne(EnemyNear));
			TestEqual(TEXT("the gauge of slot 0 starts empty"), ReadFactionGauge(State, 0), 0);
			const bool bNearGone = RunUntilGone(EnemyNear);
			TArray<uint32> AliveIds;
			Units->GetAllUnitIds(AliveIds);
			TestTrue(*FString::Printf(TEXT("the enemy killed near the centre is gone (enemy hp %.0f, Dawn spearman hp %.0f, %d unit(s) alive)"), HealthOf(EnemyNear), HealthOf(DawnNear), AliveIds.Num()), bNearGone);
			TestEqual(TEXT("slot 0 gauge = HonorKillGain for the one kill within 12 m of its building"), ReadFactionGauge(State, 0), KillGain);
			TestEqual(TEXT("slot 1 killed nothing: no Honor"), ReadFactionGauge(State, 1), 0);

			// The contrary case: a kill 50 m from every building of the Dawn gives nothing (the radius of D859 is the rule, not any kill).
			const uint32 DawnFar = Place(0, FVector(5000.0, 0.0, 0.0));
			const uint32 EnemyFar = Place(1, FVector(5150.0, 0.0, 0.0));
			if (TestTrue(TEXT("a second pair born 50 m from the centre"), DawnFar != 0 && EnemyFar != 0))
			{
				RunStep();
				TestTrue(TEXT("the far enemy is down to 1 hit point"), SetHealthToOne(EnemyFar));
				TestTrue(*FString::Printf(TEXT("the enemy killed far from the centre is gone (enemy hp %.0f)"), HealthOf(EnemyFar)), RunUntilGone(EnemyFar));
				TestEqual(TEXT("a kill beyond 12 m of every Dawn building: the gauge stays at one kill's Honor"), ReadFactionGauge(State, 0), KillGain);
			}
		}
		UHOLHealProcessor::ConfigureHonor(0, 0);
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS
