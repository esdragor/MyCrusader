// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HUD/HOLGameNames.h"
#include "Match/HOLGameState.h"
#include "Match/HOLSpecializationSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLCommandGrid.h"

#if WITH_DEV_AUTOMATION_TESTS

// SP3 (SpecializationInProgress.md; GDD 13, D07, D332, D684, D785, answer B of Q-specialization-switch): the Forum's grid offers the
// choice of the open tier, read from the view the server's script board computes (the real chain: board -> FHOLSpecializationView ->
// HOLCommandGrid::SetSpecialization -> GetCommand):
// - tier 1 reached, nothing chosen: the Forum's two specialization cells start option 0 / option 1 of tier 1;
// - option 1 started on the board: its cell cancels (the countdown the board computes, 30 s), the other option's cell still starts (B);
// - applied 30 s later: tier 1 is final, no cell any more at tier 1 (one choice per tier); tier 2 reached: tier 2's two cells;
// - another building of the player's (not its main Forum): no specialization cell.
namespace UE::HeroesOfLiberty::Tests::SpecializationGrid
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FHOLGridContext ForumContext(const int32 PlayerTier, const FHOLSpecializationView& View)
	{
		FHOLGridContext Context;
		Context.bOwnBuilding = true;
		Context.bBuildingFinished = true;
		Context.bPrimaryForum = true;
		Context.Produces = { FName(TEXT("Peasant")) };
		HOLCommandGrid::SetSpecialization(Context, PlayerTier, View);
		return Context;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSpecializationGridTest, "HeroesOfLiberty.Specialization.ForumGrid", UE::HeroesOfLiberty::Tests::SpecializationGrid::Flags)

bool FHOLSpecializationGridTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::SpecializationGrid;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* GameState = World->SpawnActor<AHOLGameState>();
	World->SetGameState(GameState);

	UHOLSpecializationSubsystem* Board = UHOLSpecializationSubsystem::Find(World);
	const int32 CellA = HOLCommandGrid::GetSpecializationCell(0);
	const int32 CellB = HOLCommandGrid::GetSpecializationCell(1);
	if (TestNotNull(TEXT("the script board of specializations"), Board))
	{
		TestTrue(TEXT("Two distinct cells of the grid"), CellA >= 0 && CellB < HOLCommandGrid::NumCells && CellA != CellB);
		TestTrue(TEXT("The Peasant stays on A"), HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), 0).Command == EHOLGridCommand::Produce);

		FHOLGridCommand A = HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), CellA);
		FHOLGridCommand B = HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), CellB);
		TestTrue(TEXT("Tier 1 open: the first cell starts a specialization"), A.Command == EHOLGridCommand::Specialize);
		TestEqual(TEXT("Of tier 1"), A.Tier, 1);
		TestEqual(TEXT("Option 0"), A.Option, 0);
		TestTrue(TEXT("The second cell starts one too"), B.Command == EHOLGridCommand::Specialize);
		TestEqual(TEXT("Option 1 of tier 1"), B.Tier * 10 + B.Option, 11);
		TestTrue(TEXT("Tier 0: no choice"), HOLCommandGrid::GetCommand(ForumContext(0, Board->ViewNow(0)), CellA).Command == EHOLGridCommand::None);

		// The click's way (ExecuteGridCommand -> ServerStartSpecialization -> StartNow), then what the owner's grid reads.
		TestEqual(TEXT("The board starts option 1 of tier 1"), Board->StartNow(0, B.Tier, B.Option, 1, true), 0);
		B = HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), CellB);
		A = HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), CellA);
		TestTrue(TEXT("Running: its cell cancels"), B.Command == EHOLGridCommand::CancelSpecialization);
		TestEqual(TEXT("It shows the board's 30 s left (D684)"), B.RemainingMs, 30000);
		TestTrue(TEXT("The other option still starts (answer B: it replaces)"), A.Command == EHOLGridCommand::Specialize);

		// ELROND-1115i (Retours/E-specialization_button.md: four lines overflowed cell D): the running cell's text, from the game's
		// text table, fits the cell: two lines at most, none longer than the other option's cell label, the name left to the tooltip.
		const FString Running = HOLGameNames::GetSpecializationCellText(B).ToString();
		const FString Other = HOLGameNames::GetSpecializationCellText(A).ToString();
		TArray<FString> Lines;
		Running.ParseIntoArrayLines(Lines);
		int32 Longest = 0;
		for (const FString& Line : Lines)
		{
			Longest = FMath::Max(Longest, Line.Len());
		}
		const FString Shown = Running.Replace(TEXT("\n"), TEXT(" / "));
		TestTrue(FString::Printf(TEXT("The running cell writes two lines at most ('%s': %d)"), *Shown, Lines.Num()), Lines.Num() >= 1 && Lines.Num() <= 2);
		TestTrue(FString::Printf(TEXT("No line longer than the other cell's label '%s' (%d, %d)"), *Other, Longest, Other.Len()), Longest <= Other.Len());
		// ELROND-1116a (specialization_button_running.png: "En cours 29 s" wrapped to a third line and "Annuler" was cut): one line of a
		// grid cell holds "Voie A du" (9 characters, specialization_button_open.png), so no line of the running cell is longer.
		TestTrue(FString::Printf(TEXT("Each running line fits one line of the cell, 9 characters ('%s': %d)"), *Shown, Longest), Longest <= 9);
		TestFalse(FString::Printf(TEXT("The option's name is in the tooltip only, not on the running cell ('%s')"), *Shown),
			Running.Contains(HOLGameNames::GetSpecializationName(B.Tier, B.Option).ToString()));
		TestTrue(FString::Printf(TEXT("It writes the board's seconds left ('%s')"), *Shown), Running.Contains(FString::FromInt(FMath::DivideAndRoundUp(B.RemainingMs, 1000))));

		// hol.HUD.Report lists the specialization cells (ShownGridTypes), so a scenario waits for them in the standard report.
		const FName StartReport = HOLCommandGrid::GetReportName(A);
		const FName RunningReport = HOLCommandGrid::GetReportName(B);
		TestFalse(TEXT("The report names the cell that starts"), StartReport.IsNone());
		TestFalse(TEXT("And the running one"), RunningReport.IsNone());
		TestNotEqual(TEXT("Starting and cancelling read apart in the report"), StartReport, RunningReport);
		TestEqual(TEXT("The running one: cancel, tier 1, option B"), RunningReport.ToString(), FString(TEXT("CancelSpecializationT1B")));
		TestEqual(TEXT("The other: start, tier 1, option A"), StartReport.ToString(), FString(TEXT("SpecializeT1A")));
		TestTrue(TEXT("The Peasant keeps its own report name"), HOLCommandGrid::GetReportName(HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), 0)).IsNone());
		Board->AdvanceNow(0, 10000, true);
		TestEqual(TEXT("10 s later: 20 s left on the cell"), HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), CellB).RemainingMs, 20000);

		TestEqual(TEXT("20 s more: tier 1 applied"), Board->AdvanceNow(0, 20000, true), 1);
		TestTrue(TEXT("Tier 1 final: no cell at tier 1"), HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), CellA).Command == EHOLGridCommand::None);
		TestTrue(TEXT("Nor the other"), HOLCommandGrid::GetCommand(ForumContext(1, Board->ViewNow(0)), CellB).Command == EHOLGridCommand::None);
		const FHOLGridCommand Tier2 = HOLCommandGrid::GetCommand(ForumContext(2, Board->ViewNow(0)), CellA);
		TestTrue(TEXT("Tier 2 reached: its choice"), Tier2.Command == EHOLGridCommand::Specialize);
		TestEqual(TEXT("Of tier 2"), Tier2.Tier, 2);

		FHOLGridContext Barracks = ForumContext(2, Board->ViewNow(0));
		Barracks.bPrimaryForum = false;
		TestTrue(TEXT("Another building: no specialization"), HOLCommandGrid::GetCommand(Barracks, CellA).Command == EHOLGridCommand::None);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
