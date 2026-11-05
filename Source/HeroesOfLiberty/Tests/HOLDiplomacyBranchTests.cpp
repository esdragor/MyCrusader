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
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::DiplomacyBranch
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A Diplomacy (or Standard) launch: slots 0, 1, 2 human, the others closed; slot 0 and 1 of the given teams. */
	void Launch(AHOLGameState* GameState, const EHOLMatchMode Mode, const int32 TeamOfSlot0 = 0, const int32 TeamOfSlot1 = 0)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		Config.Mode = Mode;
		Config.Slots.SetNum(FHOLMatchConfig::MaxSlots);
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			Config.Slots[Slot].State = Slot < 3 ? EHOLSlotState::Human : EHOLSlotState::Closed;
			Config.Slots[Slot].Team = Slot == 0 ? TeamOfSlot0 : Slot == 1 ? TeamOfSlot1 : 0;
		}
		GameState->InitVictory(Config);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyCampsTest, "HeroesOfLiberty.Diplomacy.CampTable", UE::HeroesOfLiberty::Tests::DiplomacyBranch::Flags)

// D6 (DiplomacyInProgress.md): what the simulation reads (FHOLCampTable::FromWorld), built from the stances the players ordered through the
// real chain (command -> step -> game state -> book), not typed in:
// - the Diplomacy mode: Enemy fires at once and is an adversary; both Ally: neither; both Neutral: an adversary that does not fire (D401);
//   one side Ally only: Enemy (D375); the neutral camps' owner is an enemy of everybody (D401), oneself a friend;
// - the standard modes: the teams as before (same team friends, others enemies, no Neutral).
bool FHOLDiplomacyCampsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyBranch;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands))
	{
		int64 Step = 0;
		const auto Say = [&](const int32 From, const int32 To, const int32 Stance)
		{
			Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Stance, To, Stance);
			Simulation->GetOnStepStarted().Broadcast(++Step);
		};
		const auto Camps = [World] { return FHOLCampTable::FromWorld(World); };

		Launch(GameState, EHOLMatchMode::Diplomacy);
		TestTrue(TEXT("the Diplomacy mode reads the book"), Camps().IsDiplomacy());
		TestTrue(TEXT("at the launch (Enemy toward all): an adversary"), Camps().AreHostile(0, 1));
		TestTrue(TEXT("... that fires at once"), Camps().FiresAtOnce(0, 1) && Camps().FiresAtOnce(1, 0));
		TestFalse(TEXT("oneself is no adversary"), Camps().AreHostile(0, 0));

		Say(0, 1, HOLDiplomacy::Ally);
		TestTrue(TEXT("Ally said by one side only: still at war (D369, D375)"), Camps().FiresAtOnce(0, 1) && Camps().FiresAtOnce(1, 0));
		Say(1, 0, HOLDiplomacy::Ally);
		TestFalse(TEXT("both Ally: not an adversary"), Camps().AreHostile(0, 1) || Camps().AreHostile(1, 0));
		TestFalse(TEXT("... nor firing"), Camps().FiresAtOnce(0, 1) || Camps().FiresAtOnce(1, 0));
		TestTrue(TEXT("the alliance is not transitive: 0 and 2 still at war"), Camps().FiresAtOnce(0, 2));
		Say(1, 2, HOLDiplomacy::Ally);
		Say(2, 1, HOLDiplomacy::Ally);
		TestFalse(TEXT("1 allied to 2"), Camps().AreHostile(1, 2));
		TestTrue(TEXT("... 0-1 and 1-2 allied, 0-2 still enemies (no chain, D375)"), Camps().FiresAtOnce(0, 2) && !Camps().AreHostile(0, 1));

		Say(1, 0, HOLDiplomacy::Neutral);
		TestTrue(TEXT("Neutral on one side, Ally on the other: Neutral (the most hostile)"), Camps().AreHostile(0, 1));
		TestFalse(TEXT("... no automatic fire"), Camps().FiresAtOnce(0, 1) || Camps().FiresAtOnce(1, 0));
			// E-diplomacy (the user, 2026-11-05): the attack-move click (Q + click) on a unit or a building of a Neutral aims at it, as at an enemy: an adversary that
			// does not fire on its own (the book unchanged, the order never breaks the stance). Not at an Enemy (the units already take it on), not at oneself.
			TestTrue(TEXT("a Neutral: the attack-move click gives a direct attack order"), Camps().IsPassiveAdversary(0, 1));
			TestFalse(TEXT("oneself: no attack"), Camps().IsPassiveAdversary(0, 0));
			TestFalse(TEXT("an Enemy fires on its own: nothing to aim at"), Camps().IsPassiveAdversary(0, 2));
		Say(1, 0, HOLDiplomacy::Enemy);
		TestTrue(TEXT("the break is immediate: fires at once"), Camps().FiresAtOnce(0, 1));

		TestTrue(TEXT("the neutral camps are the enemy of every slot (D401)"), Camps().FiresAtOnce(FHOLMatchConfig::NeutralOwner, 0) && Camps().FiresAtOnce(1, FHOLMatchConfig::NeutralOwner));
		TestFalse(TEXT("... but a camp's own units are friends"), Camps().AreHostile(FHOLMatchConfig::NeutralOwner, FHOLMatchConfig::NeutralOwner));

		// The standard modes keep the teams: 0 and 1 one team, 2 alone.
		Launch(GameState, EHOLMatchMode::Standard, 1, 1);
		TestFalse(TEXT("the standard mode: no book"), Camps().IsDiplomacy());
		TestFalse(TEXT("the same team: friends"), Camps().AreHostile(0, 1));
		TestTrue(TEXT("another slot: enemies, firing at once"), Camps().AreHostile(0, 2) && Camps().FiresAtOnce(0, 2));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyVictoryTest, "HeroesOfLiberty.Diplomacy.AlliedVictoryInMatch", UE::HeroesOfLiberty::Tests::DiplomacyBranch::Flags)

// D6 (D373): the end of a Diplomacy match through the real chain (surrender of a slot, stance orders): three humans; one beaten and the
// two survivors enemies: it goes on; they become allies (both say Ally): the match ends at that step, both winners, together. With nobody
// beaten an alliance of all the players does not end it. A free-for-all with a last survivor still ends as before.
bool FHOLDiplomacyVictoryTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyBranch;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands))
	{
		int64 Step = 0;
		const auto Say = [&](const int32 From, const int32 To, const int32 Stance)
		{
			Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Stance, To, Stance);
			Simulation->GetOnStepStarted().Broadcast(++Step);
		};
		const auto Ally = [&](const int32 A, const int32 B)
		{
			Say(A, B, HOLDiplomacy::Ally);
			Say(B, A, HOLDiplomacy::Ally);
		};

		// Nobody beaten: all three allied, it goes on.
		Launch(GameState, EHOLMatchMode::Diplomacy);
		Ally(0, 1);
		Ally(1, 2);
		Ally(0, 2);
		TestFalse(TEXT("all allied, nobody beaten: no victory (D373)"), GameState->IsMatchOver());

		// One beaten, the survivors enemies: it goes on, then they ally and win together.
		Launch(GameState, EHOLMatchMode::Diplomacy);
		TestTrue(TEXT("slot 2 gives up"), GameState->DefeatSlot(2, EHOLDefeatReason::Surrender));
		TestFalse(TEXT("two survivors at war: the match goes on"), GameState->IsMatchOver());
		Say(0, 1, HOLDiplomacy::Ally);
		TestFalse(TEXT("one side only says Ally: still on"), GameState->IsMatchOver());
		Say(1, 0, HOLDiplomacy::Ally);
		TestTrue(TEXT("both say Ally: the match ends at that step"), GameState->IsMatchOver());
		TestEqual(TEXT("... slots 0 and 1 win together"), GameState->GetWinnerMask(), static_cast<uint8>(0b011));

		// One beaten, the last survivor wins alone (as before); an alliance broken after the end changes nothing.
		Launch(GameState, EHOLMatchMode::Diplomacy);
		GameState->DefeatSlot(2, EHOLDefeatReason::Surrender);
		GameState->DefeatSlot(1, EHOLDefeatReason::Surrender);
		TestTrue(TEXT("the last survivor: over"), GameState->IsMatchOver());
		TestEqual(TEXT("... slot 0 wins"), GameState->GetWinnerMask(), static_cast<uint8>(0b001));

		// Standard mode: the teams decide as before (the two of team 1 against slot 2).
		Launch(GameState, EHOLMatchMode::Standard, 1, 1);
		GameState->DefeatSlot(2, EHOLDefeatReason::Surrender);
		TestTrue(TEXT("standard: the team stands, the other fell: over"), GameState->IsMatchOver());
		TestEqual(TEXT("... team 1 wins"), GameState->GetWinnerMask(), static_cast<uint8>(0b011));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyCombatTest, "HeroesOfLiberty.Diplomacy.CombatStances", UE::HeroesOfLiberty::Tests::DiplomacyBranch::Flags)

// D6 (D369, D370, D401), through the real combat processor, the real orders and book, a unit's health read before and after:
// two Men-at-arms of slots 0 and 1 side by side. Enemy (the launch): they fight. Mutual Neutral: nobody is hurt in 5 s. Mutual Ally: nobody
// is hurt. An alliance made in the middle of a fight stops the blows. The break (Enemy) brings them back at once. A third player's
// Spearman (Enemy to both) is still fought by an allied pair.
bool FHOLDiplomacyCombatTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyBranch;
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
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("commands"), Commands))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 FiveSeconds = FMath::CeilToInt(5.f / StepSeconds);
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
		const auto Say = [&](const int32 From, const int32 To, const int32 Stance)
		{
			Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Stance, To, Stance);
			RunStep();
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
		// Runs Steps steps; whether either unit lost health (a unit that died counts as hurt).
		const auto HurtWithin = [&](const uint32 A, const uint32 B, const int32 Steps)
		{
			const float HealthA = HealthOf(A);
			const float HealthB = HealthOf(B);
			for (int32 Index = 0; Index < Steps; ++Index)
			{
				RunStep();
				if (HealthOf(A) < HealthA || HealthOf(B) < HealthB)
				{
					return true;
				}
			}
			return false;
		};
		const auto Clear = [Units]
		{
			TArray<uint32> All;
			Units->GetAllUnitIds(All);
			Units->RequestDestroyUnits(All);
			Units->FlushDestructions();
		};
		const FName ManAtArms(TEXT("ManAtArms"));
		const FName Spearman(TEXT("Spearman"));

		Launch(GameState, EHOLMatchMode::Diplomacy);
		RunStep();
		const uint32 A = Place(ManAtArms, 0, FVector(0.0, 0.0, 0.0));
		const uint32 B = Place(ManAtArms, 1, FVector(150.0, 0.0, 0.0));
		if (TestTrue(TEXT("a Man-at-arms of slot 0 and one of slot 1 born"), A != 0 && B != 0))
		{
			TestTrue(TEXT("Enemy toward each other (the launch): they fight"), HurtWithin(A, B, 2 * FiveSeconds));
			// The alliance made in the middle of the fight: blows stop (D370 the stances act at the step; a unit's target is let go).
			Say(0, 1, HOLDiplomacy::Ally);
			Say(1, 0, HOLDiplomacy::Ally);
			for (int32 Index = 0; Index < 4; ++Index)
			{
				RunStep(); // the targets chosen before are dropped within a few steps
			}
			TestFalse(TEXT("an alliance made in the middle of the fight: no more blows in 5 s"), HurtWithin(A, B, FiveSeconds));
			Say(1, 0, HOLDiplomacy::Enemy);
			TestTrue(TEXT("the break (one side Enemy): the fight starts again"), HurtWithin(A, B, 2 * FiveSeconds));
		}
		Clear();

		// Mutual Neutral from fresh units: no automatic fire at all.
		Launch(GameState, EHOLMatchMode::Diplomacy);
		RunStep();
		// Said BEFORE the units are born: a fight already begun between two Enemies is another case (each one's target attacks it: the defender's right).
		Say(0, 1, HOLDiplomacy::Neutral);
		Say(1, 0, HOLDiplomacy::Neutral);
		const uint32 C = Place(ManAtArms, 0, FVector(0.0, 0.0, 0.0));
		const uint32 D = Place(ManAtArms, 1, FVector(150.0, 0.0, 0.0));
		if (TestTrue(TEXT("two fresh Men-at-arms born"), C != 0 && D != 0))
		{
			TestFalse(TEXT("both Neutral: nobody is hurt in 10 s (no automatic fire, D369)"), HurtWithin(C, D, 2 * FiveSeconds));
			// Neutral is no friendship: a third player at war with both is fought by both.
			const uint32 Stranger = Place(Spearman, 2, FVector(-150.0, 0.0, 0.0));
			if (TestTrue(TEXT("a Spearman of slot 2 born"), Stranger != 0))
			{
				const float StrangerStart = HealthOf(Stranger);
				bool bStrangerHurt = false;
				for (int32 Index = 0; Index < 2 * FiveSeconds; ++Index)
				{
					RunStep();
					bStrangerHurt |= HealthOf(Stranger) < StrangerStart;
				}
				TestTrue(TEXT("a player at war with the Neutrals is still fought"), bStrangerHurt);
			}
		}
		Clear();

		// A fight already engaged, then Neutral on both sides (D661): a unit still struck defends itself, so the blows go on.
		Launch(GameState, EHOLMatchMode::Diplomacy);
		RunStep();
		const uint32 E = Place(ManAtArms, 0, FVector(0.0, 0.0, 0.0));
		const uint32 F = Place(ManAtArms, 1, FVector(150.0, 0.0, 0.0));
		if (TestTrue(TEXT("two Men-at-arms born for the engaged fight"), E != 0 && F != 0))
		{
			TestTrue(TEXT("Enemy first: they fight"), HurtWithin(E, F, 2 * FiveSeconds));
			Say(0, 1, HOLDiplomacy::Neutral);
			Say(1, 0, HOLDiplomacy::Neutral);
			TestTrue(TEXT("Neutral made in the middle of the fight: the defence goes on (D661)"), HurtWithin(E, F, 2 * FiveSeconds));
		}
		Clear();
		CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyAttackNeutralTest, "HeroesOfLiberty.Diplomacy.AttackOnNeutral", UE::HeroesOfLiberty::Tests::DiplomacyBranch::Flags)

// E-diplomacy BUG (the user, 2026-11-05): the attack order given at a NEUTRAL player's unit (Q + click gives it, AHOLPlayerController::AttackMoveAt, through
// HOLPlayerOrders::Attack as the server's RPC does) must HURT it: the effect is read, not a predicate. Real order processor and combat processor, the
// stances said through the real book. Two Men-at-arms of slots 0 and 1, mutual Neutral: nobody is hurt without an order (D369); the order of slot 0 on the
// unit of slot 1: its health falls; the stance is still Neutral (the order never breaks it).
bool FHOLDiplomacyAttackNeutralTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyBranch;
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
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("commands"), Commands))
	{
		TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
		UHOLUnitOrderProcessor* Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
		Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
		Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int32 FiveSeconds = FMath::CeilToInt(5.f / StepSeconds);
		int64 Step = 0;
		// Order processor then combat processor, as the simulation chains them.
		const auto RunStep = [Simulation, Units, Orders, Combat, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Orders, ProcessingContext);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Say = [&](const int32 From, const int32 To, const int32 Stance)
		{
			Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Stance, To, Stance);
			RunStep();
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

		Launch(GameState, EHOLMatchMode::Diplomacy);
		RunStep();
		Say(0, 1, HOLDiplomacy::Neutral);
		Say(1, 0, HOLDiplomacy::Neutral);
		const uint32 Attacker = Place(FName(TEXT("ManAtArms")), 0, FVector(0.0, 0.0, 0.0));
		const uint32 Target = Place(FName(TEXT("ManAtArms")), 1, FVector(150.0, 0.0, 0.0));
		if (TestTrue(TEXT("a Man-at-arms of slot 0 and one of slot 1 born"), Attacker != 0 && Target != 0))
		{
			const float TargetStart = HealthOf(Target);
			const float AttackerStart = HealthOf(Attacker);
			bool bHurtWithoutOrder = false;
			for (int32 Index = 0; Index < FiveSeconds; ++Index)
			{
				RunStep();
				bHurtWithoutOrder |= HealthOf(Target) < TargetStart || HealthOf(Attacker) < AttackerStart;
			}
			TestFalse(TEXT("both Neutral and no order: the Neutral is not struck (D369)"), bHurtWithoutOrder);

			// The order, through the very function the server's RPC of the click calls.
			const uint32 Ordered[] = { Attacker };
			TestTrue(TEXT("the order of slot 0 is accepted into the queue"), HOLPlayerOrders::Attack(*World, 0, Ordered, Target, false));
			bool bTargetHurt = false;
			for (int32 Index = 0; Index < 2 * FiveSeconds && !bTargetHurt; ++Index)
			{
				RunStep();
				bTargetHurt = HealthOf(Target) < TargetStart;
			}
			TestTrue(TEXT("the attack order on a Neutral's unit: its health falls (the effect, not a predicate)"), bTargetHurt);
			const FHOLCampTable Camps = FHOLCampTable::FromWorld(World);
			TestEqual(TEXT("the stance is unchanged: still Neutral"), Camps.Relation(0, 1), static_cast<int32>(HOLDiplomacy::Neutral));
		}
		CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyAttackMoveNeutralTest, "HeroesOfLiberty.Diplomacy.AttackMoveOnNeutral", UE::HeroesOfLiberty::Tests::DiplomacyBranch::Flags)

// D733 (the user, 2026-11-05): the attack-move on the GROUND (Q then a click at the ground, behind Neutral men) takes on the Neutrals it meets: a player's
// units that said Neutral (both ways) and a map camp's members (the Neutral owner, enemy of everybody, D401). The EFFECT is read (the health of the unit
// met falls), by the real order (HOLPlayerOrders::AttackMove, the function of the server's RPC) and the real order and combat processors. Controls: with no
// order nobody strikes a Neutral (D369), and the stance stays Neutral (the order never breaks it). One world per trial.
bool FHOLDiplomacyAttackMoveNeutralTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyBranch;
	IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
	if (!TestNotNull(TEXT("hol.Combat.Enabled"), CombatEnabled))
	{
		return false;
	}
	const int32 FormerEnabled = CombatEnabled->GetInt();
	CombatEnabled->Set(1, ECVF_SetByCode);

	struct FTrial
	{
		const TCHAR* Name;
		EHOLMatchMode Mode;
		int32 TargetOwner;
		bool bBothNeutral;
		bool bGroundOrder; // the attack-move order; false: the control, no order
		bool bExpectHurt;
	};
	const FTrial Trials[] = {
		{ TEXT("a Neutral player's man, attack-move on the ground behind him"), EHOLMatchMode::Diplomacy, 1, true, true, true },
		{ TEXT("a Neutral player's man, no order (D369)"), EHOLMatchMode::Diplomacy, 1, true, false, false },
		{ TEXT("a map camp's member (Neutral owner), attack-move on the ground behind him, Diplomacy"), EHOLMatchMode::Diplomacy, FHOLMatchConfig::NeutralOwner, false, true, true },
		{ TEXT("a map camp's member (Neutral owner), attack-move on the ground behind him, Standard"), EHOLMatchMode::Standard, FHOLMatchConfig::NeutralOwner, false, true, true },
	};
	for (const FTrial& Trial : Trials)
	{
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
		if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("simulation"), Simulation)
			&& TestNotNull(TEXT("entities"), EntitySubsystem) && TestNotNull(TEXT("commands"), Commands))
		{
			TSharedPtr<FMassEntityManager> EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			UHOLUnitOrderProcessor* Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
			UHOLCombatProcessor* Combat = NewObject<UHOLCombatProcessor>(Simulation);
			Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			const int32 TenSeconds = FMath::CeilToInt(10.f / StepSeconds);
			int64 Step = 0;
			const auto RunStep = [Simulation, Units, Orders, Combat, &EntityManager, StepSeconds, &Step]
			{
				++Step;
				Simulation->GetOnStepStarted().Broadcast(Step);
				Units->ServeBirthRequests();
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
				UE::Mass::Executor::Run(*Combat, ProcessingContext);
				Units->FlushDestructions();
				Simulation->GetOnStepEnded().Broadcast(Step);
			};
			const auto Say = [&](const int32 From, const int32 To, const int32 Stance)
			{
				Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Stance, To, Stance);
				RunStep();
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

			Launch(GameState, Trial.Mode);
			RunStep();
			if (Trial.bBothNeutral)
			{
				Say(0, 1, HOLDiplomacy::Neutral);
				Say(1, 0, HOLDiplomacy::Neutral);
			}
			const uint32 Attacker = Place(FName(TEXT("ManAtArms")), 0, FVector(0.0, 0.0, 0.0));
			const uint32 Met = Place(FName(TEXT("ManAtArms")), Trial.TargetOwner, FVector(800.0, 0.0, 0.0));
			if (TestTrue(*FString::Printf(TEXT("[%s] the two men born"), Trial.Name), Attacker != 0 && Met != 0))
			{
				const float MetStart = HealthOf(Met);
				if (Trial.bGroundOrder)
				{
					// The order of the click on the GROUND past the man met (the goal is further than him, on his side of the field).
					const uint32 Ordered[] = { Attacker };
					TestTrue(*FString::Printf(TEXT("[%s] the attack-move is accepted"), Trial.Name), HOLPlayerOrders::AttackMove(*World, 0, Ordered, FVector(3000.0, 0.0, 0.0), false));
				}
				bool bMetHurt = false;
				for (int32 Index = 0; Index < TenSeconds && !bMetHurt; ++Index)
				{
					RunStep();
					bMetHurt = HealthOf(Met) < MetStart;
				}
				TestEqual(*FString::Printf(TEXT("[%s] the health of the man met falls (the effect)"), Trial.Name), bMetHurt, Trial.bExpectHurt);
				if (Trial.bBothNeutral)
				{
					TestEqual(*FString::Printf(TEXT("[%s] the stance is unchanged: still Neutral"), Trial.Name), FHOLCampTable::FromWorld(World).Relation(0, 1), static_cast<int32>(HOLDiplomacy::Neutral));
				}
			}
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
