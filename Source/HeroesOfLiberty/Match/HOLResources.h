// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLResources.generated.h"

/**
 * An amount of the four base resources (§ 5.1): a player's stock, or a cost. Written in the type tables as
 * "(Food=50,Wood=0,Stone=0,Gold=0)", like any struct column.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLResources
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Resources")
	int32 Food = 0;

	UPROPERTY(EditAnywhere, Category = "Resources")
	int32 Wood = 0;

	UPROPERTY(EditAnywhere, Category = "Resources")
	int32 Stone = 0;

	UPROPERTY(EditAnywhere, Category = "Resources")
	int32 Gold = 0;

	FHOLResources() = default;
	FHOLResources(int32 InFood, int32 InWood, int32 InStone, int32 InGold)
		: Food(InFood), Wood(InWood), Stone(InStone), Gold(InGold)
	{
	}

	/** Every resource of this stock covers the same resource of Cost. */
	bool Covers(const FHOLResources& Cost) const
	{
		return Food >= Cost.Food && Wood >= Cost.Wood && Stone >= Cost.Stone && Gold >= Cost.Gold;
	}

	FHOLResources& operator+=(const FHOLResources& Other)
	{
		Food += Other.Food;
		Wood += Other.Wood;
		Stone += Other.Stone;
		Gold += Other.Gold;
		return *this;
	}

	FHOLResources& operator-=(const FHOLResources& Other)
	{
		Food -= Other.Food;
		Wood -= Other.Wood;
		Stone -= Other.Stone;
		Gold -= Other.Gold;
		return *this;
	}

	bool operator==(const FHOLResources& Other) const = default;

	/** "F 200 W 200 S 200 G 200": for the logs of the scripted checks. */
	FString ToString() const { return FString::Printf(TEXT("F %d W %d S %d G %d"), Food, Wood, Stone, Gold); }
};
