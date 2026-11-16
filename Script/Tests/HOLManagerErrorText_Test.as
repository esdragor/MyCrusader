// Lot E3-12 of the migration (D830, D836, D839): the script port of HOLManagerErrorTextTests.cpp (3 tests), run against the read library of the
// rule that STAYS in C++ (UHOLManagerErrorTextReadLibrary, namespace HOLManagerErrorTextRead, S1D-1116c). Same vectors as the C++ tests.
// Not ported (the C++ test stays until lot MIG-C++-FINAL): in HeroesOfLiberty.UI.MatchLossText, the 401 of FHOLManagerClientRules::ReadFailure
// (not exposed: the bad_key text and the greyed "Creer un salon" are checked on a hand-made bad_key instead).

FHOLManagerErrorValue ManagerErrorText_Protocol(const FString& Code, const FString& Message)
{
	FHOLManagerErrorValue Error;
	Error.Kind = EHOLManagerErrorKind::Protocol;
	Error.HttpCode = 409;
	Error.Code = Code;
	Error.Message = Message;
	return Error;
}

void ManagerErrorText_CheckCode(FUnitTest& T, const FString& Code, const FString& Text)
{
	FString Shown = HOLManagerErrorTextRead::GetManagerErrorText(ManagerErrorText_Protocol(Code, "Raw message.")).ToString();
	T.AssertEquals(Text, Shown, Code);
}

// HeroesOfLiberty.UI.ManagerErrorText.Codes: the text shown for each failure of a call to the lobby manager (R4.3), every code of Manager/PROTOCOL.md.
void Test_ManagerErrorTextCodes(FUnitTest& T)
{
	FHOLManagerErrorValue Unreachable;
	Unreachable.Kind = EHOLManagerErrorKind::Unreachable;
	T.AssertEquals("Le serveur de parties ne répond pas.", HOLManagerErrorTextRead::GetManagerErrorText(Unreachable).ToString(), "unreachable");
	FHOLManagerErrorValue Unreadable;
	Unreadable.Kind = EHOLManagerErrorKind::Unreadable;
	Unreadable.HttpCode = 502;
	T.AssertEquals("Réponse illisible du serveur de parties.", HOLManagerErrorTextRead::GetManagerErrorText(Unreadable).ToString(), "unreadable");

	// The manager's message as LobbyStore words it: "Game version {client}, served {served}."
	T.AssertEquals("Version différente : le serveur a 2026-10-23-b4a8c8c, vous avez 2026-10-24-aaaaaaa.",
		HOLManagerErrorTextRead::GetManagerErrorText(ManagerErrorText_Protocol("version_mismatch",
			"Game version 2026-10-24-aaaaaaa, served 2026-10-23-b4a8c8c.")).ToString(), "version_mismatch");
	T.AssertEquals("Version différente de celle du serveur de parties.",
		HOLManagerErrorTextRead::GetManagerErrorText(ManagerErrorText_Protocol("version_mismatch", "Other version.")).ToString(),
		"version_mismatch without both versions");
	// The /status of a manager serving another version: the same text.
	FHOLManagerErrorValue FromStatus = HOLManagerErrorTextRead::MakeVersionMismatchError("other-version", "2026-10-24-aaaaaaa");
	T.AssertTrue(FromStatus.Kind == EHOLManagerErrorKind::Protocol, "status mismatch is a protocol error");
	T.AssertEquals("version_mismatch", FromStatus.Code, "status mismatch code");
	T.AssertEquals("Version différente : le serveur a other-version, vous avez 2026-10-24-aaaaaaa.",
		HOLManagerErrorTextRead::GetManagerErrorText(FromStatus).ToString(), "status mismatch text");

	ManagerErrorText_CheckCode(T, "protocol_mismatch", "Version différente de celle du serveur de parties.");
	ManagerErrorText_CheckCode(T, "invalid_name", "Nom refusé : 1 à 16 lettres, chiffres, espaces, - ou _.");
	ManagerErrorText_CheckCode(T, "lobby_not_found", "Ce salon n'existe plus.");
	ManagerErrorText_CheckCode(T, "not_in_lobby", "Vous n'êtes plus dans ce salon.");
	ManagerErrorText_CheckCode(T, "lobby_full", "Ce salon est plein.");
	ManagerErrorText_CheckCode(T, "name_taken", "Ce nom est déjà pris dans ce salon.");
	ManagerErrorText_CheckCode(T, "lobby_in_game", "Ce salon est en partie : vous pouvez la regarder.");
	ManagerErrorText_CheckCode(T, "color_taken", "Cette couleur est déjà prise.");
	ManagerErrorText_CheckCode(T, "not_leader", "Seul le chef du salon peut le faire.");
	ManagerErrorText_CheckCode(T, "slot_taken", "Cet emplacement est tenu par un joueur.");
	ManagerErrorText_CheckCode(T, "not_all_ready", "Tous les joueurs ne sont pas prêts.");
	ManagerErrorText_CheckCode(T, "not_enough_players", "Il faut au moins 2 joueurs.");
	ManagerErrorText_CheckCode(T, "max_servers", "Trop de parties en cours : réessayez plus tard.");
	ManagerErrorText_CheckCode(T, "server_start_failed", "Le serveur de la partie n'a pas démarré.");
	ManagerErrorText_CheckCode(T, "no_match", "Ce salon n'est pas en partie.");
	ManagerErrorText_CheckCode(T, "match_not_found", "Cette partie n'existe plus.");
	// The general case: the protocol's message.
	ManagerErrorText_CheckCode(T, "bad_request", "Erreur du serveur de parties : Raw message.");
	ManagerErrorText_CheckCode(T, "not_local", "Erreur du serveur de parties : Raw message.");
	ManagerErrorText_CheckCode(T, "some_future_code", "Erreur du serveur de parties : Raw message.");
}

void ManagerErrorText_CheckLoss(FUnitTest& T, ENetworkFailure Failure, const FString& Message, const FString& Code, const FString& Text)
{
	FHOLManagerErrorValue Error = HOLManagerErrorTextRead::MakeMatchLossError(Failure, Message);
	FString What = "" + Message + " -> " + Code;
	T.AssertTrue(Error.Kind != EHOLManagerErrorKind::None, What + " is an error");
	T.AssertEquals(Code, Error.Code, What + " code");
	T.AssertEquals(Text, HOLManagerErrorTextRead::GetManagerErrorText(Error).ToString(), What + " text");
}

// HeroesOfLiberty.UI.MatchLossText (R4.9): why a match opened from a lobby was lost, each network or travel failure, its code and text.
void Test_MatchLossText(FUnitTest& T)
{
	ManagerErrorText_CheckLoss(T, ENetworkFailure::ConnectionLost, "Your connection to the host has been lost.", "match_lost", "La connexion à la partie est perdue.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::ConnectionTimeout, "UNetConnection::Tick: Connection TIMED OUT.", "match_lost", "La connexion à la partie est perdue.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::FailureReceived, "Server shutting down", "match_closed", "La partie a été fermée par son serveur.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::PendingConnectionFailure, "Version mismatch: server a, client b", "match_version", "Version différente de celle de la partie.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::OutdatedClient, "", "match_version", "Version différente de celle de la partie.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::OutdatedServer, "", "match_version", "Version différente de celle de la partie.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::PendingConnectionFailure, "Server full.", "match_full", "La partie est pleine.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::PendingConnectionFailure, "Something else", "match_refused", "La partie a refusé la connexion.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::PendingConnectionFailure, "Access key mismatch", "match_key", "Clé d'accès refusée : réinstallez le jeu avec le kit reçu.");
	ManagerErrorText_CheckLoss(T, ENetworkFailure::NetDriverCreateFailure, "", "match_failed", "La connexion à la partie a échoué.");

	// A travel to the match that fails (no server at the address...): the match could not be reached.
	FHOLManagerErrorValue Travel = HOLManagerErrorTextRead::MakeMatchTravelError("Failed to load package");
	T.AssertEquals("match_failed", Travel.Code, "travel code");
	T.AssertEquals("La connexion à la partie a échoué.", HOLManagerErrorTextRead::GetManagerErrorText(Travel).ToString(), "travel text");

	// Launcher L11: the manager's 401 for a wrong or missing X-HOL-Key (a hand-made bad_key: ReadFailure is not exposed).
	FHOLManagerErrorValue BadKey = ManagerErrorText_Protocol("bad_key", "Wrong access key.");
	T.AssertEquals("Clé d'accès refusée : réinstallez le jeu avec le kit reçu.", HOLManagerErrorTextRead::GetManagerErrorText(BadKey).ToString(), "401 bad_key text");
	// Launcher L13b: a refused key greys "Creer un salon"; no error or any other failure keeps it.
	T.AssertFalse(HOLManagerErrorTextRead::CanCreateLobbyAfter(BadKey), "bad_key greys Create");
	T.AssertTrue(HOLManagerErrorTextRead::CanCreateLobbyAfter(FHOLManagerErrorValue()), "no error keeps Create");
	FHOLManagerErrorValue Unreachable;
	Unreachable.Kind = EHOLManagerErrorKind::Unreachable;
	T.AssertTrue(HOLManagerErrorTextRead::CanCreateLobbyAfter(Unreachable), "unreachable keeps Create");
	T.AssertTrue(HOLManagerErrorTextRead::CanCreateLobbyAfter(ManagerErrorText_Protocol("max_servers", "Raw.")), "other code keeps Create");
}

void ManagerErrorText_CheckReturn(FUnitTest& T, const FString& Code, const FString& Text)
{
	FHOLManagerErrorValue Error = HOLManagerErrorTextRead::MakeReturnToLobbyError(ManagerErrorText_Protocol(Code, "Raw message."));
	T.AssertTrue(Error.Kind != EHOLManagerErrorKind::None, Code + " is an error");
	T.AssertEquals(Text, HOLManagerErrorTextRead::GetManagerErrorText(Error).ToString(), Code);
}

// HeroesOfLiberty.UI.ManagerErrorText.ReturnToLobby (M6-Q14, D2b): an ordinary join's refusal, but a lobby back in game was relaunched without us.
void Test_ManagerErrorTextReturnToLobby(FUnitTest& T)
{
	ManagerErrorText_CheckReturn(T, "lobby_in_game", "La partie a été relancée sans vous.");
	ManagerErrorText_CheckReturn(T, "name_taken", "Ce nom est déjà pris dans ce salon.");
	ManagerErrorText_CheckReturn(T, "lobby_full", "Ce salon est plein.");
	ManagerErrorText_CheckReturn(T, "lobby_not_found", "Ce salon n'existe plus.");
	FHOLManagerErrorValue Unreachable;
	Unreachable.Kind = EHOLManagerErrorKind::Unreachable;
	T.AssertEquals("Le serveur de parties ne répond pas.",
		HOLManagerErrorTextRead::GetManagerErrorText(HOLManagerErrorTextRead::MakeReturnToLobbyError(Unreachable)).ToString(), "unreachable");
}
