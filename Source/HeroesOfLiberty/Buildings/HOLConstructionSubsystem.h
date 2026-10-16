// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLConstructionRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLConstructionSubsystem.generated.h"

class AHOLBuilding;
class AHOLPlayerState;
struct FHOLBuildingTypeRow;
struct FHOLConstructionCommand;

/**
 * Construction of the buildings (J6, D311, D312), on the server (or a solo match) only, at the start of each simulation
 * step: applies the construction orders of the order queue. A placement is checked again from what the server knows
 * (D288: grid, map, buildings and foundations, sources, enemy main centres and their anti-rush radius, the navmesh under
 * the footprint), paid (taken at placement, all or nothing) and spawns a replicated foundation (AHOLBuilding in the
 * Foundation state) owned by the player, and sends the units selected to build it (a build order, UHOLBuildProcessor,
 * J6a.4); a cancel refunds (all before the first work, half after) and removes it. The placement check is shared with the client's ghost (CheckPlacement), which
 * shows what the server would answer from what the client knows.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLConstructionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The footprint of a type placed around a ground point: snapped to the tile grid (D311), the grid's orientation. */
	static FBox2D SnapFootprint(const FHOLBuildingTypeRow& Type, const FVector& Point);

	/**
	 * Whether a foundation of PlayerIndex may go on Footprint, from what this machine knows (D311): its map bounds, every
	 * building and foundation, the standing sources, the other players' main centres, and the navmesh when this machine
	 * has one (a client has none: the server answers; bOutNavChecked tells).
	 */
	static EHOLPlacementRefusal CheckPlacement(const UWorld& World, const FBox2D& Footprint, int32 PlayerIndex, bool* bOutNavChecked = nullptr);

	/** "on a building", "too near an enemy main centre"...: for the logs (J6a.5 turns them into short messages). */
	static const TCHAR* DescribeRefusal(EHOLPlacementRefusal Refusal);

	/** Height of the ground at a point, buildings ignored; DefaultZ if no ground is found there. */
	static double FindGroundZ(const UWorld& World, const FVector2D& Point, double DefaultZ);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void OnStepStarted(int64 StepIndex);
	void ApplyOrders();
	void Place(const FHOLConstructionCommand& Order, AHOLPlayerState& PlayerState);
	void Cancel(const FHOLConstructionCommand& Order, AHOLPlayerState& PlayerState);
	AHOLPlayerState* FindPlayerState(int32 PlayerIndex) const;

	FDelegateHandle StepStartedHandle;
};
