#include "Containers/Ticker.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLCheatPanel.h"
#include "HUD/HOLGameHUD.h"
#include "HUD/HOLGameScreen.h"
#include "HUD/HOLScenarioPanel.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLPlayerState.h"
#include "Widgets/SWindow.h"

/**
 * The game HUD for the scripted checks (J3, D284): what it shows, and a click on it by the same path as the mouse.
 *   hol.HUD.Report [DelaySeconds=0]   logs "HUD report: resources ..., selection ..., grid ..., queue ..." and, with
 *   the grid shown, "HUD cells: A: Maison; ..." (what the cells write) and "HUD tooltips: A: Maison | W 50 | +10
 *   population; ..." (J6a.5); a group of buildings' selection ends with its active subgroup (Tab): "7 buildings (own):
 *   MainCentre x1 House x6, subgroup All (7)", the cells being that subgroup's
 *   the "HUD fog" line (F7.3): the fog as the player has it, UHOLVisionSubsystem::MakeViewerLine
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
				// The selected unit's statistics (attack with the technologies' bonus...), on their own line too.
				UE_LOG(LogHOLUI, Display, TEXT("HUD stats: %s"), *Screen.DescribeStats());
			// The main forums (V2): the selected one's lives and state, the players' table.
			UE_LOG(LogHOLUI, Display, TEXT("HUD forum: %s"), *Screen.DescribeForum());
			// The thread of messages (V4.2), on its own line too.
			UE_LOG(LogHOLUI, Display, TEXT("HUD messages: %s"), *Screen.DescribeMessages());
			// The major alert's banner (V4.3), on its own line too.
			UE_LOG(LogHOLUI, Display, TEXT("HUD alert: %s"), *Screen.DescribeAlert());
			// EtapeDefini T: the bottom bar against the minimap's place, in the window's width.
			UE_LOG(LogHOLUI, Display, TEXT("HUD layout: %s"), *Screen.DescribeLayout());
			// F7.3: the fog as the player has it (cells in sight, explored, units of others shown or hidden).
			{
				const APlayerController* Owner = Screen.GetOwningPlayer();
				const AHOLPlayerState* OwnerState = Owner ? Owner->GetPlayerState<AHOLPlayerState>() : nullptr;
				const UHOLVisionSubsystem* Vision = Screen.GetWorld() ? Screen.GetWorld()->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
				UE_LOG(LogHOLUI, Display, TEXT("HUD fog: %s"), Vision ? *Vision->MakeViewerLine(OwnerState ? OwnerState->GetPlayerIndex() : INDEX_NONE) : TEXT("none"));
			}
			// J8a.5: the part in script, a line per block ("HUD hero: ...").
			TArray<FString> ScriptLines;
			Screen.DescribeScript().ParseIntoArray(ScriptLines, TEXT("\n"));
			for (const FString& Line : ScriptLines)
			{
				UE_LOG(LogHOLUI, Display, TEXT("HUD %s"), *Line);
			}
			// Where the window is on the desktop (the HUD's boxes are read in the viewport's space, not the desktop's).
			if (const UGameViewportClient* GameViewport = Screen.GetWorld() ? Screen.GetWorld()->GetGameViewport() : nullptr)
			{
				if (const TSharedPtr<SWindow> Window = GameViewport->GetWindow())
				{
					UE_LOG(LogHOLUI, Display, TEXT("HUD window: at %d,%d on the desktop, client %dx%d"), FMath::RoundToInt(Window->GetPositionInScreen().X),
						FMath::RoundToInt(Window->GetPositionInScreen().Y), FMath::RoundToInt(Window->GetClientSizeInScreen().X), FMath::RoundToInt(Window->GetClientSizeInScreen().Y));
				}
			}
			// Lesson 12b: which panels cover each other (the script's, and the cheats'), as drawn; preflight.ps1 fails on anything but "none".
			UE_LOG(LogHOLUI, Display, TEXT("HUD overlap: %s"), *Screen.DescribeOverlaps());
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
			UHOLScenarioPanel* ScenarioPanel = AHOLGameHUD::FindScenarioPanel(Screen.GetOwningPlayer());
			const bool bClicked = Screen.ClickWidget(Name) || (CheatPanel && CheatPanel->ClickWidget(Name))
				|| (ScenarioPanel && ScenarioPanel->ClickWidget(Name));
			UE_LOG(LogHOLUI, Display, TEXT("HUD click %s: %s"), *Name.ToString(), bClicked ? TEXT("done") : TEXT("hidden, greyed or unknown"));
		});
	}

	/** hol.HUD.Cheats <0|1>: the cheats' panel folded or open, without keeping the choice (scenarios: a known state, whatever was kept). */
	void Cheats(const TArray<FString>& Args, UWorld* World)
	{
		const bool bOpen = Args.Num() > 0 && FCString::Atoi(*Args[0]) != 0;
		WithGameScreen(World, 0.f, [bOpen](UHOLGameScreen& Screen)
		{
			if (UHOLCheatPanel* CheatPanel = AHOLGameHUD::FindCheatPanel(Screen.GetOwningPlayer()))
			{
				CheatPanel->SetOpen(bOpen, /*bKeep*/ false);
			}
			UE_LOG(LogHOLUI, Display, TEXT("HUD cheats set: %s"), bOpen ? TEXT("open") : TEXT("folded"));
		});
	}

	/** hol.Window.Move <X> <Y>: moves the game's window on the desktop (scenarios: the HUD must not depend on where the window is). */
	void MoveWindow(const TArray<FString>& Args, UWorld* World)
	{
		const UGameViewportClient* Viewport = World ? World->GetGameViewport() : nullptr;
		const TSharedPtr<SWindow> Window = Viewport ? Viewport->GetWindow() : nullptr;
		if (Args.Num() < 2 || !Window.IsValid())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.Window.Move <X> <Y>: no window or no coordinates"));
			return;
		}
		Window->MoveWindowTo(FVector2f(FCString::Atof(*Args[0]), FCString::Atof(*Args[1])));
		const FVector2f Place = Window->GetPositionInScreen();
		UE_LOG(LogHOLUI, Display, TEXT("Window moved: at %d,%d on the desktop, client %dx%d"), FMath::RoundToInt(Place.X), FMath::RoundToInt(Place.Y),
			FMath::RoundToInt(Window->GetClientSizeInScreen().X), FMath::RoundToInt(Window->GetClientSizeInScreen().Y));
	}

	FAutoConsoleCommandWithWorldAndArgs MoveWindowCommand(
		TEXT("hol.Window.Move"),
		TEXT("hol.Window.Move <X> <Y>: moves the game's window to X,Y on the desktop (scenarios)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&MoveWindow));

	FAutoConsoleCommandWithWorldAndArgs CheatsCommand(
		TEXT("hol.HUD.Cheats"),
		TEXT("hol.HUD.Cheats <0|1>: folds or opens the cheats' panel without keeping the choice (scenarios)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Cheats));

	FAutoConsoleCommandWithWorldAndArgs ReportCommand(
		TEXT("hol.HUD.Report"),
		TEXT("hol.HUD.Report [DelaySeconds=0]: logs what the game HUD shows (resources, selection, grid, queue)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Report));

	FAutoConsoleCommandWithWorldAndArgs ClickCommand(
		TEXT("hol.HUD.Click"),
		TEXT("hol.HUD.Click <WidgetName> [DelaySeconds=0]: clicks a cell of the command panel (GridCell00-11: production, or the units' orders GridCell04 attack-move, GridCell05 stop, GridCell11 hold), a building of the builders' age panels (AgeCell<age><cell>, one row per category: AgeCell000 the house, AgeCell004 the barracks), an item of the production queue (QueueItem0-4), the idle workers' counter (IdleWorkers) a button of the cheats' panel (CheatToggle, CheatFood, ...) or of the scenario's panel (ScenarioRestart, ScenarioClose), as the mouse would"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Click));
}
