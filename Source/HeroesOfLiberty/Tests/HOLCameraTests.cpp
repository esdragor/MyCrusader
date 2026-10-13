// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLStartSlot.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Player/HOLRTSCamera.h"
#include "Settings/HOLGameUserSettings.h"

#if WITH_DEV_AUTOMATION_TESTS

// J1 checks (D257): the camera's rules (D254), its settings (D253) and the start slots of the match map (D256).
namespace UE::HeroesOfLiberty::Tests::Camera
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const TCHAR* MatchMapPath = TEXT("/Game/Maps/Plain.Plain");
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCameraBoundsTest, "HeroesOfLiberty.Camera.Bounds", UE::HeroesOfLiberty::Tests::Camera::Flags)
bool FHOLCameraBoundsTest::RunTest(const FString& Parameters)
{
	const FBox2D Area(FVector2D(-100.0, -200.0), FVector2D(300.0, 400.0));
	TestEqual(TEXT("inside: unchanged"), AHOLRTSCamera::ClampFocus(FVector2D(10.0, 20.0), Area), FVector2D(10.0, 20.0));
	TestEqual(TEXT("past a corner: on the corner"), AHOLRTSCamera::ClampFocus(FVector2D(1000.0, -1000.0), Area), FVector2D(300.0, -200.0));
	TestEqual(TEXT("past one side: on that side"), AHOLRTSCamera::ClampFocus(FVector2D(-500.0, 50.0), Area), FVector2D(-100.0, 50.0));
	TestEqual(TEXT("no area: unchanged"), AHOLRTSCamera::ClampFocus(FVector2D(1.0e6, 0.0), FBox2D(ForceInit)), FVector2D(1.0e6, 0.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCameraPanSpeedTest, "HeroesOfLiberty.Camera.PanSpeed", UE::HeroesOfLiberty::Tests::Camera::Flags)
bool FHOLCameraPanSpeedTest::RunTest(const FString& Parameters)
{
	const float Normal = AHOLRTSCamera::GetPanSpeed(AHOLRTSCamera::DefaultHeight, EHOLCameraSpeed::Normal);
	TestTrue(TEXT("~40 m/s at the default height"), FMath::IsNearlyEqual(Normal, 4025.f, 50.f));
	TestEqual(TEXT("twice as high, twice as fast"), AHOLRTSCamera::GetPanSpeed(2.f * AHOLRTSCamera::DefaultHeight, EHOLCameraSpeed::Normal), 2.f * Normal);
	TestTrue(TEXT("slow < normal < fast"),
		AHOLRTSCamera::GetPanSpeed(AHOLRTSCamera::DefaultHeight, EHOLCameraSpeed::Slow) < Normal
		&& Normal < AHOLRTSCamera::GetPanSpeed(AHOLRTSCamera::DefaultHeight, EHOLCameraSpeed::Fast));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLCameraZoomTest, "HeroesOfLiberty.Camera.Zoom", UE::HeroesOfLiberty::Tests::Camera::Flags)
bool FHOLCameraZoomTest::RunTest(const FString& Parameters)
{
	// A world of its own, so that the camera actor can be spawned.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);

	AHOLRTSCamera* Camera = World->SpawnActor<AHOLRTSCamera>();
	if (TestNotNull(TEXT("camera spawned"), Camera))
	{
		TestEqual(TEXT("default height"), Camera->GetHeight(), AHOLRTSCamera::DefaultHeight);
		Camera->Zoom(100.f);
		TestEqual(TEXT("zoomed in: stops at the lowest"), Camera->GetHeight(), AHOLRTSCamera::MinHeight);
		Camera->Zoom(-100.f);
		TestEqual(TEXT("zoomed out: stops at the highest"), Camera->GetHeight(), AHOLRTSCamera::MaxHeight);
		Camera->Zoom(1.f);
		TestEqual(TEXT("one wheel step"), Camera->GetHeight(), AHOLRTSCamera::MaxHeight - AHOLRTSCamera::ZoomStep);

		Camera->SetArea(FBox2D(FVector2D(-1000.0, -1000.0), FVector2D(1000.0, 1000.0)));
		Camera->SetFocus(FVector(0.0, 0.0, 50.0));
		Camera->Pan(FVector2D(1.0, 0.0), 1.f, EHOLCameraSpeed::Normal);
		TestEqual(TEXT("screen right is world +Y, held by the bounds"), Camera->GetFocus(), FVector(0.0, 1000.0, 50.0));
		Camera->Pan(FVector2D(0.0, 1.0), 0.01f, EHOLCameraSpeed::Normal);
		TestTrue(TEXT("screen up is world +X"), Camera->GetFocus().X > 0.0 && Camera->GetFocus().Y == 1000.0);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSettingsCameraTest, "HeroesOfLiberty.Settings.Camera", UE::HeroesOfLiberty::Tests::Camera::Flags)
bool FHOLSettingsCameraTest::RunTest(const FString& Parameters)
{
	UHOLGameUserSettings* Written = NewObject<UHOLGameUserSettings>();
	Written->SetToDefaults();
	TestEqual(TEXT("normal speed by default"), Written->GetCameraSpeed(), EHOLCameraSpeed::Normal);
	TestTrue(TEXT("edge scrolling on by default"), Written->IsEdgeScrollEnabled());
	Written->SetCameraSpeed(EHOLCameraSpeed::Fast);
	Written->SetEdgeScrollEnabled(false);

	// Saved then read back through a file of its own, never the player's GameUserSettings.ini.
	const FString Ini = FConfigCacheIni::NormalizeConfigIniPath(
		FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("HOLSettingsCamera.ini")));
	IFileManager::Get().Delete(*Ini);
	Written->SaveConfig(CPF_Config, *Ini);
	UHOLGameUserSettings* Read = NewObject<UHOLGameUserSettings>();
	Read->LoadConfig(nullptr, *Ini);
	TestEqual(TEXT("speed read back"), Read->GetCameraSpeed(), EHOLCameraSpeed::Fast);
	TestFalse(TEXT("edge scrolling read back"), Read->IsEdgeScrollEnabled());
	GConfig->UnloadFile(Ini);
	IFileManager::Get().Delete(*Ini);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapStartSlotsTest, "HeroesOfLiberty.Map.StartSlots", UE::HeroesOfLiberty::Tests::Camera::Flags)
bool FHOLMapStartSlotsTest::RunTest(const FString& Parameters)
{
	// The actors as saved in the match map, read without starting it: their components are not registered, so their
	// places are read from the roots' relative transforms (the world transforms are computed at registration).
	const auto Place = [](const AActor& Actor) { return FVector2D(Actor.GetRootComponent()->GetRelativeLocation()); };
	const UWorld* Map = LoadObject<UWorld>(nullptr, UE::HeroesOfLiberty::Tests::Camera::MatchMapPath);
	if (!TestNotNull(TEXT("match map (Tools/Editor/create_match_map.py)"), Map) || !Map->PersistentLevel)
	{
		return false;
	}
	TMap<int32, FVector2D> Starts;
	const AHOLMapBounds* Bounds = nullptr;
	for (const AActor* Actor : Map->PersistentLevel->Actors)
	{
		if (const AHOLStartSlot* Start = Cast<AHOLStartSlot>(Actor))
		{
			TestFalse(FString::Printf(TEXT("slot %d placed once"), Start->GetSlotIndex()), Starts.Contains(Start->GetSlotIndex()));
			Starts.Add(Start->GetSlotIndex(), Place(*Start));
		}
		Bounds = Bounds ? Bounds : Cast<AHOLMapBounds>(Actor);
	}
	if (!TestNotNull(TEXT("map bounds"), Bounds) || !TestEqual(TEXT("one start per slot"), Starts.Num(), FHOLMatchConfig::MaxSlots))
	{
		return false;
	}

	const FVector2D Centre = Place(*Bounds);
	const FVector2D HalfSize = Bounds->GetArea().GetExtent();
	const FBox2D Area(Centre - HalfSize, Centre + HalfSize);
	const double Radius = FVector2D::Distance(Starts[0], Centre);
	const auto Angle = [&Starts, &Centre](const int32 Slot)
	{
		const FVector2D Offset = Starts[Slot] - Centre;
		return FMath::RadiansToDegrees(FMath::Atan2(Offset.Y, Offset.X));
	};
	const auto Apart = [&Angle](const int32 A, const int32 B)
	{
		return FMath::Abs(FMath::FindDeltaAngleDegrees(Angle(A), Angle(B)));
	};
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		TestTrue(FString::Printf(TEXT("slot %d inside the bounds"), Slot), Area.IsInside(Starts[Slot]));
		TestTrue(FString::Printf(TEXT("slot %d as far from the centre as slot 0 (symmetric map)"), Slot),
			FMath::IsNearlyEqual(FVector2D::Distance(Starts[Slot], Centre), Radius, 1.0));
	}
	// The first slots spread around: face to face in one against one, the four points of the compass at four players.
	TestTrue(TEXT("slots 0 and 1 face to face"), FMath::IsNearlyEqual(Apart(0, 1), 180.0, 1.0));
	TestTrue(TEXT("slots 2 and 3 face to face"), FMath::IsNearlyEqual(Apart(2, 3), 180.0, 1.0));
	TestTrue(TEXT("slot 2 a quarter turn from slot 0"), FMath::IsNearlyEqual(Apart(0, 2), 90.0, 1.0));
	return true;
}

#endif
