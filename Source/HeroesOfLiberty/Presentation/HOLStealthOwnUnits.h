// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLStealthOwnUnits.generated.h"

/**
 * Stealth binding for Angelscript (D824 Q3 B, a binding to expose, D839): the simulation identifiers and positions of ONE player's living units, so the
 * script (HOLStealthApply::ApplyOwnUnits, Script/Fog/HOLStealthOpacityApply.as) can pose the half-opacity of the local player's own units in a stealth
 * zone. The rule stays in script (HOLStealth::OwnUnitOpacity); this only reads. The identifier is the one the drawing pass reads
 * (FHOLUnitStateFragment::UnitId, converted to int32 for script, as the opacity table's key). No WorldContext meta (the script wrappers have none):
 * the world is passed as an object.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLStealthOwnUnitsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * The living units (not dying, health above 0) owned by the slot Slot (an owner player index): OutUnitIds[i] is the simulation identifier of the
	 * unit whose position (X, Y) is OutPositions[i]. Both arrays are cleared first; the other players' units are never listed. On a client the
	 * units read are the ones received (UHOLUnitReceptionSubsystem), on the server and in solo the simulation's (UHOLUnitSubsystem). One walk of
	 * the units per call: the caller paces it.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Stealth")
	static void GetPlayerUnits(const UObject* WorldObject, int32 Slot, TArray<int32>& OutUnitIds, TArray<FVector2D>& OutPositions);

	/** The slot of the local player (its AHOLPlayerState index); INDEX_NONE until its state has come, or without a world or a controller. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Stealth")
	static int32 GetLocalPlayerSlot(const UObject* WorldObject);
};
