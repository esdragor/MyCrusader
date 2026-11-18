// H-1: the wait and absence banners' rules (HOLWaitBanner.as), pure, in literals: when each is shown, its text and countdown, the signature that
// decides the journal line, and the C++ screen's journal lines word for word ("Wait banner: shown, ...", "Absence banner: hidden"...).

void Test_WaitBannerHiddenUnlessWaitingForNamedPlayers(FUnitTest& T)
{
	TArray<FString> Missing;
	Missing.Add("Bob");
	TArray<FString> None;
	const FHOLWaitBannerView NotWaiting = HOLWaitBanner::WaitView(false, Missing, 90);
	T.AssertFalse(NotWaiting.bVisible, "the match is not waiting: no banner, even with a name left in the list");
	T.AssertEquals(NotWaiting.Describe, "hidden", "line");
	const FHOLWaitBannerView NobodyMissing = HOLWaitBanner::WaitView(true, None, 90);
	T.AssertFalse(NobodyMissing.bVisible, "waiting but nobody missing: no banner");
	const FHOLWaitBannerView Waiting = HOLWaitBanner::WaitView(true, Missing, 90);
	T.AssertTrue(Waiting.bVisible, "waiting for Bob: the banner is on the screen");
}

void Test_WaitBannerTextNamesPlayersAndCountsDown(FUnitTest& T)
{
	TArray<FString> Missing;
	Missing.Add("Bob");
	const FHOLWaitBannerView One = HOLWaitBanner::WaitView(true, Missing, 125);
	T.AssertEquals(One.Text, "En attente de Bob (2:05)", "the text of the C++ screen, the countdown as m:ss");
	T.AssertEquals(One.Describe, "wait Bob 2:05", "line");
	Missing.Add("Carol");
	const FHOLWaitBannerView Two = HOLWaitBanner::WaitView(true, Missing, 7);
	T.AssertEquals(Two.Text, "En attente de Bob, Carol (0:07)", "several players are joined by a comma, seconds padded to two digits");
	T.AssertEquals(Two.Signature, "Bob, Carol", "the journal line follows the players, not the clock");
	const FHOLWaitBannerView Zero = HOLWaitBanner::WaitView(true, Missing, 0);
	T.AssertEquals(Zero.Text, "En attente de Bob, Carol (0:00)", "the wait's end reached: 0:00, never negative");
}

void Test_WaitBannerChangeLogsOncePerSignature(FUnitTest& T)
{
	TArray<FString> Missing;
	Missing.Add("Bob");
	const FHOLWaitBannerView First = HOLWaitBanner::WaitView(true, Missing, 100);
	const FHOLWaitBannerView Tick = HOLWaitBanner::WaitView(true, Missing, 99);
	T.AssertEquals(HOLWaitBanner::Change("", First), HOLWaitBanner::ChangeShown(), "first sight: shown");
	T.AssertEquals(HOLWaitBanner::Change("Bob", Tick), HOLWaitBanner::ChangeNone(), "a countdown second does not log again");
	Missing.Add("Carol");
	const FHOLWaitBannerView More = HOLWaitBanner::WaitView(true, Missing, 98);
	T.AssertEquals(HOLWaitBanner::Change("Bob", More), HOLWaitBanner::ChangeShown(), "who is missing changed: logged again");
	TArray<FString> None;
	const FHOLWaitBannerView Gone = HOLWaitBanner::WaitView(false, None, 0);
	T.AssertEquals(HOLWaitBanner::Change("Bob, Carol", Gone), HOLWaitBanner::ChangeHidden(), "the wait is over: hidden, once");
	T.AssertEquals(HOLWaitBanner::Change("", Gone), HOLWaitBanner::ChangeNone(), "already hidden: nothing");
	const FString ShownLine = HOLWaitBanner::WaitLogLine(HOLWaitBanner::ChangeShown(), First);
	T.AssertEquals(ShownLine, "Wait banner: shown, En attente de Bob (1:40)", "the journal line quick_check net reads");
	T.AssertEquals(HOLWaitBanner::WaitLogLine(HOLWaitBanner::ChangeHidden(), Gone), "Wait banner: hidden", "the journal line when the match starts");
	T.AssertEquals(HOLWaitBanner::WaitLogLine(HOLWaitBanner::ChangeNone(), Tick), "", "no change, no line");
}

void Test_AbsenceBannerOneLinePerNoticeAndSignature(FUnitTest& T)
{
	TArray<FString> NoLines;
	TArray<FString> Names;
	TArray<bool> Back;
	const FHOLWaitBannerView Quiet = HOLWaitBanner::AbsenceView(NoLines, Names, Back);
	T.AssertFalse(Quiet.bVisible, "no line: no banner");
	TArray<FString> Lines;
	Lines.Add("Bob : connexion perdue — 2:41");
	Names.Add("Bob");
	Back.Add(false);
	const FHOLWaitBannerView Away = HOLWaitBanner::AbsenceView(Lines, Names, Back);
	T.AssertTrue(Away.bVisible, "Bob is gone: the banner is on the screen");
	T.AssertEquals(Away.Signature, "1;Bob-", "who is gone, not the countdown");
	T.AssertEquals(Away.Describe, "absence 1", "line");
	Lines.Add("Eve : de retour");
	Names.Add("Eve");
	Back.Add(true);
	const FHOLWaitBannerView Two = HOLWaitBanner::AbsenceView(Lines, Names, Back);
	T.AssertEquals(Two.Signature, "2;Bob-;Eve+", "a player back is a + in the signature");
	T.AssertEquals(Two.Text, "Bob : connexion perdue — 2:41\nEve : de retour", "one line per notice, joined by a line break");
}

void Test_AbsenceBannerJournalLines(FUnitTest& T)
{
	TArray<FString> Lines;
	Lines.Add("Bob : connexion perdue — 0:09");
	TArray<FString> Names;
	Names.Add("Bob");
	TArray<bool> Back;
	Back.Add(false);
	const FHOLWaitBannerView Away = HOLWaitBanner::AbsenceView(Lines, Names, Back);
	T.AssertEquals(HOLWaitBanner::AbsenceLogLine(HOLWaitBanner::ChangeShown(), Away), "Absence banner: shown, Bob : connexion perdue — 0:09", "the line lobby_match reads");
	Lines.Add("Eve : de retour");
	Names.Add("Eve");
	Back.Add(true);
	const FHOLWaitBannerView Two = HOLWaitBanner::AbsenceView(Lines, Names, Back);
	T.AssertEquals(HOLWaitBanner::AbsenceLogLine(HOLWaitBanner::ChangeShown(), Two), "Absence banner: shown, Bob : connexion perdue — 0:09 | Eve : de retour", "several lines in one, separated by ' | '");
	T.AssertEquals(HOLWaitBanner::Change("1;Bob-", Away), HOLWaitBanner::ChangeNone(), "a countdown second: no new line");
	T.AssertEquals(HOLWaitBanner::Change("1;Bob-", Two), HOLWaitBanner::ChangeShown(), "a player came back: a new line");
	TArray<FString> NoLines;
	TArray<FString> NoNames;
	TArray<bool> NoBack;
	const FHOLWaitBannerView Gone = HOLWaitBanner::AbsenceView(NoLines, NoNames, NoBack);
	T.AssertEquals(HOLWaitBanner::Change("2;Bob-;Eve+", Gone), HOLWaitBanner::ChangeHidden(), "every notice over: hidden, once");
	T.AssertEquals(HOLWaitBanner::AbsenceLogLine(HOLWaitBanner::ChangeHidden(), Gone), "Absence banner: hidden", "the journal line lobby_match waits for");
}
