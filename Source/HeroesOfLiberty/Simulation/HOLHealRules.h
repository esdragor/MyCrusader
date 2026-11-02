// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The heal of the Monk (Q-chapelle-moine = A, GDD 7.1, D134, D212, D471), pure rules read by UHOLHealProcessor and the tests: a healer
 * (a unit type with a HealRate) heals one wounded ally at a time, within its HealRange, never itself, never a King (HOLCanBeHealed),
 * never a siege engine (it is repaired, not healed), never a dying unit.
 */
namespace HOLHeal
{
	/** A unit the healer may weigh: what the rules read, filled once per step by the processor from the unit's fragments. */
	struct FCandidate
	{
		uint32 UnitId = 0;
		FVector2D Location = FVector2D::ZeroVector;
		/** Health and maximum health (floats: a heal of 0.25 per step is not rounded). */
		float Health = 0.f;
		float MaxHealth = 0.f;
		/** Its camp (FHOLCampTable): the healer's camp is the allies' (itself included). */
		uint8 Camp = 0;
		/** A King, a siege engine or a dying unit: never healed. */
		bool bUnhealable = false;
	};

	/** Whether a candidate may be healed by the healer UnitId (not itself, an ally, wounded, alive, healable). */
	HEROESOFLIBERTY_API bool IsHealable(uint32 HealerId, uint8 HealerCamp, const FCandidate& Candidate);

	/**
	 * The candidate the healer heals this step, an index of Candidates, INDEX_NONE if none: the unit its player ordered it to heal\r\n	 * (OrderedId) while it is healable and within Range, else its current target KeptId while it is still
	 * healable and within Range (one ally at a time, not a flip to a nearer one at each step), else the nearest healable ally within
	 * Range (centre to centre, cm), the lowest unit identifier on a tie. The healer itself is never chosen.
	 */
	HEROESOFLIBERTY_API int32 ChooseTarget(uint32 HealerId, uint8 HealerCamp, const FVector2D& HealerLocation, float Range, uint32 OrderedId, uint32 KeptId, TConstArrayView<FCandidate> Candidates);

	/** The health after one step of heal: Health + HealRate * StepMs / 1000, held at MaxHealth. */
	HEROESOFLIBERTY_API float GetHealedHealth(float Health, float MaxHealth, float HealRate, int32 StepMs);
}
