// Copyright Epic Games, Inc. All Rights Reserved.

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HUD/HOLGameScreen.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

// D202, Q-alerts-minor default A (AlertsMinorInProgress.md): the population full alert on the real chain of the HUD: the player state's
// population (SetPopulation, the production subsystem's call) -> the game screen's script part (its Refresh, HOLGameScreenScript.as,
// the rule HOLAlertRules::ShouldAlertPopulationFull) -> a line in the C++ screen's thread of messages (hol.HUD.Report's "HUD messages").
// One line when the population becomes full, none while it stays full, none sooner than 30 s after the last one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLPopulationAlertThreadTest, "HeroesOfLiberty.UI.Alerts.PopulationFullThread",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLPopulationAlertThreadTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);
	AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
	Player->SetMatchSlot(0, 0);

	// The screen builds its part in script in NativeOnInitialized, which the engine runs only with a player (UUserWidget::Initialize):
	// a local player and its controller in the game world, as HOLNewGameSitesBoxTests (GALADRIEL-1115h: without them, no part in script).
	ULocalPlayer* LocalPlayer = NewObject<ULocalPlayer>(GEngine);
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	Controller->Player = LocalPlayer;
	LocalPlayer->PlayerController = Controller;
	if (!World->GetPlayerControllerIterator())
	{
		World->AddController(Controller);
	}
	UHOLGameScreen* Screen = NewObject<UHOLGameScreen>(GetTransientPackage());
	Screen->SetPlayerContext(FLocalPlayerContext(LocalPlayer, World));
	TestTrue(TEXT("the screen has a player"), Screen->GetPlayerContext().IsValid());
	Screen->Initialize();
	if (TestNotNull(TEXT("the game screen"), Screen) && TestNotNull(TEXT("its part in script"), Screen->ScriptScreen != nullptr ? Screen : nullptr))
	{
		Screen->BindPlayerState(Player);
		// The line's text as the screen shows it, in the current language (the key the production queue's note uses).
		const FString Full = NSLOCTEXT("HOLGameScreen", "PopulationFull", "Population pleine").ToString();
		// The population, the HUD's real time, a refresh of the script part (as hol.HUD.Report does); the population full lines in the thread.
		const auto Step = [&](const int32 Used, const int32 Cap, const double Seconds)
		{
			Player->SetPopulation(Used, Cap);
			World->RealTimeSeconds = Seconds;
			Screen->DescribeScript();
			const FString Messages = Screen->DescribeMessages();
			int32 Count = 0;
			for (int32 At = Messages.Find(Full); At != INDEX_NONE; At = Messages.Find(Full, ESearchCase::IgnoreCase, ESearchDir::FromStart, At + 1))
			{
				++Count;
			}
			return Count;
		};
		TestEqual(TEXT("room left: no line"), Step(5, 10, 100.0), 0);
		TestEqual(TEXT("the population becomes full: one line"), Step(10, 10, 101.0), 1);
		TestEqual(TEXT("still full: no other line"), Step(10, 10, 102.0), 1);
		Step(9, 10, 103.0);
		TestEqual(TEXT("full again 9 s after the line: quiet (30 s)"), Step(10, 10, 110.0), 1);
		Step(9, 10, 120.0);
		TestEqual(TEXT("full again 34 s after the line: a second line"), Step(10, 10, 135.0), 2);
		Screen->BindPlayerState(nullptr);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
