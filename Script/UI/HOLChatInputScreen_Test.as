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
	T.AssertEquals(OnAllies, "Allies", "opened on Allies");
	HOLChatInput::PressTab(State, true, true);
	const FString OnAll = HOLChatInputScreen::ChannelMark(State.Channel, "Bob");
	T.AssertEquals(OnAll, "Tous", "Tab: All");
	HOLChatInput::PressTab(State, true, true);
	const FString OnPrivate = HOLChatInputScreen::ChannelMark(State.Channel, "Bob");
	T.AssertEquals(OnPrivate, "Bob", "Tab again: the chosen player's name");
	const FString Line = HOLChatInputScreen::MarkLine(State.Channel, "Bob");
	T.AssertEquals(Line, "Bob : ", "the line shown before the text");
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
