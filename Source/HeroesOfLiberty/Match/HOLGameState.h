// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "HOLGameState.generated.h"

class AHOLBuilding;
class FHOLForumPresenceCounter;

/** Why a slot lost (D356; the dead King comes with Regicide). None: still in the match. */
UENUM()
enum class EHOLDefeatReason : uint8
{
	None,
	Surrender,
	Cheat,
	/** V2 (D349): its main forum fell with no life left. */
	Forum,
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

	/**
	 * V2 (D349-D353): each slot's main forum, public (every machine, the players' table of V2.4): its falls, its phase,
	 * its ruin's steps left, its rebuilding's work. The default (standing, no fall) for a closed slot.
	 */
	const FHOLForumState& GetForumState(int32 Slot) const;
	/** Lives left of a slot's main forum (D349: 1 + tier - falls; 0 once it fell for good). */
	int32 GetLives(int32 Slot) const;
	/** The slot's main forum is in ruin or rebuilding: unbreakable, no production, no deposit (D351). */
	bool IsForumDown(int32 Slot) const { return GetForumState(Slot).IsDown(); }
	/** Share of its rebuilding done (0 to 1; 1 standing). */
	float GetForumRebuildProgress(int32 Slot) const;
	/** V2.3a: a main forum is in ruin, its Peasants waiting without striking (HOLForumRules::BuildersWait): the drawing looks. */
	bool AnyForumBuildersWait() const;

	/**
	 * Server only, by UHOLConstructionSubsystem::DamageBuilding when a main forum (AHOLBuilding::IsPrimaryForum) reaches 0
	 * health: it falls instead of being destroyed (true; false: not a main forum standing, of a slot still in the match,
	 * the caller destroys it). A life less (HOLForumRules::Fall), in ruin, its production queue refunded in full as on a
	 * destruction, its rally point kept, the population it gives kept. With no life left, the slot is defeated (reason
	 * Forum) at the end of the step: the damage can come from a Mass processor, whose entities the collapse must not
	 * touch while they are processed. Logs "Victory: forum of slot N fell, lives L".
	 */
	bool HandleForumFall(AHOLBuilding& Forum);

	/**
	 * Server only, cheat (hol.Victory.SkipRuin): the slot's forum leaves its phase at once (ruin -> rebuilding, rebuilding ->
	 * rebuilt). False when it stands.
	 */
	bool SkipForumPhase(int32 Slot);

	/** Server only (V2.2): the Peasants rebuilding the slot's forum this step (D312; nothing during the ruin). */
	void SetForumBuilders(int32 Slot, int32 Builders);

	/**
	 * V3.1: who stands in the zone of a slot's forum down, asked by the server each step (HOLForumRules::Step). Replaceable:
	 * V3.2 gives the units counted by team; none given, nobody (the rule then lets the hold recede).
	 */
	using FForumPresenceProvider = TFunction<FHOLForumPresence(const AHOLGameState& GameState, int32 Slot)>;
	void SetForumPresenceProvider(FForumPresenceProvider Provider) { ForumPresenceProvider = MoveTemp(Provider); }

	/**
	 * Server only, cheat and tests (hol.Victory.HoldPresence): the slot's presence fixed, over the provider's, until cleared.
	 * With attackers and no AttackerSlot, the first slot of another camp holds.
	 */
	void SetForumPresenceOverride(int32 Slot, const FHOLForumPresence& Presence);
	void ClearForumPresenceOverride(int32 Slot);

	/**
	 * V3.2: the units counted by team, the provider InitVictory gives on the server (FHOLForumPresenceCounter); its cost
	 * (walks, units walked). Null on a client.
	 */
	const FHOLForumPresenceCounter* GetForumPresenceCounter() const { return ForumPresenceCounter.Get(); }

	/** Server: the simulation step under way (its start), -1 before the first. */
	int64 GetCurrentStep() const { return CurrentStep; }

	/** Server: the presence the rule gets for the slot this step (the override, else the provider, else nobody). */
	FHOLForumPresence GetForumPresence(int32 Slot) const;

	/** Server only: forum phases and the deferred defeats follow the simulation's steps from InitVictory on. */
	void AdvanceForums();
	void ApplyPendingDefeats();

	/** The slot's main forum in World (AHOLBuilding::IsPrimaryForum), null if none (destroyed, never placed). */
	static AHOLBuilding* FindPrimaryForum(const UWorld& World, int32 Slot);

	/** Every machine: a forum's state changed (server at once, a client by OnRep). For the HUD of V2.3, V2.4. */
	DECLARE_MULTICAST_DELEGATE(FOnForumsChanged);
	FOnForumsChanged OnForumsChanged;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnRep_Forums(const TArray<FHOLForumState>& OldForumStates);

	void OnStepStarted(int64 StepIndex);
	void OnStepEnded(int64 StepIndex);
	void UnbindSteps();
	void SetForumState(int32 Slot, const FHOLForumState& State);
	/**
	 * V2.3a: the main forums' look follows their state (rubble, a site rising, itself), on every machine that draws, at the
	 * frame after (AHOLBuilding::NotifyForumChanged, D325).
	 */
	void RefreshForumLooks() const;

	/** One per slot (MaxSlots), public (D353). */
	UPROPERTY(ReplicatedUsing = OnRep_Forums)
	TArray<FHOLForumState> ForumStates;

	/** Server only: Peasants rebuilding each slot's forum (V2.2). */
	TArray<int32> ForumBuilders;

	/** Server only (V3.1): the presence in each forum's zone, fixed by the cheat or the tests. */
	TArray<TOptional<FHOLForumPresence>> ForumPresenceOverrides;
	FForumPresenceProvider ForumPresenceProvider;
	TSharedPtr<FHOLForumPresenceCounter> ForumPresenceCounter;
	int64 CurrentStep = -1;

	/** Server only, bit Slot: its forum fell with no life left, its defeat waits for the end of the step. */
	uint8 PendingForumDefeats = 0;

	FDelegateHandle StepStartedHandle;
	FDelegateHandle StepEndedHandle;

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
