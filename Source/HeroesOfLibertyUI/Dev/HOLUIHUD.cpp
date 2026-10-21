#include "Containers/Ticker.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLCheatPanel.h"
#include "HUD/HOLGameHUD.h"
#include "HUD/HOLGameScreen.h"

/**
 * The game HUD for the scripted checks (J3, D284): what it shows, and a click on it by the same path as the mouse.
 *   hol.HUD.Report [DelaySeconds=0]   logs "HUD report: resources ..., selection ..., grid ..., queue ..." and, with
 *   the grid shown, "HUD cells: A: Maison; ..." (what the cells write) and "HUD tooltips: A: Maison | W 50 | +10
 *   population; ..." (J6a.5); a group of buildings' selection ends with its active subgroup (Tab): "7 buildings (own):
 *   MainCentre x1 House x6, subgroup All (7)", the cells being that subgroup's
 *   hol.HUD.Click <WidgetName> [DelaySeconds=0]   clicks a cell of the command panel (GridCell00 to GridCell11, A to
 *   V: a building's production, a site's « Annuler » GridCell11; with the player's units their orders, GridCell04
 *   attack-move, GridCell05 stop, GridCell11 hold), with Peasants selected the buildings of the age panels
 *   AgeCell<age><cell> (AgeCell000 house, AgeCell001 lumber camp, AgeCell002 mining camp, AgeCell004 barracks; an age not
 *   reached takes the click and does nothing; the medallions take none), an item of the queue (QueueItem0 to
 *   QueueItem4), the idle workers' counter (IdleWorkers),
 *   or a button of the cheats' panel (CheatToggle, CheatFood, CheatWood, CheatStone, CheatGold, CheatAll,
 *   CheatPopulation, CheatFastBuild); a hidden or greyed one is not clicked
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
			// On their own lines too (J6a.5): what the grid's cells write, and their tooltips.
			UE_LOG(LogHOLUI, Display, TEXT("HUD cells: %s"), *Screen.DescribeCells());
			UE_LOG(LogHOLUI, Display, TEXT("HUD tooltips: %s"), *Screen.DescribeTooltips());
			// On its own line: the checks read the report's line to its end.
			if (const UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(Screen.GetOwningPlayer()))
			{
				UE_LOG(LogHOLUI, Display, TEXT("HUD cheats: %s"), *CheatPanel->Describe());
			}
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
			UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(Screen.GetOwningPlayer());
			const bool bClicked = Screen.ClickWidget(Name) || (CheatPanel && CheatPanel->ClickWidget(Name));
			UE_LOG(LogHOLUI, Display, TEXT("HUD click %s: %s"), *Name.ToString(), bClicked ? TEXT("done") : TEXT("hidden, greyed or unknown"));
		});
	}

	FAutoConsoleCommandWithWorldAndArgs ReportCommand(
		TEXT("hol.HUD.Report"),
		TEXT("hol.HUD.Report [DelaySeconds=0]: logs what the game HUD shows (resources, selection, grid, queue)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Report));

	FAutoConsoleCommandWithWorldAndArgs ClickCommand(
		TEXT("hol.HUD.Click"),
		TEXT("hol.HUD.Click <WidgetName> [DelaySeconds=0]: clicks a cell of the command panel (GridCell00-11: production, or the units' orders GridCell04 attack-move, GridCell05 stop, GridCell11 hold), a building of the builders' age panels (AgeCell<age><cell>, one row per category: AgeCell000 the house, AgeCell004 the barracks), an item of the production queue (QueueItem0-4), the idle workers' counter (IdleWorkers) or a button of the cheats' panel (CheatToggle, CheatFood, ...), as the mouse would"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Click));
}
