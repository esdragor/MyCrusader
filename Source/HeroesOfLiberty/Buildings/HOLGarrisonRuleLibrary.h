// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLGarrisonRuleLibrary.generated.h"

/**
 * The C++ twin of the garrison rule (HOLGarrisonRules.h, the one UHOLGarrisonSubsystem applies) for Angelscript (namespace HOLGarrisonRule
 * there), so that Script/Tests/HOLGarrisonParity_Test.as checks that both copies admit the same units on the same table (F7). Kinds are the
 * script's numbers (KindMeleeInfantry 0 .. KindHero 4); any other number is a kind that never enters, as in the script.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLGarrisonRuleLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLGarrison::Admitted with the three kind settings given. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Garrison")
	static TArray<int32> AdmittedWith(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, int32 BuildingSlot, int32 Slots, int32 Occupants,
		bool bMeleeInfantry, bool bHero, bool bCavalry);

	/** HOLGarrison::AdmittedBySettings: the three settings read by name, as the server does. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Garrison")
	static TArray<int32> AdmittedBySettings(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, int32 BuildingSlot, int32 Slots, int32 Occupants);
};
