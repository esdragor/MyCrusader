// The exploration XP rule, alone (ExplorationXPInProgress.md, GDD 9.4, D05, D06): states built by NewState, the XP summed from what
// OnCellExplored returns along a walk (never a preset's number retyped as the expected total).

// A walk along row Y from cell 0 to cell Count - 1: the XP the side gets in all.
int WalkRow(const FHOLExplorationXPSettings& S, FHOLExplorationXPState& State, const TArray<FHOLExplorationPlace>& Places, int Y, int Count, bool bHeroDead)
{
	int Total = 0;
	for (int X = 0; X < Count; X++)
		Total += HOLExplorationXP::OnCellExplored(S, State, Places, X, Y, bHeroDead);
	return Total;
}

// A new tranche pays once, also while the hero is recovering (D06): 25 cells of 10-cell tranches touch 3 tranches = 3 x 7 XP; the
// same walk again pays nothing; a new tranche explored while the hero is dead still pays.
void Test_ExplorationXPNewTranchePaysOnce(FUnitTest& T)
{
	FHOLExplorationXPSettings S;
	S.CellsPerTranche = 10;
	S.XPPerTranche = 7;
	S.XPPerPlace = 0;
	TArray<FHOLExplorationPlace> NoPlace;
	FHOLExplorationXPState State = HOLExplorationXP::NewState(S, 105, 0);
	T.AssertEquals(21, WalkRow(S, State, NoPlace, 0, 25, false), "3 tranches touched, each pays once");
	T.AssertEquals(0, WalkRow(S, State, NoPlace, 0, 25, false), "the same tranches again pay nothing");
	T.AssertEquals(0, WalkRow(S, State, NoPlace, 9, 25, true), "row 9 is in the same tranches: nothing, hero dead or not");
	T.AssertEquals(21, WalkRow(S, State, NoPlace, 10, 25, true), "row 10 opens 3 new tranches: they pay while the hero is dead (D06)");
	T.AssertEquals(7, HOLExplorationXP::OnCellExplored(S, State, NoPlace, 104, 104, true), "the last, partial tranche of the map pays too");
}

// A place pays once when its cell is explored, on top of its tranche; a neighbouring cell does not discover it.
void Test_ExplorationXPPlacePaysOnce(FUnitTest& T)
{
	FHOLExplorationXPSettings S;
	S.CellsPerTranche = 10;
	S.XPPerTranche = 2;
	S.XPPerPlace = 30;
	TArray<FHOLExplorationPlace> Places;
	FHOLExplorationPlace Relay;
	Relay.CellX = 14;
	Relay.CellY = 3;
	Places.Add(Relay);
	FHOLExplorationXPState State = HOLExplorationXP::NewState(S, 110, Places.Num());
	T.AssertEquals(2, HOLExplorationXP::OnCellExplored(S, State, Places, 13, 3, false), "next to the relay: its tranche only");
	T.AssertEquals(30, HOLExplorationXP::OnCellExplored(S, State, Places, 14, 3, true), "the relay's cell: the place pays, hero dead (D06)");
	T.AssertEquals(0, HOLExplorationXP::OnCellExplored(S, State, Places, 14, 3, false), "seen again: nothing");
	FHOLExplorationXPState Fresh = HOLExplorationXP::NewState(S, 110, Places.Num());
	T.AssertEquals(2 + 30 + 2, WalkRow(S, Fresh, Places, 3, 20, false), "a fresh side walking past: 2 tranches and the relay");
}

// What is explored at the start (the bases' first sight, "Explored map") never pays; outside the grid nothing pays.
void Test_ExplorationXPStartAndOutside(FUnitTest& T)
{
	FHOLExplorationXPSettings S = HOLExplorationXP::Preset(0);
	TArray<FHOLExplorationPlace> Places;
	FHOLExplorationPlace Camp;
	Camp.CellX = 5;
	Camp.CellY = 5;
	Places.Add(Camp);
	FHOLExplorationXPState State = HOLExplorationXP::NewState(S, 110, Places.Num());
	HOLExplorationXP::MarkExploredAtStart(State, S, Places, 5, 5);
	T.AssertEquals(0, HOLExplorationXP::OnCellExplored(S, State, Places, 5, 5, false), "the start cell and its camp: nothing");
	T.AssertEquals(0, HOLExplorationXP::OnCellExplored(S, State, Places, 0, 0, false), "the start's tranche: nothing");
	T.AssertEquals(0, HOLExplorationXP::OnCellExplored(S, State, Places, -1, 0, false), "left of the map");
	T.AssertEquals(0, HOLExplorationXP::OnCellExplored(S, State, Places, 0, 110, false), "below the map");
	T.AssertTrue(HOLExplorationXP::OnCellExplored(S, State, Places, 109, 0, false) > 0, "a far tranche pays with the default preset");
}

// The three presets of the question: C pays both, A the surface only, B the places only.
void Test_ExplorationXPPresets(FUnitTest& T)
{
	TArray<FHOLExplorationPlace> Places;
	FHOLExplorationPlace Altar;
	Altar.CellX = 40;
	Altar.CellY = 0;
	Places.Add(Altar);
	FHOLExplorationXPSettings C = HOLExplorationXP::Preset(0);
	FHOLExplorationXPSettings A = HOLExplorationXP::Preset(1);
	FHOLExplorationXPSettings B = HOLExplorationXP::Preset(2);
	FHOLExplorationXPState SC = HOLExplorationXP::NewState(C, 110, 1);
	FHOLExplorationXPState SA = HOLExplorationXP::NewState(A, 110, 1);
	FHOLExplorationXPState SB = HOLExplorationXP::NewState(B, 110, 1);
	int XPC = WalkRow(C, SC, Places, 0, 110, false);
	int XPA = WalkRow(A, SA, Places, 0, 110, false);
	int XPB = WalkRow(B, SB, Places, 0, 110, false);
	T.AssertTrue(XPA > 0, "A: the surface pays");
	T.AssertTrue(XPB > 0, "B: the altar pays");
	FHOLExplorationXPState SB2 = HOLExplorationXP::NewState(B, 110, 1);
	T.AssertEquals(XPB, HOLExplorationXP::OnCellExplored(B, SB2, Places, 40, 0, false), "B: the altar alone, no tranche");
	T.AssertTrue(XPC > XPB, "C pays the altar and the surface");
	T.AssertEquals(0, WalkRow(A, SA, Places, 1, 110, false), "A: row 1 in the same tranches, the altar paid nothing");
}
