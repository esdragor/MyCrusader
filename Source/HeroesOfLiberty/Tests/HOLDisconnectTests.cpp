// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"

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
		State->InitVictory(MakeNamedConfig());
		const FHOLResources Starting(200, 200, 200, 200);
		Bob->SetMatchSlot(1, 1);
		State->GiveSlotStock(*Bob, Starting);
		TestTrue(TEXT("first login: the stock of the start"), Bob->GetResources() == Starting);
		Bob->SetResources(FHOLResources(0, 300, 0, 0));

		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10.));
		TestTrue(TEXT("slot 1 absent"), State->IsSlotAbsent(1));
		TestEqual(TEXT("no defeat 179 s later"), State->DefeatExpiredAbsences(189., 180.).Num(), 0);
		BobAgain->SetMatchSlot(1, 1);
		State->GiveSlotStock(*BobAgain, Starting);
		TestTrue(TEXT("back in the window: his 300 wood, not the stock of the start"), BobAgain->GetResources() == FHOLResources(0, 300, 0, 0));
		TestFalse(TEXT("slot 1 no longer absent"), State->IsSlotAbsent(1));
		TestEqual(TEXT("back: no defeat after the window"), State->DefeatExpiredAbsences(1000., 180.).Num(), 0);

		TestTrue(TEXT("Bob leaves again"), State->KeepAbsentSlot(*BobAgain, 1000.));
		TestEqual(TEXT("the end of the window: slot 1 defeated"), State->DefeatExpiredAbsences(1180., 180.), TArray<int32>{ 1 });
		TestTrue(TEXT("reason Disconnected"), State->GetDefeatReason(1) == EHOLDefeatReason::Disconnected);
		TestEqual(TEXT("reason's name"), FString(AHOLGameState::GetReasonName(EHOLDefeatReason::Disconnected)), FString(TEXT("Disconnected")));
		TestFalse(TEXT("no longer absent once defeated"), State->IsSlotAbsent(1));
		TestFalse(TEXT("not over: the AI of Bob's camp plays on"), State->IsMatchOver());
		TestFalse(TEXT("a defeated slot is not kept"), State->KeepAbsentSlot(*BobAgain, 1200.));
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

#endif
