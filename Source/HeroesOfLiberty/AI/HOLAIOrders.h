// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAIOrders.generated.h"

/**
 * What a unit or building type costs and does, read from the type tables (BuildingTypes.csv, UnitTypes.csv) for the AI's rules
 * (IA1.6): the numbers the script used to copy. Costs are the four resources; BuildTime in seconds; Population is the places a unit
 * takes or a building gives; the gathering fields are the unit's (0 for a building).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAITypeFigures
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostFood = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostWood = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostStone = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostGold = 0;

	/** Seconds one builder (a building) or the centre (a unit) needs. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float BuildTime = 0.f;

	/** Places a unit takes, or a building gives. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Population = 0;

	/** Walk speed (cm/s) of a unit. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float Speed = 0.f;

	/** What a gatherer carries before it goes back to the deposit. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CarryCapacity = 0;

	/** Resources gathered per second at the source (the unit's rates). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherFood = 0.f;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherWood = 0.f;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherStone = 0.f;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	float GatherGold = 0.f;
};

/**
 * An AI slot's orders for Blueprint and Angelscript (IA0.2, IA0.4's brain): the same queue and the same limits as a
 * human's (HOLPlayerOrders), without a controller. Server and solo only (the authority): refused on a client. Unit
 * identifiers as the summary gives them (FHOLAISummary::IdlePeasants: int32, script has no uint32).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAIOrderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * PlayerIndex's units gather the source SourceIndex (FHOLAISourceNear::SourceIndex), applied at the next step; its
	 * soldiers among them walk to it. Whether the order entered the queue.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIGatherOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, int32 SourceIndex, bool bQueue = false);

	/** PlayerIndex's units go to Target, applied at the next step. Whether the order entered the queue. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIMoveOrder(const UObject* WorldContextObject, int32 PlayerIndex, const TArray<int32>& UnitIds, FVector Target, bool bQueue = false);

	// IA1.4: building, production. A building is named by its ground point (FHOLAIBuilding::Location): the summary gives no
	// identifier, the nearest building of the slot within 100 cm is meant.

	/** Why a foundation of TypeName cannot go at Point for PlayerIndex (EHOLPlacementRefusal as an integer: 0 = it can), -1 for an unknown type. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static int32 CheckAIPlacement(const UObject* WorldContextObject, int32 PlayerIndex, FName TypeName, FVector Point);

	/**
	 * The free spot of TypeName nearest Centre, searched in rings of tiles around it (ring 0 touching, up to MaxRings), by the
	 * rules of a player's placement (CheckPlacement: map, buildings, sources, enemy centres, navmesh).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool FindAIPlacement(const UObject* WorldContextObject, int32 PlayerIndex, FName TypeName, FVector Centre, int32 MaxRings, FVector& OutPoint);

	/** A Mill with the 8 Farms around it (IA1.2): the first block, nearest Centre, where all nine are free. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool FindAIFarmBlock(const UObject* WorldContextObject, int32 PlayerIndex, FVector Centre, int32 MaxRings, FVector& OutMill, TArray<FVector>& OutFarms);

	/** PlayerIndex places a foundation of TypeName at Point (cost paid at the next step), UnitIds go and build it. False when the placement is refused. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIPlaceOrder(const UObject* WorldContextObject, int32 PlayerIndex, FName TypeName, FVector Point, const TArray<int32>& UnitIds, bool bQueue = false);

	/** PlayerIndex's units build (or repair) its building at BuildingLocation. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIBuildOrder(const UObject* WorldContextObject, int32 PlayerIndex, FVector BuildingLocation, const TArray<int32>& UnitIds, bool bQueue = false);

	/** One unit of TypeName queued at PlayerIndex's building at BuildingLocation. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAIProduceOrder(const UObject* WorldContextObject, int32 PlayerIndex, FVector BuildingLocation, FName TypeName);

	/** The rally point of PlayerIndex's building at BuildingLocation: Point, or the source SourceIndex (INDEX_NONE: the ground). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool IssueAISetRallyOrder(const UObject* WorldContextObject, int32 PlayerIndex, FVector BuildingLocation, FVector Point, int32 SourceIndex = -1);

	// IA1.6: the type tables, read as the human's panels read them.

	/** The figures of a unit type (UnitTypes.csv); false for an unknown type. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool ReadAIUnitFigures(const UObject* WorldContextObject, FName TypeName, FHOLAITypeFigures& OutFigures);

	/** The figures of a building type (BuildingTypes.csv); false for an unknown type. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static bool ReadAIBuildingFigures(FName TypeName, FHOLAITypeFigures& OutFigures);
};
