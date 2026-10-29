// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Simulation/HOLHeroSheet.h"
#include "HOLGameState.generated.h"

class AHOLBuilding;
class AHOLPlayerState;
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
	/** V5.2 (Regicide, D355): its King died. */
	King,
	/** R7.3 (R-Q5, D71): its player left and did not come back within the window. */
	Disconnected,
};

/**
 * R7.5: a slot kept for its player gone, as every machine sees it (the disconnection banner); its stock stays on the
 * server (FHOLAbsentSlot). Kept BackNoticeSeconds after a return, for « Bob : de retour ».
 */
USTRUCT()
struct FHOLAbsentNotice
{
	GENERATED_BODY()

	/** INDEX_NONE: a fake one (hol.Net.FakeAbsent). */
	UPROPERTY()
	int32 Slot = INDEX_NONE;

	UPROPERTY()
	FString Name;

	/** Server world time (GetServerWorldTimeSeconds) of the window's end: the defeat if he is not back. */
	UPROPERTY()
	double EndsAt = 0.;

	/** Server world time of his return; 0 while away. */
	UPROPERTY()
	double BackAt = 0.;

	/** The window's length: the banner never shows more (a client's clock of the server can be a little behind). */
	UPROPERTY()
	double WindowSeconds = 0.;
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

	/**
	 * R3.9-R3.10: a match launched by the lobby manager waits for its named players; meanwhile no simulation step and
	 * no order (the one guard: UHOLSimulationSubsystem and UHOLCommandSubsystem::IsRefused read it).
	 */
	bool IsWaitingForPlayers() const { return bWaitingForPlayers; }
	/** The named players still missing while waiting (the configuration's names). */
	const TArray<FString>& GetMissingPlayers() const { return MissingPlayers; }
	/** The server's world time (GetServerWorldTimeSeconds) when the wait ends at the latest: the banner's countdown. */
	double GetWaitEndSeconds() const { return WaitEndSeconds; }
	/** Server only, by the game mode. */
	void SetWaitingForPlayers(bool bWaiting, const TArray<FString>& Missing, double EndSeconds = 0.);

	/**
	 * R7.2 (R-Q5), server only, by the game mode's Logout: the leaving player's slot kept for the window
	 * (FHOLDisconnectRules::KeepsSlot), its stock remembered; its units and buildings stay in the world. False: not kept.
	 * Logs "Disconnect: slot N (Name) kept, window W s".
	 */
	bool KeepAbsentSlot(const AHOLPlayerState& Leaving, double Now, double WindowSeconds = FHOLDisconnectRules::DefaultWindowSeconds);
	/** R7 (R3.9), server only: a named slot whose player missed the start, kept for the window from Now with Resources. */
	bool KeepMissingSlot(int32 Slot, const FHOLResources& Resources, double Now,
		double WindowSeconds = FHOLDisconnectRules::DefaultWindowSeconds);
	/**
	 * R7.2, server only, by the game mode's PostLogin: the stock of the player taking his slot, the one kept at his
	 * departure if he comes back within the window (the slot no longer absent), else Starting. True when he came back
	 * (M6-Q15: the game mode tells the manager, "back").
	 */
	bool GiveSlotStock(AHOLPlayerState& Taking, const FHOLResources& Starting);
	/** The same at the server time Now (the tests): the return noticed at Now (R7.5's « de retour »). */
	bool GiveSlotStock(AHOLPlayerState& Taking, const FHOLResources& Starting, double Now);
	/**
	 * R7.3, server only, by the game mode each second: the absent slots whose window has run out are defeated
	 * (reason Disconnected), as by a surrender; those defeated returned. R7.5: the notices of a return older than
	 * BackNoticeSeconds dropped too.
	 */
	TArray<int32> DefeatExpiredAbsences(double Now, double WindowSeconds);
	bool IsSlotAbsent(int32 Slot) const { return FHOLDisconnectRules::FindAbsent(AbsentSlots, Slot) != INDEX_NONE; }
	/**
	 * B6b (R7), server only, by the game mode's PreLogin and PostLogin: the slot a login of Name takes
	 * (FHOLMatchConfig::FindSlotForName), a defeated slot never (its window past, or beaten otherwise): he watches, as a
	 * name absent from the configuration. With bLog, logs "Disconnect: slot N (Name) defeated, joins as spectator".
	 */
	int32 FindSlotOnLogin(const FHOLMatchConfig& Config, const FString& Name, TConstArrayView<int32> TakenSlots, bool bLog = true) const;

	/**
	 * M6-Q16 (R7): the stock of slot Slot, its player's (AHOLPlayerState) or, while he is away, the one kept for him
	 * (AbsentSlots): what his Peasants unload and what comes back to him (refunds) goes there and is his on his return.
	 * False (OutStock zero): the slot has none (no player state, not absent).
	 */
	static bool GetSlotStock(const UWorld& World, int32 Slot, FHOLResources& OutStock);
	/** Server only: writes slot Slot's stock where GetSlotStock reads it. False: nowhere (nothing written). */
	static bool SetSlotStock(UWorld& World, int32 Slot, const FHOLResources& Stock);
	/** Server only: adds Gain to slot Slot's stock (a deposit, a refund). False: nowhere (Gain lost); OutStock: after it. */
	static bool AddSlotStock(UWorld& World, int32 Slot, const FHOLResources& Gain, FHOLResources& OutStock);
	/** Server only: the slots kept for their players gone (R7.5's banner will replicate them). */
	const TArray<FHOLAbsentSlot>& GetAbsentSlots() const { return AbsentSlots; }

	/** R7.5, every machine (replicated): the players gone and those just back, for the banner. */
	const TArray<FHOLAbsentNotice>& GetAbsentNotices() const { return AbsentNotices; }
	/**
	 * R7.5: the banner's lines at the server time Now (GetServerWorldTimeSeconds), one per notice, in the order of the
	 * departures: « Bob : connexion perdue — 2:41 » while away, « Bob : de retour » BackNoticeSeconds after his return.
	 */
	TArray<FText> GetAbsenceBannerLines(double Now) const;
	/**
	 * Server or solo, development (hol.Net.FakeAbsent): a fake notice for Name, SecondsLeft before its end (no slot, no
	 * defeat); SecondsLeft < 0: Name back at once. An empty Name clears the fake ones.
	 */
	void SetFakeAbsentNotice(const FString& Name, double SecondsLeft);

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
	 * TC3 (TC-Q6): the place on the map (AHOLStartSlot's index) of a slot, every machine, from the configuration of the
	 * launch (FHOLMapSizeRules::GetPlaces: fixed, or drawn from its seed): INDEX_NONE for a closed slot or a configuration
	 * refused (FHOLMapSizeRules::CheckPlaces); the slot itself while the configuration is not known yet (HasStartPlaces).
	 */
	int32 GetStartPlace(int32 Slot) const;

	/** TC3: the places are known (the server from InitVictory on, a client once the configuration has come). */
	bool HasStartPlaces() const { return !StartPlaces.IsEmpty(); }

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
	 * V5.2, server only (Regicide, D355), by the combat processor when a King starts dying (UHOLCombatProcessor::KillUnit):
	 * the slot's King died (true; false: a slot already defeated, its death already counted, out of the match, or the match
	 * over: the collapse of a defeated slot kills its King without a second defeat). A KingDied event sent at once, the slot
	 * defeated (reason King) at the end of the step, as the forum's last fall. A slot without King never loses this way.
	 * Logs "Regicide: king of slot N died".
	 */
	bool HandleKingDeath(int32 Slot);

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

	/**
	 * V4.1 (V4-Q1): every machine (players and spectators) hears each event of the ruin once (fell, hold started, razed,
	 * defeated), from MulticastForumEvent; the thread of messages of V4.2 and the alert of V4.3 listen to it
	 * (HOLForumMessages::GetMessage for its text and its alert).
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnForumEvent, const FHOLForumEvent& /*Event*/);
	FOnForumEvent OnForumEvent;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void OnRep_Forums(const TArray<FHOLForumState>& OldForumStates);

	void OnStepStarted(int64 StepIndex);

	/** V4.1, server only: sends Event to every machine (MulticastForumEvent), at once (reliable, once). */
	void SendForumEvent(const FHOLForumEvent& Event);

	/**
	 * V4.1: every machine, the server included (listen or solo): logs "Message: <text> (alert)" as its own player sees it
	 * (none on a dedicated server), then OnForumEvent. The game state is always relevant: every client receives it.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastForumEvent(const FHOLForumEvent& Event);
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

	/** Server only (R7): the slots kept for their players gone, in the order of the departures. */
	TArray<FHOLAbsentSlot> AbsentSlots;
	/**
	 * R7.5: what the banner shows of AbsentSlots, replicated (a state rather than a multicast: a client joining or coming
	 * back late sees the players gone and those just back as the others do).
	 */
	UPROPERTY(Replicated)
	TArray<FHOLAbsentNotice> AbsentNotices;
	/** The stock kept for slot Slot while its player is away; nullptr: not absent (or no game state of ours). */
	static FHOLResources* FindAbsentStock(const UWorld& World, int32 Slot);

	/** Server only: Peasants rebuilding each slot's forum (V2.2). */
	TArray<int32> ForumBuilders;

	/** Server only (V3.1): the presence in each forum's zone, fixed by the cheat or the tests. */
	TArray<TOptional<FHOLForumPresence>> ForumPresenceOverrides;
	FForumPresenceProvider ForumPresenceProvider;
	TSharedPtr<FHOLForumPresenceCounter> ForumPresenceCounter;
	int64 CurrentStep = -1;

	/** Server only, bit Slot: its forum fell with no life left, its defeat waits for the end of the step. */
	uint8 PendingForumDefeats = 0;
	/** Server only, bit Slot: its King died (V5.2), its defeat waits for the end of the step. */
	uint8 PendingKingDefeats = 0;

	/** Server only (V4.2): per slot's forum, a bit per attacker whose hold was announced (HOLForumMessages::ShouldAnnounceHold). */
	uint8 ForumHoldAnnounced[FHOLMatchConfig::MaxSlots] = {};

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
	UPROPERTY(ReplicatedUsing = OnRep_LaunchConfig)
	FHOLMatchConfig LaunchConfig;

	/** TC3: each slot's place, from LaunchConfig (UpdateStartPlaces), MaxSlots of them once known. */
	TArray<int32> StartPlaces;

	/** TC3: StartPlaces from LaunchConfig; a configuration refused gives no place to anyone (logged on the server). */
	void UpdateStartPlaces();

	UFUNCTION()
	void OnRep_LaunchConfig();

	/** One per slot (MaxSlots). */
	UPROPERTY(ReplicatedUsing = OnRep_Victory)
	TArray<EHOLDefeatReason> SlotDefeats;

	UPROPERTY(ReplicatedUsing = OnRep_Victory)
	bool bMatchOver = false;

	/** R3.10: replicated for the waiting banner (B7). */
	UPROPERTY(Replicated)
	bool bWaitingForPlayers = false;

	UPROPERTY(Replicated)
	TArray<FString> MissingPlayers;

	UPROPERTY(Replicated)
	double WaitEndSeconds = 0.;

	/** Bit Slot: that slot won (FHOLVictoryResult). */
	UPROPERTY(ReplicatedUsing = OnRep_Victory)
	uint8 WinnerMask = 0;

public:
	/**
	 * J8a.3 (J8-Q5, J8a3-Q1 = A): the public part of each slot's hero sheet (level, exact HP, state), on every machine; a
	 * copy of the true sheet the server keeps. The default (None) for a slot without hero, or before its replication.
	 */
	const FHOLHeroSheet& GetHeroSheet(int32 Slot) const;
	/** Server only: true when the slot's sheet changed (and is then sent at once); false when equal or out of range. */
	bool SetHeroSheet(int32 Slot, const FHOLHeroSheet& Sheet);

	/** Every machine: a hero sheet changed (server at once, a client by OnRep). For the HUD (J8a.3 loop 4b). */
	DECLARE_MULTICAST_DELEGATE(FOnHeroSheetsChanged);
	FOnHeroSheetsChanged OnHeroSheetsChanged;

	/**
	 * J8a.4 (D397, D202): every machine with a player, its own hero ready to return (its public sheet turned
	 * ReadyToReturn, FHOLHeroReadyWatch): once per death, its owner alone. Logs "Hero message: <text> (slot N)", then this,
	 * for the HUD's thread (a minor alert: a line, no banner nor sound).
	 */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeroReadyMessage, const FHOLForumMessage& /*Message*/);
	FOnHeroReadyMessage OnHeroReadyMessage;

	/**
	 * J8b.4 (J8b-Q4): every machine with a player, a slot's hero gone up from OldLevel to NewLevel in its public sheet
	 * (FHOLHeroLevelWatch: not the first level seen). Logs "Hero level up: slot N level A -> B", then this: his golden flash
	 * for all, the alerts for his owner (the HUD).
	 */
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnHeroLevelUp, int32 /*Slot*/, int32 /*OldLevel*/, int32 /*NewLevel*/);
	FOnHeroLevelUp OnHeroLevelUp;

private:
	UFUNCTION()
	void OnRep_HeroSheets();

	/** J8a.4: before OnHeroSheetsChanged, the local player's hero ready (OnHeroReadyMessage). */
	void NoteHeroReady();
	FHOLHeroReadyWatch HeroReadyWatch;

	/** J8b.4: before OnHeroSheetsChanged, the heroes gone up (OnHeroLevelUp). */
	void NoteHeroLevels();
	FHOLHeroLevelWatch HeroLevelWatch;

	/** One per slot (MaxSlots), public (J8-Q5). */
	UPROPERTY(ReplicatedUsing = OnRep_HeroSheets)
	TArray<FHOLHeroSheet> HeroSheets;
};
