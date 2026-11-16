// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLChatSubsystem.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// A4 (ChatInProgress.md; GDD 15, D385, D376, D792, D806): the chat's transport on the server's side (AHOLPlayerController::ServerPostChat
// asks the script board UHOLChatSubsystem::CheckPostNow then ReachesNow for each seat; ClientReceiveChat gives the thread's line by
// AHOLGameState::NoteChatReceived). MIG-P1b (D830, D835): the rule is the script's alone (Script/Chat/HOLChatRules.as, tested by
// Script/Tests/HOLChatRules_Test.as: Test_ChatRouteBySeats, Test_ChatGateRefusals, Test_ChatGateRate, Test_ChatObserverBySeats, ported
// from the former Chat.Route and Chat.Gate); here the bridge to it. The net harness plays the same chain with three clients (quick_check,
// part netsig3: Bob's Allies message read by Alice, not by Carol).
namespace UE::HeroesOfLiberty::Tests::Chat
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLChatServerBoardTest, "HeroesOfLiberty.Chat.ServerBoard", UE::HeroesOfLiberty::Tests::Chat::Flags)

bool FHOLChatServerBoardTest::RunTest(const FString& Parameters)
{
	// The real chain: C++ bridge -> Script/Chat/HOLChatBoardScript.as -> HOLChat::CheckPost and ReachesBySeats. Three seats as netsig3:
	// Alice (0) and Bob (1) mutual allies, Carol (2) apart, in Diplomacy.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLChatSubsystem* Board = UHOLChatSubsystem::Find(World);
	if (TestNotNull(TEXT("the script board of the chat"), Board))
	{
		FHOLChatSeats Seats;
		Seats.Slots = 3;
		Seats.MateByPair.Init(false, 9);
		Seats.MateByPair[0 * 3 + 1] = true;
		Seats.MateByPair[1 * 3 + 0] = true;
		Seats.SpectatorBySlot.Init(false, 3);
		TestTrue(TEXT("Bob's Allies reaches Alice"), Board->ReachesNow(Seats, UHOLChatSubsystem::ChannelAllies, 1, INDEX_NONE, 0));
		TestFalse(TEXT("Bob's Allies does not reach Carol"), Board->ReachesNow(Seats, UHOLChatSubsystem::ChannelAllies, 1, INDEX_NONE, 2));
		TestFalse(TEXT("D806: an observer's private reaches nobody"), Board->ReachesNow(Seats, UHOLChatSubsystem::ChannelPrivate, INDEX_NONE, 2, 2));

		const FHOLChatGateAnswer First = Board->CheckPostNow(Seats, UHOLChatSubsystem::ChannelAll, 1, INDEX_NONE, TEXT("hi"), TArray<int32>(), 1000, 0, 0, 0);
		TestEqual(TEXT("A post passes the gate"), First.Answer, UHOLChatSubsystem::AnswerOk);
		TestEqual(TEXT("And is kept in the sender's rate"), First.RecentMs.Num(), 1);
		constexpr int32 TooLong = 2, TooFast = 3;
		TestEqual(TEXT("201 characters: refused by the script's default (limits 0)"),
			Board->CheckPostNow(Seats, UHOLChatSubsystem::ChannelAll, 1, INDEX_NONE, FString::ChrN(201, TEXT('a')), TArray<int32>(), 1000, 0, 0, 0).Answer, TooLong);
		const FHOLChatGateAnswer Third = Board->CheckPostNow(Seats, UHOLChatSubsystem::ChannelAll, 1, INDEX_NONE, TEXT("hi"), TArray<int32>({900, 950}), 1000, 0, 2, 5000);
		TestEqual(TEXT("The cvar's limit: 2 per window, the third refused"), Third.Answer, TooFast);
		TestEqual(TEXT("A refused post is not kept"), Third.RecentMs.Num(), 2);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLChatThreadLineTest, "HeroesOfLiberty.Chat.ThreadLine", UE::HeroesOfLiberty::Tests::Chat::Flags)

bool FHOLChatThreadLineTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	TArray<int32> Viewers;
	TArray<FString> Lines;
	GameState->OnSignalMessage.AddLambda([&](const int32 ViewerSlot, const FHOLForumMessage& Message)
	{
		Viewers.Add(ViewerSlot);
		Lines.Add(Message.Text.ToString());
	});
	GameState->NoteChatReceived(1, UHOLChatSubsystem::ChannelAllies, TEXT("Hello allies"), 0);
	GameState->NoteChatReceived(0, UHOLChatSubsystem::ChannelAll, TEXT("Hi"), 0);
	if (TestEqual(TEXT("One thread line per received message"), Lines.Num(), 2))
	{
		TestEqual(TEXT("For its viewer alone"), Viewers[0], 0);
		TestTrue(TEXT("The ally named as the players' table names him, then the text"), Lines[0].StartsWith(TEXT("Joueur 2 : ")) && Lines[0].EndsWith(TEXT("Hello allies")));
		TestTrue(TEXT("The Allies channel shown, the All one not"), Lines[0].Len() > FString(TEXT("Joueur 2 : Hello allies")).Len());
		TestEqual(TEXT("The viewer's own All message"), Lines[1], FString(TEXT("Vous : Hi")));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
