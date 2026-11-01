// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The rules of hol.Test.Sanity (Auto_Journey, "Qualité des essais"): what a trial must never show, decided here from a
 * unit's recorded steps (no Mass, no world: the watcher in HOLSanity.cpp feeds it the real simulation's steps).
 * Thresholds live in FSanityRules, one place.
 */
namespace HOLSanity
{
	struct FSanityRules
	{
		/** The window (s) a spinning or a trembling is judged over. */
		double WindowSeconds = 3.0;
		/** A heading is taken once the unit has moved this far (cm) from the last anchor: smaller moves are noise, not a course. */
		double CourseSegmentCm = 40.0;
		/** Spinning: a unit walking all window long turned this much in total (degrees) ... */
		double SpinTurnDegrees = 540.0;
		/** ... and got less than this share of the length it walked (net advance / path length) closer to anywhere. */
		double SpinAdvanceShare = 0.25;
		/** Spinning needs at least this much walked (cm), so a unit that barely moves is not a spinner. */
		double SpinMinPathCm = 300.0;
		/** Trembling: a unit with no order moved more than this (cm) in a step ... */
		double ShakeStepCm = 1.0;
		/** ... in at least this share of the steps of the window ... */
		double ShakeStepShare = 0.8;
		/** ... and went nowhere (net / path under this share): it vibrates, it is not pushed along. */
		double ShakeAdvanceShare = 0.25;
	};

	/** What a unit is doing at a step, for the rules: walking to an order, standing with nothing to do, or busy (fighting, working). */
	enum class EUnitMotion : uint8
	{
		Ordered,
		Idle,
		Busy,
	};

	/** One unit's recent steps. Add one sample per simulation step; Judge says whether the window shows a fault. */
	class HEROESOFLIBERTY_API FUnitTrack
	{
	public:
		/** The unit at Time (s) at Location (XY), doing Motion. */
		void Add(double Time, const FVector2D& Location, EUnitMotion Motion, const FSanityRules& Rules);

		/** Empty while the window holds no fault; otherwise "spins" or "shakes" with the numbers. Judged once the window is full. */
		FString Judge(const FSanityRules& Rules) const;

	private:
		struct FSample
		{
			double Time = 0.;
			FVector2D Location = FVector2D::ZeroVector;
			EUnitMotion Motion = EUnitMotion::Busy;
		};
		TArray<FSample> Samples;
	};
}
