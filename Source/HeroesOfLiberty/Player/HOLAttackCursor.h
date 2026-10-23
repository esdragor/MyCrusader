// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class SWidget;

namespace HOLAttackCursor
{
	/**
	 * The attack cursor (the user's trial of A.2, AoE4's way): a red sight, drawn by Slate (no asset), shown for
	 * EMouseCursor::Crosshairs while the attack-move waits for its point (HOLCommandGrid::GetMouseCursor). A software
	 * cursor: centred on the mouse, in Slate units (the DPI scale makes it as large on a 4K screen).
	 */
	TSharedRef<SWidget> Make();
}
