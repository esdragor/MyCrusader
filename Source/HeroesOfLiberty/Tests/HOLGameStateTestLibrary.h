// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLResources.h"
#include "UObject/Object.h"
#include "HOLGameStateTestLibrary.generated.h"

class AHOLGameState;
class AHOLPlayerState;

/**
 * The forum events of a game state, recorded as plain ints (Kind 0 Fell, 1 HoldStarted, 2 Razed, 3 Defeated, 4 KingDied; Reason 0 None,
 * 1 Disconnected, as FHOLForumEventValue). Made by UHOLGameStateTestLibrary::RecordForumEvents; the script reads them back.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLForumEventRecorder : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	int32 GetCount() const { return Kinds.Num(); }

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	int32 GetKind(int32 Index) const { return Kinds.IsValidIndex(Index) ? Kinds[Index] : -1; }

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	int32 GetSlot(int32 Index) const { return Slots.IsValidIndex(Index) ? Slots[Index] : -1; }

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	int32 GetOtherSlot(int32 Index) const { return OtherSlots.IsValidIndex(Index) ? OtherSlots[Index] : -1; }

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	int32 GetLives(int32 Index) const { return Lives.IsValidIndex(Index) ? Lives[Index] : -1; }

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	int32 GetReason(int32 Index) const { return Reasons.IsValidIndex(Index) ? Reasons[Index] : -1; }

	UPROPERTY()
	TArray<int32> Kinds;
	UPROPERTY()
	TArray<int32> Slots;
	UPROPERTY()
	TArray<int32> OtherSlots;
	UPROPERTY()
	TArray<int32> Lives;
	UPROPERTY()
	TArray<int32> Reasons;
};

/**
 * Access to AHOLGameState and to the match rules for the tests in Angelscript (namespace HOLGameStateTest there; D839: a binding exposed to the
 * script; D853: tests of a world without Mass ported). Separate from UHOLTestWorldLibrary so that Match/HOLGameState.h stays untouched. Plain
 * wrappers, no rule. The bodies exist only where WITH_DEV_AUTOMATION_TESTS; elsewhere they do nothing and return 0 / false / empty. A type the
 * script cannot see (EHOLDefeatReason and the like: UENUM without a script name here) is passed as int32, a config as its JSON
 * (HOLMatchConfigRead::MakeDefaultJson, SetSlotJson).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLGameStateTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** AHOLGameState::InitVictory with the configuration read from Json (FHOLMatchConfig::FromJson). False if the state is null or the JSON fails. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool InitVictoryFromJson(AHOLGameState* State, const FString& Json);

	/** AHOLGameState::DefeatSlot; Reason is the EHOLDefeatReason value (0 None, 1 Surrender, 2 Cheat, 3 Forum, 4 King, 5 Disconnected, 6 SacredSites, 7 Nomad). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool DefeatSlot(AHOLGameState* State, int32 Slot, int32 Reason);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool IsSlotDefeated(const AHOLGameState* State, int32 Slot);

	/** The EHOLDefeatReason value of the slot (0 None for a null state or a slot out of the table). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 GetDefeatReason(const AHOLGameState* State, int32 Slot);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool IsMatchOver(const AHOLGameState* State);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 GetLives(const AHOLGameState* State, int32 Slot);

	/** AHOLGameState::IsForumDown (the forum state's IsDown: FHOLForumState has no script type). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool IsForumDown(const AHOLGameState* State, int32 Slot);

	/** AGameStateBase::AddPlayerState (a PostLogin does it): the player state counts in the state's PlayerArray. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static void AddPlayerState(AHOLGameState* State, AHOLPlayerState* Player);

	/** AHOLGameState::GiveSlotStock(Player, Starting, Now): true when the player came back in his window. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool GiveSlotStock(AHOLGameState* State, AHOLPlayerState* Player, const FHOLResources& Starting, double Now);

	/** AHOLGameState::KeepAbsentSlot(Player, Now, WindowSeconds); a WindowSeconds of 0 or less takes FHOLDisconnectRules::DefaultWindowSeconds. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool KeepAbsentSlot(AHOLGameState* State, AHOLPlayerState* Player, double Now, double WindowSeconds);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool IsSlotAbsent(const AHOLGameState* State, int32 Slot);

	/** Records the OnForumEvent of the state from now on (the recorder lives as long as the script holds it). nullptr for a null state. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static UHOLForumEventRecorder* RecordForumEvents(AHOLGameState* State);

	/** How many Defeated entries the world's UHOLMatchStatsSubsystem timeline holds for PlayerIndex (0 without the subsystem). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 CountStatsDefeats(const AHOLGameState* State, int32 PlayerIndex);

	/**
	 * HOLForumRules::GetPlayersTable for the configuration Json (no forums, default tuning, no book), as one line a row, fields joined by a tab:
	 * Name, State, bAway (0/1), bAIHeld (0/1), Shown. DefeatedMask, AbsentMask, AIHeldMask: bit N = slot N. Empty if the JSON fails.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static TArray<FString> GetPlayersTableRows(const FString& Json, int32 DefeatedMask, int32 LocalSlot, int32 AbsentMask, int32 AIHeldMask);

	/**
	 * FHOLDisconnectRules::FormatSeatReport. SeatState: 0 Present, 1 AbsentAIHolds, 2 AIHolds, 3 Defeated (ESeatState); AIMode: 0 None, 1 Hold, 2 Full
	 * (EAIMode).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static FString FormatSeatReport(int32 Slot, const FString& Name, int32 SeatState, double SecondsLeft, int32 AIMode);
};
