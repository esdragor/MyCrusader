// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
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
};
