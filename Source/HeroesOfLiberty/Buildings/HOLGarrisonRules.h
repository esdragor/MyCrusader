// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Garrison, the rule the server applies to an order (F7 lot 2, FortificationsInProgress.md; D693, D374, D793). The C++ twin of
 * Script/Garrison/HOLGarrisonRules.as (HOLGarrison:: there, tested by Script/Tests/HOLGarrisonRules_Test.as): the order is applied by the
 * simulation (UHOLGarrisonSubsystem), which cannot call the script, as HOLUnitAbilityRules.h for the abilities. Same kinds, same order, same
 * settings (the script's hol.Garrison.MeleeInfantry / Hero / Cavalry, read by name). Keep both in step.
 */
namespace HOLGarrison
{
	/** The script's KindMeleeInfantry .. KindHero, same numbers. */
	enum class EKind : uint8
	{
		MeleeInfantry = 0,
		RangedInfantry = 1,
		Cavalry = 2,
		Siege = 3,
		Hero = 4,
	};

	/** A unit's kind from its type: siege first (the Ram, the Mangonel), then the hero, the cavalry, the shooters, the rest on foot. */
	inline EKind KindOf(const bool bSiege, const bool bHero, const bool bCavalry, const bool bRanged)
	{
		return bSiege ? EKind::Siege : bHero ? EKind::Hero : bCavalry ? EKind::Cavalry : bRanged ? EKind::RangedInfantry : EKind::MeleeInfantry;
	}

	/** Siege never enters (D793); ranged foot always do; the three others as their setting says. */
	inline bool KindMayEnter(const EKind Kind, const bool bMeleeInfantry, const bool bHero, const bool bCavalry)
	{
		switch (Kind)
		{
		case EKind::RangedInfantry: return true;
		case EKind::MeleeInfantry: return bMeleeInfantry;
		case EKind::Hero: return bHero;
		case EKind::Cavalry: return bCavalry;
		default: return false;
		}
	}

	/**
	 * Indexes (into UnitSlots / UnitKinds, in the order asked) of the candidates that enter a building of BuildingSlot with Slots places and
	 * Occupants already inside: only the building's own slot (an ally's, D374, and an enemy's refused), only an admitted kind, up to the free
	 * places; a refused candidate takes no place.
	 */
	inline TArray<int32> Admitted(const TConstArrayView<int32> UnitSlots, const TConstArrayView<EKind> UnitKinds, const int32 BuildingSlot,
		const int32 Slots, const int32 Occupants, const bool bMeleeInfantry, const bool bHero, const bool bCavalry)
	{
		TArray<int32> In;
		const int32 Free = FMath::Max(0, Slots - FMath::Max(0, Occupants));
		for (int32 Index = 0; Index < UnitSlots.Num() && In.Num() < Free; ++Index)
		{
			if (UnitKinds.IsValidIndex(Index) && BuildingSlot >= 0 && UnitSlots[Index] == BuildingSlot
				&& KindMayEnter(UnitKinds[Index], bMeleeInfantry, bHero, bCavalry))
			{
				In.Add(Index);
			}
		}
		return In;
	}
}
