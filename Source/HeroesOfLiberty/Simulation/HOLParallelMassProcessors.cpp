// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLParallelMassProcessors.h"

#include "Async/ParallelFor.h"
#include "Avoidance/MassAvoidanceFragments.h"
#include "HAL/IConsoleManager.h"
#include "MassArchetypeTypes.h"
#include "MassCommonFragments.h"
#include "MassEntityQuery.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassNavigationSubsystem.h"
#include "MassNavigationUtils.h"
#include "MassSimulationLOD.h"
#include "RandomSequence.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Steering/MassSteeringFragments.h"

namespace UE::HeroesOfLiberty::Simulation
{
	int32 ParallelMass = 1;
	FAutoConsoleVariableRef CVarParallelMass(TEXT("hol.Simulation.ParallelMass"), ParallelMass,
		TEXT("1: Mass' moving avoidance, steering and orientation run their chunks in parallel (D302); 0: one after the other, as Mass does, for A / B measures"));

	/** Runs Execute (a Mass processor's own), each of its queries' chunks in parallel; see HOLParallelMassProcessors.h. */
	void ExecuteChunksInParallel(FMassExecutionContext& Context, const TFunctionRef<void(FMassExecutionContext&)> Execute)
	{
		if (ParallelMass == 0)
		{
			Execute(Context);
			return;
		}

		// Each query's per-chunk function, in the order Execute runs the queries (the same for every chunk of a run).
		struct FQueryRun
		{
			FMassEntityQuery* Query = nullptr;
			FMassExecuteFunction Function;
		};
		TArray<FQueryRun, TInlineAllocator<2>> Runs;
		{
			FMassExecutionContext CollectContext(Context);
			CollectContext.SetChunkQueueFunc([&Runs](FMassChunkProcessingQueueParams&& Params)
			{
				if (Params.SourceQuery && Params.ExecuteFunction && (Runs.IsEmpty() || Runs.Last().Query != Params.SourceQuery))
				{
					Runs.Add({ Params.SourceQuery, *Params.ExecuteFunction });
				}
			});
			Execute(CollectContext);
		}
		for (FQueryRun& Run : Runs)
		{
			Run.Query->ParallelForEachEntityChunk(Context, Run.Function);
		}
	}
}

UHOLParallelMovingAvoidanceProcessor::UHOLParallelMovingAvoidanceProcessor()
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
}

void UHOLParallelMovingAvoidanceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UE::HeroesOfLiberty::Simulation::ExecuteChunksInParallel(Context, [this, &EntityManager](FMassExecutionContext& RunContext)
	{
		Super::Execute(EntityManager, RunContext);
	});
}

namespace UE::HeroesOfLiberty::Simulation
{
	int32 DeterministicSteering = 1;
	FAutoConsoleVariableRef CVarDeterministicSteering(TEXT("hol.Simulation.DeterministicSteering"), DeterministicSteering,
		TEXT("1: Mass' steering with its two global random draws replaced by a hash of the unit, the step and the call site, the same results in every run (J7c.4); 0: Mass' own Execute, for A / B measures"));

	/** A float in [Min, Max] from the unit, the fixed step and the call site, in place of FMath::RandRange (global state). */
	static float DeterministicRandRange(const uint32 UnitKey, const int64 StepIndex, const uint32 CallSite, const float Min, const float Max)
	{
		// 64-bit mix (splitmix64's finaliser) of the three, so that close units, steps and call sites draw unrelated values.
		uint64 Hash = (static_cast<uint64>(UnitKey) << 32) ^ static_cast<uint64>(StepIndex) ^ (static_cast<uint64>(CallSite) * 0x9E3779B97F4A7C15ull);
		Hash ^= Hash >> 30;
		Hash *= 0xBF58476D1CE4E5B9ull;
		Hash ^= Hash >> 27;
		Hash *= 0x94D049BB133111EBull;
		Hash ^= Hash >> 31;
		return UE::RandomSequence::FRandRange(static_cast<int32>(static_cast<uint32>(Hash)), Min, Max);
	}

	// Copied verbatim from MassSteeringProcessors.cpp:21-52 (UE::MassNavigation, file-static there).
	static FVector::FReal CalcDirectionalSpeedScale(const FVector ForwardDirection, const FVector SteerDirection)
	{
		// @todo: make these configurable
		constexpr FVector::FReal ForwardSpeedScale = 1.;
		constexpr FVector::FReal BackwardSpeedScale = 0.25;
		constexpr FVector::FReal SideSpeedScale = 0.5;

		const FVector LeftDirection = FVector::CrossProduct(ForwardDirection, FVector::UpVector);
		const FVector::FReal DirX = FVector::DotProduct(LeftDirection, SteerDirection);
		const FVector::FReal DirY = FVector::DotProduct(ForwardDirection, SteerDirection);

		// Calculate intersection between a direction vector and ellipse, where A & B are the size of the ellipse.
		// The direction vector is starting from the center of the ellipse.
		constexpr FVector::FReal SideA = SideSpeedScale;
		const FVector::FReal SideB = DirY > 0. ? ForwardSpeedScale : BackwardSpeedScale;
		const FVector::FReal Disc = FMath::Square(SideA) * FMath::Square(DirY) + FMath::Square(SideB) * FMath::Square(DirX);
		const FVector::FReal Speed = (Disc > SMALL_NUMBER) ? (SideA * SideB / FMath::Sqrt(Disc)) : 0.;

		return Speed;
	}

	/** Speed envelope when approaching a point. NormalizedDistance in range [0..1] */
	static FVector::FReal ArrivalSpeedEnvelope(const FVector::FReal NormalizedDistance)
	{
		return FMath::Sqrt(NormalizedDistance);
	}
}

UHOLParallelSteerToMoveTargetProcessor::UHOLParallelSteerToMoveTargetProcessor()
	: SteerQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
}

void UHOLParallelSteerToMoveTargetProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager); // Mass' own query, for hol.Simulation.DeterministicSteering 0

	// Copied from UMassSteerToMoveTargetProcessor::ConfigureQueries (MassSteeringProcessors.cpp:66-87).
	SteerQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	SteerQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	SteerQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	SteerQuery.AddRequirement<FMassSteeringFragment>(EMassFragmentAccess::ReadWrite);
	SteerQuery.AddRequirement<FMassStandingSteeringFragment>(EMassFragmentAccess::ReadWrite);
	SteerQuery.AddRequirement<FMassGhostLocationFragment>(EMassFragmentAccess::ReadWrite);
	SteerQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	SteerQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	SteerQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	SteerQuery.AddConstSharedRequirement<FMassMovingSteeringParameters>(EMassFragmentPresence::All);
	SteerQuery.AddConstSharedRequirement<FMassStandingSteeringParameters>(EMassFragmentPresence::All);
	SteerQuery.AddTagRequirement<FMassSteerToMoveTargetPreventSlowdownTag>(EMassFragmentPresence::Optional);
	SteerQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
	SteerQuery.AddTagRequirement<FMassCodeDrivenMovementTag>(EMassFragmentPresence::Optional);
	// The seed of the draws: the UnitId, stable, never recycled (D223); Entity.Index on an entity without it.
	SteerQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

void UHOLParallelSteerToMoveTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	if (DeterministicSteering == 0)
	{
		ExecuteChunksInParallel(Context, [this, &EntityManager](FMassExecutionContext& RunContext)
		{
			Super::Execute(EntityManager, RunContext);
		});
		return;
	}

	const UWorld* World = GetWorld();
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	const int64 StepIndex = Simulation ? Simulation->GetStepIndex() : 0;

	// Each unit only writes its own fragments and reads nothing another unit's run writes: chunks in parallel (D302).
	const FMassExecuteFunction ChunkFunction = [StepIndex](FMassExecutionContext& Context)
	{
		// Copied from MassSteeringProcessors.cpp:93-433 (the per-chunk lambda of UMassSteerToMoveTargetProcessor::Execute),
		// the same expressions in the same order, but for: the visual logger's debug blocks (WITH_MASSGAMEPLAY_DEBUG,
		// no effect on the simulation) left out, and the two FMath::RandRange (lines 272 and 304) drawn by
		// DeterministicRandRange.
		const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FMassDesiredMovementFragment> MovementList = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const TArrayView<FMassForceFragment> ForceList = Context.GetMutableFragmentView<FMassForceFragment>();
		const TArrayView<FMassSteeringFragment> SteeringList = Context.GetMutableFragmentView<FMassSteeringFragment>();
		const TArrayView<FMassStandingSteeringFragment> StandingSteeringList = Context.GetMutableFragmentView<FMassStandingSteeringFragment>();
		const TArrayView<FMassGhostLocationFragment> GhostList = Context.GetMutableFragmentView<FMassGhostLocationFragment>();
		const TConstArrayView<FHOLUnitStateFragment> UnitStateList = Context.GetFragmentView<FHOLUnitStateFragment>();
		const FMassMovementParameters& MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();
		const FMassMovingSteeringParameters& MovingSteeringParams = Context.GetConstSharedFragment<FMassMovingSteeringParameters>();
		const FMassStandingSteeringParameters& StandingSteeringParams = Context.GetConstSharedFragment<FMassStandingSteeringParameters>();
		const bool bIsCodeDriven = Context.DoesArchetypeHaveTag<FMassCodeDrivenMovementTag>();
		const bool bPreventSteeringSlowdown = Context.DoesArchetypeHaveTag<FMassSteerToMoveTargetPreventSlowdownTag>();

		const FVector::FReal SteerK = 1. / MovingSteeringParams.ReactionTime;
		const float DeltaTime = Context.GetDeltaTimeSeconds();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FTransformFragment& TransformFragment = TransformList[EntityIt];
			const FMassVelocityFragment& VelocityFragment = VelocityList[EntityIt];
			FMassSteeringFragment& Steering = SteeringList[EntityIt];
			FMassStandingSteeringFragment& StandingSteering = StandingSteeringList[EntityIt];
			FMassGhostLocationFragment& Ghost = GhostList[EntityIt];
			FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIt];
			FMassForceFragment& Force = ForceList[EntityIt];
			FMassDesiredMovementFragment& DesiredMovement = MovementList[EntityIt];
			const FMassEntityHandle Entity = Context.GetEntity(EntityIt);
			const uint32 UnitKey = UnitStateList.IsEmpty() ? static_cast<uint32>(Entity.Index) : UnitStateList[EntityIt].UnitId;

			const FTransform& Transform = TransformFragment.GetTransform();

			FVector::FReal SlowdownScale = 1;

			// Calculate velocity for steering.
			const FVector CurrentLocation = Transform.GetLocation();
			const FVector CurrentForward = Transform.GetRotation().GetForwardVector();

			const FVector::FReal LookAheadDistance = FMath::Max(1.0f, MovingSteeringParams.LookAheadTime * MoveTarget.DesiredSpeed.Get());

			if (MoveTarget.GetCurrentAction() == EMassMovementAction::Move)
			{
				// Tune down avoidance and speed when arriving at goal.
				FVector::FReal ArrivalFade = 1.;
				if (MoveTarget.IntentAtGoal == EMassMovementAction::Stand)
				{
					ArrivalFade = FMath::Clamp(MoveTarget.DistanceToGoal / LookAheadDistance, 0., 1.);
				}
				const FVector::FReal SteeringPredictionDistance = LookAheadDistance * ArrivalFade;

				// Steer towards and along the move target.
				const FVector TargetSide = FVector::CrossProduct(MoveTarget.Forward, FVector::UpVector);
				const FVector Delta = CurrentLocation - MoveTarget.Center;

				const FVector::FReal ForwardOffset = FVector::DotProduct(MoveTarget.Forward, Delta);

				// Calculate steering direction. When far away from the line defined by TargetPosition and TargetTangent,
				// the steering direction is towards the line, the close we get, the more it aligns with the line.
				const FVector::FReal SidewaysOffset = FVector::DotProduct(TargetSide, Delta);
				const FVector::FReal SteerForward = FMath::Sqrt(FMath::Max(0., FMath::Square(SteeringPredictionDistance) - FMath::Square(SidewaysOffset)));

				// The Max() here makes the steering directions behind the TargetPosition to steer towards it directly.
				FVector SteerTarget = MoveTarget.Center + MoveTarget.Forward * FMath::Clamp(ForwardOffset + SteerForward, 0., SteeringPredictionDistance);

				FVector SteerDirection = SteerTarget - CurrentLocation;
				SteerDirection.Z = 0.;
				const FVector::FReal DistanceToSteerTarget = SteerDirection.Length();
				if (DistanceToSteerTarget > KINDA_SMALL_NUMBER)
				{
					SteerDirection *= 1. / DistanceToSteerTarget;
				}

				FVector::FReal DesiredSpeed = MoveTarget.DesiredSpeed.Get();

				// When being animation driven, animation has authority over the movement, so it might be useful to disable
				// this catchup mechanic to avoid subtle speed variations affecting animation.
				if (MovingSteeringParams.bAllowSpeedVariance)
				{
					const FVector::FReal DirSpeedScale = CalcDirectionalSpeedScale(CurrentForward, SteerDirection);
					DesiredSpeed *= DirSpeedScale;

					// Control speed based relation to the forward axis of the move target.
					FVector::FReal CatchupDesiredSpeed = DesiredSpeed;
					if (ForwardOffset < 0.)
					{
						// Falling behind, catch up
						const FVector::FReal T = FMath::Min(-ForwardOffset / LookAheadDistance, 1.);
						CatchupDesiredSpeed = FMath::Lerp(DesiredSpeed, MovementParams.MaxSpeed, T);
					}
					else if (ForwardOffset > 0.)
					{
						// Ahead, slow down.
						const FVector::FReal T = FMath::Min(ForwardOffset / LookAheadDistance, 1.);
						CatchupDesiredSpeed = FMath::Lerp(DesiredSpeed, DesiredSpeed * 0., 1. - FMath::Square(1. - T));
					}

					// Control speed based on distance to move target. This allows to catch up even if speed above reaches zero.
					const FVector::FReal DeviantSpeed = FMath::Min(FMath::Abs(SidewaysOffset) / LookAheadDistance, 1.) * DesiredSpeed;

					DesiredSpeed = FMath::Max(CatchupDesiredSpeed, DeviantSpeed);
				}

				// Slow down towards the end of path.
				if (MoveTarget.IntentAtGoal == EMassMovementAction::Stand)
				{
					const FVector::FReal NormalizedDistanceToSteerTarget = FMath::Clamp(DistanceToSteerTarget / LookAheadDistance, 0., 1.);
					DesiredSpeed *= ArrivalSpeedEnvelope(FMath::Max(ArrivalFade, NormalizedDistanceToSteerTarget));
				}

				constexpr FVector::FReal FallingBehindScale = 0.8;
				if (MoveTarget.EntityDistanceToGoal != FMassMoveTargetFragment::UnsetDistance)
				{
					MoveTarget.bSteeringFallingBehind = (MoveTarget.EntityDistanceToGoal - MoveTarget.DistanceToGoal) > LookAheadDistance * FallingBehindScale;
				}
				else
				{
					// If EntityDistanceToGoal is not available, use ForwardOffset.
					MoveTarget.bSteeringFallingBehind = ForwardOffset < -LookAheadDistance * FallingBehindScale;
				}

				// @todo: This current completely overrides steering, we probably should have one processor that resets the steering at the beginning of the frame.
				Steering.DesiredVelocity = SteerDirection * DesiredSpeed;

				// Important: we want steering force to be stable against noisy velocity, so we use
				// the difference between current desired velocity and target desired velocity.
				// We dont want to read the actual agent velocity directly since this can create a feedback loop for animated characters
				Force.Value = SteerK * (Steering.DesiredVelocity - DesiredMovement.DesiredVelocity); // Goal force

				// Apply only if DesiredVelocity is above or equal to DefaultDesiredSpeed, else we want the steering force to slow us down to the desired speed.
				if (bPreventSteeringSlowdown && DesiredMovement.DesiredVelocity.Size() <= MovementParams.DefaultDesiredSpeed)
				{
					const FVector::FReal DistanceToMoveTarget = FVector::Dist(CurrentLocation, MoveTarget.Center);
					const FVector::FReal ExtraDistance = FMath::Max(0, DistanceToMoveTarget - (1.1*LookAheadDistance)); // Multiplied by 1.1 to add some buffer
					SlowdownScale = 1 - FMath::Clamp(ExtraDistance/MovingSteeringParams.SteeringPreventSlowdownAttenuationDistance, 0, 1);

					// Make sure the steering force does not reduce DesiredMovement.DesiredVelocity.
					const FVector DesiredVelocityNormal = DesiredMovement.DesiredVelocity.GetSafeNormal();
					const FVector::FReal ProjectedForceSize = Force.Value.Dot(DesiredVelocityNormal);
					if (ProjectedForceSize < 0)
					{
						// The force is slowing the entity, avoid that.
						Force.Value += SlowdownScale * FMath::Abs(ProjectedForceSize) * DesiredVelocityNormal;
					}
				}
			}
			else if (MoveTarget.GetCurrentAction() == EMassMovementAction::Stand)
			{
				// Calculate unique target move threshold so that different agents react a bit differently.
				const FVector::FReal PerEntityScale = UE::RandomSequence::FRand(Entity.Index);
				const FVector::FReal TargetMoveThreshold = StandingSteeringParams.TargetMoveThreshold * (1. - StandingSteeringParams.TargetMoveThresholdVariance + PerEntityScale * StandingSteeringParams.TargetMoveThresholdVariance * 2.);

				if (Ghost.LastSeenActionID != MoveTarget.GetCurrentActionID())
				{
					// Reset when action changes. @todo: should reset only when move->stand?
					Ghost.Location = MoveTarget.Center;
					Ghost.Velocity = FVector::ZeroVector;
					Ghost.LastSeenActionID = MoveTarget.GetCurrentActionID();

					StandingSteering.TargetLocation = MoveTarget.Center;
					StandingSteering.TrackedTargetSpeed = 0.0f;
					StandingSteering.bIsUpdatingTarget = false;
					// Was FMath::RandRange (global random state, line 272).
					StandingSteering.TargetSelectionCooldown = StandingSteeringParams.TargetSelectionCooldown * DeterministicRandRange(UnitKey, StepIndex, 1, 1.f - StandingSteeringParams.TargetSelectionCooldownVariance, 1.f + StandingSteeringParams.TargetSelectionCooldownVariance);
					StandingSteering.bEnteredFromMoveAction = MoveTarget.GetPreviousAction() == EMassMovementAction::Move;
				}

				StandingSteering.TargetSelectionCooldown = FMath::Max(0.0f, StandingSteering.TargetSelectionCooldown - DeltaTime);

				if (!StandingSteering.bIsUpdatingTarget)
				{
					// Update the move target if enough time has passed and the target has moved.
					if (StandingSteering.TargetSelectionCooldown <= 0.0f
						&& FVector::DistSquared(StandingSteering.TargetLocation, Ghost.Location) > FMath::Square(TargetMoveThreshold))
					{
						StandingSteering.TargetLocation = Ghost.Location;
						StandingSteering.TrackedTargetSpeed = 0.0f;
						StandingSteering.bIsUpdatingTarget = true;
						StandingSteering.bEnteredFromMoveAction = false;
					}
				}
				else
				{
					// Updating target
					StandingSteering.TargetLocation = Ghost.Location;
					const FVector::FReal GhostSpeed = Ghost.Velocity.Length();

					if (GhostSpeed > (StandingSteering.TrackedTargetSpeed * StandingSteeringParams.TargetSpeedHysteresisScale))
					{
						const FVector::FReal TrackedTargetSpeed = FMath::Max(StandingSteering.TrackedTargetSpeed, GhostSpeed);
						StandingSteering.TrackedTargetSpeed = static_cast<float>(TrackedTargetSpeed);
					}
					else
					{
						// Speed is dropping, we have found the peak change, stop updating the target and start cooldown.
						// Was FMath::RandRange (global random state, line 304).
						StandingSteering.TargetSelectionCooldown = StandingSteeringParams.TargetSelectionCooldown * DeterministicRandRange(UnitKey, StepIndex, 2, 1.0f - StandingSteeringParams.TargetSelectionCooldownVariance, 1.0f + StandingSteeringParams.TargetSelectionCooldownVariance);
						StandingSteering.bIsUpdatingTarget = false;
					}
				}

				// Move directly towards the move target when standing.
				FVector SteerDirection = FVector::ZeroVector;
				FVector::FReal DesiredSpeed = 0.;

				FVector Delta = StandingSteering.TargetLocation - CurrentLocation;
				Delta.Z = 0.;
				const FVector::FReal Distance = Delta.Size();
				if (Distance > StandingSteeringParams.DeadZoneRadius)
				{
					SteerDirection = Delta / Distance;
					if (StandingSteering.bEnteredFromMoveAction)
					{
						// If the current steering target is from approaching a move target, use the same speed logic as movement to ensure smooth transition.
						const FVector::FReal Range = FMath::Max(1., LookAheadDistance - StandingSteeringParams.DeadZoneRadius);
						const FVector::FReal SpeedFade = FMath::Clamp((Distance - StandingSteeringParams.DeadZoneRadius) / Range, 0., 1.);
						DesiredSpeed = MoveTarget.DesiredSpeed.Get() * CalcDirectionalSpeedScale(CurrentForward, SteerDirection) * ArrivalSpeedEnvelope(SpeedFade);
					}
					else
					{
						const FVector::FReal Range = FMath::Max(1., LookAheadDistance - StandingSteeringParams.DeadZoneRadius);
						const FVector::FReal SpeedFade = FMath::Clamp((Distance - StandingSteeringParams.DeadZoneRadius) / Range, 0., 1.);
						// Not using the directional scaling so that the steps we take to avoid are done quickly, and the behavior is reactive.
						DesiredSpeed = MoveTarget.DesiredSpeed.Get() * ArrivalSpeedEnvelope(SpeedFade);
					}

					// @todo: This current completely overrides steering, we probably should have one processor that resets the steering at the beginning of the frame.
					Steering.DesiredVelocity = SteerDirection * DesiredSpeed;
					Force.Value = SteerK * (Steering.DesiredVelocity - DesiredMovement.DesiredVelocity); // Goal force
					Force.Value = Force.Value.GetClampedToMaxSize(MovementParams.MaxAcceleration);
				}
				else
				{
					// When reached destination, clamp small desired velocities to zero to avoid tiny drifting.
					if (DesiredMovement.DesiredVelocity.SquaredLength() < FMath::Square(StandingSteeringParams.LowSpeedThreshold))
					{
						DesiredMovement.DesiredVelocity = FVector::ZeroVector;
						Force.Value = FVector::ZeroVector;
					}
				}

				MoveTarget.bSteeringFallingBehind = false;
			}
			else if (MoveTarget.GetCurrentAction() == EMassMovementAction::Animate)
			{
				Steering.Reset();
				MoveTarget.bSteeringFallingBehind = false;
				Force.Value = FVector::ZeroVector;

				if (bIsCodeDriven)
				{
					// Stop all movement when animating.
					DesiredMovement.DesiredVelocity = FVector::ZeroVector;
				}
				else
				{
					// Animation driven, sync desired movement to animation
					Steering.DesiredVelocity = VelocityFragment.Value;
					DesiredMovement.DesiredVelocity = VelocityFragment.Value;
					if (!DesiredMovement.DesiredVelocity.IsNearlyZero())
					{
						DesiredMovement.DesiredFacing = FQuat(DesiredMovement.DesiredVelocity.Rotation());
					}
				}
			}
		}
	};

	if (ParallelMass == 0)
	{
		SteerQuery.ForEachEntityChunk(Context, ChunkFunction);
	}
	else
	{
		SteerQuery.ParallelForEachEntityChunk(Context, ChunkFunction);
	}
}

namespace UE::HeroesOfLiberty::Simulation
{
	int32 ParallelStandingAvoidance = 1;
	FAutoConsoleVariableRef CVarParallelStandingAvoidance(TEXT("hol.Simulation.ParallelStandingAvoidance"), ParallelStandingAvoidance,
		TEXT("1: Mass' standing avoidance finds each unit's neighbours in parallel, the same results (J7c.4, S3); 0: Mass' own Execute, for A / B measures"));

	// Copied verbatim from MassAvoidanceProcessors.cpp:49-112 (UE::MassAvoidance, file-static there): the neighbour
	// search must visit the same cells in the same order (unstable sort included) to find the same neighbours.
	constexpr int32 MaxExpectedAgentsPerCell = 6;
	constexpr int32 MinTouchingCellCount = 4;
	constexpr int32 MaxObstacleResults = MaxExpectedAgentsPerCell * MinTouchingCellCount;

	static void FindCloseObstacles(const FVector& Center, const FVector::FReal SearchRadius, const FNavigationObstacleHashGrid2D& AvoidanceObstacleGrid,
									TArray<FMassNavigationObstacleItem, TFixedAllocator<MaxObstacleResults>>& OutCloseEntities, const int32 MaxResults)
	{
		OutCloseEntities.Reset();
		const FVector Extent(SearchRadius, SearchRadius, 0.);
		const FBox QueryBox = FBox(Center - Extent, Center + Extent);

		struct FSortingCell
		{
			int32 X;
			int32 Y;
			int32 Level;
			FVector::FReal SqDist;
		};
		TArray<FSortingCell, TInlineAllocator<64>> Cells;
		const FVector QueryCenter = QueryBox.GetCenter();

		for (int32 Level = 0; Level < AvoidanceObstacleGrid.NumLevels; Level++)
		{
			const FVector::FReal CellSize = AvoidanceObstacleGrid.GetCellSize(Level);
			const FNavigationObstacleHashGrid2D::FCellRect Rect = AvoidanceObstacleGrid.CalcQueryBounds(QueryBox, Level);

			// Use int64 to prevent overflow when MaxX or MaxY are equal to the maximum value of int32.
			for (int64 Y = Rect.MinY; Y <= Rect.MaxY; Y++)
			{
				for (int64 X = Rect.MinX; X <= Rect.MaxX; X++)
				{
					const FVector::FReal CenterX = (X + 0.5) * CellSize;
					const FVector::FReal CenterY = (Y + 0.5) * CellSize;
					const FVector::FReal DX = CenterX - QueryCenter.X;
					const FVector::FReal DY = CenterY - QueryCenter.Y;
					const FVector::FReal SqDist = DX * DX + DY * DY;
					FSortingCell SortCell;
					SortCell.X = X;
					SortCell.Y = Y;
					SortCell.Level = Level;
					SortCell.SqDist = SqDist;
					Cells.Add(SortCell);
				}
			}
		}

		Cells.Sort([](const FSortingCell& A, const FSortingCell& B) { return A.SqDist < B.SqDist; });

		for (const FSortingCell& SortedCell : Cells)
		{
			if (const FNavigationObstacleHashGrid2D::FCell* Cell = AvoidanceObstacleGrid.FindCell(SortedCell.X, SortedCell.Y, SortedCell.Level))
			{
				const TSparseArray<FNavigationObstacleHashGrid2D::FItem>&  Items = AvoidanceObstacleGrid.GetItems();
				for (int32 Idx = Cell->First; Idx != INDEX_NONE; Idx = Items[Idx].Next)
				{
					OutCloseEntities.Add(Items[Idx].ID);
					if (OutCloseEntities.Num() >= MaxResults)
					{
						return;
					}
				}
			}
		}
	}

	constexpr int32 MaxCloseObstacleTreated = 6; // MassAvoidanceProcessors.cpp:1294

	/** A standing unit with a valid ghost, listed by phase A in Mass' visiting order. */
	struct FStandingUnit
	{
		FMassGhostLocationFragment* Ghost = nullptr;
		const FMassMoveTargetFragment* MoveTarget = nullptr;
		const FMassStandingAvoidanceParameters* StandingParams = nullptr;
		const FNavigationObstacleHashGrid2D* ObstacleGrid = nullptr;
		TConstArrayView<FMassEntityHandle> EntitiesToIgnore;
		FMassEntityHandle Entity;
		FVector AgentLocation = FVector::ZeroVector;
		FVector::FReal AgentRadius = 0.;
		float DeltaTime = 0.f;
		int32 NumCloseObstacles = 0; // filled by phase B, at most MaxCloseObstacleTreated
	};

	/** One of the (up to) 6 closest neighbours phase B keeps for a unit: only what this pass never writes. */
	struct FStandingNeighbour
	{
		FVector Location = FVector::ZeroVector;
		FVector Forward = FVector::ForwardVector;
		FVector::FReal OtherRadius = 0.;
		const FMassMoveTargetFragment* OtherMoveTarget = nullptr;
		const FMassGhostLocationFragment* OtherGhost = nullptr; // its Location is read live, by phase C
		bool bOtherHasGhost = false;
	};
}

UHOLParallelStandingAvoidanceProcessor::UHOLParallelStandingAvoidanceProcessor()
	: StandingQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
}

void UHOLParallelStandingAvoidanceProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	Super::ConfigureQueries(EntityManager); // Mass' own query, for hol.Simulation.ParallelStandingAvoidance 0

	// Copied from UMassStandingAvoidanceProcessor::ConfigureQueries (MassAvoidanceProcessors.cpp:1119-1135).
	StandingQuery.AddRequirement<FMassGhostLocationFragment>(EMassFragmentAccess::ReadWrite);
	StandingQuery.AddRequirement<FMassNavigationEdgesFragment>(EMassFragmentAccess::ReadOnly);
	StandingQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	StandingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	StandingQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	StandingQuery.AddRequirement<FMassAvoidanceEntitiesToIgnoreFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	StandingQuery.AddTagRequirement<FMassMediumLODTag>(EMassFragmentPresence::None);
	StandingQuery.AddTagRequirement<FMassLowLODTag>(EMassFragmentPresence::None);
	StandingQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
	StandingQuery.AddConstSharedRequirement<FMassStandingAvoidanceParameters>(EMassFragmentPresence::All);
	StandingQuery.AddSubsystemRequirement<UMassNavigationSubsystem>(EMassFragmentAccess::ReadOnly);

	// Indirect access: fragments read on neighbor entities returned by FindCloseObstacles
	StandingQuery.AddIndirectFragmentRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	StandingQuery.AddIndirectFragmentRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	StandingQuery.AddIndirectFragmentRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	StandingQuery.AddIndirectFragmentRequirement<FMassGhostLocationFragment>(EMassFragmentAccess::ReadOnly);
}

void UHOLParallelStandingAvoidanceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;
	if (ParallelStandingAvoidance == 0)
	{
		Super::Execute(EntityManager, Context);
		return;
	}
	QUICK_SCOPE_CYCLE_COUNTER(UHOLParallelStandingAvoidanceProcessor);

	// Mass' debug drawing and visual logging (WITH_MASSGAMEPLAY_DEBUG) are left out: they change no result.

	// Phase A (MassAvoidanceProcessors.cpp:1149-1217): the standing units with a valid ghost, in the order Mass' loop
	// visits them (the same query, so the same archetypes, chunks and entities). Nothing is written: Ghost.IsValid
	// reads LastSeenActionID, which this pass never writes (only Velocity and Location, in phase C).
	TArray<FStandingUnit> Units;
	StandingQuery.ForEachEntityChunk(Context, [&Units](FMassExecutionContext& Context)
	{
		const UMassNavigationSubsystem* NavigationSubsystem = Context.GetSubsystem<UMassNavigationSubsystem>();
		if (!NavigationSubsystem)
		{
			return;
		}
		const float DeltaTime = Context.GetDeltaTimeSeconds();

		const TArrayView<FMassGhostLocationFragment> GhostList = Context.GetMutableFragmentView<FMassGhostLocationFragment>();
		const TConstArrayView<FTransformFragment> LocationList = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> RadiusList = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassAvoidanceEntitiesToIgnoreFragment> EntitiesToIgnoreList = Context.GetFragmentView<FMassAvoidanceEntitiesToIgnoreFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargetList = Context.GetFragmentView<FMassMoveTargetFragment>();
		const FMassStandingAvoidanceParameters& StandingParams = Context.GetConstSharedFragment<FMassStandingAvoidanceParameters>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			// @todo: this check should eventually be part of the query.
			const FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIt];
			if (MoveTarget.GetCurrentAction() != EMassMovementAction::Stand)
			{
				continue;
			}

			FMassGhostLocationFragment& Ghost = GhostList[EntityIt];
			// Skip if the ghost is not valid for this movement action yet.
			if (Ghost.IsValid(MoveTarget.GetCurrentActionID()) == false)
			{
				continue;
			}

			FStandingUnit& Unit = Units.AddDefaulted_GetRef();
			Unit.Ghost = &Ghost;
			Unit.MoveTarget = &MoveTarget;
			Unit.StandingParams = &StandingParams;
			Unit.ObstacleGrid = &NavigationSubsystem->GetObstacleGrid();
			Unit.EntitiesToIgnore =
				EntitiesToIgnoreList.IsEmpty() ?
				TConstArrayView<FMassEntityHandle>() :
					EntitiesToIgnoreList[EntityIt].EntitiesToIgnore;
			Unit.Entity = Context.GetEntity(EntityIt);
			Unit.AgentLocation = LocationList[EntityIt].GetTransform().GetTranslation();
			Unit.AgentRadius = RadiusList[EntityIt].Radius;
			Unit.DeltaTime = DeltaTime;
		}
	});

	// Phase B (MassAvoidanceProcessors.cpp:1237-1277 and 1294-1316), in parallel: each unit's close neighbours, sorted,
	// the 6 closest kept. Reads only transforms, radii, move targets and the ghosts' LastSeenActionID, none written by
	// this pass, so the order does not matter; the neighbours' ghost locations are left to phase C.
	TArray<FStandingNeighbour> Neighbours;
	Neighbours.SetNum(Units.Num() * MaxCloseObstacleTreated);
	ParallelFor(TEXT("HOLStandingAvoidanceNeighbours"), Units.Num(), 16, [&Units, &Neighbours, &EntityManager](const int32 UnitIndex)
	{
		FStandingUnit& Unit = Units[UnitIndex];
		const FMassStandingAvoidanceParameters& StandingParams = *Unit.StandingParams;
		const FMassEntityHandle Entity = Unit.Entity;
		const FVector AgentLocation = Unit.AgentLocation;
		const TConstArrayView<FMassEntityHandle> EntitiesToIgnore = Unit.EntitiesToIgnore;

		// Arrays used to store close agents
		TArray<FMassNavigationObstacleItem, TFixedAllocator<MaxObstacleResults>> CloseEntities;

		struct FSortedObstacle
		{
			FSortedObstacle() = default;
			FSortedObstacle(const FMassEntityHandle InEntity, const FVector InLocation, const FVector InForward, const FVector::FReal InDistSq)
				: Entity(InEntity)
				, Location(InLocation)
				, Forward(InForward)
				, DistSq(InDistSq)
			{}

			FMassEntityHandle Entity;
			FVector Location = FVector::ZeroVector;
			FVector Forward = FVector::ForwardVector;
			FVector::FReal DistSq = 0.;
		};
		TArray<FSortedObstacle, TFixedAllocator<MaxObstacleResults>> ClosestObstacles;

		// Find close obstacles
		// @todo: optimize FindCloseObstacles() and cache results. We're intentionally using agent location here, to allow to share the results with moving avoidance.
		const FNavigationObstacleHashGrid2D& ObstacleGrid = *Unit.ObstacleGrid;
		FindCloseObstacles(AgentLocation, StandingParams.GhostObstacleDetectionDistance, ObstacleGrid, CloseEntities, MaxObstacleResults);

		// Remove unwanted and find the closest in the CloseEntities
		const FVector::FReal DistanceCutOffSqr = FMath::Square(StandingParams.GhostObstacleDetectionDistance);
		ClosestObstacles.Reset();
		for (const FNavigationObstacleHashGrid2D::ItemIDType OtherEntity : CloseEntities)
		{
			// Skip self
			if (OtherEntity.Entity == Entity)
			{
				continue;
			}

			// Skip invalid entities.
			if (!EntityManager.IsEntityValid(OtherEntity.Entity))
			{
				continue;
			}

			// Skip too far
			const FTransformFragment& OtherTransform = EntityManager.GetFragmentDataChecked<FTransformFragment>(OtherEntity.Entity);
			const FVector OtherLocation = OtherTransform.GetTransform().GetLocation();
			const FVector::FReal DistSq = FVector::DistSquared(AgentLocation, OtherLocation);
			if (DistSq > DistanceCutOffSqr)
			{
				continue;
			}

			// Skip entities to ignore
			if (UNLIKELY(!EntitiesToIgnore.IsEmpty()) && EntitiesToIgnore.Contains(OtherEntity.Entity))
			{
				continue;
			}

			ClosestObstacles.Emplace(OtherEntity.Entity, OtherLocation, OtherTransform.GetTransform().GetRotation().GetForwardVector(), DistSq);
		}
		ClosestObstacles.Sort([](const FSortedObstacle& A, const FSortedObstacle& B) { return A.DistSq < B.DistSq; });

		const int32 NumCloseObstacles = FMath::Min(ClosestObstacles.Num(), MaxCloseObstacleTreated);
		Unit.NumCloseObstacles = NumCloseObstacles;
		for (int32 Index = 0; Index < NumCloseObstacles; Index++)
		{
			FSortedObstacle& OtherAgent = ClosestObstacles[Index];
			FMassEntityView OtherEntityView(EntityManager, OtherAgent.Entity);
			FStandingNeighbour& Neighbour = Neighbours[UnitIndex * MaxCloseObstacleTreated + Index];
			Neighbour.Location = OtherAgent.Location;
			Neighbour.Forward = OtherAgent.Forward;
			Neighbour.OtherRadius = OtherEntityView.GetFragmentData<FAgentRadiusFragment>().Radius;

			// @todo: this is heavy fragment to access, see if we could handle this differently.
			const FMassMoveTargetFragment* OtherMoveTarget = OtherEntityView.GetFragmentDataPtr<FMassMoveTargetFragment>();
			const FMassGhostLocationFragment* OtherGhost = OtherEntityView.GetFragmentDataPtr<FMassGhostLocationFragment>();
			Neighbour.OtherMoveTarget = OtherMoveTarget;
			Neighbour.OtherGhost = OtherGhost;
			Neighbour.bOtherHasGhost = OtherMoveTarget != nullptr && OtherGhost != nullptr
										&& OtherMoveTarget->GetCurrentAction() == EMassMovementAction::Stand
										&& OtherGhost->IsValid(OtherMoveTarget->GetCurrentActionID());
		}
	});

	// Phase C (MassAvoidanceProcessors.cpp:1165-1169, 1219-1235, 1279 and 1293-1406), one unit after the other in the
	// order of phase A, i.e. Mass' order: a neighbour's ghost location is read live, already moved this pass if that
	// neighbour came earlier, as in Mass' single loop. Each unit writes only its own ghost, and only here.
	for (int32 UnitIndex = 0; UnitIndex < Units.Num(); UnitIndex++)
	{
		const FStandingUnit& Unit = Units[UnitIndex];
		const FMassStandingAvoidanceParameters& StandingParams = *Unit.StandingParams;
		const FMassMoveTargetFragment& MoveTarget = *Unit.MoveTarget;
		FMassGhostLocationFragment& Ghost = *Unit.Ghost;
		const FVector::FReal AgentRadius = Unit.AgentRadius;
		const float DeltaTime = Unit.DeltaTime;

		const FVector::FReal GhostSeparationDistance = StandingParams.GhostSeparationDistance;
		const FVector::FReal GhostSeparationStiffness = StandingParams.GhostSeparationStiffness;

		const FVector::FReal MovingSeparationDistance = StandingParams.GhostSeparationDistance * StandingParams.MovingObstacleAvoidanceScale;
		const FVector::FReal MovingSeparationStiffness = StandingParams.GhostSeparationStiffness * StandingParams.MovingObstacleAvoidanceScale;

		// Steer ghost to move target.
		const FVector::FReal SteerK = 1. / StandingParams.GhostSteeringReactionTime;
		constexpr FVector::FReal SteeringMinDistance = 1.; // Do not bother to steer if the distance is less than this.

		FVector SteerDirection = FVector::ZeroVector;
		FVector Delta = MoveTarget.Center - Ghost.Location;
		Delta.Z = 0.;
		const FVector::FReal Distance = Delta.Size();
		FVector::FReal SpeedFade = 0.;
		if (Distance > SteeringMinDistance)
		{
			SteerDirection = Delta / Distance;
			SpeedFade = FMath::Clamp(Distance / FMath::Max(KINDA_SMALL_NUMBER, StandingParams.GhostStandSlowdownRadius), 0., 1.);
		}

		const FVector GhostDesiredVelocity = SteerDirection * StandingParams.GhostMaxSpeed * SpeedFade;
		FVector GhostSteeringForce = SteerK * (GhostDesiredVelocity - Ghost.Velocity); // Goal force

		const FVector::FReal GhostRadius = AgentRadius * StandingParams.GhostSeparationRadiusScale;

		// Compute forces
		for (int32 Index = 0; Index < Unit.NumCloseObstacles; Index++)
		{
			const FStandingNeighbour& OtherAgent = Neighbours[UnitIndex * MaxCloseObstacleTreated + Index];
			const FVector::FReal OtherRadius = OtherAgent.OtherRadius;
			const FVector::FReal TotalRadius = GhostRadius + OtherRadius;
			const FMassMoveTargetFragment* OtherMoveTarget = OtherAgent.OtherMoveTarget;
			const FMassGhostLocationFragment* OtherGhost = OtherAgent.OtherGhost;

			// If other has ghost active, avoid that, else avoid the actual agent.
			if (OtherAgent.bOtherHasGhost)
			{
				// Avoid the other agent more, when it is further away from its goal location.
				const FVector::FReal OtherDistanceToGoal = FVector::Distance(OtherGhost->Location, OtherMoveTarget->Center);
				const FVector::FReal OtherSteerFade = FMath::Clamp(OtherDistanceToGoal / StandingParams.GhostToTargetMaxDeviation, 0., 1.);
				const FVector::FReal SeparationStiffness = FMath::Lerp(GhostSeparationStiffness, MovingSeparationStiffness, OtherSteerFade);

				// Ghost separation
				FVector RelPos = Ghost.Location - OtherGhost->Location;
				RelPos.Z = 0.; // we assume we work on a flat plane for now
				const FVector::FReal ConDist = RelPos.Size();
				const FVector ConNorm = ConDist > 0. ? RelPos / ConDist : FVector::ForwardVector;

				// Separation force (stay away from obstacles if possible)
				const FVector::FReal PenSep = (TotalRadius + GhostSeparationDistance) - ConDist;
				const FVector::FReal SeparationMag = UE::MassNavigation::Smooth(FMath::Clamp(PenSep / GhostSeparationDistance, 0., 1.));
				const FVector SeparationForce = ConNorm * SeparationStiffness * SeparationMag;

				GhostSteeringForce += SeparationForce;
			}
			else
			{
				// Avoid more when the avoidance other is in front,
				const FVector DirToOther = (OtherAgent.Location - Ghost.Location).GetSafeNormal();
				const FVector::FReal DirectionalFade = FMath::Square(FMath::Max(0., FVector::DotProduct(MoveTarget.Forward, DirToOther)));
				const FVector::FReal DirectionScale = FMath::Lerp(StandingParams.MovingObstacleDirectionalScale, 1., DirectionalFade);

				// Treat the other agent as a 2D capsule protruding towards forward.
				const FVector OtherBasePosition = OtherAgent.Location;
				const FVector OtherPersonalSpacePosition = OtherAgent.Location + OtherAgent.Forward * OtherRadius * StandingParams.MovingObstaclePersonalSpaceScale * DirectionScale;
				const FVector OtherLocation = FMath::ClosestPointOnSegment(Ghost.Location, OtherBasePosition, OtherPersonalSpacePosition);

				FVector RelPos = Ghost.Location - OtherLocation;
				RelPos.Z = 0.;
				const FVector::FReal ConDist = RelPos.Size();
				const FVector ConNorm = ConDist > 0. ? RelPos / ConDist : FVector::ForwardVector;

				// Separation force (stay away from obstacles if possible)
				const FVector::FReal PenSep = (TotalRadius + MovingSeparationDistance) - ConDist;
				const FVector::FReal SeparationMag = UE::MassNavigation::Smooth(FMath::Clamp(PenSep / MovingSeparationDistance, 0., 1.));
				const FVector SeparationForce = ConNorm * MovingSeparationStiffness * SeparationMag;

				GhostSteeringForce += SeparationForce;
			}
		}

		GhostSteeringForce.Z = 0.;
		GhostSteeringForce = UE::MassNavigation::ClampVector(GhostSteeringForce, StandingParams.GhostMaxAcceleration); // Assume unit mass

		Ghost.Velocity += GhostSteeringForce * DeltaTime;
		Ghost.Velocity.Z = 0.;

		// Damping
		FMath::ExponentialSmoothingApprox(Ghost.Velocity, FVector::ZeroVector, DeltaTime, StandingParams.GhostVelocityDampingTime);

		Ghost.Location += Ghost.Velocity * DeltaTime;

		// Don't let the ghost location too far from move target center.
		const FVector DirToCenter = Ghost.Location - MoveTarget.Center;
		const FVector::FReal DistToCenter = DirToCenter.Length();
		if (DistToCenter > StandingParams.GhostToTargetMaxDeviation)
		{
			Ghost.Location = MoveTarget.Center + DirToCenter * (StandingParams.GhostToTargetMaxDeviation / DistToCenter);
		}
	}
}
