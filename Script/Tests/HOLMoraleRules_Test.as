// Morale effects, pure rules (M1, HOLMoraleRules.as; MoraleInProgress.md D704-D709, GDD 7.2, D74, D125, D404). Percents are signed
// "benefit" values: a bonus is positive on every lever (on the damage taken, +25 means 25 % less taken), a malus negative. Every
// expectation is read from the rules' own caps and constants where it concerns a cap; the caps' starting values have a test of their own.

const int MoraleStepMs = 100;

// A temporary bonus on one lever (benefit percent).
HOLMorale::FHOLMoraleEffect MoraleTemp(int Id, int Lever, int Percent, int DurationMs)
{
	return HOLMorale::MakeEffect(Id, Lever == HOLMorale::LeverDealt ? Percent : 0, Lever == HOLMorale::LeverTaken ? Percent : 0, Lever == HOLMorale::LeverRate ? Percent : 0, DurationMs, false);
}

// An aura effect on one lever (it lasts while the unit is in the aura).
HOLMorale::FHOLMoraleEffect MoraleAura(int Id, int Lever, int Percent)
{
	return HOLMorale::MakeEffect(Id, Lever == HOLMorale::LeverDealt ? Percent : 0, Lever == HOLMorale::LeverTaken ? Percent : 0, Lever == HOLMorale::LeverRate ? Percent : 0, 0, true);
}

// The caps are the starting values of D704: 50 % on the damage taken, 30 % on the damage dealt and on the attack rate; 75 % of attenuation (D706).
void Test_MoraleStartingCaps(FUnitTest& T)
{
	T.AssertEquals(50, HOLMorale::BonusCap(HOLMorale::LeverTaken), "damage taken, bonus cap");
	T.AssertEquals(40, HOLMorale::BonusCap(HOLMorale::LeverDealt), "damage dealt, bonus cap (40 since Q-legions-rage-cap = A, 2026-11-11)");
	T.AssertEquals(30, HOLMorale::BonusCap(HOLMorale::LeverRate), "attack rate, bonus cap");
	T.AssertEquals(30, HOLMorale::MalusCap(HOLMorale::LeverDealt), "damage dealt, malus cap (stays 30: only the bonus rises to 40, D791)");
	T.AssertEquals(30, HOLMorale::MalusCap(HOLMorale::LeverRate), "attack rate, malus cap");
	T.AssertEquals(50, HOLMorale::MalusCap(HOLMorale::LeverTaken), "damage taken, malus cap (symmetric, decided by the user on 2026-11-05)");
	T.AssertEquals(75, HOLMorale::AttenuationCap, "attenuation never reaches 100 %");
}

// Q-legions-rage-cap = A (2026-11-11): the necrotic rage of the effects table gives its whole +40 % of damage dealt, not +30 %; with
// Triumph on top it stays at 40, never more; its +25 % of rate is unchanged.
void Test_MoraleNecroticRageGivesFullForty(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::NecroticRage));
	T.AssertEquals(40, HOLMorale::Total(State, HOLMorale::LeverDealt), "the rage alone: +40 % of damage dealt");
	T.AssertEquals(25, HOLMorale::Total(State, HOLMorale::LeverRate), "the rage alone: +25 % of rate");
	HOLMorale::Apply(State, HOLMoraleTable::Effect(HOLMoraleTable::Triumph));
	T.AssertEquals(40, HOLMorale::Total(State, HOLMorale::LeverDealt), "rage and Triumph: still 40, the cap");
}

// D791 (2026-11-12, "bonus only"): the malus of damage dealt stays capped at 30 %. Two maluses of -20 on the damage dealt leave the unit
// dealing 70 % of its damage (not 60 %), while two bonuses of +25 let it deal 140 %.
void Test_MoraleDealtMalusCapStaysThirty(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState Malus;
	HOLMorale::Apply(Malus, MoraleTemp(1, HOLMorale::LeverDealt, -20, 15000));
	HOLMorale::Apply(Malus, MoraleTemp(2, HOLMorale::LeverDealt, -20, 15000));
	T.AssertEquals(-30, HOLMorale::Total(Malus, HOLMorale::LeverDealt), "-20 and -20 on the damage dealt are cut at the malus cap of 30");
	T.AssertEquals(70, HOLMorale::Multiplier(Malus, HOLMorale::LeverDealt), "the unit still deals 70 % of its damage");
	HOLMorale::FHOLMoraleState Bonus;
	HOLMorale::Apply(Bonus, MoraleTemp(1, HOLMorale::LeverDealt, 25, 15000));
	HOLMorale::Apply(Bonus, MoraleTemp(2, HOLMorale::LeverDealt, 25, 15000));
	T.AssertEquals(140, HOLMorale::Multiplier(Bonus, HOLMorale::LeverDealt), "+25 and +25 on the damage dealt are cut at the bonus cap of 40");
}

// The same effect posed twice stays one; it keeps the strongest intensity and the longest time left.
void Test_MoraleSameEffectStaysOne(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	T.AssertEquals(HOLMorale::ApplyPosed, HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverTaken, 15, 20000)), "first pose");
	T.AssertEquals(HOLMorale::ApplyRefreshed, HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverTaken, 25, 5000)), "second pose, stronger but shorter");
	T.AssertEquals(1, HOLMorale::Count(State), "still one effect");
	T.AssertEquals(25, HOLMorale::Total(State, HOLMorale::LeverTaken), "the strongest intensity");
	T.AssertEquals(20000, State.Effects[0].DurationMs, "the longest time left");
	HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverTaken, 10, 30000));
	T.AssertEquals(25, HOLMorale::Total(State, HOLMorale::LeverTaken), "a weaker pose does not lower it");
	T.AssertEquals(30000, State.Effects[0].DurationMs, "a longer pose refreshes the time");
	T.AssertEquals(1, HOLMorale::Count(State), "and still one");
}

// Different effects add up on a lever, then the lever's cap bounds the sum.
void Test_MoraleSumsPerLeverUnderCap(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, MoraleAura(1, HOLMorale::LeverTaken, 15));
	T.AssertEquals(15, HOLMorale::Total(State, HOLMorale::LeverTaken), "the aura alone");
	HOLMorale::Apply(State, MoraleTemp(2, HOLMorale::LeverTaken, 25, 15000));
	T.AssertEquals(40, HOLMorale::Total(State, HOLMorale::LeverTaken), "aura and cry add up");
	HOLMorale::Apply(State, MoraleTemp(3, HOLMorale::LeverTaken, 25, 15000));
	T.AssertEquals(HOLMorale::BonusCap(HOLMorale::LeverTaken), HOLMorale::Total(State, HOLMorale::LeverTaken), "the third is cut at the cap");
	HOLMorale::FHOLMoraleState Dealt;
	HOLMorale::Apply(Dealt, MoraleTemp(1, HOLMorale::LeverDealt, 15, 15000));
	HOLMorale::Apply(Dealt, MoraleTemp(2, HOLMorale::LeverDealt, 25, 15000));
	T.AssertEquals(HOLMorale::BonusCap(HOLMorale::LeverDealt), HOLMorale::Total(Dealt, HOLMorale::LeverDealt), "15 + 25 on the damage dealt is cut at its own cap");
}

// A bonus on one lever and a malus on another never offset each other.
void Test_MoraleLeversDoNotOffset(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverDealt, 10, 15000));
	HOLMorale::Apply(State, MoraleTemp(2, HOLMorale::LeverTaken, -10, 15000));
	T.AssertEquals(10, HOLMorale::Total(State, HOLMorale::LeverDealt), "the damage dealt keeps its bonus");
	T.AssertEquals(-10, HOLMorale::Total(State, HOLMorale::LeverTaken), "the damage taken keeps its malus");
	T.AssertEquals(0, HOLMorale::Total(State, HOLMorale::LeverRate), "the rate is untouched");
}

// On one lever the bonus and the malus each stay under their own cap, then they offset: the cap is not put on the net.
void Test_MoraleBonusAndMalusHaveOwnCaps(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState Same;
	HOLMorale::Apply(Same, MoraleTemp(1, HOLMorale::LeverDealt, 10, 15000));
	HOLMorale::Apply(Same, MoraleTemp(2, HOLMorale::LeverDealt, -10, 15000));
	T.AssertEquals(0, HOLMorale::Total(Same, HOLMorale::LeverDealt), "+10 and -10 on the same lever offset to nothing");

	const int BonusCap = HOLMorale::BonusCap(HOLMorale::LeverDealt);
	const int MalusCap = HOLMorale::MalusCap(HOLMorale::LeverDealt);
	HOLMorale::FHOLMoraleState Big;
	HOLMorale::Apply(Big, MoraleTemp(1, HOLMorale::LeverDealt, BonusCap + 20, 15000));
	HOLMorale::Apply(Big, MoraleTemp(2, HOLMorale::LeverDealt, -(MalusCap + 20), 15000));
	T.AssertEquals(BonusCap - MalusCap, HOLMorale::Total(Big, HOLMorale::LeverDealt), "the bonus and the malus are each cut at their cap, then added");
	HOLMorale::Apply(Big, MoraleTemp(3, HOLMorale::LeverDealt, -10, 15000));
	T.AssertEquals(BonusCap - MalusCap, HOLMorale::Total(Big, HOLMorale::LeverDealt), "a malus beyond its cap adds nothing");
}

// What combat reads: the percent of damage dealt, of damage taken, of attack rate.
void Test_MoraleMultipliers(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	T.AssertEquals(100, HOLMorale::Multiplier(State, HOLMorale::LeverDealt), "no effect: 100 % dealt");
	T.AssertEquals(100, HOLMorale::Multiplier(State, HOLMorale::LeverTaken), "no effect: 100 % taken");
	T.AssertEquals(100, HOLMorale::Multiplier(State, HOLMorale::LeverRate), "no effect: 100 % rate");
	HOLMorale::Apply(State, MoraleAura(1, HOLMorale::LeverTaken, 15));
	HOLMorale::Apply(State, MoraleTemp(2, HOLMorale::LeverTaken, 25, 15000));
	T.AssertEquals(60, HOLMorale::Multiplier(State, HOLMorale::LeverTaken), "aura 15 and cry 25: 60 % of the damage taken");
	HOLMorale::Apply(State, MoraleTemp(3, HOLMorale::LeverDealt, 20, 15000));
	HOLMorale::Apply(State, MoraleTemp(4, HOLMorale::LeverRate, -10, 15000));
	T.AssertEquals(120, HOLMorale::Multiplier(State, HOLMorale::LeverDealt), "+20 dealt: 120 %");
	T.AssertEquals(90, HOLMorale::Multiplier(State, HOLMorale::LeverRate), "-10 rate: 90 %");
	HOLMorale::FHOLMoraleState Weak;
	HOLMorale::Apply(Weak, MoraleTemp(1, HOLMorale::LeverTaken, -20, 15000));
	T.AssertEquals(120, HOLMorale::Multiplier(Weak, HOLMorale::LeverTaken), "a malus on the damage taken: 120 % taken");
}

// A malus is attenuated: a temporary one in its time, an aura one in its intensity, by the same fraction. A bonus is never attenuated.
void Test_MoraleAttenuationHalves(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState Temp;
	HOLMorale::Apply(Temp, MoraleTemp(1, HOLMorale::LeverDealt, -20, 20000), 50);
	T.AssertEquals(10000, Temp.Effects[0].DurationMs, "half the attenuation: 20 s become 10 s");
	T.AssertEquals(-20, HOLMorale::Total(Temp, HOLMorale::LeverDealt), "the intensity of a temporary malus is kept");

	HOLMorale::FHOLMoraleState Aura;
	HOLMorale::Apply(Aura, MoraleAura(1, HOLMorale::LeverDealt, -20), 50);
	T.AssertEquals(-10, HOLMorale::Total(Aura, HOLMorale::LeverDealt), "an aura malus of -20 becomes -10");
	T.AssertTrue(Aura.Effects[0].bAura, "still an aura");

	HOLMorale::FHOLMoraleState Bonus;
	HOLMorale::Apply(Bonus, MoraleTemp(1, HOLMorale::LeverDealt, 20, 20000), 50);
	T.AssertEquals(20000, Bonus.Effects[0].DurationMs, "a bonus keeps its time");
	T.AssertEquals(20, HOLMorale::Total(Bonus, HOLMorale::LeverDealt), "a bonus keeps its intensity");
	HOLMorale::Apply(Bonus, MoraleAura(2, HOLMorale::LeverTaken, 15), 50);
	T.AssertEquals(15, HOLMorale::Total(Bonus, HOLMorale::LeverTaken), "an aura bonus keeps its intensity");

	HOLMorale::FHOLMoraleState None;
	HOLMorale::Apply(None, MoraleTemp(1, HOLMorale::LeverDealt, -20, 20000), 0);
	T.AssertEquals(20000, None.Effects[0].DurationMs, "no attenuation: no change");
}

// Several sources of attenuation add up, never beyond 75 %; a source of the same nature does not add to a stronger one (D706).
void Test_MoraleAttenuationSources(FUnitTest& T)
{
	TArray<int> Natures;
	TArray<int> Percents;
	T.AssertEquals(0, HOLMorale::CombineAttenuation(Natures, Percents), "no source: 0");

	Natures.Add(0);
	Percents.Add(50);
	T.AssertEquals(50, HOLMorale::CombineAttenuation(Natures, Percents), "one source of half");

	Natures.Add(1);
	Percents.Add(50);
	T.AssertEquals(HOLMorale::AttenuationCap, HOLMorale::CombineAttenuation(Natures, Percents), "two sources of half give the cap, not 100 %");

	TArray<int> SameNatures;
	TArray<int> SamePercents;
	SameNatures.Add(0);
	SamePercents.Add(50);
	SameNatures.Add(0);
	SamePercents.Add(30);
	T.AssertEquals(50, HOLMorale::CombineAttenuation(SameNatures, SamePercents), "two of the same nature: the strongest counts alone");

	TArray<int> ThreeNatures;
	TArray<int> ThreePercents;
	ThreeNatures.Add(0);
	ThreePercents.Add(20);
	ThreeNatures.Add(1);
	ThreePercents.Add(30);
	T.AssertEquals(50, HOLMorale::CombineAttenuation(ThreeNatures, ThreePercents), "two natures under the cap add up");

	TArray<int> Odd;
	TArray<int> OddPercents;
	Odd.Add(0);
	OddPercents.Add(-20);
	T.AssertEquals(0, HOLMorale::CombineAttenuation(Odd, OddPercents), "a negative source is ignored");
	Odd.Add(1);
	T.AssertEquals(0, HOLMorale::CombineAttenuation(Odd, OddPercents), "arrays of unequal length: the extra nature is ignored");
}

// Immunity: a malus is not posed while it lasts, a bonus is; it runs out on the fixed step.
void Test_MoraleImmunityBlocksMalusOnly(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::GrantImmunity(State, 5000);
	T.AssertEquals(5000, State.ImmunityMs, "the immunity is set");
	T.AssertEquals(HOLMorale::ApplyBlockedByImmunity, HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverDealt, -10, 15000)), "a temporary malus is refused");
	T.AssertEquals(HOLMorale::ApplyBlockedByImmunity, HOLMorale::Apply(State, MoraleAura(2, HOLMorale::LeverTaken, -15)), "an aura malus is refused");
	T.AssertEquals(0, HOLMorale::Count(State), "nothing was posed");
	T.AssertEquals(HOLMorale::ApplyPosed, HOLMorale::Apply(State, MoraleTemp(3, HOLMorale::LeverDealt, 10, 15000)), "a bonus is posed");
	T.AssertEquals(1, HOLMorale::Count(State), "the bonus is there");

	HOLMorale::Tick(State, 4900);
	T.AssertEquals(HOLMorale::ApplyBlockedByImmunity, HOLMorale::Apply(State, MoraleTemp(4, HOLMorale::LeverDealt, -10, 15000)), "still immune 100 ms before the end");
	HOLMorale::Tick(State, 100);
	T.AssertEquals(0, State.ImmunityMs, "the immunity has run out");
	T.AssertEquals(HOLMorale::ApplyPosed, HOLMorale::Apply(State, MoraleTemp(4, HOLMorale::LeverDealt, -10, 15000)), "the malus is posed again");
	HOLMorale::Tick(State, 10000);
	T.AssertEquals(0, State.ImmunityMs, "the immunity does not go below 0");

	HOLMorale::FHOLMoraleState Long;
	HOLMorale::GrantImmunity(Long, 5000);
	HOLMorale::GrantImmunity(Long, 2000);
	T.AssertEquals(5000, Long.ImmunityMs, "a shorter grant does not cut the immunity");
	HOLMorale::GrantImmunity(Long, 8000);
	T.AssertEquals(8000, Long.ImmunityMs, "a longer grant extends it");
}

// The removal of the malus (Lumiere sacree): the malus go, temporary or aura, the bonus stay.
void Test_MoraleRemovalLeavesBonus(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverDealt, -10, 15000));
	HOLMorale::Apply(State, MoraleAura(2, HOLMorale::LeverTaken, -15));
	HOLMorale::Apply(State, MoraleTemp(3, HOLMorale::LeverTaken, 25, 15000));
	HOLMorale::Apply(State, MoraleAura(4, HOLMorale::LeverRate, 10));
	T.AssertEquals(4, HOLMorale::Count(State), "four effects");
	T.AssertEquals(2, HOLMorale::RemoveMalus(State), "two malus removed");
	T.AssertEquals(2, HOLMorale::Count(State), "two bonus left");
	T.AssertEquals(25, HOLMorale::Total(State, HOLMorale::LeverTaken), "the bonus on the damage taken is intact");
	T.AssertEquals(10, HOLMorale::Total(State, HOLMorale::LeverRate), "the aura bonus is intact");
	T.AssertEquals(0, HOLMorale::Total(State, HOLMorale::LeverDealt), "the malus is gone");
	T.AssertEquals(0, HOLMorale::RemoveMalus(State), "nothing more to remove");
	T.AssertEquals(0, State.ImmunityMs, "the removal alone gives no immunity (the caller grants it)");
}

// The time runs down on the fixed step; a temporary effect ends at 0, an aura does not run down.
void Test_MoraleTickCountsDown(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverTaken, 25, 15000));
	HOLMorale::Apply(State, MoraleAura(2, HOLMorale::LeverTaken, 15));
	for (int Elapsed = 0; Elapsed < 15000 - MoraleStepMs; Elapsed += MoraleStepMs)
	{
		HOLMorale::Tick(State, MoraleStepMs);
	}
	T.AssertEquals(MoraleStepMs, State.Effects[0].DurationMs, "one step before the end: one step left");
	T.AssertEquals(40, HOLMorale::Total(State, HOLMorale::LeverTaken), "both still count");
	HOLMorale::Tick(State, MoraleStepMs);
	T.AssertEquals(1, HOLMorale::Count(State), "the cry has ended, the aura has not");
	T.AssertEquals(15, HOLMorale::Total(State, HOLMorale::LeverTaken), "only the aura counts");
	HOLMorale::Tick(State, 100000);
	T.AssertEquals(1, HOLMorale::Count(State), "an aura does not run down");
}

// An aura stops as soon as the unit leaves it: the engine clears the auras each step and poses those still in range.
void Test_MoraleClearAurasKeepsTemporary(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverTaken, 25, 15000));
	HOLMorale::Apply(State, MoraleAura(2, HOLMorale::LeverTaken, 15));
	HOLMorale::Apply(State, MoraleAura(3, HOLMorale::LeverDealt, -10));
	HOLMorale::ClearAuras(State);
	T.AssertEquals(1, HOLMorale::Count(State), "only the cry stays");
	T.AssertEquals(25, HOLMorale::Total(State, HOLMorale::LeverTaken), "the aura is gone at once");
	T.AssertEquals(0, HOLMorale::Total(State, HOLMorale::LeverDealt), "the aura malus too");
	T.AssertTrue(HOLMorale::Has(State, 1), "the cry is there");
	T.AssertFalse(HOLMorale::Has(State, 2), "the aura is not");
}

// A talent or a technology scales one owner's intensity of an effect (Chuchoteurs: the malus +10 %); halves round away from zero.
void Test_MoraleOwnerIntensity(FUnitTest& T)
{
	T.AssertEquals(-22, HOLMorale::Scale(-20, 110), "a malus of -20 with +10 %");
	T.AssertEquals(17, HOLMorale::Scale(15, 110), "16.5 rounds to 17");
	T.AssertEquals(-17, HOLMorale::Scale(-15, 110), "-16.5 rounds to -17");
	T.AssertEquals(8, HOLMorale::Scale(15, 50), "7.5 rounds to 8");
	T.AssertEquals(0, HOLMorale::Scale(15, 0), "scaled to nothing");
	HOLMorale::FHOLMoraleEffect Base = MoraleTemp(1, HOLMorale::LeverDealt, -20, 15000);
	HOLMorale::FHOLMoraleEffect Scaled = HOLMorale::WithIntensity(Base, 110);
	T.AssertEquals(-22, Scaled.Dealt, "the intensity is scaled");
	T.AssertEquals(15000, Scaled.DurationMs, "the time is not");
	T.AssertEquals(-20, Base.Dealt, "the original is untouched");
	HOLMorale::FHOLMoraleState State;
	HOLMorale::Apply(State, Scaled);
	T.AssertEquals(-22, HOLMorale::Total(State, HOLMorale::LeverDealt), "posed at the scaled intensity");
}

// An effect with a malus on any lever is a malus; one with none is a bonus.
void Test_MoraleSignOfAnEffect(FUnitTest& T)
{
	T.AssertTrue(HOLMorale::IsMalus(MoraleTemp(1, HOLMorale::LeverRate, -5, 1000)), "negative: malus");
	T.AssertFalse(HOLMorale::IsMalus(MoraleTemp(1, HOLMorale::LeverRate, 5, 1000)), "positive: bonus");
	HOLMorale::FHOLMoraleEffect Triumph = HOLMorale::MakeEffect(7, 10, 10, 0, 40000, false);
	HOLMorale::FHOLMoraleEffect Demoralised = HOLMorale::MakeEffect(8, -10, -10, 0, 25000, false);
	T.AssertFalse(HOLMorale::IsMalus(Triumph), "a duel's Triumph is a bonus");
	T.AssertTrue(HOLMorale::IsMalus(Demoralised), "a duel's Demoralised is a malus");
}

// A temporary effect attenuated down to no time at all is not posed.
void Test_MoraleNothingToPose(FUnitTest& T)
{
	HOLMorale::FHOLMoraleState State;
	T.AssertEquals(HOLMorale::ApplyNothing, HOLMorale::Apply(State, MoraleTemp(1, HOLMorale::LeverDealt, -10, 0)), "a temporary effect with no time");
	T.AssertEquals(HOLMorale::ApplyNothing, HOLMorale::Apply(State, HOLMorale::MakeEffect(2, 0, 0, 0, 5000, false)), "an effect with no intensity");
	T.AssertEquals(0, HOLMorale::Count(State), "nothing posed");
	T.AssertEquals(HOLMorale::ApplyNothing, HOLMorale::Apply(State, MoraleTemp(3, HOLMorale::LeverDealt, -10, 1), 75), "1 ms attenuated by 75 % rounds to nothing");
}
