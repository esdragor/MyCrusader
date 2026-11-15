// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLChatRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// A4 (ChatInProgress.md; GDD 15, D385, D376, D792): the chat's transport on the server's side (AHOLPlayerController::ServerPostChat
// asks HOLChat::CheckPost then HOLChat::Reaches for each seat; ClientReceiveChat gives the thread's line by AHOLGameState::NoteChatReceived).
// The recipients and the answers are computed by HOLChat from the seats; the net harness plays the same chain with three clients
// (quick_check, part netsig3: Bob's Allies message read by Alice, not by Carol).
namespace UE::HeroesOfLiberty::Tests::Chat
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	// Three seats as netsig3: Alice (0) and Bob (1) mutual allies, Carol (2) apart; DefeatedSlot a spectator (D376).
	HOLChat::FSeats ThreeSeats(const bool bTeamGame, const int32 DefeatedSlot = INDEX_NONE)
	{
		HOLChat::FSeats Seats;
		Seats.Slots = 3;
		Seats.bTeamGame = bTeamGame;
		Seats.Mates.Init(false, 9);
		Seats.Mates[0 * 3 + 1] = true;
		Seats.Mates[1 * 3 + 0] = true;
		Seats.Spectator.Init(false, 3);
		if (DefeatedSlot >= 0)
		{
			Seats.Spectator[DefeatedSlot] = true;
		}
		return Seats;
	}

	FString Describe(const TArray<int32>& Slots)
	{
		return FString::JoinBy(Slots, TEXT(","), [](const int32 Slot) { return FString::FromInt(Slot); });
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLChatRouteTest, "HeroesOfLiberty.Chat.Route", UE::HeroesOfLiberty::Tests::Chat::Flags)

bool FHOLChatRouteTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Chat;
	const HOLChat::FSeats Diplomacy = ThreeSeats(false);
	TestEqual(TEXT("Bob's Allies: Bob then Alice, not Carol"), Describe(HOLChat::Recipients(Diplomacy, HOLChat::Allies, 1, INDEX_NONE)), FString(TEXT("1,0")));
	TestEqual(TEXT("Bob's All: every seat"), Describe(HOLChat::Recipients(Diplomacy, HOLChat::All, 1, INDEX_NONE)), FString(TEXT("1,0,2")));
	TestEqual(TEXT("Bob's private to Carol: Carol alone"), Describe(HOLChat::Recipients(Diplomacy, HOLChat::Private, 1, 2)), FString(TEXT("1,2")));
	TestFalse(TEXT("A pure spectator (no seat) never reads an Allies message"), HOLChat::Reaches(Diplomacy, HOLChat::Allies, 1, INDEX_NONE, INDEX_NONE));
	TestTrue(TEXT("D792 1 A: a pure spectator reads the All of a living player"), HOLChat::Reaches(Diplomacy, HOLChat::All, 1, INDEX_NONE, INDEX_NONE));

	// D376: Carol defeated in Diplomacy talks with the spectators alone; D792 1 A: Bob's All still reaches her.
	const HOLChat::FSeats CarolOut = ThreeSeats(false, 2);
	TestEqual(TEXT("Defeated Carol's All: herself alone"), Describe(HOLChat::Recipients(CarolOut, HOLChat::All, 2, INDEX_NONE)), FString(TEXT("2")));
	TestEqual(TEXT("Bob's All reaches the spectator Carol"), Describe(HOLChat::Recipients(CarolOut, HOLChat::All, 1, INDEX_NONE)), FString(TEXT("1,0,2")));

	// Alice defeated: in Diplomacy Bob's Allies does not reach her (D376); in a team game it does (D792 2 A, D357).
	TestEqual(TEXT("Diplomacy: Bob's Allies, not the spectating Alice"), Describe(HOLChat::Recipients(ThreeSeats(false, 0), HOLChat::Allies, 1, INDEX_NONE)), FString(TEXT("1")));
	TestEqual(TEXT("Team game: Bob's Allies reaches his spectating teammate"), Describe(HOLChat::Recipients(ThreeSeats(true, 0), HOLChat::Allies, 1, INDEX_NONE)), FString(TEXT("1,0")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLChatGateTest, "HeroesOfLiberty.Chat.Gate", UE::HeroesOfLiberty::Tests::Chat::Flags)

bool FHOLChatGateTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Chat;
	const HOLChat::FSeats Seats = ThreeSeats(false);
	constexpr int32 MaxLength = 12, PerWindow = 3, Window = 5000;
	auto Post = [&Seats](TArray<int32>& Recent, const int32 Channel, const int32 Target, const FString& Text, const int32 NowMs)
	{
		return HOLChat::CheckPost(Seats, Channel, 1, Target, Text, Recent, NowMs, MaxLength, PerWindow, Window);
	};
	TArray<int32> Recent;
	TestEqual(TEXT("A text of the maximum length passes"), Post(Recent, HOLChat::All, INDEX_NONE, FString::ChrN(MaxLength, TEXT('a')), 0), HOLChat::Ok);
	TestEqual(TEXT("One character more is refused"), Post(Recent, HOLChat::All, INDEX_NONE, FString::ChrN(MaxLength + 1, TEXT('a')), 0), HOLChat::TooLong);
	TestEqual(TEXT("An empty text is refused"), Post(Recent, HOLChat::All, INDEX_NONE, TEXT("   "), 0), HOLChat::Empty);
	TestEqual(TEXT("A private message to oneself is refused"), Post(Recent, HOLChat::Private, 1, TEXT("hi"), 0), HOLChat::BadTarget);
	TestEqual(TEXT("An unknown channel is refused"), Post(Recent, 7, INDEX_NONE, TEXT("hi"), 0), HOLChat::BadChannel);

	// The rate: PerWindow posts within the window, the next refused; once the first is older than the window, one more passes.
	TArray<int32> Answers;
	TArray<int32> Rate;
	for (int32 Index = 0; Index <= PerWindow; ++Index)
	{
		Answers.Add(Post(Rate, HOLChat::Allies, INDEX_NONE, TEXT("go"), Index * 100));
	}
	Answers.Add(Post(Rate, HOLChat::Allies, INDEX_NONE, TEXT("go"), Window));
	TestEqual(TEXT("The rate: ok, ok, ok, too fast, then ok after the window"), Describe(Answers),
		FString::Printf(TEXT("%d,%d,%d,%d,%d"), HOLChat::Ok, HOLChat::Ok, HOLChat::Ok, HOLChat::TooFast, HOLChat::Ok));
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
	GameState->NoteChatReceived(1, HOLChat::Allies, TEXT("Hello allies"), 0);
	GameState->NoteChatReceived(0, HOLChat::All, TEXT("Hi"), 0);
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
