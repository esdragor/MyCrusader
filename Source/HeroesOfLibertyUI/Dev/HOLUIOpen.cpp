#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "Misc/FileHelper.h"
#include "Screens/HOLNewGameScreen.h"
#include "Screens/HOLSettingsScreen.h"
#include "Screens/HOLTitleScreen.h"

/**
 * Opens a menu screen or launches a match by the same path as their buttons, for the scripted end-to-end checks
 * (D246).
 *   hol.UI.Open <NewGame|Settings[:Tab]|Quit> [DelaySeconds=0]   (Tab: 0 Video, 1 Sound, 2 Controls, 3 Language)
 *   hol.UI.NewGame.Launch [ConfigFile] [DelaySeconds=0]   (the file, JSON as for -MatchConfig=, is shown on the screen first)
 */
namespace UE::HeroesOfLiberty::UI
{
	void AfterDelay(const float Delay, TFunction<void()> Action)
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Action = MoveTemp(Action)](float)
		{
			Action();
			return false;
		}), Delay);
	}

	APlayerController* GetPlayer(const TWeakObjectPtr<UWorld>& WeakWorld)
	{
		return WeakWorld.IsValid() ? WeakWorld->GetFirstPlayerController() : nullptr;
	}

	void Open(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Open <NewGame|Settings[:Tab]|Quit> [DelaySeconds=0]"));
			return;
		}
		const FString Screen = Args[0];
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
		AfterDelay(Delay, [Screen, WeakWorld = TWeakObjectPtr<UWorld>(World)]()
		{
			APlayerController* PlayerController = GetPlayer(WeakWorld);
			if (Screen == TEXT("NewGame"))
			{
				UHOLNewGameScreen::Open(PlayerController);
			}
			else if (Screen.StartsWith(TEXT("Settings")))
			{
				// The settings screen already open is reused, so that a script can go through the tabs.
				UHOLSettingsScreen* Settings = UHOLSettingsScreen::Find(PlayerController);
				Settings = Settings ? Settings : UHOLSettingsScreen::Open(PlayerController);
				FString Tab;
				if (Settings && Screen.Split(TEXT(":"), nullptr, &Tab))
				{
					Settings->SelectTab(FCString::Atoi(*Tab));
				}
			}
			else if (Screen == TEXT("Quit"))
			{
				UHOLTitleScreen::AskToQuit(PlayerController);
			}
			else
			{
				UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Open: unknown screen '%s'"), *Screen);
				return;
			}
			UE_LOG(LogHOLUI, Display, TEXT("Screen opened: %s"), *Screen);
		});
	}

	void LaunchNewGame(const TArray<FString>& Args, UWorld* World)
	{
		const FString File = Args.Num() > 0 ? Args[0] : FString();
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
		AfterDelay(Delay, [File, WeakWorld = TWeakObjectPtr<UWorld>(World)]()
		{
			APlayerController* PlayerController = GetPlayer(WeakWorld);
			UHOLNewGameScreen* Screen = UHOLNewGameScreen::Find(PlayerController);
			if (!Screen)
			{
				Screen = UHOLNewGameScreen::Open(PlayerController);
			}
			if (!Screen)
			{
				UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.NewGame.Launch: no menu to open the New game screen in"));
				return;
			}
			if (!File.IsEmpty())
			{
				FString Json;
				FString Error;
				FHOLMatchConfig Config;
				if (!FFileHelper::LoadFileToString(Json, *File) || !FHOLMatchConfig::FromJson(Json, Config, &Error))
				{
					UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.NewGame.Launch: cannot read '%s' %s"), *File, *Error);
					return;
				}
				Screen->SetConfig(Config);
			}
			Screen->Launch();
		});
	}

	FAutoConsoleCommandWithWorldAndArgs OpenCommand(
		TEXT("hol.UI.Open"),
		TEXT("hol.UI.Open <NewGame|Settings[:Tab]|Quit> [DelaySeconds=0]: opens a menu screen by the same path as its button"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Open));

	FAutoConsoleCommandWithWorldAndArgs LaunchCommand(
		TEXT("hol.UI.NewGame.Launch"),
		TEXT("hol.UI.NewGame.Launch [ConfigFile] [DelaySeconds=0]: launches a match from the New game screen, as its Launch button"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&LaunchNewGame));
}
