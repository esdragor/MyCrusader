// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLNewGameReadLibrary.generated.h"

struct FHOLLobby;

/**
 * One row of the lobby as the New game screen shows it (FHOLLobbySlot of Net/HOLManagerClientRules.h, a plain C++ struct there, flattened).
 * Numbers: State 0 Open, 1 Closed, 2 Player, 3 AI (EHOLLobbySlotState); Level is EHOLAIDifficulty (0 Easy, 1 Normal, 2 Hard, 3 Evolving);
 * Faction is EHOLFaction (0 DawnOrder .. 5 Templars) or -1 when the manager's name is unknown. The three bCan* are the FHOLLobby rules of
 * the same names, already computed for this slot (the script applies them, it does not re-derive them).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLNewGameLobbySlotView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 Index = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 State = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 ColorIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 Team = 0;

	/** The manager's faction name ("DawnOrder"...), empty for an open or closed slot. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString FactionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 Faction = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bReady = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 Level = 1;

	/** This row is a Player slot held by the lobby's leader (the crown of the screen). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bIsLeader = false;

	/** This row is the token's own slot. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bIsYou = false;

	/** FHOLLobby::CanEditSlot(Index): colour, team and Ready (my own line). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bCanEdit = false;

	/** FHOLLobby::CanToggleSlot(Index): open / close / put an AI. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bCanToggle = false;

	/** FHOLLobby::CanEditAISlot(Index): the AI's colour and team (the leader's). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bCanEditAI = false;
};

/**
 * The lobby's state as the New game screen (lobby mode) reads it: FHOLLobby (Net/HOLManagerClientRules.h) is a plain C++ struct, not exposed
 * to the script, so this is its flat copy (D836: the rules stay in C++, the script reads). Enum fields are numbers: State 0 Lobby, 1 Starting,
 * 2 InGame (EHOLLobbyState); MapSize is EHOLMapSize (0 None, 1 Small, 2 Medium, 3 Normal, 4 Large, 5 Huge); FogMode is EHOLFogMode (0 Disabled,
 * 1 Explored, 2 Normal; Normal when the manager says nothing); Mode is EHOLMatchMode (0 Standard, 1 Diplomacy). Fields not named after an
 * FHOLLobby field are its member functions' result (GetNumPlaces, FindYourSlot, IsYouLeader, IsYouSpectator, HasOpenSlot, CanStart, GetBiomeKey).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLNewGameLobbyView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString Id;

	/** The lobby's name (the heading of the screen). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 State = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString Leader;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString You;

	/** The map's key ("Plain"). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString Map;

	/** The size's key ("Small"...), empty for a map without sizes. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString Size;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 MapSize = 0;

	/** FHOLLobby::GetNumPlaces. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 NumPlaces = 8;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bTeamsTogether = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 FogMode = 2;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	FString BiomeKey = TEXT("default");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 Mode = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bNomad = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bRegicide = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bCatastrophe = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bSacredSites = false;

	/** Index of the token's slot; -1 (INDEX_NONE) without one (a spectator). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 YourSlot = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bYouLeader = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bYouSpectator = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bHasOpenSlot = false;

	/** FHOLLobby::CanStart (the manager still decides). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bCanStart = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	bool bHasMatch = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 NumPlayers = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 NumAIs = 0;

	/** Player slots that are Ready. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	int32 NumReady = 0;

	/** The spectators' names, in order of entry. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	TArray<FString> Spectators;

	/** Always eight rows once built from a lobby. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|NewGame")
	TArray<FHOLNewGameLobbySlotView> Slots;
};

/**
 * Reads the New game screen needs and the other *ReadLibrary of the module do not give (script namespace HOLNewGameRead). Already exposed, so
 * NOT repeated here: the maps and their sites (HOLMapRulesRead: GetMaps, FindByKey, GetAssetPath...), the sizes and the places check
 * (HOLMapSizeRead: GetNumSizes, GetSizeAt, FindSize, IsAllowed, GetAllowedSizes, GetDefaultSize, GetMapFile, GetConfigNumPlaces,
 * CheckConfigPlaces, FitSlotsToSize...), the faction list and the cheat (HOLFactionChoiceRead: IsCheatOn, Sanitize, IsListed, GetListed,
 * TryFindByName...), the configuration as JSON (HOLMatchConfigRead: Describe, SetSlotJson, SetOptionsJson...). Wrappers only: no rule here.
 * Numbers as in those libraries; FogMode is EHOLFogMode (0 Disabled, 1 Explored, 2 Normal).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLNewGameReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** FHOLMapRules::GetSiteCount of a map's asset path (HOLMapRulesRead::GetAssetPath(Key)): the "Sites sacres" box is greyed at 0; 0 for an unknown map. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|NewGame")
	static int32 GetSiteCount(const FString& AssetPath);

	/** The Fog, Mode, Biome, TeamsTogether, MapSize and per-slot AI fields of the configuration Json (what HOLMatchConfigRead::Describe does not give); false (outs untouched) if unreadable. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|NewGame")
	static bool DescribeGlobals(const FString& Json, int32& OutFogMode, bool& bOutTeamsTogether, FString& OutBiomeKey, int32& OutStartingPeasants, TArray<int32>& OutDifficulties,
		TArray<int32>& OutAIBrains);

	/**
	 * Json read, Map (path as text), MapSize (EHOLMapSize number), DrawPlaces, TeamsTogether, FogMode (EHOLFogMode number), Mode (EHOLMatchMode
	 * number) and Biome key set (the others kept), written back; empty if unreadable. An out-of-range number is brought back to None / Disabled / Standard.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|NewGame")
	static FString SetGlobalsJson(const FString& Json, const FString& Map, int32 MapSize, bool bDrawPlaces, bool bTeamsTogether, int32 FogMode, int32 Mode, const FString& BiomeKey);

	/** Json read, the slot Index's Difficulty (EHOLAIDifficulty number, out of range = Normal) and bAIBrain set (the others kept), written back; empty if unreadable or Index outside 0..7. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|NewGame")
	static FString SetSlotAIJson(const FString& Json, int32 Index, int32 Difficulty, bool bAIBrain);

	/** The lobby of the screen as a flat view (C++ only: FHOLLobby is not a reflected type; the screen's script-facing getter calls this). */
	static FHOLNewGameLobbyView MakeLobbyView(const FHOLLobby& Lobby);
};
