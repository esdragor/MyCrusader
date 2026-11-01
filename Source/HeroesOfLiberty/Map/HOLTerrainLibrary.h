// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLTerrainLibrary.generated.h"

/**
 * C3: the server terrain rule (FHOLTerrainRules) for Angelscript (namespace HOLTerrain:: there, the library name without U and
 * Library), so that the script own rule (Script/Terrain/HOLTerrainRules.as) is checked against it, as the talent rules are.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTerrainLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The server percent of speed at (X, Y) for one marsh zone (centre ZoneX, ZoneY, radius ZoneRadius) of strength MarshPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 ServerSpeedPercent(int32 MarshPercent, int32 ZoneX, int32 ZoneY, int32 ZoneRadius, int32 X, int32 Y);

	/** The server percent of speed at (X, Y) for one ROAD zone of speed RoadPercent (C5). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 ServerRoadSpeedPercent(int32 RoadPercent, int32 ZoneX, int32 ZoneY, int32 ZoneRadius, int32 X, int32 Y);

	/** The road speed the server starts with (percent). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetServerDefaultRoadPercent();

	/** The server scaled speed. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 ServerScaledSpeed(int32 Speed, int32 Percent);

	/** The marsh strength the server starts with (percent): the script preset 0 is checked against it. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetServerDefaultMarshPercent();
};
