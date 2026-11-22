// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Player/HOLUnitPicking.h"

#if WITH_DEV_AUTOMATION_TESTS

// GLORFINDEL-1122g: hol.Follow.OwnUnits. 0 (default): a unit of the local player is never a FOLLOW target (the right click on it stays a
// move) while a teammate's unit is, and a heal (PickAllyAt, Q-chapelle-moine = A) still takes an own unit outside the selection.
// 1: the coded behaviour of 6494b40, an own unit outside the selection is followed.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFollowOwnUnitsTest, "HeroesOfLiberty.Movement.FollowOwnUnits", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLFollowOwnUnitsTest::RunTest(const FString& Parameters)
{
	auto Unit = [](const uint32 Id, const double X, const double FootY, const double Depth, const bool bOwn, const bool bHostile)
	{
		FHOLAllyCandidate Candidate;
		Candidate.Pick.Foot = FVector2D(X, FootY);
		Candidate.Pick.Head = FVector2D(X, FootY - 40.);
		Candidate.Pick.HalfWidth = 10.;
		Candidate.Pick.Depth = Depth;
		Candidate.UnitId = Id;
		Candidate.bOwn = bOwn;
		Candidate.bHostile = bHostile;
		return Candidate;
	};
	// 0: an own unit alone at (100, 200); 1: a teammate's unit alone at (300, 200); 2: a teammate's unit at (500, 200) with 3: an own unit
	// drawn in front of it (overlapping); 4: an enemy alone at (700, 200).
	const TArray<FHOLAllyCandidate> Units = {
		Unit(10, 100., 200., 3000., true, false), Unit(11, 300., 200., 3000., false, false), Unit(12, 500., 200., 3000., false, false),
		Unit(13, 504., 204., 2900., true, false), Unit(14, 700., 200., 3000., false, true) };
	const TArray<uint32> NoneExcluded;
	using HOLUnitPicking::PickAllyAt;
	using HOLUnitPicking::PickFollowAt;
	const FVector2D OnOwn(95., 170.), OnTeammate(295., 170.), OnOwnInFront(504., 195.), OnEnemy(700., 180.);

	// hol.Follow.OwnUnits 0.
	TestEqual(TEXT("0: an own unit outside the selection is not a follow target"), PickFollowAt(Units, NoneExcluded, OnOwn, false), INDEX_NONE);
	TestEqual(TEXT("0: a heal still takes that own unit"), PickAllyAt(Units, NoneExcluded, OnOwn), 0);
	TestEqual(TEXT("0: a teammate's unit is a follow target"), PickFollowAt(Units, NoneExcluded, OnTeammate, false), 1);
	TestEqual(TEXT("0: a teammate's unit is a heal target"), PickAllyAt(Units, NoneExcluded, OnTeammate), 1);
	TestEqual(TEXT("0: an own unit drawn in front of a teammate is skipped, the teammate behind it is followed"), PickFollowAt(Units, NoneExcluded, OnOwnInFront, false), 2);
	TestEqual(TEXT("0: a heal takes the own unit in front"), PickAllyAt(Units, NoneExcluded, OnOwnInFront), 3);
	TestEqual(TEXT("0: an enemy is never followed"), PickFollowAt(Units, NoneExcluded, OnEnemy, false), INDEX_NONE);

	// hol.Follow.OwnUnits 1.
	TestEqual(TEXT("1: an own unit outside the selection is a follow target"), PickFollowAt(Units, NoneExcluded, OnOwn, true), 0);
	TestEqual(TEXT("1: an own unit in front of a teammate is followed"), PickFollowAt(Units, NoneExcluded, OnOwnInFront, true), 3);
	TestEqual(TEXT("1: a teammate's unit is a follow target"), PickFollowAt(Units, NoneExcluded, OnTeammate, true), 1);
	const TArray<uint32> Selected = { 13 };
	TestEqual(TEXT("1: the selection is still excluded, the teammate behind is followed"), PickFollowAt(Units, Selected, OnOwnInFront, true), 2);
	TestEqual(TEXT("1: an enemy is never followed"), PickFollowAt(Units, NoneExcluded, OnEnemy, true), INDEX_NONE);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS