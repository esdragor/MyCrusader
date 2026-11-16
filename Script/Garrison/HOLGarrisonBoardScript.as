// The server's garrison board (F7, bascule Garrison, D830/D836 point 4), the script side of UHOLGarrisonBoard: UHOLGarrisonSubsystem
// (C++, Mass fragments) gives the candidates (their slots, their kinds, HOLGarrison::KindOf's numbers), the building's slot, its places
// and its occupants; the rule is HOLGarrisonRules.as (HOLGarrison::Admitted, the settings hol.Garrison.MeleeInfantry / Hero / Cavalry).
// Server and solo only. Tested by HeroesOfLiberty.Garrison.ServerBoard.
class UHOLGarrisonBoardScript : UHOLGarrisonBoard
{
	UFUNCTION(BlueprintOverride)
	TArray<int> GarrisonAdmitted(const TArray<int>& UnitSlots, const TArray<int>& UnitKinds, int BuildingSlot, int Slots, int Occupants)
	{
		return HOLGarrison::Admitted(UnitSlots, UnitKinds, BuildingSlot, Slots, Occupants);
	}
}
