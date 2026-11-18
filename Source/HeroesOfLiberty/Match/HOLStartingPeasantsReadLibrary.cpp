// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLStartingPeasantsReadLibrary.h"

#include "Match/HOLStartRules.h"

FVector UHOLStartingPeasantsReadLibrary::GetKingLocation(const FVector& CentreLocation, const float HalfBody)
{
	return FHOLStartRules::GetKingLocation(CentreLocation, HalfBody);
}

TArray<FVector> UHOLStartingPeasantsReadLibrary::GetStartingPeasantLocations(const FVector& CentreLocation, const float HalfBody, const int32 Num)
{
	return FHOLStartRules::GetStartingPeasantLocations(CentreLocation, HalfBody, Num);
}
