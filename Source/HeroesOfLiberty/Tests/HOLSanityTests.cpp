// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLSanityRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr double StepSeconds = 1. / 30.;

	/** Judges the track of a unit whose place at Time (s) is Place(Time), over Seconds, with the given motion. */
	FString JudgeTrack(const HOLSanity::EUnitMotion Motion, const double Seconds, const TFunctionRef<FVector2D(double)> Place)
	{
		const HOLSanity::FSanityRules Rules;
		HOLSanity::FUnitTrack Track;
		FString Last;
		for (double Time = 0.; Time <= Seconds; Time += StepSeconds)
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

#endif // WITH_DEV_AUTOMATION_TESTS
