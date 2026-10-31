// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLTerrainLayout.generated.h"

/** One round zone a map lays (C3): its kind (HOLTerrainKind), its centre on the ground and its radius in centimetres. */
USTRUCT(BlueprintType)
struct FHOLPlacedTerrainZone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Terrain")
	int32 Kind = 0;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Terrain")
	float Radius = 0.f;
};

/**
 * The terrain zones of a map (C3, El Dorado marsh), as data: the map script (Tools/Editor/maps/common.py) fills the list, the
 * server reads it when the match starts (UHOLTerrainSubsystem). Like the source layout, nothing of this actor is drawn or sent; the
 * ground under the zones is drawn by the map's own meshes.
 */
UCLASS(NotBlueprintable)
class HEROESOFLIBERTY_API AHOLTerrainLayout : public AActor
{
	GENERATED_BODY()

public:
	AHOLTerrainLayout();

	const TArray<FHOLPlacedTerrainZone>& GetZones() const { return Zones; }

private:
	UPROPERTY(EditAnywhere, Category = "Terrain")
	TArray<FHOLPlacedTerrainZone> Zones;
};
