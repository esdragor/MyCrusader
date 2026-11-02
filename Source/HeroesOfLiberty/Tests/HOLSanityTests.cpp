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
