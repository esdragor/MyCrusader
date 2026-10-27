// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dom/JsonObject.h"
#include "Map/HOLMapSizeRules.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Net/HOLManagerClientRules.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_DEV_AUTOMATION_TESTS

// TC3 loop 2 (map sizes in the lobby): what the client reads and sends of the lobby's size and "Équipes ensemble"
// (Manager/PROTOCOL.md), and the match.json version 3 the manager writes.
namespace UE::HeroesOfLiberty::Tests::MapSizeLobby
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

	TSharedPtr<FJsonObject> ReadBody(const FString& Body)
	{
		TSharedPtr<FJsonObject> Object;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Body), Object);
		return Object;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeLobbyReadTest, "HeroesOfLiberty.MapSize.Lobby.Read", UE::HeroesOfLiberty::Tests::MapSizeLobby::Flags)
bool FHOLMapSizeLobbyReadTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSizeLobby;

	FHOLLobby Lobby;
	if (TestTrue(TEXT("state with a size read"), FHOLManagerClientRules::ParseLobby(MakeState(TEXT(R"(, "size": "Small", "teamsTogether": false)")), Lobby)))
	{
		TestEqual(TEXT("size key"), Lobby.Size, FString(TEXT("Small")));
		TestFalse(TEXT("teams not together"), Lobby.bTeamsTogether);
		TestTrue(TEXT("size Small"), Lobby.GetMapSize() == EHOLMapSize::Small);
		TestEqual(TEXT("2 places"), Lobby.GetNumPlaces(), 2);
		// The leader (Bob, you) opens or closes the slots within the places only.
		TestTrue(TEXT("Small: slot 1 (open, within the places) can be closed"), Lobby.CanToggleSlot(1));
		TestFalse(TEXT("Small: slot 2 (past the places) cannot be opened"), Lobby.CanToggleSlot(2));
	}
	// An older manager: no size, teams together, the eight slots.
	if (TestTrue(TEXT("state without a size read"), FHOLManagerClientRules::ParseLobby(MakeState(TEXT("")), Lobby)))
	{
		TestTrue(TEXT("no size"), Lobby.Size.IsEmpty() && Lobby.GetMapSize() == EHOLMapSize::None);
		TestTrue(TEXT("teams together by default"), Lobby.bTeamsTogether);
		TestEqual(TEXT("8 places without a size"), Lobby.GetNumPlaces(), FHOLMatchConfig::MaxSlots);
		TestTrue(TEXT("no size: slot 2 can be opened"), Lobby.CanToggleSlot(2));
	}
	FHOLManagerClientRules::ParseLobby(MakeState(TEXT(R"(, "size": "huge")")), Lobby);
	TestTrue(TEXT("size key without case"), Lobby.GetMapSize() == EHOLMapSize::Huge);

	TArray<FHOLLobbySummary> Rows;
	if (TestTrue(TEXT("list read"), FHOLManagerClientRules::ParseLobbies(TEXT(R"({ "lobbies": [
		{ "id": "L7", "name": "Salon", "map": "Plain", "size": "Medium", "players": 1, "maxPlayers": 4, "state": "lobby", "minutes": null, "canJoin": true, "canWatch": false },
		{ "id": "L8", "name": "Vieux", "players": 1, "maxPlayers": 8, "state": "lobby", "minutes": null, "canJoin": true, "canWatch": false } ] })"), Rows))
		&& TestEqual(TEXT("two rows"), Rows.Num(), 2))
	{
		TestEqual(TEXT("row map"), Rows[0].Map, FString(TEXT("Plain")));
		TestEqual(TEXT("row size"), Rows[0].Size, FString(TEXT("Medium")));
		TestTrue(TEXT("older row: no map nor size"), Rows[1].Map.IsEmpty() && Rows[1].Size.IsEmpty());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeLobbySendTest, "HeroesOfLiberty.MapSize.Lobby.Send", UE::HeroesOfLiberty::Tests::MapSizeLobby::Flags)
bool FHOLMapSizeLobbySendTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSizeLobby;

	const TSharedPtr<FJsonObject> Sized = ReadBody(FHOLManagerClientRules::MakeSettingsBody(TEXT("Plain"), false, false, false, TEXT("Small"), false));
	if (TestTrue(TEXT("settings with a size"), Sized.IsValid()))
	{
		TestEqual(TEXT("size sent"), Sized->GetStringField(TEXT("size")), FString(TEXT("Small")));
		TestTrue(TEXT("teams together sent"), Sized->HasField(TEXT("teamsTogether")) && !Sized->GetBoolField(TEXT("teamsTogether")));
	}
	const TSharedPtr<FJsonObject> Plain = ReadBody(FHOLManagerClientRules::MakeSettingsBody(TEXT("Plain"), false, false, false));
	TestTrue(TEXT("no size nor teams together unless given"), Plain.IsValid() && !Plain->HasField(TEXT("size")) && !Plain->HasField(TEXT("teamsTogether")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeLobbyMatchConfigTest, "HeroesOfLiberty.MapSize.Lobby.ManagerReference", UE::HeroesOfLiberty::Tests::MapSizeLobby::Flags)
bool FHOLMapSizeLobbyMatchConfigTest::RunTest(const FString& Parameters)
{
	// The match.json version 3 the lobby manager writes (Manager/HOLManager.Tests checks it against its own output).
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Manager/HOLManager.Tests/Data/match_v3_reference.json"));
	FString Json;
	if (!TestTrue(FString::Printf(TEXT("reference file read (%s)"), *Path), FFileHelper::LoadFileToString(Json, *Path)))
	{
		return false;
	}
	FHOLMatchConfig Read;
	FString Error;
	if (!TestTrue(FString::Printf(TEXT("reference parsed (%s)"), *Error), FHOLMatchConfig::FromJson(Json, Read, &Error)))
	{
		return false;
	}
	TestEqual(TEXT("version 3"), Read.Version, 3);
	TestTrue(TEXT("Large, places drawn, teams together"), Read.MapSize == EHOLMapSize::Large && Read.bDrawPlaces && Read.bTeamsTogether);
	TestEqual(TEXT("file played"), FHOLMapSizeRules::GetMapFile(Read.Map, Read.MapSize).ToString(), FString(TEXT("/Game/Maps/Plain_Large.Plain_Large")));
	TestTrue(TEXT("fits its map"), FHOLMapSizeRules::CheckPlaces(Read));
	return true;
}

#endif
