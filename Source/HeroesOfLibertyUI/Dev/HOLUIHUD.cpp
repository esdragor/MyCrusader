#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLGameHUD.h"
#include "HUD/HOLGameScreen.h"

/**
 * The game HUD for the scripted checks (J3, D284): what it shows, and a click on it by the same path as the mouse.
 *   hol.HUD.Report [DelaySeconds=0]   logs "HUD report: resources ..., selection ..., grid ..., queue ..."
 *   hol.HUD.Click <WidgetName> [DelaySeconds=0]   clicks a cell of the grid (GridCell00 to GridCell11, A to V) or an
 *   item of the queue (QueueItem0 to QueueItem4); a hidden or greyed one is not clicked
 */
namespace UE::HeroesOfLiberty::UI::HUD
{
	/** Runs Action on the game screen of the world's first player, after Delay seconds. */
	void WithGameScreen(UWorld* World, const float Delay, TFunction<void(UHOLGameScreen&)> Action)
	{
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
			[WeakWorld = TWeakObjectPtr<UWorld>(World), Action = MoveTemp(Action)](float)
			{
				UHOLGameScreen* Screen = WeakWorld.IsValid() ? AHOLGameHUD::FindScreen(WeakWorld->GetFirstPlayerController()) : nullptr;
				if (Screen)
				{
					Action(*Screen);
				}
				else
				{
					UE_LOG(LogHOLUI, Warning, TEXT("hol.HUD: no game screen (not in a match)"));
				}
				return false;
			}), Delay);
	}

	void Report(const TArray<FString>& Args, UWorld* World)
	{
		const float Delay = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.f;
		WithGameScreen(World, Delay, [](UHOLGameScreen& Screen)
		{
			UE_LOG(LogHOLUI, Display, TEXT("HUD report: %s"), *Screen.Describe());
		});
	}

	void Click(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.HUD.Click <WidgetName> [DelaySeconds=0]"));
			return;
		}
		const FName Name(*Args[0]);
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 0.f;
		WithGameScreen(World, Delay, [Name](UHOLGameScreen& Screen)
		{
			const bool bClicked = Screen.ClickWidget(Name);
			UE_LOG(LogHOLUI, Display, TEXT("HUD click %s: %s"), *Name.ToString(), bClicked ? TEXT("done") : TEXT("hidden, greyed or unknown"));
		});
	}

	FAutoConsoleCommandWithWorldAndArgs ReportCommand(
		TEXT("hol.HUD.Report"),
		TEXT("hol.HUD.Report [DelaySeconds=0]: logs what the game HUD shows (resources, selection, grid, queue)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Report));

	FAutoConsoleCommandWithWorldAndArgs ClickCommand(
		TEXT("hol.HUD.Click"),
		TEXT("hol.HUD.Click <WidgetName> [DelaySeconds=0]: clicks a cell of the command grid (GridCell00-11) or an item of the production queue (QueueItem0-4), as the mouse would"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Click));
}
