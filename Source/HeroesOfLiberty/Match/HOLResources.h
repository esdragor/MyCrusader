// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLResources.generated.h"

/** One of the four base resources (§ 5.1): what a source gives and what a Peasant carries (J5, D295). */
UENUM()
enum class EHOLResource : uint8
{
	Food,
	Wood,
	Stone,
	Gold,
};

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

	/** The amount of one resource. */
	int32& operator[](const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return Wood;
		case EHOLResource::Stone: return Stone;
		case EHOLResource::Gold: return Gold;
		default: return Food;
		}
	}

	int32 operator[](const EHOLResource Resource) const { return const_cast<FHOLResources&>(*this)[Resource]; }

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

/**
 * A rate (per second) for each of the four base resources: how fast a unit type gathers each of them (J5, D296; the
 * worker variants of D66, D91 and D39 change only these). Written "(Food=0.7,Wood=0.7,Stone=0.7,Gold=0.7)" in the table.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLResourceRates
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0"))
	float Food = 0.f;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0"))
	float Wood = 0.f;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0"))
	float Stone = 0.f;

	UPROPERTY(EditAnywhere, Category = "Resources", meta = (ClampMin = "0"))
	float Gold = 0.f;

	FHOLResourceRates() = default;
	FHOLResourceRates(float InFood, float InWood, float InStone, float InGold)
		: Food(InFood), Wood(InWood), Stone(InStone), Gold(InGold)
	{
	}

	float operator[](const EHOLResource Resource) const
	{
		switch (Resource)
		{
		case EHOLResource::Wood: return Wood;
		case EHOLResource::Stone: return Stone;
		case EHOLResource::Gold: return Gold;
		default: return Food;
		}
	}

	bool operator==(const FHOLResourceRates& Other) const = default;
};
