// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"

#include "HeroesOfLibertyLog.h"

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
