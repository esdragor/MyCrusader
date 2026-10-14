// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSimulationProcessors.h"

#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Misc/ScopeExit.h"
#include "Simulation/HOLSimulationFragments.h"

namespace UE::HeroesOfLiberty::Simulation
{
	/** Distance (cm) to its slot under which a unit of the final leg leaves the steering and walks straight to it (D290). */
	constexpr double StraightToSlotDistance = 300.0;

	/** Buckets of the look-ahead's cell table (a power of two), and the width (cm) of its cells. */
	constexpr int32 LookAheadBuckets = 4096;
	constexpr double LookAheadCellSize = 250.0;
	/** Angle (degrees) between two ways a walker tries around a body ahead (D293). */
	constexpr double AvoidAngleStep = 10.0;
	/** Part of the bodies' radii the look-ahead keeps clear: a graze is left to the separation, not turned aside for. */
	constexpr double AvoidRadiusScale = 0.95;

	int32 AvoidEnabled = 1;
	FAutoConsoleVariableRef CVarAvoid(TEXT("hol.Units.Avoid"), AvoidEnabled,
		TEXT("1: a walker turns aside from the bodies it would meet within hol.Units.AvoidTime, and pushes only when no way within hol.Units.AvoidMaxAngle is clear (D293); 2: the same, and its average cost logged every 200 steps; 0: off, for A / B measures"));
	float AvoidTime = 0.5f;
	FAutoConsoleVariableRef CVarAvoidTime(TEXT("hol.Units.AvoidTime"), AvoidTime,
		TEXT("Seconds a walker looks ahead for bodies on its way (D293)"));
	float AvoidMaxAngle = 60.f;
	FAutoConsoleVariableRef CVarAvoidMaxAngle(TEXT("hol.Units.AvoidMaxAngle"), AvoidMaxAngle,
		TEXT("Largest turn (degrees) a walker makes around a body ahead; beyond, the detour is too long and it pushes its way (D293)"));

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
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

bool UHOLFullSpeedProcessor::BuildLookAhead(FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	Entities.Reset();
	Positions.Reset();
	Velocities.Reset();
	Radii.Reset();
	MaxRadius = 1.f;
	MaxSpeed = 0.f;
	bool bAnyWalking = false;
	EntityQuery.ForEachEntityChunk(Context, [this, &bAnyWalking](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> UnitVelocities = Context.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			const FVector2D Velocity(UnitVelocities[EntityIt].Value.X, UnitVelocities[EntityIt].Value.Y);
			Entities.Add(Context.GetEntity(EntityIt));
			Positions.Add(FVector2D(Location.X, Location.Y));
			Velocities.Add(Velocity);
			Radii.Add(AgentRadii[EntityIt].Radius);
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

FVector UHOLFullSpeedProcessor::AvoidBodies(const FMassEntityHandle Entity, const FVector& Location, const float Radius, const FVector& Direction, const float Speed, const double Horizon) const
{
	using namespace UE::HeroesOfLiberty::Simulation;

	// The bodies that could be met within the horizon, whatever way this one takes: in the reach of both speeds, and not
	// behind it (no turn reaches them; one catching up from behind is its own to avoid).
	const FVector2D Position(Location.X, Location.Y);
	const FVector2D Ahead(Direction.X, Direction.Y);
	const double Reach = (Speed + MaxSpeed) * Horizon + Radius + MaxRadius;
	const int32 CellReach = FMath::CeilToInt32(Reach / LookAheadCellSize);
	const FIntPoint Center(FMath::FloorToInt32(Position.X / LookAheadCellSize), FMath::FloorToInt32(Position.Y / LookAheadCellSize));
	TArray<int32, TInlineAllocator<64>> Near;
	for (int32 DY = -CellReach; DY <= CellReach; ++DY)
	{
		for (int32 DX = -CellReach; DX <= CellReach; ++DX)
		{
			const FIntPoint Cell(Center.X + DX, Center.Y + DY);
			for (int32 Other = CellHeads[LookAheadBucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
			{
				if (Cells[Other] != Cell || Entities[Other] == Entity)
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
			int32 Unused = INDEX_NONE;
			if (FirstContact(Way, Unused) >= Horizon)
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

	EntityQuery.ParallelForEachEntityChunk(Context, [this, bLookAhead, LookAheadTime](FMassExecutionContext& Context)
	{
		// As UMassApplyMovementProcessor, which moves the unit by this velocity next.
		const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
		const TArrayView<FMassDesiredMovementFragment> Movements = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();

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
			if (Order.Phase == EHOLMovePhase::FinalLeg && SlotDistance <= Speed * DeltaTime)
			{
				// The last step lands on the slot: the order processor stands the unit there at the next step.
				Velocity = ToSlot / DeltaTime;
				continue;
			}
			FVector Direction;
			if (Order.Phase == EHOLMovePhase::FinalLeg && SlotDistance <= StraightToSlotDistance)
			{
				// Near its slot, straight to it: the steering turns over ~0.3 s, at full speed it would circle the slot.
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
			if (bLookAhead)
			{
				// Around the bodies on its way (D293), those beyond its slot aside: Mass' avoidance runs one step out of 6
				// (D220) and turns too late at full speed, so the walker pushed through them.
				const double Horizon = Order.Phase == EHOLMovePhase::FinalLeg ? FMath::Min(LookAheadTime, SlotDistance / Speed) : LookAheadTime;
				Direction = AvoidBodies(Context.GetEntity(EntityIt), Location, AgentRadii[EntityIt].Radius, Direction, Speed, Horizon);
			}
			Velocity = Direction * Speed;
		}
	});
}

namespace UE::HeroesOfLiberty::Simulation
{
	/** Buckets of the separation's cell table (a power of two): a few per unit at 2 000 units, 32 KB to clear each step. */
	constexpr int32 SeparationBuckets = 8192;
	/** Rounds of the separation a step (D292). */
	constexpr int32 SeparationRounds = 2;

	int32 SeparationEnabled = 1;
	FAutoConsoleVariableRef CVarSeparation(TEXT("hol.Units.Separation"), SeparationEnabled,
		TEXT("1: no body overlaps another once the units have moved (D292); 2: the same, and its average cost logged every 200 steps; 0: off, for A / B measures"));

	/** Time spent and steps run since the last log (hol.Units.Separation 2). */
	double SeparationSeconds = 0.;
	int32 SeparationRuns = 0;
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
				UE_LOG(LogHOLSimulation, Display, TEXT("Separation: %.3f ms per step on average (%d units)"), SeparationSeconds * 1000. / SeparationRuns, Positions.Num());
				SeparationSeconds = 0.;
				SeparationRuns = 0;
			}
		}
	};
	Positions.Reset();
	Radii.Reset();
	Walking.Reset();
	float MaxRadius = 1.f;
	bool bAnyWalking = false;
	EntityQuery.ForEachEntityChunk(Context, [this, &MaxRadius, &bAnyWalking](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			Positions.Add(FVector2D(Location.X, Location.Y));
			Radii.Add(AgentRadii[EntityIt].Radius);
			const bool bWalking = MoveTargets[EntityIt].GetCurrentAction() == EMassMovementAction::Move;
			Walking.Add(bWalking);
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
						if (Other == Index || Looked[Other] || Cells[Other] != Cell)
						{
							continue;
						}
						const FVector2D Delta = Corrections[Index] - Corrections[Other];
						const double MinDistance = Radii[Index] + Radii[Other];
						const double DistanceSquared = Delta.SizeSquared();
						if (DistanceSquared >= FMath::Square(MinDistance))
						{
							continue;
						}
						const double Distance = FMath::Sqrt(DistanceSquared);
						// Two units on the same point: apart along a direction of their own, the same at every run.
						const FVector2D Normal = Distance > UE_KINDA_SMALL_NUMBER ? Delta / Distance
							: FVector2D(FMath::Cos(Index * 2.399), FMath::Sin(Index * 2.399));
						// Each takes half: a walker pushes a standing unit out of its way as it would push a walker.
						const double Overlap = MinDistance - Distance;
						Corrections[Index] += Normal * (Overlap * 0.5);
						Corrections[Other] -= Normal * (Overlap * 0.5);
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
