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

	// Family 1: ages and keys (HOLCommandGridRules / HOLCommandGridKeys).
	UFUNCTION(BlueprintOverride)
	bool IsAgeReachedAnswer(const FHOLGridContextView& Context, int Age)
	{
		return HOLCommandGridRules::IsAgeReached(Context, Age);
	}

	UFUNCTION(BlueprintOverride)
	bool IsAgeOpenAnswer(const FHOLGridContextView& Context, int Age)
	{
		return HOLCommandGridRules::IsAgeOpen(Context, Age);
	}

	UFUNCTION(BlueprintOverride)
	FHOLGridCommandView GetAgeBuildingAnswer(const FHOLGridContextView& Context, int Age, int Cell)
	{
		return HOLCommandGridRules::GetAgeBuilding(Context, Age, Cell);
	}

	UFUNCTION(BlueprintOverride)
	FHOLGridKeyResultView PressKeyAnswer(const FHOLGridContextView& Context, int ChosenAge, int Cell)
	{
		return HOLCommandGridKeys::PressKey(Context, ChosenAge, Cell);
	}

	UFUNCTION(BlueprintOverride)
	FHOLGridKeyResultView ClickAgeAnswer(const FHOLGridContextView& Context, int ChosenAge, int Age, int Cell)
	{
		return HOLCommandGridKeys::ClickAge(Context, ChosenAge, Age, Cell);
	}

	UFUNCTION(BlueprintOverride)
	int GetChosenAgeAfterAnswer(int ChosenAge, EHOLAgeEventKind Event)
	{
		return HOLCommandGridKeys::GetChosenAgeAfter(ChosenAge, Event);
	}

	// Family 2: checks and Describe (HOLCommandGridKeys). CheckResearch has no script rule: not bridged.
	UFUNCTION(BlueprintOverride)
	FHOLFeedbackView CheckProductionAnswer(const FHOLResources& Stock, const FHOLResources& Cost, int QueueLength, int MaxQueue, int PopulationUsed, int PopulationCap, int Places)
	{
		return HOLCommandGridKeys::CheckProduction(Stock, Cost, QueueLength, MaxQueue, PopulationUsed, PopulationCap, Places);
	}

	UFUNCTION(BlueprintOverride)
	FHOLFeedbackView CheckBuildAnswer(const FHOLResources& Stock, const FHOLResources& Cost)
	{
		return HOLCommandGridKeys::CheckBuild(Stock, Cost);
	}

	UFUNCTION(BlueprintOverride)
	FString DescribeAnswer(const FHOLFeedbackView& Feedback)
	{
		return HOLCommandGridKeys::Describe(Feedback);
	}}
