// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLMapBounds.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "HeroesOfLibertyLog.h"
#include "HAL/IConsoleManager.h"

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

FBox2D AHOLMapBounds::GetPlayableArea() const
{
	const FBox2D Whole = GetArea();
	const FVector2D Inset(FMath::Min(PlayInset, 0.5 * (Whole.Max.X - Whole.Min.X)), FMath::Min(PlayInset, 0.5 * (Whole.Max.Y - Whole.Min.Y)));
	return FBox2D(Whole.Min + Inset, Whole.Max - Inset);
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

namespace
{
	/** hol.Map.ShowBounds [0|1]: the playable area drawn as a red rectangle on the ground (for the trials of the map's edge), 0 removes it. */
	FAutoConsoleCommandWithWorldAndArgs CommandShowBounds(TEXT("hol.Map.ShowBounds"),
		TEXT("hol.Map.ShowBounds [0|1]: draws the walkers' playable area (AHOLMapBounds::GetPlayableArea) as a red rectangle, 0 removes it"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (!World)
			{
				return;
			}
			FlushPersistentDebugLines(World);
			if (!Args.IsEmpty() && Args[0] == TEXT("0"))
			{
				return;
			}
			const AHOLMapBounds* Bounds = AHOLMapBounds::Find(World);
			if (!Bounds)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("hol.Map.ShowBounds: no map bounds"));
				return;
			}
			const FBox2D Area = Bounds->GetPlayableArea();
			const float Z = 40.f;
			const FVector Corners[4] = { FVector(Area.Min.X, Area.Min.Y, Z), FVector(Area.Max.X, Area.Min.Y, Z), FVector(Area.Max.X, Area.Max.Y, Z), FVector(Area.Min.X, Area.Max.Y, Z) };
			for (int32 Side = 0; Side < 4; ++Side)
			{
				DrawDebugLine(World, Corners[Side], Corners[(Side + 1) % 4], FColor::Red, true, -1.f, SDPG_World, 25.f);
			}
			UE_LOG(LogHOLMatch, Display, TEXT("Map bounds shown: %s"), *Area.ToString());
		}));
}
