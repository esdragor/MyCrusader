// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLArrowSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UStaticMesh;

/** An arrow to draw (J7c.2b): who shoots whom, when it left the bow and how long it flies (fixed at release, Q2). */
struct FHOLArrowLaunch
{
	/** The shooter's and the target's drawn ground positions at release (cm). */
	FVector From = FVector::ZeroVector;
	FVector To = FVector::ZeroVector;
	/** The target, followed while it lives (its drawn position); invalid or gone: the arrow flies to its last position. */
	FMassEntityHandle Target;
	/** The flight (ms): the arrow lands at its end, always on its target (Q1). */
	int32 FlightMs = 0;
	/** The shooter's unit type (AR9, AR-Q5): its row's Projectile is the mesh drawn (SM_Bolt for the Crossbowman); none or
	 *  empty: SM_Arrow. Known on every machine (the client has the shooter's type): nothing more is sent. */
	FName ShooterType;
};

/**
 * The arrows in flight (J7c.2b), drawn on a displaying machine only (solo, host, client; never a dedicated server): one
 * instanced static mesh per projectile mesh (AR9: the shooter type's Projectile, SM_Arrow or SM_Bolt; no shadow, D258), no
 * actor per arrow, nothing replicated (Q4). The same flight for all of them (speed and arc, AR-Q5).
 * An arrow leaves the shooter's shoulder, rises in a light arc, follows its target's drawn position, reaches its chest at
 * the end of the flight and goes. Fed by the server's release in solo / host (HOLCombatProcessor); the client will feed
 * it by deduction (J7c.2b2) through the same Launch.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLArrowSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Launches an arrow now (a dedicated server: nothing). */
	void Launch(const FHOLArrowLaunch& Arrow);

	int32 GetInFlightCount() const { return Arrows.Num(); }
	int64 GetLaunchedCount() const { return Launched; }
	int64 GetLandedCount() const { return Landed; }
	int64 GetGapCount() const { return GapCount; }
	double GetGapMeanMs() const { return GapCount > 0 ? GapSumMs / GapCount : 0.0; }
	double GetGapMinMs() const { return GapMinMs; }
	double GetGapMaxMs() const { return GapMaxMs; }

	/**
	 * A client received Target's health going down (J7c.2b2): the gap (ms) between that and the arrow drawn on it, its
	 * earliest one in flight (negative: the health came first) or else its last landed within MaxGapSeconds, measured
	 * for hol.Combat.ArrowReport. No arrow then: not counted (a melee hit).
	 */
	void NoteHealthDrop(const FMassEntityHandle& Target);

	/** The mesh drawn for a shot of this unit type (AR9): its row's Projectile, else SM_Arrow; null if neither loads. */
	UStaticMesh* GetProjectileMesh(FName ShooterType);

	/** The instances drawing the shots of Mesh (AR9: one component per mesh); null before its first shot. */
	const UInstancedStaticMeshComponent* GetInstances(const UStaticMesh* Mesh) const;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	struct FArrow
	{
		FVector From;
		FVector LastTarget;
		FMassEntityHandle Target;
		double Start = 0.0;
		double Duration = 0.0;
		/** Its mesh's index in Meshes and Instances; INDEX_NONE: not drawn (no mesh). */
		int32 Group = INDEX_NONE;
	};

	static constexpr double MaxGapSeconds = 2.0;

	/** The index of the mesh drawing a shot of this type in Meshes (its component made at its first shot); INDEX_NONE: none. */
	int32 FindOrAddGroup(FName ShooterType);

	TArray<FArrow> Arrows;
	/** The projectile meshes met so far and their instances, side by side (AR9). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMesh>> Meshes;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> Instances;
	UPROPERTY(Transient)
	TObjectPtr<AActor> InstancesOwner;
	/** Each shooter type's group (INDEX_NONE: its mesh missing, warned once). */
	TMap<FName, int32> GroupByType;
	int64 Launched = 0;
	int64 Landed = 0;
	/** The last arrow landed on each target (world seconds), for the gaps; pruned past MaxGapSeconds. */
	TMap<FMassEntityHandle, double> LastLanding;
	int64 GapCount = 0;
	double GapSumMs = 0.0;
	double GapMinMs = 0.0;
	double GapMaxMs = 0.0;
};
