// HOLGarrisonFrameRules (F7 lot 4a, D813 Q-garrison-ui Q1 = C): the frame draws a garrison for the player's own finished building with places only;
// each value asserted is computed by HOLGarrisonView through the rule from the occupants it is given.

void Test_GarrisonFrameOwnFinishedBuildingDrawsTheGarrison(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(8);
	Occupants.Add(21);
	FHOLGarrisonView View = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, true);
	T.AssertEquals(View.CounterText, "2 / 5", "own finished tower: the counter");
	T.AssertEquals(View.PortraitUnitIds.Num(), 2, "one portrait per occupant");
	T.AssertTrue(View.bShowUngarrisonAll, "Sortir tous is offered");
	FHOLGarrisonOrder Click = HOLGarrisonView::OrderForPortraitClick(View, 1);
	T.AssertEquals(Click.Kind, HOLGarrisonView::OrderUngarrisonUnit, "a click on the second portrait takes that unit out");
	T.AssertEquals(Click.UnitId, 21, "the unit of the clicked portrait");
}

void Test_GarrisonFrameOtherPlayersOrUnfinishedBuildingDrawsNothing(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(8);
	FHOLGarrisonView Other = HOLGarrisonFrameRules::ViewFor(Occupants, 5, false, true);
	T.AssertEquals(Other.CounterText, "", "another player's tower: no counter");
	T.AssertEquals(Other.PortraitUnitIds.Num(), 0, "no portrait of an enemy's occupants");
	T.AssertFalse(Other.bShowUngarrisonAll, "no cell");
	FHOLGarrisonOrder OtherKey = HOLGarrisonView::OrderForUngarrisonAll(Other);
	T.AssertEquals(OtherKey.Kind, HOLGarrisonView::OrderNone, "no order from it");

	FHOLGarrisonView Site = HOLGarrisonFrameRules::ViewFor(Occupants, 5, true, false);
	T.AssertEquals(Site.CounterText, "", "a foundation or a site holds nobody: nothing drawn");
	FHOLGarrisonView NoPlaces = HOLGarrisonFrameRules::ViewFor(Occupants, 0, true, true);
	T.AssertEquals(NoPlaces.CounterText, "", "a building without places: nothing drawn");
}

void Test_GarrisonFramePortraitNamesAreDistinct(FUnitTest& T)
{
	const FString First = HOLGarrisonFrameRules::PortraitName(0).ToString();
	const FString Second = HOLGarrisonFrameRules::PortraitName(1).ToString();
	T.AssertEquals(First, "GarrisonPortrait0", "the first portrait's widget name");
	T.AssertEquals(Second, "GarrisonPortrait1", "the second portrait's widget name");
}