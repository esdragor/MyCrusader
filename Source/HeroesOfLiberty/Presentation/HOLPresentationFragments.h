// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Math/Transform.h"
#include "HOLPresentationFragments.generated.h"

/**
 * What the client has started playing on a unit's skinned instance (D234). The sequence itself lives in Mass'
 * FMassRepresentationAnimationFragment; this only tells a unit that has not been given one yet from a unit at
 * sequence 0, so the first animation starts at the unit's own random offset.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLUnitAnimationStateFragment : public FMassFragment
{
	GENERATED_BODY()

	bool bStarted = false;

	/**
	 * Gathering at its source (J5c.1), or building at its site (J6a.4): drawn turned towards it, at FacingYaw (deg), found
	 * when its gesture starts; the orientation of the simulation does not change.
	 */
	bool bFacingSource = false;
	float FacingYaw = 0.f;

	/**
	 * Falling (J7b code review, point 4): drawn for its whole fall at FallYaw (deg), the yaw it was last drawn at alive
	 * (turned towards its target if it was striking), in solo, on the host and on a client alike. FallTurnDegrees is the
	 * largest gap seen between its drawn yaw and FallYaw while falling (read by hol.Units.AnimReport only).
	 */
	bool bFalling = false;
	float FallYaw = 0.f;
	float FallTurnDegrees = 0.f;
	/** When its fall started (J7c, bodies kept 30 s: World->GetTimeSeconds()), so the body laid in its place resumes it. */
	double FallStartSeconds = 0.;

	/**
	 * What the instance was last given besides its sequence (J5c.4, read by hol.Units.AnimReport only): the prop shown and
	 * the frame it was drawn at (GFrameCounter), so a unit not drawn since (off screen) is told apart.
	 */
	uint8 Prop = 0;
	uint64 DrawnFrame = 0;
};

/**
 * A body (J7c, bodies kept 30 s): a separate entity a displaying machine lays where a unit died, with no FHOLLightUnitTag
 * nor any fragment of the simulation, so no query of the simulation, the avoidance, the selection or the replication
 * sees it. Drawn by the visualization processors (FHOLLightUnitTag or FHOLCorpseTag).
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLCorpseTag : public FMassTag
{
	GENERATED_BODY()
};

/**
 * What a body is drawn from (J7c, bodies kept 30 s): where it lies, when it was laid, and the fall it ends (HOLCorpse).
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLCorpseFragment : public FMassFragment
{
	GENERATED_BODY()

	/** Where it lies, as its unit was last drawn (the mesh's offset is the visualization trait's). */
	FTransform Base = FTransform::Identity;
	/** When it was laid (World->GetTimeSeconds()): it holds HOLCorpse::HoldSeconds, then sinks and goes. */
	double LaidAt = 0.;
	/** Position (s) of the fall its unit was at when the body was laid, its play rate and its length (s). */
	float StartPosition = 0.f;
	float PlayRate = 1.f;
	float Length = 0.f;
	/** The fall's sequence in the type's animation provider. */
	int32 Sequence = INDEX_NONE;
	/** Its unit's player (the team colour). */
	uint8 OwnerPlayer = 0;
	/** The prop its unit fell with (FHOLUnitAnimationStateFragment::Prop). */
	uint8 Prop = 0;
	/** Its unit's type (the row of the type table). */
	FName TypeName;
};
