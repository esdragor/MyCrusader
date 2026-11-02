// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/Box2D.h"

/**
 * Pure layout rules of the HUD's panels (the "panel hiding another" check of Auto_Journey.md, lesson 12b): whether two panels' boxes cover
 * each other, in the same units (Slate units, the HUD's scale). The overlap line of hol.HUD.Report reads it and preflight.ps1 fails on it.
 */
namespace HOLHUDLayout
{
	/** Whether the two boxes cover each other by more than Tolerance (units) on both axes: touching edges and a hairline do not count. */
	inline bool Overlaps(const FBox2D& A, const FBox2D& B, const float Tolerance = 1.f)
	{
		const double Across = FMath::Min(A.Max.X, B.Max.X) - FMath::Max(A.Min.X, B.Min.X);
		const double Down = FMath::Min(A.Max.Y, B.Max.Y) - FMath::Max(A.Min.Y, B.Min.Y);
		return Across > Tolerance && Down > Tolerance;
	}
}
