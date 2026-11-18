// H-2: the short message's pure rules (HOLShortMessage.as), literals only. Each test says the mutation that turns it red (a red to SEE before the commit goes to main).

// Opacity: full for the first 2.5 s, then a fade to 0 over the last 0.5 s.
// Red: FadeSeconds 0.5 -> 1.0 in HOLShortMessage.as (the 2.75 s line reads 0.75 instead of 0.5) ; or Opacity returns 1.0 always.
void Test_ShortMessageOpacityFadesOverTheLastHalfSecond(FUnitTest& T)
{
	T.AssertTrue(Math::Abs(HOLShortMessage::Opacity(0.0) - 1.0) < 0.001, "full at the start");
	T.AssertTrue(Math::Abs(HOLShortMessage::Opacity(2.5) - 1.0) < 0.001, "still full when the fade begins (3 - 0.5)");
	T.AssertTrue(Math::Abs(HOLShortMessage::Opacity(2.75) - 0.5) < 0.001, "half way through the fade");
	T.AssertTrue(Math::Abs(HOLShortMessage::Opacity(3.0) - 0.0) < 0.001, "gone at 3 s");
}

// The message lasts 3 s of steps and is then gone, its key too (the Describe line says nothing after).
// Red: Seconds 3.0 -> 4.0 (still active after 3 s) ; or Advance leaves State.Key set (ShownKey keeps the key).
void Test_ShortMessageLastsThreeSecondsThenIsGone(FUnitTest& T)
{
	FHOLShortMessageState State;
	T.AssertFalse(HOLShortMessage::Advance(State, 0.1), "nothing to draw before a message");
	HOLShortMessage::Show(State, "NotEnoughWood");
	T.AssertEquals(HOLShortMessage::ShownKey(State), "NotEnoughWood", "the key is shown");
	T.AssertTrue(HOLShortMessage::Advance(State, 1.5), "1.5 s: still there");
	T.AssertTrue(HOLShortMessage::Advance(State, 1.4), "2.9 s: still there");
	const FString StillShown = HOLShortMessage::ShownKey(State);
	T.AssertEquals(StillShown, "NotEnoughWood", "the key lasts as long as the message");
	T.AssertFalse(HOLShortMessage::Advance(State, 0.2), "3.1 s: over");
	const FString AfterKey = HOLShortMessage::ShownKey(State);
	T.AssertEquals(AfterKey, "", "no key once over");
	T.AssertFalse(HOLShortMessage::Advance(State, 0.2), "and nothing to draw after");
}

// Priority: the latest message replaces the one on the screen and starts its 3 s again (C++ ShowMessage).
// Red: Show keeps the old Age (State.Age = 0.0 line removed): the second message ends 2.9 s sooner and the last check fails.
void Test_ShortMessageLatestWinsAndRestartsTheClock(FUnitTest& T)
{
	FHOLShortMessageState State;
	HOLShortMessage::Show(State, "NotEnoughWood");
	HOLShortMessage::Advance(State, 2.9);
	HOLShortMessage::Show(State, "QueueFull");
	T.AssertEquals(HOLShortMessage::ShownKey(State), "QueueFull", "the new message replaces the old one");
	T.AssertTrue(HOLShortMessage::Advance(State, 2.9), "its own 3 s: still there 2.9 s after the new one");
	T.AssertFalse(HOLShortMessage::Advance(State, 0.2), "then over");
}

// Population held: said once when the production becomes held, not at each reading, and again after the production has gone on.
// Red: ShouldShowPopulationHeld ignores bWasHeld (returns bHeldNow): the second reading says it again ; or Watch.bWasHeld never reset: the last check fails.
void Test_PopulationHeldIsSaidOncePerBlock(FUnitTest& T)
{
	FHOLPopulationHeldWatch Watch;
	T.AssertFalse(HOLShortMessage::ShouldShowPopulationHeld(Watch, false), "not held: nothing");
	T.AssertTrue(HOLShortMessage::ShouldShowPopulationHeld(Watch, true), "becomes held: said");
	T.AssertFalse(HOLShortMessage::ShouldShowPopulationHeld(Watch, true), "still held: not again");
	T.AssertFalse(HOLShortMessage::ShouldShowPopulationHeld(Watch, true), "still held: not again");
	T.AssertFalse(HOLShortMessage::ShouldShowPopulationHeld(Watch, false), "the unit came out: nothing");
	T.AssertTrue(HOLShortMessage::ShouldShowPopulationHeld(Watch, true), "held again later: said again");
}

// The journal lines are the C++'s, word for word (quick_check: 'HUD message: NotEnoughWood "Pas assez de bois"', "HUD message: PopulationFull",
// the scenario alert_population: "production held at 100 %").
// Red: MessageLogLine without the quotes around the text, or HeldLogLine with "100 %%".
void Test_ShortMessageJournalLinesAreTheCppOnes(FUnitTest& T)
{
	T.AssertEquals(HOLShortMessage::MessageLogLine("NotEnoughWood", "Pas assez de bois"), "HUD message: NotEnoughWood \"Pas assez de bois\"", "message line");
	T.AssertEquals(HOLShortMessage::HeldLogLine(), "HUD: production held at 100 % (population full), line to the thread", "held line");
	const FString Max = HOLShortMessage::PopulationMaxText().ToString();
	T.AssertTrue(Max.Contains("construisez des maisons"), "the population full text is the C++'s source text");
}

// The place: BottomGap over the box under it, never under the window's bottom.
// Red: BottomPadding returns ViewHeight - AnchorTop (no gap: 6 less) ; or without the Max (an anchor under the window gives a negative padding).
void Test_ShortMessagePlaceIsAboveTheQueueOrGrid(FUnitTest& T)
{
	T.AssertTrue(Math::Abs(HOLShortMessage::BottomPadding(1080.0, 880.0) - 206.0) < 0.001, "queue top at 880 in a 1080 window: 200 + 6");
	T.AssertTrue(Math::Abs(HOLShortMessage::BottomPadding(1080.0, 1080.0) - 6.0) < 0.001, "anchor at the bottom: the gap alone");
	T.AssertTrue(HOLShortMessage::BottomPadding(1080.0, 1200.0) >= 0.0, "anchor under the window: not negative");
}

// Fidelity (D839): the C++ UpdateMessage returned at once for an empty key, so a message WITHOUT key never aged nor ended; the script does the same.
// Red: remove the "State.Key.IsEmpty()" line of Advance (the keyless message then ends after 3 s and the second check fails).
void Test_ShortMessageWithoutKeyNeverEnds(FUnitTest& T)
{
	FHOLShortMessageState State;
	HOLShortMessage::Show(State, "");
	T.AssertTrue(HOLShortMessage::Advance(State, 10.0), "no key: still there after 10 s (the C++ never ended it)");
	T.AssertTrue(HOLShortMessage::Advance(State, 100.0), "and after 110 s");
	const FString Shown = HOLShortMessage::ShownKey(State);
	T.AssertEquals(Shown, "", "no key to show in Describe");
	HOLShortMessage::Show(State, "NotEnoughWood");
	T.AssertTrue(HOLShortMessage::Advance(State, 2.9), "a keyed message afterwards lasts its 3 s");
	T.AssertFalse(HOLShortMessage::Advance(State, 0.2), "then ends");
}