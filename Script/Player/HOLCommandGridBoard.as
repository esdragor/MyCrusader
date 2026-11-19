// MIG-CPP lot G1 bridge: the script side of UHOLCommandGridBoard (Source/HeroesOfLiberty/Player/HOLCommandGridBoard.h; D839: a binding to expose).
// HOLCommandGrid::GetCommand and GetOrder (HOLCommandGrid.cpp) ask this board first; the answer is the rule of HOLCommandGridRules (HOLCommandGridRules.as,
// tested against the C++ body by the agreement tests, which compare with the read library, itself on the C++ Native bodies). Without this class compiled,
// there is no board: the C++ keeps its own body (the twin, removed in a later lot after the proof).
// NOT in a Dev / Developers / Examples / Editor folder: a client runs it (the grid is a client read). NOT COMPILED at the time of writing.
//
// To do (next lots of the bridge, same pattern): GetAgeBuilding, PressKey, ClickAge (HOLCommandGridKeys), MergeCommands / SplitSubgroups / PickEvenly /
// NextInCycle (HOLCommandGridGroups), CheckProduction / CheckBuild / CheckResearch / Describe.

class UHOLCommandGridBoardScript : UHOLCommandGridBoard
{
	UFUNCTION(BlueprintOverride)
	FHOLGridCommandView GetCommandAnswer(const FHOLGridContextView& Context, int Cell)
	{
		return HOLCommandGridRules::GetCommand(Context, Cell);
	}

	UFUNCTION(BlueprintOverride)
	FHOLGridCommandView GetOrderAnswer(const FHOLGridContextView& Context, int Cell)
	{
		return HOLCommandGridRules::GetOrder(Context, Cell);
	}
}
