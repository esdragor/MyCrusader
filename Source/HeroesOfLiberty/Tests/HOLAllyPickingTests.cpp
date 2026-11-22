// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Player/HOLUnitPicking.h"

#if WITH_DEV_AUTOMATION_TESTS

// GLORFINDEL-1122e: the unit a right click takes to heal (Q-chapelle-moine = A) or follow (D818): an ally's body drawn
// under the cursor, never an enemy, a neutral unit, nor (to follow) a unit of the selection.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAllyPickingTest, "HeroesOfLiberty.Movement.AllyPicking", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAllyPickingTest::RunTest(const FString& Parameters)
{
	// 40 px tall bodies, 10 px wide on each side; Depth: the smaller is in front.
	auto Unit = [](const uint32 Id, const double X, const double FootY, const double Depth, const bool bHostile, const bool bNeutral)
	{
		FHOLAllyCandidate Candidate;
		Candidate.Pick.Foot = FVector2D(X, FootY);
		Candidate.Pick.Head = FVector2D(X, FootY - 40.);
		Candidate.Pick.HalfWidth = 10.;
		Candidate.Pick.Depth = Depth;
		Candidate.UnitId = Id;
		Candidate.bHostile = bHostile;
		Candidate.bNeutral = bNeutral;
		return Candidate;
	};
	// 0: an ally at (100, 200); 1: an enemy drawn in front of it, overlapping; 2: a neutral unit alone at (300, 200);
	// 3: an ally at (500, 200); 4: another ally in front of it, overlapping.
	const TArray<FHOLAllyCandidate> Units = {
		Unit(10, 100., 200., 3000., false, false), Unit(11, 104., 204., 2900., true, false), Unit(12, 300., 200., 2500., false, true),
		Unit(13, 500., 200., 3000., false, false), Unit(14, 504., 204., 2900., false, false) };
	const TArray<uint32> NoneExcluded;
	using HOLUnitPicking::PickAllyAt;

	TestEqual(TEXT("an ally's body under the cursor is taken"), PickAllyAt(Units, NoneExcluded, FVector2D(95., 170.)), 0);
	TestEqual(TEXT("an enemy in front of the ally is skipped, the ally behind it is taken"), PickAllyAt(Units, NoneExcluded, FVector2D(102., 195.)), 0);
	TestEqual(TEXT("an enemy alone is not taken"), PickAllyAt(Units, NoneExcluded, FVector2D(112., 200.)), INDEX_NONE);
	TestEqual(TEXT("a neutral unit is not taken"), PickAllyAt(Units, NoneExcluded, FVector2D(300., 180.)), INDEX_NONE);
	TestEqual(TEXT("two allies overlapping: the one in front"), PickAllyAt(Units, NoneExcluded, FVector2D(504., 195.)), 4);
	TestEqual(TEXT("the ground, 11 px beside a body: nothing (no margin, the click stays a move)"), PickAllyAt(Units, NoneExcluded, FVector2D(515., 190.)), INDEX_NONE);

	// A follow excludes the selection.
	const TArray<uint32> Selected = { 14 };
	TestEqual(TEXT("the selected unit in front is skipped, the ally behind it is taken"), PickAllyAt(Units, Selected, FVector2D(504., 195.)), 3);
	const TArray<uint32> BothSelected = { 13, 14 };
	TestEqual(TEXT("only selected units under the cursor: nothing"), PickAllyAt(Units, BothSelected, FVector2D(504., 195.)), INDEX_NONE);
	TestEqual(TEXT("no candidate: nothing"), PickAllyAt(TArray<FHOLAllyCandidate>(), NoneExcluded, FVector2D(100., 100.)), INDEX_NONE);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS