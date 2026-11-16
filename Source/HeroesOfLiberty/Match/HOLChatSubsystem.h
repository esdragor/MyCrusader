// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLChatSubsystem.generated.h"

/**
 * The seats as the server sees them for the chat (A4; D385, D376, D792, D806): per pair a mutual alliance (MateByPair[From * Slots + To],
 * the relation Ally of the replicated stances, D375: not transitive in Diplomacy), per slot a spectator (defeated), a team game. A slot
 * outside them is an observer without a seat (D806). Read by UHOLChatSubsystem::ReadSeats, judged by Script/Chat/HOLChatRules.as.
 */
USTRUCT(BlueprintType)
struct FHOLChatSeats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Chat")
	int32 Slots = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Chat")
	TArray<bool> MateByPair;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Chat")
	TArray<bool> SpectatorBySlot;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Chat")
	bool bTeamGame = false;
};

/** The gate's answer to a post (HOLChat::CheckPost) and the sender's recent posts after it (the rate, ms of the world's time). */
USTRUCT(BlueprintType)
struct FHOLChatGateAnswer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Chat")
	int32 Answer = -1;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Chat")
	TArray<int32> RecentMs;
};

/**
 * The chat's rules where the match runs (A4, ChatInProgress.md; MIG-P1b, D830/D835: the C++ twin HOLChatRules.cpp is gone). The thinnest
 * bridge, as UHOLSignalSubsystem: the server reads the seats (C++, replicated state) and asks its script subclass
 * (Script/Chat/HOLChatBoardScript.as, the rule HOLChatRules.as) the gate's answer and who reads a message; the player controller's
 * ServerPostChat sends it to those seats only. Abstract: only a script subclass is born. A client never decides.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLChatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The channels on the wire (HOLChat::All, Allies, Private of the script) and the gate's Ok; NoScript without the script board. */
	static constexpr int32 ChannelAll = 0;
	static constexpr int32 ChannelAllies = 1;
	static constexpr int32 ChannelPrivate = 2;
	static constexpr int32 AnswerOk = 0;
	static constexpr int32 NoScript = -1;

	/** The world's chat board (its script subclass), or null (no script compiled, or no world). */
	static UHOLChatSubsystem* Find(const UWorld* World);

	/** The seats of World's match (the replicated stances of AHOLGameState and its defeated slots; a team game outside Diplomacy). */
	static FHOLChatSeats ReadSeats(const UWorld* World);

	/** "all", "allies" or "private" (case ignored): the channel, INDEX_NONE otherwise (hol.Chat.Say). */
	static int32 ParseChannel(const FString& Name);

	/** The server's gate for a post of Sender at NowMs (a limit <= 0: the script's default). NoScript without the script board. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Chat")
	FHOLChatGateAnswer CheckPostNow(const FHOLChatSeats& Seats, int32 Channel, int32 Sender, int32 Target, const FString& Text, const TArray<int32>& RecentMs,
		int32 NowMs, int32 MaxLength, int32 MaxPerWindow, int32 WindowMs);

	/** Does the message of Sender on Channel (to Target for a private one) reach Receiver? False without the script board. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Chat")
	bool ReachesNow(const FHOLChatSeats& Seats, int32 Channel, int32 Sender, int32 Target, int32 Receiver);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Chat")
	FHOLChatGateAnswer ChatCheckPost(const FHOLChatSeats& Seats, int32 Channel, int32 Sender, int32 Target, const FString& Text, const TArray<int32>& RecentMs,
		int32 NowMs, int32 MaxLength, int32 MaxPerWindow, int32 WindowMs);
	virtual FHOLChatGateAnswer ChatCheckPost_Implementation(const FHOLChatSeats& Seats, int32 Channel, int32 Sender, int32 Target, const FString& Text,
		const TArray<int32>& RecentMs, int32 NowMs, int32 MaxLength, int32 MaxPerWindow, int32 WindowMs);

	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Chat")
	bool ChatReaches(const FHOLChatSeats& Seats, int32 Channel, int32 Sender, int32 Target, int32 Receiver);
	virtual bool ChatReaches_Implementation(const FHOLChatSeats& Seats, int32 Channel, int32 Sender, int32 Target, int32 Receiver) { return false; }
};
