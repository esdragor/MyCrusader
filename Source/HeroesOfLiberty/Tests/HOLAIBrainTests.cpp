// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA0.4 checks: the AI brain is a script class (Script/AI/HOLAIBrain.as) deriving the C++ UHOLAIBrainSubsystem.
namespace UE::HeroesOfLiberty::Tests::AIBrain
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A bare game world, as the other tests' worlds: no game state, no begin play. */
	struct FBareWorld
	{
		UWorld* World = nullptr;

		FBareWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
		}

		~FBareWorld()
		{
			if (World)
			{
				GEngine->DestroyWorldContext(World);
				World->DestroyWorld(false);
			}
		}
	};
}

/**
 * IA0.4 B1: a game world has a brain whose class comes from the script (not the abstract C++ class), and ThinkNow reaches
 * the script's Think. Red without the .as: no subclass, no brain.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIBrainExistsTest, "HeroesOfLiberty.AI.Brain.Exists", UE::HeroesOfLiberty::Tests::AIBrain::Flags)
bool FHOLAIBrainExistsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIBrain;

	FBareWorld Bare;
	if (!TestNotNull(TEXT("Test world"), Bare.World))
	{
		return false;
	}
	UHOLAIBrainSubsystem* Brain = UHOLAIBrainSubsystem::Find(Bare.World);
	if (!TestNotNull(TEXT("A brain subsystem in a game world (a script class deriving UHOLAIBrainSubsystem)"), Brain))
	{
		return false;
	}
	const UClass* BrainClass = Brain->GetClass();
	TestTrue(FString::Printf(TEXT("Brain class %s is a subclass, not the C++ base"), *BrainClass->GetName()),
		BrainClass != UHOLAIBrainSubsystem::StaticClass());
	TestFalse(FString::Printf(TEXT("Brain class %s comes from the script (not native)"), *BrainClass->GetName()),
		BrainClass->HasAnyClassFlags(CLASS_Native));

	// No game state: no AI slot, and nothing breaks.
	TestEqual(TEXT("No AI slot without a game state"), Brain->GetAISlots().Num(), 0);

	// The script's Think answers true (the C++ base answers false).
	TestEqual(TEXT("No thought before ThinkNow"), Brain->GetNumThoughts(), 0);
	TestTrue(TEXT("ThinkNow(1) reaches the script's Think"), Brain->ThinkNow(1));
	TestEqual(TEXT("One thought counted"), Brain->GetNumThoughts(), 1);
	return true;
}

#endif
