// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "HOLMapSizeRules.generated.h"

struct FHOLMatchConfig;
struct FHOLMatchSlot;

/**
 * TC1 (TC-Q3 to TC-Q8): the named sizes of a map. Each map exists once per size, as its own file; the size sets its
 * number of places (TC-Q4).
 */
UENUM()
enum class EHOLMapSize : uint8
{
	/** No size: the map file is taken as it is (configurations written before TC1, test maps). */
	None,
	Small,		// Petite, 2 players
	Medium,		// Moyenne, 4 players
	Normal,		// Normale, 6 players
	Large,		// Grande, 8 players
	Huge,		// Immense, 8 players
};

/** One named size (TC-Q5: AoE2 proportions brought to the 880 m limit of D344). Tools/Data/MapSizes.csv mirrors it. */
struct FHOLMapSizeInfo
{
	EHOLMapSize Size = EHOLMapSize::None;

	/** Suffix of the map files of this size and key of Tools/Data/MapSizes.csv. */
	const TCHAR* Key = TEXT("");

	/** Places of the map, numbered in order around its circle (TC-Q4). */
	int32 NumPlaces = 0;

	/** Side of the playable area, in tiles of UHOLBuildingSettings::TileSize (4 m). */
	int32 SideTiles = 0;
};

/** TC1: pure rules of the map sizes and of the start places, for the menu, the lobby and the game mode (TC3). */
struct HEROESOFLIBERTY_API FHOLMapSizeRules
{
	/** The five sizes, Small to Huge. */
	static TConstArrayView<FHOLMapSizeInfo> GetSizes();

	/** The size's line, or null for None. */
	static const FHOLMapSizeInfo* Find(EHOLMapSize Size);

	/** The size of a key (Small... Huge, case ignored); None if unknown. */
	static EHOLMapSize FindByKey(const FString& Key);

	/** Places of a size; 0 for None. */
	static int32 GetNumPlaces(EHOLMapSize Size);

	/** Side in tiles of a size; 0 for None. */
	static int32 GetSideTiles(EHOLMapSize Size);

	/** TC-Q4: a size can hold NumPlayers seated players (the others are greyed in the list). */
	static bool IsAllowed(EHOLMapSize Size, int32 NumPlayers);

	/** The sizes that can hold NumPlayers, Small to Huge. */
	static TArray<EHOLMapSize> GetAllowedSizes(int32 NumPlayers);

	/** TC-Q4: the size offered for NumPlayers, the smallest that holds them; None past eight. */
	static EHOLMapSize GetDefaultSize(int32 NumPlayers);

	/**
	 * TC-Q3: the file of a map at a size: /Game/Maps/Plain.Plain at Large is /Game/Maps/Plain_Large.Plain_Large. None
	 * gives the map back as it is.
	 */
	static FSoftObjectPath GetMapFile(const FSoftObjectPath& Map, EHOLMapSize Size);

	/** The place of each slot without a draw: slot i at place i, INDEX_NONE for a closed slot. */
	static TArray<int32> GetFixedPlaces(TConstArrayView<FHOLMatchSlot> Slots);

	/**
	 * TC-Q6: the places drawn for the occupied slots (INDEX_NONE for a closed one), the same for the same seed. Fewer
	 * players than places: places as evenly spread around the circle as can be, turned at random. bTeamsTogether: the
	 * order of the teams drawn, the members of a team on neighbouring places (a slot without a team, Team 0, is a team
	 * of its own). Otherwise any order. Empty when the players don't fit.
	 */
	static TArray<int32> DrawPlaces(int32 Seed, TConstArrayView<FHOLMatchSlot> Slots, int32 NumPlaces, bool bTeamsTogether);

	/**
	 * The places of a configuration: fixed without bDrawPlaces (test scenarios, quick_check, older files), otherwise drawn
	 * from its seed on the places of its size (MaxSlots without a size).
	 */
	static TArray<int32> GetPlaces(const FHOLMatchConfig& Config);

	/** Places of a configuration's map: those of its size, MaxSlots without a size (a test map such as Plain). */
	static int32 GetNumPlaces(const FHOLMatchConfig& Config);

	/**
	 * TC3: whether a configuration fits its map, checked by the game mode at the launch (and by the menu before it): its
	 * occupied slots no more than the places of its size and, with fixed places, each on a place of the map. False, with
	 * the reason in OutError, otherwise: the match starts with no start for anyone.
	 */
	static bool CheckPlaces(const FHOLMatchConfig& Config, FString* OutError = nullptr);

	/**
	 * TC3 (TC-Q4, New game screen): the slots made to fit a size, the slots past its places closed. An occupied slot past
	 * them first moves, as it is, onto the first closed slot below; the order of the players is kept. False (nothing
	 * changed) when there are more occupied slots than places.
	 */
	static bool FitSlotsToSize(TArray<FHOLMatchSlot>& Slots, EHOLMapSize Size);
};
