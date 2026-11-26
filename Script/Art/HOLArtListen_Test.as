// AR6-e: the pure rules of the listening table (Script\Art\HOLArtListen.as).

// The five slots of the art asset, in the order hol.Art.Listen writes them.
void Test_ArtListenSlotsInOrder(FUnitTest& T)
{
	TArray<FName> Slots = HOLArtListen::Slots();
	T.AssertEquals(5, Slots.Num(), "five sound slots");
	if (Slots.Num() != 5)
		return;
	T.AssertTrue(Slots[0] == n"Selection" && Slots[1] == n"Order" && Slots[2] == n"Attack" && Slots[3] == n"Death" && Slots[4] == n"Construction",
		"Selection, Order, Attack, Death, Construction");
}

// Which slot the game rings for which family (where the code raises it: building selection, unit orders, blows and tower volleys,
// bodies, foundation laid and building finished). A unit's selection has no sound hook in the game: never played.
void Test_ArtListenSlotPlayedByFamily(FUnitTest& T)
{
	T.AssertTrue(HOLArtListen::SlotPlayedFor("Units", n"Order"), "a unit's order rings");
	T.AssertTrue(HOLArtListen::SlotPlayedFor("Units", n"Attack"), "a unit's blow rings");
	T.AssertTrue(HOLArtListen::SlotPlayedFor("Units", n"Death"), "a unit's death rings");
	T.AssertFalse(HOLArtListen::SlotPlayedFor("Units", n"Selection"), "a unit's selection is never rung by the game");
	T.AssertFalse(HOLArtListen::SlotPlayedFor("Units", n"Construction"), "a unit has no construction");
	T.AssertTrue(HOLArtListen::SlotPlayedFor("Buildings", n"Selection"), "a building's selection rings");
	T.AssertTrue(HOLArtListen::SlotPlayedFor("Buildings", n"Attack"), "a tower's volley rings");
	T.AssertTrue(HOLArtListen::SlotPlayedFor("Buildings", n"Construction"), "a building's construction rings");
	T.AssertFalse(HOLArtListen::SlotPlayedFor("Buildings", n"Order"), "a building gets no unit order");
	T.AssertFalse(HOLArtListen::SlotPlayedFor("Sources", n"Attack"), "a source rings nothing");
	T.AssertFalse(HOLArtListen::SlotPlayedFor("", n"Order"), "an unknown type rings nothing");
}

// The table's cue behind a slot, empty when the table is silent there: the Barracks' elected selection sound is in the table; the
// unit orders and the end of a construction are silent by the user's choice (HOLAudioTable_Test.as AudioTestIsSilentEvent).
void Test_ArtListenTableCue(FUnitTest& T)
{
	const FString Barracks = HOLArtListen::TableCueFor(n"Barracks", "Buildings", n"Selection");
	T.AssertFalse(Barracks.IsEmpty(), f"the Barracks' selection has its table cue (got '{Barracks}')");
	T.AssertTrue(Barracks.StartsWith("Select."), "a selection cue of the table");
	T.AssertTrue(HOLArtListen::TableCueFor(n"ManAtArms", "Units", n"Order").IsEmpty(), "the unit orders are silent in the table");
	T.AssertTrue(HOLArtListen::TableCueFor(n"House", "Buildings", n"Construction").IsEmpty(), "the end of a construction is silent in the table");
	T.AssertTrue(HOLArtListen::TableCueFor(n"ManAtArms", "Units", n"Attack").IsEmpty(), "the table has no attack cue");
}

// The line written for a slot.
void Test_ArtListenDiagnostic(FUnitTest& T)
{
	T.AssertEquals("art /Game/Audio/X.X", HOLArtListen::Diagnostic(true, "/Game/Audio/X.X", "Select.Wood"), "the asset's sound first");
	T.AssertEquals("table Select.Wood", HOLArtListen::Diagnostic(true, "", "Select.Wood"), "then the table's cue");
	T.AssertEquals("silent", HOLArtListen::Diagnostic(true, "", ""), "else silent");
	T.AssertTrue(HOLArtListen::Diagnostic(false, "/Game/Audio/X.X", "").StartsWith("never played"), "a slot the game never rings for this family, even with a sound");
}

// hol.Art.TrySound refuses an unknown slot (1) and an empty path (2).
void Test_ArtListenTrySoundRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLArtListen::TrySoundRefusal(n"Attack", "/Game/Audio/X.X"), "a known slot and a path");
	T.AssertEquals(1, HOLArtListen::TrySoundRefusal(n"Shout", "/Game/Audio/X.X"), "an unknown slot");
	T.AssertEquals(2, HOLArtListen::TrySoundRefusal(n"Death", ""), "an empty path");
}
