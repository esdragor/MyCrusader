// Copyright Epic Games, Inc. All Rights Reserved.

#include "Technologies/HOLTechnologyTypes.h"

const UDataTable* UHOLTechnologySettings::GetTable()
{
	const UHOLTechnologySettings* Settings = GetDefault<UHOLTechnologySettings>();
	return Settings->TechnologiesTable.IsNull() ? nullptr : Settings->TechnologiesTable.LoadSynchronous();
}

const FHOLTechnologyRow* UHOLTechnologySettings::FindTechnology(const FName Name)
{
	const UDataTable* Table = GetTable();
	return Table ? Table->FindRow<FHOLTechnologyRow>(Name, TEXT("technology"), /*bWarnIfRowMissing*/ false) : nullptr;
}

void UHOLTechnologySettings::GetBuildingTechnologies(const FName Building, TArray<FName>& OutNames)
{
	GetBuildingTechnologies(Building, NAME_None, OutNames);
}

void UHOLTechnologySettings::GetBuildingTechnologies(const FName Building, const FName FactionKey, TArray<FName>& OutNames)
{
	OutNames.Reset();
	const UDataTable* Table = GetTable();
	if (!Table)
	{
		return;
	}
	for (const FName Name : Table->GetRowNames())
	{
		const FHOLTechnologyRow* Row = Table->FindRow<FHOLTechnologyRow>(Name, TEXT("technology"), /*bWarnIfRowMissing*/ false);
		if (Row && Row->Building == Building)
		{
			OutNames.Add(Name);
		}
	}
}
