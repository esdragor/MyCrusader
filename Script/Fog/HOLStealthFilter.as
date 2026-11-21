// The script filter the network layer asks (D824 Q1 A): the C++ hook UHOLStealthFilter (HOLVisionSubsystem.h) calls BP_IsVisibleToCamp for each
// enemy camp that would receive a unit; this subclass answers with the rule HOLStealth::IsVisibleToCamp (HOLStealthRules.as), fed with the zones
// of the map and the positions of the viewing camp (UHOLVisionSubsystem::GetCampUnitPositions). The rule stays here (D836); the C++ copies nothing.

class UHOLStealthRuleFilter : UHOLStealthFilter
{
	// The stealth zones of the map (empty: the rule hides nothing, so the filter answers at once).
	TArray<FHOLStealthZone> Zones;

	// Where the camps' units are read from (null: no viewer at all, e.g. in a test without a world).
	UHOLVisionSubsystem Vision;

	UFUNCTION(BlueprintOverride)
	bool BP_IsVisibleToCamp(int ViewerCamp, int UnitCamp, FVector2D UnitPosition) const
	{
		// STUB (red first): sees everything.
		return true;
	}
}
