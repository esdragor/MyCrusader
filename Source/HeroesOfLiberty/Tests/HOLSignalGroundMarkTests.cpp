// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/InstancedStaticMeshComponent.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLPlayerController.h"
#include "Player/HOLSelectionMarkers.h"
#include "Player/HOLSignalInput.h"

#if WITH_DEV_AUTOMATION_TESTS

// D825 (the user's trial of signals_altclick, SignalsInProgress.md): a received signal shows in the WORLD too, at its point, while it
// lives (5 s of the world's time, D784), in its kind's colour (D821). On the real chain: ClientReceiveSignal -> ReceiveSignal ->
// the controller's ReceivedSignals -> GetActiveSignals (Expire) -> AHOLSelectionMarkers::UpdateSignalMarks -> its instances.
namespace UE::HeroesOfLiberty::Tests::SignalGroundMark
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSignalGroundMarkTest, "HeroesOfLiberty.Signals.GroundMark", UE::HeroesOfLiberty::Tests::SignalGroundMark::Flags)

bool FHOLSignalGroundMarkTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLPlayerController* Controller = World->SpawnActor<AHOLPlayerController>();
	AHOLSelectionMarkers* Markers = World->SpawnActor<AHOLSelectionMarkers>();
	if (!TestNotNull(TEXT("The controller"), Controller) || !TestNotNull(TEXT("The markers"), Markers))
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}
	Markers->Init(Controller);
	const UInstancedStaticMeshComponent* Marks = Markers->GetSignalMarks();
	constexpr int32 Attack = 1, Rally = 4;
	constexpr int32 Pieces = AHOLSelectionMarkers::SignalMarkPieces;

	World->TimeSeconds = 10.0;
	Markers->UpdateSignalMarks();
	TestEqual(TEXT("No signal, no mark in the world"), Marks ? Marks->GetInstanceCount() : -1, 0);

	// An ally's attack signal arrives at 10 s at (1200, -3400, 150).
	const FVector Point(1200.0, -3400.0, 150.0);
	Controller->ReceiveSignal(1, Attack, Point);
	Markers->UpdateSignalMarks();
	if (TestEqual(TEXT("The signal's mark is in the world (its pieces)"), Marks ? Marks->GetInstanceCount() : -1, Pieces))
	{
		FTransform Piece;
		Marks->GetInstanceTransform(0, Piece, /*bWorldSpace*/ true);
		TestTrue(TEXT("At the signal's point (x, y)"), FVector2D(Piece.GetLocation()).Equals(FVector2D(Point), 1.0));
		TestTrue(TEXT("On the ground there, just above it"), FMath::IsNearlyEqual(Piece.GetLocation().Z, Point.Z, 10.0));
		// D870 divided the mark by 3: the size is the cvar's (hol.Signal.GroundMarkSize), not a number of the test; at least half of it after the pulse, and at least 1 m.
		const IConsoleVariable* SizeVar = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Signal.GroundMarkSize"));
		const double ConfiguredSide = SizeVar ? static_cast<double>(SizeVar->GetFloat()) : 0.0;
		TestTrue(TEXT("The mark's size is the one of hol.Signal.GroundMarkSize (at least half of it, pulse aside)"), ConfiguredSide > 0.0 && Piece.GetScale3D().X * 100.0 >= 0.5 * ConfiguredSide);
		TestTrue(TEXT("Wide enough to be seen from the camera (at least 1 m)"), Piece.GetScale3D().X * 100.0 >= 100.0);
		const FLinearColor Drawn(Marks->PerInstanceSMCustomData[0], Marks->PerInstanceSMCustomData[1], Marks->PerInstanceSMCustomData[2]);
		const TArray<FHOLReceivedSignal> Active = Controller->GetActiveSignals();
		if (TestEqual(TEXT("One living signal"), Active.Num(), 1))
		{
			TestTrue(TEXT("In the colour the signal was given (its kind's, D821)"), Drawn.Equals(Active[0].Color, 0.01f));
		}
		TestFalse(TEXT("The attack's mark is not the neutral white"), Drawn.Equals(FLinearColor::White, 0.01f));
	}
	TestTrue(TEXT("Described for the scenario"), Markers->DescribeSignalMarks().StartsWith(TEXT("1 | kind 1 (1200, -3400)")));

	// A second one, a rally, at 12 s elsewhere: two marks.
	World->TimeSeconds = 12.0;
	Controller->ReceiveSignal(0, Rally, FVector(-500.0, 800.0, 90.0));
	Markers->UpdateSignalMarks();
	TestEqual(TEXT("Two signals, two marks"), Marks ? Marks->GetInstanceCount() : -1, 2 * Pieces);

	// At 15 s of the world's time the first has lived its 5 s (D784): only the rally's mark is left; at 17 s, none.
	World->TimeSeconds = 15.0;
	Markers->UpdateSignalMarks();
	TestEqual(TEXT("At 5 s the first mark is gone"), Marks ? Marks->GetInstanceCount() : -1, Pieces);
	World->TimeSeconds = 17.0;
	Markers->UpdateSignalMarks();
	TestEqual(TEXT("At 17 s no mark is left"), Marks ? Marks->GetInstanceCount() : -1, 0);
	TestEqual(TEXT("Described as none"), Markers->DescribeSignalMarks(), FString(TEXT("none")));

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// D860 H2: the mark PULSES. Same signal read at two ages: with hol.Signal.MarkStyle 1 the diamond's instance has two different sizes
// (the ratio is the rule's PulseScale, read through the same chain: ReceiveSignal -> GetActiveSignals -> UpdateSignalMarks -> instance);
// with style 0 (the fixed mark of before) the two sizes are equal.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSignalGroundMarkPulseTest, "HeroesOfLiberty.Signals.GroundMarkPulse", UE::HeroesOfLiberty::Tests::SignalGroundMark::Flags)

bool FHOLSignalGroundMarkPulseTest::RunTest(const FString& Parameters)
{
	IConsoleVariable* Style = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Signal.MarkStyle"));
	if (!TestNotNull(TEXT("hol.Signal.MarkStyle exists"), Style))
	{
		return false;
	}
	const int32 OldStyle = Style->GetInt();
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLPlayerController* Controller = World->SpawnActor<AHOLPlayerController>();
	AHOLSelectionMarkers* Markers = World->SpawnActor<AHOLSelectionMarkers>();
	if (!TestNotNull(TEXT("The controller"), Controller) || !TestNotNull(TEXT("The markers"), Markers))
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}
	Markers->Init(Controller);
	const UInstancedStaticMeshComponent* Marks = Markers->GetSignalMarks();
	World->TimeSeconds = 10.0;
	Controller->ReceiveSignal(1, 1, FVector(1200.0, -3400.0, 150.0));
	const double Quarter = HOLSignalInput::MarkLifetime() * 0.25;

	// The width of the first piece (the diamond) at an age, in the world.
	auto WidthAt = [&](const double Age) -> double
	{
		World->TimeSeconds = 10.0 + Age;
		Markers->UpdateSignalMarks();
		FTransform Piece;
		if (!Marks || Marks->GetInstanceCount() < 1)
		{
			return -1.0;
		}
		Marks->GetInstanceTransform(0, Piece, /*bWorldSpace*/ true);
		return Piece.GetScale3D().X;
	};

	Style->Set(1, ECVF_SetByCode);
	const double Born = WidthAt(0.0);
	const double Later = WidthAt(Quarter);
	if (TestTrue(TEXT("Both ages drew the mark"), Born > 0.0 && Later > 0.0))
	{
		TestTrue(TEXT("Pulsing: the mark's size differs between two ages of its life"), !FMath::IsNearlyEqual(Born, Later, 0.001));
		const double Expected = HOLSignalInput::PulseScale(Quarter) / HOLSignalInput::PulseScale(0.0);
		TestTrue(TEXT("By the rule's factor (PulseScale), not an arbitrary one"), FMath::IsNearlyEqual(Later / Born, Expected, 0.001));
	}

	Style->Set(0, ECVF_SetByCode);
	const double FixedBorn = WidthAt(0.0);
	const double FixedLater = WidthAt(Quarter);
	TestTrue(TEXT("Style 0: the fixed mark of before, the same size at both ages"), FixedBorn > 0.0 && FMath::IsNearlyEqual(FixedBorn, FixedLater, 0.001));

	Style->Set(OldStyle, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
