// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLMapRules.h"

#include "Algo/Find.h"

namespace UE::HeroesOfLiberty::Map
{
	/**
	 * The match maps in the order of Tools/Data/Maps.csv, which says the same (test HeroesOfLiberty.Map.Csv). Made at the
	 * first call: the FText of a static array would be made before the localization is up.
	 */
	TConstArrayView<FHOLMapInfo> MakeMaps()
	{
		static const FHOLMapInfo Maps[] = {
			{ TEXT("Plain"), NSLOCTEXT("HOLMaps", "Map_Plain", "Plaine"), true },
			{ TEXT("Steppe"), NSLOCTEXT("HOLMaps", "Map_Steppe", "La Steppe"), true },
			{ TEXT("Hinterland"), NSLOCTEXT("HOLMaps", "Map_Hinterland", "L'Arrière-pays"), true },
			{ TEXT("ElDorado"), NSLOCTEXT("HOLMaps", "Map_ElDorado", "El Dorado"), true, true },
			{ TEXT("Cols"), NSLOCTEXT("HOLMaps", "Map_Cols", "Les Cols"), true },
			{ TEXT("Forest"), NSLOCTEXT("HOLMaps", "Map_Forest", "La Forêt profonde"), true },
		};
		return Maps;
	}
}

TConstArrayView<FHOLMapInfo> FHOLMapRules::GetMaps()
{
	return UE::HeroesOfLiberty::Map::MakeMaps();
}

const FHOLMapInfo* FHOLMapRules::FindByKey(const FString& Key)
{
	return Algo::FindByPredicate(GetMaps(), [&Key](const FHOLMapInfo& Line)
	{
		return Key.TrimStartAndEnd().Equals(Line.Key, ESearchCase::IgnoreCase);
	});
}

FSoftObjectPath FHOLMapRules::GetAssetPath(const TCHAR* Key)
{
	return FSoftObjectPath(FString::Printf(TEXT("/Game/Maps/%s.%s"), Key, Key));
}

const FHOLMapInfo* FHOLMapRules::FindByAsset(const FSoftObjectPath& Asset)
{
	return Asset.IsNull() ? nullptr : FindByKey(Asset.GetAssetName());
}

FText FHOLMapRules::GetDisplayName(const FSoftObjectPath& Asset)
{
	const FHOLMapInfo* Info = FindByAsset(Asset);
	return Info ? Info->DisplayName : FText::AsCultureInvariant(Asset.GetAssetName());
}
