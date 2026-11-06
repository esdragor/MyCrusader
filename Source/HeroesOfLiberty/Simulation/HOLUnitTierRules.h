// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The unit tier rule (Palier3InProgress.md G1, D711; GDD 11.1): ONE rule says why a unit cannot be queued now. The simulation's twin of
 * Script/Units/HOLUnitTierRules.as (namespace HOLUnitTier there), compared with it by the script test
 * Test_UnitTierScriptAndSimulationRulesAgree. It reads the tier from the column MinTier of the unit type table. The reason numbers are
 * HOLSiege's (Available 0, TierTooLow 2, NoBuilding 3, CannotPay 4); 1 is "not in the table". No call site (production, grid, AI) uses it yet.
 */
namespace HOLUnitTier
{
	enum class EBlock : uint8
	{
		Available = 0,
		/** The unit has no tier (a type outside the table). */
		NotListed = 1,
		TierTooLow = 2,
		NoBuilding = 3,
		CannotPay = 4,
	};

	/** The MinTier of the type's row in the unit type table, -1 when the type has no row. */
	HEROESOFLIBERTY_API int32 GetMinTier(FName UnitType);

	/** Why a unit of tier MinTier cannot be queued now (a negative MinTier is NotListed): Tier is the player's, in the order tier, building, stock. */
	HEROESOFLIBERTY_API EBlock Check(int32 MinTier, int32 Tier, bool bBuildingFinished, bool bCanPay);
}
