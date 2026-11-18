// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitTierRulesLibrary.h"

#include "Simulation/HOLUnitTierSubsystem.h"

int32 UHOLUnitTierRulesLibrary::GetUnitMinTier(const FName UnitType)
{
	return UHOLUnitTierSubsystem::GetMinTier(UnitType);
}
