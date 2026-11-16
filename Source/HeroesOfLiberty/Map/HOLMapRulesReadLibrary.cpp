// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLMapRulesReadLibrary.h"

#include "Map/HOLMapRules.h"

namespace
{
	FHOLMapInfoValue ToValue(const FHOLMapInfo& Info)
	{
		FHOLMapInfoValue Value;
		Value.Key = Info.Key;
		Value.DisplayName = Info.DisplayName;
		Value.bBudget = Info.bBudget;
		Value.bGoldAtCentre = Info.bGoldAtCentre;
		Value.bDenseWood = Info.bDenseWood;
		Value.NumSites = Info.NumSites;
		return Value;
	}
}

TArray<FHOLMapInfoValue> UHOLMapRulesReadLibrary::GetMaps()
{
	TArray<FHOLMapInfoValue> Result;
	for (const FHOLMapInfo& Info : FHOLMapRules::GetMaps())
	{
		Result.Add(ToValue(Info));
	}
	return Result;
}

bool UHOLMapRulesReadLibrary::FindByKey(const FString& Key, FHOLMapInfoValue& Out)
{
	const FHOLMapInfo* Info = FHOLMapRules::FindByKey(Key);
	if (Info == nullptr)
	{
		return false;
	}
	Out = ToValue(*Info);
	return true;
}

FString UHOLMapRulesReadLibrary::GetAssetPath(const FString& Key)
{
	return FHOLMapRules::GetAssetPath(*Key).ToString();
}

bool UHOLMapRulesReadLibrary::FindByAsset(const FString& AssetPath, FHOLMapInfoValue& Out)
{
	const FHOLMapInfo* Info = FHOLMapRules::FindByAsset(FSoftObjectPath(AssetPath));
	if (Info == nullptr)
	{
		return false;
	}
	Out = ToValue(*Info);
	return true;
}

FText UHOLMapRulesReadLibrary::GetDisplayName(const FString& AssetPath)
{
	return FHOLMapRules::GetDisplayName(FSoftObjectPath(AssetPath));
}
