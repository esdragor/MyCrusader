// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLProductionRules.h"
#include "GameFramework/Actor.h"
#include "HOLBuilding.generated.h"

class UBoxComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
struct FHOLBuildingTypeRow;

/**
 * A building (D260): a classic replicated actor (§ 16.4, buildings are few), made by the server. It carries its type
 * (a row of the building type table), its owner (the player's slot, a byte like the units' OwnerPlayer, D233) and its
 * owner's colour; every machine builds its look from the type, so only these three values travel. GAS comes with
 * the first ability or damage (M5). Selection is the local player's own (D262): its pieces block the selection trace
 * channel; nothing on the building shows it (no ring, no hover: the user's test of J2). Its footprint is a forbidden
 * area of the navmesh (D286): a dynamic obstacle, applied on the server when the building appears.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLBuilding : public AActor
{
	GENERATED_BODY()

public:
	AHOLBuilding();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server only, between a deferred spawn and its FinishSpawning (or right after a spawn). */
	void Setup(FName InTypeName, uint8 InOwnerPlayer, uint8 InColorIndex);

	FName GetTypeName() const { return TypeName; }
	uint8 GetOwnerPlayer() const { return OwnerPlayer; }
	uint8 GetColorIndex() const { return ColorIndex; }

	/** The row of its type, or null (logged) if missing. */
	const FHOLBuildingTypeRow* GetType() const;

	/**
	 * Production queue (J3, D282): the head is in production (or waits for population), the others wait their turn.
	 * Written by the server alone (UHOLProductionSubsystem), replicated to the owner only: the building's network
	 * owner is its player's controller (set by the game mode).
	 */
	const TArray<FHOLProductionItem>& GetProductionQueue() const { return ProductionQueue; }
	/** Server only, for UHOLProductionSubsystem; call NotifyProductionChanged after a change. */
	TArray<FHOLProductionItem>& GetMutableProductionQueue() { return ProductionQueue; }
	void NotifyProductionChanged();

	/** The queue changed: on the server when it is written, on the owner's machine when it arrives. For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnProductionChanged);
	FOnProductionChanged OnProductionChanged;

	/**
	 * Rally point (J4, D288): where the units it produces walk once born. Written by the server alone (a rally order
	 * of the command queue), replicated to the owner only, like the queue.
	 */
	bool HasRallyPoint() const { return bHasRallyPoint; }
	FVector GetRallyPoint() const { return RallyPoint; }
	/** The resource source at the rally point (J5, D298): the units born here gather it. INDEX_NONE: the ground. */
	int32 GetRallySource() const { return RallySource; }
	/** Server only. */
	void SetRallyPoint(const FVector& Point, int32 SourceIndex = INDEX_NONE);
	void ClearRallyPoint();
	/** True when a ground point is on the building's footprint (a rally order there clears the rally point). */
	bool IsOnFootprint(const FVector& Point) const;

	/** The rally point changed: on the server, on the owner's machine when it arrives. For the flag (J4.6). */
	DECLARE_MULTICAST_DELEGATE(FOnRallyChanged);
	FOnRallyChanged OnRallyChanged;

	/** Values of the provisional shape (D261), as fractions of the footprint and the height. */
	static constexpr float BaseHeightRatio = 0.45f;
	static constexpr float TowerSizeRatio = 0.4f;
	static constexpr float RoofSizeRatio = 0.5f;
	static constexpr float CapThicknessRatio = 0.04f;
	/** The top of the block is a darker tone of the owner's colour, so that the tower's roof stands out on it. */
	static constexpr float BaseTopTone = 0.55f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_Setup();

	UFUNCTION()
	void OnRep_ProductionQueue();

	UFUNCTION()
	void OnRep_Rally();

	/** Builds the look from the type, then paints the owner's colour. Safe to call again. */
	void RefreshAppearance();
	/** Writes the owner's colour on the pieces that wear it. */
	void ApplyColors();
	void BuildProvisionalShape(const FHOLBuildingTypeRow& Type);
	void BuildFinalMesh(const FHOLBuildingTypeRow& Type);
	/** Sizes the navmesh's forbidden area to the type's footprint and height. */
	void FitNavObstacle(const FHOLBuildingTypeRow& Type);
	/** A piece of Size (cm) laid on BottomZ, painted Color, or the owner's colour times OwnerTone when bOwnerColor. */
	UStaticMeshComponent* AddPiece(UStaticMesh* Mesh, const FVector& Size, float BottomZ, const FLinearColor& Color, bool bOwnerColor,
		float OwnerTone = 1.f);

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	FName TypeName;

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	uint8 OwnerPlayer = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Setup)
	uint8 ColorIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ProductionQueue)
	TArray<FHOLProductionItem> ProductionQueue;

	UPROPERTY(ReplicatedUsing = OnRep_Rally)
	FVector_NetQuantize RallyPoint = FVector::ZeroVector;

	UPROPERTY(ReplicatedUsing = OnRep_Rally)
	bool bHasRallyPoint = false;

	UPROPERTY(ReplicatedUsing = OnRep_Rally)
	int32 RallySource = INDEX_NONE;

	/** The type the pieces were built for: the look is rebuilt only when it changes. */
	FName BuiltTypeName;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Pieces;

	/** The footprint as a forbidden area of the navmesh (D286), sized from the type; no collision. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> NavObstacle;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	/** Matte, lit (D265). */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> PieceMaterial;

	/** Unlit, for the owner's colour (D261). */
	UPROPERTY()
	TObjectPtr<UMaterialInterface> OwnerPieceMaterial;

	/** Materials painted with the owner's colour, and the tone of each. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> OwnerMaterials;

	TArray<float> OwnerTones;
};
