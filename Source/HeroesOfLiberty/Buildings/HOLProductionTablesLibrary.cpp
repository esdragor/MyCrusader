// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLProductionTablesLibrary.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Match/HOLMatchSettings.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

bool UHOLProductionTablesLibrary::FindUnitType(const FName TypeName, FHOLUnitTypeSummary& OutRow)
{
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Row = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TypeName, TEXT("HOLProductionTablesLibrary"), false) : nullptr;
	if (!Row)
	{
		return false;
	}
	OutRow = FHOLUnitTypeSummary();
	OutRow.Cost = Row->Cost;
	OutRow.BuildTime = Row->BuildTime;
	OutRow.Population = Row->Population;
	OutRow.MaxHealth = Row->MaxHealth;
	OutRow.Attack = Row->Attack;
	OutRow.MeleeArmor = Row->MeleeArmor;
	OutRow.RangedArmor = Row->RangedArmor;
	OutRow.HeroArmor = Row->HeroArmor;
	OutRow.AttackInterval = Row->AttackInterval;
	OutRow.AttackRange = Row->AttackRange;
	OutRow.AggressionRadius = Row->AggressionRadius;
	OutRow.Speed = Row->Speed;
	OutRow.AttackType = static_cast<int32>(Row->AttackType);
	for (const EHOLArmorCategory Category : Row->ArmorCategories)
	{
		OutRow.ArmorCategories.Add(static_cast<int32>(Category));
	}
	for (const FHOLAttackBonus& Bonus : Row->Bonuses)
	{
		OutRow.BonusCategories.Add(static_cast<int32>(Bonus.Category));
		OutRow.BonusDamages.Add(Bonus.Damage);
	}
	return true;
}

bool UHOLProductionTablesLibrary::FindBuildingType(const FName TypeName, FHOLBuildingProductionSummary& OutRow)
{
	const FHOLBuildingTypeRow* Row = UHOLBuildingSettings::FindType(TypeName);
	if (!Row)
	{
		return false;
	}
	OutRow = FHOLBuildingProductionSummary();
	OutRow.Category = static_cast<int32>(Row->Category);
	OutRow.Produces = Row->Produces;
	OutRow.PopulationProvided = Row->PopulationProvided;
	OutRow.MaxQueue = Row->MaxQueue;
	OutRow.AcceptsCount = Row->Accepts.Num();
	return true;
}

FHOLResources UHOLProductionTablesLibrary::GetStartingResources()
{
	return GetDefault<UHOLMatchSettings>()->StartingResources;
}
