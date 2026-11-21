// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLUnitScriptLibrary.generated.h"

/**
 * Where a unit is and whether it lives, for Angelscript (a binding to expose, D839): what HOLFollowOrders.as needs to read the followed unit and the
 * follower at each order step (Auto_Journey_File.md l.118). It reads HOLUnitView::Find, so it answers the same on the server, in solo and on a client
 * (the units this machine knows). Unit ids are int32 here (script). A unit Dying or garrisoned is not found: reported as not alive.
 * A rule has no copy: this only carries.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLUnitScriptLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** True when the unit is known to this machine and alive; its position (cm) in OutLocation. False: unknown id, dying, garrisoned or no world (OutLocation set to zero). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Units", meta = (WorldContext = "WorldContextObject"))
	static bool GetUnitLocation(const UObject* WorldContextObject, int32 UnitId, FVector& OutLocation);

	/** True when the unit is known to this machine and alive (not dying, not garrisoned). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Units", meta = (WorldContext = "WorldContextObject"))
	static bool IsUnitAlive(const UObject* WorldContextObject, int32 UnitId);
};
