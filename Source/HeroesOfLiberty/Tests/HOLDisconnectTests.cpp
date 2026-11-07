// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
#include "AI/HOLAIOrders.h"
#include "AI/HOLAITechnologyLibrary.h"
#include "AI/HOLAITools.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLGatherRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Technologies/HOLTechnologyRules.h"
#include "Technologies/HOLTechnologyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// R7 (R-Q5, D71): a named player gone during the match keeps his slot for a window, then loses as by a surrender.
namespace UE::HeroesOfLiberty::Tests::Disconnect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Alice (slot 0) and Bob (slot 1), humans of two camps; an AI in slot 2 with Bob; the rest closed. */
	FHOLMatchConfig MakeNamedConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (FHOLMatchSlot& Slot : Config.Slots)
		{
			Slot.State = EHOLSlotState::Closed;
			Slot.Team = 0;
		}
		Config.Slots[0].State = EHOLSlotState::Human;
		Config.Slots[0].PlayerName = TEXT("Alice");
		Config.Slots[0].Team = 1;
		Config.Slots[1].State = EHOLSlotState::Human;
		Config.Slots[1].PlayerName = TEXT("Bob");
		Config.Slots[1].Team = 2;
		Config.Slots[2].State = EHOLSlotState::AI;
		Config.Slots[2].Team = 2;
		return Config;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectRulesTest, "HeroesOfLiberty.Match.DisconnectRules",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectRulesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;
	const FHOLMatchConfig Named = MakeNamedConfig();

	TestTrue(TEXT("Bob leaves during the match: his slot kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, false, false, false));
	TestFalse(TEXT("still waiting for the players: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, true, false, false));
	TestFalse(TEXT("the match over: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, false, true, false));
	TestFalse(TEXT("his slot defeated: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, false, false, true));
	TestFalse(TEXT("an AI slot: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 2, false, false, false));
	TestFalse(TEXT("a closed slot: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 4, false, false, false));
	TestFalse(TEXT("no slot (spectator): not kept"), FHOLDisconnectRules::KeepsSlot(Named, INDEX_NONE, false, false, false));
	TestFalse(TEXT("a configuration without names (solo, direct): not kept"),
		FHOLDisconnectRules::KeepsSlot(FHOLMatchConfig::MakeDefault(), 0, false, false, false));

	TestEqual(TEXT("the window by default: 3 min"), FHOLDisconnectRules::DefaultWindowSeconds, 180.);
	TestFalse(TEXT("left at 10, now 189.9: still in the window"), FHOLDisconnectRules::IsWindowOver(10., 189.9, 180.));
	TestTrue(TEXT("left at 10, now 190: over"), FHOLDisconnectRules::IsWindowOver(10., 190., 180.));
	TestEqual(TEXT("seconds left"), FHOLDisconnectRules::GetSecondsLeft(10., 31., 180.), 159.);
	TestEqual(TEXT("no seconds left after the end"), FHOLDisconnectRules::GetSecondsLeft(10., 500., 180.), 0.);

	TArray<FHOLAbsentSlot> Absent;
	Absent.Add({ 3, TEXT("Dan"), 0., FHOLResources() });
	Absent.Add({ 1, TEXT("Bob"), 50., FHOLResources(0, 300, 0, 0) });
	TestEqual(TEXT("Bob's slot found"), FHOLDisconnectRules::FindAbsent(Absent, 1), 1);
	TestEqual(TEXT("Alice not absent"), FHOLDisconnectRules::FindAbsent(Absent, 0), INDEX_NONE);
	TestEqual(TEXT("at 200: Dan's window over, not Bob's"), FHOLDisconnectRules::GetExpired(Absent, 200., 180.), TArray<int32>{ 3 });
	TestEqual(TEXT("at 230: both, in the order of the departures"), FHOLDisconnectRules::GetExpired(Absent, 230., 180.),
		(TArray<int32>{ 3, 1 }));

	const FHOLResources Starting(200, 200, 200, 200);
	TestTrue(TEXT("back: the stock kept"), FHOLDisconnectRules::GetStockOnTaking(&Absent[1], Starting) == FHOLResources(0, 300, 0, 0));
	TestTrue(TEXT("first login: the stock of the start"), FHOLDisconnectRules::GetStockOnTaking(nullptr, Starting) == Starting);
	return true;
}

// R7.2-R7.3 on the game state: Bob (300 wood) leaves, comes back with his 300 wood; leaves again, and at the end of the
// window his slot is defeated (Disconnected), collapsed as by a surrender, and Alice wins.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectGameStateTest, "HeroesOfLiberty.Match.DisconnectGameState",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectGameStateTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state spawned"), State) && TestNotNull(TEXT("player states spawned"), Bob) && BobAgain)
	{
		// A 1v1 (the AI seat of Bob's camp closed): the end of his window is his defeat (D718, T5: with others in his camp the AI keeps the seat).
		FHOLMatchConfig Config1v1 = MakeNamedConfig();
		Config1v1.Slots[2].State = EHOLSlotState::Closed;
		State->InitVictory(Config1v1);
		const FHOLResources Starting(200, 200, 200, 200);
		Bob->SetMatchSlot(1, 1);
		TestFalse(TEXT("first login: not a return (no back to the manager)"), State->GiveSlotStock(*Bob, Starting));
		TestTrue(TEXT("first login: the stock of the start"), Bob->GetResources() == Starting);
		Bob->SetResources(FHOLResources(0, 300, 0, 0));

		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10.));
		Bob->Destroy();	// as after a Logout (a live second holder of the seat would make a later KeepAbsentSlot a duplicate, T4)
		TestTrue(TEXT("slot 1 absent"), State->IsSlotAbsent(1));
		TestEqual(TEXT("no defeat 179 s later"), State->DefeatExpiredAbsences(189., 180.).Num(), 0);
		BobAgain->SetMatchSlot(1, 1);
		TestTrue(TEXT("back in the window: a return (M6-Q15, back to the manager)"), State->GiveSlotStock(*BobAgain, Starting));
		TestTrue(TEXT("back in the window: his 300 wood, not the stock of the start"), BobAgain->GetResources() == FHOLResources(0, 300, 0, 0));
		TestFalse(TEXT("slot 1 no longer absent"), State->IsSlotAbsent(1));
		TestEqual(TEXT("back: no defeat after the window"), State->DefeatExpiredAbsences(1000., 180.).Num(), 0);

		TestTrue(TEXT("Bob leaves again"), State->KeepAbsentSlot(*BobAgain, 1000.));
		TestEqual(TEXT("the end of the window: slot 1 defeated"), State->DefeatExpiredAbsences(1180., 180.), TArray<int32>{ 1 });
		TestTrue(TEXT("reason Disconnected"), State->GetDefeatReason(1) == EHOLDefeatReason::Disconnected);
		TestEqual(TEXT("reason's name"), FString(AHOLGameState::GetReasonName(EHOLDefeatReason::Disconnected)), FString(TEXT("Disconnected")));
		TestFalse(TEXT("no longer absent once defeated"), State->IsSlotAbsent(1));
		TestTrue(TEXT("1v1: his defeat ends the match"), State->IsMatchOver());
		TestFalse(TEXT("a defeated slot is not kept"), State->KeepAbsentSlot(*BobAgain, 1200.));

		// B6b: Bob joins again after his defeat: he watches (no slot, no stock of the start on a beaten camp).
		TestEqual(TEXT("Alice, not defeated: her slot"), State->FindSlotOnLogin(MakeNamedConfig(), TEXT("Alice"), {}), 0);
		TestEqual(TEXT("Bob after his defeat: a spectator"), State->FindSlotOnLogin(MakeNamedConfig(), TEXT("Bob"), {}), INDEX_NONE);
	}

	// Without names (solo, a direct match): no window, the slot freed at once as before.
	AHOLGameState* Solo = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("solo game state spawned"), Solo) && Player)
	{
		Solo->InitVictory(FHOLMatchConfig::MakeDefault());
		Player->SetMatchSlot(0, 0);
		TestFalse(TEXT("solo: not kept"), Solo->KeepAbsentSlot(*Player, 0.));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// M6-Q16 (R7): what Bob gains while away is kept for him. Bob leaves (his player state destroyed, as after a Logout); a
// Peasant of his unloads 10 wood (the gathering's deposit), the enemy destroys his house's foundation (its cost back) and
// his centre with a Peasant queued (the queue back); back in the window, he finds his stock plus all of that.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectGainsTest, "HeroesOfLiberty.Match.DisconnectGains",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectGainsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Foundation = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Centre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("production"), Production) && TestNotNull(TEXT("player states"), Bob)
		&& TestNotNull(TEXT("player states"), BobAgain) && TestNotNull(TEXT("buildings"), Foundation) && TestNotNull(TEXT("buildings"), Centre))
	{
		State->InitVictory(MakeNamedConfig());
		const FHOLResources Starting(500, 300, 100, 100);
		Bob->SetMatchSlot(1, 1);
		State->GiveSlotStock(*Bob, Starting);
		Foundation->Setup(HOLBuildingTypeNames::House, 1, 1, EHOLConstructionState::Foundation);
		Centre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);

		// Bob queues a Peasant (paid at once, as the production's command does).
		const FName Peasant(TEXT("Peasant"));
		const FHOLProductionCost PeasantCost = Production->GetCost(Peasant);
		FHOLResources Stock = Bob->GetResources();
		TestTrue(TEXT("a Peasant queued"), HOLProduction::TryEnqueue(Centre->GetMutableProductionQueue(), Peasant, PeasantCost, 5, Stock)
			== EHOLProductionRefusal::None);
		Bob->SetResources(Stock);
		const FHOLResources AtDeparture = Bob->GetResources();
		const FHOLBuildingTypeRow* HouseType = Foundation->GetType();
		const FHOLResources FoundationRefund = HouseType ? HOLConstruction::GetCancelRefund(HouseType->Cost, Foundation->GetConstructionState(),
			Foundation->GetConstructionWork(), Foundation->GetTotalWork()) : FHOLResources();
		TestTrue(TEXT("a house's foundation gives something back"), FoundationRefund.Wood > 0);
		TestTrue(TEXT("a Peasant costs something"), PeasantCost.Resources.Food > 0);

		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10.));
		Bob->Destroy();
		bool bBobListed = false;
		for (const APlayerState* Each : State->PlayerArray)
		{
			bBobListed |= Each == Bob;
		}
		TestFalse(TEXT("his player state gone, as after a Logout"), bBobListed);

		// A Peasant of his unloads 10 wood at his centre (the gathering's deposit).
		FHOLGatherLoad Load;
		Load.Resource = EHOLResource::Wood;
		Load.Amount = 10;
		FHOLResources AfterDeposit;
		TestEqual(TEXT("his Peasant unloads 10 wood"), UHOLGatherProcessor::DepositLoad(*World, 1, Load, AfterDeposit), 10);
		TestEqual(TEXT("its load emptied"), Load.Amount, 0);

		// The enemy destroys his foundation and his centre (the combat's path): the cost of the foundation and the queue back.
		UHOLConstructionSubsystem::DestroyBuilding(*Foundation);
		UHOLConstructionSubsystem::DestroyBuilding(*Centre);

		BobAgain->SetMatchSlot(1, 1);
		State->GiveSlotStock(*BobAgain, Starting);
		FHOLResources Expected = AtDeparture;
		Expected.Wood += 10;
		Expected += FoundationRefund;
		Expected += PeasantCost.Resources;
		AddInfo(FString::Printf(TEXT("at departure %s, back with %s, expected %s"), *AtDeparture.ToString(), *BobAgain->GetResources().ToString(),
			*Expected.ToString()));
		TestTrue(TEXT("back: his stock plus the 10 wood, the foundation's cost and the queue's"), BobAgain->GetResources() == Expected);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// R7.5: the banner of the others. Bob leaves at 10 (window 180 s): every machine gets his notice (replicated), its end at
// 190; at 29, « Bob : connexion perdue — 2:41 »; back at 100, « Bob : de retour » until 105, then nothing. Gone again and not
// back: his notice goes with his defeat (no more text here).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectBannerTest, "HeroesOfLiberty.Match.DisconnectBanner",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectBannerTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	const FString At161 = FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), 161.).ToString();
	const FString Back = FHOLDisconnectRules::FormatBackLine(TEXT("Bob")).ToString();
	AddInfo(FString::Printf(TEXT("161 s: %s; back: %s"), *At161, *Back));
	// Neutral wording (the game does not know the players' gender), 2026-10-26.
	TestEqual(TEXT("161 s: neutral text"), At161, FString(TEXT("Bob : connexion perdue — 2:41")));
	TestEqual(TEXT("back: neutral text"), Back, FString(TEXT("Bob : de retour")));
	TestTrue(TEXT("160.2 s rounded up: 2:41"), FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), 160.2).ToString().Contains(TEXT("2:41")));
	TestTrue(TEXT("59 s: 0:59"), FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), 59.).ToString().Contains(TEXT("0:59")));
	TestTrue(TEXT("past the end: 0:00"), FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), -3.).ToString().Contains(TEXT("0:00")));
	TestTrue(TEXT("back: Bob, no time"), Back.Contains(TEXT("Bob")) && !Back.Contains(TEXT("0:")) && !Back.Contains(TEXT("—")));
	TestFalse(TEXT("gone and back: two texts"), At161 == Back);

	const FProperty* Notices = AHOLGameState::StaticClass()->FindPropertyByName(TEXT("AbsentNotices"));
	TestTrue(TEXT("the notices replicated (players and spectators see them)"), Notices && Notices->HasAnyPropertyFlags(CPF_Net));
	const FProperty* Kept = AHOLGameState::StaticClass()->FindPropertyByName(TEXT("AbsentSlots"));
	TestTrue(TEXT("the stock kept is not (server only)"), !Kept || !Kept->HasAnyPropertyFlags(CPF_Net));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state spawned"), State) && TestNotNull(TEXT("player states spawned"), Bob) && BobAgain)
	{
		// A 1v1 (the AI seat of Bob's camp closed): the end of his window is his defeat (D718, T5: with others in his camp the AI keeps the seat).
		FHOLMatchConfig Config1v1 = MakeNamedConfig();
		Config1v1.Slots[2].State = EHOLSlotState::Closed;
		State->InitVictory(Config1v1);
		const FHOLResources Starting(200, 200, 200, 200);
		Bob->SetMatchSlot(1, 1);
		State->GiveSlotStock(*Bob, Starting, 0.);
		TestEqual(TEXT("first login: no notice"), State->GetAbsentNotices().Num(), 0);

		TestTrue(TEXT("Bob leaves at 10"), State->KeepAbsentSlot(*Bob, 10., 180.));
		Bob->Destroy();	// as after a Logout
		if (TestEqual(TEXT("one notice"), State->GetAbsentNotices().Num(), 1))
		{
			const FHOLAbsentNotice& Notice = State->GetAbsentNotices()[0];
			TestEqual(TEXT("his slot"), Notice.Slot, 1);
			TestEqual(TEXT("his name"), Notice.Name, FString(TEXT("Bob")));
			TestEqual(TEXT("the window's end at 190"), Notice.EndsAt, 190.);
			TestEqual(TEXT("not back"), Notice.BackAt, 0.);
		}
		// A client's clock of the server a little behind at the departure: never more than the window (3:01 seen in
		// lobby_match -Rejoin, 2026-10-26).
		const TArray<FText> At9 = State->GetAbsenceBannerLines(9.6);
		if (TestEqual(TEXT("at 9.6: one line"), At9.Num(), 1))
		{
			TestEqual(TEXT("at 9.6 (clock behind): 3:00, not 3:01"), At9[0].ToString(), FString(TEXT("Bob : connexion perdue — 3:00")));
		}
		const TArray<FText> At29 = State->GetAbsenceBannerLines(29.);
		if (TestEqual(TEXT("at 29: one line"), At29.Num(), 1))
		{
			TestEqual(TEXT("at 29: Bob, 2:41"), At29[0].ToString(), At161);
		}

		BobAgain->SetMatchSlot(1, 1);
		TestTrue(TEXT("back at 100"), State->GiveSlotStock(*BobAgain, Starting, 100.));
		const TArray<FText> At101 = State->GetAbsenceBannerLines(101.);
		if (TestEqual(TEXT("at 101: one line"), At101.Num(), 1))
		{
			TestEqual(TEXT("at 101: Bob is back"), At101[0].ToString(), Back);
		}
		TestEqual(TEXT("at 104.9: still back"), State->GetAbsenceBannerLines(104.9).Num(), 1);
		TestEqual(TEXT("at 105: nothing (even before the server drops it)"), State->GetAbsenceBannerLines(105.).Num(), 0);
		State->DefeatExpiredAbsences(105., 180.);
		TestEqual(TEXT("at 105: the notice dropped"), State->GetAbsentNotices().Num(), 0);

		TestTrue(TEXT("Bob leaves again at 200"), State->KeepAbsentSlot(*BobAgain, 200., 180.));
		TestEqual(TEXT("his notice again"), State->GetAbsentNotices().Num(), 1);
		TestEqual(TEXT("not back by 380: defeated"), State->DefeatExpiredAbsences(380., 180.), TArray<int32>{ 1 });
		TestEqual(TEXT("defeated: no notice left (the defeat says the rest)"), State->GetAbsentNotices().Num(), 0);
		TestEqual(TEXT("defeated: no line"), State->GetAbsenceBannerLines(381.).Num(), 0);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}


// R7.10: back within the window, his hero's private sheet (XP, cooldowns, death step) is in his new player state at once,
// as his stock: the hero subsystem keeps the true sheet while he is gone, the return writes its copy (no step needed).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectHeroSheetTest, "HeroesOfLiberty.Match.DisconnectHeroSheet",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectHeroSheetTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("hero subsystem"), Heroes) && TestNotNull(TEXT("player state"), Bob))
	{
		State->InitVictory(MakeNamedConfig());
		const FHOLResources Starting(200, 200, 200, 200);
		Bob->SetMatchSlot(1, 1);
		State->GiveSlotStock(*Bob, Starting, 0.);

		// His hero earns XP (hol.Hero.SetXP, the real entry): straight to his player state.
		TestTrue(TEXT("SetXP(1, 12)"), Heroes->SetXP(1, 12));
		TestEqual(TEXT("his XP in his player state"), Bob->GetHeroPrivate().XP, 12);

		TestTrue(TEXT("Bob leaves at 10: slot kept"), State->KeepAbsentSlot(*Bob, 10., 180.));
		Bob->Destroy();
		// D717 (T2): the player state of slot 1 while he is gone is the AI's that holds his seat (not his).
		const AHOLPlayerState* Holder = UHOLHeroSubsystem::FindPlayerState(*World, 1);
		TestTrue(TEXT("only the AI's player state is left for slot 1 while he is gone"), Holder != nullptr && Holder != Bob && Holder->IsABot());
		TestEqual(TEXT("the subsystem keeps his XP while he is gone"), Heroes->GetSheets(1).Private.XP, 12);

		// Back at 100 with a new player state (PostLogin's order: his slot, then the stock); no simulation step.
		AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("new player state"), BobAgain))
		{
			TestEqual(TEXT("a new player state starts empty"), BobAgain->GetHeroPrivate().XP, 0);
			BobAgain->SetMatchSlot(1, 1);
			TestTrue(TEXT("back at 100"), State->GiveSlotStock(*BobAgain, Starting, 100.));
			AddInfo(FString::Printf(TEXT("back: xp %d, death %d, cooldowns %d"), BobAgain->GetHeroPrivate().XP,
				BobAgain->GetHeroPrivate().DeathStep, BobAgain->GetHeroPrivate().CooldownEndSteps.Num()));
			TestEqual(TEXT("back: his XP in his new player state at once"), BobAgain->GetHeroPrivate().XP, 12);
			TestTrue(TEXT("back: his whole private sheet is the subsystem's"), BobAgain->GetHeroPrivate() == Heroes->GetSheets(1).Private);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

namespace UE::HeroesOfLiberty::Tests::Disconnect
{
	/** What the game logs while a test runs (the journal of the research: « Technology: slot N researched X »). */
	class FCaptureLog : public FOutputDevice
	{
	public:
		TArray<FString> Lines;
		virtual void Serialize(const TCHAR* Text, ELogVerbosity::Type, const FName&) override { Lines.Add(Text); }
		int32 CountContaining(const FString& A, const FString& B) const
		{
			int32 Count = 0;
			for (const FString& Line : Lines)
			{
				Count += Line.Contains(A) && Line.Contains(B) ? 1 : 0;
			}
			return Count;
		}
	};
}

// D719 (T0): what the absent gains is kept for him, the technologies as the stock. Bob has the Plough I, queues the Plough II at
// his farm and leaves; the research ends while he is away (the real production step); the AI's reader still sees his Plough I
// (what a technology that asks for it needs); back in the window he finds the Plough I and the Plough II, and their effect.
// Alice, who never leaves, is as before; a research ending for a slot that exists nowhere is not logged as done.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectTechnologiesTest, "HeroesOfLiberty.Match.DisconnectTechnologies",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectTechnologiesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	const FName PloughI(TEXT("PloughI"));
	const FName PloughII(TEXT("PloughII"));
	if (!TestNotNull(TEXT("the technology table is loaded (Plough II)"), UHOLTechnologySettings::FindTechnology(PloughII)))
	{
		return false;
	}
	IConsoleVariable* BuildTime = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Prod.BuildTime"));
	if (!TestNotNull(TEXT("hol.Prod.BuildTime"), BuildTime))
	{
		return false;
	}
	BuildTime->Set(0.1f, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* BobFarm = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* AliceFarm = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* NobodyFarm = World->SpawnActor<AHOLBuilding>(FVector(-5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("production"), Production) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("player states"), Alice) && TestNotNull(TEXT("player states"), Bob) && TestNotNull(TEXT("farms"), BobFarm)
		&& TestNotNull(TEXT("farms"), AliceFarm) && TestNotNull(TEXT("farms"), NobodyFarm))
	{
		State->InitVictory(MakeNamedConfig());
		Alice->SetMatchSlot(0, 0);
		Bob->SetMatchSlot(1, 1);
		State->AddPlayerState(Alice);
		State->AddPlayerState(Bob);
		const FHOLResources Starting(2000, 2000, 500, 500);
		State->GiveSlotStock(*Alice, Starting);
		State->GiveSlotStock(*Bob, Starting);
		BobFarm->Setup(HOLBuildingTypeNames::Farm, 1, 1);
		AliceFarm->Setup(HOLBuildingTypeNames::Farm, 0, 0);
		// Slot 3 is nobody's (closed, no player state, not absent): its farm researches into the void.
		NobodyFarm->Setup(HOLBuildingTypeNames::Farm, 3, 3);
		Production->OnWorldBeginPlay(*World);

		const auto Queue = [&](AHOLBuilding* Farm, AHOLPlayerState* Payer, const int32 Slot, const FName Technology)
		{
			const FHOLProductionCost Cost = Production->GetCost(Technology, Slot);
			FHOLResources Stock = Payer ? Payer->GetResources() : Starting;
			const bool bQueued = HOLProduction::TryEnqueue(Farm->GetMutableProductionQueue(), Technology, Cost, 5, Stock) == EHOLProductionRefusal::None;
			if (Payer)
			{
				Payer->SetResources(Stock);
			}
			return bQueued;
		};
		int64 StepCount = 0;
		const auto RunSteps = [&](const int32 Steps)
		{
			for (int32 Each = 0; Each < Steps; ++Each)
			{
				Simulation->GetOnStepStarted().Broadcast(++StepCount);
			}
		};

		// Bob has the Plough I and queues the Plough II; Alice and slot 3 queue the Plough I.
		TestTrue(TEXT("Bob has the Plough I"), Bob->AddTechnology(PloughI));
		TestTrue(TEXT("Bob queues the Plough II"), Queue(BobFarm, Bob, 1, PloughII));
		TestTrue(TEXT("Alice queues the Plough I"), Queue(AliceFarm, Alice, 0, PloughI));
		TestTrue(TEXT("slot 3 queues the Plough I"), Queue(NobodyFarm, nullptr, 3, PloughI));

		// Bob leaves; his player state goes, as after a Logout.
		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10., 180.));
		Bob->Destroy();

		// What a technology that asks for the Plough I reads (the AI's reader): his Plough I is still his.
		TArray<FName> Done;
		UHOLAITechnologyLibrary::ReadAITechnologiesDone(World, 1, Done);
		AddInfo(FString::Printf(TEXT("while he is away, his technologies read: %d"), Done.Num()));
		TestTrue(TEXT("away: his Plough I is still read as done (the Plough II stays offered)"), Done.Contains(PloughI));

		// The researches end while he is away (the real production step).
		FCaptureLog Log;
		GLog->AddOutputDevice(&Log);
		RunSteps(8);
		GLog->FlushThreadedLogs();
		GLog->RemoveOutputDevice(&Log);

		TestTrue(TEXT("Alice's research landed in her player state, as ever"), Alice->GetTechnologies().Has(PloughI));
		TestEqual(TEXT("Alice's research logged once"), Log.CountContaining(TEXT("researched PloughI"), TEXT("slot 0")), 1);
		UHOLAITechnologyLibrary::ReadAITechnologiesDone(World, 1, Done);
		TestTrue(TEXT("away: the research ended while he was gone is his"), Done.Contains(PloughII));
		TestEqual(TEXT("slot 3's research, going nowhere, is not logged as researched"), Log.CountContaining(TEXT("researched PloughI"), TEXT("slot 3")), 0);

		// Back in the window with a new player state: everything is his.
		AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("new player state"), BobAgain))
		{
			TestFalse(TEXT("a new player state starts without technologies"), BobAgain->GetTechnologies().Has(PloughI));
			BobAgain->SetMatchSlot(1, 1);
			State->AddPlayerState(BobAgain);
			TestTrue(TEXT("back at 100"), State->GiveSlotStock(*BobAgain, Starting, 100.));
			AddInfo(FString::Printf(TEXT("back: %d technologies, food gathering +%d %%"), BobAgain->GetTechnologies().Done.Num(),
				BobAgain->GetTechnologyBonusPercent(HOLTechnology::GetGatherStat(EHOLResource::Food))));
			TestTrue(TEXT("back: his Plough I"), BobAgain->GetTechnologies().Has(PloughI));
			TestTrue(TEXT("back: his Plough II, ended while he was away"), BobAgain->GetTechnologies().Has(PloughII));
			TestTrue(TEXT("back: their effect (food gathering) counts at once"), BobAgain->GetTechnologyBonusPercent(HOLTechnology::GetGatherStat(EHOLResource::Food)) > 0);
			TestFalse(TEXT("back: Alice's technologies are not his"), BobAgain->GetTechnologies().Done.Num() > 2);
		}
		// A slot never absent keeps what it had (nothing gained, nothing lost).
		TestEqual(TEXT("Alice never left: her technologies are hers"), Alice->GetTechnologies().Done.Num(), 1);
	}

	BuildTime->Set(0.f, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D717 (T2): the AI holds the seat of the player who left, from the Logout to his return. Bob (slot 1) leaves: the brain holds the seat
// (SetHeld by the game state), reads it as a Normal level (a human seat has none), makes it think (it has no player state any more);
// the thought runs. Alice (present) is never held. Bob back: released, no longer thought for.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectHeldTest, "HeroesOfLiberty.Match.DisconnectHeld", UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectHeldTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(World);
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("the script brain"), Brain) && TestNotNull(TEXT("player states"), Alice)
		&& TestNotNull(TEXT("player states"), Bob))
	{
		State->InitVictory(MakeNamedConfig());
		Alice->SetMatchSlot(0, 0);
		Bob->SetMatchSlot(1, 1);
		State->AddPlayerState(Alice);
		State->AddPlayerState(Bob);
		const FHOLResources Starting(2000, 2000, 500, 500);
		State->GiveSlotStock(*Alice, Starting);
		State->GiveSlotStock(*Bob, Starting);
		const int32 Normal = static_cast<int32>(EHOLAIDifficulty::Normal);

		TestFalse(TEXT("present: Bob's seat is not held"), Brain->IsHeld(1));
		TestEqual(TEXT("present: a human seat has no level"), UHOLAIToolsLibrary::GetSlotLevel(World, 1), -1);
		TestFalse(TEXT("present: nobody thinks for him"), Brain->GetThinkingSlots().Contains(1));

		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10., 180.));
		Bob->Destroy();
		TestTrue(TEXT("away: the AI holds his seat"), Brain->IsHeld(1));
		TestFalse(TEXT("away: Alice's seat is not held"), Brain->IsHeld(0));
		TestEqual(TEXT("away: a held seat is read as Normal"), UHOLAIToolsLibrary::GetSlotLevel(World, 1), Normal);
		TestTrue(TEXT("away: the brain thinks for his seat"), Brain->GetThinkingSlots().Contains(1));
		TestFalse(TEXT("away: and not for Alice's"), Brain->GetThinkingSlots().Contains(0));
		Brain->ThinkNow(1);

		AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("new player state"), BobAgain))
		{
			BobAgain->SetMatchSlot(1, 1);
			State->AddPlayerState(BobAgain);
			TestTrue(TEXT("back at 100"), State->GiveSlotStock(*BobAgain, Starting, 100.));
			TestFalse(TEXT("back: the AI lets the seat go"), Brain->IsHeld(1));
			TestFalse(TEXT("back: the brain no longer thinks for it"), Brain->GetThinkingSlots().Contains(1));
			TestEqual(TEXT("back: a human seat has no level again"), UHOLAIToolsLibrary::GetSlotLevel(World, 1), -1);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D717 (T2): the AI that holds the seat plays with HIS stock. Bob (slot 1, a Peasant-making centre) leaves; the real chain
// (KeepAbsentSlot, as the game mode's Logout) gives the seat to a player state of the AI (his name, his stock, his technologies),
// so what the brain orders passes the same queue and is paid on that stock (before: "ignored: no player state", nothing
// happened). A Peasant ordered by the AI for slot 1 enters his centre's queue and costs food; what a Peasant unloads is added
// there too (SetSlotStock/AddSlotStock). A second connection of his name does not find his seat "taken" (PostLogin's slots in use
// leave the held seat out). Back (GiveSlotStock): the new player state has the stock as the AI left it, the spent food spent,
// his technologies (without raising twice what was raised), and only one player state is left for the seat; Alice is untouched.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectHeldStockTest, "HeroesOfLiberty.Match.DisconnectHeldStock", UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectHeldStockTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* BobCentre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* AliceCentre = World->SpawnActor<AHOLBuilding>(FVector(-20000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("production"), Production) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("player states"), Alice) && TestNotNull(TEXT("player states"), Bob) && TestNotNull(TEXT("centres"), BobCentre)
		&& TestNotNull(TEXT("centres"), AliceCentre))
	{
		State->InitVictory(MakeNamedConfig());
		Alice->SetMatchSlot(0, 0);
		Bob->SetMatchSlot(1, 1);
		State->AddPlayerState(Alice);
		State->AddPlayerState(Bob);
		const FHOLResources Starting(2000, 2000, 500, 500);
		State->GiveSlotStock(*Alice, Starting);
		State->GiveSlotStock(*Bob, Starting);
		BobCentre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		AliceCentre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Production->OnWorldBeginPlay(*World);
		const FName Peasant(TEXT("Peasant"));
		const FName PloughI(TEXT("PloughI"));
		Bob->AddTechnology(PloughI);

		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10., 180.));
		Bob->Destroy();

		// The seat has a player state of the AI: his name, his slot, his stock and technologies; one only.
		const AHOLPlayerState* Holder = AHOLGameState::FindHeldBot(*World, 1);
		if (TestNotNull(TEXT("away: a player state of the AI holds the seat"), Holder))
		{
			TestTrue(TEXT("it is an AI's"), Holder->IsABot());
			TestEqual(TEXT("it keeps his name"), Holder->GetPlayerName(), FString(TEXT("Bob")));
			TestEqual(TEXT("it has his stock"), Holder->GetResources().ToString(), Starting.ToString());
			TestTrue(TEXT("it has his Plough I"), Holder->GetTechnologies().Has(PloughI));
		}
		TestNull(TEXT("Alice's seat has none"), AHOLGameState::FindHeldBot(*World, 0));
		// A second connection of his name: the seat is not "in use" by that player state of the AI (else he joins as a spectator).
		TestFalse(TEXT("away: PostLogin does not see his seat as taken"), State->GetSlotsInUse(nullptr).Contains(1));
		TestTrue(TEXT("... Alice's is"), State->GetSlotsInUse(nullptr).Contains(0));

		// What the brain orders for the seat is paid on his stock (the real order path, as the script's).
		FHOLResources Before;
		TestTrue(TEXT("away: his stock is read"), AHOLGameState::GetSlotStock(*World, 1, Before));
		TestTrue(TEXT("the AI orders a Peasant at his centre"), UHOLAIOrderLibrary::IssueAIProduceOrder(World, 1, FVector::ZeroVector, Peasant));
		Simulation->GetOnStepStarted().Broadcast(1);
		TestEqual(TEXT("next step: the Peasant is in his centre's queue"), BobCentre->GetProductionQueue().Num(), 1);
		FHOLResources After;
		AHOLGameState::GetSlotStock(*World, 1, After);
		TestTrue(TEXT("paid: his food went down"), After.Food < Before.Food);
		// A deposit (what a Peasant unloads) goes where the orders pay.
		FHOLResources Stock;
		TestTrue(TEXT("a deposit is added to his stock"), AHOLGameState::AddSlotStock(*World, 1, FHOLResources(0, 100, 0, 0), Stock));
		TestEqual(TEXT("... and the orders pay on it (wood +100)"), Stock.Wood, After.Wood + 100);
		FHOLResources Read;
		AHOLGameState::GetSlotStock(*World, 1, Read);
		TestEqual(TEXT("... read back"), Read.ToString(), Stock.ToString());
		TestEqual(TEXT("Alice's stock is untouched"), Alice->GetResources().ToString(), Starting.ToString());

		// Back with a new player state: what the AI left, the AI gone.
		AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("new player state"), BobAgain))
		{
			BobAgain->SetMatchSlot(1, 1);
			State->AddPlayerState(BobAgain);
			TestTrue(TEXT("back at 100"), State->GiveSlotStock(*BobAgain, Starting, 100.));
			TestEqual(TEXT("back: the stock as the AI left it (spent, deposited)"), BobAgain->GetResources().ToString(), Stock.ToString());
			TestTrue(TEXT("back: his Plough I"), BobAgain->GetTechnologies().Has(PloughI));
			TestNull(TEXT("back: no player state of the AI any more"), AHOLGameState::FindHeldBot(*World, 1));
			int32 Seats = 0;
			for (const APlayerState* Each : State->PlayerArray)
			{
				const AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				Seats += HOLEach && HOLEach->GetPlayerIndex() == 1 ? 1 : 0;
			}
			TestEqual(TEXT("back: one player state for his seat"), Seats, 1);
			TestEqual(TEXT("back: his order stays in the centre's queue"), BobCentre->GetProductionQueue().Num(), 1);
		}
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D717 (T4): a name that comes back while its OLD connection is still alive (he restarted in less than the engine's timeout) takes his
// seat back; he is not a spectator. The real chain of the game mode (FindSlotForLogin, RetireHolder, then PostLogin's SetMatchSlot and
// GiveSlotStock): the new player state has the stock and technologies the old one had (not the starting stock), his order stays in the
// centre's queue, the old player state is no longer a holder, and its LATE Logout (KeepAbsentSlot) puts nobody "away" under the new
// player (nor a second duplicate still holding the seat). An unknown name watches; Alice's seat is untouched.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectReplaceTest, "HeroesOfLiberty.Match.DisconnectReplace", UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectReplaceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobOld = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobNew = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* BobCentre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* AliceCentre = World->SpawnActor<AHOLBuilding>(FVector(-20000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("production"), Production) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("player states"), Alice) && TestNotNull(TEXT("player states"), BobOld) && TestNotNull(TEXT("player states"), BobNew)
		&& TestNotNull(TEXT("centres"), BobCentre) && TestNotNull(TEXT("centres"), AliceCentre))
	{
		const FHOLMatchConfig Config = MakeNamedConfig();
		State->InitVictory(Config);
		Alice->SetMatchSlot(0, 0);
		BobOld->SetMatchSlot(1, 1);
		State->AddPlayerState(Alice);
		State->AddPlayerState(BobOld);
		const FHOLResources Starting(2000, 2000, 500, 500);
		State->GiveSlotStock(*Alice, Starting);
		State->GiveSlotStock(*BobOld, Starting);
		BobCentre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);
		AliceCentre->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Production->OnWorldBeginPlay(*World);
		const FName Peasant(TEXT("Peasant"));
		const FName PloughI(TEXT("PloughI"));
		BobOld->AddTechnology(PloughI);
		// His order in progress at the centre (paid on his stock), before he "restarts".
		TestTrue(TEXT("Bob orders a Peasant"), UHOLAIOrderLibrary::IssueAIProduceOrder(World, 1, FVector::ZeroVector, Peasant));
		Simulation->GetOnStepStarted().Broadcast(1);
		TestEqual(TEXT("the Peasant is in his centre's queue"), BobCentre->GetProductionQueue().Num(), 1);
		const FHOLResources BobStock = BobOld->GetResources();
		TestTrue(TEXT("(his stock is no longer the starting one)"), BobStock.ToString() != Starting.ToString());

		// The new connection of his name, no slot yet (PostLogin's state), while the old one is still in the player array.
		State->AddPlayerState(BobNew);
		AHOLPlayerState* Replaced = nullptr;
		const int32 Slot = State->FindSlotForLogin(Config, TEXT("Bob"), *BobNew, Replaced);
		TestEqual(TEXT("his name finds his seat, not a spectator"), Slot, 1);
		TestTrue(TEXT("the old connection's player state is the one to replace"), Replaced == BobOld);
		AHOLPlayerState* ReplacedUnknown = nullptr;
		TestEqual(TEXT("an unknown name watches"), State->FindSlotForLogin(Config, TEXT("Zed"), *BobNew, ReplacedUnknown), INDEX_NONE);
		TestNull(TEXT("... and replaces nobody"), ReplacedUnknown);

		// PostLogin's order: the old one handed over, then the slot and the stock to the new one.
		if (Slot == 1 && Replaced == BobOld)
		{
			State->RetireHolder(*BobOld, 100., 180.);
			BobNew->SetMatchSlot(Slot, 1);
			TestTrue(TEXT("he takes his seat back"), State->GiveSlotStock(*BobNew, Starting, 100.));
			TestEqual(TEXT("his stock is the one of his old connection, not the starting one"), BobNew->GetResources().ToString(), BobStock.ToString());
			TestTrue(TEXT("his Plough I"), BobNew->GetTechnologies().Has(PloughI));
			TestEqual(TEXT("his order stays in the centre's queue"), BobCentre->GetProductionQueue().Num(), 1);
			TestTrue(TEXT("the old player state is a spectator now"), BobOld->IsOnlyASpectator());
			TestTrue(TEXT("... without a seat"), BobOld->GetPlayerIndex() != 1);
			TestTrue(TEXT("the new player state alone holds the seat"), AHOLGameState::FindLiveHolder(*World, 1, nullptr) == BobNew);
			TestNull(TEXT("no player state of the AI is left on the seat"), AHOLGameState::FindHeldBot(*World, 1));
			TestFalse(TEXT("his seat is not absent"), State->IsSlotAbsent(1));
			// The old connection closes late: nobody is put away under the new player.
			TestFalse(TEXT("late Logout of the old connection: the seat is not kept"), State->KeepAbsentSlot(*BobOld, 101., 180.));
			TestFalse(TEXT("... the seat stays present"), State->IsSlotAbsent(1));
			TestNull(TEXT("... no AI on it"), AHOLGameState::FindHeldBot(*World, 1));
			TestEqual(TEXT("... his stock is untouched"), BobNew->GetResources().ToString(), BobStock.ToString());
		}

		// A duplicate that was NOT handed over (killed another way): its late Logout is no absence either, the new holder is there.
		AHOLPlayerState* Duplicate = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("duplicate"), Duplicate))
		{
			Duplicate->SetMatchSlot(1, 1);
			State->AddPlayerState(Duplicate);
			TestFalse(TEXT("a duplicate closing late: the seat is not kept"), State->KeepAbsentSlot(*Duplicate, 102., 180.));
			TestFalse(TEXT("... the seat stays present"), State->IsSlotAbsent(1));
		}
		TestEqual(TEXT("Alice's stock is untouched"), Alice->GetResources().ToString(), Starting.ToString());
		TestFalse(TEXT("Alice is not absent"), State->IsSlotAbsent(0));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D720 (T6): the window does not run during a pause; the pause of the player who leaves is lifted (his pause stays counted).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectPauseTest, "HeroesOfLiberty.Match.DisconnectPause",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectPauseTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("Alice"), Alice) && TestNotNull(TEXT("Bob"), Bob))
	{
		FHOLMatchConfig Config1v1 = MakeNamedConfig();
		Config1v1.Slots[2].State = EHOLSlotState::Closed;
		State->InitVictory(Config1v1);
		Alice->SetMatchSlot(0, 0);
		Bob->SetMatchSlot(1, 1);
		const FHOLResources Starting(200, 200, 200, 200);
		State->GiveSlotStock(*Alice, Starting);
		State->GiveSlotStock(*Bob, Starting);

		TestEqual(TEXT("no pause: the window clock is the time"), State->GetWindowClock(50.), 50.);
		// Alice pauses at 100 and resumes at 160: the clock stood still for 60 s; Bob's departure lifts nobody else's pause.
		TestTrue(TEXT("Alice pauses"), State->RequestPause(*Alice, 100.));
		TestEqual(TEXT("paused: the window clock stands still"), State->GetWindowClock(130.), 100.);
		TestFalse(TEXT("Bob's departure lifts only his own pause"), State->LiftPauseOf(1, 130.));
		TestTrue(TEXT("Alice's pause stays"), State->IsMatchPaused());
		TestTrue(TEXT("Alice resumes"), State->RequestResume(*Alice, 160.));
		TestEqual(TEXT("resumed: 60 s lost to the window"), State->GetWindowClock(200.), 140.);

		// Bob pauses at 300 and leaves at 305: the match runs again for the others, the pause stays counted.
		TestTrue(TEXT("Bob pauses"), State->RequestPause(*Bob, 300.));
		const int32 PausesAtLeaving = State->GetPausesLeft();
		TestTrue(TEXT("Bob leaves: his pause is lifted"), State->LiftPauseOf(1, 305.));
		TestFalse(TEXT("the match runs again"), State->IsMatchPaused());
		TestEqual(TEXT("his pause stays counted"), State->GetPausesLeft(), PausesAtLeaving);
		TestEqual(TEXT("5 s more lost to the window"), State->GetWindowClock(310.), 310. - 60. - 5.);
		TestFalse(TEXT("nothing to lift twice"), State->LiftPauseOf(1, 306.));

		// A window of 10 s from 310 (clock 245), with a pause of Alice's of 5 s inside it, ends at 325 of the time (10 s + 5 s).
		TestTrue(TEXT("Bob is away"), State->KeepAbsentSlot(*Bob, State->GetWindowClock(310.), 10.));
		Bob->Destroy();
		TestTrue(TEXT("Alice pauses"), State->RequestPause(*Alice, 312.));
		TestTrue(TEXT("Alice resumes"), State->RequestResume(*Alice, 317.));
		TestEqual(TEXT("14.9 s of time: the window is not over (5 s were paused)"), State->DefeatExpiredAbsences(State->GetWindowClock(324.9), 10.).Num(), 0);
		TestEqual(TEXT("15 s of time: the window is over"), State->DefeatExpiredAbsences(State->GetWindowClock(325.), 10.), TArray<int32>{ 1 });
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D720 (T6, cosmetic): the banner of the clients counts down on the window's clock (the time minus the pauses), as the server's window does.
// A pause of 60 s inside Bob's window of 180 s: seen at the server time 200 the banner reads 0:50 (clock 140, end 190), not 0:00; during a
// pause the figure stands still; the server's figure of the pauses is replicated (a client cannot guess it).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectBannerClockTest, "HeroesOfLiberty.Match.DisconnectBannerClock", UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectBannerClockTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	const FProperty* PausedSeconds = AHOLGameState::StaticClass()->FindPropertyByName(TEXT("WindowPausedSeconds"));
	TestTrue(TEXT("the seconds of the pauses are replicated (the clients' banner reads them)"), PausedSeconds && PausedSeconds->HasAnyPropertyFlags(CPF_Net));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("Alice"), Alice) && TestNotNull(TEXT("Bob"), Bob))
	{
		FHOLMatchConfig Config1v1 = MakeNamedConfig();
		Config1v1.Slots[2].State = EHOLSlotState::Closed;
		State->InitVictory(Config1v1);
		Alice->SetMatchSlot(0, 0);
		Bob->SetMatchSlot(1, 1);
		const FHOLResources Starting(200, 200, 200, 200);
		State->GiveSlotStock(*Alice, Starting);
		State->GiveSlotStock(*Bob, Starting);
		TestTrue(TEXT("Bob leaves at the clock 10 (his window ends at 190)"), State->KeepAbsentSlot(*Bob, State->GetWindowClock(10.), 180.));
		Bob->Destroy();

		auto BannerAt = [State](const double Time)
		{
			const TArray<FText> Lines = State->GetAbsenceBannerLines(Time);
			return Lines.Num() == 1 ? Lines[0].ToString() : FString::Printf(TEXT("%d lines"), Lines.Num());
		};
		TestEqual(TEXT("no pause, time 100: 1:30"), BannerAt(100.), FString(TEXT("Bob : connexion perdue — 1:30")));
		TestTrue(TEXT("Alice pauses at 100"), State->RequestPause(*Alice, 100.));
		TestEqual(TEXT("paused, time 130: the figure stands still (1:30)"), BannerAt(130.), FString(TEXT("Bob : connexion perdue — 1:30")));
		TestTrue(TEXT("Alice resumes at 160"), State->RequestResume(*Alice, 160.));
		TestEqual(TEXT("time 200, 60 s paused: 0:50 and not 0:00"), BannerAt(200.), FString(TEXT("Bob : connexion perdue — 0:50")));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// T9: hol.Net.FakeAbsent Back <Slot>, the return without a second client, on the real chain (the console command, as the scenarios run it):
// Leave 1 keeps the seat (the AI holds it), the AI's stock grows, Back 1 gives the seat to a returning player (a player state of the
// name, not the AI's) with the stock as the AI left it, the AI lets go, the banner says "de retour" and the seat is no longer the AI's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectFakeBackTest, "HeroesOfLiberty.Match.DisconnectFakeBack", UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectFakeBackTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("Alice"), Alice))
	{
		State->InitVictory(MakeNamedConfig());
		Alice->SetMatchSlot(0, 0);
		State->AddPlayerState(Alice);
		State->GiveSlotStock(*Alice, FHOLResources(200, 200, 200, 200));

		IConsoleManager::Get().ProcessUserConsoleInput(TEXT("hol.Net.FakeAbsent Leave 1"), *GLog, World);
		TestNotNull(TEXT("Leave 1: the AI holds Bob's seat"), AHOLGameState::FindHeldBot(*World, 1));
		TestTrue(TEXT("Leave 1: his banner is up"), State->GetAbsentNotices().ContainsByPredicate([](const FHOLAbsentNotice& Notice) { return Notice.Slot == 1 && Notice.BackAt <= 0.; }));
		FHOLResources Stock;
		TestTrue(TEXT("the AI's stock grows (a deposit)"), AHOLGameState::AddSlotStock(*World, 1, FHOLResources(0, 100, 0, 0), Stock));

		IConsoleManager::Get().ProcessUserConsoleInput(TEXT("hol.Net.FakeAbsent Back 1"), *GLog, World);
		TestNull(TEXT("Back 1: the AI's player state is gone"), AHOLGameState::FindHeldBot(*World, 1));
		TestTrue(TEXT("Back 1: the seat is a player's again"), State->GetSlotsInUse(nullptr).Contains(1));
		FHOLResources Read;
		TestTrue(TEXT("Back 1: his stock is read"), AHOLGameState::GetSlotStock(*World, 1, Read));
		TestEqual(TEXT("Back 1: the stock as the AI left it (the deposit is there)"), Read.ToString(), Stock.ToString());
		// The "de retour" line itself (BackAt is the server time of the return, 0 meaning "not back") cannot be told apart at the time 0 of this
		// bare world: the scenario disconnect_look checks it in a running game.
		TestEqual(TEXT("Back 1: Alice's stock is untouched"), Alice->GetResources().ToString(), FHOLResources(200, 200, 200, 200).ToString());
		int32 Seats = 0;
		for (const APlayerState* Each : State->PlayerArray)
		{
			const AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
			Seats += HOLEach && HOLEach->GetPlayerIndex() == 1 ? 1 : 0;
		}
		TestEqual(TEXT("Back 1: one player state for his seat"), Seats, 1);
		AHOLPlayerState* Returned = nullptr;
		for (APlayerState* Each : State->PlayerArray)
		{
			AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
			Returned = HOLEach && HOLEach->GetPlayerIndex() == 1 ? HOLEach : Returned;
		}
		TestTrue(TEXT("Back 1: that player state is Bob's, not an AI's"), Returned && !Returned->IsABot() && Returned->GetPlayerName() == TEXT("Bob"));
		// Nobody away any more: a second Back has nothing to give back.
		const int32 Notices = State->GetAbsentNotices().Num();
		IConsoleManager::Get().ProcessUserConsoleInput(TEXT("hol.Net.FakeAbsent Back 1"), *GLog, World);
		TestEqual(TEXT("a second Back changes nothing"), State->GetAbsentNotices().Num(), Notices);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D718 (T5): the end of the window by the rule of T1 (GetWindowEnd). Bob (slot 1) shares his camp with an AI seat (slot 2): at the end of
// his window his seat is NOT defeated; the AI goes from the "hold" mode to the full brain at the Normal level, the banner goes out,
// the seat stays his and he takes it back AFTER the window, with the stock the AI left. Contrast, a real 1v1 (Bob alone in his camp, two
// camps in all): the end of the window is his defeat (Disconnected), as before.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectLateReturnTest, "HeroesOfLiberty.Match.DisconnectLateReturn", UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectLateReturnTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;
	const int32 Normal = static_cast<int32>(EHOLAIDifficulty::Normal);
	const FHOLResources Starting(2000, 2000, 500, 500);

	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		AHOLGameState* State = World->SpawnActor<AHOLGameState>();
		UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(World);
		AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
		AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("the script brain"), Brain) && TestNotNull(TEXT("player states"), Alice)
			&& TestNotNull(TEXT("player states"), Bob))
		{
			State->InitVictory(MakeNamedConfig());
			Alice->SetMatchSlot(0, 0);
			Bob->SetMatchSlot(1, 1);
			State->AddPlayerState(Alice);
			State->AddPlayerState(Bob);
			State->GiveSlotStock(*Alice, Starting);
			State->GiveSlotStock(*Bob, Starting);
			TestTrue(TEXT("Bob leaves at 10"), State->KeepAbsentSlot(*Bob, 10., 180.));
			Bob->Destroy();
			TestTrue(TEXT("in the window: the seat is held (hold mode)"), Brain->IsHeld(1));
			TestTrue(TEXT("in the window: his banner is up"), State->GetAbsentNotices().ContainsByPredicate([](const FHOLAbsentNotice& Notice) { return Notice.Slot == 1; }));

			// The window runs out (10 + 180): the AI keeps the seat.
			const TArray<int32> Defeated = State->DefeatExpiredAbsences(191., 180.);
			TestEqual(TEXT("end of the window: nobody defeated (his camp has an AI seat)"), Defeated.Num(), 0);
			TestFalse(TEXT("... his seat is not defeated"), State->IsSlotDefeated(1));
			TestNotNull(TEXT("... the AI's player state still holds it"), AHOLGameState::FindHeldBot(*World, 1));
			TestFalse(TEXT("... the hold mode is over"), Brain->IsHeld(1));
			TestTrue(TEXT("... the brain thinks for it, the full brain"), Brain->GetThinkingSlots().Contains(1));
			TestEqual(TEXT("... at the Normal level"), UHOLAIToolsLibrary::GetSlotLevel(World, 1), Normal);
			TestFalse(TEXT("... his banner is out"), State->GetAbsentNotices().ContainsByPredicate([](const FHOLAbsentNotice& Notice) { return Notice.Slot == 1; }));
			State->DefeatExpiredAbsences(300., 180.);
			TestFalse(TEXT("a second pass later: still not defeated"), State->IsSlotDefeated(1));
			Brain->ThinkNow(1);

			// He comes back AFTER the window and takes the seat.
			AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
			if (TestNotNull(TEXT("new player state"), BobAgain))
			{
				BobAgain->SetMatchSlot(1, 1);
				State->AddPlayerState(BobAgain);
				TestTrue(TEXT("late return at 500"), State->GiveSlotStock(*BobAgain, Starting, 500.));
				TestEqual(TEXT("... his stock"), BobAgain->GetResources().ToString(), Starting.ToString());
				TestNull(TEXT("... the AI's player state gone"), AHOLGameState::FindHeldBot(*World, 1));
				TestFalse(TEXT("... the brain no longer thinks for it"), Brain->GetThinkingSlots().Contains(1));
				TestEqual(TEXT("... a human seat has no level again"), UHOLAIToolsLibrary::GetSlotLevel(World, 1), -1);
				TestFalse(TEXT("... the seat is no longer absent"), State->IsSlotAbsent(1));
				TestFalse(TEXT("... and never defeated"), State->IsSlotDefeated(1));
			}
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	}

	// A 1v1: Bob alone in his camp (the AI seat closed): his window's end is his defeat.
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		AHOLGameState* State = World->SpawnActor<AHOLGameState>();
		AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
		AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("player states"), Alice) && TestNotNull(TEXT("player states"), Bob))
		{
			FHOLMatchConfig Config = MakeNamedConfig();
			Config.Slots[2].State = EHOLSlotState::Closed;
			State->InitVictory(Config);
			Alice->SetMatchSlot(0, 0);
			Bob->SetMatchSlot(1, 1);
			State->AddPlayerState(Alice);
			State->AddPlayerState(Bob);
			State->GiveSlotStock(*Alice, Starting);
			State->GiveSlotStock(*Bob, Starting);
			TestTrue(TEXT("1v1: Bob leaves at 10"), State->KeepAbsentSlot(*Bob, 10., 180.));
			Bob->Destroy();
			const TArray<int32> Defeated = State->DefeatExpiredAbsences(191., 180.);
			TestTrue(TEXT("1v1: the end of the window is his defeat"), Defeated.Contains(1) && State->IsSlotDefeated(1));
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	}
	return true;
}

// D718 (T1): the end of the window by the camps of the launch (D357). Alone in his camp with only two camps in all (the 1v1,
// the 1 against 2 too): defeat; otherwise the AI keeps the seat (teams, free for all of 3 and more, a camp with an AI seat
// beside him). The Diplomacy mode has every slot alone in its camp (GetCamp). The seat states and the return.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectWindowEndTest, "HeroesOfLiberty.Match.DisconnectWindowEnd",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectWindowEndTest::RunTest(const FString& Parameters)
{
	using EEnd = FHOLDisconnectRules::EWindowEnd;
	// Each entry: the team of a human slot (0 = no team, free for all); a negative one is an AI of the team -value.
	const auto Make = [](const TArray<int32>& Seats, const EHOLMatchMode Mode = EHOLMatchMode::Standard)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (FHOLMatchSlot& Slot : Config.Slots)
		{
			Slot.State = EHOLSlotState::Closed;
			Slot.Team = 0;
		}
		for (int32 Index = 0; Index < Seats.Num(); ++Index)
		{
			Config.Slots[Index].State = Seats[Index] < 0 ? EHOLSlotState::AI : EHOLSlotState::Human;
			Config.Slots[Index].Team = FMath::Abs(Seats[Index]);
			Config.Slots[Index].PlayerName = Seats[Index] < 0 ? FString() : FString::Printf(TEXT("P%d"), Index);
		}
		Config.Mode = Mode;
		return Config;
	};
	const auto End = [&Make](const TArray<int32>& Seats, const int32 Slot, const EHOLMatchMode Mode = EHOLMatchMode::Standard)
	{
		return FHOLDisconnectRules::GetWindowEnd(Make(Seats, Mode), Slot);
	};

	TestTrue(TEXT("1v1: defeat"), End({ 1, 2 }, 0) == EEnd::Defeat && End({ 1, 2 }, 1) == EEnd::Defeat);
	TestTrue(TEXT("1v1 free for all (no teams): defeat"), End({ 0, 0 }, 0) == EEnd::Defeat);
	TestTrue(TEXT("2v2: the AI keeps the seat"), End({ 1, 1, 2, 2 }, 0) == EEnd::AIHolds && End({ 1, 1, 2, 2 }, 3) == EEnd::AIHolds);
	TestTrue(TEXT("free for all of 3: the AI keeps the seat"), End({ 0, 0, 0 }, 1) == EEnd::AIHolds);
	TestTrue(TEXT("free for all of 8: the AI keeps the seat"), End({ 0, 0, 0, 0, 0, 0, 0, 0 }, 7) == EEnd::AIHolds);
	TestTrue(TEXT("1 against 2: the one alone loses"), End({ 1, 2, 2 }, 0) == EEnd::Defeat);
	TestTrue(TEXT("1 against 2: one of the two: the AI keeps the seat"), End({ 1, 2, 2 }, 1) == EEnd::AIHolds);
	TestTrue(TEXT("1 against 3 and 1 against 1 against 1: three camps or the camp of two: the AI keeps"),
		End({ 1, 2, 2, 2 }, 2) == EEnd::AIHolds && End({ 1, 2, 3 }, 0) == EEnd::AIHolds);
	// A human with an AI seat in his camp (Bob and an AI against Alice, as the other tests' configuration): the camp is not alone.
	TestTrue(TEXT("a human with an AI of his camp against one: the AI keeps"), End({ 1, 2, -2 }, 1) == EEnd::AIHolds);
	TestTrue(TEXT("... the one alone against that camp of two: defeat"), End({ 1, 2, -2 }, 0) == EEnd::Defeat);
	TestTrue(TEXT("a human against an AI alone (two camps): defeat"), End({ 1, -2 }, 0) == EEnd::Defeat);
	// The camps are those of the launch configuration (D357): the lobby's teams do not count in the Diplomacy mode.
	TestTrue(TEXT("Diplomacy, 2v2 teams ignored: four camps, the AI keeps"), End({ 1, 1, 2, 2 }, 0, EHOLMatchMode::Diplomacy) == EEnd::AIHolds);
	TestTrue(TEXT("Diplomacy, two players: two camps alone, defeat"), End({ 1, 1 }, 0, EHOLMatchMode::Diplomacy) == EEnd::Defeat);
	TestTrue(TEXT("a closed or unknown slot holds nothing: defeat"), End({ 1, 2 }, 5) == EEnd::Defeat && End({ 1, 2 }, INDEX_NONE) == EEnd::Defeat);

	// The seat's states: back in the window (the AI holds it for him), back after it while the match lasts, never when defeated.
	using ESeat = FHOLDisconnectRules::ESeatState;
	TestTrue(TEXT("present: his name takes the seat again (a second connection)"), FHOLDisconnectRules::MayReturn(ESeat::Present, false));
	TestTrue(TEXT("away, the AI holds it (window): he may return"), FHOLDisconnectRules::MayReturn(ESeat::AbsentAIHolds, false));
	TestTrue(TEXT("the AI holds it after the window: he may return while the match lasts"), FHOLDisconnectRules::MayReturn(ESeat::AIHolds, false));
	TestFalse(TEXT("... not when the match is over"), FHOLDisconnectRules::MayReturn(ESeat::AIHolds, true));
	TestFalse(TEXT("defeated: he returns as a spectator only"), FHOLDisconnectRules::MayReturn(ESeat::Defeated, false));
	TestFalse(TEXT("a seat defeated is not kept (KeepsSlot)"), FHOLDisconnectRules::KeepsSlot(Make({ 1, 2 }), 0, false, false, true));
	return true;
}

#endif
