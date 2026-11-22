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

// One pass over the local player's own units (ARWEN-1122d). STUB (red commit): reads even without a zone and writes every unit at every pass.
class UHOLStealthOwnApplier : UObject
{
	TArray<FHOLStealthZone> Zones;

	// The object the binding takes the world from (the installer).
	UObject WorldObject;

	void SetZones(const TArray<FHOLStealthZone>& NewZones)
	{
		Zones = NewZones;
	}

	// Overridden by the test: where the own units come from.
	void ReadOwnUnits(TArray<int>& UnitIds, TArray<FVector2D>& Positions)
	{
		int Slot = HOLStealthOwnUnits::GetLocalPlayerSlot(WorldObject);
		if (Slot < 0)
			return;
		HOLStealthOwnUnits::GetPlayerUnits(WorldObject, Slot, UnitIds, Positions);
	}

	// Overridden by the test: where the opacity is written.
	void WriteOpacity(int UnitId, float Opacity)
	{
		HOLUnitOpacity::SetStealthOpacity(UnitId, Opacity);
	}

	// Returns the number of opacities written.
	int Pass()
	{
		TArray<int> UnitIds;
		TArray<FVector2D> Positions;
		ReadOwnUnits(UnitIds, Positions);
		int Writes = 0;
		for (int Index = 0; Index < UnitIds.Num() && Index < Positions.Num(); Index++)
		{
			WriteOpacity(UnitIds[Index], HOLStealth::OwnUnitOpacity(Zones, Positions[Index]));
			Writes++;
		}
		return Writes;
	}
}
