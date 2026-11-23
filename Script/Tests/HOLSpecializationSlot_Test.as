// SP3 (ARWEN-1123c): the faction slot's specialization button (HOLSpecializationSlot in Script/UI/HOLSpecializationSlot.as) and the routing of
// hol.HUD.Click FactionBox<n> (HOLFactionSlot::BoxIndexOfName). Every view comes from the REAL rules (HOLSpecialization::Start / Advance on a real
// state, copied to the view as the board does): the button is lit, the bar is at 25 %, the click starts or cancels, because the rules say so.

namespace SpecializationSlotTest
{
	// What the owner reads of the state, as UHOLSpecializationBoardScript::ViewSpecialization copies it.
	FHOLSpecializationView ViewOf(FHOLSpecializationState State)
	{
		FHOLSpecializationView View;
		View.PendingTier = State.PendingTier;
		View.PendingOption = State.PendingTier > 0 && State.PendingTier < State.RunningOption.Num() ? State.RunningOption[State.PendingTier] : -1;
		View.RemainingMs = HOLSpecialization::RemainingMs(State);
		View.Chosen = State.Chosen;
		return View;
	}
}

void Test_FactionBoxNameGivesTheBoxIndex(FUnitTest& T)
{
	int First = HOLFactionSlot::BoxIndexOfName(n"FactionBox0");
	int Last = HOLFactionSlot::BoxIndexOfName(n"FactionBox3");
	T.AssertEquals(0, First, "FactionBox0 is box 0");
	T.AssertEquals(3, Last, "FactionBox3 is box 3");
	int Two = HOLFactionSlot::BoxIndexOfName(n"FactionBox12");
	T.AssertEquals(12, Two, "two digits read as a number");
	int Label = HOLFactionSlot::BoxIndexOfName(n"FactionBoxLabel1");
	T.AssertEquals(-1, Label, "a label is not a box");
	int Column = HOLFactionSlot::BoxIndexOfName(n"FactionBoxColumn");
	T.AssertEquals(-1, Column, "the column is not a box");
	int Bare = HOLFactionSlot::BoxIndexOfName(n"FactionBox");
	T.AssertEquals(-1, Bare, "no digit, no box");
	int Other = HOLFactionSlot::BoxIndexOfName(n"GridCell01");
	T.AssertEquals(-1, Other, "another widget is not ours");
}

void Test_SpecializationSlotDarkUntilATierIsOpen(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(2);
	FHOLSpecializationSlotView Early = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 0, true, 2);
	T.AssertTrue(!Early.bLit, "faction tier 0: no tier reached, the button is dark");
	FHOLSpecializationSlotView NoForum = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, false, 2);
	T.AssertTrue(!NoForum.bLit, "no complete Forum: the button stays out (Q-nomad-forum Q2)");
	FHOLSpecializationSlotView Open = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, true, 2);
	T.AssertTrue(Open.bLit, "tier 1 reached, Forum complete: lit");
	T.AssertEquals(1, Open.OpenTier, "the open tier is 1");
	T.AssertTrue(!Open.bRunning, "nothing runs yet");
	T.AssertEquals(2, Open.OptionTexts.Num(), "two options in the panel");
	T.AssertEquals("Voie A du palier 1", Open.OptionTexts[0], "the table's text of the first option");
	T.AssertEquals("Voie B du palier 1", Open.OptionTexts[1], "the table's text of the second option");
	T.AssertEquals("spec lit tier 1 options 2", Open.Describe, "stable line");
}

void Test_SpecializationSlotTemplarsShowThreeOptions(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewStateFor(EHOLFaction::Templars);
	FHOLSpecializationSlotView View = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, true, HOLSpecialization::OptionCountOf(EHOLFaction::Templars));
	T.AssertEquals(3, View.OptionTexts.Num(), "three options for the Templars");
	T.AssertEquals("Voie C du palier 1", View.OptionTexts[2], "the third option is Voie C");
}

void Test_SpecializationSlotBarCountsDownWhileTheUnlockRuns(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(2);
	int Started = HOLSpecialization::Start(State, 1, 0, 1);
	T.AssertEquals(HOLSpecialization::Ok, Started, "the rule starts tier 1 option 0");
	int Applied = HOLSpecialization::Advance(State, 7500, true);
	T.AssertEquals(0, Applied, "7.5 s of 30 s: nothing applied yet");
	FHOLSpecializationSlotView View = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, true, 2);
	T.AssertTrue(View.bLit, "the button stays lit while its unlock runs");
	T.AssertTrue(View.bRunning, "the unlock runs");
	T.AssertEquals(0, View.PendingOption, "option 0 runs");
	T.AssertEquals(22500, View.RemainingMs, "22.5 s left");
	T.AssertTrue(Math::Abs(View.Fraction - 0.25) < 0.001, "the bar is at a quarter");
	T.AssertEquals("23 s", View.BarText, "the table's seconds text, rounded up");
	T.AssertEquals("spec running tier 1 option 0 23 s", View.Describe, "stable line");
}

void Test_SpecializationSlotMovesToTheNextTierOnceApplied(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(2);
	HOLSpecialization::Start(State, 1, 1, 2);
	int Applied = HOLSpecialization::Advance(State, 30000, true);
	T.AssertEquals(1, Applied, "tier 1 applies after its delay");
	FHOLSpecializationSlotView Tier1Only = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, true, 2);
	T.AssertTrue(!Tier1Only.bLit, "tier 1 applied and tier 2 not reached: dark");
	FHOLSpecializationSlotView Tier2 = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 2, true, 2);
	T.AssertTrue(Tier2.bLit, "tier 2 reached: lit again");
	T.AssertEquals(2, Tier2.OpenTier, "the open tier is 2");
	T.AssertEquals("Voie A du palier 2", Tier2.OptionTexts[0], "the panel is about tier 2");
}

void Test_SpecializationSlotClickStartsThenCancelsOnTheRealState(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(2);
	FHOLSpecializationSlotView Idle = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, true, 2);
	FHOLSpecializationClick Pick = HOLSpecializationSlot::ClickOption(Idle, 1);
	T.AssertTrue(Pick.Action == EHOLSpecializationAction::Start, "a click on an option of the lit button starts it");
	T.AssertEquals(1, Pick.Tier, "tier 1");
	T.AssertEquals(1, Pick.Option, "option 1");
	int Result = HOLSpecialization::Start(State, Pick.Tier, Pick.Option, 1);
	T.AssertEquals(HOLSpecialization::Ok, Result, "the server rule accepts what the click asked");
	FHOLSpecializationSlotView Running = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, true, 2);
	T.AssertTrue(Running.bRunning && Running.PendingOption == 1, "the slot now shows option 1 running");
	FHOLSpecializationClick Other = HOLSpecializationSlot::ClickOption(Running, 0);
	T.AssertTrue(Other.Action == EHOLSpecializationAction::Nothing, "another option while one runs: nothing (the rule would say Busy)");
	FHOLSpecializationClick Stop = HOLSpecializationSlot::ClickOption(Running, 1);
	T.AssertTrue(Stop.Action == EHOLSpecializationAction::Cancel, "a click on the running option cancels it");
	int Cancelled = HOLSpecialization::Cancel(State, Stop.Tier);
	T.AssertEquals(HOLSpecialization::Ok, Cancelled, "the server rule cancels");
	FHOLSpecializationSlotView After = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 1, true, 2);
	T.AssertTrue(!After.bRunning && After.bLit, "back to a lit button with nothing running");
	FHOLSpecializationSlotView Dark = HOLSpecializationSlot::Make(SpecializationSlotTest::ViewOf(State), 0, true, 2);
	FHOLSpecializationClick Nothing = HOLSpecializationSlot::ClickOption(Dark, 0);
	T.AssertTrue(Nothing.Action == EHOLSpecializationAction::Nothing, "a dark button takes no option click");
}

// SP3 widget (ARWEN-1123d): the widget reads the owner through HOLSpecializationSlot::Resolve (ReadView or an injected read). Until
// AHOLPlayerState::GetSpecialization is a UFUNCTION, ReadView is empty: the button stays dark; an injected read built from the REAL rules lights it.
void Test_SpecializationSlotWidgetReadIsDarkUntilBoundAndLitWhenInjected(FUnitTest& T)
{
	AHOLPlayerController NoController = nullptr;
	FHOLSpecializationRead NoRead;
	FHOLSpecializationSlotView Unbound = HOLSpecializationSlot::Resolve(NoController, 0, NoRead, false);
	T.AssertTrue(!Unbound.bLit, "the unbound read (ReadView) is empty: the button stays dark");
	FHOLSpecializationState State = HOLSpecialization::NewState(2);
	FHOLSpecializationRead Injected;
	Injected.View = SpecializationSlotTest::ViewOf(State);
	Injected.PlayerTier = 1;
	Injected.bForumComplete = true;
	Injected.OptionCount = 2;
	FHOLSpecializationSlotView Lit = HOLSpecializationSlot::Resolve(NoController, 0, Injected, true);
	T.AssertTrue(Lit.bLit, "an injected view with a tier open lights the button");
	T.AssertEquals(2, Lit.OptionTexts.Num(), "and its panel shows the 2 options");
	HOLSpecialization::Start(State, 1, 0, 1);
	Injected.View = SpecializationSlotTest::ViewOf(State);
	FHOLSpecializationSlotView Running = HOLSpecializationSlot::Resolve(NoController, 0, Injected, true);
	T.AssertTrue(Running.bRunning && Running.Fraction >= 0.0, "once the real rule started an unlock, the injected view shows the bar running");
	FHOLSpecializationSlotView Ignored = HOLSpecializationSlot::Resolve(NoController, 0, Injected, false);
	T.AssertTrue(!Ignored.bLit, "without the injection flag the injected read is ignored");
}

// SP3 (ELROND-1123pd): the read from the state. The test world cannot set a player's specialization (AHOLPlayerState::SetSpecialization is not a
// UFUNCTION), so the pure conversion ReadOf is tested: the view of a REAL state (rules Start / Advance) and the faction tier give the voie, its progress
// and the faction's option count; no specialization at tier 1 gives the lit "to choose" view.
void Test_SpecializationSlotReadOfShowsTheVoieAndItsProgress(FUnitTest& T)
{
	FHOLSpecializationState Fresh = HOLSpecialization::NewStateFor(EHOLFaction::Templars);
	FHOLSpecializationRead ToChoose = HOLSpecializationSlot::ReadOf(SpecializationSlotTest::ViewOf(Fresh), 1, EHOLFaction::Templars);
	FHOLSpecializationSlotView Choose = HOLSpecializationSlot::Make(ToChoose.View, ToChoose.PlayerTier, ToChoose.bForumComplete, ToChoose.OptionCount);
	T.AssertTrue(Choose.bLit && !Choose.bRunning, "no specialization at faction tier 1: lit, to choose");
	T.AssertEquals(3, Choose.OptionTexts.Num(), "the Templars' read carries their three voies");

	HOLSpecialization::Start(Fresh, 1, 2, 1);
	HOLSpecialization::Advance(Fresh, 7500, true);
	FHOLSpecializationRead Running = HOLSpecializationSlot::ReadOf(SpecializationSlotTest::ViewOf(Fresh), 1, EHOLFaction::Templars);
	FHOLSpecializationSlotView Voie = HOLSpecializationSlot::Make(Running.View, Running.PlayerTier, Running.bForumComplete, Running.OptionCount);
	T.AssertTrue(Voie.bRunning && Voie.PendingOption == 2, "the read carries the voie that runs (Voie C)");
	T.AssertEquals(22500, Voie.RemainingMs, "and its time left");

	FHOLSpecializationRead Early = HOLSpecializationSlot::ReadOf(SpecializationSlotTest::ViewOf(Fresh), 0, EHOLFaction::Templars);
	T.AssertEquals(0, Early.PlayerTier, "the faction tier is the one given");
	FHOLSpecializationRead Dawn = HOLSpecializationSlot::ReadOf(SpecializationSlotTest::ViewOf(Fresh), 1, EHOLFaction::DawnOrder);
	T.AssertEquals(HOLSpecialization::OptionCountOf(EHOLFaction::DawnOrder), Dawn.OptionCount, "the option count follows the faction");
}
