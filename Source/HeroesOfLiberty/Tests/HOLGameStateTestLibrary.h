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
 * A forum's public state as plain ints (FHOLForumState has no script type): Phase as EHOLForumPhase, Hold as EHOLForumHold, HolderSlot -1 for none,
 * bDown = IsDown. Made by UHOLGameStateTestLibrary::GetForumState.
 */
USTRUCT(BlueprintType)
struct FHOLTestForumState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|GameStateTest")
	int32 Falls = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|GameStateTest")
	int32 Phase = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|GameStateTest")
	int32 RuinStepsLeft = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|GameStateTest")
	int32 HoldSteps = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|GameStateTest")
	int32 Hold = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|GameStateTest")
	int32 HolderSlot = -1;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|GameStateTest")
	bool bDown = false;
};

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

	// ---- Lot 2, rest (D853, ELROND-1119u): forum state, notices, absences, login, pauses, stock ----

	/** AHOLGameState::GetForumState(Slot) as ints (the default state for a null state). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static FHOLTestForumState GetForumState(const AHOLGameState* State, int32 Slot);

	/** The player state of the state's PlayerArray whose GetPlayerIndex is Slot (the first one found, an AI's included); nullptr if none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static AHOLPlayerState* GetPlayerStateOfSlot(const AHOLGameState* State, int32 Slot);

	/** AHOLGameState::GetAbsentNotices, one line a notice, fields joined by a tab: Slot (-1 a fake one), Name, EndsAt, BackAt, WindowSeconds (%.3f). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static TArray<FString> GetAbsentNotices(const AHOLGameState* State);

	/** GetAbsentNotices().Num(). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 CountAbsentNotices(const AHOLGameState* State);

	/** True when a notice has this Slot and, with bBack false, BackAt <= 0 (still away, banner up); with bBack true, BackAt > 0 (the back notice). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool HasAbsentNotice(const AHOLGameState* State, int32 Slot, bool bBack);

	/** AHOLGameState::GetAbsenceBannerLines(Now), each line FText::ToString. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static TArray<FString> GetAbsenceBannerLines(const AHOLGameState* State, double Now);

	/** AHOLGameState::GetWindowClock(Now): Now minus the time the match spent paused. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static double GetWindowClock(const AHOLGameState* State, double Now);

	/** AHOLGameState::GetSlotsInUse(Except); Except may be null. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static TArray<int32> GetSlotsInUse(const AHOLGameState* State, const AHOLPlayerState* Except);

	/** AHOLGameState::DefeatExpiredAbsences(Now, WindowSeconds) (WindowSeconds of 0 or less: the default window): the slots it defeated. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static TArray<int32> DefeatExpiredAbsences(AHOLGameState* State, double Now, double WindowSeconds);

	/** AHOLGameState::DescribeDisconnections(Now, WindowSeconds) (WindowSeconds of 0 or less: the default window). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static TArray<FString> DescribeDisconnections(const AHOLGameState* State, double Now, double WindowSeconds);

	/** AHOLGameState::SimulateReturn(Slot, Starting, Now) (the hol.Net.FakeAbsent Back path). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool SimulateReturn(AHOLGameState* State, int32 Slot, const FHOLResources& Starting, double Now);

	/** AHOLGameState::FindSlotOnLogin(Config from Json, Name, TakenSlots). -1 (INDEX_NONE) if the JSON fails. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 FindSlotOnLogin(const AHOLGameState* State, const FString& Json, const FString& Name, const TArray<int32>& TakenSlots);

	/** AHOLGameState::FindSlotForLogin(Config from Json, Name, Logging, OutReplaced): the slot (-1 if the JSON fails). The holder it would replace: FindReplacedForLogin. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 FindSlotForLogin(const AHOLGameState* State, const FString& Json, const FString& Name, const AHOLPlayerState* Logging);

	/** The OutReplaced of the same FindSlotForLogin call (the live holder of the seat of that name, to hand over by RetireHolder); nullptr if none. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static AHOLPlayerState* FindReplacedForLogin(const AHOLGameState* State, const FString& Json, const FString& Name, const AHOLPlayerState* Logging);

	/** AHOLGameState::RetireHolder(Old, Now, WindowSeconds) (WindowSeconds of 0 or less: the default window). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static void RetireHolder(AHOLGameState* State, AHOLPlayerState* Old, double Now, double WindowSeconds);

	/** AHOLGameState::RequestPause(Requester, Now): the rule decides. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool RequestPause(AHOLGameState* State, AHOLPlayerState* Requester, double Now);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool RequestResume(AHOLGameState* State, AHOLPlayerState* Requester, double Now);

	/** AHOLGameState::LiftPauseOf(Slot, Now). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool LiftPauseOf(AHOLGameState* State, int32 Slot, double Now);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool IsMatchPaused(const AHOLGameState* State);

	/** AHOLGameState::GetPausesLeft() (one count for the match, no slot). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 GetPausesLeft(const AHOLGameState* State);

	/** AHOLPlayerState::GetResources / SetResources (a slot's stock). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static FHOLResources GetResources(const AHOLPlayerState* Player);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static void SetResources(AHOLPlayerState* Player, const FHOLResources& Resources);

	/** AHOLPlayerState::AddTechnology(Technology), its one-time effects included. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static bool AddTechnology(AHOLPlayerState* Player, FName Technology);

	/** AHOLPlayerState::GetHeroPrivate().XP (0 for null). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|GameStateTest")
	static int32 GetPlayerHeroXP(const AHOLPlayerState* Player);
};
