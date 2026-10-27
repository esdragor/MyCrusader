// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSimulationSubsystem.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassProcessor.h"
#include "MassProcessorDependencySolver.h"
#include "MassSimulationSubsystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Avoidance/MassAvoidanceFragments.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Steering/MassSteeringFragments.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace UE::HeroesOfLiberty::Simulation
{
	/** Stress test knob (D220): overrides the settings' interval when > 0. */
	int32 ThrottledStepIntervalOverride = 0;
	FAutoConsoleVariableRef CVarThrottledStepInterval(TEXT("hol.Simulation.ThrottledStepInterval"), ThrottledStepIntervalOverride,
		TEXT("Steps between two runs of the throttled processors, 0 = UHOLSimulationSettings value"));

	/** Determinism probe: hashes every unit's movement state, to find where two runs part. */
	int32 StepHashes = 0;
	FAutoConsoleVariableRef CVarStepHashes(TEXT("hol.Simulation.StepHashes"), StepHashes,
		TEXT("1: logs a hash of every unit's transform, velocity, force and move target at the tail of each step; 2: after each processor too (two runs' logs then show the first step and processor that differ); 3: also every unit's place at the tail of steps 0 and 1"));

	/** FNV-1a over the exact bits of each living unit's movement state, in identifier order. */
	/**
	 * Separate hashes of the other simulation state (J7c.4, determinism): combat, ghost, steering and standing steering,
	 * order, and the raw entity handles; logged beside the movement hash, so two runs' logs name the part that differs.
	 */
	struct FPartHashes
	{
		uint64 Combat = 14695981039346656037ull;
		uint64 Ghost = 14695981039346656037ull;
		uint64 Steering = 14695981039346656037ull;
		uint64 Order = 14695981039346656037ull;
		uint64 Handles = 14695981039346656037ull;
	};

	void MixInto(uint64& Hash, const uint64 Value)
	{
		for (int32 Byte = 0; Byte < 8; ++Byte)
		{
			Hash = (Hash ^ ((Value >> (Byte * 8)) & 0xFFull)) * 1099511628211ull;
		}
	}

	uint64 HashUnits(const UWorld* World, const FMassEntityManager& EntityManager, const bool bDump = false, FPartHashes* Parts = nullptr)
	{
		uint64 Hash = 14695981039346656037ull;
		const auto Mix = [&Hash](const uint64 Value)
		{
			MixInto(Hash, Value);
		};
		// A handle by the unit it designates (entity indices may be handed out in another order without harm).
		const auto UnitIdOf = [&EntityManager](const FMassEntityHandle Other) -> uint64
		{
			if (!Other.IsSet() || !EntityManager.IsEntityValid(Other))
			{
				return MAX_uint64;
			}
			const FHOLUnitStateFragment* OtherState = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Other);
			return OtherState ? OtherState->UnitId : MAX_uint64 - 1;
		};
		const auto MixVector = [&Mix](const FVector& Value)
		{
			Mix(BitCast<uint64>(Value.X));
			Mix(BitCast<uint64>(Value.Y));
			Mix(BitCast<uint64>(Value.Z));
		};
		const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		if (!Units)
		{
			return Hash;
		}
		TArray<uint32> UnitIds;
		Units->GetAllUnitIds(UnitIds);
		UnitIds.Sort();
		for (const uint32 UnitId : UnitIds)
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			if (!EntityManager.IsEntityValid(Entity))
			{
				continue;
			}
			Mix(UnitId);
			if (const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity))
			{
				if (bDump)
				{
					const FVector Location = Transform->GetTransform().GetLocation();
					const FVector Facing = Transform->GetTransform().GetRotation().Vector();
					const FMassVelocityFragment* DumpVelocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);
					const FMassForceFragment* DumpForce = EntityManager.GetFragmentDataPtr<FMassForceFragment>(Entity);
					const FMassMoveTargetFragment* DumpTarget = EntityManager.GetFragmentDataPtr<FMassMoveTargetFragment>(Entity);
					UE_LOG(LogHOLSimulation, Display, TEXT("Unit dump: #%u at %.17g %.17g %.17g facing %.17g %.17g velocity %.17g %.17g force %.17g %.17g target %.17g %.17g"),
						UnitId, Location.X, Location.Y, Location.Z, Facing.X, Facing.Y, DumpVelocity ? DumpVelocity->Value.X : 0., DumpVelocity ? DumpVelocity->Value.Y : 0.,
						DumpForce ? DumpForce->Value.X : 0., DumpForce ? DumpForce->Value.Y : 0., DumpTarget ? DumpTarget->Center.X : 0., DumpTarget ? DumpTarget->Center.Y : 0.);
				}
				MixVector(Transform->GetTransform().GetLocation());
				MixVector(Transform->GetTransform().GetRotation().Vector());
			}
			if (const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity))
			{
				MixVector(Velocity->Value);
			}
			if (const FMassForceFragment* Force = EntityManager.GetFragmentDataPtr<FMassForceFragment>(Entity))
			{
				MixVector(Force->Value);
			}
			if (const FMassMoveTargetFragment* MoveTarget = EntityManager.GetFragmentDataPtr<FMassMoveTargetFragment>(Entity))
			{
				MixVector(MoveTarget->Center);
				Mix(static_cast<uint64>(MoveTarget->GetCurrentAction()));
			}
			if (!Parts)
			{
				continue;
			}
			const auto MixV = [](uint64& Into, const FVector& Value)
			{
				MixInto(Into, BitCast<uint64>(Value.X));
				MixInto(Into, BitCast<uint64>(Value.Y));
				MixInto(Into, BitCast<uint64>(Value.Z));
			};
			MixInto(Parts->Combat, UnitId);
			if (const FHOLCombatFragment* Combat = EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity))
			{
				MixInto(Parts->Combat, UnitIdOf(Combat->Target));
				MixInto(Parts->Combat, UnitIdOf(Combat->LastAttacker));
				MixInto(Parts->Combat, static_cast<uint64>(Combat->CooldownMs));
				MixInto(Parts->Combat, static_cast<uint64>(Combat->AttackerCount));
				MixInto(Parts->Combat, static_cast<uint64>(Combat->Hits));
				MixInto(Parts->Combat, static_cast<uint64>(BitCast<uint32>(Combat->DamageDealt)));
				MixInto(Parts->Combat, static_cast<uint64>(Combat->Kills));
				MixInto(Parts->Combat, static_cast<uint64>(Combat->Mode));
				MixInto(Parts->Combat, static_cast<uint64>(Combat->DeathStep));
				MixInto(Parts->Combat, (Combat->bAttacking ? 1u : 0u) | (Combat->bApproaching ? 2u : 0u) | (Combat->bAttackMoveDetour ? 4u : 0u) | (Combat->bOrderedTarget ? 8u : 0u));
				MixInto(Parts->Combat, BitCast<uint64>(Combat->ApproachPoint.X));
				MixInto(Parts->Combat, BitCast<uint64>(Combat->ApproachPoint.Y));
				MixInto(Parts->Handles, ((uint64(uint32(Combat->Target.Index)) << 32) | uint32(Combat->Target.SerialNumber)));
				MixInto(Parts->Handles, ((uint64(uint32(Combat->LastAttacker.Index)) << 32) | uint32(Combat->LastAttacker.SerialNumber)));
			}
			if (const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity))
			{
				MixInto(Parts->Combat, static_cast<uint64>(BitCast<uint32>(State->CurrentHealth)));
			}
			MixInto(Parts->Handles, ((uint64(uint32(Entity.Index)) << 32) | uint32(Entity.SerialNumber)));
			if (const FMassGhostLocationFragment* Ghost = EntityManager.GetFragmentDataPtr<FMassGhostLocationFragment>(Entity))
			{
				MixV(Parts->Ghost, Ghost->Location);
				MixV(Parts->Ghost, Ghost->Velocity);
				MixInto(Parts->Ghost, Ghost->LastSeenActionID);
			}
			if (const FMassSteeringFragment* Steering = EntityManager.GetFragmentDataPtr<FMassSteeringFragment>(Entity))
			{
				MixV(Parts->Steering, Steering->DesiredVelocity);
			}
			if (const FMassStandingSteeringFragment* Standing = EntityManager.GetFragmentDataPtr<FMassStandingSteeringFragment>(Entity))
			{
				MixV(Parts->Steering, Standing->TargetLocation);
				MixInto(Parts->Steering, BitCast<uint32>(Standing->TrackedTargetSpeed));
				MixInto(Parts->Steering, BitCast<uint32>(Standing->TargetSelectionCooldown));
				MixInto(Parts->Steering, (Standing->bIsUpdatingTarget ? 1u : 0u) | (Standing->bEnteredFromMoveAction ? 2u : 0u));
			}
			if (const FHOLMoveOrderFragment* Order = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity))
			{
				MixInto(Parts->Order, UnitId);
				MixInto(Parts->Order, static_cast<uint64>(Order->Phase));
				MixV(Parts->Order, Order->Slot);
				MixInto(Parts->Order, Order->OrderSerial);
				MixInto(Parts->Order, (Order->bFormation ? 1u : 0u) | (Order->bReforming ? 2u : 0u) | (Order->bStraggler ? 4u : 0u) | (Order->bFacesFront ? 8u : 0u) | (Order->bShortOfSlot ? 16u : 0u));
				MixInto(Parts->Order, Order->RowKind | (Order->BlockIndex << 8) | (uint64(Order->StuckSteps) << 16) | (uint64(Order->BuildingPushes) << 32));
				MixInto(Parts->Order, BitCast<uint32>(Order->MarchSlot.X));
				MixInto(Parts->Order, BitCast<uint32>(Order->MarchSlot.Y));
				MixInto(Parts->Order, BitCast<uint32>(Order->LastSlotDistance));
				MixInto(Parts->Order, BitCast<uint32>(Order->MarchSpeed));
				MixInto(Parts->Order, BitCast<uint32>(Order->FrontYaw));
			}
		}
		return Hash;
	}
}

bool UHOLSimulationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLSimulationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// The simulation is authoritative: clients only interpolate replicated states (D29).
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}

	UMassEntitySubsystem* EntitySubsystem = InWorld.GetSubsystem<UMassEntitySubsystem>();
	UMassSimulationSubsystem* MassSimulation = InWorld.GetSubsystem<UMassSimulationSubsystem>();
	if (!EntitySubsystem || !MassSimulation)
	{
		UE_LOG(LogHOLSimulation, Error, TEXT("Mass subsystems missing: fixed-step simulation disabled"));
		return;
	}

	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	StepSeconds = Settings->GetStepSeconds();
	MaxStepsPerFrame = Settings->MaxStepsPerFrame;
	ThrottledProcessors = Settings->ThrottledProcessors;
	ThrottledStepInterval = FMath::Max(1, Settings->ThrottledProcessorStepInterval);
	EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();

	TArray<UMassProcessor*> Processors;
	for (const TSubclassOf<UMassProcessor>& ProcessorClass : Settings->SimulationProcessors)
	{
		if (!ProcessorClass)
		{
			continue;
		}
		// A processor left in Mass' phases would run twice, once per frame with the frame delta.
		if (ProcessorClass->GetDefaultObject<UMassProcessor>()->ShouldAutoAddToGlobalList())
		{
			UE_LOG(LogHOLSimulation, Error, TEXT("%s still auto-registers with Mass phases: set bAutoRegisterWithProcessingPhases=False in DefaultMass.ini"), *ProcessorClass->GetName());
		}
		UMassProcessor* Processor = NewObject<UMassProcessor>(this, ProcessorClass);
		Processor->CallInitialize(this, EntityManager.ToSharedRef());
		Processors.Add(Processor);
	}

	// Ordered from the processors' declared groups and requirements. Solved once, without runtime pruning:
	// at begin play no unit exists yet, so every processor would be pruned for matching no archetype.
	FMassProcessorDependencySolver Solver(Processors, /*bIsGameRuntime*/ false);
	TArray<FMassProcessorOrderInfo> SortedProcessors;
	Solver.ResolveDependencies(SortedProcessors, EntityManager);
	for (const FMassProcessorOrderInfo& Info : SortedProcessors)
	{
		if (Info.NodeType == FMassProcessorOrderInfo::EDependencyNodeType::Processor && Info.Processor)
		{
			StepPipeline.AppendProcessor(*Info.Processor);
			ProcessorThrottled.Add(IsThrottled(*Info.Processor));
		}
	}

	PhaseStartedHandle = MassSimulation->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics)
		.AddUObject(this, &UHOLSimulationSubsystem::OnPrePhysicsPhaseStarted);

	FString Pipeline;
	for (const UMassProcessor* Processor : StepPipeline.GetProcessors())
	{
		Pipeline += FString::Printf(TEXT(" %s"), *Processor->GetProcessorName());
	}
	UE_LOG(LogHOLSimulation, Log, TEXT("Fixed-step simulation at %d Hz, %d of %d processors in order:%s"),
		Settings->SimulationRateHz, StepPipeline.Num(), Processors.Num(), *Pipeline);
	if (StepPipeline.Num() != Processors.Num())
	{
		UE_LOG(LogHOLSimulation, Error, TEXT("Some simulation processors were dropped by the dependency solver (%d sorted nodes)"), SortedProcessors.Num());
	}
}

void UHOLSimulationSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UMassSimulationSubsystem* MassSimulation = World->GetSubsystem<UMassSimulationSubsystem>())
		{
			MassSimulation->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics).Remove(PhaseStartedHandle);
		}
	}
	PhaseStartedHandle.Reset();
	OnStepStarted.Clear();
	OnStepEnded.Clear();
	StepPipeline.Reset();
	ProcessorThrottled.Reset();
	EntityManager.Reset();

	Super::Deinitialize();
}

void UHOLSimulationSubsystem::OnPrePhysicsPhaseStarted(const float DeltaSeconds)
{
	// R3.10: no step (movement, combat, production) while the match waits for its named players.
	const AHOLGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AHOLGameState>() : nullptr;
	if (GameState && GameState->IsWaitingForPlayers())
	{
		Accumulator = 0.f;
		return;
	}
	Accumulator += DeltaSeconds;

	int32 NumSteps = 0;
	while (Accumulator >= StepSeconds && NumSteps < MaxStepsPerFrame)
	{
		RunStep();
		Accumulator -= StepSeconds;
		++NumSteps;
	}

	if (Accumulator >= StepSeconds)
	{
		// The host cannot keep up: drop the late time rather than spiral (the simulation slows down).
		Accumulator = FMath::Fmod(Accumulator, StepSeconds);
		++StepStats.NumDroppedFrames;
	}
}

void UHOLSimulationSubsystem::RunStep()
{
	// Listeners (the stress test) run before the step's chrono: their orders belong to this step, their cost does not.
	OnStepStarted.Broadcast(StepIndex);

	// Insights scopes for Tools/Tests/trace_report.py; the processors already have theirs (Mass).
	TRACE_CPUPROFILER_EVENT_SCOPE(HOLSimulationStep);
	const double StartSeconds = FPlatformTime::Seconds();

	// Births are served at the head of the step and destructions applied at its tail (D224): neither is a
	// processor, because Mass forbids the synchronous entity API inside a processor's execution scope. The
	// step is the unit of the simulation (D219), so they belong to it.
	UHOLUnitSubsystem* Units = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	const double BirthStartSeconds = FPlatformTime::Seconds();
	int32 NumUnitsBorn = 0;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(HOLSimulationBirths);
		NumUnitsBorn = Units ? Units->ServeBirthRequests() : 0;
	}
	const double BirthMs = (FPlatformTime::Seconds() - BirthStartSeconds) * 1000.0;

	{
		FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
		const TConstArrayView<TObjectPtr<UMassProcessor>> Processors = StepPipeline.GetProcessors();
		RegisterProcessorStats(Processors);
		// One decision per step (D220), read live so the console override applies at once, and published for the
		// processors that depend on it (D230).
		const int32 ThrottledInterval = GetThrottledStepInterval();
		bThrottledProcessorsRunThisStep = (StepIndex % ThrottledInterval) == 0;
		// Run one by one to time each processor (stress test breakdown). A throttled processor keeps its own
		// runs and total (D220), so its breakdown stays a per-run cost.
		for (int32 Index = 0; Index < Processors.Num(); ++Index)
		{
			FHOLSimulationProcessorStats& ProcessorStats = StepStats.Processors[Index];
			const bool bThrottled = ProcessorThrottled.IsValidIndex(Index) && ProcessorThrottled[Index];
			ProcessorStats.StepInterval = bThrottled ? ThrottledInterval : 1;
			if (bThrottled && !bThrottledProcessorsRunThisStep)
			{
				continue;
			}
			const double ProcessorStartSeconds = FPlatformTime::Seconds();
			UE::Mass::Executor::Run(*Processors[Index], ProcessingContext);
			ProcessorStats.TotalMs += (FPlatformTime::Seconds() - ProcessorStartSeconds) * 1000.0;
			++ProcessorStats.Runs;
			if (UE::HeroesOfLiberty::Simulation::StepHashes >= 2)
			{
				UE::HeroesOfLiberty::Simulation::FPartHashes Parts;
				const uint64 MoveHash = UE::HeroesOfLiberty::Simulation::HashUnits(GetWorld(), *EntityManager, false, &Parts);
				const UHOLCombatProcessor* CombatProcessor = Cast<UHOLCombatProcessor>(Processors[Index]);
				UE_LOG(LogHOLSimulation, Display, TEXT("Step hash: step %lld after %s %016llx combat %016llx ghost %016llx steering %016llx order %016llx handles %016llx arrows %016llx"),
					StepIndex, *Processors[Index]->GetProcessorName(), MoveHash, Parts.Combat, Parts.Ghost, Parts.Steering, Parts.Order, Parts.Handles,
					CombatProcessor ? CombatProcessor->HashArrows() : 0ull);
			}
		}
	}

	const double DestructionStartSeconds = FPlatformTime::Seconds();
	int32 NumUnitsDestroyed = 0;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(HOLSimulationDestructions);
		NumUnitsDestroyed = Units ? Units->FlushDestructions() : 0;
	}
	const double DestructionMs = (FPlatformTime::Seconds() - DestructionStartSeconds) * 1000.0;
	if (UE::HeroesOfLiberty::Simulation::StepHashes >= 1)
	{
		UE_LOG(LogHOLSimulation, Display, TEXT("Step hash: step %lld %016llx"), StepIndex, UE::HeroesOfLiberty::Simulation::HashUnits(GetWorld(), *EntityManager,
			UE::HeroesOfLiberty::Simulation::StepHashes >= 3 && StepIndex <= 1));
	}

	const double StepMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	++StepIndex;
	++StepStats.NumSteps;
	StepStats.TotalMs += StepMs;
	StepStats.MaxMs = FMath::Max(StepStats.MaxMs, StepMs);
	StepStats.TotalBirthMs += BirthMs;
	StepStats.NumUnitsBorn += NumUnitsBorn;
	StepStats.TotalDestructionMs += DestructionMs;
	StepStats.NumUnitsDestroyed += NumUnitsDestroyed;

	OnStepEnded.Broadcast(StepIndex);
}

void UHOLSimulationSubsystem::RegisterProcessorStats(const TConstArrayView<TObjectPtr<UMassProcessor>> Processors)
{
	if (StepStats.Processors.Num() == Processors.Num())
	{
		return;
	}
	StepStats.Processors.Reset();
	for (const UMassProcessor* Processor : Processors)
	{
		FHOLSimulationProcessorStats& Stats = StepStats.Processors.AddDefaulted_GetRef();
		Stats.Name = Processor->GetClass()->GetName();
	}
}

bool UHOLSimulationSubsystem::IsThrottled(const UMassProcessor& Processor) const
{
	for (const TSubclassOf<UMassProcessor>& Throttled : ThrottledProcessors)
	{
		if (Throttled && Processor.GetClass()->IsChildOf(Throttled.Get()))
		{
			return true;
		}
	}
	return false;
}

int32 UHOLSimulationSubsystem::GetThrottledStepInterval() const
{
	using namespace UE::HeroesOfLiberty::Simulation;
	const int32 Interval = ThrottledStepIntervalOverride > 0 ? ThrottledStepIntervalOverride : ThrottledStepInterval;
	return FMath::Max(1, Interval);
}
