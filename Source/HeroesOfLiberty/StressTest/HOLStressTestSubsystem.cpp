// Copyright Epic Games, Inc. All Rights Reserved.

#include "StressTest/HOLStressTestSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLUnitOrderProcessor.h"
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
#include "HeroesOfLibertyLog.h"
#include "NavigationSystem.h"
#include "RenderTimer.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "RHIStats.h"
#include "Simulation/HOLAvoidanceHoldProcessors.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationProcessors.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "UnrealClient.h"
#include "Widgets/SWindow.h"

namespace UE::HeroesOfLiberty::StressTest
{
	constexpr float WarmUpSeconds = 3.f;
	/**
	 * S7 (D299): the warm-up lets the Peasants be born, walk to their trees and settle in their round trips, so the
	 * measure sees gathering, not the first walk.
	 */
	constexpr float GatheringWarmUpSeconds = 25.f;

	/** S7: the centres stand this far from the middle of the map (cm), one per player around it. */
	constexpr double GatheringCentreRadius = 5000.;
	/** S7: rings of trees behind each centre, from this distance from the middle (cm), this far apart. */
	constexpr double GatheringFirstRing = 6000.;
	constexpr int32 GatheringRings = 9;
	/** S7: lone trees this far apart (cm) along a ring, within this half-width (cm) of the player's direction. */
	constexpr double GatheringTreeSpacing = 400.;
	constexpr double GatheringHalfWidth = 1600.;
	/** S7: Peasants are born this far from the middle (cm), towards their centre, within this radius (cm). */
	constexpr double GatheringSpawnRadius = 3800.;
	constexpr float GatheringSpawnDisc = 1000.f;
	/** S7: Peasants sent to one tree. */
	constexpr int32 GatherersPerTree = 4;
	/**
	 * S7: lumber camps in each player's trees (stand-ins for the camps of the construction step, D83), this far from the
	 * middle (cm), this far apart (cm) across: about 60 gatherers per deposit, as many as the camps of a real economy
	 * see at most (with the centre alone, 250 crowded one side of it).
	 */
	constexpr double GatheringCampRing = 7600.;
	constexpr double GatheringCampSpacing = 1100.;
	constexpr int32 GatheringCamps = 3;
	/** S7: no tree this close (cm) to a camp's centre. */
	constexpr double GatheringCampClearance = 550.;

	/** S7: wood in each tree of the run (the table's 100 would leave the field bare before the end of the measure). */
	int32 TreeAmount = 400;
	FAutoConsoleVariableRef CVarTreeAmount(TEXT("hol.StressTest.TreeAmount"), TreeAmount,
		TEXT("S7: wood in each tree the stress test places (the source table says 100; 400 keeps the field standing through the measure)"));
	/** Time granted to the last fixed steps after the run asked to quit, so the units are destroyed (D224). */
	constexpr float QuitDelaySeconds = 0.5f;
	/**
	 * Camera of the measure (D236), 2 000 units on screen in both: "Wide" frames the whole crowd from ~70 m,
	 * "Game" looks from an RTS camera height on a tighter crowd (the big melee seen up close).
	 */
	FString View = TEXT("Wide");
	FAutoConsoleVariableRef CVarView(TEXT("hol.StressTest.View"), View,
		TEXT("Camera of the stress test: Wide (whole crowd, ~70 m) or Game (RTS camera height on a tighter crowd), D236"));

	bool IsGameView()
	{
		return View.Equals(TEXT("Game"), ESearchCase::IgnoreCase);
	}

	/** Units appear on the navmesh within this radius of the map centre; negative = the view's crowd (D236). */
	float SpawnRadius = -1.f;
	FAutoConsoleVariableRef CVarSpawnRadius(TEXT("hol.StressTest.SpawnRadius"), SpawnRadius,
		TEXT("Radius (cm) around the map centre where stress test units appear (crowd density), negative = the view's crowd: Wide 4500, Game 3000 (D236)"));

	float GetSpawnRadius()
	{
		// D236: the wide crowd shrank from 80 to 45 m so 2 000 units stay in frame with the camera at ~70 m.
		return SpawnRadius >= 0.f ? SpawnRadius : IsGameView() ? 3000.f : 4500.f;
	}

	/** Type of the units of a run: a row of the unit type table (D221). */
	FString UnitType = TEXT("LightInfantry");
	FAutoConsoleVariableRef CVarUnitType(TEXT("hol.StressTest.UnitType"), UnitType,
		TEXT("Row of /Game/Units/UnitTypes used by the stress test"));

	/**
	 * Render scale of the measure (D245 guard): the Settings screen of the menu saves its own render scale, which the
	 * engine applies at start-up and "scalability" leaves alone. A run forces this one (sg.ResolutionQuality, % of the
	 * screen resolution), the value of every measure until the menu existed (GameUserSettings.ini of the test machine,
	 * 2026-10-13). Negative = keep the saved setting.
	 */
	float ResolutionQuality = 87.f;
	FAutoConsoleVariableRef CVarResolutionQuality(TEXT("hol.StressTest.ResolutionQuality"), ResolutionQuality,
		TEXT("Render scale (sg.ResolutionQuality, %) forced by a stress test run over the saved settings; negative = keep the saved one (D245)"));

	/** Forces what the menu can change and the usual command line does not (D245). */
	void ForceDisplaySettings()
	{
		if (ResolutionQuality >= 0.f)
		{
			if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ResolutionQuality")))
			{
				// Above the saved settings (ECVF_SetByGameSetting), so the menu cannot change a measure.
				CVar->Set(ResolutionQuality, ECVF_SetByCode);
			}
		}
	}

	/** What the menu can change, as the run saw it, for the report (D245). */
	FString DescribeDisplaySettings()
	{
		const auto Value = [](const FString& Name)
		{
			const IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
			return CVar ? CVar->GetString() : FString(TEXT("?"));
		};
		FString Groups;
		for (const TCHAR* Group : { TEXT("ViewDistance"), TEXT("AntiAliasing"), TEXT("Shadow"), TEXT("GlobalIllumination"), TEXT("Reflection"),
			TEXT("PostProcess"), TEXT("Texture"), TEXT("Effects"), TEXT("Foliage"), TEXT("Shading"), TEXT("Landscape") })
		{
			Groups += FString::Printf(TEXT(" %s %s"), Group, *Value(FString::Printf(TEXT("sg.%sQuality"), Group)));
		}
		FIntPoint Viewport = FIntPoint::ZeroValue;
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
		{
			Viewport = GEngine->GameViewport->Viewport->GetSizeXY();
		}
		return FString::Printf(TEXT("render scale %s %% (r.ScreenPercentage %s), vsync %s, frame limit %s, viewport %dx%d, quality groups:%s"),
			*Value(TEXT("sg.ResolutionQuality")), *Value(TEXT("r.ScreenPercentage")), *Value(TEXT("r.VSync")), *Value(TEXT("t.MaxFPS")),
			Viewport.X, Viewport.Y, *Groups);
	}

	/** Players the units are shared between, so the drawn colours are the ones of a full match (D01, D233). */
	int32 Players = 8;
	FAutoConsoleVariableRef CVarPlayers(TEXT("hol.StressTest.Players"), Players,
		TEXT("Players the stress test units are shared between (owner colours, D233)"));

	/** S2 walks by move orders (D229): a unit is ordered a new destination within this radius of itself. */
	float OrderRadius = 3000.f;
	FAutoConsoleVariableRef CVarOrderRadius(TEXT("hol.StressTest.OrderRadius"), OrderRadius,
		TEXT("Radius (cm) within which S2 picks the destination of a new move order"));

	/** Orders issued per step at most (the name predates the step pass): 2 000 births must not all be ordered at once. */
	int32 MaxOrdersPerFrame = 200;
	FAutoConsoleVariableRef CVarMaxOrdersPerFrame(TEXT("hol.StressTest.MaxOrdersPerFrame"), MaxOrdersPerFrame,
		TEXT("Move orders the stress test issues per fixed step at most"));

	/** The two units of the crossing scenario start this far from the origin they both walk through (cm). */
	float CrossingHalfDistance = 1500.f;
	FAutoConsoleVariableRef CVarCrossingHalfDistance(TEXT("hol.StressTest.CrossingHalfDistance"), CrossingHalfDistance,
		TEXT("Distance (cm) between a crossing unit and the origin it walks through"));

	/** Angle between the two crossing paths: 0 = head-on, 90 = at right angles, beyond 90 = converging (D230). */
	float CrossingAngle = 0.f;
	FAutoConsoleVariableRef CVarCrossingAngle(TEXT("hol.StressTest.CrossingAngle"), CrossingAngle,
		TEXT("Crossing scenario: angle in degrees between the two units' paths, 0 = head-on, 90 = at right angles, at most 150"));
	constexpr float MaxCrossingAngle = 150.f;

	float GetCrossingAngle()
	{
		return FMath::Clamp(CrossingAngle, 0.f, MaxCrossingAngle);
	}

	// D218 thresholds.
	constexpr double MaxSimulationMs = 3.0;
	constexpr double MinFramesPerSecond = 144.0;

	UHOLStressTestSubsystem* FindSubsystem(UWorld* World)
	{
		return World ? World->GetSubsystem<UHOLStressTestSubsystem>() : nullptr;
	}

	/** A minimized window keeps a placeholder back buffer, and capturing it crashes the D3D12 RHI. */
	bool IsGameWindowMinimized(const UWorld* World)
	{
		const UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr;
		const TSharedPtr<SWindow> Window = GameViewport ? GameViewport->GetWindow() : nullptr;
		return Window.IsValid() && Window->IsWindowMinimized();
	}

	FAutoConsoleCommandWithWorldAndArgs StartCommand(
		TEXT("hol.StressTest"),
		TEXT("hol.StressTest S1|S2|C|S7 [Count=2000] [Seconds=20] [quit] - S1: units standing still, S2: units walking by move orders (D190, D218, D229), C: two units crossing at hol.StressTest.CrossingAngle (avoidance check), S7: Peasants gathering wood (D299)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLStressTestSubsystem* Subsystem = FindSubsystem(World);
			if (!Subsystem || Args.Num() < 1)
			{
				UE_LOG(LogHOLStressTest, Warning, TEXT("Usage: hol.StressTest S1|S2|C|S7 [Count=2000] [Seconds=20] [quit] (in a game world)"));
				return;
			}
			EHOLStressScenario Scenario = EHOLStressScenario::S1_Idle;
			if (Args[0].Equals(TEXT("S2"), ESearchCase::IgnoreCase))
			{
				Scenario = EHOLStressScenario::S2_Moving;
			}
			else if (Args[0].Equals(TEXT("C"), ESearchCase::IgnoreCase) || Args[0].Equals(TEXT("Crossing"), ESearchCase::IgnoreCase))
			{
				Scenario = EHOLStressScenario::Crossing;
			}
			else if (Args[0].Equals(TEXT("S7"), ESearchCase::IgnoreCase))
			{
				Scenario = EHOLStressScenario::S7_Gathering;
			}
			// The crossing needs exactly two units, at two known places.
			const int32 Count = Scenario == EHOLStressScenario::Crossing
				? 2
				: (Args.Num() > 1 ? FMath::Max(1, FCString::Atoi(*Args[1])) : 2000);
			const float Seconds = Args.Num() > 2 ? FMath::Max(1.f, FCString::Atof(*Args[2])) : 20.f;
			const bool bQuit = Args.ContainsByPredicate([](const FString& Arg) { return Arg.Equals(TEXT("quit"), ESearchCase::IgnoreCase); });
			Subsystem->StartTest(Scenario, Count, Seconds, bQuit);
		}));

	FAutoConsoleCommandWithWorld StopCommand(
		TEXT("hol.StressTest.Stop"),
		TEXT("Stops the running stress test and removes its units"),
		FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
		{
			if (UHOLStressTestSubsystem* Subsystem = FindSubsystem(World))
			{
				Subsystem->StopTest();
			}
		}));

	const TCHAR* ToString(const EHOLStressScenario Scenario)
	{
		switch (Scenario)
		{
		case EHOLStressScenario::S2_Moving: return TEXT("S2 (moving)");
		case EHOLStressScenario::Crossing: return TEXT("C (crossing)");
		case EHOLStressScenario::S7_Gathering: return TEXT("S7 (gathering)");
		default: return TEXT("S1 (idle)");
		}
	}

	const TCHAR* Verdict(const bool bPassed)
	{
		return bPassed ? TEXT("PASS") : TEXT("FAIL");
	}
}

bool UHOLStressTestSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLStressTestSubsystem::Deinitialize()
{
	if (UHOLSimulationSubsystem* Simulation = GetWorld() ? GetWorld()->GetSubsystem<UHOLSimulationSubsystem>() : nullptr)
	{
		Simulation->GetOnStepStarted().Remove(StepStartedHandle);
	}
	StepStartedHandle.Reset();
	UnitIds.Reset();
	FrameTimes.Reset();
	LocationsAtMeasureStart.Reset();
	Super::Deinitialize();
}

TStatId UHOLStressTestSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLStressTestSubsystem, STATGROUP_Tickables);
}

UHOLUnitSubsystem* UHOLStressTestSubsystem::GetUnitSubsystem() const
{
	return GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
}

void UHOLStressTestSubsystem::StartTest(const EHOLStressScenario InScenario, const int32 InCount, const float InMeasureSeconds, const bool bInQuitWhenDone)
{
	StopTest();

	UE::HeroesOfLiberty::StressTest::ForceDisplaySettings();
	Scenario = InScenario;
	Count = InCount;
	MeasureSeconds = InMeasureSeconds;
	bQuitWhenDone = bInQuitWhenDone;
	// S7 gathers: Peasants, whatever the type of the other scenarios.
	UnitTypeName = Scenario == EHOLStressScenario::S7_Gathering ? FName(TEXT("Peasant")) : FName(*UE::HeroesOfLiberty::StressTest::UnitType);

	UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>();
	if (Simulation == nullptr)
	{
		UE_LOG(LogHOLStressTest, Error, TEXT("No fixed-step simulation: the stress test cannot run"));
		return;
	}
	if (!StepStartedHandle.IsValid())
	{
		StepStartedHandle = Simulation->GetOnStepStarted().AddUObject(this, &UHOLStressTestSubsystem::OnSimulationStep);
	}
	const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	WarmUpSteps = FMath::CeilToInt32((Scenario == EHOLStressScenario::S7_Gathering ? UE::HeroesOfLiberty::StressTest::GatheringWarmUpSeconds
		: UE::HeroesOfLiberty::StressTest::WarmUpSeconds) / StepSeconds);
	MeasureSteps = FMath::Max(1, FMath::RoundToInt32(MeasureSeconds / StepSeconds));

	UsedPhysicalBeforeSpawn = FPlatformMemory::GetStats().UsedPhysical;
	if (UHOLUnitSubsystem* Units = GetUnitSubsystem())
	{
		// Same crowd for every run, even several in one process (D230).
		Units->ResetSpawnSeed();
	}
	if (Scenario == EHOLStressScenario::S7_Gathering)
	{
		SetUpGathering();
	}
	SpawnUnits();
	PlaceViewer();

	Phase = EPhase::WarmingUp;
	PhaseSteps = 0;
	if (UHOLUnitSubsystem* Units = GetUnitSubsystem())
	{
		UnitIds = TArray<uint32>();
		UE_LOG(LogHOLStressTest, Display, TEXT("Stress test %s: %d units of type %s requested (D224: they are born on the following steps), measuring for %.0f s after %.0f s of warm-up"),
			UE::HeroesOfLiberty::StressTest::ToString(Scenario), Count, *UnitTypeName.ToString(), MeasureSeconds, WarmUpSteps * StepSeconds);
		return;
	}
	UE_LOG(LogHOLStressTest, Error, TEXT("No unit subsystem: nothing spawned"));
}

void UHOLStressTestSubsystem::StopTest()
{
	DestroyUnits();
	TearDownGathering();
	Phase = EPhase::Idle;
}

void UHOLStressTestSubsystem::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bQuitPending)
	{
		// The units of the run were queued for destruction (D224), applied at the tail of the next fixed step:
		// give the steps the time to do it, then report and leave.
		QuitDelayElapsed += DeltaTime;
		if (QuitDelayElapsed >= UE::HeroesOfLiberty::StressTest::QuitDelaySeconds)
		{
			bQuitPending = false;
			if (const UHOLUnitSubsystem* Units = GetUnitSubsystem())
			{
				UE_LOG(LogHOLStressTest, Display, TEXT("Units left after the run: %d (D224: the queued destructions are applied at the tail of the step)"), Units->GetNumUnits());
			}
			UKismetSystemLibrary::QuitGame(GetWorld(), GetWorld()->GetFirstPlayerController(), EQuitPreference::Quit, /*bIgnorePlatformRestrictions*/ false);
		}
		return;
	}

	// The phases advance with the fixed steps (OnSimulationStep); the frames are only measured here.
	if (Phase != EPhase::Measuring)
	{
		return;
	}

	FrameTimes.Add(DeltaTime);
	GameThreadMsSum += FPlatformTime::ToMilliseconds(GGameThreadTime);
	RenderThreadMsSum += FPlatformTime::ToMilliseconds(GRenderThreadTime);
	GpuMsSum += FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	// Last finished frame's RHI draw calls (D236). Its triangle counter is left out: it only counts the draws whose
	// size the CPU knows, and skinned instances and Nanite are drawn indirectly, driven by the GPU (always 0 here).
	DrawCallsSum += GNumDrawCallsRHI[0];

	// One capture per run, at half of the measure. Skipped while the window is minimized: the capture would
	// be lost, and its placeholder back buffer trips a D3D12 assert (hence no retry later in the run).
	if (!bScreenshotTaken && PhaseSteps >= MeasureSteps / 2)
	{
		bScreenshotTaken = true;
		if (UE::HeroesOfLiberty::StressTest::IsGameWindowMinimized(GetWorld()))
		{
			UE_LOG(LogHOLStressTest, Warning, TEXT("No capture for this run: the game window is minimized"));
		}
		else
		{
			const TCHAR* ScenarioName = Scenario == EHOLStressScenario::S2_Moving ? TEXT("S2") : (Scenario == EHOLStressScenario::Crossing ? TEXT("C")
				: (Scenario == EHOLStressScenario::S7_Gathering ? TEXT("S7") : TEXT("S1")));
			FScreenshotRequest::RequestScreenshot(FString::Printf(TEXT("StressTest_%s.png"), ScenarioName),
				/*bInShowUI*/ false, /*bAddFilenameSuffix*/ false);
		}
	}

}

void UHOLStressTestSubsystem::OnSimulationStep(const int64 StepIndex)
{
	if (Phase == EPhase::Idle)
	{
		return;
	}

	++PhaseSteps;
	if (Phase == EPhase::WarmingUp && PhaseSteps >= WarmUpSteps)
	{
		BeginMeasure();
	}
	else if (Phase == EPhase::Measuring && PhaseSteps >= MeasureSteps)
	{
		// MeasureSteps whole steps measured: report before this step runs.
		FinishMeasure();
		return;
	}

	// S2 walks by orders (D229): the units that arrived are given a new destination, warm-up included. Issued at
	// the head of the step, the orders are applied by this very step (D216).
	if (Scenario == EHOLStressScenario::S2_Moving)
	{
		KeepUnitsWalking();
	}
	else if (Scenario == EHOLStressScenario::Crossing)
	{
		MaintainCrossing();
	}
	else if (Scenario == EHOLStressScenario::S7_Gathering && !bGatheringOrdered)
	{
		OrderGathering();
	}
}

void UHOLStressTestSubsystem::SpawnUnits()
{
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	if (Units == nullptr)
	{
		return;
	}

	if (Scenario == EHOLStressScenario::Crossing)
	{
		// Two units CrossingHalfDistance from the origin (15 m by default): the first walks along +X, the second
		// along a path CrossingAngle away from the first's opposite direction (0 = head-on, 90 = at right angles).
		// Each is ordered to the mirror of its start, so both reach the origin at the same instant — the case where
		// avoidance has to give way.
		const float HalfDistance = UE::HeroesOfLiberty::StressTest::CrossingHalfDistance;
		const float AngleRadians = FMath::DegreesToRadians(UE::HeroesOfLiberty::StressTest::GetCrossingAngle());
		Units->RequestUnits(FHOLUnitSpawnRequest{ UnitTypeName, 1, FVector(-HalfDistance, 0.f, 0.f), 1.f, 0 });
		Units->RequestUnits(FHOLUnitSpawnRequest{ UnitTypeName, 1, FVector(HalfDistance * FMath::Cos(AngleRadians), -HalfDistance * FMath::Sin(AngleRadians), 0.f), 1.f, 1 });
		return;
	}

	// The authority creates the units, spread over the following steps by the per-step budget (D224). The S1
	// units are born standing still, which is what an immobile unit means (D220). One request per player: the
	// births are served in order and draw their positions one after the other, so the crowd does not depend on
	// how it is shared. S7: each player's Peasants are born on their own side of the map, near their centre.
	const bool bGathering = Scenario == EHOLStressScenario::S7_Gathering;
	const int32 NumPlayers = bGathering ? GetNumPlayers() : FMath::Clamp(UE::HeroesOfLiberty::StressTest::Players, 1, 255);
	for (int32 Player = 0; Player < NumPlayers; ++Player)
	{
		const int32 PlayerCount = Count / NumPlayers + (Player < Count % NumPlayers ? 1 : 0);
		if (PlayerCount > 0)
		{
			Units->RequestUnits(FHOLUnitSpawnRequest{ UnitTypeName, PlayerCount, bGathering ? GetGatheringSpawnPoint(Player) : FVector::ZeroVector,
				bGathering ? UE::HeroesOfLiberty::StressTest::GatheringSpawnDisc : UE::HeroesOfLiberty::StressTest::GetSpawnRadius(),
				static_cast<uint8>(Player) });
		}
	}
}

int32 UHOLStressTestSubsystem::GetNumPlayers() const
{
	return FMath::Clamp(UE::HeroesOfLiberty::StressTest::Players, 1, 8);
}

FVector UHOLStressTestSubsystem::GetGatheringSpawnPoint(const int32 Player) const
{
	const double Angle = 2. * UE_DOUBLE_PI * Player / GetNumPlayers();
	return FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.) * UE::HeroesOfLiberty::StressTest::GatheringSpawnRadius;
}

void UHOLStressTestSubsystem::SetUpGathering()
{
	using namespace UE::HeroesOfLiberty::StressTest;
	TearDownGathering();
	UWorld* World = GetWorld();
	UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	if (!Sources || !Sources->HasAuthority())
	{
		UE_LOG(LogHOLStressTest, Error, TEXT("S7: no source registry with authority here, no trees"));
		return;
	}
	const int32 NumPlayers = GetNumPlayers();
	GatheringTrees.SetNum(NumPlayers);
	int32 NumTrees = 0;
	for (int32 Player = 0; Player < NumPlayers; ++Player)
	{
		const double Angle = 2. * UE_DOUBLE_PI * Player / NumPlayers;
		const FVector Out(FMath::Cos(Angle), FMath::Sin(Angle), 0.);
		const FVector Side(-Out.Y, Out.X, 0.);

		// Its main centre, facing the middle of the map: a deposit for the four resources (D295).
		const FTransform Transform(FRotator(0.f, static_cast<float>(FMath::RadiansToDegrees(Angle)) + 180.f, 0.f), Out * GatheringCentreRadius);
		AHOLBuilding* Centre = World->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), Transform, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Centre)
		{
			Centre->Setup(HOLBuildingTypeNames::MainCentre, static_cast<uint8>(Player), static_cast<uint8>(Player));
			Centre->FinishSpawning(Transform);
			GatheringCentres.Add(Centre);
		}

		// Its lumber camps, among its trees.
		TArray<FVector> Camps;
		for (int32 Camp = 0; Camp < GatheringCamps; ++Camp)
		{
			const FVector Location = Out * GatheringCampRing + Side * ((Camp - (GatheringCamps - 1) * 0.5) * GatheringCampSpacing);
			const FTransform CampTransform(Transform.GetRotation(), Location);
			AHOLBuilding* CampBuilding = World->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), CampTransform, nullptr, nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if (CampBuilding)
			{
				CampBuilding->Setup(HOLBuildingTypeNames::LumberCamp, static_cast<uint8>(Player), static_cast<uint8>(Player));
				CampBuilding->FinishSpawning(CampTransform);
				GatheringCentres.Add(CampBuilding);
				Camps.Add(Location);
			}
		}

		// Lone trees behind it, on rings: each one open on every side, room for its four gatherers; every other ring
		// half a step aside, so the trees of two rings do not line up.
		for (int32 Ring = 0; Ring < GatheringRings; ++Ring)
		{
			const double Radius = GatheringFirstRing + Ring * GatheringTreeSpacing;
			const double Shift = (Ring % 2) * GatheringTreeSpacing * 0.5;
			for (double Along = -GatheringHalfWidth; Along <= GatheringHalfWidth + 1.; Along += GatheringTreeSpacing)
			{
				const FVector Location = Out * Radius + Side * (Along + Shift);
				if (Camps.ContainsByPredicate([&Location](const FVector& Camp) { return FVector::Dist2D(Camp, Location) < GatheringCampClearance; }))
				{
					continue;
				}
				const int32 Index = Sources->AddSource(HOLSourceTypeNames::Tree, Location, TreeAmount);
				if (Index != INDEX_NONE)
				{
					GatheringTrees[Player].Add(Index);
					++NumTrees;
				}
			}
		}
	}
	UE_LOG(LogHOLStressTest, Display, TEXT("S7: %d deposits (a main centre and %d lumber camps each) and %d trees of %d wood placed for %d players"),
		GatheringCentres.Num(), GatheringCamps, NumTrees, TreeAmount, NumPlayers);
}

void UHOLStressTestSubsystem::OrderGathering()
{
	using namespace UE::HeroesOfLiberty::StressTest;
	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (!Units || !Commands || !EntitySubsystem)
	{
		return;
	}
	Units->GetUnitIdsOfType(UnitTypeName, UnitIds);
	if (UnitIds.Num() < Count)
	{
		return;	// the births are served over a few steps (D224)
	}
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TArray<TArray<uint32>> PlayerUnits;
	PlayerUnits.SetNum(GatheringTrees.Num());
	for (const uint32 UnitId : UnitIds)
	{
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(UnitId));
		if (State && PlayerUnits.IsValidIndex(State->OwnerPlayer))
		{
			PlayerUnits[State->OwnerPlayer].Add(UnitId);
		}
	}
	for (int32 Player = 0; Player < PlayerUnits.Num(); ++Player)
	{
		const TArray<int32>& Trees = GatheringTrees[Player];
		for (int32 First = 0; First < PlayerUnits[Player].Num() && Trees.Num() > 0; First += GatherersPerTree)
		{
			const int32 Num = FMath::Min(GatherersPerTree, PlayerUnits[Player].Num() - First);
			Commands->IssueGatherOrder(TConstArrayView<uint32>(PlayerUnits[Player].GetData() + First, Num), Trees[(First / GatherersPerTree) % Trees.Num()],
				Player);
			++NumOrdersIssued;
		}
	}
	bGatheringOrdered = true;
	UE_LOG(LogHOLStressTest, Display, TEXT("S7: %d Peasants sent to their trees, %d per tree"), UnitIds.Num(), GatherersPerTree);
}

void UHOLStressTestSubsystem::TearDownGathering()
{
	for (const TWeakObjectPtr<AHOLBuilding>& Centre : GatheringCentres)
	{
		if (Centre.IsValid())
		{
			Centre->Destroy();
		}
	}
	GatheringCentres.Reset();
	if (UHOLSourceSubsystem* Sources = GetWorld() ? GetWorld()->GetSubsystem<UHOLSourceSubsystem>() : nullptr)
	{
		// The registry keeps its indices for the whole match: the run's trees are used up, they leave the map.
		for (const TArray<int32>& Trees : GatheringTrees)
		{
			for (const int32 Tree : Trees)
			{
				Sources->Take(Tree, MAX_int32);
			}
		}
	}
	GatheringTrees.Reset();
	bGatheringOrdered = false;
}

void UHOLStressTestSubsystem::LogGatheringSummary() const
{
	const UWorld* World = GetWorld();
	const UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	if (!Sources || !EntitySubsystem || !Units)
	{
		return;
	}
	FHOLResources Unloaded;
	for (int32 Player = 0; Player < GetNumPlayers(); ++Player)
	{
		Unloaded += Sources->GetDeposited(static_cast<uint8>(Player));
	}
	Unloaded -= UnloadedAtMeasureStart;
	int32 Phases[4] = {};
	int32 Carrying = 0;
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	for (const uint32 UnitId : UnitIds)
	{
		if (const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Units->FindEntity(UnitId)))
		{
			++Phases[static_cast<int32>(Gather->Phase)];
			Carrying += Gather->Load.Amount > 0 ? 1 : 0;
		}
	}
	const int64 Paths = UHOLUnitOrderProcessor::GetTotalPathsServed() - PathsAtMeasureStart;
	const int64 Reused = UHOLUnitOrderProcessor::GetTotalPathsReused() - PathsReusedAtMeasureStart;
	UE_LOG(LogHOLStressTest, Display, TEXT("  Gathering (D296, D299): %d wood unloaded during the measure (%.0f per second), %d trees used up (%d standing), %lld paths served (%.1f per second), %lld kept paths reused (%.1f per second, D305)"),
		Unloaded.Wood, Unloaded.Wood / FMath::Max(MeasureSeconds, 1.f), StandingAtMeasureStart - Sources->GetNumStanding(), Sources->GetNumStanding(),
		Paths, Paths / FMath::Max(MeasureSeconds, 1.f), Reused, Reused / FMath::Max(MeasureSeconds, 1.f));
	const UHOLGatherProcessor::FCounters& Counters = UHOLGatherProcessor::GetCounters();
	UE_LOG(LogHOLStressTest, Display, TEXT("  Gathering trips during the measure: %lld unloads, walks to a tree %lld and to a deposit %lld, missed spots %lld and deposits %lld, idle for want of a free spot %lld"),
		Counters.Unloads - GatherCountersAtMeasureStart[4], Counters.WalksToSource - GatherCountersAtMeasureStart[0],
		Counters.WalksToDeposit - GatherCountersAtMeasureStart[1], Counters.MissedSpots - GatherCountersAtMeasureStart[2],
		Counters.MissedDeposits - GatherCountersAtMeasureStart[3], Counters.NoSpotStops - GatherCountersAtMeasureStart[5]);
	UE_LOG(LogHOLStressTest, Display, TEXT("  Buildings (D310): %lld bodies put back out of a footprint, %lld walkers given a new path for it, during the measure"),
		UHOLFootprintKeepOutProcessor::GetNumPushedOut() - PushedOutAtMeasureStart, UHOLUnitOrderProcessor::GetTotalBuildingRepaths() - RepathsAtMeasureStart);
	UE_LOG(LogHOLStressTest, Display, TEXT("  New footprints (J6b.2): %lld walkers given a new path, %lld kept paths dropped, during the measure"),
		UHOLUnitOrderProcessor::GetTotalFootprintRepaths() - CutRepathsAtMeasureStart, UHOLUnitOrderProcessor::GetTotalFootprintDroppedPaths() - CutDroppedAtMeasureStart);
	UE_LOG(LogHOLStressTest, Display, TEXT("  Gatherers at the end: to their source %d, gathering %d, to the deposit %d, not gathering %d; %d carrying a load"),
		Phases[static_cast<int32>(EHOLGatherPhase::ToSource)], Phases[static_cast<int32>(EHOLGatherPhase::Gathering)],
		Phases[static_cast<int32>(EHOLGatherPhase::ToDeposit)], Phases[static_cast<int32>(EHOLGatherPhase::None)], Carrying);
}

void UHOLStressTestSubsystem::MaintainCrossing()
{
	using namespace UE::HeroesOfLiberty::StressTest;

	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (Units == nullptr || Commands == nullptr || EntitySubsystem == nullptr)
	{
		return;
	}

	if (UnitIds.Num() < 2)
	{
		Units->GetUnitIdsOfType(UnitTypeName, UnitIds); // the two births are served at the head of a step (D224)
		return;
	}

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	const FMassEntityHandle First = Units->FindEntity(UnitIds[0]);
	const FMassEntityHandle Second = Units->FindEntity(UnitIds[1]);
	const FTransformFragment* FirstTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(First);
	const FTransformFragment* SecondTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Second);
	const FHOLUnitStateFragment* FirstState = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(First);
	const FHOLUnitStateFragment* SecondState = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Second);
	if (FirstTransform == nullptr || SecondTransform == nullptr || FirstState == nullptr || SecondState == nullptr)
	{
		return;
	}

	const FVector FirstLocation = FirstTransform->GetTransform().GetLocation();
	const FVector SecondLocation = SecondTransform->GetTransform().GetLocation();
	CrossingClosestDistance = FMath::Min<double>(CrossingClosestDistance, FVector::Dist(FirstLocation, SecondLocation));

	// The map is centred on the origin: the two ends of each unit's path are its birth position and its mirror.
	if (!bCrossingStartRead)
	{
		CrossingStart[0] = FirstLocation;
		CrossingStart[1] = SecondLocation;
		bCrossingStartRead = true;
	}

	// Only once both units stand: they then walk to the other end of their path (D216: orders go through the
	// queue and are applied at the fixed step), so both reach the origin at the same time.
	if (FirstState->Activity != EHOLUnitActivity::Idle || SecondState->Activity != EHOLUnitActivity::Idle)
	{
		return;
	}

	for (int32 Index = 0; Index < 2; ++Index)
	{
		const uint32 UnitId = UnitIds[Index];
		const FVector Target = bCrossingFarEnd ? -CrossingStart[Index] : CrossingStart[Index];
		Commands->IssueMoveOrder(MakeArrayView(&UnitId, 1), Target);
		++NumOrdersIssued;
	}
	bCrossingFarEnd = !bCrossingFarEnd;
	++CrossingSwaps;
}

void UHOLStressTestSubsystem::LogCrossingSummary() const
{
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	if (EntitySubsystem == nullptr || Units == nullptr || UnitIds.Num() < 2)
	{
		return;
	}

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	const FAgentRadiusFragment* FirstRadius = EntityManager.GetFragmentDataPtr<FAgentRadiusFragment>(Units->FindEntity(UnitIds[0]));
	const FAgentRadiusFragment* SecondRadius = EntityManager.GetFragmentDataPtr<FAgentRadiusFragment>(Units->FindEntity(UnitIds[1]));
	const double RadiusSum = (FirstRadius ? FirstRadius->Radius : 0.0) + (SecondRadius ? SecondRadius->Radius : 0.0);
	const double Closest = CrossingClosestDistance == TNumericLimits<double>::Max() ? 0.0 : CrossingClosestDistance;

	// Two units that walk through each other come closer than the sum of their radii; if they step aside, they
	// graze past each other around that distance. This is the check the fixed-step chrono cannot make (D228).
	UE_LOG(LogHOLStressTest, Display, TEXT("  Crossing (D228, D230): paths %.0f degrees apart (0 = head-on), %d swaps ordered, closest the two units came to each other %.1f cm, their two radii sum %.1f cm - %s"),
		UE::HeroesOfLiberty::StressTest::GetCrossingAngle(), CrossingSwaps, Closest, RadiusSum,
		Closest < RadiusSum ? TEXT("they walk through each other, the avoidance is not doing its job") : TEXT("they step aside"));
}

void UHOLStressTestSubsystem::DestroyUnits()
{
	if (UnitIds.Num() > 0)
	{
		if (UHOLUnitSubsystem* Units = GetUnitSubsystem())
		{
			// Destruction is deferred to the end of the step (D224): the units disappear on the next step.
			Units->RequestDestroyUnits(UnitIds);
		}
	}
	UnitIds.Reset();
}

void UHOLStressTestSubsystem::KeepUnitsWalking()
{
	using namespace UE::HeroesOfLiberty::StressTest;

	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	if (Units == nullptr || Commands == nullptr || EntitySubsystem == nullptr)
	{
		return;
	}

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
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		if (State == nullptr || Transform == nullptr || State->Activity != EHOLUnitActivity::Idle)
		{
			continue;
		}

		// Destination drawn from our own stream, so two runs of the same scenario are comparable (D230); the
		// command subsystem brings the point back onto the navmesh (D228) — the test does not walk the unit.
		const FVector Location = Transform->GetTransform().GetLocation();
		const float Angle = OrderRandom.FRandRange(0.f, 2.f * PI);
		const float Distance = OrderRadius * FMath::Sqrt(OrderRandom.FRand());
		const FVector Target = Location + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);
		Commands->IssueMoveOrder(MakeArrayView(&UnitId, 1), Target);
		--OrdersLeft;
		++NumOrdersIssued;
	}
}

TArray<FMassEntityHandle> UHOLStressTestSubsystem::GetUnitEntities() const
{
	TArray<FMassEntityHandle> Entities;
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
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

int32 UHOLStressTestSubsystem::CountLocationsInFrame(const TArray<FVector>& Locations) const
{
	const APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PlayerController == nullptr)
	{
		return 0;
	}
	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	int32 NumInFrame = 0;
	for (const FVector& Location : Locations)
	{
		FVector2D ScreenLocation;
		if (PlayerController->ProjectWorldLocationToScreen(Location, ScreenLocation)
			&& ScreenLocation.X >= 0.0 && ScreenLocation.X < ViewportX && ScreenLocation.Y >= 0.0 && ScreenLocation.Y < ViewportY)
		{
			++NumInFrame;
		}
	}
	return NumInFrame;
}

TArray<FVector> UHOLStressTestSubsystem::GetUnitLocations() const
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

void UHOLStressTestSubsystem::PlaceViewer() const
{
	// Three-quarter view over the spawn area, from the distance of the view (D236, D237): "Wide" at ~100 m keeps the
	// whole crowd in frame (D218; the 59 degree vertical field of view sets that distance, a soldier ~18 pixels high
	// at 1080p against ~13 from the former 136 m), "Game" at an RTS camera height (~38 m above the ground), part of
	// the tighter crowd out of frame: what a player zoomed on a big melee sees.
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}
	const bool bGameView = UE::HeroesOfLiberty::StressTest::IsGameView();
	const float PitchRadians = FMath::DegreesToRadians(bGameView ? 55.f : 45.f);
	const float Distance = bGameView ? 4600.f : 10000.f;
	const FVector ViewLocation(-Distance * FMath::Cos(PitchRadians), 0.f, Distance * FMath::Sin(PitchRadians));
	const FRotator ViewRotation = (FVector::ZeroVector - ViewLocation).Rotation();
	if (APawn* Pawn = PlayerController->GetPawn())
	{
		Pawn->SetActorLocation(ViewLocation, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);
	}
	PlayerController->SetControlRotation(ViewRotation);
}

void UHOLStressTestSubsystem::BeginMeasure()
{
	Phase = EPhase::Measuring;
	PhaseSteps = 0;
	FrameTimes.Reset();
	GameThreadMsSum = RenderThreadMsSum = GpuMsSum = 0.0;
	DrawCallsSum = 0.0;
	bScreenshotTaken = false;
	NumOrdersIssued = 0;
	CrossingClosestDistance = TNumericLimits<double>::Max();
	CrossingSwaps = 0;
	bCrossingStartRead = false;
	bCrossingFarEnd = true;
	OrderRandom.Initialize(1066);

	if (UHOLUnitSubsystem* Units = GetUnitSubsystem())
	{
		Units->GetUnitIdsOfType(UnitTypeName, UnitIds);
	}
	if (UnitIds.Num() < Count)
	{
		UE_LOG(LogHOLStressTest, Warning, TEXT("Only %d of the %d requested units exist at the start of the measure"), UnitIds.Num(), Count);
	}
	LocationsAtMeasureStart = GetUnitLocations();
	CountInterpenetration(InterpenetrationAtMeasureStart, SmallestGapAtMeasureStart);
	if (const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>())
	{
		UnloadedAtMeasureStart = FHOLResources();
		for (int32 Player = 0; Player < GetNumPlayers(); ++Player)
		{
			UnloadedAtMeasureStart += Sources->GetDeposited(static_cast<uint8>(Player));
		}
		StandingAtMeasureStart = Sources->GetNumStanding();
	}
	PathsAtMeasureStart = UHOLUnitOrderProcessor::GetTotalPathsServed();
	PathsReusedAtMeasureStart = UHOLUnitOrderProcessor::GetTotalPathsReused();
	PushedOutAtMeasureStart = UHOLFootprintKeepOutProcessor::GetNumPushedOut();
	RepathsAtMeasureStart = UHOLUnitOrderProcessor::GetTotalBuildingRepaths();
	CutRepathsAtMeasureStart = UHOLUnitOrderProcessor::GetTotalFootprintRepaths();
	CutDroppedAtMeasureStart = UHOLUnitOrderProcessor::GetTotalFootprintDroppedPaths();
	const UHOLGatherProcessor::FCounters& Counters = UHOLGatherProcessor::GetCounters();
	const int64 Now[6] = { Counters.WalksToSource, Counters.WalksToDeposit, Counters.MissedSpots, Counters.MissedDeposits, Counters.Unloads, Counters.NoSpotStops };
	FMemory::Memcpy(GatherCountersAtMeasureStart, Now, sizeof(Now));
	if (UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>())
	{
		Simulation->ResetStepStats();
	}
}

void UHOLStressTestSubsystem::LogRepresentationSummary() const
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
		UE_LOG(LogHOLStressTest, Display, TEXT("  Representation: %s x %d"), *Entry.Key, Entry.Value);
	}
}

void UHOLStressTestSubsystem::LogUnitStateSummary() const
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

		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
		const FString UnitState = State ? UEnum::GetDisplayValueAsText(State->Activity).ToString() : TEXT("no unit state");
		++UnitStates.FindOrAdd(UnitState);
	}
	for (const TPair<FString, int32>& Entry : MoveActions)
	{
		UE_LOG(LogHOLStressTest, Display, TEXT("  Move action: %s x %d"), *Entry.Key, Entry.Value);
	}
	for (const TPair<FString, int32>& Entry : UnitStates)
	{
		UE_LOG(LogHOLStressTest, Display, TEXT("  Unit state: %s x %d"), *Entry.Key, Entry.Value);
	}
}

void UHOLStressTestSubsystem::LogUnitConstantsSummary() const
{
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	if (EntitySubsystem == nullptr || Units == nullptr)
	{
		return;
	}

	// Every living unit, not only this run's (two types coexist easily), and the values the units really carry
	// rather than the table: Mass looks a shared fragment up by the CRC of its reflected fields, so a fragment
	// with fields that are not UPROPERTY would be shared between two types and the second type would silently
	// walk at the first one's speed (D221, D222).
	TArray<uint32> AllUnitIds;
	Units->GetAllUnitIds(AllUnitIds);

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TMap<FString, int32> Histogram;
	for (const uint32 UnitId : AllUnitIds)
	{
		const FHOLUnitTypeConstants* Constants = EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Units->FindEntity(UnitId));
		const FString Key = Constants
			? FString::Printf(TEXT("%s: radius %.0f cm, speed %.0f cm/s, %d HP, avoidance %.0f cm"), *Constants->TypeName.ToString(), Constants->Radius, Constants->Speed, Constants->MaxHealth, Constants->AvoidanceDistance)
			: TEXT("no unit constants");
		++Histogram.FindOrAdd(Key);
	}
	for (const TPair<FString, int32>& Entry : Histogram)
	{
		UE_LOG(LogHOLStressTest, Display, TEXT("  Unit constants: %s x %d"), *Entry.Key, Entry.Value);
	}
}

void UHOLStressTestSubsystem::CountInterpenetration(int32& OutNumPairs, double& OutSmallestGap) const
{
	OutNumPairs = 0;
	OutSmallestGap = 0.0;

	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	if (EntitySubsystem == nullptr || Units == nullptr)
	{
		return;
	}

	TArray<uint32> AllUnitIds;
	Units->GetAllUnitIds(AllUnitIds);

	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	TArray<FVector> Locations;
	TArray<float> Radii;
	float MaxRadius = 0.f;
	for (const uint32 UnitId : AllUnitIds)
	{
		const FMassEntityHandle Entity = Units->FindEntity(UnitId);
		const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		const FAgentRadiusFragment* Radius = EntityManager.GetFragmentDataPtr<FAgentRadiusFragment>(Entity);
		if (Transform == nullptr || Radius == nullptr)
		{
			continue;
		}
		Locations.Add(Transform->GetTransform().GetLocation());
		Radii.Add(Radius->Radius);
		MaxRadius = FMath::Max(MaxRadius, Radius->Radius);
	}
	if (Locations.Num() < 2)
	{
		return;
	}

	// Uniform grid of one cell per two radii: any pair that could touch is among the nine neighbouring cells.
	const float CellSize = FMath::Max(2.f * MaxRadius, 1.f);
	TMap<FIntPoint, TArray<int32>> Grid;
	Grid.Reserve(Locations.Num());
	for (int32 Index = 0; Index < Locations.Num(); ++Index)
	{
		const FIntPoint Cell(FMath::FloorToInt(Locations[Index].X / CellSize), FMath::FloorToInt(Locations[Index].Y / CellSize));
		Grid.FindOrAdd(Cell).Add(Index);
	}

	double SmallestGap = TNumericLimits<double>::Max();
	for (int32 Index = 0; Index < Locations.Num(); ++Index)
	{
		const FIntPoint Cell(FMath::FloorToInt(Locations[Index].X / CellSize), FMath::FloorToInt(Locations[Index].Y / CellSize));
		for (int32 DeltaX = -1; DeltaX <= 1; ++DeltaX)
		{
			for (int32 DeltaY = -1; DeltaY <= 1; ++DeltaY)
			{
				const TArray<int32>* Neighbours = Grid.Find(FIntPoint(Cell.X + DeltaX, Cell.Y + DeltaY));
				if (Neighbours == nullptr)
				{
					continue;
				}
				for (const int32 Other : *Neighbours)
				{
					if (Other <= Index)
					{
						continue; // each pair once
					}
					const double Gap = FVector::Dist2D(Locations[Index], Locations[Other]) - Radii[Index] - Radii[Other];
					SmallestGap = FMath::Min(SmallestGap, Gap);
					if (Gap < 0.0)
					{
						++OutNumPairs;
					}
				}
			}
		}
	}
	OutSmallestGap = SmallestGap == TNumericLimits<double>::Max() ? 0.0 : SmallestGap;
}

void UHOLStressTestSubsystem::LogAvoidanceSummary() const
{
	int32 NumPairsAtEnd = 0;
	double SmallestGapAtEnd = 0.0;
	CountInterpenetration(NumPairsAtEnd, SmallestGapAtEnd);

	// Overlapping units are normal after a random spawn; what matters is whether walking makes it worse, which
	// is what a dead avoidance force looks like (D228: Mass scales it down to 10 % when it believes the unit has
	// arrived). A negative gap means two bodies are inside each other.
	UE_LOG(LogHOLStressTest, Display, TEXT("  Avoidance (D228): at the start of the measure %d pairs over each other (smallest gap %.1f cm), at the end %d pairs (smallest gap %.1f cm)"),
		InterpenetrationAtMeasureStart, SmallestGapAtMeasureStart, NumPairsAtEnd, SmallestGapAtEnd);
}

void UHOLStressTestSubsystem::LogUnitRegistrySummary() const
{
	const UWorld* World = GetWorld();
	const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
	UHOLUnitSubsystem* Units = GetUnitSubsystem();
	if (EntitySubsystem == nullptr || Units == nullptr)
	{
		return;
	}

	// The identifier of a unit is its own, not Mass' handle (D223): the table must agree with the fragments.
	const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
	int32 NumMismatched = 0;
	for (const uint32 UnitId : UnitIds)
	{
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(UnitId));
		if (State == nullptr || State->UnitId != UnitId)
		{
			++NumMismatched;
		}
	}
	UE_LOG(LogHOLStressTest, Display, TEXT("  Unit registry (D223/D224): %d units of %d requested, %d births waiting, identifier table checked on %d units, %d mismatch"),
		Units->GetNumUnits(), Count, Units->GetNumPendingBirths(), UnitIds.Num(), NumMismatched);
}

void UHOLStressTestSubsystem::FinishMeasure()
{
	using namespace UE::HeroesOfLiberty::StressTest;

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

	FHOLSimulationStepStats StepStats;
	if (const UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepStats = Simulation->GetStepStats();
	}
	const double UsedPhysicalMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0 * 1024.0);
	const double UnitsDeltaMB = (static_cast<double>(FPlatformMemory::GetStats().UsedPhysical) - static_cast<double>(UsedPhysicalBeforeSpawn)) / (1024.0 * 1024.0);

	const double NumFrames = FMath::Max(1, FrameTimes.Num());
	const double GameThreadMs = GameThreadMsSum / NumFrames;
	const double RenderThreadMs = RenderThreadMsSum / NumFrames;
	const double GpuMs = GpuMsSum / NumFrames;
	const double DrawCalls = DrawCallsSum / NumFrames;

	const TArray<FVector> LocationsAtMeasureEnd = GetUnitLocations();
	double TotalDisplacement = 0.0;
	for (int32 Index = 0; Index < FMath::Min(LocationsAtMeasureStart.Num(), LocationsAtMeasureEnd.Num()); ++Index)
	{
		TotalDisplacement += FVector::Dist(LocationsAtMeasureStart[Index], LocationsAtMeasureEnd[Index]);
	}
	const double AverageDisplacementM = LocationsAtMeasureEnd.Num() > 0 ? TotalDisplacement / LocationsAtMeasureEnd.Num() / 100.0 : 0.0;
	const int32 NumUnitsInFrame = CountLocationsInFrame(LocationsAtMeasureEnd);

	const bool bSimulationPassed = StepStats.GetAverageMs() <= MaxSimulationMs;
	const bool bRenderPassed = AverageFps >= MinFramesPerSecond;

	const FString Report = FString::Printf(
		TEXT("Stress test %s, %d units of type %s, %.0f s, %s view (crowd radius %.0f m, D236)\n")
		TEXT("  Simulation: %.3f ms average per step (max %.3f ms, %d steps, %d frames dropped time) - threshold %.1f ms: %s\n")
		TEXT("  Rendering: %.1f fps average (1%% low %.1f fps, %d frames) - threshold %.0f fps: %s\n")
		TEXT("  Frame: game thread %.2f ms, render thread %.2f ms, GPU %.2f ms\n")
		TEXT("  Draw: %.0f draw calls per frame, %d of %d units in frame at the end of the measure (D237)\n")
		TEXT("  Memory: %.0f MB used (%+.0f MB since spawn)\n")
		TEXT("  Movement: units moved %.1f m on average during the measure, %d move orders issued (D229)"),
		ToString(Scenario), UnitIds.Num(), *UnitTypeName.ToString(), MeasureSeconds, IsGameView() ? TEXT("Game") : TEXT("Wide"), GetSpawnRadius() / 100.f,
		StepStats.GetAverageMs(), StepStats.MaxMs, StepStats.NumSteps, StepStats.NumDroppedFrames, MaxSimulationMs, Verdict(bSimulationPassed),
		AverageFps, OnePercentLowFps, FrameTimes.Num(), MinFramesPerSecond, Verdict(bRenderPassed),
		GameThreadMs, RenderThreadMs, GpuMs,
		DrawCalls, NumUnitsInFrame, LocationsAtMeasureEnd.Num(),
		UsedPhysicalMB, UnitsDeltaMB,
		AverageDisplacementM, NumOrdersIssued);

	UE_LOG(LogHOLStressTest, Display, TEXT("%s"), *Report);
	UE_LOG(LogHOLStressTest, Display, TEXT("  Display (D245): %s"), *UE::HeroesOfLiberty::StressTest::DescribeDisplaySettings());
	for (const FHOLSimulationProcessorStats& Processor : StepStats.Processors)
	{
		const FString Throttled = Processor.StepInterval > 1 ? FString::Printf(TEXT(" (1 step out of %d)"), Processor.StepInterval) : FString();
		UE_LOG(LogHOLStressTest, Display, TEXT("  Step breakdown: %s %.3f ms%s"), *Processor.Name, Processor.GetAverageMs(), *Throttled);
	}
	// Births and destructions are part of the step but not processors (D224): worth a line only in a run that
	// creates or destroys units while measuring.
	if (StepStats.NumUnitsBorn > 0 || StepStats.NumUnitsDestroyed > 0)
	{
		UE_LOG(LogHOLStressTest, Display, TEXT("  Step breakdown: unit births %.3f ms (%d born), unit destructions %.3f ms (%d destroyed)"),
			StepStats.GetAverageBirthMs(), StepStats.NumUnitsBorn, StepStats.GetAverageDestructionMs(), StepStats.NumUnitsDestroyed);
	}
	// The avoidance settings of the run, so two reports of an A/B tell themselves apart (D220, D230).
	if (const UHOLSimulationSubsystem* Simulation = GetWorld()->GetSubsystem<UHOLSimulationSubsystem>())
	{
		UE_LOG(LogHOLStressTest, Display, TEXT("  Avoidance cadence (D220, D230): solver 1 step out of %d, impulse re-applied on %d steps after each run, decay %.2f"),
			Simulation->GetThrottledStepInterval(), UHOLAvoidanceHoldProcessor::GetHoldSteps(), UHOLAvoidanceHoldProcessor::GetHoldDecay());
	}
	LogRepresentationSummary();
	LogUnitStateSummary();
	LogUnitConstantsSummary();
	LogAvoidanceSummary();
	LogUnitRegistrySummary();
	if (Scenario == EHOLStressScenario::Crossing)
	{
		LogCrossingSummary();
	}
	if (Scenario == EHOLStressScenario::S7_Gathering)
	{
		LogGatheringSummary();
	}
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
