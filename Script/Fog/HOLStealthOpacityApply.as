// Applies the stealth rule's opacity for an own unit (HOLStealth::OwnUnitOpacity, D824 Q3 B) to where the units are drawn
// (UHOLUnitOpacityLibrary::SetStealthOpacity, Source/HeroesOfLiberty/Presentation/HOLUnitOpacity.h). No WorldContext: the setter is local to the machine
// and takes no world. The rule stays in HOLStealthRules.as; this only carries it.
//
// CALL POINT (not wired yet): UHOLStealthInstaller::Tick (HOLStealthFilter.as), once installed, calls ApplyOwnUnits each tick with the local player's units.
// It needs a C++ binding that lists a player's unit ids with their positions (none exists in Script yet: TYPES_A_EXPOSER in ARWEN-1121sg.md).
namespace HOLStealthApply
{
	void ApplyOwnUnit(const TArray<FHOLStealthZone>& Zones, int UnitId, const FVector2D& UnitPosition)
	{
		UHOLUnitOpacityLibrary::SetStealthOpacity(UnitId, HOLStealth::OwnUnitOpacity(Zones, UnitPosition));
	}

	void ApplyOwnUnits(const TArray<FHOLStealthZone>& Zones, const TArray<int>& UnitIds, const TArray<FVector2D>& Positions)
	{
		for (int Index = 0; Index < UnitIds.Num() && Index < Positions.Num(); Index++)
			ApplyOwnUnit(Zones, UnitIds[Index], Positions[Index]);
	}
}
