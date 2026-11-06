// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitTierRules.h"

#include "Engine/DataTable.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

int32 HOLUnitTier::GetMinTier(const FName UnitType)
{
	const UDataTable* Table = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Row = Table ? Table->FindRow<FHOLUnitTypeRow>(UnitType, TEXT("unit tier"), /*bWarnIfRowMissing*/ false) : nullptr;
	return Row ? Row->MinTier : -1;
}

HOLUnitTier::EBlock HOLUnitTier::Check(const int32 MinTier, const int32 Tier, const bool bBuildingFinished, const bool bCanPay)
{
	if (MinTier < 0)
	{
		return EBlock::NotListed;
	}
	if (Tier < MinTier)
	{
		return EBlock::TierTooLow;
	}
	if (!bBuildingFinished)
	{
		return EBlock::NoBuilding;
	}
	if (!bCanPay)
	{
		return EBlock::CannotPay;
	}
	return EBlock::Available;
}
