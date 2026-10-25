// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Net/HOLManagerErrorText.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	FHOLManagerError MakeProtocolError(const TCHAR* Code, const TCHAR* Message)
	{
		FHOLManagerError Error;
		Error.Kind = FHOLManagerError::EKind::Protocol;
		Error.HttpCode = 409;
		Error.Code = Code;
		Error.Message = Message;
		return Error;
	}
}

// The text shown for each failure of a call to the lobby manager (R4.3): every code of Manager/PROTOCOL.md.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLManagerErrorTextTest, "HeroesOfLiberty.UI.ManagerErrorText",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLManagerErrorTextTest::RunTest(const FString& Parameters)
{
	FHOLManagerError Unreachable;
	Unreachable.Kind = FHOLManagerError::EKind::Unreachable;
	TestEqual(TEXT("unreachable"), GetManagerErrorText(Unreachable).ToString(), FString(TEXT("Le serveur de parties ne répond pas.")));
	FHOLManagerError Unreadable;
	Unreadable.Kind = FHOLManagerError::EKind::Unreadable;
	Unreadable.HttpCode = 502;
	TestEqual(TEXT("unreadable"), GetManagerErrorText(Unreadable).ToString(), FString(TEXT("Réponse illisible du serveur de parties.")));

	// The manager's message as LobbyStore words it: "Game version {client}, served {served}."
	TestEqual(TEXT("version_mismatch"), GetManagerErrorText(MakeProtocolError(TEXT("version_mismatch"),
		TEXT("Game version 2026-10-24-aaaaaaa, served 2026-10-23-b4a8c8c."))).ToString(),
		FString(TEXT("Version différente : le serveur a 2026-10-23-b4a8c8c, vous avez 2026-10-24-aaaaaaa.")));
	TestEqual(TEXT("version_mismatch without both versions"), GetManagerErrorText(MakeProtocolError(TEXT("version_mismatch"),
		TEXT("Other version."))).ToString(), FString(TEXT("Version différente de celle du serveur de parties.")));
	// The /status of a manager serving another version: the same text.
	const FHOLManagerError FromStatus = MakeVersionMismatchError(TEXT("other-version"), TEXT("2026-10-24-aaaaaaa"));
	TestTrue(TEXT("status mismatch is a protocol error"), FromStatus.Kind == FHOLManagerError::EKind::Protocol);
	TestEqual(TEXT("status mismatch code"), FromStatus.Code, FString(TEXT("version_mismatch")));
	TestEqual(TEXT("status mismatch text"), GetManagerErrorText(FromStatus).ToString(),
		FString(TEXT("Version différente : le serveur a other-version, vous avez 2026-10-24-aaaaaaa.")));

	struct FCase { const TCHAR* Code; const TCHAR* Text; };
	const FCase Cases[] = {
		{ TEXT("protocol_mismatch"), TEXT("Version différente de celle du serveur de parties.") },
		{ TEXT("invalid_name"), TEXT("Nom refusé : 1 à 16 lettres, chiffres, espaces, - ou _.") },
		{ TEXT("lobby_not_found"), TEXT("Ce salon n'existe plus.") },
		{ TEXT("not_in_lobby"), TEXT("Vous n'êtes plus dans ce salon.") },
		{ TEXT("lobby_full"), TEXT("Ce salon est plein.") },
		{ TEXT("name_taken"), TEXT("Ce nom est déjà pris dans ce salon.") },
		{ TEXT("lobby_in_game"), TEXT("Ce salon est en partie : vous pouvez la regarder.") },
		{ TEXT("color_taken"), TEXT("Cette couleur est déjà prise.") },
		{ TEXT("not_leader"), TEXT("Seul le chef du salon peut le faire.") },
		{ TEXT("slot_taken"), TEXT("Cet emplacement est tenu par un joueur.") },
		{ TEXT("not_all_ready"), TEXT("Tous les joueurs ne sont pas prêts.") },
		{ TEXT("not_enough_players"), TEXT("Il faut au moins 2 joueurs.") },
		{ TEXT("max_servers"), TEXT("Trop de parties en cours : réessayez plus tard.") },
		{ TEXT("server_start_failed"), TEXT("Le serveur de la partie n'a pas démarré.") },
		{ TEXT("no_match"), TEXT("Ce salon n'est pas en partie.") },
		{ TEXT("match_not_found"), TEXT("Cette partie n'existe plus.") },
		// The general case: the protocol's message.
		{ TEXT("bad_request"), TEXT("Erreur du serveur de parties : Raw message.") },
		{ TEXT("not_local"), TEXT("Erreur du serveur de parties : Raw message.") },
		{ TEXT("some_future_code"), TEXT("Erreur du serveur de parties : Raw message.") },
	};
	for (const FCase& Case : Cases)
	{
		TestEqual(Case.Code, GetManagerErrorText(MakeProtocolError(Case.Code, TEXT("Raw message."))).ToString(), FString(Case.Text));
	}
	return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

// Why a match opened from a lobby was lost (R4.9): each network or travel failure, its code and text.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchLossTextTest, "HeroesOfLiberty.UI.MatchLossText",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMatchLossTextTest::RunTest(const FString& Parameters)
{
	struct FCase { ENetworkFailure::Type Failure; const TCHAR* Message; const TCHAR* Code; const TCHAR* Text; };
	const FCase Cases[] = {
		{ ENetworkFailure::ConnectionLost, TEXT("Your connection to the host has been lost."), TEXT("match_lost"), TEXT("La connexion à la partie est perdue.") },
		{ ENetworkFailure::ConnectionTimeout, TEXT("UNetConnection::Tick: Connection TIMED OUT."), TEXT("match_lost"), TEXT("La connexion à la partie est perdue.") },
		{ ENetworkFailure::FailureReceived, TEXT("Server shutting down"), TEXT("match_closed"), TEXT("La partie a été fermée par son serveur.") },
		{ ENetworkFailure::PendingConnectionFailure, TEXT("Version mismatch: server a, client b"), TEXT("match_version"), TEXT("Version différente de celle de la partie.") },
		{ ENetworkFailure::OutdatedClient, TEXT(""), TEXT("match_version"), TEXT("Version différente de celle de la partie.") },
		{ ENetworkFailure::OutdatedServer, TEXT(""), TEXT("match_version"), TEXT("Version différente de celle de la partie.") },
		{ ENetworkFailure::PendingConnectionFailure, TEXT("Server full."), TEXT("match_full"), TEXT("La partie est pleine.") },
		{ ENetworkFailure::PendingConnectionFailure, TEXT("Something else"), TEXT("match_refused"), TEXT("La partie a refusé la connexion.") },
		{ ENetworkFailure::NetDriverCreateFailure, TEXT(""), TEXT("match_failed"), TEXT("La connexion à la partie a échoué.") },
	};
	for (const FCase& Case : Cases)
	{
		const FHOLManagerError Error = MakeMatchLossError(Case.Failure, Case.Message);
		const FString What = FString::Printf(TEXT("%s (%s)"), ENetworkFailure::ToString(Case.Failure), Case.Message);
		TestTrue(What + TEXT(" is an error"), Error.IsError());
		TestEqual(What + TEXT(" code"), Error.Code, FString(Case.Code));
		TestEqual(What + TEXT(" text"), GetManagerErrorText(Error).ToString(), FString(Case.Text));
	}
	// A travel to the match that fails (no server at the address...): the match could not be reached.
	const FHOLManagerError Travel = MakeMatchTravelError(TEXT("Failed to load package"));
	TestEqual(TEXT("travel code"), Travel.Code, FString(TEXT("match_failed")));
	TestEqual(TEXT("travel text"), GetManagerErrorText(Travel).ToString(), FString(TEXT("La connexion à la partie a échoué.")));
	return true;
}

// "Retour au salon" refused (M6-Q14, D2b): an ordinary join's refusal, but a lobby back in game was relaunched without us.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLReturnToLobbyTextTest, "HeroesOfLiberty.UI.ManagerErrorText.ReturnToLobby",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLReturnToLobbyTextTest::RunTest(const FString& Parameters)
{
	struct FCase { const TCHAR* Code; const TCHAR* Text; };
	const FCase Cases[] = {
		{ TEXT("lobby_in_game"), TEXT("La partie a été relancée sans vous.") },
		{ TEXT("name_taken"), TEXT("Ce nom est déjà pris dans ce salon.") },
		{ TEXT("lobby_full"), TEXT("Ce salon est plein.") },
		{ TEXT("lobby_not_found"), TEXT("Ce salon n'existe plus.") },
	};
	for (const FCase& Case : Cases)
	{
		const FHOLManagerError Error = MakeReturnToLobbyError(MakeProtocolError(Case.Code, TEXT("Raw message.")));
		TestTrue(FString(Case.Code) + TEXT(" is an error"), Error.IsError());
		TestEqual(Case.Code, GetManagerErrorText(Error).ToString(), FString(Case.Text));
	}
	FHOLManagerError Unreachable;
	Unreachable.Kind = FHOLManagerError::EKind::Unreachable;
	TestEqual(TEXT("unreachable"), GetManagerErrorText(MakeReturnToLobbyError(Unreachable)).ToString(),
		FString(TEXT("Le serveur de parties ne répond pas.")));
	return true;
}

#endif
