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
	// The stealth zones of the current map. NO MAP LAYS ZONES YET (MapsInProgress.md): the list is empty, so the installed filter hides nothing
	// and the game plays as before. When a map carries its zones, read them here (one source, FHOLStealthZone list).
	// STUB (red commit): the zones of a map by its name (D824 Q2 B: only the maps with groves; an open map has none). Centre = point of symmetry
	// (the centre of the camps' start positions, CenterOfCamps), MapHalfExtent = half side of the playable square, in cm.
	TArray<FHOLStealthZone> ZonesForMap(const FString& MapName, const FVector2D& Center, float MapHalfExtent)
	{
		TArray<FHOLStealthZone> Zones;
		return Zones;
	}

	// STUB (red commit): the symmetry centre = mean of the camps' start positions; no position known = the centre of the map.
	FVector2D CenterOfCamps(const TArray<FVector2D>& CampPositions, const FVector2D& MapCenter)
	{
		return MapCenter;
	}
	TArray<FHOLStealthZone> ZonesOfMap()
	{
		TArray<FHOLStealthZone> Zones;
		return Zones;
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

	UFUNCTION(BlueprintOverride)
	void Tick(float DeltaTime)
	{
		if (bInstalled)
			return;
		UHOLVisionSubsystem Vision = UHOLVisionSubsystem::Get();
		if (Vision == nullptr)
			return;
		Vision.SetStealthFilter(HOLStealthInstall::MakeFilter(this, Vision, HOLStealthInstall::ZonesOfMap()));
		bInstalled = true;
	}
}
