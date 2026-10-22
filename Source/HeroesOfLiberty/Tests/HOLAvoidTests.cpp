// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLAvoidRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// MV.2 checks (D293): the remembered detour's rules.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidDetourSideTest, "HeroesOfLiberty.Movement.DetourSide",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidDetourSideTest::RunTest(const FString& Parameters)
{
	const double Contact = 130.;
	TestEqual(TEXT("a body on the right: turn left"), HOLAvoid::ChooseDetourSide({ 1., 0. }, { 300., 60. }, Contact), -1.);
	TestEqual(TEXT("a body on the left: turn right"), HOLAvoid::ChooseDetourSide({ 1., 0. }, { 300., -60. }, Contact), 1.);
	// Face to face, a hair off the line on either side: each on its own right, opposite sides of the world.
	const double First = HOLAvoid::ChooseDetourSide({ 1., 0. }, { 300., 5. }, Contact);
	const double Second = HOLAvoid::ChooseDetourSide({ -1., 0. }, { -300., 5. }, Contact);
	TestTrue(TEXT("face to face: both on their own right"), First == 1. && Second == 1.);
	TestEqual(TEXT("face to face the other way off the line: on its own right"), HOLAvoid::ChooseDetourSide({ 1., 0. }, { 300., -5. }, Contact), 1.);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidDetourTurnTest, "HeroesOfLiberty.Movement.DetourTurn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidDetourTurnTest::RunTest(const FString& Parameters)
{
	const auto Next = [](const int32 Remembered, const int32 Smallest, uint8& ClearSteps)
	{
		return HOLAvoid::NextDetourTurn(Remembered, 6, ClearSteps, [Smallest](const int32 Turn) { return Turn >= Smallest; });
	};
	uint8 Clear = 0;
	bool bHeld = true;
	for (int32 Step = 1; Step < HOLAvoid::EaseBackSteps; ++Step)
	{
		bHeld &= Next(4, 1, Clear) == 4;
	}
	TestTrue(TEXT("held while the eased course has not been clear long enough"), bHeld);
	TestEqual(TEXT("eases back one turn after a run of clear steps"), Next(4, 1, Clear), 3);
	TestEqual(TEXT("the run starts again after easing back"), Clear, static_cast<uint8>(0));
	Clear = HOLAvoid::EaseBackSteps - 1;
	TestEqual(TEXT("a step not clear holds the turn"), Next(3, 3, Clear), 3);
	TestEqual(TEXT("and starts the run again"), Clear, static_cast<uint8>(0));
	Clear = HOLAvoid::EaseBackSteps - 1;
	TestEqual(TEXT("never more than one turn back a step"), Next(5, 2, Clear), 4);
	Clear = HOLAvoid::EaseBackSteps - 1;
	TestEqual(TEXT("the last turn back is the straight course"), Next(1, 0, Clear), 0);
	Clear = 2;
	TestEqual(TEXT("wider at once when the remembered course is not clear"), Next(3, 5, Clear), 5);
	TestEqual(TEXT("widening starts the run again"), Clear, static_cast<uint8>(0));
	Clear = 0;
	TestEqual(TEXT("narrower at once when only the eased course is clear"), HOLAvoid::NextDetourTurn(3, 6, Clear, [](const int32 Turn) { return Turn == 2; }), 2);
	Clear = 0;
	TestEqual(TEXT("nothing clear up to the largest turn"), Next(2, 7, Clear), INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidCrossingRoleTest, "HeroesOfLiberty.Movement.CrossingRole",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidCrossingRoleTest::RunTest(const FString& Parameters)
{
	using HOLAvoid::ECrossingRole;
	const auto Role = [](const FVector2D& MyPosition, const FVector2D& MyVelocity, const FVector2D& OtherPosition, const FVector2D& OtherVelocity)
	{
		return HOLAvoid::ChooseCrossingRole(MyPosition, MyVelocity, OtherPosition, OtherVelocity);
	};
	// A goes along X at 1 m/s from the origin; B along +Y (A's right) at a right angle, crossing A's way at (500, 0).
	const FVector2D APosition(0., 0.), AVelocity(100., 0.), BVelocity(0., 100.);
	// A clearly first (5 s against 8 s): A on, B behind it, turning to its right (-X, where A comes from).
	const FVector2D BLate(500., -800.);
	TestTrue(TEXT("first: keeps its course"), Role(APosition, AVelocity, BLate, BVelocity).Role == ECrossingRole::KeepCourse);
	const HOLAvoid::FCrossingChoice Late = Role(BLate, BVelocity, APosition, AVelocity);
	TestTrue(TEXT("second: passes behind on the side the other comes from"), Late.Role == ECrossingRole::PassBehind && Late.Side == 1.);
	// B clearly first (3 s against 5 s): A behind it, turning to its left (-Y, where B comes from).
	const FVector2D BEarly(500., -300.);
	const HOLAvoid::FCrossingChoice Second = Role(APosition, AVelocity, BEarly, BVelocity);
	TestTrue(TEXT("second the other way: passes behind on its left"), Second.Role == ECrossingRole::PassBehind && Second.Side == -1.);
	TestTrue(TEXT("first the other way: keeps its course"), Role(BEarly, BVelocity, APosition, AVelocity).Role == ECrossingRole::KeepCourse);
	// Together (5 s each, and 0.1 s apart): priority to the right. B has A on its right: B gives way, A keeps its course.
	const FVector2D BTogether(500., -500.), BNearly(500., -510.);
	TestTrue(TEXT("together: the one with the other on its left keeps its course"), Role(APosition, AVelocity, BTogether, BVelocity).Role == ECrossingRole::KeepCourse);
	const HOLAvoid::FCrossingChoice Right = Role(BTogether, BVelocity, APosition, AVelocity);
	TestTrue(TEXT("together: the one with the other on its right passes behind"), Right.Role == ECrossingRole::PassBehind && Right.Side == 1.);
	TestTrue(TEXT("nearly together: priority to the right still"), Role(APosition, AVelocity, BNearly, BVelocity).Role == ECrossingRole::KeepCourse
		&& Role(BNearly, BVelocity, APosition, AVelocity).Role == ECrossingRole::PassBehind);
	// One already past the crossing point keeps its course, the other passes behind it.
	TestTrue(TEXT("past the point: keeps its course"), Role({ 600., 0. }, AVelocity, BEarly, BVelocity).Role == ECrossingRole::KeepCourse);
	TestTrue(TEXT("the other past the point: passes behind"), Role(BEarly, BVelocity, { 600., 0. }, AVelocity).Role == ECrossingRole::PassBehind);
	// Face to face, the same way, or standing: not a crossing, the detour side as before.
	TestTrue(TEXT("face to face: none"), Role(APosition, AVelocity, { 1000., 10. }, { -100., 0. }).Role == ECrossingRole::None);
	TestTrue(TEXT("nearly face to face (20 degrees): none"), Role(APosition, AVelocity, { 1000., 100. }, FVector2D(-100., 0.).GetRotated(20.)).Role == ECrossingRole::None);
	TestTrue(TEXT("overtaking: none"), Role(APosition, AVelocity, { 300., 10. }, { 50., 0. }).Role == ECrossingRole::None);
	TestTrue(TEXT("standing: none"), Role(APosition, AVelocity, { 300., 10. }, FVector2D::ZeroVector).Role == ECrossingRole::None);
	TestTrue(TEXT("both past the point: none"), Role({ 600., 0. }, AVelocity, { 500., 100. }, BVelocity).Role == ECrossingRole::None);
	return true;
}

// MV.4 checks (D293): a body already in contact and a wall of enemies.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidTimeToContactTest, "HeroesOfLiberty.Movement.TimeToContact",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidTimeToContactTest::RunTest(const FString& Parameters)
{
	const double Contact = 100.;
	const double Never = TNumericLimits<double>::Max();
	TestEqual(TEXT("ahead out of contact: met when the gap is closed"), HOLAvoid::TimeToContact({ 300., 0. }, { 100., 0. }, Contact, true), 2., 1.e-6);
	TestEqual(TEXT("in contact, closing in: met now"), HOLAvoid::TimeToContact({ 80., 0. }, { 100., 0. }, Contact, true), 0.);
	TestEqual(TEXT("in contact, sliding along it at a slant inwards: met now"), HOLAvoid::TimeToContact({ 80., 0. }, { 20., 100. }, Contact, true), 0.);
	TestEqual(TEXT("in contact, moving away: never"), HOLAvoid::TimeToContact({ 80., 0. }, { -100., 0. }, Contact, true), Never);
	TestEqual(TEXT("in contact, along it: never"), HOLAvoid::TimeToContact({ 80., 0. }, { 0., 100. }, Contact, true), Never);
	TestEqual(TEXT("passing wide: never"), HOLAvoid::TimeToContact({ 300., 200. }, { 100., 0. }, Contact, true), Never);
	TestEqual(TEXT("a friend in contact, closing in: never (it gives way)"), HOLAvoid::TimeToContact({ 80., 0. }, { 100., 0. }, Contact, false), Never);
	TestEqual(TEXT("a friend ahead out of contact: met as an enemy"), HOLAvoid::TimeToContact({ 300., 0. }, { 100., 0. }, Contact, false), 2., 1.e-6);
	TestEqual(TEXT("standing: never"), HOLAvoid::TimeToContact({ 80., 0. }, FVector2D::ZeroVector, Contact, true), Never);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidNoClearWayTest, "HeroesOfLiberty.Movement.NoClearWay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidNoClearWayTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("against an enemy: one turn wider"), HOLAvoid::TurnWithoutClearWay(3, 6, true), 4);
	TestEqual(TEXT("against an enemy: held at the largest turn"), HOLAvoid::TurnWithoutClearWay(6, 6, true), 6);
	TestEqual(TEXT("against a friend: one turn back, pushing"), HOLAvoid::TurnWithoutClearWay(3, 6, false), 2);
	return true;
}

#endif
