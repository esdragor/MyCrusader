// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLRTSCamera.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

namespace UE::HeroesOfLiberty::Camera
{
	/** Pan speed per cm of height, per second: at 35 m the focus moves ~40 m/s at normal speed. */
	constexpr float PanSpeedPerHeight = 1.15f;
	constexpr float SpeedFactors[] = { 0.6f, 1.f, 1.6f };
}

AHOLRTSCamera::AHOLRTSCamera()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	Arm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	Arm->SetupAttachment(RootComponent);
	Arm->SetRelativeRotation(FRotator(-Pitch, 0.f, 0.f));
	// Nothing comes between the camera and the ground it looks at, and the camera follows the focus exactly.
	Arm->bDoCollisionTest = false;
	Arm->bEnableCameraLag = false;
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Arm, USpringArmComponent::SocketName);
	ApplyHeight();
}

void AHOLRTSCamera::SetArea(const FBox2D& InArea)
{
	Area = InArea;
	SetFocus(GetFocus());
}

void AHOLRTSCamera::SetFocus(const FVector& Point)
{
	const FVector2D Clamped = ClampFocus(FVector2D(Point), Area);
	SetActorLocation(FVector(Clamped, Point.Z));
}

void AHOLRTSCamera::Pan(const FVector2D& Direction, const float DeltaSeconds, const EHOLCameraSpeed Speed)
{
	if (Direction.IsNearlyZero())
	{
		return;
	}
	// No rotation (D254): screen right is world +Y, screen up is world +X.
	const FVector2D Step = Direction.GetClampedToMaxSize(1.f) * GetPanSpeed(Height, Speed) * DeltaSeconds;
	SetFocus(GetFocus() + FVector(Step.Y, Step.X, 0.f));
}

void AHOLRTSCamera::Zoom(const float Steps)
{
	Height = FMath::Clamp(Height - Steps * ZoomStep, MinHeight, MaxHeight);
	ApplyHeight();
}

void AHOLRTSCamera::ApplyHeight()
{
	Arm->TargetArmLength = Height / FMath::Sin(FMath::DegreesToRadians(Pitch));
}

bool AHOLRTSCamera::GetGroundPoint(const FVector& WorldOrigin, const FVector& WorldDirection, FVector& OutPoint) const
{
	const FPlane Ground(GetFocus(), FVector::UpVector);
	if (FMath::IsNearlyZero(FVector::DotProduct(WorldDirection, Ground.GetNormal())))
	{
		return false;
	}
	OutPoint = FMath::RayPlaneIntersection(WorldOrigin, WorldDirection, Ground);
	return true;
}

FVector2D AHOLRTSCamera::ClampFocus(const FVector2D& Focus, const FBox2D& InArea)
{
	if (!InArea.bIsValid)
	{
		return Focus;
	}
	return FVector2D(FMath::Clamp(Focus.X, InArea.Min.X, InArea.Max.X), FMath::Clamp(Focus.Y, InArea.Min.Y, InArea.Max.Y));
}

float AHOLRTSCamera::GetPanSpeed(const float InHeight, const EHOLCameraSpeed Speed)
{
	using namespace UE::HeroesOfLiberty::Camera;

	const int32 Index = FMath::Clamp(static_cast<int32>(Speed), 0, UE_ARRAY_COUNT(SpeedFactors) - 1);
	return InHeight * PanSpeedPerHeight * SpeedFactors[Index];
}
