// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLUnitOpacity.generated.h"

/**
 * The render opacity of a unit, for Angelscript (a binding to expose, D839): the stealth rule (HOLStealth::OwnUnitOpacity, Script/Fog/HOLStealthRules.as,
 * D824 Q3 B) decides the value, this only carries it to where the units are drawn. One value per unit id (1 = opaque, the default), local to this
 * machine: it is a presentation datum, never replicated. The drawing pass (UHOLInterpolatedInstancesProcessor, where a unit's instance custom data is
 * written) reads it through GetStealthOpacity. Unit ids are int32 here (script). Thread-safe (the drawing pass may run off the game thread).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitOpacityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Sets the opacity a unit is drawn with, clamped to 0..1. Setting 1 forgets the unit (back to the default). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Units")
	static void SetStealthOpacity(int32 UnitId, float Opacity);

	/** The opacity a unit is drawn with: 1 when none was set. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Units")
	static float GetStealthOpacity(int32 UnitId);

	/** Forgets every unit's opacity (end of match, tests). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Units")
	static void ClearStealthOpacities();
};
