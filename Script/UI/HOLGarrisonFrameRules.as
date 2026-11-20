// F7 lot 4a, garrison frame (D813 Q-garrison-ui Q1 = C): what the building frame does with HOLGarrisonView. The frame draws a garrison only for the
// player's OWN FINISHED building with places (the occupants are replicated to the owner only, HOLBuilding.h GetGarrisonOccupants): any other
// building gives the empty view (nothing drawn, no order). The widget is HOLGarrisonFrame.as.
namespace HOLGarrisonFrameRules
{
	// STUB (red): the real rule is in the next commit.
	FHOLGarrisonView ViewFor(const TArray<int>& OccupantUnitIds, int Slots, bool bOwn, bool bFinished)
	{
		FHOLGarrisonView View;
		return View;
	}

	// The widget name of the Index-th portrait button (hol.HUD.Click names it).
	FName PortraitName(int Index)
	{
		return NAME_None;
	}
}