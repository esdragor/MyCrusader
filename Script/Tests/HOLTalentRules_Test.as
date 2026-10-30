// The talent tree's rules (T1, HOLTalentRules.as; GDD 9.6, D103-D105): points by level, one talent of three per row in order,
// the seal at 3 talents of a family, the key talent at 5. Families: 0 Guardian, 1 Captain, 2 Champion (the Dawn Order's
// three Oaths, D106). The levels are the hero's (level 2 opens row 2); the maximum level is read from the real hero rules.

// A hero who picked these families, row 2 first.
TArray<int> TalentPicks(int F0, int F1 = -1, int F2 = -1, int F3 = -1, int F4 = -1, int F5 = -1, int F6 = -1, int F7 = -1)
{
	TArray<int> Picks;
	if (F0 >= 0) Picks.Add(F0);
	if (F1 >= 0) Picks.Add(F1);
	if (F2 >= 0) Picks.Add(F2);
	if (F3 >= 0) Picks.Add(F3);
	if (F4 >= 0) Picks.Add(F4);
	if (F5 >= 0) Picks.Add(F5);
	if (F6 >= 0) Picks.Add(F6);
	if (F7 >= 0) Picks.Add(F7);
	return Picks;
}

// D04: one point a level from 2 to 9, none at level 1 (aura and ability 1 only) and none at the ultimate's level 10.
void Test_TalentPointsByLevel(FUnitTest& T)
{
	T.AssertEquals(0, HOLTalent::PointsEarned(0), "no hero, no point");
	T.AssertEquals(0, HOLTalent::PointsEarned(1), "level 1: none");
	T.AssertEquals(1, HOLTalent::PointsEarned(2), "level 2: 1");
	T.AssertEquals(5, HOLTalent::PointsEarned(6), "level 6: 5");
	T.AssertEquals(8, HOLTalent::PointsEarned(9), "level 9: 8");
	T.AssertEquals(8, HOLTalent::PointsEarned(10), "level 10: still 8 (the ultimate takes no talent)");
}

// The prototype stops at the real maximum level (HOLHero::GetMaxLevel, 6): 5 points; the levels' XP step is the hero's own.
void Test_TalentPointsOfThePrototype(FUnitTest& T)
{
	const int MaxLevel = HOLHero::GetMaxLevel();
	T.AssertEquals(6, MaxLevel, "the prototype's maximum level");
	T.AssertEquals(5, HOLTalent::PointsEarned(MaxLevel), "5 points at the prototype's maximum level (GDD 9.6)");
	for (int Level = 1; Level <= MaxLevel; Level++)
	{
		T.AssertTrue(HOLHero::GetLevelXP(Level) >= 0, "the level's XP is read from the hero's rules");
		T.AssertEquals(Level - 1, HOLTalent::PointsEarned(Level), "a point per level from level 2, level " + Level);
	}
}

// The reserve (D105): points earned minus talents taken, never below 0.
void Test_TalentPointsAvailable(FUnitTest& T)
{
	T.AssertEquals(0, HOLTalent::PointsAvailable(1, TalentPicks(-1)), "level 1, nothing taken: 0");
	T.AssertEquals(1, HOLTalent::PointsAvailable(2, TalentPicks(-1)), "level 2, nothing taken: 1");
	T.AssertEquals(0, HOLTalent::PointsAvailable(2, TalentPicks(0)), "level 2, one taken: 0");
	T.AssertEquals(3, HOLTalent::PointsAvailable(6, TalentPicks(0, 0)), "level 6, two taken: 3 kept in reserve");
	T.AssertEquals(0, HOLTalent::PointsAvailable(3, TalentPicks(0, 0, 0)), "never below 0");
}

// Rows fill in order (D105): the next row is 2, then 3...; none after the row 9.
void Test_TalentRowsInOrder(FUnitTest& T)
{
	T.AssertEquals(2, HOLTalent::NextRow(TalentPicks(-1)), "first row to fill: 2");
	T.AssertEquals(4, HOLTalent::NextRow(TalentPicks(0, 1)), "after two: row 4");
	T.AssertEquals(HOLTalent::RowNone, HOLTalent::NextRow(TalentPicks(0, 0, 0, 0, 0, 0, 0, 0)), "all 8 rows filled: none");
}

// One talent of three: the family (the column of the row) 0, 1 or 2; a point is needed; refused otherwise, nothing taken.
void Test_TalentPickOneOfThree(FUnitTest& T)
{
	TArray<int> Picks;
	T.AssertFalse(HOLTalent::CanPick(1, Picks, 0), "level 1: no point, no pick");
	T.AssertFalse(HOLTalent::Pick(1, Picks, 0), "refused at level 1");
	T.AssertEquals(0, Picks.Num(), "nothing taken");
	T.AssertFalse(HOLTalent::CanPick(2, Picks, -1), "family -1 refused");
	T.AssertFalse(HOLTalent::CanPick(2, Picks, 3), "family 3 refused: a row has 3 talents");
	T.AssertTrue(HOLTalent::Pick(2, Picks, 2), "level 2: a Champion talent in row 2");
	T.AssertEquals(1, Picks.Num(), "one taken");
	T.AssertFalse(HOLTalent::CanPick(2, Picks, 0), "level 2, the point is spent: row 3 is not open");
	T.AssertTrue(HOLTalent::Pick(3, Picks, 0), "level 3: row 3");
	T.AssertEquals(2, Picks.Num(), "two taken");
	T.AssertEquals(2, Picks[0], "row 2's talent kept");
	T.AssertEquals(0, Picks[1], "row 3's talent");
}

// A kept point is spent when the player wants, rows in order (D105): at level 6 with nothing taken, five picks in a row.
void Test_TalentReserveSpentLater(FUnitTest& T)
{
	TArray<int> Picks;
	for (int Row = 2; Row <= 6; Row++)
		T.AssertTrue(HOLTalent::Pick(6, Picks, 1), "kept point spent, row " + Row);
	T.AssertFalse(HOLTalent::CanPick(6, Picks, 1), "the 6th row needs level 7");
	T.AssertEquals(5, Picks.Num(), "5 taken at level 6");
	T.AssertEquals(0, HOLTalent::PointsAvailable(6, Picks), "none left");
}

// No more than 8 talents, whatever the level.
void Test_TalentNoNinthRow(FUnitTest& T)
{
	TArray<int> Picks = TalentPicks(0, 0, 0, 0, 0, 0, 0, 0);
	T.AssertFalse(HOLTalent::CanPick(10, Picks, 0), "8 rows filled: no more at level 10");
}

// The seal at 3 talents of one family (D103), not before; each family counted apart.
void Test_TalentSeal(FUnitTest& T)
{
	T.AssertFalse(HOLTalent::HasSeal(TalentPicks(0, 0), 0), "2 of a family: no seal");
	T.AssertTrue(HOLTalent::HasSeal(TalentPicks(0, 0, 0), 0), "3 of a family: the seal");
	T.AssertFalse(HOLTalent::HasSeal(TalentPicks(0, 0, 0), 1), "the other families have none");
	T.AssertTrue(HOLTalent::HasSeal(TalentPicks(1, 0, 1, 2, 1), 1), "3 of a family taken apart still count");
	T.AssertEquals(3, HOLTalent::CountOf(TalentPicks(1, 0, 1, 2, 1), 1), "count of the Captain's");
	T.AssertEquals(1, HOLTalent::CountOf(TalentPicks(1, 0, 1, 2, 1), 2), "count of the Champion's");
	T.AssertEquals(3, HOLTalent::FamilyCount, "a row offers 3 talents, one per family (D103)");
}

// The key talent at 5 talents of one family (D103), not before.
void Test_TalentKey(FUnitTest& T)
{
	T.AssertFalse(HOLTalent::HasKey(TalentPicks(2, 2, 2, 2), 2), "4: no key");
	T.AssertTrue(HOLTalent::HasKey(TalentPicks(2, 2, 2, 2, 2), 2), "5: the key");
	T.AssertTrue(HOLTalent::HasSeal(TalentPicks(2, 2, 2, 2, 2), 2), "the seal stays with the key");
	T.AssertFalse(HOLTalent::HasKey(TalentPicks(2, 2, 2, 2, 2), 0), "another family: none");
}

// At 8 points the maximum is one key and one seal (5 + 3) or two seals and two lone talents (D103).
void Test_TalentMaximumAtEightPoints(FUnitTest& T)
{
	const TArray<int> KeyAndSeal = TalentPicks(0, 0, 1, 0, 1, 0, 1, 0);
	T.AssertTrue(HOLTalent::HasKey(KeyAndSeal, 0), "a key");
	T.AssertTrue(HOLTalent::HasSeal(KeyAndSeal, 1), "and a seal");
	T.AssertFalse(HOLTalent::HasKey(KeyAndSeal, 1), "but no second key");
	const TArray<int> TwoSeals = TalentPicks(0, 0, 0, 1, 1, 1, 2, 2);
	T.AssertTrue(HOLTalent::HasSeal(TwoSeals, 0) && HOLTalent::HasSeal(TwoSeals, 1), "two seals");
	T.AssertFalse(HOLTalent::HasSeal(TwoSeals, 2), "the third family stays at 2");
	T.AssertEquals(2, HOLTalent::SealCount(TwoSeals), "SealCount: 2");
	T.AssertEquals(0, HOLTalent::KeyCount(TwoSeals), "KeyCount: 0");
	T.AssertEquals(1, HOLTalent::KeyCount(KeyAndSeal), "KeyCount: 1");
}

// Prototype (D103): at 5 points the key needs one family from row 2 to row 6, no stray pick allowed.
void Test_TalentKeyInThePrototype(FUnitTest& T)
{
	const int MaxLevel = HOLHero::GetMaxLevel();
	TArray<int> Picks;
	T.AssertTrue(HOLTalent::CanStillReachKey(Picks, MaxLevel, 0), "nothing taken: the key is reachable");
	Picks = TalentPicks(0, 0, 0, 0);
	T.AssertTrue(HOLTalent::CanStillReachKey(Picks, MaxLevel, 0), "4 of a family, one point left: reachable");
	Picks = TalentPicks(0, 1);
	T.AssertFalse(HOLTalent::CanStillReachKey(Picks, MaxLevel, 0), "a stray pick in row 3: the key is out of reach");
	T.AssertFalse(HOLTalent::CanStillReachKey(Picks, MaxLevel, 1), "nor for the other family (row 2 was not its)");
	T.AssertTrue(HOLTalent::CanStillReachKey(Picks, 9, 0), "with the 8 points of the whole game it still is");
}

// The advised choice (D105): the family most filled; a tie goes to the family of the latest talent (TQ1, provisional);
// none before any talent.
void Test_TalentAdvisedFamily(FUnitTest& T)
{
	T.AssertEquals(HOLTalent::FamilyNone, HOLTalent::AdvisedFamily(TalentPicks(-1)), "no talent taken: no advice");
	T.AssertEquals(2, HOLTalent::AdvisedFamily(TalentPicks(2)), "one taken: its family");
	T.AssertEquals(0, HOLTalent::AdvisedFamily(TalentPicks(0, 1, 0)), "the most filled");
	T.AssertEquals(1, HOLTalent::AdvisedFamily(TalentPicks(0, 0, 1, 1)), "a tie: the latest one's family");
	T.AssertEquals(0, HOLTalent::AdvisedFamily(TalentPicks(0, 1, 2, 0)), "0 leads over a tie of 1 and 2");
}
