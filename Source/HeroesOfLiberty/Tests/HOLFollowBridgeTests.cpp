// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Player/HOLFollowBridge.h"
#include "Player/HOLPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS

// GLORFINDEL-1122a (L22-5): the Follow bridge is an INSTANCE owned by the controller, never the class's default object (a script call on the CDO
// answered 19 727 errors "requires WorldContext"). On the real chain: GetFor(Controller) -> the script class's NewObject, kept by the controller.
namespace UE::HeroesOfLiberty::Tests::FollowBridge
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFollowBridgeIsAnInstanceTest, "HeroesOfLiberty.Follow.BridgeIsAnInstanceOfTheController", UE::HeroesOfLiberty::Tests::FollowBridge::Flags)

bool FHOLFollowBridgeIsAnInstanceTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLPlayerController* First = World->SpawnActor<AHOLPlayerController>();
	AHOLPlayerController* Second = World->SpawnActor<AHOLPlayerController>();
	if (!TestNotNull(TEXT("The first controller"), First) || !TestNotNull(TEXT("The second controller"), Second))
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}

	UHOLFollowBridge* Bridge = UHOLFollowBridge::GetFor(First);
	if (TestNotNull(TEXT("The script class (Script/Orders/HOLFollowBridge.as) gives a bridge"), Bridge))
	{
		TestFalse(TEXT("The bridge is not the class's default object (RF_ClassDefaultObject)"), Bridge->HasAnyFlags(RF_ClassDefaultObject));
		TestEqual(TEXT("The bridge belongs to its controller (GetOuter)"), Bridge->GetOuter(), static_cast<UObject*>(First));
		TestTrue(TEXT("It is an instance of the script class, not of the abstract base"), Bridge->GetClass() != UHOLFollowBridge::StaticClass());

		// The effect: one bridge per controller (the state of a Follow is the controller's), stable from one call to the next.
		TestTrue(TEXT("The same controller gets the same bridge again"), UHOLFollowBridge::GetFor(First) == Bridge);
		UHOLFollowBridge* OtherBridge = UHOLFollowBridge::GetFor(Second);
		TestNotNull(TEXT("The second controller has its bridge"), OtherBridge);
		TestTrue(TEXT("Another controller gets another bridge"), OtherBridge != Bridge);
		if (OtherBridge)
		{
			TestEqual(TEXT("The second bridge belongs to the second controller"), OtherBridge->GetOuter(), static_cast<UObject*>(Second));
		}
	}
	TestTrue(TEXT("No controller, no bridge"), UHOLFollowBridge::GetFor(nullptr) == nullptr);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
