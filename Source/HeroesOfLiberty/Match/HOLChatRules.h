// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;

/**
 * A4 (ChatInProgress.md; GDD 15, D385 channels Allies, All and private messages; D376 and D357 the spectator; D792): the chat's
 * transport rules, pure, for the server (AHOLPlayerController::ServerPostChat). Reaches / Recipients are the C++ mirror of the script
 * rule Script/Chat/HOLChatRules.as (simulation and network in C++, as HOLDiplomacyRules mirrors HOLDiplomacyRules.as): "a camp" is
 * here a mutual alliance of two seats (the relation Ally, D375) instead of a team number, which is the same thing in a team game and
 * the stances' truth in Diplomacy. CheckPost is the server's gate: the text's length and each player's rate (values provisional, cvars
 * hol.Chat.MaxLength, hol.Chat.MaxPerWindow, hol.Chat.WindowMs; Tools/Data/ValueSheet.csv). No display here, no input (Enter, field).
 */
namespace HOLChat
{
	/** The channels (HOLChatRules.as). */
	constexpr int32 All = 0;
	constexpr int32 Allies = 1;
	constexpr int32 Private = 2;

	/** The server's answers to a post (logged as "Chat: slot S channel C answer A"). */
	constexpr int32 Ok = 0;
	constexpr int32 Empty = 1;
	constexpr int32 TooLong = 2;
	constexpr int32 TooFast = 3;
	constexpr int32 BadChannel = 4;
	constexpr int32 BadTarget = 5;

	/** The defaults of the gate (provisional; the cvars read them). */
	constexpr int32 MaxTextLength = 200;
	constexpr int32 MaxPerWindow = 5;
	constexpr int32 WindowMs = 10000;

	/** The seats as the server sees them: per pair a mutual alliance (From * Slots + To), per slot a spectator (defeated), team game. */
	struct FSeats
	{
		int32 Slots = 0;
		TArray<bool> Mates;
		TArray<bool> Spectator;
		bool bTeamGame = false;

		HEROESOFLIBERTY_API bool AreMates(int32 A, int32 B) const;
		/** A slot outside the match (INDEX_NONE: a pure spectator, no seat) is a spectator. */
		HEROESOFLIBERTY_API bool IsSpectator(int32 Slot) const;
	};

	/** Does the message of Sender on Channel (to Target for a private one) reach Receiver? The sender always sees its own. */
	HEROESOFLIBERTY_API bool Reaches(const FSeats& Seats, int32 Channel, int32 Sender, int32 Target, int32 Receiver);

	/** Every slot of the match the message reaches, the sender's first. */
	HEROESOFLIBERTY_API TArray<int32> Recipients(const FSeats& Seats, int32 Channel, int32 Sender, int32 Target);

	/**
	 * The server's gate for a post at NowMs: Ok (and NowMs kept in RecentMs, the sender's recent posts), or why it is refused (an
	 * empty text, longer than MaxLength characters, more than MaxPerWindow posts within WindowMs, an unknown channel, a private
	 * message to no other seat of the match).
	 */
	HEROESOFLIBERTY_API int32 CheckPost(const FSeats& Seats, int32 Channel, int32 Sender, int32 Target, const FString& Text, TArray<int32>& RecentMs,
		int32 NowMs, int32 MaxLength, int32 MaxPerWindow, int32 InWindowMs);

	/** The seats of World's match (the replicated stances of AHOLGameState and its defeated slots; a team game outside Diplomacy). */
	HEROESOFLIBERTY_API FSeats ReadSeats(const UWorld* World);

	/** "all", "allies" or "private" (case ignored): the channel, INDEX_NONE otherwise (hol.Chat.Say). */
	HEROESOFLIBERTY_API int32 ParseChannel(const FString& Name);
}
