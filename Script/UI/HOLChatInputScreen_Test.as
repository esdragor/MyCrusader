// D807 chat input screen, pure part (HOLChatInputScreen.as): the channel mark shown in the field and the text that leaves it.
// The channel comes from the real rule (HOLChatInput state walked by Enter / Tab), the length limit from HOLChat::MaxTextLength: nothing typed by hand.
// Each test says the mutation that turns it red.

FString ChatScreenText(int Length)
{
	FString Text = "";
	for (int I = 0; I < Length; ++I)
	{
		Text += "a";
	}
	return Text;
}

// Enter opens on Allies, Tab goes to All, Tab again to Private: the field's mark follows ("Allies", "Tous", the chosen player's name).
// Red: the stub returns an empty mark ; or Private shows "Tous" instead of the player ; or a mark is swapped.
void Test_ChatInputScreenMarkFollowsTheChannel(FUnitTest& T)
{
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", true, true);
	const FString OnAllies = HOLChatInputScreen::ChannelMark(State.Channel, "Bob");
	// The same tag as the thread's own line (HOLForumMessages.cpp:302, LOCTEXT ChatAllies "(Alliés) " minus its trailing space): RECETTE-1120sa saw "Allies".
	T.AssertEquals(OnAllies, "(Alliés)", "opened on Allies: the mark reads (Alliés), as the thread does");
	HOLChatInput::PressTab(State, true, true);
	const FString OnAll = HOLChatInputScreen::ChannelMark(State.Channel, "Bob");
	T.AssertEquals(OnAll, "Tous", "Tab: All");
	HOLChatInput::PressTab(State, true, true);
	const FString OnPrivate = HOLChatInputScreen::ChannelMark(State.Channel, "Bob");
	T.AssertEquals(OnPrivate, "Bob", "Tab again: the chosen player's name");
	const FString Line = HOLChatInputScreen::MarkLine(State.Channel, "Bob");
	T.AssertEquals(Line, "Bob : ", "the line shown before the text");
}

// ARWEN-1120pj (RECETTE-1120pa: after a send the 2nd Enter did not reopen the field): the closed field reopens on the next Enter, a fraction of a second
// after the commit of the send (the scenario's 2nd Enter comes ~0.2 s later), and still refuses the polled Enter of the SAME press (same instant).
// Red: the stub EnterOpensField returns false ; or the guard is longer than the pause of a real second press ; or the guard is gone (same instant opens).
void Test_ChatInputScreenSecondEnterReopensAfterSend(FUnitTest& T)
{
	FHOLChatInputState State;
	const int First = HOLChatInput::PressEnter(State, "", true, false);
	T.AssertEquals(First, HOLChatInput::Opened, "first Enter opens");
	const int Second = HOLChatInput::PressEnter(State, "salut les allies", true, false);
	T.AssertEquals(Second, HOLChatInput::Send, "second Enter sends and closes");
	const float CommitTime = 100.0;
	const bool bSamePress = HOLChatInputScreen::EnterOpensField(State.bOpen, CommitTime, CommitTime);
	T.AssertFalse(bSamePress, "the polled Enter of the same press does not reopen the field it just sent from");
	const bool bNextPress = HOLChatInputScreen::EnterOpensField(State.bOpen, CommitTime + 0.2, CommitTime);
	T.AssertTrue(bNextPress, "the next Enter, 0.2 s after the send, reopens the field");
	const int Third = HOLChatInput::PressEnter(State, "", true, false);
	T.AssertEquals(Third, HOLChatInput::Opened, "the rule opens the closed field again");
	const bool bWhileOpen = HOLChatInputScreen::EnterOpensField(State.bOpen, CommitTime + 5.0, CommitTime);
	T.AssertFalse(bWhileOpen, "an open field never 'reopens'");
}

// Without an ally the field opens on All: the mark says "Tous", never "Allies" (nobody would read it).
// Red: the mark ignores the state's channel and always says Allies.
void Test_ChatInputScreenMarkWithoutAllyIsAll(FUnitTest& T)
{
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", false, false);
	const FString Line = HOLChatInputScreen::MarkLine(State.Channel, "");
	T.AssertEquals(Line, "Tous : ", "no ally: Tous");
}

// The text that leaves is trimmed; a blank one is refused (Enter on it dismisses the field, HOLChatInput).
// Red: the stub sends the text untrimmed and accepts a blank.
void Test_ChatInputScreenSendsTrimmedRefusesBlank(FUnitTest& T)
{
	const FString Sent = HOLChatInputScreen::SendText("   attack the gate  ");
	T.AssertEquals(Sent, "attack the gate", "trimmed");
	const int Blank = HOLChatInputScreen::SendRefusal("    ");
	T.AssertEquals(Blank, HOLChatInputScreen::Blank, "whitespace only is refused");
	const int Fine = HOLChatInputScreen::SendRefusal("  hello ");
	T.AssertEquals(Fine, HOLChatInputScreen::Ok, "a text is accepted");
}

// The board's length (HOLChat::MaxTextLength) is the limit: exactly that long goes, one more is refused and the field keeps its text.
// Red: no limit (stub) ; or off by one at the boundary.
void Test_ChatInputScreenRefusesTooLong(FUnitTest& T)
{
	const int AtLimit = HOLChatInputScreen::SendRefusal(ChatScreenText(HOLChat::MaxTextLength));
	T.AssertEquals(AtLimit, HOLChatInputScreen::Ok, "exactly the limit goes");
	const int Over = HOLChatInputScreen::SendRefusal(ChatScreenText(HOLChat::MaxTextLength + 1));
	T.AssertEquals(Over, HOLChatInputScreen::TooLong, "one more is refused");
}

// The stances of a book, read both ways for every slot, as the game screen reads them (HOLDiplomacyRead::StanceOf).
bool ChatScreenHasAlly(FHOLDiplomacyStances& Book, int Viewer, int Slots)
{
	TArray<int> Mine;
	TArray<int> Theirs;
	TArray<bool> There;
	for (int Slot = 0; Slot < Slots; Slot++)
	{
		Mine.Add(Viewer >= 0 ? HOLDiplomacyRead::StanceOf(Book, Viewer, Slot) : HOLDiplomacyRead::GetStanceEnemy());
		Theirs.Add(Viewer >= 0 ? HOLDiplomacyRead::StanceOf(Book, Slot, Viewer) : HOLDiplomacyRead::GetStanceEnemy());
		There.Add(true);
	}
	return HOLChatInputScreen::HasAlly(Viewer, Mine, Theirs, There);
}

FHOLDiplomacyStances ChatScreenBook(int Slots)
{
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < Slots; Slot++)
	{
		NoAI.Add(false);
	}
	return HOLDiplomacyRead::NewBook(Slots, NoAI);
}

// An ally exists only when BOTH sides wrote Ally (D369): one side alone, neutrals and enemies are not allies; a spectator has none.
// Red: the stub says false always ; or one-sided Ally counts ; or the viewer counts as its own ally ; or a spectator gets one.
void Test_ChatInputScreenHasAllyReadsTheBook(FUnitTest& T)
{
	FHOLDiplomacyStances Book = ChatScreenBook(4);
	const bool Alone = ChatScreenHasAlly(Book, 0, 4);
	T.AssertFalse(Alone, "a fresh book: nobody is an ally");
	HOLDiplomacyRead::SetStance(Book, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	const bool OneSided = ChatScreenHasAlly(Book, 0, 4);
	T.AssertFalse(OneSided, "only the viewer offered: not an ally yet");
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	const bool Allied = ChatScreenHasAlly(Book, 0, 4);
	T.AssertTrue(Allied, "both sides Ally: an ally");
	const bool SeenByTheOther = ChatScreenHasAlly(Book, 1, 4);
	T.AssertTrue(SeenByTheOther, "and the other sees one too");
	const bool Third = ChatScreenHasAlly(Book, 2, 4);
	T.AssertFalse(Third, "a third player outside the alliance has none");
	const bool Spectator = ChatScreenHasAlly(Book, -1, 4);
	T.AssertFalse(Spectator, "a spectator has no ally");
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceNeutral());
	const bool Cooled = ChatScreenHasAlly(Book, 0, 4);
	T.AssertFalse(Cooled, "the other went Neutral: no ally");
}

// The field opens on Allies when the screen finds an ally, on All otherwise: the whole chain book -> HasAlly -> PressEnter -> mark.
// Red: the stub's false opens on Tous even with an ally.
void Test_ChatInputScreenOpensOnAlliesWithAnAlly(FUnitTest& T)
{
	FHOLDiplomacyStances Book = ChatScreenBook(3);
	HOLDiplomacyRead::SetStance(Book, 0, 2, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, 2, 0, HOLDiplomacyRead::GetStanceAlly());
	const bool bAlly = ChatScreenHasAlly(Book, 0, 3);
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", bAlly, false);
	T.AssertEquals(State.Channel, HOLChat::Allies, "an ally: the field opens on Allies");
	const FString Line = HOLChatInputScreen::MarkLine(State.Channel, "");
	T.AssertEquals(Line, "(Alliés) : ", "and says so");
	FHOLChatInputState Lonely;
	const bool bNone = ChatScreenHasAlly(Book, 1, 3);
	HOLChatInput::PressEnter(Lonely, "", bNone, false);
	T.AssertEquals(Lonely.Channel, HOLChat::All, "no ally: the field opens on All");
}


// A refused whisper (unknown name / no text) shows a line, and the parse result is the one computed by ParseWhisper on a real text, not typed by hand.
// Red: both refusals share one text, or an accepted whisper shows a refusal.
void Test_ChatInputScreenWhisperRefusalTexts(FUnitTest& T)
{
	TArray<FString> Names;
	Names.Add("Aragorn");
	TArray<int> Slots;
	Slots.Add(1);
	int Slot = -1;
	FString Body;
	const FString Unknown = HOLChatInputScreen::WhisperRefusalText(HOLChatInput::ParseWhisper("/w Gimli hi", Names, Slots, Slot, Body));
	const FString Empty = HOLChatInputScreen::WhisperRefusalText(HOLChatInput::ParseWhisper("/w Aragorn", Names, Slots, Slot, Body));
	const FString Fine = HOLChatInputScreen::WhisperRefusalText(HOLChatInput::ParseWhisper("/w Aragorn hi", Names, Slots, Slot, Body));
	T.AssertFalse(Unknown.IsEmpty(), "an unknown name shows a refusal");
	T.AssertFalse(Empty.IsEmpty(), "no text shows a refusal");
	T.AssertTrue(Unknown != Empty, "the two refusals say different things");
	T.AssertTrue(Fine.IsEmpty(), "an accepted whisper shows no refusal");
}

// GLORFINDEL-1120sa (RECETTE-1120pb: after a send the 2nd Enter reached the game as `Key pressed: Enter (focus: none)` and never reopened the field): the
// send's own Close() leaves the collapsed box with the Slate focus, so the viewport must get the keyboard back or the polled Enter (KeyPressed) never comes.
// The chain: the field opens (Enter), a text is sent (Enter), the field closes ; the focus must have been handed back to the game viewport once, and a later
// Enter heard with no focus (KeyPressed) must be accepted by the real rule (a pause longer than the guard after the commit).
// Red: Close() does not hand the focus back (stub: FocusReturns stays 0).
void Test_ChatInputScreenCloseAfterSendReturnsFocusToGame(FUnitTest& T)
{
	UHOLChatInputScreen Chat = Cast<UHOLChatInputScreen>(NewObject(GetTransientPackage(), UHOLChatInputScreen));
	const int Opened = HOLChatInput::PressEnter(Chat.State, "", false, false);
	T.AssertEquals(Opened, HOLChatInput::Opened, "first Enter opens the field");
	const int Sent = HOLChatInput::PressEnter(Chat.State, "salut", false, false);
	T.AssertEquals(Sent, HOLChatInput::Send, "second Enter sends and closes the state");
	Chat.Close();
	T.AssertEquals(Chat.FocusReturns, 1, "the closed field handed the keyboard focus back to the game viewport");
	const bool bNextEnter = HOLChatInputScreen::EnterOpensField(Chat.State.bOpen, Chat.LastCommitTime + 0.2, Chat.LastCommitTime);
	T.AssertTrue(bNextEnter, "the next Enter, heard with no focus, reopens the field");
}

// GLORFINDEL-1120sc (RECETTE-1120sa chat_input: after a click on the player 2 line, Enter + Tab Tab left the field on Allies with an empty box): the player's
// whole gesture on the real chain: the panel's chosen slot (DipMessageSlot = 1, the players the field can name = {1}) becomes the private target through
// ChosenTarget, Enter opens on Allies, Tab Tab reaches Private, the mark is the player's name and the message is routed to that slot.
// Red: ChosenTarget ignores the chosen slot (stub returns -1: Tab then skips Private and the field stays on Allies/All).
void Test_ChatInputScreenClickedPlayerThenTabTabIsPrivate(FUnitTest& T)
{
	TArray<int> Slots;
	Slots.Add(1);
	const int Chosen = HOLChatInputScreen::ChosenTarget(Slots, 1);
	T.AssertEquals(Chosen, 1, "the clicked player of the match is the private target");
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", true, Chosen >= 0);
	T.AssertEquals(State.Channel, HOLChat::Allies, "Enter opens on Allies");
	HOLChatInput::PressTab(State, true, Chosen >= 0);
	HOLChatInput::PressTab(State, true, Chosen >= 0);
	T.AssertEquals(State.Channel, HOLChat::Private, "Tab Tab: Private");
	const FString Line = HOLChatInputScreen::MarkLine(State.Channel, "Joueur 2");
	T.AssertEquals(Line, "Joueur 2 : ", "the mark is the player's name");
	const int Target = HOLChatInput::TargetSlot(State, Chosen);
	T.AssertEquals(Target, 1, "the message goes to the clicked slot");
	TArray<int> Others;
	Others.Add(2);
	const int NotInMatch = HOLChatInputScreen::ChosenTarget(Others, 1);
	T.AssertEquals(NotInMatch, -1, "a chosen player who left the match is no target");
}