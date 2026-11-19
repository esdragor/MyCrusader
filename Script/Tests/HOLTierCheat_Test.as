// Script port of HeroesOfLiberty.Match.TierCheat (HOLTierCheatTests.cpp, D853, lot 1 of PLAN-TESTS-1119, ELROND-1119r): same case, same values.
// G0 (Palier3InProgress.md, D710): hol.Cheat.Tier forces a slot's tier whatever its hero's level. The effect is read through the rule that matters,
// the check of the siege engines (HOLSiegeCheck::Check, tier 3), fed by AHOLGameState::GetTier as production, grid and AI do.
// The world is the one the C++ test builds (HOLTestWorld::CreateTestWorld), the game state the bare actor (SpawnGameState(.., 0, 0): no InitVictory, as there).
// The C++ test stays until this one has been seen red then green (D853: the main session removes it).

namespace TierCheatTest
{
	// What HOLSiegeCheck::Check says of a Ram (workshop finished, can pay) at this tier: EHOLSiegeBlock::TierTooLow or Available.
	int RamBlockAt(int Tier)
	{
		return HOLSiegeCheck::Check(int(EHOLSiegeEngine::Ram), Tier, true, true);
	}

	void Run(FUnitTest& T, AHOLGameState State)
	{
		// Each value is read into a local first (a call as the argument of an Assert has given null accesses, brief_commun pitfall 4).
		int Tier = State.GetTier(0);
		T.AssertEquals(0, Tier, "without the command the tier is the hero's level's (no hero: 0)");
		T.AssertEquals(int(EHOLSiegeBlock::TierTooLow), RamBlockAt(Tier), "a Ram (tier 1) is refused at tier 0");

		State.SetTierOverride(0, 3);
		Tier = State.GetTier(0);
		T.AssertEquals(3, Tier, "the forced tier is read by GetTier though the hero is at level 1");
		T.AssertEquals(int(EHOLSiegeBlock::Available), RamBlockAt(Tier), "a Ram is no longer refused for its tier");
		int OtherTier = State.GetTier(1);
		T.AssertEquals(0, OtherTier, "the other slots are untouched");
		int OtherOverride = State.GetTierOverride(1);
		T.AssertEquals(-1, OtherOverride, "and report no override");

		State.SetTierOverride(0, 9);
		Tier = State.GetTier(0);
		T.AssertEquals(3, Tier, "a tier above 3 is held to 3");
		State.SetTierOverride(0, 1);
		Tier = State.GetTier(0);
		T.AssertEquals(1, Tier, "a lower forced tier is read as well (0 to 3 both ways)");
		State.SetTierOverride(0, 0);
		int Override = State.GetTierOverride(0);
		T.AssertEquals(0, Override, "tier 0 forced is an override, not 'none'");
		State.SetTierOverride(0, -1);
		Tier = State.GetTier(0);
		T.AssertEquals(0, Tier, "off: back to the hero's level's tier");
		State.SetTierOverride(99, 3);
		Tier = State.GetTier(99);
		T.AssertEquals(0, Tier, "a slot out of the table changes nothing");

		T.AssertTrue(HOLTestWorld::IsPropertyReplicated(State, n"TierOverrides"), "the override is a replicated property: a client reads the same tier");
		// The reflection answers no for a property that is not replicated, and for a name the class does not have (it does not say yes to everything).
		T.AssertFalse(HOLTestWorld::IsPropertyReplicated(State, n"NoSuchProperty"), "a property the class does not have is not replicated");
	}
}

void Test_TierCheat(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	T.AssertTrue(State != nullptr, "game state");
	if (State != nullptr)
		TierCheatTest::Run(T, State);
	// No assertion stops the test (they record and go on), so the world is always destroyed here.
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
