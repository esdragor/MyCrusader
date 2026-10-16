// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionRules.h"
#include "GameFramework/Actor.h"
#include "HOLBuilding.generated.h"

class UBoxComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * A building (D260): a classic replicated actor (§ 16.4, buildings are few), made by the server. It carries its type
 * (a row of the building type table), its owner (the player's slot, a byte like the units' OwnerPlayer, D233) and its
 * owner's colour; every machine builds its look from the type, so only these three values travel. GAS comes with
 * the first ability or damage (M5). Selection is the local player's own (D262): its pieces block the selection trace
 * channel; nothing on the building shows it (no ring, no hover: the user's test of J2). Its footprint is a forbidden
 * area of the navmesh (D286): a dynamic obstacle, applied on the server when the building appears.
 * Construction (J6a.3, D311, D312): a building placed by a player is born a foundation (a flat outline at its owner's
 * colour, which blocks nothing and gives nothing), its state and work replicated to everyone; the main centres are
 * born finished. The footprint blocks the navmesh from the first work on (J6a.4), and the building serves (population,
 * deposit, production) once finished.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLBuilding : public AActor
{
	GENERATED_BODY()

public:
	AHOLBuilding();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * Server only, between a deferred spawn and its FinishSpawning (or right after a spawn). InState: Finished for the
	 * main centres and the buildings placed by code, Foundation for a player's placement (D311).
	 */
	void Setup(FName InTypeName, uint8 InOwnerPlayer, uint8 InColorIndex, EHOLConstructionState InState = EHOLConstructionState::Finished);

	FName GetTypeName() const { return TypeName; }
	uint8 GetOwnerPlayer() const { return OwnerPlayer; }
	uint8 GetColorIndex() const { return ColorIndex; }

	/** The row of its type, or null (logged) if missing. */
	const FHOLBuildingTypeRow* GetType() const;

	/** Where it stands in its construction (J6, replicated to everyone). */
	EHOLConstructionState GetConstructionState() const { return ConstructionState; }
	/** Finished: it serves its owner (population, deposit, production; D312). */
	bool IsFinished() const;
	/** A bare foundation: placed and paid, no work yet (D311). */
	bool IsFoundation() const { return ConstructionState == EHOLConstructionState::Foundation; }
	/** Its footprint is a forbidden area of the navmesh (D312: not as a bare foundation). */
	bool BlocksNavigation() const;
	/** Work done on it (HOLConstruction::WorkPerBuilder per builder and step, J6a.4); 0 for a foundation. */
	int64 GetConstructionWork() const { return ConstructionWork; }
	/** Some work has been done (D311: a cancel refunds half). */
	bool HasWorkStarted() const;
	/**
	 * Server only (J6a.4): its state, its work. The look and the navmesh follow on every machine (the footprint blocks
	 * from UnderConstruction on).
	 */
	void SetConstructionState(EHOLConstructionState InState);
	void SetConstructionWork(int64 InWork);

	/** The construction state or work changed: on the server when set, on every machine when it arrives. For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnConstructionChanged);
	FOnConstructionChanged OnConstructionChanged;

	/** Its footprint on the ground (cm), turned by its yaw: the box placements are checked against (D311). */
	FBox2D GetFootprintBounds() const;

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

	/** The foundation's look (J6a.3, cm): a low slab, its edges at the owner's colour, a stake at each corner. */
	static constexpr float FoundationSlabHeight = 12.f;
	static constexpr float FoundationBorderWidth = 35.f;
	static constexpr float FoundationBorderHeight = 20.f;
	static constexpr float FoundationStakeSize = 20.f;
	static constexpr float FoundationStakeHeight = 90.f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_Setup();

	UFUNCTION()
	void OnRep_ProductionQueue();

	UFUNCTION()
	void OnRep_Rally();

	UFUNCTION()
	void OnRep_Construction();

	/** Builds the look from the type and the construction state, then paints the owner's colour. Safe to call again. */
	void RefreshAppearance();
	/** Writes the owner's colour on the pieces that wear it. */
	void ApplyColors();
	void BuildProvisionalShape(const FHOLBuildingTypeRow& Type);
	void BuildFinalMesh(const FHOLBuildingTypeRow& Type);
	/** A foundation (J6a.3): a low slab over the footprint, a border at the owner's colour, four corner stakes. */
	void BuildFoundationShape(const FHOLBuildingTypeRow& Type);
	/**
	 * Sizes the navmesh's forbidden area to the type's footprint, widened by the bodies' clearance (D310), and height,
	 * and makes it count for the navmesh while the building blocks it (D312: not as a bare foundation).
	 */
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

	UPROPERTY(ReplicatedUsing = OnRep_Construction)
	EHOLConstructionState ConstructionState = EHOLConstructionState::Finished;

	UPROPERTY(ReplicatedUsing = OnRep_Construction)
	int64 ConstructionWork = 0;

	/** The type the pieces were built for, and whether as a foundation: the look is rebuilt only when either changes. */
	FName BuiltTypeName;
	bool bBuiltAsFoundation = false;

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
