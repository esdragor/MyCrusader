// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionRules.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/World.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "UObject/UnrealType.h"

namespace HOLFactionRules
{
	namespace
	{
		// The same figures as Tools/Data/Factions.csv (a packaged game has no Tools/Data); a test keeps the two equal.
		const TCHAR* BuiltInRoles = TEXT(
			"Name,Faction,Role,Type\n"
			"Dawn_Worker,Dawn,Worker,Peasant\n"
			"Dawn_Hero,Dawn,Hero,Hero\n"
			"Dawn_MainCentre,Dawn,MainCentre,MainCentre\n"
			"Legions_Worker,Legions,Worker,Peasant\n"
			"Legions_Hero,Legions,Hero,Hero\n"
			"Legions_MainCentre,Legions,MainCentre,MainCentre\n");
		const TCHAR* BuiltInOverrides = TEXT("Name,Faction,Type,Field,Value\n");

		TOptional<FTable> GameTable;
		const FTable* TestTable = nullptr;
		int32 TableVersion = 0;

		/** One CSV line into its cells (quotes, doubled quotes inside a quoted cell). */
		void SplitLine(const FString& Line, TArray<FString>& OutCells)
		{
			OutCells.Reset();
			FString Cell;
			bool bQuoted = false;
			for (int32 Index = 0; Index < Line.Len(); ++Index)
			{
				const TCHAR Char = Line[Index];
				if (bQuoted)
				{
					if (Char == TEXT('"'))
					{
						if (Index + 1 < Line.Len() && Line[Index + 1] == TEXT('"'))
						{
							Cell.AppendChar(TEXT('"'));
							++Index;
						}
						else
						{
							bQuoted = false;
						}
					}
					else
					{
						Cell.AppendChar(Char);
					}
				}
				else if (Char == TEXT('"'))
				{
					bQuoted = true;
				}
				else if (Char == TEXT(','))
				{
					OutCells.Add(Cell);
					Cell.Reset();
				}
				else
				{
					Cell.AppendChar(Char);
				}
			}
			OutCells.Add(Cell);
		}

		/** The rows of a text under the expected header: cells of each, a problem for a wrong header, a wrong cell count, an empty or repeated name. */
		void ReadRows(const FString& Text, const TCHAR* Label, const TArray<FString>& Header, TArray<TArray<FString>>& OutRows, TArray<FString>& OutProblems)
		{
			TArray<FString> Lines;
			FString Clean = Text;
			Clean.RemoveFromStart(TEXT("\xFEFF"));
			Clean.ReplaceInline(TEXT("\r"), TEXT(""));
			Clean.ParseIntoArray(Lines, TEXT("\n"), /*bCullEmpty*/ true);
			if (Lines.IsEmpty())
			{
				OutProblems.Add(FString::Printf(TEXT("%s: empty"), Label));
				return;
			}
			TArray<FString> Cells;
			SplitLine(Lines[0], Cells);
			bool bHeaderOk = Cells.Num() == Header.Num();
			for (int32 Index = 0; bHeaderOk && Index < Header.Num(); ++Index)
			{
				bHeaderOk = Cells[Index].TrimStartAndEnd() == Header[Index];
			}
			if (!bHeaderOk)
			{
				OutProblems.Add(FString::Printf(TEXT("%s: the header is not %s..."), Label, *Header[0]));
				return;
			}
			TSet<FString> Names;
			for (int32 Line = 1; Line < Lines.Num(); ++Line)
			{
				SplitLine(Lines[Line], Cells);
				if (Cells.Num() != Header.Num())
				{
					OutProblems.Add(FString::Printf(TEXT("%s line %d: %d cells, %d expected"), Label, Line + 1, Cells.Num(), Header.Num()));
					continue;
				}
				for (FString& Cell : Cells)
				{
					Cell.TrimStartAndEndInline();
				}
				if (Cells[0].IsEmpty())
				{
					OutProblems.Add(FString::Printf(TEXT("%s line %d: no name"), Label, Line + 1));
					continue;
				}
				if (Names.Contains(Cells[0]))
				{
					OutProblems.Add(FString::Printf(TEXT("%s line %d: the name %s is there twice"), Label, Line + 1, *Cells[0]));
					continue;
				}
				Names.Add(Cells[0]);
				OutRows.Add(Cells);
			}
		}

		bool IsKnownRole(const FName Role)
		{
			return Role == Roles::Worker || Role == Roles::Hero || Role == Roles::MainCentre;
		}

		FTable Load()
		{
			FTable Table;
			const FString Folder = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"));
			FString RolesText;
			FString OverridesText;
			const bool bFiles = FFileHelper::LoadFileToString(RolesText, *FPaths::Combine(Folder, TEXT("Factions.csv")))
				&& FFileHelper::LoadFileToString(OverridesText, *FPaths::Combine(Folder, TEXT("FactionOverrides.csv")));
			TArray<FString> Problems;
			if (bFiles)
			{
				ParseRoles(RolesText, Table.Roles, Problems);
				ParseOverrides(OverridesText, Table.Overrides, Problems);
			}
			if (!bFiles || !Problems.IsEmpty() || Table.Roles.IsEmpty())
			{
				if (!Problems.IsEmpty())
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("Faction tables of Tools/Data not used (%s): the built-in ones instead"), *Problems[0]);
				}
				Table = FTable();
				Problems.Reset();
				ParseRoles(BuiltInRoles, Table.Roles, Problems);
				ParseOverrides(BuiltInOverrides, Table.Overrides, Problems);
			}
			return Table;
		}
	}

	bool FTable::HasFaction(const FName Key) const
	{
		return Roles.ContainsByPredicate([Key](const FRoleRow& Row) { return Row.Faction == Key; });
	}

	FName FTable::ResolveKey(const FName Key) const
	{
		return HasFaction(Key) ? Key : Dawn;
	}

	FName FTable::FindRoleType(const FName Key, const FName Role) const
	{
		for (const FName Wanted : { ResolveKey(Key), Dawn })
		{
			for (const FRoleRow& Row : Roles)
			{
				if (Row.Faction == Wanted && Row.Role == Role)
				{
					return Row.Type;
				}
			}
		}
		return NAME_None;
	}

	bool FTable::Allows(const FName ColumnValue, const FName Key) const
	{
		if (ColumnValue.IsNone() || ColumnValue == Common)
		{
			return true;
		}
		TArray<FString> Keys;
		ColumnValue.ToString().ParseIntoArray(Keys, TEXT(";"), /*bCullEmpty*/ true);
		const FName Resolved = ResolveKey(Key);
		return Keys.ContainsByPredicate([Resolved](const FString& Each) { return FName(*Each.TrimStartAndEnd()) == Resolved; });
	}

	bool FTable::HasOverride(const FName Key, const FName Type) const
	{
		return Overrides.ContainsByPredicate([Key, Type](const FOverrideRow& Row) { return Row.Faction == Key && Row.Type == Type; });
	}

	int32 FTable::ApplyOverrides(const FName Key, const FName Type, const UStruct* Struct, void* Row, TArray<FString>* OutProblems) const
	{
		int32 Applied = 0;
		for (const FOverrideRow& Each : Overrides)
		{
			if (Each.Faction != Key || Each.Type != Type)
			{
				continue;
			}
			FProperty* Property = Struct ? Struct->FindPropertyByName(Each.Field) : nullptr;
			if (!Property)
			{
				if (OutProblems)
				{
					OutProblems->Add(FString::Printf(TEXT("%s: the type %s has no field %s"), *Each.Name.ToString(), *Type.ToString(), *Each.Field.ToString()));
				}
				continue;
			}
			void* Where = Property->ContainerPtrToValuePtr<void>(Row);
			if (!Property->ImportText_Direct(*Each.Value, Where, nullptr, PPF_None))
			{
				if (OutProblems)
				{
					OutProblems->Add(FString::Printf(TEXT("%s: '%s' does not read as %s"), *Each.Name.ToString(), *Each.Value, *Each.Field.ToString()));
				}
				continue;
			}
			++Applied;
		}
		return Applied;
	}

	void ParseRoles(const FString& Text, TArray<FRoleRow>& OutRows, TArray<FString>& OutProblems)
	{
		TArray<TArray<FString>> Rows;
		ReadRows(Text, TEXT("Factions.csv"), TArray<FString>{ TEXT("Name"), TEXT("Faction"), TEXT("Role"), TEXT("Type") }, Rows, OutProblems);
		for (const TArray<FString>& Cells : Rows)
		{
			FRoleRow Row{ FName(*Cells[0]), FName(*Cells[1]), FName(*Cells[2]), FName(*Cells[3]) };
			if (Row.Faction.IsNone() || Row.Type.IsNone())
			{
				OutProblems.Add(FString::Printf(TEXT("Factions.csv: %s has no faction or no type"), *Cells[0]));
			}
			else if (!IsKnownRole(Row.Role))
			{
				OutProblems.Add(FString::Printf(TEXT("Factions.csv: %s has an unknown role %s (Worker, Hero, MainCentre)"), *Cells[0], *Cells[2]));
			}
			else if (OutRows.ContainsByPredicate([&Row](const FRoleRow& Other) { return Other.Faction == Row.Faction && Other.Role == Row.Role; }))
			{
				OutProblems.Add(FString::Printf(TEXT("Factions.csv: %s repeats the role %s of faction %s"), *Cells[0], *Cells[2], *Cells[1]));
			}
			else
			{
				OutRows.Add(Row);
			}
		}
	}

	void ParseOverrides(const FString& Text, TArray<FOverrideRow>& OutRows, TArray<FString>& OutProblems)
	{
		TArray<TArray<FString>> Rows;
		ReadRows(Text, TEXT("FactionOverrides.csv"), TArray<FString>{ TEXT("Name"), TEXT("Faction"), TEXT("Type"), TEXT("Field"), TEXT("Value") }, Rows, OutProblems);
		for (const TArray<FString>& Cells : Rows)
		{
			FOverrideRow Row{ FName(*Cells[0]), FName(*Cells[1]), FName(*Cells[2]), FName(*Cells[3]), Cells[4] };
			if (Row.Faction.IsNone() || Row.Type.IsNone() || Row.Field.IsNone())
			{
				OutProblems.Add(FString::Printf(TEXT("FactionOverrides.csv: %s has no faction, type or field"), *Cells[0]));
			}
			else if (OutRows.ContainsByPredicate([&Row](const FOverrideRow& Other)
				{ return Other.Faction == Row.Faction && Other.Type == Row.Type && Other.Field == Row.Field; }))
			{
				OutProblems.Add(FString::Printf(TEXT("FactionOverrides.csv: %s repeats the field %s of %s for faction %s"), *Cells[0], *Cells[3], *Cells[2], *Cells[1]));
			}
			else
			{
				OutRows.Add(Row);
			}
		}
	}

	const FTable& GetTable()
	{
		if (TestTable)
		{
			return *TestTable;
		}
		if (!GameTable.IsSet())
		{
			GameTable = Load();
		}
		return GameTable.GetValue();
	}

	FTable GetBuiltInTable()
	{
		FTable Table;
		TArray<FString> Problems;
		ParseRoles(BuiltInRoles, Table.Roles, Problems);
		ParseOverrides(BuiltInOverrides, Table.Overrides, Problems);
		return Table;
	}

	void SetTableForTests(const FTable* Table)
	{
		TestTable = Table;
		++TableVersion;
	}

	int32 GetTableVersion()
	{
		return TableVersion;
	}

	FName GetFactionKey(const EHOLFaction Faction)
	{
		switch (Faction)
		{
		case EHOLFaction::DawnOrder: return Dawn;
		case EHOLFaction::BlackLegions: return FName(TEXT("Legions"));
		case EHOLFaction::DragonChildren: return FName(TEXT("Dragon"));
		case EHOLFaction::ShadowCircle: return FName(TEXT("Shadow"));
		case EHOLFaction::FireHeirs: return FName(TEXT("Fire"));
		case EHOLFaction::Templars: return FName(TEXT("Templars"));
		}
		return Dawn;
	}

	FName GetSlotFactionKey(const FHOLMatchConfig& Config, const int32 Slot)
	{
		return Config.Slots.IsValidIndex(Slot) ? GetFactionKey(Config.Slots[Slot].Faction) : Dawn;
	}

	FName GetSlotFactionKey(const UWorld* World, const int32 Slot)
	{
		const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
		return GameState ? GetSlotFactionKey(GameState->GetLaunchConfig(), Slot) : Dawn;
	}

	FName GetDefaultRoleType(const FName Role)
	{
		return Role == Roles::Worker ? FName(TEXT("Peasant")) : Role == Roles::Hero ? FName(TEXT("Hero")) : FName(TEXT("MainCentre"));
	}

	FName GetRoleTypeName(const FHOLMatchConfig& Config, const int32 Slot, const FName Role)
	{
		const FName Type = GetTable().FindRoleType(GetSlotFactionKey(Config, Slot), Role);
		return Type.IsNone() ? GetDefaultRoleType(Role) : Type;
	}

	FName GetRoleTypeName(const UWorld* World, const int32 Slot, const FName Role)
	{
		const FName Type = GetTable().FindRoleType(GetSlotFactionKey(World, Slot), Role);
		return Type.IsNone() ? GetDefaultRoleType(Role) : Type;
	}

	bool IsUnitOpenTo(const UWorld* World, const FName UnitType, const int32 Slot)
	{
		const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		const FHOLUnitTypeRow* Row = Units ? Units->FindTypeRow(UnitType) : nullptr;
		return !Row || GetTable().Allows(Row->Faction, GetSlotFactionKey(World, Slot));
	}

	bool IsBuildingOpenTo(const UWorld* World, const FName BuildingType, const int32 Slot)
	{
		const FHOLBuildingTypeRow* Row = BuildingType.IsNone() ? nullptr : UHOLBuildingSettings::FindType(BuildingType);
		return !Row || GetTable().Allows(Row->Faction, GetSlotFactionKey(World, Slot));
	}

	TArray<FName> FilterProduces(const UWorld* World, const TConstArrayView<FName> Produces, const int32 Slot)
	{
		TArray<FName> Open;
		Open.Reserve(Produces.Num());
		for (const FName Type : Produces)
		{
			if (IsUnitOpenTo(World, Type, Slot))
			{
				Open.Add(Type);
			}
		}
		return Open;
	}
}
