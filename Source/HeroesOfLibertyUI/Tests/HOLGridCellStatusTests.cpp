#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "HUD/HOLGameScreen.h"
#include "Widgets/HOLGridCell.h"

#if WITH_DEV_AUTOMATION_TESTS

// GALADRIEL-1122g (InterfaceInProgress.md, Auto_Journey_File.md): in a 76 px cell, "Bientot" glued under a name of 3 to 4 lines was cut by the
// cell. The status is now a line of its own at the bottom of the cell and the name keeps a zone bounded to whole lines above it. Layout
// rule: the zone of the name, its top band and the status line fit in the cell; a cell without status is not touched (no bound).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGridCellNameZoneTest, "HeroesOfLiberty.UI.GridCell.NameZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGridCellNameZoneTest::RunTest(const FString& Parameters)
{
	const float Cell = UHOLGridCell::DefaultCellSize;
	const float Top = UHOLGridCell::NameTopBand;
	const float Line = 15.f;

	// With a status: the name's zone, the top band and the status line fit in the cell, the zone is whole lines, and a name of 4 lines
	// (4 x 15 + 12 + the status line > 76) is cut to fewer.
	const FHOLGridCellNameZone Zone = UHOLGridCell::ComputeNameZone(Cell, Top, Line, /*bHasStatus*/ true);
	TestTrue(TEXT("the status line has a height (a line of its own)"), Zone.StatusHeight >= Line);
	TestTrue(TEXT("top band + name zone + status line fit in the cell"), Top + Zone.NameHeight + Zone.StatusHeight <= Cell);
	TestTrue(TEXT("the name keeps at least one line"), Zone.NameLines >= 1);
	TestTrue(TEXT("a name of 4 lines is bounded"), Zone.NameLines < 4);
	TestEqual(TEXT("the name's zone is a whole number of lines"), Zone.NameHeight, Zone.NameLines * Line);

	// A larger cell gives the name more lines (the zone is computed, not a constant).
	const FHOLGridCellNameZone Large = UHOLGridCell::ComputeNameZone(Cell + 30.f, Top, Line, true);
	TestTrue(TEXT("a larger cell keeps more name lines"), Large.NameLines > Zone.NameLines);

	// A line taller than the room left still keeps one line of the name.
	const FHOLGridCellNameZone Tall = UHOLGridCell::ComputeNameZone(Cell, Top, 60.f, true);
	TestEqual(TEXT("the name keeps one line when the lines are tall"), Tall.NameLines, 1);

	// A cell without a status is not modified: no bound, no line kept.
	const FHOLGridCellNameZone None = UHOLGridCell::ComputeNameZone(Cell, Top, Line, /*bHasStatus*/ false);
	TestEqual(TEXT("no status: the name is not bounded"), None.NameHeight, 0.f);
	TestEqual(TEXT("no status: no line kept"), None.StatusHeight, 0.f);
	return true;
}

// The real cell: its widgets built (the engine builds them only with a player, as the New game screen test), the name given by SetCommand
// and the status by SetStatus; the text the cell shows keeps them apart (DescribeContent: name, new line, status, as the former glued text,
// so the scripted checks read the same) and the name's zone is bounded only on the cell that has a status.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGridCellStatusTest, "HeroesOfLiberty.UI.GridCell.StatusLine",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGridCellStatusTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	ULocalPlayer* Player = NewObject<ULocalPlayer>(GEngine);
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	Controller->Player = Player;
	Player->PlayerController = Controller;
	if (!World->GetPlayerControllerIterator())
	{
		World->AddController(Controller);
	}
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	const FText Name = FText::FromString(TEXT("Cloche d'alarme amelioree"));
	const FText Status = FText::FromString(TEXT("Bientot"));

	UHOLGridCell* Soon = CreateWidget<UHOLGridCell>(Controller);
	if (!TestNotNull(TEXT("the cell is built"), Soon))
	{
		return false;
	}
	Soon->SetCommand(Name, EHOLGridCellState::Greyed);
	TestEqual(TEXT("no status yet: the cell shows the name alone"), Soon->DescribeContent(), Name.ToString());
	TestEqual(TEXT("no status: the name is not bounded"), Soon->GetNameZoneHeight(), 0.f);

	Soon->SetStatus(Status);
	TestEqual(TEXT("the name and the status are two lines of the cell"), Soon->DescribeContent(), Name.ToString() + TEXT("\n") + Status.ToString());
	const float Zone = Soon->GetNameZoneHeight();
	TestTrue(TEXT("the name has a bounded zone"), Zone > 0.f);
	TestTrue(TEXT("the zone, its top band and a status line fit in the cell"),
		UHOLGridCell::NameTopBand + Zone < UHOLGridCell::DefaultCellSize);

	// A new command resets the status (the cell is reused by the grid for the next command).
	Soon->SetCommand(Name, EHOLGridCellState::Ready);
	TestEqual(TEXT("a new command clears the status"), Soon->DescribeContent(), Name.ToString());
	TestEqual(TEXT("a new command unbounds the name"), Soon->GetNameZoneHeight(), 0.f);
	return true;
}

// The cache the script's grid panel applies to the cell (HUD H-10, FHOLGridCellFill, ApplyGridCell): RefreshGrid writes the status there
// after the command, SetCommand of a new command drops it, ClearUnit drops it too (the cell is rebuilt each frame).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGridCellFillStatusTest, "HeroesOfLiberty.UI.GridCell.FillStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGridCellFillStatusTest::RunTest(const FString& Parameters)
{
	const FText Status = FText::FromString(TEXT("Bientot"));
	FHOLGridCellFill Fill;
	Fill.SetCommand(FText::FromString(TEXT("Hallebardes trempees")), EHOLGridCellState::Greyed);
	TestTrue(TEXT("a command alone has no status"), Fill.Status.IsEmpty());
	Fill.SetStatus(Status);
	TestEqual(TEXT("the fill keeps the status for the cell"), Fill.Status.ToString(), Status.ToString());
	TestEqual(TEXT("the name stays the name alone"), Fill.Name.ToString(), FString(TEXT("Hallebardes trempees")));
	Fill.SetCommand(FText::FromString(TEXT("Maison")), EHOLGridCellState::Ready);
	TestTrue(TEXT("a new command drops the status"), Fill.Status.IsEmpty());
	Fill.SetStatus(Status);
	Fill.ClearUnit();
	TestTrue(TEXT("a cleared fill has no status"), Fill.Status.IsEmpty());
	return true;
}

// GALADRIEL-1122h (recette 1122d): at 1280x720 a cell is about 51 px wide and "Coming soon" (about 60 px) overflowed onto the next cell. The status
// line is reduced to the width of the cell (bounded below), in the 3 languages, and a cell without a status is not touched.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGridCellStatusFitTest, "HeroesOfLiberty.UI.GridCell.StatusFit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGridCellStatusFitTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	ULocalPlayer* Player = NewObject<ULocalPlayer>(GEngine);
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	Controller->Player = Player;
	Player->PlayerController = Controller;
	if (!World->GetPlayerControllerIterator())
	{
		World->AddController(Controller);
	}
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	UHOLGridCell* Cell = CreateWidget<UHOLGridCell>(Controller);
	if (!TestNotNull(TEXT("the cell is built"), Cell))
	{
		return false;
	}
	const FText Name = FText::FromString(TEXT("Hallebardes trempees"));
	const float CellWidth = 51.f;
	const float Available = CellWidth - 2.f * UHOLGridCell::StatusSidePad;

	// The 3 languages of the status (FR, EN, JA), measured by the cell on its own font; the line, once fitted, is not wider than the cell.
	const TCHAR* const Statuses[] = { TEXT("Bientot"), TEXT("Coming soon"), TEXT("近日公開") };
	bool bAnyReduced = false;
	for (const TCHAR* const Text : Statuses)
	{
		Cell->SetCommand(Name, EHOLGridCellState::Greyed);
		Cell->SetStatus(FText::FromString(Text));
		const float Width = Cell->MeasureStatusWidth();
		TestTrue(*FString::Printf(TEXT("'%s' has a measured width"), Text), Width > 0.f);
		const FHOLGridCellStatusFit Fit = UHOLGridCell::ComputeStatusFit(Width, Available);
		TestTrue(*FString::Printf(TEXT("'%s' fits in a 51 px cell (%.1f <= %.1f)"), Text, Fit.Width, Available), Fit.Width <= Available + KINDA_SMALL_NUMBER);
		TestTrue(*FString::Printf(TEXT("'%s': the scale stays in [min, 1]"), Text), Fit.Scale >= UHOLGridCell::MinStatusScale && Fit.Scale <= 1.f);
		if (Width <= Available)
		{
			TestEqual(*FString::Printf(TEXT("'%s' already fits: not reduced"), Text), Fit.Scale, 1.f);
		}
		else
		{
			bAnyReduced = true;
		}
	}
	TestTrue(TEXT("at least one language is wider than the cell (the reduction is exercised)"), bAnyReduced);

	// D889: "Coming soon" is too wide for one line at the readable bound (55.8 px for 47), so it takes TWO lines ("Coming" / "soon"), each of
	// them readable (scale above the bound, the fitted width inside the cell); the one-word statuses keep one line, and the name's zone makes room.
	Cell->SetCommand(Name, EHOLGridCellState::Greyed);
	Cell->SetStatus(FText::FromString(TEXT("Bientot")));
	TestEqual(TEXT("a one-word status takes one line"), Cell->GetStatusLineCount(), 1);
	const float OneLineZone = Cell->GetNameZoneHeight();
	Cell->SetCommand(Name, EHOLGridCellState::Greyed);
	Cell->SetStatus(FText::FromString(TEXT("Coming soon")));
	TestEqual(TEXT("'Coming soon' takes two lines"), Cell->GetStatusLineCount(), 2);
	TestEqual(TEXT("the content still reads the status as given"), Cell->DescribeContent(), Name.ToString() + TEXT("\nComing soon"));
	const float TwoLinesWidth = Cell->MeasureStatusWidth();
	const FHOLGridCellStatusFit TwoLinesFit = UHOLGridCell::ComputeStatusFit(TwoLinesWidth, Available);
	TestTrue(*FString::Printf(TEXT("two lines fit in the 51 px cell (%.1f <= %.1f)"), TwoLinesFit.Width, Available), TwoLinesFit.Width <= Available + KINDA_SMALL_NUMBER);
	TestTrue(TEXT("two lines are not reduced to the bound"), TwoLinesFit.Scale > UHOLGridCell::MinStatusScale);
	TestTrue(TEXT("the name's zone is not taller with two status lines"), Cell->GetNameZoneHeight() <= OneLineZone + KINDA_SMALL_NUMBER);
	TestEqual(TEXT("a status broken stays a single word when it has no space"), UHOLGridCell::BreakStatusLines(TEXT("Bientot")), FString(TEXT("Bientot")));
	TestEqual(TEXT("a status is broken at the space nearest its middle"), UHOLGridCell::BreakStatusLines(TEXT("Coming soon")), FString(TEXT("Coming\nsoon")));

	// GALADRIEL-1123j (D889, recette 1123i): the break follows the REAL width of the cell, not a constant. The cell is laid out at 51 px (1280x720) then
	// at 76 px (1920x1080): "Coming soon", measured by the cell on its own font, is wider than both and takes TWO lines each time; a one-word status
	// and a text without a space keep one line; a cell wide enough for the whole text keeps one line (the break is undone when the cell widens).
	const float NarrowWidths[] = { 51.f, 76.f };
	for (const float LaidOutWidth : NarrowWidths)
	{
		Cell->SetCommand(Name, EHOLGridCellState::Greyed);
		Cell->SetStatus(FText::FromString(TEXT("Coming soon")));
		Cell->FitStatusToWidth(LaidOutWidth);
		TestEqual(*FString::Printf(TEXT("'Coming soon' in a %.0f px cell takes two lines"), LaidOutWidth), Cell->GetStatusLineCount(), 2);
		Cell->FitStatusToWidth(1000.f);
		TestEqual(*FString::Printf(TEXT("'Coming soon' in a 1000 px cell takes one line (after %.0f px)"), LaidOutWidth), Cell->GetStatusLineCount(), 1);
		Cell->FitStatusToWidth(LaidOutWidth);
		TestEqual(*FString::Printf(TEXT("'Coming soon' back in a %.0f px cell takes two lines"), LaidOutWidth), Cell->GetStatusLineCount(), 2);
		Cell->SetStatus(FText::FromString(TEXT("近日公開")));
		Cell->FitStatusToWidth(LaidOutWidth);
		TestEqual(*FString::Printf(TEXT("a Japanese status without a space keeps one line in %.0f px"), LaidOutWidth), Cell->GetStatusLineCount(), 1);
		Cell->SetStatus(FText::FromString(TEXT("Bientot")));
		Cell->FitStatusToWidth(LaidOutWidth);
		TestEqual(*FString::Printf(TEXT("a one-word status keeps one line in %.0f px"), LaidOutWidth), Cell->GetStatusLineCount(), 1);
	}

	// A cell too narrow for the bound keeps the bound (the text is cut by the clip, never made unreadable).
	TestEqual(TEXT("the reduction is bounded"), UHOLGridCell::ComputeStatusFit(100.f, 5.f).Scale, UHOLGridCell::MinStatusScale);

	// A cell without a status is not touched: no width, the name alone, no bound.
	Cell->SetCommand(Name, EHOLGridCellState::Ready);
	TestEqual(TEXT("no status: no width"), Cell->MeasureStatusWidth(), 0.f);
	TestEqual(TEXT("no status: the name alone"), Cell->DescribeContent(), Name.ToString());
	TestEqual(TEXT("no status: the name is not bounded"), Cell->GetNameZoneHeight(), 0.f);
	return true;
}

#endif
