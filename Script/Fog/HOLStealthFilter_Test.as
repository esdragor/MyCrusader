// The filter that carries the stealth rule to the network layer (HOLStealthFilter.as): the real chain is C++ MaskCamps -> script BP_IsVisibleToCamp
// -> HOLStealth::IsVisibleToCamp. Camp 0's unit stands in the test zone (StealthOneZone, HOLStealthRules_Test.as); the filter has no Vision, so
// the enemy camp 1 has no viewer in range and must not receive the unit; camp 0 (its owner) always keeps it.

void Test_StealthRuleFilterMasksEnemyCampForHiddenUnit(FUnitTest& T)
{
	UHOLStealthRuleFilter Filter = Cast<UHOLStealthRuleFilter>(NewObject(GetTransientPackage(), UHOLStealthRuleFilter::StaticClass()));
	// StealthOneZone (HOLStealthRules_Test.as) is a global of another file, not visible here: the same zone built locally.
	FHOLStealthZone Zone;
	Zone.Center = FVector2D(0.0, 0.0);
	Zone.HalfExtent = FVector2D(400.0, 400.0);
	Filter.Zones.Add(Zone);
	int Mask = UHOLStealthFilter::MaskCamps(Filter, 3, 2, 0, FVector2D(0.0, 0.0));
	T.AssertEquals(1, Mask, "unit of camp 0 hidden in the zone: enemy camp 1 does not receive it, camp 0 does");
}

void Test_StealthRuleFilterMasksNothingWithoutZones(FUnitTest& T)
{
	UHOLStealthRuleFilter Filter = Cast<UHOLStealthRuleFilter>(NewObject(GetTransientPackage(), UHOLStealthRuleFilter::StaticClass()));
	int Mask = UHOLStealthFilter::MaskCamps(Filter, 3, 2, 0, FVector2D(0.0, 0.0));
	T.AssertEquals(3, Mask, "no zone on the map (none is laid yet): nothing is hidden, both camps receive the unit");
}
