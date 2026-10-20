// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLUnitOrderProcessor.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "Match/HOLPlayerState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
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
#include "NavMesh/NavMeshPath.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavMesh/RecastNavMeshGenerator.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLBuildFragments.h"
#include "Simulation/HOLBuildProcessor.h"
#include "Simulation/HOLGatherProcessor.h"
#include "UObject/UObjectIterator.h"
#include "Simulation/HOLFormationRules.h"
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

	int32 BlockMarch = 1;
	FAutoConsoleVariableRef CVarBlockMarch(TEXT("hol.Units.BlockMarch"), BlockMarch,
		TEXT("1: a group's move in formation forms up, then marches as a block along one path at its slowest's pace (J7a.4, as in AoE4); 0: each unit its own way to its place"));

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

	int32 ThroughWays = 1;
	FAutoConsoleVariableRef CVarThroughWays(TEXT("hol.Units.ThroughWays"), ThroughWays,
		TEXT("1: a path that goes around buildings is compared with the path through the way between two neighbours near its line ")
		TEXT("(D324: the navmesh's search often misses that strip one body wide); 0: the navmesh's path as it is, for A / B measures."));

	int32 CutPaths = 1;
	FAutoConsoleVariableRef CVarCutPaths(TEXT("hol.Units.CutPaths"), CutPaths,
		TEXT("1: a new footprint (a site's first work, J6b.2, D314) drops the kept paths across it at once, and once its tiles are rebuilt every walker ")
		TEXT("whose way crosses it gets a new path; 0: as before (the kept paths near it forgotten, a walker repathed only once the building puts it back, D310), for A / B measures."));

	/** Steps (5 s) the tiles of a new footprint are waited for at most: its walkers are then checked anyway, and it is logged. */
	constexpr uint32 FootprintRebuildSteps = 100;
	/** Corners of a new footprint probed this far inside it (cm, HOLConstruction::GetNavProbes), as for a placement. */
	constexpr double FootprintProbeInset = 100.;
	/**
	 * Half size (cm) of the box a probe looks for the navmesh in: once the tiles are rebuilt, the navmesh ends 85 cm at least
	 * outside the footprint (agent 60, clearance 25, D310), a metre at least from a probe.
	 */
	constexpr double FootprintProbeExtent = 50.;
	/** A path ending farther than this (cm) from its slot leaves it short: the slot cannot be reached (J6b.2). */
	constexpr double ShortOfSlotDistance = 100.;
	/** Gap (cm) between a body and the clearance of a new footprint at a destination moved out of it (as a builder's spot, D312). */
	constexpr double MovedSlotGap = 20.;

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

	/** Its speed in its order (cm/s): in a group's formation, no faster than the slowest of the group's walkers (J7a.4). */
	float GetOrderWalkSpeed(const FHOLUnitTypeConstants& Constants, const FHOLMoveOrderFragment& Order)
	{
		return HOLFormation::GetMarchSpeed(Constants.Speed, Order.bFormation ? Order.MarchSpeed : 0.f) * FMath::Clamp(SpeedScale, MinSpeedScale, MaxSpeedScale);
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
	/**
	 * Width (cm) of a portal of the corridor (where the units' centres may pass) below which two units cannot walk abreast:
	 * a Peasant's diameter (D324). Through two such portals in a row (a passage) every unit walks the path itself, whatever its
	 * place across the march.
	 */
	constexpr double NarrowPortalWidth = 120.0;
	/** Distance (cm) to the end of its path within which a unit leaves it for its slot when the navmesh lets it walk straight there (D294). */
	constexpr double LeavePathDistance = 300.0;
	/** Steps in a row a building puts a walker back (D310) before it gets a new path: half a second. */
	constexpr uint8 RepathBuildingPushes = 10;
	/** Steps (3 s) a unit following its path may stay within PathStuckDistance of one spot before it gets a new path (J6b.3). */
	constexpr uint16 PathStuckSteps = 60;
	/** Distance (cm) a unit following its path must move away from its anchor to count as moving on (J6b.3). */
	constexpr float PathStuckDistance = 50.f;

	/** A path at most this much longer than the straight line is not compared with the ways between buildings (D324). */
	constexpr double ThroughWayMinRatio = 1.01;
	/** A way between buildings farther than this (cm) from the straight line of a path is not tried (D324). */
	constexpr double ThroughWayReach = 400.;
	/** Ways between buildings tried for one path, at most (two searches each), the most promising first (D324). */
	constexpr int32 MaxThroughWayTries = 2;

	/**
	 * The way between two neighbouring buildings (D324: their tiles touching, their bodies 2 x BodyInset apart) is a strip of
	 * the navmesh one body wide; the navmesh's search, its costs measured between the middles of its polygons' edges, often
	 * finds the way around the pair shorter (a third of the walks across such a way went around, 6 to 12 % longer). A path
	 * longer than its straight line is compared with the path through the middle of each way near that line (a search to the
	 * way's middle, another on to the target, spliced: points and polygons); the shorter is kept.
	 */
	FNavPathSharedPtr FindPathThroughWaysImpl(UWorld& World, UNavigationSystemV1& NavSystem, const ANavigationData& NavData, const FVector& Start,
		const FVector& Target, const FNavPathSharedPtr& Found)
	{
		double Best = GetPathLength(Found->GetPathPoints());
		if (ThroughWays == 0 || Best <= FVector::Dist2D(Start, Target) * ThroughWayMinRatio)
		{
			return Found;
		}
		const FVector2D Line[] = { FVector2D(Start), FVector2D(Target) };
		TArray<FBox2D, TInlineAllocator<32>> Bodies;
		for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
		{
			if (It->BlocksNavigation())
			{
				Bodies.Add(It->GetBodyBounds());
			}
		}
		if (Bodies.Num() < 2)
		{
			return Found;
		}
		// The ways near the straight line (it may run down the way itself, clear of both buildings), shorter by their middle.
		TArray<FBox2D> Ways;
		HOLConstruction::FindWays(Bodies, 2. * UHOLBuildingSettings::GetBodyInset() + HOLConstruction::TouchTolerance, Ways);
		TArray<TPair<double, FVector2D>, TInlineAllocator<8>> Candidates;
		for (const FBox2D& Way : Ways)
		{
			const FVector2D Middle = Way.GetCenter();
			const double Estimate = FVector2D::Distance(Line[0], Middle) + FVector2D::Distance(Middle, Line[1]);
			if (Estimate < Best - 1. && HOLConstruction::GetSegmentDistance(Way, Line[0], Line[1]) <= ThroughWayReach)
			{
				Candidates.Emplace(Estimate, Middle);
			}
		}
		Candidates.Sort([](const TPair<double, FVector2D>& A, const TPair<double, FVector2D>& B) { return A.Key < B.Key; });
		FNavPathSharedPtr Chosen = Found;
		for (int32 Index = 0; Index < FMath::Min(Candidates.Num(), MaxThroughWayTries); ++Index)
		{
			FNavLocation Middle;
			if (!NavSystem.ProjectPointToNavigation(FVector(Candidates[Index].Value, Start.Z), Middle, FVector(50., 50., 500.), &NavData))
			{
				continue;
			}
			const FPathFindingResult In = NavSystem.FindPathSync(FPathFindingQuery(&World, NavData, Start, Middle.Location));
			const FPathFindingResult Out = In.IsSuccessful() && In.Path.IsValid() && !In.Path->IsPartial()
				? NavSystem.FindPathSync(FPathFindingQuery(&World, NavData, Middle.Location, Target)) : FPathFindingResult();
			FNavMeshPath* InMesh = In.Path.IsValid() ? In.Path->CastPath<FNavMeshPath>() : nullptr;
			const FNavMeshPath* OutMesh = Out.IsSuccessful() && Out.Path.IsValid() && !Out.Path->IsPartial() ? Out.Path->CastPath<FNavMeshPath>() : nullptr;
			if (!InMesh || !OutMesh)
			{
				continue;
			}
			const double Length = GetPathLength(In.Path->GetPathPoints()) + GetPathLength(Out.Path->GetPathPoints());
			if (Length >= Best - 1.)
			{
				continue;
			}
			// Spliced at the way's middle: the second path's points after its first (the middle again), its polygons after the
			// first path's last one (the polygon of the middle, in both), with their costs.
			TArray<FNavPathPoint>& Points = In.Path->GetPathPoints();
			const TArray<FNavPathPoint>& More = Out.Path->GetPathPoints();
			for (int32 Point = 1; Point < More.Num(); ++Point)
			{
				Points.Add(More[Point]);
			}
			for (int32 Poly = 0; Poly < OutMesh->PathCorridor.Num(); ++Poly)
			{
				if (Poly == 0 && InMesh->PathCorridor.Num() > 0 && InMesh->PathCorridor.Last() == OutMesh->PathCorridor[0])
				{
					continue;
				}
				InMesh->PathCorridor.Add(OutMesh->PathCorridor[Poly]);
				if (OutMesh->PathCorridorCost.IsValidIndex(Poly))
				{
					InMesh->PathCorridorCost.Add(OutMesh->PathCorridorCost[Poly]);
				}
			}
			In.Path->EnableRecalculationOnInvalidation(false);
			In.Path->RemoveOverlappingPoints(FNavCorridor::OverlappingPointTolerance);
			Best = Length;
			Chosen = In.Path;
		}
		UE_CLOG(LogDetours != 0 && Chosen != Found, LogHOLCommands, Display, TEXT("Detour: through the way between two buildings, %.0f cm instead of %.0f cm, from (%.0f, %.0f) to (%.0f, %.0f)"),
			Best, GetPathLength(Found->GetPathPoints()), Start.X, Start.Y, Target.X, Target.Y);
		return Chosen;
	}

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
		// A passage with no room for two abreast (D324: the way between two buildings): two portals in a row narrower than a
		// body. A lone narrow portal is a corner of the path (its fan), where the group keeps its places.
		TStaticArray<bool, FMassNavMeshShortPathFragment::MaxPoints> Narrow;
		for (uint8 Index = 0; Index < ShortPath.NumPoints; ++Index)
		{
			Narrow[Index] = FVector::Dist2D(ShortPath.Points[Index].Left, ShortPath.Points[Index].Right) < NarrowPortalWidth;
		}
		for (uint8 Index = 0; Index < ShortPath.NumPoints; ++Index)
		{
			const bool bInPassage = Narrow[Index] && ((Index > 0 && Narrow[Index - 1]) || (Index + 1 < ShortPath.NumPoints && Narrow[Index + 1]));
			const FMassNavMeshPathPoint& Point = ShortPath.Points[Index];
			const FVector Before = ShortPath.Points[FMath::Max(Index - 1, 0)].Position;
			const FVector After = ShortPath.Points[FMath::Min<int32>(Index + 1, ShortPath.NumPoints - 1)].Position;
			const FVector Along = FVector(After - Before).GetSafeNormal2D();
			const FVector Right(-Along.Y, Along.X, 0.);
			const double ToLeft = FVector::DotProduct(Point.Left - Point.Position, Right);
			const double ToRight = FVector::DotProduct(Point.Right - Point.Position, Right);
			// In a passage every unit walks the path itself, one after another; side by side, the pair was pushed apart into the
			// walls and put back at every step.
			const double Offset = bInPassage ? 0. : FMath::Clamp<double>(Lateral, FMath::Min(ToLeft, ToRight), FMath::Max(ToLeft, ToRight));
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
		MoveTarget.DesiredSpeed.Set(GetOrderWalkSpeed(Constants, Order));
		MoveTarget.CreateNewAction(EMassMovementAction::Move, World);

		Order.Phase = EHOLMovePhase::FinalLeg;
		Order.LastSlotDistance = MoveTarget.DistanceToGoal;
		Order.StuckSteps = 0;
	}

	/**
	 * A unit of a block march (J7a.4) steered straight at its slot (Order.Slot: its form-up place, or its slot in the
	 * marching block) at Speed, without slowing down there (UHOLFullSpeedProcessor lands it on the slot). DistanceToGoal:
	 * what is left of the block's path, so Mass keeps its avoidance at full strength.
	 */
	void SteerInBlock(const UWorld& World, const FTransformFragment& Transform, const FHOLMoveOrderFragment& Order, FMassMoveTargetFragment& MoveTarget,
		const float Speed, const float DistanceToGoal, const FVector2D& Direction)
	{
		const FVector Location = Transform.GetTransform().GetLocation();
		const FVector ToSlot(Order.Slot.X - Location.X, Order.Slot.Y - Location.Y, 0.);
		MoveTarget.Center = Order.Slot;
		MoveTarget.Forward = ToSlot.SizeSquared() > 100. ? ToSlot.GetSafeNormal() : FVector(Direction.X, Direction.Y, 0.);
		MoveTarget.DistanceToGoal = FMath::Max(DistanceToGoal, static_cast<float>(ToSlot.Size()));
		MoveTarget.EntityDistanceToGoal = FMassMoveTargetFragment::UnsetDistance;
		MoveTarget.SlackRadius = 0.f;
		MoveTarget.IntentAtGoal = EMassMovementAction::Move;
		MoveTarget.DesiredSpeed.Set(Speed);
		if (MoveTarget.GetCurrentAction() != EMassMovementAction::Move)
		{
			MoveTarget.CreateNewAction(EMassMovementAction::Move, World);
		}
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

namespace UE::HeroesOfLiberty::Orders
{
	/** Row priority of a unit's kind in a group's formation (J7-Q3): HOLFormation::GetRowPriority. */
	int32 GetFormationPriority(const FHOLUnitTypeConstants& Constants)
	{
		return HOLFormation::GetRowPriority(Constants.TypeName, Constants.CarryCapacity);
	}

	/**
	 * A formation place brought where a unit can stand (J7a.4): the navmesh's point under it if the navmesh has it there
	 * (not in an obstacle) and reaches it straight from the target, as the slots of D287; otherwise the first such point
	 * on the way back to the target (3/4, 1/2, 1/4 of the way), else the target itself. Units sent to one point then
	 * share it and stand side by side (D292).
	 */
	FVector ProjectFormationPlace(const FVector& Target, const FVector2D& Place, const double Spacing, const UNavigationSystemV1* NavSystem,
		const ANavigationData* NavData)
	{
		if (NavSystem == nullptr || NavData == nullptr)
		{
			return FVector(Place.X, Place.Y, Target.Z);
		}
		const FVector Extent(Spacing * 0.5, Spacing * 0.5, 500.);
		static constexpr double Fractions[] = { 1., 0.75, 0.5, 0.25 };
		for (const double Fraction : Fractions)
		{
			const FVector2D Point = FMath::Lerp(FVector2D(Target), Place, Fraction);
			const FVector Wanted(Point.X, Point.Y, Target.Z);
			FNavLocation OnNav;
			FVector Hit;
			if (NavSystem->ProjectPointToNavigation(Wanted, OnNav, Extent, NavData)
				&& FVector::Dist2D(OnNav.Location, Wanted) <= Spacing * 0.4
				&& !NavData->Raycast(Target, OnNav.Location, Hit, NavData->GetDefaultQueryFilter()))
			{
				return OnNav.Location;
			}
		}
		return Target;
	}
}

int64 UHOLUnitOrderProcessor::TotalPathsServed = 0;
int64 UHOLUnitOrderProcessor::TotalPathsReused = 0;
int64 UHOLUnitOrderProcessor::TotalBuildingRepaths = 0;
int64 UHOLUnitOrderProcessor::TotalPlaceReturns = 0;
UHOLUnitOrderProcessor::FBlockReport UHOLUnitOrderProcessor::LastBlockReport;
uint32 UHOLUnitOrderProcessor::LastBlockSerial = 0;
int64 UHOLUnitOrderProcessor::TotalStuckRepaths = 0;
int64 UHOLUnitOrderProcessor::TotalFootprintRepaths = 0;
int64 UHOLUnitOrderProcessor::TotalFootprintDroppedPaths = 0;
int64 UHOLUnitOrderProcessor::TotalStaleCombatWalks = 0;

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
	LastWorld = World;
	UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance() : nullptr;
	const double StartSeconds = FPlatformTime::Seconds();
	++StepCount;
	if (UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr)
	{
		ForgetChangedPaths(*Commands);
		UpdateNewFootprints(*Commands, NavSystem, NavData);
	}
	ApplyQueuedOrders(EntityManager, NavSystem, NavData);
	const double OrdersSeconds = FPlatformTime::Seconds();
	ServePathRequests(EntityManager, NavSystem, NavData);
	const double PathsSeconds = FPlatformTime::Seconds();
	AdvanceUnits(EntityManager, Context, NavData);
	AdvanceBlocks(EntityManager, Context.GetDeltaTimeSeconds(), NavData);
	WatchArrivals(EntityManager);
	StartQueuedMoves(EntityManager);
	if (World)
	{
		PublishChains(*World, Context.GetDeltaTimeSeconds());
	}
	if (!ReadyFootprints.IsEmpty())
	{
		RequestCutPaths(EntityManager, NavSystem, NavData);
		// One line per footprint, for the scripted checks (J6b.2).
		for (const FNewFootprint& Ready : ReadyFootprints)
		{
			TotalFootprintRepaths += Ready.Walkers;
			UE_LOG(LogHOLSimulation, Display, TEXT("Buildings (D310): footprint %s at (%.0f, %.0f): %d walkers given a new path, %d reused gather paths dropped%s (%s)"),
				*Ready.Building.TypeName.ToString(), Ready.Building.Centre.X, Ready.Building.Centre.Y, Ready.Walkers, Ready.DroppedPaths,
				Ready.MovedSlots > 0 ? *FString::Printf(TEXT(", %d destinations moved out of it"), Ready.MovedSlots) : TEXT(""),
				Ready.bRebuilt ? *FString::Printf(TEXT("tiles rebuilt after %u steps"), StepCount - Ready.SinceStep)
					: *FString::Printf(TEXT("tiles not seen rebuilt after %u steps"), StepCount - Ready.SinceStep));
		}
		ReadyFootprints.Reset();
	}
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

void UHOLUnitOrderProcessor::UpdateNewFootprints(UHOLCommandSubsystem& Commands, const UNavigationSystemV1* NavSystem, const ANavigationData* NavData)
{
	using namespace UE::HeroesOfLiberty::Orders;
	ReadyFootprints.Reset();
	TArray<FHOLBlockedFootprint> Blocked;
	Commands.ConsumeBlockedFootprints(Blocked);
	// The margin of a kept path: the navmesh's agent (60 cm, the walker of a task) and the clearance of D310.
	const double KeptMargin = UHOLBuildingSettings::GetBodyClearance() + (NavData ? NavData->GetConfig().AgentRadius : 0.f);
	TArray<FVector2D, TInlineAllocator<16>> Points;
	for (const FHOLBlockedFootprint& Building : Blocked)
	{
		// As any change of the navmesh: no path found across it in the next seconds is kept (D305).
		const FBox2D Area(FVector2D(Building.Area.Min), FVector2D(Building.Area.Max));
		RecentNavChanges.Add({ Area, StepCount + NavChangeSettleSteps });
		if (CutPaths == 0)
		{
			// As before J6b.2: the kept paths whose bounds meet it forgotten, nothing else.
			for (auto It = ReusedPaths.CreateIterator(); It; ++It)
			{
				if (Area.Intersect(It.Value().Bounds))
				{
					It.RemoveCurrent();
				}
			}
			continue;
		}
		FNewFootprint& New = NewFootprints.AddDefaulted_GetRef();
		New.Building = Building;
		New.Bounds = HOLConstruction::GetFootprintBounds(Building.Centre, Building.Size, Building.Yaw);
		New.SinceStep = StepCount;
		// Probed as a placement is (D311), in the footprint's own frame, turned with it.
		HOLConstruction::GetNavProbes(HOLConstruction::MakeFootprint(FVector2D::ZeroVector, Building.Size), UHOLBuildingSettings::GetTileSize(), FootprintProbeInset,
			New.Probes);
		for (FVector2D& Probe : New.Probes)
		{
			Probe = Building.Centre + Probe.GetRotated(Building.Yaw);
		}
		// The kept paths across it, at once (D305): its tiles change, a walk taking one now would cross it.
		const FBox2D Reach = New.Bounds.ExpandBy(KeptMargin);
		for (auto It = ReusedPaths.CreateIterator(); It; ++It)
		{
			const FReusedPath& Kept = It.Value();
			if (!Reach.Intersect(Kept.Bounds))
			{
				continue;
			}
			Points.Reset();
			if (Kept.Path.IsValid())
			{
				for (const FNavPathPoint& Point : Kept.Path->GetPathPoints())
				{
					Points.Add(FVector2D(Point.Location));
				}
			}
			else
			{
				Points.Add(Kept.From);
				Points.Add(Kept.To);
			}
			if (HOLConstruction::DoesWalkCross(New.Bounds, KeptMargin, Points))
			{
				It.RemoveCurrent();
				++New.DroppedPaths;
			}
		}
		TotalFootprintDroppedPaths += New.DroppedPaths;
	}
	if (NewFootprints.IsEmpty())
	{
		return;
	}
	// Its tiles rebuilt: the navmesh's generator has none left to rebuild there, and every probe is off the navmesh (the
	// change reached the generator and its tiles were rebuilt, asynchronously).
	const ARecastNavMesh* Recast = Cast<const ARecastNavMesh>(NavData);
	const FRecastNavMeshGenerator* Generator = Recast ? static_cast<const FRecastNavMeshGenerator*>(Recast->GetGenerator()) : nullptr;
	const FVector Extent(FootprintProbeExtent, FootprintProbeExtent, 500.);
	for (int32 Index = 0; Index < NewFootprints.Num();)
	{
		FNewFootprint& New = NewFootprints[Index];
		bool bRebuilt = NavSystem && NavData && !(Generator && Generator->HasDirtyTiles(New.Building.Area));
		for (int32 Probe = 0; bRebuilt && Probe < New.Probes.Num(); ++Probe)
		{
			FNavLocation OnNav;
			bRebuilt = !NavSystem->ProjectPointToNavigation(FVector(New.Probes[Probe], New.Building.Area.Min.Z), OnNav, Extent, NavData);
		}
		if (bRebuilt || StepCount - New.SinceStep >= FootprintRebuildSteps)
		{
			New.bRebuilt = bRebuilt;
			ReadyFootprints.Add(MoveTemp(New));
			NewFootprints.RemoveAt(Index);
		}
		else
		{
			++Index;
		}
	}
}

bool UHOLUnitOrderProcessor::IsWayCut(const FHOLMoveOrderFragment& Order, const FVector& Location, const FMassNavMeshCachedPathFragment& CachedPath, const float Radius,
	const FMassEntityHandle Entity)
{
	const FVector2D Here(Location);
	// Its way: from where it is, along what is left of its corridor from the part nearest it (at its place across it), then
	// to its slot; on the final leg, straight to its slot.
	CutWay.Reset();
	CutWay.Add(Here);
	double Across = 0.;
	if (Order.Phase == EHOLMovePhase::Path && CachedPath.Corridor.IsValid() && CachedPath.Corridor->Portals.Num() >= 2)
	{
		const TArray<FNavCorridorPortal>& Portals = CachedPath.Corridor->Portals;
		int32 Next = 1;
		double Nearest = TNumericLimits<double>::Max();
		for (int32 Index = 0; Index + 1 < Portals.Num(); ++Index)
		{
			const double DistanceSquared = FVector2D::DistSquared(Here, FMath::ClosestPointOnSegment2D(Here, FVector2D(Portals[Index].Location), FVector2D(Portals[Index + 1].Location)));
			if (DistanceSquared < Nearest)
			{
				Nearest = DistanceSquared;
				Next = Index + 1;
			}
		}
		for (int32 Index = Next; Index < Portals.Num(); ++Index)
		{
			CutWay.Add(FVector2D(Portals[Index].Location));
		}
		Across = FMath::Abs(Order.Lateral);
	}
	CutWay.Add(FVector2D(Order.Slot));
	const double Clearance = UHOLBuildingSettings::GetBodyClearance();
	for (int32 Index = 0; Index < ReadyFootprints.Num(); ++Index)
	{
		FNewFootprint& Ready = ReadyFootprints[Index];
		// Standing in it (its centre inside): the footprints' guard puts it out by the nearest side (D310), not this check;
		// only the rest of its way counts, from its next point on.
		const bool bInside = HOLConstruction::GetDistance(Ready.Bounds, Here) <= 0.;
		const TConstArrayView<FVector2D> Way = bInside ? TConstArrayView<FVector2D>(CutWay).RightChop(1) : TConstArrayView<FVector2D>(CutWay);
		if (HOLConstruction::DoesWalkCross(Ready.Bounds, Clearance + Radius + Across, Way))
		{
			++Ready.Walkers;
			CutWalkers.Add({ Entity, Order.OrderSerial, Location, Order.Slot, Index, Radius });
			return true;
		}
	}
	return false;
}

void UHOLUnitOrderProcessor::RequestCutPaths(FMassEntityManager& EntityManager, const UNavigationSystemV1* NavSystem, const ANavigationData* NavData)
{
	using namespace UE::HeroesOfLiberty::Orders;
	if (CutWalkers.IsEmpty())
	{
		return;
	}
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const double Clearance = UHOLBuildingSettings::GetBodyClearance();
	// A destination in the footprint (a slot of an order given before, a spot by a source or a deposit now under it): moved
	// out along the ray from its middle, a little beyond the navmesh's edge (the slots of a group keep their spread around
	// it), onto the navmesh; failing that, its nearest point. The task of a unit sent there finds another place on arrival
	// (gathering, D309; building).
	for (FCutWalker& Walker : CutWalkers)
	{
		FNewFootprint& Ready = ReadyFootprints[Walker.Footprint];
		if (!NavSystem || !NavData || HOLConstruction::GetDistance(Ready.Bounds, FVector2D(Walker.Slot)) >= Clearance + Walker.Radius)
		{
			continue;
		}
		const FVector2D Out = HOLConstruction::PushOutOfFootprint(Ready.Bounds, Clearance + Walker.Radius + MovedSlotGap, FVector2D(Walker.Slot));
		FNavLocation OnNav;
		if (NavSystem->ProjectPointToNavigation(FVector(Out, Walker.Slot.Z), OnNav, FVector(MovedSlotGap * 2., MovedSlotGap * 2., 500.), NavData)
			|| NavSystem->ProjectPointToNavigation(Walker.Slot, OnNav, FVector(Settings->OrderTargetProjectionExtent), NavData))
		{
			Walker.Slot = OnNav.Location;
			EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Walker.Entity).Slot = OnNav.Location;
			++Ready.MovedSlots;
		}
	}
	// One request per cluster of each order (D286): a group cut by the footprint goes round it together, each one to its
	// own slot after the path.
	CutWalkers.StableSort([](const FCutWalker& A, const FCutWalker& B) { return A.OrderSerial < B.OrderSerial; });
	TArray<FVector2D> Positions;
	TArray<int32> ClusterOfUnit;
	for (int32 First = 0; First < CutWalkers.Num();)
	{
		int32 End = First + 1;
		while (End < CutWalkers.Num() && CutWalkers[End].OrderSerial == CutWalkers[First].OrderSerial)
		{
			++End;
		}
		Positions.Reset();
		for (int32 Index = First; Index < End; ++Index)
		{
			Positions.Add(FVector2D(CutWalkers[Index].Location));
		}
		const int32 NumClusters = HOLGroupMove::MakeClusters(Positions, HOLGroupMove::ClusterLinkDistance, ClusterOfUnit);
		for (int32 Cluster = 0; Cluster < NumClusters; ++Cluster)
		{
			FPathRequest& Request = PendingPaths.AddDefaulted_GetRef();
			Request.OrderSerial = CutWalkers[First].OrderSerial;
			Request.bForBuilding = true;
			FVector Start = FVector::ZeroVector;
			FVector SlotsCentre = FVector::ZeroVector;
			for (int32 Index = First; Index < End; ++Index)
			{
				if (ClusterOfUnit[Index - First] == Cluster)
				{
					Request.Entities.Add(CutWalkers[Index].Entity);
					Start += CutWalkers[Index].Location;
					SlotsCentre += CutWalkers[Index].Slot;
				}
			}
			Request.Start = Start / Request.Entities.Num();
			SlotsCentre /= Request.Entities.Num();
			// The path to the slot nearest the middle of theirs: a point on the navmesh, as an order's target.
			double Nearest = TNumericLimits<double>::Max();
			for (int32 Index = First; Index < End; ++Index)
			{
				const double Distance = FVector::DistSquared2D(CutWalkers[Index].Slot, SlotsCentre);
				if (ClusterOfUnit[Index - First] == Cluster && Distance < Nearest)
				{
					Nearest = Distance;
					Request.Target = CutWalkers[Index].Slot;
				}
			}
			// Each one's place across the new corridor, to the right of the new march, as for an order (its old place was
			// across a way that no longer goes there).
			const FVector2D Forward = (FVector2D(Request.Target) - FVector2D(Request.Start)).GetSafeNormal();
			const FVector2D Right(-Forward.Y, Forward.X);
			for (int32 Index = First; Index < End; ++Index)
			{
				if (ClusterOfUnit[Index - First] == Cluster)
				{
					const FCutWalker& Walker = CutWalkers[Index];
					const double Lateral = Forward.IsNearlyZero() ? 0. : FVector2D::DotProduct(FVector2D(Walker.Location) - FVector2D(Request.Start), Right);
					const double MaxLateral = FMath::Max(0., Settings->PathCorridorWidth * 0.5 - Walker.Radius);
					EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Walker.Entity).Lateral = static_cast<float>(FMath::Clamp(Lateral, -MaxLateral, MaxLateral));
				}
			}
		}
		First = End;
	}
	CutWalkers.Reset();
}

FNavPathSharedPtr UHOLUnitOrderProcessor::FindPathThroughWays(UWorld& World, UNavigationSystemV1& NavSystem, const ANavigationData& NavData, const FVector& Start,
	const FVector& Target, const FNavPathSharedPtr& Found)
{
	return Found.IsValid() && !Found->IsPartial() && Found->GetPathPoints().Num() > 2
		? UE::HeroesOfLiberty::Orders::FindPathThroughWaysImpl(World, NavSystem, NavData, Start, Target, Found) : Found;
}

bool UHOLUnitOrderProcessor::DoesCrossBuilding(const UWorld& World, const FVector& Start, const FVector& Target, const ANavigationData* NavData,
	const double ExtraMargin)
{
	// Rare (a partial path out of a pocket of the navmesh, a group's straight path): the buildings read from the world.
	const double Margin = UHOLBuildingSettings::GetBodyClearance() + (NavData ? NavData->GetConfig().AgentRadius : 0.f) + ExtraMargin;
	const FVector2D Way[] = { FVector2D(Start), FVector2D(Target) };
	for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
	{
		if (It->BlocksNavigation() && HOLConstruction::DoesWalkCross(It->GetBodyBounds(), Margin, Way))
		{
			return true;
		}
	}
	return false;
}

bool UHOLUnitOrderProcessor::IsClaimLive(const FMassEntityManager& EntityManager, const FMassEntityHandle Entity, const FIntPoint& Cell)
{
	const FHOLMoveOrderFragment* Order = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
	return Order && Order->Phase != EHOLMovePhase::None && GetClaimCell(FVector2D(Order->Slot)) == Cell;
}

void UHOLUnitOrderProcessor::ApplyQueuedOrders(FMassEntityManager& EntityManager, UNavigationSystemV1* NavSystem, const ANavigationData* NavData)
{
	using namespace UE::HeroesOfLiberty::Orders;
	UWorld* World = EntityManager.GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (Units == nullptr || Commands == nullptr)
	{
		return;
	}

	// Shift's moves started at the last step first (J7a.4): this step's new orders come after them, and win.
	TArray<FHOLMoveOrder> Orders;
	Commands->ConsumeMoveOrders(Orders);
	const int32 NumStartedQueued = StartedQueuedMoves.Num();
	if (NumStartedQueued > 0)
	{
		Orders.Insert(MoveTemp(StartedQueuedMoves), 0);
		StartedQueuedMoves.Reset();
	}

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
	TArray<int32> KindOfUnit;
	TArray<int32> ClusterOfUnit;
	TArray<FVector2D> Occupied;
	TArray<FMassNavigationObstacleItem> Nearby;
	const UMassNavigationSubsystem* Navigation = World->GetSubsystem<UMassNavigationSubsystem>();
	const FNavigationObstacleHashGrid2D* ObstacleGrid = Navigation ? &Navigation->GetObstacleGrid() : nullptr;
	TArray<uint32> EntityIds;
	for (int32 OrderIndex = 0; OrderIndex < Orders.Num(); ++OrderIndex)
	{
		const FHOLMoveOrder& Order = Orders[OrderIndex];
		// A unit can be ordered before it is born or after it is gone: it is left out (M4 will keep the orders a
		// player gave to a group that is still being created).
		Entities.Reset();
		EntityIds.Reset();
		Positions.Reset();
		float Radius = 0.f;
		int32 NumRefused = 0;
		// An attack (J7b.3, D288): the target must be a unit alive, an enemy of the player who gave it (a client could send
		// any identifier); a server's own order, an enemy of each unit.
		FMassEntityHandle AttackTarget;
		int32 TargetOwner = INDEX_NONE;
		if (Order.Kind == EHOLOrderKind::Attack)
		{
			AttackTarget = Units->FindEntity(Order.TargetUnitId);
			const FHOLUnitStateFragment* TargetState = EntityManager.IsEntityValid(AttackTarget) ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(AttackTarget) : nullptr;
			if (!TargetState || TargetState->CurrentHealth <= 0 || !EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(AttackTarget)
				|| (Order.PlayerIndex != INDEX_NONE && TargetState->OwnerPlayer == Order.PlayerIndex))
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Attack order of slot %d on #%u refused: not an enemy unit alive"), Order.PlayerIndex, Order.TargetUnitId);
				continue;
			}
			TargetOwner = TargetState->OwnerPlayer;
		}
		int32 NumStale = 0;
		for (const uint32 UnitId : Order.UnitIds)
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			// A unit Dying (J7b.4) obeys no order.
			if (!EntityManager.IsEntityValid(Entity) || Entities.Contains(Entity)
				|| EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).CurrentHealth <= 0)
			{
				continue;
			}
			// Only a player's own units obey it (D288): a client could send any identifier.
			if (Order.PlayerIndex != INDEX_NONE && EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).OwnerPlayer != Order.PlayerIndex)
			{
				++NumRefused;
				continue;
			}
			// A combat walk given before a newer player order (J7b.3): dropped, the newer order kept.
			if (Order.CombatStamp != 0)
			{
				const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
				if (!Combat || Combat->EngageSerial != Order.CombatStamp)
				{
					++NumStale;
					continue;
				}
			}
			if (TargetOwner != INDEX_NONE && EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).OwnerPlayer == TargetOwner)
			{
				++NumRefused;
				continue;
			}
			Entities.Add(Entity);
			EntityIds.Add(UnitId);
			Positions.Add(FVector2D(EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation()));
			Radius = FMath::Max(Radius, EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity).Radius);
		}
		if (NumRefused > 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Move order of slot %d: %d units ignored, not its own"), Order.PlayerIndex, NumRefused);
		}
		if (NumStale > 0)
		{
			++TotalStaleCombatWalks;
			UE_LOG(LogHOLSimulation, Verbose, TEXT("Combat walk dropped: a newer order was given"));
		}
		if (Entities.Num() == 0)
		{
			continue;
		}
		// Shift (J7a.4): while one of its units still walks a player's move (or waits for queued ones), the whole move
		// waits in their queues; its places are made when it starts. None walking: it goes at once.
		if (Order.bQueue && !Order.bFromTask)
		{
			bool bAnyWalking = false;
			for (int32 Unit = 0; Unit < Entities.Num() && !bAnyWalking; ++Unit)
			{
				const FHOLMoveOrderFragment& Move = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entities[Unit]);
				const FHOLUnitStateFragment& State = EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entities[Unit]);
				const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entities[Unit]);
				const FHOLBuildFragment* Build = EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(Entities[Unit]);
				const bool bOnTask = (Gather && Gather->IsGathering()) || (Build && Build->IsBuilding());
				const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entities[Unit]);
				bAnyWalking = MoveQueueOfUnit.Contains(EntityIds[Unit]) || (Combat && Combat->bOrderedTarget)
					|| (!bOnTask && State.Activity == EHOLUnitActivity::Moving && Move.Phase != EHOLMovePhase::None && !Move.PlaceHold.bReturning);
			}
			if (bAnyWalking)
			{
				// Chains (D328): a unit's first queued move keeps its current action as the head of its chain.
				for (int32 Unit = 0; Unit < Entities.Num(); ++Unit)
				{
					const uint32 UnitId = EntityIds[Unit];
					if (MoveQueueOfUnit.Contains(UnitId) || ChainHeadOfUnit.Contains(UnitId))
					{
						continue;
					}
					const FHOLMoveOrderFragment& Move = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entities[Unit]);
					const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entities[Unit]);
					const FHOLUnitStateFragment* TargetState = Combat && Combat->bOrderedTarget && EntityManager.IsEntityValid(Combat->Target)
						? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Combat->Target) : nullptr;
					const HOLOrderChain::FStep* OrderStep = Move.Phase != EHOLMovePhase::None && !Move.PlaceHold.bReturning ? OrderSteps.Find(Move.OrderSerial) : nullptr;
					if (TargetState)
					{
						ChainHeadOfUnit.Add(UnitId, {HOLOrderChain::FStep::MakeAttack(TargetState->UnitId), StepCount});
					}
					else if (OrderStep)
					{
						ChainHeadOfUnit.Add(UnitId, {*OrderStep, StepCount});
					}
				}
				const uint32 QueuedId = NextQueuedMoveId++;
				FQueuedMove& Queued = QueuedMoves.Add(QueuedId);
				Queued.Order = Order;
				Queued.Order.bQueue = false;
				Queued.Members = EntityIds;
				for (const uint32 UnitId : EntityIds)
				{
					MoveQueueOfUnit.FindOrAdd(UnitId).Add(QueuedId);
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Move order of slot %d queued (Shift) for %d units to (%.0f, %.0f)"), Order.PlayerIndex,
					EntityIds.Num(), Order.Target.X, Order.Target.Y);
				continue;
			}
		}
		// Any other order forgets the units' queued moves (J7a.4); a queued move that starts keeps those after it.
		if (OrderIndex >= NumStartedQueued && !Order.bFromTask && (!MoveQueueOfUnit.IsEmpty() || !ChainHeadOfUnit.IsEmpty()))
		{
			for (const uint32 UnitId : EntityIds)
			{
				ForgetMoveQueue(UnitId);
			}
		}
		// Chains (D328, the user's trial of J7b.5b): a player's order without Shift (or with Shift, nothing before it) has its
		// chain too, its one step, until it is over (StartQueuedMoves). Only a player's orders: the server's own (tasks,
		// rally points, stress tests) cost nothing here.
		if (Order.bChain && OrderIndex >= NumStartedQueued && !Order.bFromTask
			&& (Order.Kind == EHOLOrderKind::Move || Order.Kind == EHOLOrderKind::AttackMove || Order.Kind == EHOLOrderKind::Attack))
		{
			const HOLOrderChain::FStep Step = MakeChainStep(Order);
			for (const uint32 UnitId : EntityIds)
			{
				ChainHeadOfUnit.Add(UnitId, {Step, StepCount});
			}
		}
		// The combat orders without a walk of their own (J7b.3): the units stand where they are, their task and fight
		// forgotten; an attack's walk to its target is the combat's approach (UHOLCombatProcessor, the same step).
		if (Order.Kind == EHOLOrderKind::Attack || Order.Kind == EHOLOrderKind::Stop || Order.Kind == EHOLOrderKind::Hold)
		{
			ApplyCombatOrder(EntityManager, *World, Order, Entities, AttackTarget);
			continue;
		}
		const uint32 OrderSerial = NextOrderSerial++;
		// Chains (D328): the step of each player order, by its serial (one entry per order), the recent ones only.
		if (!Order.bFromTask)
		{
			OrderSteps.Add(OrderSerial, MakeChainStep(Order));
			if (OrderSteps.Num() > 256)
			{
				for (auto It = OrderSteps.CreateIterator(); It; ++It)
				{
					if (OrderSerial - It.Key() > 128)
					{
						It.RemoveCurrent();
					}
				}
			}
		}

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
		// A group's order on the ground: in rows across the march (J7a.4, J7-Q3), the target in the middle of the front
		// row, for every group, Peasants included (the tasks' walks are one unit each: their spots stay theirs, D309,
		// D312). One unit: the nearest free slot of D287.
		const bool bFormation = !bTaskSpot && !Order.bFromTask && Entities.Num() > 1;
		float FrontYaw = 0.f;
		// What the block march (J7a.4) needs of the formation: its facing, gaps, width and kinds.
		FVector2D Forward(1., 0.);
		double ColumnSpacing = 0.;
		double RowSpacing = 0.;
		int32 FrontWidth = 0;
		bool bPeasantsOnly = true;
		if (bFormation)
		{
			// A right-button drag (J7a.4, the user's 2nd trial): an arrow from the target, facing along it; else towards the target.
			const bool bFront = !Order.FrontLine.IsNearlyZero();
			Forward = FVector2D(Order.Target) - Centre;
			Forward = bFront ? HOLFormation::GetFrontFacing(Order.FrontLine)
				: Forward.IsNearlyZero() ? FVector2D(1., 0.) : Forward.GetSafeNormal();
			FrontYaw = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(Forward.Y, Forward.X)));
			KindOfUnit.Reset();
			for (const FMassEntityHandle Entity : Entities)
			{
				const int32 Priority = GetFormationPriority(EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity));
				KindOfUnit.Add(Priority);
				bPeasantsOnly &= Priority == HOLFormation::PeasantRowPriority;
			}
			ColumnSpacing = 2. * Radius * HOLFormation::ColumnSpacingInDiameters;
			RowSpacing = 2. * Radius * HOLFormation::RowSpacingInDiameters;
			// Ctrl + drag (the user's 1st trial): its width too; a plain drag keeps the default rows, turned to face across it.
			FrontWidth = bFront && Order.bFrontWidth ? HOLFormation::GetFrontWidth(Order.FrontLine.Size(), ColumnSpacing, Entities.Num()) : 0;
			HOLFormation::PlaceGroup(Positions, KindOfUnit, bPeasantsOnly, FVector2D(Order.Target), Forward, ColumnSpacing, RowSpacing,
				[&](const FVector2D& Place)
				{
					const FVector OnNav = ProjectFormationPlace(Order.Target, Place, ColumnSpacing, NavSystem, NavData);
					Slots.Add(OnNav);
					return FVector2D(OnNav);
				}, Slots2D, SlotOfUnit, FrontWidth);
		}
		else
		{
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
		}

		// A group in formation marches at its slowest unit's speed (J7a.4, J7-Q3).
		float Slowest = 0.f;
		if (bFormation)
		{
			Slowest = TNumericLimits<float>::Max();
			for (const FMassEntityHandle Entity : Entities)
			{
				Slowest = FMath::Min(Slowest, EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity).Speed);
			}
		}

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
			const FVector2D ClusterForward = (FVector2D(Request.Target) - FVector2D(Request.Start)).GetSafeNormal();
			const FVector2D Right(-ClusterForward.Y, ClusterForward.X);
			const double Lateral = ClusterForward.IsNearlyZero() ? 0. : FVector2D::DotProduct(Positions[Unit] - FVector2D(Request.Start), Right);

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
			// And its fight (J7b.2): a player's order, a queued one when it starts included, prevails over the melee; the
			// combat processor, after this one in the step, sees it walking an order and takes on nobody until it is done.
			FHOLCombatFragment* Combat = Order.bFromTask ? nullptr : EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity);
			MoveOrder.Slot = SlotOfUnit[Unit] != INDEX_NONE ? Slots[SlotOfUnit[Unit]] : Order.Target;
			if (Combat)
			{
				// Any move lets a hold go (J7b.3); an attack-move's units remember their slot, walked on to after a fight.
				Combat->Disengage();
				Combat->Mode = Order.Kind == EHOLOrderKind::AttackMove ? EHOLCombatMode::AttackMove : EHOLCombatMode::Normal;
				Combat->AttackMoveGoal = MoveOrder.Slot;
			}
			MoveOrder.Lateral = static_cast<float>(FMath::Clamp(Lateral, -(double)CorridorHalfWidth + Radius, (double)CorridorHalfWidth - Radius));
			MoveOrder.Phase = EHOLMovePhase::WaitingPath;
			MoveOrder.bShortOfSlot = false;
			// A new order lets its formation place go; a group's formation move gives it a new one (J7a.4).
			MoveOrder.bFormation = bFormation;
			MoveOrder.MarchSpeed = Slowest;
			MoveOrder.PlaceHold.Clear();
			MoveOrder.FrontYaw = FrontYaw;
			MoveOrder.bFacesFront = false;
			MoveOrder.bReforming = false;
			SlotClaims.FindOrAdd(GetClaimCell(FVector2D(MoveOrder.Slot))).AddUnique(Entity);
			// Moving from now on: the unit keeps its current walk until its path comes (S2 orders only idle units).
			EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).Activity = EHOLUnitActivity::Moving;
		}

		// A group's move in formation far enough: a block march (J7a.4, the user's 1st trial, as in AoE4) instead of the
		// clusters' paths. Peasants alone walk straight to their places (no form-up: workers, not a troop).
		if (bFormation && BlockMarch != 0 && !bPeasantsOnly)
		{
			StartBlockMarch(EntityManager, *World, NavSystem, NavData, OrderSerial, Entities, Positions, KindOfUnit, bPeasantsOnly, FVector2D(Order.Target),
				Forward, ColumnSpacing, RowSpacing, FrontWidth, Slots, Slowest, FirstRequest, NumClusters);
		}
	}
}

void UHOLUnitOrderProcessor::StartBlockMarch(FMassEntityManager& EntityManager, UWorld& World, UNavigationSystemV1* NavSystem, const ANavigationData* NavData,
	const uint32 OrderSerial, TConstArrayView<FMassEntityHandle> Entities, TConstArrayView<FVector2D> Positions, TConstArrayView<int32> KindOfUnit,
	const bool bPeasantsOnly, const FVector2D& Target, const FVector2D& Forward, const double ColumnSpacing, const double RowSpacing, const int32 FrontWidth,
	TConstArrayView<FVector> Places, const float Pace, const int32 FirstRequest, const int32 NumClusters)
{
	using namespace UE::HeroesOfLiberty::Orders;
	const int32 Num = Entities.Num();
	if (Num < 2 || Places.Num() == 0)
	{
		return;
	}
	// Where the block ends: the middle of its arrival places; how big it is: its larger side there, plus a row.
	FVector EndCentre = FVector::ZeroVector;
	for (const FVector& Place : Places)
	{
		EndCentre += Place;
	}
	EndCentre /= Places.Num();
	FVector2D Min(TNumericLimits<double>::Max()), Max(-TNumericLimits<double>::Max());
	for (const FVector& Place : Places)
	{
		const FVector2D Local = HOLFormation::WorldToBlockOffset(FVector2D(Place) - FVector2D(EndCentre), Forward);
		Min = FVector2D::Min(Min, Local);
		Max = FVector2D::Max(Max, Local);
	}
	double Height = 0.;
	FVector2D Centre = FVector2D::ZeroVector;
	for (int32 Unit = 0; Unit < Num; ++Unit)
	{
		Height += EntityManager.GetFragmentDataChecked<FTransformFragment>(Entities[Unit]).GetTransform().GetLocation().Z;
		Centre += Positions[Unit];
	}
	Height /= Num;
	Centre /= Num;
	const double Size = FMath::Max(Max.X - Min.X, Max.Y - Min.Y) + RowSpacing;
	// A short move (within the block's size): straight to the places, no form-up.
	if (FVector2D::Distance(Centre, FVector2D(EndCentre)) <= Size * HOLFormation::BlockMarchMinDistanceInDepths)
	{
		return;
	}

	// The group's one path, from its centre (on the navmesh) to the middle of its arrival places; none: straight.
	FBlockMarch Block;
	FVector Start(Centre.X, Centre.Y, Height);
	if (NavSystem && NavData && DirectPaths == 0)
	{
		FNavLocation StartOnNav;
		if (NavSystem->ProjectPointToNavigation(Start, StartOnNav, FVector(500.f, 500.f, 500.f), NavData))
		{
			Start = StartOnNav.Location;
			const FPathFindingQuery Query(&World, *NavData, Start, EndCentre);
			const FPathFindingResult Result = NavSystem->FindPathSync(Query);
			if (Result.IsSuccessful() && Result.Path.IsValid())
			{
				Result.Path->RemoveOverlappingPoints(FNavCorridor::OverlappingPointTolerance);
				for (const FNavPathPoint& Point : Result.Path->GetPathPoints())
				{
					Block.Path.Add(FVector2D(Point.Location));
				}
			}
		}
		++NumPathsServed;
		++TotalPathsServed;
	}
	if (Block.Path.Num() < 2)
	{
		Block.Path = { FVector2D(Start), FVector2D(EndCentre) };
	}
	Block.Length = HOLFormation::GetLineLength(Block.Path);
	if (Block.Length <= Size * HOLFormation::BlockMarchMinDistanceInDepths)
	{
		return;
	}
	const FVector2D StartForward = HOLFormation::GetDirectionOnLine(Block.Path, 0., HOLFormation::BlockTurnWindow, Forward);

	// Form-up: the same rows facing the path's start, their middle on the path's first point; each unit's offset in the block.
	TArray<FVector2D> UpPlaces;
	TArray<int32> UpOfUnit;
	HOLFormation::PlaceGroup(Positions, KindOfUnit, bPeasantsOnly, FVector2D::ZeroVector, StartForward, ColumnSpacing, RowSpacing,
		[](const FVector2D& Place) { return Place; }, UpPlaces, UpOfUnit, FrontWidth);
	FVector2D Middle = FVector2D::ZeroVector;
	for (int32 Unit = 0; Unit < Num; ++Unit)
	{
		Middle += UpOfUnit[Unit] != INDEX_NONE ? UpPlaces[UpOfUnit[Unit]] : FVector2D::ZeroVector;
	}
	Middle /= Num;
	TArray<FVector2D> RawUp;
	TArray<FVector> UpSlots;
	HOLFormation::PlaceGroup(Positions, KindOfUnit, bPeasantsOnly, Block.Path[0] - Middle, StartForward, ColumnSpacing, RowSpacing,
		[&](const FVector2D& Place)
		{
			RawUp.Add(Place);
			const FVector OnNav = ProjectFormationPlace(Start, Place, ColumnSpacing, NavSystem, NavData);
			UpSlots.Add(OnNav);
			return FVector2D(OnNav);
		}, UpPlaces, UpOfUnit, FrontWidth);
	// Each unit's offset in the block; its arrival place the one of its slot (the same rows, same row and column: the
	// form-up's place index is the arrival's, the user's 2nd trial), so the block turned at the end stands on them.
	for (int32 Unit = 0; Unit < Num; ++Unit)
	{
		const FVector2D Offset = UpOfUnit[Unit] != INDEX_NONE ? HOLFormation::WorldToBlockOffset(RawUp[UpOfUnit[Unit]] - Block.Path[0], StartForward) : FVector2D::ZeroVector;
		Block.Offsets.Add(Offset);
	}
	Block.FinalForward = Forward.GetSafeNormal();
	Block.BlendDistance = HOLFormation::GetBlockBlendDistance(Size, Block.Length);
	// Already in rows, or close together (the user's 4th trial): no stop, the block starts at once and its units take their
	// slots walking (FormUp phase while they join, through each other).
	double FarthestUp = 0.;
	for (int32 Unit = 0; Unit < Num; ++Unit)
	{
		const FVector2D Up = UpOfUnit[Unit] != INDEX_NONE ? FVector2D(UpSlots[UpOfUnit[Unit]]) : Block.Path[0];
		FarthestUp = FMath::Max(FarthestUp, FVector2D::Distance(Positions[Unit], Up));
	}
	if (FarthestUp <= HOLFormation::BlockJoinDistance)
	{
		Block.bMarching = true;
		Block.bJoining = true;
		Block.Report.FormUpSteps = 0;
	}

	// The clusters' paths are not needed: the block walks its own.
	PendingPaths.RemoveAt(FirstRequest, NumClusters, EAllowShrinking::No);
	for (int32 Unit = 0; Unit < Num; ++Unit)
	{
		const FMassEntityHandle Entity = Entities[Unit];
		FHOLMoveOrderFragment& MoveOrder = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity);
		Block.Entities.Add(Entity);
		Block.Kinds.Add(KindOfUnit.IsValidIndex(Unit) ? KindOfUnit[Unit] : 0);
		Block.Places.Add(UpOfUnit[Unit] != INDEX_NONE && Places.IsValidIndex(UpOfUnit[Unit]) ? Places[UpOfUnit[Unit]] : MoveOrder.Slot);
		Block.HeldSteps.Add(0);
		MoveOrder.Slot = UpOfUnit[Unit] != INDEX_NONE ? UpSlots[UpOfUnit[Unit]] : FVector(Block.Path[0].X, Block.Path[0].Y, Start.Z);
		MoveOrder.Phase = EHOLMovePhase::FormUp;
		FMassNavMeshShortPathFragment& ShortPath = EntityManager.GetFragmentDataChecked<FMassNavMeshShortPathFragment>(Entity);
		FMassNavMeshCachedPathFragment& CachedPath = EntityManager.GetFragmentDataChecked<FMassNavMeshCachedPathFragment>(Entity);
		ShortPath.Reset();
		CachedPath.NavPath.Reset();
		CachedPath.Corridor.Reset();
		CachedPath.PathSource = EMassNavigationPathSource::Unset;
		// To its form-up place at its own speed: the stop is short.
		SteerInBlock(World, EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity), MoveOrder,
			EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(Entity),
			GetWalkSpeed(EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity)), static_cast<float>(Block.Length), StartForward);
	}
	Block.Pace = Pace;
	Block.Report.Units = Num;
	UE_LOG(LogHOLSimulation, Display, TEXT("Block march: %s (farthest %.0f cm from its form-up place)"), Block.bJoining ? TEXT("forming up on the move") : TEXT("gathering first"), FarthestUp);
	Block.Report.Pace = Pace;
	LastBlockReport = Block.Report;
	LastBlockSerial = OrderSerial;
	UE_LOG(LogHOLSimulation, Display, TEXT("Block march: %d units, path of %d points, %.0f cm, pace %.0f cm/s"), Num, Block.Path.Num(), Block.Length, Pace);
	Blocks.Add(OrderSerial, MoveTemp(Block));
}

void UHOLUnitOrderProcessor::AdvanceBlocks(FMassEntityManager& EntityManager, const float DeltaTime, const ANavigationData* NavData)
{
	using namespace UE::HeroesOfLiberty::Orders;
	UWorld* World = EntityManager.GetWorld();
	if (World == nullptr || Blocks.IsEmpty())
	{
		return;
	}
	const float Scale = FMath::Clamp(SpeedScale, MinSpeedScale, MaxSpeedScale);
	const auto Publish = [](const uint32 Serial, const FBlockReport& Report)
	{
		if (Serial == LastBlockSerial)
		{
			LastBlockReport = Report;
		}
	};
	for (auto It = Blocks.CreateIterator(); It; ++It)
	{
		const uint32 Serial = It.Key();
		FBlockMarch& Block = It.Value();
		// Its units still in it (a new order, gone: out).
		for (int32 Unit = Block.Entities.Num() - 1; Unit >= 0; --Unit)
		{
			const FMassEntityHandle Entity = Block.Entities[Unit];
			const FHOLMoveOrderFragment* Order = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
			if (!Order || Order->OrderSerial != Serial || (Order->Phase != EHOLMovePhase::FormUp && Order->Phase != EHOLMovePhase::Block))
			{
				Block.Entities.RemoveAt(Unit);
				Block.Offsets.RemoveAt(Unit);
				Block.Places.RemoveAt(Unit);
				Block.Kinds.RemoveAt(Unit);
				Block.HeldSteps.RemoveAt(Unit);
			}
		}
		if (Block.Entities.IsEmpty())
		{
			Publish(Serial, Block.Report);
			It.RemoveCurrent();
			continue;
		}
		++Block.Steps;
		const FVector2D Fallback = HOLFormation::GetDirectionOnLine(Block.Path, 0., HOLFormation::BlockTurnWindow, FVector2D(1., 0.));
		const double Pace = Block.Pace * Scale;

		// Form-up: everyone on its place, or long enough.
		if (!Block.bMarching)
		{
			bool bAllThere = true;
			for (const FMassEntityHandle Entity : Block.Entities)
			{
				const FVector Location = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation();
				bAllThere &= FVector::Dist2D(Location, EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity).Slot) <= HOLFormation::BlockJoinDistance;
			}
			if (!bAllThere && Block.Steps < HOLFormation::FormUpMaxSteps)
			{
				continue;
			}
			// Close enough (the user's 4th trial): the rest is done on the move.
			Block.bMarching = true;
			Block.bJoining = true;
			Block.Report.FormUpSteps = Block.Steps;
			Block.Steps = 0;
		}

		// The march: the block's point moves on only while every unit keeps up with its slot.
		bool bHeld = false;
		float MaxGap = 0.f;
		for (int32 Unit = 0; Unit < Block.Entities.Num(); ++Unit)
		{
			const FMassEntityHandle Entity = Block.Entities[Unit];
			const FVector Location = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation();
			const float Gap = static_cast<float>(FVector::Dist2D(Location, EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity).Slot));
			MaxGap = FMath::Max(MaxGap, Gap);
			// Joining or re-forming: the block slows down instead of stopping; only a unit far behind holds it.
			const bool bLagging = Gap > (Block.bJoining || Block.bBlending ? HOLFormation::BlockSoftHoldDistance : HOLFormation::BlockLagDistance);
			bHeld |= bLagging;
			Block.HeldSteps[Unit] = bLagging ? Block.HeldSteps[Unit] + 1 : 0;
			if (Block.Steps > 1)
			{
				if (const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity))
				{
					Block.Report.MaxSpeed = FMath::Max(Block.Report.MaxSpeed, static_cast<float>(Velocity->Value.Size2D()) / Scale);
				}
			}
		}
		// Joined: everyone on its slot (or long enough), the march proper.
		if (Block.bJoining && (MaxGap <= HOLFormation::BlockLagDistance || Block.Steps >= HOLFormation::FormUpMaxSteps))
		{
			Block.bJoining = false;
			Block.Report.JoinSteps = Block.Steps;
			for (const FMassEntityHandle Entity : Block.Entities)
			{
				EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity).Phase = EHOLMovePhase::Block;
			}
		}
		if (Block.Steps > 1 && !Block.bJoining && !Block.bBlending)
		{
			Block.Report.MaxSlotGap = FMath::Max(Block.Report.MaxSlotGap, MaxGap);
		}
		Block.Report.HeldSteps += bHeld ? 1 : 0;
		Block.Report.MarchSteps = Block.Steps;
		// A unit that holds the block too long leaves it: its own path to its place.
		for (int32 Unit = Block.Entities.Num() - 1; Unit >= 0; --Unit)
		{
			if (Block.HeldSteps[Unit] < HOLFormation::BlockWaitMaxSteps)
			{
				continue;
			}
			const FMassEntityHandle Entity = Block.Entities[Unit];
			FHOLMoveOrderFragment& Order = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity);
			Order.Slot = Block.Places[Unit];
			Order.Phase = EHOLMovePhase::WaitingPath;
			FPathRequest& Request = PendingPaths.AddDefaulted_GetRef();
			Request.OrderSerial = Serial;
			Request.Start = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation();
			Request.Target = Order.Slot;
			Request.Entities.Add(Entity);
			SlotClaims.FindOrAdd(GetClaimCell(FVector2D(Order.Slot))).AddUnique(Entity);
			++Block.Report.LeftUnits;
			Block.Entities.RemoveAt(Unit);
			Block.Offsets.RemoveAt(Unit);
			Block.Places.RemoveAt(Unit);
			Block.Kinds.RemoveAt(Unit);
			Block.HeldSteps.RemoveAt(Unit);
			bHeld = false;
		}
		if (Block.Entities.IsEmpty())
		{
			Publish(Serial, Block.Report);
			It.RemoveCurrent();
			continue;
		}
		if (!bHeld)
		{
			const double Share = Block.bJoining || Block.bBlending ? HOLFormation::GetBlockSoftPace(MaxGap) : 1.;
			Block.Progress = FMath::Min(Block.Progress + Pace * Share * DeltaTime, Block.Length);
		}

		// The arrival blended into the march (the user's 4th trial): over the last BlendDistance, each unit's target slides
		// from its slot to a final place of its own kind, given once here from the slots where the block would stand at the
		// end (no two ways of one kind crossing), through the others of its order meanwhile.
		if (!Block.bBlending && Block.Length - Block.Progress <= Block.BlendDistance)
		{
			const FVector2D EndPoint = HOLFormation::GetPointOnLine(Block.Path, Block.Length);
			const FVector2D EndDirection = HOLFormation::GetDirectionOnLine(Block.Path, Block.Length, HOLFormation::BlockTurnWindow, Fallback);
			TArray<FVector2D> Starts, Targets;
			for (int32 Unit = 0; Unit < Block.Entities.Num(); ++Unit)
			{
				Starts.Add(EndPoint + HOLFormation::BlockOffsetToWorld(Block.Offsets[Unit], EndDirection));
				Targets.Add(FVector2D(Block.Places[Unit]));
			}
			TArray<int32> PlaceOfUnit;
			HOLFormation::AssignPlacesByKind(Starts, Block.Kinds, Targets, Block.FinalForward, PlaceOfUnit);
			const TArray<FVector> Held = Block.Places;
			for (int32 Unit = 0; Unit < Block.Entities.Num(); ++Unit)
			{
				Block.Places[Unit] = Held.IsValidIndex(PlaceOfUnit[Unit]) ? Held[PlaceOfUnit[Unit]] : Held[Unit];
				FHOLMoveOrderFragment& Order = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Block.Entities[Unit]);
				Order.bReforming = true;
				SlotClaims.FindOrAdd(GetClaimCell(FVector2D(Block.Places[Unit]))).AddUnique(Block.Entities[Unit]);
			}
			Block.bBlending = true;
		}

		// Near the path's end (the user's 3rd trial, as in AoE4: no rigid turn of the block): each unit straight to a final
		// place of its own kind, given again from where it stands (no two ways of one kind crossing), through the others
		// of its order meanwhile; then the order's final leg (arrival, facing the front, holding it).
		// The path's end: each unit, about on its final place by now, ends its walk there (final leg: arrival, facing the
		// front, holding it).
		if (Block.bBlending && Block.Length - Block.Progress <= 1.)
		{
			for (int32 Unit = 0; Unit < Block.Entities.Num(); ++Unit)
			{
				const FMassEntityHandle Entity = Block.Entities[Unit];
				FHOLMoveOrderFragment& Order = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity);
				const FTransformFragment& Transform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
				Order.Slot = Block.Places[Unit];
				Order.bReforming = true;
				FVector Hit;
				if (NavData && NavData->Raycast(Transform.GetTransform().GetLocation(), Order.Slot, Hit, NavData->GetDefaultQueryFilter()))
				{
					// Its place is not straight ahead (behind a tree): a path there.
					Order.Phase = EHOLMovePhase::WaitingPath;
					FPathRequest& Request = PendingPaths.AddDefaulted_GetRef();
					Request.OrderSerial = Serial;
					Request.Start = Transform.GetTransform().GetLocation();
					Request.Target = Order.Slot;
					Request.Entities.Add(Entity);
					continue;
				}
				StartFinalLeg(*World, Transform, EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity), Order,
					EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(Entity), EntityManager.GetFragmentDataChecked<FMassNavMeshShortPathFragment>(Entity),
					EntityManager.GetFragmentDataChecked<FMassNavMeshCachedPathFragment>(Entity));
			}
			Block.Report.bArrived = true;
			{
				// The arrival watched (the user's 2nd trial): the straight ways to the places that cross, then the settling.
				FArrivalWatch Watch;
				Watch.Entities = Block.Entities;
				Watch.Places = Block.Places;
				TArray<FVector2D> From;
				for (int32 Unit = 0; Unit < Block.Entities.Num(); ++Unit)
				{
					const FVector Location = EntityManager.GetFragmentDataChecked<FTransformFragment>(Block.Entities[Unit]).GetTransform().GetLocation();
					From.Add(FVector2D(Location));
					Watch.LastOff.Add(FVector::Dist2D(Location, Block.Places[Unit]));
					Block.Report.ArrivalOff = FMath::Max(Block.Report.ArrivalOff, static_cast<float>(Watch.LastOff.Last()));
				}
				const auto Cross = [](const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D)
				{
					const auto Side = [](const FVector2D& P, const FVector2D& Q, const FVector2D& R) { return FVector2D::CrossProduct(Q - P, R - P); };
					return Side(A, B, C) * Side(A, B, D) < 0. && Side(C, D, A) * Side(C, D, B) < 0.;
				};
				// Two of one kind whose ways cross would shove each other; two kinds go through each other (a half turn
				// sends the front row through the others to the new front).
				int32 KindCrossings = 0;
				for (int32 First = 0; First < From.Num(); ++First)
				{
					for (int32 Second = First + 1; Second < From.Num(); ++Second)
					{
						if (Cross(From[First], FVector2D(Block.Places[First]), From[Second], FVector2D(Block.Places[Second])))
						{
							Block.Report.Crossings += Block.Kinds[First] == Block.Kinds[Second] ? 1 : 0;
							++KindCrossings;
						}
					}
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Block march: re-forming, %d crossings of one kind, %d in all"), Block.Report.Crossings, KindCrossings);
				Watch.Report = Block.Report;
				Arrivals.Add(Serial, MoveTemp(Watch));
			}
			UE_LOG(LogHOLSimulation, Display, TEXT("Block march: arrived after %d steps of form-up and %d of march (%d held), slot gap %.0f cm at most, speed %.0f cm/s at most, %d units left it"),
				Block.Report.FormUpSteps, Block.Steps, Block.Report.HeldSteps, Block.Report.MaxSlotGap, Block.Report.MaxSpeed, Block.Report.LeftUnits);
			Publish(Serial, Block.Report);
			It.RemoveCurrent();
			continue;
		}

		// Each unit's slot: the block's point plus its offset turned with the path; off the navmesh (a tree, an edge), pulled
		// in towards the path, the unit squeezed in.
		const FVector2D Point = HOLFormation::GetPointOnLine(Block.Path, Block.Progress);
		const FVector2D Direction = HOLFormation::GetDirectionOnLine(Block.Path, Block.Progress, HOLFormation::BlockTurnWindow, Fallback);
		for (int32 Unit = 0; Unit < Block.Entities.Num(); ++Unit)
		{
			const FMassEntityHandle Entity = Block.Entities[Unit];
			FHOLMoveOrderFragment& Order = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity);
			const FTransformFragment& Transform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
			const FHOLUnitTypeConstants& Constants = EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity);
			const double Z = Transform.GetTransform().GetLocation().Z;
			FVector Slot(Point + HOLFormation::BlockOffsetToWorld(Block.Offsets[Unit], Direction), Z);
			FVector Hit;
			const FVector OnPath(Point, Z);
			if (NavData && NavData->Raycast(OnPath, Slot, Hit, NavData->GetDefaultQueryFilter()))
			{
				const FVector2D Back = FVector2D(Hit) - Point;
				const double Room = FMath::Max(Back.Size() - Constants.Radius, 0.);
				Slot = FVector(Point + Back.GetSafeNormal() * Room, Z);
			}
			if (Block.bBlending)
			{
				const double Blend = HOLFormation::GetBlockBlend(Block.Length - Block.Progress, Block.BlendDistance);
				Slot = FVector(FMath::Lerp(FVector2D(Slot), FVector2D(Block.Places[Unit]), Blend), Z);
			}
			Order.Slot = Slot;
			const float Speed = FMath::Min(GetWalkSpeed(Constants), static_cast<float>(Pace * HOLFormation::BlockCatchUpShare));
			SteerInBlock(*World, Transform, Order, EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(Entity), Speed,
				static_cast<float>(Block.Length - Block.Progress), Direction);
		}
		Publish(Serial, Block.Report);
	}
}

void UHOLUnitOrderProcessor::WatchArrivals(FMassEntityManager& EntityManager)
{
	for (auto It = Arrivals.CreateIterator(); It; ++It)
	{
		FArrivalWatch& Watch = It.Value();
		++Watch.Steps;
		bool bAllThere = true;
		for (int32 Unit = 0; Unit < Watch.Entities.Num(); ++Unit)
		{
			const FMassEntityHandle Entity = Watch.Entities[Unit];
			const FHOLMoveOrderFragment* Order = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
			if (!Order || Order->OrderSerial != It.Key())
			{
				continue;
			}
			const FVector Location = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation();
			const double Off = FVector::Dist2D(Location, Watch.Places[Unit]);
			Watch.Report.Pushes += Off > Watch.LastOff[Unit] + 2. ? 1 : 0;
			Watch.LastOff[Unit] = Off;
			const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);
			bAllThere &= Off <= HOLFormation::FormUpTolerance && (!Velocity || Velocity->Value.Size2D() < 20.);
		}
		if (bAllThere || Watch.Steps >= 400)
		{
			// Settled: they keep clear of each other again.
			for (const FMassEntityHandle Entity : Watch.Entities)
			{
				FHOLMoveOrderFragment* Order = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
				if (Order && Order->OrderSerial == It.Key())
				{
					Order->bReforming = false;
				}
			}
			Watch.Report.SettleSteps = bAllThere ? Watch.Steps : -1;
			UE_LOG(LogHOLSimulation, Display, TEXT("Block march: settled after %d steps, %d crossings, %d pushes, %.0f cm from its place at most at the end"),
				Watch.Report.SettleSteps, Watch.Report.Crossings, Watch.Report.Pushes, Watch.Report.ArrivalOff);
			if (It.Key() == LastBlockSerial)
			{
				LastBlockReport = Watch.Report;
			}
			It.RemoveCurrent();
		}
	}
}

void UHOLUnitOrderProcessor::ApplyCombatOrder(FMassEntityManager& EntityManager, const UWorld& World, const FHOLMoveOrder& Order,
	const TConstArrayView<FMassEntityHandle> Entities, const FMassEntityHandle AttackTarget)
{
	for (const FMassEntityHandle Entity : Entities)
	{
		if (FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity); Gather && Gather->IsGathering())
		{
			Gather->Phase = EHOLGatherPhase::None;
			Gather->SourceIndex = INDEX_NONE;
			Gather->bWalkIssued = false;
		}
		if (FHOLBuildFragment* Build = EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(Entity))
		{
			Build->Clear();
		}
		// Stands dead where it is (as an arrival, D290): its order serial cleared, a path still to come is not applied.
		FMassMoveTargetFragment& MoveTarget = EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(Entity);
		MoveTarget.Center = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity).GetTransform().GetLocation();
		MoveTarget.DistanceToGoal = 0.f;
		MoveTarget.CreateNewAction(EMassMovementAction::Stand, World);
		EntityManager.GetFragmentDataChecked<FMassDesiredMovementFragment>(Entity).DesiredVelocity = FVector::ZeroVector;
		FHOLMoveOrderFragment& MoveOrder = EntityManager.GetFragmentDataChecked<FHOLMoveOrderFragment>(Entity);
		MoveOrder.Phase = EHOLMovePhase::None;
		MoveOrder.OrderSerial = 0;
		MoveOrder.bFormation = false;
		MoveOrder.MarchSpeed = 0.f;
		MoveOrder.PlaceHold.Clear();
		MoveOrder.bFacesFront = false;
		MoveOrder.bReforming = false;
		EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).Activity = EHOLUnitActivity::Idle;
		if (FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity))
		{
			Combat->Disengage();
			Combat->Mode = Order.Kind == EHOLOrderKind::Hold ? EHOLCombatMode::Hold : EHOLCombatMode::Normal;
			if (Order.Kind == EHOLOrderKind::Attack)
			{
				Combat->Target = AttackTarget;
				Combat->bOrderedTarget = true;
			}
		}
	}
	UE_LOG(LogHOLSimulation, Display, TEXT("%s order of slot %d for %d units%s"), Order.Kind == EHOLOrderKind::Attack ? TEXT("Attack")
		: Order.Kind == EHOLOrderKind::Hold ? TEXT("Hold") : TEXT("Stop"), Order.PlayerIndex, Entities.Num(),
		Order.Kind == EHOLOrderKind::Attack ? *FString::Printf(TEXT(" on #%u"), Order.TargetUnitId) : TEXT(""));
}

HOLOrderChain::FStep UHOLUnitOrderProcessor::MakeChainStep(const FHOLMoveOrder& Order)
{
	return Order.bRally ? HOLOrderChain::FStep::MakeRally(Order.Target)
		: Order.Kind == EHOLOrderKind::Attack ? HOLOrderChain::FStep::MakeAttack(Order.TargetUnitId)
		: HOLOrderChain::FStep::MakeMove(Order.Target, Order.Kind == EHOLOrderKind::AttackMove);
}

void UHOLUnitOrderProcessor::GetChainUnitIds(TArray<uint32>& OutUnitIds) const
{
	MoveQueueOfUnit.GetKeys(OutUnitIds);
	for (const TPair<uint32, FChainHead>& Pair : ChainHeadOfUnit)
	{
		OutUnitIds.AddUnique(Pair.Key);
	}
}

void UHOLUnitOrderProcessor::GetChainSteps(const uint32 UnitId, TArray<HOLOrderChain::FStep>& OutSteps) const
{
	OutSteps.Reset();
	if (const FChainHead* Head = ChainHeadOfUnit.Find(UnitId))
	{
		OutSteps.Add(Head->Step);
	}
	if (const TArray<uint32>* Queue = MoveQueueOfUnit.Find(UnitId))
	{
		for (const uint32 QueuedId : *Queue)
		{
			if (const FQueuedMove* Queued = QueuedMoves.Find(QueuedId))
			{
				OutSteps.Add(MakeChainStep(Queued->Order));
			}
		}
	}
}

void UHOLUnitOrderProcessor::ForgetMoveQueue(const uint32 UnitId)
{
	ChainHeadOfUnit.Remove(UnitId);
	TArray<uint32> Queue;
	if (!MoveQueueOfUnit.RemoveAndCopyValue(UnitId, Queue))
	{
		return;
	}
	for (const uint32 QueuedId : Queue)
	{
		if (FQueuedMove* Queued = QueuedMoves.Find(QueuedId))
		{
			Queued->Members.Remove(UnitId);
		}
	}
}

namespace
{
	/** An attack-move not over yet (the user's trial of J7b): its unit fights all it finds, then walks on to its slot; only
	 * there, with nobody left in reach of its search, does the combat processor end it. With the melee off, it never would. */
	bool IsAttackMoveOn(const FHOLCombatFragment* Combat)
	{
		static const IConsoleVariable* CombatEnabled = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Combat.Enabled"));
		return Combat && Combat->Mode == EHOLCombatMode::AttackMove && (!CombatEnabled || CombatEnabled->GetInt() != 0);
	}
}

void UHOLUnitOrderProcessor::StartQueuedMoves(FMassEntityManager& EntityManager)
{
	if (QueuedMoves.IsEmpty() && ChainHeadOfUnit.IsEmpty())
	{
		return;
	}
	const UWorld* World = EntityManager.GetWorld();
	const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (Units == nullptr)
	{
		return;
	}
	// A unit gone, or put to work (gathering, building), leaves its moves.
	TArray<uint32> Gone;
	for (const TPair<uint32, TArray<uint32>>& Pair : MoveQueueOfUnit)
	{
		const FMassEntityHandle Entity = Units->FindEntity(Pair.Key);
		const bool bValid = EntityManager.IsEntityValid(Entity);
		const FHOLGatherFragment* Gather = bValid ? EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity) : nullptr;
		const FHOLBuildFragment* Build = bValid ? EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(Entity) : nullptr;
		if (!bValid || (Gather && Gather->IsGathering()) || (Build && Build->IsBuilding()))
		{
			Gone.Add(Pair.Key);
		}
	}
	for (const uint32 UnitId : Gone)
	{
		ForgetMoveQueue(UnitId);
	}
	// Chains (D328): a unit with no queue left keeps its line until its last step is over (a short grace for its start).
	for (auto It = ChainHeadOfUnit.CreateIterator(); It; ++It)
	{
		if (MoveQueueOfUnit.Contains(It.Key()) || StepCount - It.Value().SinceStep < 10)
		{
			continue;
		}
		const FMassEntityHandle Entity = Units->FindEntity(It.Key());
		const FHOLMoveOrderFragment* Move = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
		const FHOLCombatFragment* Combat = Move ? EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
		const bool bWalking = Move && Move->Phase != EHOLMovePhase::None && !Move->PlaceHold.bReturning;
		const bool bFighting = (Combat && (Combat->bOrderedTarget || Combat->bAttackMoveDetour)) || IsAttackMoveOn(Combat);
		// A task's walks (gathering, building: their own chains, or none) and a death end the player's move too.
		const FHOLGatherFragment* Gather = Move ? EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity) : nullptr;
		const FHOLBuildFragment* Build = Move ? EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(Entity) : nullptr;
		const FHOLUnitStateFragment* State = Move ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		const bool bOnTask = (Gather && Gather->IsGathering()) || (Build && Build->IsBuilding()) || !State || State->CurrentHealth <= 0;
		if ((!bWalking && !bFighting) || bOnTask)
		{
			It.RemoveCurrent();
		}
	}
	// A unit whose queued move starts in this pass stands still until the next step applies it: the move after it in its
	// queue must not start in the same pass, or both are applied together and the later one overrides it (P1, P2, P3:
	// P2 skipped, the user's trial of J7b).
	TSet<uint32> StartedNow;
	for (auto It = QueuedMoves.CreateIterator(); It; ++It)
	{
		FQueuedMove& Queued = It.Value();
		if (Queued.Members.IsEmpty())
		{
			It.RemoveCurrent();
			continue;
		}
		// Every unit still in it has it at the head of its queue, has ended its walk and its attack-move (a unit that stands to
		// strike has ended its walk, not its attack-move: the next step waited for neither the fight nor the slot).
		bool bReady = true;
		for (const uint32 UnitId : Queued.Members)
		{
			if (StartedNow.Contains(UnitId))
			{
				bReady = false;
				break;
			}
			const TArray<uint32>* Queue = MoveQueueOfUnit.Find(UnitId);
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			const FHOLMoveOrderFragment* Move = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity) : nullptr;
			const FHOLCombatFragment* Combat = Move ? EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
			if (!Queue || Queue->IsEmpty() || (*Queue)[0] != It.Key() || !Move || (Move->Phase != EHOLMovePhase::None && !Move->PlaceHold.bReturning)
				|| (Combat && Combat->bOrderedTarget) || IsAttackMoveOn(Combat))
			{
				bReady = false;
				break;
			}
		}
		if (!bReady)
		{
			continue;
		}
		FHOLMoveOrder& Started = StartedQueuedMoves.Add_GetRef(Queued.Order);
		Started.UnitIds = Queued.Members;
		for (const uint32 UnitId : Queued.Members)
		{
			StartedNow.Add(UnitId);
			ChainHeadOfUnit.Add(UnitId, {MakeChainStep(Queued.Order), StepCount});
			TArray<uint32>& Queue = MoveQueueOfUnit.FindChecked(UnitId);
			Queue.RemoveAt(0);
			if (Queue.IsEmpty())
			{
				MoveQueueOfUnit.Remove(UnitId);
			}
		}
		UE_LOG(LogHOLSimulation, Display, TEXT("Queued move started for %d units to (%.0f, %.0f)"), Started.UnitIds.Num(), Started.Target.X, Started.Target.Y);
		It.RemoveCurrent();
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
		// A partial path of a single point: the unit already stands as near its target as it can get (J6b.2: shut in by
		// buildings); where that point is.
		bool bNoWay = false;
		FVector NoWayEnd = FVector::ZeroVector;
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
						// Its corridor is made once from its points: the engine must not find it again in place when tiles change
						// under it (from the walk's first start, the corridor left as it was); a new footprint across it gives
						// its walkers a new path instead (J6b.2).
						Result.Path->EnableRecalculationOnInvalidation(false);
						Result.Path->RemoveOverlappingPoints(FNavCorridor::OverlappingPointTolerance);
						if (Result.Path->GetPathPoints().Num() >= 2)
						{
							Path = Result.Path;
							// Around two neighbouring buildings: perhaps shorter between them (D324).
							if (!Path->IsPartial() && Path->GetPathPoints().Num() > 2)
							{
								Path = FindPathThroughWaysImpl(*World, *NavSystem, *NavData, Start.Location, Request.Target, Path);
							}
						}
						else if (Result.Path->IsPartial() && Result.Path->GetPathPoints().Num() == 1)
						{
							bNoWay = true;
							NoWayEnd = Result.Path->GetPathPoints()[0].Location;
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
				if (!RecentNavChanges.ContainsByPredicate([&Bounds](const FRecentNavChange& Change) { return Change.Area.Intersect(Bounds); })
					&& !NewFootprints.ContainsByPredicate([&Bounds](const FNewFootprint& New) { return New.Bounds.Intersect(Bounds); }))
				{
					ReusedPaths.Add(ReuseKey, { bDetour ? Path : FNavPathSharedPtr(), Corridor, Bounds, FVector2D(Request.Start), FVector2D(Request.Target), StepCount });
				}
			}
		}
		// A partial path ending where the walk starts: the unit stands in a pocket of the navmesh (pushed between the trees
		// of a wood, D308), cut off from the rest. It walks out straight to its slot, the way it came in (light units only
		// keep off obstacles through their paths); before, each walk ended at once and the gatherer gave up with its load.
		// Not for a new path around a building (D310, J6b.2), nor straight through one: a unit shut in by buildings goes where
		// its path ends, its slot out of reach (J6b.2); it walked into the wall and was put back, step after step.
		if (Path.IsValid() && Path->IsPartial() && FVector::Dist2D(Path->GetPathPoints().Last().Location, Request.Start) < PocketEscapeDistance
			&& !Request.bForBuilding && !DoesCrossBuilding(*World, Request.Start, Request.Target, NavData))
		{
			UE_CLOG(LogDetours != 0, LogHOLCommands, Display, TEXT("Detour: out of a navmesh pocket at (%.0f, %.0f), straight to (%.0f, %.0f)"),
				Request.Start.X, Request.Start.Y, Request.Target.X, Request.Target.Y);
			Path.Reset();
			Corridor.Reset();
		}
		// No way at all and a building in the way: it stays where it is (before, it walked straight into the wall, was put
		// back step after step and given the same path again, without end); out of a pocket of trees, it still walks out
		// straight (D308).
		const bool bStayHere = bNoWay && (Request.bForBuilding || DoesCrossBuilding(*World, Request.Start, Request.Target, NavData));
		const bool bFollowPath = Corridor.IsValid() && Corridor->Portals.Num() >= 2 && DirectPaths == 0;
		// A straight path past buildings (D324: the way between two neighbours, room for one unit at a time): straight for the
		// cluster, not for each of its units, walking side by side to their slots; a unit whose own straight way meets the
		// navmesh's edge follows the path through its corridor instead, its place across the march held within the corridor's
		// width (one after another through the way, instead of walking into a wall and being put back at every step, D310).
		const bool bCheckOwnWays = !bFollowPath && !bStayHere && DirectPaths == 0 && NavData && Path.IsValid() && !Path->IsPartial()
			&& Path->GetPathPoints().Num() == 2 && Request.Entities.Num() > 1
			&& DoesCrossBuilding(*World, Request.Start, Request.Target, NavData, Settings->PathCorridorWidth * 0.5);
		TSharedPtr<FNavCorridor> StraightCorridor;
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
			Order.bShortOfSlot = false;
			if (bStayHere)
			{
				Order.bShortOfSlot = FVector::Dist2D(NoWayEnd, Order.Slot) > ShortOfSlotDistance;
				Order.Slot = NoWayEnd;
			}
			if (Path.IsValid() && Path->IsPartial())
			{
				// The target cannot be reached: the units gather where the path ends; the task of a unit sent there knows
				// that its walk's end is no arrival (J6b.2).
				Order.bShortOfSlot = FVector::Dist2D(Path->GetPathPoints().Last().Location, Order.Slot) > ShortOfSlotDistance;
				Order.Slot = Path->GetPathPoints().Last().Location;
			}
			bool bOwnWayBlocked = false;
			if (bCheckOwnWays)
			{
				FVector OwnHit;
				bOwnWayBlocked = NavData->Raycast(Transform.GetTransform().GetLocation(), Order.Slot, OwnHit, NavData->GetDefaultQueryFilter());
				if (bOwnWayBlocked && !StraightCorridor.IsValid())
				{
					StraightCorridor = MakeCorridor(*Path, *NavData, Settings->PathCorridorWidth);
				}
				bOwnWayBlocked = bOwnWayBlocked && StraightCorridor.IsValid() && StraightCorridor->Portals.Num() >= 2;
			}
			if (!bFollowPath && !bOwnWayBlocked)
			{
				// A straight path, or none (start off the navmesh, already there): straight to the slot.
				StartFinalLeg(*World, Transform, Constants, Order, MoveTarget, ShortPath, CachedPath);
				continue;
			}
			CachedPath.NavPath = Path;
			CachedPath.Corridor = bFollowPath ? Corridor : StraightCorridor;
			CachedPath.PathSource = EMassNavigationPathSource::NavMesh;
			RequestShortPath(ShortPath, CachedPath, /*StartIndex*/0, /*NumLeadingPoints*/0, Order.Lateral);
			// A unit ahead of the cluster's start goes on from where it is (short of the last point, so it still ends).
			const float LastDistance = ShortPath.Points[ShortPath.NumPoints - 1].Distance.Get();
			ShortPath.MoveTargetProgressDistance = FMath::Clamp(GetProgressAt(ShortPath, Transform.GetTransform().GetLocation()), 0.f,
				FMath::Max(LastDistance - EndReachedDistance * 2.f, 0.f));

			MoveTarget.DesiredSpeed.Set(GetOrderWalkSpeed(Constants, Order));
			// The slot comes after the path: no slowing down at its end.
			MoveTarget.IntentAtGoal = EMassMovementAction::Move;
			MoveTarget.SlackRadius = 0.f;
			MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);
			Order.Phase = EHOLMovePhase::Path;
			Order.PathAnchor = FVector2f(FVector2D(Transform.GetTransform().GetLocation()));
			Order.StuckSteps = 0;
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
			// In a block march (J7a.4): steered by its block (AdvanceBlocks).
			if (Order.Phase == EHOLMovePhase::FormUp || Order.Phase == EHOLMovePhase::Block)
			{
				continue;
			}

			// A walker takes the units' speed of the cheats at once (hol.Units.SpeedScale): Mass moves its path's point on at
			// that speed too.
			// A group in formation (J7a.4): no faster than the slowest of its order's walkers at the step before (its own
			// speed once the slowest are there or gone to another order); this step's slowest for the next one.
			if (Order.bFormation && Order.Phase != EHOLMovePhase::None)
			{
				if (const float* Slowest = SlowestOfOrder.Find(Order.OrderSerial))
				{
					Order.MarchSpeed = *Slowest;
				}
				float& NextSlowest = NextSlowestOfOrder.FindOrAdd(Order.OrderSerial, Constants.Speed);
				NextSlowest = FMath::Min(NextSlowest, Constants.Speed);
			}
			if (MoveTarget.GetCurrentAction() == EMassMovementAction::Move && MoveTarget.DesiredSpeed.Get() > 0.f)
			{
				MoveTarget.DesiredSpeed.Set(GetOrderWalkSpeed(Constants, Order));
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
				Request.bForBuilding = true;
				Order.Phase = EHOLMovePhase::WaitingPath;
				++TotalBuildingRepaths;
				continue;
			}
			// The tiles under a new footprint are rebuilt (J6b.2, D314): a walker whose way crosses it gets a new path around it
			// (RequestCutPaths, after this pass).
			if (!ReadyFootprints.IsEmpty() && (Order.Phase == EHOLMovePhase::Path || Order.Phase == EHOLMovePhase::FinalLeg)
				&& IsWayCut(Order, Transform.GetTransform().GetLocation(), CachedPath, Constants.Radius, Context.GetEntity(EntityIt)))
			{
				Order.Phase = EHOLMovePhase::WaitingPath;
				continue;
			}
			// Held on its path (J6b.3): Mass moves its target on along the path while it keeps up; a unit pushed behind at a
			// corner of the navmesh (a group going round a building, a neighbour pressing it against the wall) steers at a
			// target already round the corner, straight into the wall, and stood there without end, far from its slot (the
			// final leg's check counts only near it). Not moved on for a while: a new path from where it stands.
			if (Order.Phase == EHOLMovePhase::Path)
			{
				const FVector2f Here(FVector2D(Transform.GetTransform().GetLocation()));
				if (FVector2f::DistSquared(Here, Order.PathAnchor) > FMath::Square(PathStuckDistance))
				{
					Order.PathAnchor = Here;
					Order.StuckSteps = 0;
				}
				else if (++Order.StuckSteps >= PathStuckSteps)
				{
					Order.StuckSteps = 0;
					FPathRequest& Request = PendingPaths.AddDefaulted_GetRef();
					Request.OrderSerial = Order.OrderSerial;
					Request.Start = Transform.GetTransform().GetLocation();
					Request.Target = Order.Slot;
					Request.Entities.Add(Context.GetEntity(EntityIt));
					Request.bForBuilding = true;
					Order.Phase = EHOLMovePhase::WaitingPath;
					++TotalStuckRepaths;
					UE_CLOG(LogDetours != 0, LogHOLCommands, Display, TEXT("Detour: held on its path at (%.0f, %.0f), new path to (%.0f, %.0f)"),
						Here.X, Here.Y, Order.Slot.X, Order.Slot.Y);
					continue;
				}
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
			if (Order.Phase == EHOLMovePhase::None)
			{
				// Standing on its formation place (J7a.4): pushed off it, it walks back straight once the push is over; a
				// unit given work (gathering, building) lets its place go.
				if (States[EntityIt].Activity != EHOLUnitActivity::Idle)
				{
					Order.bFacesFront = false;
				}
				if (Order.PlaceHold.bHeld)
				{
					const FVector Location = Transform.GetTransform().GetLocation();
					FVector Hit;
					if (States[EntityIt].Activity != EHOLUnitActivity::Idle)
					{
						Order.PlaceHold.Clear();
					}
					else if (HOLFormation::WatchPlace(Order.PlaceHold, FVector2D(Location), FVector2D(Order.Slot), Constants.Radius))
					{
						if (NavData && NavData->Raycast(Location, Order.Slot, Hit, NavData->GetDefaultQueryFilter()))
						{
							Order.PlaceHold.Clear();
						}
						else
						{
							Order.bShortOfSlot = false;
							StartFinalLeg(*World, Transform, Constants, Order, MoveTarget, ShortPath, CachedPath);
							States[EntityIt].Activity = EHOLUnitActivity::Moving;
							++TotalPlaceReturns;
						}
					}
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
			// At its formation place (J7a.4), or back on it: it holds it if it stands there; the march is over.
			if (Order.bFormation || Order.PlaceHold.bReturning)
			{
				HOLFormation::EndWalkToPlace(Order.PlaceHold, FVector2D(Transform.GetTransform().GetLocation()), FVector2D(Order.Slot), Constants.Radius);
				// Standing there, it turns towards the front of its rows (held by UHOLInstantOrientationProcessor).
				Order.bFacesFront = true;
			}
			Order.bFormation = false;
			Order.MarchSpeed = 0.f;
		}
	});
	Swap(SlowestOfOrder, NextSlowestOfOrder);
	NextSlowestOfOrder.Reset();
}

void UHOLUnitOrderProcessor::CollectChains(const UWorld& World, const UHOLUnitOrderProcessor* Orders, const UHOLBuildProcessor* Builds,
	const UHOLGatherProcessor* Gathers, const int32 Slot, TArray<HOLOrderChain::FChain>& OutChains)
{
	OutChains.Reset();
	const UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>();
	const UMassEntitySubsystem* EntitySubsystem = World.GetSubsystem<UMassEntitySubsystem>();
	if (!Units || !EntitySubsystem)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	auto StateOfSlot = [&EntityManager, Slot](const FMassEntityHandle Entity) -> const FHOLUnitStateFragment*
	{
		const FHOLUnitStateFragment* State = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
		return State && (Slot == INDEX_NONE || State->OwnerPlayer == Slot) ? State : nullptr;
	};
	TArray<HOLOrderChain::FUnitSteps> UnitSteps;
	TSet<uint32> Builders;
	if (Builds)
	{
		TArray<UHOLBuildProcessor::FChainOfBuilder> BuilderChains;
		Builds->GetChains(BuilderChains);
		for (UHOLBuildProcessor::FChainOfBuilder& Chain : BuilderChains)
		{
			if (const FHOLUnitStateFragment* State = StateOfSlot(Chain.Entity))
			{
				Builders.Add(State->UnitId);
				UnitSteps.Add({State->UnitId, MoveTemp(Chain.Steps)});
			}
		}
	}
	// A player's gather order without Shift (D328): its one step until the unit first reaches the source.
	if (Gathers)
	{
		const UHOLSourceSubsystem* Sources = World.GetSubsystem<UHOLSourceSubsystem>();
		for (const TPair<uint32, UHOLGatherProcessor::FChainSource>& Pair : Gathers->GetChainSources())
		{
			if (!Builders.Contains(Pair.Key) && StateOfSlot(Units->FindEntity(Pair.Key)))
			{
				Builders.Add(Pair.Key);
				HOLOrderChain::FUnitSteps& Steps = UnitSteps.AddDefaulted_GetRef();
				Steps.UnitId = Pair.Key;
				// Born for a rally point on the source (D328): a white step to it.
				const int32 SourceIndex = Pair.Value.SourceIndex;
				Steps.Steps.Add(Pair.Value.bRally && Sources && Sources->GetSources().IsValidIndex(SourceIndex)
					? HOLOrderChain::FStep::MakeRally(Sources->GetSources()[SourceIndex].Location) : HOLOrderChain::FStep::MakeGather(SourceIndex));
			}
		}
	}
	if (Orders)
	{
		TArray<uint32> UnitIds;
		Orders->GetChainUnitIds(UnitIds);
		for (const uint32 UnitId : UnitIds)
		{
			if (!Builders.Contains(UnitId) && StateOfSlot(Units->FindEntity(UnitId)))
			{
				HOLOrderChain::FUnitSteps& Steps = UnitSteps.AddDefaulted_GetRef();
				Steps.UnitId = UnitId;
				Orders->GetChainSteps(UnitId, Steps.Steps);
			}
		}
	}
	UnitSteps.Sort([](const HOLOrderChain::FUnitSteps& A, const HOLOrderChain::FUnitSteps& B) { return A.UnitId < B.UnitId; });
	HOLOrderChain::MakeChains(UnitSteps, OutChains);
}

bool UHOLUnitOrderProcessor::LocateChainStep(const UWorld& World, const HOLOrderChain::FStep& Step, TMap<uint32, const AHOLBuilding*>& Sites,
	FVector& OutLocation, FString* OutTarget)
{
	OutLocation = Step.Point;
	switch (Step.Kind)
	{
	case HOLOrderChain::EStepKind::Attack:
	{
		if (OutTarget)
		{
			*OutTarget = FString::Printf(TEXT(" #%u"), Step.TargetId);
		}
		const UHOLUnitSubsystem* Units = World.GetSubsystem<UHOLUnitSubsystem>();
		const UMassEntitySubsystem* EntitySubsystem = World.GetSubsystem<UMassEntitySubsystem>();
		const FMassEntityManager* EntityManager = EntitySubsystem ? &EntitySubsystem->GetEntityManager() : nullptr;
		const FMassEntityHandle Entity = Units ? Units->FindEntity(Step.TargetId) : FMassEntityHandle();
		const FTransformFragment* Transform = EntityManager && EntityManager->IsEntityValid(Entity) ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
		OutLocation = Transform ? Transform->GetTransform().GetLocation() : FVector::ZeroVector;
		return Transform != nullptr;
	}
	case HOLOrderChain::EStepKind::Build:
	{
		if (Sites.IsEmpty())
		{
			for (TActorIterator<AHOLBuilding> It(const_cast<UWorld*>(&World)); It; ++It)
			{
				Sites.Add(It->GetUniqueID(), *It);
			}
		}
		const AHOLBuilding* const* Site = Sites.Find(Step.TargetId);
		if (OutTarget)
		{
			*OutTarget = FString::Printf(TEXT(" %s"), Site ? *(*Site)->GetTypeName().ToString() : TEXT("(gone)"));
		}
		OutLocation = Site ? (*Site)->GetActorLocation() : FVector::ZeroVector;
		return Site != nullptr;
	}
	case HOLOrderChain::EStepKind::Gather:
	{
		if (OutTarget)
		{
			*OutTarget = FString::Printf(TEXT(" source %d"), Step.SourceIndex);
		}
		const UHOLSourceSubsystem* Sources = World.GetSubsystem<UHOLSourceSubsystem>();
		const bool bFound = Sources && Sources->GetSources().IsValidIndex(Step.SourceIndex);
		OutLocation = bFound ? Sources->GetSources()[Step.SourceIndex].Location : FVector::ZeroVector;
		return bFound;
	}
	default:
		return true;
	}
}

void UHOLUnitOrderProcessor::PublishChains(const UWorld& World, const float DeltaSeconds)
{
	// D328, J7b.5b-1b: at most every 0.25 s, and only while some player has chains or just lost them (nothing otherwise).
	ChainPublishWait -= DeltaSeconds;
	if (ChainPublishWait > 0.)
	{
		return;
	}
	ChainPublishWait = 0.25;
	const UHOLBuildProcessor* Builds = ChainBuilds.Get();
	if (!Builds)
	{
		for (TObjectIterator<UHOLBuildProcessor> It; It; ++It)
		{
			if (It->GetLastWorld() == &World)
			{
				Builds = *It;
				ChainBuilds = *It;
			}
		}
	}
	const UHOLGatherProcessor* Gathers = ChainGathers.Get();
	if (!Gathers)
	{
		for (TObjectIterator<UHOLGatherProcessor> It; It; ++It)
		{
			if (It->GetLastWorld() == &World)
			{
				Gathers = *It;
				ChainGathers = *It;
			}
		}
	}
	const bool bAnyQueue = !MoveQueueOfUnit.IsEmpty() || !ChainHeadOfUnit.IsEmpty() || (Builds && Builds->HasChains())
		|| (Gathers && !Gathers->GetChainSources().IsEmpty());
	if (!bAnyQueue && !bChainsPublished)
	{
		return;
	}
	const AGameStateBase* GameState = World.GetGameState();
	if (!GameState)
	{
		return;
	}
	bool bAnyPublished = false;
	TMap<uint32, const AHOLBuilding*> Sites;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		const int32 Slot = HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE;
		if (Slot == INDEX_NONE)
		{
			continue;
		}
		TArray<HOLOrderChain::FChain> Chains;
		if (bAnyQueue)
		{
			CollectChains(World, this, Builds, Gathers, Slot, Chains);
		}
		TArray<FHOLNetOrderChain> NetChains;
		NetChains.Reserve(Chains.Num());
		for (const HOLOrderChain::FChain& Chain : Chains)
		{
			FHOLNetOrderChain& NetChain = NetChains.AddDefaulted_GetRef();
			NetChain.UnitIds = Chain.UnitIds;
			for (const HOLOrderChain::FStep& Step : Chain.Steps)
			{
				FHOLNetChainStep& NetStep = NetChain.Steps.AddDefaulted_GetRef();
				NetStep.Kind = static_cast<uint8>(Step.Kind);
				FVector Location;
				LocateChainStep(World, Step, Sites, Location);
				NetStep.Location = Location;
				NetStep.TargetId = Step.Kind == HOLOrderChain::EStepKind::Attack ? Step.TargetId : 0;
			}
		}
		bAnyPublished |= !NetChains.IsEmpty();
		HOLPlayerState->SetOrderChains(MoveTemp(NetChains));
	}
	bChainsPublished = bAnyPublished;
}

namespace UE::HeroesOfLiberty::Orders
{
	/** Scripted tries of J4 (D289): every unit, or those of one slot, ordered to a ground point as one group. */
	FAutoConsoleCommandWithWorldAndArgs MoveCommand(TEXT("hol.Units.Move"),
		TEXT("hol.Units.Move X Y [Slot] [q|a|qa]: orders the units (of one slot, or all: -1) to the ground point X Y (cm), as one group; q: queued (Shift), a: attack-move (server or solo)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Units.Move X Y [Slot]"));
				return;
			}
			const FVector Target(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
			const int32 Slot = Args.Num() >= 3 ? FCString::Atoi(*Args[2]) : INDEX_NONE;
			const bool bQueue = Args.Num() >= 4 && Args[3].Contains(TEXT("q"));
			const bool bAttackMove = Args.Num() >= 4 && Args[3].Contains(TEXT("a"));
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
				// One slot's units: as that player's order (D328, its chain drawn); all: the server's own (no chain).
				TOptional<UHOLCommandSubsystem::FPlayerOrderScope> PlayerOrder;
				if (Slot != INDEX_NONE)
				{
					PlayerOrder.Emplace(*Commands);
				}
				if (bAttackMove)
				{
					Commands->IssueAttackMoveOrder(UnitIds, Target, INDEX_NONE, bQueue);
				}
				else
				{
					Commands->IssueMoveOrder(UnitIds, Target, INDEX_NONE, false, FVector2D::ZeroVector, false, bQueue);
				}
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

	/**
	 * The chains of queued orders of a player (D328, J7b.5b-1a; server or solo): one per common queue, its units, its steps
	 * (kind, target) and where each step is now (the point; the unit taken on; the site; the source).
	 */
	FAutoConsoleCommandWithWorldAndArgs ChainsCommand(TEXT("hol.Orders.Chains"),
		TEXT("hol.Orders.Chains [Slot]: logs the chains of queued orders (Shift) of a player, one per common queue: units, steps and their locations now (server or solo); on a client, the chains it received (J7b.5b-1b)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			const int32 Slot = Args.IsEmpty() ? INDEX_NONE : FCString::Atoi(*Args[0]);
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				if (!World || !World->IsGameWorld())
				{
					continue;
				}
				// What this machine's player received (D328, J7b.5b-1b), its attack targets where it shows them.
				const APlayerController* LocalController = World->GetFirstPlayerController();
				const AHOLPlayerState* LocalState = LocalController && LocalController->IsLocalController() ? LocalController->GetPlayerState<AHOLPlayerState>() : nullptr;
				if (LocalState)
				{
					const TArray<FHOLNetOrderChain>& Received = LocalState->GetOrderChains();
					UE_LOG(LogHOLSimulation, Display, TEXT("Order chains (received) of slot %d: %d"), LocalState->GetPlayerIndex(), Received.Num());
					for (int32 ChainIndex = 0; ChainIndex < Received.Num(); ++ChainIndex)
					{
						const FHOLNetOrderChain& Chain = Received[ChainIndex];
						TArray<FString> UnitTexts;
						for (const uint32 UnitId : Chain.UnitIds)
						{
							UnitTexts.Add(FString::Printf(TEXT("#%u"), UnitId));
						}
						TArray<FString> StepTexts;
						for (const FHOLNetChainStep& Step : Chain.Steps)
						{
							FVector Location;
							const bool bFound = AHOLPlayerState::LocateChainStep(*World, Step, Location);
							const FString Target = Step.TargetId != 0 ? FString::Printf(TEXT(" #%u"), Step.TargetId) : FString();
							StepTexts.Add(FString::Printf(TEXT("%s%s at (%.0f, %.0f)%s"), HOLOrderChain::KindName(static_cast<HOLOrderChain::EStepKind>(Step.Kind)),
								*Target, Location.X, Location.Y, bFound ? TEXT("") : TEXT(" (fallback)")));
						}
						UE_LOG(LogHOLSimulation, Display, TEXT("Order chain (received) %d: units %s; steps %s"), ChainIndex + 1, *FString::Join(UnitTexts, TEXT(" ")),
							*FString::Join(StepTexts, TEXT(" -> ")));
					}
				}
				if (World->GetNetMode() == NM_Client)
				{
					continue;
				}
				const UHOLUnitOrderProcessor* Orders = nullptr;
				for (TObjectIterator<UHOLUnitOrderProcessor> It; It; ++It)
				{
					Orders = It->GetLastWorld() == World ? *It : Orders;
				}
				const UHOLBuildProcessor* Builds = nullptr;
				for (TObjectIterator<UHOLBuildProcessor> It; It; ++It)
				{
					Builds = It->GetLastWorld() == World ? *It : Builds;
				}
				const UHOLGatherProcessor* Gathers = nullptr;
				for (TObjectIterator<UHOLGatherProcessor> It; It; ++It)
				{
					Gathers = It->GetLastWorld() == World ? *It : Gathers;
				}
				TArray<HOLOrderChain::FChain> Chains;
				UHOLUnitOrderProcessor::CollectChains(*World, Orders, Builds, Gathers, Slot, Chains);
				TMap<uint32, const AHOLBuilding*> Sites;
				UE_LOG(LogHOLSimulation, Display, TEXT("Order chains of slot %d: %d"), Slot, Chains.Num());
				for (int32 ChainIndex = 0; ChainIndex < Chains.Num(); ++ChainIndex)
				{
					const HOLOrderChain::FChain& Chain = Chains[ChainIndex];
					TArray<FString> UnitTexts;
					for (const uint32 UnitId : Chain.UnitIds)
					{
						UnitTexts.Add(FString::Printf(TEXT("#%u"), UnitId));
					}
					TArray<FString> StepTexts;
					for (const HOLOrderChain::FStep& Step : Chain.Steps)
					{
						FString Target;
						FVector Location;
						const bool bFound = UHOLUnitOrderProcessor::LocateChainStep(*World, Step, Sites, Location, &Target);
						StepTexts.Add(bFound ? FString::Printf(TEXT("%s%s at (%.0f, %.0f)"), HOLOrderChain::KindName(Step.Kind), *Target, Location.X, Location.Y)
							: FString::Printf(TEXT("%s%s (gone)"), HOLOrderChain::KindName(Step.Kind), *Target));
					}
					UE_LOG(LogHOLSimulation, Display, TEXT("Order chain %d: units %s; steps %s"), ChainIndex + 1, *FString::Join(UnitTexts, TEXT(" ")),
						*FString::Join(StepTexts, TEXT(" -> ")));
				}
			}
		}));

	/**
	 * Scripted checks of the formation (J7a.4, quick_check "formation"; server or solo): each unit's type, place, yaw,
	 * speed (its velocity, its own walking speed, the march speed of its order), front yaw and hold on its place.
	 */
	FAutoConsoleCommandWithWorldAndArgs FormationCommand(TEXT("hol.Units.Formation"),
		TEXT("hol.Units.Formation [Slot]: logs each unit's type, location, yaw, speed, march speed, front yaw and hold on its formation place, and the place returns so far (server or solo)"),
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
					const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);
					const FHOLUnitTypeConstants* Constants = EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Entity);
					if (!State || !Order || !Transform || !Constants || (Slot != INDEX_NONE && State->OwnerPlayer != Slot))
					{
						continue;
					}
					const FVector Location = Transform->GetTransform().GetLocation();
					Lines.Add(FString::Printf(TEXT("#%u %s at (%.0f, %.0f) yaw %.1f speed %.0f own %.0f march %.0f front %.1f faces %d held %d returning %d place (%.0f, %.0f) off %.0f"),
						UnitId, *Constants->TypeName.ToString(), Location.X, Location.Y, Transform->GetTransform().Rotator().Yaw,
						Velocity ? Velocity->Value.Size2D() : -1., GetWalkSpeed(*Constants), Order->bFormation ? GetOrderWalkSpeed(*Constants, *Order) : 0.f,
						Order->FrontYaw, Order->bFacesFront ? 1 : 0, Order->PlaceHold.bHeld ? 1 : 0, Order->PlaceHold.bReturning ? 1 : 0,
						Order->Slot.X, Order->Slot.Y, FVector::Dist2D(Order->Slot, Location)) + FString::Printf(TEXT(" phase %s"),
						Order->Phase == EHOLMovePhase::Block && Order->bReforming ? TEXT("Reform") : *StaticEnum<EHOLMovePhase>()->GetNameStringByValue(static_cast<int64>(Order->Phase))));
				}
				UE_LOG(LogHOLSimulation, Display, TEXT("Formation report: returns %lld; %s"), UHOLUnitOrderProcessor::GetTotalPlaceReturns(), *FString::Join(Lines, TEXT("; ")));
				const UHOLUnitOrderProcessor::FBlockReport& Block = UHOLUnitOrderProcessor::GetLastBlockReport();
				UE_LOG(LogHOLSimulation, Display, TEXT("Block report: units %d formup %d march %d held %d left %d pace %.0f gap %.0f speed %.0f arrived %d settle %d crossings %d pushes %d arrivaloff %.0f"),
					Block.Units, Block.FormUpSteps, Block.MarchSteps, Block.HeldSteps, Block.LeftUnits, Block.Pace, Block.MaxSlotGap, Block.MaxSpeed, Block.bArrived ? 1 : 0,
					Block.SettleSteps, Block.Crossings, Block.Pushes, Block.ArrivalOff);
			}
		}));
}
