#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "Screens/HOLTitleScreen.h"

/**
 * Opens a menu screen by the same path as its button, for the scripted end-to-end checks (D246).
 *   hol.UI.Open <Quit> [DelaySeconds=0]
 */
namespace UE::HeroesOfLiberty::UI
{
	void Open(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Open <Quit> [DelaySeconds=0]"));
			return;
		}
		const FString Screen = Args[0];
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
		const TWeakObjectPtr<UWorld> WeakWorld(World);

		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Screen, WeakWorld](float)
		{
			APlayerController* PlayerController = WeakWorld.IsValid() ? WeakWorld->GetFirstPlayerController() : nullptr;
			if (Screen == TEXT("Quit"))
			{
				UHOLTitleScreen::AskToQuit(PlayerController);
			}
			else
			{
				UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Open: unknown screen '%s'"), *Screen);
				return false;
			}
			UE_LOG(LogHOLUI, Display, TEXT("Screen opened: %s"), *Screen);
			return false;
		}), Delay);
	}

	FAutoConsoleCommandWithWorldAndArgs OpenCommand(
		TEXT("hol.UI.Open"),
		TEXT("hol.UI.Open <Quit> [DelaySeconds=0]: opens a menu screen by the same path as its button"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Open));
}
