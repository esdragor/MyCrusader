// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestHooks.h"
#include "HAL/FileManager.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLStartRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA1.1 checks: the 5 Peasants of every occupied slot at the start of a real match, none in the test scenarios unless
// their .json asks.
namespace UE::HeroesOfLiberty::Tests::StartingPeasants
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLStartingPeasantsCountTest, "HeroesOfLiberty.Match.StartingPeasants.Count",
	UE::HeroesOfLiberty::Tests::StartingPeasants::Flags)

bool FHOLStartingPeasantsCountTest::RunTest(const FString& Parameters)
{
	// The rule of the game, from DefaultGame.ini.
	const int32 Setting = GetDefault<UHOLMatchSettings>()->StartingPeasants;
	TestEqual(TEXT("The setting: 5 Peasants"), Setting, 5);

	// A real match: the menu (its first configuration is the default one) and the lobby (a file without the field).
	TestEqual(TEXT("Menu: the setting"), FHOLStartRules::GetNumStartingPeasants(FHOLMatchConfig::MakeDefault(), Setting), 5);
	FHOLMatchConfig Lobby;
	TestTrue(TEXT("Lobby file read"), FHOLMatchConfig::FromJson(
		TEXT(R"({ "version": 2, "map": "/Game/Maps/Plain.Plain", "slots": [ { "state": "Human", "colorIndex": 0, "playerName": "Bob" }, { "state": "AI", "colorIndex": 1 } ] })"),
		Lobby));
	TestEqual(TEXT("Lobby: the setting"), FHOLStartRules::GetNumStartingPeasants(Lobby, Setting), 5);

	// Every scenario, through the reading of hol.Scenario: none (they make their own Peasants), unless its .json asks for
	// them with "startingPeasants" (starting_peasants: 5, the trial of the rule).
	const FString ScenarioDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Scenarios")));
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(ScenarioDir, TEXT("*.json")), true, false);
	TestTrue(TEXT("Scenarios with a .json found"), Files.Num() >= 10);
	bool bTrialFound = false;
	for (const FString& File : Files)
	{
		FString Json;
		FHOLMatchConfig Config;
		TestTrue(*FString::Printf(TEXT("%s read"), *File), FFileHelper::LoadFileToString(Json, *FPaths::Combine(ScenarioDir, File))
			&& HOLTestHooks::MakeScenarioConfig(&Json, Config));
		const bool bAsks = Json.Contains(TEXT("\"startingPeasants\""), ESearchCase::IgnoreCase);
		const int32 Num = FHOLStartRules::GetNumStartingPeasants(Config, Setting);
		if (File.Equals(TEXT("starting_peasants.json"), ESearchCase::IgnoreCase))
		{
			bTrialFound = true;
			TestEqual(TEXT("starting_peasants.json: the 5 of the rule"), Num, 5);
		}
		else if (bAsks)
		{
			TestTrue(*FString::Printf(TEXT("%s: the number it asks for"), *File), Num == Config.StartingPeasants && Num >= 0);
		}
		else
		{
			TestEqual(*FString::Printf(TEXT("%s: no starting Peasant"), *File), Num, 0);
		}
	}
	TestTrue(TEXT("Trial scenario starting_peasants.json found"), bTrialFound);
	FHOLMatchConfig NoFile;
	TestTrue(TEXT("Scenario without .json"), HOLTestHooks::MakeScenarioConfig(nullptr, NoFile));
	TestEqual(TEXT("Scenario without .json: no starting Peasant"), FHOLStartRules::GetNumStartingPeasants(NoFile, Setting), 0);

	// A scenario whose .json asks for them (the rule's or the AI's trials).
	for (const int32 Asked : { 5, 3 })
	{
		const FString Json = FString::Printf(
			TEXT(R"({ "map": "/Game/Maps/Plain.Plain", "startingPeasants": %d, "slots": [ { "state": "Human", "colorIndex": 0 }, { "state": "AI", "colorIndex": 1 } ] })"),
			Asked);
		FHOLMatchConfig Config;
		TestTrue(*FString::Printf(TEXT("Scenario asking %d read"), Asked), HOLTestHooks::MakeScenarioConfig(&Json, Config));
		TestEqual(*FString::Printf(TEXT("Scenario asking %d: %d"), Asked, Asked), FHOLStartRules::GetNumStartingPeasants(Config, Setting), Asked);
	}
	return true;
}

#endif
