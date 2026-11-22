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

// ---- Zones of a map (D824 Q2 B): the Forest map has 2 to 4 symmetric groves, an open map none ----

// The camps' start positions of the tests (not centred on the origin: the symmetry must follow them, not the world origin).
TArray<FVector2D> StealthTestCamps()
{
	TArray<FVector2D> Camps;
	Camps.Add(FVector2D(1000.0, 2000.0));
	Camps.Add(FVector2D(9000.0, 6000.0));
	return Camps;
}

void Test_StealthForestMapHasTwoToFourZones(FUnitTest& T)
{
	FVector2D Center = HOLStealthInstall::CenterOfCamps(StealthTestCamps(), FVector2D(0.0, 0.0));
	TArray<FHOLStealthZone> Zones = HOLStealthInstall::ZonesForMap("Forest", Center, 6000.0);
	T.AssertTrue(Zones.Num() >= 2 && Zones.Num() <= 4, "the Forest map carries 2 to 4 stealth zones");
}

void Test_StealthForestZonesAreSymmetricAboutTheCampsCentre(FUnitTest& T)
{
	FVector2D Center = HOLStealthInstall::CenterOfCamps(StealthTestCamps(), FVector2D(0.0, 0.0));
	TArray<FHOLStealthZone> Zones = HOLStealthInstall::ZonesForMap("Forest", Center, 6000.0);
	T.AssertTrue(Zones.Num() >= 2, "Forest has zones to compare");
	int Unmatched = 0;
	for (const FHOLStealthZone& Zone : Zones)
	{
		FVector2D Mirror = Center * 2.0 - Zone.Center;
		bool bFound = false;
		for (const FHOLStealthZone& Other : Zones)
		{
			if (Math::Abs(Other.Center.X - Mirror.X) < 1.0 && Math::Abs(Other.Center.Y - Mirror.Y) < 1.0 && Math::Abs(Other.HalfExtent.X - Zone.HalfExtent.X) < 1.0 && Math::Abs(Other.HalfExtent.Y - Zone.HalfExtent.Y) < 1.0)
				bFound = true;
		}
		if (!bFound)
			Unmatched++;
	}
	T.AssertEquals(0, Unmatched, "every Forest zone has its mirror (point symmetry about the camps' centre, same size)");
}

void Test_StealthCenterOfCampsIsTheMeanOfTheStartPositions(FUnitTest& T)
{
	FVector2D Center = HOLStealthInstall::CenterOfCamps(StealthTestCamps(), FVector2D(-777.0, -777.0));
	T.AssertTrue(Math::Abs(Center.X - 5000.0) < 1.0 && Math::Abs(Center.Y - 4000.0) < 1.0, "centre of two camps = their mean (5000, 4000)");
	TArray<FVector2D> NoCamps;
	FVector2D Fallback = HOLStealthInstall::CenterOfCamps(NoCamps, FVector2D(-777.0, 123.0));
	T.AssertTrue(Math::Abs(Fallback.X + 777.0) < 1.0 && Math::Abs(Fallback.Y - 123.0) < 1.0, "no camp position known: the centre of the map");
}

void Test_StealthOpenMapKeepsNoZone(FUnitTest& T)
{
	FVector2D Center = FVector2D(5000.0, 4000.0);
	T.AssertEquals(0, HOLStealthInstall::ZonesForMap("Plain", Center, 6000.0).Num(), "an open map has no zone");
	T.AssertEquals(0, HOLStealthInstall::ZonesForMap("", Center, 6000.0).Num(), "no map name: no zone");
}

void Test_StealthForestZonesStayInsideTheMap(FUnitTest& T)
{
	FVector2D Center = FVector2D(5000.0, 4000.0);
	float Half = 6000.0;
	TArray<FHOLStealthZone> Zones = HOLStealthInstall::ZonesForMap("Forest", Center, Half);
	T.AssertTrue(Zones.Num() >= 2, "Forest has zones to check");
	int Outside = 0;
	for (const FHOLStealthZone& Zone : Zones)
	{
		if (Math::Abs(Zone.Center.X - Center.X) + Zone.HalfExtent.X > Half || Math::Abs(Zone.Center.Y - Center.Y) + Zone.HalfExtent.Y > Half)
			Outside++;
	}
	T.AssertEquals(0, Outside, "every zone lies entirely inside the playable square");
}

// The real chain: the zones of the map feed the filter; a unit standing in a Forest zone is not sent to the enemy camp, the same position on an open map is.
void Test_StealthForestZoneHidesAUnitThroughTheFilter(FUnitTest& T)
{
	FVector2D Center = FVector2D(5000.0, 4000.0);
	TArray<FHOLStealthZone> ForestZones = HOLStealthInstall::ZonesForMap("Forest", Center, 6000.0);
	T.AssertTrue(ForestZones.Num() >= 1, "Forest has a zone");
	FVector2D InZone = Center;
	if (ForestZones.Num() >= 1)
		InZone = ForestZones[0].Center;
	UHOLStealthRuleFilter Forest = Cast<UHOLStealthRuleFilter>(NewObject(GetTransientPackage(), UHOLStealthRuleFilter::StaticClass()));
	Forest.Zones = ForestZones;
	int ForestMask = UHOLStealthFilter::MaskCamps(Forest, 3, 2, 0, InZone);
	UHOLStealthRuleFilter Open = Cast<UHOLStealthRuleFilter>(NewObject(GetTransientPackage(), UHOLStealthRuleFilter::StaticClass()));
	Open.Zones = HOLStealthInstall::ZonesForMap("Plain", Center, 6000.0);
	int OpenMask = UHOLStealthFilter::MaskCamps(Open, 3, 2, 0, InZone);
	T.AssertEquals(1, ForestMask, "Forest: the unit in a grove is hidden from camp 1, camp 0 keeps it");
	T.AssertEquals(3, OpenMask, "open map: the same position is seen by both camps");
}