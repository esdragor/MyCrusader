// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCampTable.h"

namespace
{
	/** The non-empty, trimmed lines of a CSV text (Windows or Unix line ends). */
	TArray<FString> DataLines(const FString& Text)
	{
		TArray<FString> Lines;
		Text.ParseIntoArrayLines(Lines, false);
		for (FString& Line : Lines)
		{
			Line.TrimStartAndEndInline();
		}
		Lines.RemoveAll([](const FString& Line) { return Line.IsEmpty(); });
		return Lines;
	}
}

void HOLCampTable::Parse(const FString& Text, TArray<FCampDef>& OutCamps, TArray<FString>& OutProblems)
{
	const TArray<FString> Lines = DataLines(Text);
	if (Lines.IsEmpty() || Lines[0] != TEXT("Name,DisplayName,Members"))
	{
		OutProblems.Add(TEXT("the header must be Name,DisplayName,Members"));
		return;
	}
	TSet<FName> Seen;
	for (int32 Index = 1; Index < Lines.Num(); ++Index)
	{
		TArray<FString> Cells;
		Lines[Index].ParseIntoArray(Cells, TEXT(","), false);
		if (Cells.Num() != 3 || Cells[0].IsEmpty())
		{
			OutProblems.Add(FString::Printf(TEXT("line %d: 3 cells and a name expected, got '%s'"), Index + 1, *Lines[Index]));
			continue;
		}
		FCampDef Camp;
		Camp.Name = FName(*Cells[0]);
		Camp.DisplayName = Cells[1];
		TArray<FString> Types;
		Cells[2].ParseIntoArray(Types, TEXT(";"), false);
		bool bEmptyMember = Types.IsEmpty();
		for (const FString& Type : Types)
		{
			bEmptyMember |= Type.TrimStartAndEnd().IsEmpty();
			Camp.Members.Add(FName(*Type.TrimStartAndEnd()));
		}
		if (bEmptyMember)
		{
			OutProblems.Add(FString::Printf(TEXT("line %d: camp %s has an empty member"), Index + 1, *Cells[0]));
			continue;
		}
		if (Seen.Contains(Camp.Name))
		{
			OutProblems.Add(FString::Printf(TEXT("line %d: camp %s is listed twice"), Index + 1, *Cells[0]));
			continue;
		}
		Seen.Add(Camp.Name);
		OutCamps.Add(MoveTemp(Camp));
	}
}

TSet<FName> HOLCampTable::ReadTypeNames(const FString& UnitTypesText)
{
	TSet<FName> Names;
	const TArray<FString> Lines = DataLines(UnitTypesText);
	for (int32 Index = 1; Index < Lines.Num(); ++Index)
	{
		int32 Comma = INDEX_NONE;
		if (Lines[Index].FindChar(TEXT(','), Comma) && Comma > 0)
		{
			Names.Add(FName(*Lines[Index].Left(Comma)));
		}
	}
	return Names;
}

TArray<FName> HOLCampTable::UnknownMembers(const FCampDef& Camp, const TSet<FName>& KnownTypes)
{
	TArray<FName> Unknown;
	for (const FName Member : Camp.Members)
	{
		if (!KnownTypes.Contains(Member))
		{
			Unknown.AddUnique(Member);
		}
	}
	return Unknown;
}
