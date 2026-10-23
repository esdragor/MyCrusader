// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/HOLSimulationFragments.h"

struct FHOLUnitTypeConstants;

/**
 * Pure rule of the animation a client plays on a unit (D234, J5c.1), from what the unit does as this machine knows it:
 * the simulation in solo, the last state received on a client (D272: one display code). No engine state: tested on its
 * own (HeroesOfLiberty.Animation.Choice).
 */

/** What the animation is chosen from. */
struct FHOLUnitAnimationInput
{
	EHOLUnitActivity Activity = EHOLUnitActivity::Idle;
	/** What it gathers, read while Activity is Gathering. */
	EHOLResource GatherResource = EHOLResource::Food;
	/** Whether it carries a load, and of what. */
	bool bCarrying = false;
	EHOLResource LoadResource = EHOLResource::Food;
	/** Horizontal speed it is drawn at (cm/s). */
	float Speed = 0.f;
	/**
	 * Speed (cm/s) its walk's play rate is made for: its type's, times hol.Units.SpeedScale (J7a.4). A walk started slower
	 * or faster (a group in formation marching at its slowest unit's speed) plays in proportion, so its feet do not
	 * slide. 0: the play rate of the table as it is.
	 */
	float WalkSpeed = 0.f;
	/** Sequence it plays now, INDEX_NONE before its first: a walk keeps walking down to a lower speed (hysteresis). */
	int32 PlayingSequence = INDEX_NONE;
	/**
	 * Building, at a site that waits (V2.3a: its main forum in ruin, HOLForumRules::BuildersWait): it stands there without
	 * striking, still not idle.
	 */
	bool bSiteWaits = false;
};

/** The animation to play. Mass hands it to the instance only when Sequence changes (play rate and start fixed then). */
struct FHOLUnitAnimationChoice
{
	int32 Sequence = 0;
	float PlayRate = 1.f;
	/** Gathering at its source: drawn facing it (display only, the simulation's orientation does not change). */
	bool bFacesSource = false;
	/** Building at its site (J6a.4): drawn facing it, as a gatherer its source. */
	bool bFacesSite = false;
	/**
	 * Prop of the mesh shown (J5c.2: the tool of the gesture, the load carried; FHOLResourceProps), 0 none. Sent every frame
	 * as the instance's 4th custom float: it changes at once, while the sequence blends in 0.2 s.
	 */
	uint8 Prop = 0;
	/**
	 * Striking (J7b.5): the loop lasts one attack interval (PlayRate = its length / AttackInterval) and the caller starts it
	 * at GetStrikeStartPosition, so its hit falls on the server's; else a random start offset (D234).
	 */
	bool bStrike = false;
	/** Played once from its start and held on its last frame (J7b.5: the fall of a dying unit), not looped. */
	bool bOnce = false;

	bool operator==(const FHOLUnitAnimationChoice& Other) const = default;
};

namespace HOLUnitAnimation
{
	/**
	 * Walk or stand from the drawn speed, as a share of the type's speed (D234). Two thresholds so a unit hovering around
	 * one does not flicker between the two animations. Provisional, to tune in test.
	 */
	constexpr float WalkStartSpeedShare = 0.15f;
	constexpr float WalkStopSpeedShare = 0.10f;

	/**
	 * What a walk's play rate is scaled by at most (J7a.4): Speed / WalkSpeed, held to this range (a stride shorter than half,
	 * or longer than 1.2 times the walk's, looks wrong in another way).
	 */
	constexpr float MinWalkRateShare = 0.5f;
	constexpr float MaxWalkRateShare = 1.2f;

	/** The share a walk's play rate is scaled by at Speed, for a walk made for WalkSpeed (1 if WalkSpeed is 0). */
	HEROESOFLIBERTY_API float GetWalkRateShare(float Speed, float WalkSpeed);

	/**
	 * Where the hit falls in a strike's loop (J7b.5), as a share of it: the type's wind-up (AttackWindUpPercent / 100, J7c,
	 * retour d'essai 8: hit and run), as the server's first hit falls that share of an interval after the unit starts
	 * attacking (HOLCombat::GetWindUpMilliseconds, checked by HeroesOfLiberty.Animation.Strike). The melee gestures are
	 * made with the hit at half the loop (soldier_poses.py, ATTACK_HIT_FRACTION); the Archer's release at 0.125 of its loop
	 * (ARCHER_RELEASE, frame 6 of 48) for its 12 %: 0.005 of a loop (~8 ms) after its arrow, accepted.
	 */
	HEROESOFLIBERTY_API float GetStrikeHitShare(const FHOLUnitTypeConstants& Type);

	/**
	 * Position (s, in the sequence's time) to start a strike's loop of Length seconds played at PlayRate, for its hit (at
	 * HitShare of the loop) to fall in SecondsToHit: the hit's position minus the time to it, wrapped into the loop. A loop
	 * lasting one attack interval then keeps every hit of the server's cadence on its hit frame.
	 */
	HEROESOFLIBERTY_API float GetStrikeStartPosition(float Length, float PlayRate, float SecondsToHit, float HitShare);

	/**
	 * Seconds until a unit's next hit (J7b.5), from its cool-down on the fixed step: the hit falls at the first step that
	 * takes the cool-down to 0 (HOLCombat::TickAttack), so a whole number of steps.
	 */
	HEROESOFLIBERTY_API float GetSecondsToHit(int32 CooldownMs, int32 StepMs);

	/** Sequence is one of the type's walks: the plain one or a carrying one. A loaded idle is not a walk. */
	HEROESOFLIBERTY_API bool IsWalkSequence(const FHOLUnitTypeConstants& Type, int32 Sequence);

	/**
	 * Dying (J7b.5): the type's fall, once (bOnce), at play rate 1, no prop; else striking (Attacking): the type's strike,
	 * looped at its attack cadence (bStrike: play rate = the sequence's length / AttackInterval, the caller sets its start),
	 * and its AttackProp alone (J7b: the Peasant's knife; 0, the soldiers: none). Else gathering at its source: the gesture of the resource (axe, pick, picking berries) and its tool, whatever its speed
	 * (pushed aside, it keeps working); else working at its site (J6a.4, Building; walking there is Moving): the type's
	 * building gesture (the Peasant's hammering) and its prop (the hammer), the same way, a load not shown; else walking
	 * (any activity: an order, the round trip, a push) with a load: the carrying walk of the load and the load; else
	 * standing with a load: the loaded idle of the load and the load (J5c.2: the load stays shown); else the walk or the
	 * idle by speed, as before (D234), no prop. A walk's play rate is divided
	 * by the mesh scale (a larger mesh takes longer strides) and scaled by GetWalkRateShare (J7a.4), a gesture's or an
	 * idle's is not. A type without the sequence
	 * wanted plays its idle or walk, without a prop.
	 */
	HEROESOFLIBERTY_API FHOLUnitAnimationChoice Choose(const FHOLUnitTypeConstants& Type, const FHOLUnitAnimationInput& Input);
}
