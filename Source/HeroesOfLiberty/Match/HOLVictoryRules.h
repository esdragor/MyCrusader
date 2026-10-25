// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"

class UWorld;

/** Every slot fits a byte of flags (bit Slot): the defeated slots, the winners. */
static_assert(FHOLMatchConfig::MaxSlots <= 8, "a slot mask is one byte");

/** What the rules say of a match at a given time. */
struct HEROESOFLIBERTY_API FHOLVictoryResult
{
	bool bOver = false;
	/** Bit Slot: that slot won, defeated or not (D357: a team wins when all its opponents have lost). 0: nobody won. */
	uint8 WinnerMask = 0;

	TArray<int32> GetWinners() const;
};

/**
 * Each unit owner's camp as a byte, for the simulation's hot loops (combat, avoidance): built once a step, compared per
 * pair. Two owners of one team 1..4 share a camp (the team's first occupied slot); any other owner (team 0, a closed
 * slot, an owner beyond the slots) is alone. The single place where the stances of D369 (diplomacy) will plug in.
 */
struct HEROESOFLIBERTY_API FHOLCampTable
{
	/** Each owner alone (no configuration: tests without a game state, free for all). */
	FHOLCampTable();
	explicit FHOLCampTable(const FHOLMatchConfig& Config);
	/** The launch configuration of the world's AHOLGameState; each owner alone without one. */
	static FHOLCampTable FromWorld(const UWorld* World);

	uint8 Get(const int32 Owner) const { return Camps[static_cast<uint8>(Owner)]; }
	/** Whether units of these owners fight each other. */
	bool AreHostile(const int32 OwnerA, const int32 OwnerB) const { return Get(OwnerA) != Get(OwnerB); }

private:
	uint8 Camps[256];
};

/**
 * Who has won (D357), pure: no world, no actor. The camps are those of the configuration read at the launch (teams
 * locked): a slot of team 0 is alone in its camp, each team 1..4 is one camp; only the occupied slots (human or AI) count.
 */
namespace HOLVictoryRules
{
	/** The camp of an occupied slot (team 1..4 as is, team 0 as MaxTeams + 1 + Slot: alone), INDEX_NONE for a closed one. */
	HEROESOFLIBERTY_API int32 GetCamp(const FHOLMatchConfig& Config, int32 Slot);

	/** Whether two slots' units fight each other (FHOLCampTable's rule): not of one team 1..4. */
	HEROESOFLIBERTY_API bool AreHostile(const FHOLMatchConfig& Config, int32 SlotA, int32 SlotB);

	/**
	 * Over when a single camp still stands while there were several at the launch (it wins, its fallen members with it;
	 * free for all: the last one standing), or when nobody stands any more (no winner). A match of a single camp (one
	 * human alone on the map, or everyone in one team) never wins by itself: it ends only when that camp falls.
	 */
	HEROESOFLIBERTY_API FHOLVictoryResult Evaluate(const FHOLMatchConfig& Config, uint8 DefeatedMask);
}
