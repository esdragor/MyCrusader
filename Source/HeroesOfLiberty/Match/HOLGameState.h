// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLMarketRules.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLRelayView.h"
#include "Match/HOLVictoryRules.h"
#include "Match/HOLVolcanoView.h"
#include "Match/HOLSiteCount.h"
#include "Net/HOLPauseRules.h"
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
	/** S10 (D431, D703): another camp held every sacred site for the whole count. */
	SacredSites,
	/** N2 (D354, Nomad option): its last Peasant died before its first Forum was complete (HOLNomad::IsDefeated). */
	Nomad,
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
		double WindowSeconds = FHOLDisconnectRules::DefaultWindowSeconds, const FHOLTechnologyState& Technologies = FHOLTechnologyState());
	/**
	 * R7.2, server only, by the game mode's PostLogin: the stock of the player taking his slot, the one kept at his
	 * departure if he comes back within the window (the slot no longer absent), else Starting. True when he came back
	 * (M6-Q15: the game mode tells the manager, "back").
	 */
	bool GiveSlotStock(AHOLPlayerState& Taking, const FHOLResources& Starting);
	/** The same at the server time Now (the tests): the return noticed at Now (R7.5's « de retour »). */
	bool GiveSlotStock(AHOLPlayerState& Taking, const FHOLResources& Starting, double Now);
	/**
	 * T9, server or solo, development (hol.Net.FakeAbsent Back <Slot>): the return of the player of an absent slot without a second client.
	 * A player state of his name (no connection) takes the seat as a PostLogin does (GiveSlotStock: the stock as the AI left it, the AI lets
	 * go, "de retour" in the banner). False when nobody is away on that slot.
	 */
	bool SimulateReturn(int32 Slot, const FHOLResources& Starting, double Now);
	/**
	 * R7.3, server only, by the game mode each second: the absent slots whose window has run out are defeated
	 * (reason Disconnected), as by a surrender; those defeated returned. R7.5: the notices of a return older than
	 * BackNoticeSeconds dropped too.
	 */
	TArray<int32> DefeatExpiredAbsences(double Now, double WindowSeconds);
	bool IsSlotAbsent(int32 Slot) const { return FHOLDisconnectRules::FindAbsent(AbsentSlots, Slot) != INDEX_NONE; }
	/**
	 * D717 (T2): the player state of the AI that holds the seat of a player who left (a bot on a seat the launch configuration gave to
	 * a human), or null. It holds his stock and technologies, so the orders of the brain pass the same queues and are paid on it.
	 */
	static AHOLPlayerState* FindHeldBot(const UWorld& World, int32 Slot);
	/**
	 * The slots in use by player states, Except left out (PostLogin's TakenSlots): the player state of the AI that holds a human's seat
	 * does not count, else his own return (or a second connection of his name) would find his seat taken and join as a spectator.
	 */
	TArray<int32> GetSlotsInUse(const AHOLPlayerState* Except) const;
	/**
	 * B6b (R7), server only, by the game mode's PreLogin and PostLogin: the slot a login of Name takes
	 * (FHOLMatchConfig::FindSlotForName), a defeated slot never (its window past, or beaten otherwise): he watches, as a
	 * name absent from the configuration. With bLog, logs "Disconnect: slot N (Name) defeated, joins as spectator".
	 */
	int32 FindSlotOnLogin(const FHOLMatchConfig& Config, const FString& Name, TConstArrayView<int32> TakenSlots, bool bLog = true) const;
	/**
	 * D717 (T4), server only, by the game mode's PostLogin: the slot a new connection (Logging, no slot yet) of Name takes. As
	 * FindSlotOnLogin on the slots in use (the AI's seat of an absent player left out), but a LIVE player state of another connection
	 * that already holds the seat of that name (he restarted before the old connection timed out) does not make him a spectator: it is
	 * OutReplaced (null otherwise), to be handed over by RetireHolder and kicked. A defeated seat is never taken again.
	 */
	int32 FindSlotForLogin(const FHOLMatchConfig& Config, const FString& Name, const AHOLPlayerState& Logging, AHOLPlayerState*& OutReplaced) const;
	/**
	 * D717 (T4), server only: the old connection's seat handed over to its successor (called before the successor is given the slot,
	 * then GiveSlotStock): its stock, technologies and hero sheet are kept for him as at a Logout, with no banner; the old player state
	 * becomes a spectator without a seat, so its late Logout (KeepAbsentSlot) puts nobody "away" under the new player. The orders,
	 * building queues and units stay (they belong to the slot).
	 */
	void RetireHolder(AHOLPlayerState& Old, double Now, double WindowSeconds);
	/** A player state of a connection that is still there (not the AI's, not a spectator) holding Slot, Except left out; null: none. */
	static AHOLPlayerState* FindLiveHolder(const UWorld& World, int32 Slot, const AHOLPlayerState* Except);

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
	/**
	 * D719 (T0): the technologies of slot Slot, its player's (AHOLPlayerState) or, while he is away, the ones kept for him
	 * (AbsentSlots). False (OutTechnologies empty): the slot has none (no player state, not absent).
	 */
	static bool GetSlotTechnologies(const UWorld& World, int32 Slot, FHOLTechnologyState& OutTechnologies);
	/**
	 * D719 (T0), server only: a research of slot Slot ended: added where GetSlotTechnologies reads (the player state, with its
	 * effects, or the absent's record, given back on his return). False: nowhere, or already had it (nothing written).
	 */
	static bool AddSlotTechnology(UWorld& World, int32 Slot, FName Technology);
	/** Server only: the slots kept for their players gone (R7.5's banner will replicate them). */
	const TArray<FHOLAbsentSlot>& GetAbsentSlots() const { return AbsentSlots; }

	/** R7.5, every machine (replicated): the players gone and those just back, for the banner. */
	const TArray<FHOLAbsentNotice>& GetAbsentNotices() const { return AbsentNotices; }

	/**
	 * R5: the neutral relays as every machine knows them (position, zone, owner, taker, share of the capture); the server writes them
	 * (UHOLRelaySubsystem, each step, only when they changed), the markers draw the rings from them.
	 */
	const TArray<FHOLRelayView>& GetRelayViews() const { return RelayViews; }
	/** Server or solo only. */
	void SetRelayViews(const TArray<FHOLRelayView>& Views);

	/**
	 * V9a: the volcano sites as every machine knows them (phase, flow drawn, front, crater, altar); the server writes them
	 * (UHOLVolcanoSubsystem, each step, only when they changed), a client reads them: the map's site is not replicated.
	 */
	const TArray<FHOLVolcanoSiteNet>& GetVolcanoViews() const { return VolcanoViews; }
	/** Server or solo only. */
	void SetVolcanoViews(const TArray<FHOLVolcanoSiteNet>& Views);

	/**
	 * S10 (D431): the sacred sites' victory count as every machine knows it (camp in the lead, seconds left, frozen, winner). Public, shown to all;
	 * the server writes it (UHOLSiteSubsystem::StepVictory), a client reads it.
	 */
	const FHOLSiteCountNet& GetSiteCount() const { return SiteCount; }
	/** Server or solo only. */
	void SetSiteCount(const FHOLSiteCountNet& Count);
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

	/** T8 (D722): bit Slot: that slot's player is away, the window running (read from the replicated notices, on every machine). */
	uint8 GetAbsentMask() const;
	/** T8 (D722): bit Slot: the window is over and the AI keeps the seat (replicated). */
	uint8 GetAIHeldMask() const { return AIHeldMask; }
	/** T9: one line per occupied human seat, as the server reads it (hol.Disconnect.Report). */
	TArray<FString> DescribeDisconnections(double Now, double WindowSeconds) const;
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
	 * F6e.4 (D353): the camp of ViewerSlot has some cell of ForumSlot's main forum in sight, on this machine (a client's own fog, the
	 * server's or the solo game's for the slot). False without a fog (Sans brouillard: the alerts stay those of the camps), or for a
	 * spectator.
	 */
	bool DoesViewerSeeForum(int32 ViewerSlot, int32 ForumSlot) const;

	/**
	 * F6e: the fog's mode of the match (EHOLFogMode as a number), decided by the server when its fog starts and replicated, so a
	 * client computes its own fog with it (D525); below 0 while it is not known. Until F8 carries it in the configuration.
	 */
	int32 GetFogMode() const { return FogMode; }
	void SetFogMode(const int32 Mode) { FogMode = static_cast<int8>(Mode); }

	/**
	 * V2 (D349-D353): each slot's main forum, public (every machine, the players' table of V2.4): its falls, its phase,
	 * its ruin's steps left, its rebuilding's work. The default (standing, no fall) for a closed slot.
	 */
	const FHOLForumState& GetForumState(int32 Slot) const;
	/** Lives left of a slot's main forum (D349: 1 + tier - falls; 0 once it fell for good). */
	int32 GetLives(int32 Slot) const;
	/** P3: the civilisation tier the slot's hero level opens (HOLHeroXP::GetTier; 0 without a hero), as the HUD and the rules read it. */
	int32 GetTier(int32 Slot) const;
	/** G0 (D710), development: forces the slot's tier (0 to 3, whatever its hero's level; GetTier then returns it); Tier below 0 lifts it. Server side, replicated (hol.Cheat.Tier). */
	void SetTierOverride(int32 Slot, int32 Tier);
	/** The tier forced for the slot, -1 when none (every machine: the property is replicated). */
	int32 GetTierOverride(int32 Slot) const;
	/** P2: the forum's values for a slot: the tier its hero's level opens (HOLHeroXP::GetTier), at least the setting's (tests). */
	FHOLForumTuning GetForumTuning(int32 Slot) const;
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
	 * N2 (D354): Slot has PeasantCount Peasants left; a Nomad seat with none before its first complete Forum is defeated at once
	 * (EHOLDefeatReason::Nomad, HOLNomad::IsDefeated read on the script board). A standard seat is never defeated here. True when defeated.
	 */
	bool HandleNomadPeasants(int32 Slot, int32 PeasantCount);

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

	/** G0 (D710): one per slot (MaxSlots), the tier forced for a slot by hol.Cheat.Tier, -1 for none; public, so a client reads the same tier. */
	UPROPERTY(Replicated)
	TArray<int32> TierOverrides;

	/** Server only (R7): the slots kept for their players gone, in the order of the departures. */
	TArray<FHOLAbsentSlot> AbsentSlots;
	/** D717 (T2), server only: the AI's player state for the seat just kept (KeepMissingSlot), with his name, stock and technologies. */
	void SpawnHeldBot(int32 Slot, const FString& Name, const FHOLResources& Resources, const FHOLTechnologyState& Technologies);
	/**
	 * R7.5: what the banner shows of AbsentSlots, replicated (a state rather than a multicast: a client joining or coming
	 * back late sees the players gone and those just back as the others do).
	 */
	UPROPERTY(Replicated)
	TArray<FHOLAbsentNotice> AbsentNotices;
	/** T8 (D722): bit Slot: away, the window over, the AI keeps the seat to the end (set with bWindowOver, cleared on the return or the defeat). */
	UPROPERTY(Replicated)
	uint8 AIHeldMask = 0;

	/** R5: the neutral relays, public. */
	UPROPERTY(Replicated)
	TArray<FHOLRelayView> RelayViews;

	/** V9a: the volcano sites, public. */
	UPROPERTY(Replicated)
	TArray<FHOLVolcanoSiteNet> VolcanoViews;

	/** S10: the sacred sites' victory count, public. */
	UPROPERTY(Replicated)
	FHOLSiteCountNet SiteCount;
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

	/** F6e: see GetFogMode. */
	UPROPERTY(Replicated)
	int8 FogMode = -1;

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
	 * D372 amended (TB-Q1): a tribute applied (server, ApplyTribute): the message for its RECEIVER alone (HOLForumMessages::GetTributeMessage),
	 * the receiver's slot first. The HUD of the machine whose player is that slot shows it in its thread; nobody else is told (not the giver).
	 * Logs "Tribute message: <text> (slot N)". Delivery to a remote client's machine (a client RPC to the receiver) is not made yet.
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTributeMessage, int32 /*ReceiverSlot*/, const FHOLForumMessage& /*Message*/);
	FOnTributeMessage OnTributeMessage;

	/** Server: a tribute from GiverSlot to ReceiverSlot arrived (Arrives after the tax Tax) of Resource (an EHOLResource's value). */
	void NoteTributeReceived(int32 GiverSlot, int32 ReceiverSlot, int32 Resource, int32 Arrives, int32 Tax);

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

public:
	/**
	 * M4c (Market, GDD 5.5, D84): the one list of prices every player trades at, allies and enemies alike; set by the server alone
	 * (the exchange order, the way back to rest), replicated to everyone. Resources by index: food, wood, stone, gold.
	 */
	HOLMarket::FBook GetMarketBook() const;
	/** Whether the book has been set (a match started); until then GetMarketBook gives a book of 100s that nothing should trade at. */
	bool HasMarketBook() const { return MarketPrices.Num() == HOLMarket::BookSize; }
	/** Server only. */
	void SetMarketBook(const HOLMarket::FBook& InBook);
	/** Server only, at the start of the match: every price at its base (the Standard row of /Game/Match/MarketRates). */
	void ResetMarketBook();

private:
	/** The four prices of the book (HOLMarket::FBook): replicated to everyone; empty until the match starts. */
	UPROPERTY(Replicated)
	TArray<int32> MarketPrices;

public:
	/**
	 * D5 of DiplomacyInProgress.md (GDD 14.3, D369 to D376): the stances of the match, on every machine. In the Diplomacy mode the
	 * server owns them and replicates them (DiplomacyStances / DiplomacyVision: the stance of From toward To at From * MaxSlots + To);
	 * in the other modes the book is only the fixed teams of LaunchConfig (locked, nothing replicated). The AIs and the closed slots
	 * come from LaunchConfig, so both sides build the same book.
	 */
	bool IsDiplomacyMode() const { return LaunchConfig.Mode == EHOLMatchMode::Diplomacy; }
	HOLDiplomacy::FBook GetDiplomacyBook() const;
	/** The relation of a pair on this machine (HOLDiplomacy::Relation of the book). */
	int32 GetDiplomacyRelation(int32 A, int32 B) const;
	/**
	 * Server only: From says Stance toward To (HOLDiplomacy::SetStance; refused for a closed slot, outside the Diplomacy mode, an AI...),
	 * the change stored and replicated at once, announced (log "Diplomacy: ...", then OnDiplomacyAnnounced).
	 */
	HOLDiplomacy::FStanceChange ApplyStance(int32 From, int32 To, int32 Stance);
	/**
	 * D6 (D373): the end of the match by the camps and, in the Diplomacy mode, by an alliance of all the survivors (at least one player
	 * beaten): over at once, together. Server only; sets bMatchOver / WinnerMask and broadcasts OnMatchOver. Called by each defeat and each
	 * stance change (an alliance formed after the last defeat ends the match at that second). True when the match just ended.
	 */
	bool EvaluateMatchEnd();
	/** Server only: From opens (bShare) or closes its vision to To (HOLDiplomacy::SetVision). */
	HOLDiplomacy::FVisionChange ApplyVision(int32 From, int32 To, bool bShare);
	/** The log of this machine's view: one line per pair of occupied slots ("Diplomacy report: ..."), for the scenarios and quick_check. */
	void ReportDiplomacy() const;
	/** Server: a stance really changed (the line told to all, D369). The clients learn it by OnDiplomacyChanged. */
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDiplomacyAnnounced, const HOLDiplomacy::FStanceChange& /*Change*/);
	FOnDiplomacyAnnounced OnDiplomacyAnnounced;
	/** Every machine: the stances or the shared visions changed (server at once, a client by OnRep). */
	DECLARE_MULTICAST_DELEGATE(FOnDiplomacyChanged);
	FOnDiplomacyChanged OnDiplomacyChanged;

private:
	/** Server, at the launch: every stance at HOLDiplomacy::DefaultStance (the DQ-1 provisional) in the Diplomacy mode, nothing otherwise. */
	void ResetDiplomacy();
	/** Server: the orders waiting (UHOLCommandSubsystem::ConsumeDiplomacyOrders), at the start of a step. */
	void ApplyDiplomacyOrders();
	void StoreDiplomacyBook(const HOLDiplomacy::FBook& Book);
	UFUNCTION()
	void OnRep_Diplomacy();

	/** Diplomacy mode only, MaxSlots * MaxSlots once the match started: the stance of From toward To (HOLDiplomacy::Enemy / Neutral / Ally). */
	UPROPERTY(ReplicatedUsing = OnRep_Diplomacy)
	TArray<int8> DiplomacyStances;

	/** Same indexing: 1 when From lets To see what it sees (D371). */
	UPROPERTY(ReplicatedUsing = OnRep_Diplomacy)
	TArray<uint8> DiplomacyVision;

public:
	/**
	 * P5 (D513), every machine: the match is paused (FHOLPauseRules): no simulation step and no order, as while waiting
	 * for the players (UHOLSimulationSubsystem, UHOLCommandSubsystem::IsRefused read it).
	 */
	bool IsMatchPaused() const { return PausedBySlot != INDEX_NONE; }
	int32 GetPausedBySlot() const { return PausedBySlot; }
	const FString& GetPausedByName() const { return PausedByName; }
	int32 GetPausesLeft() const { return PausesLeft; }
	/** Server world time of the pause (the timer's start). */
	double GetPausedAtSeconds() const { return PausedAtSeconds; }
	/**
	 * The pause's timer at the server time Now (GetServerWorldTimeSeconds): seconds left, 0 once over (anyone resumes). A
	 * client counts it in real time from the pause's arrival (its world is paused meanwhile: its clock stands still).
	 */
	double GetPauseTimerLeft(double Now) const;
	/** Solo pauses freely (no limit, no timer). */
	bool IsPauseFree() const { return GetNetMode() == NM_Standalone; }

	/**
	 * P5, server only, by the controller's ServerRequestPause / ServerRequestResume: the rule decides (a player still in
	 * the match, a pause left, the timer). Logs "Pause: slot N (Name) paused, P left", "Pause: resumed by Name (slot N)"
	 * or "Pause: refused to Name (slot N): <reason>". True when done.
	 */
	bool RequestPause(const AHOLPlayerState& Requester, double Now);
	bool RequestResume(const AHOLPlayerState& Requester, double Now);
	/** D720 (T6), server only: the clock of the disconnection windows: Now less the time the match was paused (the window does not run during a pause). */
	double GetWindowClock(const double Now) const { return Now - WindowPausedSeconds - (IsMatchPaused() ? FMath::Max(Now - PausedAtSeconds, 0.) : 0.); }
	/** D720 (T6), server only: Slot leaves; if he paused the match, it resumes (his pause stays counted). True when lifted. */
	bool LiftPauseOf(int32 Slot, double Now);

	/** Every machine: paused, resumed or the count changed (server at once, a client by OnRep). For the pause screen. */
	DECLARE_MULTICAST_DELEGATE(FOnPauseChanged);
	FOnPauseChanged OnPauseChanged;

private:
	UFUNCTION()
	void OnRep_Pause();

	/**
	 * Every machine, on each change: a displaying one (solo, client) pauses its own world while the match is paused (the
	 * engine's pause, local: its clock stops, so do the units' animations, the falls, the arrows and the bodies, all drawn
	 * on it); the dedicated server keeps its clock (the timer). Then OnPauseChanged.
	 */
	void NotifyPauseChanged();

	/** A client: FPlatformTime::Seconds() at which its timer started (the pause's arrival less what had already run). */
	double PauseRealStartSeconds = 0.;
	/** This machine's world paused for the match's pause (a change applied once: each replicated property calls OnRep_Pause). */
	bool bLocalPauseApplied = false;

	/** A slot still in the match (not defeated, the match not over): a player who may pause or resume. */
	bool IsInMatch(const AHOLPlayerState& Requester) const;

	/** P5 (D513): shared by every player of the match; set from hol.Match.PausesPerMatch at InitVictory. */
	UPROPERTY(ReplicatedUsing = OnRep_Pause)
	int32 PausesLeft = FHOLPauseRules::DefaultPausesPerMatch;

	/** INDEX_NONE while the match runs. */
	UPROPERTY(ReplicatedUsing = OnRep_Pause)
	int32 PausedBySlot = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_Pause)
	FString PausedByName;

	/** Server world time (GetServerWorldTimeSeconds) of the pause: the timer's start. */
	UPROPERTY(ReplicatedUsing = OnRep_Pause)
	double PausedAtSeconds = 0.;
	/** D720 (T6): the seconds of the pauses already ended (taken off the clock of the disconnection windows). Replicated: the clients' banner counts on the same clock. */
	UPROPERTY(Replicated)
	double WindowPausedSeconds = 0.;
};
