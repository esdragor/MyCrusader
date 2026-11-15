// Stealth zones, pure rule (B3; GDD 8.2, D383, D396): a unit standing in a stealth zone laid by the map is hidden from a camp, unless a unit
// of that camp is in the same zone or within RevealDistance of it (the "very close" enemy, Retours\Q-stealth.md). No world access, tested in
// Script\Fog\HOLStealthRules_Test.as. Not here: the per-camp vision (UHOLVisionSubsystem, C++), the network filter, the zones of a map, the look.
// Outside every zone the rule hides nothing: the normal fog vision applies. Visible = cell in sight AND not hidden by stealth.

// The "very close" distance in cm (D383; default 800 = 2 cells, waiting for Retours\Q-stealth.md).
const FConsoleVariable CVar_StealthRevealDistance("hol.Stealth.RevealDistance", 800);

// A stealth zone, provisional shape: an axis-aligned rectangle (centre, half extent), like a blocker cell; the real shape comes with the maps.
struct FHOLStealthZone
{
	FVector2D Center;
	FVector2D HalfExtent;
}

namespace HOLStealth
{
	float RevealDistance()
	{
		return Math::Max(0, CVar_StealthRevealDistance.GetInt());
	}

	bool IsInZone(const FHOLStealthZone& Zone, const FVector2D& Position)
	{
		return Math::Abs(Position.X - Zone.Center.X) <= Zone.HalfExtent.X && Math::Abs(Position.Y - Zone.Center.Y) <= Zone.HalfExtent.Y;
	}

	// Is the unit at UnitPosition hidden from the camp whose units stand at ViewerPositions?
	// Hidden when the unit stands in at least one zone and no viewer is within Distance (inclusive) nor inside a zone holding the unit.
	bool IsHiddenFrom(const TArray<FHOLStealthZone>& Zones, const FVector2D& UnitPosition, const TArray<FVector2D>& ViewerPositions, float Distance)
	{
		bool bInZone = false;
		for (const FHOLStealthZone& Zone : Zones)
		{
			if (IsInZone(Zone, UnitPosition))
			{
				bInZone = true;
				break;
			}
		}
		if (!bInZone)
		{
			return false;
		}
		const float DistanceSquared = Distance * Distance;
		for (const FVector2D& Viewer : ViewerPositions)
		{
			const float DX = Viewer.X - UnitPosition.X;
			const float DY = Viewer.Y - UnitPosition.Y;
			if (DX * DX + DY * DY <= DistanceSquared)
			{
				return false;
			}
			for (const FHOLStealthZone& Zone : Zones)
			{
				if (IsInZone(Zone, UnitPosition) && IsInZone(Zone, Viewer))
				{
					return false;
				}
			}
		}
		return true;
	}
}
