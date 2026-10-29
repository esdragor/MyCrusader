// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLHeroAbilityRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// J8c.1 checks (J8c-Q1 to Q3): the hero's aura and abilities, numbers written from the decisions, not from the code.
namespace UE::HeroesOfLiberty::Tests::HeroAbility
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	constexpr float Tolerance = 1.e-4f;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroAbilityUnlockTest, "HeroesOfLiberty.HeroAbility.Unlock", UE::HeroesOfLiberty::Tests::HeroAbility::Flags)

bool FHOLHeroAbilityUnlockTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeroAbility;
	// D52: aura and cry at level 1, banner at level 4.
	TestEqual(TEXT("Aura at level 1"), GetUnlockLevel(EHOLHeroAbility::Aura), 1);
	TestEqual(TEXT("Cry at level 1"), GetUnlockLevel(EHOLHeroAbility::CloseRanks), 1);
	TestEqual(TEXT("Banner at level 4"), GetUnlockLevel(EHOLHeroAbility::Banner), 4);
	TestTrue(TEXT("Level 1 has the aura"), IsUnlocked(EHOLHeroAbility::Aura, 1));
	TestTrue(TEXT("Level 1 has the cry"), IsUnlocked(EHOLHeroAbility::CloseRanks, 1));
	TestFalse(TEXT("Level 3 has no banner"), IsUnlocked(EHOLHeroAbility::Banner, 3));
	TestTrue(TEXT("Level 4 has the banner"), IsUnlocked(EHOLHeroAbility::Banner, 4));
	TestTrue(TEXT("Level 6 has the banner"), IsUnlocked(EHOLHeroAbility::Banner, 6));

	// J8c-Q2 and Q3: cool-downs 60 s and 120 s; the aura is passive.
	TestEqual(TEXT("Cry: 60 s"), GetCooldownMs(EHOLHeroAbility::CloseRanks), 60000);
	TestEqual(TEXT("Banner: 120 s"), GetCooldownMs(EHOLHeroAbility::Banner), 120000);
	TestEqual(TEXT("Aura: none"), GetCooldownMs(EHOLHeroAbility::Aura), 0);
	TestEqual(TEXT("The cry lasts 15 s"), CloseRanksDurationMs, 15000);
	TestEqual(TEXT("A banner stands 45 s"), BannerDurationMs, 45000);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroAbilityCastTest, "HeroesOfLiberty.HeroAbility.Cast", UE::HeroesOfLiberty::Tests::HeroAbility::Flags)

bool FHOLHeroAbilityCastTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeroAbility;
	TestTrue(TEXT("Cry ready, hero alive, level 1"), CanCast(EHOLHeroAbility::CloseRanks, 1, true, 0));
	TestFalse(TEXT("Cry while the hero is dead (§ 9.8)"), CanCast(EHOLHeroAbility::CloseRanks, 1, false, 0));
	TestFalse(TEXT("Cry 50 ms before its cool-down ends"), CanCast(EHOLHeroAbility::CloseRanks, 1, true, 50));
	TestFalse(TEXT("Banner at level 3"), CanCast(EHOLHeroAbility::Banner, 3, true, 0));
	TestTrue(TEXT("Banner at level 4"), CanCast(EHOLHeroAbility::Banner, 4, true, 0));
	TestFalse(TEXT("The aura is not cast"), CanCast(EHOLHeroAbility::Aura, 6, true, 0));

	// J8c-Q3: 4 tiles at most from the hero.
	TestTrue(TEXT("Banner at his feet"), CanPlantBanner(0.f));
	TestTrue(TEXT("Banner 16 m away"), CanPlantBanner(1600.f));
	TestFalse(TEXT("Banner 16.01 m away"), CanPlantBanner(1601.f));

	// 12 m, edge included (J8c-Q1, as the command of J8b-Q1).
	TestTrue(TEXT("Unit 11 m away covered"), IsWithinAura(1100.f));
	TestTrue(TEXT("Unit 12 m away covered"), IsWithinAura(1200.f));
	TestFalse(TEXT("Unit 12.01 m away not covered"), IsWithinAura(1201.f));

	// On 50 ms steps: the cry's cool-down is 1 200 steps, never below 0.
	int32 Cooldown = GetCooldownMs(EHOLHeroAbility::CloseRanks);
	for (int32 Step = 0; Step < 1199; ++Step)
	{
		Tick(Cooldown, 50);
	}
	TestEqual(TEXT("One step left after 1 199 steps"), Cooldown, 50);
	Tick(Cooldown, 50);
	TestEqual(TEXT("Over after 1 200 steps"), Cooldown, 0);
	Tick(Cooldown, 50);
	TestEqual(TEXT("Stays at 0"), Cooldown, 0);
	int32 Short = 30;
	Tick(Short, 50);
	TestEqual(TEXT("A step longer than what is left: 0"), Short, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLHeroAbilityDamageTest, "HeroesOfLiberty.HeroAbility.Damage", UE::HeroesOfLiberty::Tests::HeroAbility::Flags)

bool FHOLHeroAbilityDamageTest::RunTest(const FString& Parameters)
{
	using namespace HOLHeroAbility;
	using UE::HeroesOfLiberty::Tests::HeroAbility::Tolerance;
	// J8c-Q1: aura -15 %; J8c-Q2: cry -25 %, added up (-40 %), cap 50 %.
	TestEqual(TEXT("No effect: all of it"), GetDamageTakenPercent(false, false), 100);
	TestEqual(TEXT("Aura: 85 %"), GetDamageTakenPercent(true, false), 85);
	TestEqual(TEXT("Cry: 75 %"), GetDamageTakenPercent(false, true), 75);
	TestEqual(TEXT("Aura and cry: 60 %"), GetDamageTakenPercent(true, true), 60);
	TestTrue(TEXT("Never under the cap"), GetDamageTakenPercent(true, true) >= 100 - MoraleReductionCapPercent);

	// Health is a float: an Archer's 5 in the aura is 4.25; a Peasant's 3 under both 1.8; a troop's 1.5 on a hero
	// (D17) in the aura 1.275.
	TestEqual(TEXT("Archer's 5 in the aura"), GetReducedDamage(5.f, 85), 4.25f, Tolerance);
	TestEqual(TEXT("Peasant's 3 under aura and cry"), GetReducedDamage(3.f, 60), 1.8f, Tolerance);
	TestEqual(TEXT("1.5 on a hero in his aura"), GetReducedDamage(1.5f, 85), 1.275f, Tolerance);
	TestEqual(TEXT("Nothing changed at 100 %"), GetReducedDamage(12.f, 100), 12.f, Tolerance);
	return true;
}

#endif
