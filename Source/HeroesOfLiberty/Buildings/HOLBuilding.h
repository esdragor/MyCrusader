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
 * area of the navmesh (D286): a dynamic obstacle, applied on the server when the building appears. It occupies whole tiles
 * (D311) but the building itself, its body, stands inset in them (D324): its shape, the forbidden area and every rule of the
 * units around it (D310) take the body, the placement rules the tiles.
 * Construction (J6a.3, J6a.4, D311, D312): a building placed by a player is born a foundation, which blocks nothing and
 * gives nothing: only a ghost (a frame on the ground and the edges of its volume, at its owner's colour) that its owner
 * alone sees and receives (the user's choice of 2026-10-16, as in AoE IV: the other players do not learn the plans). At
 * the first work it becomes a site seen by everyone, its footprint blocks the navmesh (D312), and it rises from the
 * ground with its work, over the foundation's slab; its state and work replicated to everyone. It serves (population,
 * deposit, production) once finished. The main centres are born finished.
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
	/**
	 * A camp (HOLConstruction::SendsBuildersToGather: a deposit other than a main centre): finished, its builders with nothing
	 * left in their queue gather the nearest source it accepts, and a rally point on it sends the units born to it (D312, J6b.1).
	 */
	bool IsCamp() const;
	/** A bare foundation: placed and paid, no work yet (D311). */
	bool IsFoundation() const { return ConstructionState == EHOLConstructionState::Foundation; }
	/** Its footprint is a forbidden area of the navmesh (D312: not as a bare foundation). */
	bool BlocksNavigation() const;
	/** Work done on it (HOLConstruction::WorkPerBuilder per builder and step, J6a.4); 0 for a foundation. */
	int64 GetConstructionWork() const { return ConstructionWork; }
	/** Some work has been done (D311: a cancel refunds half). */
	bool HasWorkStarted() const;
	/** Work it needs from nothing to finished (its type's BuildTime for one builder, J6a.2); 0 without a type. */
	int64 GetTotalWork() const;
	/** Share of its construction done, 0 to 1; 1 once finished (it rises from the ground with it, D312). */
	float GetProgress() const;
	/**
	 * This machine shows it: always, but a bare foundation only on its owner's machine (a client receives only its own,
	 * J6a.4); on another (a solo match's computer players, a listen server), it is not drawn and cannot be clicked.
	 */
	bool IsSeenHere() const;
	/**
	 * Server only (J6a.4): its state, its work. The look and the navmesh follow on every machine (the footprint blocks
	 * from UnderConstruction on).
	 */
	void SetConstructionState(EHOLConstructionState InState);
	void SetConstructionWork(int64 InWork);
	/**
	 * Its builders at work this step (J6a.5, D313: the site's block shows them), replicated to its owner alone; 0 once
	 * finished or when nobody works on it. Server only to set.
	 */
	int32 GetBuildersAtWork() const { return BuildersAtWork; }
	void SetBuildersAtWork(int32 InBuilders);

	/** The construction state or work changed: on the server when set, on every machine when it arrives. For the HUD. */
	DECLARE_MULTICAST_DELEGATE(FOnConstructionChanged);
	FOnConstructionChanged OnConstructionChanged;

	/** Its footprint on the ground (cm), turned by its yaw: its tiles, the box placements are checked against (D311). */
	FBox2D GetFootprintBounds() const;
	/**
	 * Its body (D324): the building itself, inset in its tiles by UHOLBuildingSettings::BodyInset on every side; its size,
	 * and its box on the ground turned by its yaw. What units walk around, stand by and are kept out of (D310).
	 */
	FVector2D GetBodySize() const;
	FBox2D GetBodyBounds() const;

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
	/**
	 * The owner's site at the rally point (the user's trial of J6a.4): the units born here build it while it is not finished;
	 * null: the ground or a source. Server only (the flag stands on its middle, as on a source).
	 */
	AHOLBuilding* GetRallySite() const { return RallySite.Get(); }
	/** Server only. */
	void SetRallyPoint(const FVector& Point, int32 SourceIndex = INDEX_NONE, AHOLBuilding* Site = nullptr);
	void ClearRallyPoint();
	/** True when a ground point is on the building's body (D324; a rally order there clears the rally point). */
	bool IsOnBody(const FVector& Point) const;

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

	/** A site's height at its first work (share of the full height, J6a.4): it shows at once, then rises with its work. */
	static constexpr float MinRiseRatio = 0.04f;
	/** The ghost of a bare foundation (J6a.4, cm): its frame on the ground, the edges of its volume. */
	static constexpr float GhostFrameWidth = 30.f;
	static constexpr float GhostFrameHeight = 4.f;
	static constexpr float GhostEdgeWidth = 10.f;
	/** The ghost's colour: the owner's colour this much toward white (a light outline, as the placement ghost). */
	static constexpr float GhostWhiten = 0.35f;

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

	UFUNCTION()
	void OnRep_BuildersAtWork();

	/** Builds the look from the type and the construction state, then paints the owner's colour. Safe to call again. */
	void RefreshAppearance();
	/** A site's height follows its work (J6a.4): the pieces under RiseRoot squashed to it, at least MinRiseRatio. */
	void UpdateRise();
	/**
	 * The relevancy of a bare foundation, its owner alone (J6a.4), or of anything else, everyone: the actor's flags (the
	 * legacy replication) and the owner filter of Iris, cleared at the first work.
	 */
	void ApplyRelevancy();
	/** Writes the owner's colour on the pieces that wear it. */
	void ApplyColors();
	void BuildProvisionalShape(const FHOLBuildingTypeRow& Type);
	void BuildFinalMesh(const FHOLBuildingTypeRow& Type);
	/** A foundation (J6a.3): a low slab over the footprint, a border at the owner's colour, four corner stakes. */
	void BuildFoundationShape(const FHOLBuildingTypeRow& Type);
	/**
	 * A bare foundation seen by its owner (J6a.4): a ghost, its frame on the ground and the edges of its volume (no slab),
	 * and an unseen box over its footprint that a click selects.
	 */
	void BuildGhostShape(const FHOLBuildingTypeRow& Type);
	/**
	 * Sizes the navmesh's forbidden area to the building's body (D324: inset in its tiles), widened by the bodies' clearance
	 * (D310), and height,
	 * and makes it count for the navmesh while the building blocks it (D312: not as a bare foundation).
	 */
	void FitNavObstacle(const FHOLBuildingTypeRow& Type);
	/** A piece of Size (cm) laid on BottomZ, painted Color, or the owner's colour times OwnerTone when bOwnerColor. */
	UStaticMeshComponent* AddPiece(UStaticMesh* Mesh, const FVector& Size, float BottomZ, const FLinearColor& Color, bool bOwnerColor,
		float OwnerTone = 1.f);
	/** The pieces AddPiece makes from now on go under Parent (RiseRoot for the rising building of a site), else the root. */
	USceneComponent* PieceParent = nullptr;

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

	/** The server's alone: the owner sees the flag at RallyPoint. */
	TWeakObjectPtr<AHOLBuilding> RallySite;

	UPROPERTY(ReplicatedUsing = OnRep_Construction)
	EHOLConstructionState ConstructionState = EHOLConstructionState::Finished;

	UPROPERTY(ReplicatedUsing = OnRep_Construction)
	int64 ConstructionWork = 0;

	/** Changes only when a builder arrives or leaves (one byte, to the owner alone). */
	UPROPERTY(ReplicatedUsing = OnRep_BuildersAtWork)
	uint8 BuildersAtWork = 0;

	/** What the pieces show (J6a.4): nothing, a ghost, a site, the finished building. */
	enum class ELook : uint8
	{
		None,
		Hidden,
		Ghost,
		Site,
		Finished,
	};
	/** The type the pieces were built for, and their look: the look is rebuilt only when either changes. */
	FName BuiltTypeName;
	ELook BuiltLook = ELook::None;

	/** The building above its foundation, squashed to a site's progress (J6a.4); scale 1 once finished. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> RiseRoot;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> Pieces;

	/** The body as a forbidden area of the navmesh (D286, D324), sized from the type; no collision. */
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
	/** How far toward white each one is (the ghost's lighter edges, J6a.4). */
	TArray<float> OwnerWhitens;
};
