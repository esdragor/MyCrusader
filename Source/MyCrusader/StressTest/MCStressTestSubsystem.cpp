// Copyright Epic Games, Inc. All Rights Reserved.

#include "StressTest/MCStressTestSubsystem.h"

#include "Avoidance/MassAvoidanceTrait.h"
#include "Avoidance/MassNavigationObstacleTrait.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "DynamicRHI.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "MassLODTrait.h"
#include "MassMovableVisualizationTrait.h"
#include "MassNavMeshNavigationTrait.h"
#include "MassSpawnerSubsystem.h"
#include "MassSpawnerTypes.h"
#include "MassSpawnLocationProcessor.h"
#include "MassCommonFragments.h"
#include "MassRepresentationFragments.h"
#include "Movement/MassMovementTrait.h"
#include "MyCrusaderLog.h"
#include "NavigationSystem.h"
#include "RenderTimer.h"
#include "Simulation/MCLightUnitTrait.h"
#include "Simulation/MCSimulationFragments.h"
#include "Simulation/MCSimulationSubsystem.h"
#include "SmoothOrientation/MassSmoothOrientationTrait.h"
#include "Steering/MassSteeringTrait.h"
#include "UnrealClient.h"

namespace UE::MyCrusader::StressTest
{
	constexpr float WarmUpSeconds = 3.f;
	/** Units appear on the navmesh within this radius of the map centre. */
	float SpawnRadius = 8000.f;
	FAutoConsoleVariableRef CVarSpawnRadius(TEXT("mc.StressTest.SpawnRadius"), SpawnRadius,
		TEXT("Radius (cm) around the map centre where stress test units appear: sets the crowd density"));
	const TCHAR* MeshPath = TEXT("/Game/Units/Mannequin/SM_Mannequin.SM_Mannequin");

	// D218 thresholds.
	constexpr double MaxSimulationMs = 3.0;
	constexpr double MinFramesPerSecond = 144.0;

	UMCStressTestSubsystem* FindSubsystem(UWorld* World)
	{
		return World ? World->GetSubsystem<UMCStressTestSubsystem>() : nullptr;
	}

	FAutoConsoleCommandWithWorldAndArgs StartCommand(
		TEXT("mc.StressTest"),
		TEXT("mc.StressTest S1|S2 [Count=2000] [Seconds=20] [quit] - S1: units standing still, S2: units walking on the navmesh (D190, D218)"),
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
	Units.Reset();
	IdleConfig.Reset();
	WanderConfig.Reset();
	Traits.Reset();
	Super::Deinitialize();
}

TStatId UMCStressTestSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMCStressTestSubsystem, STATGROUP_Tickables);
}

void UMCStressTestSubsystem::StartTest(const EMCStressScenario InScenario, const int32 InCount, const float InMeasureSeconds, const bool bInQuitWhenDone)
{
	StopTest();

	Scenario = InScenario;
	Count = InCount;
	MeasureSeconds = InMeasureSeconds;
	bQuitWhenDone = bInQuitWhenDone;

	UsedPhysicalBeforeSpawn = FPlatformMemory::GetStats().UsedPhysical;
	SpawnUnits();
	PlaceViewer();

	Phase = EPhase::WarmingUp;
	PhaseElapsed = 0.f;
	UE_LOG(LogMCStressTest, Display, TEXT("Stress test %s: %d units spawned, measuring for %.0f s after %.0f s of warm-up"),
		UE::MyCrusader::StressTest::ToString(Scenario), Units.Num(), MeasureSeconds, UE::MyCrusader::StressTest::WarmUpSeconds);
}

void UMCStressTestSubsystem::StopTest()
{
	DestroyUnits();
	Phase = EPhase::Idle;
}

void UMCStressTestSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Phase == EPhase::Idle)
	{
		return;
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

	if (!bScreenshotTaken && PhaseElapsed >= MeasureSeconds * 0.5f)
	{
		bScreenshotTaken = true;
		FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("StressTest_%s.png"), Scenario == EMCStressScenario::S2_Moving ? TEXT("S2") : TEXT("S1")),
			/*bInShowUI*/ false, /*bAddFilenameSuffix*/ false);
	}

	if (PhaseElapsed >= MeasureSeconds)
	{
		FinishMeasure();
	}
}

const FMassEntityConfig& UMCStressTestSubsystem::GetOrCreateConfig(const bool bWander)
{
	TSharedPtr<FMassEntityConfig>& Config = bWander ? WanderConfig : IdleConfig;
	if (Config.IsValid())
	{
		return *Config;
	}

	Config = MakeShared<FMassEntityConfig>(*this);

	UMCLightUnitTrait* LightUnitTrait = NewObject<UMCLightUnitTrait>(this);
	LightUnitTrait->bWander = bWander;

	UMassMovableVisualizationTrait* VisualizationTrait = NewObject<UMassMovableVisualizationTrait>(this);
	FMassStaticMeshInstanceVisualizationMeshDesc& MeshDesc = VisualizationTrait->StaticMeshInstanceDesc.Meshes.AddDefaulted_GetRef();
	MeshDesc.Mesh = LoadObject<UStaticMesh>(nullptr, UE::MyCrusader::StressTest::MeshPath);
	MeshDesc.bCastShadows = true;
	// The mannequin faces +Y, Mass moves along +X.
	VisualizationTrait->StaticMeshInstanceDesc.bUseTransformOffset = true;
	VisualizationTrait->StaticMeshInstanceDesc.TransformOffset = FTransform(FRotator(0.f, -90.f, 0.f));
	// Instances at every distance: D218 measures 2 000 units on screen.
	for (int32 LODIndex = 0; LODIndex < EMassLOD::Max; ++LODIndex)
	{
		VisualizationTrait->Params.LODRepresentation[LODIndex] = LODIndex == EMassLOD::Off ? EMassRepresentationType::None : EMassRepresentationType::StaticMeshInstance;
		VisualizationTrait->LODParams.LODMaxCount[LODIndex] = 5000; // above the 2 000 floor (D190)
	}
	VisualizationTrait->LODParams.BaseLODDistance[EMassLOD::Off] = 1000000.f;
	VisualizationTrait->LODParams.VisibleLODDistance[EMassLOD::Off] = 1000000.f;

	const TArray<UMassEntityTraitBase*> ConfigTraits = { LightUnitTrait,
		NewObject<UMassMovementTrait>(this),
		NewObject<UMassSteeringTrait>(this),
		NewObject<UMassObstacleAvoidanceTrait>(this),
		NewObject<UMassNavigationObstacleTrait>(this),
		NewObject<UMassSmoothOrientationTrait>(this),
		NewObject<UMassNavMeshNavigationTrait>(this),
		NewObject<UMassLODCollectorTrait>(this),
		VisualizationTrait };
	for (UMassEntityTraitBase* Trait : ConfigTraits)
	{
		Config->AddTrait(*Trait);
		Traits.Add(Trait);
	}
	return *Config;
}

void UMCStressTestSubsystem::SpawnUnits()
{
	UWorld* World = GetWorld();
	UMassSpawnerSubsystem* Spawner = World->GetSubsystem<UMassSpawnerSubsystem>();
	UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!Spawner || !EntitySubsystem || !NavSystem)
	{
		UE_LOG(LogMCStressTest, Error, TEXT("Mass or navigation missing: nothing spawned"));
		return;
	}

	const FMassEntityTemplate& Template = GetOrCreateConfig(Scenario == EMCStressScenario::S2_Moving).GetOrCreateEntityTemplate(*World);

	FMassTransformsSpawnData SpawnData;
	SpawnData.bRandomize = false;
	SpawnData.Transforms.Reserve(Count);
	FRandomStream Random(/*Seed*/ 1066);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		FNavLocation Location;
		if (!NavSystem->GetRandomPointInNavigableRadius(FVector::ZeroVector, UE::MyCrusader::StressTest::SpawnRadius, Location))
		{
			continue;
		}
		SpawnData.Transforms.Emplace(FRotator(0.f, Random.FRandRange(-180.f, 180.f), 0.f), Location.Location);
	}

	{
		TSharedPtr<FMassEntityManager::FEntityCreationContext> CreationContext = Spawner->SpawnEntities(
			Template.GetTemplateID(), SpawnData.Transforms.Num(), FConstStructView::Make(SpawnData), UMassSpawnLocationProcessor::StaticClass(), Units);
	}

	// Start interpolating from the spawn location, not from the origin.
	FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
	for (const FMassEntityHandle Unit : Units)
	{
		EntityManager.GetFragmentDataChecked<FMCPreviousTransformFragment>(Unit).Transform = EntityManager.GetFragmentDataChecked<FTransformFragment>(Unit).GetTransform();
	}
}

void UMCStressTestSubsystem::DestroyUnits()
{
	if (Units.Num() > 0)
	{
		if (UMassSpawnerSubsystem* Spawner = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>())
		{
			Spawner->DestroyEntities(Units);
		}
	}
	Units.Reset();
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
	LocationsAtMeasureStart = GetUnitLocations();
	if (UMCSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UMCSimulationSubsystem>())
	{
		Simulation->ResetStepStats();
	}
}

TArray<FVector> UMCStressTestSubsystem::GetUnitLocations() const
{
	TArray<FVector> Locations;
	if (const UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>())
	{
		const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
		Locations.Reserve(Units.Num());
		for (const FMassEntityHandle Unit : Units)
		{
			const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Unit);
			Locations.Add(Transform ? Transform->GetTransform().GetLocation() : FVector::ZeroVector);
		}
	}
	return Locations;
}

void UMCStressTestSubsystem::LogRepresentationSummary() const
{
	const UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem)
	{
		return;
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TMap<FString, int32> Histogram;
	for (const FMassEntityHandle Unit : Units)
	{
		const FMassRepresentationFragment* Representation = EntityManager.GetFragmentDataPtr<FMassRepresentationFragment>(Unit);
		const FMassRepresentationLODFragment* LOD = EntityManager.GetFragmentDataPtr<FMassRepresentationLODFragment>(Unit);
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
		TEXT("Stress test %s, %d units, %.0f s\n")
		TEXT("  Simulation: %.3f ms average per step (max %.3f ms, %d steps, %d frames dropped time) - threshold %.1f ms: %s\n")
		TEXT("  Rendering: %.1f fps average (1%% low %.1f fps, %d frames) - threshold %.0f fps: %s\n")
		TEXT("  Frame: game thread %.2f ms, render thread %.2f ms, GPU %.2f ms\n")
		TEXT("  Memory: %.0f MB used (%+.0f MB since spawn)\n")
		TEXT("  Movement: units moved %.1f m on average during the measure"),
		ToString(Scenario), Units.Num(), MeasureSeconds,
		StepStats.GetAverageMs(), StepStats.MaxMs, StepStats.NumSteps, StepStats.NumDroppedFrames, MaxSimulationMs, Verdict(bSimulationPassed),
		AverageFps, OnePercentLowFps, FrameTimes.Num(), MinFramesPerSecond, Verdict(bRenderPassed),
		GameThreadMs, RenderThreadMs, GpuMs,
		UsedPhysicalMB, UnitsDeltaMB,
		AverageDisplacementM);

	UE_LOG(LogMCStressTest, Display, TEXT("%s"), *Report);
	for (const TPair<FString, double>& Processor : StepStats.ProcessorTotalMs)
	{
		UE_LOG(LogMCStressTest, Display, TEXT("  Step breakdown: %s %.3f ms"), *Processor.Key, StepStats.NumSteps > 0 ? Processor.Value / StepStats.NumSteps : 0.0);
	}
	LogRepresentationSummary();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 30.f, bSimulationPassed && bRenderPassed ? FColor::Green : FColor::Red, Report);
	}

	if (bQuitWhenDone)
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, /*bIgnorePlatformRestrictions*/ false);
	}
}
