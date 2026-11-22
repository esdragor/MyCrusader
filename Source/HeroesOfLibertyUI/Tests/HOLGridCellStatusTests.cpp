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

#endif
