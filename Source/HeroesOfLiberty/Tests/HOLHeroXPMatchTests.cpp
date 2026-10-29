// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLConstructionCommand.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLBuildProcessor.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8b.2 loop 2: the XP given in a match (J8b-Q1, Q2, Q5), numbers written from the decisions, not from the code.
namespace UE::HeroesOfLiberty::Tests::HeroXPMatch
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Hero(TEXT("Hero"));
	const FName Peasant(TEXT("Peasant"));
	const FName ManAtArms(TEXT("ManAtArms"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPKillTest, "HeroesOfLiberty.HeroXP.Kill", UE::HeroesOfLiberty::Tests::HeroXPMatch::Flags)

bool FHOLHeroXPKillTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeroXP;
	TOptional<EHOLXPSource> Source;
	FKillFacts Facts;
	Facts.VictimValue = 120;
	Facts.bHeroAlive = true;
	Facts.bKilledByHero = true;
	TestEqual(TEXT("A Man-at-arms killed by the hero: 120"), GetKillXP(Facts, Source), 120);
	TestTrue(TEXT("... through combat"), Source == EHOLXPSource::Combat);

	Facts.bKilledByHero = false;
	Facts.DistanceToHero = 1200.f;
	TestEqual(TEXT("By his troops at 12 m: 120 x 0.75 = 90"), GetKillXP(Facts, Source), 90);
	TestTrue(TEXT("... through command"), Source == EHOLXPSource::Command);
	Facts.VictimValue = 50;
	TestEqual(TEXT("A Peasant by his troops: 37 (37.5 rounded down)"), GetKillXP(Facts, Source), 37);

	Facts.DistanceToHero = 1201.f;
	TestEqual(TEXT("Farther than 12 m: nothing"), GetKillXP(Facts, Source), 0);
	TestFalse(TEXT("... no source"), Source.IsSet());
	Facts.bKilledByHero = true;
	Facts.bHeroAlive = false;
	TestEqual(TEXT("His hero dead (an arrow landing after his death): nothing (D06)"), GetKillXP(Facts, Source), 0);
	TestFalse(TEXT("... no source"), Source.IsSet());

	// A hero killed (J8b-Q5): 300 x the levels' ratio, whatever his cost.
	Facts.bHeroAlive = true;
	Facts.bVictimHero = true;
	Facts.VictimValue = 0;
	Facts.VictimLevel = 3;
	Facts.HeroLevel = 1;
	TestEqual(TEXT("A level 3 hero killed by a level 1 hero: 900"), GetKillXP(Facts, Source), 900);
	Facts.VictimLevel = 2;
	Facts.HeroLevel = 4;
	TestEqual(TEXT("A level 2 hero killed by a level 4 hero: 150"), GetKillXP(Facts, Source), 150);
	Facts.bKilledByHero = false;
	Facts.DistanceToHero = 500.f;
	TestEqual(TEXT("... by his troops near him: 150 x 0.75 = 112"), GetKillXP(Facts, Source), 112);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPKillInMatchTest, "HeroesOfLiberty.HeroXP.KillInMatch", UE::HeroesOfLiberty::Tests::HeroXPMatch::Flags)

// Through the combat processor, as in a match (no movement: nobody walks): slot 0's hero, his troops and slot 1's victims.
// A Peasant killed by a Man-at-arms 7.5 m from the hero (command 37), another 50 m away (nothing), a Peasant killed by
// the hero (combat 50), slot 1's hero at 1 HP killed by him (300): 387, level 2 at the step's end.
bool FHOLHeroXPKillInMatchTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroXPMatch;
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
		// As HeroesOfLiberty.Hero.ResistanceInCombat: born again until its identifier is a multiple of 4 (one search step of 4).
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
		const auto IsDown = [Units](const uint32 Id)
		{
			FHOLUnitView View;
			return !Units->GetUnitView(Id, View) || View.Health <= 0;
		};
		// Steps until Victim is down (10 s at most); true if it is.
		const auto RunUntilDown = [&RunStep, &IsDown, StepSeconds](const uint32 Victim)
		{
			for (int32 Steps = 0; Steps < FMath::CeilToInt(10.f / StepSeconds) && !IsDown(Victim); ++Steps)
			{
				RunStep();
			}
			RunStep();
			return IsDown(Victim);
		};
		const auto GetXP = [Heroes] { return Heroes->GetSheets(0).Private.XP; };

		const uint32 HeroId = Place(Hero, 0, FVector(0.0, 0.0, 0.0));
		const uint32 Troop = Place(ManAtArms, 0, FVector(600.0, 0.0, 0.0));
		const uint32 NearVictim = Place(Peasant, 1, FVector(750.0, 0.0, 0.0));
		if (TestTrue(TEXT("the hero, his Man-at-arms and a Peasant of slot 1 born"), HeroId != 0 && Troop != 0 && NearVictim != 0))
		{
			TestTrue(TEXT("the Peasant 7.5 m from the hero killed by the Man-at-arms"), RunUntilDown(NearVictim));
			TestEqual(TEXT("command: 50 x 0.75 = 37"), GetXP(), 37);

			const uint32 FarTroop = Place(ManAtArms, 0, FVector(5000.0, 0.0, 0.0));
			const uint32 FarVictim = Place(Peasant, 1, FVector(5150.0, 0.0, 0.0));
			TestTrue(TEXT("a Peasant 50 m from the hero killed by another Man-at-arms"), FarTroop != 0 && FarVictim != 0 && RunUntilDown(FarVictim));
			TestEqual(TEXT("farther than 12 m: nothing more"), GetXP(), 37);

			const uint32 HeroVictim = Place(Peasant, 1, FVector(-150.0, 0.0, 0.0));
			TestTrue(TEXT("a Peasant killed by the hero"), HeroVictim != 0 && RunUntilDown(HeroVictim));
			TestEqual(TEXT("combat: 37 + 50 = 87"), GetXP(), 87);

			const uint32 EnemyHero = Place(Hero, 1, FVector(0.0, 150.0, 0.0));
			FHOLUnitStateFragment* EnemyState = EnemyHero != 0
				? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(EnemyHero)) : nullptr;
			if (TestNotNull(TEXT("slot 1's hero born"), EnemyState))
			{
				EnemyState->CurrentHealth = 1.f;
				TestTrue(TEXT("slot 1's hero at 1 HP killed by him"), RunUntilDown(EnemyHero));
				TestEqual(TEXT("a level 1 hero by a level 1 hero: 87 + 300 = 387"), GetXP(), 387);
				TestEqual(TEXT("slot 1 gained nothing"), Heroes->GetSheets(1).Private.XP, 0);
				const FHOLCombatFragment* HeroCombat = EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Units->FindEntity(HeroId));
				TestTrue(TEXT("his body at level 2 at the step's end"), HeroCombat && HeroCombat->Level == 2);
			}
		}
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPBuildingTest, "HeroesOfLiberty.HeroXP.Building", UE::HeroesOfLiberty::Tests::HeroXPMatch::Flags)

// A House of slot 0 finished by a Peasant (hol.Build.Instant), as HeroesOfLiberty.Construction.FarmBuilders runs its
// steps (orders, gathering, builders): development 50 x 0.5 = 25, with no hero alive (D06: development goes on).
bool FHOLHeroXPBuildingTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroXPMatch;
	IConsoleVariable* Instant = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Build.Instant"));
	if (!TestNotNull(TEXT("hol.Build.Instant"), Instant))
	{
		return false;
	}
	const int32 InstantBefore = Instant->GetInt();
	Instant->Set(1, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	if (TestTrue(TEXT("test world ready"), State && Units && Sources && Commands && Heroes && Simulation && EntitySubsystem))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
		}
		State->InitVictory(Config);
		Sources->OnWorldBeginPlay(*World);
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLUnitOrderProcessor* Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
		Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLGatherProcessor* Gather = NewObject<UHOLGatherProcessor>(Simulation);
		Gather->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLBuildProcessor* Builders = NewObject<UHOLBuildProcessor>(Simulation);
		Builders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();

		AHOLBuilding* Site = World->SpawnActor<AHOLBuilding>(FVector::ZeroVector, FRotator::ZeroRotator);
		if (Site)
		{
			Site->Setup(HOLBuildingTypeNames::House, 0, 0, EHOLConstructionState::Foundation);
		}
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Peasant;
		Request.Count = 1;
		Request.OwnerPlayer = 0;
		Request.Center = Site ? FVector(Site->GetBodyBounds().Min.X - 70.0, Site->GetBodyBounds().GetCenter().Y, 0.0) : FVector::ZeroVector;
		Units->RequestUnits(Request);
		Units->ServeBirthRequests();
		TArray<uint32> Ids;
		Units->GetAllUnitIds(Ids);
		if (TestNotNull(TEXT("a House site"), Site) && TestEqual(TEXT("one Peasant born"), Ids.Num(), 1))
		{
			FHOLBuildOrder Order;
			Order.UnitIds = Ids;
			Order.Building = Site;
			Order.PlayerIndex = 0;
			Commands->IssueBuildOrder(Order);
			for (int32 Steps = 0; Steps < 20 && !Site->IsFinished(); ++Steps)
			{
				for (UMassProcessor* Processor : TArray<UMassProcessor*>{ Orders, Gather, Builders })
				{
					FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
					UE::Mass::Executor::Run(*Processor, ProcessingContext);
				}
			}
			TestTrue(TEXT("the House finished"), Site->IsFinished());
			TestEqual(TEXT("development: 50 x 0.5 = 25, no hero alive"), Heroes->GetSheets(0).Private.XP, 25);
			TestEqual(TEXT("slot 1 gained nothing"), Heroes->GetSheets(1).Private.XP, 0);
		}
	}
	Instant->Set(InstantBefore, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroXPLevelAlertsTest, "HeroesOfLiberty.HeroXP.LevelAlerts", UE::HeroesOfLiberty::Tests::HeroXPMatch::Flags)

// J8b.4 (J8b-Q4): the alerts of a level up, in French (the source), and when a machine sees a hero go up.
bool FHOLHeroXPLevelAlertsTest::RunTest(const FString& Parameters)
{
	const auto Lines = [](const int32 OldLevel, const int32 NewLevel)
	{
		TArray<FString> Out;
		for (const FHOLForumMessage& Message : HOLForumMessages::GetHeroLevelMessages(OldLevel, NewLevel))
		{
			Out.Add(Message.Shown);
		}
		return FString::Join(Out, TEXT(" | "));
	};
	TestEqual(TEXT("1 -> 2: his level"), Lines(1, 2), FString(TEXT("Hero message: Paladin-Commandant : niveau 2")));
	TestEqual(TEXT("2 -> 3: his level, then tier 1"), Lines(2, 3),
		FString(TEXT("Hero message: Paladin-Commandant : niveau 3 | Hero message: Palier 1 atteint")));
	TestEqual(TEXT("2 -> 6 at once: level 6, both tiers"), Lines(2, 6),
		FString(TEXT("Hero message: Paladin-Commandant : niveau 6 | Hero message: Palier 1 atteint | Hero message: Palier 2 atteint")));
	TestEqual(TEXT("3 -> 4: no tier"), Lines(3, 4), FString(TEXT("Hero message: Paladin-Commandant : niveau 4")));
	TestEqual(TEXT("5 -> 6: tier 2"), Lines(5, 6), FString(TEXT("Hero message: Paladin-Commandant : niveau 6 | Hero message: Palier 2 atteint")));
	TestEqual(TEXT("Not up: none"), Lines(3, 3), FString());
	TestEqual(TEXT("Down: none"), Lines(4, 2), FString());

	FHOLHeroLevelWatch Watch;
	TestEqual(TEXT("first seen at level 1: no level up"), Watch.Update(0, 1), 0);
	TestEqual(TEXT("1 -> 2: up from 1"), Watch.Update(0, 2), 1);
	TestEqual(TEXT("2 again: nothing"), Watch.Update(0, 2), 0);
	TestEqual(TEXT("another slot first seen at 3: nothing"), Watch.Update(5, 3), 0);
	TestEqual(TEXT("down to 1 (a cheat): nothing"), Watch.Update(0, 1), 0);
	TestEqual(TEXT("1 -> 3: up from 1"), Watch.Update(0, 3), 1);
	TestEqual(TEXT("no hero (level 0): nothing"), Watch.Update(0, 0), 0);
	TestEqual(TEXT("born again seen at 2: first seen, nothing"), Watch.Update(0, 2), 0);
	TestEqual(TEXT("a slot out of range: nothing"), Watch.Update(-1, 4), 0);
	return true;
}

#endif
