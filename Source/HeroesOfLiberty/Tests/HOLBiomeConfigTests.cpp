// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dom/JsonObject.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Net/HOLManagerClientRules.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_DEV_AUTOMATION_TESTS

// B2 (BiomeInProgress.md, D651-D656): the lobby's biome through the match configuration (biome), the manager's lobby view and the settings body.
namespace UE::HeroesOfLiberty::Tests::BiomeConfig
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A lobby's state as the manager gives it, with Settings put in "settings" after "map". */
	FString MakeState(const TCHAR* Settings)
	{
		return FString::Printf(TEXT(R"({ "id": "L7", "name": "Salon", "state": "lobby", "minutes": null, "leader": "Bob", "you": "Bob",
			"settings": { "map": "Plain", "nomad": false, "regicide": false, "catastrophe": false%s },
			"maps": [ { "name": "Plain", "maxPlayers": 8, "sizes": [ { "key": "Small", "places": 2 } ] } ],
			"slots": [ { "index": 0, "state": "player", "playerName": "Bob", "colorIndex": 0, "team": 0, "faction": "DawnOrder", "ready": false },
				{ "index": 1, "state": "open" }, { "index": 2, "state": "closed" }, { "index": 3, "state": "closed" },
				{ "index": 4, "state": "closed" }, { "index": 5, "state": "closed" }, { "index": 6, "state": "closed" },
				{ "index": 7, "state": "closed" } ],
			"match": null, "notice": null })"), Settings);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBiomeConfigFileTest, "HeroesOfLiberty.Biome.Config.File", UE::HeroesOfLiberty::Tests::BiomeConfig::Flags)
bool FHOLBiomeConfigFileTest::RunTest(const FString& Parameters)
{
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	TestEqual(TEXT("a configuration without the field: the map's own biome (the scenarios, the tests)"), Config.Biome, FString(TEXT("default")));

	Config.Biome = TEXT("snow");
	FHOLMatchConfig Read;
	FString Error;
	TestTrue(TEXT("the configuration is read back"), FHOLMatchConfig::FromJson(Config.ToJson(), Read, &Error));
	TestEqual(TEXT("the biome survives the file"), Read.Biome, FString(TEXT("snow")));

	FHOLMatchConfig FromManager;
	TestTrue(TEXT("the manager's match.json"), FHOLMatchConfig::FromJson(TEXT("{\"version\": 3, \"map\": \"/Game/Maps/Plain.Plain\", \"biome\": \"dark_forest\"}"), FromManager, &Error));
	TestEqual(TEXT("... its biome by key"), FromManager.Biome, FString(TEXT("dark_forest")));
	TestTrue(TEXT("a file without it"), FHOLMatchConfig::FromJson(TEXT("{\"version\": 3, \"map\": \"/Game/Maps/Plain.Plain\"}"), FromManager, &Error));
	TestEqual(TEXT("... is the map's own"), FromManager.Biome, FString(TEXT("default")));

	// The manager's reference files (Manager/HOLManager.Tests checks them against its own output) carry "biome": "default".
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Manager/HOLManager.Tests/Data/match_v3_reference.json"));
	FString Json;
	if (TestTrue(FString::Printf(TEXT("reference file read (%s)"), *Path), FFileHelper::LoadFileToString(Json, *Path)))
	{
		TestTrue(TEXT("the reference file says the biome"), Json.Contains(TEXT("\"biome\": \"default\"")));
		FHOLMatchConfig Reference;
		TestTrue(TEXT("the reference file is read"), FHOLMatchConfig::FromJson(Json, Reference, &Error));
		TestEqual(TEXT("... its biome"), Reference.Biome, FString(TEXT("default")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBiomeConfigLobbyTest, "HeroesOfLiberty.Biome.Config.Lobby", UE::HeroesOfLiberty::Tests::BiomeConfig::Flags)
bool FHOLBiomeConfigLobbyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BiomeConfig;

	FHOLLobby Lobby;
	if (TestTrue(TEXT("state with a biome read"), FHOLManagerClientRules::ParseLobby(MakeState(TEXT(R"(, "biome": "autumn")")), Lobby)))
	{
		TestEqual(TEXT("the lobby's biome"), Lobby.Biome, FString(TEXT("autumn")));
		TestEqual(TEXT("... its key"), Lobby.GetBiomeKey(), FString(TEXT("autumn")));
	}
	// An older manager: no biome, shown as the map's own.
	if (TestTrue(TEXT("state without a biome read"), FHOLManagerClientRules::ParseLobby(MakeState(TEXT("")), Lobby)))
	{
		TestTrue(TEXT("no biome"), Lobby.Biome.IsEmpty());
		TestEqual(TEXT("... shows the map's own"), Lobby.GetBiomeKey(), FString(TEXT("default")));
	}

	// The settings body: the biome only when given.
	const auto ReadBody = [](const FString& Body)
	{
		TSharedPtr<FJsonObject> Object;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Body), Object);
		return Object;
	};
	const TSharedPtr<FJsonObject> Sent = ReadBody(FHOLManagerClientRules::MakeSettingsBody(TEXT("Plain"), false, false, false, FString(), {}, FString(), TEXT("snow")));
	if (TestTrue(TEXT("settings with a biome"), Sent.IsValid()))
	{
		TestEqual(TEXT("biome sent"), Sent->GetStringField(TEXT("biome")), FString(TEXT("snow")));
		TestFalse(TEXT("no fog sent unless given"), Sent->HasField(TEXT("fog")));
	}
	const TSharedPtr<FJsonObject> Plain = ReadBody(FHOLManagerClientRules::MakeSettingsBody(TEXT("Plain"), false, false, false));
	TestTrue(TEXT("no biome unless given"), Plain.IsValid() && !Plain->HasField(TEXT("biome")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
