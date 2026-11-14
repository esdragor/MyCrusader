// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypeLibrary.h"

#include "Engine/DataTable.h"

bool UHOLBuildingTypeLibrary::FindRow(const FName TypeName, FHOLBuildingTypeRow& OutRow)
{
	const UDataTable* Table = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	if (!Table)
	{
		return false;
	}
	// Quiet on purpose: the garrison rule asks for tower names that have no row yet (F8, F10, F12) and falls back on its settings.
	const FHOLBuildingTypeRow* Row = Table->FindRow<FHOLBuildingTypeRow>(TypeName, TEXT("building type"), /*bWarnIfRowMissing*/ false);
	if (!Row)
	{
		return false;
	}
	OutRow = *Row;
	return true;
}
