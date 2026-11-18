// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLStartingPeasantsReadLibrary.generated.h"

/**
 * Where the King and the starting Peasants are born (FHOLStartRules, Match/HOLStartRules.h, C++, applied by the server) for
 * Angelscript (namespace HOLStartingPeasantsRead there): the "Locations" part of HOLStartingPeasantsTests.cpp (pure geometry). The
 * "Count" part reads Tools/Scenarios/*.json and a CDO and stays in C++. D836, D839. Wrappers only: no rule here.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLStartingPeasantsReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLStartRules::GetKingLocation: where the King is born, by its main forum (at CentreLocation, its body HalfBody cm from the centre to its edge). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|StartRules")
	static FVector GetKingLocation(const FVector& CentreLocation, float HalfBody);

	/** FHOLStartRules::GetStartingPeasantLocations: where the Num starting Peasants of a slot are born (one row in front of the forum). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|StartRules")
	static TArray<FVector> GetStartingPeasantLocations(const FVector& CentreLocation, float HalfBody, int32 Num);
};
