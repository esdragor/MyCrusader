// D807 chat input field, pure rule (HOLChatInput.as). Every case ends in HOLChat::Recipients (Script\Chat\HOLChatRules.as): the readers of what the field
// would send, computed by the rule of the chat, never typed by hand. Four slots: 0 and 1 allies (team 0), 2 and 3 their enemies (team 1); the sender is slot 0.
// Each test says the mutation that turns it red.

TArray<int> ChatInputTeams()
{
	TArray<int> Teams;
	Teams.Add(0);
	Teams.Add(0);
	Teams.Add(1);
	Teams.Add(1);
	return Teams;
}

TArray<int> ChatInputFreeForAllTeams()
{
	TArray<int> Teams;
	Teams.Add(0);
	Teams.Add(1);
	Teams.Add(2);
	Teams.Add(3);
	return Teams;
}

TArray<bool> ChatInputSpectators()
{
	TArray<bool> Spectators;
	Spectators.Add(false);
	Spectators.Add(false);
	Spectators.Add(false);
	Spectators.Add(false);
	return Spectators;
}

// The slots the field's current channel reaches for the sender 0, Chosen = the slot picked in the Diplomacy panel (-1: none).
TArray<int> ChatInputReaders(const FHOLChatInputState& State, int Chosen, bool bFreeForAll)
{
	return HOLChat::Recipients(State.Channel, 0, HOLChatInput::TargetSlot(State, Chosen), bFreeForAll ? ChatInputFreeForAllTeams() : ChatInputTeams(), ChatInputSpectators(), false);
}

// Enter opens the field on Allies; Enter again with a text sends it: the sender and his ally read it, no enemy, and the field closes.
// Red: PressEnter does nothing (stub) ; or opens on All ; or does not close after the send.
void Test_ChatInputEnterOpensOnAlliesThenSends(FUnitTest& T)
{
	FHOLChatInputState State;
	const int First = HOLChatInput::PressEnter(State, "", true, false);
	T.AssertEquals(First, HOLChatInput::Opened, "Enter on a closed field opens it");
	T.AssertTrue(State.bOpen, "the field is open");
	T.AssertEquals(State.Channel, HOLChat::Allies, "with an ally it opens on Allies");
	const int Second = HOLChatInput::PressEnter(State, "attack the north gate", true, false);
	T.AssertEquals(Second, HOLChatInput::Send, "Enter with a text sends");
	T.AssertFalse(State.bOpen, "the field closes after the send");
	TArray<int> To = ChatInputReaders(State, -1, false);
	T.AssertTrue(To.Contains(0) && To.Contains(1), "Allies: the sender and his ally read it");
	T.AssertFalse(To.Contains(2) || To.Contains(3), "Allies: no enemy reads it");
}

// Without an ally (free-for-all) Enter opens on All: everybody reads what is sent.
// Red: opens on Allies whatever bHasAlly says (the message would reach the sender alone).
void Test_ChatInputWithoutAllyOpensOnAll(FUnitTest& T)
{
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", false, false);
	T.AssertEquals(State.Channel, HOLChat::All, "no ally: the field opens on All");
	HOLChatInput::PressEnter(State, "hello", false, false);
	TArray<int> To = ChatInputReaders(State, -1, true);
	T.AssertEquals(To.Num(), 4, "All in a free-for-all: the four slots read it");
}

// Tab walks Allies > All > Private > Allies; each step changes who reads.
// Red: Tab does nothing (stub) ; or a different order ; or Private ignores the chosen player.
void Test_ChatInputTabCyclesAlliesAllPrivate(FUnitTest& T)
{
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", true, true);
	TArray<int> OnAllies = ChatInputReaders(State, 3, false);
	T.AssertEquals(OnAllies.Num(), 2, "Allies reach 2 slots");
	T.AssertTrue(HOLChatInput::PressTab(State, true, true), "Tab on an open field changes the channel");
	TArray<int> OnAll = ChatInputReaders(State, 3, false);
	T.AssertEquals(OnAll.Num(), 4, "then All: 4 slots");
	T.AssertTrue(HOLChatInput::PressTab(State, true, true), "Tab again");
	TArray<int> OnPrivate = ChatInputReaders(State, 3, false);
	T.AssertEquals(OnPrivate.Num(), 2, "then Private: the sender and the chosen player");
	T.AssertTrue(OnPrivate.Contains(3) && !OnPrivate.Contains(1), "the chosen player (slot 3) reads it, the ally does not");
	T.AssertTrue(HOLChatInput::PressTab(State, true, true), "Tab again");
	T.AssertEquals(State.Channel, HOLChat::Allies, "back to Allies");
}

// Private only with a chosen player: Tab skips it without one; All stays when alone without a chosen player.
// Red: Tab lands on Private with no chosen player (the message would reach the sender alone) ; or Allies is offered to a player without an ally.
void Test_ChatInputPrivateOnlyWithAChosenPlayer(FUnitTest& T)
{
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", true, false);
	HOLChatInput::PressTab(State, true, false);
	T.AssertEquals(State.Channel, HOLChat::All, "Allies > All");
	HOLChatInput::PressTab(State, true, false);
	T.AssertEquals(State.Channel, HOLChat::Allies, "no chosen player: All > Allies, Private skipped");
	FHOLChatInputState Alone;
	HOLChatInput::PressEnter(Alone, "", false, false);
	HOLChatInput::PressTab(Alone, false, false);
	T.AssertEquals(Alone.Channel, HOLChat::All, "alone and nobody chosen: Tab stays on All");
	T.AssertEquals(HOLChatInput::TargetSlot(State, 3), -1, "off Private there is no target slot");
	State.Channel = HOLChat::Private;
	T.AssertEquals(HOLChatInput::TargetSlot(State, 3), 3, "on Private the target is the chosen slot");
	T.AssertEquals(HOLChatInput::TargetSlot(State, -1), -1, "on Private with nobody chosen: no target");
}

// Escape closes the field without sending; the next Enter opens it again (it does not send).
// Red: Escape does nothing ; or leaves bOpen set.
void Test_ChatInputEscapeClosesWithoutSending(FUnitTest& T)
{
	FHOLChatInputState State;
	T.AssertFalse(HOLChatInput::PressEscape(State), "Escape on a closed field changes nothing");
	HOLChatInput::PressEnter(State, "", true, false);
	T.AssertTrue(HOLChatInput::PressEscape(State), "Escape on an open field closes it");
	T.AssertFalse(State.bOpen, "closed");
	T.AssertEquals(HOLChatInput::PressEnter(State, "forgotten text", true, false), HOLChatInput::Opened, "the next Enter opens, it does not send");
	FHOLChatInputState Closed;
	T.AssertFalse(HOLChatInput::PressTab(Closed, true, true), "Tab on a closed field does nothing");
}

// Enter on an empty (or blank) field closes it and sends nothing.
// Red: sends the empty text (the gate would answer Empty) ; or keeps the field open.
void Test_ChatInputEnterOnABlankFieldDismisses(FUnitTest& T)
{
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", true, false);
	T.AssertEquals(HOLChatInput::PressEnter(State, "   ", true, false), HOLChatInput::Dismissed, "a blank text is not sent");
	T.AssertFalse(State.bOpen, "the field is closed");
}

// The audience never widens silently: the ally dies while the field is on Allies, Enter does NOT send on All; the field falls back to All
// and the player presses Enter again. Same for Private when the chosen player is gone (Diplomacy panel changed).
// Red: PressEnter sends on a channel nobody would read (None expected) ; or sends the Private text publicly.
void Test_ChatInputLostAudienceDoesNotWidenSilently(FUnitTest& T)
{
	FHOLChatInputState State;
	HOLChatInput::PressEnter(State, "", true, false);
	T.AssertEquals(HOLChatInput::PressEnter(State, "secret plan", false, false), HOLChatInput::None, "the ally is gone: nothing is sent");
	T.AssertTrue(State.bOpen, "the field stays open");
	T.AssertEquals(State.Channel, HOLChat::All, "it falls back to All, visibly");
	T.AssertEquals(HOLChatInput::PressEnter(State, "secret plan", false, false), HOLChatInput::Send, "a second Enter sends on All");
	FHOLChatInputState PrivateState;
	PrivateState.bOpen = true;
	PrivateState.Channel = HOLChat::Private;
	T.AssertEquals(HOLChatInput::PressEnter(PrivateState, "for you", true, false), HOLChatInput::None, "private without a chosen player: nothing is sent");
	T.AssertEquals(PrivateState.Channel, HOLChat::All, "private without a chosen player falls back to All, not sent");
}
