// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/HOLUnitOrderProcessor.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

UHOLUnitOrderProcessor::UHOLUnitOrderProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // orders apply on the server's step (D231)
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavMeshPathFollowProcessor"));
	bRequiresGameThreadExecution = true; // the order queue belongs to the game thread
}

void UHOLUnitOrderProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLUnitOrderProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ApplyQueuedOrders(EntityManager);
	StopArrivedUnits(EntityManager, Context);
}

void UHOLUnitOrderProcessor::ApplyQueuedOrders(FMassEntityManager& EntityManager)
{
	UWorld* World = EntityManager.GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (Units == nullptr || Commands == nullptr)
	{
		return;
	}

	TArray<FHOLUnitMoveCommand> Orders;
	Commands->ConsumeMoveOrders(Orders);
	for (const FHOLUnitMoveCommand& Order : Orders)
	{
		const FMassEntityHandle Entity = Units->FindEntity(Order.UnitId);
		// A unit can be ordered before it is born or after it is gone: the order is dropped (M4 will keep the
		// orders a player gave to a group that is still being created).
		if (!EntityManager.IsEntityValid(Entity))
		{
			continue;
		}

		const FTransformFragment& Transform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
		FMassMoveTargetFragment& MoveTarget = EntityManager.GetFragmentDataChecked<FMassMoveTargetFragment>(Entity);
		const FHOLUnitTypeConstants& Constants = EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity);

		const FVector Location = Transform.GetTransform().GetLocation();
		const FVector Direction = (Order.Target - Location).GetSafeNormal();
		MoveTarget.Center = Order.Target;
		// A zero forward would leave the steering without a reference: keep the facing when the goal is here.
		MoveTarget.Forward = Direction.IsNearlyZero() ? Transform.GetTransform().GetRotation().GetForwardVector() : Direction;
		MoveTarget.DistanceToGoal = 0.f;
		MoveTarget.EntityDistanceToGoal = FMassMoveTargetFragment::UnsetDistance;
		MoveTarget.SlackRadius = 0.f;
		MoveTarget.IntentAtGoal = EMassMovementAction::Stand; // Slow down on approach, stand at the goal.
		MoveTarget.DesiredSpeed.Set(Constants.Speed);
		MoveTarget.CreateNewAction(EMassMovementAction::Move, *World);

		EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity).Activity = EHOLUnitActivity::Moving;
	}
}

void UHOLUnitOrderProcessor::StopArrivedUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	const float Tolerance = Settings ? Settings->OrderArrivalTolerance : 50.f;

	EntityQuery.ForEachEntityChunk(Context, [World, Tolerance](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLUnitStateFragment> States = Context.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (States[EntityIt].Activity != EHOLUnitActivity::Moving)
			{
				continue;
			}

			FMassMoveTargetFragment& MoveTarget = MoveTargets[EntityIt];
			const FVector ToGoal = MoveTarget.Center - Transforms[EntityIt].GetTransform().GetLocation();
			const FVector::FReal DistanceToGoal2D = FVector(ToGoal.X, ToGoal.Y, 0.).Size();

			// Mass only maintains the distance to the goal while a navmesh path is followed. Without one (no path
			// before M3, D228) it believes the unit has arrived and scales the avoidance force down to
			// EndOfPathAvoidanceScale (10 %): the walkers would cross each other. Keep the goal up to date here.
			MoveTarget.DistanceToGoal = static_cast<float>(DistanceToGoal2D);
			const FVector Direction(ToGoal.X, ToGoal.Y, 0.);
			if (!Direction.IsNearlyZero())
			{
				MoveTarget.Forward = Direction.GetSafeNormal();
			}

			if (DistanceToGoal2D > Tolerance)
			{
				continue;
			}

			MoveTarget.CreateNewAction(EMassMovementAction::Stand, *World);
			States[EntityIt].Activity = EHOLUnitActivity::Idle;
		}
	});
}
