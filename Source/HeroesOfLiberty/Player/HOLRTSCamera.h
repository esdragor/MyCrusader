// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLRTSCamera.generated.h"

class UCameraComponent;
class USpringArmComponent;

/** Camera speed setting of the Controls tab (D253), in the order shown. */
UENUM()
enum class EHOLCameraSpeed : uint8
{
	Slow,
	Normal,
	Fast,
};

/**
 * RTS camera of a match (D254, D255): a local actor, spawned by the player's controller on its own machine only and
 * never replicated. It looks at a focus point on the ground from a fixed pitch and no rotation; the height above the
 * focus is the zoom. The focus stays inside the map bounds. Values are indicative, to be tuned in play.
 */
UCLASS(NotPlaceable)
class HEROESOFLIBERTY_API AHOLRTSCamera : public AActor
{
	GENERATED_BODY()

public:
	AHOLRTSCamera();

	/** Looks at the given ground point (kept inside the area). */
	void SetFocus(const FVector& Point);
	FVector GetFocus() const { return GetActorLocation(); }

	/** Area the focus stays in; none until set. */
	void SetArea(const FBox2D& InArea);

	/** Moves the focus by a direction on screen (X right, Y up, each -1 to 1) for DeltaSeconds, at the speed setting. */
	void Pan(const FVector2D& Direction, float DeltaSeconds, EHOLCameraSpeed Speed);

	/** Zooms by mouse wheel steps (positive: closer). */
	void Zoom(float Steps);

	float GetHeight() const { return Height; }

	/** Where the line from the camera through a screen point meets the ground plane of the focus. */
	bool GetGroundPoint(const FVector& WorldOrigin, const FVector& WorldDirection, FVector& OutPoint) const;

	// Pure rules, also used by the automation tests.
	static FVector2D ClampFocus(const FVector2D& Focus, const FBox2D& Area);
	/** Pan speed, cm/s: grows with the height, so that the screen moves at the same pace at any zoom. */
	static float GetPanSpeed(float Height, EHOLCameraSpeed Speed);

	/** Pitch below the horizon, degrees (D254: ~55). */
	static constexpr float Pitch = 55.f;
	/** Height above the focus, cm (D254: 15 to 70 m, 30-40 m by default). */
	static constexpr float MinHeight = 1500.f;
	static constexpr float MaxHeight = 7000.f;
	static constexpr float DefaultHeight = 3500.f;
	/** Height change per wheel step, cm. */
	static constexpr float ZoomStep = 500.f;

private:
	void ApplyHeight();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> Arm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	float Height = DefaultHeight;
	FBox2D Area = FBox2D(ForceInit);
};
