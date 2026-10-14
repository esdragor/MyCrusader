// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLSelectionMarkers.generated.h"

class AHOLPlayerController;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

/**
 * What the ground shows of the local player's selection (J4, D288), on the player's machine only (never replicated,
 * like the RTS camera): a ring under each selected unit and a square frame under the selected building, a neutral
 * white; a brief ring where the player gave an order; the rally flag of the player's own selected building, at its
 * colour.
 * Unlit masked planes (Tools/Editor/make_marker_materials.py), instanced, with no collision and no shadow.
 */
UCLASS(NotPlaceable, Transient)
class HEROESOFLIBERTY_API AHOLSelectionMarkers : public AActor
{
	GENERATED_BODY()

public:
	AHOLSelectionMarkers();

	/** Follows that controller's selection and orders. */
	void Init(AHOLPlayerController* InController);

	virtual void Tick(float DeltaSeconds) override;

	/** "rings 3, frame 1, flag 1": what the ground shows, for the scripted checks (hol.Select.Report). */
	FString Describe() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleOrderGiven(const FVector& Point);
	void UpdateRings();
	void UpdateFrame();
	void UpdateFlag();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Rings;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Frames;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FlagPole;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FlagCloth;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ClothMaterial;

	TWeakObjectPtr<AHOLPlayerController> Controller;
	FDelegateHandle OrderHandle;

	/** The last order's point, and the seconds since it was given (the ring shows while under PingSeconds). */
	FVector PingPoint = FVector::ZeroVector;
	float PingAge = TNumericLimits<float>::Max();
};
