// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "HOLGameState.generated.h"

/** Why a slot lost (D356; the forum at 0 lives and the dead King come with V2 and Regicide). None: still in the match. */
UENUM()
enum class EHOLDefeatReason : uint8
{
	None,
	Surrender,
	Cheat,
};

/**
 * The end of the match (D356, D357): which slots are defeated and, once over, who won, replicated to every machine
 * (the end banner and the spectator of V1.3 read it). The server alone writes it, through DefeatSlot; the camps are those
 * of the configuration given at the launch (InitVictory), locked for the match.
 */
UCLASS()
class HEROESOFLIBERTY_API AHOLGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AHOLGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Server only, by the game mode once it has read the configuration: the slots and camps of the match. */
	void InitVictory(const FHOLMatchConfig& Config);

	/**
	 * Server only: the slot loses (an occupied slot not defeated yet, the match not over; false otherwise, nothing
	 * changes), then the rules decide whether the match is over (D357). Logs "Victory: slot N defeated (Reason)" and
	 * "Victory: match over, winners a,b" for the scripted checks.
	 */
	bool DefeatSlot(int32 Slot, EHOLDefeatReason Reason);

	bool IsSlotDefeated(int32 Slot) const { return GetDefeatReason(Slot) != EHOLDefeatReason::None; }
	EHOLDefeatReason GetDefeatReason(int32 Slot) const { return SlotDefeats.IsValidIndex(Slot) ? SlotDefeats[Slot] : EHOLDefeatReason::None; }
	bool IsMatchOver() const { return bMatchOver; }
	bool IsWinner(int32 Slot) const { return Slot >= 0 && Slot < FHOLMatchConfig::MaxSlots && (WinnerMask & (1 << Slot)); }
	TArray<int32> GetWinners() const;

	/**
	 * Server: a slot has just lost, its collapse done (D358, CollapseSlot): its units dying, its buildings in ruins, its
	 * stock at 0; from now on its orders and births are refused (UHOLCommandSubsystem, UHOLUnitSubsystem::RequestUnits).
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSlotDefeated, int32 /*Slot*/, EHOLDefeatReason);
	FOnSlotDefeated OnSlotDefeated;

	/** Server: the match has just ended (after the OnSlotDefeated of the last defeat). */
	DECLARE_MULTICAST_DELEGATE(FOnMatchOver);
	FOnMatchOver OnMatchOver;

	/**
	 * Every machine (D357, V1.3b): the defeats, the end or the winners changed, as this machine knows them; a client by
	 * the OnReps (the whole state at once too), the server (listen or solo) at once in DefeatSlot. The end banner and
	 * the spectator listen to it.
	 */
	DECLARE_MULTICAST_DELEGATE(FOnVictoryChanged);
	FOnVictoryChanged OnVictoryChanged;

	/** Bit Slot: that slot is defeated. */
	uint8 GetDefeatedMask() const;
	uint8 GetWinnerMask() const { return WinnerMask; }

	static const TCHAR* GetReasonName(EHOLDefeatReason Reason);

	/**
	 * The configuration of the launch, on every machine (D.3: the selection's allies are the slots of the player's camp,
	 * HOLUnitPicking::GetSide); the default one before InitVictory, or on a client before its replication.
	 */
	const FHOLMatchConfig& GetLaunchConfig() const { return LaunchConfig; }

private:
	UFUNCTION()
	void OnRep_Victory();

	/**
	 * D358: the collapse of a slot just defeated, by the paths of combat: its units killed (UHOLUnitSubsystem::
	 * KillUnitsOfOwner: their fall and bodies as usual), its buildings, sites and foundations destroyed
	 * (UHOLConstructionSubsystem::DestroyBuilding: rubble, footprint reopened), then its stock at 0 (the refunds of its
	 * queues and sites lost too) and its orders waiting dropped. Logs "Victory: slot N collapsed (U units, B buildings)".
	 */
	void CollapseSlot(int32 Slot);

	/**
	 * The configuration of the launch: its occupied slots and teams make the camps (D357). Written by the server alone;
	 * replicated (D.3) so that each client knows its allies.
	 */
	UPROPERTY(Replicated)
	FHOLMatchConfig LaunchConfig;

	/** One per slot (MaxSlots). */
	UPROPERTY(ReplicatedUsing = OnRep_Victory)
	TArray<EHOLDefeatReason> SlotDefeats;

	UPROPERTY(ReplicatedUsing = OnRep_Victory)
	bool bMatchOver = false;

	/** Bit Slot: that slot won (FHOLVictoryResult). */
	UPROPERTY(ReplicatedUsing = OnRep_Victory)
	uint8 WinnerMask = 0;
};
