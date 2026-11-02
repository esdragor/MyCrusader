// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Duel D5b (DuelInProgress.md): the units that are not duelists leave the duel circle by themselves. Pure rule: where a unit standing in
// the circle should walk to.
namespace HOLDuelClear
{
	/** Past the circle's edge by this much (cm) the unit is out for good. */
	constexpr double MarginCm = 200.0;

	/** True if a non-duelist at Position is to be sent out of the circle (inside its radius). */
	inline bool IsInside(const FVector2D& Position, const FVector2D& Center, const double RadiusCm)
	{
		return RadiusCm > 0.0 && FVector2D::DistSquared(Position, Center) <= RadiusCm * RadiusCm;
	}

	/** The point outside: straight out from the centre through the unit, at the radius plus the margin (to +X from the exact centre). */
	inline FVector2D GetTarget(const FVector2D& Position, const FVector2D& Center, const double RadiusCm)
	{
		FVector2D Out = Position - Center;
		if (!Out.Normalize())
		{
			Out = FVector2D(1.0, 0.0);
		}
		return Center + Out * (RadiusCm + MarginCm);
	}
}
