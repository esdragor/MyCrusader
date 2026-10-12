// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/MCAvoidanceHoldProcessors.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "MyCrusaderLog.h"
#include "Simulation/MCAvoidanceHoldFragments.h"
#include "Simulation/MCSimulationFragments.h"
#include "Simulation/MCSimulationSettings.h"
#include "Simulation/MCSimulationSubsystem.h"

namespace UE::MyCrusader::Avoidance
{
	/** Steps an avoidance impulse is kept for; negative = the settings' value, 0 = off (D230). */
	int32 HoldStepsOverride = -1;
	FAutoConsoleVariableRef CVarAvoidanceHold(TEXT("mc.Units.AvoidanceHold"), HoldStepsOverride,
		TEXT("Steps an avoidance impulse is re-applied for, negative = UMCSimulationSettings value, 0 = off"));

	/** Decay applied to the held impulse after each re-application; negative = the settings' value (D230). */
	float HoldDecayOverride = -1.f;
	FAutoConsoleVariableRef CVarAvoidanceHoldDecay(TEXT("mc.Units.AvoidanceHoldDecay"), HoldDecayOverride,
		TEXT("Factor applied to the held avoidance impulse after each re-application, negative = UMCSimulationSettings value, 1 = no decay, 0 = one re-application only"));

	/** Units whose avoidance is written step by step to Saved/AvoidanceTrace.csv; empty = no trace. */
	FString TraceUnitIds;
	FAutoConsoleVariableRef CVarAvoidanceTrace(TEXT("mc.Units.AvoidanceTrace"), TraceUnitIds,
		TEXT("Unit identifiers (D223) separated by + (-ExecCmds splits on commas), whose avoidance forces are written at every step to Saved/AvoidanceTrace.csv, empty = off"));

	int32 GetHoldSteps()
	{
		if (HoldStepsOverride >= 0)
		{
			return HoldStepsOverride;
		}
		const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
		return Settings ? FMath::Max(0, Settings->AvoidanceHoldSteps) : 0;
	}

	float GetHoldDecay()
	{
		if (HoldDecayOverride >= 0.f)
		{
			return FMath::Min(HoldDecayOverride, 1.f);
		}
		const UMCSimulationSettings* Settings = GetDefault<UMCSimulationSettings>();
		return Settings ? FMath::Clamp(Settings->AvoidanceHoldDecay, 0.f, 1.f) : 1.f;
	}

	TSet<uint32> ParseTraceUnitIds()
	{
		TSet<uint32> UnitIds;
		TArray<FString> Parts;
		TraceUnitIds.Replace(TEXT(","), TEXT("+")).ParseIntoArray(Parts, TEXT("+"));
		for (const FString& Part : Parts)
		{
			const uint32 UnitId = static_cast<uint32>(FCString::Strtoui64(*Part.TrimStartAndEnd(), nullptr, 10));
			if (UnitId != 0)
			{
				UnitIds.Add(UnitId);
			}
		}
		return UnitIds;
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

int32 UMCAvoidanceHoldProcessor::GetHoldSteps()
{
	return UE::MyCrusader::Avoidance::GetHoldSteps();
}

float UMCAvoidanceHoldProcessor::GetHoldDecay()
{
	return UE::MyCrusader::Avoidance::GetHoldDecay();
}

void UMCAvoidanceHoldProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMCAvoidanceHoldFragment>(EMassFragmentAccess::ReadWrite);
	// The solver clamps its force to MaxAcceleration: the held impulse must not get past that cap.
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	// Read for the trace only (mc.Units.AvoidanceTrace).
	EntityQuery.AddRequirement<FMCUnitStateFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
}

void UMCAvoidanceHoldProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::MyCrusader::Avoidance;

	const UWorld* World = EntityManager.GetWorld();
	const UMCSimulationSubsystem* Simulation = World ? World->GetSubsystem<UMCSimulationSubsystem>() : nullptr;
	// The step tells whether the throttled solvers ran in it (D220): their impulse is then fresh.
	const bool bSolverRan = Simulation == nullptr || Simulation->AreThrottledProcessorsRunningThisStep();
	const int64 StepIndex = Simulation ? Simulation->GetStepIndex() : 0;
	const int32 HoldSteps = GetHoldSteps();
	const float Decay = GetHoldDecay();

	const TSet<uint32> TracedUnits = TraceUnitIds.IsEmpty() ? TSet<uint32>() : ParseTraceUnitIds();
	TArray<FString> TraceRows;
	FCriticalSection TraceLock;

	EntityQuery.ParallelForEachEntityChunk(Context, [bSolverRan, StepIndex, HoldSteps, Decay, &TracedUnits, &TraceRows, &TraceLock](FMassExecutionContext& Context)
	{
		const TArrayView<FMassForceFragment> Forces = Context.GetMutableFragmentView<FMassForceFragment>();
		const TArrayView<FMCAvoidanceHoldFragment> Holds = Context.GetMutableFragmentView<FMCAvoidanceHoldFragment>();
		const FMassMovementParameters& MovementParams = Context.GetConstSharedFragment<FMassMovementParameters>();
		const FVector::FReal MaxAcceleration = MovementParams.MaxAcceleration;

		const TConstArrayView<FMCUnitStateFragment> States = Context.GetFragmentView<FMCUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FMassDesiredMovementFragment> DesiredMovements = Context.GetFragmentView<FMassDesiredMovementFragment>();
		const bool bTraceChunk = TracedUnits.Num() > 0 && States.Num() > 0;

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassForceFragment& Force = Forces[EntityIt];
			FMCAvoidanceHoldFragment& Hold = Holds[EntityIt];
			FVector Applied = FVector::ZeroVector;
			bool bReapplied = false;

			if (bSolverRan)
			{
				// The avoidance added its own force to the one the snapshot took, and that force is already in this
				// step's total: only remember it. A solver that produced nothing clears the memory - an old impulse
				// must not outlive a fresh run.
				Applied = Force.Value - Hold.ForceBefore;
				if (HoldSteps > 0 && !Applied.IsNearlyZero(0.01f))
				{
					Hold.HeldForce = Applied;
					Hold.StepsLeft = HoldSteps;
				}
				else
				{
					Hold.HeldForce = FVector::ZeroVector;
					Hold.StepsLeft = 0;
				}
			}
			else if (Hold.StepsLeft > 0)
			{
				// Exactly HoldSteps re-applications after a run, capped like the solver caps its own result.
				Applied = Hold.HeldForce;
				Force.Value = (Force.Value + Hold.HeldForce).GetClampedToMaxSize(MaxAcceleration);
				Hold.HeldForce *= Decay;
				--Hold.StepsLeft;
				bReapplied = true;
			}

			if (bTraceChunk && TracedUnits.Contains(States[EntityIt].UnitId))
			{
				const FVector Location = Transforms.Num() > 0 ? Transforms[EntityIt].GetTransform().GetLocation() : FVector::ZeroVector;
				const FVector Velocity = Velocities.Num() > 0 ? Velocities[EntityIt].Value : FVector::ZeroVector;
				const FVector DesiredVelocity = DesiredMovements.Num() > 0 ? DesiredMovements[EntityIt].DesiredVelocity : FVector::ZeroVector;
				const FString Row = FString::Printf(TEXT("%lld,%u,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d"),
					StepIndex, States[EntityIt].UnitId, bSolverRan ? 1 : 0, Location.X, Location.Y, Velocity.X, Velocity.Y,
					DesiredVelocity.X, DesiredVelocity.Y, Hold.ForceBefore.X, Hold.ForceBefore.Y, Applied.X, Applied.Y,
					Force.Value.X, Force.Value.Y, Force.Value.Size(), MaxAcceleration, Hold.StepsLeft, bReapplied ? 1 : 0);
				FScopeLock Lock(&TraceLock);
				TraceRows.Add(Row);
			}
		}
	});

	WriteTraceRows(TraceRows);
}

void UMCAvoidanceHoldProcessor::WriteTraceRows(TArray<FString>& Rows)
{
	if (Rows.Num() == 0)
	{
		return;
	}
	const FString Path = FPaths::ProjectSavedDir() / TEXT("AvoidanceTrace.csv");
	if (!bTraceFileStarted)
	{
		// One file per process: the first traced step replaces whatever an earlier run left.
		bTraceFileStarted = true;
		const FString Header = TEXT("Step,UnitId,SolverRan,X,Y,VelX,VelY,DesiredVelX,DesiredVelY,SteeringX,SteeringY,ImpulseX,ImpulseY,ForceX,ForceY,Force,MaxAcceleration,StepsLeft,Reapplied\n");
		FFileHelper::SaveStringToFile(Header, *Path);
		UE_LOG(LogMCSimulation, Display, TEXT("Avoidance trace of units %s written to %s"), *UE::MyCrusader::Avoidance::TraceUnitIds, *Path);
	}
	Rows.Sort();
	FFileHelper::SaveStringToFile(FString::Join(Rows, TEXT("\n")) + TEXT("\n"), *Path, FFileHelper::EEncodingOptions::AutoDetect,
		&IFileManager::Get(), FILEWRITE_Append);
}
