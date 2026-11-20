// HOLGarrisonView.as (F7 lot 4a, D813 Q-garrison-ui Q1 = C): counter "N / M", one portrait per occupant, "Sortir tous" only with occupants,
// a clicked portrait gives "this unit comes out". Every value asserted is computed by the rule from the occupants it is given.

void Test_GarrisonViewCounterAndPortraits(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(41);
	Occupants.Add(77);
	FHOLGarrisonView View = HOLGarrisonView::Build(Occupants, 5);
	T.AssertEquals(View.CounterText, "2 / 5", "two occupants in a five-place tower");
	T.AssertEquals(View.PortraitUnitIds.Num(), 2, "one portrait per occupant");
	if (View.PortraitUnitIds.Num() == 2)
	{
		T.AssertEquals(View.PortraitUnitIds[0], 41, "first portrait is the first occupant");
		T.AssertEquals(View.PortraitUnitIds[1], 77, "second portrait is the second occupant");
	}
	T.AssertTrue(View.bShowUngarrisonAll, "with occupants the Sortir tous cell shows");
}

void Test_GarrisonViewEmptyHidesUngarrisonAll(FUnitTest& T)
{
	TArray<int> None;
	FHOLGarrisonView Empty = HOLGarrisonView::Build(None, 3);
	T.AssertEquals(Empty.CounterText, "0 / 3", "an empty watch tower still says its places");
	T.AssertEquals(Empty.PortraitUnitIds.Num(), 0, "no portrait");
	T.AssertFalse(Empty.bShowUngarrisonAll, "nobody inside: no Sortir tous cell");
	FHOLGarrisonOrder Key = HOLGarrisonView::OrderForUngarrisonAll(Empty);
	T.AssertEquals(Key.Kind, HOLGarrisonView::OrderNone, "the key does nothing when the cell is hidden");

	FHOLGarrisonView NoPlaces = HOLGarrisonView::Build(None, 0);
	T.AssertEquals(NoPlaces.CounterText, "", "a building without places draws no counter");
	T.AssertFalse(NoPlaces.bShowUngarrisonAll, "and no cell");
}

void Test_GarrisonViewPortraitClickOrdersThatUnit(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(12);
	Occupants.Add(34);
	Occupants.Add(56);
	FHOLGarrisonView View = HOLGarrisonView::Build(Occupants, 8);
	FHOLGarrisonOrder Order = HOLGarrisonView::OrderForPortraitClick(View, 1);
	T.AssertEquals(Order.Kind, HOLGarrisonView::OrderUngarrisonUnit, "a portrait click is the single-unit order");
	T.AssertEquals(Order.UnitId, 34, "the unit of the clicked portrait comes out (index 1 = 34)");
	FHOLGarrisonOrder Last = HOLGarrisonView::OrderForPortraitClick(View, 2);
	T.AssertEquals(Last.UnitId, 56, "the last portrait gives the last occupant");
	FHOLGarrisonOrder Outside = HOLGarrisonView::OrderForPortraitClick(View, 3);
	T.AssertEquals(Outside.Kind, HOLGarrisonView::OrderNone, "a click past the last portrait gives nothing");
	FHOLGarrisonOrder Negative = HOLGarrisonView::OrderForPortraitClick(View, -1);
	T.AssertEquals(Negative.Kind, HOLGarrisonView::OrderNone, "a negative index gives nothing");
}

void Test_GarrisonViewUngarrisonAllKey(FUnitTest& T)
{
	TArray<int> Occupants;
	Occupants.Add(9);
	FHOLGarrisonView View = HOLGarrisonView::Build(Occupants, 3);
	FHOLGarrisonOrder Order = HOLGarrisonView::OrderForUngarrisonAll(View);
	T.AssertEquals(Order.Kind, HOLGarrisonView::OrderUngarrisonAll, "the Sortir tous cell or key gives the ungarrison-all order");
}
