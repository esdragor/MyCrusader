// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLSanityRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr double SanityStepSeconds = 1. / 30.;

	/** Judges the track of a unit whose place at Time (s) is Place(Time), over Seconds, with the given motion. */
	FString JudgeTrack(const HOLSanity::EUnitMotion Motion, const double Seconds, const TFunctionRef<FVector2D(double)> Place)
	{
		const HOLSanity::FSanityRules Rules;
		HOLSanity::FUnitTrack Track;
		FString Last;
		for (double Time = 0.; Time <= Seconds; Time += SanityStepSeconds)
		{
			Track.Add(Time, Place(Time), Motion, Rules);
			const FString Fault = Track.Judge(Rules);
			if (!Fault.IsEmpty())
			{
				return Fault;
			}
		}
		return FString();
	}
}

// hol.Test.Sanity rules (checks 2 and 3): a fault is found, a sound unit is not. Each fault case is the mutation of the sound case
// next to it (same speed, same window): the rule fails for the right reason only.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSanitySpinTest, "HeroesOfLiberty.Sanity.Spin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLSanitySpinTest::RunTest(const FString& Parameters)
{
	using HOLSanity::EUnitMotion;
	const double Speed = 400.;
	// Sound: walking straight; walking a bend (a detour round a house); walking a long arc.
	TestTrue(TEXT("straight walk is sound"), JudgeTrack(EUnitMotion::Ordered, 10., [Speed](const double T) { return FVector2D(Speed * T, 0.); }).IsEmpty());
	TestTrue(TEXT("a detour is sound"), JudgeTrack(EUnitMotion::Ordered, 10., [Speed](const double T)
	{
		return T < 2. ? FVector2D(Speed * T, 0.) : T < 3. ? FVector2D(Speed * 2., Speed * (T - 2.)) : FVector2D(Speed * 2. + Speed * (T - 3.), Speed);
	}).IsEmpty());
	// Fault: the same speed round a circle of 1 m (a spear carrier turning round its goal), 10 s.
	const double Radius = 100.;
	const FString Spin = JudgeTrack(EUnitMotion::Ordered, 10., [Speed, Radius](const double T)
	{
		const double Angle = Speed * T / Radius;
		return FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle));
	});
	TestTrue(TEXT("a circle with an order is found: ") + Spin, Spin.StartsWith(TEXT("spins")));
	// Fault: back and forth along a line, 4 m each way, every 1/4 s turn (zigzag).
	const FString Zigzag = JudgeTrack(EUnitMotion::Ordered, 10., [](const double T)
	{
		const double Phase = FMath::Fmod(T, 0.8);
		return FVector2D(Phase < 0.4 ? 800. * Phase : 800. * (0.8 - Phase), 0.);
	});
	TestTrue(TEXT("back and forth with an order is found: ") + Zigzag, Zigzag.StartsWith(TEXT("spins")));
	// The same circle without an order is a unit being carried, not a spin (the other rule judges it, and finds no step small enough).
	TestTrue(TEXT("a circle on a unit busy with something else is not a spin"),
		JudgeTrack(EUnitMotion::Busy, 10., [Speed, Radius](const double T)
		{
			const double Angle = Speed * T / Radius;
			return FVector2D(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle));
		}).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSanityShakeTest, "HeroesOfLiberty.Sanity.Shake",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLSanityShakeTest::RunTest(const FString& Parameters)
{
	using HOLSanity::EUnitMotion;
	TestTrue(TEXT("standing still is sound"), JudgeTrack(EUnitMotion::Idle, 6., [](const double) { return FVector2D(100., 100.); }).IsEmpty());
	// Sound: pushed along by a passing unit, 30 cm in a second, straight.
	TestTrue(TEXT("being pushed along is sound"), JudgeTrack(EUnitMotion::Idle, 6., [](const double T) { return FVector2D(100. + 30. * T, 100.); }).IsEmpty());
	// Sound: a settling of half a centimetre a step.
	TestTrue(TEXT("a tiny settling is sound"), JudgeTrack(EUnitMotion::Idle, 6., [](const double T) { return FVector2D(100. + 0.5 * FMath::Sin(T * 60.), 100.); }).IsEmpty());
	// Fault: 3 cm each way every step (the trembling seen around the hero).
	int32 Step = 0;
	const FString Shake = JudgeTrack(EUnitMotion::Idle, 6., [&Step](const double) { return FVector2D(100. + ((Step++ & 1) ? 3. : -3.), 100.); });
	TestTrue(TEXT("a tremor of 3 cm a step is found: ") + Shake, Shake.StartsWith(TEXT("shakes")));
	// The same tremor on a unit that is walking is not "shakes" (an order is no immobile unit).
	Step = 0;
	TestTrue(TEXT("the same on a unit with an order is not a tremor"),
		!JudgeTrack(EUnitMotion::Ordered, 6., [&Step](const double) { return FVector2D(100. + ((Step++ & 1) ? 3. : -3.), 100.); }).StartsWith(TEXT("shakes")));
	return true;
}

// Merged from hero-j8-close: the whole-window rules (HOLSanity::Analyse) that hol.Test.Sanity also runs, on traces made by hand.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSanityMotionTest, "HeroesOfLiberty.Sanity.Motion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// The motion audit's rules on traces made by hand (20 Hz): a walk is fine, a circle on an order spins, a zigzag shakes.
bool FHOLSanityMotionTest::RunTest(const FString& Parameters)
{
	constexpr float Step = 0.05f;
	const auto Make = [](const TFunction<FVector2D(int32)>& Place, const bool bOrdered, const int32 Steps = 160)
	{
		HOLSanity::FTrace Trace;
		for (int32 Index = 0; Index < Steps; ++Index)
		{
			Trace.Places.Add(Place(Index));
			Trace.Ordered.Add(bOrdered ? 1 : 0);
		}
		return Trace;
	};
	// 8 s walk at 400 cm/s: fine, ordered or not.
	const HOLSanity::FResult Walk = HOLSanity::Analyse(Make([](int32 I) { return FVector2D(20. * I, 0.); }, true), Step);
	TestFalse(TEXT("a straight walk does not spin"), Walk.bSpin);
	TestFalse(TEXT("a straight walk does not shake"), Walk.bShake);
	// Round a circle of 100 cm radius at 500 cm/s (0.25 rad a step, the rate measured on the real chain), 8 s, ordered the whole time.
	const auto Circle = [](int32 I) { const double A = 0.25 * I; return FVector2D(100. * FMath::Cos(A), 100. * FMath::Sin(A)); };
	const HOLSanity::FResult Round = HOLSanity::Analyse(Make(Circle, true), Step);
	TestTrue(*FString::Printf(TEXT("an ordered unit going round in a circle spins (turn %.1f)"), Round.Turn), Round.bSpin);
	TestFalse(TEXT("the same circle with no order is not a spin"), HOLSanity::Analyse(Make(Circle, false), Step).bSpin);
	// A body that trembles: 2 cm to and fro at each step, no order.
	const HOLSanity::FResult Tremble = HOLSanity::Analyse(Make([](int32 I) { return FVector2D((I % 2) ? 2. : 0., 0.); }, false), Step);
	TestTrue(*FString::Printf(TEXT("a body going to and fro by small steps shakes (%d reversals)"), Tremble.Reversals), Tremble.bShake);
	// A walk that trembles: 35 cm ahead a step, sideways +14 then -11 (the units round the hero, measured on the real chain).
	const HOLSanity::FResult Wobble = HOLSanity::Analyse(Make([](int32 I) { return FVector2D(35. * I, (I % 2) ? 14. : 0.); }, true), Step);
	TestTrue(*FString::Printf(TEXT("a walk whose heading wobbles at each step shakes (turn %.1f rad)"), Wobble.Turn), Wobble.bShake);
	// A charge through a crowd (neutrals trial, measured on the real chain): 35 cm a step, the heading bends one way for a while, then the other
	// (an S of +-0.8 rad every 24 steps, 2.6 rad/s of turning in all): a weave, never a tremor, since no turn undoes the one just before it.
	const auto Weave = [](int32 I)
	{
		FVector2D Where = FVector2D::ZeroVector;
		for (int32 K = 0; K < I; ++K)
		{
			const double Heading = 0.8 * FMath::Sin(2. * UE_DOUBLE_PI * K / 24.);
			Where += 35. * FVector2D(FMath::Cos(Heading), FMath::Sin(Heading));
		}
		return Where;
	};
	const HOLSanity::FResult Slalom = HOLSanity::Analyse(Make(Weave, true), Step);
	TestTrue(*FString::Printf(TEXT("the weave turns a lot (%.1f rad), as the old rule judged it"), Slalom.Turn), Slalom.Turn / 8.f >= 2.f);
	TestFalse(*FString::Printf(TEXT("a weave through a crowd does not shake (turn %.1f, to and fro %.1f rad)"), Slalom.Turn, Slalom.Zigzag), Slalom.bShake);
	TestTrue(*FString::Printf(TEXT("a wobble is mostly to and fro (%.1f of %.1f rad)"), Wobble.Zigzag, Wobble.Turn), Wobble.Zigzag > 0.9f * Wobble.Turn);
	// A man at arms jostled in the neutrals trial's crowd (steps 180-219 of unit #11, measured on the real chain): it stands, takes 6 steps, stands
	// 19 steps, then is shoved sideways and back by its neighbours (turns of +60, -22, -12, +150, +26, -32, -18, +25, -26 ... degrees). Its turns
	// add up to 8 radians, but they cancel one another: it never goes round anything, so it is not a spin.
	{
		struct FStep { float Length; float Degrees; };
		TArray<FStep> Moves;
		Moves.Init({ 0.f, 0.f }, 4);
		Moves.Append({ { 10.f, -36.f }, { 21.f, -32.f }, { 24.f, -22.f }, { 31.f, 23.f }, { 35.f, 8.f }, { 35.f, -1.f } });
		Moves.AddZeroed(19);
		Moves.Append({ { 10.f, 58.f }, { 19.f, 36.f }, { 25.f, 24.f }, { 29.f, 174.f }, { 13.f, -160.f }, { 19.f, 167.f }, { 27.f, 150.f }, { 14.f, 175.f }, { 21.f, 149.f }, { 29.f, 135.f }, { 34.f, 118.f } });
		HOLSanity::FTrace Jostled;
		FVector2D Where = FVector2D::ZeroVector;
		Jostled.Places.Add(Where);
		Jostled.Ordered.Add(1);
		for (const FStep& Move : Moves)
		{
			Where += Move.Length * FVector2D(FMath::Cos(FMath::DegreesToRadians(Move.Degrees)), FMath::Sin(FMath::DegreesToRadians(Move.Degrees)));
			Jostled.Places.Add(Where);
			Jostled.Ordered.Add(1);
		}
		const HOLSanity::FResult Jostle = HOLSanity::Analyse(Jostled, Step);
		TestTrue(*FString::Printf(TEXT("the jostled unit does turn a lot (%.1f rad), as the old rule judged it"), Jostle.Turn), Jostle.Turn >= 2.f * UE_PI);
		TestFalse(TEXT("a unit jostled to and fro in a crowd does not spin"), Jostle.bSpin);
	}
	// A walker with one real turn at a corner: neither.
	const HOLSanity::FResult Corner = HOLSanity::Analyse(Make([](int32 I) { return I < 80 ? FVector2D(20. * I, 0.) : FVector2D(1580., 20. * (I - 80)); }, true), Step);
	TestFalse(TEXT("a walk round a corner does not spin"), Corner.bSpin);
	TestFalse(TEXT("a walk round a corner does not shake"), Corner.bShake);
	// Q-flank-demi-tour = A: a rear flank in progress (a hairpin round the end of the line) is a deliberate manoeuvre, neither a spin nor a tremor.
	const auto Flank = [&Make](const TFunction<FVector2D(int32)>& Place, const int32 FlankFrom, const int32 FlankTo)
	{
		HOLSanity::FTrace Trace = Make(Place, true);
		for (int32 Index = 0; Index < Trace.Places.Num(); ++Index)
		{
			Trace.Flanking.Add(Index >= FlankFrom && Index < FlankTo ? 1 : 0);
		}
		return Trace;
	};
	TestFalse(TEXT("a circle made all along on a flank is not a spin"), HOLSanity::Analyse(Flank(Circle, 0, 160), Step).bSpin);
	TestFalse(TEXT("a wobbling walk made all along on a flank is not a tremor"), HOLSanity::Analyse(Flank([](int32 I) { return FVector2D(35. * I, (I % 2) ? 14. : 0.); }, 0, 160), Step).bShake);
	TestTrue(TEXT("a circle with no flank flag is still a spin"), HOLSanity::Analyse(Flank(Circle, 0, 0), Step).bSpin);
	TestTrue(TEXT("a wobbling walk with no flank flag is still a tremor"), HOLSanity::Analyse(Flank([](int32 I) { return FVector2D(35. * I, (I % 2) ? 14. : 0.); }, 0, 0), Step).bShake);
	// Q-sanity-circling = C: the walk round ONE side to the claimed place is tolerated, a turn back on the soldier's steps is not.
		const auto PlaceWalking = [&Make](const TFunction<FVector2D(int32)>& Place)
		{
			HOLSanity::FTrace Trace = Make(Place, true);
			Trace.PlaceWalk.Init(1, Trace.Places.Num());
			return Trace;
		};
		TestFalse(TEXT("a circle made all along on a place walk (one side) is not a spin"), HOLSanity::Analyse(PlaceWalking(Circle), Step).bSpin);
		TestTrue(TEXT("the same circle without the place-walk flag is still a spin"), HOLSanity::Analyse(Make(Circle, true), Step).bSpin);
		TestTrue(TEXT("a wobbling (to and fro) walk on a place walk is still a tremor"), HOLSanity::Analyse(PlaceWalking([](int32 I) { return FVector2D(35. * I, (I % 2) ? 14. : 0.); }), Step).bShake);
		// Q-sanity-micro = A: the 8 s rule has the floor of the 2 s one (a path over 6 radii). A soldier that fidgets 0.5 cm a step round a 2 cm circle
	// (80 cm in 8 s, 40 radians of turning, ordered) was counted a spinner (the old rule: no floor); a real loop of 503 cm still is.
	HOLSanity::FSettings NoFloor;
	NoFloor.SpinMinPathRadii = 0.f;
	const auto Fidget = [](int32 I) { const double A = 0.25 * I; return FVector2D(2. * FMath::Cos(A), 2. * FMath::Sin(A)); };
	TestTrue(TEXT("without the floor (the rule as it was) a 2 cm fidget is a spin"), HOLSanity::Analyse(Make(Fidget, true), Step, NoFloor).bSpin);
	TestFalse(TEXT("with the floor a 2 cm fidget is not a spin"), HOLSanity::Analyse(Make(Fidget, true), Step).bSpin);
	const auto Loop = [](int32 I) { const double A = 0.25 * I; return FVector2D(80. * FMath::Cos(A), 80. * FMath::Sin(A)); };
	const HOLSanity::FResult Real = HOLSanity::Analyse(Make(Loop, true, 40), Step);
	TestTrue(*FString::Printf(TEXT("a real loop of 503 cm a turn is still a spin (path %.0f cm, net turn %.1f rad)"), Real.Path, Real.SignedTurn), Real.bSpin);
	return true;
}

// Check 4 of hol.Test.Sanity (the user's retour on the neutrals trial): a soldier standing still out of reach while a place round its target is free.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSanityStallTest, "HeroesOfLiberty.Sanity.Stall", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLSanityStallTest::RunTest(const FString& Parameters)
{
	// A target at the origin (radius 30) ringed by bodies of radius 30 touching it, every 60 degrees: six round it, no room for a seventh.
	TArray<HOLSanity::FBody> Ring;
	for (int32 Index = 0; Index < 6; ++Index)
	{
		const double Angle = UE_DOUBLE_PI / 3. * Index;
		Ring.Add({ FVector2D(62. * FMath::Cos(Angle), 62. * FMath::Sin(Angle)), 30.f });
	}
	TestFalse(TEXT("a target ringed by six bodies has no free place"), HOLSanity::HasFreePlaceAround(FVector2D::ZeroVector, 30.f, 30.f, Ring));
	Ring.RemoveAt(2);
	TestTrue(TEXT("with one body gone a place is free"), HOLSanity::HasFreePlaceAround(FVector2D::ZeroVector, 30.f, 30.f, Ring));
	TestTrue(TEXT("a lone target has a free place"), HOLSanity::HasFreePlaceAround(FVector2D(500., 0.), 30.f, 30.f, TConstArrayView<HOLSanity::FBody>()));
	// D641 (hol.Combat.ClaimPlace): the free spot nearest to the side asked for; none when the ring is full.
	{
		TArray<HOLCombat::FBody> Full;
		for (int32 Index = 0; Index < 6; ++Index)
		{
			const double Angle = UE_DOUBLE_PI / 3. * Index;
			Full.Add({ FVector2D(62. * FMath::Cos(Angle), 62. * FMath::Sin(Angle)), 30.f });
		}
		double Chosen = -1.;
		TestFalse(TEXT("no place is found round a full ring"), HOLCombat::FindFreePlaceAround(FVector2D::ZeroVector, 30.f, 30.f, Full, 0., Chosen));
		Full.RemoveAt(3);
		TestTrue(TEXT("the one place left is found"), HOLCombat::FindFreePlaceAround(FVector2D::ZeroVector, 30.f, 30.f, Full, 0., Chosen));
		TestTrue(TEXT("it is the one opposite the body that left (180 degrees)"), FMath::IsNearlyEqual(Chosen, UE_DOUBLE_PI, 1e-6));
		TArray<HOLCombat::FBody> None;
		TestTrue(TEXT("a lone target: the spot nearest to 1.0 rad"), HOLCombat::FindFreePlaceAround(FVector2D::ZeroVector, 30.f, 30.f, None, 1.0, Chosen));
		TestTrue(TEXT("is the third spot (67.5 degrees)"), FMath::IsNearlyEqual(Chosen, 2. * UE_DOUBLE_PI * 3. / 16., 1e-6));
	}
	// D641 (hol.Combat.PlaceBySide, the neutrals trace of 2026-11-05): a claimed place that gets taken is replaced by the free spot on the soldier's OWN
		// side, not the one nearest the old place (which could lie across the whole melee: the soldier walked round the crowd and back on its steps).
		// Sixteen spots round a target (radius 30, ring at 62 cm); bodies on spots 9, 10, 0 and 3 (each blocks its spot and the two either side) leave
		// spots 6 (135 degrees) and 13 (292.5 degrees) free. The soldier stands west of the target (180 degrees); its claimed place was spot 12 (270).
		{
			TArray<HOLCombat::FBody> Crowd;
			for (const int32 Spot : { 9, 10, 0, 3 })
			{
				const double Angle = 2. * UE_DOUBLE_PI * Spot / 16.;
				Crowd.Add({ FVector2D(62. * FMath::Cos(Angle), 62. * FMath::Sin(Angle)), 30.f });
			}
			const double Spot6 = 2. * UE_DOUBLE_PI * 6. / 16.;
			const double Spot13 = 2. * UE_DOUBLE_PI * 13. / 16.;
			double Old = -1.;
			TestTrue(TEXT("control: the rule as it was finds a spot"), HOLCombat::FindFreePlaceAround(FVector2D::ZeroVector, 30.f, 30.f, Crowd, 1.5 * UE_DOUBLE_PI, Old));
			TestTrue(TEXT("control: the rule as it was takes the spot nearest the old place (13, on the far side of the crowd)"), FMath::IsNearlyEqual(Old, Spot13, 1e-6));
			double Chosen = -1.;
			TestTrue(TEXT("a taken place is replaced"), HOLCombat::ChooseFreePlace(FVector2D::ZeroVector, 30.f, 30.f, Crowd, true, 1.5 * UE_DOUBLE_PI, UE_DOUBLE_PI, true, Chosen));
			TestTrue(*FString::Printf(TEXT("by the spot on the soldier's own side (spot 6, 135 degrees), not across the melee (got %.1f degrees)"), FMath::RadiansToDegrees(Chosen)),
				FMath::IsNearlyEqual(Chosen, Spot6, 1e-6));
			TestTrue(TEXT("with hol.Combat.PlaceBySide 0 the old choice stands"), HOLCombat::ChooseFreePlace(FVector2D::ZeroVector, 30.f, 30.f, Crowd, true, 1.5 * UE_DOUBLE_PI, UE_DOUBLE_PI, false, Chosen)
				&& FMath::IsNearlyEqual(Chosen, Spot13, 1e-6));
			TestTrue(TEXT("a claimed place still free is kept, whatever the side"), HOLCombat::ChooseFreePlace(FVector2D::ZeroVector, 30.f, 30.f, Crowd, true, static_cast<float>(Spot13), UE_DOUBLE_PI, true, Chosen)
				&& FMath::IsNearlyEqual(Chosen, Spot13, 1e-6));
			TestTrue(TEXT("no place yet: the spot nearest the soldier's side"), HOLCombat::ChooseFreePlace(FVector2D::ZeroVector, 30.f, 30.f, Crowd, false, 0., UE_DOUBLE_PI, true, Chosen)
				&& FMath::IsNearlyEqual(Chosen, Spot6, 1e-6));
			Crowd.Add({ FVector2D(62. * FMath::Cos(Spot6), 62. * FMath::Sin(Spot6)), 30.f });
			Crowd.Add({ FVector2D(62. * FMath::Cos(Spot13), 62. * FMath::Sin(Spot13)), 30.f });
			TestFalse(TEXT("every spot taken: no place"), HOLCombat::ChooseFreePlace(FVector2D::ZeroVector, 30.f, 30.f, Crowd, true, 1.5 * UE_DOUBLE_PI, UE_DOUBLE_PI, true, Chosen));
		}
		// Q-neutrals-hold = C: bodies close on both sides wall a soldier in (a free place it cannot reach is no stall); one side only, or far, does not.
	{
		const FVector2D Self(0., 0.);
		const FVector2D Target(300., 0.);
		TArray<HOLCombat::FBody> Sides;
		Sides.Add({ FVector2D(0., 125.), 55.f });
		TestFalse(TEXT("a friend on one side only does not box a soldier in"), HOLCombat::IsBoxedIn(Self, 55.f, Target, Sides));
		Sides.Add({ FVector2D(0., -140.), 55.f });
		TestTrue(TEXT("friends 125 and 140 cm off on both sides box it in"), HOLCombat::IsBoxedIn(Self, 55.f, Target, Sides));
		Sides[1].Where = FVector2D(0., -400.);
		TestFalse(TEXT("a body 400 cm off on one side is no wall"), HOLCombat::IsBoxedIn(Self, 55.f, Target, Sides));
		TestFalse(TEXT("nobody near: not boxed in"), HOLCombat::IsBoxedIn(Self, 55.f, Target, TConstArrayView<HOLCombat::FBody>()));
		// The trace of neutrals (2026-11-05, #11): friends at (-56, 109) and (64, 115) from the soldier, the target at (153, 262), one friend almost on the
		// line (cross +2) and the other left of it: both on the same side, a wall all the same (their gap, 10 cm, takes no body of 110).
		{
			TArray<HOLCombat::FBody> Wall;
			Wall.Add({ FVector2D(-56., 109.), 55.f });
			Wall.Add({ FVector2D(64., 115.), 55.f });
			TestTrue(TEXT("two friends ahead, 10 cm apart, on one side of the line: boxed in"), HOLCombat::IsBoxedIn(FVector2D::ZeroVector, 55.f, FVector2D(153., 262.), Wall));
			Wall[1].Where = FVector2D(64., 215.);
			TestFalse(TEXT("the second friend 215 cm off: no wall (out of reach)"), HOLCombat::IsBoxedIn(FVector2D::ZeroVector, 55.f, FVector2D(153., 262.), Wall));
			Wall[1].Where = FVector2D(-60., -100.);
			TestFalse(TEXT("one friend behind the soldier: no wall ahead"), HOLCombat::IsBoxedIn(FVector2D::ZeroVector, 55.f, FVector2D(153., 262.), Wall));
		}
		TestTrue(TEXT("a claimed place at 170 cm holds the rear flank back"), HOLCombat::ShouldSuspendRearFlank(true, 170.));
		TestFalse(TEXT("a claimed place at 600 cm does not"), HOLCombat::ShouldSuspendRearFlank(true, 600.));
		TestFalse(TEXT("no claimed place: the flank is not suspended"), HOLCombat::ShouldSuspendRearFlank(false, 100.));
		TestTrue(TEXT("a target 100 cm from the last is a neighbour"), HOLCombat::IsNeighbourTarget(FVector2D(0., 0.), FVector2D(60., 80.)));
		TestFalse(TEXT("a target 300 cm from the last is not"), HOLCombat::IsNeighbourTarget(FVector2D(0., 0.), FVector2D(300., 0.)));
	}
	// Q-neutrals-second-rank = C (the trace of neutrals, #11 behind #9 and #13, 2026-11-05): walled in for 1 s, the soldier turns to the nearest enemy
		// with a free place it can reach; not the target it leaves, not an enemy walled in by a ring of bodies.
		{
			TArray<HOLCombat::FBody> Bodies;
			Bodies.Add({ FVector2D(153., 262.), 55.f });   // 0: the target it leaves (edge 193)
			Bodies.Add({ FVector2D(-56., 109.), 55.f });   // 1: friend
			Bodies.Add({ FVector2D(64., 115.), 55.f });    // 2: friend
			Bodies.Add({ FVector2D(-400., -100.), 55.f }); // 3: enemy A, free and reachable (edge 302)
			const FVector2D RingCentre(212., -212.);
			Bodies.Add({ RingCentre, 55.f });              // 4: enemy B (edge 190), ringed by twelve bodies: no free place
			for (int32 Spot = 0; Spot < 12; ++Spot)
			{
				const double Angle = 2. * UE_DOUBLE_PI * Spot / 12;
				Bodies.Add({ RingCentre + 112. * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)), 55.f });
			}
			const TArray<int32> Enemies = { 0, 3, 4 };
			const TArray<uint64> Ids = { 10, 11, 12 };
			TestEqual(TEXT("it turns to enemy A: not the target it leaves, not the ringed enemy B"), HOLCombat::FindReachableEnemy(FVector2D::ZeroVector, 55.f, Bodies, Enemies, Ids, 0), 1);
			const TArray<int32> OnlyRinged = { 0, 4 };
			const TArray<uint64> RingedIds = { 10, 12 };
			TestEqual(TEXT("no enemy with a place it can reach: none chosen"), HOLCombat::FindReachableEnemy(FVector2D::ZeroVector, 55.f, Bodies, OnlyRinged, RingedIds, 0), static_cast<int32>(INDEX_NONE));
			TestTrue(TEXT("walled in for 1 s: it retargets"), HOLCombat::ShouldRetargetWhenBoxedIn(1.f, true, false));
			TestFalse(TEXT("walled in for 0.9 s: not yet"), HOLCombat::ShouldRetargetWhenBoxedIn(0.9f, true, false));
			TestFalse(TEXT("not walled in: it keeps its target"), HOLCombat::ShouldRetargetWhenBoxedIn(5.f, false, false));
			TestFalse(TEXT("a target the player ordered is kept"), HOLCombat::ShouldRetargetWhenBoxedIn(5.f, true, true));
		}
		// D641 (hol.Combat.FarPlaceRetarget, melee spin of 2026-11-05): a place claimed more than 1.5 rad from the soldier's side of the target is refused, and the
		// soldier turns to the nearest enemy with a place on ITS side (else it waits).
		{
			TestFalse(TEXT("a place on the soldier's own side is kept"), HOLCombat::IsPlaceTooFar(0.1, 0.0));
			TestTrue(TEXT("a place 2 rad round is too far"), HOLCombat::IsPlaceTooFar(2.0, 0.0));
			TestFalse(TEXT("the gap wraps round 2 pi: 6.0 against 0.1 is 0.38 rad"), HOLCombat::IsPlaceTooFar(6.0, 0.1));
			TestTrue(TEXT("the gap wraps round 2 pi: 4.0 against 0.1 is 2.38 rad"), HOLCombat::IsPlaceTooFar(4.0, 0.1));
			TestFalse(TEXT("exactly 1.5 rad is kept"), HOLCombat::IsPlaceTooFar(1.5, 0.0));
			TestTrue(TEXT("1.6 rad is too far"), HOLCombat::IsPlaceTooFar(1.6, 0.0));
			// Enemy E at (300, 0): its side towards the soldier is blocked by two friends of E (not walling the soldier in); the nearest free spot is 1.57 rad round.
			TArray<HOLCombat::FBody> Bodies;
			const FVector2D EWhere(300., 0.);
			Bodies.Add({ EWhere, 55.f });                                                                                           // 0: enemy E
			Bodies.Add({ EWhere + 112. * FVector2D(FMath::Cos(UE_DOUBLE_PI + 0.6), FMath::Sin(UE_DOUBLE_PI + 0.6)), 55.f });        // 1: blocks E's near side
			Bodies.Add({ EWhere + 112. * FVector2D(FMath::Cos(UE_DOUBLE_PI - 0.6), FMath::Sin(UE_DOUBLE_PI - 0.6)), 55.f });        // 2: blocks E's near side
			Bodies.Add({ FVector2D(-300., 100.), 55.f });                                                                           // 3: enemy F, open on the soldier's side
			const TArray<int32> Enemies = { 0, 3 };
			const TArray<uint64> Ids = { 10, 11 };
			TestEqual(TEXT("without the side rule E (edge 190) is the nearest with a place"), HOLCombat::FindReachableEnemy(FVector2D::ZeroVector, 55.f, Bodies, Enemies, Ids, INDEX_NONE), 0);
			TestEqual(TEXT("with the side rule E (its only places are across) is skipped: F"), HOLCombat::FindReachableEnemy(FVector2D::ZeroVector, 55.f, Bodies, Enemies, Ids, INDEX_NONE, 1.5), 1);
			const TArray<int32> OnlyE = { 0 };
			const TArray<uint64> OnlyEId = { 10 };
			TestEqual(TEXT("with the side rule and only E: none, the soldier waits"), HOLCombat::FindReachableEnemy(FVector2D::ZeroVector, 55.f, Bodies, OnlyE, OnlyEId, INDEX_NONE, 1.5), static_cast<int32>(INDEX_NONE));
		}
		// The run: 0.5 s over, told once; a step that moves ends the run, and the longest is kept.
	HOLSanity::FStall Stall;
	int32 Told = 0;
	for (int32 Index = 0; Index < 30; ++Index)
	{
		Told += Stall.Step(true, 0.05, 0.5) ? 1 : 0;
	}
	TestEqual(TEXT("a stall of 1.5 s over the 0.5 s limit is told once"), Told, 1);
	Stall.Step(false, 0.05, 0.5);
	TestEqual(TEXT("a step that moves ends the run"), Stall.Seconds, 0.);
	TestTrue(TEXT("the longest run is kept"), Stall.Longest > 1.4);
	HOLSanity::FStall Brief;
	for (int32 Index = 0; Index < 100; ++Index)
	{
		Brief.Step(Index % 8 != 0, 0.05, 0.5);
	}
	TestTrue(TEXT("a pause of 7 steps (0.35 s) now and again is not a stall"), Brief.Longest < 0.5 && !Brief.bTold);
	return true;
}

// hol.Test.Sanity, check 1 (SAN-Q1 = A, D616): the colour of an owner is the game's one rule (HOLPlayerColors::GetOwnerLinearColor), so a
// neutral owner (8) or an owner outside the slots is grey and clashes with no player; an owner that IS a slot, taken for a neutral one, is
// drawn in that slot's colour and the audit reports it (the mutation next to the sound case).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSanityColourTest, "HeroesOfLiberty.Sanity.Colour", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLSanityColourTest::RunTest(const FString& Parameters)
{
	const TArray<uint8> Colours = { 0, 1, 2, 3, 4, 5, 6, 7 };
	const TArray<int32> Players = { 0, 1 };
	TestTrue(TEXT("the neutral owner (8) is grey: no clash with slots 0 and 1"), HOLSanity::FindColourClashes(TArray<int32>{ 8 }, Players, Colours).IsEmpty());
	TestTrue(TEXT("an owner far out of the slots (200) is grey too"), HOLSanity::FindColourClashes(TArray<int32>{ 200 }, Players, Colours).IsEmpty());
	TestTrue(TEXT("a negative owner is grey too"), HOLSanity::FindColourClashes(TArray<int32>{ -1 }, Players, Colours).IsEmpty());
	const TArray<FString> Clash = HOLSanity::FindColourClashes(TArray<int32>{ 1 }, Players, Colours);
	TestEqual(TEXT("an owner that is slot 1, taken for a neutral one, clashes with slot 1 only"), Clash.Num(), 1);
	TestTrue(TEXT("the clash line names the owner"), !Clash.IsEmpty() && Clash[0].Contains(TEXT("owner 1")));
	// Two slots that chose the same colour: a neutral owner is still grey, the clash is only between the neutral and the slots.
	const TArray<uint8> Same = { 3, 3, 2, 2, 1, 1, 0, 0 };
	TestTrue(TEXT("slots sharing a colour do not make the grey neutral clash"), HOLSanity::FindColourClashes(TArray<int32>{ 8 }, Players, Same).IsEmpty());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
