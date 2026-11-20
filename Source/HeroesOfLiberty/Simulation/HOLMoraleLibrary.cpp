// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMoraleLibrary.h"
#include "Simulation/HOLMoraleRules.h"

// Wrappers only (D839): each function converts the script state to the fixed state of the rule, calls HOLMorale::*, converts back. No rule here.

namespace
{
	HOLMorale::FHOLMoraleEffect ToRule(const FHOLMoraleEffectData& Data)
	{
		HOLMorale::FHOLMoraleEffect Effect;
		Effect.Id = Data.Id;
		Effect.Dealt = Data.Dealt;
		Effect.Taken = Data.Taken;
		Effect.Rate = Data.Rate;
		Effect.DurationMs = Data.DurationMs;
		Effect.bAura = Data.bAura;
		return Effect;
	}

	FHOLMoraleEffectData ToData(const HOLMorale::FHOLMoraleEffect& Effect)
	{
		FHOLMoraleEffectData Data;
		Data.Id = Effect.Id;
		Data.Dealt = Effect.Dealt;
		Data.Taken = Effect.Taken;
		Data.Rate = Effect.Rate;
		Data.DurationMs = Effect.DurationMs;
		Data.bAura = Effect.bAura;
		return Data;
	}

	// The first MaxEffects effects of the script state (a script state never holds more: Apply refuses a fifth effect).
	HOLMorale::FHOLMoraleState ToRule(const FHOLMoraleStateData& Data)
	{
		HOLMorale::FHOLMoraleState State;
		State.Num = FMath::Min(Data.Effects.Num(), HOLMorale::MaxEffects);
		for (int32 Index = 0; Index < State.Num; Index++)
			State.Effects[Index] = ToRule(Data.Effects[Index]);
		State.ImmunityMs = Data.ImmunityMs;
		return State;
	}

	void WriteBack(const HOLMorale::FHOLMoraleState& State, FHOLMoraleStateData& Data)
	{
		Data.Effects.Reset();
		for (int32 Index = 0; Index < State.Num; Index++)
			Data.Effects.Add(ToData(State.Effects[Index]));
		Data.ImmunityMs = State.ImmunityMs;
	}
}

int32 UHOLMoraleLibrary::NativeAttenuationCap() { return HOLMorale::AttenuationCap; }

int32 UHOLMoraleLibrary::NativeBonusCap(const int32 Lever) { return HOLMorale::BonusCap(Lever); }

int32 UHOLMoraleLibrary::NativeMalusCap(const int32 Lever) { return HOLMorale::MalusCap(Lever); }

FHOLMoraleEffectData UHOLMoraleLibrary::NativeMakeEffect(const int32 Id, const int32 Dealt, const int32 Taken, const int32 Rate, const int32 DurationMs, const bool bAura)
{
	return ToData(HOLMorale::MakeEffect(Id, Dealt, Taken, Rate, DurationMs, bAura));
}

FHOLMoraleEffectData UHOLMoraleLibrary::NativeMakeEffectFromRow(const int32 RowId) { return ToData(HOLMorale::MakeEffectFromRow(RowId)); }

int32 UHOLMoraleLibrary::NativeLeverValue(const FHOLMoraleEffectData& Effect, const int32 Lever) { return HOLMorale::LeverValue(ToRule(Effect), Lever); }

bool UHOLMoraleLibrary::NativeIsMalus(const FHOLMoraleEffectData& Effect) { return HOLMorale::IsMalus(ToRule(Effect)); }

int32 UHOLMoraleLibrary::NativeScale(const int32 Value, const int32 FactorPercent) { return HOLMorale::Scale(Value, FactorPercent); }

FHOLMoraleEffectData UHOLMoraleLibrary::NativeWithIntensity(const FHOLMoraleEffectData& Effect, const int32 FactorPercent)
{
	return ToData(HOLMorale::WithIntensity(ToRule(Effect), FactorPercent));
}

bool UHOLMoraleLibrary::NativeHasIntensity(const FHOLMoraleEffectData& Effect) { return HOLMorale::HasIntensity(ToRule(Effect)); }

FHOLMoraleEffectData UHOLMoraleLibrary::NativeAttenuated(const FHOLMoraleEffectData& Effect, const int32 AttenuationPercent)
{
	return ToData(HOLMorale::Attenuated(ToRule(Effect), AttenuationPercent));
}

int32 UHOLMoraleLibrary::NativeCombineAttenuation(const TArray<int32>& Natures, const TArray<int32>& Percents)
{
	return HOLMorale::CombineAttenuation(Natures.GetData(), Percents.GetData(), FMath::Min(Natures.Num(), Percents.Num()));
}

bool UHOLMoraleLibrary::NativeHas(const FHOLMoraleStateData& State, const int32 Id) { return HOLMorale::Has(ToRule(State), Id); }

int32 UHOLMoraleLibrary::NativeApply(FHOLMoraleStateData& State, const FHOLMoraleEffectData& Effect, const int32 AttenuationPercent)
{
	HOLMorale::FHOLMoraleState Rule = ToRule(State);
	const int32 Result = HOLMorale::Apply(Rule, ToRule(Effect), AttenuationPercent);
	WriteBack(Rule, State);
	return Result;
}

int32 UHOLMoraleLibrary::NativeTotal(const FHOLMoraleStateData& State, const int32 Lever) { return HOLMorale::Total(ToRule(State), Lever); }

int32 UHOLMoraleLibrary::NativeMultiplier(const FHOLMoraleStateData& State, const int32 Lever) { return HOLMorale::Multiplier(ToRule(State), Lever); }

void UHOLMoraleLibrary::NativeTick(FHOLMoraleStateData& State, const int32 ElapsedMs)
{
	HOLMorale::FHOLMoraleState Rule = ToRule(State);
	HOLMorale::Tick(Rule, ElapsedMs);
	WriteBack(Rule, State);
}

void UHOLMoraleLibrary::NativeClearAuras(FHOLMoraleStateData& State)
{
	HOLMorale::FHOLMoraleState Rule = ToRule(State);
	HOLMorale::ClearAuras(Rule);
	WriteBack(Rule, State);
}

int32 UHOLMoraleLibrary::NativeRemoveMalus(FHOLMoraleStateData& State)
{
	HOLMorale::FHOLMoraleState Rule = ToRule(State);
	const int32 Removed = HOLMorale::RemoveMalus(Rule);
	WriteBack(Rule, State);
	return Removed;
}

void UHOLMoraleLibrary::NativeGrantImmunity(FHOLMoraleStateData& State, const int32 DurationMs)
{
	HOLMorale::FHOLMoraleState Rule = ToRule(State);
	HOLMorale::GrantImmunity(Rule, DurationMs);
	WriteBack(Rule, State);
}
