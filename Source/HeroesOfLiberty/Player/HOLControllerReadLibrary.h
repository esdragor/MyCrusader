// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLControllerReadLibrary.generated.h"

class AHOLBuilding;
class AHOLPlayerController;

/**
 * What the controller reads for the development consoles written in script (C1 step 5, SAURON-1119n, D839 "liaison a exposer"):
 * a ground point in the frame of a main centre, the selected building's place, the camera's focus, a unit birth around a point
 * (namespace HOLControllerRead in script). Wrappers only: no rule here (the cheats' routes are the script RPCs of AHOLRpcPlayer).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLControllerReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The main centre of the controller's player (or, with bEnemy, of the first other player known here), or null (the single source, also used by the C++ console commands). */
	static const AHOLBuilding* FindMainCentre(const AHOLPlayerController& Controller, bool bEnemy);

	/** A ground point (Z = 0) at (Dx, Dy) cm in the frame of the player's main centre (or an enemy's): false when no such centre is known here. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Controller")
	static bool PointNearMainCentre(AHOLPlayerController* Controller, double Dx, double Dy, bool bEnemy, FVector& OutPoint);

	/** The place of the building the controller has selected (GetSelection()): false when none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Controller")
	static bool SelectedBuildingLocation(AHOLPlayerController* Controller, FVector& OutLocation);

	/** The focus of the controller's RTS camera (where it looks): false when it has no camera yet. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Controller")
	static bool CameraFocus(AHOLPlayerController* Controller, FVector& OutFocus);

	/** Units of a slot around Center (ground), free and over the cap, through the unit subsystem of the controller's world: false on a client or without the subsystem (nothing done). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Controller")
	static bool SpawnUnitsAround(AHOLPlayerController* Controller, int32 Count, FName TypeName, int32 Slot, double Radius, const FVector& Center);
};
