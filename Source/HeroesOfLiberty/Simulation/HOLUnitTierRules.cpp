// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitTierRules.h"

#include "Engine/DataTable.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

int32 HOLUnitTier::GetMinTier(const FName UnitType)
{
	// G1 stub (red test first): no tier read.
	return -1;
}

HOLUnitTier::EBlock HOLUnitTier::Check(const int32 MinTier, const int32 Tier, const bool bBuildingFinished, const bool bCanPay)
{
	// G1 stub (red test first): nothing stops a unit.
	return EBlock::Available;
}
