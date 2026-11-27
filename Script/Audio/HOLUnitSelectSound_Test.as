// SAURON-1127ar1: the pure rules of the unit selection sound (Script\Audio\HOLUnitSelectSound.as).

TArray<int> UnitSelectTestIds(int A, int B, int C)
{
	TArray<int> Out;
	if (A > 0)
		Out.Add(A);
	if (B > 0)
		Out.Add(B);
	if (C > 0)
		Out.Add(C);
	return Out;
}

// A new set of units rings; the same set reaffirmed (in any order), a set that only lost units, or nothing selected stays silent.
void Test_UnitSelectSoundOnlyOnNewSelection(FUnitTest& T)
{
	const TArray<int> None;
	const bool bFirst = HOLUnitSelectSound::IsNewSelection(None, UnitSelectTestIds(7, 0, 0));
	T.AssertTrue(bFirst, "a first unit selected rings");
	const bool bOther = HOLUnitSelectSound::IsNewSelection(UnitSelectTestIds(7, 0, 0), UnitSelectTestIds(9, 0, 0));
	T.AssertTrue(bOther, "another unit selected rings");
	const bool bAdded = HOLUnitSelectSound::IsNewSelection(UnitSelectTestIds(7, 0, 0), UnitSelectTestIds(7, 9, 0));
	T.AssertTrue(bAdded, "a unit added to the selection rings");
	const bool bSame = HOLUnitSelectSound::IsNewSelection(UnitSelectTestIds(7, 9, 0), UnitSelectTestIds(9, 7, 0));
	T.AssertFalse(bSame, "the same units reaffirmed (another order) stay silent");
	const bool bLost = HOLUnitSelectSound::IsNewSelection(UnitSelectTestIds(7, 9, 11), UnitSelectTestIds(7, 11, 0));
	T.AssertFalse(bLost, "a selection that only lost a unit (a death) stays silent");
	const bool bEmpty = HOLUnitSelectSound::IsNewSelection(UnitSelectTestIds(7, 0, 0), None);
	T.AssertFalse(bEmpty, "nothing selected stays silent");
	const bool bAgain = HOLUnitSelectSound::IsNewSelection(None, UnitSelectTestIds(7, 0, 0));
	T.AssertTrue(bAgain, "the same unit selected again after nothing rings");
}

// The type that rings: the most numerous of a mixed selection, the first met on a tie, None for no unit.
void Test_UnitSelectSoundingType(FUnitTest& T)
{
	TArray<FName> Mixed;
	Mixed.Add(n"Archer");
	Mixed.Add(n"ManAtArms");
	Mixed.Add(n"ManAtArms");
	Mixed.Add(n"Archer");
	Mixed.Add(n"ManAtArms");
	const FName Most = HOLUnitSelectSound::SoundingType(Mixed);
	T.AssertTrue(Most == n"ManAtArms", f"the most numerous type rings (got {Most})");
	TArray<FName> Tie;
	Tie.Add(n"Spearman");
	Tie.Add(n"Archer");
	Tie.Add(n"Archer");
	Tie.Add(n"Spearman");
	const FName First = HOLUnitSelectSound::SoundingType(Tie);
	T.AssertTrue(First == n"Spearman", f"on a tie the first met rings (got {First})");
	TArray<FName> One;
	One.Add(n"Peasant");
	const FName Alone = HOLUnitSelectSound::SoundingType(One);
	T.AssertTrue(Alone == n"Peasant", f"one unit rings its type (got {Alone})");
	const TArray<FName> Empty;
	const FName Nothing = HOLUnitSelectSound::SoundingType(Empty);
	T.AssertTrue(Nothing == NAME_None, f"no unit, no type (got {Nothing})");
}
