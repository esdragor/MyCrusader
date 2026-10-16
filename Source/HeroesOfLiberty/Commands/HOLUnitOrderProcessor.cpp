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
#include "MassMovementFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationSubsystem.h"
#include "NavCorridor.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "Simulation/HOLBuildFragments.h"
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

	int32 OrderTiming = 0;
	FAutoConsoleVariableRef CVarOrderTiming(TEXT("hol.Units.OrderTiming"), OrderTiming,
		TEXT("1: the average cost of the order processor's parts (orders applied, paths served, units advanced) logged every 200 steps (D302)"));

	int32 ReusePaths = 1;
	FAutoConsoleVariableRef CVarReusePaths(TEXT("hol.Units.ReusePaths"), ReusePaths,
		TEXT("1: a task's walk (J5: to the source, to the deposit) takes the path kept from a walk between the same points (D305); 0: a new request each time."));

	int32 CheckReusedStraight = 1;
	FAutoConsoleVariableRef CVarCheckReusedStraight(TEXT("hol.Units.ReuseCheckStraight"), CheckReusedStraight,
		TEXT("1: a kept straight walk is reused only if a navmesh ray from the unit to its target is clear (D305)."));

	int32 CheckReusedDetour = 1;
	FAutoConsoleVariableRef CVarCheckReusedDetour(TEXT("hol.Units.ReuseCheckDetour"), CheckReusedDetour,
		TEXT("1: a kept detour is reused only where this walk needs one and it fits it (D308): a clear navmesh ray walks straight; ")
		TEXT("the walk must see the detour's first corner and its last corner its target, for no longer a walk than the kept one's."));

	int32 LogDetours = 0;
	FAutoConsoleVariableRef CVarLogDetours(TEXT("hol.Units.LogDetours"), LogDetours,
		TEXT("1: every task walk much longer than the straight line is logged (D308: kept or new path, lengths)."));

	float SpeedScale = 1.f;
	FAutoConsoleVariableRef CVarSpeedScale(TEXT("hol.Units.SpeedScale"), SpeedScale,
		TEXT("Every unit walks this many times as fast, on this server (development: hol.Cheat.UnitSpeed and the cheats' panel set it; D291); ")
		TEXT("a walker takes a new value at the next step. 1: the table's speeds"));
	/** What hol.Units.SpeedScale is held to. */
	constexpr float MinSpeedScale = 0.1f;
	constexpr float MaxSpeedScale = 10.f;

	/** A unit's walking speed (cm/s): its type's, times hol.Units.SpeedScale. */
	float GetWalkSpeed(const FHOLUnitTypeConstants& Constants)
	{
		return Constants.Speed * FMath::Clamp(SpeedScale, MinSpeedScale, MaxSpeedScale);
	}

	/** A walk is a detour for hol.Units.LogDetours when longer than this share of the straight line, plus DetourSlack. */
	constexpr double DetourRatio = 1.3;
	constexpr double DetourSlack = 300.;
	/** A kept detour fits a walk if its own length is at most this share of the kept walk's (over the straight line), plus the slack. */
	constexpr double ReusedDetourRatioMargin = 1.1;

	/** A partial path ending closer than this (cm) to its start: the unit is shut in a pocket of the navmesh (D308). */
	constexpr double PocketEscapeDistance = 200.;

	/** Length (cm) of a walk from Start along Points (from the second one) to Target, as a unit takes a kept path. */
	double GetSplicedLength(const TArray<FNavPathPoint>& Points, const FVector& Start, const FVector& Target)
	{
		double Length = FVector::Dist2D(Start, Points[1].Location);
		for (int32 Index = 1; Index + 2 < Points.Num(); ++Index)
		{
			Length += FVector::Dist2D(Points[Index].Location, Points[Index + 1].Location);
		}
		return Length + FVector::Dist2D(Points[Points.Num() - 2].Location, Target);
	}

	double GetPathLength(const TArray<FNavPathPoint>& Points)
	{
		double Length = 0.;
		for (int32 Index = 0; Index + 1 < Points.Num(); ++Index)
		{
			Length += FVector::Dist2D(Points[Index].Location, Points[Index + 1].Location);
		}
		return Length;
	}

	float ReuseCellSize = 600.f;
	FAutoConsoleVariableRef CVarReuseCellSize(TEXT("hol.Units.ReuseCellSize"), ReuseCellSize,
		TEXT("Size (cm) of the cells of a kept path's start and end (D305): a walk takes it when it starts and ends in the same cells."));

	/** A kept path no walk took for this many steps (60 s) is forgotten; the register is swept every SweepSteps. */
	constexpr uint32 ReuseForgetSteps = 1200;
	constexpr uint32 ReuseSweepSteps = 200;
	/** Steps (5 s) during which a path found across a changed area is not kept: the tiles there are being rebuilt. */
	constexpr uint32 NavChangeSettleSteps = 100;
	/** Margin (cm) around a kept path within which a change of the navmesh forgets it: half the corridor and more. */
	constexpr double ReuseBoundsMargin = 400.;

	/** Time spent in each part and steps run since the last log (hol.Units.OrderTiming 1). */
	double TimingSeconds[3] = {};
	int32 TimingRuns = 0;

	/** A short path is done this close (cm) to its last point; the final leg brings the unit to its slot. */
	constexpr float EndReachedDistance = 50.f;
	/** Distance (cm) to its slot within which a unit that no longer gets closer counts as stuck (D287, D292). */
	constexpr float StuckCountDistance = 500.f;
	/** Distance (cm) to the end of its path within which a unit leaves it for its slot when the navmesh lets it walk straight there (D294). */
	constexpr double LeavePathDistance = 300.0;
	/** Steps in a row a building puts a walker back (D310) before it gets a new path: half a second. */
	constexpr uint8 RepathBuildingPushes = 10;

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
		MoveTarget.DesiredSpeed.Set(GetWalkSpeed(Constants));
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
		// Room for many more than the units: when standing units hold the nearest places, the next rings serve (a unit
		// left without a slot went to the target itself, several on one point, D292). Tried in order until enough.
		HOLGroupMove::MakeSlots(NumUnits * 3 + 20, Spacing, Offsets);
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

int64 UHOLUnitOrderProcessor::TotalPathsServed = 0;
int64 UHOLUnitOrderProcessor::TotalPathsReused = 0;
int64 UHOLUnitOrderProcessor::TotalBuildingRepaths = 0;

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
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNavMeshShortPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassNavMeshCachedPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLUnitOrderProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Orders;
	UWorld* World = EntityManager.GetWorld();
	UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance() : nullptr;
	const double StartSeconds = FPlatformTime::Seconds();
	++StepCount;
	if (UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr)
	{
		ForgetChangedPaths(*Commands);
	}
	ApplyQueuedOrders(EntityManager, NavSystem, NavData);
	const double OrdersSeconds = FPlatformTime::Seconds();
	ServePathRequests(EntityManager, NavSystem, NavData);
	const double PathsSeconds = FPlatformTime::Seconds();
	AdvanceUnits(EntityManager, Context, NavData);
	if (OrderTiming != 0)
	{
		TimingSeconds[0] += OrdersSeconds - StartSeconds;
		TimingSeconds[1] += PathsSeconds - OrdersSeconds;
		TimingSeconds[2] += FPlatformTime::Seconds() - PathsSeconds;
		if (++TimingRuns == 200)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Orders: %.3f ms per step on average (orders applied %.3f, paths %.3f, units advanced %.3f)"),
				(TimingSeconds[0] + TimingSeconds[1] + TimingSeconds[2]) * 1000. / TimingRuns, TimingSeconds[0] * 1000. / TimingRuns,
				TimingSeconds[1] * 1000. / TimingRuns, TimingSeconds[2] * 1000. / TimingRuns);
			TimingSeconds[0] = TimingSeconds[1] = TimingSeconds[2] = 0.;
			TimingRuns = 0;
		}
	}
}

FIntVector4 UHOLUnitOrderProcessor::GetReuseKey(const FVector& Start, const FVector& Target)
{
	using namespace UE::HeroesOfLiberty::Orders;
	const double Cell = FMath::Max(ReuseCellSize, 1.f);
	return FIntVector4(FMath::FloorToInt32(Start.X / Cell), FMath::FloorToInt32(Start.Y / Cell), FMath::FloorToInt32(Target.X / Cell),
		FMath::FloorToInt32(Target.Y / Cell));
}

void UHOLUnitOrderProcessor::ForgetChangedPaths(UHOLCommandSubsystem& Commands)
{
	using namespace UE::HeroesOfLiberty::Orders;
	TArray<FBox> Changed;
	Commands.ConsumeNavigationChanges(Changed);
	RecentNavChanges.RemoveAll([this](const FRecentNavChange& Change) { return Change.UntilStep < StepCount; });
	const int32 FirstNew = RecentNavChanges.Num();
	for (const FBox& Area : Changed)
	{
		RecentNavChanges.Add({ FBox2D(FVector2D(Area.Min), FVector2D(Area.Max)), StepCount + NavChangeSettleSteps });
	}
	// The kept paths across a new change are forgotten once; until its tiles are rebuilt, a path found across it is
	// not kept (ServePathRequests), since it may cross what the navmesh does not show yet (a building placed).
	const TConstArrayView<FRecentNavChange> NewChanges = TConstArrayView<FRecentNavChange>(RecentNavChanges).RightChop(FirstNew);
	const bool bSweep = StepCount % ReuseSweepSteps == 0;
	if (NewChanges.IsEmpty() && !bSweep)
	{
		return;
	}
	for (auto It = ReusedPaths.CreateIterator(); It; ++It)
	{
		const FReusedPath& Kept = It.Value();
		if ((bSweep && StepCount - Kept.LastUsedStep > ReuseForgetSteps)
			|| NewChanges.ContainsByPredicate([&Kept](const FRecentNavChange& Change) { return Change.Area.Intersect(Kept.Bounds); }))
		{
			It.RemoveCurrent();
		}
	}
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
		// A task's walk of one unit (J5: a gatherer's spot, chosen free and on the navmesh by its gathering): the target
		// is its slot, no slot to look for (S7: the slot search was most of the orders' cost).
		const bool bTaskSpot = Order.bFromTask && Entities.Num() == 1;
		// Standing units outside the order around the target: their places are taken.
		Occupied.Reset();
		if (ObstacleGrid && !bTaskSpot)
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
		if (!bTaskSpot)
		{
			MakeOrderSlots(Order.Target, FVector2D(Order.Target) - Centre, Entities.Num(), Radius, NavSystem, NavData, Occupied, Slots);
		}
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
			Request.bReusable = bTaskSpot;
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
			// A player's order ends the unit's gathering, its load kept (D296); a walk its gathering gave it does not.
			FHOLGatherFragment* Gather = Order.bFromTask ? nullptr : EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity);
			if (Gather && Gather->IsGathering())
			{
				Gather->Phase = EHOLGatherPhase::None;
				Gather->SourceIndex = INDEX_NONE;
				Gather->bWalkIssued = false;
			}
			// And its building, its queue of sites forgotten (J6a.4, D312).
			if (FHOLBuildFragment* Build = Order.bFromTask ? nullptr : EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(Entity))
			{
				Build->Clear();
			}
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
	for (; Next < PendingPaths.Num(); ++Next)
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

		// A task's walk between the same cells as a kept one takes its path (D305): no request, nothing off the budget.
		const bool bReuse = Request.bReusable && ReusePaths != 0 && DirectPaths == 0 && NavData;
		const FIntVector4 ReuseKey = bReuse ? GetReuseKey(Request.Start, Request.Target) : FIntVector4();
		FReusedPath* Kept = bReuse ? ReusedPaths.Find(ReuseKey) : nullptr;
		// A kept straight walk is straight between its own points: this one, between others of the same cells, only if
		// the navmesh lets it walk straight (one ray, much cheaper than a path; a tree may stand between).
		FVector Hit;
		const bool bCheckKept = Kept && (Kept->Corridor.IsValid() ? CheckReusedDetour != 0 : CheckReusedStraight != 0);
		const bool bStraightBlocked = bCheckKept && NavData->Raycast(Request.Start, Request.Target, Hit, NavData->GetDefaultQueryFilter());
		bool bWalkStraight = false;
		if (Kept && !Kept->Corridor.IsValid() && bStraightBlocked)
		{
			Kept = nullptr;
		}
		else if (Kept && Kept->Corridor.IsValid() && bCheckKept)
		{
			// A kept detour is another walk's (D308): this one goes straight when it can; else it takes the detour only if
			// it sees its first corner, its last corner sees its target, and it walks no farther than the kept walk did
			// (the user's trial of J5b.2: gatherers went around the wood for a tree beside them).
			const TArray<FNavPathPoint>& Points = Kept->Path->GetPathPoints();
			if (!bStraightBlocked)
			{
				bWalkStraight = true;
			}
			else
			{
				const double KeptRatio = GetPathLength(Points) / FMath::Max(FVector::Dist2D(Points[0].Location, Points.Last().Location), 1.);
				const double Allowed = FVector::Dist2D(Request.Start, Request.Target) * FMath::Max(KeptRatio, 1.) * ReusedDetourRatioMargin + DetourSlack;
				if (GetSplicedLength(Points, Request.Start, Request.Target) > Allowed
					|| NavData->Raycast(Request.Start, Points[1].Location, Hit, NavData->GetDefaultQueryFilter())
					|| NavData->Raycast(Points[Points.Num() - 2].Location, Request.Target, Hit, NavData->GetDefaultQueryFilter()))
				{
					Kept = nullptr;
				}
			}
		}
		FNavPathSharedPtr Path;
		TSharedPtr<FNavCorridor> Corridor;
		if (Kept)
		{
			Kept->LastUsedStep = StepCount;
			Path = bWalkStraight ? FNavPathSharedPtr() : Kept->Path;
			Corridor = bWalkStraight ? TSharedPtr<FNavCorridor>() : Kept->Corridor;
			++TotalPathsReused;
		}
		else
		{
			if (Budget > 0 && Served >= Budget)
			{
				break;
			}
			++Served;
			if (NavSystem && NavData && DirectPaths == 0)
			{
				// Failing that, as far as an order's target is looked for (1000 cm): a unit born in a building (S7) is up to
				// ~8 m from the navmesh; at 500 cm it found no start and walked straight to its slot, into the building (D310).
				FNavLocation Start;
				const FVector StartExtent(500.f, 500.f, 500.f);
				if (NavSystem->ProjectPointToNavigation(Request.Start, Start, StartExtent, NavData)
					|| NavSystem->ProjectPointToNavigation(Request.Start, Start, FVector(Settings->OrderTargetProjectionExtent), NavData))
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
			Corridor = bDetour ? MakeCorridor(*Path, *NavData, Settings->PathCorridorWidth) : nullptr;
			// Kept for the next walk between the same cells: a whole path (not a partial one, not none), and not across
			// an area whose tiles are being rebuilt. A straight one is kept without its points (the walk goes straight).
			if (bReuse && Path.IsValid() && !Path->IsPartial())
			{
				FBox2D Bounds(ForceInit);
				for (const FNavPathPoint& Point : Path->GetPathPoints())
				{
					Bounds += FVector2D(Point.Location);
				}
				Bounds = Bounds.ExpandBy(ReuseBoundsMargin);
				if (!RecentNavChanges.ContainsByPredicate([&Bounds](const FRecentNavChange& Change) { return Change.Area.Intersect(Bounds); }))
				{
					ReusedPaths.Add(ReuseKey, { bDetour ? Path : FNavPathSharedPtr(), Corridor, Bounds, StepCount });
				}
			}
		}
		// A partial path ending where the walk starts: the unit stands in a pocket of the navmesh (pushed between the trees
		// of a wood, D308), cut off from the rest. It walks out straight to its slot, the way it came in (light units only
		// keep off obstacles through their paths); before, each walk ended at once and the gatherer gave up with its load.
		if (Path.IsValid() && Path->IsPartial() && FVector::Dist2D(Path->GetPathPoints().Last().Location, Request.Start) < PocketEscapeDistance)
		{
			UE_CLOG(LogDetours != 0, LogHOLCommands, Display, TEXT("Detour: out of a navmesh pocket at (%.0f, %.0f), straight to (%.0f, %.0f)"),
				Request.Start.X, Request.Start.Y, Request.Target.X, Request.Target.Y);
			Path.Reset();
			Corridor.Reset();
		}
		const bool bFollowPath = Corridor.IsValid() && Corridor->Portals.Num() >= 2 && DirectPaths == 0;
		if (LogDetours != 0 && Request.bReusable && bFollowPath && Path.IsValid())
		{
			const double Straight = FVector::Dist2D(Request.Start, Request.Target);
			const double Walked = GetSplicedLength(Path->GetPathPoints(), Request.Start, Request.Target);
			if (Walked > Straight * DetourRatio + DetourSlack)
			{
				UE_LOG(LogHOLCommands, Display, TEXT("Detour: %s path, %.0f cm walked for %.0f cm straight, from (%.0f, %.0f) to (%.0f, %.0f)"),
					Kept ? TEXT("kept") : TEXT("new"), Walked, Straight, Request.Start.X, Request.Start.Y, Request.Target.X, Request.Target.Y);
			}
		}

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

			MoveTarget.DesiredSpeed.Set(GetWalkSpeed(Constants));
			// The slot comes after the path: no slowing down at its end.
			MoveTarget.IntentAtGoal = EMassMovementAction::Move;
			MoveTarget.SlackRadius = 0.f;
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
			Order.Phase = EHOLMovePhase::Path;
		}
	}
	PendingPaths.RemoveAt(0, Next, EAllowShrinking::No);
	NumPathsServed += Served;
	TotalPathsServed += Served;
}

void UHOLUnitOrderProcessor::AdvanceUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context, const ANavigationData* NavData)
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

	EntityQuery.ForEachEntityChunk(Context, [this, World, Tolerance, StuckLimit, NavData](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLUnitStateFragment> States = Context.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TArrayView<FHOLMoveOrderFragment> Orders = Context.GetMutableFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FMassDesiredMovementFragment> Movements = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
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

			// A walker takes the units' speed of the cheats at once (hol.Units.SpeedScale): Mass moves its path's point on at
			// that speed too.
			if (MoveTarget.GetCurrentAction() == EMassMovementAction::Move && MoveTarget.DesiredSpeed.Get() > 0.f)
			{
				MoveTarget.DesiredSpeed.Set(GetWalkSpeed(Constants));
			}

			// Its way crosses a building the navmesh did not show yet when its path was found (S7's first paths, a building
			// placed): put back at the wall step after step (D310), it gets a new path from where it stands.
			if ((Order.Phase == EHOLMovePhase::Path || Order.Phase == EHOLMovePhase::FinalLeg) && Order.BuildingPushes >= RepathBuildingPushes)
			{
				Order.BuildingPushes = 0;
				FPathRequest& Request = PendingPaths.AddDefaulted_GetRef();
				Request.OrderSerial = Order.OrderSerial;
				Request.Start = Transform.GetTransform().GetLocation();
				Request.Target = Order.Slot;
				Request.Entities.Add(Context.GetEntity(EntityIt));
				Order.Phase = EHOLMovePhase::WaitingPath;
				++TotalBuildingRepaths;
				continue;
			}
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
			if (Order.Phase == EHOLMovePhase::Path && NavData && CachedPath.NavPath.IsValid() && CachedPath.NavPath->GetPathPoints().Num() > 0)
			{
				// The path ends at the order's point, often taken: a unit already at a rally point, a group standing
				// there. Once as close to that end as its slot is, a unit leaves the path for its slot if it can walk
				// straight there (D294): looking ahead (D293), it went round the taken end and never reached it.
				const FVector Location = Transform.GetTransform().GetLocation();
				const FVector PathEnd = CachedPath.NavPath->GetPathPoints().Last().Location;
				const double LeaveDistance = FMath::Max(LeavePathDistance, FVector::Dist2D(Order.Slot, PathEnd) + 100.);
				FVector Hit;
				if (FVector::DistSquared2D(Location, PathEnd) <= FMath::Square(LeaveDistance)
					&& !NavData->Raycast(Location, Order.Slot, Hit, NavData->GetDefaultQueryFilter()))
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
			// A unit that no longer gets closer near its slot (taken, or its neighbours in the way) stands where it is. Far
			// from it, a unit pushed back by a crowd keeps going (D292): it stopped 110 m short of its slot.
			const bool bGettingCloser = Distance < Order.LastSlotDistance - 2.f;
			Order.StuckSteps = bGettingCloser || Distance > StuckCountDistance ? 0 : static_cast<uint16>(FMath::Min<int32>(Order.StuckSteps + 1, MAX_uint16));
			Order.LastSlotDistance = FMath::Min(Order.LastSlotDistance, Distance);
			if (Distance > Tolerance && Order.StuckSteps < StuckLimit)
			{
				continue;
			}
			// It stops dead where it is (D290): Mass' standing steering would otherwise carry it on with the walk's
			// velocity, then walk it slowly back to the order's centre (its slot, which a unit stopped short of it never
			// reached).
			MoveTarget.Center = Transform.GetTransform().GetLocation();
			MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
			Movements[EntityIt].DesiredVelocity = FVector::ZeroVector;
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

	/** Diagnosis of the orders (server or solo): where each unit is in its order. */
	FAutoConsoleCommandWithWorldAndArgs OrdersCommand(TEXT("hol.Units.Orders"),
		TEXT("hol.Units.Orders [Slot]: logs each unit's order phase, slot, distance to it and steps without getting closer (server or solo)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			const int32 Slot = Args.IsEmpty() ? INDEX_NONE : FCString::Atoi(*Args[0]);
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				const UHOLUnitSubsystem* Units = World && World->IsGameWorld() && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
				const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
				if (!Units || !EntitySubsystem)
				{
					continue;
				}
				const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
				TArray<uint32> UnitIds;
				Units->GetAllUnitIds(UnitIds);
				UnitIds.Sort();
				TArray<FString> Lines;
				for (const uint32 UnitId : UnitIds)
				{
					const FMassEntityHandle Entity = Units->FindEntity(UnitId);
					const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
					const FHOLMoveOrderFragment* Order = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
					const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
					if (!State || !Order || !Transform || (Slot != INDEX_NONE && State->OwnerPlayer != Slot))
					{
						continue;
					}
					Lines.Add(FString::Printf(TEXT("#%u %s slot (%.0f, %.0f) at %.0f cm, stuck %d"), UnitId, *UEnum::GetValueAsString(Order->Phase),
						Order->Slot.X, Order->Slot.Y, FVector::Dist2D(Order->Slot, Transform->GetTransform().GetLocation()), Order->StuckSteps));
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Unit orders: %s"), *FString::Join(Lines, TEXT("; ")));
			}
		}));
}
