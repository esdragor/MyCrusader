// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "UObject/SoftObjectPath.h"
#include "HOLMatchConfig.generated.h"

/** Who holds a slot (§ 14.4: every slot can be a human or an AI). */
UENUM()
enum class EHOLSlotState : uint8
{
	Closed,
	Human,
	AI,
};

/** The six factions of § 13; the prototype plays the first two (D11). */
UENUM()
enum class EHOLFaction : uint8
{
	DawnOrder,		// Ordre de l'Aube
	BlackLegions,	// Légions Noires
	DragonChildren,	// Enfants du Dragon
	ShadowCircle,	// Cercle de l'Ombre
	FireHeirs,		// Héritiers du Feu
	Templars,		// Templiers
};

/** An AI slot's level (D422, D329; IA0.1-Q1b): changes nothing in the game yet, only its name ("IA Difficile 3"). */
UENUM()
enum class EHOLAIDifficulty : uint8
{
	Easy,		// Facile
	Normal,
	Hard,		// Difficile
	Evolving,	// Évolutive (D329)
};

/** Game modes (§ 14.3): only Standard until the prototype is done (D205). */
UENUM()
enum class EHOLMatchMode : uint8
{
	Standard,
};

/** One of the eight slots of a match (D244). The start position is the slot index: the maps are symmetric. */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMatchSlot
{
	GENERATED_BODY()

	UPROPERTY()
	EHOLSlotState State = EHOLSlotState::Closed;

	UPROPERTY()
	EHOLFaction Faction = EHOLFaction::DawnOrder;

	/** Index in the player palette (HOLPlayerColors, D233). */
	UPROPERTY()
	int32 ColorIndex = 0;

	/** 0 = no team, 1 to MaxTeams. */
	UPROPERTY()
	int32 Team = 0;

	/** Name of the human the lobby manager put in this slot (version 2, R3.1); empty otherwise. */
	UPROPERTY()
	FString PlayerName;

	/**
	 * IA0.1-Q1b: an AI slot's level, read from the .json ("difficulty": "Hard"); a missing or unknown value is Normal
	 * (FromJson warns of the unknown one). Ignored for a human slot.
	 */
	UPROPERTY()
	EHOLAIDifficulty Difficulty = EHOLAIDifficulty::Normal;

	bool operator==(const FHOLMatchSlot&) const = default;
};

/**
 * Match configuration (D243, D244): the contract between whoever sets up a match (the menu in single player, the
 * lobby of M2.5, a JSON file given to a dedicated server with -MatchConfig=) and the game mode that reads it.
 * Only what the GDD has decided; new fields are added one decision at a time, under a new Version.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLMatchConfig
{
	GENERATED_BODY()

	/** Format version: a file of an older version is read with defaults for its missing fields. */
	static constexpr int32 CurrentVersion = 2;
	static constexpr int32 MaxSlots = 8;
	static constexpr int32 MaxTeams = 4;

	UPROPERTY()
	int32 Version = CurrentVersion;

	/** Random seed of the match (Nomad placement, reproducible runs). Not shown in the menu. */
	UPROPERTY()
	int32 Seed = 0;

	UPROPERTY()
	FSoftObjectPath Map;

	/** Always MaxSlots after Normalize(). */
	UPROPERTY()
	TArray<FHOLMatchSlot> Slots;

	UPROPERTY()
	EHOLMatchMode Mode = EHOLMatchMode::Standard;

	/** Options of the Standard mode (D204, D205). */
	UPROPERTY()
	bool bNomad = false;

	UPROPERTY()
	bool bRegicide = false;

	UPROPERTY()
	bool bCatastrophe = false;

	/**
	 * IA1.1: Peasants each occupied slot gets at the start; -1 (no such field in the file, the menu, the lobby) = the
	 * rule of the game, UHOLMatchSettings::StartingPeasants. The test scenarios (hol.Scenario) read it from their .json,
	 * 0 when the field is missing. Not shown in the menu.
	 */
	UPROPERTY()
	int32 StartingPeasants = -1;

	/**
	 * J8a.2 (J8a-Q2): whether each occupied slot gets its hero (the Paladin-Commander) by its main forum at the start; -1
	 * (no such field in the file, the menu, the lobby) = the rule of the game, UHOLMatchSettings::bStartingHero; 0 none,
	 * 1 one. The test scenarios (hol.Scenario) read it from their .json ("startingHero": true), none when the field is
	 * missing. Not shown in the menu.
	 */
	UPROPERTY()
	int32 StartingHero = -1;

	bool operator==(const FHOLMatchConfig&) const = default;

	/** What runs without a configuration (stress test, BattleMap opened directly): one human in slot 0. */
	static FHOLMatchConfig MakeDefault();

	/** Eight slots exactly, colour and team brought back into range, current version. */
	void Normalize();

	/**
	 * E2-2: each occupied slot on HOLPlayerColors::RandomColor gets, in slot order, a colour no other occupied slot has
	 * (chosen or already drawn); a closed one the colour of its index. Before Normalize, which would clamp it.
	 */
	void DrawRandomColors(FRandomStream& Stream);

	/** Slots held by a human or an AI. */
	int32 GetNumOccupiedSlots() const;

	/** First human slot not in TakenSlots, or INDEX_NONE. */
	int32 FindFreeHumanSlot(TConstArrayView<int32> TakenSlots) const;

	/** True when the lobby manager named the human slots (R3.2); a configuration without names seats by arrival. */
	bool HasPlayerNames() const;

	/**
	 * The slot of a player who logs in under Name (R3.2, M6-Q8, M6-Q9). Without names in the configuration: the first free
	 * human slot. With names: the human slot of that name (case ignored, as the lobby manager), if not taken; an unknown,
	 * empty or taken name gets none (INDEX_NONE: a spectator).
	 */
	int32 FindSlotForName(const FString& Name, TConstArrayView<int32> TakenSlots) const;

	FString ToJson() const;

	/** Reads and normalises a configuration; fails on malformed JSON or a version newer than CurrentVersion. */
	static bool FromJson(const FString& Json, FHOLMatchConfig& OutConfig, FString* OutError = nullptr);
};
