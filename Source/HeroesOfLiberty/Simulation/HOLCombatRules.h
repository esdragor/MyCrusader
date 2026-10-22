// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FHOLUnitTypeConstants;

/**
 * Pure rules of the melee (J7b.1, J7-Q4: home-made processor, AoE4-like rules): the damage of a hit, the cadence of the
 * hits on the fixed step, the reach, and which enemy a unit takes on. No engine state, integers and plain comparisons
 * (the server runs them; the outcome does not depend on the order the candidates come in): tested on their own. The
 * combat processor of J7b.2 calls them.
 */
namespace HOLCombat
{
	/** Steps a unit at 0 health stays Dying before its destruction (J7b.4, J7-Q5): ~1 s at 20 Hz, for its fall. */
	constexpr int32 DyingSteps = 20;

	/**
	 * Damage of one hit (D21): Attack, plus BonusDamage when the defender is of the attacker's BonusCategory, minus the
	 * defender's melee armour (or its ranged armour, bRanged), 1 at least (a hit always hurts).
	 */
	HEROESOFLIBERTY_API int32 GetDamage(const FHOLUnitTypeConstants& Attacker, const FHOLUnitTypeConstants& Defender, bool bRanged);

	/** Hits that take Health down to 0 (0 when Health <= 0). */
	HEROESOFLIBERTY_API int32 GetHitsToKill(int32 Health, int32 Damage);

	// Cadence. The time between two hits is counted in whole milliseconds, not in steps: a cool-down loses the step's
	// length each step, a hit falls when it reaches 0, then the interval is added back. The average cadence is the
	// table's exactly (1.875 s on 50 ms steps: hits 37 or 38 steps apart, no drift from rounding to 38 steps), and it
	// stays integer, so deterministic.

	/** Seconds to whole milliseconds (the table's 1.875 s -> 1875). */
	HEROESOFLIBERTY_API int32 ToMilliseconds(float Seconds);

	/**
	 * Wind-up: the first hit on a target falls WindUpPercent of an interval after the unit starts attacking it (in reach),
	 * not at once, so the strike gesture has time to show and a unit coming into reach does not hit in the same step. The
	 * percent is the type's (FHOLUnitTypeConstants::AttackWindUpPercent, J7c, retour d'essai 8: hit and run): half the
	 * interval for the melee, 12 % for the Archer (it looses ~0.2 s after it stops, then may walk off while it
	 * reloads, the cool-down running down meanwhile, TickIdle). Clamped to 1..100.
	 */
	inline constexpr int32 DefaultWindUpPercent = 50;

	HEROESOFLIBERTY_API int32 GetWindUpMilliseconds(int32 IntervalMs, int32 WindUpPercent);

	/**
	 * The cool-down when a unit starts (or switches) an attack: what it still had to wait, the wind-up at least. A unit
	 * that just struck keeps its own rhythm; a switch of target never gives a free hit.
	 */
	HEROESOFLIBERTY_API int32 StartAttack(int32 CooldownMs, int32 IntervalMs, int32 WindUpPercent);

	/**
	 * One step of an attacking unit (in reach of its target): the cool-down loses StepMs; when it reaches 0 the unit
	 * strikes (true) and IntervalMs is added back. One hit per step at most (an interval shorter than the step is taken
	 * as one step).
	 */
	HEROESOFLIBERTY_API bool TickAttack(int32& CooldownMs, int32 StepMs, int32 IntervalMs);

	/** One step of a unit that does not attack: the cool-down runs down to 0 and stays there. */
	HEROESOFLIBERTY_API void TickIdle(int32& CooldownMs, int32 StepMs);

	/** Gap between the edges of two bodies (negative when they overlap). */
	HEROESOFLIBERTY_API double GetEdgeDistance(const FVector2D& A, float RadiusA, const FVector2D& B, float RadiusB);

	/**
	 * Reach (AttackRange is between the two bodies' edges): the gap between the edges, centre distance minus both radii,
	 * at most Range. Touching bodies (gap 0 or less, pushed into each other) are in reach.
	 */
	HEROESOFLIBERTY_API bool IsInRange(const FVector2D& A, float RadiusA, const FVector2D& B, float RadiusB, float Range);

	/**
	 * Share (0, 0.5 or 1) of the overlap of two bodies that the separation gives to A, the unit that looks for its
	 * neighbours (a walker, or a standing unit pushed this step), B taking the rest. Two units of one player take half
	 * each, the soft push of the separation (D292). Two enemies never push each other back (the user's trial of J7b: the
	 * lines hold, as in AoE4): a walker stops against a standing enemy (1), a standing enemy is never moved, and a unit
	 * pushed into an enemy by its own side steps back itself; two enemies both walking into each other take half each.
	 */
	HEROESOFLIBERTY_API double GetOverlapShare(bool bEnemies, bool bWalkingA, bool bWalkingB);

	// Target choice.

	/** How the unit looks for an enemy. */
	enum class ESeekMode : uint8
	{
		/** Standing or walking without an order to fight: takes on enemies within its aggression radius (0: none). */
		Auto,
		/** Attack-move: as Auto, but a unit without an aggression radius uses AttackMoveRadius. */
		AttackMove,
		/** Hold position: only enemies already in reach, never a chase. */
		Hold,
	};

	/** Search radius (cm, edge to edge) on an attack-move of a unit with no aggression of its own (the soldiers' 12 m). */
	inline constexpr float AttackMoveRadius = 1200.f;

	/**
	 * Spreading: an enemy out of reach already taken on by MaxAttackers others comes after one less crowded (about six
	 * bodies of 60 cm fit around one of 60 cm). One in reach is never passed over for that: it can be hit now.
	 */
	inline constexpr int32 MaxAttackers = 6;

	/** The unit looking for a target. */
	struct FSeeker
	{
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 60.f;
		float AttackRange = 50.f;
		float AggressionRadius = 0.f;
		ESeekMode Mode = ESeekMode::Auto;
		/** A shooter (IsRangedAttack, J7c-Q6): no spreading to a less crowded enemy, the nearest, kept while in reach. */
		bool bRanged = false;
	};

	/** One enemy the grid found near the seeker (the processor fills them). */
	struct FCandidate
	{
		/** Tie-break, last: a stable id of the entity (in the processor, its index then its serial number). */
		uint64 Id = 0;
		FVector2D Position = FVector2D::ZeroVector;
		float Radius = 60.f;
		/** Hook for later parts: a building comes after any unit (none in J7b). */
		bool bIsBuilding = false;
		/** It attacks the seeker (riposte: such an enemy is taken on even beyond the search radius). */
		bool bAttacksMe = false;
		/** The seeker's current target: kept while in reach, so the choice does not flicker between two equal ones. */
		bool bIsCurrentTarget = false;
		/** Others of the seeker's side already attacking it (the seeker left out). */
		int32 AttackerCount = 0;
		/**
		 * Out of the seeker's reach, every place around it taken (MaxAttackers bodies of any side touching it; the processor
		 * sets it under hol.Combat.PreferFreeTarget): one with a place comes first, the current target first among the full.
		 */
		bool bNoPlace = false;
	};

	/**
	 * Radius (cm, edge to edge) within which the seeker takes on an enemy it was not ordered at: on hold, its reach;
	 * else its aggression radius (an attack-move gives AttackMoveRadius to a unit without one), never less than its
	 * reach. 0: it takes on nobody by itself (the Peasant, unless on an attack-move or on hold).
	 */
	HEROESOFLIBERTY_API float GetSearchRadius(const FSeeker& Seeker);

	/**
	 * The enemy the seeker takes on by itself, as an index into Candidates, INDEX_NONE for none (an ordered target does
	 * not come through here). Eligible: within the search radius, or attacking the seeker (riposte) when it has a search
	 * radius at all; on hold, in reach only. The current target, in reach, is kept. Otherwise the best by, in order: a
	 * unit before a building; in reach before out of reach; attacking the seeker before not; (out of reach) fewer than
	 * MaxAttackers attackers before crowded (not for a shooter, bRanged); nearest by edge distance; smallest Id. Any order of the same candidates
	 * gives the same enemy.
	 */
	HEROESOFLIBERTY_API int32 ChooseTarget(const FSeeker& Seeker, TConstArrayView<FCandidate> Candidates);

	// Shots (J7c.2, J7c-Q1 to Q4). A shooter's hit is an arrow: released at the strike's moment of the gesture (the
	// wind-up, the type's AttackWindUpPercent of the cadence, 12 % for the Archer), it flies the centre-to-centre
	// distance of the shooter and its target at release at the arrow speed, and always hits: its damage (GetDamage, ranged armour, reckoned at release)
	// falls on the target when it lands, lost when the target died meanwhile. No minimum reach, no height bonus. The client
	// reckons the same flight from the same positions (J7c.2b): nothing more is replicated.

	/** Reach (cm) above which a unit's hits are shots (the ranged armour, an arrow): the Archer's 15 m, not a melee's 50 cm. */
	inline constexpr float RangedReach = 200.f;

	/** Speed of an arrow by default (cm/s): 37.5 m/s (J7c-Q2 25 m/s, x1.5 at the trial of D345), hol.Combat.ArrowSpeed. */
	inline constexpr float DefaultArrowSpeed = 3750.f;

	/** A unit of this reach shoots (AttackRange > RangedReach). */
	HEROESOFLIBERTY_API bool IsRangedAttack(float AttackRange);

	/** Flight of an arrow (whole ms): the 2D distance From -> To (centres, at release) over SpeedCmPerSecond, rounded. */
	HEROESOFLIBERTY_API int32 GetArrowFlightMilliseconds(const FVector2D& From, const FVector2D& To, float SpeedCmPerSecond);

	/**
	 * Steps from the release to the landing: the flight rounded up to whole steps, 1 at least (an arrow released at step S
	 * lands at the start of step S + GetArrowFlightSteps, never in its own step).
	 */
	HEROESOFLIBERTY_API int32 GetArrowFlightSteps(int32 FlightMs, int32 StepMs);

	/** What an arrow does when it lands. */
	enum class EArrowOutcome : uint8
	{
		/** Its target died or is gone meanwhile: lost, no damage (J7c-Q2). */
		Lost,
		/** It hurt its target, which still stands. */
		Hit,
		/** It took its target to 0: the target dies (the dying path, as in melee). */
		Kill,
	};

	/** An arrow landing: on a standing target (bTargetStands, Health > 0) Damage comes off Health; Lost otherwise. */
	HEROESOFLIBERTY_API EArrowOutcome ResolveArrow(bool bTargetStands, int32& Health, int32 Damage);

	/** hol.Combat.ArrowSpeed (cm/s, 1 at least): not a pure rule, the setting the server and the client both read. */
	HEROESOFLIBERTY_API float GetArrowSpeed();
}
