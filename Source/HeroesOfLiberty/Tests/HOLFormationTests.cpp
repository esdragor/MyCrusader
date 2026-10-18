// Copyright Epic Games, Inc. All Rights Reserved.

#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLFormationRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// J7a checks (J7-Q3): the rules of the simple formation (rows, kinds in the rows, places, assignment).
namespace UE::HeroesOfLiberty::Tests::Formation
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Row widths written as the user writes them: "7/7/5/5". */
	FString WidthsText(TConstArrayView<int32> Widths)
	{
		FString Text;
		for (const int32 Width : Widths)
		{
			Text += (Text.IsEmpty() ? TEXT("") : TEXT("/")) + FString::FromInt(Width);
		}
		return Text;
	}

	/** A formation written as the user draws it, front row first, each row from left to right: "7L/3L2H2L/5H/5H". */
	FString FormationText(TConstArrayView<int32> Widths, TConstArrayView<int32> KindOfPlace)
	{
		static const TCHAR* const Letters = TEXT("LHA");
		FString Text;
		int32 Place = 0;
		for (int32 Row = 0; Row < Widths.Num(); ++Row)
		{
			Text += Row > 0 ? TEXT("/") : TEXT("");
			for (int32 Column = 0; Column < Widths[Row];)
			{
				const int32 Kind = KindOfPlace[Place + Column];
				int32 Run = 0;
				while (Column < Widths[Row] && KindOfPlace[Place + Column] == Kind)
				{
					++Run;
					++Column;
				}
				Text += FString::Printf(TEXT("%d%c"), Run, Kind >= 0 && Kind < 3 ? Letters[Kind] : TEXT('?'));
			}
			Place += Widths[Row];
		}
		return Text;
	}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationRowsTest, "HeroesOfLiberty.Formation.Rows", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationRowsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Formation;
	TArray<int32> Widths;

	// Peasants: one row up to 5, then rows of ceil(sqrt(2 N)).
	for (const TPair<int32, const TCHAR*>& Case : TArray<TPair<int32, const TCHAR*>>{ { 0, TEXT("") }, { 1, TEXT("1") },
		{ 5, TEXT("5") }, { 6, TEXT("4/2") }, { 10, TEXT("5/5") }, { 15, TEXT("6/6/3") }, { 20, TEXT("7/7/6") }, { 50, TEXT("10/10/10/10/10") } })
	{
		HOLFormation::MakePeasantRows(Case.Key, Widths);
		TestEqual(FString::Printf(TEXT("%d Peasants"), Case.Key), WidthsText(Widths), FString(Case.Value));
	}

	// One kind of soldier: a square.
	for (const TPair<int32, const TCHAR*>& Case : TArray<TPair<int32, const TCHAR*>>{ { 1, TEXT("1") }, { 12, TEXT("4/4/4") },
		{ 20, TEXT("5/5/5/5") }, { 10, TEXT("4/4/2") } })
	{
		HOLFormation::MakeSquareRows(Case.Key, Widths);
		TestEqual(FString::Printf(TEXT("%d soldiers of one kind"), Case.Key), WidthsText(Widths), FString(Case.Value));
	}

	// Several kinds: odd rows, the remainder on the front rows or one more row.
	for (const TPair<int32, const TCHAR*>& Case : TArray<TPair<int32, const TCHAR*>>{ { 1, TEXT("1") }, { 2, TEXT("2") },
		{ 4, TEXT("3/1") }, { 19, TEXT("5/5/5/4") }, { 20, TEXT("5/5/5/5") }, { 24, TEXT("7/7/5/5") },
		{ 40, TEXT("9/9/8/7/7") }, { 60, TEXT("11/11/11/9/9/9") } })
	{
		HOLFormation::MakeMixedRows(Case.Key, Widths);
		TestEqual(FString::Printf(TEXT("%d soldiers of several kinds"), Case.Key), WidthsText(Widths), FString(Case.Value));
	}

	// The drag's width: full rows, the last one shorter, never wider than the group.
	HOLFormation::MakeRowsOfWidth(17, 6, Widths);
	TestEqual(TEXT("17 in rows of 6"), WidthsText(Widths), FString(TEXT("6/6/5")));
	HOLFormation::MakeRowsOfWidth(3, 10, Widths);
	TestEqual(TEXT("3 in rows of 10"), WidthsText(Widths), FString(TEXT("3")));
	HOLFormation::MakeRowsOfWidth(3, 0, Widths);
	TestEqual(TEXT("a width of 0 is one column"), WidthsText(Widths), FString(TEXT("1/1/1")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationKindsTest, "HeroesOfLiberty.Formation.Kinds", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationKindsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Formation;

	// The user's drawings (L Spearman, H Man-at-arms, A Archer; front row first, left to right). The odd wing has its
	// extra unit on the left: the first drawing's "2 L 2 H 3 L" reads "3 L 2 H 2 L".
	struct FScenario
	{
		TArray<int32> CountOfKind;
		const TCHAR* Expected;
	};
	const TArray<FScenario> Scenarios = {
		{ { 12, 12 }, TEXT("7L/3L2H2L/5H/5H") },
		{ { 10, 10 }, TEXT("5L/5L/5H/5H") },
		{ { 14, 5 }, TEXT("5L/5L/2L1H2L/4H") },
		{ { 7, 17 }, TEXT("7L/7H/5H/5H") },
		{ { 3, 21 }, TEXT("2H3L2H/7H/5H/5H") },
		{ { 8, 8, 8 }, TEXT("7L/1L6H/1H3A1H/5A") },
		{ { 3, 1 }, TEXT("3L/1H") },
		{ { 30, 30 }, TEXT("11L/11L/4L3H4L/9H/9H/9H") },
		{ { 20, 20 }, TEXT("9L/9L/1L6H1L/7H/7H") },
	};
	TArray<int32> Widths;
	TArray<int32> KindOfPlace;
	for (const FScenario& Scenario : Scenarios)
	{
		int32 Count = 0;
		FString Name;
		for (const int32 KindCount : Scenario.CountOfKind)
		{
			Count += KindCount;
			Name += (Name.IsEmpty() ? TEXT("") : TEXT(" + ")) + FString::FromInt(KindCount);
		}
		HOLFormation::MakeMixedRows(Count, Widths);
		HOLFormation::PlaceKinds(Widths, Scenario.CountOfKind, KindOfPlace);
		TestEqual(Name, FormationText(Widths, KindOfPlace), FString(Scenario.Expected));
	}

	// A kind with no unit is skipped; one kind fills its square.
	HOLFormation::MakeMixedRows(4, Widths);
	HOLFormation::PlaceKinds(Widths, { 3, 0, 1 }, KindOfPlace);
	TestEqual(TEXT("an empty kind skipped"), FormationText(Widths, KindOfPlace), FString(TEXT("3L/1A")));
	HOLFormation::MakeSquareRows(12, Widths);
	HOLFormation::PlaceKinds(Widths, { 12 }, KindOfPlace);
	TestEqual(TEXT("one kind"), FormationText(Widths, KindOfPlace), FString(TEXT("4L/4L/4L")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationPlacesTest, "HeroesOfLiberty.Formation.Places", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationPlacesTest::RunTest(const FString& Parameters)
{
	// Marching along +X from (1000, 0): the front row on the target, the next one 100 cm behind; the right side is +Y.
	TArray<FVector2D> Places;
	HOLFormation::MakePlaces({ 3, 2 }, FVector2D(1000., 0.), FVector2D(5., 0.), 150., 100., Places);
	TestEqual(TEXT("a place per unit"), Places.Num(), 5);
	TestTrue(TEXT("front row, left"), Places[0].Equals(FVector2D(1000., -150.), 0.01));
	TestTrue(TEXT("front row, middle on the target"), Places[1].Equals(FVector2D(1000., 0.), 0.01));
	TestTrue(TEXT("front row, right"), Places[2].Equals(FVector2D(1000., 150.), 0.01));
	TestTrue(TEXT("even row at -0.5 spacing"), Places[3].Equals(FVector2D(900., -75.), 0.01));
	TestTrue(TEXT("even row at +0.5 spacing"), Places[4].Equals(FVector2D(900., 75.), 0.01));

	// Marching along +Y: the right side is -X (HOLGroupMove::ToWorld), the rows behind towards -Y.
	HOLFormation::MakePlaces({ 2 }, FVector2D::ZeroVector, FVector2D(0., 1.), 100., 100., Places);
	TestTrue(TEXT("turned: left end"), Places[0].Equals(FVector2D(50., 0.), 0.01));
	TestTrue(TEXT("turned: right end"), Places[1].Equals(FVector2D(-50., 0.), 0.01));
	return true;
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
		int32 Crossings = 0;
		for (int32 First = 0; First < Count; ++First)
		{
			for (int32 Second = First + 1; Second < Count; ++Second)
			{
				Crossings += Cross(Positions[First], Places[PlaceOfUnit[First]], Positions[Second], Places[PlaceOfUnit[Second]]) ? 1 : 0;
			}
		}
		TestEqual(FString::Printf(TEXT("trial %d: no paths cross (%d units, %d places)"), Trial, Count, Places.Num()), Crossings, 0);
	}
	return true;
}

#endif
