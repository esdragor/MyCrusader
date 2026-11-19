// HUD H-5a (uncompiled): the alert banner's rules (HOLAlertBanner.as), in literals copied
// from the C++ screen (HOLGameScreen.cpp PresentAlert 879-905, FocusAlertSlot 958-965, HandleAlertMouseDown 937-945, ClickWidget 2836-2844) and from the C++ alert state
// (FHOLForumAlert, reached through UHOLForumAlertHolder: the state machine stays C++, the script draws it).

FHOLForumEventValue AlertBannerEv(int Kind, int Slot, int Lives)
{
	FHOLForumEventValue Event;
	Event.Kind = Kind;
	Event.Slot = Slot;
	Event.Lives = Lives;
	return Event;
}

// Slots 0 and 1 in team 1, slot 2 in team 2 (as the Victory.Messages tests).
FHOLForumScenario AlertBannerTeams()
{
	FHOLForumScenario Scenario;
	Scenario.States.Add(1);
	Scenario.States.Add(2);
	Scenario.States.Add(2);
	Scenario.Teams.Add(1);
	Scenario.Teams.Add(1);
	Scenario.Teams.Add(2);
	return Scenario;
}

void Test_AlertBannerLogLinesAreTheCppOnes(FUnitTest& T)
{
	const FString Text = "Votre forum est tombé — 1 vie";
	T.AssertEquals("Alert: " + Text + " (sound /Engine/EngineSounds/1kSineTonePing.1kSineTonePing)", HOLAlertBanner::PresentLogLine(Text, true, true), "the first banner of a burst, sound found");
	T.AssertEquals("Alert: " + Text + " (sound /Engine/EngineSounds/1kSineTonePing.1kSineTonePing, missing)", HOLAlertBanner::PresentLogLine(Text, true, false), "sound asset missing");
	T.AssertEquals("Alert: " + Text + " (no sound, its burst's played)", HOLAlertBanner::PresentLogLine(Text, false, false), "a banner of the same burst: silent");
	T.AssertEquals("HUD: alert of slot 3 clicked (camera on its forum)", HOLAlertBanner::ClickLogLine(3), "the click's line");
	T.AssertEquals("HUD: alert of slot 0 clicked (camera on its forum)", HOLAlertBanner::ClickLogLine(0), "slot 0 is a slot");
}

void Test_AlertBannerNamesAreThePiecesWithASlot(FUnitTest& T)
{
	// Through the real C++ rule: the pieces of "Le forum de <name> est tombé" carry the name's slot, the words -1.
	FHOLForumScenario Lobby = AlertBannerTeams();
	Lobby.Names.Add("Elrond");
	Lobby.Names.Add("Aragorn");
	TArray<FHOLForumPieceValue> Pieces = HOLForumMessagesRead::SplitNames(AlertBannerEv(0, 1, 1), Lobby, 0);
	int Names = 0;
	for (int Index = 0; Index < Pieces.Num(); Index++)
	{
		if (HOLAlertBanner::PieceIsName(Pieces[Index].Slot))
			Names++;
	}
	T.AssertEquals(1, Names, "one name coloured in the ally's fall");
	T.AssertFalse(HOLAlertBanner::PieceIsName(-1), "INDEX_NONE: a plain word");
	T.AssertTrue(HOLAlertBanner::PieceIsName(0), "slot 0 is a name");
}

// The view driven by the real C++ state machine (UHOLForumAlertHolder, as the C++ screen drives FHOLForumAlert), the serial counted as PresentAlert's caller would:
// a banner shown, its fade, the next waiting one replacing it with no hidden frame, then gone.
void Test_AlertBannerFollowsTheCppStateMachine(FUnitTest& T)
{
	FHOLForumScenario Teams = AlertBannerTeams();
	UHOLForumAlertHolder Alert = UHOLForumAlertHolder::Create();
	int Serial = 0;
	int ShownSerial = 0;
	bool bVisible = false;

	// Nothing yet: nothing to do.
	T.AssertEquals(HOLAlertBanner::ChangeNone(), HOLAlertBanner::Decide(Alert.GetSlot() >= 0, Serial, ShownSerial, bVisible), "no alert, no panel: nothing");

	// My forum falls (slot 0, viewer 0): shown at once, a new serial: present.
	if (Alert.ShowEvent(AlertBannerEv(0, 0, 1), Teams, 0, 0, false, 10.0))
		Serial++;
	T.AssertEquals(1, Serial, "the fall's banner was presented");
	T.AssertEquals(HOLAlertBanner::ChangePresent(), HOLAlertBanner::Decide(true, Serial, ShownSerial, bVisible), "a new serial: the banner is drawn");
	ShownSerial = Serial;
	bVisible = true;
	T.AssertEquals(HOLAlertBanner::ChangeFade(), HOLAlertBanner::Decide(true, Serial, ShownSerial, bVisible), "same serial next frame: only the opacity follows");
	T.AssertEquals(1.0, Alert.GetOpacity(10.0), "opaque at first");

	// The hold of the same forum comes 40 ms later: it waits; the banner keeps its serial (no redraw).
	T.AssertFalse(Alert.ShowEvent(AlertBannerEv(1, 1, 0), Teams, 0, 1, false, 10.04), "waits behind the shown banner");
	T.AssertEquals(HOLAlertBanner::ChangeFade(), HOLAlertBanner::Decide(true, Serial, ShownSerial, bVisible), "still the first banner");

	// 1.5 s later it replaces the first with no hidden frame: a new serial while the state still shows one.
	if (Alert.Advance(11.5, false))
		Serial++;
	T.AssertEquals(2, Serial, "the waiting alert was presented");
	T.AssertEquals(HOLAlertBanner::ChangePresent(), HOLAlertBanner::Decide(true, Serial, ShownSerial, bVisible), "replaced in place: drawn again, not hidden then shown");
	ShownSerial = Serial;
	int Slot = Alert.GetSlot();
	T.AssertEquals(1, Slot, "the banner now points at the ally's forum (the click's slot)");

	// Its 4 s over: the state shows none, the panel is up: hide.
	T.AssertTrue(Alert.Expire(15.5), "gone at 4 s");
	T.AssertEquals(HOLAlertBanner::ChangeHide(), HOLAlertBanner::Decide(false, Serial, ShownSerial, bVisible), "the state shows none, the panel is up: hide");
	bVisible = false;
	T.AssertEquals(HOLAlertBanner::ChangeNone(), HOLAlertBanner::Decide(false, Serial, ShownSerial, bVisible), "hidden, nothing more to do");
}

// The sound rule is the C++ state's (FHOLForumAlert: one sound for a burst, the first banner's): the journal line follows what the real state says, so the
// line quick_check reads (3240) is the one the C++ state implies.
void Test_AlertBannerLineFollowsTheCppSoundRule(FUnitTest& T)
{
	FHOLForumScenario Teams = AlertBannerTeams();
	UHOLForumAlertHolder Alert = UHOLForumAlertHolder::Create();
	T.AssertTrue(Alert.ShowEvent(AlertBannerEv(0, 0, 1), Teams, 0, 0, false, 10.0), "the first banner is shown at once");
	const bool bFirstSound = Alert.HasSound();
	T.AssertTrue(bFirstSound, "the first banner of a burst carries the sound");
	T.AssertEquals("Alert: x (sound /Engine/EngineSounds/1kSineTonePing.1kSineTonePing)", HOLAlertBanner::PresentLogLine("x", bFirstSound, true), "its line says the sound");
	T.AssertTrue(Alert.Expire(14.5), "gone at 4 s");
	T.AssertTrue(Alert.ShowEvent(AlertBannerEv(0, 0, 0), Teams, 0, 0, false, 40.0), "a new fall long after");
	const bool bLaterSound = Alert.HasSound();
	T.AssertTrue(bLaterSound, "a banner long after the last one is a new burst: its own sound");
}
