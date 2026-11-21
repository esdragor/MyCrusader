// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLMoraleTableRows.h"

/**
 * Morale effects, pure rules (PLAN-MORAL-1120 step 2, D704-D709, D836 (1)/(5): the C++ is the single source of the rule, the script
 * Script/Morale/HOLMoraleRules.as becomes a wrapper of UHOLMoraleLibrary). Integers only, no world access, no fragment: the simulation
 * calls these per unit and per step. A unit holds at most MaxEffects named effects (a fixed array, no allocation), each temporary (a time
 * left) or an aura (it lasts while the unit is in range: the engine clears the auras each step and poses again those still in range). An
 * effect acts on three levers in percent: the damage dealt, the damage taken, the attack rate (walking speed stays out of morale, D704).
 * Percents are signed "benefit" values: positive helps the unit on every lever (on the damage taken, +25 means 25 % less taken), negative
 * hurts it. An effect with a negative lever is a malus.
 * Rules: the same effect is never added to itself (the strongest intensity and the longest time are kept); different effects add up per
 * lever, the bonus and the malus each under their own cap, then offset (D704); the attenuation (D125, D706) shortens a temporary malus and
 * weakens an aura malus by the same fraction, never a bonus; sources add up to 75 % at most, the strongest of one nature counting alone;
 * the immunity (D404) stops a malus from being posed, never a bonus; the removal takes the malus and leaves the bonus.
 * Caps: 50 % on the damage taken (bonus and malus, symmetric), on the damage dealt 40 % for the bonus (Q-legions-rage-cap = A) but 30 %
 * for the malus (D791, "bonus only"), 30 % on the rate. The effects come from the rows of HOLMoraleTableRows.h (Effects.csv).
 */
namespace HOLMorale
{
	// The three levers.
	constexpr int32 LeverDealt = 0;
	constexpr int32 LeverTaken = 1;
	constexpr int32 LeverRate = 2;
	constexpr int32 LeverCount = 3;

	// The effects a unit can carry at once (the fragment of step 3 has the same four slots).
	constexpr int32 MaxEffects = 4;

	// The most attenuation any sources can give together: never 100 % (only Lumiere sacree removes, D404, D706).
	constexpr int32 AttenuationCap = 75;

	// The immunity to the malus that Lumiere sacree grants, in milliseconds.
	constexpr int32 SacredLightImmunityMs = 5000;

	// What Apply did.
	constexpr int32 ApplyNothing = 0;
	constexpr int32 ApplyPosed = 1;
	constexpr int32 ApplyRefreshed = 2;
	constexpr int32 ApplyBlockedByImmunity = 3;
	// The unit already carries MaxEffects other effects: the new one is not posed.
	constexpr int32 ApplyNoRoom = 4;

	// One effect on a unit. Id names it (a row of the table); Dealt / Taken / Rate are its benefit percents; DurationMs the time left of a
	// temporary effect (ignored for an aura).
	struct FHOLMoraleEffect
	{
		int32 Id = -1;
		int32 Dealt = 0;
		int32 Taken = 0;
		int32 Rate = 0;
		int32 DurationMs = 0;
		bool bAura = false;
	};

	// What a unit carries: its effects (the first Num slots) and the time left of its immunity to the malus.
	struct FHOLMoraleState
	{
		FHOLMoraleEffect Effects[MaxEffects];
		int32 Num = 0;
		int32 ImmunityMs = 0;
	};

	// The cap of the bonus on a lever, in percent.
	int32 BonusCap(int32 Lever);

	// The cap of the malus on a lever, in percent (of the magnitude).
	int32 MalusCap(int32 Lever);

	FHOLMoraleEffect MakeEffect(int32 Id, int32 Dealt, int32 Taken, int32 Rate, int32 DurationMs, bool bAura);

	// The effect of a table row (HOLMoraleTableRows::Rows): a row with Gives (a banner) gives the levers of the row it names, under that
	// row's id, so a banner and the hero's own aura are one effect and never add up. A bad id gives an effect with no intensity (Id -1).
	FHOLMoraleEffect MakeEffectFromRow(int32 RowId);

	int32 LeverValue(const FHOLMoraleEffect& Effect, int32 Lever);

	// A malus is an effect with a negative lever.
	bool IsMalus(const FHOLMoraleEffect& Effect);

	// Value x FactorPercent / 100, rounded half away from zero.
	int32 Scale(int32 Value, int32 FactorPercent);

	// The effect with its intensity scaled (a talent or a technology of the owner; the time is not changed).
	FHOLMoraleEffect WithIntensity(const FHOLMoraleEffect& Effect, int32 FactorPercent);

	bool HasIntensity(const FHOLMoraleEffect& Effect);

	// The effect after an attenuation (percent): a malus only; a temporary one loses time, an aura one loses intensity, by the same fraction.
	FHOLMoraleEffect Attenuated(const FHOLMoraleEffect& Effect, int32 AttenuationPercent);

	// The attenuation of several sources: the strongest of each nature, the natures added, 75 % at most. Natures and Percents go in pairs
	// (Count of each); a negative source counts for nothing.
	int32 CombineAttenuation(const int32* Natures, const int32* Percents, int32 Count);

	bool Has(const FHOLMoraleState& State, int32 Id);

	// Poses an effect on a unit after the attenuation of its sources (a malus only). A malus is refused during the immunity; an effect with
	// no intensity, or a temporary one left with no time, is not posed; the same effect already there is refreshed (strongest intensity,
	// longest time); a full unit takes no new effect (ApplyNoRoom). Returns one of the Apply* codes.
	int32 Apply(FHOLMoraleState& State, const FHOLMoraleEffect& Effect, int32 AttenuationPercent = 0);

	// The net percent on a lever: the bonuses added and cut at the bonus cap, the malus added and cut at the malus cap, the two offset.
	int32 Total(const FHOLMoraleState& State, int32 Lever);

	// What combat reads: the percent of the damage dealt, of the damage taken, of the attack rate. The damage taken falls when the benefit rises.
	int32 Multiplier(const FHOLMoraleState& State, int32 Lever);

	// The time passes on the fixed step: temporary effects run down and end at 0, auras do not run down, the immunity runs down to 0.
	void Tick(FHOLMoraleState& State, int32 ElapsedMs);

	// The unit leaves every aura: the engine poses again those still in range.
	void ClearAuras(FHOLMoraleState& State);

	// Takes off every malus (Lumiere sacree); the count removed. The bonus stay; the immunity is granted apart.
	int32 RemoveMalus(FHOLMoraleState& State);

	// A unit with no effect and no immunity: its fragment is left out of the step's fingerprint (D190, as RageMs).
	bool IsEmpty(const FHOLMoraleState& State);

	// What the step's fingerprint mixes for a unit: 0 when IsEmpty, else a non-zero hash of every effect (id, three levers, time, aura) and of the immunity time.
	uint64 FingerprintKey(const FHOLMoraleState& State);

	// The immunity to the malus for this long; a shorter grant never cuts a longer one.
	void GrantImmunity(FHOLMoraleState& State, int32 DurationMs);
}
