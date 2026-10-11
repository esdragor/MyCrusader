// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCAvoidanceHoldProcessors.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "Simulation/MCAvoidanceHoldFragments.h"
#include "Simulation/MCSimulationSettings.h"
#include "Simulation/MCSimulationSubsystem.h"

namespace UE::MyCrusader::Avoidance
{
	/** Steps an avoidance impulse is kept for; 0 = the settings' value (D230). */
	int32 HoldStepsOverride = 0;
	FAutoConsoleVariableRef CVarAvoidanceHold(TEXT("mc.Units.AvoidanceHold"), HoldStepsOverride,
		TEXT("Steps an avoidance impulse is re-applied for, 0 = UMCSimulationSettings value, negative = off"));

	/** Decay applied to the held impulse at every step; 0 = the settings' value (D230). */
	float HoldDecayOverride = 0.f;
	FAutoConsoleVariableRef CVarAvoidanceHoldDecay(TEXT("mc.Units.AvoidanceHoldDecay"), HoldDecayOverride,
		TEXT("Per step decay of the held avoidance impulse, 0 = UMCSimulationSettings value, 1 = no decay"));

	int32 GetHoldSteps()
	{
		if (HoldStepsOverride != 0)
		{
			return FMath::Max(0, HoldStepsOverride);
		}
		const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
		return Settings ? FMath::Max(0, Settings->AvoidanceHoldSteps) : 0;
	}

	float GetHoldDecay()
	{
		return HoldDecayOverride > 0.f ? HoldDecayOverride : (GetDefault<UMCSimulationSettings>() ? GetDefault<UMCSimulationSettings>()->AvoidanceHoldDecay : 1.f);
	}
}

//----------------------------------------------------------------------//
// UMCAvoidanceSnapshotProcessor
//----------------------------------------------------------------------//

UMCAvoidanceSnapshotProcessor::UMCAvoidanceSnapshotProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	// Between the steering, which writes the force of the step, and the avoidance, which adds to it.
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassSteerToMoveTargetProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassMovingAvoidanceProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassStandingAvoidanceProcessor"));
}

void UMCAvoidanceSnapshotProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMCAvoidanceHoldFragment>(EMassFragmentAccess::ReadWrite);
}

void UMCAvoidanceSnapshotProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FMassForceFragment> Forces = Context.GetFragmentView<FMassForceFragment>();
		const TArrayView<FMCAvoidanceHoldFragment> Holds = Context.GetMutableFragmentView<FMCAvoidanceHoldFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			Holds[EntityIt].ForceBefore = Forces[EntityIt].Value;
		}
	});
}

//----------------------------------------------------------------------//
// UMCAvoidanceHoldProcessor
//----------------------------------------------------------------------//

UMCAvoidanceHoldProcessor::UMCAvoidanceHoldProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::AllNetModes);
	// Between the avoidance solvers and the processor that turns the force into a velocity.
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassMovingAvoidanceProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassStandingAvoidanceProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassApplyForceProcessor"));
}

void UMCAvoidanceHoldProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCAvoidanceHoldFragment>(EMassFragmentAccess::ReadWrite);
}

void UMCAvoidanceHoldProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::MyCrusader::Avoidance;

	const UWorld* World = EntityManager.GetWorld();
	const UMCSimulationSubsystem* Simulation = World ? World->GetSubsystem<UMCSimulationSubsystem>() : nullptr;
	// The solver ran on this step when the step index is a multiple of its interval: the impulse is then fresh.
	const bool bSolverRan = Simulation == nullptr || Simulation->DidThrottledProcessorsRunThisStep();
	const int32 HoldSteps = GetHoldSteps();
	const float Decay = GetHoldDecay();

	EntityQuery.ParallelForEachEntityChunk(Context, [bSolverRan, HoldSteps, Decay](FMassExecutionContext& Context)
	{
		const TArrayView<FMassForceFragment> Forces = Context.GetMutableFragmentView<FMassForceFragment>();
		const TArrayView<FMCAvoidanceHoldFragment> Holds = Context.GetMutableFragmentView<FMCAvoidanceHoldFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassForceFragment& Force = Forces[EntityIt];
			FMCAvoidanceHoldFragment& Hold = Holds[EntityIt];

			// The avoidance added its own force to the one the snapshot took: what is left is the impulse to keep.
			const FVector Impulse = Force.Value - Hold.ForceBefore;
			if (bSolverRan && !Impulse.IsNearlyZero(0.01f))
			{
				Hold.HeldForce = Impulse;
				Hold.StepsLeft = HoldSteps;
			}
			else if (Hold.StepsLeft > 0)
			{
				Hold.HeldForce *= Decay;
				--Hold.StepsLeft;
			}
			else
			{
				Hold.HeldForce = FVector::ZeroVector;
			}

			if (Hold.StepsLeft > 0)
			{
				Force.Value += Hold.HeldForce;
			}
		}
	});
}
