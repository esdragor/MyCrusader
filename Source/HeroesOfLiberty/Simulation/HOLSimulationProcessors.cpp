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
	const uint32 FormUpSerial, const uint8 FormUpKind) const
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
				if (Cells[Other] != Cell || Entities[Other] == Entity || CanPassThrough(FormUpSerial, FormUpKind, FormUpSerials[Other], FormUpKinds[Other]))
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
					GetPassThroughSerial(Order), GetPassThroughKind(Order));
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

	float SeparationSlop = 0.f;
	FAutoConsoleVariableRef CVarSeparationSlop(TEXT("hol.Units.SeparationSlop"), SeparationSlop,
		TEXT("Overlap (cm) of two bodies the separation leaves (D294): only what goes beyond is undone"));

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

void UHOLUnitSeparationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

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
			MaxRadius = FMath::Max(MaxRadius, AgentRadii[EntityIt].Radius);
			bAnyWalking |= bWalking;
		}
	});
	if (!bAnyWalking)
	{
		return;
	}

	// Cells as wide as the largest pair of bodies, whatever the map's size: the overlaps of a unit are all in the 3 x 3
	// cells around it. The cells go into a fixed table by a hash; a bucket shared by two cells keeps each unit's own.
	const double CellSize = 2.0 * MaxRadius;
	const auto BucketOf = [](const FIntPoint& Cell)
	{
		return static_cast<int32>((static_cast<uint32>(Cell.X) * 73856093u ^ static_cast<uint32>(Cell.Y) * 19349663u) & (SeparationBuckets - 1));
	};
	CellHeads.Init(INDEX_NONE, SeparationBuckets);
	NextInCell.SetNumUninitialized(Positions.Num());
	Cells.SetNumUninitialized(Positions.Num());
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		Cells[Index] = FIntPoint(FMath::FloorToInt32(Positions[Index].X / CellSize), FMath::FloorToInt32(Positions[Index].Y / CellSize));
		int32& Head = CellHeads[BucketOf(Cells[Index])];
		NextInCell[Index] = Head;
		Head = Index;
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
	for (int32 Round = 0; Round < SeparationRounds; ++Round)
	{
		Looked.Init(false, Positions.Num());
		for (int32 Next = 0; Next < ToLook.Num(); ++Next)
		{
			const int32 Index = ToLook[Next];
			Looked[Index] = true;
			for (int32 DY = -1; DY <= 1; ++DY)
			{
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					const FIntPoint Cell(Cells[Index].X + DX, Cells[Index].Y + DY);
					for (int32 Other = CellHeads[BucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
					{
						// A pair is undone once a round, by the first of the two to look; another cell of the same bucket is not this one.
						if (Other == Index || Looked[Other] || Ghosts[Other] || Cells[Other] != Cell)
						{
							continue;
						}
						// Two units of one order forming up in different kinds of rows, of one block's re-forming, or a
						// straggler walking into its slot (J7a.4, J7b note 1: CanPassThrough): through each other, the ways to the
						// new rows cross (three bodies pressed together, each walking into the next, held each other still).
						// Two forming up in one kind of row keep apart (the user's trial of playtest note 1 of J7b).
						if (CanPassThrough(FormUpSerials[Index], FormUpKinds[Index], FormUpSerials[Other], FormUpKinds[Other]))
						{
							continue;
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
							continue;
						}
						const double Distance = FMath::Sqrt(DistanceSquared);
						// Two units on the same point: apart along a direction of their own, the same at every run.
						const FVector2D Normal = Distance > UE_KINDA_SMALL_NUMBER ? Delta / Distance
							: FVector2D(FMath::Cos(Index * 2.399), FMath::Sin(Index * 2.399));
						// Two units of one player take half each: a walker pushes a standing unit out of its way as it would push a
						// walker. Two enemies never push each other back (the user's trial of J7b): a walker stops against a
						// standing enemy, which stays where it is; the one that looks is a walker or pushed this step, so it moves.
						const double Overlap = MinDistance - Distance;
						const double Share = HOLCombat::GetOverlapShare(Owners[Index] != Owners[Other], Walking[Index], Walking[Other]);
						Corrections[Index] += Normal * (Overlap * Share);
						if (Share >= 1.)
						{
							continue;
						}
						Corrections[Other] -= Normal * (Overlap * (1. - Share));
						if (!Queued[Other])
						{
							Queued[Other] = true;
							ToLook.Add(Other);
						}
					}
				}
			}
		}
	}

	SeparationRoundsSeconds += FPlatformTime::Seconds() - GridSeconds;

	int32 Index = 0;
	EntityQuery.ForEachEntityChunk(Context, [this, &Index](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt, ++Index)
		{
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
