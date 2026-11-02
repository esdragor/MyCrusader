// Copyright Epic Games, Inc. All Rights Reserved.

#include "Internationalization/Text.h"
#include "Map/HOLMapRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// C0.1 checks (Cartes): the table of the match maps, Tools/Data/Maps.csv mirrored in code (and in the manager, C0.2).
namespace UE::HeroesOfLiberty::Tests::MapRules
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The source string of a text (the French, the key's own text); empty for a text without one. */
	FString SourceOf(const FText& Text)
	{
		const FString* Source = FTextInspector::GetSourceString(Text);
		return Source ? *Source : FString();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapCsvTest, "HeroesOfLiberty.Map.Csv", UE::HeroesOfLiberty::Tests::MapRules::Flags)
bool FHOLMapCsvTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapRules;

	// Tools/Data/Maps.csv (read by the map scripts, mirrored by the manager) says what the code says.
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools/Data/Maps.csv"));
	TArray<FString> Lines;
	if (!TestTrue(FString::Printf(TEXT("CSV read (%s)"), *Path), FFileHelper::LoadFileToStringArray(Lines, *Path)))
	{
		return false;
	}
	Lines.RemoveAll([](const FString& Line) { return Line.TrimStartAndEnd().IsEmpty(); });
	const TConstArrayView<FHOLMapInfo> Maps = FHOLMapRules::GetMaps();
	TestTrue(TEXT("at least one map"), Maps.Num() >= 1);
	TestEqual(TEXT("header and one line per map"), Lines.Num(), 1 + Maps.Num());
	TestEqual(TEXT("header"), Lines.IsEmpty() ? FString() : Lines[0].TrimStartAndEnd(), FString(TEXT("Name,DisplayName,Recipe,Budget")));
	for (int32 Index = 1; Index < Lines.Num(); ++Index)
	{
		TArray<FString> Cells;
		Lines[Index].TrimStartAndEnd().ParseIntoArray(Cells, TEXT(","), false);
		if (!TestEqual(FString::Printf(TEXT("line %d cells"), Index), Cells.Num(), 4))
		{
			continue;
		}
		const FHOLMapInfo* Info = FHOLMapRules::FindByKey(Cells[0]);
		if (!TestNotNull(FString::Printf(TEXT("line %d known map %s"), Index, *Cells[0]), Info))
		{
			continue;
		}
		TestTrue(FString::Printf(TEXT("line %d in order"), Index), Maps.IsValidIndex(Index - 1) && &Maps[Index - 1] == Info);
		TestEqual(FString::Printf(TEXT("%s display name"), *Cells[0]), SourceOf(Info->DisplayName), Cells[1]);
		TestTrue(FString::Printf(TEXT("%s recipe, lower case"), *Cells[0]), !Cells[2].IsEmpty() && Cells[2] == Cells[2].ToLower());
		// C1b: the Budget column, `standard` (the map holds the per-player budget of Map.Table), `centre` (C3: the same, the gold in the
		// middle of the map), `dense` (C5, CQ7 = B: the same, twice the wood) or `none`.
		TestTrue(FString::Printf(TEXT("%s budget is standard, centre or none (%s)"), *Cells[0], *Cells[3]), Cells[3] == TEXT("standard") || Cells[3] == TEXT("centre") || Cells[3] == TEXT("dense") || Cells[3] == TEXT("none"));
		TestEqual(FString::Printf(TEXT("%s budget"), *Cells[0]), Info->bBudget, Cells[3] == TEXT("standard") || Cells[3] == TEXT("centre") || Cells[3] == TEXT("dense"));
		TestEqual(FString::Printf(TEXT("%s dense wood"), *Cells[0]), Info->bDenseWood, Cells[3] == TEXT("dense"));
		TestEqual(FString::Printf(TEXT("%s gold at the centre"), *Cells[0]), Info->bGoldAtCentre, Cells[3] == TEXT("centre"));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapRulesTest, "HeroesOfLiberty.Map.Rules", UE::HeroesOfLiberty::Tests::MapRules::Flags)
bool FHOLMapRulesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapRules;

	// Plain, the first map (the default of a new game), found by its key whatever the case.
	const TConstArrayView<FHOLMapInfo> Maps = FHOLMapRules::GetMaps();
	TestTrue(TEXT("Plain first"), !Maps.IsEmpty() && FCString::Strcmp(Maps[0].Key, TEXT("Plain")) == 0);
	const FHOLMapInfo* Plain = FHOLMapRules::FindByKey(TEXT("Plain"));
	TestNotNull(TEXT("Plain by key"), Plain);
	TestTrue(TEXT("key case ignored"), Plain && FHOLMapRules::FindByKey(TEXT("plain")) == Plain && FHOLMapRules::FindByKey(TEXT(" PLAIN ")) == Plain);
	TestNull(TEXT("unknown key"), FHOLMapRules::FindByKey(TEXT("Desert")));
	TestNull(TEXT("empty key"), FHOLMapRules::FindByKey(TEXT("")));
	TestEqual(TEXT("Plain's display name"), Plain ? SourceOf(Plain->DisplayName) : FString(), FString(TEXT("Plaine")));
	TestEqual(TEXT("Plain's display name key"), Plain ? FTextInspector::GetTextId(Plain->DisplayName).GetKey().ToString() : FString(), FString(TEXT("Map_Plain")));
	TestEqual(TEXT("Plain's display name namespace"), Plain ? FTextInspector::GetTextId(Plain->DisplayName).GetNamespace().ToString() : FString(), FString(TEXT("HOLMaps")));

	// La Steppe (C1), the second map: found by its key, its French name the source text of Map_Steppe.
	const FHOLMapInfo* Steppe = FHOLMapRules::FindByKey(TEXT("steppe"));
	TestTrue(TEXT("Steppe second"), Steppe && Maps.Num() >= 2 && &Maps[1] == Steppe);
	TestEqual(TEXT("Steppe's display name"), Steppe ? SourceOf(Steppe->DisplayName) : FString(), FString(TEXT("La Steppe")));
	TestEqual(TEXT("Steppe's display name key"), Steppe ? FTextInspector::GetTextId(Steppe->DisplayName).GetKey().ToString() : FString(), FString(TEXT("Map_Steppe")));
	TestTrue(TEXT("Steppe by file"), Steppe && FHOLMapRules::FindByAsset(FSoftObjectPath(TEXT("/Game/Maps/Steppe.Steppe"))) == Steppe);

	// The file of a map, and the map of a file: the sized files (TC-Q3) and a test map are not maps of the table.
	const FSoftObjectPath PlainPath(TEXT("/Game/Maps/Plain.Plain"));
	TestEqual(TEXT("Plain's file"), FHOLMapRules::GetAssetPath(TEXT("Plain")).ToString(), PlainPath.ToString());
	TestTrue(TEXT("Plain by file"), Plain && FHOLMapRules::FindByAsset(PlainPath) == Plain);
	TestTrue(TEXT("Plain by file, case ignored"), Plain && FHOLMapRules::FindByAsset(FSoftObjectPath(TEXT("/Game/Maps/plain.plain"))) == Plain);
	TestNull(TEXT("a sized file is not a map"), FHOLMapRules::FindByAsset(FSoftObjectPath(TEXT("/Game/Maps/Plain_Large.Plain_Large"))));
	TestNull(TEXT("a test map is not a map"), FHOLMapRules::FindByAsset(FSoftObjectPath(TEXT("/Game/Maps/Test.Test"))));
	TestNull(TEXT("no file, no map"), FHOLMapRules::FindByAsset(FSoftObjectPath()));

	// The name shown: the table's, or the asset name as it is.
	TestEqual(TEXT("Plain shown"), SourceOf(FHOLMapRules::GetDisplayName(PlainPath)), FString(TEXT("Plaine")));
	TestTrue(TEXT("Plain's name is the table's text"), Plain && FHOLMapRules::GetDisplayName(PlainPath).IdenticalTo(Plain->DisplayName));
	const FText Unknown = FHOLMapRules::GetDisplayName(FSoftObjectPath(TEXT("/Game/Maps/Test.Test")));
	TestEqual(TEXT("an unknown map shown by its asset name"), Unknown.ToString(), FString(TEXT("Test")));
	TestTrue(TEXT("an unknown map's name is not translated"), Unknown.IsCultureInvariant());
	TestEqual(TEXT("a sized file shown by its asset name"), FHOLMapRules::GetDisplayName(FSoftObjectPath(TEXT("/Game/Maps/Plain_Large.Plain_Large"))).ToString(), FString(TEXT("Plain_Large")));
	return true;
}

#endif
