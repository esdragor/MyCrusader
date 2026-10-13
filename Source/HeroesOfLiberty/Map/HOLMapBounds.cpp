// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLMapBounds.h"

#include "Components/BoxComponent.h"
#include "EngineUtils.h"

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
	return World ? *TActorIterator<AHOLMapBounds>(const_cast<UWorld*>(World)) : nullptr;
}
