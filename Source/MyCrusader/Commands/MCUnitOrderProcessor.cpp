// Copyright Epic Games, Inc. All Rights Reserved.

#include "Commands/MCUnitOrderProcessor.h"

#include "Commands/MCCommandSubsystem.h"
#include "Engine/World.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "Simulation/MCSimulationFragments.h"
#include "Simulation/MCSimulationSettings.h"
#include "Simulation/MCUnitSubsystem.h"
#include "Simulation/MCUnitTypes.h"

UMCUnitOrderProcessor::UMCUnitOrderProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavMeshPathFollowProcessor"));
	bRequiresGameThreadExecution = true; // the order queue belongs to the game thread
}

void UMCUnitOrderProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMCUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMCLightUnitTag>(EMassFragmentPresence::All);
}

void UMCUnitOrderProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	ApplyQueuedOrders(EntityManager);
	StopArrivedUnits(EntityManager, Context);
}

void UMCUnitOrderProcessor::ApplyQueuedOrders(FMassEntityManager& EntityManager)
{
	UWorld* World = EntityManager.GetWorld();
	UMCUnitSubsystem* Units = World ? World->GetSubsystem<UMCUnitSubsystem>() : nullptr;
	UMCCommandSubsystem* Commands = World ? World->GetSubsystem<UMCCommandSubsystem>() : nullptr;
	if (Units == nullptr || Commands == nullptr)
	{
		return;
	}

	TArray<FMCUnitMoveCommand> Orders;
	Commands->ConsumeMoveOrders(Orders);
	for (const FMCUnitMoveCommand& Order : Orders)
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
		const FMCUnitTypeConstants& Constants = EntityManager.GetConstSharedFragmentDataChecked<FMCUnitTypeConstants>(Entity);

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

		EntityManager.GetFragmentDataChecked<FMCUnitStateFragment>(Entity).Activity = EMCUnitActivity::Moving;
	}
}

void UMCUnitOrderProcessor::StopArrivedUnits(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
	const float Tolerance = Settings ? Settings->OrderArrivalTolerance : 50.f;

	EntityQuery.ForEachEntityChunk(Context, [World, Tolerance](FMassExecutionContext& Context)
	{
		const TArrayView<FMCUnitStateFragment> States = Context.GetMutableFragmentView<FMCUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (States[EntityIt].Activity != EMCUnitActivity::Moving)
			{
				continue;
			}

			const FVector ToGoal = MoveTargets[EntityIt].Center - Transforms[EntityIt].GetTransform().GetLocation();
			if (ToGoal.SizeSquared2D() > FMath::Square(Tolerance))
			{
				continue;
			}

			MoveTargets[EntityIt].CreateNewAction(EMassMovementAction::Stand, *World);
			States[EntityIt].Activity = EMCUnitActivity::Idle;
		}
	});
}
