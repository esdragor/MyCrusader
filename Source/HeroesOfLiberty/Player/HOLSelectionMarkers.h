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
 * like the RTS camera): a ring under each selected unit or around the selected resource source (J5b), and a square
 * frame under the selected building, a neutral
 * white; a brief ring where the player gave an order; the rally flag of the player's own selected building, at its
 * colour; in placement mode (J6a.3, D311), the ghost of the building: its footprint on the ground, tile by tile, and the
 * edges of its volume, green where it may go, red where not.
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

	/** "rings 3, frame 1, flag 1, ghost valid": what the ground shows, for the scripted checks (hol.Select.Report). */
	FString Describe() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleOrderGiven(const FVector& Point);
	void UpdateRings();
	void UpdateFrame();
	void UpdateFlag();
	void UpdateGhost();

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

	/** The placement ghost (J6a.3): its outline and tiles on the ground, and the edges of its volume (4 posts, 4 beams). */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> GhostFrames;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UStaticMeshComponent>> GhostEdges;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> GhostMaterial;

	/** What the ghost shows: none, valid (green) or refused (red). */
	enum class EGhost : uint8 { None, Valid, Refused };
	EGhost ShownGhost = EGhost::None;

	TWeakObjectPtr<AHOLPlayerController> Controller;
	FDelegateHandle OrderHandle;

	/** The last order's point, and the seconds since it was given (the ring shows while under PingSeconds). */
	FVector PingPoint = FVector::ZeroVector;
	float PingAge = TNumericLimits<float>::Max();
};
