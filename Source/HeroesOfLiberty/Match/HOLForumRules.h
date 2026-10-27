// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "HOLForumRules.generated.h"

/** Where a slot's main forum stands (V2, D350-D352). */
UENUM()
enum class EHOLForumPhase : uint8
{
	/** Up: it serves its owner as any finished building. */
	Standing,
	/** Fallen: rubble for ForumRuinTime, unbreakable and inert (no production, no deposit), nothing rebuilds it yet. */
	Ruin,
	/** Rising again: unbreakable and inert until its work is done, then Standing with all its health. */
	Rebuilding,
};

/** V3 (D348, D352): what the hold of a forum down does this step (the ring of V3.3 follows it). */
UENUM()
enum class EHOLForumHold : uint8
{
	/** Standing, or down with its count at 0 and no attacker. */
	None,
	/** Attackers in the zone, no defender: the count goes up (the ring fills in the holder's colour). */
	Holding,
	/** Attackers and defenders: the count frozen (the ring grey). */
	Contested,
	/** No attacker: the count goes back down (ForumHoldDecay seconds a second) to 0. */
	Receding,
};

/**
 * V3.1: who stands in the zone of a slot's forum down this step, given to the rule (V3.2 counts the units, by team).
 * bAttackers: a living unit of an adversary of the forum's owner (Neutral included, D401); bDefenders: a unit of the owner
 * or an ally. AttackerSlot: the slot whose colour fills the ring, the attacker with the most units in the zone (the lowest
 * slot on a tie); INDEX_NONE if unknown.
 */
struct FHOLForumPresence
{
	bool bAttackers = false;
	bool bDefenders = false;
	int32 AttackerSlot = INDEX_NONE;

	static FHOLForumPresence Attackers(const int32 Slot) { FHOLForumPresence Presence; Presence.bAttackers = true; Presence.AttackerSlot = Slot; return Presence; }
};

/**
 * A slot's main forum (V2): its falls (its lives follow, D349) and its phase. Public, replicated to everyone in
 * AHOLGameState (D353: the players' table shows every slot's lives and forum). V3: its hold, public too.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLForumState
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Falls = 0;

	UPROPERTY()
	EHOLForumPhase Phase = EHOLForumPhase::Standing;

	/** Ruin: simulation steps left before the rebuilding starts. */
	UPROPERTY()
	int32 RuinStepsLeft = 0;

	/** Rebuilding: work done (HOLConstruction::WorkPerBuilder per step of the rebuilding alone). */
	UPROPERTY()
	int64 RebuildWork = 0;

	/** V3 (D352): steps held so far, from the fall on (the ruin included); ForumHoldTime reached, the foundations razed. 0 standing. */
	UPROPERTY()
	int32 HoldSteps = 0;

	/** V3: what the hold did this step (holding, contested, receding). */
	UPROPERTY()
	EHOLForumHold Hold = EHOLForumHold::None;

	/** V3: the attacker's slot whose colour fills the ring (FHOLForumPresence::AttackerSlot); kept while the count recedes. */
	UPROPERTY()
	int8 HolderSlot = INDEX_NONE;

	bool IsDown() const { return Phase != EHOLForumPhase::Standing; }
	bool IsContested() const { return Hold == EHOLForumHold::Contested; }
};

/** What a main forum shows (V2.3a, V2-Q1): itself, rubble at its size through the ruin, a site while it rises again. */
enum class EHOLForumLook : uint8
{
	Standing,
	Rubble,
	/** The look of any site under construction (AHOLBuilding), at the rebuilding's progress (FHOLForumLookChoice::Progress). */
	Site,
};

struct FHOLForumLookChoice
{
	EHOLForumLook Look = EHOLForumLook::Standing;
	/** Share of the forum shown risen, 0 to 1 (1 standing, 0 in rubble). */
	float Progress = 1.f;
};

/** The values of the forum's rules, in simulation steps (FromSettings: UHOLBuildingSettings and the step's length). */
struct HEROESOFLIBERTY_API FHOLForumTuning
{
	/** D349: lives = HOLForumRules::BaseLives + tier - falls (no tier in the game yet: 0, 2 lives). */
	int32 Tier = 0;
	int32 RuinSteps = 1;
	/** Steps the rebuilding takes alone (no Peasant). */
	int32 RebuildSteps = 1;
	/** D312: N builders work N^Exponent times as fast as one. */
	float BuilderExponent = 0.63f;
	/** Seconds of one simulation step (the ruin's countdown in seconds, V2.3b). */
	float StepSeconds = 0.1f;
	/** V3 (D348): steps held without contest that raze the foundations (ForumHoldTime). */
	int32 HoldSteps = 1;
	/** V3 (V3-Q3): steps of hold given back each step with no attacker (ForumHoldDecay, seconds a second, rounded). */
	int32 HoldDecaySteps = 2;
	/** V3.2 (V3-Q1): the zone's reach from the forum's edge (cm, ForumHoldRadius). */
	float HoldRadius = 1200.f;

	static FHOLForumTuning FromSettings();
};

/**
 * V2.3b: what the selection block shows of a main forum, its own or another player's (D353: the lives are public).
 * Standing: its health as before, its lives under it; in ruin: the seconds left in place of the health (0 until it stands
 * again); rebuilding: its progress, its builders at work for its owner alone (as a site's, D313), then its lives.
 */
struct FHOLForumBlock
{
	/** False: not a main forum (a secondary centre included), the block unchanged. */
	bool bShown = false;
	/** In ruin or rebuilding: the health row hidden, the phase's line in its place. */
	bool bHideHealth = false;
	/** For the HUD's Describe: ", lives 2", ", in ruin 25 s, lives 1", ", rebuilding 40%, builders 2, lives 1". */
	FString Shown;
	/** One line each, empty standing: « En ruine — 25 s », « Reconstruction — 40 % », « Bâtisseurs : 2 ». */
	FText Text;
	/** Its lives, on a row of their own as the health's (V2.3d: a small grey « Vies » label, then the value). */
	int32 Lives = 0;
};

/** E4-1: who a row of the players' table is to the one looking (its icon before the name, B8b). */
enum class EHOLPlayerRelation : uint8
{
	/** Seen by a spectator (no slot): no icon. */
	None,
	/** The viewer's own row (« Vous »): no icon for now. */
	Self,
	/** Same camp as the viewer (HOLVictoryRules::GetCamp): a green shield. */
	Ally,
	/** Another camp: red crossed swords. */
	Enemy,
};

/**
 * V2.4 (V2-Q4): a row of the players' table, top right (D353: every slot's lives and forum public, AoE2's score's way).
 */
struct FHOLPlayersTableRow
{
	int32 Slot = INDEX_NONE;
	int32 ColorIndex = 0;
	int32 Team = 0;
	/** « Vous » on the local slot's row, « Joueur N » otherwise (as the selection block). */
	FText Name;
	/** « 2 vies », « 1 vie », « 0 vies »; empty once defeated (V2.3d: only « Vaincu »). */
	FText Lives;
	/** Empty standing, « Ruine », « Reconstruction 40 % », « Vaincu ». */
	FText State;
	/** A defeated slot: its row greyed. */
	bool bGreyed = false;
	/** E4-2: a defeated slot: its name struck through. */
	bool bStruck = false;
	/** E4-1: the row's slot seen from the viewer (LocalSlot). */
	EHOLPlayerRelation Relation = EHOLPlayerRelation::None;
	/** First row of a new team (a gap above it); never on the first row. */
	bool bNewGroup = false;
	/** For the tests and the HUD's report: "slot 1 you team 2 colour 1 lives 1 ruin", "slot 3 enemy colour 3 lives 0 defeated greyed struck". */
	FString Shown;
};

/** V3.3 (V3-Q5): the colour of the hold's arc on the ring. */
enum class EHOLForumRingTint : uint8
{
	/** Holding: the holder's colour. */
	Holder,
	/** Contested (or no holder known): grey, frozen. */
	Grey,
	/** Receding: the holder's colour darkened halfway to a dark grey (GetRingColor). */
	Faded,
};

/**
 * V3.3 (V3-Q5): the ring on the ground around a main forum down, at the zone's reach (its footprint widened by HoldRadius,
 * corners rounded): shown to everyone (D353, no fog yet) while the count is above 0 or someone holds or contests it. The
 * hold's arc (Fill, from the top, clockwise) in its Tint; the rest of the ring a thin dark outline of the zone.
 */
struct FHOLForumRing
{
	bool bShown = false;
	/** Share of the ring filled (GetHoldProgress), 0 to 1. */
	float Fill = 0.f;
	EHOLForumRingTint Tint = EHOLForumRingTint::Grey;
	/** The holder's slot (FHOLForumState::HolderSlot), INDEX_NONE if unknown. */
	int32 HolderSlot = INDEX_NONE;
	/** For the tests: "none", or "fill 0.50 holder slot 1" / "fill 0.50 grey" / "fill 0.25 faded slot 1". */
	FString Shown;
};

struct FHOLMatchConfig;

/**
 * The main forum's lives (V2, D349-D352), pure: no world, no actor. A fall costs a life and lays the forum in ruin; the
 * fall that leaves no life is the defeat. The ruin lasts RuinSteps, nothing builds; then the rebuilding works alone at
 * one builder's rate, each Peasant adding by the curve of D312 (1 Peasant as much as the rebuilding alone: 0 -> 120 s,
 * 1 -> 60 s, 5 -> ~32 s, 10 -> ~23 s, 40 -> ~11 s); done, the forum stands again with all its health.
 */
namespace HOLForumRules
{
	/**
	 * D349: 2 lives at tier 0, 3 at tier 1, 4, then 5 at tier 3; the fall that leaves none is the defeat. (D350 writes
	 * "1 + tier - falls", which counts the falls the forum can still rise from: the same rule.)
	 */
	inline constexpr int32 BaseLives = 2;

	/** Lives left: BaseLives + Tier - Falls, never below 0. */
	HEROESOFLIBERTY_API int32 GetLives(int32 Tier, int32 Falls);
	HEROESOFLIBERTY_API int32 GetLives(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/** Work the rebuilding needs (RebuildSteps * WorkPerBuilder). */
	HEROESOFLIBERTY_API int64 GetRebuildTotalWork(const FHOLForumTuning& Tuning);

	/** Work of one step of rebuilding: WorkPerBuilder (alone) + the Builders' (HOLConstruction::GetWorkPerStep). */
	HEROESOFLIBERTY_API int32 GetRebuildWorkPerStep(int32 Builders, float Exponent);

	/** Steps of rebuilding from nothing with Builders Peasants all along. */
	HEROESOFLIBERTY_API int32 GetStepsToRebuild(const FHOLForumTuning& Tuning, int32 Builders);

	/** Share of the rebuilding done, 0 to 1 (1 standing, 0 in ruin). */
	HEROESOFLIBERTY_API float GetRebuildProgress(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/**
	 * The forum falls: one more fall, in ruin for RuinSteps, its rebuilding at 0. Returns the lives left (0: the slot is
	 * defeated); INDEX_NONE if it was not standing (unbreakable: nothing changes).
	 */
	HEROESOFLIBERTY_API int32 Fall(FHOLForumState& State, const FHOLForumTuning& Tuning);

	/**
	 * One simulation step: the ruin counts down (Builders do nothing, D352) then turns to rebuilding; the rebuilding adds its
	 * work (alone + Builders) and stands up once done. Returns true when the phase changed.
	 */
	HEROESOFLIBERTY_API bool Advance(FHOLForumState& State, const FHOLForumTuning& Tuning, int32 Builders);

	/** V3.1: what Step did. */
	struct FStepResult
	{
		/** The phase changed (ruin -> rebuilding, rebuilding -> standing, or the razing's ruin). */
		bool bPhaseChanged = false;
		/** The foundations razed (D348): a new fall; Lives left (0: the slot is defeated). */
		bool bRazed = false;
		int32 Lives = INDEX_NONE;
	};

	/**
	 * V3.1 (D348, D352): one simulation step of a forum with Presence in its zone. Down (ruin or rebuilding): attackers
	 * without defenders raise the hold a step, attackers with defenders freeze it (contested), no attacker lets it recede by
	 * HoldDecaySteps (never below 0); the rebuilding is frozen while attackers stand there (contested or not); the ruin counts
	 * down as ever (nothing builds in it). The hold reaching HoldSteps razes the foundations (Raze, no Advance this step);
	 * otherwise Advance. Standing: the hold at 0, nothing else.
	 */
	HEROESOFLIBERTY_API FStepResult Step(FHOLForumState& State, const FHOLForumTuning& Tuning, int32 Builders, const FHOLForumPresence& Presence);

	/**
	 * V3.1 (V3-Q4): the foundations of a forum down razed: a new complete fall (one more fall, in ruin for RuinSteps, its
	 * rebuilding at 0, its hold at 0). Returns the lives left (0: the defeat); INDEX_NONE if it was standing.
	 */
	HEROESOFLIBERTY_API int32 Raze(FHOLForumState& State, const FHOLForumTuning& Tuning);

	/** V3.1: whole seconds held (HoldSteps * StepSeconds, rounded down). */
	HEROESOFLIBERTY_API int32 GetHoldSeconds(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/** V3.1: share of the hold done, 0 to 1 (the ring's fill, V3.3). */
	HEROESOFLIBERTY_API float GetHoldProgress(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	HEROESOFLIBERTY_API const TCHAR* GetHoldName(EHOLForumHold Hold);

	/** V3.3: the hold shows (ring, block, table): down, with its count above 0 or someone in the zone (Hold not None). */
	HEROESOFLIBERTY_API bool ShowsHold(const FHOLForumState& State);

	/**
	 * V3.3: the hold for the HUD's "HUD forum" line: "hold 35 s holding by slot 1" ("contested", "receding"; " by slot A"
	 * left out with no holder known); empty when it does not show (ShowsHold).
	 */
	HEROESOFLIBERTY_API FString DescribeHold(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/** V3.3 (V3-Q5): the ring of a forum in State (not shown standing, nor down with no count and no one). */
	HEROESOFLIBERTY_API FHOLForumRing GetRing(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/** V3.3: the arc's colour: Holder HolderColor, Grey a dark grey, Faded HolderColor halfway to a darker grey (V3.4b). */
	HEROESOFLIBERTY_API FLinearColor GetRingColor(EHOLForumRingTint Tint, const FLinearColor& HolderColor);

	/**
	 * V3.3: a point of the zone's outline (Body widened by Radius, its corners rounded at Radius), Share 0 to 1 of its
	 * length, from the middle of its +X side (the top of the screen), clockwise seen from above (towards +Y first).
	 */
	HEROESOFLIBERTY_API FVector2D GetZoneOutlinePoint(const FBox2D& Body, double Radius, double Share);

	/** Ruin -> Rebuilding, Rebuilding -> Standing at once (cheat hol.Victory.SkipRuin). False when standing. */
	HEROESOFLIBERTY_API bool Skip(FHOLForumState& State);

	HEROESOFLIBERTY_API const TCHAR* GetPhaseName(EHOLForumPhase Phase);

	/** V2.3a: the look of a forum in State (Standing: itself; Ruin: rubble; Rebuilding: a site at its progress). */
	HEROESOFLIBERTY_API FHOLForumLookChoice GetLook(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/** V2.3a: Peasants at a forum in ruin wait standing, no hammer (they do not work it, D352); they strike once it rises. */
	HEROESOFLIBERTY_API bool BuildersWait(const FHOLForumState& State);

	/** V2.3b: seconds of ruin left, rounded up (RuinStepsLeft * StepSeconds); 0 out of the ruin. */
	HEROESOFLIBERTY_API int32 GetRuinSecondsLeft(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/** V2.3b: the rebuilding's whole percents done (0 in ruin, 100 standing). */
	HEROESOFLIBERTY_API int32 GetRebuildPercent(const FHOLForumState& State, const FHOLForumTuning& Tuning);

	/**
	 * V2.3b: the selection block of a main forum in State (null: not a main forum, nothing shown); Builders (at work, as
	 * the owner alone receives them) shown to its owner only (bOwn), while it rises again.
	 */
	HEROESOFLIBERTY_API FHOLForumBlock GetSelectionBlock(const FHOLForumState* State, const FHOLForumTuning& Tuning, bool bOwn, int32 Builders);

	/**
	 * V2.4 (V2-Q4): the players' table, a row per occupied slot (human or AI; a closed slot has none): in slot order, grouped
	 * by team when there are teams (team 1 first, the slots without a team last). Forums by slot (a missing one standing),
	 * DefeatedMask's bit Slot a defeated slot, LocalSlot the machine's own (INDEX_NONE: none).
	 */
	HEROESOFLIBERTY_API TArray<FHOLPlayersTableRow> GetPlayersTable(const FHOLMatchConfig& Config, TConstArrayView<FHOLForumState> Forums,
		uint8 DefeatedMask, int32 LocalSlot, const FHOLForumTuning& Tuning);
}
