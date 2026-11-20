// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMoraleRules.h"

// STUB (red test first, PLAN-MORAL-1120 step 2): compiles, does nothing. The body comes in the next commit.
namespace HOLMorale
{
	int32 BonusCap(int32 Lever) { return 0; }
	int32 MalusCap(int32 Lever) { return 0; }
	FHOLMoraleEffect MakeEffect(int32 Id, int32 Dealt, int32 Taken, int32 Rate, int32 DurationMs, bool bAura) { return FHOLMoraleEffect(); }
	FHOLMoraleEffect MakeEffectFromRow(int32 RowId) { return FHOLMoraleEffect(); }
	int32 LeverValue(const FHOLMoraleEffect& Effect, int32 Lever) { return 0; }
	bool IsMalus(const FHOLMoraleEffect& Effect) { return false; }
	int32 Scale(int32 Value, int32 FactorPercent) { return 0; }
	FHOLMoraleEffect WithIntensity(const FHOLMoraleEffect& Effect, int32 FactorPercent) { return Effect; }
	bool HasIntensity(const FHOLMoraleEffect& Effect) { return false; }
	FHOLMoraleEffect Attenuated(const FHOLMoraleEffect& Effect, int32 AttenuationPercent) { return Effect; }
	int32 CombineAttenuation(const int32* Natures, const int32* Percents, int32 Count) { return 0; }
	bool Has(const FHOLMoraleState& State, int32 Id) { return false; }
	int32 Apply(FHOLMoraleState& State, const FHOLMoraleEffect& Effect, int32 AttenuationPercent) { return ApplyNothing; }
	int32 Total(const FHOLMoraleState& State, int32 Lever) { return 0; }
	int32 Multiplier(const FHOLMoraleState& State, int32 Lever) { return 100; }
	void Tick(FHOLMoraleState& State, int32 ElapsedMs) {}
	void ClearAuras(FHOLMoraleState& State) {}
	int32 RemoveMalus(FHOLMoraleState& State) { return 0; }
	void GrantImmunity(FHOLMoraleState& State, int32 DurationMs) {}
}
