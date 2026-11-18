// GALADRIEL-1117s: the five "Victory.Rules" tests of HOLVictoryTests.cpp (OneVsOne, Teams, FreeForAll, SingleCamp, AllDefeated), same vectors, run
// against UHOLVictoryReadLibrary (HOLVictoryRead::Evaluate / GetCamp / AreHostile, wrappers of HOLVictoryRules, slots given as States and Teams).
// NOT COMPILED (no build in the agent): the C++ library must be built first. The C++ tests are NOT removed yet (red then green to be seen first).
const int VictoryHuman = 1;
const int VictoryAI = 2;

/** The first N of A, B, C, D as an array (slots 0..N-1 of a States / Teams list, or the slots of a mask). */
TArray<int32> VictoryList(int N, int A, int B, int C, int D)
{
	TArray<int32> Result;
	if (N > 0) Result.Add(A);
	if (N > 1) Result.Add(B);
	if (N > 2) Result.Add(C);
	if (N > 3) Result.Add(D);
	return Result;
}

/** Bit Slot set for each slot of the list. */
int VictoryMask(const TArray<int32>& Slots)
{
	int Result = 0;
	for (int i = 0; i < Slots.Num(); ++i)
	{
		Result = Result | (1 << Slots[i]);
	}
	return Result;
}

/** Calls HOLVictoryRead::Evaluate; the winners are checked against ExpectedWinners (in order) and WinnerMask against their bits; returns bOver. */
bool VictoryOver(FUnitTest& T, const TArray<int32>& States, const TArray<int32>& Teams, const TArray<int32>& Defeated, const TArray<int32>& ExpectedWinners, const FString& Label)
{
	int WinnerMask = -1;
	TArray<int32> Winners;
	bool bOver = HOLVictoryRead::Evaluate(States, Teams, VictoryMask(Defeated), WinnerMask, Winners);
	T.AssertEquals(ExpectedWinners.Num(), Winners.Num(), Label + ": number of winners");
	if (Winners.Num() == ExpectedWinners.Num())
	{
		for (int i = 0; i < Winners.Num(); ++i)
		{
			T.AssertEquals(ExpectedWinners[i], Winners[i], Label + ": a winner (in order)");
		}
	}
	T.AssertEquals(VictoryMask(ExpectedWinners), WinnerMask, Label + ": winner mask");
	return bOver;
}

// HeroesOfLiberty.Victory.Rules.OneVsOne
void Test_VictoryRulesOneVsOne(FUnitTest& T)
{
	TArray<int32> States = VictoryList(2, VictoryHuman, VictoryAI, 0, 0);
	TArray<int32> Teams = VictoryList(2, 0, 0, 0, 0);
	TArray<int32> None = VictoryList(0, 0, 0, 0, 0);
	TArray<int32> Slot0 = VictoryList(1, 0, 0, 0, 0);
	TArray<int32> Slot1 = VictoryList(1, 1, 0, 0, 0);

	T.AssertFalse(VictoryOver(T, States, Teams, None, None, "nobody defeated"), "nobody defeated: not over");
	T.AssertTrue(VictoryOver(T, States, Teams, Slot1, Slot0, "slot 1 defeated"), "slot 1 defeated: over, slot 0 wins");
	T.AssertTrue(VictoryOver(T, States, Teams, Slot0, Slot1, "the other way round"), "the other way round: over, slot 1 wins");
	T.AssertTrue(HOLVictoryRead::GetCamp(States, Teams, 0) != HOLVictoryRead::GetCamp(States, Teams, 1), "two slots of team 0: two camps");
	T.AssertEquals(-1, HOLVictoryRead::GetCamp(States, Teams, 2), "a closed slot has no camp");
}

// The camps and the hostility of two slots: team 0 each alone (hostile), one team 1..4 one camp (friends), two teams hostile.
void Test_VictoryRulesCampsAndHostility(FUnitTest& T)
{
	TArray<int32> States = VictoryList(2, VictoryHuman, VictoryAI, 0, 0);
	TArray<int32> Alone = VictoryList(2, 0, 0, 0, 0);
	T.AssertTrue(HOLVictoryRead::AreHostile(States, Alone, 0, 1), "two slots of team 0 fight each other");

	TArray<int32> Same = VictoryList(2, 2, 2, 0, 0);
	T.AssertFalse(HOLVictoryRead::AreHostile(States, Same, 0, 1), "two slots of team 2 do not");
	T.AssertEquals(HOLVictoryRead::GetCamp(States, Same, 0), HOLVictoryRead::GetCamp(States, Same, 1), "two slots of team 2: one camp");

	TArray<int32> Other = VictoryList(2, 1, 2, 0, 0);
	T.AssertTrue(HOLVictoryRead::AreHostile(States, Other, 0, 1), "team 1 against team 2 fight");
}

// HeroesOfLiberty.Victory.Rules.Teams
void Test_VictoryRulesTeams(FUnitTest& T)
{
	// 2v2: slots 0 and 2 in team 1, slots 1 and 3 in team 2.
	TArray<int32> States = VictoryList(4, VictoryHuman, VictoryAI, VictoryAI, VictoryHuman);
	TArray<int32> Teams = VictoryList(4, 1, 2, 1, 2);
	TArray<int32> None = VictoryList(0, 0, 0, 0, 0);

	T.AssertFalse(VictoryOver(T, States, Teams, VictoryList(1, 1, 0, 0, 0), None, "one of team 2 down"), "one of team 2 down: not over");
	T.AssertFalse(VictoryOver(T, States, Teams, VictoryList(2, 1, 2, 0, 0), None, "one down on each side"), "one down on each side: not over");
	T.AssertTrue(VictoryOver(T, States, Teams, VictoryList(3, 1, 2, 3, 0), VictoryList(2, 0, 2, 0, 0), "team 2 all down"),
		"team 2 all down: over, team 1 wins, its fallen slot 2 too (D357)");
}

// HeroesOfLiberty.Victory.Rules.FreeForAll
void Test_VictoryRulesFreeForAll(FUnitTest& T)
{
	TArray<int32> States = VictoryList(3, VictoryHuman, VictoryAI, VictoryAI, 0);
	TArray<int32> Teams = VictoryList(3, 0, 0, 0, 0);
	TArray<int32> None = VictoryList(0, 0, 0, 0, 0);

	T.AssertFalse(VictoryOver(T, States, Teams, VictoryList(1, 0, 0, 0, 0), None, "one of three down"), "one of three down: not over");
	T.AssertTrue(VictoryOver(T, States, Teams, VictoryList(2, 0, 2, 0, 0), VictoryList(1, 1, 0, 0, 0), "two of three down"),
		"two of three down: over, the last one standing wins");
}

// HeroesOfLiberty.Victory.Rules.SingleCamp
void Test_VictoryRulesSingleCamp(FUnitTest& T)
{
	TArray<int32> None = VictoryList(0, 0, 0, 0, 0);
	TArray<int32> Slot0 = VictoryList(1, 0, 0, 0, 0);

	TArray<int32> AloneStates = VictoryList(1, VictoryHuman, 0, 0, 0);
	TArray<int32> AloneTeams = VictoryList(1, 0, 0, 0, 0);
	T.AssertFalse(VictoryOver(T, AloneStates, AloneTeams, None, None, "a human alone at the launch"), "a human alone: no victory at the launch");
	T.AssertTrue(VictoryOver(T, AloneStates, AloneTeams, Slot0, None, "a human alone who falls"), "a human alone who falls: over, nobody wins");

	TArray<int32> OneTeamStates = VictoryList(3, VictoryHuman, VictoryAI, VictoryHuman, 0);
	TArray<int32> OneTeamTeams = VictoryList(3, 3, 3, 3, 0);
	T.AssertFalse(VictoryOver(T, OneTeamStates, OneTeamTeams, None, None, "everyone in one team at the launch"), "everyone in one team: no victory at the launch");
	T.AssertFalse(VictoryOver(T, OneTeamStates, OneTeamTeams, VictoryList(2, 0, 1, 0, 0), None, "one team, two down"), "one team, two down: not over");
	T.AssertTrue(VictoryOver(T, OneTeamStates, OneTeamTeams, VictoryList(3, 0, 1, 2, 0), None, "one team, all down"), "one team, all down: over, nobody wins");

	T.AssertFalse(VictoryOver(T, None, None, None, None, "no occupied slot"), "no occupied slot: never over");
}

// HeroesOfLiberty.Victory.Rules.AllDefeated
void Test_VictoryRulesAllDefeated(FUnitTest& T)
{
	TArray<int32> States = VictoryList(3, VictoryHuman, VictoryAI, VictoryAI, 0);
	TArray<int32> Teams = VictoryList(3, 1, 2, 0, 0);
	TArray<int32> None = VictoryList(0, 0, 0, 0, 0);
	T.AssertTrue(VictoryOver(T, States, Teams, VictoryList(3, 0, 1, 2, 0), None, "everyone defeated"), "everyone defeated: over, nobody wins");
}

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
