// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLFactionRulesReadLibrary.generated.h"

/**
 * The pure part of the faction tables (namespace HOLFactionRules, Match/HOLFactionRules.h, C++, no UFUNCTION there) for Angelscript
 * (namespace HOLFactionRulesRead there): the script test of the CSV parsers, FTable (FindRoleType, ResolveKey, HasFaction, Allows,
 * HasOverride, ApplyOverrides) and the built-in table (HOLFactionRulesTests.cpp, HeroesOfLiberty.Match.Factions.Tables). Wrappers
 * only, no rule here: FTable is not a script type, so every call takes the CSV TEXT of the roles (and of the overrides) and builds
 * the table with the real parsers, then asks the real FTable. Faction, role and type are plain names (FString).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLFactionRulesReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** ParseRoles: the number of rows kept. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 CountRoleRows(const FString& RolesText);

	/** ParseRoles: the number of problems (bad header, wrong cell count, duplicate, unknown role, empty type). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 CountRoleProblems(const FString& RolesText);

	/** ParseRoles: the cell of a kept row; Column is Name, Faction, Role or Type (empty when the row or the column does not exist). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static FString GetRoleCell(const FString& RolesText, int32 RowIndex, const FString& Column);

	/** ParseOverrides: the number of rows kept. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 CountOverrideRows(const FString& OverridesText);

	/** ParseOverrides: the number of problems (bad header, wrong cell count, duplicate, empty field). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 CountOverrideProblems(const FString& OverridesText);

	/** ParseOverrides: the cell of a kept row; Column is Name, Faction, Type, Field or Value (empty when the row or the column does not exist). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static FString GetOverrideCell(const FString& OverridesText, int32 RowIndex, const FString& Column);

	/** FTable::FindRoleType on the table read from the text: the type name, or "None". */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static FString FindRoleType(const FString& RolesText, const FString& Faction, const FString& Role);

	/** FTable::HasFaction on the table read from the text. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static bool HasFaction(const FString& RolesText, const FString& Faction);

	/** FTable::ResolveKey on the table read from the text: the faction's own key, else "Dawn". */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static FString ResolveKey(const FString& RolesText, const FString& Faction);

	/** FTable::Allows on the table read from the text: whether a Faction column value (`Common`, or keys joined by ';') includes the faction. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static bool Allows(const FString& RolesText, const FString& ColumnValue, const FString& Faction);

	/** FTable::HasOverride on the tables read from the texts. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static bool HasOverride(const FString& RolesText, const FString& OverridesText, const FString& Faction, const FString& Type);

	/** FTable::ApplyOverrides on a default unit type row (FHOLUnitTypeRow): how many overrides were written. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 ApplyOverridesToUnitRow(const FString& OverridesText, const FString& Faction, const FString& Type);

	/** FTable::ApplyOverrides on a default unit type row: how many problems it reported (a field that does not exist, a value that does not read). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 CountApplyOverridesProblems(const FString& OverridesText, const FString& Faction, const FString& Type);

	/** FTable::ApplyOverrides on a default unit type row, then the value of one of its int32 fields (by name; 0 when it is not an int32 field). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 ApplyOverridesToUnitRowReadInt(const FString& OverridesText, const FString& Faction, const FString& Type, const FString& IntField);

	/** GetBuiltInTable: the number of role rows built in (what a packaged game reads). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 CountBuiltInRoleRows();

	/** GetBuiltInTable: the number of override rows built in. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static int32 CountBuiltInOverrideRows();

	/** GetBuiltInTable: FindRoleType of the built-in table (the type name, or "None"). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static FString FindBuiltInRoleType(const FString& Faction, const FString& Role);

	/** GetBuiltInTable: whether the built-in overrides hold a row equal (name, faction, type, field, value) to the row RowIndex read from the text. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static bool BuiltInHasOverrideRowOf(const FString& OverridesText, int32 RowIndex);

	/**
	 * The roles of a match through the real chain: FHOLStartRules::GetWorkerTypeName / GetHeroTypeName / GetMainCentreTypeName on a
	 * config of the test's shape (slot 0 an AI of the Dawn, slot 1 an AI of the Legions, the rest closed, Dawn by default), the tables
	 * read from the two texts and set for the call only (SetTableForTests, then handed back). Role is Worker, Hero or MainCentre.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|FactionRules")
	static FString GetSlotStartType(const FString& RolesText, const FString& OverridesText, int32 Slot, const FString& Role);
};
