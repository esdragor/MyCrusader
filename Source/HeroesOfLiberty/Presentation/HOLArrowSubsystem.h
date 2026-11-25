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
	/** AR0-a3: the shooter's owner slot: its faction's art asset (DA_Art_<Type>_<Faction>) gives the projectile and the smoke; INDEX_NONE: the common one. */
	int32 ShooterSlot = INDEX_NONE;
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

	/** S3-a: the scale an arrow is drawn at: the base (1.5, 4, 4) times hol.Arrow.Scale (default 1: unchanged; the trials raise it to see the arrow). */
	static FVector GetArrowScale();
	/** S3-a: how long an arrow of FlightMs milliseconds of flight is drawn (s): the flight times hol.Arrow.SlowMotion (default 1: unchanged; the trials slow the drawing only, the landing stays on time). */
	static double GetFlightDurationSeconds(int32 FlightMs);

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

	/**
	 * The mesh drawn for a shot of this unit type (AR9): its art asset's Projectile (AR0-a3: DA_Art_<Type>_<Faction> of the
	 * shooter's faction, then DA_Art_<Type>), else its row's Projectile, else SM_Arrow; null if none loads.
	 */
	UStaticMesh* GetProjectileMesh(FName ShooterType, FName ShooterFaction = NAME_None);

	/** The puff drawn at the muzzle at a shot of this unit type: its art asset's (faction, then common), else its row's MuzzleSmoke (the Arquebusier's); null when empty. */
	UStaticMesh* GetMuzzleSmokeMesh(FName ShooterType, FName ShooterFaction = NAME_None);

	/**
	 * Where a puff appears, from the shooter's ground position: MuzzleForward cm ahead of him (toward his target) and
	 * MuzzleHeight cm above his feet: the muzzle of a gun held at his shoulder (mesh scale 1.5: the mannequin's shoulder is at
	 * 151 cm, the gun's muzzle ~110 cm in front of its butt, both ~1.5 times larger in the game).
	 */
	static constexpr double MuzzleForward = 150.0;
	static constexpr double MuzzleHeight = 225.0;
	/** How long a puff lasts (s): it swells, rises and fades out (translucent). */
	static constexpr double SmokeSeconds = 1.6;
	/** A puff's scale at its start and at its biggest (SM_PowderSmoke is a 50 cm ball: radius = 50 x scale, cm), and how far it rises (cm). */
	static constexpr double SmokeStartScale = 0.1;
	static constexpr double SmokeEndScale = 0.4;
	static constexpr double SmokeRise = 90.0;
	/** The opacity of a puff at its start (0 to 1): a cloud's, not a ball's; M_PowderSmoke reads it as per-instance custom data 0. */
	static constexpr double SmokeOpacity = 0.55;
	/** A puff's opacity at this age (0 = just launched, 1 = end of its life): SmokeOpacity falling to 0 (squared, so the tail thins out). */
	static double GetSmokeOpacity(const double Age) { return SmokeOpacity * FMath::Square(1.0 - FMath::Clamp(Age, 0.0, 1.0)); }

	int32 GetSmokeInFlightCount() const { return Puffs.Num(); }
	/** The last puff launched (cm, world): for the tests. */
	FVector GetLastSmokeOrigin() const { return LastSmokeOrigin; }

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

	/** A puff of powder smoke at the muzzle (Arquebusier): where it appeared and when. */
	struct FPuff
	{
		FVector Origin = FVector::ZeroVector;
		double Start = 0.0;
		int32 Group = INDEX_NONE;
	};

	static constexpr double MaxGapSeconds = 2.0;

	/** The index of the mesh drawing a shot of this type in Meshes (its component made at its first shot); INDEX_NONE: none. */
	int32 FindOrAddGroup(FName ShooterType, FName ShooterFaction);

	/**
	 * The index of the group drawing this mesh (its component made on the first call); INDEX_NONE: no mesh or no world.
	 * NumCustomData: the per-instance custom floats of its component (the smoke's opacity: 1), set at the first call.
	 */
	int32 FindOrAddMeshGroup(UStaticMesh* Mesh, int32 NumCustomData = 0);

	/** The group drawing the muzzle smoke of this type (INDEX_NONE: the type has none), looked up once. */
	int32 FindOrAddSmokeGroup(FName ShooterType, FName ShooterFaction);

	TArray<FArrow> Arrows;
	/** The projectile meshes met so far and their instances, side by side (AR9). */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMesh>> Meshes;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> Instances;
	UPROPERTY(Transient)
	TObjectPtr<AActor> InstancesOwner;
	/** Each (shooter type, faction)'s group (INDEX_NONE: its mesh missing, warned once). */
	TMap<TPair<FName, FName>, int32> GroupByType;
	/** Each (shooter type, faction)'s smoke group (INDEX_NONE: none). */
	TMap<TPair<FName, FName>, int32> SmokeGroupByType;
	TArray<FPuff> Puffs;
	FVector LastSmokeOrigin = FVector::ZeroVector;
	int64 Launched = 0;
	int64 Landed = 0;
	/** The last arrow landed on each target (world seconds), for the gaps; pruned past MaxGapSeconds. */
	TMap<FMassEntityHandle, double> LastLanding;
	int64 GapCount = 0;
	double GapSumMs = 0.0;
	double GapMinMs = 0.0;
	double GapMaxMs = 0.0;
};
