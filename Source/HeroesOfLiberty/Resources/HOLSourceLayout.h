// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLSourceLayout.generated.h"

/**
 * One source a map places (D297): its type (a row of the source type table) and its ground point. A Blueprint type: the
 * map script makes them.
 */
USTRUCT(BlueprintType)
struct FHOLPlacedSource
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Source")
	FName Type;

	UPROPERTY(EditAnywhere, Category = "Source")
	FVector Location = FVector::ZeroVector;
};

/**
 * The resource sources of a map (D295, D297), as data: the map script (Tools/Editor/create_match_map.py) fills the list,
 * the server reads it when the match starts and keeps the sources in its registry (UHOLSourceSubsystem); nothing of
 * this actor is drawn or sent. A list rather than an actor per source: ~500 sources per map, 9 out of 10 trees.
 */
UCLASS(NotBlueprintable)
class HEROESOFLIBERTY_API AHOLSourceLayout : public AActor
{
	GENERATED_BODY()

public:
	AHOLSourceLayout();

	const TArray<FHOLPlacedSource>& GetSources() const { return Sources; }

private:
	UPROPERTY(EditAnywhere, Category = "Sources")
	TArray<FHOLPlacedSource> Sources;
};
