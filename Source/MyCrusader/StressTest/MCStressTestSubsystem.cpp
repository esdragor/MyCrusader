// Copyright Epic Games, Inc. All Rights Reserved.

#include "StressTest/MCStressTestSubsystem.h"

#include "Commands/MCCommandSubsystem.h"
#include "DynamicRHI.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassNavigationFragments.h"
#include "MassRepresentationFragments.h"
#include "MyCrusaderLog.h"
#include "NavigationSystem.h"
#include "RenderTimer.h"
#include "Simulation/MCSimulationFragments.h"
#include "Simulation/MCSimulationSubsystem.h"
#include "Simulation/MCUnitSubsystem.h"
#include "UnrealClient.h"
#include "Widgets/SWindow.h"

namespace UE::MyCrusader::StressTest
{
	constexpr float WarmUpSeconds = 3.f;
	/** Time granted to the last fixed steps after the run asked to quit, so the units are destroyed (D224). */
	constexpr float QuitDelaySeconds = 0.5f;
	/** Units appear on the navmesh within this radius of the map centre. */
	float SpawnRadius = 8000.f;
	FAutoConsoleVariableRef CVarSpawnRadius(TEXT("mc.StressTest.SpawnRadius"), SpawnRadius,
		TEXT("Radius (cm) around the map centre where stress test units appear: sets the crowd density"));

	/** Type of the units of a run: a row of the unit type table (D221). */
	FString UnitType = TEXT("LightInfantry");
	FAutoConsoleVariableRef CVarUnitType(TEXT("mc.StressTest.UnitType"), UnitType,
		TEXT("Row of /Game/Units/UnitTypes used by the stress test"));

	/** S2 walks by move orders (D229): a unit is ordered a new destination within this radius of itself. */
	float OrderRadius = 3000.f;
	FAutoConsoleVariableRef CVarOrderRadius(TEXT("mc.StressTest.OrderRadius"), OrderRadius,
		TEXT("Radius (cm) within which S2 picks the destination of a new move order"));

	/** Orders issued per frame at most: the birth of 2 000 units must not order them all in one frame. */
	int32 MaxOrdersPerFrame = 200;
	FAutoConsoleVariableRef CVarMaxOrdersPerFrame(TEXT("mc.StressTest.MaxOrdersPerFrame"), MaxOrdersPerFrame,
		TEXT("Move orders the stress test issues per frame at most"));

	// D218 thresholds.
	constexpr double MaxSimulationMs = 3.0;
	constexpr double MinFramesPerSecond = 144.0;

	UMCStressTestSubsystem* FindSubsystem(UWorld* World)
	{
		return World ? World->GetSubsystem<UMCStressTestSubsystem>() : nullptr;
	}

	/** A minimized window keeps a placeholder back buffer, and capturing it crashes the D3D12 RHI. */
	bool IsGameWindowMinimized(const UWorld* World)
	{
		const UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr;
		const TSharedPtr<SWindow> Window = GameViewport ? GameViewport->GetWindow() : nullptr;
		return Window.IsValid() && Window->IsWindowMinimized();
	}

	FAutoConsoleCommandWithWorldAndArgs StartCommand(
		TEXT("mc.StressTest"),
		TEXT("mc.StressTest S1|S2 [Count=2000] [Seconds=20] [quit] - S1: units standing still, S2: units walking by move orders (D190, D218, D229)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UMCStressTestSubsystem* Subsystem = FindSubsystem(World);
			if (!Subsystem || Args.Num() < 1)
			{
				UE_LOG(LogMCStressTest, Warning, TEXT("Usage: mc.StressTest S1|S2 [Count=2000] [Seconds=20] [quit] (in a game world)"));
				return;
			}
			const EMCStressScenario Scenario = Args[0].Equals(TEXT("S2"), ESearchCase::IgnoreCase) ? EMCStressScenario::S2_Moving : EMCStressScenario::S1_Idle;
			const int32 Count = Args.Num() > 1 ? FMath::Max(1, FCString::Atoi(*Args[1])) : 2000;
			const float Seconds = Args.Num() > 2 ? FMath::Max(1.f, FCString::Atof(*Args[2])) : 20.f;
			const bool bQuit = Args.ContainsByPredicate([](const FString& Arg) { return Arg.Equals(TEXT("quit"), ESearchCase::IgnoreCase); });
			Subsystem->StartTest(Scenario, Count, Seconds, bQuit);
		}));

	FAutoConsoleCommandWithWorld StopCommand(
		TEXT("mc.StressTest.Stop"),
		TEXT("Stops the running stress test and removes its units"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UMCStressTestSubsystem* Subsystem = FindSubsystem(World))
			{
				Subsystem->StopTest();
			}
		}));

	const TCHAR* ToString(const EMCStressScenario Scenario)
	{
		return Scenario == EMCStressScenario::S2_Moving ? TEXT("S2 (moving)") : TEXT("S1 (idle)");
	}

	const TCHAR* Verdict(const bool bPassed)
	{
		return bPassed ? TEXT("PASS") : TEXT("FAIL");
	}
}

bool UMCStressTestSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UMCStressTestSubsystem::Deinitialize()
{
	UnitIds.Reset();
	FrameTimes.Reset();
	LocationsAtMeasureStart.Reset();
	Super::Deinitialize();
}

TStatId UMCStressTestSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMCStressTestSubsystem, STATGROUP_Tickables);
}

UMCUnitSubsystem* UMCStressTestSubsystem::GetUnitSubsystem() const
{
	return GetWorld() ? GetWorld()->GetSubsystem<UMCUnitSubsystem>() : nullptr;
}

void UMCStressTestSubsystem::StartTest(const EMCStressScenario InScenario, const int32 InCount, const float InMeasureSeconds, const bool bInQuitWhenDone)
{
	StopTest();

	Scenario = InScenario;
	Count = InCount;
	MeasureSeconds = InMeasureSeconds;
	bQuitWhenDone = bInQuitWhenDone;
	UnitTypeName = FName(*UE::MyCrusader::StressTest::UnitType);

	UsedPhysicalBeforeSpawn = FPlatformMemory::GetStats().UsedPhysical;
	SpawnUnits();
	PlaceViewer();

	Phase = EPhase::WarmingUp;
	PhaseElapsed = 0.f;
	if (UMCUnitSubsystem* Units = GetUnitSubsystem())
	{
		UnitIds = TArray<uint32>();
		UE_LOG(LogMCStressTest, Display, TEXT("Stress test %s: %d units of type %s requested (D224: they are born on the following steps), measuring for %.0f s after %.0f s of warm-up"),
			UE::MyCrusader::StressTest::ToString(Scenario), Count, *UnitTypeName.ToString(), MeasureSeconds, UE::MyCrusader::StressTest::WarmUpSeconds);
		return;
	}
	UE_LOG(LogMCStressTest, Error, TEXT("No unit subsystem: nothing spawned"));
}

void UMCStressTestSubsystem::StopTest()
{
	DestroyUnits();
	Phase = EPhase::Idle;
}

void UMCStressTestSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bQuitPending)
	{
		// The units of the run were queued for destruction (D224), applied at the tail of the next fixed step:
		// give the steps the time to do it, then report and leave.
		QuitDelayElapsed += DeltaTime;
		if (QuitDelayElapsed >= UE::MyCrusader::StressTest::QuitDelaySeconds)
		{
			bQuitPending = false;
			if (const UMCUnitSubsystem* Units = GetUnitSubsystem())
			{
				UE_LOG(LogMCStressTest, Display, TEXT("Units left after the run: %d (D224: the queued destructions are applied at the tail of the step)"), Units->GetNumUnits());
			}
			UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, /*bIgnorePlatformRestrictions*/ false);
		}
		return;
	}

	if (Phase == EPhase::Idle)
	{
		return;
	}

	// S2 walks by orders (D229): the units that arrived are given a new destination, warm-up included.
	if (Scenario == EMCStressScenario::S2_Moving)
	{
		KeepUnitsWalking();
	}

	PhaseElapsed += DeltaTime;
	if (Phase == EPhase::WarmingUp)
	{
		if (PhaseElapsed >= UE::MyCrusader::StressTest::WarmUpSeconds)
		{
			BeginMeasure();
		}
		return;
	}

	FrameTimes.Add(DeltaTime);
	GameThreadMsSum += FPlatformTime::ToMilliseconds(GGameThreadTime);
	RenderThreadMsSum += FPlatformTime::ToMilliseconds(GRenderThreadTime);
	GpuMsSum += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());

	// One capture per run, at half of the measure. Skipped while the window is minimized: the capture would
	// be lost, and its placeholder back buffer trips a D3D12 assert (hence no retry later in the run).
	if (!bScreenshotTaken && PhaseElapsed >= MeasureSeconds * 0.5f)
	{
		bScreenshotTaken = true;
		if (UE::MyCrusader::StressTest::IsGameWindowMinimized(GetWorld()))
		{
			UE_LOG(LogMCStressTest, Warning, TEXT("No capture for this run: the game window is minimized"));
		}
		else
		{
			FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("StressTest_%s.png"), Scenario == EMCStressScenario::S2_Moving ? TEXT("S2") : TEXT("S1")),
				/*bInShowUI*/ false, /*bAddFilenameSuffix*/ false);
		}
	}

	if (PhaseElapsed >= MeasureSeconds)
	{
		FinishMeasure();
	}
}

void UMCStressTestSubsystem::SpawnUnits()
{
	UMCUnitSubsystem* Units = GetUnitSubsystem();
	if (Units == nullptr)
	{
		return;
	}

	// The authority creates the units, spread over the following steps by the per-step budget (D224). The S1
	// units are born standing still, which is what an immobile unit means (D220).
	Units->RequestUnits(FMCUnitSpawnRequest{ UnitTypeName, Count, FVector::ZeroVector, UE::MyCrusader::StressTest::SpawnRadius });
}

void UMCStressTestSubsystem::DestroyUnits()
{
	if (UnitIds.Num() > 0)
	{
		if (UMCUnitSubsystem* Units = GetUnitSubsystem())
		{
			// Destruction is deferred to the end of the step (D224): the units disappear on the next step.
			Units->RequestDestroyUnits(UnitIds);
		}
	}
	UnitIds.Reset();
}

void UMCStressTestSubsystem::KeepUnitsWalking()
{
	using namespace UE::MyCrusader::StressTest;

	UWorld* World = GetWorld();
	UMCUnitSubsystem* Units = GetUnitSubsystem();
	UMCCommandSubsystem* Commands = World ? World->GetSubsystem<UMCCommandSubsystem>() : nullptr;
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UNavigationSystemV1* NavSystem = World ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
	if (Units == nullptr || Commands == nullptr || EntitySubsystem == nullptr || NavSystem == nullptr)
	{
		return;
	}

	// Orders sit in the queue until the next fixed step applies them (D216, D219): one pass per step, otherwise
	// a unit still standing would be given a new destination every frame until the step walks it.
	const UMCSimulationSubsystem* Simulation = World->GetSubsystem<UMCSimulationSubsystem>();
	const int64 StepIndex = Simulation ? Simulation->GetStepIndex() : 0;
	if (StepIndex == LastOrderStepIndex)
	{
		return;
	}
	LastOrderStepIndex = StepIndex;

	// While the births are served (D224), refresh the identifiers of this run: afterwards the list is stable
	// and the measure compares the same units from start to end.
	if (UnitIds.Num() < Count)
	{
		Units->GetUnitIdsOfType(UnitTypeName, UnitIds);
	}

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	int32 OrdersLeft = MaxOrdersPerFrame;
	for (const uint32 UnitId : UnitIds)
	{
		if (OrdersLeft <= 0)
		{
			break;
		}

		const FMassEntityHandle Entity = Units->FindEntity(UnitId);
		const FMCUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMCUnitStateFragment>(Entity);
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		if (State == nullptr || Transform == nullptr || State->Activity != EMCUnitActivity::Idle)
		{
			continue;
		}

		FNavLocation Destination;
		if (NavSystem->GetRandomPointInNavigableRadius(Transform->GetTransform().GetLocation(), OrderRadius, Destination))
		{
			// An order goes through the queue (D216, D226) and its destination is brought back to the navmesh
			// by the command subsystem (D228) — the test does not walk the unit itself.
			Commands->IssueMoveOrder(MakeArrayView(&UnitId, 1), Destination.Location);
			--OrdersLeft;
			++NumOrdersIssued;
		}
	}
}

TArray<FMassEntityHandle> UMCStressTestSubsystem::GetUnitEntities() const
{
	TArray<FMassEntityHandle> Entities;
	UMCUnitSubsystem* Units = GetUnitSubsystem();
	if (Units == nullptr)
	{
		return Entities;
	}
	Entities.Reserve(UnitIds.Num());
	for (const uint32 UnitId : UnitIds)
	{
		Entities.Add(Units->FindEntity(UnitId));
	}
	return Entities;
}

TArray<FVector> UMCStressTestSubsystem::GetUnitLocations() const
{
	TArray<FVector> Locations;
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (EntitySubsystem == nullptr)
	{
		return Locations;
	}

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	const TArray<FMassEntityHandle> Entities = GetUnitEntities();
	Locations.Reserve(Entities.Num());
	for (const FMassEntityHandle Entity : Entities)
	{
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		Locations.Add(Transform ? Transform->GetTransform().GetLocation() : FVector::ZeroVector);
	}
	return Locations;
}

void UMCStressTestSubsystem::PlaceViewer() const
{
	// High three-quarter view over the spawn area: all units on screen (D218).
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}
	const FVector ViewLocation(-11000.f, 0.f, 8000.f);
	const FRotator ViewRotation = (FVector::ZeroVector - ViewLocation).Rotation();
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		Pawn->SetActorLocation(ViewLocation, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);
	}
	PlayerController->SetControlRotation(ViewRotation);
}

void UMCStressTestSubsystem::BeginMeasure()
{
	Phase = EPhase::Measuring;
	PhaseElapsed = 0.f;
	FrameTimes.Reset();
	GameThreadMsSum = RenderThreadMsSum = GpuMsSum = 0.0;
	bScreenshotTaken = false;
	NumOrdersIssued = 0;
	LastOrderStepIndex = -1;

	if (UMCUnitSubsystem* Units = GetUnitSubsystem())
	{
		Units->GetUnitIdsOfType(UnitTypeName, UnitIds);
	}
	if (UnitIds.Num() < Count)
	{
		UE_LOG(LogMCStressTest, Warning, TEXT("Only %d of the %d requested units exist at the start of the measure"), UnitIds.Num(), Count);
	}
	LocationsAtMeasureStart = GetUnitLocations();
	if (UMCSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UMCSimulationSubsystem>())
	{
		Simulation->ResetStepStats();
	}
}

void UMCStressTestSubsystem::LogRepresentationSummary() const
{
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (EntitySubsystem == nullptr)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TMap<FString, int32> Histogram;
	for (const FMassEntityHandle Entity : GetUnitEntities())
	{
		const FMassRepresentationFragment* Representation = EntityManager.GetFragmentDataPtr<FMassRepresentationFragment>(Entity);
		const FMassRepresentationLODFragment* LOD = EntityManager.GetFragmentDataPtr<FMassRepresentationLODFragment>(Entity);
		const FString Key = FString::Printf(TEXT("%s / LOD %d"),
			Representation ? *UEnum::GetValueAsString(Representation->CurrentRepresentation) : TEXT("no representation"),
			LOD ? static_cast<int32>(LOD->LOD) : -1);
		++Histogram.FindOrAdd(Key);
	}
	for (const TPair<FString, int32>& Entry : Histogram)
	{
		UE_LOG(LogMCStressTest, Display, TEXT("  Representation: %s x %d"), *Entry.Key, Entry.Value);
	}
}

void UMCStressTestSubsystem::LogUnitStateSummary() const
{
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (EntitySubsystem == nullptr)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TMap<FString, int32> MoveActions;
	TMap<FString, int32> UnitStates;
	for (const FMassEntityHandle Entity : GetUnitEntities())
	{
		const FMassMoveTargetFragment* MoveTarget = EntityManager.GetFragmentDataPtr<FMassMoveTargetFragment>(Entity);
		const FString MoveAction = MoveTarget ? UEnum::GetDisplayValueAsText(MoveTarget->GetCurrentAction()).ToString() : TEXT("no move target");
		++MoveActions.FindOrAdd(MoveAction);

		const FMCUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMCUnitStateFragment>(Entity);
		const FString UnitState = State ? UEnum::GetDisplayValueAsText(State->Activity).ToString() : TEXT("no unit state");
		++UnitStates.FindOrAdd(UnitState);
	}
	for (const TPair<FString, int32>& Entry : MoveActions)
	{
		UE_LOG(LogMCStressTest, Display, TEXT("  Move action: %s x %d"), *Entry.Key, Entry.Value);
	}
	for (const TPair<FString, int32>& Entry : UnitStates)
	{
		UE_LOG(LogMCStressTest, Display, TEXT("  Unit state: %s x %d"), *Entry.Key, Entry.Value);
	}
}

void UMCStressTestSubsystem::LogUnitRegistrySummary() const
{
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UMCUnitSubsystem* Units = GetUnitSubsystem();
	if (EntitySubsystem == nullptr || Units == nullptr)
	{
		return;
	}

	// The identifier of a unit is its own, not Mass' handle (D223): the table must agree with the fragments.
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	int32 NumMismatched = 0;
	for (const uint32 UnitId : UnitIds)
	{
		const FMCUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FMCUnitStateFragment>(Units->FindEntity(UnitId));
		if (State == nullptr || State->UnitId != UnitId)
		{
			++NumMismatched;
		}
	}
	UE_LOG(LogMCStressTest, Display, TEXT("  Unit registry (D223/D224): %d units of %d requested, %d births waiting, identifier table checked on %d units, %d mismatch"),
		Units->GetNumUnits(), Count, Units->GetNumPendingBirths(), UnitIds.Num(), NumMismatched);
}

void UMCStressTestSubsystem::FinishMeasure()
{
	using namespace UE::MyCrusader::StressTest;

	Phase = EPhase::Idle;

	double TotalFrameSeconds = 0.0;
	for (const float FrameTime : FrameTimes)
	{
		TotalFrameSeconds += FrameTime;
	}
	const double AverageFps = TotalFrameSeconds > 0.0 ? FrameTimes.Num() / TotalFrameSeconds : 0.0;
	TArray<float> SortedFrameTimes = FrameTimes;
	SortedFrameTimes.Sort();
	const float Percentile99FrameTime = SortedFrameTimes.Num() > 0 ? SortedFrameTimes[FMath::Min(SortedFrameTimes.Num() - 1, FMath::FloorToInt(SortedFrameTimes.Num() * 0.99f))] : 0.f;
	const double OnePercentLowFps = Percentile99FrameTime > 0.f ? 1.0 / Percentile99FrameTime : 0.0;

	FMCSimulationStepStats StepStats;
	if (const UMCSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UMCSimulationSubsystem>())
	{
		StepStats = Simulation->GetStepStats();
	}
	const double UsedPhysicalMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0 * 1024.0);
	const double UnitsDeltaMB = (static_cast<double>(FPlatformMemory::GetStats().UsedPhysical) - static_cast<double>(UsedPhysicalBeforeSpawn)) / (1024.0 * 1024.0);

	const double NumFrames = FMath::Max(1, FrameTimes.Num());
	const double GameThreadMs = GameThreadMsSum / NumFrames;
	const double RenderThreadMs = RenderThreadMsSum / NumFrames;
	const double GpuMs = GpuMsSum / NumFrames;

	const TArray<FVector> LocationsAtMeasureEnd = GetUnitLocations();
	double TotalDisplacement = 0.0;
	for (int32 Index = 0; Index < FMath::Min(LocationsAtMeasureStart.Num(), LocationsAtMeasureEnd.Num()); ++Index)
	{
		TotalDisplacement += FVector::Dist(LocationsAtMeasureStart[Index], LocationsAtMeasureEnd[Index]);
	}
	const double AverageDisplacementM = LocationsAtMeasureEnd.Num() > 0 ? TotalDisplacement / LocationsAtMeasureEnd.Num() / 100.0 : 0.0;

	const bool bSimulationPassed = StepStats.GetAverageMs() <= MaxSimulationMs;
	const bool bRenderPassed = AverageFps >= MinFramesPerSecond;

	const FString Report = FString::Printf(
		TEXT("Stress test %s, %d units of type %s, %.0f s\n")
		TEXT("  Simulation: %.3f ms average per step (max %.3f ms, %d steps, %d frames dropped time) - threshold %.1f ms: %s\n")
		TEXT("  Rendering: %.1f fps average (1%% low %.1f fps, %d frames) - threshold %.0f fps: %s\n")
		TEXT("  Frame: game thread %.2f ms, render thread %.2f ms, GPU %.2f ms\n")
		TEXT("  Memory: %.0f MB used (%+.0f MB since spawn)\n")
		TEXT("  Movement: units moved %.1f m on average during the measure, %d move orders issued (D229)"),
		ToString(Scenario), UnitIds.Num(), *UnitTypeName.ToString(), MeasureSeconds,
		StepStats.GetAverageMs(), StepStats.MaxMs, StepStats.NumSteps, StepStats.NumDroppedFrames, MaxSimulationMs, Verdict(bSimulationPassed),
		AverageFps, OnePercentLowFps, FrameTimes.Num(), MinFramesPerSecond, Verdict(bRenderPassed),
		GameThreadMs, RenderThreadMs, GpuMs,
		UsedPhysicalMB, UnitsDeltaMB,
		AverageDisplacementM, NumOrdersIssued);

	UE_LOG(LogMCStressTest, Display, TEXT("%s"), *Report);
	for (const FMCSimulationProcessorStats& Processor : StepStats.Processors)
	{
		const FString Throttled = Processor.StepInterval > 1 ? FString::Printf(TEXT(" (1 step out of %d)"), Processor.StepInterval) : FString();
		UE_LOG(LogMCStressTest, Display, TEXT("  Step breakdown: %s %.3f ms%s"), *Processor.Name, Processor.GetAverageMs(), *Throttled);
	}
	// Births and destructions are part of the step but not processors (D224): worth a line only in a run that
	// creates or destroys units while measuring.
	if (StepStats.NumUnitsBorn > 0 || StepStats.NumUnitsDestroyed > 0)
	{
		UE_LOG(LogMCStressTest, Display, TEXT("  Step breakdown: unit births %.3f ms (%d born), unit destructions %.3f ms (%d destroyed)"),
			StepStats.GetAverageBirthMs(), StepStats.NumUnitsBorn, StepStats.GetAverageDestructionMs(), StepStats.NumUnitsDestroyed);
	}
	LogRepresentationSummary();
	LogUnitStateSummary();
	LogUnitRegistrySummary();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 30.f, bSimulationPassed && bRenderPassed ? FColor::Green : FColor::Red, Report);
	}

	if (bQuitWhenDone)
	{
		// Leave the world clean: the units are queued for destruction and the step applies it, then Tick quits.
		StopTest();
		bQuitPending = true;
		QuitDelayElapsed = 0.f;
	}
}
