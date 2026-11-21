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

	// Per-camp view (D824 Q1 A): is the unit of UnitCamp at UnitPosition visible, as far as stealth goes, to the camp ViewerCamp whose units stand at ViewerPositions?
	// Allies (same camp) always see it; an enemy camp sees it only through IsHiddenFrom (within Distance, inclusive, or inside the zone).
	bool IsVisibleToCamp(const TArray<FHOLStealthZone>& Zones, int UnitCamp, const FVector2D& UnitPosition, int ViewerCamp, const TArray<FVector2D>& ViewerPositions, float Distance)
	{
		if (ViewerCamp == UnitCamp)
		{
			return true;
		}
		return !IsHiddenFrom(Zones, UnitPosition, ViewerPositions, Distance);
	}

	// Look (D824 Q3 B): the discreet ground outline of a zone shows when the player hovers it or when one of the player's own units stands in it.
	bool ShowsOutline(const FHOLStealthZone& Zone, bool bHovered, const TArray<FVector2D>& OwnPositions)
	{
		if (bHovered)
		{
			return true;
		}
		for (const FVector2D& Own : OwnPositions)
		{
			if (IsInZone(Zone, Own))
			{
				return true;
			}
		}
		return false;
	}
}
