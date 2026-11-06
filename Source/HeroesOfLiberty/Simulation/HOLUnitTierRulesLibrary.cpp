// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitTierRulesLibrary.h"

#include "Simulation/HOLUnitTierRules.h"

int32 UHOLUnitTierRulesLibrary::CheckUnit(const FName UnitType, const int32 Tier, const bool bBuildingFinished, const bool bCanPay)
{
	return static_cast<int32>(HOLUnitTier::Check(HOLUnitTier::GetMinTier(UnitType), Tier, bBuildingFinished, bCanPay));
}

int32 UHOLUnitTierRulesLibrary::GetUnitMinTier(const FName UnitType)
{
	return HOLUnitTier::GetMinTier(UnitType);
}
