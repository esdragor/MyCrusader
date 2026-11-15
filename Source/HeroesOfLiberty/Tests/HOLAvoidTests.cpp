// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLAvoidRules.h"
#include "Dev/HOLSanityRules.h"
#include "HAL/IConsoleManager.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/** A console variable of the game (its shipped default in a test run), Fallback when it does not exist. */
	double GetGameCVar(const TCHAR* Name, const double Fallback)
	{
		const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name);
		return Variable ? Variable->GetFloat() : Fallback;
	}

	/** A walker of a block's march: its velocity of the last step and its remembered detour (FHOLDetourFragment's fields). */
	struct FMarchWalker
	{
		FVector2D Velocity = FVector2D::ZeroVector;
		int8 Side = 0;
		uint8 Turn = 0;
		uint8 ClearSteps = 0;
		uint8 MemberSteps = 0;
	};

	/**
	 * One step of UHOLFullSpeedProcessor::AvoidBodies for a walker going straight to its slot (ToSlot) with one body near (ToOther), a member of
	 * its own order going at Other's last velocity: the straight course when clear, the remembered detour (HOLAvoid::NextDetourTurn), else the
	 * smallest clear turn away from it, remembered only by HOLAvoid::RemembersMemberDetour. Returns the way (unit).
	 */
	FVector2D StepMarchMember(FMarchWalker& Me, const FMarchWalker& Other, const FVector2D& ToSlot, const FVector2D& ToOther, const double Speed, const double Contact,
		const double AvoidTime, const int32 Turns, const int32 MemberMaxSteps)
	{
		constexpr double AngleStep = 10.; // AvoidAngleStep of HOLSimulationProcessors.cpp
		const FVector2D Ahead = ToSlot.GetSafeNormal();
		const double Horizon = FMath::Min(AvoidTime, ToSlot.Size() / Speed);
		const bool bNear = FVector2D::DotProduct(ToOther, Ahead) > -Contact && ToOther.SizeSquared() < FMath::Square((Speed + Other.Velocity.Size()) * Horizon + Contact);
		const auto IsClear = [&](const FVector2D& Way) { return !bNear || HOLAvoid::TimeToContact(ToOther, Way * Speed - Other.Velocity, Contact, false) >= Horizon; };
		const auto Forget = [&Me] { Me.Side = 0; Me.Turn = 0; Me.ClearSteps = 0; Me.MemberSteps = 0; };
		if (Me.Turn > 0 && !bNear)
		{
			Me.ClearSteps = static_cast<uint8>(FMath::Max<int32>(Me.ClearSteps, HOLAvoid::EaseBackSteps - 1));
		}
		if (Me.Turn == 0 && IsClear(Ahead))
		{
			return Ahead;
		}
		if (Me.Turn > 0)
		{
			const double Hand = Me.Side;
			const int32 Turn = HOLAvoid::NextDetourTurn(Me.Turn, Turns, Me.ClearSteps, [&](const int32 Each) { return IsClear(Ahead.GetRotated(Hand * Each * AngleStep)); });
			Me.Turn = static_cast<uint8>(Turn != INDEX_NONE ? Turn : HOLAvoid::TurnWithoutClearWay(Me.Turn, Turns, false));
			if (Me.Turn == 0)
			{
				Forget();
				return Ahead;
			}
			const FVector2D Way = Ahead.GetRotated(Hand * Me.Turn * AngleStep);
			if (Me.MemberSteps > 0 && HOLAvoid::IsMemberDetourOver(Me.MemberSteps, MemberMaxSteps))
			{
				Forget();
			}
			return Way;
		}
		const double Side = HOLAvoid::ChooseDetourSide(Ahead, ToOther, Contact * 1.1);
		for (int32 Index = 0; Index < 2 * Turns; ++Index)
		{
			const int32 Turn = Index / 2 + 1;
			const double Hand = Index % 2 == 0 ? Side : -Side;
			const FVector2D Way = Ahead.GetRotated(Hand * Turn * AngleStep);
			if (IsClear(Way))
			{
				if (HOLAvoid::RemembersMemberDetour(MemberMaxSteps))
				{
					Me.Side = static_cast<int8>(Hand);
					Me.Turn = static_cast<uint8>(Turn);
					Me.MemberSteps = 1;
				}
				return Way;
			}
		}
		return Ahead;
	}

	/**
	 * neutrals_trace run3 (CELEBORN-1115a, Saved\c1115a\nt_run3.log, steps 306-329): #9 and #10, two Men-at-arms of one block marching north at 697 cm/s,
	 * #10 229 cm to the right of #9, #9's slot 650 cm ahead and 130 cm to its right, #10's 166 cm to its left: they converge. Their places across held
	 * by the block (the separation), both step together, each predicting the other from its last velocity. The audit's whole-window rules
	 * (HOLSanity::Analyse, 30 Hz, 8 s) on the places the steps make.
	 */
	HOLSanity::FResult RunMarchPair(const int32 MemberMaxSteps, const bool bRight)
	{
		constexpr double Speed = 697.;
		constexpr double Contact = 120.; // two bodies of 60 cm, members of one block bare (hol.Units.AvoidMarchMembersBare)
		constexpr float Step = 1.f / 30.f;
		const double AvoidTime = GetGameCVar(TEXT("hol.Units.AvoidTime"), 0.5);
		const int32 Turns = FMath::FloorToInt32(GetGameCVar(TEXT("hol.Units.AvoidMaxAngle"), 60.) / 10.);
		const FVector2D LeftToSlot(130., 650.);
		const FVector2D RightToSlot(-166., 650.);
		const FVector2D LeftToRight(229., 0.);
		FMarchWalker Left;
		FMarchWalker Right;
		HOLSanity::FTrace Trace;
		FVector2D Place = FVector2D::ZeroVector;
		for (int32 Index = 0; Index < 240; ++Index)
		{
			const FMarchWalker LastLeft = Left;
			const FMarchWalker LastRight = Right;
			Left.Velocity = StepMarchMember(Left, LastRight, LeftToSlot, LeftToRight, Speed, Contact, AvoidTime, Turns, MemberMaxSteps) * Speed;
			Right.Velocity = StepMarchMember(Right, LastLeft, RightToSlot, -LeftToRight, Speed, Contact, AvoidTime, Turns, MemberMaxSteps) * Speed;
			Place += (bRight ? Right.Velocity : Left.Velocity) * Step;
			Trace.Places.Add(Place);
			Trace.Ordered.Add(1);
		}
		return HOLSanity::Analyse(Trace, Step);
	}
}

// CELEBORN-1115b: the trembling walk of a block's march (neutrals_trace run3, #9/#10 'shakes over 8 s ... to and fro 26.6 rad'). Two members converging
// on their slots, a detour round one of its own order never remembered: each turned one step (its straight course met the other's), went straight the
// next (the other, turned, looked clear), in phase. With the game's hol.Units.MarchMemberDetourSteps neither shakes, and both still march.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidMarchPairTest, "HeroesOfLiberty.Movement.MarchPairNoTremble",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidMarchPairTest::RunTest(const FString& Parameters)
{
	const HOLSanity::FResult Before = RunMarchPair(0, false);
	TestTrue(*FString::Printf(TEXT("the old rule (never remembered) trembles, as on the real chain (to and fro %.1f rad in 8 s)"), Before.Zigzag), Before.bShake);
	const int32 MemberMaxSteps = static_cast<int32>(GetGameCVar(TEXT("hol.Units.MarchMemberDetourSteps"), 0.));
	for (const bool bRight : { false, true })
	{
		const HOLSanity::FResult Now = RunMarchPair(MemberMaxSteps, bRight);
		const TCHAR* Who = bRight ? TEXT("#10") : TEXT("#9");
		TestFalse(*FString::Printf(TEXT("%s does not tremble with hol.Units.MarchMemberDetourSteps %d (to and fro %.1f rad in 8 s, turn %.1f)"), Who, MemberMaxSteps, Now.Zigzag, Now.Turn), Now.bShake);
		TestTrue(*FString::Printf(TEXT("%s still marches (net %.0f cm in 8 s)"), Who, Now.Net), Now.Net > 0.9f * 697.f * 8.f);
	}
	return true;
}

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

// Celeborn-12: the commit straight to the place is not taken against a walker coming head on; a place inside a body is blocked.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidHeadOnTest, "HeroesOfLiberty.Movement.HeadOnAndBlockedPlace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidHeadOnTest::RunTest(const FString& Parameters)
{
	const FVector2D East(1., 0.);
	TestTrue(TEXT("a friend ahead walking back at 650 cm/s is head on"), HOLAvoid::IsHeadOn(East, { 200., 20. }, { -650., 0. }, 360., 100.));
	TestFalse(TEXT("a friend ahead walking the same way is not"), HOLAvoid::IsHeadOn(East, { 200., 20. }, { 650., 0. }, 360., 100.));
	TestFalse(TEXT("a friend ahead standing is not"), HOLAvoid::IsHeadOn(East, { 200., 20. }, { 0., 0. }, 360., 100.));
	TestFalse(TEXT("a friend behind is not"), HOLAvoid::IsHeadOn(East, { -200., 20. }, { 650., 0. }, 360., 100.));
	TestFalse(TEXT("a friend too far is not"), HOLAvoid::IsHeadOn(East, { 500., 0. }, { -650., 0. }, 360., 100.));
	TestFalse(TEXT("a friend crossing sideways is not"), HOLAvoid::IsHeadOn(East, { 200., 20. }, { 0., -650. }, 360., 100.));
	TestTrue(TEXT("a place on a body's centre is in it"), HOLAvoid::IsPlaceInBody({ -10312., -55. }, { -10312., -55. }, 60.));
	TestTrue(TEXT("a place 40 cm from a body is in it"), HOLAvoid::IsPlaceInBody({ -10066., -216. }, { -10066., -176. }, 60.));
	TestFalse(TEXT("a place 80 cm from a body is not"), HOLAvoid::IsPlaceInBody({ 0., 0. }, { 80., 0. }, 60.));
	// Celeborn-31: the blocked-slot flag counts only when written on this order and this leg (a flag of the previous walk ended every new walk at once).
	TestTrue(TEXT("a flag written on this order and leg counts"), HOLAvoid::IsSlotBlockedSeen(1, 7u, 3, 7u, 3));
	TestFalse(TEXT("a flag of a previous order does not"), HOLAvoid::IsSlotBlockedSeen(1, 6u, 3, 7u, 3));
	TestFalse(TEXT("a flag of a previous leg does not"), HOLAvoid::IsSlotBlockedSeen(1, 7u, 2, 7u, 3));
	TestFalse(TEXT("no flag, no block"), HOLAvoid::IsSlotBlockedSeen(0, 7u, 3, 7u, 3));
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

// EtapeDefini E (build gap, build cut): a detour into a building's body is no clear way.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidFootprintTest, "HeroesOfLiberty.Movement.DetourFootprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidFootprintTest::RunTest(const FString& Parameters)
{
	// A house 400 x 400 around (0, 0); a body of 40 walking along its south wall, its centre 40 off it (y = -240).
	HOLAvoid::FFootprintBox House;
	House.HalfExtent = { 200., 200. };
	const double Keep = 40.;
	TestEqual(TEXT("distance out of the wall, beyond the keep"), HOLAvoid::GetFootprintDistance(House, Keep, { 0., -300. }), 60., 1.e-6);
	TestTrue(TEXT("inside: negative"), HOLAvoid::GetFootprintDistance(House, Keep, { 0., 0. }) < 0.);
	const FVector2D Along(-1000., -240.);
	TestFalse(TEXT("along the wall: not led in"), HOLAvoid::LeadsIntoFootprint(House, Keep, Along, { 1., 0. }, 450.));
	TestTrue(TEXT("turned towards the wall: led in"), HOLAvoid::LeadsIntoFootprint(House, Keep, { -100., -240. }, FVector2D(1., 0.).GetRotated(30.), 450.));
	TestFalse(TEXT("turned away from the wall: not led in"), HOLAvoid::LeadsIntoFootprint(House, Keep, { -100., -240. }, FVector2D(1., 0.).GetRotated(-30.), 450.));
	TestFalse(TEXT("far from it: not led in"), HOLAvoid::LeadsIntoFootprint(House, Keep, { -3000., 0. }, { 1., 0. }, 450.));
	// Turned by 90 degrees: the same body, the frame turned.
	HOLAvoid::FFootprintBox Turned = House;
	Turned.HalfExtent = { 100., 300. };
	Turned.Cos = 0.;
	Turned.Sin = 1.;
	TestEqual(TEXT("turned: its long side along x"), HOLAvoid::GetFootprintDistance(Turned, 0., { 0., -150. }), 50., 1.e-6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAvoidLoopPatienceTest, "HeroesOfLiberty.Movement.LoopPatience",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLAvoidLoopPatienceTest::RunTest(const FString& Parameters)
{
	// D641: a melee barred by the line of its friends waits instead of going round it in loops (20 Hz steps, 270 degrees, 2.5 s, 4 s leak).
	const double Dt = 0.05;
	const auto Step = [Dt](HOLAvoid::FLoopPatience& State, const double Degrees, const double Speed = 697.)
	{
		return HOLAvoid::StepLoopPatience(State, FVector2D(Speed, 0.).GetRotated(Degrees), Dt, 270., 2.5, 4.);
	};
	{
		HOLAvoid::FLoopPatience State;
		bool bHeld = false;
		for (int32 Index = 0; Index < 200; ++Index)
		{
			bHeld |= Step(State, 0.);
		}
		TestFalse(TEXT("a straight walk never waits"), bHeld);
	}
	{
		// A detour round an obstacle and back: 60 degrees out, then back, over a second: no wait.
		HOLAvoid::FLoopPatience State;
		bool bHeld = false;
		for (int32 Index = 0; Index < 80; ++Index)
		{
			bHeld |= Step(State, Index < 10 ? Index * 6. : Index < 40 ? 60. : Index < 50 ? 60. - (Index - 40) * 6. : 0.);
		}
		TestFalse(TEXT("a detour out and back does not wait"), bHeld);
	}
	{
		// A loop: the heading turns 12 degrees a step (a circle in 1.5 s): it waits before the circle is closed.
		HOLAvoid::FLoopPatience State;
		int32 HeldAt = INDEX_NONE;
		for (int32 Index = 0; Index < 40 && HeldAt == INDEX_NONE; ++Index)
		{
			if (Step(State, Index * 12.))
			{
				HeldAt = Index;
			}
		}
		TestTrue(TEXT("a loop makes it wait"), HeldAt != INDEX_NONE);
		TestTrue(TEXT("before it closes the circle"), HeldAt != INDEX_NONE && HeldAt * 12 < 330);
		// It waits 2.5 s (50 steps), whatever it is asked, then walks again with an empty pile.
		int32 Steps = 0;
		while (Step(State, 0.) && Steps < 200)
		{
			++Steps;
		}
		TestTrue(TEXT("the wait lasts 2.5 s"), Steps >= 49 && Steps <= 51);
		TestFalse(TEXT("then it walks"), Step(State, 0.));
	}
	{
		// A sway to and fro (a U-turn every 4 steps) piles up through its reversals.
		HOLAvoid::FLoopPatience State;
		bool bHeld = false;
		for (int32 Index = 0; Index < 30; ++Index)
		{
			bHeld |= Step(State, (Index / 4) % 2 == 0 ? 0. : 180.);
		}
		TestTrue(TEXT("a sway to and fro makes it wait"), bHeld);
	}
	{
		// Standing (asked less than 50 cm/s) keeps its heading and piles nothing; off at 0 degrees.
		HOLAvoid::FLoopPatience State;
		bool bHeld = false;
		for (int32 Index = 0; Index < 100; ++Index)
		{
			bHeld |= Step(State, Index * 30., 10.);
		}
		TestFalse(TEXT("standing piles no turn"), bHeld);
		HOLAvoid::FLoopPatience Off;
		bool bOffHeld = false;
		for (int32 Index = 0; Index < 100; ++Index)
		{
			bOffHeld |= HOLAvoid::StepLoopPatience(Off, FVector2D(697., 0.).GetRotated(Index * 40.), Dt, 0., 2.5, 4.);
		}
		TestFalse(TEXT("0 degrees: off"), bOffHeld);
	}
	return true;
}

#endif
