#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

/**
 * Screenshot of the interface taken by the game itself, for the scripted end-to-end checks (D246): the image holds
 * the game viewport and its interface only, never the desktop around it.
 *   hol.UI.Capture <Name> [DelaySeconds=3] [quit]  ->  Saved/Screenshots/HOL/<Name>.png
 */
namespace UE::HeroesOfLiberty::UI
{
	void Capture(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Capture <Name> [DelaySeconds=3] [quit]"));
			return;
		}

		const FString Path = FPaths::Combine(FPaths::ScreenShotDir(), TEXT("HOL"), Args[0] + TEXT(".png"));
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 3.f;
		const bool bQuit = Args.Contains(TEXT("quit"));
		const TWeakObjectPtr<UWorld> WeakWorld(World);

		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Path, bQuit, WeakWorld](float)
		{
			FScreenshotRequest::RequestScreenshot(Path, /*bShowUI*/ true, /*bAddFilenameSuffix*/ false);
			UE_LOG(LogHOLUI, Display, TEXT("Capture requested: %s"), *Path);
			if (bQuit)
			{
				// One second for the screenshot to be written before quitting.
				FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakWorld](float)
				{
					GEngine->Exec(WeakWorld.Get(), TEXT("quit"));
					return false;
				}), 1.f);
			}
			return false;
		}), Delay);
	}

	FAutoConsoleCommandWithWorldAndArgs CaptureCommand(
		TEXT("hol.UI.Capture"),
		TEXT("hol.UI.Capture <Name> [DelaySeconds=3] [quit]: screenshot with the interface to Saved/Screenshots/HOL/<Name>.png"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Capture));
}
