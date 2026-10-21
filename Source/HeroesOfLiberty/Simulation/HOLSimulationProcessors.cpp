// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSimulationProcessors.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassNavigationFragments.h"
#include "Misc/ScopeExit.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLSimulationFragments.h"

namespace UE::HeroesOfLiberty::Simulation
{
	/** Buckets of the look-ahead's cell table (a power of two), and the width (cm) of its cells. */
	constexpr int32 LookAheadBuckets = 4096;
	constexpr double LookAheadCellSize = 250.0;
	/** Angle (degrees) between two ways a walker tries around a body ahead (D293). */
	constexpr double AvoidAngleStep = 10.0;
	/** Below this speed (cm/s) a unit keeps its facing (UHOLInstantOrientationProcessor). */
	constexpr double MinFacingSpeed = 20.0;
	/** Part of the two bodies' radii a walker keeps between them when it looks ahead (D293, D294). */
	float AvoidRadiusScale = 1.1f;
	FAutoConsoleVariableRef CVarAvoidClearance(TEXT("hol.Units.AvoidClearance"), AvoidRadiusScale,
		TEXT("Distance a walker keeps from the bodies it looks ahead for, as a part of the two radii (D293, D294): above 1, a margin"));

	int32 AvoidEnabled = 1;
	FAutoConsoleVariableRef CVarAvoid(TEXT("hol.Units.Avoid"), AvoidEnabled,
		TEXT("1: a walker turns aside from the bodies it would meet within hol.Units.AvoidTime, and pushes only when no way within hol.Units.AvoidMaxAngle is clear (D293); 2: the same, and its average cost logged every 200 steps; 0: off, for A / B measures"));
	int32 AvoidEnemies = 1;
	FAutoConsoleVariableRef CVarAvoidEnemies(TEXT("hol.Simulation.AvoidEnemies"), AvoidEnemies,
		TEXT("1: a walker looking ahead turns aside from enemies as from friends, as before (D293); 0: not from enemies, it walks into them and stops against them (J7c.4 loop 11)"));
	float AvoidTime = 0.5f;
	FAutoConsoleVariableRef CVarAvoidTime(TEXT("hol.Units.AvoidTime"), AvoidTime,
		TEXT("Seconds a walker looks ahead for bodies on its way (D293)"));
	float AvoidMaxAngle = 60.f;
	FAutoConsoleVariableRef CVarAvoidMaxAngle(TEXT("hol.Units.AvoidMaxAngle"), AvoidMaxAngle,
		TEXT("Largest turn (degrees) a walker makes around a body ahead; beyond, the detour is too long and it pushes its way (D293)"));

	int32 PathAim = 1;
	FAutoConsoleVariableRef CVarPathAim(TEXT("hol.Units.PathAim"), PathAim,
		TEXT("1: a walker following a path heads at once for the point the path follow moves ahead of it (D308); 0: the steering's direction, which turns at ~50 degrees a second and swings wide at the corners"));
	/** How far beyond the path follow's point a walker aims (cm), along the path (Mass' steering aims ahead the same way). */
	constexpr double PathAimLead = 100.;

	int32 GhostTrips = 1;
	FAutoConsoleVariableRef CVarGhostTrips(TEXT("hol.Units.GhostTrips"), GhostTrips,
		TEXT("1: a gatherer on its round trip passes through every unit, none turns aside for it nor pushes it (D309); 0: bodies as any unit's"));

	/**
	 * Where a walker following its short path heads (D308): Aim, PathDistance along the path, unless the path has a point
	 * between the walker and Aim: then that point, the next one once within Reached (cm) of it. The path follow's point
	 * runs up to ~5 m ahead of the walker (Mass' look-ahead at its speed): round a building's corner, the straight line to
	 * it cut through the building (D310, J6a.1: 1.9 m into the main centre).
	 */
	FVector GetAimOnPath(const FMassNavMeshShortPathFragment& ShortPath, const FVector& Location, const FVector& Aim, const double PathDistance,
		const double Reached)
	{
		if (ShortPath.NumPoints < 2)
		{
			return Aim;
		}
		// The walker's segment: the nearest one, as Mass' path follow finds it.
		uint8 Segment = 0;
		double Nearest = TNumericLimits<double>::Max();
		for (uint8 Index = 0; Index + 1 < ShortPath.NumPoints; ++Index)
		{
			const double DistanceSquared = FMath::PointDistToSegmentSquared(Location, ShortPath.Points[Index].Position, ShortPath.Points[Index + 1].Position);
			if (DistanceSquared < Nearest)
			{
				Nearest = DistanceSquared;
				Segment = Index;
			}
		}
		uint8 Next = Segment + 1;
		while (Next + 1 < ShortPath.NumPoints && FVector::DistSquared2D(Location, ShortPath.Points[Next].Position) <= FMath::Square(Reached))
		{
			++Next;
		}
		return PathDistance > ShortPath.Points[Next].Distance.Get() ? ShortPath.Points[Next].Position : Aim;
	}

	/** A gatherer on its round trip (D309): no look-ahead, no separation, for it or against it. */
	bool IsGhost(const FHOLUnitStateFragment& State)
	{
		return GhostTrips != 0 && State.Activity == EHOLUnitActivity::GatherTrip;
	}

	/** Time spent and steps run since the last log (hol.Units.Avoid 2). */
	double AvoidSeconds = 0.;
	int32 AvoidRuns = 0;

	/**
	 * Time (s) at which two bodies come into contact, the other one at RelativePosition from this one and this one moving
	 * at RelativeVelocity towards it; never (the largest double) if they do not meet, move apart or already overlap
	 * (the separation's business).
	 */
	double TimeToContact(const FVector2D& RelativePosition, const FVector2D& RelativeVelocity, const double ContactDistance)
	{
		const double SpeedSquared = RelativeVelocity.SizeSquared();
		const double Approach = FVector2D::DotProduct(RelativePosition, RelativeVelocity);
		const double Gap = RelativePosition.SizeSquared() - FMath::Square(ContactDistance);
		if (Gap <= 0. || Approach <= 0. || SpeedSquared <= UE_KINDA_SMALL_NUMBER)
		{
			return TNumericLimits<double>::Max();
		}
		const double Discriminant = FMath::Square(Approach) - SpeedSquared * Gap;
		return Discriminant < 0. ? TNumericLimits<double>::Max() : (Approach - FMath::Sqrt(Discriminant)) / SpeedSquared;
	}
}

UHOLStorePreviousTransformProcessor::UHOLStorePreviousTransformProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Tasks);
}

void UHOLStorePreviousTransformProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLPreviousTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UHOLStorePreviousTransformProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHOLPreviousTransformFragment> PreviousTransforms = Context.GetMutableFragmentView<FHOLPreviousTransformFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			PreviousTransforms[EntityIt].Transform = Transforms[EntityIt].GetTransform();
		}
	});
}

UHOLFullSpeedProcessor::UHOLFullSpeedProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the forces are in the desired velocity, before it moves the unit.
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassApplyForceProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassApplyMovementProcessor"));
}

void UHOLFullSpeedProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassNavMeshShortPathFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

bool UHOLFullSpeedProcessor::BuildLookAhead(FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	Entities.Reset();
	Positions.Reset();
	Velocities.Reset();
	Radii.Reset();
	FormUpSerials.Reset();
	FormUpKinds.Reset();
	Owners.Reset();
	MaxRadius = 1.f;
	MaxSpeed = 0.f;
	bool bAnyWalking = false;
	EntityQuery.ForEachEntityChunk(Context, [this, &bAnyWalking](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> UnitVelocities = Context.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			// Nobody turns aside for a gatherer on its round trip (D309).
			if (IsGhost(States[EntityIt]))
			{
				continue;
			}
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			const FVector2D Velocity(UnitVelocities[EntityIt].Value.X, UnitVelocities[EntityIt].Value.Y);
			Entities.Add(Context.GetEntity(EntityIt));
			Positions.Add(FVector2D(Location.X, Location.Y));
			Velocities.Add(Velocity);
			Radii.Add(AgentRadii[EntityIt].Radius);
			FormUpSerials.Add(GetPassThroughSerial(Orders[EntityIt]));
			FormUpKinds.Add(GetPassThroughKind(Orders[EntityIt]));
			Owners.Add(States[EntityIt].OwnerPlayer);
			MaxRadius = FMath::Max(MaxRadius, AgentRadii[EntityIt].Radius);
			MaxSpeed = FMath::Max(MaxSpeed, static_cast<float>(Velocity.Size()));
			bAnyWalking |= MoveTargets[EntityIt].GetCurrentAction() == EMassMovementAction::Move;
		}
	});
	if (!bAnyWalking)
	{
		return false;
	}
	CellHeads.Init(INDEX_NONE, LookAheadBuckets);
	NextInCell.SetNumUninitialized(Positions.Num());
	Cells.SetNumUninitialized(Positions.Num());
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		Cells[Index] = FIntPoint(FMath::FloorToInt32(Positions[Index].X / LookAheadCellSize), FMath::FloorToInt32(Positions[Index].Y / LookAheadCellSize));
		int32& Head = CellHeads[LookAheadBucketOf(Cells[Index])];
		NextInCell[Index] = Head;
		Head = Index;
	}
	return true;
}

int32 UHOLFullSpeedProcessor::LookAheadBucketOf(const FIntPoint& Cell)
{
	return static_cast<int32>((static_cast<uint32>(Cell.X) * 73856093u ^ static_cast<uint32>(Cell.Y) * 19349663u) & (UE::HeroesOfLiberty::Simulation::LookAheadBuckets - 1));
}

FVector UHOLFullSpeedProcessor::AvoidBodies(const FMassEntityHandle Entity, const FVector& Location, const float Radius, const FVector& Direction, const float Speed, const double Horizon,
	const uint32 FormUpSerial, const uint8 FormUpKind, const uint8 Owner) const
{
	using namespace UE::HeroesOfLiberty::Simulation;

	// The bodies that could be met within the horizon, whatever way this one takes: in the reach of both speeds, and not
	// behind it (no turn reaches them; one catching up from behind is its own to avoid).
	const FVector2D Position(Location.X, Location.Y);
	const FVector2D Ahead(Direction.X, Direction.Y);
	// Only the cells the square around that reach overlaps (D302: 9 to 16 instead of 25 at 3 m/s), in the same order.
	const double Reach = (Speed + MaxSpeed) * Horizon + (Radius + MaxRadius) * FMath::Max(1.f, AvoidRadiusScale);
	const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y - Reach) / LookAheadCellSize));
	const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y + Reach) / LookAheadCellSize));
	TArray<int32, TInlineAllocator<64>> Near;
	for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
	{
		for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
		{
			const FIntPoint Cell(X, Y);
			for (int32 Other = CellHeads[LookAheadBucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
			{
				if (Cells[Other] != Cell || Entities[Other] == Entity || CanPassThrough(FormUpSerial, FormUpKind, FormUpSerials[Other], FormUpKinds[Other])
					|| (AvoidEnemies == 0 && Owners[Other] != Owner))
				{
					continue;
				}
				const FVector2D ToOther = Positions[Other] - Position;
				const double ContactDistance = (Radius + Radii[Other]) * AvoidRadiusScale;
				const double OtherReach = (Speed + Velocities[Other].Size()) * Horizon + ContactDistance;
				if (FVector2D::DotProduct(ToOther, Ahead) > -ContactDistance && ToOther.SizeSquared() < FMath::Square(OtherReach))
				{
					Near.Add(Other);
				}
			}
		}
	}
	if (Near.IsEmpty())
	{
		return Direction;
	}

	// The first body met going this way, and when.
	const auto FirstContact = [this, &Near, &Position, Radius, Speed](const FVector2D& Way, int32& OutOther)
	{
		double First = TNumericLimits<double>::Max();
		OutOther = INDEX_NONE;
		for (const int32 Other : Near)
		{
			const double Time = TimeToContact(Positions[Other] - Position, Way * Speed - Velocities[Other], (Radius + Radii[Other]) * AvoidRadiusScale);
			if (Time < First)
			{
				First = Time;
				OutOther = Other;
			}
		}
		return First;
	};
	// Whether nothing is met going this way within the horizon: the first contact found is enough (D302).
	const auto IsClear = [this, &Near, &Position, Radius, Speed, Horizon](const FVector2D& Way)
	{
		for (const int32 Other : Near)
		{
			if (TimeToContact(Positions[Other] - Position, Way * Speed - Velocities[Other], (Radius + Radii[Other]) * AvoidRadiusScale) < Horizon)
			{
				return false;
			}
		}
		return true;
	};
	int32 Blocker = INDEX_NONE;
	if (FirstContact(Ahead, Blocker) >= Horizon)
	{
		return Direction;
	}

	// Around it on the side away from it (the same at every step while it passes; head-on, both bodies keep the same
	// hand and pass each other), the smallest turn first; a turn beyond the largest is too long a detour: on, pushing.
	const FVector2D ToBlocker = Positions[Blocker] - Position;
	const double Side = FVector2D::CrossProduct(Ahead, ToBlocker) > 0. ? -1. : 1.;
	const int32 Turns = FMath::FloorToInt32(FMath::Max(0.f, AvoidMaxAngle) / AvoidAngleStep);
	for (int32 Turn = 1; Turn <= Turns; ++Turn)
	{
		for (const double Hand : { Side, -Side })
		{
			const FVector2D Way = Ahead.GetRotated(Hand * Turn * AvoidAngleStep);
			if (IsClear(Way))
			{
				return FVector(Way.X, Way.Y, 0.);
			}
		}
	}
	return Direction;
}

void UHOLFullSpeedProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	const double StartSeconds = FPlatformTime::Seconds();
	ON_SCOPE_EXIT
	{
		if (AvoidEnabled == 2)
		{
			AvoidSeconds += FPlatformTime::Seconds() - StartSeconds;
			if (++AvoidRuns == 200)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Avoid: %.3f ms per step on average (%d units)"), AvoidSeconds * 1000. / AvoidRuns, Positions.Num());
				AvoidSeconds = 0.;
				AvoidRuns = 0;
			}
		}
	};
	const bool bLookAhead = AvoidEnabled != 0 && BuildLookAhead(Context);
	const double LookAheadTime = FMath::Max(0.f, AvoidTime);

	// Chunks taken by the threads as they come free: a chunk of a dense crowd costs more (D302).
	EntityQuery.ParallelForEachEntityChunk(Context, [this, bLookAhead, LookAheadTime](FMassExecutionContext& Context)
	{
		// As UMassApplyMovementProcessor, which moves the unit by this velocity next.
		const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
		const TArrayView<FMassDesiredMovementFragment> Movements = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FMassNavMeshShortPathFragment> ShortPaths = Context.GetFragmentView<FMassNavMeshShortPathFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassMoveTargetFragment& MoveTarget = MoveTargets[EntityIt];
			const float Speed = MoveTarget.DesiredSpeed.Get();
			FVector& Velocity = Movements[EntityIt].DesiredVelocity;
			if (MoveTarget.GetCurrentAction() != EMassMovementAction::Move || Speed <= 0.f || DeltaTime <= 0.f)
			{
				// A standing unit does not move: Mass' standing avoidance would drift it by a few cm/s for seconds. The
				// walkers go around it (moving avoidance).
				Velocity = FVector::ZeroVector;
				continue;
			}
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			const FHOLMoveOrderFragment& Order = Orders[EntityIt];

			const FVector ToSlot(Order.Slot.X - Location.X, Order.Slot.Y - Location.Y, 0.);
			const double SlotDistance = ToSlot.Size();
			const bool bToSlot = Order.Phase == EHOLMovePhase::FinalLeg || Order.Phase == EHOLMovePhase::FormUp || Order.Phase == EHOLMovePhase::Block;
			if (bToSlot && SlotDistance <= Speed * DeltaTime)
			{
				// The last step lands on the slot: the order processor stands the unit there at the next step.
				Velocity = ToSlot / DeltaTime;
				continue;
			}
			FVector Direction;
			if (Order.Phase == EHOLMovePhase::Path && PathAim != 0)
			{
				// Along its path, as straight to its slot below: the steering turned the velocity at ~50 degrees a second, at
				// full speed it swung 5 to 10 m wide of each corner of the path (the user's trial of J5b.2: gatherers going
				// round the centre or out of the wood made huge detours). The path follow's point runs ahead at the unit's
				// speed and waits for it when it falls behind; never past the path's next point (D310), the step's walk from it.
				const FVector Tangent = FVector(MoveTarget.Forward.X, MoveTarget.Forward.Y, 0.).GetSafeNormal();
				const FMassNavMeshShortPathFragment& ShortPath = ShortPaths[EntityIt];
				const FVector Aim = GetAimOnPath(ShortPath, Location, MoveTarget.Center + Tangent * PathAimLead,
					ShortPath.MoveTargetProgressDistance + PathAimLead, Speed * DeltaTime);
				const FVector ToAim(Aim.X - Location.X, Aim.Y - Location.Y, 0.);
				Direction = ToAim.SizeSquared() > 1. ? ToAim.GetSafeNormal() : Tangent.IsZero() ? ToSlot / FMath::Max(SlotDistance, 1.) : Tangent;
			}
			else if (bToSlot)
			{
				// Straight to its slot, a straight way on the navmesh: the steering turns the velocity at ~50 degrees a
				// second (its acceleration), at full speed it circled the slot (D290), and after each turn of the
				// look-ahead it circled the units standing beside the slot without ever coming in (D294).
				Direction = ToSlot / SlotDistance;
			}
			else
			{
				// The steering's direction at full speed; from a standstill it has barely any, so towards the move target.
				FVector Goal = FVector(MoveTarget.Center.X - Location.X, MoveTarget.Center.Y - Location.Y, 0.).GetSafeNormal();
				if (Goal.IsZero())
				{
					Goal = FVector(MoveTarget.Forward.X, MoveTarget.Forward.Y, 0.).GetSafeNormal();
				}
				Direction = FVector(Velocity.X, Velocity.Y, 0.).GetSafeNormal();
				if (Direction.IsZero())
				{
					Direction = Goal;
				}
				else if (FVector::DotProduct(Direction, Goal) < 0.)
				{
					// Never away from the goal at full speed (D292): units pressed together, the avoidance's push sent them
					// running backwards. Sideways only, or on towards the goal.
					const FVector Sideways = (Direction - Goal * FVector::DotProduct(Direction, Goal)).GetSafeNormal();
					Direction = Sideways.IsZero() ? Goal : Sideways;
				}
			}
			// A gatherer on its round trip goes through the others (D309).
			if (bLookAhead && !IsGhost(States[EntityIt]))
			{
				// Around the bodies on its way (D293), those beyond its slot aside: Mass' avoidance runs one step out of 6
				// (D220) and turns too late at full speed, so the walker pushed through them.
				const double Horizon = bToSlot ? FMath::Min(LookAheadTime, SlotDistance / Speed) : LookAheadTime;
				// A unit forming up for a block march goes through the others of its order forming up in other kinds of rows
				// (two of one kind keep clear of each other, the user's trial of playtest note 1 of J7b); one re-forming at a
				// block's arrival, or a straggler walking into its slot, through all of its order (J7a.4, J7b note 1: CanPassThrough),
				// as the separation lets it.
				Direction = AvoidBodies(Context.GetEntity(EntityIt), Location, AgentRadii[EntityIt].Radius, Direction, Speed, Horizon,
					GetPassThroughSerial(Order), GetPassThroughKind(Order), States[EntityIt].OwnerPlayer);
				// On a block's march, never against it (playtest note 1, loop 20: a unit forming up, going round one of
				// its kind, took a step back): the turn's part against the march dropped, sideways or on.
				if ((Order.Phase == EHOLMovePhase::FormUp || Order.Phase == EHOLMovePhase::Block) && !Order.bReforming && !Order.MarchDirection.IsNearlyZero())
				{
					const FVector March(Order.MarchDirection.X, Order.MarchDirection.Y, 0.);
					const double Along = FVector::DotProduct(Direction, March);
					if (Along < 0.)
					{
						Direction = (Direction - March * Along).GetSafeNormal();
					}
				}
			}
			Velocity = Direction * Speed;
		}
	}, FMassEntityQuery::EParallelExecutionFlags::AutoBalance);
}

namespace UE::HeroesOfLiberty::Simulation
{
	/** Buckets of the separation's cell table (a power of two): a few per unit at 2 000 units, 32 KB to clear each step. */
	constexpr int32 SeparationBuckets = 8192;
	/** Rounds of the separation a step (D292). */
	constexpr int32 SeparationRounds = 2;
	/**
	 * What (cm) a gatherer at its source and a unit standing beside it may come closer than the spots' spacing (D309): the
	 * arrival tolerance of an order (OrderArrivalTolerance), so that two gatherers on their spots are never nudged.
	 */
	constexpr double AtSourceTolerance = 5.;

	int32 SeparationEnabled = 1;
	FAutoConsoleVariableRef CVarSeparation(TEXT("hol.Units.Separation"), SeparationEnabled,
		TEXT("1: no body overlaps another once the units have moved (D292); 2: the same, and its average cost logged every 200 steps; 0: off, for A / B measures"));

	/**
	 * Cells of the separation's dense table at most (J7c.4): beyond, the units are spread too far apart for a table over
	 * their bounding box and the hashed buckets are used (1 M cells: 4 MB to clear a step).
	 */
	constexpr int64 SeparationDenseMaxCells = 1 << 20;

	int32 FastSeparation = 1;
	FAutoConsoleVariableRef CVarFastSeparation(TEXT("hol.Simulation.FastSeparation"), FastSeparation,
		TEXT("1: the separation's cells in a dense table over the units' bounding box, the same pairs met in the same order as in the hashed buckets (J7c.4); 0: the hashed buckets only, for A / B measures"));

	float SeparationSlop = 0.f;
	FAutoConsoleVariableRef CVarSeparationSlop(TEXT("hol.Units.SeparationSlop"), SeparationSlop,
		TEXT("Overlap (cm) of two bodies the separation leaves (D294): only what goes beyond is undone"));

	float SeparationFriendPushShare = 0.5f;
	FAutoConsoleVariableRef CVarSeparationFriendPushShare(TEXT("hol.Simulation.SeparationFriendPushShare"), SeparationFriendPushShare,
		TEXT("Of a walker and a standing friend's overlap, the standing one's part (J7c.4 jitter trial; the walker takes the rest): 0.5 as before"));

	int32 SeparationPushToEnemy = 0;
	FAutoConsoleVariableRef CVarSeparationPushToEnemy(TEXT("hol.Simulation.SeparationPushToEnemy"), SeparationPushToEnemy,
		TEXT("1: two standing enemies take half each, both moved (J7c.4 jitter trial); 0: all on the one that looks, as before"));

	int32 EnemyNoPush = 0;
	FAutoConsoleVariableRef CVarEnemyNoPush(TEXT("hol.Simulation.EnemyNoPush"), EnemyNoPush,
		TEXT("1: two enemies never push each other, the one that looks takes the whole overlap (J7c.4 loop 11); 0: as before"));

	// J7c.4 crossing probe (temporary): every N steps, the units past the enemy's median along X, the enemy pairs
	// overlapping by more than half a radius, and the walkers touching an enemy.
	int32 CrossProbeSteps = 0;
	FAutoConsoleVariableRef CVarCrossProbe(TEXT("hol.Debug.CrossProbe"), CrossProbeSteps,
		TEXT("J7c.4 crossing probe: every N separation steps, log the crossing counts (0: off)"));
	int32 CrossProbeCounter = 0;

	/** Time spent and steps run since the last log (hol.Units.Separation 2). */
	double SeparationSeconds = 0.;
	int32 SeparationRuns = 0;
	/** Of that time, the positions gathered into the grid, and the rounds (D302). */
	double SeparationGridSeconds = 0.;
	double SeparationRoundsSeconds = 0.;
}

UHOLUnitSeparationProcessor::UHOLUnitSeparationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the units have moved, before the obstacle grid records where they are.
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassApplyMovementProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavigationObstacleGridProcessor"));
}

void UHOLUnitSeparationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLGatherFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

float UHOLUnitSeparationProcessor::LastMaxCorrection = 0.f;
bool UHOLUnitSeparationProcessor::bSourceProbe = false;
FHOLSeparationSourceProbe UHOLUnitSeparationProcessor::LastSourceProbe;

void UHOLUnitSeparationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	LastMaxCorrection = 0.f;
	LastSourceProbe = FHOLSeparationSourceProbe();
	if (!bSourceProbe)
	{
		ProbeEnds.Reset();
	}
	if (SeparationEnabled == 0)
	{
		return;
	}
	const double StartSeconds = FPlatformTime::Seconds();
	ON_SCOPE_EXIT
	{
		if (SeparationEnabled == 2)
		{
			SeparationSeconds += FPlatformTime::Seconds() - StartSeconds;
			if (++SeparationRuns == 200)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Separation: %.3f ms per step on average (%d units; grid %.3f, rounds %.3f)"), SeparationSeconds * 1000. / SeparationRuns,
					Positions.Num(), SeparationGridSeconds * 1000. / SeparationRuns, SeparationRoundsSeconds * 1000. / SeparationRuns);
				SeparationSeconds = 0.;
				SeparationGridSeconds = 0.;
				SeparationRoundsSeconds = 0.;
				SeparationRuns = 0;
			}
		}
	};
	Positions.Reset();
	Radii.Reset();
	Walking.Reset();
	Ghosts.Reset();
	FormUpSerials.Reset();
	FormUpKinds.Reset();
	Moving.Reset();
	Owners.Reset();
	AtSource.Reset();
	UnitIds.Reset();
	float MaxRadius = 1.f;
	bool bAnyWalking = false;
	EntityQuery.ForEachEntityChunk(Context, [this, &MaxRadius, &bAnyWalking](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FHOLGatherFragment> Gathers = Context.GetFragmentView<FHOLGatherFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			FormUpSerials.Add(!Orders.IsEmpty() ? GetPassThroughSerial(Orders[EntityIt]) : 0u);
			FormUpKinds.Add(!Orders.IsEmpty() ? GetPassThroughKind(Orders[EntityIt]) : uint8(0));
			Positions.Add(FVector2D(Location.X, Location.Y));
			Radii.Add(AgentRadii[EntityIt].Radius);
			// A gatherer on its round trip neither pushes nor is pushed (D309): it never looks, nobody finds it. Landed at
			// its spot, it looks once, as a walker: a unit standing there and itself part (the user's 3rd trial of J5b.2).
			const bool bGhost = IsGhost(States[EntityIt]);
			Ghosts.Add(bGhost);
			const bool bSettling = !Gathers.IsEmpty() && Gathers[EntityIt].bSettling;
			const bool bMoving = !bGhost && MoveTargets[EntityIt].GetCurrentAction() == EMassMovementAction::Move;
			const bool bWalking = bMoving || (!bGhost && bSettling);
			Walking.Add(bWalking);
			Moving.Add(bMoving);
			Owners.Add(States[EntityIt].OwnerPlayer);
			AtSource.Add(States[EntityIt].Activity == EHOLUnitActivity::Gathering);
			UnitIds.Add(States[EntityIt].UnitId);
			MaxRadius = FMath::Max(MaxRadius, AgentRadii[EntityIt].Radius);
			bAnyWalking |= bWalking;
		}
	});
	if (!bAnyWalking)
	{
		return;
	}

	// J7c.4, determinism: the units in identifier order, as the combat's fighters (unique identifiers, the unit subsystem's
	// table): the chunks' order differs between runs for the same state, and everything below follows the units' indices
	// (the cells' lists, the units to look, the corrections applied in place). Nothing to do when the chunks gave it already.
	bool bSorted = true;
	for (int32 Index = 1; Index < UnitIds.Num() && bSorted; ++Index)
	{
		bSorted = UnitIds[Index - 1] < UnitIds[Index];
	}
	if (!bSorted)
	{
		SortKeys.Reset(UnitIds.Num());
		for (int32 Index = 0; Index < UnitIds.Num(); ++Index)
		{
			SortKeys.Add(static_cast<uint64>(UnitIds[Index]) << 32 | static_cast<uint32>(Index));
		}
		SortKeys.Sort();
		SortedIndexOf.SetNumUninitialized(UnitIds.Num());
		for (int32 Sorted = 0; Sorted < SortKeys.Num(); ++Sorted)
		{
			SortedIndexOf[static_cast<int32>(SortKeys[Sorted] & 0xFFFFFFFFull)] = Sorted;
		}
		// Each array moved once, through one scratch buffer (all of them plain values).
		const auto Permute = [this](auto& Array)
		{
			using FElement = typename TRemoveReference<decltype(Array)>::Type::ElementType;
			PermuteScratch.SetNumUninitialized(Array.Num() * sizeof(FElement), EAllowShrinking::No);
			FElement* const Moved = reinterpret_cast<FElement*>(PermuteScratch.GetData());
			for (int32 Sorted = 0; Sorted < SortKeys.Num(); ++Sorted)
			{
				Moved[Sorted] = Array[static_cast<int32>(SortKeys[Sorted] & 0xFFFFFFFFull)];
			}
			FMemory::Memcpy(Array.GetData(), Moved, Array.Num() * sizeof(FElement));
		};
		Permute(Positions);
		Permute(Radii);
		Permute(Walking);
		Permute(Ghosts);
		Permute(FormUpSerials);
		Permute(FormUpKinds);
		Permute(Moving);
		Permute(Owners);
		Permute(AtSource);
	}

	// Cells as wide as the largest pair of bodies, whatever the map's size: the overlaps of a unit are all in the 3 x 3
	// cells around it. The cells go into a fixed table by a hash; a bucket shared by two cells keeps each unit's own.
	const double CellSize = 2.0 * MaxRadius;
	const auto BucketOf = [](const FIntPoint& Cell)
	{
		return static_cast<int32>((static_cast<uint32>(Cell.X) * 73856093u ^ static_cast<uint32>(Cell.Y) * 19349663u) & (SeparationBuckets - 1));
	};
	Cells.SetNumUninitialized(Positions.Num());
	FIntPoint MinCell(MAX_int32, MAX_int32);
	FIntPoint MaxCell(MIN_int32, MIN_int32);
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		Cells[Index] = FIntPoint(FMath::FloorToInt32(Positions[Index].X / CellSize), FMath::FloorToInt32(Positions[Index].Y / CellSize));
		MinCell = FIntPoint(FMath::Min(MinCell.X, Cells[Index].X), FMath::Min(MinCell.Y, Cells[Index].Y));
		MaxCell = FIntPoint(FMath::Max(MaxCell.X, Cells[Index].X), FMath::Max(MaxCell.Y, Cells[Index].Y));
	}
	// Or a dense table (J7c.4): every cell of the units' bounding box, its units side by side. A bucket's chain is walked
	// from the last unit put in to the first, in descending index order; each cell's range is laid out the same way, so
	// the pairs are met in the same order as through the buckets (the corrections are applied in place: the order matters).
	const int64 GridWidth = static_cast<int64>(MaxCell.X) - MinCell.X + 1;
	const int64 GridHeight = static_cast<int64>(MaxCell.Y) - MinCell.Y + 1;
	const bool bDense = FastSeparation != 0 && GridWidth * GridHeight <= SeparationDenseMaxCells;
	if (bDense)
	{
		const int32 CellCount = static_cast<int32>(GridWidth * GridHeight);
		// Counted per cell (into the next cell's entry), then summed up: CellStarts[Cell + 1] is the end of the cell's range.
		CellStarts.Init(0, CellCount + 1);
		DenseCells.SetNumUninitialized(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			DenseCells[Index] = (Cells[Index].Y - MinCell.Y) * static_cast<int32>(GridWidth) + (Cells[Index].X - MinCell.X);
			++CellStarts[DenseCells[Index] + 1];
		}
		for (int32 Cell = 0; Cell < CellCount; ++Cell)
		{
			CellStarts[Cell + 1] += CellStarts[Cell];
		}
		// Each range filled from its end, the units taken in ascending order: read from its start, in descending order.
		// The end moves down to the range's start, which CellStarts[Cell + 1] then holds.
		CellUnits.SetNumUninitialized(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			CellUnits[--CellStarts[DenseCells[Index] + 1]] = Index;
		}
		// Back by one entry: CellStarts[Cell] the start of the cell's range, CellStarts[Cell + 1] its end.
		for (int32 Cell = 0; Cell < CellCount; ++Cell)
		{
			CellStarts[Cell] = CellStarts[Cell + 1];
		}
		CellStarts[CellCount] = Positions.Num();
	}
	else
	{
		CellHeads.Init(INDEX_NONE, SeparationBuckets);
		NextInCell.SetNumUninitialized(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			int32& Head = CellHeads[BucketOf(Cells[Index])];
			NextInCell[Index] = Head;
			Head = Index;
		}
	}

	// The walkers look for their neighbours, then each standing unit they pushed looks for its own (it may now overlap a
	// third one), and so on: units standing together and left alone cost nothing. Each unit looks once a round; a second
	// round undoes most of what the pushes of the first made (two walking groups meeting head-on: 36 cm left with one).
	const double GridSeconds = FPlatformTime::Seconds();
	SeparationGridSeconds += GridSeconds - StartSeconds;
	const double Slop = FMath::Max(0.f, SeparationSlop);
	// A gatherer at its source and a unit standing beside it: the spots' spacing (D309), less what an arrival may stop
	// short of its spot; two gatherers on their spots are not pushed apart, a unit on a gatherer's spot is.
	const double AtSourceDistance = FMath::Max(0., UHOLGatherProcessor::GetSpotSpacing() - AtSourceTolerance);
	Corrections = Positions;
	Queued = Walking;
	ToLook.Reset();
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		if (Walking[Index])
		{
			ToLook.Add(Index);
		}
	}
	// A pair met by the unit that looks (Index), in its cell or one around it.
	const auto Undo = [this, Slop, AtSourceDistance](const int32 Index, const int32 Other)
	{
		// Two units of one order forming up in different kinds of rows, of one block's re-forming, or a
		// straggler walking into its slot (J7a.4, J7b note 1: CanPassThrough): through each other, the ways to the
		// new rows cross (three bodies pressed together, each walking into the next, held each other still).
		// Two forming up in one kind of row keep apart (the user's trial of playtest note 1 of J7b).
		if (CanPassThrough(FormUpSerials[Index], FormUpKinds[Index], FormUpSerials[Other], FormUpKinds[Other]))
		{
			return;
		}
		const FVector2D Delta = Corrections[Index] - Corrections[Other];
		double MinDistance = Radii[Index] + Radii[Other] - Slop;
		if ((AtSource[Index] || AtSource[Other]) && !Moving[Index] && !Moving[Other])
		{
			MinDistance = FMath::Min(MinDistance, AtSourceDistance);
		}
		const double DistanceSquared = Delta.SizeSquared();
		if (DistanceSquared >= FMath::Square(MinDistance))
		{
			return;
		}
		const double Distance = FMath::Sqrt(DistanceSquared);
		// Two units on the same point: apart along a direction of their own, the same at every run.
		const FVector2D Normal = Distance > UE_KINDA_SMALL_NUMBER ? Delta / Distance
			: FVector2D(FMath::Cos(Index * 2.399), FMath::Sin(Index * 2.399));
		// Two units of one player take half each: a walker pushes a standing unit out of its way as it would push a
		// walker. Two enemies never push each other back (the user's trial of J7b): a walker stops against a
		// standing enemy, which stays where it is; the one that looks is a walker or pushed this step, so it moves.
		const double Overlap = MinDistance - Distance;
		const bool bEnemies = Owners[Index] != Owners[Other];
		double Share = HOLCombat::GetOverlapShare(bEnemies, Walking[Index], Walking[Other]);
		// J7c.4 jitter trials, off by default: the standing friend of a walker takes less; two standing enemies half each.
		if (!bEnemies && Walking[Index] != Walking[Other] && SeparationFriendPushShare != 0.5f)
		{
			const double FriendShare = FMath::Clamp(static_cast<double>(SeparationFriendPushShare), 0., 1.);
			Share = Walking[Index] ? 1. - FriendShare : FriendShare;
		}
		else if (bEnemies && !Walking[Index] && !Walking[Other] && SeparationPushToEnemy != 0 && EnemyNoPush == 0)
		{
			Share = 0.5;
		}
		// J7c.4 loop 11 (the user's proposal): two enemies never push each other, the one that looks takes the whole
		// overlap and the other is neither moved nor queued (a pair is undone once a round, by the first to look).
		if (bEnemies && EnemyNoPush != 0)
		{
			Share = 1.;
		}
		if (bSourceProbe)
		{
			// Side A: the walker of a walker and a standing unit, else the one that looks.
			const bool bOtherIsA = Walking[Other] && !Walking[Index];
			FHOLSeparationSourceCell& Cell = LastSourceProbe.Cells[ProbeRound][ProbeFromQueue ? 1 : 0][int32(Walking[Index]) + int32(Walking[Other])][Owners[Index] != Owners[Other] ? 1 : 0];
			Cell.Pairs += 1.;
			Cell.Overlap += Overlap;
			Cell.MoveA += Overlap * (bOtherIsA ? 1. - Share : Share);
			Cell.MoveB += Overlap * (bOtherIsA ? Share : 1. - Share);
		}
		Corrections[Index] += Normal * (Overlap * Share);
		if (Share >= 1.)
		{
			return;
		}
		Corrections[Other] -= Normal * (Overlap * (1. - Share));
		if (!Queued[Other])
		{
			Queued[Other] = true;
			ToLook.Add(Other);
		}
	};
	// J7c.4 source probe, read only: a unit's identifier at its sorted index, and every pair's overlap (each pair once, in
	// the cells of the step's start; the same rules as Undo, a pair whatever looks).
	const auto ProbeIdOf = [this, bSorted](const int32 Index)
	{
		return bSorted ? UnitIds[Index] : static_cast<uint32>(SortKeys[Index] >> 32);
	};
	const auto SumOverlaps = [&](const TArray<FVector2D>& Places, double& OutSum, double& OutPairs)
	{
		for (int32 Index = 0; Index < Places.Num(); ++Index)
		{
			if (Ghosts[Index])
			{
				continue;
			}
			for (int32 Other = 0; Other < Places.Num(); ++Other)
			{
				if (Other <= Index || Ghosts[Other] || FMath::Abs(Cells[Other].X - Cells[Index].X) > 1 || FMath::Abs(Cells[Other].Y - Cells[Index].Y) > 1
					|| CanPassThrough(FormUpSerials[Index], FormUpKinds[Index], FormUpSerials[Other], FormUpKinds[Other]))
				{
					continue;
				}
				double MinDistance = Radii[Index] + Radii[Other] - Slop;
				if ((AtSource[Index] || AtSource[Other]) && !Moving[Index] && !Moving[Other])
				{
					MinDistance = FMath::Min(MinDistance, AtSourceDistance);
				}
				const double DistanceSquared = (Places[Index] - Places[Other]).SizeSquared();
				if (DistanceSquared < FMath::Square(MinDistance))
				{
					OutSum += MinDistance - FMath::Sqrt(DistanceSquared);
					OutPairs += 1.;
				}
			}
		}
	};
	// J7c.4 source probe, read only: every pair's overlap before the rounds, and how far the units came since the last end.
	const int32 WalkersToLook = ToLook.Num();
	if (bSourceProbe)
	{
		SumOverlaps(Positions, LastSourceProbe.OverlapStart, LastSourceProbe.PairsStart);
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			if (const FVector2D* const End = ProbeEnds.Find(ProbeIdOf(Index)))
			{
				(Walking[Index] ? LastSourceProbe.WalkerIn : LastSourceProbe.StandingIn) += (Positions[Index] - *End).Size();
			}
		}
	}
	for (int32 Round = 0; Round < SeparationRounds; ++Round)
	{
		Looked.Init(false, Positions.Num());
		ProbeRound = FMath::Min(Round, 1);
		for (int32 Next = 0; Next < ToLook.Num(); ++Next)
		{
			const int32 Index = ToLook[Next];
			ProbeFromQueue = Next >= WalkersToLook;
			Looked[Index] = true;
			for (int32 DY = -1; DY <= 1; ++DY)
			{
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					const FIntPoint Cell(Cells[Index].X + DX, Cells[Index].Y + DY);
					if (bDense)
					{
						// A cell out of the bounding box holds no unit (nor did its bucket hold any of this cell).
						const int32 X = Cell.X - MinCell.X;
						const int32 Y = Cell.Y - MinCell.Y;
						if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight)
						{
							continue;
						}
						const int32 DenseCell = Y * static_cast<int32>(GridWidth) + X;
						for (int32 Slot = CellStarts[DenseCell]; Slot < CellStarts[DenseCell + 1]; ++Slot)
						{
							// A pair is undone once a round, by the first of the two to look.
							const int32 Other = CellUnits[Slot];
							if (Other != Index && !Looked[Other] && !Ghosts[Other])
							{
								Undo(Index, Other);
							}
						}
						continue;
					}
					for (int32 Other = CellHeads[BucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
					{
						// A pair is undone once a round, by the first of the two to look; another cell of the same bucket is not this one.
						if (Other == Index || Looked[Other] || Ghosts[Other] || Cells[Other] != Cell)
						{
							continue;
						}
						Undo(Index, Other);
					}
				}
			}
		}
	}

	SeparationRoundsSeconds += FPlatformTime::Seconds() - GridSeconds;

	// J7c.4 source probe, read only: the overlap left at the end, and each unit's place for the next step's energy.
	if (bSourceProbe)
	{
		SumOverlaps(Corrections, LastSourceProbe.OverlapEnd, LastSourceProbe.PairsEnd);
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			ProbeEnds.Add(ProbeIdOf(Index), Corrections[Index]);
		}
		LastSourceProbe.bValid = true;
	}

	// J7c.4 crossing probe (temporary), read only.
	if (CrossProbeSteps > 0 && ++CrossProbeCounter >= CrossProbeSteps)
	{
		CrossProbeCounter = 0;
		TArray<double> Xs[2];
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			if (Owners[Index] == 0 || Owners[Index] == 1)
			{
				Xs[Owners[Index]].Add(Corrections[Index].X);
			}
		}
		if (Xs[0].Num() > 0 && Xs[1].Num() > 0)
		{
			Xs[0].Sort();
			Xs[1].Sort();
			const double Median0 = Xs[0][Xs[0].Num() / 2];
			const double Median1 = Xs[1][Xs[1].Num() / 2];
			// The side starting at the lower X goes up.
			const bool bZeroLow = Median0 < Median1;
			int32 Past[2] = { 0, 0 };
			for (const double X : Xs[0]) { Past[0] += bZeroLow ? X > Median1 : X < Median1; }
			for (const double X : Xs[1]) { Past[1] += bZeroLow ? X < Median0 : X > Median0; }
			int32 DeepPairs = 0;
			int32 WalkersTouching = 0;
			// Units with an enemy behind them (more than 50 cm further back along their charge, within 3 m).
			int32 EnemyBehind[2] = { 0, 0 };
			if (Positions.Num() <= 2500)
			{
				for (int32 Index = 0; Index < Positions.Num(); ++Index)
				{
					bool bTouching = false;
					bool bBehind = false;
					const bool bGoesUp = (Owners[Index] == 0) == bZeroLow;
					for (int32 Other = 0; Other < Positions.Num(); ++Other)
					{
						if (Other == Index || Owners[Other] == Owners[Index] || Ghosts[Other])
						{
							continue;
						}
						const double Distance = FVector2D::Distance(Corrections[Index], Corrections[Other]);
						const double Sum = Radii[Index] + Radii[Other];
						DeepPairs += Other > Index && Distance < Sum - 0.5 * Radii[Index];
						bTouching |= Distance < Sum + 5.;
						const double Back = bGoesUp ? Corrections[Index].X - Corrections[Other].X : Corrections[Other].X - Corrections[Index].X;
						bBehind |= Back > 50. && Distance < 300.;
					}
					WalkersTouching += bTouching && Walking[Index];
					if (bBehind && (Owners[Index] == 0 || Owners[Index] == 1))
					{
						++EnemyBehind[Owners[Index]];
					}
				}
			}
			// Histogram of X by side, 2 m bins from 10 m on the low side of the medians' midpoint to 10 m on the high side.
			const double Front = 0.5 * (Median0 + Median1);
			FString Histogram[2];
			for (int32 Side = 0; Side < 2; ++Side)
			{
				int32 Bins[10] = {};
				for (const double X : Xs[Side])
				{
					const int32 Bin = FMath::FloorToInt32((X - Front + 1000.) / 200.);
					if (Bin >= 0 && Bin < 10)
					{
						++Bins[Bin];
					}
				}
				for (int32 Bin = 0; Bin < 10; ++Bin)
				{
					Histogram[Side] += FString::Printf(TEXT("%s%d"), Bin > 0 ? TEXT(",") : TEXT(""), Bins[Bin]);
				}
			}
			UE_LOG(LogHOLSimulation, Display, TEXT("CrossProbe: units %d/%d, medians %.0f/%.0f, past the enemy median %d/%d, deep enemy pairs %d, walkers touching an enemy %d, enemy behind (3 m) %d/%d, front %.0f, X bins (2 m, low to high) [%s] / [%s]"),
				Xs[0].Num(), Xs[1].Num(), Median0, Median1, Past[0], Past[1], DeepPairs, WalkersTouching, EnemyBehind[0], EnemyBehind[1], Front, *Histogram[0], *Histogram[1]);
		}
	}

	// J7c.4 probe, read only: the step's longest correction, over all units.
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		LastMaxCorrection = FMath::Max(LastMaxCorrection, static_cast<float>((Corrections[Index] - Positions[Index]).Size()));
	}

	// The chunks visited in the gathering's order: each unit's sorted index (J7c.4, determinism).
	int32 Gathered = 0;
	EntityQuery.ForEachEntityChunk(Context, [this, &Gathered, bSorted](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt, ++Gathered)
		{
			const int32 Index = bSorted ? Gathered : SortedIndexOf[Gathered];
			if (Corrections[Index] != Positions[Index])
			{
				FTransform& Transform = Transforms[EntityIt].GetMutableTransform();
				const FVector Location = Transform.GetLocation();
				Transform.SetLocation(FVector(Corrections[Index].X, Corrections[Index].Y, Location.Z));
			}
		}
	});
}

namespace UE::HeroesOfLiberty::Simulation
{
	/** Times a body is put back out of the footprints a step, at most: two buildings side by side, out of one into the other. */
	constexpr int32 KeepOutPasses = 3;
	/** The largest body the footprints' reach allows for (cm): above any unit's radius (60 cm, the Peasant drawn x1.5). */
	constexpr double KeepOutMaxRadius = 200.;
	/** Gap (cm) a body put back is left from the wall: the rounding of the move back does not leave it a hair inside. */
	constexpr double KeepOutSlack = 1.;
	/** Side (cm) of the cells of the keep-out's grid, and cells at most (a larger side past that). */
	constexpr double KeepOutCellSize = 2000.;
	constexpr int32 KeepOutMaxCells = 4096;

	int32 KeepOutEnabled = 1;
	FAutoConsoleVariableRef CVarKeepOut(TEXT("hol.Units.KeepOut"), KeepOutEnabled,
		TEXT("1: a body in a building's footprint is put back against its wall (D310); 2: the same, each one logged; 0: off, for A / B measures"));
}

std::atomic<int64> UHOLFootprintKeepOutProcessor::NumPushedOut{ 0 };

UHOLFootprintKeepOutProcessor::UHOLFootprintKeepOutProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the units moved and were pushed apart, before the obstacle grid records where they are.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitSeparationProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavigationObstacleGridProcessor"));
	bRequiresGameThreadExecution = true; // the buildings are read from the world
}

void UHOLFootprintKeepOutProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLFootprintKeepOutProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;
	const UWorld* World = EntityManager.GetWorld();
	if (!World || KeepOutEnabled == 0)
	{
		return;
	}
	// A few buildings per player: read again at each step, as the gathering's deposits.
	Footprints.Reset();
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		const FHOLBuildingTypeRow* Type = It->GetType();
		// A bare foundation is walked over (D312): only a footprint the navmesh forbids keeps bodies out.
		if (!Type || !It->BlocksNavigation())
		{
			continue;
		}
		FFootprint& Footprint = Footprints.AddDefaulted_GetRef();
		Footprint.Centre = FVector2D(It->GetActorLocation());
		// The building's body, inset in its tiles (D324): a unit walks between two neighbours.
		Footprint.HalfExtent = UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5;
		FMath::SinCos(&Footprint.Sin, &Footprint.Cos, FMath::DegreesToRadians(It->GetActorRotation().Yaw));
		// The bounds of the turned rectangle, widened.
		const FVector2D Half(FMath::Abs(Footprint.Cos) * Footprint.HalfExtent.X + FMath::Abs(Footprint.Sin) * Footprint.HalfExtent.Y,
			FMath::Abs(Footprint.Sin) * Footprint.HalfExtent.X + FMath::Abs(Footprint.Cos) * Footprint.HalfExtent.Y);
		const FVector2D Margin(KeepOutMaxRadius);
		Footprint.Reach = FBox2D(Footprint.Centre - Half - Margin, Footprint.Centre + Half + Margin);
	}
	if (Footprints.IsEmpty())
	{
		return;
	}
	// The grid over the footprints' reaches: a unit far from every building costs one box test.
	GridBounds = FBox2D(ForceInit);
	for (const FFootprint& Footprint : Footprints)
	{
		GridBounds += Footprint.Reach;
	}
	double CellSize = KeepOutCellSize;
	const FVector2D Extent = GridBounds.GetSize();
	while (FMath::CeilToDouble(Extent.X / CellSize) * FMath::CeilToDouble(Extent.Y / CellSize) > KeepOutMaxCells)
	{
		CellSize *= 2.;
	}
	GridSize = FIntPoint(FMath::Max(1, FMath::CeilToInt32(Extent.X / CellSize)), FMath::Max(1, FMath::CeilToInt32(Extent.Y / CellSize)));
	const auto GetCell = [this, CellSize](const FVector2D& Point)
	{
		return FIntPoint(FMath::Clamp(FMath::FloorToInt32((Point.X - GridBounds.Min.X) / CellSize), 0, GridSize.X - 1),
			FMath::Clamp(FMath::FloorToInt32((Point.Y - GridBounds.Min.Y) / CellSize), 0, GridSize.Y - 1));
	};
	CellStarts.Init(0, GridSize.X * GridSize.Y + 1);
	for (int32 Pass = 0; Pass < 2; ++Pass)
	{
		// First the count of each cell, then the footprints in place.
		TArray<int32> Fill;
		if (Pass == 1)
		{
			for (int32 Cell = 1; Cell < CellStarts.Num(); ++Cell)
			{
				CellStarts[Cell] += CellStarts[Cell - 1];
			}
			CellFootprints.SetNumUninitialized(CellStarts.Last());
			Fill = CellStarts;
		}
		for (int32 Index = 0; Index < Footprints.Num(); ++Index)
		{
			const FIntPoint Min = GetCell(Footprints[Index].Reach.Min);
			const FIntPoint Max = GetCell(Footprints[Index].Reach.Max);
			for (int32 Y = Min.Y; Y <= Max.Y; ++Y)
			{
				for (int32 X = Min.X; X <= Max.X; ++X)
				{
					const int32 Cell = Y * GridSize.X + X;
					if (Pass == 0)
					{
						++CellStarts[Cell + 1];
					}
					else
					{
						CellFootprints[Fill[Cell]++] = Index;
					}
				}
			}
		}
	}

	// One thread: a light loop (S7, 2 000 units and 32 buildings: ~0.11 ms a step, as much as on the workers).
	EntityQuery.ForEachEntityChunk(Context, [this, GetCell](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TArrayView<FHOLMoveOrderFragment> Orders = Context.GetMutableFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		int64 Pushed = 0;
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& Transform = Transforms[EntityIt].GetMutableTransform();
			const FVector Location = Transform.GetLocation();
			FVector2D Position(Location.X, Location.Y);
			// The body against the wall at most: the navmesh keeps the walkers the clearance away (D310); a crowd pressed
			// against a building touches it. Not the clearance: on the turned buildings of S7 the navmesh's cells leave the
			// walkers' paths 11 cm from the wall, a keep-out at 25 cm would put them back at every step.
			const double Keep = AgentRadii[EntityIt].Radius + KeepOutSlack;
			bool bMoved = false;
			for (int32 Pass = 0; Pass < KeepOutPasses; ++Pass)
			{
				bool bMovedThisPass = false;
				if (!GridBounds.IsInsideOrOn(Position))
				{
					break;
				}
				const FIntPoint Cell = GetCell(Position);
				const int32 CellIndex = Cell.Y * GridSize.X + Cell.X;
				for (int32 Item = CellStarts[CellIndex]; Item < CellStarts[CellIndex + 1]; ++Item)
				{
					const FFootprint& Footprint = Footprints[CellFootprints[Item]];
					if (!Footprint.Reach.IsInsideOrOn(Position))
					{
						continue;
					}
					// In the footprint's frame.
					const FVector2D Offset = Position - Footprint.Centre;
					FVector2D Local(Offset.X * Footprint.Cos + Offset.Y * Footprint.Sin, -Offset.X * Footprint.Sin + Offset.Y * Footprint.Cos);
					const FVector2D& Half = Footprint.HalfExtent;
					const FVector2D Outside(FMath::Abs(Local.X) - Half.X, FMath::Abs(Local.Y) - Half.Y);
					if (Outside.X <= 0. && Outside.Y <= 0.)
					{
						// Its centre inside: out by the nearest side.
						if (Outside.X >= Outside.Y)
						{
							Local.X = (Local.X >= 0. ? 1. : -1.) * (Half.X + Keep);
						}
						else
						{
							Local.Y = (Local.Y >= 0. ? 1. : -1.) * (Half.Y + Keep);
						}
					}
					else
					{
						// Its centre outside, its body too near: away from the footprint's nearest point.
						const FVector2D Nearest(FMath::Clamp(Local.X, -Half.X, Half.X), FMath::Clamp(Local.Y, -Half.Y, Half.Y));
						const FVector2D Away = Local - Nearest;
						const double Distance = Away.Size();
						if (Distance >= Keep - UE_KINDA_SMALL_NUMBER)
						{
							continue;
						}
						Local = Nearest + Away / Distance * Keep;
					}
					Position = Footprint.Centre + FVector2D(Local.X * Footprint.Cos - Local.Y * Footprint.Sin, Local.X * Footprint.Sin + Local.Y * Footprint.Cos);
					bMovedThisPass = true;
				}
				bMoved |= bMovedThisPass;
				if (!bMovedThisPass)
				{
					break;
				}
			}
			if (bMoved)
			{
				UE_CLOG(KeepOutEnabled == 2, LogHOLSimulation, Display, TEXT("Keep-out: #%u %s/%s at (%.0f, %.0f) put back at (%.0f, %.0f), its slot (%.0f, %.0f)"),
					States[EntityIt].UnitId, *StaticEnum<EHOLUnitActivity>()->GetNameStringByValue(static_cast<int64>(States[EntityIt].Activity)),
					*StaticEnum<EHOLMovePhase>()->GetNameStringByValue(static_cast<int64>(Orders[EntityIt].Phase)), Location.X, Location.Y,
					Position.X, Position.Y, Orders[EntityIt].Slot.X, Orders[EntityIt].Slot.Y);
				Transform.SetLocation(FVector(Position.X, Position.Y, Location.Z));
				++Pushed;
			}
			// A walker put back step after step: its way crosses the building, the order processor finds it a new path.
			FHOLMoveOrderFragment& Order = Orders[EntityIt];
			if (bMoved && (Order.Phase == EHOLMovePhase::Path || Order.Phase == EHOLMovePhase::FinalLeg))
			{
				Order.BuildingPushes = static_cast<uint8>(FMath::Min<int32>(Order.BuildingPushes + 1, MAX_uint8));
			}
			else if (Order.BuildingPushes != 0)
			{
				Order.BuildingPushes = 0;
			}
		}
		if (Pushed > 0)
		{
			NumPushedOut += Pushed;
		}
	});
}

UHOLInstantOrientationProcessor::UHOLInstantOrientationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the units moved and were pushed apart.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitSeparationProcessor"));
}

void UHOLInstantOrientationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLInstantOrientationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FMassVelocityFragment> Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			// The walk's velocity (a standing unit's is zero, D290), not the step's move: a push does not turn it.
			const FVector& Velocity = Velocities[EntityIt].Value;
			if (Velocity.SizeSquared2D() > FMath::Square(MinFacingSpeed))
			{
				Transforms[EntityIt].GetMutableTransform().SetRotation(FQuat(FVector::UpVector, FMath::Atan2(Velocity.Y, Velocity.X)));
			}
			// Standing at the end of a formation move (J7a.4): turned towards the front of its rows, at once as a walker.
			else if (!Orders.IsEmpty() && Orders[EntityIt].bFacesFront && Orders[EntityIt].Phase == EHOLMovePhase::None)
			{
				Transforms[EntityIt].GetMutableTransform().SetRotation(FQuat(FVector::UpVector, FMath::DegreesToRadians(Orders[EntityIt].FrontYaw)));
			}
		}
	});
}
