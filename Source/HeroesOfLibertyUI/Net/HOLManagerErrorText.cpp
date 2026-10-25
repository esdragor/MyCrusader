// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLManagerErrorText.h"

#define LOCTEXT_NAMESPACE "HOLManagerErrorText"

namespace UE::HeroesOfLiberty::UI::ManagerError
{
	/** "Game version {client}, served {served}." (Manager/HOLManager/LobbyStore.cs) into its two versions. */
	bool SplitVersions(const FString& Message, FString& Client, FString& Served)
	{
		const FString Head = TEXT("Game version ");
		const FString Middle = TEXT(", served ");
		int32 MiddleAt = INDEX_NONE;
		if (!Message.StartsWith(Head) || (MiddleAt = Message.Find(Middle)) == INDEX_NONE)
		{
			return false;
		}
		Client = Message.Mid(Head.Len(), MiddleAt - Head.Len());
		Served = Message.Mid(MiddleAt + Middle.Len());
		Served.RemoveFromEnd(TEXT("."));
		return !Client.IsEmpty() && !Served.IsEmpty();
	}
}

FText GetManagerErrorText(const FHOLManagerError& Error)
{
	switch (Error.Kind)
	{
	case FHOLManagerError::EKind::None:
		return FText::GetEmpty();
	case FHOLManagerError::EKind::Unreachable:
		return LOCTEXT("Unreachable", "Le serveur de parties ne répond pas.");
	case FHOLManagerError::EKind::Unreadable:
		return LOCTEXT("Unreadable", "Réponse illisible du serveur de parties.");
	default:
		break;
	}
	const FString& Code = Error.Code;
	if (Code == TEXT("version_mismatch"))
	{
		FString Client, Served;
		if (UE::HeroesOfLiberty::UI::ManagerError::SplitVersions(Error.Message, Client, Served))
		{
			return FText::Format(LOCTEXT("VersionMismatch", "Version différente : le serveur a {0}, vous avez {1}."),
				FText::FromString(Served), FText::FromString(Client));
		}
		return LOCTEXT("OtherVersion", "Version différente de celle du serveur de parties.");
	}
	if (Code == TEXT("protocol_mismatch")) { return LOCTEXT("OtherVersion", "Version différente de celle du serveur de parties."); }
	if (Code == TEXT("invalid_name")) { return LOCTEXT("InvalidName", "Nom refusé : 1 à 16 lettres, chiffres, espaces, - ou _."); }
	if (Code == TEXT("lobby_not_found")) { return LOCTEXT("LobbyNotFound", "Ce salon n'existe plus."); }
	if (Code == TEXT("not_in_lobby")) { return LOCTEXT("NotInLobby", "Vous n'êtes plus dans ce salon."); }
	if (Code == TEXT("lobby_full")) { return LOCTEXT("LobbyFull", "Ce salon est plein."); }
	if (Code == TEXT("name_taken")) { return LOCTEXT("NameTaken", "Ce nom est déjà pris dans ce salon."); }
	if (Code == TEXT("lobby_in_game")) { return LOCTEXT("LobbyInGame", "Ce salon est en partie : vous pouvez la regarder."); }
	if (Code == TEXT("color_taken")) { return LOCTEXT("ColorTaken", "Cette couleur est déjà prise."); }
	if (Code == TEXT("not_leader")) { return LOCTEXT("NotLeader", "Seul le chef du salon peut le faire."); }
	if (Code == TEXT("slot_taken")) { return LOCTEXT("SlotTaken", "Cet emplacement est tenu par un joueur."); }
	if (Code == TEXT("not_all_ready")) { return LOCTEXT("NotAllReady", "Tous les joueurs ne sont pas prêts."); }
	if (Code == TEXT("not_enough_players")) { return LOCTEXT("NotEnoughPlayers", "Il faut au moins 2 joueurs."); }
	if (Code == TEXT("max_servers")) { return LOCTEXT("MaxServers", "Trop de parties en cours : réessayez plus tard."); }
	if (Code == TEXT("server_start_failed")) { return LOCTEXT("ServerStartFailed", "Le serveur de la partie n'a pas démarré."); }
	if (Code == TEXT("no_match")) { return LOCTEXT("NoMatch", "Ce salon n'est pas en partie."); }
	if (Code == TEXT("match_not_found")) { return LOCTEXT("MatchNotFound", "Cette partie n'existe plus."); }
	// A match opened from a lobby, lost (R4.9, MakeMatchLossError).
	if (Code == TEXT("match_lost")) { return LOCTEXT("MatchLost", "La connexion à la partie est perdue."); }
	if (Code == TEXT("match_closed")) { return LOCTEXT("MatchClosed", "La partie a été fermée par son serveur."); }
	if (Code == TEXT("match_version")) { return LOCTEXT("MatchVersion", "Version différente de celle de la partie."); }
	if (Code == TEXT("match_full")) { return LOCTEXT("MatchFull", "La partie est pleine."); }
	if (Code == TEXT("match_refused")) { return LOCTEXT("MatchRefused", "La partie a refusé la connexion."); }
	// "Retour au salon" refused because the lobby went back in game (M6-Q14, MakeReturnToLobbyError).
	if (Code == TEXT("lobby_relaunched")) { return LOCTEXT("LobbyRelaunched", "La partie a été relancée sans vous."); }
	if (Code == TEXT("match_failed")) { return LOCTEXT("MatchFailed", "La connexion à la partie a échoué."); }
	// bad_request, not_local and codes the game does not know yet (PROTOCOL.md): the raw message.
	return FText::Format(LOCTEXT("General", "Erreur du serveur de parties : {0}"), FText::FromString(Error.Message));
}

FHOLManagerError MakeVersionMismatchError(const FString& Served, const FString& Ours)
{
	FHOLManagerError Error;
	Error.Kind = FHOLManagerError::EKind::Protocol;
	Error.HttpCode = 409;
	Error.Code = TEXT("version_mismatch");
	Error.Message = FString::Printf(TEXT("Game version %s, served %s."), *Ours, *Served);
	return Error;
}

FHOLManagerError MakeMatchLossError(const ENetworkFailure::Type Failure, const FString& Message)
{
	FHOLManagerError Error;
	Error.Kind = FHOLManagerError::EKind::Protocol;
	Error.Message = FString::Printf(TEXT("%s: %s"), ENetworkFailure::ToString(Failure), *Message);
	switch (Failure)
	{
	case ENetworkFailure::ConnectionLost:
	case ENetworkFailure::ConnectionTimeout:
		Error.Code = TEXT("match_lost");
		break;
	case ENetworkFailure::FailureReceived:
		Error.Code = TEXT("match_closed");
		break;
	case ENetworkFailure::OutdatedClient:
	case ENetworkFailure::OutdatedServer:
		Error.Code = TEXT("match_version");
		break;
	case ENetworkFailure::PendingConnectionFailure:
		// The server's PreLogin refusal: HOLVersion's "Version mismatch: ...", the engine's "Server full.".
		Error.Code = Message.Contains(TEXT("version")) ? TEXT("match_version")
			: Message.Contains(TEXT("full")) ? TEXT("match_full") : TEXT("match_refused");
		break;
	default:
		Error.Code = TEXT("match_failed");
		break;
	}
	return Error;
}

FHOLManagerError MakeMatchTravelError(const FString& Message)
{
	FHOLManagerError Error;
	Error.Kind = FHOLManagerError::EKind::Protocol;
	Error.Code = TEXT("match_failed");
	Error.Message = Message;
	return Error;
}

FHOLManagerError MakeReturnToLobbyError(const FHOLManagerError& JoinError)
{
	FHOLManagerError Error = JoinError;
	if (Error.Code == TEXT("lobby_in_game"))
	{
		Error.Code = TEXT("lobby_relaunched");
	}
	return Error;
}

#undef LOCTEXT_NAMESPACE
