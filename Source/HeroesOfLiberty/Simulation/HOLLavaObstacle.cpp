// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLLavaObstacle.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Components/BoxComponent.h"
#include "NavAreas/NavArea_Null.h"
#include "NavigationSystem.h"

AHOLLavaObstacle::AHOLLavaObstacle()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	bReplicates = false;
}

void AHOLLavaObstacle::Build(const TArray<HOLVolcano::FObstacleBox>& Boxes, const float Height)
{
	FBox Area(ForceInit);
	for (const HOLVolcano::FObstacleBox& Box : Boxes)
	{
		UBoxComponent* Component = NewObject<UBoxComponent>(this);
		Component->SetupAttachment(RootComponent);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetBoxExtent(FVector(Box.HalfLength, Box.HalfWidth, Height * 0.5f));
		Component->bDynamicObstacle = true;
		Component->SetAreaClassOverride(UNavArea_Null::StaticClass());
		Component->SetCanEverAffectNavigation(true);
		Component->RegisterComponent();
		Component->SetWorldLocationAndRotation(FVector(Box.Centre.X, Box.Centre.Y, Height * 0.5f), FRotator(0.f, Box.YawDegrees, 0.f));
		FNavigationSystem::UpdateComponentData(*Component);
		Area += Component->Bounds.GetBox();
		BoxComponents.Add(Component);
	}
	if (Area.IsValid)
	{
		if (UHOLCommandSubsystem* Commands = GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr)
		{
			Commands->NotifyNavigationChanged(Area);
		}
	}
}

bool AHOLLavaObstacle::IsBlockingNavigation() const
{
	for (const TObjectPtr<UBoxComponent>& Component : BoxComponents)
	{
		if (!Component || !Component->CanEverAffectNavigation())
		{
			return false;
		}
	}
	return !BoxComponents.IsEmpty();
}

void AHOLLavaObstacle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The ground is free again (basalt, passable): the navigation forgets the boxes and the paths are asked again.
	FBox Area(ForceInit);
	for (const TObjectPtr<UBoxComponent>& Component : BoxComponents)
	{
		if (Component)
		{
			Area += Component->Bounds.GetBox();
			Component->SetCanEverAffectNavigation(false);
		}
	}
	if (Area.IsValid && EndPlayReason == EEndPlayReason::Destroyed)
	{
		if (UHOLCommandSubsystem* Commands = GetWorld() ? GetWorld()->GetSubsystem<UHOLCommandSubsystem>() : nullptr)
		{
			Commands->NotifyNavigationChanged(Area);
		}
	}
	Super::EndPlay(EndPlayReason);
}