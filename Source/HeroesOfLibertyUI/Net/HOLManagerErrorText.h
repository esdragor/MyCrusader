// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Net/HOLManagerClientRules.h"

/**
 * The text shown for a failed call to the lobby manager (M6-Q5, R4.3): chosen from the kind of failure and the
 * protocol's stable code (Manager/PROTOCOL.md, "Erreurs"); an unknown code shows the protocol's raw message.
 */
HEROESOFLIBERTYUI_API FText GetManagerErrorText(const FHOLManagerError& Error);

/** The version_mismatch of a manager whose /status serves another version than ours, worded as the manager's. */
HEROESOFLIBERTYUI_API FHOLManagerError MakeVersionMismatchError(const FString& Served, const FString& Ours);

/**
 * Why a match opened from a lobby was lost (R4.9), from UEngine::OnNetworkFailure: shown by the Multiplayer screen like
 * a manager's error. Codes: match_lost (connection lost or timed out), match_closed (the server closed it), match_version,
 * match_full, match_refused (refused at login), match_failed (anything else).
 */
HEROESOFLIBERTYUI_API FHOLManagerError MakeMatchLossError(ENetworkFailure::Type Failure, const FString& Message);

/** The same for UEngine::OnTravelFailure: match_failed. */
HEROESOFLIBERTYUI_API FHOLManagerError MakeMatchTravelError(const FString& Message);

/**
 * A refused "Retour au salon" (M6-Q14): the ordinary join's refusal, but lobby_in_game becomes lobby_relaunched (the match
 * was relaunched without us). Shown over the Multiplayer screen's list.
 */
HEROESOFLIBERTYUI_API FHOLManagerError MakeReturnToLobbyError(const FHOLManagerError& JoinError);
