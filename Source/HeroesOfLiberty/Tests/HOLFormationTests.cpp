// Copyright Epic Games, Inc. All Rights Reserved.

#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLFormationRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// J7a checks (J7-Q3): the rules of the simple formation (rows, kinds in the rows, places, assignment).
namespace UE::HeroesOfLiberty::Tests::Formation
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Whether segments A-B and C-D cross (strictly, not just touch at an end). */
	bool Cross(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D)
	{
		const double ABC = FVector2D::CrossProduct(B - A, C - A);
		const double ABD = FVector2D::CrossProduct(B - A, D - A);
		const double CDA = FVector2D::CrossProduct(D - C, A - C);
		const double CDB = FVector2D::CrossProduct(D - C, B - C);
		return ABC * ABD < -1. && CDA * CDB < -1.;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationAssignTest, "HeroesOfLiberty.Formation.Assign", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationAssignTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Formation;
	TArray<int32> PlaceOfUnit;
	HOLFormation::AssignPlaces({}, {}, FVector2D(1., 0.), PlaceOfUnit);
	TestEqual(TEXT("no unit, nothing given"), PlaceOfUnit.Num(), 0);

	// A group already in the formation's shape keeps it: each unit to its own place, moved forward.
	TArray<int32> Widths;
	TArray<FVector2D> Places;
	HOLFormation::MakeSquareRows(12, Widths);
	HOLFormation::MakePlaces(Widths, FVector2D(5000., 0.), FVector2D(1., 0.), 150., 150., Places);
	TArray<FVector2D> Positions;
	for (const FVector2D& Place : Places)
	{
		Positions.Add(Place - FVector2D(3000., 0.));
	}
	HOLFormation::AssignPlaces(Positions, Places, FVector2D(1., 0.), PlaceOfUnit);
	bool bKept = PlaceOfUnit.Num() == Places.Num();
	for (int32 Unit = 0; bKept && Unit < PlaceOfUnit.Num(); ++Unit)
	{
		bKept = PlaceOfUnit[Unit] == Unit;
	}
	TestTrue(TEXT("a group in shape keeps its shape"), bKept);

	// Scattered groups, near and far, ahead and on the side: every unit gets its own place and no two paths cross.
	FRandomStream Random(7);
	for (int32 Trial = 0; Trial < 40; ++Trial)
	{
		const int32 Count = Random.RandRange(2, 60);
		const int32 ExtraPlaces = Trial % 4 == 0 ? Random.RandRange(1, 5) : 0;
		const FVector2D Forward(Random.FRandRange(-1., 1.), Random.FRandRange(-1., 1.));
		HOLFormation::MakeMixedRows(Count + ExtraPlaces, Widths);
		HOLFormation::MakePlaces(Widths, FVector2D(Random.FRandRange(-3000., 3000.), Random.FRandRange(-3000., 3000.)), Forward,
			150., 150., Places);
		Positions.Reset();
		const double Spread = Random.FRandRange(300., 4000.);
		for (int32 Unit = 0; Unit < Count; ++Unit)
		{
			Positions.Emplace(Random.FRandRange(-Spread, Spread), Random.FRandRange(-Spread, Spread));
		}
		HOLFormation::AssignPlaces(Positions, Places, Forward, PlaceOfUnit);

		TSet<int32> Given;
		bool bAllGiven = PlaceOfUnit.Num() == Count;
		for (const int32 Place : PlaceOfUnit)
		{
			bAllGiven &= Places.IsValidIndex(Place) && !Given.Contains(Place);
			Given.Add(Place);
		}
		TestTrue(FString::Printf(TEXT("trial %d: a place of its own for each of %d units"), Trial, Count), bAllGiven);
		if (!bAllGiven)
		{
			continue;
		}
		// By depth (J7c.2): the rows filled front first by the units furthest forward (a 60 cm tie either side: 120 cm
		// out of order at most), and no two paths of one row cross (paths of different rows may, on a sidestep).
		TArray<int32> RowOfPlace;
		for (int32 Row = 0; Row < Widths.Num(); ++Row)
		{
			for (int32 Column = 0; Column < Widths[Row]; ++Column)
			{
				RowOfPlace.Add(Row);
			}
		}
		const FVector2D Front = Forward.GetSafeNormal();
		int32 Crossings = 0;
		int32 OutOfOrder = 0;
		for (int32 First = 0; First < Count; ++First)
		{
			for (int32 Second = First + 1; Second < Count; ++Second)
			{
				const int32 RowA = RowOfPlace[PlaceOfUnit[First]];
				const int32 RowB = RowOfPlace[PlaceOfUnit[Second]];
				if (RowA == RowB)
				{
					Crossings += Cross(Positions[First], Places[PlaceOfUnit[First]], Positions[Second], Places[PlaceOfUnit[Second]]) ? 1 : 0;
				}
				else
				{
					const int32 Ahead = RowA < RowB ? First : Second;
					const int32 Behind = RowA < RowB ? Second : First;
					OutOfOrder += (Positions[Ahead] | Front) < (Positions[Behind] | Front) - 120.5 ? 1 : 0;
				}
			}
		}
		TestEqual(FString::Printf(TEXT("trial %d: no paths of one row cross (%d units, %d places)"), Trial, Count, Places.Num()), Crossings, 0);
		TestEqual(FString::Printf(TEXT("trial %d: the rows filled front first"), Trial), OutOfOrder, 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationCorridorTest, "HeroesOfLiberty.Formation.Corridor", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationCorridorTest::RunTest(const FString& Parameters)
{
	// MV.5 (the user's rule: B if the detour is not too long, otherwise A): 3 files 150 cm apart, a sample every 100 cm,
	// an obstacle over samples 20 to 29 of 50.
	const double Step = 100.;
	const double Slope = FMath::Tan(FMath::DegreesToRadians(HOLFormation::FileSlideMaxAngle));
	const double MinY = -150.;
	const double MaxY = 150.;
	auto Make = [&](const double Left, const double Right, const double MaxShift, TArray<double>& Shift, TArray<double>& Squeeze)
	{
		TArray<double> FreeLeft;
		TArray<double> FreeRight;
		FreeLeft.Init(1000., 50);
		FreeRight.Init(1000., 50);
		for (int32 Sample = 20; Sample < 30; ++Sample)
		{
			FreeLeft[Sample] = Left;
			FreeRight[Sample] = Right;
		}
		HOLFormation::MakeCorridor(FreeLeft, FreeRight, Step, MinY, MaxY, MaxShift, Slope, Shift, Squeeze);
		bool bInside = Shift.Num() == 50 && Squeeze.Num() == 50;
		bool bSlope = bInside;
		for (int32 Sample = 0; bInside && Sample < 50; ++Sample)
		{
			bInside &= Shift[Sample] + Squeeze[Sample] * MinY >= -FreeLeft[Sample] - 1e-6
				&& Shift[Sample] + Squeeze[Sample] * MaxY <= FreeRight[Sample] + 1e-6 && FMath::Abs(Shift[Sample]) <= MaxShift + 1e-6;
			if (Sample > 0)
			{
				bSlope &= FMath::Abs(Shift[Sample] - Shift[Sample - 1]) <= Slope * Step + 1e-6
					&& FMath::Abs(Squeeze[Sample] - Squeeze[Sample - 1]) * MaxY <= Slope * Step + 1e-6;
			}
		}
		TestTrue(TEXT("files inside the free interval"), bInside);
		TestTrue(TEXT("slope bounded"), bSlope);
	};
	TArray<double> Shift;
	TArray<double> Squeeze;
	// Open field: nothing changes.
	Make(1000., 1000., 300., Shift, Squeeze);
	bool bOpen = true;
	for (int32 Sample = 0; Sample < Shift.Num(); ++Sample)
	{
		bOpen &= FMath::IsNearlyZero(Shift[Sample]) && FMath::IsNearlyEqual(Squeeze[Sample], 1.);
	}
	TestTrue(TEXT("open field: no shift, no squeeze"), bOpen);
	// B: an obstacle 60 cm right of the path, room on the left: the block shifts left, 90 cm, ahead of time.
	Make(1000., 60., 300., Shift, Squeeze);
	bool bWhole = true;
	for (int32 Sample = 0; Sample < Squeeze.Num(); ++Sample)
	{
		bWhole &= FMath::IsNearlyEqual(Squeeze[Sample], 1.);
	}
	TestTrue(TEXT("B: no squeeze"), bWhole);
	TestEqual(TEXT("B: shifted left by what fits"), Shift[25], -90., 1e-6);
	TestTrue(TEXT("B: the shift starts before the obstacle"), Shift[18] < -1.);
	TestTrue(TEXT("B: back on the path after it"), FMath::IsNearlyZero(Shift[40]));
	// A: a gap 200 cm wide for 300 cm of files: they squeeze there, ahead of time, and reopen after.
	Make(100., 100., 300., Shift, Squeeze);
	TestEqual(TEXT("A: squeezed in the gap"), Squeeze[25], 2. / 3., 1e-6);
	TestTrue(TEXT("A: the squeeze starts before the gap"), Squeeze[19] < 1. - 1e-3);
	TestTrue(TEXT("A: reopened after it"), FMath::IsNearlyEqual(Squeeze[40], 1.));
	// The shift needed (150 cm) beyond the cap (100 cm): shifted by the cap, squeezed for the rest.
	Make(1000., 0., 100., Shift, Squeeze);
	TestEqual(TEXT("cap: shifted by the cap"), Shift[25], -100., 1e-6);
	TestEqual(TEXT("cap: squeezed for the rest"), Squeeze[25], 100. / 150., 1e-6);
	double At = 0.;
	double Scale = 0.;
	HOLFormation::GetCorridorAt(Shift, Squeeze, Step, 2550., At, Scale);
	TestEqual(TEXT("read between samples"), At, -100., 1e-6);
	HOLFormation::GetCorridorAt({}, {}, Step, 100., At, Scale);
	TestTrue(TEXT("no corridor: no change"), At == 0. && Scale == 1.);
	return true;
}


#endif
