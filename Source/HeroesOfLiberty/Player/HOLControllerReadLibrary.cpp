// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLControllerReadLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/World.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"
#include "Player/HOLRTSCamera.h"
#include "Simulation/HOLUnitSubsystem.h"

const AHOLBuilding* UHOLControllerReadLibrary::FindMainCentre(const AHOLPlayerController& Controller, const bool bEnemy)
{
	const AHOLPlayerState* PlayerState = Controller.GetHOLPlayerState();
	for (FHOLBuildingIterator It(Controller.GetWorld()); PlayerState && It; ++It)
	{
		if (It->GetTypeName() == HOLBuildingTypeNames::MainCentre && (It->GetOwnerPlayer() == PlayerState->GetPlayerIndex()) != bEnemy)
		{
			return *It;
		}
	}
	return nullptr;
}

bool UHOLControllerReadLibrary::PointNearMainCentre(AHOLPlayerController* Controller, const double Dx, const double Dy, const bool bEnemy, FVector& OutPoint)
{
	const AHOLBuilding* Centre = Controller ? FindMainCentre(*Controller, bEnemy) : nullptr;
	if (!Centre)
	{
		return false;
	}
	OutPoint = Centre->GetActorLocation() + Centre->GetActorRotation().RotateVector(FVector(Dx, Dy, 0.));
	OutPoint.Z = 0.;
	return true;
}

bool UHOLControllerReadLibrary::SelectedBuildingLocation(AHOLPlayerController* Controller, FVector& OutLocation)
{
	const AHOLBuilding* Building = Controller ? Controller->GetSelection() : nullptr;
	if (!Building)
	{
		return false;
	}
	OutLocation = Building->GetActorLocation();
	return true;
}

bool UHOLControllerReadLibrary::CameraFocus(AHOLPlayerController* Controller, FVector& OutFocus)
{
	const AHOLRTSCamera* Camera = Controller ? Controller->GetRTSCamera() : nullptr;
	if (!Camera)
	{
		return false;
	}
	OutFocus = Camera->GetFocus();
	return true;
}

bool UHOLControllerReadLibrary::SpawnUnitsAround(AHOLPlayerController* Controller, const int32 Count, const FName TypeName, const int32 Slot, const double Radius, const FVector& Center)
{
	UWorld* World = Controller ? Controller->GetWorld() : nullptr;
	UHOLUnitSubsystem* Units = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!Units)
	{
		return false;
	}
	FHOLUnitSpawnRequest Request;
	Request.Count = Count;
	Request.TypeName = TypeName;
	Request.OwnerPlayer = static_cast<uint8>(Slot);
	Request.Radius = static_cast<float>(Radius);
	Request.Center = FVector(Center.X, Center.Y, 0.);
	Units->RequestUnits(Request);
	return true;
}
