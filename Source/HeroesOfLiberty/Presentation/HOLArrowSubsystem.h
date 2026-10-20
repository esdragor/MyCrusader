// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mass/EntityHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLArrowSubsystem.generated.h"

class UInstancedStaticMeshComponent;

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
};

/**
 * The arrows in flight (J7c.2b), drawn on a displaying machine only (solo, host, client; never a dedicated server): one
 * instanced static mesh for all of them (SM_Arrow, no shadow, D258), no actor per arrow, nothing replicated (Q4).
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
	};

	bool EnsureComponent();

	TArray<FArrow> Arrows;
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> Instances;
	int64 Launched = 0;
	int64 Landed = 0;
	bool bMeshMissing = false;
};
