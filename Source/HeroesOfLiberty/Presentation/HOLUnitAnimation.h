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

	/** Sequence is one of the type's walks: the plain one or a carrying one. A loaded idle is not a walk. */
	HEROESOFLIBERTY_API bool IsWalkSequence(const FHOLUnitTypeConstants& Type, int32 Sequence);

	/**
	 * Gathering at its source: the gesture of the resource (axe, pick, picking berries) and its tool, whatever its speed
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
