// Copyright Epic Games, Inc. All Rights Reserved.

#include "Math/Box2D.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// F8 (D884): the pure rule of a building's shot reach, counted from the edge of its body (HOLCombat::IsInBuildingShotRange).
// The body is the 800 x 800 footprint of the TowerWood centred on the origin, the unit a 40 cm radius body, the reach 3000 cm.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingShotRangeTest, "HeroesOfLiberty.Combat.BuildingShotRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLBuildingShotRangeTest::RunTest(const FString& Parameters)
{
	const FBox2D Body(FVector2D(-400., -400.), FVector2D(400., 400.));
	constexpr float Radius = 40.f;
	constexpr float Range = 3000.f;

	// Along the axis: the edge of the body is at X = 400, the unit's edge 40 cm nearer than its centre.
	TestTrue(TEXT("edges exactly 3000 apart (centre at 3440): in reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(3440., 0.), Radius, Range));
	TestTrue(TEXT("1 cm inside (centre at 3439): in reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(3439., 0.), Radius, Range));
	TestFalse(TEXT("1 cm outside (centre at 3441): out of reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(3441., 0.), Radius, Range));
	TestTrue(TEXT("the same on the left side of the body"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(-3440., 0.), Radius, Range));
	TestFalse(TEXT("the same on the left side, 1 cm outside"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(-3441., 0.), Radius, Range));

	// From the EDGE, not from the centre: a centre 3300 away is 2900 from the wall, in reach of a 3000 shot.
	TestTrue(TEXT("centre 3300 away, wall 2900 away: in reach (counted from the edge)"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(3300., 0.), Radius, Range));

	// The boundary found by a sweep is the geometry's (edge of the body + the unit's radius + the reach), not a copied number.
	double LastIn = -1.;
	for (int32 X = 0; X <= 4000; ++X)
	{
		if (HOLCombat::IsInBuildingShotRange(Body, FVector2D(static_cast<double>(X), 0.), Radius, Range))
		{
			LastIn = static_cast<double>(X);
		}
	}
	TestEqual(TEXT("the last X in reach is the body's edge + radius + reach"), LastIn, Body.Max.X + static_cast<double>(Radius) + static_cast<double>(Range));

	// Corner: the nearest point of the body is the corner, the distance is the diagonal one.
	TestTrue(TEXT("diagonal from the corner, 2400/2400 (gap 2828 + 40 radius slack): in reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(2400., 2400.), Radius, Range));
	TestFalse(TEXT("diagonal from the corner, 2600/2600 (gap 3111 - 40 radius > 3000): out of reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(2600., 2600.), Radius, Range));

	// A target inside the body (a unit pushed into the wall, or on a footprint): gap 0, in reach.
	TestTrue(TEXT("target at the centre of the body: in reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(0., 0.), Radius, Range));
	TestTrue(TEXT("target inside the body, off centre: in reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(300., -250.), Radius, Range));
	TestTrue(TEXT("target on the wall: in reach"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(400., 0.), Radius, Range));

	// A reach of 0 (or less) is no shot: never, even for a target inside the body or touching it.
	TestFalse(TEXT("reach 0: nothing inside the body"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(0., 0.), Radius, 0.f));
	TestFalse(TEXT("reach 0: nothing against the wall"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(440., 0.), Radius, 0.f));
	TestFalse(TEXT("reach 0: nothing far away"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(3440., 0.), Radius, 0.f));
	TestFalse(TEXT("a negative reach: never"), HOLCombat::IsInBuildingShotRange(Body, FVector2D(0., 0.), Radius, -5.f));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
