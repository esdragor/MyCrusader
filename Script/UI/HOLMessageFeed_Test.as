// HUD H-5b (uncompiled): the thread of messages' rules (HOLMessageFeed.as), in literals copied from the C++ screen
// (HOLGameScreen.cpp RebuildMessages 894-917, HandleMessageLineMouseDown 835-851, FocusAlertSlot 861-868, BuildLayout 2620-2635).
// The thread's own rules (Add, Expire, GetOpacity, Describe) stay C++ and keep their C++ tests.

void Test_MessageFeedOnlyAnAlertLineUnderTheMouseTakesTheClick(FUnitTest& T)
{
	TArray<bool> Hovered;
	TArray<bool> Alerts;
	// Line 0 hovered but no alert; line 1 an alert not hovered; line 2 hovered AND an alert; line 3 hovered and an alert too (the first wins).
	Hovered.Add(true);
	Hovered.Add(false);
	Hovered.Add(true);
	Hovered.Add(true);
	Alerts.Add(false);
	Alerts.Add(true);
	Alerts.Add(true);
	Alerts.Add(true);
	T.AssertEquals(2, HOLMessageFeed::HoveredAlertLine(Hovered, Alerts, 4), "the first line both hovered and carrying an alert");
	T.AssertEquals(-1, HOLMessageFeed::HoveredAlertLine(Hovered, Alerts, 2), "only lines 0 and 1 are shown: no hit (a hidden line cannot be clicked)");
	Hovered[2] = false;
	Hovered[3] = false;
	T.AssertEquals(-1, HOLMessageFeed::HoveredAlertLine(Hovered, Alerts, 4), "a hovered line without alert: the click goes through");
	TArray<bool> NoLine;
	T.AssertEquals(-1, HOLMessageFeed::HoveredAlertLine(NoLine, NoLine, 0), "an empty thread: no hit");
}

void Test_MessageFeedClickLineIsTheCppOne(FUnitTest& T)
{
	T.AssertEquals("HUD: alert of slot 2 clicked (camera on its forum)", HOLMessageFeed::ClickLogLine(2), "the click's line, word for word");
	T.AssertEquals("HUD: alert of slot 0 clicked (camera on its forum)", HOLMessageFeed::ClickLogLine(0), "slot 0 is a slot");
	T.AssertEquals(HOLAlertBanner::ClickLogLine(1), HOLMessageFeed::ClickLogLine(1), "the banner and the thread write the same line (one tool text)");
}

void Test_MessageFeedLinesAndListFollowTheThreadCount(FUnitTest& T)
{
	T.AssertFalse(HOLMessageFeed::ListShown(0), "no line: the list is collapsed");
	T.AssertTrue(HOLMessageFeed::ListShown(1), "one line: the list is up");
	T.AssertTrue(HOLMessageFeed::LineShown(0, 1), "line 0 of 1 is up");
	T.AssertFalse(HOLMessageFeed::LineShown(1, 1), "line 1 of 1 is collapsed");
	T.AssertFalse(HOLMessageFeed::LineShown(-1, 3), "no negative line");
	T.AssertTrue(HOLMessageFeed::LineShown(HOLMessageFeed::MaxLines() - 1, HOLMessageFeed::MaxLines()), "the last of the five");
	T.AssertEquals(5, HOLMessageFeed::MaxLines(), "FHOLForumThread::MaxLines");
}

void Test_MessageFeedRedrawsOnlyWhenTheThreadChanged(FUnitTest& T)
{
	T.AssertTrue(HOLMessageFeed::NeedsRedraw(0, -1), "the first frame draws");
	T.AssertFalse(HOLMessageFeed::NeedsRedraw(4, 4), "same revision: only the opacity follows the fade");
	T.AssertTrue(HOLMessageFeed::NeedsRedraw(5, 4), "a line added or expired");
}

void Test_MessageFeedClicksAndPlaceAreTheCppOnes(FUnitTest& T)
{
	T.AssertTrue(HOLMessageFeed::LineTakesClick(true), "a line carrying an alert: Visible");
	T.AssertFalse(HOLMessageFeed::LineTakesClick(false), "a plain line: HitTestInvisible");
	T.AssertTrue(HOLMessageFeed::PieceIsName(2), "a slot: a name in its colour");
	T.AssertFalse(HOLMessageFeed::PieceIsName(-1), "INDEX_NONE: a word");
	T.AssertEquals(0.0, HOLMessageFeed::TopPadding(0), "the first line touches the list's top");
	T.AssertEquals(2.0, HOLMessageFeed::TopPadding(3), "2 px between two lines");
	// The look constants (LineOpacity, PaddingX, PaddingY) are copies of BuildLayout with no logic: the capture proves them, no test here.
}

void Test_MessageFeedHitTestStaysInsideTheShortestList(FUnitTest& T)
{
	// The thread says 4 lines but the hover list holds 2 and the alert list 3: only indices present in all three can be hit.
	TArray<bool> Hovered;
	TArray<bool> Alerts;
	Hovered.Add(false);
	Hovered.Add(false);
	Alerts.Add(false);
	Alerts.Add(false);
	Alerts.Add(true);
	T.AssertEquals(-1, HOLMessageFeed::HoveredAlertLine(Hovered, Alerts, 4), "line 2 has an alert but no hover entry: no hit, no out-of-range read");
	Hovered.Add(true);
	T.AssertEquals(2, HOLMessageFeed::HoveredAlertLine(Hovered, Alerts, 4), "once line 2 has a hover entry it is hit");
	T.AssertEquals(-1, HOLMessageFeed::HoveredAlertLine(Hovered, Alerts, 0), "a count of 0 hides every line");
}
