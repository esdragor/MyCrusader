// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLMinimapReadLibrary.generated.h"

/**
 * The minimap pure rules that stay in C++ (D836: UHOLMinimapView::AlertDiameter, HOLMinimapTerrain::GetSourcesSignature, both in
 * HUD/HOLMinimapView.h/.cpp, the Slate side) read by Angelscript (namespace HOLMinimapRead there), so that their tests
 * (HOLMinimapAlertRingTests.cpp, HOLMinimapTerrainTests.cpp) can be ported to script (socle step 1, S1h, D830, D839). Wrappers only:
 * no rule here, nothing moved out of the view. FHOLSource is not a USTRUCT, so the sources come as two parallel arrays.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLMinimapReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** UHOLMinimapView::AlertDiameter: the diameter (px) of an alert circle that is Age seconds old (Size at the start, 8 px at the end of the period, then again). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Minimap")
	static float AlertDiameter(float Age, float Period, float Size);

	/**
	 * HOLMinimapTerrain::GetSourcesSignature of the sources made of Locations[i] and Remaining[i] (same length; a shorter Remaining
	 * counts as 0 left; every source has a type, so it stands when it still holds something). The uint32 comes back as an int64 (0..2^32-1).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Minimap")
	static int64 GetSourcesSignature(const TArray<FVector>& Locations, const TArray<int32>& Remaining);

	/** The value of the float console variable Name (hol.Minimap.AlertSize...), -1 when there is no such variable (as the C++ test reads it). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Minimap")
	static float GetConsoleFloat(const FString& Name);
};