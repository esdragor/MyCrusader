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
enum class EHOLAttackType : uint8;

/**
 * Construction of the buildings (J6, D311, D312), on the server (or a solo match) only, at the start of each simulation
 * step: applies the construction orders of the order queue. A placement is checked again from what the server knows
 * (D288: grid, map, buildings and foundations, sources, enemy main centres and their anti-rush radius, the navmesh under
 * the footprint), paid (taken at placement, all or nothing) and spawns a replicated foundation (AHOLBuilding in the
 * Foundation state) owned by the player, and sends the units selected to build it (a build order, UHOLBuildProcessor,
 * J6a.4); a cancel refunds (the share of the cost not built, D345: all before the first work) and removes it. The placement check is shared with the client's ghost (CheckPlacement), which
 * shows what the server would answer from what the client knows.
 * Damage to buildings (D345.2a): DamageBuilding takes health off (HOLConstruction::GetBuildingDamage); at 0 the building is
 * destroyed (DestroyBuilding): its queue refunded in full, a site the share not built, rubble left for 10 s.
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

	/**
	 * A footprint of Footprint's size where a foundation of PlayerIndex may go, beside what Footprint is on (the user's
	 * trial of J7b, feedback 3): only when Footprint is refused on a building or a source (OutObstacle says which, or the
	 * refusal found), around that obstacle (HOLConstruction::FindObstacle, the one under Aim, the cursor's ground point),
	 * up to 4 tiles away (HOLConstruction::FindFreeFootprint, OutRing the gap in tiles), every rule of CheckPlacement
	 * applied to it. False otherwise: the ghost stays red under the cursor.
	 */
	static bool FindFreeFootprint(const UWorld& World, const FBox2D& Footprint, const FVector2D& Aim, int32 PlayerIndex, FBox2D& OutFootprint,
		int32& OutRing, EHOLPlacementRefusal& OutObstacle);

	/** "on a building", "too near an enemy main centre"...: for the logs (J6a.5 turns them into short messages). */
	static const TCHAR* DescribeRefusal(EHOLPlacementRefusal Refusal);

	/** Height of the ground at a point, buildings ignored; DefaultZ if no ground is found there. */
	static double FindGroundZ(const UWorld& World, const FVector2D& Point, double DefaultZ);

	/**
	 * Server only (D345.2a): one attack of Damage of AttackType on Building, less its armour (HOLConstruction::
	 * GetBuildingDamage); at 0 health it is destroyed (DestroyBuilding). Returns the health it lost (no more than it had; 0
	 * if it was not hit).
	 */
	static int32 DamageBuilding(AHOLBuilding& Building, int32 Damage, EHOLAttackType AttackType);

	/**
	 * Server only (D345.2a): Building destroyed at once. It stops serving its owner (it leaves the world: population, deposit,
	 * production, rally, selection, its builders as on a cancel), its footprint reopens; every item of its queue is refunded
	 * in full, a site the share of its cost not built (HOLConstruction::GetCancelRefund); rubble (AHOLRubble) stays 10 s
	 * where it stood, blocking nothing (none for a bare foundation, which nothing showed to the others).
	 */
	static void DestroyBuilding(AHOLBuilding& Building);

	/**
	 * Server only: every item of Building's production queue cancelled and refunded in full to its owner's stock (as a
	 * cancel, the head too; D345.2a on a destruction, V2-Q3 on a main forum's fall). Returns the refund.
	 */
	static FHOLResources RefundProductionQueue(AHOLBuilding& Building);

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
