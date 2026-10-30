// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLBannerSubsystem.generated.h"

class AActor;
class UInstancedStaticMeshComponent;
class UStaticMesh;

/**
 * The Banners of the Dawn standing in the world (J8c.4 loop 4, J8c-Q4), drawn on a displaying machine only (solo, host,
 * client; never a dedicated server), for every player to see: one instance of SM_Banner (no shadow) per slot whose banner
 * stands, read each frame from the slot's public hero sheet (its place and the step it falls at, against the server's step).
 * Nothing is sent for it. The standard stands on the ground under its place (the lowest static surface, as the units') and
 * sways a little about its pole, each out of step with the others.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLBannerSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** A banner drawn: whose, and where its foot stands (cm). */
	struct FShown
	{
		int32 Slot = INDEX_NONE;
		FVector Location = FVector::ZeroVector;
	};

	const TArray<FShown>& GetShown() const { return Shown; }

	/** The instances drawing the banners; null before the first. */
	const UInstancedStaticMeshComponent* GetInstances() const { return Instances; }

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/** The lowest static surface under a point (a tree's crown is no ground); the fallback where there is none. */
	double GroundAt(const FVector2D& Point, double Fallback) const;

	TArray<FShown> Shown;
	UPROPERTY(Transient)
	TObjectPtr<AActor> InstancesOwner;
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> Instances;
	/** The slots whose banner was drawn at the last frame, and whether a frame has been seen (a machine arriving with a banner
	 *  standing plants none: the sound is for a banner that appears). */
	TSet<int32> LastSlots;
	bool bFirstFrameSeen = false;
	/** The ground height found under each slot's banner, kept while it stands in the same place. */
	TMap<int32, TPair<FIntPoint, double>> GroundCache;
};
