// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLGameNamesReadLibrary.generated.h"

/**
 * The names of the unit and building types as the player reads them (HOLGameNames, HUD/HOLGameNames.h, C++, same module as this
 * library) for Angelscript (namespace HOLGameNamesRead there): D836 keeps the rule in C++ and the script reads it here, so that the
 * test of the rule (HOLGameNamesTests.cpp) can be ported to script (socle step 1, D830, D839). Wrappers only: no rule here. The
 * table row of the rule is always passed null (as its test does): the name is chosen from the type key.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLGameNamesReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLGameNames::GetUnitName(TypeName, nullptr): the localized name of a unit type; a type unknown to the rule shows its key. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|GameNames")
	static FText GetUnitName(FName TypeName);

	/** HOLGameNames::GetUnitName(TypeName, nullptr, FactionKey): the name for a faction (None or a faction without its own: the base name). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|GameNames")
	static FText GetUnitNameForFaction(FName TypeName, FName FactionKey);

	/** HOLGameNames::GetBuildingName(TypeName, nullptr). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|GameNames")
	static FText GetBuildingName(FName TypeName);
};
