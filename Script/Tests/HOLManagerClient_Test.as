// Lot MIG-C-ManagerClient (CELEBORN-1117l; D830, D836, D839): the script port of the tests of HOLManagerClientTests.cpp (Parse, LobbyAction,
// Failure, Lobby, Write, AI, Match, Origin, Spectator, and the countdown of Reconnect) and of the two lobby tests of HOLMapSizeLobbyTests.cpp
// (Read, Send), run against the read library of FHOLManagerClientRules (HOLManagerClientReadLibrary, namespace HOLManagerClientRead): JSON in,
// flat text out. Same vectors as the C++ tests; a JSON body that C++ read with FJsonObject is read here with HOLManagerClientRead::JsonField
// (same reader). Differences, all stated: the C++ tests that built an FHOLLobby by hand (Write rights, AI start rules) go through the JSON of
// the same lobby; "Anonymous" of Write keeps the leader (a null leader is not asserted). Not ported: the step loop of FHOLReconnect (stateful,
// stays C++), the two ManagerReference tests (FHOLMatchConfig::FromJson + files: no FHOLManagerClientRules call, see CELEBORN-1117l.md).
// This file compiles only after the C++ lot of the library is built (the build is pending); the C++ tests are NOT removed here.

void MC_Eq(FUnitTest& T, const FString& Expected, const FString& Actual, const FString& What)
{
	T.AssertEquals(Expected, Actual, What);
}

// The lobby's state: Bob leads (Leader), You is the token's name, Slots the eight slots' JSON.
FString MC_State(const FString& You, const FString& Slots)
{
	return "{ \"id\": \"L7\", \"name\": \"Salon de Bob\", \"state\": \"lobby\", \"minutes\": null, \"leader\": \"Bob\", \"you\": " + You
		+ ", \"settings\": { \"map\": \"Plain\", \"nomad\": false, \"regicide\": true, \"catastrophe\": false }, \"maps\": [ { \"name\": \"Plain\", \"maxPlayers\": 8 } ],"
		+ " \"slots\": [ " + Slots + " ], \"match\": null, \"notice\": null }";
}

const FString MC_Bob = "{ \"index\": 0, \"state\": \"player\", \"playerName\": \"Bob\", \"colorIndex\": 0, \"team\": 1, \"faction\": \"DawnOrder\", \"ready\": true }";
const FString MC_Bob_NotReady = "{ \"index\": 0, \"state\": \"player\", \"playerName\": \"Bob\", \"colorIndex\": 0, \"team\": 1, \"faction\": \"DawnOrder\", \"ready\": false }";
const FString MC_Alice = "{ \"index\": 1, \"state\": \"player\", \"playerName\": \"Alice\", \"colorIndex\": 3, \"team\": 2, \"faction\": \"DawnOrder\", \"ready\": false }";
const FString MC_Alice_Ready = "{ \"index\": 1, \"state\": \"player\", \"playerName\": \"Alice\", \"colorIndex\": 3, \"team\": 2, \"faction\": \"DawnOrder\", \"ready\": true }";
const FString MC_Open1 = "{ \"index\": 1, \"state\": \"open\" }";
const FString MC_Open2 = "{ \"index\": 2, \"state\": \"open\" }";
const FString MC_Open3 = "{ \"index\": 3, \"state\": \"open\" }";
const FString MC_Closed3 = "{ \"index\": 3, \"state\": \"closed\" }";
const FString MC_AI2 = "{ \"index\": 2, \"state\": \"ai\", \"level\": \"Normal\", \"colorIndex\": 2, \"team\": 0, \"faction\": \"DawnOrder\" }";
const FString MC_AI3 = "{ \"index\": 3, \"state\": \"ai\", \"level\": \"Normal\", \"colorIndex\": 4, \"team\": 0, \"faction\": \"DawnOrder\" }";
const FString MC_Tail4 = "{ \"index\": 4, \"state\": \"open\" }, { \"index\": 5, \"state\": \"open\" }, { \"index\": 6, \"state\": \"open\" }, { \"index\": 7, \"state\": \"open\" }";
const FString MC_Tail4Closed = "{ \"index\": 4, \"state\": \"closed\" }, { \"index\": 5, \"state\": \"open\" }, { \"index\": 6, \"state\": \"open\" }, { \"index\": 7, \"state\": \"open\" }";

// The example of PROTOCOL.md (Lobby test): Bob ready, Alice (you), 2 open, 3 closed, 4 to 7 open.
FString MC_ExampleState()
{
	return MC_State("\"Alice\"", MC_Bob + ", " + MC_Alice + ", " + MC_Open2 + ", " + MC_Closed3 + ", " + MC_Tail4);
}

void Test_ManagerClientParse(FUnitTest& T)
{
	FString Status = "{ \"protocolVersion\": 1, \"gameVersion\": \"2026-10-23-b4a8c8c\", \"lobbies\": 2, \"servers\": 1, \"maxServers\": 2 }";
	T.AssertTrue(HOLManagerClientRead::IsStatus(Status), "status read");
	MC_Eq(T, "1", HOLManagerClientRead::StatusField(Status, "ProtocolVersion"), "protocol");
	MC_Eq(T, "2026-10-23-b4a8c8c", HOLManagerClientRead::StatusField(Status, "GameVersion"), "game version");
	MC_Eq(T, "2", HOLManagerClientRead::StatusField(Status, "Lobbies"), "lobbies");
	MC_Eq(T, "1", HOLManagerClientRead::StatusField(Status, "Servers"), "servers");
	MC_Eq(T, "2", HOLManagerClientRead::StatusField(Status, "MaxServers"), "max servers");
	T.AssertFalse(HOLManagerClientRead::IsStatus("{ \"lobbies\": 2 }"), "status without a protocol");
	T.AssertFalse(HOLManagerClientRead::IsStatus("<html>"), "status not JSON");

	FString Lobbies = "{ \"lobbies\": [ { \"id\": \"L7\", \"name\": \"Salon de Bob\", \"players\": 3, \"maxPlayers\": 8, \"state\": \"lobby\", \"minutes\": null, \"canJoin\": true, \"canWatch\": false },"
		+ " { \"id\": \"L8\", \"name\": \"2v2\", \"players\": 4, \"maxPlayers\": 4, \"state\": \"in_game\", \"minutes\": 12, \"canJoin\": false, \"canWatch\": true, \"away\": [\"Bob\"] } ] }";
	T.AssertEquals(2, HOLManagerClientRead::GetNumLobbies(Lobbies), "two lobbies");
	MC_Eq(T, "L7", HOLManagerClientRead::LobbiesField(Lobbies, 0, "Id"), "id");
	MC_Eq(T, "Salon de Bob", HOLManagerClientRead::LobbiesField(Lobbies, 0, "Name"), "name");
	MC_Eq(T, "3", HOLManagerClientRead::LobbiesField(Lobbies, 0, "Players"), "players");
	MC_Eq(T, "8", HOLManagerClientRead::LobbiesField(Lobbies, 0, "MaxPlayers"), "max");
	MC_Eq(T, "lobby", HOLManagerClientRead::LobbiesField(Lobbies, 0, "State"), "state lobby");
	MC_Eq(T, "-1", HOLManagerClientRead::LobbiesField(Lobbies, 0, "Minutes"), "minutes null");
	MC_Eq(T, "true", HOLManagerClientRead::LobbiesField(Lobbies, 0, "CanJoin"), "can join");
	MC_Eq(T, "false", HOLManagerClientRead::LobbiesField(Lobbies, 0, "CanWatch"), "cannot watch");
	MC_Eq(T, "in_game", HOLManagerClientRead::LobbiesField(Lobbies, 1, "State"), "state in_game");
	MC_Eq(T, "12", HOLManagerClientRead::LobbiesField(Lobbies, 1, "Minutes"), "minutes");
	MC_Eq(T, "false", HOLManagerClientRead::LobbiesField(Lobbies, 1, "CanJoin"), "cannot join");
	MC_Eq(T, "true", HOLManagerClientRead::LobbiesField(Lobbies, 1, "CanWatch"), "can watch");
	MC_Eq(T, "", HOLManagerClientRead::LobbiesField(Lobbies, 0, "Away"), "no away field: nobody away");
	MC_Eq(T, "Bob", HOLManagerClientRead::LobbiesField(Lobbies, 1, "Away"), "away");

	FString Starting = "{ \"lobbies\": [ { \"id\": \"L9\", \"name\": \"x\", \"players\": 2, \"maxPlayers\": 8, \"state\": \"starting\", \"minutes\": null, \"canJoin\": false, \"canWatch\": false } ] }";
	T.AssertEquals(1, HOLManagerClientRead::GetNumLobbies(Starting), "starting read");
	MC_Eq(T, "starting", HOLManagerClientRead::LobbiesField(Starting, 0, "State"), "state starting");
	T.AssertEquals(0, HOLManagerClientRead::GetNumLobbies("{ \"lobbies\": [] }"), "no lobby");
	T.AssertEquals(-1, HOLManagerClientRead::GetNumLobbies("{ \"lobbies\": [ { \"id\": \"L1\", \"name\": \"x\", \"players\": 1, \"maxPlayers\": 8, \"state\": \"lost\", \"minutes\": null, \"canJoin\": true, \"canWatch\": false } ] }"), "unknown state");
	T.AssertEquals(-1, HOLManagerClientRead::GetNumLobbies("{}"), "lobbies without the list");
	FString Odd = "{ \"lobbies\": [ { \"id\": \"L1\", \"name\": \"x\", \"players\": 2, \"maxPlayers\": 8, \"state\": \"in_game\", \"minutes\": 3, \"canJoin\": false, \"canWatch\": true, \"away\": \"Bob\" } ] }";
	T.AssertEquals(1, HOLManagerClientRead::GetNumLobbies(Odd), "away not a list: still read");
	MC_Eq(T, "", HOLManagerClientRead::LobbiesField(Odd, 0, "Away"), "away not a list: nobody away");

	FString Error = "{ \"error\": \"lobby_full\", \"message\": \"The lobby is full (8 players).\" }";
	T.AssertTrue(HOLManagerClientRead::IsProtocolError(Error), "error read");
	MC_Eq(T, "lobby_full", HOLManagerClientRead::ErrorField(Error, "Code"), "error code");
	MC_Eq(T, "The lobby is full (8 players).", HOLManagerClientRead::ErrorField(Error, "Message"), "error message");
	T.AssertFalse(HOLManagerClientRead::IsProtocolError("{ \"message\": \"x\" }"), "error without a code");
}

// A row of the list as the manager writes it; Away is the JSON of the list ("" = no field).
FString MC_Row(const FString& State, bool CanJoin, bool CanWatch, const FString& Away)
{
	FString Result = "{ \"lobbies\": [ { \"id\": \"L8\", \"name\": \"x\", \"players\": 2, \"maxPlayers\": 8, \"state\": \"" + State + "\", \"minutes\": null, \"canJoin\": " + (CanJoin ? "true" : "false")
		+ ", \"canWatch\": " + (CanWatch ? "true" : "false");
	if (Away != "")
	{
		Result += ", \"away\": " + Away;
	}
	return Result + " } ] }";
}

void Test_ManagerClientLobbyAction(FUnitTest& T)
{
	const int None = 0;
	const int Join = 1;
	const int Watch = 2;
	const int Rejoin = 3;
	FString InGame = MC_Row("in_game", false, true, "[\"Alice\", \"Bob\"]");
	T.AssertEquals(Join, HOLManagerClientRead::ChooseLobbyAction(MC_Row("lobby", true, false, ""), 0, "Bob"), "gathering, room: join");
	T.AssertEquals(None, HOLManagerClientRead::ChooseLobbyAction(MC_Row("lobby", false, false, ""), 0, "Bob"), "gathering, full: nothing");
	T.AssertEquals(None, HOLManagerClientRead::ChooseLobbyAction(MC_Row("starting", false, false, "[\"Bob\"]"), 0, "Bob"), "starting: nothing");
	T.AssertEquals(Rejoin, HOLManagerClientRead::ChooseLobbyAction(InGame, 0, "Bob"), "in game, my name away: rejoin");
	T.AssertEquals(Rejoin, HOLManagerClientRead::ChooseLobbyAction(InGame, 0, "bOB"), "case ignored");
	T.AssertEquals(Rejoin, HOLManagerClientRead::ChooseLobbyAction(InGame, 0, " Bob "), "spaces around ignored");
	T.AssertEquals(Watch, HOLManagerClientRead::ChooseLobbyAction(InGame, 0, "Chloé"), "in game, my name not away: watch");
	T.AssertEquals(Watch, HOLManagerClientRead::ChooseLobbyAction(InGame, 0, "Bo"), "in game, part of a name: watch");
	T.AssertEquals(Watch, HOLManagerClientRead::ChooseLobbyAction(MC_Row("in_game", false, true, ""), 0, "Bob"), "in game, nobody away: watch");
	T.AssertEquals(Watch, HOLManagerClientRead::ChooseLobbyAction(InGame, 0, ""), "in game, no name: watch");
	T.AssertEquals(None, HOLManagerClientRead::ChooseLobbyAction(MC_Row("in_game", false, false, "[\"Bob\"]"), 0, "Bob"), "in game, cannot watch: nothing");
}

void Test_ManagerClientFailure(FUnitTest& T)
{
	MC_Eq(T, "None", HOLManagerClientRead::FailureField(true, 200, "{}", "Kind"), "200: no failure");
	MC_Eq(T, "Unreachable", HOLManagerClientRead::FailureField(false, 0, "", "Kind"), "no answer: unreachable");
	FString Full = "{ \"error\": \"lobby_full\", \"message\": \"The lobby is full (8 players).\" }";
	MC_Eq(T, "Protocol", HOLManagerClientRead::FailureField(true, 409, Full, "Kind"), "409 with the protocol's body");
	MC_Eq(T, "lobby_full", HOLManagerClientRead::FailureField(true, 409, Full, "Code"), "its code kept");
	MC_Eq(T, "409", HOLManagerClientRead::FailureField(true, 409, Full, "HttpCode"), "its HTTP code");
	MC_Eq(T, "Unreadable", HOLManagerClientRead::FailureField(true, 502, "<html>Bad gateway</html>", "Kind"), "502 of a proxy: unreadable");

	FString Servers = "{ \"servers\": [ { \"name\": \"PC de Baudouin\", \"url\": \"http://100.64.0.1:7700\" }, { \"name\": \"no url\" } ] }";
	T.AssertEquals(1, HOLManagerClientRead::GetNumServers(Servers), "one server with a url");
	MC_Eq(T, "PC de Baudouin", HOLManagerClientRead::ServersField(Servers, 0, "Name"), "server name");
	MC_Eq(T, "http://100.64.0.1:7700", HOLManagerClientRead::ServersField(Servers, 0, "Url"), "server url");
	T.AssertEquals(-1, HOLManagerClientRead::GetNumServers("servers"), "servers.json not JSON");
	FString NoKey = "{ \"servers\": [ { \"name\": \"a\", \"url\": \"http://a:7700\" } ] }";
	T.AssertTrue(HOLManagerClientRead::GetNumServers(NoKey) == 1 && HOLManagerClientRead::ServersField(NoKey, 0, "Key") == "", "servers.json without a key");
	FString Keyed = "{ \"servers\": [ { \"name\": \"Ami\", \"url\": \"http://90.55.168.58:7700\", \"key\": \" Ab-_09xyz \" } ] }";
	T.AssertEquals(1, HOLManagerClientRead::GetNumServers(Keyed), "one keyed server");
	MC_Eq(T, "Ab-_09xyz", HOLManagerClientRead::ServersField(Keyed, 0, "Key"), "key read, trimmed");

	MC_Eq(T, "http://127.0.0.1:7700/status", HOLManagerClientRead::MakeUrl("http://127.0.0.1:7700", "/status"), "url without a slash");
	MC_Eq(T, "http://127.0.0.1:7700/lobbies", HOLManagerClientRead::MakeUrl("http://127.0.0.1:7700/", "/lobbies"), "url with a slash");
}

void Test_ManagerClientLobby(FUnitTest& T)
{
	FString State = MC_ExampleState();
	T.AssertTrue(HOLManagerClientRead::IsLobby(State), "lobby read");
	MC_Eq(T, "L7", HOLManagerClientRead::LobbyField(State, "Id"), "id");
	MC_Eq(T, "Salon de Bob", HOLManagerClientRead::LobbyField(State, "Name"), "name");
	MC_Eq(T, "lobby", HOLManagerClientRead::LobbyField(State, "State"), "state lobby");
	MC_Eq(T, "-1", HOLManagerClientRead::LobbyField(State, "Minutes"), "minutes null");
	MC_Eq(T, "Bob", HOLManagerClientRead::LobbyField(State, "Leader"), "leader");
	MC_Eq(T, "Alice", HOLManagerClientRead::LobbyField(State, "You"), "you");
	MC_Eq(T, "Plain", HOLManagerClientRead::LobbyField(State, "Map"), "map");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "Nomad"), "nomad");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(State, "Regicide"), "regicide");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "Catastrophe"), "catastrophe");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "SacredSites"), "no sacred sites when the manager does not say");
	FString Sites = State.Replace("\"catastrophe\": false }", "\"catastrophe\": false, \"sacredSites\": true }");
	T.AssertTrue(HOLManagerClientRead::IsLobby(Sites), "lobby with sacred sites read");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(Sites, "SacredSites"), "lobby sacredSites read");
	MC_Eq(T, "Plain", HOLManagerClientRead::LobbyField(State, "Maps"), "maps");
	FString TwoMaps = State.Replace("\"maps\": [ { \"name\": \"Plain\", \"maxPlayers\": 8 } ]", "\"maps\": [ { \"name\": \"Plain\", \"maxPlayers\": 8 }, { \"name\": \"Desert\", \"maxPlayers\": 8 } ]");
	T.AssertTrue(HOLManagerClientRead::IsLobby(TwoMaps), "two maps read");
	MC_Eq(T, "Plain,Desert", HOLManagerClientRead::LobbyField(TwoMaps, "Maps"), "two maps in order");
	MC_Eq(T, "8", HOLManagerClientRead::LobbyField(State, "NumSlots"), "8 slots");
	MC_Eq(T, "player", HOLManagerClientRead::LobbySlotField(State, 1, "State"), "slot 1 a player");
	MC_Eq(T, "1", HOLManagerClientRead::LobbySlotField(State, 1, "Index"), "slot 1 index");
	MC_Eq(T, "Alice", HOLManagerClientRead::LobbySlotField(State, 1, "PlayerName"), "slot 1 name");
	MC_Eq(T, "3", HOLManagerClientRead::LobbySlotField(State, 1, "ColorIndex"), "slot 1 colour");
	MC_Eq(T, "2", HOLManagerClientRead::LobbySlotField(State, 1, "Team"), "slot 1 team");
	MC_Eq(T, "DawnOrder", HOLManagerClientRead::LobbySlotField(State, 1, "Faction"), "slot 1 faction");
	MC_Eq(T, "false", HOLManagerClientRead::LobbySlotField(State, 1, "Ready"), "slot 1 not ready");
	MC_Eq(T, "true", HOLManagerClientRead::LobbySlotField(State, 0, "Ready"), "slot 0 ready");
	MC_Eq(T, "open", HOLManagerClientRead::LobbySlotField(State, 2, "State"), "slot 2 open");
	MC_Eq(T, "closed", HOLManagerClientRead::LobbySlotField(State, 3, "State"), "slot 3 closed");
	MC_Eq(T, "2", HOLManagerClientRead::LobbyField(State, "NumPlayers"), "2 players");
	MC_Eq(T, "1", HOLManagerClientRead::LobbyField(State, "FindYourSlot"), "your slot");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "HasMatch"), "no match");
	MC_Eq(T, "", HOLManagerClientRead::LobbyField(State, "Notice"), "no notice");

	FString Match = "{ \"matchId\": \"M20261023-153012-3\", \"port\": 7778, \"playerName\": \"Alice\", \"gameVersion\": \"2026-10-23-b4a8c8c\" }";
	FString InGame = State.Replace("\"state\": \"lobby\"", "\"state\": \"in_game\"").Replace("\"minutes\": null", "\"minutes\": 12")
		.Replace("\"match\": null", "\"match\": " + Match).Replace("\"notice\": null", "\"notice\": \"leader_changed\"");
	T.AssertTrue(HOLManagerClientRead::IsLobby(InGame), "in game read");
	MC_Eq(T, "in_game", HOLManagerClientRead::LobbyField(InGame, "State"), "state in_game");
	MC_Eq(T, "12", HOLManagerClientRead::LobbyField(InGame, "Minutes"), "minutes");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(InGame, "HasMatch"), "match");
	MC_Eq(T, "M20261023-153012-3", HOLManagerClientRead::LobbyField(InGame, "MatchId"), "match id");
	MC_Eq(T, "7778", HOLManagerClientRead::LobbyField(InGame, "MatchPort"), "match port");
	MC_Eq(T, "Alice", HOLManagerClientRead::LobbyField(InGame, "MatchPlayerName"), "match name");
	MC_Eq(T, "2026-10-23-b4a8c8c", HOLManagerClientRead::LobbyField(InGame, "MatchGameVersion"), "match version");
	MC_Eq(T, "leader_changed", HOLManagerClientRead::LobbyField(InGame, "Notice"), "notice");

	FString Anonymous = State.Replace("\"you\": \"Alice\"", "\"you\": null");
	T.AssertTrue(HOLManagerClientRead::IsLobby(Anonymous), "read without a token");
	MC_Eq(T, "", HOLManagerClientRead::LobbyField(Anonymous, "You"), "you empty");
	MC_Eq(T, "-1", HOLManagerClientRead::LobbyField(Anonymous, "FindYourSlot"), "no slot of yours");

	T.AssertFalse(HOLManagerClientRead::IsLobby(State.Replace("{ \"index\": 6, \"state\": \"open\" }, ", "")), "a slot missing");
	T.AssertFalse(HOLManagerClientRead::IsLobby(State.Replace("\"state\": \"closed\"", "\"state\": \"gone\"")), "unknown slot state");
	T.AssertFalse(HOLManagerClientRead::IsLobby("<html>"), "not JSON");

	FString Join = "{ \"token\": \"9f2c4e0b7a3d4c1e8b6f5a2d0c9e7b14\", \"slot\": 1, \"lobby\": " + State + " }";
	T.AssertTrue(HOLManagerClientRead::IsJoin(Join), "join read");
	MC_Eq(T, "9f2c4e0b7a3d4c1e8b6f5a2d0c9e7b14", HOLManagerClientRead::JoinField(Join, "Token"), "token");
	MC_Eq(T, "1", HOLManagerClientRead::JoinField(Join, "Slot"), "slot");
	MC_Eq(T, "L7", HOLManagerClientRead::JoinField(Join, "LobbyId"), "joined lobby");
	T.AssertFalse(HOLManagerClientRead::IsJoin("{ \"slot\": 1, \"lobby\": " + State + " }"), "join without a token");
}

void Test_ManagerClientWrite(FUnitTest& T)
{
	FString Me = HOLManagerClientRead::MakeMeBody(true, 4, true, 2, true, true, false, false, "");
	MC_Eq(T, "4", HOLManagerClientRead::JsonField(Me, "colorIndex"), "me colour");
	MC_Eq(T, "2", HOLManagerClientRead::JsonField(Me, "team"), "me team");
	MC_Eq(T, "true", HOLManagerClientRead::JsonField(Me, "ready"), "me ready");
	FString ReadyOnly = HOLManagerClientRead::MakeMeBody(false, 0, false, 0, true, false, false, false, "");
	T.AssertEquals(1, HOLManagerClientRead::JsonNumFields(ReadyOnly), "ready alone");
	MC_Eq(T, "false", HOLManagerClientRead::JsonField(ReadyOnly, "ready"), "not ready");
	FString ColorOnly = HOLManagerClientRead::MakeMeBody(true, 0, false, 0, false, false, false, false, "");
	T.AssertTrue(HOLManagerClientRead::JsonNumFields(ColorOnly) == 1 && HOLManagerClientRead::JsonHasField(ColorOnly, "colorIndex"), "colour 0 alone");

	FString Settings = HOLManagerClientRead::MakeSettingsBody("Plain", true, false, true, "", false, false, "", "", "", false, false);
	MC_Eq(T, "Plain", HOLManagerClientRead::JsonField(Settings, "map"), "settings map");
	MC_Eq(T, "true", HOLManagerClientRead::JsonField(Settings, "nomad"), "settings nomad");
	MC_Eq(T, "false", HOLManagerClientRead::JsonField(Settings, "regicide"), "settings regicide");
	MC_Eq(T, "true", HOLManagerClientRead::JsonField(Settings, "catastrophe"), "settings catastrophe");
	T.AssertFalse(HOLManagerClientRead::JsonHasField(Settings, "sacredSites"), "no sacredSites when not given");
	FString Sites = HOLManagerClientRead::MakeSettingsBody("Plain", false, false, false, "", false, false, "", "", "", true, true);
	T.AssertTrue(HOLManagerClientRead::JsonHasField(Sites, "sacredSites") && HOLManagerClientRead::JsonField(Sites, "sacredSites") == "true", "settings sacredSites sent");
	MC_Eq(T, "closed", HOLManagerClientRead::JsonField(HOLManagerClientRead::MakeSlotBody(false), "state"), "slot closed");
	MC_Eq(T, "open", HOLManagerClientRead::JsonField(HOLManagerClientRead::MakeSlotBody(true), "state"), "slot open");

	// Bob leads, Alice is you: slot 0 Bob ready, slot 1 Alice not ready, 2 open, 3 closed (the C++ lobby, as its JSON).
	FString AsAlice = MC_ExampleState().Replace("\"regicide\": true", "\"regicide\": false");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AsAlice, "IsYouLeader"), "Alice does not lead");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(AsAlice, "CanEditSlot", 1), "Alice edits her line");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AsAlice, "CanEditSlot", 0), "Alice does not edit Bob's");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AsAlice, "CanEditSlot", 2), "nor an open slot");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AsAlice, "CanToggleSlot", 2), "Alice does not close a slot");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AsAlice, "CanStart"), "Alice does not start");

	FString Led = MC_State("\"Bob\"", MC_Bob + ", " + MC_Alice + ", " + MC_Open2 + ", " + MC_Closed3 + ", " + MC_Tail4);
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(Led, "IsYouLeader"), "Bob leads");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(Led, "CanEditSlot", 0), "Bob edits his line");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Led, "CanEditSlot", 1), "Bob does not edit Alice's");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(Led, "CanToggleSlot", 2), "Bob closes an open slot");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(Led, "CanToggleSlot", 3), "Bob opens a closed slot");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Led, "CanToggleSlot", 1), "not a held slot");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Led, "CanToggleSlot", 8), "not out of range");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Led, "CanStart"), "Alice not ready: no start");
	FString AllReady = MC_State("\"Bob\"", MC_Bob + ", " + MC_Alice_Ready + ", " + MC_Open2 + ", " + MC_Closed3 + ", " + MC_Tail4);
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(AllReady, "CanStart"), "all ready: start");
	FString Alone = MC_State("\"Bob\"", MC_Bob + ", " + MC_Open1 + ", " + MC_Open2 + ", " + MC_Closed3 + ", " + MC_Tail4);
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Alone, "CanStart"), "alone: no start");

	FString Anonymous = MC_State("null", MC_Bob + ", " + MC_Alice + ", " + MC_Open2 + ", " + MC_Closed3 + ", " + MC_Tail4);
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Anonymous, "IsYouLeader"), "no token: no leader");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Anonymous, "CanEditSlot", 1), "no token: no line");
}

void Test_ManagerClientAI(FUnitTest& T)
{
	FString State = MC_State("\"Bob\"", MC_Bob + ", " + MC_Alice + ", " + MC_AI2 + ", " + MC_Open3 + ", " + MC_Tail4Closed);
	T.AssertTrue(HOLManagerClientRead::IsLobby(State), "lobby with an AI read");
	MC_Eq(T, "8", HOLManagerClientRead::LobbyField(State, "NumSlots"), "8 slots");
	MC_Eq(T, "ai", HOLManagerClientRead::LobbySlotField(State, 2, "State"), "slot 2 an AI");
	MC_Eq(T, "2", HOLManagerClientRead::LobbySlotField(State, 2, "Index"), "AI index");
	MC_Eq(T, "2", HOLManagerClientRead::LobbySlotField(State, 2, "ColorIndex"), "AI colour");
	MC_Eq(T, "0", HOLManagerClientRead::LobbySlotField(State, 2, "Team"), "AI team");
	MC_Eq(T, "Normal", HOLManagerClientRead::LobbySlotField(State, 2, "Level"), "AI level Normal");
	MC_Eq(T, "DawnOrder", HOLManagerClientRead::LobbySlotField(State, 2, "Faction"), "AI faction");
	MC_Eq(T, "", HOLManagerClientRead::LobbySlotField(State, 2, "PlayerName"), "AI without a player's name");
	MC_Eq(T, "false", HOLManagerClientRead::LobbySlotField(State, 2, "Ready"), "AI without Ready");
	MC_Eq(T, "open", HOLManagerClientRead::LobbySlotField(State, 3, "State"), "slot 3 open");
	MC_Eq(T, "closed", HOLManagerClientRead::LobbySlotField(State, 4, "State"), "slot 4 closed");
	MC_Eq(T, "2", HOLManagerClientRead::LobbyField(State, "NumPlayers"), "an AI is not a player: 2 players");
	MC_Eq(T, "1", HOLManagerClientRead::LobbyField(State, "NumAIs"), "1 AI");
	MC_Eq(T, "0", HOLManagerClientRead::LobbyField(State, "FindYourSlot"), "your slot (not the AI's)");

	FString NoLevel = State.Replace("\"level\": \"Normal\", ", "");
	T.AssertTrue(HOLManagerClientRead::IsLobby(NoLevel), "AI without a level read");
	MC_Eq(T, "Normal", HOLManagerClientRead::LobbySlotField(NoLevel, 2, "Level"), "AI without a level: Normal");
	FString Hard = State.Replace("\"level\": \"Normal\"", "\"level\": \"Hard\"");
	T.AssertTrue(HOLManagerClientRead::IsLobby(Hard), "AI Hard read");
	MC_Eq(T, "Hard", HOLManagerClientRead::LobbySlotField(Hard, 2, "Level"), "AI level Hard");
	T.AssertFalse(HOLManagerClientRead::IsLobby(State.Replace("\"colorIndex\": 2, ", "")), "AI without a colour");
	T.AssertFalse(HOLManagerClientRead::IsLobby(State.Replace("\"team\": 0, ", "")), "AI without a team");

	FString Put = HOLManagerClientRead::MakeAISlotBody(false, 0, false, 0, "");
	T.AssertTrue(HOLManagerClientRead::JsonNumFields(Put) == 1 && HOLManagerClientRead::JsonField(Put, "state") == "ai", "ai alone");
	FString Coloured = HOLManagerClientRead::MakeAISlotBody(true, 5, true, 2, "");
	T.AssertEquals(3, HOLManagerClientRead::JsonNumFields(Coloured), "ai with colour and team: 3 fields");
	MC_Eq(T, "ai", HOLManagerClientRead::JsonField(Coloured, "state"), "ai state");
	MC_Eq(T, "5", HOLManagerClientRead::JsonField(Coloured, "colorIndex"), "ai colour");
	MC_Eq(T, "2", HOLManagerClientRead::JsonField(Coloured, "team"), "ai team");
	FString ColourOnly = HOLManagerClientRead::MakeAISlotBody(true, 0, false, 0, "");
	T.AssertTrue(HOLManagerClientRead::JsonNumFields(ColourOnly) == 2 && HOLManagerClientRead::JsonHasField(ColourOnly, "colorIndex") && !HOLManagerClientRead::JsonHasField(ColourOnly, "team"), "ai colour 0 alone");
	FString TeamOnly = HOLManagerClientRead::MakeAISlotBody(false, 0, true, 3, "");
	T.AssertTrue(HOLManagerClientRead::JsonNumFields(TeamOnly) == 2 && HOLManagerClientRead::JsonHasField(TeamOnly, "team") && !HOLManagerClientRead::JsonHasField(TeamOnly, "colorIndex"), "ai team alone");

	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(State, "IsYouLeader"), "Bob leads");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(State, "CanToggleSlot", 2), "Bob opens or closes the AI's slot");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(State, "CanEditAISlot", 2), "Bob edits the AI");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "CanEditSlot", 2), "an AI is not my line");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "CanEditAISlot", 3), "not an open slot's AI");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "CanEditAISlot", 1), "not a player's line");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "CanEditAISlot", 8), "not out of range");
	FString AsAlice = State.Replace("\"you\": \"Bob\"", "\"you\": \"Alice\"");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AsAlice, "CanToggleSlot", 2), "Alice does not toggle the AI's slot");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AsAlice, "CanEditAISlot", 2), "Alice does not edit the AI");

	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "CanStart"), "Alice not ready, an AI: no start");
	FString AllReady = MC_State("\"Bob\"", MC_Bob + ", " + MC_Alice_Ready + ", " + MC_AI2 + ", " + MC_Open3 + ", " + MC_Tail4Closed);
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(AllReady, "CanStart"), "both players ready, an AI: start");
	FString LeaderAndAI = MC_State("\"Bob\"", MC_Bob + ", " + MC_Open1 + ", " + MC_AI2 + ", " + MC_Open3 + ", " + MC_Tail4Closed);
	MC_Eq(T, "1", HOLManagerClientRead::LobbyField(LeaderAndAI, "NumPlayers"), "the leader alone with an AI: 1 player");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(LeaderAndAI, "CanStart"), "the leader ready alone with an AI: start");
	FString LeaderNotReady = MC_State("\"Bob\"", MC_Bob_NotReady + ", " + MC_Open1 + ", " + MC_AI2 + ", " + MC_Open3 + ", " + MC_Tail4Closed);
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(LeaderNotReady, "CanStart"), "the leader not ready with an AI: no start");
	FString AloneNoAI = MC_State("\"Bob\"", MC_Bob + ", " + MC_Open1 + ", " + MC_Open2 + ", " + MC_Open3 + ", " + MC_Tail4Closed);
	MC_Eq(T, "0", HOLManagerClientRead::LobbyField(AloneNoAI, "NumAIs"), "no AI");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(AloneNoAI, "CanStart"), "the leader alone, no AI: no start");
	FString TwoAIs = MC_State("\"Bob\"", MC_Bob + ", " + MC_Open1 + ", " + MC_AI2 + ", " + MC_AI3 + ", " + MC_Tail4Closed);
	MC_Eq(T, "2", HOLManagerClientRead::LobbyField(TwoAIs, "NumAIs"), "2 AIs");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(TwoAIs, "CanStart"), "the leader ready with two AIs: start");
}

void Test_ManagerClientMatch(FUnitTest& T)
{
	FString Slots = "";
	for (int SlotIndex = 0; SlotIndex < 8; ++SlotIndex)
	{
		Slots += (SlotIndex > 0 ? ", " : "") + "{ \"index\": " + SlotIndex + ", \"state\": \"closed\" }";
	}
	FString Json = "{ \"id\": \"L7\", \"name\": \"Salon de Bob\", \"state\": \"in_game\", \"minutes\": 0, \"leader\": \"Bob\", \"you\": \"Alice\","
		+ " \"settings\": { \"map\": \"Plain\", \"nomad\": false, \"regicide\": false, \"catastrophe\": false }, \"slots\": [" + Slots + "],"
		+ " \"match\": { \"matchId\": \"M20261023-153012-3\", \"port\": 7778, \"playerName\": \"Alice\", \"gameVersion\": \"2026-10-23-b4a8c8c\" }, \"notice\": null }";
	T.AssertTrue(HOLManagerClientRead::IsLobby(Json), "lobby in game read");
	MC_Eq(T, "in_game", HOLManagerClientRead::LobbyField(Json, "State"), "in game");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(Json, "HasMatch"), "match read");
	MC_Eq(T, "M20261023-153012-3", HOLManagerClientRead::LobbyField(Json, "MatchId"), "match id");
	MC_Eq(T, "7778", HOLManagerClientRead::LobbyField(Json, "MatchPort"), "match port");
	MC_Eq(T, "Alice", HOLManagerClientRead::LobbyField(Json, "MatchPlayerName"), "match name");
	MC_Eq(T, "2026-10-23-b4a8c8c", HOLManagerClientRead::LobbyField(Json, "MatchGameVersion"), "match version");

	MC_Eq(T, "127.0.0.1:7778?Name=Alice?Version=2026-10-23-b4a8c8c", HOLManagerClientRead::MakeMatchAddress("http://127.0.0.1:7700", 7778, "Alice", "2026-10-23-b4a8c8c"), "player's address");
	MC_Eq(T, "hote.tail1234.ts.net:7779?Name=Bob?Version=v1", HOLManagerClientRead::MakeMatchAddress("http://hote.tail1234.ts.net:7700/", 7779, "Bob", "v1"), "manager's host, path dropped");
	MC_Eq(T, "192.168.1.5:7777?Name=Bob?Version=v1", HOLManagerClientRead::MakeMatchAddress("http://192.168.1.5", 7777, "Bob", "v1"), "host without a port");
	MC_Eq(T, "127.0.0.1:7778?Name=Le%20Roi%26Co?Version=v1", HOLManagerClientRead::MakeMatchAddress("http://127.0.0.1:7700", 7778, "Le Roi&Co", "v1"), "name encoded");
	MC_Eq(T, "127.0.0.1:7778?Version=v1", HOLManagerClientRead::MakeMatchAddress("http://127.0.0.1:7700", 7778, "", "v1"), "spectator: no name");
	MC_Eq(T, "", HOLManagerClientRead::MakeMatchAddress("http://127.0.0.1:7700", 0, "Bob", "v1"), "no port");
	MC_Eq(T, "", HOLManagerClientRead::MakeMatchAddress("http://", 7778, "Bob", "v1"), "no host");

	FString Watch = "{ \"matchId\": \"M20261023-153012-3\", \"port\": 7778, \"minutes\": 12 }";
	T.AssertTrue(HOLManagerClientRead::IsWatch(Watch), "watch read");
	MC_Eq(T, "M20261023-153012-3", HOLManagerClientRead::WatchField(Watch, "MatchId"), "watch match");
	MC_Eq(T, "7778", HOLManagerClientRead::WatchField(Watch, "Port"), "watch port");
	T.AssertFalse(HOLManagerClientRead::IsWatch("{ \"matchId\": \"M1\" }"), "watch without a port");
	T.AssertFalse(HOLManagerClientRead::IsWatch("<html>"), "watch not JSON");
}

void MC_Origin(FUnitTest& T, bool HasToken, bool Rejoined, bool Named, bool InMatch, int Expected, const FString& What)
{
	T.AssertEquals(Expected, HOLManagerClientRead::GetMatchOrigin(HasToken, Rejoined, Named, InMatch), What);
}

void Test_ManagerClientOrigin(FUnitTest& T)
{
	const int Local = 0;
	const int Manager = 1;
	const int Lobby = 2;
	MC_Origin(T, true, false, true, true, Lobby, "lobby's player: token and name");
	MC_Origin(T, false, true, true, true, Lobby, "back by Revenir: no token, name");
	MC_Origin(T, true, true, true, true, Lobby, "token and Revenir both");
	MC_Origin(T, true, false, true, false, Lobby, "named, not yet travelled");
	MC_Origin(T, true, false, false, true, Manager, "spectator with a token: no name");
	MC_Origin(T, false, true, false, true, Manager, "Revenir without a name");
	MC_Origin(T, false, false, false, true, Manager, "Regarder: no token, no name");
	MC_Origin(T, false, false, true, true, Manager, "named without lobby (no token, no Revenir)");
	MC_Origin(T, false, false, false, false, Local, "nothing: local");
	MC_Origin(T, false, true, false, false, Local, "Revenir unnamed, out of match");
}

void Test_ManagerClientSpectator(FUnitTest& T)
{
	FString Rows = "{ \"lobbies\": [ { \"id\": \"L1\", \"name\": \"full\", \"players\": 8, \"maxPlayers\": 8, \"state\": \"lobby\", \"minutes\": null, \"canJoin\": false, \"canWatch\": false, \"spectators\": 2, \"canSpectate\": true },"
		+ " { \"id\": \"L2\", \"name\": \"old manager\", \"players\": 1, \"maxPlayers\": 8, \"state\": \"lobby\", \"minutes\": null, \"canJoin\": true, \"canWatch\": false },"
		+ " { \"id\": \"L3\", \"name\": \"playing\", \"players\": 2, \"maxPlayers\": 2, \"state\": \"in_game\", \"minutes\": 4, \"canJoin\": false, \"canWatch\": true, \"spectators\": 0, \"canSpectate\": false } ] }";
	T.AssertEquals(3, HOLManagerClientRead::GetNumLobbies(Rows), "three lobbies");
	MC_Eq(T, "2", HOLManagerClientRead::LobbiesField(Rows, 0, "Spectators"), "two spectators");
	MC_Eq(T, "true", HOLManagerClientRead::LobbiesField(Rows, 0, "CanSpectate"), "full lobby: can spectate");
	T.AssertEquals(0, HOLManagerClientRead::ChooseLobbyAction(Rows, 0, "Bob"), "full lobby: nothing to join");
	MC_Eq(T, "false", HOLManagerClientRead::LobbiesField(Rows, 1, "CanSpectate"), "older manager: cannot spectate");
	MC_Eq(T, "false", HOLManagerClientRead::LobbiesField(Rows, 2, "CanSpectate"), "in game: Regarder, not Observer");

	FString State = "{ \"id\": \"L7\", \"name\": \"Salon de Bob\", \"state\": \"lobby\", \"minutes\": null, \"leader\": \"Bob\", \"you\": \"Carol\","
		+ " \"settings\": { \"map\": \"Plain\", \"nomad\": false, \"regicide\": false, \"catastrophe\": false }, \"maps\": [ { \"name\": \"Plain\", \"maxPlayers\": 8 } ],"
		+ " \"slots\": [ " + MC_Bob + ", " + MC_Alice_Ready + ", " + MC_Open2 + ", " + MC_Open3 + ", " + MC_Tail4 + " ],"
		+ "  \"spectators\": [ { \"playerName\": \"Bob\" }, { \"playerName\": \"Carol\" } ],"
		+ " \"match\": null, \"notice\": null }";
	T.AssertTrue(HOLManagerClientRead::IsLobby(State), "lobby read");
	MC_Eq(T, "Bob,Carol", HOLManagerClientRead::LobbyField(State, "Spectators"), "spectators by name");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(State, "IsYouSpectator"), "I am a spectator");
	MC_Eq(T, "-1", HOLManagerClientRead::LobbyField(State, "FindYourSlot"), "and have no slot");
	T.AssertTrue(HOLManagerClientRead::LobbyField(State, "CanEditSlot", 0) == "false" && HOLManagerClientRead::LobbyField(State, "CanEditSlot", 1) == "false"
		&& HOLManagerClientRead::LobbyField(State, "CanEditSlot", 2) == "false", "a spectator edits no line");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(State, "HasOpenSlot"), "a slot is open to take");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(State, "IsYouLeader"), "not the leader");
	FString AsLeader = State.Replace("\"you\": \"Carol\"", "\"you\": \"Bob\"");
	T.AssertTrue(HOLManagerClientRead::IsLobby(AsLeader), "leader spectator read");
	T.AssertTrue(HOLManagerClientRead::LobbyField(AsLeader, "IsYouLeader") == "true" && HOLManagerClientRead::LobbyField(AsLeader, "IsYouSpectator") == "true", "leader and spectator");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(AsLeader, "CanStart"), "he can start: the spectators neither count nor block");
	FString Old = State.Replace("  \"spectators\": [ { \"playerName\": \"Bob\" }, { \"playerName\": \"Carol\" } ],", "");
	T.AssertTrue(HOLManagerClientRead::IsLobby(Old), "older state read");
	T.AssertTrue(HOLManagerClientRead::LobbyField(Old, "Spectators") == "" && HOLManagerClientRead::LobbyField(Old, "IsYouSpectator") == "false", "no spectator");
	FString Full = MC_State("\"Carol\"", MC_Bob + ", " + MC_Alice_Ready + ", { \"index\": 2, \"state\": \"closed\" }, " + MC_Closed3
		+ ", { \"index\": 4, \"state\": \"closed\" }, { \"index\": 5, \"state\": \"closed\" }, { \"index\": 6, \"state\": \"closed\" }, { \"index\": 7, \"state\": \"closed\" }");
	T.AssertTrue(HOLManagerClientRead::IsLobby(Full), "full lobby read");
	// The C++ test closed every slot, the players' included; here the two players stay and the six others are closed: no slot is open.
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Full, "HasOpenSlot"), "no open slot: Jouer greyed");

	FString Join = HOLManagerClientRead::MakeJoinBody("Carol", "v1", true);
	T.AssertTrue(HOLManagerClientRead::JsonField(Join, "spectator") == "true" && HOLManagerClientRead::JsonField(Join, "playerName") == "Carol"
		&& HOLManagerClientRead::JsonField(Join, "gameVersion") == "v1", "join as spectator");
	FString JoinPlayer = HOLManagerClientRead::MakeJoinBody("Carol", "v1", false);
	T.AssertTrue(HOLManagerClientRead::JsonNumFields(JoinPlayer) == 2 && !HOLManagerClientRead::JsonHasField(JoinPlayer, "spectator"), "join as a player: no spectator field (the older body)");
	FString ToSpectator = HOLManagerClientRead::MakeMeBody(false, 0, false, 0, false, false, true, true, "");
	T.AssertTrue(HOLManagerClientRead::JsonNumFields(ToSpectator) == 1 && HOLManagerClientRead::JsonField(ToSpectator, "spectator") == "true", "me: spectator alone");
	FString ToPlayer = HOLManagerClientRead::MakeMeBody(false, 0, false, 0, false, false, true, false, "");
	T.AssertTrue(HOLManagerClientRead::JsonNumFields(ToPlayer) == 1 && HOLManagerClientRead::JsonField(ToPlayer, "spectator") == "false", "me: back to a player");
}

void Test_ManagerClientReconnectLine(FUnitTest& T)
{
	MC_Eq(T, "Connexion perdue — nouvelle tentative, 2:41", HOLManagerClientRead::FormatReconnectLine(160.4), "countdown 2:41");
	double Left = HOLManagerClientRead::GetReconnectSecondsLeft(100.0, 139.2, 180.0);
	T.AssertTrue(Left > 140.79 && Left < 140.81, "countdown is the window left");
}

// HOLMapSizeLobbyTests.cpp, Lobby.Read and Lobby.Send: a state with a size, and the settings body.
FString MC_SizedState(const FString& Settings)
{
	return "{ \"id\": \"L7\", \"name\": \"Salon\", \"state\": \"lobby\", \"minutes\": null, \"leader\": \"Bob\", \"you\": \"Bob\","
		+ " \"settings\": { \"map\": \"Plain\", \"nomad\": false, \"regicide\": false, \"catastrophe\": false" + Settings + " },"
		+ " \"maps\": [ { \"name\": \"Plain\", \"maxPlayers\": 8, \"sizes\": [ { \"key\": \"Small\", \"places\": 2 } ] } ],"
		+ " \"slots\": [ { \"index\": 0, \"state\": \"player\", \"playerName\": \"Bob\", \"colorIndex\": 0, \"team\": 0, \"faction\": \"DawnOrder\", \"ready\": false },"
		+ " { \"index\": 1, \"state\": \"open\" }, { \"index\": 2, \"state\": \"closed\" }, { \"index\": 3, \"state\": \"closed\" }, { \"index\": 4, \"state\": \"closed\" },"
		+ " { \"index\": 5, \"state\": \"closed\" }, { \"index\": 6, \"state\": \"closed\" }, { \"index\": 7, \"state\": \"closed\" } ], \"match\": null, \"notice\": null }";
}

void Test_ManagerClientMapSizeLobbyRead(FUnitTest& T)
{
	const int Small = 1;
	const int Huge = 5;
	FString Sized = MC_SizedState(", \"size\": \"Small\", \"teamsTogether\": false");
	T.AssertTrue(HOLManagerClientRead::IsLobby(Sized), "state with a size read");
	MC_Eq(T, "Small", HOLManagerClientRead::LobbyField(Sized, "Size"), "size key");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Sized, "TeamsTogether"), "teams not together");
	MC_Eq(T, "" + Small, HOLManagerClientRead::LobbyField(Sized, "MapSize"), "size Small");
	MC_Eq(T, "2", HOLManagerClientRead::LobbyField(Sized, "NumPlaces"), "2 places");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(Sized, "CanToggleSlot", 1), "Small: slot 1 (open, within the places) can be closed");
	MC_Eq(T, "false", HOLManagerClientRead::LobbyField(Sized, "CanToggleSlot", 2), "Small: slot 2 (past the places) cannot be opened");

	FString NoSize = MC_SizedState("");
	T.AssertTrue(HOLManagerClientRead::IsLobby(NoSize), "state without a size read");
	T.AssertTrue(HOLManagerClientRead::LobbyField(NoSize, "Size") == "" && HOLManagerClientRead::LobbyField(NoSize, "MapSize") == "0", "no size");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(NoSize, "TeamsTogether"), "teams together by default");
	MC_Eq(T, "8", HOLManagerClientRead::LobbyField(NoSize, "NumPlaces"), "8 places without a size");
	MC_Eq(T, "true", HOLManagerClientRead::LobbyField(NoSize, "CanToggleSlot", 2), "no size: slot 2 can be opened");
	MC_Eq(T, "" + Huge, HOLManagerClientRead::LobbyField(MC_SizedState(", \"size\": \"huge\""), "MapSize"), "size key without case");
	MC_Eq(T, "Diplomacy", HOLManagerClientRead::LobbyField(MC_SizedState(", \"mode\": \"Diplomacy\""), "MatchMode"), "mode Diplomacy read");
	MC_Eq(T, "Standard", HOLManagerClientRead::LobbyField(NoSize, "MatchMode"), "no mode: Standard");
	MC_Eq(T, "Standard", HOLManagerClientRead::LobbyField(MC_SizedState(", \"mode\": \"Battlefield\""), "MatchMode"), "unknown mode: Standard");

	FString List = "{ \"lobbies\": [ { \"id\": \"L7\", \"name\": \"Salon\", \"map\": \"Plain\", \"size\": \"Medium\", \"players\": 1, \"maxPlayers\": 4, \"state\": \"lobby\", \"minutes\": null, \"canJoin\": true, \"canWatch\": false },"
		+ " { \"id\": \"L8\", \"name\": \"Vieux\", \"players\": 1, \"maxPlayers\": 8, \"state\": \"lobby\", \"minutes\": null, \"canJoin\": true, \"canWatch\": false } ] }";
	T.AssertEquals(2, HOLManagerClientRead::GetNumLobbies(List), "two rows");
	MC_Eq(T, "Plain", HOLManagerClientRead::LobbiesField(List, 0, "Map"), "row map");
	MC_Eq(T, "Medium", HOLManagerClientRead::LobbiesField(List, 0, "Size"), "row size");
	T.AssertTrue(HOLManagerClientRead::LobbiesField(List, 1, "Map") == "" && HOLManagerClientRead::LobbiesField(List, 1, "Size") == "", "older row: no map nor size");
}

void Test_ManagerClientMapSizeLobbySend(FUnitTest& T)
{
	FString Sized = HOLManagerClientRead::MakeSettingsBody("Plain", false, false, false, "Small", true, false, "", "", "", false, false);
	MC_Eq(T, "Small", HOLManagerClientRead::JsonField(Sized, "size"), "size sent");
	T.AssertTrue(HOLManagerClientRead::JsonHasField(Sized, "teamsTogether") && HOLManagerClientRead::JsonField(Sized, "teamsTogether") == "false", "teams together sent");
	FString Plain = HOLManagerClientRead::MakeSettingsBody("Plain", false, false, false, "", false, false, "", "", "", false, false);
	T.AssertTrue(!HOLManagerClientRead::JsonHasField(Plain, "size") && !HOLManagerClientRead::JsonHasField(Plain, "teamsTogether"), "no size nor teams together unless given");
	T.AssertFalse(HOLManagerClientRead::JsonHasField(Plain, "mode"), "no mode unless given");
	FString Diplomatic = HOLManagerClientRead::MakeSettingsBody("Plain", false, false, false, "", false, false, "", "Diplomacy", "", false, false);
	MC_Eq(T, "Diplomacy", HOLManagerClientRead::JsonField(Diplomatic, "mode"), "mode sent");
}
