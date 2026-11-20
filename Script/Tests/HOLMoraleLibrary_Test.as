// The bridge of the C++ morale rule (UHOLMoraleLibrary, PLAN-MORAL-1120 step 2; rule: Simulation/HOLMoraleRules.h). Every value asserted is computed by
// the rule through the bridge, read from the caps and the table, never retyped. Until the body of the library exists these tests are red (the stub
// returns 0 and changes nothing).

const int MoraleLibLeverDealt = 0;
const int MoraleLibLeverTaken = 1;

// The same effect posed twice is one effect (the second pose refreshes it), the lever reads the effect, the time then removes it.
void Test_MoraleLibraryApplyRefreshTotalTick(FUnitTest& T)
{
	FHOLMoraleStateData State;
	const FHOLMoraleEffectData Cry = HOLMorale::NativeMakeEffect(1, 0, 25, 0, 15000, false);
	const int First = HOLMorale::NativeApply(State, Cry, 0);
	T.AssertEquals(1, First, "the first pose is a pose");
	const int Second = HOLMorale::NativeApply(State, Cry, 0);
	T.AssertEquals(2, Second, "the second pose refreshes, it does not add");
	T.AssertEquals(1, State.Effects.Num(), "still one effect");
	const int Taken = HOLMorale::NativeTotal(State, MoraleLibLeverTaken);
	T.AssertEquals(25, Taken, "one effect of 25 on the damage taken, not 50");
	HOLMorale::NativeTick(State, 15000);
	const int After = HOLMorale::NativeTotal(State, MoraleLibLeverTaken);
	T.AssertEquals(0, After, "the time ran out: the total falls back to 0");
	T.AssertEquals(0, State.Effects.Num(), "and the effect is gone");
}

// The cap is read from the rule: two bonuses on the damage dealt stop at the bonus cap; the malus of the immunity is refused, the bonus is not.
void Test_MoraleLibraryCapsAndImmunity(FUnitTest& T)
{
	FHOLMoraleStateData State;
	HOLMorale::NativeApply(State, HOLMorale::NativeMakeEffect(1, 30, 0, 0, 10000, false), 0);
	HOLMorale::NativeApply(State, HOLMorale::NativeMakeEffect(2, 30, 0, 0, 10000, false), 0);
	const int Cap = HOLMorale::NativeBonusCap(MoraleLibLeverDealt);
	const int Dealt = HOLMorale::NativeTotal(State, MoraleLibLeverDealt);
	T.AssertEquals(Cap, Dealt, "60 of bonus is cut at the bonus cap");
	HOLMorale::NativeGrantImmunity(State, 5000);
	const int Blocked = HOLMorale::NativeApply(State, HOLMorale::NativeMakeEffect(3, -20, 0, 0, 10000, false), 0);
	T.AssertEquals(3, Blocked, "a malus is refused during the immunity");
	const int Posed = HOLMorale::NativeApply(State, HOLMorale::NativeMakeEffect(4, 0, 0, 10, 10000, false), 0);
	T.AssertEquals(1, Posed, "a bonus is not");
	const int Removed = HOLMorale::NativeRemoveMalus(State);
	T.AssertEquals(0, Removed, "no malus to remove");
}

// The C++ rows of the table (the rule's own, HOLMoraleTableRows.h) give the effect the script table gives, row by row.
void Test_MoraleTableCppRowsAgreeWithCsv(FUnitTest& T)
{
	for (int Id = 0; Id < HOLMoraleTable::Count; Id++)
	{
		const FHOLMoraleEffectData Native = HOLMorale::NativeMakeEffectFromRow(Id);
		const FHOLMoraleEffectData Script = HOLMoraleTable::Effect(Id);
		T.AssertEquals(Script.Id, Native.Id, "row " + Id + ": the effect id");
		T.AssertEquals(Script.Dealt, Native.Dealt, "row " + Id + ": damage dealt");
		T.AssertEquals(Script.Taken, Native.Taken, "row " + Id + ": damage taken");
		T.AssertEquals(Script.Rate, Native.Rate, "row " + Id + ": rate");
		T.AssertEquals(Script.DurationMs, Native.DurationMs, "row " + Id + ": duration");
		T.AssertEquals(Script.bAura, Native.bAura, "row " + Id + ": aura");
	}
}
