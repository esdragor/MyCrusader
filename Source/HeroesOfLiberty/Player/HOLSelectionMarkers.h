// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HOLSelectionMarkers.generated.h"

class AHOLPlayerController;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * What the ground shows of the local player's selection (J4, D288), on the player's machine only (never replicated,
 * like the RTS camera): a ring under each selected unit or around the selected resource source (J5b), and a square
 * frame under the selected building (under each of a group, an F key, D30), a neutral
 * white; a brief ring where the player gave an order; the rally flag of the player's own selected building, at its
 * colour (of each building of a group's active subgroup that has one, all of them for « Tous », the user's 3rd trial
 * of J6a.5); in placement mode (J6a.3, D311), the ghost of the building: its footprint on the ground, tile by tile, and the
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

	/**
	 * "rings 3, frame 1, flag 1 at (-16800, -1500), ghost valid": what the ground shows, for the scripted checks
	 * (hol.Select.Report); each flag shown is followed by its foot.
	 */
	FString Describe() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleOrderGiven(const FVector& Point);
	void UpdateRings();
	void UpdateFrame();
	void UpdateFlag();
	void UpdateGhost();
	/** A pole and its cloth more for the flags (one per building with a rally point), made the first time needed. */
	void AddFlag();

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Rings;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> Frames;

	/** The rally flags, a pole and a cloth each, as many as ever shown together (hidden beyond the number shown). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> FlagPoles;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> FlagCloths;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PoleMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ClothMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMesh> CubeMesh;

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
	/** Frames of a group drawn thinner (out of its active subgroup, Tab), for Describe. */
	int32 DimmedFrames = 0;
	/** The feet of the flags shown, for Describe. */
	TArray<FVector> FlagFeet;

	TWeakObjectPtr<AHOLPlayerController> Controller;
	FDelegateHandle OrderHandle;

	/** The last order's point, and the seconds since it was given (the ring shows while under PingSeconds). */
	FVector PingPoint = FVector::ZeroVector;
	float PingAge = TNumericLimits<float>::Max();
};
