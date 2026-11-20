// F7 lot 4a, garrison interface (D813 Q-garrison-ui Q1 = C): the pure rule of what the building frame shows of a garrison and which order a
// click gives. No world here: the widget reads the occupants (unit ids, in the order the building holds them) and the places, calls Build,
// draws the counter, the portraits and the "Sortir tous" grid cell from the result, and sends the order it gets back from a click or the key.
// The orders themselves are the server's (HOLPlayerOrders::UngarrisonAll exists; the single-unit one is still to write, see FortificationsInProgress.md).

// What the frame shows for one garrison building.
struct FHOLGarrisonView
{
	// The counter text, "2 / 5"; empty for a building without places (not a garrison building: nothing to draw).
	FString CounterText;
	// The portraits to draw, one unit id per occupant, left to right.
	TArray<int> PortraitUnitIds;
	// The "Sortir tous" grid cell is shown (only when there is at least one occupant to take out).
	bool bShowUngarrisonAll = false;
}

// The order a click or the key gives; the widget sends it through the controller (RequestUngarrisonAll, or the single-unit request).
struct FHOLGarrisonOrder
{
	int Kind = 0;
	// For HOLGarrisonView::OrderUngarrisonUnit: the unit that comes out.
	int UnitId = -1;
}

namespace HOLGarrisonView
{
	const int OrderNone = 0;
	const int OrderUngarrisonAll = 1;
	const int OrderUngarrisonUnit = 2;

	// STUB (red first): the view of a building holding OccupantUnitIds with Slots places.
	FHOLGarrisonView Build(const TArray<int>& OccupantUnitIds, int Slots)
	{
		FHOLGarrisonView View;
		return View;
	}

	// STUB (red first): the order a click on portrait PortraitIndex gives.
	FHOLGarrisonOrder OrderForPortraitClick(const FHOLGarrisonView& View, int PortraitIndex)
	{
		FHOLGarrisonOrder Order;
		return Order;
	}

	// STUB (red first): the order of the "Sortir tous" key or cell.
	FHOLGarrisonOrder OrderForUngarrisonAll(const FHOLGarrisonView& View)
	{
		FHOLGarrisonOrder Order;
		return Order;
	}
}
