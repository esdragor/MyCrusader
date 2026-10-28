// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Dev/HOLHeroStepProbe.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLHeroLibrary.h"
#include "Simulation/HOLHeroReturnSubsystem.h"
#include "Simulation/HOLHeroSheet.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8a.4 loop 2 checks: the C++ bridges of the hero's return (UHOLHeroLibrary, UHOLHeroSubsystem::OnHeroStep) on the
// server's real chain: a test world with the real game state, player state, units, combat processor and hero subsystem.
namespace UE::HeroesOfLiberty::Tests::HeroReturn
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Slots 0 (a human, with a player state), 1 and 2 (AI, none): three camps, so one defeat does not end the match. */
	FHOLMatchConfig MakeConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : Index <= 2 ? EHOLSlotState::AI : EHOLSlotState::Closed;
			Config.Slots[Index].Team = 0;
		}
		return Config;
	}

	int32 State(const EHOLHeroState HeroState) { return static_cast<int32>(HeroState); }

	/** A server world as HeroesOfLiberty.Hero.SheetInCombat's: steps run by hand, the combat processor in each. */
	struct FWorld
	{
		FAutomationTestBase& Test;
		UWorld* World = nullptr;
		AHOLGameState* GameState = nullptr;
		AHOLPlayerState* Player = nullptr;
		AHOLBuilding* Forum = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSimulationSubsystem* Simulation = nullptr;
		UHOLHeroSubsystem* Heroes = nullptr;
		UHOLCombatProcessor* Combat = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		IConsoleVariable* CombatEnabled = nullptr;
		int32 FormerEnabled = 0;
		int64 Step = 0;
		float StepSeconds = 0.05f;

		explicit FWorld(FAutomationTestBase& InTest) : Test(InTest)
		{
			CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
			if (CombatEnabled)
			{
				FormerEnabled = CombatEnabled->GetInt();
				CombatEnabled->Set(1, ECVF_SetByCode);
			}
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			GameState = World->SpawnActor<AHOLGameState>();
			World->SetGameState(GameState);
			Player = World->SpawnActor<AHOLPlayerState>();
			Forum = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 2000.0, 0.0), FRotator::ZeroRotator);
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			if (!GameState || !Player || !Forum || !Units || !Simulation || !Heroes || !EntitySubsystem || !CombatEnabled)
			{
				return;
			}
			GameState->InitVictory(MakeConfig());
			Player->SetMatchSlot(0, 0);
			if (!GameState->PlayerArray.Contains(Player))
			{
				GameState->AddPlayerState(Player);
			}
			Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
			Forum->SetPrimaryForum(true);
			Heroes->BindSteps();
			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Combat = NewObject<UHOLCombatProcessor>(Simulation);
			Combat->CallInitialize(Simulation, EntityManager.ToSharedRef());
			StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		}

		~FWorld()
		{
			if (CombatEnabled)
			{
				CombatEnabled->Set(FormerEnabled, ECVF_SetByCode);
			}
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Combat != nullptr; }

		/** One step as UHOLSimulationSubsystem::RunStep makes it: the true step index (death steps, server step) moves on. */
		void RunStep()
		{
			Simulation->GetOnStepStarted().Broadcast(Simulation->GetStepIndex());
			Units->ServeBirthRequests();
			FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
			UE::Mass::Executor::Run(*Combat, ProcessingContext);
			Units->FlushDestructions();
			Step = Simulation->AdvanceStepIndexForTests();
			Simulation->GetOnStepEnded().Broadcast(Step);
		}

		/** Steps run until the simulation's index is Target (none if it is there already). */
		void RunStepsTo(const int64 Target)
		{
			while (Simulation->GetStepIndex() < Target)
			{
				RunStep();
			}
		}

		/** Slot's main forum at Location (slot 0's is made by the world). */
		AHOLBuilding* PlaceForum(const int32 Slot, const FVector& Location)
		{
			AHOLBuilding* SlotForum = World->SpawnActor<AHOLBuilding>(Location, FRotator::ZeroRotator);
			if (SlotForum)
			{
				SlotForum->Setup(HOLBuildingTypeNames::MainCentre, static_cast<uint8>(Slot), static_cast<uint8>(Slot));
				SlotForum->SetPrimaryForum(true);
			}
			return SlotForum;
		}

		/** Whether slot's hero stands within 1 m of the starting hero's place by Building. */
		bool IsByForum(const int32 Slot, const AHOLBuilding& Building) const
		{
			FHOLUnitView View;
			if (!Units->GetUnitView(Units->FindHeroUnit(Slot), View))
			{
				return false;
			}
			const FHOLBuildingTypeRow* Type = Building.GetType();
			const float HalfBody = 0.5f * static_cast<float>((Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D(1200.0)).GetMax());
			return FVector::Dist2D(View.Location, FHOLStartRules::GetHeroLocation(Building.GetActorLocation(), HalfBody)) <= 100.0;
		}

		int32 HeroHealth(const int32 Slot) const
		{
			FHOLUnitView View;
			return Units->GetUnitView(Units->FindHeroUnit(Slot), View) ? View.Health : -1;
		}

		/** Slot's hero born at Location (served at once). His identifier, 0 if none. */
		uint32 PlaceHero(const int32 Slot, const FVector& Location)
		{
			FHOLUnitSpawnRequest Request;
			Request.TypeName = FHOLStartRules::HeroTypeName;
			Request.Count = 1;
			Request.OwnerPlayer = static_cast<uint8>(Slot);
			Request.Center = Location;
			Units->RequestUnits(Request);
			Units->ServeBirthRequests();
			return Units->FindHeroUnit(Slot);
		}

		/** Slot's hero killed (hol.Hero.Kill's path), one step for the combat processor to note his fall, then his body destroyed. */
		void KillHero(const int32 Slot, const bool bDestroyBody = true)
		{
			const uint32 Hero = Units->FindHeroUnit(Slot);
			Units->KillUnitsOfOwner(static_cast<uint8>(Slot), FHOLStartRules::HeroTypeName);
			RunStep();
			if (bDestroyBody && Hero != 0)
			{
				const uint32 Gone[] = { Hero };
				Units->RequestDestroyUnits(Gone);
				Units->FlushDestructions();
				RunStep();
			}
		}

		int32 NumHeroes(const int32 Slot) const
		{
			TArray<uint32> Ids;
			Units->GetUnitIdsOfType(FHOLStartRules::HeroTypeName, Ids);
			int32 Num = 0;
			for (const uint32 Id : Ids)
			{
				FHOLUnitView View;
				Num += Units->GetUnitView(Id, View) && View.OwnerPlayer == Slot ? 1 : 0;
			}
			return Num;
		}

		EHOLHeroState PublicState(const int32 Slot) const { return GameState->GetHeroSheet(Slot).State; }

		/** Slot's player buys its hero's return through the real order queue (ServerReturnHero's path), taken at the next step. */
		void OrderReturn(const int32 Slot)
		{
			if (UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>())
			{
				Commands->IssueHeroReturnOrder(Slot);
			}
			else
			{
				Test.AddError(TEXT("No order queue in the test world"));
			}
		}

		int32 Gold() const { return Player->GetResources().Gold; }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnBirthTest, "HeroesOfLiberty.Hero.ReturnBirth", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// Born again at full health by the main forum standing (the starting hero's place), once; refused while alive, for a place
// that does not exist, with the main forum down and no other forum, and for a defeated slot.
bool FHOLHeroReturnBirthTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready (game state, player state, forum, subsystems, combat)"), W.IsReady()))
	{
		return false;
	}
	UWorld* World = W.World;
	TestTrue(TEXT("The main forum stands"), UHOLHeroLibrary::IsMainForumStanding(World, 0));
	TestTrue(FString::Printf(TEXT("... at (5000, 2000): %s"), *UHOLHeroLibrary::GetMainForumPosition(World, 0).ToString()), UHOLHeroLibrary::GetMainForumPosition(World, 0) == FIntPoint(5000, 2000));
	TestEqual(TEXT("No other forum (D384 later)"), UHOLHeroLibrary::GetOtherForumPositions(World, 0).Num(), 0);
	TestFalse(TEXT("Slot 3 (closed): no main forum"), UHOLHeroLibrary::IsMainForumStanding(World, 3));

	const uint32 First = W.PlaceHero(0, FVector(0.0, 0.0, 0.0));
	W.RunStep();
	TestTrue(TEXT("Slot 0's hero born"), First != 0);
	TestEqual(TEXT("... Alive"), State(UHOLHeroLibrary::GetHeroState(World, 0)), State(EHOLHeroState::Alive));
	TestEqual(TEXT("... level 1"), UHOLHeroLibrary::GetHeroLevel(World, 0), 1);
	TestFalse(TEXT("Return refused while he is alive"), UHOLHeroLibrary::RequestHeroReturn(World, 0, -1));
	W.RunStep();
	TestEqual(TEXT("... still one hero"), W.NumHeroes(0), 1);

	W.KillHero(0);
	TestEqual(TEXT("Killed, body gone: Dead"), State(W.PublicState(0)), State(EHOLHeroState::Dead));
	TestEqual(TEXT("... no hero unit"), W.Units->FindHeroUnit(0), 0u);
	TestFalse(TEXT("Return to other forum 0: refused (none)"), UHOLHeroLibrary::RequestHeroReturn(World, 0, 0));
	TestFalse(TEXT("Return to place -2 (none): refused"), UHOLHeroLibrary::RequestHeroReturn(World, 0, -2));
	TestFalse(TEXT("Return of slot 1 (never had a hero): refused"), UHOLHeroLibrary::RequestHeroReturn(World, 1, -1));
	TestTrue(TEXT("Return by the main forum: accepted"), UHOLHeroLibrary::RequestHeroReturn(World, 0, -1));
	TestFalse(TEXT("... asked again before his birth: refused"), UHOLHeroLibrary::RequestHeroReturn(World, 0, -1));
	W.RunStep();
	const uint32 Back = W.Units->FindHeroUnit(0);
	FHOLUnitView View;
	if (TestTrue(TEXT("Born again (a new unit)"), Back != 0 && Back != First && W.Units->GetUnitView(Back, View)))
	{
		TestEqual(TEXT("... one hero for slot 0"), W.NumHeroes(0), 1);
		TestEqual(TEXT("... of slot 0"), static_cast<int32>(View.OwnerPlayer), 0);
		TestEqual(TEXT("... full health (450)"), View.Health, 450);
		const FHOLBuildingTypeRow* Type = W.Forum->GetType();
		const float HalfBody = 0.5f * static_cast<float>((Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D(1200.0)).GetMax());
		TestTrue(TEXT("... at the starting hero's place by the main forum (within 1 m)"),
			FVector::Dist2D(View.Location, FHOLStartRules::GetHeroLocation(W.Forum->GetActorLocation(), HalfBody)) <= 100.0);
		TestEqual(TEXT("... Alive on his sheet"), State(W.PublicState(0)), State(EHOLHeroState::Alive));
		TestEqual(TEXT("... 450 HP on his sheet"), static_cast<int32>(W.GameState->GetHeroSheet(0).Health), 450);
		TestEqual(TEXT("... no death step"), UHOLHeroLibrary::GetHeroDeathStep(World, 0), -1);
	}
	W.RunStep();
	TestEqual(TEXT("Still one hero a step later"), W.NumHeroes(0), 1);

	// The main forum down (a fall: in ruin) and no other forum: no return.
	W.KillHero(0);
	TestTrue(TEXT("The main forum falls (2 lives: in ruin)"), W.GameState->HandleForumFall(*W.Forum));
	TestFalse(TEXT("... not standing"), UHOLHeroLibrary::IsMainForumStanding(World, 0));
	TestTrue(TEXT("... still at (5000, 2000)"), UHOLHeroLibrary::GetMainForumPosition(World, 0) == FIntPoint(5000, 2000));
	TestFalse(TEXT("Main forum down, no other forum: return refused"), UHOLHeroLibrary::RequestHeroReturn(World, 0, -1));
	W.RunStep();
	TestEqual(TEXT("... no hero born"), W.NumHeroes(0), 0);

	// A defeated slot: no return, its forum standing all the same.
	AHOLBuilding* AIForum = World->SpawnActor<AHOLBuilding>(FVector(-5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("slot 2's forum"), AIForum))
	{
		AIForum->Setup(HOLBuildingTypeNames::MainCentre, 2, 2);
		AIForum->SetPrimaryForum(true);
		TestTrue(TEXT("Slot 2's hero born"), W.PlaceHero(2, FVector(-3000.0, 0.0, 0.0)) != 0);
		W.RunStep();
		W.KillHero(2);
		TestTrue(TEXT("Slot 2 defeated"), W.GameState->DefeatSlot(2, EHOLDefeatReason::Cheat));
		TestFalse(TEXT("Defeated slot 2: return refused"), UHOLHeroLibrary::RequestHeroReturn(World, 2, -1));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnPayTest, "HeroesOfLiberty.Hero.ReturnPay", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// Gold taken from the slot's own stock when it has enough, nothing taken otherwise; false for a slot without stock.
bool FHOLHeroReturnPayTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UWorld* World = W.World;
	W.Player->SetResources(FHOLResources(10, 20, 30, 200));
	TestTrue(TEXT("Pay 150 of 200: taken"), UHOLHeroLibrary::TryPayGold(World, 0, 150));
	TestEqual(TEXT("... 50 gold left"), W.Player->GetResources().Gold, 50);
	TestFalse(TEXT("Pay 51 of 50: refused"), UHOLHeroLibrary::TryPayGold(World, 0, 51));
	TestEqual(TEXT("... still 50"), W.Player->GetResources().Gold, 50);
	TestFalse(TEXT("Pay -5: refused"), UHOLHeroLibrary::TryPayGold(World, 0, -5));
	TestEqual(TEXT("... still 50"), W.Player->GetResources().Gold, 50);
	TestTrue(TEXT("Pay 50 of 50: taken"), UHOLHeroLibrary::TryPayGold(World, 0, 50));
	TestEqual(TEXT("... 0 gold left"), W.Player->GetResources().Gold, 0);
	TestTrue(TEXT("Pay 0 of 0 (the price at its end): taken"), UHOLHeroLibrary::TryPayGold(World, 0, 0));
	TestTrue(TEXT("... the other resources untouched"), W.Player->GetResources() == FHOLResources(10, 20, 30, 0));
	TestFalse(TEXT("Slot 1 (AI, no player state): no stock, false"), UHOLHeroLibrary::TryPayGold(World, 1, 0));
	TestFalse(TEXT("Slot 9 (out of range): false"), UHOLHeroLibrary::TryPayGold(World, 9, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnReadyTest, "HeroesOfLiberty.Hero.ReturnReady", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// ReadyToReturn on the public sheet (game state) from SetHeroReturnReady, kept while his body lies and once it is gone, his
// death step kept; Alive again once born back. Refused while alive.
bool FHOLHeroReturnReadyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UWorld* World = W.World;
	TestEqual(TEXT("The simulation's rate: 20 steps per second"), UHOLHeroLibrary::GetSimulationRateHz(), GetDefault<UHOLSimulationSettings>()->SimulationRateHz);
	TestEqual(TEXT("The server step: the simulation's"), static_cast<int64>(UHOLHeroLibrary::GetServerStep(World)), W.Simulation->GetStepIndex());

	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	W.RunStep();
	TestFalse(TEXT("Ready refused while he is alive"), UHOLHeroLibrary::SetHeroReturnReady(World, 0));
	TestFalse(TEXT("Ready refused for slot 1 (no hero)"), UHOLHeroLibrary::SetHeroReturnReady(World, 1));

	W.KillHero(0, /*bDestroyBody*/ false);
	TestEqual(TEXT("Killed: Dead"), State(UHOLHeroLibrary::GetHeroState(World, 0)), State(EHOLHeroState::Dead));
	const int32 DeathStep = UHOLHeroLibrary::GetHeroDeathStep(World, 0);
	TestTrue(FString::Printf(TEXT("... a death step (%d)"), DeathStep), DeathStep >= 0);
	TestEqual(TEXT("... the player state's copy"), W.Player->GetHeroPrivate().DeathStep, DeathStep);

	TestTrue(TEXT("Ready: accepted"), UHOLHeroLibrary::SetHeroReturnReady(World, 0));
	TestEqual(TEXT("... ReadyToReturn in the game state at once"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));
	TestEqual(TEXT("... and for the library"), State(UHOLHeroLibrary::GetHeroState(World, 0)), State(EHOLHeroState::ReadyToReturn));
	W.RunStep();
	TestEqual(TEXT("... kept while his body lies"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));
	const uint32 Body = W.Units->FindHeroUnit(0);
	const uint32 Gone[] = { Body };
	W.Units->RequestDestroyUnits(Gone);
	W.Units->FlushDestructions();
	W.RunStep();
	TestEqual(TEXT("... kept once his body is gone"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));
	TestEqual(TEXT("... his death step kept"), UHOLHeroLibrary::GetHeroDeathStep(World, 0), DeathStep);
	TestEqual(TEXT("... level kept"), UHOLHeroLibrary::GetHeroLevel(World, 0), 1);

	TestTrue(TEXT("Return: accepted"), UHOLHeroLibrary::RequestHeroReturn(World, 0, -1));
	W.RunStep();
	TestEqual(TEXT("Born again: Alive"), State(W.PublicState(0)), State(EHOLHeroState::Alive));
	TestEqual(TEXT("... no death step"), W.Player->GetHeroPrivate().DeathStep, -1);
	TestFalse(TEXT("Ready refused now he is alive"), UHOLHeroLibrary::SetHeroReturnReady(World, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReadyAlertTest, "HeroesOfLiberty.Hero.ReadyAlert", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// J8a.4 (D397, D202): the minor alert « Votre héros est prêt à revenir » from the real public sheets (the game state's
// OnHeroSheetsChanged, as every machine hears it): one line for the owner (slot 0) when his hero turns ReadyToReturn, none
// for slot 1 nor a spectator, no double at the next steps nor once Alive, a new one after a second death; slot 1's own
// later. A line only: no banner (bAlert false). The test world has no player: its game state, a spectator, says nothing.
bool FHOLHeroReadyAlertTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UWorld* World = W.World;
	const int32 Viewers[] = { 0, 1, INDEX_NONE };
	FHOLHeroReadyWatch Watches[3];
	int32 Lines[3] = { 0, 0, 0 };
	W.GameState->OnHeroSheetsChanged.AddLambda([&]()
	{
		for (int32 Index = 0; Index < 3; ++Index)
		{
			Lines[Index] += Watches[Index].Update(Viewers[Index], W.GameState->GetHeroSheet(Viewers[Index]).State) ? 1 : 0;
		}
	});
	int32 OwnLines = 0;
	W.GameState->OnHeroReadyMessage.AddLambda([&OwnLines](const FHOLForumMessage&) { ++OwnLines; });
	auto Count = [&Lines]() { return FString::Printf(TEXT("%d %d %d"), Lines[0], Lines[1], Lines[2]); };

	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	TestTrue(TEXT("Slot 1's hero born"), W.PlaceHero(1, FVector(-3000.0, 0.0, 0.0)) != 0);
	W.RunStep();
	TestEqual(TEXT("Both alive: no line (slot 0, slot 1, spectator)"), Count(), FString(TEXT("0 0 0")));
	W.KillHero(0, /*bDestroyBody*/ false);
	W.RunStep();
	TestEqual(TEXT("Slot 0 Dead: no line yet"), Count(), FString(TEXT("0 0 0")));
	TestTrue(TEXT("Slot 0 ready"), UHOLHeroLibrary::SetHeroReturnReady(World, 0));
	TestEqual(TEXT("... one line for slot 0 alone"), Count(), FString(TEXT("1 0 0")));
	W.RunStep();
	W.RunStep();
	const uint32 Gone[] = { W.Units->FindHeroUnit(0) };
	W.Units->RequestDestroyUnits(Gone);
	W.Units->FlushDestructions();
	W.RunStep();
	TestTrue(TEXT("... ready again (no change)"), UHOLHeroLibrary::SetHeroReturnReady(World, 0));
	TestEqual(TEXT("... no double at the next steps, his body gone, ready asked again"), Count(), FString(TEXT("1 0 0")));
	TestTrue(TEXT("Return: accepted"), UHOLHeroLibrary::RequestHeroReturn(World, 0, -1));
	W.RunStep();
	TestEqual(TEXT("Born again: Alive"), State(W.PublicState(0)), State(EHOLHeroState::Alive));
	TestEqual(TEXT("... no line when Alive"), Count(), FString(TEXT("1 0 0")));
	W.KillHero(0, /*bDestroyBody*/ false);
	W.RunStep();
	TestEqual(TEXT("Second death: no line while Dead"), Count(), FString(TEXT("1 0 0")));
	TestTrue(TEXT("Ready again"), UHOLHeroLibrary::SetHeroReturnReady(World, 0));
	TestEqual(TEXT("... a new line for slot 0"), Count(), FString(TEXT("2 0 0")));
	W.KillHero(1, /*bDestroyBody*/ false);
	W.RunStep();
	TestTrue(TEXT("Slot 1 ready"), UHOLHeroLibrary::SetHeroReturnReady(World, 1));
	TestEqual(TEXT("... its own line, slot 0 none more"), Count(), FString(TEXT("2 1 0")));
	TestEqual(TEXT("No player here (a spectator): the game state said nothing"), OwnLines, 0);
	const FHOLForumMessage Message = HOLForumMessages::GetHeroReadyMessage();
	TestEqual(TEXT("The line's text"), Message.Text.ToString(), FString(TEXT("Votre héros est prêt à revenir")));
	TestFalse(TEXT("... a minor alert: no banner nor sound"), Message.bAlert);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroStepEventTest, "HeroesOfLiberty.Hero.StepEvent", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// OnHeroStep heard once per simulation step, with that step, the sheets already written (the state the step made).
bool FHOLHeroStepEventTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UHOLHeroStepProbe* Probe = NewObject<UHOLHeroStepProbe>(W.Heroes);
	Probe->Listen(*W.Heroes, 0);
	TestTrue(TEXT("Slot 0's hero asked"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	const int64 First = W.Step + 1;
	W.RunStep();
	W.RunStep();
	W.RunStep();
	if (TestEqual(TEXT("Heard once per step: 3"), Probe->Steps.Num(), 3))
	{
		TestTrue(TEXT("... the steps in order"), Probe->Steps == TArray<int32>{ static_cast<int32>(First), static_cast<int32>(First + 1), static_cast<int32>(First + 2) });
		TestEqual(TEXT("... the sheet already Alive at the first"), State(Probe->States[0]), State(EHOLHeroState::Alive));
	}
	W.Units->KillUnitsOfOwner(0, FHOLStartRules::HeroTypeName);
	W.RunStep();
	TestEqual(TEXT("The step of his fall: heard once more"), Probe->Steps.Num(), 4);
	TestEqual(TEXT("... the sheet already Dead"), Probe->States.Num() == 4 ? State(Probe->States[3]) : -1, State(EHOLHeroState::Dead));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnStepClockTest, "HeroesOfLiberty.Hero.ReturnStepClock", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// The test world's steps move the true step index: the death step and the server step are the simulation's, not 0.
bool FHOLHeroReturnStepClockTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(50);
	TestEqual(TEXT("50 steps run: the server step is 50"), UHOLHeroLibrary::GetServerStep(W.World), 50);
	W.KillHero(0, /*bDestroyBody*/ false);
	TestEqual(TEXT("Killed during step 50 (the index while its processors ran)"), UHOLHeroLibrary::GetHeroDeathStep(W.World, 0), 50);
	return true;
}

namespace UE::HeroesOfLiberty::Tests::HeroReturn
{
	/** Delay and price drop of the GDD (J8a-Q5, D25), computed here apart from the script: steps at Level. */
	int64 DelaySteps(const int32 Level) { return static_cast<int64>(30 + 8 * Level) * GetDefault<UHOLSimulationSettings>()->SimulationRateHz; }
	int64 DropSteps() { return static_cast<int64>(60) * GetDefault<UHOLSimulationSettings>()->SimulationRateHz; }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnByItselfTest, "HeroesOfLiberty.Hero.ReturnByItself", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// The script's decision (Script/Hero/HOLHeroReturnRule.as) on the real steps, without any shortcut (level 1: 38 s then
// 60 s): slot 0 (a human) and slot 1 (AI, no player state) Dead until the ready step, ReadyToReturn from it, back by
// themselves at price 0 only, at full health by their main forum, once each, free.
bool FHOLHeroReturnByItselfTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UWorld* World = W.World;
	UHOLHeroReturnSubsystem* Decision = UHOLHeroReturnSubsystem::Find(World);
	if (!TestNotNull(TEXT("A return decision in a game world (a script class deriving UHOLHeroReturnSubsystem)"), Decision))
	{
		return false;
	}
	TestFalse(FString::Printf(TEXT("... %s comes from the script (not native)"), *Decision->GetClass()->GetName()), Decision->GetClass()->HasAnyClassFlags(CLASS_Native));

	AHOLBuilding* AIForum = W.PlaceForum(1, FVector(-5000.0, 0.0, 0.0));
	W.Player->SetResources(FHOLResources(0, 0, 0, 500));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	TestTrue(TEXT("Slot 1's hero born"), W.PlaceHero(1, FVector(-3000.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(20);
	const int32 DecisionsBefore = Decision->GetNumDecisions();
	W.RunStep();
	TestEqual(TEXT("The script decides once per step"), Decision->GetNumDecisions(), DecisionsBefore + 1);

	// Both fall during the same step; their bodies lie until the combat processor takes them away.
	W.Units->KillUnitsOfOwner(0, FHOLStartRules::HeroTypeName);
	W.Units->KillUnitsOfOwner(1, FHOLStartRules::HeroTypeName);
	W.RunStep();
	const int32 DeathStep = UHOLHeroLibrary::GetHeroDeathStep(World, 0);
	TestEqual(TEXT("Slot 0 Dead"), State(W.PublicState(0)), State(EHOLHeroState::Dead));
	TestEqual(TEXT("Slot 1 Dead"), State(W.PublicState(1)), State(EHOLHeroState::Dead));
	TestEqual(TEXT("... the same death step"), UHOLHeroLibrary::GetHeroDeathStep(World, 1), DeathStep);
	TestTrue(FString::Printf(TEXT("... a true death step (%d, not 0)"), DeathStep), DeathStep > 0);
	TestEqual(TEXT("Level 1"), UHOLHeroLibrary::GetHeroLevel(World, 0), 1);

	const int64 ReadyStep = DeathStep + DelaySteps(1);
	TestEqual(TEXT("Level 1: ready 760 steps (38 s) after death"), ReadyStep - DeathStep, static_cast<int64>(760));
	W.RunStepsTo(ReadyStep - 1);
	TestEqual(TEXT("One step before the ready step: slot 0 still Dead"), State(W.PublicState(0)), State(EHOLHeroState::Dead));
	TestEqual(TEXT("... slot 1 still Dead"), State(W.PublicState(1)), State(EHOLHeroState::Dead));
	W.RunStep();
	TestEqual(TEXT("At the ready step: slot 0 ReadyToReturn"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));
	TestEqual(TEXT("... slot 1 ReadyToReturn"), State(W.PublicState(1)), State(EHOLHeroState::ReadyToReturn));
	TestEqual(TEXT("... no hero yet"), W.NumHeroes(0) + W.NumHeroes(1), 0);

	const int64 AutoStep = ReadyStep + DropSteps();
	TestEqual(TEXT("Back by itself 1 960 steps (98 s) after death"), AutoStep - DeathStep, static_cast<int64>(1960));
	W.RunStepsTo(AutoStep - 1);
	TestEqual(TEXT("One step before price 0: slot 0 has no hero"), W.NumHeroes(0), 0);
	TestEqual(TEXT("... slot 0 still ReadyToReturn"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));
	W.RunStep();
	TestEqual(TEXT("At price 0 (asked at its end, born at the next step's start): no hero yet"), W.NumHeroes(0) + W.NumHeroes(1), 0);
	W.RunStep();
	TestEqual(TEXT("After price 0: slot 0's hero back, one"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... Alive"), State(W.PublicState(0)), State(EHOLHeroState::Alive));
	TestEqual(TEXT("... full health (450)"), W.HeroHealth(0), 450);
	TestTrue(TEXT("... by its main forum"), W.IsByForum(0, *W.Forum));
	TestEqual(TEXT("... free: 500 gold kept"), W.Player->GetResources().Gold, 500);
	TestEqual(TEXT("Slot 1 (AI) back, one"), W.NumHeroes(1), 1);
	TestEqual(TEXT("... Alive"), State(W.PublicState(1)), State(EHOLHeroState::Alive));
	TestEqual(TEXT("... full health (450)"), W.HeroHealth(1), 450);
	TestTrue(TEXT("... by its own main forum"), AIForum && W.IsByForum(1, *AIForum));
	W.RunStepsTo(AutoStep + 40);
	TestEqual(TEXT("40 steps later: still one hero for slot 0"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... and for slot 1"), W.NumHeroes(1), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnForumDownTest, "HeroesOfLiberty.Hero.ReturnForumDown", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// The main forum down and no other forum: still ReadyToReturn, no return past price 0; once the forum stands again
// (rebuilt: hol.Victory.SkipRuin's path), the hero comes back by itself at the next steps.
bool FHOLHeroReturnForumDownTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UWorld* World = W.World;
	if (!TestNotNull(TEXT("A return decision (script)"), UHOLHeroReturnSubsystem::Find(World)))
	{
		return false;
	}
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	W.KillHero(0, /*bDestroyBody*/ false);
	const int32 DeathStep = UHOLHeroLibrary::GetHeroDeathStep(World, 0);
	TestTrue(TEXT("The main forum falls (in ruin)"), W.GameState->HandleForumFall(*W.Forum));
	TestFalse(TEXT("... not standing"), UHOLHeroLibrary::IsMainForumStanding(World, 0));

	const int64 AutoStep = DeathStep + DelaySteps(1) + DropSteps();
	W.RunStepsTo(AutoStep + 60);
	TestEqual(TEXT("3 s past price 0, forum down: no hero"), W.NumHeroes(0), 0);
	TestEqual(TEXT("... ReadyToReturn"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));

	TestTrue(TEXT("... the forum still down (its ruin over by itself: rebuilding, no builder)"), W.GameState->GetForumState(0).IsDown());

	// Rebuilt at once (hol.Victory.SkipRuin's path): through the phases left.
	for (int32 Phase = 0; Phase < 2 && W.GameState->GetForumState(0).IsDown(); ++Phase)
	{
		W.GameState->SkipForumPhase(0);
	}
	TestTrue(TEXT("... standing"), UHOLHeroLibrary::IsMainForumStanding(World, 0));
	W.RunStep();
	W.RunStep();
	TestEqual(TEXT("Forum up: the hero back by itself, one"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... Alive"), State(W.PublicState(0)), State(EHOLHeroState::Alive));
	TestTrue(TEXT("... by the main forum"), W.IsByForum(0, *W.Forum));
	W.RunStepsTo(W.Simulation->GetStepIndex() + 20);
	TestEqual(TEXT("20 steps later: still one"), W.NumHeroes(0), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnOrderQueueTest, "HeroesOfLiberty.Hero.ReturnOrderQueue", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// J8a.4 loop 4: the player's paid return through the order queue, priced at the step that takes the order (the step after
// the increment, as the decision's): refused while alive and one step before the ready step (gold kept); two orders taken
// at the same step: 150 gold taken once, one hero back at full health by the main forum; bought again once back: refused.
bool FHOLHeroReturnOrderQueueTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()) || !TestNotNull(TEXT("A return decision (script)"), UHOLHeroReturnSubsystem::Find(W.World)))
	{
		return false;
	}
	UWorld* World = W.World;
	W.Player->SetResources(FHOLResources(0, 0, 0, 500));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("Bought while alive: refused, 500 gold kept"), W.Gold(), 500);
	TestEqual(TEXT("... one hero"), W.NumHeroes(0), 1);

	W.KillHero(0);
	const int32 DeathStep = UHOLHeroLibrary::GetHeroDeathStep(World, 0);
	const int64 ReadyStep = DeathStep + DelaySteps(1);
	W.RunStepsTo(ReadyStep - 2);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("Order taken at the step before the ready step"), W.Simulation->GetStepIndex(), ReadyStep - 1);
	TestEqual(TEXT("... refused: 500 gold kept"), W.Gold(), 500);
	W.RunStep();
	TestEqual(TEXT("... no hero"), W.NumHeroes(0), 0);
	TestEqual(TEXT("... ReadyToReturn now (the ready step passed)"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));

	// Two orders taken at the same step (the step after the ready one: still 150, rounded up).
	W.OrderReturn(0);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("Two orders at ReadyStep + 1: 150 gold taken once"), W.Gold(), 350);
	TestEqual(TEXT("... born at the next step's start: no hero yet"), W.NumHeroes(0), 0);
	W.RunStep();
	TestEqual(TEXT("... then one hero"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... Alive"), State(W.PublicState(0)), State(EHOLHeroState::Alive));
	TestEqual(TEXT("... full health (450)"), W.HeroHealth(0), 450);
	TestTrue(TEXT("... by the main forum"), W.IsByForum(0, *W.Forum));
	W.OrderReturn(0);
	W.RunStepsTo(W.Simulation->GetStepIndex() + 40);
	TestEqual(TEXT("Bought again once back: refused, 40 steps later still one hero"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... 350 gold kept"), W.Gold(), 350);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnOrderReadyStepTest, "HeroesOfLiberty.Hero.ReturnOrderReadyStep", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// The exact limit: an order taken at the ready step itself is bought at 150.
bool FHOLHeroReturnOrderReadyStepTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	W.Player->SetResources(FHOLResources(0, 0, 0, 500));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	W.KillHero(0);
	const int64 ReadyStep = UHOLHeroLibrary::GetHeroDeathStep(W.World, 0) + DelaySteps(1);
	W.RunStepsTo(ReadyStep - 1);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("Order taken at the ready step"), W.Simulation->GetStepIndex(), ReadyStep);
	TestEqual(TEXT("... bought at 150"), W.Gold(), 350);
	W.RunStep();
	TestEqual(TEXT("... one hero"), W.NumHeroes(0), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnOrderPriceTest, "HeroesOfLiberty.Hero.ReturnOrderPrice", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// Bought later: the price of the step that takes the order, rounded up (599 steps after the ready step: 75.125 -> 76).
bool FHOLHeroReturnOrderPriceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	W.Player->SetResources(FHOLResources(0, 0, 0, 500));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	W.KillHero(0);
	const int64 ReadyStep = UHOLHeroLibrary::GetHeroDeathStep(W.World, 0) + DelaySteps(1);
	W.RunStepsTo(ReadyStep + 598);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("599 steps after the ready step: 76 gold taken (75.125 rounded up)"), W.Gold(), 500 - 76);
	W.RunStep();
	TestEqual(TEXT("... one hero"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... full health (450)"), W.HeroHealth(0), 450);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnOrderGoldShortTest, "HeroesOfLiberty.Hero.ReturnOrderGoldShort", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// Not enough gold (149 for 150): refused, gold kept, the hero still dead; an AI slot (no stock) refused too; at price 0 an
// order taken at the same step as the return by itself: one birth, nothing paid. Both back at price 0.
bool FHOLHeroReturnOrderGoldShortTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	AHOLBuilding* AIForum = W.PlaceForum(1, FVector(-5000.0, 0.0, 0.0));
	W.Player->SetResources(FHOLResources(0, 0, 0, 149));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	TestTrue(TEXT("Slot 1's hero born"), W.PlaceHero(1, FVector(-3000.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	W.Units->KillUnitsOfOwner(1, FHOLStartRules::HeroTypeName);
	W.KillHero(0, /*bDestroyBody*/ false);
	const int32 DeathStep = UHOLHeroLibrary::GetHeroDeathStep(W.World, 0);
	TestEqual(TEXT("Both fell at the same step"), UHOLHeroLibrary::GetHeroDeathStep(W.World, 1), DeathStep);
	const int64 ReadyStep = DeathStep + DelaySteps(1);
	W.RunStepsTo(ReadyStep - 1);
	W.OrderReturn(0);
	W.OrderReturn(1);
	W.RunStep();
	TestEqual(TEXT("149 gold for 150: refused, 149 kept"), W.Gold(), 149);
	W.RunStep();
	TestEqual(TEXT("... no hero for slot 0"), W.NumHeroes(0), 0);
	TestEqual(TEXT("... ReadyToReturn"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));
	TestEqual(TEXT("Slot 1 (AI, no stock): refused, no hero"), W.NumHeroes(1), 0);

	const int64 AutoStep = ReadyStep + DropSteps();
	W.RunStepsTo(AutoStep - 1);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("Order at price 0, with the return by itself: 149 kept"), W.Gold(), 149);
	W.RunStep();
	TestEqual(TEXT("... one hero for slot 0"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... Alive"), State(W.PublicState(0)), State(EHOLHeroState::Alive));
	TestEqual(TEXT("Slot 1 back by itself, one"), W.NumHeroes(1), 1);
	TestTrue(TEXT("... by its forum"), AIForum && W.IsByForum(1, *AIForum));
	W.RunStepsTo(AutoStep + 40);
	TestEqual(TEXT("40 steps later: still one hero for slot 0"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... 149 gold kept"), W.Gold(), 149);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnOrderRefusedTest, "HeroesOfLiberty.Hero.ReturnOrderRefused", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// The main forum down (no other forum): refused, gold kept; a defeated slot: its order refused by the queue, gold kept.
bool FHOLHeroReturnOrderRefusedTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	W.Player->SetResources(FHOLResources(0, 0, 0, 500));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	W.KillHero(0, /*bDestroyBody*/ false);
	const int64 ReadyStep = UHOLHeroLibrary::GetHeroDeathStep(W.World, 0) + DelaySteps(1);
	TestTrue(TEXT("The main forum falls (in ruin)"), W.GameState->HandleForumFall(*W.Forum));
	W.RunStepsTo(ReadyStep + 4);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("Forum down: refused, 500 gold kept"), W.Gold(), 500);
	W.RunStep();
	TestEqual(TEXT("... no hero"), W.NumHeroes(0), 0);

	for (int32 Phase = 0; Phase < 2 && W.GameState->GetForumState(0).IsDown(); ++Phase)
	{
		W.GameState->SkipForumPhase(0);
	}
	TestTrue(TEXT("The forum stands again"), UHOLHeroLibrary::IsMainForumStanding(W.World, 0));
	TestTrue(TEXT("Slot 0 defeated (surrender)"), W.GameState->DefeatSlot(0, EHOLDefeatReason::Surrender));
	// The collapse of a defeated slot (D358) takes its stock and its forum: the order must take nothing more.
	const int32 GoldDefeated = W.Gold();
	W.OrderReturn(0);
	W.RunStep();
	W.RunStep();
	TestEqual(TEXT("Defeated: refused, gold kept"), W.Gold(), GoldDefeated);
	TestEqual(TEXT("... no hero"), W.NumHeroes(0), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnTimePercentTest, "HeroesOfLiberty.Hero.ReturnTimePercent", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// The network test's shortcut (hol.Hero.ReturnTimePercent, J8a.4 loop 5) on the real chain (cvar -> HOLHero:: bridge ->
// the script's rule and decision): 100 by default (the real times, the other tests); at 20 % slot 0 (a human) and slot 1
// (AI) ready 152 steps after death (not 151), slot 0's order 60 steps after the ready step paid by the same formula on
// the shorter drop (150 x 180 / 240 = 112.5 -> 113), slot 1 back by itself, free, 392 steps after death.
bool FHOLHeroReturnTimePercentTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	IConsoleVariable* Percent = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Hero.ReturnTimePercent"));
	if (!TestNotNull(TEXT("hol.Hero.ReturnTimePercent exists"), Percent))
	{
		return false;
	}
	TestEqual(TEXT("Default: 100 % (the rules' times)"), Percent->GetInt(), 100);
	TestEqual(TEXT("... read by the bridge"), UHOLHeroLibrary::GetReturnTimePercent(), 100);
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	ON_SCOPE_EXIT { Percent->Set(100, ECVF_SetByCode); };
	Percent->Set(20, ECVF_SetByCode);
	TestEqual(TEXT("Set to 20 %"), UHOLHeroLibrary::GetReturnTimePercent(), 20);

	AHOLBuilding* AIForum = W.PlaceForum(1, FVector(-5000.0, 0.0, 0.0));
	W.Player->SetResources(FHOLResources(0, 0, 0, 500));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	TestTrue(TEXT("Slot 1's hero born"), W.PlaceHero(1, FVector(-3000.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	W.Units->KillUnitsOfOwner(0, FHOLStartRules::HeroTypeName);
	W.Units->KillUnitsOfOwner(1, FHOLStartRules::HeroTypeName);
	W.RunStep();
	const int32 DeathStep = UHOLHeroLibrary::GetHeroDeathStep(W.World, 0);
	TestTrue(FString::Printf(TEXT("Both dead at the same step (%d)"), DeathStep), DeathStep > 0 && UHOLHeroLibrary::GetHeroDeathStep(W.World, 1) == DeathStep);
	const int64 ReadyStep = DeathStep + 152;
	W.RunStepsTo(ReadyStep - 1);
	TestEqual(TEXT("151 steps after death: slot 0 still Dead"), State(W.PublicState(0)), State(EHOLHeroState::Dead));
	W.RunStep();
	TestEqual(TEXT("152 steps after death (38 s x 20 %): slot 0 ReadyToReturn"), State(W.PublicState(0)), State(EHOLHeroState::ReadyToReturn));
	TestEqual(TEXT("... slot 1 ReadyToReturn"), State(W.PublicState(1)), State(EHOLHeroState::ReadyToReturn));
	W.RunStepsTo(ReadyStep + 59);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("Ordered, taken 60 steps after the ready step: 113 gold (150 x 180 / 240 rounded up)"), W.Gold(), 500 - 113);
	W.RunStep();
	TestEqual(TEXT("... slot 0's hero back"), W.NumHeroes(0), 1);
	const int64 AutoStep = DeathStep + 392;
	W.RunStepsTo(AutoStep - 1);
	TestEqual(TEXT("391 steps after death: slot 1 not back"), W.NumHeroes(1), 0);
	W.RunStepsTo(AutoStep + 1);
	TestEqual(TEXT("392 steps after death (98 s x 20 %): slot 1 back by itself"), W.NumHeroes(1), 1);
	TestEqual(TEXT("... Alive at full health"), W.HeroHealth(1), 450);
	TestTrue(TEXT("... by its main forum"), AIForum && W.IsByForum(1, *AIForum));
	TestEqual(TEXT("... slot 0's gold untouched by it"), W.Gold(), 500 - 113);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroReturnQuoteTest, "HeroesOfLiberty.Hero.ReturnQuote", UE::HeroesOfLiberty::Tests::HeroReturn::Flags)

// J8a.5 loop 1b: the quote (the script's HOLHeroReturnQuote::Read through UHOLHeroReturnSubsystem::QuoteNow) on the real
// chain, at the server step an order is sent: alive; waiting two steps before the ready step (one step left, no price);
// ready one step before it (an order sent then is taken at the ready step: 150); 598 steps after it the quoted price is
// the gold the order really takes at the next step (76); 50 gold for 150: not buyable; the main forum down: no place, not
// buyable; an AI slot (no stock): gold unknown, not buyable.
bool FHOLHeroReturnQuoteTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::HeroReturn;
	FWorld W(*this);
	if (!TestTrue(TEXT("test world ready"), W.IsReady()))
	{
		return false;
	}
	UHOLHeroReturnSubsystem* Rule = UHOLHeroReturnSubsystem::Find(W.World);
	if (!TestNotNull(TEXT("The script's return rule in the world"), Rule))
	{
		return false;
	}
	auto QuoteNow = [&W, Rule](const int32 Slot) { return Rule->QuoteNow(Slot, UHOLHeroLibrary::GetServerStep(W.World)); };
	auto QuoteState = [](const FHOLHeroReturnQuote& Quote) { return static_cast<int32>(Quote.State); };

	W.Player->SetResources(FHOLResources(0, 0, 0, 500));
	AHOLBuilding* AIForum = W.PlaceForum(1, FVector(-5000.0, 0.0, 0.0));
	TestTrue(TEXT("Slot 0's hero born"), W.PlaceHero(0, FVector(0.0, 0.0, 0.0)) != 0);
	TestTrue(TEXT("Slot 1's hero born"), W.PlaceHero(1, FVector(-3000.0, 0.0, 0.0)) != 0);
	W.RunStepsTo(10);
	TestEqual(TEXT("The bridge reads slot 0's gold (500)"), UHOLHeroLibrary::GetSlotGold(W.World, 0), 500);
	TestEqual(TEXT("... and none for slot 1 (AI, no stock)"), UHOLHeroLibrary::GetSlotGold(W.World, 1), -1);
	const FHOLHeroReturnQuote Alive = QuoteNow(0);
	TestEqual(TEXT("Alive"), QuoteState(Alive), static_cast<int32>(EHOLHeroReturnQuoteState::Alive));
	TestEqual(TEXT("... no price"), Alive.Price, -1);
	TestFalse(TEXT("... not buyable"), Alive.bCanBuy);

	W.Units->KillUnitsOfOwner(1, FHOLStartRules::HeroTypeName);
	W.KillHero(0, /*bDestroyBody*/ false);
	const int64 ReadyStep = UHOLHeroLibrary::GetHeroDeathStep(W.World, 0) + DelaySteps(1);
	W.RunStepsTo(ReadyStep - 2);
	const FHOLHeroReturnQuote Waiting = QuoteNow(0);
	TestEqual(TEXT("Two steps before the ready step: waiting"), QuoteState(Waiting), static_cast<int32>(EHOLHeroReturnQuoteState::Waiting));
	TestEqual(TEXT("... taken at the next step"), Waiting.TakeStep, static_cast<int32>(ReadyStep - 1));
	TestEqual(TEXT("... one step left"), Waiting.StepsToReady, 1);
	TestEqual(TEXT("... one second (rounded up)"), Waiting.SecondsToReady, 1);
	TestEqual(TEXT("... no price"), Waiting.Price, -1);
	TestFalse(TEXT("... not buyable"), Waiting.bCanBuy);
	TestEqual(TEXT("... 500 gold"), Waiting.Gold, 500);
	TestTrue(TEXT("... a place (the main forum)"), Waiting.bHasPlace);

	W.RunStep();
	const FHOLHeroReturnQuote First = QuoteNow(0);
	TestEqual(TEXT("One step before the ready step: ready (an order is taken at the ready step)"), QuoteState(First), static_cast<int32>(EHOLHeroReturnQuoteState::Ready));
	TestEqual(TEXT("... 0 step left"), First.StepsToReady, 0);
	TestEqual(TEXT("... 150 gold"), First.Price, 150);
	TestTrue(TEXT("... buyable"), First.bCanBuy);

	const FHOLHeroReturnQuote AIQuote = QuoteNow(1);
	TestEqual(TEXT("Slot 1 (AI): ready too"), QuoteState(AIQuote), static_cast<int32>(EHOLHeroReturnQuoteState::Ready));
	TestEqual(TEXT("... gold unknown (no stock)"), AIQuote.Gold, -1);
	TestFalse(TEXT("... not buyable"), AIQuote.bCanBuy);
	TestTrue(TEXT("... a place (its forum)"), AIQuote.bHasPlace && AIForum != nullptr);

	W.Player->SetResources(FHOLResources(0, 0, 0, 50));
	const FHOLHeroReturnQuote Short = QuoteNow(0);
	TestEqual(TEXT("50 gold for 150: still 150"), Short.Price, 150);
	TestFalse(TEXT("... not buyable"), Short.bCanBuy);
	W.Player->SetResources(FHOLResources(0, 0, 0, 500));

	W.RunStepsTo(ReadyStep + 598);
	const FHOLHeroReturnQuote Later = QuoteNow(0);
	TestEqual(TEXT("598 steps after the ready step: 76 quoted (the price at the next step, 75.125 rounded up)"), Later.Price, 76);
	TestTrue(TEXT("... buyable"), Later.bCanBuy);
	W.OrderReturn(0);
	W.RunStep();
	TestEqual(TEXT("The order sent then takes the quoted price"), 500 - W.Gold(), Later.Price);
	W.RunStep();
	TestEqual(TEXT("... one hero"), W.NumHeroes(0), 1);
	TestEqual(TEXT("... the quote: alive"), QuoteState(QuoteNow(0)), static_cast<int32>(EHOLHeroReturnQuoteState::Alive));

	TestTrue(TEXT("Slot 1's forum falls (in ruin)"), AIForum && W.GameState->HandleForumFall(*AIForum));
	const FHOLHeroReturnQuote NoPlace = QuoteNow(1);
	TestFalse(TEXT("Slot 1's forum down: no place"), NoPlace.bHasPlace);
	TestFalse(TEXT("... not buyable"), NoPlace.bCanBuy);
	return true;
}

#endif
