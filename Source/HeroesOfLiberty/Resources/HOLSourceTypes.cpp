// Copyright Epic Games, Inc. All Rights Reserved.

#include "Resources/HOLSourceTypes.h"

#include "HeroesOfLibertyLog.h"

const FHOLSourceTypeRow* UHOLSourceSettings::FindType(const FName TypeName)
{
	const UDataTable* Table = GetDefault<UHOLSourceSettings>()->SourceTypesTable.LoadSynchronous();
	if (!Table)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No source type table (HOLSourceSettings.SourceTypesTable in DefaultGame.ini)"));
		return nullptr;
	}
	const FHOLSourceTypeRow* Row = Table->FindRow<FHOLSourceTypeRow>(TypeName, TEXT("source type"), /*bWarnIfRowMissing*/ false);
	if (!Row)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("No source type %s in %s"), *TypeName.ToString(), *Table->GetPathName());
	}
	return Row;
}
