// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLMapSizeReadLibrary.generated.h"

/**
 * The pure rules of the map sizes (FHOLMapSizeRules, Map/HOLMapSizeRules.h, C++, same module as this library) for Angelscript (namespace
 * HOLMapSizeRead there): D836 keeps the rule in C++ and the script reads it here, so that the tests of the rule (HOLMapSizeTests.cpp)
 * can be ported to script (D830, D839). Wrappers only: no rule here. A size is its number (EHOLMapSize: 0 None, 1 Small, 2 Medium, 3 Normal,
 * 4 Large, 5 Huge); the slots of a match are arrays read by index, States (EHOLSlotState: 0 Closed, 1 Human, 2 AI) and Teams (0 = none),
 * the slots past the arrays closed; a file is its path as text ("/Game/Maps/Plain.Plain"); a nullable pointer is a bool plus an out value.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMapSizeReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLMapSizeRules::GetSizes().Num(): the five sizes. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static int32 GetNumSizes();

	/** The Index-th size of FHOLMapSizeRules::GetSizes() (0 = Small) as its number; 0 (None) past the table. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static int32 GetSizeAt(int32 Index);

	/** FHOLMapSizeRules::Find: false (Out untouched) for None; Key, NumPlaces and SideTiles of the size's line. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static bool FindSize(int32 Size, FString& OutKey, int32& OutNumPlaces, int32& OutSideTiles);

	/** FHOLMapSizeRules::FindByKey: the number of a key (case ignored); 0 (None) if unknown. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static int32 FindByKey(const FString& Key);

	/** FHOLMapSizeRules::GetNumPlaces(size): 0 for None. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static int32 GetNumPlaces(int32 Size);

	/** FHOLMapSizeRules::GetSideTiles: 0 for None. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static int32 GetSideTiles(int32 Size);

	/** FHOLMapSizeRules::IsAllowed. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static bool IsAllowed(int32 Size, int32 NumPlayers);

	/** FHOLMapSizeRules::GetAllowedSizes: the numbers of the sizes that can hold NumPlayers, Small to Huge. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static TArray<int32> GetAllowedSizes(int32 NumPlayers);

	/** FHOLMapSizeRules::GetDefaultSize: the number of the smallest size that holds NumPlayers; 0 (None) past eight. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static int32 GetDefaultSize(int32 NumPlayers);

	/** FHOLMapSizeRules::GetMapFile: the file of a map (as text) at a size; the map as it is for None, empty for an empty map. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static FString GetMapFile(const FString& Map, int32 Size);

	/** FHOLMapSizeRules::GetFixedPlaces of the slots (States, Teams as above). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static TArray<int32> GetFixedPlaces(const TArray<int32>& States, const TArray<int32>& Teams);

	/** FHOLMapSizeRules::DrawPlaces: one place per slot (INDEX_NONE = -1 for a closed one); empty when the players do not fit. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static TArray<int32> DrawPlaces(int32 Seed, const TArray<int32>& States, const TArray<int32>& Teams, int32 NumPlaces, bool bTeamsTogether);

	/** FHOLMapSizeRules::GetNumPlaces(FHOLMatchConfig): the places of a configuration's size, 8 without a size. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static int32 GetConfigNumPlaces(int32 Size);

	/** FHOLMapSizeRules::GetPlaces of a configuration made of these fields (the others default), its slots those of States and Teams. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static TArray<int32> GetConfigPlaces(int32 Seed, int32 Size, bool bDrawPlaces, bool bTeamsTogether, const TArray<int32>& States, const TArray<int32>& Teams);

	/** FHOLMapSizeRules::CheckPlaces of the same configuration: false with the reason in OutError, true with it empty. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Map")
	static bool CheckConfigPlaces(int32 Seed, int32 Size, bool bDrawPlaces, bool bTeamsTogether, const TArray<int32>& States, const TArray<int32>& Teams, FString& OutError);

	/**
	 * FHOLMapSizeRules::FitSlotsToSize on slots made of States, Teams and Colors (completed to eight slots, closed): false (nothing
	 * changed) when the players do not fit; true with the arrays moved.
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Map")
	static bool FitSlotsToSize(UPARAM(ref) TArray<int32>& States, UPARAM(ref) TArray<int32>& Teams, UPARAM(ref) TArray<int32>& Colors, int32 Size);
};
