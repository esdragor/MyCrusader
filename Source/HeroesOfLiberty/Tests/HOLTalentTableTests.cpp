// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// T2 (TalentsInProgress.md): Tools/Data/Talents.csv, the Dawn Order's talent grid (GDD 13.2, D111, D195): 8 rows (levels 2 to 9) of one
// talent per family, Guardian, Captain, Champion in this order (the column of the row, the family index of the script rules,
// HOLTalentRules.as). The rules (script) take the family index only; this table is what each choice is.
namespace UE::HeroesOfLiberty::Tests::TalentTable
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
	constexpr int32 FirstRow = 2;
	constexpr int32 RowCount = 8;
	constexpr int32 FamilyCount = 3;
	constexpr int32 CellCount = 7;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTalentTableCsvTest, "HeroesOfLiberty.Talents.Csv", UE::HeroesOfLiberty::Tests::TalentTable::Flags)
bool FHOLTalentTableCsvTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::TalentTable;
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools/Data/Talents.csv"));
	TArray<FString> Lines;
	if (!TestTrue(FString::Printf(TEXT("CSV read (%s)"), *Path), FFileHelper::LoadFileToStringArray(Lines, *Path)))
	{
		return false;
	}
	Lines.RemoveAll([](const FString& Line) { return Line.TrimStartAndEnd().IsEmpty(); });
	TestEqual(TEXT("header and 24 talents"), Lines.Num(), 1 + RowCount * FamilyCount);
	TestEqual(TEXT("header"), Lines.IsEmpty() ? FString() : Lines[0].TrimStartAndEnd(), FString(TEXT("Row,Family,Id,NameFr,Target,Effect,Decision")));

	const TCHAR* Families[FamilyCount] = { TEXT("Guardian"), TEXT("Captain"), TEXT("Champion") };
	TSet<FString> Ids;
	for (int32 Index = 1; Index < Lines.Num(); ++Index)
	{
		TArray<FString> Cells;
		Lines[Index].TrimStartAndEnd().ParseIntoArray(Cells, TEXT(","), false);
		if (!TestEqual(FString::Printf(TEXT("line %d cells"), Index), Cells.Num(), CellCount))
		{
			continue;
		}
		const int32 Position = Index - 1;
		TestEqual(FString::Printf(TEXT("line %d row"), Index), FCString::Atoi(*Cells[0]), FirstRow + Position / FamilyCount);
		TestEqual(FString::Printf(TEXT("line %d family"), Index), Cells[1], FString(Families[Position % FamilyCount]));
		TestFalse(FString::Printf(TEXT("line %d id %s is new"), Index, *Cells[2]), Cells[2].IsEmpty() || Ids.Contains(Cells[2]));
		Ids.Add(Cells[2]);
		TestFalse(FString::Printf(TEXT("line %d has a name"), Index), Cells[3].IsEmpty());
		TestFalse(FString::Printf(TEXT("line %d has a target"), Index), Cells[4].IsEmpty());
		TestFalse(FString::Printf(TEXT("line %d has an effect"), Index), Cells[5].IsEmpty());
		// Rows 2 to 6 come from the validated grid (D111), rows 7 to 9 from D195.
		TestEqual(FString::Printf(TEXT("line %d decision"), Index), Cells[6], FString(FCString::Atoi(*Cells[0]) <= 6 ? TEXT("D111") : TEXT("D195")));
	}
	TestEqual(TEXT("24 different ids"), Ids.Num(), RowCount * FamilyCount);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
