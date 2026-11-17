// GALADRIEL-1117r: script port of the PURE part of HOLVictoryTests.cpp. HOLVictoryRules (Evaluate, GetCamp, AreHostile), FHOLMatchConfig and the
// rest of HOLForumRules (Fall, Advance, Step, Raze, GetPlayersTable...) are NOT exposed to the script, so only the lives of the Forum.Rules
// test (HOLForumRules::GetLives, reached through HOLHeroXPRead::GetForumLives) are ported here. Same vectors as the C++ test.
// NOT COMPILED. The C++ test FHOLVictoryForumRulesTest stays (it also covers Fall / Advance / Skip / tuning, not exposed): nothing to remove yet.

void Test_VictoryForumLives(FUnitTest& T)
{
	// D349: 2 lives at tier 0, one more a tier, a fall takes one, never below 0.
	T.AssertEquals(2, HOLHeroXPRead::GetForumLives(0, 0), "tier 0, no fall: 2 lives (D349)");
	T.AssertEquals(1, HOLHeroXPRead::GetForumLives(0, 1), "tier 0, one fall: 1 life");
	T.AssertEquals(0, HOLHeroXPRead::GetForumLives(0, 2), "tier 0, two falls: 0 life");
	T.AssertEquals(0, HOLHeroXPRead::GetForumLives(0, 3), "never below 0");
	T.AssertEquals(3, HOLHeroXPRead::GetForumLives(1, 0), "tier 1, no fall: 3 lives");
}
