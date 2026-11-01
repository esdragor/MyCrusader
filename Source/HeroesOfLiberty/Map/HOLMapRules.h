// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

/**
 * C0.1 (Cartes): one map of the match: its file is /Game/Maps/<Key>, one file per size beside it (TC-Q3). Tools/Data/
 * Maps.csv mirrors it, with the python recipe that draws the map (Tools/Editor/maps/<Recipe>.py).
 */
struct FHOLMapInfo
{
	/** Asset name under /Game/Maps and key of Tools/Data/Maps.csv. */
	const TCHAR* Key = TEXT("");

	/** The translated name ("Plaine"; NSLOCTEXT HOLMaps, Map_<Key>). */
	FText DisplayName;

	/**
	 * The Budget column: true for `standard` and `centre`, the map holds the resources per player of the Map.Table test (M-Q4 = A: no
	 * resource problem before 3/4 of a classic 30 min game); false for `none`, a map that says otherwise.
	 */
	bool bBudget = false;

	/**
	 * C3 (El Dorado, GDD 8.5): the Budget `centre`. Same budget per player, but the gold is one cluster in the middle of the map (each
	 * place's share of it is what its territory holds): no gold in the bases, the cluster as far from every place as from any other.
	 */
	bool bGoldAtCentre = false;

	/**
	 * C5 (La Foret profonde, CQ7 = B, GDD D598): the Budget `dense`. The same budget per player, except the wood: twice the Steppe's trees
	 * (Map.Table), in groves of 30 to 40.
	 */
	bool bDenseWood = false;
};

/** C0.1: pure rules of the match maps, for the menu, the list of lobbies and the manager client (one table, Maps.csv). */
struct HEROESOFLIBERTY_API FHOLMapRules
{
	/** The maps, in the order of Tools/Data/Maps.csv (the first one is the default of a new game). */
	static TConstArrayView<FHOLMapInfo> GetMaps();

	/** The map of a key (case ignored); null if unknown. */
	static const FHOLMapInfo* FindByKey(const FString& Key);

	/** The file of a map, as the configuration carries it: /Game/Maps/Plain.Plain. */
	static FSoftObjectPath GetAssetPath(const TCHAR* Key);

	/**
	 * The map of a file, by its asset name: /Game/Maps/Plain.Plain is Plain; null if unknown, a sized file
	 * (/Game/Maps/Plain_Large.Plain_Large, TC-Q3) or a test map too.
	 */
	static const FHOLMapInfo* FindByAsset(const FSoftObjectPath& Asset);

	/** The translated name of a map, or its asset name as it is for one the table does not know (a test map). */
	static FText GetDisplayName(const FSoftObjectPath& Asset);
};
