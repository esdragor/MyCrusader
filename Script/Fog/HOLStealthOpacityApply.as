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

// One pass over the local player's own units (ARWEN-1122d): cheap by construction. No zone = no read of any unit. A unit farther than the
// bounding circle of every zone (squared distance, no allocation) is opaque without asking the rule. An opacity is written ONLY when it changes
// from the last one written (a unit never written is opaque: 1.0 is the default, so a unit that never enters a zone costs no write at all).
class UHOLStealthOwnApplier : UObject
{
	TArray<FHOLStealthZone> Zones;

	// Per zone: the squared radius of its bounding circle (half extent X squared + half extent Y squared), computed once by SetZones.
	TArray<float> ReachSquared;

	// The last opacity written per unit id, only for the units NOT opaque (1.0 = absent).
	TMap<int, float> Last;

	// The object the binding takes the world from (the installer).
	UObject WorldObject;

	void SetZones(const TArray<FHOLStealthZone>& NewZones)
	{
		Zones = NewZones;
		ReachSquared.Empty();
		for (const FHOLStealthZone& Zone : Zones)
			ReachSquared.Add(Zone.HalfExtent.X * Zone.HalfExtent.X + Zone.HalfExtent.Y * Zone.HalfExtent.Y);
	}

	// Could this position be inside a zone? Conservative (bounding circle): false means surely outside every zone.
	bool NearAZone(const FVector2D& Position) const
	{
		for (int Index = 0; Index < Zones.Num(); Index++)
		{
			double DeltaX = Position.X - Zones[Index].Center.X;
			double DeltaY = Position.Y - Zones[Index].Center.Y;
			if (DeltaX * DeltaX + DeltaY * DeltaY <= ReachSquared[Index])
				return true;
		}
		return false;
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
		// One journal line per CHANGE of opacity (the pass writes only changes).
		Log(n"LogHOLMatch", f"StealthOwn opacity unit {UnitId} = {Opacity}");
		HOLUnitOpacity::SetStealthOpacity(UnitId, Opacity);
	}

	// Returns the number of opacities written.
	int Pass()
	{
		if (Zones.Num() == 0)
			return 0;
		TArray<int> UnitIds;
		TArray<FVector2D> Positions;
		ReadOwnUnits(UnitIds, Positions);
		int Writes = 0;
		for (int Index = 0; Index < UnitIds.Num() && Index < Positions.Num(); Index++)
		{
			float Now = 1.0;
			if (NearAZone(Positions[Index]))
				Now = HOLStealth::OwnUnitOpacity(Zones, Positions[Index]);
			float Before = 1.0;
			Last.Find(UnitIds[Index], Before);
			if (Math::Abs(Now - Before) < 0.0001)
				continue;
			WriteOpacity(UnitIds[Index], Now);
			Writes++;
			if (Now >= 0.9999)
				Last.Remove(UnitIds[Index]);
			else
				Last.Add(UnitIds[Index], Now);
		}
		return Writes;
	}
}
