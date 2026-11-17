// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionRulesReadLibrary.h"

#include "Match/HOLFactionRules.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLStartRules.h"
#include "Simulation/HOLUnitTypes.h"
#include "UObject/UnrealType.h"

namespace
{
	/** The table the text of the roles (and of the overrides) makes, by the real parsers. */
	HOLFactionRules::FTable MakeTable(const FString& RolesText, const FString& OverridesText, TArray<FString>* OutProblems = nullptr)
	{
		HOLFactionRules::FTable Table;
		TArray<FString> Problems;
		HOLFactionRules::ParseRoles(RolesText, Table.Roles, Problems);
		HOLFactionRules::ParseOverrides(OverridesText, Table.Overrides, Problems);
		if (OutProblems)
		{
			*OutProblems = Problems;
		}
		return Table;
	}

	/** Holds a test table in place for the life of the object, hands the game's back after it. */
	struct FScopedTable
	{
		explicit FScopedTable(const HOLFactionRules::FTable& Table) { HOLFactionRules::SetTableForTests(&Table); }
		~FScopedTable() { HOLFactionRules::SetTableForTests(nullptr); }
	};

	bool SameOverride(const HOLFactionRules::FOverrideRow& A, const HOLFactionRules::FOverrideRow& B)
	{
		return A.Name == B.Name && A.Faction == B.Faction && A.Type == B.Type && A.Field == B.Field && A.Value == B.Value;
	}

	/** A table of overrides only (the roles are not read by ApplyOverrides), and a default unit type row it is applied to. */
	int32 ApplyToDefaultRow(const FString& OverridesText, const FString& Faction, const FString& Type, FHOLUnitTypeRow& OutRow, TArray<FString>& OutProblems)
	{
		const HOLFactionRules::FTable Table = MakeTable(FString(), OverridesText);
		return Table.ApplyOverrides(FName(*Faction), FName(*Type), FHOLUnitTypeRow::StaticStruct(), &OutRow, &OutProblems);
	}
}

int32 UHOLFactionRulesReadLibrary::CountRoleRows(const FString& RolesText)
{
	TArray<HOLFactionRules::FRoleRow> Rows;
	TArray<FString> Problems;
	HOLFactionRules::ParseRoles(RolesText, Rows, Problems);
	return Rows.Num();
}

int32 UHOLFactionRulesReadLibrary::CountRoleProblems(const FString& RolesText)
{
	TArray<HOLFactionRules::FRoleRow> Rows;
	TArray<FString> Problems;
	HOLFactionRules::ParseRoles(RolesText, Rows, Problems);
	return Problems.Num();
}

FString UHOLFactionRulesReadLibrary::GetRoleCell(const FString& RolesText, const int32 RowIndex, const FString& Column)
{
	TArray<HOLFactionRules::FRoleRow> Rows;
	TArray<FString> Problems;
	HOLFactionRules::ParseRoles(RolesText, Rows, Problems);
	if (!Rows.IsValidIndex(RowIndex))
	{
		return FString();
	}
	const HOLFactionRules::FRoleRow& Row = Rows[RowIndex];
	if (Column == TEXT("Name"))
	{
		return Row.Name.ToString();
	}
	if (Column == TEXT("Faction"))
	{
		return Row.Faction.ToString();
	}
	if (Column == TEXT("Role"))
	{
		return Row.Role.ToString();
	}
	if (Column == TEXT("Type"))
	{
		return Row.Type.ToString();
	}
	return FString();
}

int32 UHOLFactionRulesReadLibrary::CountOverrideRows(const FString& OverridesText)
{
	TArray<HOLFactionRules::FOverrideRow> Rows;
	TArray<FString> Problems;
	HOLFactionRules::ParseOverrides(OverridesText, Rows, Problems);
	return Rows.Num();
}

int32 UHOLFactionRulesReadLibrary::CountOverrideProblems(const FString& OverridesText)
{
	TArray<HOLFactionRules::FOverrideRow> Rows;
	TArray<FString> Problems;
	HOLFactionRules::ParseOverrides(OverridesText, Rows, Problems);
	return Problems.Num();
}

FString UHOLFactionRulesReadLibrary::GetOverrideCell(const FString& OverridesText, const int32 RowIndex, const FString& Column)
{
	TArray<HOLFactionRules::FOverrideRow> Rows;
	TArray<FString> Problems;
	HOLFactionRules::ParseOverrides(OverridesText, Rows, Problems);
	if (!Rows.IsValidIndex(RowIndex))
	{
		return FString();
	}
	const HOLFactionRules::FOverrideRow& Row = Rows[RowIndex];
	if (Column == TEXT("Name"))
	{
		return Row.Name.ToString();
	}
	if (Column == TEXT("Faction"))
	{
		return Row.Faction.ToString();
	}
	if (Column == TEXT("Type"))
	{
		return Row.Type.ToString();
	}
	if (Column == TEXT("Field"))
	{
		return Row.Field.ToString();
	}
	if (Column == TEXT("Value"))
	{
		return Row.Value;
	}
	return FString();
}

FString UHOLFactionRulesReadLibrary::FindRoleType(const FString& RolesText, const FString& Faction, const FString& Role)
{
	return MakeTable(RolesText, FString()).FindRoleType(FName(*Faction), FName(*Role)).ToString();
}

bool UHOLFactionRulesReadLibrary::HasFaction(const FString& RolesText, const FString& Faction)
{
	return MakeTable(RolesText, FString()).HasFaction(FName(*Faction));
}

FString UHOLFactionRulesReadLibrary::ResolveKey(const FString& RolesText, const FString& Faction)
{
	return MakeTable(RolesText, FString()).ResolveKey(FName(*Faction)).ToString();
}

bool UHOLFactionRulesReadLibrary::Allows(const FString& RolesText, const FString& ColumnValue, const FString& Faction)
{
	return MakeTable(RolesText, FString()).Allows(FName(*ColumnValue), FName(*Faction));
}

bool UHOLFactionRulesReadLibrary::HasOverride(const FString& RolesText, const FString& OverridesText, const FString& Faction, const FString& Type)
{
	return MakeTable(RolesText, OverridesText).HasOverride(FName(*Faction), FName(*Type));
}

int32 UHOLFactionRulesReadLibrary::ApplyOverridesToUnitRow(const FString& OverridesText, const FString& Faction, const FString& Type)
{
	FHOLUnitTypeRow Row;
	TArray<FString> Problems;
	return ApplyToDefaultRow(OverridesText, Faction, Type, Row, Problems);
}

int32 UHOLFactionRulesReadLibrary::CountApplyOverridesProblems(const FString& OverridesText, const FString& Faction, const FString& Type)
{
	FHOLUnitTypeRow Row;
	TArray<FString> Problems;
	ApplyToDefaultRow(OverridesText, Faction, Type, Row, Problems);
	return Problems.Num();
}

int32 UHOLFactionRulesReadLibrary::ApplyOverridesToUnitRowReadInt(const FString& OverridesText, const FString& Faction, const FString& Type, const FString& IntField)
{
	FHOLUnitTypeRow Row;
	TArray<FString> Problems;
	ApplyToDefaultRow(OverridesText, Faction, Type, Row, Problems);
	const FIntProperty* Property = FindFProperty<FIntProperty>(FHOLUnitTypeRow::StaticStruct(), *IntField);
	return Property ? Property->GetPropertyValue_InContainer(&Row) : 0;
}

int32 UHOLFactionRulesReadLibrary::CountBuiltInRoleRows()
{
	return HOLFactionRules::GetBuiltInTable().Roles.Num();
}

int32 UHOLFactionRulesReadLibrary::CountBuiltInOverrideRows()
{
	return HOLFactionRules::GetBuiltInTable().Overrides.Num();
}

FString UHOLFactionRulesReadLibrary::FindBuiltInRoleType(const FString& Faction, const FString& Role)
{
	return HOLFactionRules::GetBuiltInTable().FindRoleType(FName(*Faction), FName(*Role)).ToString();
}

bool UHOLFactionRulesReadLibrary::BuiltInHasOverrideRowOf(const FString& OverridesText, const int32 RowIndex)
{
	TArray<HOLFactionRules::FOverrideRow> Rows;
	TArray<FString> Problems;
	HOLFactionRules::ParseOverrides(OverridesText, Rows, Problems);
	if (!Rows.IsValidIndex(RowIndex))
	{
		return false;
	}
	const HOLFactionRules::FTable BuiltIn = HOLFactionRules::GetBuiltInTable();
	for (const HOLFactionRules::FOverrideRow& Each : BuiltIn.Overrides)
	{
		if (SameOverride(Each, Rows[RowIndex]))
		{
			return true;
		}
	}
	return false;
}

FString UHOLFactionRulesReadLibrary::GetSlotStartType(const FString& RolesText, const FString& OverridesText, const int32 Slot, const FString& Role)
{
	// The config of HOLFactionRulesTests.cpp (MakeConfig): slot 0 an AI of the Dawn, slot 1 an AI of the Legions, the rest closed.
	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
	{
		Config.Slots[Index].State = Index <= 1 ? EHOLSlotState::AI : EHOLSlotState::Closed;
		Config.Slots[Index].ColorIndex = Index;
		Config.Slots[Index].Faction = Index == 1 ? EHOLFaction::BlackLegions : EHOLFaction::DawnOrder;
	}
	const HOLFactionRules::FTable Table = MakeTable(RolesText, OverridesText);
	FScopedTable Scoped(Table);
	if (Role == TEXT("Worker"))
	{
		return FHOLStartRules::GetWorkerTypeName(Config, Slot).ToString();
	}
	if (Role == TEXT("Hero"))
	{
		return FHOLStartRules::GetHeroTypeName(Config, Slot).ToString();
	}
	if (Role == TEXT("MainCentre"))
	{
		return FHOLStartRules::GetMainCentreTypeName(Config, Slot).ToString();
	}
	return FString();
}
