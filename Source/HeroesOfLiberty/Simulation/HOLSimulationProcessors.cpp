// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSimulationProcessors.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Simulation/HOLSimulationFragments.h"

namespace UE::HeroesOfLiberty::Simulation
{
	/** Distance (cm) to its slot under which a unit of the final leg leaves the steering and walks straight to it (D290). */
	constexpr double StraightToSlotDistance = 300.0;
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
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLFullSpeedProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		// As UMassApplyMovementProcessor, which moves the unit by this velocity next.
		const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
		const TArrayView<FMassDesiredMovementFragment> Movements = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();

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
			if (Order.Phase == EHOLMovePhase::FinalLeg && SlotDistance <= UE::HeroesOfLiberty::Simulation::StraightToSlotDistance)
			{
				// Near its slot, straight to it: the steering turns over ~0.3 s, at full speed it would circle the slot.
				Velocity = ToSlot / SlotDistance * Speed;
				continue;
			}

			// The steering's direction at full speed; from a standstill it has barely any, so towards the move target.
			FVector Direction = FVector(Velocity.X, Velocity.Y, 0.).GetSafeNormal();
			if (Direction.IsZero())
			{
				Direction = FVector(MoveTarget.Center.X - Location.X, MoveTarget.Center.Y - Location.Y, 0.).GetSafeNormal();
			}
			if (Direction.IsZero())
			{
				Direction = FVector(MoveTarget.Forward.X, MoveTarget.Forward.Y, 0.).GetSafeNormal();
			}
			Velocity = Direction * Speed;
		}
	});
}
