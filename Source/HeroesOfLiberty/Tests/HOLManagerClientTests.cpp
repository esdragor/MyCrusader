// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLManagerClientRules.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_DEV_AUTOMATION_TESTS

// The game's client of the lobby manager (R4.1): the examples of Manager/PROTOCOL.md, copied as they are.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLManagerClientParseTest, "HeroesOfLiberty.Net.ManagerClient.Parse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLManagerClientParseTest::RunTest(const FString& Parameters)
{
	FHOLManagerStatus Status;
	TestTrue(TEXT("status read"), FHOLManagerClientRules::ParseStatus(
		TEXT(R"({ "protocolVersion": 1, "gameVersion": "2026-10-23-b4a8c8c", "lobbies": 2, "servers": 1, "maxServers": 2 })"), Status));
	TestEqual(TEXT("protocol"), Status.ProtocolVersion, 1);
	TestEqual(TEXT("game version"), Status.GameVersion, FString(TEXT("2026-10-23-b4a8c8c")));
	TestEqual(TEXT("lobbies"), Status.Lobbies, 2);
	TestEqual(TEXT("servers"), Status.Servers, 1);
	TestEqual(TEXT("max servers"), Status.MaxServers, 2);
	FHOLManagerStatus Bad;
	TestFalse(TEXT("status without a protocol"), FHOLManagerClientRules::ParseStatus(TEXT(R"({ "lobbies": 2 })"), Bad));
	TestFalse(TEXT("status not JSON"), FHOLManagerClientRules::ParseStatus(TEXT("<html>"), Bad));

	TArray<FHOLLobbySummary> Lobbies;
	TestTrue(TEXT("lobbies read"), FHOLManagerClientRules::ParseLobbies(TEXT(R"({
  "lobbies": [
    { "id": "L7", "name": "Salon de Bob", "players": 3, "maxPlayers": 8, "state": "lobby",
      "minutes": null, "canJoin": true, "canWatch": false },
    { "id": "L8", "name": "2v2", "players": 4, "maxPlayers": 4, "state": "in_game",
      "minutes": 12, "canJoin": false, "canWatch": true }
  ]
})"), Lobbies));
	if (TestEqual(TEXT("two lobbies"), Lobbies.Num(), 2))
	{
		TestEqual(TEXT("id"), Lobbies[0].Id, FString(TEXT("L7")));
		TestEqual(TEXT("name"), Lobbies[0].Name, FString(TEXT("Salon de Bob")));
		TestEqual(TEXT("players"), Lobbies[0].Players, 3);
		TestEqual(TEXT("max"), Lobbies[0].MaxPlayers, 8);
		TestTrue(TEXT("state lobby"), Lobbies[0].State == EHOLLobbyState::Lobby);
		TestEqual(TEXT("minutes null"), Lobbies[0].Minutes, -1);
		TestTrue(TEXT("can join"), Lobbies[0].bCanJoin);
		TestFalse(TEXT("cannot watch"), Lobbies[0].bCanWatch);
		TestTrue(TEXT("state in_game"), Lobbies[1].State == EHOLLobbyState::InGame);
		TestEqual(TEXT("minutes"), Lobbies[1].Minutes, 12);
		TestFalse(TEXT("cannot join"), Lobbies[1].bCanJoin);
		TestTrue(TEXT("can watch"), Lobbies[1].bCanWatch);
	}
	TArray<FHOLLobbySummary> Starting;
	TestTrue(TEXT("starting read"), FHOLManagerClientRules::ParseLobbies(TEXT(R"({ "lobbies": [ { "id": "L9", "name": "x",
		"players": 2, "maxPlayers": 8, "state": "starting", "minutes": null, "canJoin": false, "canWatch": false } ] })"), Starting));
	TestTrue(TEXT("state starting"), Starting.Num() == 1 && Starting[0].State == EHOLLobbyState::Starting);
	TArray<FHOLLobbySummary> None;
	TestTrue(TEXT("no lobby"), FHOLManagerClientRules::ParseLobbies(TEXT(R"({ "lobbies": [] })"), None) && None.IsEmpty());
	TestFalse(TEXT("unknown state"), FHOLManagerClientRules::ParseLobbies(TEXT(R"({ "lobbies": [ { "id": "L1", "name": "x",
		"players": 1, "maxPlayers": 8, "state": "lost", "minutes": null, "canJoin": true, "canWatch": false } ] })"), None));
	TestFalse(TEXT("lobbies without the list"), FHOLManagerClientRules::ParseLobbies(TEXT("{}"), None));

	FHOLManagerError Error;
	TestTrue(TEXT("error read"), FHOLManagerClientRules::ParseError(
		TEXT(R"({ "error": "lobby_full", "message": "The lobby is full (8 players)." })"), Error));
	TestEqual(TEXT("error code"), Error.Code, FString(TEXT("lobby_full")));
	TestEqual(TEXT("error message"), Error.Message, FString(TEXT("The lobby is full (8 players).")));
	TestFalse(TEXT("error without a code"), FHOLManagerClientRules::ParseError(TEXT(R"({ "message": "x" })"), Error));
	return true;
}

// Answers and failures, servers.json and the URLs.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLManagerClientFailureTest, "HeroesOfLiberty.Net.ManagerClient.Failure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLManagerClientFailureTest::RunTest(const FString& Parameters)
{
	using EKind = FHOLManagerError::EKind;
	TestTrue(TEXT("200: no failure"), FHOLManagerClientRules::ReadFailure(true, 200, TEXT("{}")).Kind == EKind::None);
	TestTrue(TEXT("no answer: unreachable"), FHOLManagerClientRules::ReadFailure(false, 0, TEXT("")).Kind == EKind::Unreachable);
	const FHOLManagerError Full = FHOLManagerClientRules::ReadFailure(true, 409,
		TEXT(R"({ "error": "lobby_full", "message": "The lobby is full (8 players)." })"));
	TestTrue(TEXT("409 with the protocol's body"), Full.Kind == EKind::Protocol);
	TestEqual(TEXT("its code kept"), Full.Code, FString(TEXT("lobby_full")));
	TestEqual(TEXT("its HTTP code"), Full.HttpCode, 409);
	TestTrue(TEXT("500 of a proxy: unreadable"), FHOLManagerClientRules::ReadFailure(true, 502, TEXT("<html>Bad gateway</html>")).Kind == EKind::Unreadable);

	TArray<FHOLManagerServer> Servers;
	TestTrue(TEXT("servers.json read"), FHOLManagerClientRules::ParseServers(
		TEXT(R"({ "servers": [ { "name": "PC de Baudouin", "url": "http://100.64.0.1:7700" }, { "name": "no url" } ] })"), Servers));
	if (TestEqual(TEXT("one server with a url"), Servers.Num(), 1))
	{
		TestEqual(TEXT("server name"), Servers[0].Name, FString(TEXT("PC de Baudouin")));
		TestEqual(TEXT("server url"), Servers[0].Url, FString(TEXT("http://100.64.0.1:7700")));
	}
	TestFalse(TEXT("servers.json not JSON"), FHOLManagerClientRules::ParseServers(TEXT("servers"), Servers));

	TestEqual(TEXT("url without a slash"), FHOLManagerClientRules::MakeUrl(TEXT("http://127.0.0.1:7700"), TEXT("/status")),
		FString(TEXT("http://127.0.0.1:7700/status")));
	TestEqual(TEXT("url with a slash"), FHOLManagerClientRules::MakeUrl(TEXT("http://127.0.0.1:7700/"), TEXT("/lobbies")),
		FString(TEXT("http://127.0.0.1:7700/lobbies")));
	return true;
}

// The lobby's state and the answer of join (R4.4): the example of Manager/PROTOCOL.md, its 8 slots written out.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLManagerClientLobbyTest, "HeroesOfLiberty.Net.ManagerClient.Lobby",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLManagerClientLobbyTest::RunTest(const FString& Parameters)
{
	const FString State = TEXT(R"({
  "id": "L7",
  "name": "Salon de Bob",
  "state": "lobby",
  "minutes": null,
  "leader": "Bob",
  "you": "Alice",
  "settings": { "map": "Plain", "nomad": false, "regicide": true, "catastrophe": false },
  "maps": [ { "name": "Plain", "maxPlayers": 8 } ],
  "slots": [
    { "index": 0, "state": "player", "playerName": "Bob",   "colorIndex": 0, "team": 1, "faction": "DawnOrder", "ready": true },
    { "index": 1, "state": "player", "playerName": "Alice", "colorIndex": 3, "team": 2, "faction": "DawnOrder", "ready": false },
    { "index": 2, "state": "open" },
    { "index": 3, "state": "closed" },
    { "index": 4, "state": "open" }, { "index": 5, "state": "open" }, { "index": 6, "state": "open" }, { "index": 7, "state": "open" }
  ],
  "match": null,
  "notice": null
})");
	FHOLLobby Lobby;
	TestTrue(TEXT("lobby read"), FHOLManagerClientRules::ParseLobby(State, Lobby));
	TestEqual(TEXT("id"), Lobby.Id, FString(TEXT("L7")));
	TestEqual(TEXT("name"), Lobby.Name, FString(TEXT("Salon de Bob")));
	TestTrue(TEXT("state lobby"), Lobby.State == EHOLLobbyState::Lobby);
	TestEqual(TEXT("minutes null"), Lobby.Minutes, -1);
	TestEqual(TEXT("leader"), Lobby.Leader, FString(TEXT("Bob")));
	TestEqual(TEXT("you"), Lobby.You, FString(TEXT("Alice")));
	TestEqual(TEXT("map"), Lobby.Map, FString(TEXT("Plain")));
	TestFalse(TEXT("nomad"), Lobby.bNomad);
	TestTrue(TEXT("regicide"), Lobby.bRegicide);
	TestFalse(TEXT("catastrophe"), Lobby.bCatastrophe);
	TestTrue(TEXT("maps"), Lobby.Maps.Num() == 1 && Lobby.Maps[0] == TEXT("Plain"));
	if (TestEqual(TEXT("8 slots"), Lobby.Slots.Num(), 8))
	{
		const FHOLLobbySlot& Alice = Lobby.Slots[1];
		TestTrue(TEXT("slot 1 a player"), Alice.State == EHOLLobbySlotState::Player);
		TestEqual(TEXT("slot 1 index"), Alice.Index, 1);
		TestEqual(TEXT("slot 1 name"), Alice.PlayerName, FString(TEXT("Alice")));
		TestEqual(TEXT("slot 1 colour"), Alice.ColorIndex, 3);
		TestEqual(TEXT("slot 1 team"), Alice.Team, 2);
		TestEqual(TEXT("slot 1 faction"), Alice.Faction, FString(TEXT("DawnOrder")));
		TestFalse(TEXT("slot 1 not ready"), Alice.bReady);
		TestTrue(TEXT("slot 0 ready"), Lobby.Slots[0].bReady);
		TestTrue(TEXT("slot 2 open"), Lobby.Slots[2].State == EHOLLobbySlotState::Open);
		TestTrue(TEXT("slot 3 closed"), Lobby.Slots[3].State == EHOLLobbySlotState::Closed);
	}
	TestEqual(TEXT("2 players"), Lobby.GetNumPlayers(), 2);
	TestEqual(TEXT("your slot"), Lobby.FindYourSlot(), 1);
	TestFalse(TEXT("no match"), Lobby.bHasMatch);
	TestTrue(TEXT("no notice"), Lobby.Notice.IsEmpty());

	// In game: match and a notice.
	FString InGame = State.Replace(TEXT(R"("state": "lobby")"), TEXT(R"("state": "in_game")"))
		.Replace(TEXT(R"("minutes": null)"), TEXT(R"("minutes": 12)"))
		.Replace(TEXT(R"("match": null)"), TEXT(R"("match": { "matchId": "M20261023-153012-3", "port": 7778, "playerName": "Alice", "gameVersion": "2026-10-23-b4a8c8c" })"))
		.Replace(TEXT(R"("notice": null)"), TEXT(R"("notice": "leader_changed")"));
	FHOLLobby Playing;
	TestTrue(TEXT("in game read"), FHOLManagerClientRules::ParseLobby(InGame, Playing));
	TestTrue(TEXT("state in_game"), Playing.State == EHOLLobbyState::InGame);
	TestEqual(TEXT("minutes"), Playing.Minutes, 12);
	TestTrue(TEXT("match"), Playing.bHasMatch);
	TestEqual(TEXT("match id"), Playing.MatchId, FString(TEXT("M20261023-153012-3")));
	TestEqual(TEXT("match port"), Playing.MatchPort, 7778);
	TestEqual(TEXT("match name"), Playing.MatchPlayerName, FString(TEXT("Alice")));
	TestEqual(TEXT("match version"), Playing.MatchGameVersion, FString(TEXT("2026-10-23-b4a8c8c")));
	TestEqual(TEXT("notice"), Playing.Notice, FString(TEXT("leader_changed")));

	// Without a token: you is null.
	FHOLLobby Anonymous;
	TestTrue(TEXT("read without a token"), FHOLManagerClientRules::ParseLobby(State.Replace(TEXT(R"("you": "Alice")"), TEXT(R"("you": null)")), Anonymous));
	TestTrue(TEXT("you empty"), Anonymous.You.IsEmpty());
	TestEqual(TEXT("no slot of yours"), Anonymous.FindYourSlot(), static_cast<int32>(INDEX_NONE));

	FHOLLobby Bad;
	TestFalse(TEXT("a slot missing"), FHOLManagerClientRules::ParseLobby(State.Replace(TEXT(R"({ "index": 6, "state": "open" }, )"), TEXT("")), Bad));
	TestFalse(TEXT("unknown slot state"), FHOLManagerClientRules::ParseLobby(State.Replace(TEXT(R"("state": "closed")"), TEXT(R"("state": "gone")")), Bad));
	TestFalse(TEXT("not JSON"), FHOLManagerClientRules::ParseLobby(TEXT("<html>"), Bad));

	FString Token;
	int32 SlotIndex = INDEX_NONE;
	FHOLLobby Joined;
	TestTrue(TEXT("join read"), FHOLManagerClientRules::ParseJoin(FString(TEXT(R"({ "token": "9f2c4e0b7a3d4c1e8b6f5a2d0c9e7b14", "slot": 1, "lobby": )")) + State + TEXT(" }"),
		Token, SlotIndex, Joined));
	TestEqual(TEXT("token"), Token, FString(TEXT("9f2c4e0b7a3d4c1e8b6f5a2d0c9e7b14")));
	TestEqual(TEXT("slot"), SlotIndex, 1);
	TestEqual(TEXT("joined lobby"), Joined.Id, FString(TEXT("L7")));
	TestFalse(TEXT("join without a token"), FHOLManagerClientRules::ParseJoin(FString(TEXT(R"({ "slot": 1, "lobby": )")) + State + TEXT(" }"), Token, SlotIndex, Joined));
	return true;
}

namespace
{
	TSharedPtr<FJsonObject> ReadBody(const FString& Body)
	{
		TSharedPtr<FJsonObject> Object;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Body), Object);
		return Object;
	}
}

// The lobby's writing (R4.5): the bodies of PUT me, settings and slots; the rights by line (M6-Q3, M6-Q4).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLManagerClientWriteTest, "HeroesOfLiberty.Net.ManagerClient.Write",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLManagerClientWriteTest::RunTest(const FString& Parameters)
{
	// Only the fields given.
	const TSharedPtr<FJsonObject> Me = ReadBody(FHOLManagerClientRules::MakeMeBody(4, 2, true));
	if (TestTrue(TEXT("me body JSON"), Me.IsValid()))
	{
		TestEqual(TEXT("me colour"), static_cast<int32>(Me->GetNumberField(TEXT("colorIndex"))), 4);
		TestEqual(TEXT("me team"), static_cast<int32>(Me->GetNumberField(TEXT("team"))), 2);
		TestTrue(TEXT("me ready"), Me->GetBoolField(TEXT("ready")));
	}
	const TSharedPtr<FJsonObject> ReadyOnly = ReadBody(FHOLManagerClientRules::MakeMeBody({}, {}, false));
	if (TestTrue(TEXT("ready body JSON"), ReadyOnly.IsValid()))
	{
		TestEqual(TEXT("ready alone"), ReadyOnly->Values.Num(), 1);
		TestFalse(TEXT("not ready"), ReadyOnly->GetBoolField(TEXT("ready")));
	}
	const TSharedPtr<FJsonObject> ColorOnly = ReadBody(FHOLManagerClientRules::MakeMeBody(0, {}, {}));
	TestTrue(TEXT("colour 0 alone"), ColorOnly.IsValid() && ColorOnly->Values.Num() == 1 && ColorOnly->HasField(TEXT("colorIndex")));

	const TSharedPtr<FJsonObject> Settings = ReadBody(FHOLManagerClientRules::MakeSettingsBody(TEXT("Plain"), true, false, true));
	if (TestTrue(TEXT("settings body JSON"), Settings.IsValid()))
	{
		TestEqual(TEXT("settings map"), Settings->GetStringField(TEXT("map")), FString(TEXT("Plain")));
		TestTrue(TEXT("settings nomad"), Settings->GetBoolField(TEXT("nomad")));
		TestFalse(TEXT("settings regicide"), Settings->GetBoolField(TEXT("regicide")));
		TestTrue(TEXT("settings catastrophe"), Settings->GetBoolField(TEXT("catastrophe")));
	}
	const TSharedPtr<FJsonObject> Closed = ReadBody(FHOLManagerClientRules::MakeSlotBody(false));
	TestTrue(TEXT("slot closed"), Closed.IsValid() && Closed->GetStringField(TEXT("state")) == TEXT("closed"));
	const TSharedPtr<FJsonObject> Opened = ReadBody(FHOLManagerClientRules::MakeSlotBody(true));
	TestTrue(TEXT("slot open"), Opened.IsValid() && Opened->GetStringField(TEXT("state")) == TEXT("open"));

	// Bob leads, Alice is you: slot 0 Bob ready, slot 1 Alice not ready, 2 open, 3 closed.
	FHOLLobby Lobby;
	Lobby.Leader = TEXT("Bob");
	Lobby.You = TEXT("Alice");
	Lobby.Slots.SetNum(8);
	for (int32 Index = 0; Index < 8; ++Index)
	{
		Lobby.Slots[Index].Index = Index;
	}
	Lobby.Slots[0].State = EHOLLobbySlotState::Player;
	Lobby.Slots[0].PlayerName = TEXT("Bob");
	Lobby.Slots[0].bReady = true;
	Lobby.Slots[1].State = EHOLLobbySlotState::Player;
	Lobby.Slots[1].PlayerName = TEXT("Alice");
	Lobby.Slots[3].State = EHOLLobbySlotState::Closed;

	TestFalse(TEXT("Alice does not lead"), Lobby.IsYouLeader());
	TestTrue(TEXT("Alice edits her line"), Lobby.CanEditSlot(1));
	TestFalse(TEXT("Alice does not edit Bob's"), Lobby.CanEditSlot(0));
	TestFalse(TEXT("nor an open slot"), Lobby.CanEditSlot(2));
	TestFalse(TEXT("Alice does not close a slot"), Lobby.CanToggleSlot(2));
	TestFalse(TEXT("Alice does not start"), Lobby.CanStart());

	FHOLLobby Led = Lobby;
	Led.You = TEXT("Bob");
	TestTrue(TEXT("Bob leads"), Led.IsYouLeader());
	TestTrue(TEXT("Bob edits his line"), Led.CanEditSlot(0));
	TestFalse(TEXT("Bob does not edit Alice's"), Led.CanEditSlot(1));
	TestTrue(TEXT("Bob closes an open slot"), Led.CanToggleSlot(2));
	TestTrue(TEXT("Bob opens a closed slot"), Led.CanToggleSlot(3));
	TestFalse(TEXT("not a held slot"), Led.CanToggleSlot(1));
	TestFalse(TEXT("not out of range"), Led.CanToggleSlot(8));
	TestFalse(TEXT("Alice not ready: no start"), Led.CanStart());
	Led.Slots[1].bReady = true;
	TestTrue(TEXT("all ready: start"), Led.CanStart());
	FHOLLobby Alone = Led;
	Alone.Slots[1] = FHOLLobbySlot();
	Alone.Slots[1].Index = 1;
	TestFalse(TEXT("alone: no start"), Alone.CanStart());

	FHOLLobby Anonymous = Lobby;
	Anonymous.You.Reset();
	Anonymous.Leader.Reset();
	TestFalse(TEXT("no token: no leader"), Anonymous.IsYouLeader());
	TestFalse(TEXT("no token: no line"), Anonymous.CanEditSlot(1));
	return true;
}

// Toward the match (R4.7): the lobby's match, the address the client opens, the spectator's port.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLManagerClientMatchTest, "HeroesOfLiberty.Net.ManagerClient.Match",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLManagerClientMatchTest::RunTest(const FString& Parameters)
{
	FString Json = TEXT(R"({ "id": "L7", "name": "Salon de Bob", "state": "in_game", "minutes": 0, "leader": "Bob", "you": "Alice",
  "settings": { "map": "Plain", "nomad": false, "regicide": false, "catastrophe": false }, "slots": [)");
	for (int32 SlotIndex = 0; SlotIndex < 8; ++SlotIndex)
	{
		Json += FString::Printf(TEXT(R"(%s{ "index": %d, "state": "closed" })"), SlotIndex ? TEXT(", ") : TEXT(""), SlotIndex);
	}
	Json += TEXT(R"(], "match": { "matchId": "M20261023-153012-3", "port": 7778, "playerName": "Alice", "gameVersion": "2026-10-23-b4a8c8c" },
  "notice": null })");
	FHOLLobby Lobby;
	TestTrue(TEXT("lobby in game read"), FHOLManagerClientRules::ParseLobby(Json, Lobby));
	TestTrue(TEXT("in game"), Lobby.State == EHOLLobbyState::InGame);
	TestTrue(TEXT("match read"), Lobby.bHasMatch);
	TestEqual(TEXT("match id"), Lobby.MatchId, FString(TEXT("M20261023-153012-3")));
	TestEqual(TEXT("match port"), Lobby.MatchPort, 7778);
	TestEqual(TEXT("match name"), Lobby.MatchPlayerName, FString(TEXT("Alice")));
	TestEqual(TEXT("match version"), Lobby.MatchGameVersion, FString(TEXT("2026-10-23-b4a8c8c")));

	TestEqual(TEXT("player's address"), FHOLManagerClientRules::MakeMatchAddress(TEXT("http://127.0.0.1:7700"), 7778, TEXT("Alice"),
		TEXT("2026-10-23-b4a8c8c")), FString(TEXT("127.0.0.1:7778?Name=Alice?Version=2026-10-23-b4a8c8c")));
	TestEqual(TEXT("manager's host, path dropped"), FHOLManagerClientRules::MakeMatchAddress(TEXT("http://hote.tail1234.ts.net:7700/"), 7779,
		TEXT("Bob"), TEXT("v1")), FString(TEXT("hote.tail1234.ts.net:7779?Name=Bob?Version=v1")));
	TestEqual(TEXT("host without a port"), FHOLManagerClientRules::MakeMatchAddress(TEXT("http://192.168.1.5"), 7777, TEXT("Bob"), TEXT("v1")),
		FString(TEXT("192.168.1.5:7777?Name=Bob?Version=v1")));
	TestEqual(TEXT("name encoded"), FHOLManagerClientRules::MakeMatchAddress(TEXT("http://127.0.0.1:7700"), 7778, TEXT("Le Roi&Co"), TEXT("v1")),
		FString(TEXT("127.0.0.1:7778?Name=Le%20Roi%26Co?Version=v1")));
	TestEqual(TEXT("spectator: no name"), FHOLManagerClientRules::MakeMatchAddress(TEXT("http://127.0.0.1:7700"), 7778, FString(), TEXT("v1")),
		FString(TEXT("127.0.0.1:7778?Version=v1")));
	TestEqual(TEXT("no port"), FHOLManagerClientRules::MakeMatchAddress(TEXT("http://127.0.0.1:7700"), 0, TEXT("Bob"), TEXT("v1")), FString());
	TestEqual(TEXT("no host"), FHOLManagerClientRules::MakeMatchAddress(TEXT("http://"), 7778, TEXT("Bob"), TEXT("v1")), FString());

	FString MatchId;
	int32 Port = 0;
	TestTrue(TEXT("watch read"), FHOLManagerClientRules::ParseWatch(TEXT(R"({ "matchId": "M20261023-153012-3", "port": 7778, "minutes": 12 })"),
		MatchId, Port));
	TestEqual(TEXT("watch match"), MatchId, FString(TEXT("M20261023-153012-3")));
	TestEqual(TEXT("watch port"), Port, 7778);
	TestFalse(TEXT("watch without a port"), FHOLManagerClientRules::ParseWatch(TEXT(R"({ "matchId": "M1" })"), MatchId, Port));
	TestFalse(TEXT("watch not JSON"), FHOLManagerClientRules::ParseWatch(TEXT("<html>"), MatchId, Port));
	return true;
}

#endif
