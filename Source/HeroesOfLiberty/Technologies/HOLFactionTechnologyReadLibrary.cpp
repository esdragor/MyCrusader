// Copyright Epic Games, Inc. All Rights Reserved.

#include "Technologies/HOLFactionTechnologyReadLibrary.h"

#include "Match/HOLFactionRules.h"
#include "Technologies/HOLTechnologyTypes.h"

bool UHOLFactionTechnologyReadLibrary::IsTechnologyOpenTo(const FName Technology, const FName FactionKey)
{
	return HOLFactionRules::IsTechnologyOpenTo(Technology, FactionKey);
}

TArray<FName> UHOLFactionTechnologyReadLibrary::GetBuildingTechnologies(const FName Building, const FName FactionKey)
{
	TArray<FName> Names;
	UHOLTechnologySettings::GetBuildingTechnologies(Building, FactionKey, Names);
	return Names;
}
