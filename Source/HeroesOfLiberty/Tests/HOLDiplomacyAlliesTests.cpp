// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Map/HOLVisionSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLHealProcessor.h"
#include "Simulation/HOLHealRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// D6 (shared fog, healing between allies) and D8 (the AI stays an Enemy) of DiplomacyInProgress.md, through the real chains.
namespace UE::HeroesOfLiberty::Tests::DiplomacyAllies
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A Diplomacy launch: slots 0 and 1 human, slot 2 human or an AI, the others closed; every lobby team set to 1 (they must not count). */
	void Launch(AHOLGameState* GameState, const bool bAIOnSlot2)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		Config.Mode = EHOLMatchMode::Diplomacy;
		Config.Slots.SetNum(FHOLMatchConfig::MaxSlots);
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			Config.Slots[Slot].State = Slot < 2 ? EHOLSlotState::Human : Slot == 2 ? (bAIOnSlot2 ? EHOLSlotState::AI : EHOLSlotState::Human) : EHOLSlotState::Closed;
			Config.Slots[Slot].Team = 1;
		}
		GameState->InitVictory(Config);
	}

	/** A unit of Type born for Owner at Location, its identifier (0 if none); bSearching: born again until its identifier is a multiple of 4 (one search step of 4). */
	uint32 Place(UHOLUnitSubsystem* Units, const FName Type, const int32 Owner, const FVector& Location, const bool bSearching = false)
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
			After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
			const uint32 Born = After.Num() == 1 ? After[0] : 0;
			if (Born == 0 || !bSearching || Born % 4 == 0)
			{
				return Born;
			}
			Units->RequestDestroyUnits({ Born });
			Units->FlushDestructions();
		}
		return 0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyCampsIgnoreTeamsTest, "HeroesOfLiberty.Diplomacy.CampsIgnoreTeams", UE::HeroesOfLiberty::Tests::DiplomacyAllies::Flags)

// D5/D6: in the Diplomacy mode the lobby's teams do not make camps (HOLVictoryRules::GetCamp: each slot alone), in a standard mode they do.
bool FHOLDiplomacyCampsIgnoreTeamsTest::RunTest(const FString& Parameters)
{
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	Config.Slots.SetNum(FHOLMatchConfig::MaxSlots);
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		Config.Slots[Slot].State = Slot < 3 ? EHOLSlotState::Human : EHOLSlotState::Closed;
		Config.Slots[Slot].Team = Slot < 2 ? 1 : 0;
	}
	Config.Mode = EHOLMatchMode::Standard;
	TestEqual(TEXT("standard: slots 0 and 1 of team 1 share a camp"), HOLVictoryRules::GetCamp(Config, 0), HOLVictoryRules::GetCamp(Config, 1));
	TestNotEqual(TEXT("standard: slot 2 has its own"), HOLVictoryRules::GetCamp(Config, 2), HOLVictoryRules::GetCamp(Config, 0));
	Config.Mode = EHOLMatchMode::Diplomacy;
	TestNotEqual(TEXT("Diplomacy: team 1 counts for nothing, slots 0 and 1 are two camps"), HOLVictoryRules::GetCamp(Config, 0), HOLVictoryRules::GetCamp(Config, 1));
	TestEqual(TEXT("Diplomacy: a closed slot has none"), HOLVictoryRules::GetCamp(Config, 5), static_cast<int32>(INDEX_NONE));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacySharedFogTest, "HeroesOfLiberty.Diplomacy.SharedFog", UE::HeroesOfLiberty::Tests::DiplomacyAllies::Flags)

// D6 / D371, through the real chain (stance and vision orders -> step -> book -> the fog's observers): a Peasant of slot 0 sees around it.
// Slot 1 does not, even allied (the vision is given, not automatic); once slot 0 shares its vision with its ally, slot 1 sees there (and
// the third player, not allied, does not); the other way round slot 0 sees nothing of slot 1's Peasant; the break cuts it at that step
// (the cells stay explored); a new alliance does not give it back by itself.
bool FHOLDiplomacySharedFogTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyAllies;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	if (TestNotNull(TEXT("game state"), GameState) && TestNotNull(TEXT("units"), Units) && TestNotNull(TEXT("sources"), Sources)
		&& TestNotNull(TEXT("vision"), Vision) && TestNotNull(TEXT("simulation"), Simulation) && TestNotNull(TEXT("commands"), Commands))
	{
		Sources->OnWorldBeginPlay(*World);
		Launch(GameState, false);
		Vision->Start(GameState->GetLaunchConfig(), EHOLFogMode::Normal);
		TestEqual(TEXT("one camp per slot (the lobby's team 1 does not make one)"), Vision->GetMap().GetNumCamps(), 3);

		int64 Step = 0;
		const auto Order = [&](const int32 From, const EHOLDiplomacyOrderKind Kind, const int32 To, const int32 Value)
		{
			Commands->IssueDiplomacyOrder(From, Kind, To, Value);
			Simulation->GetOnStepStarted().Broadcast(++Step);
			Vision->Update();
		};
		const auto Say = [&](const int32 From, const int32 To, const int32 Stance) { Order(From, EHOLDiplomacyOrderKind::Stance, To, Stance); };
		const FName Peasant(TEXT("Peasant"));
		const FVector Near(2000., 0., 0.);
		const FVector Far(20000., 0., 0.);

		const uint32 ScoutA = Place(Units, Peasant, 0, FVector::ZeroVector);
		const uint32 ScoutB = Place(Units, Peasant, 1, Far);
		if (TestTrue(TEXT("a Peasant of slot 0 and one of slot 1 born"), ScoutA != 0 && ScoutB != 0))
		{
			Vision->Update();
			TestTrue(TEXT("slot 0 sees around its Peasant"), Vision->IsVisible(0, Near));
			TestFalse(TEXT("slot 1 does not (enemies at the launch)"), Vision->IsVisible(1, Near));

			Say(0, 1, HOLDiplomacy::Ally);
			Say(1, 0, HOLDiplomacy::Ally);
			TestFalse(TEXT("allied but the vision is not given: slot 1 still sees nothing there"), Vision->IsVisible(1, Near));

			Order(0, EHOLDiplomacyOrderKind::Vision, 1, 1);
			TestTrue(TEXT("slot 0 shares its vision: slot 1 sees around the Peasant of slot 0"), Vision->IsVisible(1, Near));
			TestTrue(TEXT("... and slot 0 still sees it"), Vision->IsVisible(0, Near));
			TestFalse(TEXT("the third player, not allied, does not"), Vision->IsVisible(2, Near));
			TestFalse(TEXT("one way only: slot 0 sees nothing of slot 1's Peasant"), Vision->IsVisible(0, Far));
			TestTrue(TEXT("slot 1 sees around its own Peasant"), Vision->IsVisible(1, Far));

			Say(1, 0, HOLDiplomacy::Enemy);
			TestFalse(TEXT("the break cuts the shared vision at that step"), Vision->IsVisible(1, Near));
			TestEqual(TEXT("... what slot 1 saw stays explored"), Vision->GetVision(1, Near), EHOLCellVision::Explored);
			TestTrue(TEXT("... slot 0 keeps seeing around its Peasant"), Vision->IsVisible(0, Near));

			Say(1, 0, HOLDiplomacy::Ally);
			TestFalse(TEXT("allied again: the vision must be given again (D371)"), Vision->IsVisible(1, Near));
			Order(0, EHOLDiplomacyOrderKind::Vision, 1, 1);
			TestTrue(TEXT("given again: seen again"), Vision->IsVisible(1, Near));

			// A moving observer drags the shared sight with it.
			Units->RequestDestroyUnits({ ScoutA });
			Units->FlushDestructions();
			Vision->Update();
			TestFalse(TEXT("the Peasant is gone: slot 1 no longer sees there"), Vision->IsVisible(1, Near));
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyHealRulesTest, "HeroesOfLiberty.Diplomacy.HealRules", UE::HeroesOfLiberty::Tests::DiplomacyAllies::Flags)

// D6: the healer's allies are its camp's, and in the Diplomacy mode the owners it is allied to (HealerAllyOwners); nobody else.
bool FHOLDiplomacyHealRulesTest::RunTest(const FString& Parameters)
{
	const auto Wounded = [](const uint32 Id, const uint8 Camp, const uint8 Owner, const double X)
	{
		HOLHeal::FCandidate Candidate;
		Candidate.UnitId = Id;
		Candidate.Camp = Camp;
		Candidate.Owner = Owner;
		Candidate.Location = FVector2D(X, 0.);
		Candidate.Health = 10.f;
		Candidate.MaxHealth = 100.f;
		return Candidate;
	};
	// The healer #1 of owner 0 (its own camp 0); #2 of owner 1 (camp 1, an ally), #3 of owner 2 (camp 2, not allied), #4 of its own owner.
	const TArray<HOLHeal::FCandidate> Units = { Wounded(1, 0, 0, 0.), Wounded(2, 1, 1, 100.), Wounded(3, 2, 2, 50.), Wounded(4, 0, 0, 400.) };
	TestEqual(TEXT("no allied owner: its own camp only (#4, though farther than #3)"), HOLHeal::ChooseTarget(1, 0, FVector2D::ZeroVector, 600.f, 0, 0, Units), 3);
	TestEqual(TEXT("owner 1 allied: the nearest allied wounded, #2 (not #3 of owner 2)"), HOLHeal::ChooseTarget(1, 0, FVector2D::ZeroVector, 600.f, 0, 0, Units, 1u << 1), 1);
	TestEqual(TEXT("ordered on the ally #2"), HOLHeal::ChooseTarget(1, 0, FVector2D::ZeroVector, 600.f, 2, 0, Units, 1u << 1), 1);
	TestEqual(TEXT("ordered on the stranger #3: refused, the nearest ally instead"), HOLHeal::ChooseTarget(1, 0, FVector2D::ZeroVector, 600.f, 3, 0, Units, 1u << 1), 1);
	TestEqual(TEXT("owner 2 allied too: #3 is the nearest then"), HOLHeal::ChooseTarget(1, 0, FVector2D::ZeroVector, 600.f, 0, 0, Units, (1u << 1) | (1u << 2)), 2);
	TestTrue(TEXT("IsAllyOf: the camp, or an allied owner"), HOLHeal::IsAllyOf(0, 0, Units[3]) && HOLHeal::IsAllyOf(0, 1u << 1, Units[1]) && !HOLHeal::IsAllyOf(0, 1u << 1, Units[2]));
	TestFalse(TEXT("an owner not told (255) is never allied by the mask"), HOLHeal::IsAllyOf(9, 0xFFFFFFFFu, HOLHeal::FCandidate()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyHealInMatchTest, "HeroesOfLiberty.Diplomacy.HealInMatch", UE::HeroesOfLiberty::Tests::DiplomacyAllies::Flags)

// D6, through the real heal processor and the real stance orders: a Monk of slot 0 and a wounded Spearman of slot 1 side by side. At war
// (the launch) the Monk does not heal it; the alliance (both say Ally): it does; the break: it stops.
bool FHOLDiplomacyHealInMatchTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyAllies;
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
		UHOLHealProcessor* Heal = NewObject<UHOLHealProcessor>(Simulation);
		Heal->CallInitialize(Simulation, EntityManager.ToSharedRef());
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		int64 Step = 0;
		const auto RunStep = [Simulation, Units, Heal, &EntityManager, StepSeconds, &Step]
		{
			++Step;
			Simulation->GetOnStepStarted().Broadcast(Step);
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Heal, ProcessingContext);
			Simulation->GetOnStepEnded().Broadcast(Step);
		};
		const auto Say = [&](const int32 From, const int32 To, const int32 Stance)
		{
			Commands->IssueDiplomacyOrder(From, EHOLDiplomacyOrderKind::Stance, To, Stance);
			RunStep();
		};
		const auto StateOf = [Units, &EntityManager](const uint32 Id) -> FHOLUnitStateFragment*
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			return Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		};
		Launch(GameState, false);
		RunStep();
		const uint32 Monk = Place(Units, FName(TEXT("Monk")), 0, FVector::ZeroVector);
		const uint32 Patient = Place(Units, FName(TEXT("Spearman")), 1, FVector(150., 0., 0.));
		FHOLUnitStateFragment* PatientState = StateOf(Patient);
		if (TestTrue(TEXT("a Monk of slot 0 and a Spearman of slot 1 born"), Monk != 0 && PatientState != nullptr))
		{
			PatientState->CurrentHealth = 1.f;
			for (int32 Index = 0; Index < 80; ++Index)
			{
				RunStep();
			}
			TestEqual(TEXT("at war: the Monk does not heal the Spearman"), PatientState->CurrentHealth, 1.f);
			Say(0, 1, HOLDiplomacy::Ally);
			TestEqual(TEXT("one side only says Ally: still not healed"), PatientState->CurrentHealth, 1.f);
			Say(1, 0, HOLDiplomacy::Ally);
			for (int32 Index = 0; Index < 80; ++Index)
			{
				RunStep();
			}
			TestTrue(TEXT("allied: the Monk heals the Spearman of slot 1"), PatientState->CurrentHealth > 1.f);
			Say(1, 0, HOLDiplomacy::Enemy);
			const float AtBreak = PatientState->CurrentHealth;
			for (int32 Index = 0; Index < 80; ++Index)
			{
				RunStep();
			}
			TestTrue(TEXT("the break: the healing stops at that step"), PatientState->CurrentHealth <= AtBreak + 0.001f);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyAIStaysEnemyTest, "HeroesOfLiberty.Diplomacy.AIStaysEnemy", UE::HeroesOfLiberty::Tests::DiplomacyAllies::Flags)

// D8 / D367, the non-regression of the Diplomacy mode for the AI, through the real orders: slots 0 and 1 human, slot 2 an AI. A human
// offers it an alliance and the vision: the AI is still an Enemy that fires at once and never shares a fog nor a victory; the AI cannot
// say a stance (refused); its units still fight a human unit that said Ally to it; a victory by alliance is not possible while it lives.
bool FHOLDiplomacyAIStaysEnemyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DiplomacyAllies;
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
		const auto Order = [&](const int32 From, const EHOLDiplomacyOrderKind Kind, const int32 To, const int32 Value)
		{
			Commands->IssueDiplomacyOrder(From, Kind, To, Value);
			RunStep();
		};

		Launch(GameState, true);
		RunStep();
		Order(0, EHOLDiplomacyOrderKind::Stance, 2, HOLDiplomacy::Ally);
		Order(2, EHOLDiplomacyOrderKind::Stance, 0, HOLDiplomacy::Ally);
		Order(0, EHOLDiplomacyOrderKind::Vision, 2, 1);
		const HOLDiplomacy::FBook Book = GameState->GetDiplomacyBook();
		TestEqual(TEXT("the AI is an Enemy though the human said Ally to it (D367)"), HOLDiplomacy::Relation(Book, 0, 2), HOLDiplomacy::Enemy);
		TestEqual(TEXT("the AI said nothing (refused)"), HOLDiplomacy::StanceOf(Book, 2, 0), HOLDiplomacy::Enemy);
		TestFalse(TEXT("no vision is shared with the AI (nor from it)"), HOLDiplomacy::SharesVision(Book, 0, 2) || HOLDiplomacy::SharesVision(Book, 2, 0));
		const FHOLCampTable Camps = FHOLCampTable::FromWorld(World);
		TestTrue(TEXT("the AI fires at once at the human, and he at it"), Camps.FiresAtOnce(0, 2) && Camps.FiresAtOnce(2, 0));
		TestFalse(TEXT("the AI is no friend of the other human either"), HOLDiplomacy::IsAlly(Book, 1, 2));

		// Its units still fight the human unit that offered it an alliance.
		const FName ManAtArms(TEXT("ManAtArms"));
		const uint32 Human = Place(Units, ManAtArms, 0, FVector::ZeroVector, true);
		const uint32 Machine = Place(Units, ManAtArms, 2, FVector(150., 0., 0.), true);
		const auto HealthOf = [Units, &EntityManager](const uint32 Id)
		{
			const FMassEntityHandle Entity = Id != 0 ? Units->FindEntity(Id) : FMassEntityHandle();
			const FHOLUnitStateFragment* State = Entity.IsSet() ? EntityManager->GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
			return State ? State->CurrentHealth : -1.f;
		};
		if (TestTrue(TEXT("a human Man-at-arms and an AI one born"), Human != 0 && Machine != 0))
		{
			const float HumanStart = HealthOf(Human);
			const float MachineStart = HealthOf(Machine);
			bool bFought = false;
			for (int32 Index = 0; Index < 600 && !bFought; ++Index)
			{
				RunStep();
				bFought = HealthOf(Human) < HumanStart || HealthOf(Machine) < MachineStart;
			}
			TestTrue(TEXT("they fight though the human said Ally to the AI"), bFought);
		}
		TArray<uint32> All;
		Units->GetAllUnitIds(All);
		Units->RequestDestroyUnits(All);
		Units->FlushDestructions();

		// No victory by alliance while the AI lives: slot 1 gives up, slot 0 (who said Ally to the AI) and the AI survive.
		TestTrue(TEXT("slot 1 gives up"), GameState->DefeatSlot(1, EHOLDefeatReason::Surrender));
		RunStep();
		TestFalse(TEXT("the survivors are a human and an AI: the match goes on (D367)"), GameState->IsMatchOver());
		CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDiplomacyPlayersTableTest, "HeroesOfLiberty.Diplomacy.PlayersTable", UE::HeroesOfLiberty::Tests::DiplomacyAllies::Flags)

// B-diplomacy-table: in the Diplomacy mode the players' table marks an ally (green shield) by the book (both said Ally), not by the lobby's
// teams (all set to team 1 here: they must count for nothing); Enemy a red cross, Neutral a grey dot; a standard mode keeps its teams.
bool FHOLDiplomacyPlayersTableTest::RunTest(const FString& Parameters)
{
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	Config.Slots.SetNum(FHOLMatchConfig::MaxSlots);
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		Config.Slots[Slot].State = Slot < 5 ? (Slot == 3 ? EHOLSlotState::AI : EHOLSlotState::Human) : EHOLSlotState::Closed;
		Config.Slots[Slot].Team = 1;
	}
	Config.Mode = EHOLMatchMode::Diplomacy;
	TArray<bool> IsAI;
	IsAI.Init(false, FHOLMatchConfig::MaxSlots);
	IsAI[3] = true;
	HOLDiplomacy::FBook Book = HOLDiplomacy::NewBook(FHOLMatchConfig::MaxSlots, IsAI);
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 0, 2, HOLDiplomacy::Neutral);
	HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Neutral);
	HOLDiplomacy::SetStance(Book, 0, 3, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 0, 4, HOLDiplomacy::Ally);
	const TArray<FHOLForumState> Forums;
	const auto RelationOf = [](const TArray<FHOLPlayersTableRow>& Rows, const int32 Slot)
	{
		for (const FHOLPlayersTableRow& Row : Rows)
		{
			if (Row.Slot == Slot)
			{
				return Row.Relation;
			}
		}
		return EHOLPlayerRelation::None;
	};
	const TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(Config, Forums, 0, 0, FHOLForumTuning(), {}, &Book);
	TestEqual(TEXT("one row per occupied slot"), Rows.Num(), 5);
	TestTrue(TEXT("oneself"), RelationOf(Rows, 0) == EHOLPlayerRelation::Self);
	TestTrue(TEXT("0 and 1 both said Ally: an ally (green shield), though the lobby put them in different or no teams"), RelationOf(Rows, 1) == EHOLPlayerRelation::Ally);
	TestTrue(TEXT("Neutral on both sides: the grey dot (Q-diplomacy-table = B)"), RelationOf(Rows, 2) == EHOLPlayerRelation::Neutral);
	TestTrue(TEXT("an AI is an enemy though the viewer said Ally to it (D367)"), RelationOf(Rows, 3) == EHOLPlayerRelation::Enemy);
	TestTrue(TEXT("Ally said by the viewer only: still an enemy (awaiting the other)"), RelationOf(Rows, 4) == EHOLPlayerRelation::Enemy);
	bool bGroups = false;
	for (const FHOLPlayersTableRow& Row : Rows)
	{
		bGroups |= Row.bNewGroup || Row.Team != 0;
	}
	TestFalse(TEXT("the lobby's teams make no group in the Diplomacy mode"), bGroups);
	// The other side of the same alliance.
	const TArray<FHOLPlayersTableRow> FromOne = HOLForumRules::GetPlayersTable(Config, Forums, 0, 1, FHOLForumTuning(), {}, &Book);
	TestTrue(TEXT("seen from slot 1, slot 0 is an ally"), RelationOf(FromOne, 0) == EHOLPlayerRelation::Ally);
	// A break shows at once: the book changes, the table is rebuilt.
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Enemy);
	TestTrue(TEXT("after the break: an enemy"), RelationOf(HOLForumRules::GetPlayersTable(Config, Forums, 0, 0, FHOLForumTuning(), {}, &Book), 1) == EHOLPlayerRelation::Enemy);
	// A spectator (no slot) sees no relation.
	TestTrue(TEXT("a spectator sees none"), RelationOf(HOLForumRules::GetPlayersTable(Config, Forums, 0, INDEX_NONE, FHOLForumTuning(), {}, &Book), 1) == EHOLPlayerRelation::None);
	// The standard mode: the teams, as before (no book).
	Config.Mode = EHOLMatchMode::Standard;
	TestTrue(TEXT("standard: the same team is an ally"), RelationOf(HOLForumRules::GetPlayersTable(Config, Forums, 0, 0, FHOLForumTuning()), 1) == EHOLPlayerRelation::Ally);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
