// Socle step 1, lot S1f (D830, D836, D839, SAURON-1117a): one smoke test per read library of the rules that STAY in C++ (relay ring, hero ability,
// deployable engine), each on ONE vector read in its C++ test. The full port of each C++ test is the next lot (L17-07, L17-08, L17-10 of
// Auto_Journey_File.md), after the build. This file compiles only once the plugin is rebuilt with the three libraries (HOLRelayReadLibrary,
// HOLHeroAbilityReadLibrary, HOLDeployReadLibrary): until then the script compile fails, as expected.

// HOLRelayRingTests.cpp (HeroesOfLiberty.Relays.Ring): a quarter held is 25 %, never 100 ; the view of a relay half filled.
void Test_RelayReadLibrarySmoke(FUnitTest& T)
{
	T.AssertEquals(25, HOLRelayRead::FillPercent(-1, -1, 150, 600), "a quarter: 25");
	T.AssertEquals(99, HOLRelayRead::FillPercent(-1, -1, 900, 600), "past the count: 99, never 100");
	FHOLRelayReadView View = HOLRelayRead::MakeView(FVector2D(1500.0, -2500.0), 1200.0f, 2, 1, 300, 600);
	T.AssertEquals(2, View.Owner, "view: owner");
	T.AssertEquals(50, View.FillPercent, "view: half filled");
	T.AssertTrue(View.Description == "(1500, -2500) r1200 owner 2 taker 1 fill 50", "view: the line the markers and the report read");
}

// HOLHeroAbilityTests.cpp (HeroesOfLiberty.HeroAbility.Unlock / Cast / Damage): the banner opens at level 4, the cry is cast ready, aura and cry add up.
void Test_HeroAbilityReadLibrarySmoke(FUnitTest& T)
{
	T.AssertEquals(4, HOLHeroAbilityRead::GetUnlockLevel(EHOLHeroAbility::Banner), "Banner at level 4");
	T.AssertFalse(HOLHeroAbilityRead::IsUnlocked(EHOLHeroAbility::Banner, 3), "Level 3 has no banner");
	T.AssertTrue(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::CloseRanks, 1, true, 0), "Cry ready, hero alive, level 1");
	T.AssertFalse(HOLHeroAbilityRead::CanCast(EHOLHeroAbility::CloseRanks, 1, false, 0), "Cry while the hero is dead");
	T.AssertEquals(60, HOLHeroAbilityRead::GetDamageTakenPercent(true, true), "Aura and cry: 60 %");
}

// HOLDeployRulesTests.cpp (HeroesOfLiberty.Siege.Deploy.WhoCanDoWhat): only a deployed engine fires, only a packed one walks, no action while changing.
void Test_DeployReadLibrarySmoke(FUnitTest& T)
{
	FHOLDeployState State;
	State.Phase = EHOLDeployPhase::Deployed;
	T.AssertTrue(HOLDeployRead::CanFire(State), "deployed: fires");
	T.AssertFalse(HOLDeployRead::CanMove(State), "deployed: does not walk");
	State.Phase = EHOLDeployPhase::Deploying;
	T.AssertFalse(HOLDeployRead::CanAct(State), "deploying: no action, no defence");
	State.Phase = EHOLDeployPhase::Packed;
	T.AssertTrue(HOLDeployRead::CanMove(State), "packed: walks");
}