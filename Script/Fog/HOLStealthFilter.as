// The script filter the network layer asks (D824 Q1 A): the C++ hook UHOLStealthFilter (HOLVisionSubsystem.h) calls BP_IsVisibleToCamp for each
// enemy camp that would receive a unit; this subclass answers with the rule HOLStealth::IsVisibleToCamp (HOLStealthRules.as), fed with the zones
// of the map and the positions of the viewing camp (UHOLVisionSubsystem::GetCampUnitPositions). The rule stays here (D836); the C++ copies nothing.
// UHOLStealthInstaller (below) sets it on the vision subsystem once the world runs.

class UHOLStealthRuleFilter : UHOLStealthFilter
{
	// The stealth zones of the map (empty: the rule hides nothing, so the filter answers at once, without reading any unit).
	TArray<FHOLStealthZone> Zones;

	// Where the camps' units are read from (null: no viewer at all, e.g. in a test without a world).
	UHOLVisionSubsystem Vision;

	UFUNCTION(BlueprintOverride)
	bool IsVisibleToCamp(int ViewerCamp, int UnitCamp, FVector2D UnitPosition) const
	{
		if (Zones.Num() == 0)
			return true;
		TArray<FVector2D> Viewers;
		if (Vision != nullptr)
			Viewers = Vision.GetCampUnitPositions(ViewerCamp);
		return HOLStealth::IsVisibleToCamp(Zones, UnitCamp, UnitPosition, ViewerCamp, Viewers, HOLStealth::RevealDistance());
	}
}

namespace HOLStealthInstall
{
	// THE TABLE (level design, TO VALIDATE, D824 Q2 B): the groves of the Forest map, one half only; the other half is the point mirror. Each line =
	// centre offset from the map centre and half extent, as a FRACTION of the map half side (so it follows the map size). Edit only here.
	TArray<FHOLStealthZone> ForestGroveHalf()
	{
		TArray<FHOLStealthZone> Half;
		FHOLStealthZone North;
		North.Center = FVector2D(0.30, 0.35);
		North.HalfExtent = FVector2D(0.12, 0.12);
		Half.Add(North);
		FHOLStealthZone East;
		East.Center = FVector2D(0.35, -0.25);
		East.HalfExtent = FVector2D(0.10, 0.10);
		Half.Add(East);
		return Half;
	}

	// Does this map carry groves? Only the maps listed here do (Tools\Data\Maps.csv Name, compared without case; a level name "Forest_..." counts).
	bool MapHasGroves(const FString& MapName)
	{
		FString Lower = MapName.ToLower();
		return Lower == "forest" || Lower.StartsWith("forest_");
	}

	// The zones of a map by its name (D824 Q2 B: only the maps with groves; an open map has none). Centre = point of symmetry (CenterOfCamps),
	// MapHalfExtent = half side of the playable square, in cm. Each grove and its mirror through Center: 2 to 4 zones, all inside the square.
	TArray<FHOLStealthZone> ZonesForMap(const FString& MapName, const FVector2D& Center, float MapHalfExtent)
	{
		TArray<FHOLStealthZone> Zones;
		if (!MapHasGroves(MapName) || MapHalfExtent <= 0.0)
			return Zones;
		for (const FHOLStealthZone& Grove : ForestGroveHalf())
		{
			FHOLStealthZone Zone;
			Zone.Center = Center + Grove.Center * MapHalfExtent;
			Zone.HalfExtent = Grove.HalfExtent * MapHalfExtent;
			Zones.Add(Zone);
			FHOLStealthZone Mirror;
			Mirror.Center = Center - Grove.Center * MapHalfExtent;
			Mirror.HalfExtent = Zone.HalfExtent;
			Zones.Add(Mirror);
		}
		return Zones;
	}

	// The symmetry centre = mean of the camps' start positions; no position known = the centre of the map.
	FVector2D CenterOfCamps(const TArray<FVector2D>& CampPositions, const FVector2D& MapCenter)
	{
		if (CampPositions.Num() == 0)
			return MapCenter;
		FVector2D Sum = FVector2D(0.0, 0.0);
		for (const FVector2D& Position : CampPositions)
			Sum += Position;
		return Sum / CampPositions.Num();
	}
	UHOLStealthRuleFilter MakeFilter(UObject Outer, UHOLVisionSubsystem Vision, const TArray<FHOLStealthZone>& Zones)
	{
		UHOLStealthRuleFilter Filter = Cast<UHOLStealthRuleFilter>(NewObject(Outer, UHOLStealthRuleFilter::StaticClass()));
		Filter.Zones = Zones;
		Filter.Vision = Vision;
		return Filter;
	}
}

// Installs the filter at the start of the match: the first tick of the world where the vision subsystem exists (the same way the biome applier
// waits for its inputs). Once installed it does nothing more. Installed on every machine: a client never replicates units, and with no zone
// the filter returns at once.
class UHOLStealthInstaller : UScriptWorldSubsystem
{
	bool bInstalled = false;

	// The stealth zones of the current map: the level name (Gameplay::GetCurrentLevelName) and the bounds of the map (AHOLMapBounds, as the biome
	// applier reads them) feed the rule HOLStealthInstall::ZonesForMap. The centre is the middle of the map bounds (the camps' start positions are
	// not readable from the script yet: CenterOfCamps is ready for them). No bounds or an open map: empty list, the game plays as before.
	TArray<FHOLStealthZone> ZonesOfMap()
	{
		TArray<AHOLMapBounds> Bounds;
		GetAllActorsOfClass(Bounds);
		if (Bounds.Num() == 0)
		{
			TArray<FHOLStealthZone> None;
			return None;
		}
		FVector Origin;
		FVector Extent;
		Bounds[0].GetActorBounds(false, Origin, Extent);
		FString MapName = Gameplay::GetCurrentLevelName(true);
		return HOLStealthInstall::ZonesForMap(MapName, FVector2D(Origin.X, Origin.Y), Math::Max(Extent.X, Extent.Y));
	}

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		if (bInstalled)
			return;
		UHOLVisionSubsystem Vision = UHOLVisionSubsystem::Get();
		if (Vision == nullptr)
			return;
		Vision.SetStealthFilter(HOLStealthInstall::MakeFilter(this, Vision, ZonesOfMap()));
		bInstalled = true;
	}
}
