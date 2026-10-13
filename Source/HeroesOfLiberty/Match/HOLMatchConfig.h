// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
	static constexpr int32 CurrentVersion = 1;
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

	bool operator==(const FHOLMatchConfig&) const = default;

	/** What runs without a configuration (stress test, BattleMap opened directly): one human in slot 0. */
	static FHOLMatchConfig MakeDefault();

	/** Eight slots exactly, colour and team brought back into range, current version. */
	void Normalize();

	/** Slots held by a human or an AI. */
	int32 GetNumOccupiedSlots() const;

	/** First human slot not in TakenSlots, or INDEX_NONE. */
	int32 FindFreeHumanSlot(TConstArrayView<int32> TakenSlots) const;

	FString ToJson() const;

	/** Reads and normalises a configuration; fails on malformed JSON or a version newer than CurrentVersion. */
	static bool FromJson(const FString& Json, FHOLMatchConfig& OutConfig, FString* OutError = nullptr);
};
