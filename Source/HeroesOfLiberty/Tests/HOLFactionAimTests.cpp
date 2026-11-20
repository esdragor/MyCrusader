// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS

// D839 (FARAMIR-1120e, GALADRIEL-1120f): the aim of a faction power on the player's controller. SetFactionAim(i, range) arms the aim of
// power i (-1: none), Escape (the real HandleDeselect, through ScriptEscape) and any other order leave it, and the script reads the
// ground click back with ConsumeFactionAimClick. Real chain: the controller's own state, no value written by hand.
namespace UE::HeroesOfLiberty::Tests::FactionAim
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionAimTest, "HeroesOfLiberty.Faction.Aim.StateAndEscape", UE::HeroesOfLiberty::Tests::FactionAim::Flags)

bool FHOLFactionAimTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLPlayerController* Controller = World->SpawnActor<AHOLPlayerController>();
	if (!TestNotNull(TEXT("The controller"), Controller))
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}

	TestEqual(TEXT("No aim at first"), Controller->GetFactionAim(), -1);

	Controller->SetFactionAim(2, 800.0);
	TestEqual(TEXT("The aim of power 2 is armed"), Controller->GetFactionAim(), 2);
	TestEqual(TEXT("With the reach it was given"), Controller->GetFactionAimRangeCm(), 800.0);

	Controller->SetFactionAim(0, 650.0);
	TestEqual(TEXT("Aiming another power switches the aim"), Controller->GetFactionAim(), 0);
	TestEqual(TEXT("And its reach"), Controller->GetFactionAimRangeCm(), 650.0);

	FVector Point = FVector::ZeroVector;
	int32 Index = -7;
	TestFalse(TEXT("No click came in, none to consume"), Controller->ConsumeFactionAimClick(Point, Index));

	Controller->ScriptEscape();
	TestEqual(TEXT("Escape leaves the aim"), Controller->GetFactionAim(), -1);

	Controller->SetFactionAim(1, 800.0);
	Controller->SetFactionAim(-1, 800.0);
	TestEqual(TEXT("SetFactionAim(-1) leaves the aim"), Controller->GetFactionAim(), -1);

	Controller->SetFactionAim(1, 800.0);
	Controller->ScriptRightClick();
	TestEqual(TEXT("A right click (any other order) leaves the aim"), Controller->GetFactionAim(), -1);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
