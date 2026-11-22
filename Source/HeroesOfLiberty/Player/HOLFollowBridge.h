// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HOLFollowBridge.generated.h"

class AHOLPlayerController;

/**
 * Follow order (GLORFINDEL-1121sk, GDD 7.3, D818 Q2 = B, D839): the controller's call into the script rule, ENVELOPES ONLY (no rule here). The single source
 * is Script/Orders/HOLFollowOrders.as (ApplyRightClick, StepUnit); the script class deriving from this one (Script/Orders/HOLFollowBridge.as) forwards to it.
 * Without the script class every call answers its neutral value (no Follow, no step): the right click on an ally keeps its ordinary path.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLFollowBridge : public UObject
{
	GENERATED_BODY()

public:
	/** The script's instance (its class's default object), nullptr without a script class. Hot reload keeps older classes: the newest is looked up again. */
	UFUNCTION(BlueprintCallable, Category = "HOL|Orders")
	static UHOLFollowBridge* Get();

	/** The bridge of this controller: an INSTANCE owned by it (GetOuter() == Controller), never the CDO. nullptr without a script class or a controller. */
	static UHOLFollowBridge* GetFor(AHOLPlayerController* Controller);

	/** True when the right click on the unit TargetId started a Follow of the selected units (no ordinary order then). False without a script class. */
	static bool NativeRightClickOnUnit(AHOLPlayerController* Controller, const TArray<int32>& Selected, int32 TargetId, bool bTargetIsAlly);
	/** One order step of the followers of the local player (called from the controller's Tick). Nothing without a script class. */
	static void NativeStep(AHOLPlayerController* Controller, int32 PlayerIndex);

	/** Any manual order of the units (right click on the ground or an enemy, Stop, Hold) ends their Follow. Nothing without a script class. */
	static void NativeManualOrder(const TConstArrayView<uint32> UnitIds);

	// The script overrides (Angelscript drops the BP_ prefix).
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Orders")
	bool BP_RightClickOnUnit(AHOLPlayerController* Controller, const TArray<int32>& Selected, int32 TargetId, bool bTargetIsAlly);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Orders")
	void BP_Step(AHOLPlayerController* Controller, int32 PlayerIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|Orders")
	void BP_ManualOrder(const TArray<int32>& UnitIds);
};
