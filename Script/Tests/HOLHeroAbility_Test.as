// Lot L17-08 of the migration (D830, D836, D839, SAURON-1117b): the script port of HOLHeroAbilityTests.cpp (3 tests), run against the read
// library of the rule that STAYS in C++ (UHOLHeroAbilityReadLibrary, S1f). Same vectors as the C++ tests. The C++ tests are kept until the
// registry says otherwise.
// Not ported: nothing of the three C++ tests is missing. The library adds GetCloseRanksSpeedPercent and GetHealRatePercent, which the C++ tests
// do not check (no vector to carry over: not covered here either).
// Differences of form: Tick takes its remaining time by value and returns the new one; the constants CloseRanksDurationMs, BannerDurationMs and
// MoraleReductionCapPercent are read through their getters; the float tolerance is a local helper.

bool HeroAbilityNear(float A, float B)
{
	return Math::Abs(A - B) < 0.0001f;
}

// HeroesOfLiberty.HeroAbility.Unlock (D52, J8c-Q2, J8c-Q3): aura and cry at level 1, banner at level 4; the cool-downs and durations.
void Test_HeroAbilityUnlock(FUnitTest& T)
{
	T.AssertEquals(1, HOLHeroAbilityRead::GetUnlockLevel(EHOLHeroAbility::Aura), "Aura at level 1");
	T.AssertEquals(1, HOLHeroAbilityRead::GetUnlockLevel(EHOLHeroAbility::CloseRanks), "Cry at level 1");
	T.AssertEquals(4, HOLHeroAbilityRead::GetUnlockLevel(EHOLHeroAbility::Banner), "Banner at level 4");
	T.AssertTrue(HOLHeroAbilityRead::IsUnlocked(EHOLHeroAbility::Aura, 1), "Level 1 has the aura");
	T.AssertTrue(HOLHeroAbilityRead::IsUnlocked(EHOLHeroAbility::CloseRanks, 1), "Level 1 has the cry");
	T.AssertFalse(HOLHeroAbilityRead::IsUnlocked(EHOLHeroAbility::Banner, 3), "Level 3 has no banner");
	T.AssertTrue(HOLHeroAbilityRead::IsUnlocked(EHOLHeroAbility::Banner, 4), "Level 4 has the banner");
	T.AssertTrue(HOLHeroAbilityRead::IsUnlocked(EHOLHeroAbility::Banner, 6), "Level 6 has the banner");

	T.AssertEquals(60000, HOLHeroAbilityRead::GetCooldownMs(EHOLHeroAbility::CloseRanks), "Cry: 60 s");
	T.AssertEquals(120000, HOLHeroAbilityRead::GetCooldownMs(EHOLHeroAbility::Banner), "Banner: 120 s");
	T.AssertEquals(0, HOLHeroAbilityRead::GetCooldownMs(EHOLHeroAbility::Aura), "Aura: none");
	T.AssertEquals(15000, HOLHeroAbilityRead::GetCloseRanksDurationMs(), "The cry lasts 15 s");
	T.AssertEquals(45000, HOLHeroAbilityRead::GetBannerDurationMs(), "A banner stands 45 s");
}

// HeroesOfLiberty.HeroAbility.Cast (J8c-Q1, Q3): who can cast, the banner's reach, the aura's reach, the cool-down on 50 ms steps.
void Test_HeroAbilityCast(FUnitTest& T)
{
	T.AssertTrue(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::CloseRanks, 1, true, 0), "Cry ready, hero alive, level 1");
	T.AssertFalse(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::CloseRanks, 1, false, 0), "Cry while the hero is dead (9.8)");
	T.AssertFalse(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::CloseRanks, 1, true, 50), "Cry 50 ms before its cool-down ends");
	T.AssertFalse(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::Banner, 3, true, 0), "Banner at level 3");
	T.AssertTrue(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::Banner, 4, true, 0), "Banner at level 4");
	T.AssertFalse(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::Aura, 6, true, 0), "The aura is not cast");

	T.AssertTrue(HOLHeroAbilityRead::CanPlantBanner(0.0f), "Banner at his feet");
	T.AssertTrue(HOLHeroAbilityRead::CanPlantBanner(1600.0f), "Banner 16 m away");
	T.AssertFalse(HOLHeroAbilityRead::CanPlantBanner(1601.0f), "Banner 16.01 m away");

	T.AssertTrue(HOLHeroAbilityRead::IsWithinAura(1100.0f), "Unit 11 m away covered");
	T.AssertTrue(HOLHeroAbilityRead::IsWithinAura(1200.0f), "Unit 12 m away covered");
	T.AssertFalse(HOLHeroAbilityRead::IsWithinAura(1201.0f), "Unit 12.01 m away not covered");

	int Cooldown = HOLHeroAbilityRead::GetCooldownMs(EHOLHeroAbility::CloseRanks);
	for (int Step = 0; Step < 1199; ++Step)
	{
		Cooldown = HOLHeroAbilityRead::Tick(Cooldown, 50);
	}
	T.AssertEquals(50, Cooldown, "One step left after 1 199 steps");
	Cooldown = HOLHeroAbilityRead::Tick(Cooldown, 50);
	T.AssertEquals(0, Cooldown, "Over after 1 200 steps");
	Cooldown = HOLHeroAbilityRead::Tick(Cooldown, 50);
	T.AssertEquals(0, Cooldown, "Stays at 0");
	T.AssertEquals(0, HOLHeroAbilityRead::Tick(30, 50), "A step longer than what is left: 0");
}

// HeroesOfLiberty.HeroAbility.Damage (J8c-Q1, Q2): aura -15 %, cry -25 %, added up (-40 %), cap 50 %; health is a float.
void Test_HeroAbilityDamage(FUnitTest& T)
{
	T.AssertEquals(100, HOLHeroAbilityRead::GetDamageTakenPercent(false, false), "No effect: all of it");
	T.AssertEquals(85, HOLHeroAbilityRead::GetDamageTakenPercent(true, false), "Aura: 85 %");
	T.AssertEquals(75, HOLHeroAbilityRead::GetDamageTakenPercent(false, true), "Cry: 75 %");
	T.AssertEquals(60, HOLHeroAbilityRead::GetDamageTakenPercent(true, true), "Aura and cry: 60 %");
	T.AssertTrue(HOLHeroAbilityRead::GetDamageTakenPercent(true, true) >= 100 - HOLHeroAbilityRead::GetMoraleReductionCapPercent(), "Never under the cap");

	T.AssertTrue(HeroAbilityNear(HOLHeroAbilityRead::GetReducedDamage(5.0f, 85), 4.25f), "Archer's 5 in the aura");
	T.AssertTrue(HeroAbilityNear(HOLHeroAbilityRead::GetReducedDamage(3.0f, 60), 1.8f), "Peasant's 3 under aura and cry");
	T.AssertTrue(HeroAbilityNear(HOLHeroAbilityRead::GetReducedDamage(1.5f, 85), 1.275f), "1.5 on a hero in his aura");
	T.AssertTrue(HeroAbilityNear(HOLHeroAbilityRead::GetReducedDamage(12.0f, 100), 12.0f), "Nothing changed at 100 %");
}