// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLDamageRedirectRules.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::DamageRedirect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	float ShareOf(const HOLDamageRedirect::FResult& Result, int32 KnightId)
	{
		for (const HOLDamageRedirect::FShare& Share : Result.Shares)
		{
			if (Share.KnightId == KnightId)
			{
				return Share.Damage;
			}
		}
		return 0.f;
	}

	float SharesSum(const HOLDamageRedirect::FResult& Result)
	{
		float Sum = 0.f;
		for (const HOLDamageRedirect::FShare& Share : Result.Shares)
		{
			Sum += Share.Damage;
		}
		return Sum;
	}
}

// Aube A2, D19 passive: who takes which part of a blow. Expected numbers by hand (damage 100, 30 %, radius 500 cm; the point (300, 400) is exactly
// 500 cm from the origin, a 3-4-5 triangle), none copied from the code under test. Conservation is checked as an EFFECT: Kept + shares = damage.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDamageRedirectRulesTest, "HeroesOfLiberty.DamageRedirect.Rules", UE::HeroesOfLiberty::Tests::DamageRedirect::Flags)

bool FHOLDamageRedirectRulesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::DamageRedirect;
	using namespace HOLDamageRedirect;
	constexpr int32 Radius = 500;
	constexpr int32 Percent = 30;
	const FKnight Knight{ 7, 0, 0, true };

	// 1. An ally on the edge: 30 % goes to the Knight, 70 % stays; the sum is conserved.
	{
		const FKnight Knights[] = { Knight };
		const FResult Result = Redirect(100.f, false, 1, 300, 400, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("the ally keeps 70"), Result.Kept, 70.f);
		TestEqual(TEXT("the Knight takes 30"), ShareOf(Result, 7), 30.f);
		TestEqual(TEXT("conservation: kept + shares = 100"), Result.Kept + SharesSum(Result), 100.f);
	}
	// 2. One centimetre out: no effect.
	{
		const FKnight Knights[] = { Knight };
		const FResult Result = Redirect(100.f, false, 1, 300, 400, Knights, Radius - 1, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("out of the radius: the ally keeps 100"), Result.Kept, 100.f);
		TestEqual(TEXT("out of the radius: no share"), Result.Shares.Num(), 0);
	}
	// 3. A hero is never protected; a dead Knight, and the Knight itself, redirect nothing.
	{
		const FKnight Knights[] = { Knight };
		const FResult Hero = Redirect(100.f, true, 1, 100, 0, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("a hero keeps 100"), Hero.Kept, 100.f);
		const FKnight Dead{ 8, 0, 0, false };
		const FKnight DeadOnly[] = { Dead };
		const FResult DeadResult = Redirect(100.f, false, 1, 100, 0, DeadOnly, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("a dead Knight takes nothing"), DeadResult.Kept, 100.f);
		const FKnight Knights2[] = { Knight };
		const FResult Self = Redirect(100.f, false, 7, 0, 0, Knights2, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("the Knight does not redirect its own damage"), Self.Kept, 100.f);
	}
	// 4. Two Knights, Nearest: only the nearer one takes the percent (never twice).
	{
		const FKnight Near{ 7, 100, 0, true };
		const FKnight Far{ 3, 0, 300, true };
		const FKnight Knights[] = { Far, Near };
		const FResult Result = Redirect(100.f, false, 1, 150, 0, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("Nearest: the nearer Knight takes 30"), ShareOf(Result, 7), 30.f);
		TestEqual(TEXT("Nearest: the farther Knight takes nothing"), ShareOf(Result, 3), 0.f);
		TestEqual(TEXT("Nearest: the ally keeps 70"), Result.Kept, 70.f);
	}
	// 5. Two Knights, CappedSum, 30 % each, cap 50 %: 50 redirected (not 60), 25 each, 50 kept.
	{
		const FKnight A{ 7, 100, 0, true };
		const FKnight B{ 3, 0, 300, true };
		const FKnight Knights[] = { A, B };
		const FResult Result = Redirect(100.f, false, 1, 150, 0, Knights, Radius, Percent, ECumulMode::CappedSum, 50);
		TestEqual(TEXT("CappedSum: the ally keeps 50"), Result.Kept, 50.f);
		TestEqual(TEXT("CappedSum: the first Knight takes 25"), ShareOf(Result, 7), 25.f);
		TestEqual(TEXT("CappedSum: the second Knight takes 25"), ShareOf(Result, 3), 25.f);
		TestEqual(TEXT("CappedSum: conservation"), Result.Kept + SharesSum(Result), 100.f);
	}
	// 6. The active version (100 %) takes everything; a percent above 100 is clamped; zero or negative damage redirects nothing.
	{
		const FKnight Knights[] = { Knight };
		const FResult Active = Redirect(80.f, false, 1, 100, 0, Knights, Radius, 100, ECumulMode::Nearest, 50);
		TestEqual(TEXT("100 %: the ally keeps 0"), Active.Kept, 0.f);
		TestEqual(TEXT("100 %: the Knight takes 80"), ShareOf(Active, 7), 80.f);
		const FResult Clamped = Redirect(80.f, false, 1, 100, 0, Knights, Radius, 250, ECumulMode::Nearest, 50);
		TestEqual(TEXT("250 % clamped to 100: the Knight takes 80"), ShareOf(Clamped, 7), 80.f);
		const FResult Zero = Redirect(0.f, false, 1, 100, 0, Knights, Radius, Percent, ECumulMode::Nearest, 50);
		TestEqual(TEXT("zero damage: nothing"), Zero.Shares.Num(), 0);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
