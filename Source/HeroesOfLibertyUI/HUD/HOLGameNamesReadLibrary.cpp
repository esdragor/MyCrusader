// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLGameNamesReadLibrary.h"

#include "HUD/HOLGameNames.h"

FText UHOLGameNamesReadLibrary::GetUnitName(const FName TypeName)
{
	return HOLGameNames::GetUnitName(TypeName, nullptr);
}

FText UHOLGameNamesReadLibrary::GetUnitNameForFaction(const FName TypeName, const FName FactionKey)
{
	return HOLGameNames::GetUnitName(TypeName, nullptr, FactionKey);
}

FText UHOLGameNamesReadLibrary::GetBuildingName(const FName TypeName)
{
	return HOLGameNames::GetBuildingName(TypeName, nullptr);
}
