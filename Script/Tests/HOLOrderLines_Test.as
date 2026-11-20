// BUG-order_lines_multi_blocs (2026-11-16, ELROND-1120d): a selection of units from several blocks draws one movement line per
// block, from the centre of that block; units of one block share one line. The rule is HOLFormation::FindBlockCentres
// (HOLFormationRules.h, read by AHOLSelectionMarkers::UpdateOrderLines), exposed as HOLFormationRead::FindBlockCentres.
// Values asserted are computed by the rule from the units' own locations, never copied from a constant.
// The link distance is HOLFormation::BlockJoinDistance (450 cm), the one the order processor cuts blocks with.

const double OLJoin = 450.0;

FVector OLMean(const TArray<FVector>& Locations, int From, int To)
{
	FVector Sum = FVector::ZeroVector;
	for (int I = From; I < To; I++)
	{
		Sum += Locations[I];
	}
	return Sum / double(To - From);
}

// Three units around (X, Y): a block 150 cm apart.
void OLAddBlock(TArray<FVector>& Locations, double X, double Y)
{
	Locations.Add(FVector(X, Y, 0.0));
	Locations.Add(FVector(X + 150.0, Y, 0.0));
	Locations.Add(FVector(X, Y + 150.0, 0.0));
}

void Test_OrderLines_TwoBlocksTwoStarts(FUnitTest& T)
{
	TArray<FVector> Locations;
	OLAddBlock(Locations, 0.0, 0.0);
	OLAddBlock(Locations, 3000.0, 0.0);
	TArray<FVector> Centres;
	int Count = HOLFormationRead::FindBlockCentres(Locations, OLJoin, Centres);
	T.AssertEquals(2, Count, "two blocks 30 m apart, two lines");
	T.AssertEquals(2, Centres.Num(), "two start points returned");
	if (Centres.Num() != 2)
	{
		return;
	}
	// Each start is the middle of ITS block, not the middle of the whole selection.
	FVector MeanA = OLMean(Locations, 0, 3);
	FVector MeanB = OLMean(Locations, 3, 6);
	FVector MeanAll = OLMean(Locations, 0, 6);
	bool bAThenB = Centres[0].Distance(MeanA) < 1.0 && Centres[1].Distance(MeanB) < 1.0;
	bool bBThenA = Centres[0].Distance(MeanB) < 1.0 && Centres[1].Distance(MeanA) < 1.0;
	T.AssertTrue(bAThenB || bBThenA, f"each start is the centre of its block ({Centres[0]} and {Centres[1]})");
	T.AssertTrue(Centres[0].Distance(MeanAll) > 1000.0 && Centres[1].Distance(MeanAll) > 1000.0, "no start at the middle of nowhere between the blocks");
}

void Test_OrderLines_OneBlockOneStart(FUnitTest& T)
{
	TArray<FVector> Locations;
	OLAddBlock(Locations, 500.0, -200.0);
	Locations.Add(FVector(650.0, -50.0, 0.0));
	Locations.Add(FVector(350.0, -50.0, 0.0));
	TArray<FVector> Centres;
	int Count = HOLFormationRead::FindBlockCentres(Locations, OLJoin, Centres);
	T.AssertEquals(1, Count, "units of one block share one line");
	if (Centres.Num() == 1)
	{
		FVector Mean = OLMean(Locations, 0, Locations.Num());
		T.AssertTrue(Centres[0].Distance(Mean) < 1.0, f"the one start is the centre of the block ({Centres[0]} vs {Mean})");
	}
}

void Test_OrderLines_InterleavedSelectionStillGroupsByBlock(FUnitTest& T)
{
	// The selection lists its units in any order (shift-click): A1 B1 A2 B2 are still two blocks.
	TArray<FVector> Locations;
	Locations.Add(FVector(0.0, 0.0, 0.0));
	Locations.Add(FVector(4000.0, 0.0, 0.0));
	Locations.Add(FVector(150.0, 0.0, 0.0));
	Locations.Add(FVector(4150.0, 0.0, 0.0));
	TArray<FVector> Centres;
	T.AssertEquals(2, HOLFormationRead::FindBlockCentres(Locations, OLJoin, Centres), "interleaved units: two blocks, two lines");
}

void Test_OrderLines_BlockAndLoneUnit(FUnitTest& T)
{
	// A unit alone is a block of one: its own line, from where it stands (the body keeps its own).
	TArray<FVector> Locations;
	OLAddBlock(Locations, 0.0, 0.0);
	Locations.Add(FVector(5000.0, 2000.0, 0.0));
	TArray<FVector> Centres;
	int Count = HOLFormationRead::FindBlockCentres(Locations, OLJoin, Centres);
	T.AssertEquals(2, Count, "a block and a lone unit, two lines");
	if (Centres.Num() == 2)
	{
		T.AssertTrue(Centres[1].Distance(Locations[3]) < 1.0, f"the lone unit's line starts at the unit ({Centres[1]})");
	}
}

void Test_OrderLines_NoUnitsNoStart(FUnitTest& T)
{
	TArray<FVector> Locations;
	TArray<FVector> Centres;
	T.AssertEquals(0, HOLFormationRead::FindBlockCentres(Locations, OLJoin, Centres), "no units, no line");
	T.AssertEquals(0, Centres.Num(), "no start point");
}
