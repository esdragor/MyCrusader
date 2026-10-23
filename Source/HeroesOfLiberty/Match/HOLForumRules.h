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

/**
 * A slot's main forum (V2): its falls (its lives follow, D349) and its phase. Public, replicated to everyone in
 * AHOLGameState (D353: the players' table shows every slot's lives and forum).
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

	bool IsDown() const { return Phase != EHOLForumPhase::Standing; }
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
	/** First row of a new team (a gap above it); never on the first row. */
	bool bNewGroup = false;
	/** For the tests and the HUD's report: "slot 1 you team 2 colour 1 lives 1 ruin". */
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
