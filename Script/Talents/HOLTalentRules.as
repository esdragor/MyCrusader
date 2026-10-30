// The hero's talent tree, pure rules (T1; GDD 9.6, D103-D105, D194): integers only, no world access, tested in HOLTalentRules_Test.as.
// One point of talent a hero level from 2 to 9 (8 a game, D04; none at level 1 nor at the ultimate's level 10). A row a level (2 to 9),
// the player takes ONE talent of the row's three, one per family (the column of the row: for the Dawn Order the Oaths, 0 Guardian,
// 1 Captain, 2 Champion, D106). Rows fill in order and a point may be kept in reserve, spent when the player wants (D105). Three
// talents of a family give its seal, five its key talent (D103). The picks are a list of families, the row 2 first: the row of the
// n-th pick is 2 + n, so the list says everything about what the hero took.
// The Templars' four families (D104) are for after the prototype: the number of families is a constant of the rules.
namespace HOLTalent
{
	// Rows: one a level from 2 to 9.
	const int FirstRow = 2;
	const int LastRow = 9;
	const int RowCount = LastRow - FirstRow + 1;

	// A row's talents, one per family (3; the Templars 4, D104).
	const int FamilyCount = 3;

	// Talents of one family for its seal and for its key talent (D103).
	const int SealThreshold = 3;
	const int KeyThreshold = 5;

	// NextRow with no row left; AdvisedFamily with nothing to advise.
	const int RowNone = -1;
	const int FamilyNone = -1;

	// Points earned at this hero level: 1 from level 2 to level 9, 8 at the most.
	int PointsEarned(int Level)
	{
		return Math::Clamp(Level - FirstRow + 1, 0, RowCount);
	}

	// Points kept in reserve: earned minus talents taken.
	int PointsAvailable(int Level, const TArray<int>& Picks)
	{
		return Math::Max(0, PointsEarned(Level) - Picks.Num());
	}

	// The row the next talent goes to (rows fill in order), RowNone when the 8 are taken.
	int NextRow(const TArray<int>& Picks)
	{
		const int Row = FirstRow + Picks.Num();
		if (Row > LastRow)
			return RowNone;
		return Row;
	}

	// A talent of this family can be taken now: a family of the row, a row left and a point in reserve.
	bool CanPick(int Level, const TArray<int>& Picks, int Family)
	{
		if (Family < 0 || Family >= FamilyCount)
			return false;
		if (NextRow(Picks) == RowNone)
			return false;
		return PointsAvailable(Level, Picks) > 0;
	}

	// Takes the next row's talent of this family: false and nothing changed when CanPick refuses.
	bool Pick(int Level, TArray<int>& Picks, int Family)
	{
		if (!CanPick(Level, Picks, Family))
			return false;
		Picks.Add(Family);
		return true;
	}

	// Talents taken in this family.
	int CountOf(const TArray<int>& Picks, int Family)
	{
		int Count = 0;
		for (int Index = 0; Index < Picks.Num(); Index++)
		{
			if (Picks[Index] == Family)
				Count++;
		}
		return Count;
	}

	bool HasSeal(const TArray<int>& Picks, int Family)
	{
		return CountOf(Picks, Family) >= SealThreshold;
	}

	bool HasKey(const TArray<int>& Picks, int Family)
	{
		return CountOf(Picks, Family) >= KeyThreshold;
	}

	// Seals the hero has (shown to the adversary who scouts him, D103, D203).
	int SealCount(const TArray<int>& Picks)
	{
		int Count = 0;
		for (int Family = 0; Family < FamilyCount; Family++)
		{
			if (HasSeal(Picks, Family))
				Count++;
		}
		return Count;
	}

	// Key talents the hero has.
	int KeyCount(const TArray<int>& Picks)
	{
		int Count = 0;
		for (int Family = 0; Family < FamilyCount; Family++)
		{
			if (HasKey(Picks, Family))
				Count++;
		}
		return Count;
	}

	// Whether the family's key talent can still come, the hero reaching MaxLevel at the most: what it has plus the points he can still
	// earn. In the prototype (level 6, 5 points) only one family from row 2 to row 6 reaches it (D103).
	bool CanStillReachKey(const TArray<int>& Picks, int MaxLevel, int Family)
	{
		const int Left = Math::Max(0, PointsEarned(MaxLevel) - Picks.Num());
		return CountOf(Picks, Family) + Left >= KeyThreshold;
	}

	// The "advised choice" button (D105): the family most filled. A tie goes to the family of the latest talent among those tied
	// (TQ1, provisional); FamilyNone before any talent.
	int AdvisedFamily(const TArray<int>& Picks)
	{
		int Best = FamilyNone;
		int BestCount = 0;
		for (int Index = Picks.Num() - 1; Index >= 0; Index--)
		{
			const int Count = CountOf(Picks, Picks[Index]);
			if (Count > BestCount)
			{
				Best = Picks[Index];
				BestCount = Count;
			}
		}
		return Best;
	}
}
