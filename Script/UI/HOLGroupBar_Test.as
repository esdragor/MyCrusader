// NOT COMPILED YET (needs the C++ lot of HALDIR-1119d for HOLGroupBar.as, same module). H-6: the control groups' bar's rules (HOLGroupBar.as), pure, in literals copied
// from the C++ screen's behaviour (HOLGameScreen.cpp RefreshControlGroups / HandleGroupSlotClicked, and the strings quick_check.ps1 reads: "HUD groups: 1=9",
// "1=3 2=<n>", "HUD: group 2 clicked", ControlGroup2).

void Test_GroupBarShownTextIsEmptyUnitsCountOrBuildingsB(FUnitTest& T)
{
	T.AssertEquals(HOLGroupBar::ShownText(false, false, 0), "", "an empty group shows nothing (the chip is collapsed)");
	T.AssertEquals(HOLGroupBar::ShownText(true, false, 9), "9", "nine units: 9 (quick_check: 1=9)");
	T.AssertEquals(HOLGroupBar::ShownText(true, true, 2), "b2", "a group of two buildings, no unit: b2");
	T.AssertEquals(HOLGroupBar::ShownText(false, true, 4), "", "not full wins over everything");
}

void Test_GroupBarChipIsTouchedOnlyWhenItsGroupChanged(FUnitTest& T)
{
	T.AssertFalse(HOLGroupBar::ChipChanged("9", "9"), "same count: untouched");
	T.AssertTrue(HOLGroupBar::ChipChanged("9", "3"), "the count moved");
	T.AssertTrue(HOLGroupBar::ChipChanged("", "b1"), "a group born");
	T.AssertTrue(HOLGroupBar::ChipChanged("3", ""), "a group emptied");
}

void Test_GroupBarOrderIsTheKeysOneToNineThenZero(FUnitTest& T)
{
	int First = HOLGroupBar::GroupAt(0, 10);
	int Ninth = HOLGroupBar::GroupAt(8, 10);
	int Last = HOLGroupBar::GroupAt(9, 10);
	T.AssertEquals(First, 1, "the first chip is group 1");
	T.AssertEquals(Ninth, 9, "the ninth is group 9");
	T.AssertEquals(Last, 0, "the last is group 0, the 0 key");
	T.AssertEquals(HOLGroupBar::SlotName(2), n"ControlGroup2", "hol.HUD.Click ControlGroup2 (quick_check)");
	T.AssertEquals(HOLGroupBar::BoxName(0), n"ControlGroupBox0", "the box keeps the C++ name");
}

void Test_GroupBarChipLabelIsNumberDotCount(FUnitTest& T)
{
	T.AssertEquals(HOLGroupBar::ChipLabel(1, 9), "1 · 9", "the plan's example, group 1 of nine");
	T.AssertEquals(HOLGroupBar::ChipLabel(0, 12), "0 · 12", "group 0, twelve");
}

void Test_GroupBarTypesAreSortedMostFirstTiesKeepTheMeetingOrder(FUnitTest& T)
{
	// Types met in this order: Peasant 3, ManAtArms 5, Archer 5 -> ManAtArms, Archer (the tie keeps its order), Peasant.
	TArray<int> Counts;
	Counts.Add(3);
	Counts.Add(5);
	Counts.Add(5);
	const TArray<int> Order = HOLGroupBar::TypeOrder(Counts);
	T.AssertEquals(Order.Num(), 3, "every type stays");
	T.AssertEquals(Order[0], 1, "the first of the two fives");
	T.AssertEquals(Order[1], 2, "the second five after it (stable)");
	T.AssertEquals(Order[2], 0, "the three last");
	TArray<int> One;
	One.Add(7);
	T.AssertEquals(HOLGroupBar::TypeOrder(One).Num(), 1, "a single type");
	TArray<int> None;
	T.AssertEquals(HOLGroupBar::TypeOrder(None).Num(), 0, "no type: no line");
}

void Test_GroupBarTooltipIsGroupNHeadThenOneLineAType(FUnitTest& T)
{
	TArray<FString> Names;
	Names.Add("Paysan");
	Names.Add("Homme d'armes");
	TArray<int> Counts;
	Counts.Add(3);
	Counts.Add(6);
	T.AssertEquals(HOLGroupBar::TooltipText(1, Names, Counts), "Groupe 1\nHomme d'armes × 6\nPaysan × 3", "head, then the most numerous first");
	TArray<FString> NoNames;
	TArray<int> NoCounts;
	T.AssertEquals(HOLGroupBar::TooltipText(0, NoNames, NoCounts), "Groupe 0", "a group without types read: the head alone");
}

void Test_GroupBarReportLineIsTheOneQuickCheckReads(FUnitTest& T)
{
	TArray<FString> Parts;
	T.AssertEquals(HOLGroupBar::BarLine(Parts), "none", "no group full: none");
	Parts.Add(HOLGroupBar::LinePart(1, "3"));
	Parts.Add(HOLGroupBar::LinePart(2, "8"));
	T.AssertEquals(HOLGroupBar::BarLine(Parts), "1=3 2=8", "quick_check: 1=3 2=<soldiers>");
	Parts.Add(HOLGroupBar::LinePart(0, "b2"));
	T.AssertEquals(HOLGroupBar::BarLine(Parts), "1=3 2=8 0=b2", "0 last, a buildings' group as b2");
}

void Test_GroupBarJournalLinesAreTheCppOnesWordForWord(FUnitTest& T)
{
	T.AssertEquals(HOLGroupBar::BarLogLine("1=9"), "HUD groups: 1=9", "waitlog HUD groups: 1=9");
	T.AssertEquals(HOLGroupBar::BarLogLine("none"), "HUD groups: none", "the bar emptied");
	T.AssertEquals(HOLGroupBar::ClickLine(2), "HUD: group 2 clicked", "quick_check: HUD: group 2 clicked");
}

void Test_GroupBarLitGroupFadesAfterHalfASecond(FUnitTest& T)
{
	int LitFresh = HOLGroupBar::LitGroup(2, 0.1);
	int LitEdge = HOLGroupBar::LitGroup(2, 0.49);
	int LitOver = HOLGroupBar::LitGroup(2, 0.5);
	int LitNone = HOLGroupBar::LitGroup(-1, 0.0);
	T.AssertEquals(LitFresh, 2, "just selected: lit");
	T.AssertEquals(LitEdge, 2, "still lit just under half a second");
	T.AssertEquals(LitOver, -1, "half a second: no more (strictly under)");
	T.AssertEquals(LitNone, -1, "nothing ever selected");
	T.AssertEquals(HOLGroupBar::ItemScale(2, 2), 1.15, "the lit chip is drawn larger");
	T.AssertEquals(HOLGroupBar::ItemScale(3, 2), 1.0, "the others stay");
	T.AssertEquals(HOLGroupBar::ItemOpacity(3, 2), 0.7, "the others dimmed while one is lit");
	T.AssertEquals(HOLGroupBar::ItemOpacity(2, 2), 1.0, "the lit one opaque");
	T.AssertEquals(HOLGroupBar::ItemOpacity(3, -1), 1.0, "none lit: all opaque");
}

void Test_GroupBarClickIsRefusedOnAnEmptyChip(FUnitTest& T)
{
	T.AssertFalse(HOLGroupBar::ClickAccepted(""), "an empty group: hol.HUD.Click ControlGroup5 is false");
	T.AssertTrue(HOLGroupBar::ClickAccepted("9"), "a group of nine units");
	T.AssertTrue(HOLGroupBar::ClickAccepted("b1"), "a group of one building");
}
