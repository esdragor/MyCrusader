// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLMapBounds.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "HeroesOfLibertyLog.h"

AHOLMapBounds::AHOLMapBounds()
{
	Area = CreateDefaultSubobject<UBoxComponent>(TEXT("Area"));
	Area->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Area->SetHiddenInGame(true);
	Area->SetBoxExtent(FVector(20000.f, 20000.f, 1000.f));
	RootComponent = Area;
}

FBox2D AHOLMapBounds::GetArea() const
{
	// From the box itself rather than its cached bounds, so that it holds before the component is registered.
	const FVector2D Centre(Area->GetComponentLocation());
	const FVector2D HalfSize(Area->GetScaledBoxExtent());
	return FBox2D(Centre - HalfSize, Centre + HalfSize);
}

AHOLMapBounds* AHOLMapBounds::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	// A map without bounds (the stress test map, D314's S7 placing its camps) has none: no actor, no assert.
	const TActorIterator<AHOLMapBounds> It(const_cast<UWorld*>(World));
	return It ? *It : nullptr;
}

bool AHOLMapBounds::FitsMaxSize(const FBox2D& InArea)
{
	// 1 cm of tolerance: a box drawn to the edge in the editor may be a hair over.
	const double Half = 0.5 * MaxSideTiles * UHOLBuildingSettings::GetTileSize() + 1.0;
	return InArea.bIsValid && InArea.Min.GetMin() >= -Half && InArea.Max.GetMax() <= Half;
}

void AHOLMapBounds::BeginPlay()
{
	Super::BeginPlay();
	const FBox2D MapArea = GetArea();
	if (!FitsMaxSize(MapArea))
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Map bounds %s go past the largest map, %d x %d tiles of %.0f cm centred on the origin (D344)"),
			*MapArea.ToString(), MaxSideTiles, MaxSideTiles, UHOLBuildingSettings::GetTileSize());
	}
}
