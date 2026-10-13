// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/** Collision channels of the project ([/Script/Engine.CollisionProfile] in DefaultEngine.ini). */
namespace HOLCollision
{
	/** Trace channel of the selection under the cursor (D262): ignored by default, blocked by the pieces of buildings. */
	constexpr ECollisionChannel Selection = ECC_GameTraceChannel1;
}
