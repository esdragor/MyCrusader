// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLHUDLayoutReadLibrary.generated.h"

/** The edge-triggered state of the cheats panel fold (HOLHUDLayout::ShouldFoldCheats keeps it between two ticks), and its answer. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTYUI_API FHOLHUDFoldState
{
	GENERATED_BODY()

	/** The window was low (in: the previous tick's; out: this tick's). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|HUD")
	bool bWasLow = false;

	/** The hero block was shown (in: the previous tick's; out: this tick's). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|HUD")
	bool bWasShown = false;

	/** Out only: true on the tick the cheats panel must fold. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|HUD")
	bool bFold = false;
};

/**
 * The HUD pure layout rules (HOLHUDLayout, HUD/HOLHUDLayoutRules.h, header only, C++, same module as this library) for Angelscript
 * (namespace HOLHUDLayoutRead there): D836 keeps the rule in C++ and the script reads it here, so that the test of the rule
 * (HOLHUDLayoutTests.cpp) can be ported to script (socle step 1, D830, D839). Wrappers only: no rule here. A box is given by its two
 * corners (FVector2D Min, Max: the FBox2D of the rule is built here).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLHUDLayoutReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLHUDLayout::Overlaps: whether the two boxes cover each other by more than Tolerance (units) on both axes (the rule's default is 1). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static bool Overlaps(const FVector2D& AMin, const FVector2D& AMax, const FVector2D& BMin, const FVector2D& BMax, float Tolerance);

	/** HOLHUDLayout::LowWindowHeight: the height (px) under which the window is low. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static float GetLowWindowHeight();

	/** HOLHUDLayout::IsLowWindow: a window lower than LowWindowHeight; a height not known yet (0) is not low. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static bool IsLowWindow(float HeightPx);

	/** HOLHUDLayout::CheatsStartOpen: whether the cheats panel starts open (the kept choice), which a low window does not. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static bool CheatsStartOpen(bool bSavedOpen, float HeightPx);

	/**
	 * HOLHUDLayout::ShouldFoldCheats: Previous holds the previous tick bWasLow and bWasShown (bFold ignored); the result holds this
	 * tick bWasLow and bWasShown (to give back next tick) and bFold, the answer of the rule.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|HUD")
	static FHOLHUDFoldState ShouldFoldCheats(float HeightPx, bool bHeroBlockShown, bool bCheatsOpen, const FHOLHUDFoldState& Previous);
};
