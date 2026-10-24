// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// R3.9: a match launched by the lobby manager waits for its named players, 60 s at most, then starts without the absent.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartRulesTest, "HeroesOfLiberty.Match.StartRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLStartRulesTest::RunTest(const FString& Parameters)
{
	const TArray<FString> Expected = { TEXT("Alice"), TEXT("Bob") };

	TestEqual(TEXT("nobody here: both missing, in order"), FString::Join(FHOLStartRules::GetMissing(Expected, {}), TEXT(", ")),
		FString(TEXT("Alice, Bob")));
	const TArray<FString> BobHere = { TEXT(" bob ") };
	TestEqual(TEXT("Bob here (trimmed, case ignored): Alice missing"), FString::Join(FHOLStartRules::GetMissing(Expected, BobHere),
		TEXT(", ")), FString(TEXT("Alice")));
	const TArray<FString> AllHere = { TEXT("Bob"), TEXT("Alice"), TEXT("Chloe") };
	TestEqual(TEXT("all here (and a stranger): nobody missing"), FHOLStartRules::GetMissing(Expected, AllHere).Num(), 0);

	TestFalse(TEXT("one missing at 10 s of 60: waits"), FHOLStartRules::ShouldStart(1, 10.0, 60.0));
	TestFalse(TEXT("one missing just before the limit: waits"), FHOLStartRules::ShouldStart(1, 59.9, 60.0));
	TestTrue(TEXT("one missing at the limit: starts"), FHOLStartRules::ShouldStart(1, 60.0, 60.0));
	TestTrue(TEXT("all here at 3 s: starts"), FHOLStartRules::ShouldStart(0, 3.0, 60.0));
	TestTrue(TEXT("all here at once: starts"), FHOLStartRules::ShouldStart(0, 0.0, 60.0));
	TestTrue(TEXT("no wait limit (0): starts at once"), FHOLStartRules::ShouldStart(2, 0.0, 0.0));
	return true;
}

#endif
