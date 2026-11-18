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
		{ ENetworkFailure::PendingConnectionFailure, TEXT("Access key mismatch"), TEXT("match_key"), TEXT("Clé d'accès refusée : réinstallez le jeu avec le kit reçu.") },
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
	// Launcher L11: the manager's 401 for a wrong or missing X-HOL-Key.
	const FHOLManagerError BadKey = FHOLManagerClientRules::ReadFailure(true, 401, TEXT(R"({ "error": "bad_key", "message": "Wrong access key." })"));
	TestEqual(TEXT("401 bad_key code"), BadKey.Code, FString(TEXT("bad_key")));
	TestEqual(TEXT("401 bad_key text"), GetManagerErrorText(BadKey).ToString(), FString(TEXT("Clé d'accès refusée : réinstallez le jeu avec le kit reçu.")));
	// Launcher L13b: a refused key greys "Créer un salon"; no error or any other failure keeps it.
	TestFalse(TEXT("bad_key greys Create"), CanCreateLobbyAfter(BadKey));
	TestTrue(TEXT("no error keeps Create"), CanCreateLobbyAfter(FHOLManagerError()));
	TestTrue(TEXT("unreachable keeps Create"), CanCreateLobbyAfter(FHOLManagerError{ FHOLManagerError::EKind::Unreachable }));
	TestTrue(TEXT("other code keeps Create"), CanCreateLobbyAfter(MakeProtocolError(TEXT("max_servers"), TEXT("Raw."))));
	return true;
}

#endif
