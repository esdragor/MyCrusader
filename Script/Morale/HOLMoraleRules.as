// Morale effects, pure rules (M1; MoraleInProgress.md D704-D709, GDD 7.2, D74, D125, D404): integers only, no world access, tested in
// HOLMoraleRules_Test.as. A unit holds a few named effects, each temporary (a time left) or an aura (it lasts while the unit is in range:
// the engine clears the auras each step and poses those still in range). An effect acts on three levers in percent: the damage dealt, the
// damage taken, the attack rate (walking speed stays out of morale, D704). Percents are signed "benefit" values: positive helps the unit on
// every lever (on the damage taken, +25 means 25 % less taken), negative hurts it. An effect with a negative lever is a malus.
// Rules: the same effect is never added to itself (the strongest intensity and the longest time are kept); different effects add up per
// lever, the bonus and the malus each under their own cap, then offset (D704); the attenuation (D125, D706) shortens a temporary malus and
// weakens an aura malus by the same fraction, never a bonus; sources add up to 75 % at most, the strongest of one nature counting alone;
// the immunity (D404) stops a malus from being posed, never a bonus; the removal takes the malus and leaves the bonus.
// Caps (D704, to tune in test): 50 % on the damage taken (the existing MoraleReductionCapPercent), 30 % on the damage dealt and on the rate.
// The malus cap of the damage taken: 50 %, symmetric, decided by the user on 2026-11-05.
namespace HOLMorale
{
	// The three levers.
	const int LeverDealt = 0;
	const int LeverTaken = 1;
	const int LeverRate = 2;
	const int LeverCount = 3;

	// The most attenuation any sources can give together: never 100 % (only Lumiere sacree removes, D404, D706).
	const int AttenuationCap = 75;

	// What Apply did.
	const int ApplyNothing = 0;
	const int ApplyPosed = 1;
	const int ApplyRefreshed = 2;
	const int ApplyBlockedByImmunity = 3;

	// The cap of the bonus on a lever, in percent.
	int BonusCap(int Lever)
	{
		return Lever == LeverTaken ? 50 : 30;
	}

	// The cap of the malus on a lever, in percent (of the magnitude).
	int MalusCap(int Lever)
	{
		return Lever == LeverTaken ? 50 : 30;
	}

	// One effect on a unit. Id names it (a row of the effects table); Dealt / Taken / Rate are its benefit percents; DurationMs the time left of a
	// temporary effect (ignored for an aura).
	struct FHOLMoraleEffect
	{
		int Id = -1;
		int Dealt = 0;
		int Taken = 0;
		int Rate = 0;
		int DurationMs = 0;
		bool bAura = false;
	}

	// What a unit carries: its effects and the time left of its immunity to the malus.
	struct FHOLMoraleState
	{
		TArray<FHOLMoraleEffect> Effects;
		int ImmunityMs = 0;
	}

	FHOLMoraleEffect MakeEffect(int Id, int Dealt, int Taken, int Rate, int DurationMs, bool bAura)
	{
		FHOLMoraleEffect Effect;
		Effect.Id = Id;
		Effect.Dealt = Dealt;
		Effect.Taken = Taken;
		Effect.Rate = Rate;
		Effect.DurationMs = DurationMs;
		Effect.bAura = bAura;
		return Effect;
	}

	int LeverValue(const FHOLMoraleEffect& Effect, int Lever)
	{
		if (Lever == LeverDealt)
			return Effect.Dealt;
		if (Lever == LeverTaken)
			return Effect.Taken;
		if (Lever == LeverRate)
			return Effect.Rate;
		return 0;
	}

	// A malus is an effect with a negative lever.
	bool IsMalus(const FHOLMoraleEffect& Effect)
	{
		return Effect.Dealt < 0 || Effect.Taken < 0 || Effect.Rate < 0;
	}

	// Value x Factor / 100, rounded half away from zero (the C++ copy does the same).
	int Scale(int Value, int FactorPercent)
	{
		const int Product = Value * FactorPercent;
		if (Product >= 0)
			return Math::IntegerDivisionTrunc(Product + 50, 100);
		return -Math::IntegerDivisionTrunc(-Product + 50, 100);
	}

	// The effect with its intensity scaled (a talent or a technology of the owner; the time is not changed).
	FHOLMoraleEffect WithIntensity(const FHOLMoraleEffect& Effect, int FactorPercent)
	{
		FHOLMoraleEffect Scaled = Effect;
		Scaled.Dealt = Scale(Effect.Dealt, FactorPercent);
		Scaled.Taken = Scale(Effect.Taken, FactorPercent);
		Scaled.Rate = Scale(Effect.Rate, FactorPercent);
		return Scaled;
	}

	bool HasIntensity(const FHOLMoraleEffect& Effect)
	{
		return Effect.Dealt != 0 || Effect.Taken != 0 || Effect.Rate != 0;
	}

	// The effect after an attenuation (percent): a malus only; a temporary one loses time, an aura one loses intensity, by the same fraction.
	FHOLMoraleEffect Attenuated(const FHOLMoraleEffect& Effect, int AttenuationPercent)
	{
		if (AttenuationPercent <= 0 || !IsMalus(Effect))
			return Effect;
		const int Kept = 100 - Math::Min(AttenuationPercent, AttenuationCap);
		if (Effect.bAura)
			return WithIntensity(Effect, Kept);
		FHOLMoraleEffect Shorter = Effect;
		Shorter.DurationMs = Scale(Effect.DurationMs, Kept);
		return Shorter;
	}

	// The attenuation of several sources: the strongest of each nature, the natures added, 75 % at most. Natures and Percents go in pairs; a
	// negative source counts for nothing, an unpaired nature is ignored.
	int CombineAttenuation(const TArray<int>& Natures, const TArray<int>& Percents)
	{
		const int Sources = Math::Min(Natures.Num(), Percents.Num());
		int Total = 0;
		for (int Index = 0; Index < Sources; Index++)
		{
			// Counted at the first source of its nature, with the strongest of them.
			bool bFirst = true;
			int Best = Percents[Index];
			for (int Other = 0; Other < Sources; Other++)
			{
				if (Other == Index || Natures[Other] != Natures[Index])
					continue;
				if (Other < Index)
					bFirst = false;
				Best = Math::Max(Best, Percents[Other]);
			}
			if (bFirst && Best > 0)
				Total += Best;
		}
		return Math::Min(Total, AttenuationCap);
	}

	int Count(const FHOLMoraleState& State)
	{
		return State.Effects.Num();
	}

	bool Has(const FHOLMoraleState& State, int Id)
	{
		for (int Index = 0; Index < State.Effects.Num(); Index++)
		{
			if (State.Effects[Index].Id == Id)
				return true;
		}
		return false;
	}

	// Of two benefit percents of one effect, the one with the larger magnitude.
	int Strongest(int A, int B)
	{
		const int MagA = A < 0 ? -A : A;
		const int MagB = B < 0 ? -B : B;
		return MagB > MagA ? B : A;
	}

	// Poses an effect on a unit after the attenuation of its sources (a malus only). A malus is refused during the immunity; an effect with
	// no intensity, or a temporary one left with no time, is not posed; the same effect already there is refreshed (strongest intensity,
	// longest time).
	int Apply(FHOLMoraleState& State, const FHOLMoraleEffect& Effect, int AttenuationPercent = 0)
	{
		if (IsMalus(Effect) && State.ImmunityMs > 0)
			return ApplyBlockedByImmunity;
		const FHOLMoraleEffect Posed = Attenuated(Effect, AttenuationPercent);
		if (!HasIntensity(Posed) || (!Posed.bAura && Posed.DurationMs <= 0))
			return ApplyNothing;
		for (int Index = 0; Index < State.Effects.Num(); Index++)
		{
			if (State.Effects[Index].Id != Posed.Id)
				continue;
			State.Effects[Index].Dealt = Strongest(State.Effects[Index].Dealt, Posed.Dealt);
			State.Effects[Index].Taken = Strongest(State.Effects[Index].Taken, Posed.Taken);
			State.Effects[Index].Rate = Strongest(State.Effects[Index].Rate, Posed.Rate);
			State.Effects[Index].DurationMs = Math::Max(State.Effects[Index].DurationMs, Posed.DurationMs);
			return ApplyRefreshed;
		}
		State.Effects.Add(Posed);
		return ApplyPosed;
	}

	// The net percent on a lever: the bonuses added and cut at the bonus cap, the malus added and cut at the malus cap, the two offset.
	int Total(const FHOLMoraleState& State, int Lever)
	{
		int Bonus = 0;
		int Malus = 0;
		for (int Index = 0; Index < State.Effects.Num(); Index++)
		{
			const int Value = LeverValue(State.Effects[Index], Lever);
			if (Value > 0)
				Bonus += Value;
			else
				Malus -= Value;
		}
		return Math::Min(Bonus, BonusCap(Lever)) - Math::Min(Malus, MalusCap(Lever));
	}

	// What combat reads: the percent of the damage dealt, of the damage taken, of the attack rate. The damage taken falls when the benefit rises.
	int Multiplier(const FHOLMoraleState& State, int Lever)
	{
		const int Net = Total(State, Lever);
		return Lever == LeverTaken ? 100 - Net : 100 + Net;
	}

	// The time passes on the fixed step: temporary effects run down and end at 0, auras do not run down, the immunity runs down to 0.
	void Tick(FHOLMoraleState& State, int ElapsedMs)
	{
		for (int Index = State.Effects.Num() - 1; Index >= 0; Index--)
		{
			if (State.Effects[Index].bAura)
				continue;
			State.Effects[Index].DurationMs -= ElapsedMs;
			if (State.Effects[Index].DurationMs <= 0)
				State.Effects.RemoveAt(Index);
		}
		State.ImmunityMs = Math::Max(0, State.ImmunityMs - ElapsedMs);
	}

	// The unit leaves every aura: the engine poses again those still in range.
	void ClearAuras(FHOLMoraleState& State)
	{
		for (int Index = State.Effects.Num() - 1; Index >= 0; Index--)
		{
			if (State.Effects[Index].bAura)
				State.Effects.RemoveAt(Index);
		}
	}

	// Takes off every malus (Lumiere sacree); the count removed. The bonus stay; the immunity is granted apart.
	int RemoveMalus(FHOLMoraleState& State)
	{
		int Removed = 0;
		for (int Index = State.Effects.Num() - 1; Index >= 0; Index--)
		{
			if (IsMalus(State.Effects[Index]))
			{
				State.Effects.RemoveAt(Index);
				Removed++;
			}
		}
		return Removed;
	}

	// The immunity to the malus for this long; a shorter grant never cuts a longer one.
	void GrantImmunity(FHOLMoraleState& State, int DurationMs)
	{
		State.ImmunityMs = Math::Max(State.ImmunityMs, DurationMs);
	}
}
