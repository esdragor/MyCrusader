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

/**
 * The per-instance custom data of a drawn unit (what UHOLInterpolatedInstancesProcessor hands to Mass, one float per component and the same count
 * for every unit and corpse of a component): colour in 0-2 (M_UnitPlayerColor), the prop shown in 3 (M_UnitPlayerColorProps), the stealth opacity
 * in 4 (D824 Q3: 0.5 for a unit in a zone, 1 otherwise). Indexes 0-3 never move: the material reads them. Plain floats (Mass copies the struct at any
 * float offset).
 */
namespace HOLUnitDraw
{
	constexpr int32 NumInstanceCustomFloats = 5;

	struct FInstanceCustomData
	{
		FVector3f Color;
		float Prop;
		float Opacity;
	};
	static_assert(sizeof(FInstanceCustomData) == NumInstanceCustomFloats * sizeof(float));

	/** The custom data with an explicit opacity (the bodies: 1). */
	HEROESOFLIBERTY_API FInstanceCustomData MakeInstanceCustomData(const FVector3f& Color, uint8 Prop, float Opacity);

	/** The custom data of a living unit: its opacity is UHOLUnitOpacityLibrary::GetStealthOpacity(UnitId). Called once per unit and frame by the drawing pass. */
	HEROESOFLIBERTY_API FInstanceCustomData MakeUnitInstanceCustomData(const FVector3f& Color, uint8 Prop, int32 UnitId);
}
