// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLUnitOrderProcessor.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationSubsystem.h"
#include "NavCorridor.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "Simulation/HOLGroupMoveRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Orders
{
	int32 PathBudgetOverride = -1;
	FAutoConsoleVariableRef CVarPathBudget(TEXT("hol.Units.PathBudget"), PathBudgetOverride,
		TEXT("Navmesh path requests per fixed step at most (D286); 0 = no budget, negative = the project setting."));

	int32 DirectPaths = 0;
	FAutoConsoleVariableRef CVarDirectPaths(TEXT("hol.Units.DirectPaths"), DirectPaths,
		TEXT("1: no navmesh path, every unit walks straight to its slot (the M1 move of D228), to measure what the paths cost (D289)."));

	/** A short path is done this close (cm) to its last point; the final leg brings the unit to its slot. */
	constexpr float EndReachedDistance = 50.f;

	/** Corridor of a path, the same settings as Mass' own navmesh path follow, at the project's width. */
	TSharedPtr<FNavCorridor> MakeCorridor(const FNavigationPath& Path, const ANavigationData& NavData, const float Width)
	{
		TSharedPtr<FNavCorridor> Corridor = MakeShared<FNavCorridor>();
		FNavCorridorParams Params;
		Params.SetFromWidth(Width);
		Params.PathOffsetFromBoundaries = 10.f;
		Corridor->BuildFromPath(Path, NavData.GetDefaultQueryFilter(), Params);
		return Corridor;
	}

	/** Distances and tangents of the short path's points, as Mass computes them, after their positions moved. */
	void RecomputeShortPath(FMassNavMeshShortPathFragment& ShortPath, const uint8 NumLeadingPoints)
	{
		float PathDistance = 0.f;
		ShortPath.Points[0].Distance.Set(PathDistance);
		ShortPath.Points[0].Tangent = FMassSnorm8Vector2D((ShortPath.Points[1].Position - ShortPath.Points[0].Position).GetSafeNormal());
		for (uint8 Index = 1; Index < ShortPath.NumPoints; ++Index)
		{
			FMassNavMeshPathPoint& Previous = ShortPath.Points[Index - 1];
			FMassNavMeshPathPoint& Point = ShortPath.Points[Index];
			PathDistance += static_cast<float>(FVector::Dist(Previous.Position, Point.Position));
			Point.Distance.Set(PathDistance);
			Point.Tangent = Index + 1 < ShortPath.NumPoints
				? FMassSnorm8Vector2D(0.5 * ((Point.Position - Previous.Position).GetSafeNormal() + (ShortPath.Points[Index + 1].Position - Point.Position).GetSafeNormal()))
				: FMassSnorm8Vector2D((Point.Position - Previous.Position).GetSafeNormal());
		}
		ShortPath.MoveTargetProgressDistance = ShortPath.Points[NumLeadingPoints].Distance.Get();
	}

	/**
	 * The next part of the corridor as the unit's short path, each point moved Lateral cm to the right of the march,
	 * within its portal (the corridor's width there), so that the units of a cluster walk side by side instead of in a
	 * file. Measured along the path's own right, whatever side the corridor calls left.
	 */
	void RequestShortPath(FMassNavMeshShortPathFragment& ShortPath, FMassNavMeshCachedPathFragment& CachedPath, const int32 StartIndex,
		const uint8 NumLeadingPoints, const float Lateral)
	{
		ShortPath.RequestShortPath(CachedPath.Corridor, StartIndex, NumLeadingPoints, EndReachedDistance);
		CachedPath.NavPathNextStartIndex = static_cast<uint16>(StartIndex + FMath::Max(ShortPath.NumPoints - FMassNavMeshShortPathFragment::NumPointsBeyondUpdate
			- FMassNavMeshCachedPathFragment::NumLeadingPoints, 0));
		if (FMath::IsNearlyZero(Lateral) || ShortPath.NumPoints < 2)
		{
			return;
		}
		TStaticArray<FVector, FMassNavMeshShortPathFragment::MaxPoints> Moved;
		for (uint8 Index = 0; Index < ShortPath.NumPoints; ++Index)
		{
			const FMassNavMeshPathPoint& Point = ShortPath.Points[Index];
			const FVector Before = ShortPath.Points[FMath::Max(Index - 1, 0)].Position;
			const FVector After = ShortPath.Points[FMath::Min<int32>(Index + 1, ShortPath.NumPoints - 1)].Position;
			const FVector Along = FVector(After - Before).GetSafeNormal2D();
			const FVector Right(-Along.Y, Along.X, 0.);
			const double ToLeft = FVector::DotProduct(Point.Left - Point.Position, Right);
			const double ToRight = FVector::DotProduct(Point.Right - Point.Position, Right);
			const double Offset = FMath::Clamp<double>(Lateral, FMath::Min(ToLeft, ToRight), FMath::Max(ToLeft, ToRight));
			Moved[Index] = Point.Position + Right * Offset;
		}
		for (uint8 Index = 0; Index < ShortPath.NumPoints; ++Index)
		{
			ShortPath.Points[Index].Position = Moved[Index];
		}
		RecomputeShortPath(ShortPath, NumLeadingPoints);
	}

	/** Distance along the short path of the point nearest Location: a unit ahead of its cluster's start does not walk back. */
	float GetProgressAt(const FMassNavMeshShortPathFragment& ShortPath, const FVector& Location)
	{
		float Best = 0.f;
		double BestDistanceSquared = TNumericLimits<double>::Max();
		for (uint8 Index = 0; Index + 1 < ShortPath.NumPoints; ++Index)
		{
			const FVector& A = ShortPath.Points[Index].Position;
			const FVector& B = ShortPath.Points[Index + 1].Position;
			const FVector Closest = FMath::ClosestPointOnSegment(Location, A, B);
			const double DistanceSquared = FVector::DistSquared2D(Location, Closest);
			if (DistanceSquared < BestDistanceSquared)
			{
				BestDistanceSquared = DistanceSquared;
				Best = ShortPath.Points[Index].Distance.Get() + static_cast<float>(FVector::Dist(A, Closest));
			}
		}
		return Best;
	}

	/** From where it is straight to its slot, standing there (the M1 move of D228, now the end of every order). */
	void StartFinalLeg(const UWorld& World, const FTransformFragment& Transform, const FHOLUnitTypeConstants& Constants, FHOLMoveOrderFragment& Order,
		FMassMoveTargetFragment& MoveTarget, FMassNavMeshShortPathFragment& ShortPath, FMassNavMeshCachedPathFragment& CachedPath)
	{
		ShortPath.Reset();
		CachedPath.NavPath.Reset();
		CachedPath.Corridor.Reset();
		CachedPath.PathSource = EMassNavigationPathSource::Unset;

		const FVector Location = Transform.GetTransform().GetLocation();
		const FVector ToSlot(Order.Slot.X - Location.X, Order.Slot.Y - Location.Y, 0.);
		MoveTarget.Center = Order.Slot;
		// A zero forward would leave the steering without a reference: keep the facing when the slot is here.
		MoveTarget.Forward = ToSlot.IsNearlyZero() ? Transform.GetTransform().GetRotation().GetForwardVector() : ToSlot.GetSafeNormal();
		MoveTarget.DistanceToGoal = static_cast<float>(ToSlot.Size());
		MoveTarget.EntityDistanceToGoal = FMassMoveTargetFragment::UnsetDistance;
		MoveTarget.SlackRadius = 0.f;
		MoveTarget.IntentAtGoal = EMassMovementAction::Stand; // Slow down on approach, stand at the slot.
		MoveTarget.DesiredSpeed.Set(Constants.Speed);
		MoveTarget.CreateNewAction(EMassMovementAction::Move, World);

		Order.Phase = EHOLMovePhase::FinalLeg;
		Order.LastSlotDistance = MoveTarget.DistanceToGoal;
		Order.StuckSteps = 0;
	}

	/**
	 * Slots of an order around Target (D287), on the navmesh: a honeycomb oriented along the march, the target itself
	 * first; a slot is kept only if the navmesh has it where it is (not in an obstacle), reaches it straight from the
	 * target, and no standing unit outside the order is on it (units sent one by one to a rally point stand side by side).
	 */
	void MakeOrderSlots(const FVector& Target, const FVector2D& Forward, const int32 NumUnits, const float Radius, const UNavigationSystemV1* NavSystem,
		const ANavigationData* NavData, TConstArrayView<FVector2D> Occupied, TArray<FVector>& OutSlots)
	{
		OutSlots.Reset();
		const double Spacing = 2. * Radius * HOLGroupMove::SlotSpacingInDiameters;
		TArray<FVector2D> Offsets;
		HOLGroupMove::MakeSlots(NumUnits + NumUnits / 3 + 6, Spacing, Offsets);
		const FVector Extent(Spacing * 0.5, Spacing * 0.5, 500.);
		for (const FVector2D& Offset : Offsets)
		{
			const FVector2D World2D = FVector2D(Target) + HOLGroupMove::ToWorld(Offset, Forward);
			const FVector Wanted(World2D.X, World2D.Y, Target.Z);
			const double TakenDistance = Radius * 1.8;
			if (Occupied.ContainsByPredicate([&World2D, TakenDistance](const FVector2D& Taken) { return FVector2D::DistSquared(Taken, World2D) < TakenDistance * TakenDistance; }))
			{
				continue;
			}
			if (NavSystem && NavData)
			{
				FNavLocation OnNav;
				FVector Hit;
				if (!NavSystem->ProjectPointToNavigation(Wanted, OnNav, Extent, NavData)
					|| FVector::Dist2D(OnNav.Location, Wanted) > Spacing * 0.4
					|| NavData->Raycast(Target, OnNav.Location, Hit, NavData->GetDefaultQueryFilter()))
				{
					continue;
				}
				OutSlots.Add(OnNav.Location);
			}
			else
			{
				OutSlots.Add(Wanted);
			}
			if (OutSlots.Num() == NumUnits)
			{
				break;
			}
		}
	}
}

UHOLUnitOrderProcessor::UHOLUnitOrderProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // orders apply on the server's step (D231)
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavMeshPathFollowProcessor"));
	bRequiresGameThreadExecution = true; // the order queue and the navigation belong to the game thread
}

void UHOLUnitOrderProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNavMeshShortPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNavMeshCachedPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLUnitOrderProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance() : nullptr;
	ApplyQueuedOrders(EntityManager, NavSystem, NavData);
	ServePathRequests(EntityManager, NavSystem, NavData);
	AdvanceUnits(EntityManager, Context);
}

bool UHOLUnitOrderProcessor::IsClaimLive(const FMassEntityManager& EntityManager, const FMassEntityHandle Entity, const FIntPoint& Cell)
{
	const FHOLMoveOrderFragment* Order = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
	return Order && Order->Phase != EHOLMovePhase::None && GetClaimCell(FVector2D(Order->Slot)) == Cell;
}

void UHOLUnitOrderProcessor::ApplyQueuedOrders(FMassEntityManager& EntityManager, const UNavigationSystemV1* NavSystem, const ANavigationData* NavData)
{
	using namespace UE::HeroesOfLiberty::Orders;
	UWorld* World = EntityManager.GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (Units == nullptr || Commands == nullptr)
	{
		return;
	}

	TArray<FHOLMoveOrder> Orders;
	Commands->ConsumeMoveOrders(Orders);

	// Claims of units that stood or got another order since: swept every 10 s or so.
	if (++StepsSinceClaimSweep >= 200)
	{
		StepsSinceClaimSweep = 0;
		for (auto It = SlotClaims.CreateIterator(); It; ++It)
		{
			const FIntPoint Cell = It.Key();
			It.Value().RemoveAll([&EntityManager, &Cell](const FMassEntityHandle Entity) { return !IsClaimLive(EntityManager, Entity, Cell); });
			if (It.Value().Num() == 0)
			{
				It.RemoveCurrent();
			}
		}
	}
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const float CorridorHalfWidth = Settings->PathCorridorWidth * 0.5f;

	TArray<FMassEntityHandle> Entities;
	TArray<FVector2D> Positions;
	TArray<FVector> Slots;
	TArray<FVector2D> Slots2D;
	TArray<int32> SlotOfUnit;
	TArray<int32> ClusterOfUnit;
	TArray<FVector2D> Occupied;
	TArray<FMassNavigationObstacleItem> Nearby;
	const UMassNavigationSubsystem* Navigation = World->GetSubsystem<UMassNavigationSubsystem>();
	const FNavigationObstacleHashGrid2D* ObstacleGrid = Navigation ? &Navigation->GetObstacleGrid() : nullptr;
	for (const FHOLMoveOrder& Order : Orders)
	{
		// A unit can be ordered before it is born or after it is gone: it is left out (M4 will keep the orders a
		// player gave to a group that is still being created).
		Entities.Reset();
		Positions.Reset();
		float Radius = 0.f;
		int32 NumRefused = 0;
		for (const uint32 UnitId : Order.UnitIds)
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			if (!EntityManager.IsEntityValid(Entity) || Entities.Contains(Entity))
			{
				continue;
			}
			// Only a player's own units obey it (D288): a client could send any identifier.
			if (Order.PlayerIndex != INDEX_NONE && EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).OwnerPlayer != Order.PlayerIndex)
			{
				++NumRefused;
				continue;
			}
			Entities.Add(Entity);
			Positions.Add(FVector2D(EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation()));
			Radius = FMath::Max(Radius, EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity).Radius);
		}
		if (NumRefused > 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Move order of slot %d: %d units ignored, not its own"), Order.PlayerIndex, NumRefused);
		}
		if (Entities.Num() == 0)
		{
			continue;
		}
		const uint32 OrderSerial = NextOrderSerial++;

		// Slots (D287): one unit goes to the target itself; a group to a honeycomb oriented along the march.
		FVector2D Centre = FVector2D::ZeroVector;
		for (const FVector2D& Position : Positions)
		{
			Centre += Position;
		}
		Centre /= Positions.Num();
		// Standing units outside the order around the target: their places are taken.
		Occupied.Reset();
		if (ObstacleGrid)
		{
			const double Reach = 2. * Radius * HOLGroupMove::SlotSpacingInDiameters * (FMath::Sqrt(Entities.Num() * 1.4 + 6.) + 1.);
			Nearby.Reset();
			ObstacleGrid->Query(FBox(Order.Target - FVector(Reach, Reach, 1000.), Order.Target + FVector(Reach, Reach, 1000.)), Nearby);
			for (const FMassNavigationObstacleItem& Item : Nearby)
			{
				const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Item.Entity);
				if (State && State->Activity == EHOLUnitActivity::Idle && !Entities.Contains(Item.Entity))
				{
					Occupied.Add(FVector2D(EntityManager.GetFragmentDataChecked<FTransformFragment>(Item.Entity).GetTransform().GetLocation()));
				}
			}
			// And the slots already given to units on their way there.
			const FIntPoint MinCell = GetClaimCell(FVector2D(Order.Target) - FVector2D(Reach));
			const FIntPoint MaxCell = GetClaimCell(FVector2D(Order.Target) + FVector2D(Reach));
			for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
			{
				for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
				{
					const FIntPoint Cell(X, Y);
					TArray<FMassEntityHandle>* Claims = SlotClaims.Find(Cell);
					if (!Claims)
					{
						continue;
					}
					Claims->RemoveAll([&EntityManager, &Cell](const FMassEntityHandle Entity) { return !IsClaimLive(EntityManager, Entity, Cell); });
					for (const FMassEntityHandle Entity : *Claims)
					{
						if (!Entities.Contains(Entity))
						{
							Occupied.Add(FVector2D(EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity).Slot));
						}
					}
				}
			}
		}
		Slots.Reset();
		MakeOrderSlots(Order.Target, FVector2D(Order.Target) - Centre, Entities.Num(), Radius, NavSystem, NavData, Occupied, Slots);
		if (Slots.Num() == 0)
		{
			Slots.Add(Order.Target);
		}
		Slots2D.Reset();
		for (const FVector& Slot : Slots)
		{
			Slots2D.Add(FVector2D(Slot));
		}
		HOLGroupMove::AssignSlots(Positions, Slots2D, SlotOfUnit);

		// Clusters (D286): one path each, from their centre.
		const int32 NumClusters = HOLGroupMove::MakeClusters(Positions, HOLGroupMove::ClusterLinkDistance, ClusterOfUnit);
		const int32 FirstRequest = PendingPaths.Num();
		for (int32 Cluster = 0; Cluster < NumClusters; ++Cluster)
		{
			FPathRequest& Request = PendingPaths.AddDefaulted_GetRef();
			Request.OrderSerial = OrderSerial;
			Request.Target = Order.Target;
		}
		TArray<FVector> ClusterSums;
		ClusterSums.Init(FVector::ZeroVector, NumClusters);
		for (int32 Unit = 0; Unit < Entities.Num(); ++Unit)
		{
			PendingPaths[FirstRequest + ClusterOfUnit[Unit]].Entities.Add(Entities[Unit]);
			ClusterSums[ClusterOfUnit[Unit]] += EntityManager.GetFragmentDataChecked<FTransformFragment>(Entities[Unit]).GetTransform().GetLocation();
		}
		for (int32 Cluster = 0; Cluster < NumClusters; ++Cluster)
		{
			FPathRequest& Request = PendingPaths[FirstRequest + Cluster];
			Request.Start = ClusterSums[Cluster] / Request.Entities.Num();
		}

		for (int32 Unit = 0; Unit < Entities.Num(); ++Unit)
		{
			const FMassEntityHandle Entity = Entities[Unit];
			const FPathRequest& Request = PendingPaths[FirstRequest + ClusterOfUnit[Unit]];
			// Its sideways place in its cluster, to the right of the cluster's march, within the corridor.
			const FVector2D Forward = (FVector2D(Request.Target) - FVector2D(Request.Start)).GetSafeNormal();
			const FVector2D Right(-Forward.Y, Forward.X);
			const double Lateral = Forward.IsNearlyZero() ? 0. : FVector2D::DotProduct(Positions[Unit] - FVector2D(Request.Start), Right);

			FHOLMoveOrderFragment& MoveOrder = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity);
			MoveOrder.OrderSerial = OrderSerial;
			MoveOrder.Slot = SlotOfUnit[Unit] != INDEX_NONE ? Slots[SlotOfUnit[Unit]] : Order.Target;
			MoveOrder.Lateral = static_cast<float>(FMath::Clamp(Lateral, -(double)CorridorHalfWidth + Radius, (double)CorridorHalfWidth - Radius));
			MoveOrder.Phase = EHOLMovePhase::WaitingPath;
			SlotClaims.FindOrAdd(GetClaimCell(FVector2D(MoveOrder.Slot))).AddUnique(Entity);
			// Moving from now on: the unit keeps its current walk until its path comes (S2 orders only idle units).
			EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).Activity = EHOLUnitActivity::Moving;
		}
	}
}

void UHOLUnitOrderProcessor::ServePathRequests(FMassEntityManager& EntityManager, UNavigationSystemV1* NavSystem, const ANavigationData* NavData)
{
	using namespace UE::HeroesOfLiberty::Orders;
	UWorld* World = EntityManager.GetWorld();
	if (World == nullptr || PendingPaths.Num() == 0)
	{
		return;
	}
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const int32 Budget = PathBudgetOverride >= 0 ? PathBudgetOverride : Settings->PathRequestsPerStep;

	int32 Served = 0;
	int32 Next = 0;
	for (; Next < PendingPaths.Num() && (Budget == 0 || Served < Budget); ++Next)
	{
		FPathRequest& Request = PendingPaths[Next];
		// Its units that still follow this order (a newer order replaces a path not yet computed).
		Request.Entities.RemoveAll([&EntityManager, &Request](const FMassEntityHandle Entity)
		{
			return !EntityManager.IsEntityValid(Entity)
				|| EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity).OrderSerial != Request.OrderSerial;
		});
		if (Request.Entities.Num() == 0)
		{
			continue;
		}
		++Served;

		FNavPathSharedPtr Path;
		if (NavSystem && NavData && DirectPaths == 0)
		{
			FNavLocation Start;
			const FVector StartExtent(500.f, 500.f, 500.f);
			if (NavSystem->ProjectPointToNavigation(Request.Start, Start, StartExtent, NavData))
			{
				const FPathFindingQuery Query(World, *NavData, Start.Location, Request.Target);
				const FPathFindingResult Result = NavSystem->FindPathSync(Query);
				if (Result.IsSuccessful() && Result.Path.IsValid())
				{
					Result.Path->RemoveOverlappingPoints(FNavCorridor::OverlappingPointTolerance);
					if (Result.Path->GetPathPoints().Num() >= 2)
					{
						Path = Result.Path;
					}
				}
			}
		}
		// A straight path (nothing in the way, the usual case in the open) needs no corridor: the units walk straight
		// to their slots, which costs no path follow (S2: +0.3 ms of path follow and avoidance for nothing otherwise).
		const bool bDetour = Path.IsValid() && Path->GetPathPoints().Num() > 2;
		const TSharedPtr<FNavCorridor> Corridor = bDetour ? MakeCorridor(*Path, *NavData, Settings->PathCorridorWidth) : nullptr;
		const bool bFollowPath = Corridor.IsValid() && Corridor->Portals.Num() >= 2 && DirectPaths == 0;

		for (const FMassEntityHandle Entity : Request.Entities)
		{
			FHOLMoveOrderFragment& Order = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity);
			const FTransformFragment& Transform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
			const FHOLUnitTypeConstants& Constants = EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity);
			FMassMoveTargetFragment& MoveTarget = EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(Entity);
			FMassNavMeshShortPathFragment& ShortPath = EntityManager.GetFragmentDataChecked<FMassNavMeshShortPathFragment>(Entity);
			FMassNavMeshCachedPathFragment& CachedPath = EntityManager.GetFragmentDataChecked<FMassNavMeshCachedPathFragment>(Entity);
			if (Path.IsValid() && Path->IsPartial())
			{
				// The target cannot be reached: the units gather where the path ends.
				Order.Slot = Path->GetPathPoints().Last().Location;
			}
			if (!bFollowPath)
			{
				// A straight path, or none (start off the navmesh, already there): straight to the slot.
				StartFinalLeg(*World, Transform, Constants, Order, MoveTarget, ShortPath, CachedPath);
				continue;
			}
			CachedPath.NavPath = Path;
			CachedPath.Corridor = Corridor;
			CachedPath.PathSource = EMassNavigationPathSource::NavMesh;
			RequestShortPath(ShortPath, CachedPath, /*StartIndex*/0, /*NumLeadingPoints*/0, Order.Lateral);
			// A unit ahead of the cluster's start goes on from where it is (short of the last point, so it still ends).
			const float LastDistance = ShortPath.Points[ShortPath.NumPoints - 1].Distance.Get();
			ShortPath.MoveTargetProgressDistance = FMath::Clamp(GetProgressAt(ShortPath, Transform.GetTransform().GetLocation()), 0.f,
				FMath::Max(LastDistance - EndReachedDistance * 2.f, 0.f));

			MoveTarget.DesiredSpeed.Set(Constants.Speed);
			// The slot comes after the path: no slowing down at its end.
			MoveTarget.IntentAtGoal = EMassMovementAction::Move;
			MoveTarget.SlackRadius = 0.f;
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
			Order.Phase = EHOLMovePhase::Path;
		}
	}
	PendingPaths.RemoveAt(0, Next, EAllowShrinking::No);
	NumPathsServed += Served;
}

void UHOLUnitOrderProcessor::AdvanceUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Orders;
	UWorld* World = EntityManager.GetWorld();
	if (World == nullptr)
	{
		return;
	}
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const float Tolerance = Settings->OrderArrivalTolerance;
	const int32 StuckLimit = Settings->FinalLegStuckSteps;

	EntityQuery.ForEachEntityChunk(Context, [World, Tolerance, StuckLimit](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLUnitStateFragment> States = Context.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TArrayView<FHOLMoveOrderFragment> Orders = Context.GetMutableFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FMassNavMeshShortPathFragment> ShortPaths = Context.GetMutableFragmentView<FMassNavMeshShortPathFragment>();
		const TArrayView<FMassNavMeshCachedPathFragment> CachedPaths = Context.GetMutableFragmentView<FMassNavMeshCachedPathFragment>();
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FHOLMoveOrderFragment& Order = Orders[EntityIt];
			FMassMoveTargetFragment& MoveTarget = MoveTargets[EntityIt];
			FMassNavMeshShortPathFragment& ShortPath = ShortPaths[EntityIt];
			FMassNavMeshCachedPathFragment& CachedPath = CachedPaths[EntityIt];
			const FTransformFragment& Transform = Transforms[EntityIt];

			if (Order.Phase == EHOLMovePhase::Path && ShortPath.IsDone())
			{
				if (ShortPath.bPartialResult && CachedPath.Corridor.IsValid())
				{
					// The short path covers part of the corridor: on to the next part (one point kept behind).
					RequestShortPath(ShortPath, CachedPath, CachedPath.NavPathNextStartIndex, FMassNavMeshCachedPathFragment::NumLeadingPoints, Order.Lateral);
				}
				else
				{
					StartFinalLeg(*World, Transform, Constants, Order, MoveTarget, ShortPath, CachedPath);
				}
				continue;
			}
			if (Order.Phase != EHOLMovePhase::FinalLeg)
			{
				continue;
			}

			const FVector ToSlot = Order.Slot - Transform.GetTransform().GetLocation();
			const float Distance = static_cast<float>(FVector(ToSlot.X, ToSlot.Y, 0.).Size());
			// Mass only maintains the distance to the goal while a navmesh path is followed. Without one it believes
			// the unit has arrived and scales the avoidance force down to EndOfPathAvoidanceScale (10 %): the walkers
			// would cross each other. Kept up to date here, and the goal's direction with it (D228).
			MoveTarget.DistanceToGoal = Distance;
			if (!FVector(ToSlot.X, ToSlot.Y, 0.).IsNearlyZero())
			{
				MoveTarget.Forward = FVector(ToSlot.X, ToSlot.Y, 0.).GetSafeNormal();
			}
			// A unit that no longer gets closer (its slot taken, pushed by its neighbours) stands where it is.
			Order.StuckSteps = Distance < Order.LastSlotDistance - 2.f ? 0 : static_cast<uint16>(FMath::Min<int32>(Order.StuckSteps + 1, MAX_uint16));
			Order.LastSlotDistance = FMath::Min(Order.LastSlotDistance, Distance);
			if (Distance > Tolerance && Order.StuckSteps < StuckLimit)
			{
				continue;
			}
			MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
			Order.Phase = EHOLMovePhase::None;
			States[EntityIt].Activity = EHOLUnitActivity::Idle;
		}
	});
}

namespace UE::HeroesOfLiberty::Orders
{
	/** Scripted tries of J4 (D289): every unit, or those of one slot, ordered to a ground point as one group. */
	FAutoConsoleCommandWithWorldAndArgs MoveCommand(TEXT("hol.Units.Move"),
		TEXT("hol.Units.Move X Y [Slot]: orders the units (of one slot, or all) to the ground point X Y (cm), as one group (server or solo)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Units.Move X Y [Slot]"));
				return;
			}
			const FVector Target(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
			const int32 Slot = Args.Num() >= 3 ? FCString::Atoi(*Args[2]) : INDEX_NONE;
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				UHOLUnitSubsystem* Units = World && World->IsGameWorld() && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
				UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
				UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
				if (!Units || !Commands || !EntitySubsystem)
				{
					continue;
				}
				const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
				TArray<uint32> UnitIds;
				Units->GetAllUnitIds(UnitIds);
				UnitIds.RemoveAll([&](const uint32 UnitId)
				{
					const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(UnitId));
					return State == nullptr || (Slot != INDEX_NONE && State->OwnerPlayer != Slot);
				});
				Commands->IssueMoveOrder(UnitIds, Target);
				UE_LOG(LogHOLSimulation, Display, TEXT("Move order: %d units to (%.0f, %.0f)"), UnitIds.Num(), Target.X, Target.Y);
			}
		}));
}
