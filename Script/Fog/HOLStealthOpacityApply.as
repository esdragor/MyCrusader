// Applies the stealth rule's opacity for an own unit (HOLStealth::OwnUnitOpacity, D824 Q3 B) to where the units are drawn
// (UHOLUnitOpacityLibrary::SetStealthOpacity, Source/HeroesOfLiberty/Presentation/HOLUnitOpacity.h). No WorldContext: the setter is local to the machine
// and takes no world. The rule stays in HOLStealthRules.as; this only carries it.
//
// CALL POINT (wired, ARWEN-1122c): UHOLStealthInstaller::ApplyOwnUnitOpacity (HOLStealthFilter.as), called from its Tick 20 times a second once installed
// and only when the map has zones, calls ApplyOwnUnits with the local player's units listed by HOLStealthOwnUnits::GetPlayerUnits (UHOLStealthOwnUnitsLibrary) (C++ binding,
// Source/HeroesOfLiberty/Presentation/HOLStealthOwnUnits.h): the ids are the simulation's State.UnitId.
namespace HOLStealthApply
{
	void ApplyOwnUnit(const TArray<FHOLStealthZone>& Zones, int UnitId, const FVector2D& UnitPosition)
	{
		HOLUnitOpacity::SetStealthOpacity(UnitId, HOLStealth::OwnUnitOpacity(Zones, UnitPosition));
	}

	void ApplyOwnUnits(const TArray<FHOLStealthZone>& Zones, const TArray<int>& UnitIds, const TArray<FVector2D>& Positions)
	{
		for (int Index = 0; Index < UnitIds.Num() && Index < Positions.Num(); Index++)
			ApplyOwnUnit(Zones, UnitIds[Index], Positions[Index]);
	}
}
