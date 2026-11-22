// The own-units pass (UHOLStealthOwnApplier, HOLStealthOpacityApply.as): reads and writes are counted by a subclass that stands in for the
// binding (units fed by the test) and for the renderer (writes recorded). The opacity values asserted come from the rule HOLStealth::OwnUnitOpacity.

class UStealthTestApplier : UHOLStealthOwnApplier
{
	TArray<int> FedIds;
	TArray<FVector2D> FedPositions;
	int Reads = 0;
	TArray<int> WrittenIds;
	TArray<float> WrittenValues;

	void ReadOwnUnits(TArray<int>& UnitIds, TArray<FVector2D>& Positions) override
	{
		Reads++;
		UnitIds = FedIds;
		Positions = FedPositions;
	}

	void WriteOpacity(int UnitId, float Opacity) override
	{
		WrittenIds.Add(UnitId);
		WrittenValues.Add(Opacity);
	}
}

// One zone of 400 x 100 half extent at the origin (its bounding circle reaches (0, 300) though the zone does not).
TArray<FHOLStealthZone> StealthApplierZones()
{
	TArray<FHOLStealthZone> Zones;
	FHOLStealthZone Zone;
	Zone.Center = FVector2D(0.0, 0.0);
	Zone.HalfExtent = FVector2D(400.0, 100.0);
	Zones.Add(Zone);
	return Zones;
}

UStealthTestApplier MakeStealthTestApplier(bool bWithZones)
{
	UStealthTestApplier Applier = Cast<UStealthTestApplier>(NewObject(GetTransientPackage(), UStealthTestApplier::StaticClass()));
	if (bWithZones)
		Applier.SetZones(StealthApplierZones());
	return Applier;
}

void Test_StealthOwnApplierTwoPassesWithoutChangeWriteOnce(FUnitTest& T)
{
	UStealthTestApplier Applier = MakeStealthTestApplier(true);
	// 7 stands in the zone, 8 far away, 9 inside the zone's bounding circle but outside the zone itself.
	Applier.FedIds.Add(7);
	Applier.FedPositions.Add(FVector2D(0.0, 0.0));
	Applier.FedIds.Add(8);
	Applier.FedPositions.Add(FVector2D(5000.0, 0.0));
	Applier.FedIds.Add(9);
	Applier.FedPositions.Add(FVector2D(0.0, 300.0));
	Applier.Pass();
	int SecondWrites = Applier.Pass();
	T.AssertEquals(0, SecondWrites, "second pass, nothing moved: nothing written");
	T.AssertEquals(1, Applier.WrittenIds.Num(), "two passes: only the unit in the zone was written (once); the others stay opaque, unwritten");
	if (Applier.WrittenIds.Num() == 1)
	{
		T.AssertEquals(7, Applier.WrittenIds[0], "the unit written is the one in the zone");
		float Expected = HOLStealth::OwnUnitOpacity(StealthApplierZones(), FVector2D(0.0, 0.0));
		T.AssertTrue(Math::Abs(Applier.WrittenValues[0] - Expected) < 0.001, "the value written is the rule's opacity inside the zone");
	}
}

void Test_StealthOwnApplierUnitEnteringThenLeavingIsWrittenTwice(FUnitTest& T)
{
	UStealthTestApplier Applier = MakeStealthTestApplier(true);
	Applier.FedIds.Add(7);
	Applier.FedPositions.Add(FVector2D(5000.0, 0.0));
	Applier.Pass();
	Applier.FedPositions[0] = FVector2D(0.0, 0.0);
	Applier.Pass();
	Applier.Pass();
	Applier.FedPositions[0] = FVector2D(5000.0, 0.0);
	Applier.Pass();
	Applier.Pass();
	T.AssertEquals(2, Applier.WrittenIds.Num(), "enters then leaves: written twice (inside value, then opaque), no more");
	if (Applier.WrittenIds.Num() == 2)
	{
		float Inside = HOLStealth::OwnUnitOpacity(StealthApplierZones(), FVector2D(0.0, 0.0));
		float Outside = HOLStealth::OwnUnitOpacity(StealthApplierZones(), FVector2D(5000.0, 0.0));
		T.AssertTrue(Math::Abs(Applier.WrittenValues[0] - Inside) < 0.001, "first write = the opacity inside the zone");
		T.AssertTrue(Math::Abs(Applier.WrittenValues[1] - Outside) < 0.001, "second write = the opacity outside the zone");
	}
}

void Test_StealthOwnApplierWithoutZonesReadsNoUnit(FUnitTest& T)
{
	UStealthTestApplier Applier = MakeStealthTestApplier(false);
	Applier.FedIds.Add(7);
	Applier.FedPositions.Add(FVector2D(0.0, 0.0));
	int Writes = Applier.Pass();
	T.AssertEquals(0, Applier.Reads, "no zone on the map: the units are not read at all");
	T.AssertEquals(0, Writes, "no zone: nothing written");
}
