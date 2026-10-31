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
