// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLMatchConfigReadLibrary.generated.h"

/**
 * FHOLMatchConfig (Match/HOLMatchConfig.h, C++, same module as this library) for Angelscript (namespace HOLMatchConfigRead there): D836 keeps
 * the contract in C++ and the script reads it here, so that the tests of the configuration (HOLMatchConfigTests.cpp) can be ported to script
 * (D830, D839). Wrappers only: no rule here. A configuration is its JSON text (FHOLMatchConfig::ToJson); the script reads and edits it through
 * these functions, never through the struct. Numbers are the enums' ones (EHOLSlotState 0 Closed, 1 Human, 2 AI; EHOLFaction 0 DawnOrder,
 * 1 BlackLegions, 2 DragonChildren, 3 ShadowCircle, 4 FireHeirs, 5 Templars; EHOLMatchMode 0 Standard, 1 Diplomacy; EHOLMapSize as in
 * HOLMapSizeReadLibrary). A function given a JSON that FromJson refuses returns false / an empty text / -1 (the test then fails on that).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLMatchConfigReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLMatchConfig::CurrentVersion. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 GetCurrentVersion();

	/** FHOLMatchConfig::MaxSlots. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 GetMaxSlots();

	/** HOLPlayerColors::NumColors. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 GetNumColors();

	/** HOLPlayerColors::RandomColor (the colour index of "Aleatoire"). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 GetRandomColor();

	/** FHOLMatchConfig::MakeDefault().ToJson(). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static FString MakeDefaultJson();

	/** FromJson(Json) on a fresh configuration: false with the reason in OutError (empty when true). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool CanRead(const FString& Json, FString& OutError);

	/** Json read, the slot Index set to these fields (the others kept), written back; empty if unreadable or Index outside 0..7. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static FString SetSlotJson(const FString& Json, int32 Index, int32 State, int32 Faction, int32 ColorIndex, int32 Team, const FString& PlayerName);

	/** Json read, Seed, Map (its path as text) and the four Standard options set (the others kept), written back; empty if unreadable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static FString SetOptionsJson(const FString& Json, int32 Seed, const FString& Map, bool bNomad, bool bRegicide, bool bCatastrophe, bool bSacredSites);

	/** Json read as C, C.ToJson() read again as C2: C == C2 (nothing lost by ToJson / FromJson); false if either is unreadable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool RoundTripsLosslessly(const FString& Json);

	/** The two JSON read and compared with FHOLMatchConfig::operator== (every field); false if either is unreadable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool AreEqual(const FString& JsonA, const FString& JsonB);

	/**
	 * FromJson(BadJson) on the configuration of BaseJson: bOutParsed is its result; the return is whether the configuration is
	 * unchanged afterwards (FromJson leaves its output untouched on failure). False if BaseJson is unreadable.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool ReadLeavesOutputUntouchedOnFailure(const FString& BaseJson, const FString& BadJson, bool& bOutParsed);

	/** Every field a test of the file reads, of the configuration Json; false (outs untouched) if unreadable. Map is its path as text. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool Describe(const FString& Json, int32& OutVersion, int32& OutSeed, FString& OutMap, int32& OutMapSize, bool& bOutDrawPlaces, int32& OutMode,
		bool& bOutNomad, bool& bOutRegicide, bool& bOutCatastrophe, bool& bOutSacredSites, TArray<int32>& OutStates, TArray<int32>& OutFactions,
		TArray<int32>& OutColors, TArray<int32>& OutTeams, TArray<FString>& OutNames);

	/** FHOLMatchConfig::GetNumOccupiedSlots; -1 if unreadable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 GetNumOccupiedSlots(const FString& Json);

	/** FHOLMatchConfig::FindFreeHumanSlot; -2 if unreadable (INDEX_NONE = -1 is a real answer). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 FindFreeHumanSlot(const FString& Json, const TArray<int32>& TakenSlots);

	/** FHOLMatchConfig::HasPlayerNames; false if unreadable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool HasPlayerNames(const FString& Json);

	/** FHOLMatchConfig::FindSlotForName; -2 if unreadable (INDEX_NONE = -1 is a real answer). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 FindSlotForName(const FString& Json, const FString& Name, const TArray<int32>& TakenSlots);

	/**
	 * FHOLMatchConfig::DrawRandomColors(FRandomStream(Seed)) on a default configuration whose slots are States (completed to eight, closed) and
	 * Colors (completed with the default colour of the slot, its index;HOLPlayerColors::RandomColor allowed, which a JSON read would clamp): the eight colour indices after the draw.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static TArray<int32> DrawRandomColors(int32 Seed, const TArray<int32>& States, const TArray<int32>& Colors);

	/** FFileHelper::LoadFileToString of a path relative to the project directory (FPaths::ProjectDir()); false if unreadable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool LoadProjectFileText(const FString& RelativePath, FString& OutText);

	/** FHOLMatchConfig::NeutralOwner (the owner index of the neutral units). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static int32 GetNeutralOwner();

	/** HOLPlayerColors::GetColor(i).DWColor() for i in 0..NumColors-1, each as an int32 bit pattern (the script counts the distinct ones). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static TArray<int32> GetPaletteDWColors();

	/** HOLPlayerColors::GetColorName(ColorIndex).ToString() (empty = unnamed). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static FString GetPaletteColorName(int32 ColorIndex);

	/** HOLPlayerColors::GetLinearColor(A) == HOLPlayerColors::GetLinearColor(B) (an index of NumColors and above wraps). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool PaletteLinearColorsEqual(int32 IndexA, int32 IndexB);

	/**
	 * HOLPlayerColors::GetOwnerLinearColor(OwnerPlayer, SlotColors) compared with HOLPlayerColors::GetLinearColor(PaletteIndex) within Tolerance
	 * (0 = exact): the colour a unit of that owner is drawn with is that palette colour. SlotColors[Slot] is the colour each slot chose.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool OwnerColorIsPaletteColor(int32 OwnerPlayer, const TArray<int32>& SlotColors, int32 PaletteIndex, float Tolerance);

	/**
	 * What a dedicated server started with -MatchConfig=<file> goes through, on a game instance subsystem made here (UHOLMatchSubsystem): Json is
	 * written to a file of the automation transient directory, then a fresh subsystem is asked HasConfig and GetConfigOrDefault before the load (no
	 * configuration, the default), LoadFromFile, then HasConfig and the configuration against Json (AreEqual). The refusal of a missing file is not
	 * here (it logs an Error that only the C++ test declares expected). False if Json is unreadable or the file cannot be written.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match")
	static bool RunSubsystemFile(const FString& Json, bool& bOutNoConfigAtFirst, bool& bOutDefaultWhenNone, bool& bOutLoaded, bool& bOutConfigFromFile);
};
