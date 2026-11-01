// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLSanityRules.h"

#if WITH_DEV_AUTOMATION_TESTS

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
	return true;
}

#endif


