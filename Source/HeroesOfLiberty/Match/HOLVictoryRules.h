// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMatchConfig.h"

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
 * Who has won (D357), pure: no world, no actor. The camps are those of the configuration read at the launch (teams
 * locked): a slot of team 0 is alone in its camp, each team 1..4 is one camp; only the occupied slots (human or AI) count.
 */
namespace HOLVictoryRules
{
	/** The camp of an occupied slot (team 1..4 as is, team 0 as MaxTeams + 1 + Slot: alone), INDEX_NONE for a closed one. */
	HEROESOFLIBERTY_API int32 GetCamp(const FHOLMatchConfig& Config, int32 Slot);

	/**
	 * Over when a single camp still stands while there were several at the launch (it wins, its fallen members with it;
	 * free for all: the last one standing), or when nobody stands any more (no winner). A match of a single camp (one
	 * human alone on the map, or everyone in one team) never wins by itself: it ends only when that camp falls.
	 */
	HEROESOFLIBERTY_API FHOLVictoryResult Evaluate(const FHOLMatchConfig& Config, uint8 DefeatedMask);
}
