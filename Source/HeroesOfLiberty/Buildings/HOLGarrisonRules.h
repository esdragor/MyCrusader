// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Garrison, what the C++ keeps of the rule (F7; D693, D374, D793, D836 point 4). The rule itself (who enters: the building's own slot, an
 * admitted kind, the free places, the settings hol.Garrison.MeleeInfantry / Hero / Cavalry) is the script's alone, Script/Garrison/HOLGarrisonRules.as
 * (HOLGarrison::Admitted), tested by Script/Tests/HOLGarrisonRules_Test.as and asked by the server through UHOLGarrisonBoard. Here only the
 * kind of a unit, read from Mass fragments the script cannot see (UHOLGarrisonSubsystem feeds the board with it).
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
}