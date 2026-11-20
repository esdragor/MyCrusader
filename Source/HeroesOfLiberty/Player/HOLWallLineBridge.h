// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HOLWallLineBridge.generated.h"

/**
 * F4 (HALDIR-1120j, D839): the controller's read of the wall-line rule for the drag gesture, ENVELOPES ONLY (no rule here). The rule's single
 * source is Script/Buildings/HOLWallLineRules.as (HOLWallLine::IsWallType, DragCellsFromPoints, MaxLineCells); the script class deriving
 * from this one (Script/Buildings/HOLWallLineBridge.as) forwards to it. Without the script class every call answers its neutral value
 * (no wall type, no cell, a cap of 0): the gesture never arms.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLWallLineBridge : public UObject
{
	GENERATED_BODY()

public:
	/** The script's instance (its class's default object), nullptr without a script class. Hot reload keeps older classes: the newest is looked up again. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Walls")
	static UHOLWallLineBridge* Get();

	/** True for a type of HOLWallLine::WallTypes() (empty until F5). */
	static bool NativeIsWallType(FName TypeName);
	/** The cells the drag from Start to End shows (the cells Validate lays when it accepts), one per Tile of the grid. */
	static TArray<FIntPoint> NativeDragCells(const FVector& Start, const FVector& End, double Tile);

	// The script overrides (Angelscript drops the BP_ prefix).
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Walls")
	bool BP_IsWallType(FName TypeName);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Walls")
	TArray<FIntPoint> BP_DragCells(FVector Start, FVector End, double Tile);
};