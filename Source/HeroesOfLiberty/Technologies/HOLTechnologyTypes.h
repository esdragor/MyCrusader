// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "Match/HOLResources.h"
#include "HOLTechnologyTypes.generated.h"

/** The five categories of GDD 11: what a technology is about (economy, military statistics, infrastructure, hero, faction). */
UENUM()
enum class EHOLTechCategory : uint8
{
	Economy,
	/** Statistics and visuals only, never a replacement of a unit (D38); units come with the tier, never with a research (D180). */
	Military,
	Infrastructure,
	Hero,
	Faction,
};

/**
 * One technology (T1, GDD 11, D27): a row of /Game/Technologies/Technologies, imported from Tools/Data/Technologies.csv by
 * Tools/Editor/make_technologies.py, like the building and unit types. The three conditions of GDD 11 are three groups of
 * columns: the hero (MinTier, MinHeroLevel), the building (Building), the cost (Cost). The values are provisional by nature
 * (GDD 11.1: "indicative, to be set in test") and are balanced in the CSV.
 */
USTRUCT()
struct HEROESOFLIBERTY_API FHOLTechnologyRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Shown to the player and in the logs. */
	UPROPERTY(EditAnywhere, Category = "Display")
	FText DisplayName;

	UPROPERTY(EditAnywhere, Category = "Technology")
	EHOLTechCategory Category = EHOLTechCategory::Economy;

	/** `Common` (the ~70-80 % shared by every faction, D27) or the faction's key (`Dawn`, `Legions`...). */
	UPROPERTY(EditAnywhere, Category = "Technology")
	FName Faction = TEXT("Common");

	/** Condition 2, the building it is researched in (a row of the building type table, or one still to come). */
	UPROPERTY(EditAnywhere, Category = "Conditions")
	FName Building;

	/** Condition 1, the civilisation tier the hero must have opened (D04: tiers at levels 3, 6 and 9; 1 or 2 at the prototype). */
	UPROPERTY(EditAnywhere, Category = "Conditions", meta = (ClampMin = "0", ClampMax = "3"))
	int32 MinTier = 1;

	/** Condition 1, the hero level when the tier alone is not the rule (T-Q1): 0 = the tier only; else the stricter of the two holds. */
	UPROPERTY(EditAnywhere, Category = "Conditions", meta = (ClampMin = "0", ClampMax = "10"))
	int32 MinHeroLevel = 0;

	/** Condition 3, what starting the research costs. */
	UPROPERTY(EditAnywhere, Category = "Conditions")
	FHOLResources Cost;

	/** Research time (s). */
	UPROPERTY(EditAnywhere, Category = "Technology", meta = (ClampMin = "0", ForceUnits = "s"))
	float ResearchTime = 30.f;

	/** The technology to have first (level I of a line for its level II); none for a first level. */
	UPROPERTY(EditAnywhere, Category = "Technology")
	FName Requires;

	/** The statistics it changes (names of the statistics the effects code reads, T5), e.g. MeleeAttack, GatherWood. */
	UPROPERTY(EditAnywhere, Category = "Effect")
	TArray<FName> Stats;

	/** The change of those statistics, in percent of the base value, TOTAL of the line (level II = level I and more, T-Q2). */
	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = "0"))
	int32 Percent = 10;

	/**
	 * False while no code applies the effect (T-Q10 = B, T-Q11 = B, D601): the technology stays in the grid, greyed with "Soon", and
	 * no research of it starts. A row whose statistics the game reads (gathering, workers, melee and ranged attack) is true.
	 */
	UPROPERTY(EditAnywhere, Category = "Effect")
	bool Implemented = true;
};

/** Where the technology table lives (DefaultGame.ini), like the building type table. */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "HeroesOfLiberty Technologies"))
class HEROESOFLIBERTY_API UHOLTechnologySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Technology table (T1): one row per technology, imported from Tools/Data/Technologies.csv by Tools/Editor/make_technologies.py. */
	UPROPERTY(config, EditAnywhere, Category = "Technologies")
	TSoftObjectPtr<UDataTable> TechnologiesTable;

	/** The loaded table, or null (the asset is not created yet): the game then has no technology. */
	static const UDataTable* GetTable();

	/** A row of the table, or null. */
	static const FHOLTechnologyRow* FindTechnology(FName Name);

	/** The technologies researched in a building type (T4), in the order of the table: what its research row of the grid shows. */
	static void GetBuildingTechnologies(FName Building, TArray<FName>& OutNames);

	/** Same, for one faction (L2): without the technologies of other factions (the row's `Faction` column); a None key leaves all of them. */
	static void GetBuildingTechnologies(FName Building, FName FactionKey, TArray<FName>& OutNames);
};