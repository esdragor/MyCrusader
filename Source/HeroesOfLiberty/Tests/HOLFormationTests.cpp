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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationGroupTest, "HeroesOfLiberty.Formation.Group", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationGroupTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Formation;
	// The shape of a group's rows by its kinds (J7a.4): Peasants alone, a single kind, several kinds.
	TArray<int32> Widths;
	HOLFormation::MakeGroupRows({ 15 }, true, Widths);
	TestEqual(TEXT("15 Peasants: 6/6/3"), WidthsText(Widths), FString(TEXT("6/6/3")));
	HOLFormation::MakeGroupRows({ 0, 12, 0 }, false, Widths);
	TestEqual(TEXT("12 Men-at-arms alone: a square 4/4/4"), WidthsText(Widths), FString(TEXT("4/4/4")));
	HOLFormation::MakeGroupRows({ 12, 12 }, false, Widths);
	TestEqual(TEXT("12 Spearmen and 12 Men-at-arms: 7/7/5/5"), WidthsText(Widths), FString(TEXT("7/7/5/5")));

	// 3 Spearmen and 3 Men-at-arms marching along +X to (5000, 0), mixed up: the Spearmen in front, the target in the
	// middle of the front row, each unit a place of its own kind.
	const TArray<FVector2D> Positions = { { 0., 0. }, { 0., 200. }, { 200., 0. }, { 200., 200. }, { -200., 0. }, { -200., 200. } };
	const TArray<int32> KindOfUnit = { 1, 0, 0, 1, 1, 0 };
	TArray<FVector2D> Places;
	TArray<int32> PlaceOfUnit;
	int32 Projected = 0;
	HOLFormation::PlaceGroup(Positions, KindOfUnit, false, FVector2D(5000., 0.), FVector2D(1., 0.), 150., 180.,
		[&Projected](const FVector2D& Place) { ++Projected; return Place; }, Places, PlaceOfUnit);
	TestEqual(TEXT("one projection per place"), Projected, Places.Num());
	TestEqual(TEXT("6 places"), Places.Num(), 6);
	bool bOwn = PlaceOfUnit.Num() == 6;
	TSet<int32> Given;
	for (int32 Unit = 0; bOwn && Unit < 6; ++Unit)
	{
		const int32 Place = PlaceOfUnit[Unit];
		bOwn = Places.IsValidIndex(Place) && !Given.Contains(Place);
		Given.Add(Place);
		// Spearmen on the front row (x = 5000), Men-at-arms on the row behind (x = 4820).
		bOwn = bOwn && FMath::IsNearlyEqual(Places[Place].X, KindOfUnit[Unit] == 0 ? 5000. : 4820., 0.5);
	}
	TestTrue(TEXT("each unit a place of its own, Spearmen in front, Men-at-arms 180 cm behind"), bOwn);

	// A place the navmesh does not have is brought back: the projection's point is the place given.
	HOLFormation::PlaceGroup(MakeArrayView(Positions.GetData(), 5), { 4, 4, 4, 4, 4 }, true, FVector2D(0., 1000.), FVector2D(0., 1.), 150., 180.,
		[](const FVector2D& Place) { return FVector2D(FMath::Min(Place.X, 100.), Place.Y); }, Places, PlaceOfUnit);
	bool bClamped = Places.Num() == 5 && PlaceOfUnit.Num() == 5 && !PlaceOfUnit.Contains(INDEX_NONE);
	for (const FVector2D& Place : Places)
	{
		bClamped &= Place.X <= 100. && FMath::IsNearlyEqual(Place.Y, 1000., 0.5);
	}
	TestTrue(TEXT("5 Peasants: one row across the march, the projected places"), bClamped);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationFrontTest, "HeroesOfLiberty.Formation.Front", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationFrontTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Formation;
	// The front's width from the right-button drag (J7a.4): a column at each end, one every 150 cm.
	TestEqual(TEXT("drag of 0 cm: 1 column"), HOLFormation::GetFrontWidth(0., 150., 10), 1);
	TestEqual(TEXT("drag of 149 cm: 1 column"), HOLFormation::GetFrontWidth(149., 150., 10), 1);
	TestEqual(TEXT("drag of 150 cm: 2 columns"), HOLFormation::GetFrontWidth(150., 150., 10), 2);
	TestEqual(TEXT("drag of 600 cm: 5 columns"), HOLFormation::GetFrontWidth(600., 150., 10), 5);
	TestEqual(TEXT("drag of 3000 cm, 6 units: capped at 6"), HOLFormation::GetFrontWidth(3000., 150., 6), 6);
	TestEqual(TEXT("negative length: 1 column"), HOLFormation::GetFrontWidth(-50., 150., 6), 1);

	// Facing (the user's 2nd trial): along the arrow, from the press to the mouse.
	auto Near = [](const FVector2D& A, const FVector2D& B) { return A.Equals(B, 1e-6); };
	TestTrue(TEXT("arrow along +Y: faces +Y"), Near(HOLFormation::GetFrontFacing(FVector2D(0., 300.)), FVector2D(0., 1.)));
	TestTrue(TEXT("arrow along -X: faces -X"), Near(HOLFormation::GetFrontFacing(FVector2D(-500., 0.)), FVector2D(-1., 0.)));
	TestTrue(TEXT("diagonal arrow: normalised"), Near(HOLFormation::GetFrontFacing(FVector2D(300., 300.)), FVector2D(1., 1.).GetSafeNormal()));
	TestTrue(TEXT("null arrow: +X"), Near(HOLFormation::GetFrontFacing(FVector2D::ZeroVector), FVector2D(1., 0.)));

	// The drag's width replaces the default: 12 Spearmen and 12 Men-at-arms on a 1050 cm front (8 columns): 8/8/8,
	// the Spearmen first, then the Men-at-arms, rows front to back.
	TArray<FVector2D> Positions;
	TArray<int32> KindOfUnit;
	for (int32 Unit = 0; Unit < 24; ++Unit)
	{
		Positions.Add(FVector2D(-1000. - 200. * (Unit / 6), 200. * (Unit % 6)));
		KindOfUnit.Add(Unit < 12 ? 0 : 1);
	}
	TArray<FVector2D> Places;
	TArray<int32> PlaceOfUnit;
	const int32 Width = HOLFormation::GetFrontWidth(1050., 150., Positions.Num());
	HOLFormation::PlaceGroup(Positions, KindOfUnit, false, FVector2D(0., 500.), FVector2D(1., 0.), 150., 180.,
		[](const FVector2D& Place) { return Place; }, Places, PlaceOfUnit, Width);
	TestEqual(TEXT("8 columns"), Width, 8);
	TestEqual(TEXT("24 places"), Places.Num(), 24);
	int32 FrontSpearmen = 0;
	bool bInRows = PlaceOfUnit.Num() == 24;
	for (int32 Unit = 0; bInRows && Unit < 24; ++Unit)
	{
		const FVector2D& Place = Places[PlaceOfUnit[Unit]];
		bInRows = FMath::IsNearlyEqual(Place.X, 0., 0.5) || FMath::IsNearlyEqual(Place.X, -180., 0.5) || FMath::IsNearlyEqual(Place.X, -360., 0.5);
		bInRows = bInRows && FMath::Abs(Place.Y - 500.) <= 3.5 * 150. + 0.5;
		FrontSpearmen += KindOfUnit[Unit] == 0 && FMath::IsNearlyEqual(Place.X, 0., 0.5) ? 1 : 0;
	}
	TestTrue(TEXT("3 rows of 8, centred on the drag's middle"), bInRows);
	TestEqual(TEXT("the front row: 8 Spearmen"), FrontSpearmen, 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationSpeedTest, "HeroesOfLiberty.Formation.Speed", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationSpeedTest::RunTest(const FString& Parameters)
{
	// A group in formation walks at its slowest walker's speed (J7a.4); no group (0), its own speed.
	TestEqual(TEXT("a fast unit of a slower group: the group's speed"), HOLFormation::GetMarchSpeed(467.f, 420.f), 420.f);
	TestEqual(TEXT("the slowest itself: its own speed"), HOLFormation::GetMarchSpeed(420.f, 420.f), 420.f);
	TestEqual(TEXT("never faster than its own"), HOLFormation::GetMarchSpeed(300.f, 420.f), 300.f);
	TestEqual(TEXT("no group: its own speed"), HOLFormation::GetMarchSpeed(467.f, 0.f), 467.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationReturnTest, "HeroesOfLiberty.Formation.Return", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationReturnTest::RunTest(const FString& Parameters)
{
	// Back to its place after a push (J7a.4): radius 60 cm, the place at the origin.
	const double Radius = 60.;
	const FVector2D Place(0., 0.);
	HOLFormation::FPlaceHold Hold;
	HOLFormation::EndWalkToPlace(Hold, FVector2D(30., 0.), Place, Radius);
	TestTrue(TEXT("stopped 30 cm from its place: it holds it"), Hold.bHeld);

	// Standing on it, or nudged less than a radius: never a walk back.
	bool bWalked = false;
	for (int32 Step = 0; Step < 100; ++Step)
	{
		bWalked |= HOLFormation::WatchPlace(Hold, FVector2D(Step % 2 == 0 ? 50. : -50., 0.), Place, Radius);
	}
	TestFalse(TEXT("nudged 50 cm about its place (two idle neighbours): it stays"), bWalked);

	// Pushed 150 cm away: while the push goes on (2.5 cm farther each step), it waits; then 5 steps unpushed, it walks
	// back, even while it still drifts a little (two bodies easing apart: 1.5 cm a step, sideways).
	int32 FirstWalk = INDEX_NONE;
	for (int32 Step = 0; Step < 60 && FirstWalk == INDEX_NONE; ++Step)
	{
		const FVector2D At = Step < 20 ? FVector2D(100. + 2.5 * Step, 0.) : FVector2D(150., 1.5 * (Step - 20));
		if (HOLFormation::WatchPlace(Hold, At, Place, Radius))
		{
			FirstWalk = Step;
		}
	}
	TestEqual(TEXT("walks back 5 steps (0.25 s) after the push, drifting or not"), FirstWalk, 20 + HOLFormation::ReturnRestSteps);
	TestTrue(TEXT("on its way back"), Hold.bReturning);
	TestFalse(TEXT("no second walk while it returns"), HOLFormation::WatchPlace(Hold, FVector2D(150., 0.), Place, Radius));
	HOLFormation::EndWalkToPlace(Hold, FVector2D(10., 0.), Place, Radius);
	TestTrue(TEXT("back on it: it holds it again"), Hold.bHeld && !Hold.bReturning);

	// A shove (10 cm in a step) is a push, even sideways.
	HOLFormation::WatchPlace(Hold, FVector2D(200., 0.), Place, Radius);
	int32 Walk = INDEX_NONE;
	for (int32 Step = 0; Step < 10 && Walk == INDEX_NONE; ++Step)
	{
		Walk = HOLFormation::WatchPlace(Hold, FVector2D(200., Step < 3 ? 10. * (Step + 1) : 30.), Place, Radius) ? Step : INDEX_NONE;
	}
	TestEqual(TEXT("shoved sideways 3 steps: walks back 5 steps after"), Walk, 2 + HOLFormation::ReturnRestSteps);

	// Walks back that end short (the place taken): tried again, MaxReturnTries in a row at most, then let go.
	int32 Walks = 1;
	for (int32 Try = 1; Try < 10 && Hold.bHeld; ++Try)
	{
		HOLFormation::EndWalkToPlace(Hold, FVector2D(120., 0.), Place, Radius);
		for (int32 Step = 0; Step < 40 && Hold.bHeld && !Hold.bReturning; ++Step)
		{
			Walks += HOLFormation::WatchPlace(Hold, FVector2D(120., 0.), Place, Radius) ? 1 : 0;
		}
	}
	TestEqual(TEXT("ends short each time: walks back 3 times in all"), Walks, HOLFormation::MaxReturnTries);
	TestFalse(TEXT("then lets its place go"), Hold.bHeld);
	bool bWalkedAgain = false;
	for (int32 Step = 0; Step < 100; ++Step)
	{
		bWalkedAgain |= HOLFormation::WatchPlace(Hold, FVector2D(120., 0.), Place, Radius);
	}
	TestFalse(TEXT("then never walks back"), bWalkedAgain);

	// An arrival far from its place (taken, out of reach): no hold.
	HOLFormation::EndWalkToPlace(Hold, FVector2D(0., 90.), Place, Radius);
	TestFalse(TEXT("arrived 90 cm from its place: does not hold it"), Hold.bHeld);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFormationBlockTest, "HeroesOfLiberty.Formation.Block", UE::HeroesOfLiberty::Tests::Formation::Flags)
bool FHOLFormationBlockTest::RunTest(const FString& Parameters)
{
	// The block march (J7a.4): a unit's slot is the block's point on the path plus its offset (ahead, right) turned with the path.
	const TArray<FVector2D> Straight = { FVector2D(0., 0.), FVector2D(1000., 0.) };
	const FVector2D Fallback(1., 0.);
	TestTrue(TEXT("straight: 180 cm behind, 150 to the right of the point 500 cm along"),
		HOLFormation::GetSlotOnPath(Straight, 500., FVector2D(-180., 150.), Fallback).Equals(FVector2D(320., 150.), 0.01));
	TestTrue(TEXT("past the end: the end"), HOLFormation::GetPointOnLine(Straight, 3000.).Equals(FVector2D(1000., 0.), 0.01));
	TestTrue(TEXT("before the start: the start"), HOLFormation::GetPointOnLine(Straight, -50.).Equals(FVector2D(0., 0.), 0.01));
	const TArray<FVector2D> Corner = { FVector2D(0., 0.), FVector2D(1000., 0.), FVector2D(1000., 1000.) };
	TestEqual(TEXT("length of an L"), HOLFormation::GetLineLength(Corner), 2000., 0.01);
	const FVector2D AtCorner = HOLFormation::GetDirectionOnLine(Corner, 1000., HOLFormation::BlockTurnWindow, Fallback);
	TestTrue(TEXT("at the corner: half turned (45 degrees)"), AtCorner.Equals(FVector2D(1., 1.).GetSafeNormal(), 0.001));
	TestTrue(TEXT("after the corner: along +Y, the right-hand side towards -X"),
		HOLFormation::GetSlotOnPath(Corner, 1500., FVector2D(0., 150.), Fallback).Equals(FVector2D(850., 500.), 0.01));
	// Two units of the block keep their gap on a straight part, wherever the block is.
	const FVector2D A(0., -150.), B(-180., 150.);
	for (const double Distance : { 100., 400., 700. })
	{
		const double Gap = FVector2D::Distance(HOLFormation::GetSlotOnPath(Straight, Distance, A, Fallback), HOLFormation::GetSlotOnPath(Straight, Distance, B, Fallback));
		TestEqual(FString::Printf(TEXT("gap kept at %.0f cm"), Distance), Gap, FVector2D::Distance(A, B), 0.01);
	}
	// Offsets back and forth between the block and the world.
	const FVector2D Forward = FVector2D(3., 4.).GetSafeNormal();
	TestTrue(TEXT("offset to the world and back"),
		HOLFormation::WorldToBlockOffset(HOLFormation::BlockOffsetToWorld(B, Forward), Forward).Equals(B, 0.001));
	TestTrue(TEXT("a single point: no direction, the fallback"),
		HOLFormation::GetDirectionOnLine(TArray<FVector2D>{ FVector2D(5., 5.) }, 0., 200., Fallback).Equals(Fallback, 0.001));

	// The arrival (the user's 3rd trial): each unit to a place of its kind, from where it stands. A block of 3 soldiers
	// (kind 0, front row at X = 0) and 3 Peasants (kind 1, at X = -180) marching along +X.
	const TArray<FVector2D> Standing = { FVector2D(0., -150.), FVector2D(0., 0.), FVector2D(0., 150.),
		FVector2D(-180., -150.), FVector2D(-180., 0.), FVector2D(-180., 150.) };
	const TArray<int32> Kinds = { 0, 0, 0, 1, 1, 1 };
	const auto Run = [&](const TArray<FVector2D>& Places, const FVector2D& Facing, TArray<int32>& PlaceOfUnit)
	{
		HOLFormation::AssignPlacesByKind(Standing, Kinds, Places, Facing, PlaceOfUnit);
		int32 Crossings = 0;
		for (int32 First = 0; First < Standing.Num(); ++First)
		{
			TestEqual(FString::Printf(TEXT("unit %d keeps a place of its kind"), First), Kinds[PlaceOfUnit[First]], Kinds[First]);
			for (int32 Second = First + 1; Second < Standing.Num(); ++Second)
			{
				Crossings += Kinds[First] == Kinds[Second] && UE::HeroesOfLiberty::Tests::Formation::Cross(Standing[First], Places[PlaceOfUnit[First]], Standing[Second], Places[PlaceOfUnit[Second]]) ? 1 : 0;
			}
		}
		return Crossings;
	};
	// Same facing, 100 cm on: everyone to the place of its slot.
	TArray<FVector2D> Ahead;
	for (const FVector2D& At : Standing)
	{
		Ahead.Add(At + FVector2D(100., 0.));
	}
	TArray<int32> PlaceOfUnit;
	TestEqual(TEXT("same facing: no crossing"), Run(Ahead, FVector2D(1., 0.), PlaceOfUnit), 0);
	TestEqual(TEXT("same facing: each to its own slot's place"), PlaceOfUnit, TArray<int32>{ 0, 1, 2, 3, 4, 5 });
	// A half turn in place (facing -X): the soldiers' row now at X = -180, the Peasants' behind it at X = 0; places held
	// by kind (the units' places came from their slots of the new rows, so a soldier holds one at X = -180).
	const TArray<FVector2D> Turned = { FVector2D(-180., 150.), FVector2D(-180., 0.), FVector2D(-180., -150.),
		FVector2D(0., 150.), FVector2D(0., 0.), FVector2D(0., -150.) };
	TestEqual(TEXT("half turn: no crossing within a kind"), Run(Turned, FVector2D(-1., 0.), PlaceOfUnit), 0);
	double Longest = 0.;
	for (int32 Unit = 0; Unit < Standing.Num(); ++Unit)
	{
		Longest = FMath::Max(Longest, FVector2D::Distance(Standing[Unit], Turned[PlaceOfUnit[Unit]]));
	}
	TestTrue(FString::Printf(TEXT("half turn: straight across the block, no wheel round (%.0f cm at most)"), Longest), Longest <= 181.);

	// Forming up on the move and the blended arrival (the user's 4th trial): the block slows down with the worst gap
	// instead of stopping; the targets slide from the slots to the final places over the path's last part.
	TestEqual(TEXT("soft pace: full on the slots"), HOLFormation::GetBlockSoftPace(HOLFormation::BlockLagDistance), 1., 1e-9);
	TestEqual(TEXT("soft pace: half way"), HOLFormation::GetBlockSoftPace(0.5 * (HOLFormation::BlockLagDistance + HOLFormation::BlockSoftHoldDistance)), 0.5, 1e-9);
	TestEqual(TEXT("soft pace: stopped far behind"), HOLFormation::GetBlockSoftPace(HOLFormation::BlockSoftHoldDistance + 50.), 0., 1e-9);
	TestEqual(TEXT("blend over the block's size + margin"), HOLFormation::GetBlockBlendDistance(400., 3500.), 400. + HOLFormation::BlockBlendMargin, 1e-9);
	TestEqual(TEXT("blend: half the path at most"), HOLFormation::GetBlockBlendDistance(400., 1000.), 500., 1e-9);
	TestEqual(TEXT("blend: none before"), HOLFormation::GetBlockBlend(800., 650.), 0., 1e-9);
	TestEqual(TEXT("blend: half way"), HOLFormation::GetBlockBlend(325., 650.), 0.5, 1e-9);
	TestEqual(TEXT("blend: all at the end"), HOLFormation::GetBlockBlend(0., 650.), 1., 1e-9);
	TestTrue(TEXT("join distance: a half turn of two rows is formed on the move"), 2. * 180. < HOLFormation::BlockJoinDistance);
	return true;
}

#endif
