// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLTerrainReadLibrary.generated.h"

/**
 * The terrain rules of the forest (F4), the ford (F5) and the marsh (D810; FHOLTerrainRules, Map/HOLTerrainRules.h, C++, same module as
 * this library) for Angelscript (namespace HOLTerrainRead there). D836: the rule stays in C++ (the Mass step calls it for every walker)
 * and the script reads it here, so that Script/Terrain/HOLTerrainRules.as holds no second copy. Wrappers only: no rule here.
 * A list of zones is a flat TArray<int32> of quadruples (Kind, X, Y, Radius), in whole centimetres (FHOLTerrainZone, a plain C++ struct
 * that cannot cross to the script); a list whose length is not a multiple of 4 ignores its last incomplete zone.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTerrainReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLTerrainKind::Marsh (0). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetMarshKind();

	/** HOLTerrainKind::Road (1). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetRoadKind();

	/** HOLTerrainKind::Forest (3). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetForestKind();

	/** HOLTerrainKind::Ford (4). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetFordKind();

	/** HOLTerrainWalker::OnFoot (0). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetWalkerOnFoot();

	/** HOLTerrainWalker::Cavalry (1). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetWalkerCavalry();

	/** HOLTerrainWalker::Siege (2). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetWalkerSiege();

	/** FHOLTerrainRules::DefaultMarshSpeedPercent (60). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetDefaultMarshSpeedPercent();

	/** FHOLTerrainRules::DefaultRoadSpeedPercent (130). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetDefaultRoadSpeedPercent();

	/** FHOLTerrainRules::DefaultForestCavalrySpeedPercent (100, D844). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetDefaultForestCavalrySpeedPercent();

	/** FHOLTerrainRules::DefaultForestSiegeSpeedPercent (100, D844). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetDefaultForestSiegeSpeedPercent();

	/** FHOLTerrainRules::DefaultFordDamageTakenPercent (120). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetDefaultFordDamageTakenPercent();

	/** FHOLTerrainRules::GetForestSpeedPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetForestSpeedPercent(int32 Category, bool bInForest, int32 CavalryPercent, int32 SiegePercent);

	/** FHOLTerrainRules::GetWalkerSpeedPercentAt over the zones (quadruples Kind, X, Y, Radius). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetWalkerSpeedPercentAt(int32 MarshPercent, const TArray<int32>& ZoneQuadruples, int32 Category, int32 X, int32 Y, int32 RoadPercent);

	/** FHOLTerrainRules::IsInZoneOfKind over the zones (quadruples Kind, X, Y, Radius). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static bool IsInZoneOfKind(const TArray<int32>& ZoneQuadruples, int32 Kind, int32 X, int32 Y);

	/** FHOLTerrainRules::GetFordDamageTakenPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetFordDamageTakenPercent(bool bOnFord, int32 FordPercent);

	/** FHOLTerrainRules::GetFordDamageTaken. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static int32 GetFordDamageTaken(int32 Damage, bool bOnFord, int32 FordPercent);

	/** FHOLTerrainRules::StopsCharge over the zones (quadruples Kind, X, Y, Radius). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Terrain")
	static bool StopsCharge(bool bMarshStopsCharge, const TArray<int32>& ZoneQuadruples, int32 X, int32 Y);
};
