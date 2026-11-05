// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Match/HOLMatchConfig.h"

class UWorld;
struct FHOLMatchConfig;

/**
 * L0 (LegionsInProgress.md, LQ-7, D677): the multi-faction base. The types keep their names (Spearman, Archer...); what a
 * faction changes is read from two tables, on the model of the `Faction` column of the technologies:
 *  - Tools/Data/Factions.csv (Name, Faction, Role, Type): the unit or building type that plays a ROLE for a faction
 *    (Worker: Peasant for the Dawn, Zombie for the Legions later; Hero; MainCentre);
 *  - Tools/Data/FactionOverrides.csv (Name, Faction, Type, Field, Value): a field of a base type for one faction (its display
 *    name, its cost, its mesh...), the value written as the field's text (a cost: `(Food=0,Wood=30,Stone=0,Gold=0)`).
 * And the `Faction` column of UnitTypes.csv and BuildingTypes.csv (`Common`, or the keys joined by ';') filters what a faction
 * can produce, build or see in its grid. A faction without any row of its own (the four of the lobby that are not playable
 * yet) reads as the Dawn, so two Dawn slots, or a slot of a faction still empty, change nothing.
 *
 * The rules are pure (the tests feed them text); the tables are read once, from Tools/Data when the files are there (the
 * editor, the tests, a development game), else from the same figures built in (a packaged game).
 */
namespace HOLFactionRules
{
	/** The roles of Factions.csv. */
	namespace Roles
	{
		inline const FName Worker(TEXT("Worker"));
		inline const FName Hero(TEXT("Hero"));
		inline const FName MainCentre(TEXT("MainCentre"));
	}

	/** The key of the `Faction` columns that every faction reads. */
	inline const FName Common(TEXT("Common"));
	/** The key of the faction that is the reference: what a faction without a rule of its own reads as. */
	inline const FName Dawn(TEXT("Dawn"));

	/** One row of Factions.csv. */
	struct HEROESOFLIBERTY_API FRoleRow
	{
		FName Name;
		FName Faction;
		FName Role;
		FName Type;
	};

	/** One row of FactionOverrides.csv. */
	struct HEROESOFLIBERTY_API FOverrideRow
	{
		FName Name;
		FName Faction;
		FName Type;
		FName Field;
		FString Value;
	};

	/** Both tables, and the questions asked of them. */
	struct HEROESOFLIBERTY_API FTable
	{
		TArray<FRoleRow> Roles;
		TArray<FOverrideRow> Overrides;

		/** Whether the faction has rows of its own in the role table. */
		bool HasFaction(FName Key) const;

		/** The key the rules read for a faction: its own when it has rows, else the Dawn's. */
		FName ResolveKey(FName Key) const;

		/** The type that plays the role for the faction (its own row, else the Dawn's), or None. */
		FName FindRoleType(FName Key, FName Role) const;

		/** Whether a `Faction` column value (`Common`, or keys joined by ';') includes the faction. */
		bool Allows(FName ColumnValue, FName Key) const;

		/** Whether the faction overrides at least one field of the type. */
		bool HasOverride(FName Key, FName Type) const;

		/**
		 * Writes the faction's overrides of the type into Row (a row of the type table, of struct Struct), field by field, by the
		 * field's text. Returns how many were written; a field that does not exist or a value that does not read is a problem
		 * appended to OutProblems (and skipped).
		 */
		int32 ApplyOverrides(FName Key, FName Type, const UStruct* Struct, void* Row, TArray<FString>* OutProblems = nullptr) const;
	};

	/** Reads the text of Factions.csv. Problems (bad header, wrong cell count, duplicate name or (faction, role), unknown role, empty type) go to OutProblems. */
	HEROESOFLIBERTY_API void ParseRoles(const FString& Text, TArray<FRoleRow>& OutRows, TArray<FString>& OutProblems);

	/** Reads the text of FactionOverrides.csv. Problems (bad header, wrong cell count, duplicate name or (faction, type, field), empty field) go to OutProblems. */
	HEROESOFLIBERTY_API void ParseOverrides(const FString& Text, TArray<FOverrideRow>& OutRows, TArray<FString>& OutProblems);

	/** The tables of the game: Tools/Data when present and clean, else built in. Read once. */
	HEROESOFLIBERTY_API const FTable& GetTable();

	/** The figures built in (what a packaged game reads): a test keeps them equal to the files. */
	HEROESOFLIBERTY_API FTable GetBuiltInTable();

	/** A test's own tables, until it hands null back (the next read of GetTable is the game's again). */
	HEROESOFLIBERTY_API void SetTableForTests(const FTable* Table);

	/** Changes each time SetTableForTests swaps the tables: the caches of the rows built from them follow it. */
	HEROESOFLIBERTY_API int32 GetTableVersion();

	/** The key of a lobby faction in the tables (`Dawn`, `Legions`...). */
	HEROESOFLIBERTY_API FName GetFactionKey(EHOLFaction Faction);

	/** The key of the slot's faction: from the launch configuration (the game state, on the server and on the clients), the Dawn's without one. */
	HEROESOFLIBERTY_API FName GetSlotFactionKey(const UWorld* World, int32 Slot);
	HEROESOFLIBERTY_API FName GetSlotFactionKey(const FHOLMatchConfig& Config, int32 Slot);

	/** The type that plays the role for the slot (the table's, else the Dawn's figures: Peasant, Hero, MainCentre). */
	HEROESOFLIBERTY_API FName GetRoleTypeName(const FHOLMatchConfig& Config, int32 Slot, FName Role);
	HEROESOFLIBERTY_API FName GetRoleTypeName(const UWorld* World, int32 Slot, FName Role);

	/** The type of a role when no table says: the Dawn's. */
	HEROESOFLIBERTY_API FName GetDefaultRoleType(FName Role);

	/** Whether the slot's faction fields the unit type (the `Faction` column of the unit type table); a type the table does not know is left open. */
	HEROESOFLIBERTY_API bool IsUnitOpenTo(const UWorld* World, FName UnitType, int32 Slot);

	/** Whether the slot's faction builds the building type (the `Faction` column of the building type table); a type the table does not know is left open. */
	HEROESOFLIBERTY_API bool IsBuildingOpenTo(const UWorld* World, FName BuildingType, int32 Slot);

	/** A production list (a building's Produces) without the unit types the slot's faction does not field, the order kept (the grid's cells, D30). */
	HEROESOFLIBERTY_API TArray<FName> FilterProduces(const UWorld* World, TConstArrayView<FName> Produces, int32 Slot);
}
