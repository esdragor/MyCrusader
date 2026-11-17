// MIG-C-PetitesRegles (ELROND-1117k): script port of HOLHeroXPTests.cpp (Sources, Tiers, KillSource, Levels, Stats), run through UHOLHeroXPReadLibrary
// (Simulation/HOLSmallRulesReadLibrary.*, namespace HOLHeroXPRead, envelopes only; the rule HOLHeroXPRules.h stays in C++). Same vectors as the C++ tests.
// NEEDS THE C++ LOT: compile only after the build. Not ported: Chain (real unit table, HOLCombat, HOLHeroSheet), kept in HOLHeroXPTests.cpp.
// The C++ test file is removed only after the red then green of this one (not done yet).

const int HOLXPCombat = 0;
const int HOLXPCommand = 1;
const int HOLXPDevelopment = 2;
const int HOLXPExploration = 3;
const int HOLXPMaxLevel = 6;
const double HOLXPTolerance = 0.001;

bool HOLXPNear(double A, double B)
{
	return Math::Abs(A - B) <= HOLXPTolerance;
}

void Test_HeroXPSources(FUnitTest& T)
{
	// J8b-Q2: military > economy > exploration.
	T.AssertEquals(100, HOLHeroXPRead::GetWeightPercent(HOLXPCombat), "Combat x1");
	T.AssertEquals(75, HOLHeroXPRead::GetWeightPercent(HOLXPCommand), "Command x0.75");
	T.AssertEquals(50, HOLHeroXPRead::GetWeightPercent(HOLXPDevelopment), "Development x0.5");
	T.AssertEquals(25, HOLHeroXPRead::GetWeightPercent(HOLXPExploration), "Exploration x0.25");

	// The value is the cost, the four resources added up (food, wood, stone, gold).
	T.AssertEquals(120, HOLHeroXPRead::GetValue(100, 0, 0, 20), "Man-at-arms 100 food + 20 gold = 120");
	T.AssertEquals(80, HOLHeroXPRead::GetValue(60, 20, 0, 0), "Spearman 60 food + 20 wood = 80");
	T.AssertEquals(0, HOLHeroXPRead::GetValue(0, 0, 0, 0), "Nothing is worth 0");

	// Examples of J8b-Q2.
	T.AssertEquals(120, HOLHeroXPRead::GetXP(120, HOLXPCombat), "Man-at-arms killed by the hero: 120");
	T.AssertEquals(90, HOLHeroXPRead::GetXP(120, HOLXPCommand), "... by his troops near him: 90");
	T.AssertEquals(75, HOLHeroXPRead::GetXP(150, HOLXPDevelopment), "Barracks finished: 75");
	T.AssertEquals(25, HOLHeroXPRead::GetXP(50, HOLXPDevelopment), "House finished: 25");
	T.AssertEquals(37, HOLHeroXPRead::GetXP(75, HOLXPDevelopment), "Farm (75 wood) finished: 37, rounded down");
	T.AssertEquals(37, HOLHeroXPRead::GetXP(50, HOLXPCommand), "Peasant by his troops: 37");
	T.AssertEquals(0, HOLHeroXPRead::GetXP(0, HOLXPCombat), "Nothing gives nothing");

	// J8b-Q5: a hero killed, 300 x (victim's level / killer's level) x the weight.
	T.AssertEquals(300, HOLHeroXPRead::GetHeroKillXP(1, 1, HOLXPCombat), "Same level, by the hero: 300");
	T.AssertEquals(600, HOLHeroXPRead::GetHeroKillXP(4, 2, HOLXPCombat), "Level 4 killed by a level 2: 600");
	T.AssertEquals(100, HOLHeroXPRead::GetHeroKillXP(2, 6, HOLXPCombat), "Level 2 killed by a level 6: 100");
	T.AssertEquals(225, HOLHeroXPRead::GetHeroKillXP(3, 3, HOLXPCommand), "Level 3 by the troops of a level 3: 225");
	T.AssertEquals(37, HOLHeroXPRead::GetHeroKillXP(1, 6, HOLXPCommand), "Level 1 by the troops of a level 6: 37");

	// D06: only the civilisation's sources while dead.
	T.AssertFalse(HOLHeroXPRead::CountsWhileDead(HOLXPCombat), "No combat XP while dead");
	T.AssertFalse(HOLHeroXPRead::CountsWhileDead(HOLXPCommand), "No command XP while dead");
	T.AssertTrue(HOLHeroXPRead::CountsWhileDead(HOLXPDevelopment), "Development XP while dead");
	T.AssertTrue(HOLHeroXPRead::CountsWhileDead(HOLXPExploration), "Exploration XP while dead");
}

// P1 (D04): the hero's level opens the civilisation's tier, 3, 6 and 9 for tiers 1, 2 and 3.
void Test_HeroXPTiers(FUnitTest& T)
{
	// Level -> tier, pairs of a flat array (the script has no array literal): -1 -> 0, 0 -> 0, 1 -> 0, 2 -> 0, 3 -> 1, 4 -> 1, 5 -> 1, 6 -> 2, 7 -> 2, 8 -> 2, 9 -> 3, 10 -> 3.
	TArray<int> Tiers;
	Tiers.Add(0); Tiers.Add(0); Tiers.Add(0); Tiers.Add(0); Tiers.Add(1); Tiers.Add(1);
	Tiers.Add(1); Tiers.Add(2); Tiers.Add(2); Tiers.Add(2); Tiers.Add(3); Tiers.Add(3);
	for (int Index = 0; Index < Tiers.Num(); ++Index)
	{
		const int Level = Index - 1;
		T.AssertEquals(Tiers[Index], HOLHeroXPRead::GetTier(Level), "Level " + Level + ": tier " + Tiers[Index]);
	}
	// The prototype stops at level 6: the last tier it reaches is 2 (5 lives would need the level 9).
	T.AssertEquals(2, HOLHeroXPRead::GetTier(HOLXPMaxLevel), "The prototype's last level: tier 2");
	// D349: 2 lives at tier 0, then one more a tier, a fall takes one.
	T.AssertEquals(3, HOLHeroXPRead::GetForumLives(HOLHeroXPRead::GetTier(3), 0), "Tier 1, no fall: 3 lives");
	T.AssertEquals(3, HOLHeroXPRead::GetForumLives(HOLHeroXPRead::GetTier(6), 1), "Tier 2, one fall: 3 lives");
	T.AssertEquals(0, HOLHeroXPRead::GetForumLives(HOLHeroXPRead::GetTier(2), 2), "Tier 0, two falls: none");
}

void Test_HeroXPKillSource(FUnitTest& T)
{
	int Source = -1;
	T.AssertTrue(HOLHeroXPRead::GetKillSource(true, true, 0.0, Source), "Killed by the hero: a source");
	T.AssertEquals(HOLXPCombat, Source, "Killed by the hero: combat");
	T.AssertTrue(HOLHeroXPRead::GetKillSource(false, true, 1100.0, Source), "By a troop 11 m from him: a source");
	T.AssertEquals(HOLXPCommand, Source, "By a troop 11 m from him: command");
	T.AssertTrue(HOLHeroXPRead::GetKillSource(false, true, 1200.0, Source), "... 12 m: a source still");
	T.AssertEquals(HOLXPCommand, Source, "... 12 m: command still");
	T.AssertFalse(HOLHeroXPRead::GetKillSource(false, true, 1300.0, Source), "... 13 m: nothing");
	T.AssertFalse(HOLHeroXPRead::GetKillSource(false, false, 100.0, Source), "By a troop while he is dead: nothing (D06)");
}

void Test_HeroXPLevels(FUnitTest& T)
{
	// J8b-Q2: totals 200, 600, 1 300, 2 200, 3 300.
	TArray<int> Expected;
	Expected.Add(0); Expected.Add(200); Expected.Add(600); Expected.Add(1300); Expected.Add(2200); Expected.Add(3300);
	for (int Level = 1; Level <= 6; ++Level)
	{
		T.AssertEquals(Expected[Level - 1], HOLHeroXPRead::GetLevelXP(Level), "Level " + Level + " at " + Expected[Level - 1] + " XP");
	}
	T.AssertEquals(0, HOLHeroXPRead::GetLevelXP(0), "Below level 1: level 1's");
	T.AssertEquals(3300, HOLHeroXPRead::GetLevelXP(7), "Above 6: level 6's");

	T.AssertEquals(1, HOLHeroXPRead::GetLevel(0), "0 XP: level 1");
	T.AssertEquals(1, HOLHeroXPRead::GetLevel(199), "199: level 1");
	T.AssertEquals(2, HOLHeroXPRead::GetLevel(200), "200: level 2");
	T.AssertEquals(2, HOLHeroXPRead::GetLevel(599), "599: level 2");
	T.AssertEquals(3, HOLHeroXPRead::GetLevel(600), "600: level 3");
	T.AssertEquals(4, HOLHeroXPRead::GetLevel(1300), "1 300: level 4");
	T.AssertEquals(4, HOLHeroXPRead::GetLevel(2199), "2 199: level 4");
	T.AssertEquals(5, HOLHeroXPRead::GetLevel(2200), "2 200: level 5");
	T.AssertEquals(6, HOLHeroXPRead::GetLevel(3300), "3 300: level 6");
	T.AssertEquals(6, HOLHeroXPRead::GetLevel(100000), "100 000: level 6, the prototype's last");
	T.AssertEquals(1, HOLHeroXPRead::GetLevel(-5), "Negative XP: level 1");
}

void Test_HeroXPStats(FUnitTest& T)
{
	// J8b-Q3: +8 % a level on level 1's stats.
	T.AssertEquals(100, HOLHeroXPRead::GetStatPercent(1), "Level 1: 100 %");
	T.AssertEquals(108, HOLHeroXPRead::GetStatPercent(2), "Level 2: 108 %");
	T.AssertEquals(140, HOLHeroXPRead::GetStatPercent(6), "Level 6: 140 %");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetLevelStat(450.0, 1), 450.0), "Level 1 HP: 450");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetLevelStat(450.0, 2), 486.0), "Level 2 HP: 486");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetLevelStat(450.0, 6), 630.0), "Level 6 HP: 630");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetLevelStat(30.0, 2), 32.4), "Level 2 attack: 32.4");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetLevelStat(30.0, 6), 42.0), "Level 6 attack: 42");

	// The HP gained are added to the current health.
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetHealthAfterLevelUp(200.0, 450.0, 1, 2), 236.0), "Wounded at 200, level 1 -> 2: 236");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetHealthAfterLevelUp(450.0, 450.0, 1, 2), 486.0), "Full at 450, level 1 -> 2: full at 486");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetHealthAfterLevelUp(100.0, 450.0, 2, 4), 172.0), "Two levels at once, 2 -> 4 from 100: 172");
	T.AssertTrue(HOLXPNear(HOLHeroXPRead::GetHealthAfterLevelUp(0.0, 450.0, 1, 2), 0.0), "Dead stays dead (0)");
}
