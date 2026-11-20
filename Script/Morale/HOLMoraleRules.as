// Morale effects, script wrapper of the C++ rule (PLAN-MORAL-1120 step 2, D836 (1)/(5), D839): the rule has ONE source, Simulation/HOLMoraleRules.h
// (caps, Apply, Total, Multiplier, Tick, attenuation, immunity; see the comment of that file for the rules). This file keeps the names the script
// callers and tests use (HOLMorale::Apply, Total...) and forwards each call to UHOLMoraleLibrary (functions named Native*, same namespace HOLMorale
// on the script side). No rule here. The effect and the state are the structs of the library (FHOLMoraleEffectData, FHOLMoraleStateData).
// The lever indices and the Apply* codes are the contract of the bridge (the same numbers as the constants of HOLMoraleRules.h); the attenuation
// cap is read from the rule: HOLMorale::NativeAttenuationCap().
namespace HOLMorale
{
	// The three levers.
	const int LeverDealt = 0;
	const int LeverTaken = 1;
	const int LeverRate = 2;
	const int LeverCount = 3;

	// What Apply did.
	const int ApplyNothing = 0;
	const int ApplyPosed = 1;
	const int ApplyRefreshed = 2;
	const int ApplyBlockedByImmunity = 3;
	const int ApplyNoRoom = 4;

	// The cap of the bonus on a lever, in percent.
	int BonusCap(int Lever)
	{
		return HOLMorale::NativeBonusCap(Lever);
	}

	// The cap of the malus on a lever, in percent (of the magnitude).
	int MalusCap(int Lever)
	{
		return HOLMorale::NativeMalusCap(Lever);
	}

	FHOLMoraleEffectData MakeEffect(int Id, int Dealt, int Taken, int Rate, int DurationMs, bool bAura)
	{
		return HOLMorale::NativeMakeEffect(Id, Dealt, Taken, Rate, DurationMs, bAura);
	}

	int LeverValue(const FHOLMoraleEffectData& Effect, int Lever)
	{
		return HOLMorale::NativeLeverValue(Effect, Lever);
	}

	// A malus is an effect with a negative lever.
	bool IsMalus(const FHOLMoraleEffectData& Effect)
	{
		return HOLMorale::NativeIsMalus(Effect);
	}

	int Scale(int Value, int FactorPercent)
	{
		return HOLMorale::NativeScale(Value, FactorPercent);
	}

	FHOLMoraleEffectData WithIntensity(const FHOLMoraleEffectData& Effect, int FactorPercent)
	{
		return HOLMorale::NativeWithIntensity(Effect, FactorPercent);
	}

	bool HasIntensity(const FHOLMoraleEffectData& Effect)
	{
		return HOLMorale::NativeHasIntensity(Effect);
	}

	FHOLMoraleEffectData Attenuated(const FHOLMoraleEffectData& Effect, int AttenuationPercent)
	{
		return HOLMorale::NativeAttenuated(Effect, AttenuationPercent);
	}

	int CombineAttenuation(const TArray<int>& Natures, const TArray<int>& Percents)
	{
		return HOLMorale::NativeCombineAttenuation(Natures, Percents);
	}

	int Count(const FHOLMoraleStateData& State)
	{
		return State.Effects.Num();
	}

	bool Has(const FHOLMoraleStateData& State, int Id)
	{
		return HOLMorale::NativeHas(State, Id);
	}

	int Apply(FHOLMoraleStateData& State, const FHOLMoraleEffectData& Effect, int AttenuationPercent = 0)
	{
		return HOLMorale::NativeApply(State, Effect, AttenuationPercent);
	}

	int Total(const FHOLMoraleStateData& State, int Lever)
	{
		return HOLMorale::NativeTotal(State, Lever);
	}

	int Multiplier(const FHOLMoraleStateData& State, int Lever)
	{
		return HOLMorale::NativeMultiplier(State, Lever);
	}

	void Tick(FHOLMoraleStateData& State, int ElapsedMs)
	{
		HOLMorale::NativeTick(State, ElapsedMs);
	}

	void ClearAuras(FHOLMoraleStateData& State)
	{
		HOLMorale::NativeClearAuras(State);
	}

	int RemoveMalus(FHOLMoraleStateData& State)
	{
		return HOLMorale::NativeRemoveMalus(State);
	}

	void GrantImmunity(FHOLMoraleStateData& State, int DurationMs)
	{
		HOLMorale::NativeGrantImmunity(State, DurationMs);
	}
}
