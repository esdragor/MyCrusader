// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLFogMode.generated.h"

/** D522: the fog of a match (the menu offers the three; the manager's lobby names them: Normal, Explored, Disabled). */
UENUM()
enum class EHOLFogMode : uint8
{
	/** Everything in sight (« Sans brouillard »): the trials, the measures, the tests' default. */
	Disabled,
	/** D389: the map explored from the start (terrain and resources), the enemy's units and buildings hidden out of sight. */
	Explored,
	/** D381: the map black at the start. The game's default (F8). */
	Normal,
};
