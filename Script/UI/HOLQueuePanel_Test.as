// NOT COMPILED YET (needs the C++ lot of SAURON-1119d for HOLQueuePanel.as, same module). H-7: the production queue's rules (HOLQueuePanel.as), pure, in literals
// copied from the C++ screen's behaviour (HOLGameScreen.cpp RefreshQueue / UpdateHead / HandleQueueItemClicked / the Describe's queue part read by quick_check).

void Test_QueuePanelItemNamesAndCountAreTheCppOnes(FUnitTest& T)
{
	T.AssertEquals(HOLQueuePanel::ItemName(0), "QueueItem0", "the first button's widget name (quick_check: hol.HUD.Click QueueItem0)");
	T.AssertEquals(HOLQueuePanel::ItemName(2), "QueueItem2", "the third (quick_check: hol.HUD.Click QueueItem2)");
	T.AssertEquals(HOLQueuePanel::ItemCount(), 5, "NumQueueItems");
	T.AssertEquals(HOLQueuePanel::ItemsShown(3), 3, "a queue of 3: 3 buttons");
	T.AssertEquals(HOLQueuePanel::ItemsShown(9), 5, "a longer queue: the five buttons the panel has");
	T.AssertEquals(HOLQueuePanel::ItemsShown(0), 0, "an empty queue: none");
}

void Test_QueuePanelItemVisibilityFollowsTheQueue(FUnitTest& T)
{
	T.AssertTrue(HOLQueuePanel::PanelShown(1), "one item: the panel is there");
	T.AssertFalse(HOLQueuePanel::PanelShown(0), "an empty queue: collapsed");
	T.AssertTrue(HOLQueuePanel::ItemShown(1, 2), "the second of two");
	T.AssertFalse(HOLQueuePanel::ItemShown(2, 2), "the third of two: collapsed");
	T.AssertTrue(HOLQueuePanel::ItemShown(4, 7), "the fifth of seven");
	T.AssertFalse(HOLQueuePanel::ItemShown(5, 7), "there is no sixth button");
	T.AssertFalse(HOLQueuePanel::ItemShown(-1, 3), "a negative index is no button");
}

void Test_QueuePanelLabelIsTheResearchNameOrTheUnitName(FUnitTest& T)
{
	T.AssertEquals(HOLQueuePanel::ItemLabel(false, "Age de Pierre", "Paysan"), "Paysan", "a unit: its name");
	T.AssertEquals(HOLQueuePanel::ItemLabel(true, "Age de Pierre", "Paysan"), "Age de Pierre", "a research waits as a unit does (T6): its name");
}

void Test_QueuePanelSignatureChangesWithTheQueue(FUnitTest& T)
{
	TArray<FString> One;
	One.Add("Peasant");
	TArray<FString> Two;
	Two.Add("Peasant");
	Two.Add("Peasant");
	TArray<FString> Other;
	Other.Add("Peasant");
	Other.Add("Hoplite");
	TArray<FString> Empty;
	const FString SigOne = HOLQueuePanel::Signature(One);
	T.AssertEquals(SigOne, "1|Peasant", "the length then the types");
	T.AssertTrue(HOLQueuePanel::SignatureChanged(SigOne, HOLQueuePanel::Signature(Two)), "a second Peasant queued: changed");
	T.AssertTrue(HOLQueuePanel::SignatureChanged(HOLQueuePanel::Signature(Two), HOLQueuePanel::Signature(Other)), "same length, another type: changed");
	T.AssertFalse(HOLQueuePanel::SignatureChanged(SigOne, HOLQueuePanel::Signature(One)), "the same queue: not changed");
	T.AssertTrue(HOLQueuePanel::SignatureChanged("", HOLQueuePanel::Signature(Empty)), "never shown yet, then empty: changed once (0 is not the empty text)");
	T.AssertEquals(HOLQueuePanel::Signature(Empty), "0", "an empty queue");
}

void Test_QueuePanelHeadStateIsUpdateHeadsRule(FUnitTest& T)
{
	T.AssertEquals(HOLQueuePanel::HeadState(0, false, 0.0, false, false), HOLQueuePanel::HeadNone(), "no queue: none");
	T.AssertEquals(HOLQueuePanel::HeadState(2, true, 0.4, false, false), HOLQueuePanel::HeadInProgress(), "a started head at 40 %: in progress");
	T.AssertEquals(HOLQueuePanel::HeadState(2, true, 1.0, true, false), HOLQueuePanel::HeadPopulationFull(), "D829: 100 % and no room: population full");
	T.AssertEquals(HOLQueuePanel::HeadState(2, true, 1.0, false, false), HOLQueuePanel::HeadInProgress(), "100 % with room: still in progress (it leaves the next step)");
	T.AssertEquals(HOLQueuePanel::HeadState(2, true, 0.9, true, false), HOLQueuePanel::HeadInProgress(), "no room but not at 100 %: the block only counts at the end");
	T.AssertEquals(HOLQueuePanel::HeadState(1, false, 0.0, false, true), HOLQueuePanel::HeadPopulationFull(), "not started, no room: waits (as in AoE)");
	T.AssertEquals(HOLQueuePanel::HeadState(1, false, 0.0, false, false), HOLQueuePanel::HeadStarting(), "not started, room: starts at the server's next step");
}

void Test_QueuePanelBarAndNoteFollowTheHead(FUnitTest& T)
{
	T.AssertEquals(HOLQueuePanel::HeadPercent(HOLQueuePanel::HeadInProgress(), 0.4), 0.4, "the bar shows the head's progress");
	T.AssertEquals(HOLQueuePanel::HeadPercent(HOLQueuePanel::HeadStarting(), 0.7), 0.0, "a head not started: an empty bar");
	T.AssertEquals(HOLQueuePanel::HeadPercent(HOLQueuePanel::HeadNone(), 0.7), 0.0, "no head: an empty bar");
	T.AssertEquals(HOLQueuePanel::HeadPercent(HOLQueuePanel::HeadPopulationFull(), 1.0), 1.0, "D829: held at 100 % keeps its full bar");
	T.AssertTrue(HOLQueuePanel::ProgressShown(HOLQueuePanel::HeadInProgress(), 0.4), "in progress: the bar");
	T.AssertTrue(HOLQueuePanel::ProgressShown(HOLQueuePanel::HeadPopulationFull(), 1.0), "held at 100 %: the bar and the note together");
	T.AssertFalse(HOLQueuePanel::ProgressShown(HOLQueuePanel::HeadPopulationFull(), 0.0), "waiting for room at 0: the note alone, in the bar's place");
	T.AssertTrue(HOLQueuePanel::NoteShown(HOLQueuePanel::HeadPopulationFull()), "the note says Population pleine");
	T.AssertFalse(HOLQueuePanel::NoteShown(HOLQueuePanel::HeadInProgress()), "no note while it builds");
	T.AssertFalse(HOLQueuePanel::NoteShown(HOLQueuePanel::HeadStarting()), "no note while it starts");
}

void Test_QueuePanelReportLineIsWhatQuickCheckReads(FUnitTest& T)
{
	TArray<FString> Two;
	Two.Add("Peasant");
	Two.Add("Peasant");
	TArray<FString> Empty;
	TArray<FString> Three;
	Three.Add("Peasant");
	Three.Add("Peasant");
	Three.Add("Hoplite");
	T.AssertEquals(HOLQueuePanel::ReportLine(Two, HOLQueuePanel::HeadInProgress(), 0.4), "2 [Peasant 0.40, Peasant waiting]", "a head at 40 %, the second waits");
	T.AssertEquals(HOLQueuePanel::ReportLine(Two, HOLQueuePanel::HeadStarting(), 0.0), "2 [Peasant starting, Peasant waiting]", "the head starts");
	T.AssertEquals(HOLQueuePanel::ReportLine(Two, HOLQueuePanel::HeadPopulationFull(), 1.0), "2 [Peasant population full, Peasant waiting]", "alert_population: held at 100 %");
	T.AssertEquals(HOLQueuePanel::ReportLine(Empty, HOLQueuePanel::HeadNone(), 0.0), "0 []", "an empty queue (quick_check: queue 0)");
	T.AssertEquals(HOLQueuePanel::ReportLine(Three, HOLQueuePanel::HeadInProgress(), 1.0), "3 [Peasant 1.00, Peasant waiting, Hoplite waiting]", "a full bar prints 1.00");
	T.AssertEquals(HOLQueuePanel::TwoDecimals(0.05), "0.05", "a leading zero in the hundredths");
	T.AssertEquals(HOLQueuePanel::TwoDecimals(0.0), "0.00", "zero");
}

void Test_QueuePanelClickLineAndRefusals(FUnitTest& T)
{
	T.AssertEquals(HOLQueuePanel::ClickLine(0), "HUD: queue item 0 clicked (cancel)", "the line quick_check waits for (waitlog cancelled follows from the controller)");
	T.AssertEquals(HOLQueuePanel::ClickLine(2), "HUD: queue item 2 clicked (cancel)", "the index is the button's");
	T.AssertTrue(HOLQueuePanel::ClickAllowed(true, true), "a shown button of a shown panel");
	T.AssertFalse(HOLQueuePanel::ClickAllowed(false, true), "the panel hidden: refused");
	T.AssertFalse(HOLQueuePanel::ClickAllowed(true, false), "the button hidden: refused");
}

// SAURON-1116e (D833): the queue's note "Population pleine" covered the full bar. The bar's right edge, computed by the rule Build lays the line out with,
// must leave the note its own room: the bar ends before the note begins, and stays wide enough to read as full.
void Test_QueuePanelNoteDoesNotCoverTheBar(FUnitTest& T)
{
	const float LineWidth = 300.0;
	const float NoteWidth = 120.0;
	float BarRight = HOLQueuePanel::BarRightEdge(LineWidth, NoteWidth, true);
	float NoteLeft = HOLQueuePanel::NoteLeftEdge(LineWidth, NoteWidth, true);
	float Gap = HOLQueuePanel::NoteGap();
	T.AssertTrue(BarRight + Gap <= NoteLeft + 0.001, "the note is shown: the full bar ends before the note begins (gap kept)");
	T.AssertTrue(BarRight >= 100.0, "the bar is still wide enough to read");
	float BarAlone = HOLQueuePanel::BarRightEdge(LineWidth, NoteWidth, false);
	T.AssertTrue(Math::IsNearlyEqual(BarAlone, LineWidth), "no note: the bar takes the whole line");
}
