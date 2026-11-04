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
		/** Its owner's slot (D6 of DiplomacyInProgress.md: the Diplomacy mode's allies are other owners, not a shared camp); 255 when not told. */
		uint8 Owner = 255;
		/** A King, a siege engine or a dying unit: never healed. */
		bool bUnhealable = false;
	};

	/** Whether a candidate may be healed by the healer UnitId (not itself, an ally, wounded, alive, healable). */
	HEROESOFLIBERTY_API bool IsHealable(uint32 HealerId, uint8 HealerCamp, const FCandidate& Candidate, uint32 HealerAllyOwners = 0);

	/**
	 * The candidate the healer heals this step, an index of Candidates, INDEX_NONE if none: the unit its player ordered it to heal\r\n	 * (OrderedId) while it is healable and within Range, else its current target KeptId while it is still
	 * healable and within Range (one ally at a time, not a flip to a nearer one at each step), else the nearest healable ally within
	 * Range (centre to centre, cm), the lowest unit identifier on a tie. The healer itself is never chosen.
	 */
	HEROESOFLIBERTY_API int32 ChooseTarget(uint32 HealerId, uint8 HealerCamp, const FVector2D& HealerLocation, float Range, uint32 OrderedId, uint32 KeptId, TConstArrayView<FCandidate> Candidates, uint32 HealerAllyOwners = 0);

	/**
	 * D6 (Diplomacy mode): the allies of a healer are also the units of the owners whose bit is set in HealerAllyOwners (bit n = the slot n
	 * is allied to the healer's owner, both sides said Ally), besides its own camp. 0 = the camp only, as in the standard modes.
	 */
	HEROESOFLIBERTY_API bool IsAllyOf(uint8 HealerCamp, uint32 HealerAllyOwners, const FCandidate& Candidate);

	/** What a healer under a heal order does with its feet this step (HOLHeal::GetOrderWalk). */
	enum class EOrderWalk : uint8
	{
		/** Nothing new: stand (already in reach), or go on with the walk already given. */
		Keep,
		/** Walk (again) to the target: out of reach, or the target moved off the walk's goal. */
		Walk,
		/** Stop where it is: the target is in reach, the healer does not go on to contact. */
		Stop,
	};
	/** The healer stops this far inside its range (cm): a unit still gliding a step after the stop stays in reach. */
	constexpr float OrderStopMargin = 50.f;
	/** A walk is given again once the target is this far (cm) from the goal it was given. */
	constexpr float OrderRepathDistance = 200.f;
	/**
	 * The heal order's walk (a right click on an ally with a Monk selected): the healer walks only until its target is within its heal
	 * Range (never to contact), then stops and heals; a target that moves off is followed (a new walk) until in reach again.
	 * Distance: healer to target (cm, centre to centre); bWalking: a walk of this order is on (true as well for the first look at a new
	 * order, which also ends the walk an earlier order gave); GoalDrift: distance (cm) between the target now and the goal of that walk.
	 */
	HEROESOFLIBERTY_API EOrderWalk GetOrderWalk(float Distance, float Range, bool bWalking, float GoalDrift);

	/** The health after one step of heal: Health + HealRate * StepMs / 1000, held at MaxHealth. */
	HEROESOFLIBERTY_API float GetHealedHealth(float Health, float MaxHealth, float HealRate, int32 StepMs);
	/**
	 * What a client sees of a heal (the wire carries none): a unit whose shown health went up since the last frame is being healed for a
	 * moment. Fed once a frame with every unit's health; forgets the units that are gone and the rises older than KeepSeconds (no entry is
	 * removed while the maps are walked: the keys to forget are gathered first).
	 */
	struct HEROESOFLIBERTY_API FRiseTracker
	{
		/** Observes every unit of the frame (id, shown health, whether its maximum is known) at time Now. */
		void Update(TConstArrayView<TTuple<uint32, int32, bool>> Units, double Now, double KeepSeconds);
		/** Whether the unit's health rose within the last WindowSeconds. */
		bool IsRising(uint32 UnitId, double Now, double WindowSeconds) const;
		bool IsEmpty() const { return LastRise.IsEmpty(); }
		int32 NumRises() const { return LastRise.Num(); }

	private:
		TMap<uint32, int32> LastHealth;
		TMap<uint32, double> LastRise;
	};
}