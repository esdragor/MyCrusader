// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"

#include "Buildings/HOLConstructionRules.h"
#include "HeroesOfLibertyLog.h"

FVector2D UHOLBuildingSettings::GetBodySize(const FVector2D& Footprint)
{
	return HOLConstruction::GetBodySize(Footprint, GetBodyInset());
}

FVector2D UHOLBuildingSettings::GetVisualSize(const FVector2D& Footprint)
{
	return HOLConstruction::GetBodySize(Footprint, GetVisualInset());
}

const FHOLBuildingTypeRow* UHOLBuildingSettings::FindType(const FName TypeName)
{
	const UDataTable* Table = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No building type table (HOLBuildingSettings.BuildingTypesTable in DefaultGame.ini)"));
		return nullptr;
	}
	const FHOLBuildingTypeRow* Row = Table->FindRow<FHOLBuildingTypeRow>(TypeName, TEXT("building type"), /*bWarnIfRowMissing*/ false);
	if (!Row)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No building type %s in %s"), *TypeName.ToString(), *Table->GetPathName());
	}
	return Row;
}

void UHOLBuildingSettings::GetBuildableTypes(TArray<FHOLBuildable>& OutTypes)
{
	OutTypes.Reset();
	const UDataTable* Table = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	if (!Table)
	{
		return;
	}
	Table->ForeachRow<FHOLBuildingTypeRow>(TEXT("buildable types"), [&OutTypes](const FName& Name, const FHOLBuildingTypeRow& Row)
	{
		if (Row.IsBuildable())
		{
			OutTypes.Add({ Name, Row.BuildGridSlot, Row.Age });
		}
	});
}
