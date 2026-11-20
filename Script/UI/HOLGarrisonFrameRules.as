// F7 lot 4a, garrison frame (D813 Q-garrison-ui Q1 = C): what the building frame does with HOLGarrisonView. The frame draws a garrison only for the
// player's OWN FINISHED building with places (the occupants are replicated to the owner only, HOLBuilding.h GetGarrisonOccupants): any other
// building gives the empty view (nothing drawn, no order). The widget is HOLGarrisonFrame.as.
namespace HOLGarrisonFrameRules
{
	// The view of the selected building: the garrison of the player's own finished building with places (HOLGarrisonView::Build), else the empty view.
	FHOLGarrisonView ViewFor(const TArray<int>& OccupantUnitIds, int Slots, bool bOwn, bool bFinished)
	{
		if (!bOwn || !bFinished)
		{
			FHOLGarrisonView Empty;
			return Empty;
		}
		return HOLGarrisonView::Build(OccupantUnitIds, Slots);
	}

	// The widget name of the Index-th portrait button (hol.HUD.Click names it).
	FName PortraitName(int Index)
	{
		return FName(f"GarrisonPortrait{Index}");
	}
}