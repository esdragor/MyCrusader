// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSiegeCheckLibrary.generated.h"

/** HOLSiege::EEngine (Siege/HOLSiegeRules.h) as a UENUM, same order: what Script/Siege/HOLSiegeRules.as numbered (MIG-S1c-Siege, ELROND-1117i). */
UENUM(BlueprintType)
enum class EHOLSiegeEngine : uint8
{
	Ram,
	Mangonel,
	Trebuchet,
	Cannon,
};

/** HOLSiege::EBlock as a UENUM, same order (the numbers HOLSiegeCheck::Check returns): Available is HOLSiege::EBlock::None. */
UENUM(BlueprintType)
enum class EHOLSiegeBlock : uint8
{
	Available,
	NotInPrototype,
	TierTooLow,
	NoWorkshop,
	CannotPay,
};

/**
 * MIG2-S4 (SAURON-1118n): the figures of a unit type row (FHOLUnitTypeRow, not BlueprintType) the script tests of Siege.Table need and
 * HOLAIOrder::ReadAIUnitFigures does not carry. A copy, read only: no rule. Attack types as flags (EHOLAttackType::Siege).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLSiegeUnitFigures
{
	GENERATED_BODY()

	/** The attack on units is a siege attack (Row->AttackType == EHOLAttackType::Siege). */
	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	bool bSiegeAttack = false;

	/** The attack on buildings is a siege attack (Row->BuildingAttackType == EHOLAttackType::Siege). */
	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	bool bBuildingSiegeAttack = false;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	int32 BuildingAttack = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	float BuildingAttackInterval = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	float BuildingAttackRange = 0.f;

	/** Row->AreaRadius (cm): the Mangonel's 250, 0 for every other type. */
	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	float AreaRadius = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	float SightRadius = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	float AttackRange = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	int32 MeleeArmor = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	int32 RangedArmor = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	int32 SiegeArmor = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	int32 HeroArmor = 0;

	/** Row->HasArmorCategory(EHOLArmorCategory::Siege). */
	UPROPERTY(BlueprintReadOnly, Category = "Siege")
	bool bHasSiegeCategory = false;
};

/**
 * The simulation's rule of the siege engines (HOLSiege, Siege/HOLSiegeRules.h, C++, the one the server applies) for Angelscript
 * (namespace HOLSiegeCheck there): D836 (2) keeps the rule in C++ and the AI in script reads it here, so that its own copy,
 * Script/Siege/HOLSiegeRules.as, can go (socle step 1 of the migration, D830). Wrappers only: no rule here. The numbers are the
 * script's: engine 0 Ram, 1 Mangonel, 2 Trebuchet, 3 Cannon (EHOLSiegeEngine); block 0 none, 1 not in the prototype, 2 tier too low,
 * 3 no workshop, 4 cannot pay (HOLSiege::Available...).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSiegeCheckLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLSiege::Check: why the engine cannot be queued now (block number above), 0 when nothing stops it; -1 for an engine out of range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static int32 Check(int32 Engine, int32 HeroTier, bool bWorkshopFinished, bool bCanPay);

	/** HOLSiege::FindEngine: the engine a unit type is, by the name of its row; false (OutEngine -1) for any other type. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static bool FindEngine(FName TypeName, int32& OutEngine);

	/** HOLSiege::DescribeBlock: the words of a block in the production's log ("tier too low"). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static FString DescribeBlock(int32 Block);

	/** HOLSiege::GetTier: the tier an engine opens at; 0 for an engine out of range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static int32 GetTier(int32 Engine);

	/** HOLSiege::InPrototype: the Ram and the Mangonel (D77); false for an engine out of range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static bool InPrototype(int32 Engine);

	/** HOLSiege::GetHealthStat: the technology statistic that raises this type's health, NAME_None for any other type. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static FName GetHealthStat(FName TypeName);

	/** HOLSiege::GetDamageStat: the technology statistic that raises this type's damage, NAME_None for any other type. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static FName GetDamageStat(FName TypeName);

	/** The number of engines (EHOLSiegeEngine: Ram to Cannon, 4). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static int32 GetEngineCount();

	/** HOLCombat::GetMaxHealth at level 1: the health with the percent the player's technologies add (400 + 10 % = 440), rounded once; a negative percent is no loss. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static int32 HealthWithTechnology(int32 Health, int32 HealthPercent);

	/** HOLCombat::GetDamage of one hit at level 1 against an unarmoured, bonus-free defender: the attack alone with the technologies' percent (45 + 20 % = 54), 1 at least as in the combat; a negative percent is no loss. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static float AttackWithTechnology(int32 Attack, int32 AttackPercent);

	/** HOLCombatProcessor's FSeeker::bCanAttackUnits (S4, SQ1 = B): a unit takes units on by itself only with an attack on units above 0 (the Ram does not). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static bool TakesUnitsOn(int32 Attack);

	/** HOLCombat::IsInArea: whether Other lies within Radius cm of Centre (centre to centre, the circle's edge in). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static bool IsInArea(FVector2D Centre, FVector2D Other, float Radius);

	/** MIG2-S4: a copy of the figures of this unit type (the real unit type table, UHOLSimulationSettings::UnitTypesTable); false (OutFigures untouched) when the table or the row is missing. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Siege")
	static bool ReadUnitFigures(FName TypeName, FHOLSiegeUnitFigures& OutFigures);

	/** MIG2-S4: how many rows of the unit type table OTHER than Except have an AreaRadius above 0 (the sweep "only the Mangonel has an area": 0 expected); -1 when the table is missing. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Siege")
	static int32 CountAreaRowsExcept(FName Except);

	/** MIG2-S4: UHOLBuildingSettings::GetTileSize (cm), the grid the buildings' footprints are whole tiles of. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static float GetTileSize();

	/** MIG2-S4: whether the real technology table (UHOLTechnologySettings::FindTechnology) has this technology and its Stats contain Stat. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Siege")
	static bool TechnologyChangesStat(FName Technology, FName Stat);

	/**
	 * MIG2-S4: HOLCombat::ChooseTarget for ONE enemy candidate at CandidatePosition seen by a seeker at the origin (wrapper, no rule): the
	 * index (0) of the candidate, -1 (INDEX_NONE) for nobody. SeekMode: 0 Auto, 1 AttackMove, 2 Hold (HOLCombat::ESeekMode).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static int32 ChooseSingleTarget(int32 SeekMode, bool bCanAttackUnits, float AttackRange, FVector2D CandidatePosition);
};
