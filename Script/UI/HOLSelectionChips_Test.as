// NOT COMPILED YET (needs HOLSelectionChips.as, which needs the lot A's UHOLSelectionPanel and the C++ HOLSelectionChipsReadLibrary). H-9 lot C: the pure rules of the chips' VIEW
// (HOLSelectionChipsRules). The labels themselves ("Tous (8)" first when several types, "Nom (N)" or "Nom" alone, the 12-character cut, no active chip for another player's units)
// are built by the controller's rule in C++ (GetChipLabels, one source): here the tests hold what the view does with them, the labels given as the C++ writes them.

TArray<FString> SelectionChipsTestLabels(int Count)
{
	TArray<FString> Labels;
	for (int Index = 0; Index < Count; Index++)
		Labels.Add(f"Type{Index} (1)");
	return Labels;
}

void Test_SelectionChipsAllChipFirstThenTheTypesStayInTheControllersOrder(FUnitTest& T)
{
	TArray<FString> Labels;
	Labels.Add("Tous (8)");
	Labels.Add("Lancier (3)");
	Labels.Add("Archer (2)");
	Labels.Add("Paysan (3)");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 0), "Tous (8)", "the all chip stays first");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 1), "Lancier (3)", "then the types, soldiers first, the Peasants last");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 3), "Paysan (3)", "the Peasants last");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 4), "", "a chip beyond the labels shows nothing (its frame collapsed)");
	T.AssertFalse(HOLSelectionChipsRules::FrameShown(HOLSelectionChipsRules::ShownLabel(Labels, 4)), "no label, no frame");
	T.AssertTrue(HOLSelectionChipsRules::FrameShown("Tous (8)"), "a label, a frame");
}

void Test_SelectionChipsLabelWithoutCountIsKeptAsIs(FUnitTest& T)
{
	TArray<FString> Labels;
	Labels.Add("Tous (7)");
	Labels.Add("Maison (3)");
	Labels.Add("Centre principal");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 2), "Centre principal", "a main centre alone: the name without a count");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 1), "Maison (3)", "a type: its name and its count");
}

void Test_SelectionChipsPoolIsTwelveAndNeverMore(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionChipsRules::NumChips(), 12, "Tous and 11 types");
	TArray<FString> Labels = SelectionChipsTestLabels(14);
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 11), "Type11 (1)", "the twelfth chip shows");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, 12), "", "a thirteenth label has no chip");
	T.AssertEquals(HOLSelectionChipsRules::ShownLabel(Labels, -1), "", "no chip before the first");
}

void Test_SelectionChipsNoFrameLitWhenNothingIsActive(FUnitTest& T)
{
	// Another player's units (or no chips): the controller gives INDEX_NONE, no chip is lit.
	for (int Chip = 0; Chip < HOLSelectionChipsRules::NumChips(); Chip++)
		T.AssertFalse(HOLSelectionChipsRules::FrameLit(-1, Chip), f"no active chip: chip {Chip} is not lit");
	T.AssertTrue(HOLSelectionChipsRules::FrameLit(2, 2), "the active chip is lit");
	T.AssertFalse(HOLSelectionChipsRules::FrameLit(2, 1), "the others are not");
	T.AssertFalse(HOLSelectionChipsRules::FrameLit(2, 3), "the others are not (after)");
}

void Test_SelectionChipsNameGivesWayToTheChips(FUnitTest& T)
{
	TArray<FString> None;
	TArray<FString> Two = SelectionChipsTestLabels(2);
	T.AssertFalse(HOLSelectionChipsRules::ChipsShown(None), "no label: no chips");
	T.AssertTrue(HOLSelectionChipsRules::ChipsShown(Two), "labels: chips");
	T.AssertTrue(HOLSelectionChipsRules::NameShown(HOLSelectionChipsRules::ChipsShown(None)), "no chips: the name shows (a single unit or building)");
	T.AssertFalse(HOLSelectionChipsRules::NameShown(HOLSelectionChipsRules::ChipsShown(Two)), "with chips the name is hidden (hol.HUD.Report: selection name none)");
}

void Test_SelectionChipsLabelIsWrittenOnlyWhenItChanged(FUnitTest& T)
{
	T.AssertFalse(HOLSelectionChipsRules::LabelChanged("Lancier (3)", "Lancier (3)"), "same text: untouched");
	T.AssertTrue(HOLSelectionChipsRules::LabelChanged("Lancier (3)", "Lancier (2)"), "the count moved");
	T.AssertTrue(HOLSelectionChipsRules::LabelChanged("", "Tous (8)"), "a chip born");
	T.AssertTrue(HOLSelectionChipsRules::LabelChanged("Tous (8)", ""), "a chip emptied");
}

void Test_SelectionChipsWidgetNamesAreTheCppOnes(FUnitTest& T)
{
	// hol.HUD.Click SelectionChip1, hol.Mouse.Hover, the HUD rects look for these names.
	T.AssertEquals(HOLSelectionChipsRules::ChipName(0), n"SelectionChip0", "the first chip");
	T.AssertEquals(HOLSelectionChipsRules::ChipName(11), n"SelectionChip11", "the last chip");
	T.AssertEquals(HOLSelectionChipsRules::FrameName(3), n"SelectionChipFrame3", "a chip's frame");
}

void Test_SelectionChipsClickIsAcceptedForAChipOfThePoolOnly(FUnitTest& T)
{
	T.AssertTrue(HOLSelectionChipsRules::ClickAccepted(0), "the first chip");
	T.AssertTrue(HOLSelectionChipsRules::ClickAccepted(11), "the last chip");
	T.AssertFalse(HOLSelectionChipsRules::ClickAccepted(12), "beyond the pool");
	T.AssertFalse(HOLSelectionChipsRules::ClickAccepted(-1), "before the pool");
}

void Test_SelectionChipsChipMaxWidthIsHalfTheBlockMinusTheGap(FUnitTest& T)
{
	// SelectionWidth 340, the wrap box 24 narrower (316), a chip at most (316 - 4) / 2 = 156: two a line.
	T.AssertEquals(HOLSelectionChipsRules::WrapSize(), 316.0, "the wrap box's width");
	T.AssertEquals(HOLSelectionChipsRules::ChipMaxWidth(), 156.0, "half of it less the gap");
	T.AssertTrue(2.0 * HOLSelectionChipsRules::ChipMaxWidth() + HOLSelectionChipsRules::ChipGap() <= HOLSelectionChipsRules::WrapSize(), "two chips and their gap fit a line");
}
