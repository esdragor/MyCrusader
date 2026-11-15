// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// SG3 lot 2 (SignalsInProgress.md; GDD 15, D386, D445): a received map signal gives its line in the thread of messages, on the real
// chain the controller calls (ClientReceiveSignal -> AHOLGameState::NoteSignalReceived -> HOLForumMessages::GetSignalMessage ->
// OnSignalMessage, which the HUD adds to its thread): one line per signal, for its viewer alone, the sender named as the players' table
// names him (GetPlayerName: « Vous » for the viewer's own) and the wheel's label of its kind (the GDD's).
namespace UE::HeroesOfLiberty::Tests::SignalThread
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSignalThreadLineTest, "HeroesOfLiberty.Signals.ThreadLine", UE::HeroesOfLiberty::Tests::SignalThread::Flags)

bool FHOLSignalThreadLineTest::RunTest(const FString& Parameters)
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

	constexpr int32 Neutral = 0, Attack = 1, Scout = 5;
	// Slot 1's attack signal reaches its ally at slot 0; slot 0's own neutral signal comes back to it; slot 1's scout signal.
	GameState->NoteSignalReceived(1, Attack, 0);
	GameState->NoteSignalReceived(0, Neutral, 0);
	GameState->NoteSignalReceived(1, Scout, 0);

	if (TestEqual(TEXT("One thread line per received signal"), Lines.Num(), 3))
	{
		TestEqual(TEXT("For its viewer alone"), Viewers[0], 0);
		TestEqual(TEXT("The ally's attack, named as the players' table names him"), Lines[0], FString(TEXT("Joueur 2 : Attaque ici")));
		TestEqual(TEXT("The viewer's own neutral signal"), Lines[1], FString(TEXT("Vous : Signal")));
		TestEqual(TEXT("The ally's scout signal"), Lines[2], FString(TEXT("Joueur 2 : Éclaire ici")));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
