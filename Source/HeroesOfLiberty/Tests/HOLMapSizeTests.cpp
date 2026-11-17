// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLMapSizeRules.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// TC1 checks (TC-Q3 to TC-Q8): the named map sizes, their places, the file of a map at a size, the draw of the start
// places ("Équipes ensemble") and the configuration fields that carry them.
namespace UE::HeroesOfLiberty::Tests::MapSize
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** What is wrong with a draw, or empty: one place per occupied slot, distinct, on the map, evenly spread, allies together. */
	FString CheckDraw(const TArray<FHOLMatchSlot>& Slots, const TArray<int32>& Places, const int32 NumPlaces, const bool bTeamsTogether)
	{
		if (Places.Num() != Slots.Num())
		{
			return FString::Printf(TEXT("%d places for %d slots"), Places.Num(), Slots.Num());
		}
		TMap<int32, int32> TeamOfPlace;	// place -> team key (a slot without a team is its own team)
		for (int32 Index = 0; Index < Slots.Num(); ++Index)
		{
			const bool bOccupied = Slots[Index].State != EHOLSlotState::Closed;
			if (!bOccupied)
			{
				if (Places[Index] != INDEX_NONE)
				{
					return FString::Printf(TEXT("closed slot %d at place %d"), Index, Places[Index]);
				}
				continue;
			}
			if (Places[Index] < 0 || Places[Index] >= NumPlaces)
			{
				return FString::Printf(TEXT("slot %d at place %d, off the %d places"), Index, Places[Index], NumPlaces);
			}
			if (TeamOfPlace.Contains(Places[Index]))
			{
				return FString::Printf(TEXT("place %d given twice"), Places[Index]);
			}
			TeamOfPlace.Add(Places[Index], Slots[Index].Team > 0 ? Slots[Index].Team : 100 + Index);
		}

		// Around the circle: the gaps between two neighbouring players differ by one at most.
		TArray<int32> Sorted;
		TeamOfPlace.GenerateKeyArray(Sorted);
		Sorted.Sort();
		const int32 Num = Sorted.Num();
		int32 MinGap = MAX_int32;
		int32 MaxGap = 0;
		for (int32 Index = 0; Index < Num; ++Index)
		{
			const int32 Gap = Index + 1 < Num ? Sorted[Index + 1] - Sorted[Index] : Sorted[0] + NumPlaces - Sorted[Index];
			MinGap = FMath::Min(MinGap, Gap);
			MaxGap = FMath::Max(MaxGap, Gap);
		}
		if (Num > 1 && MaxGap - MinGap > 1)
		{
			return FString::Printf(TEXT("uneven places (gaps %d to %d)"), MinGap, MaxGap);
		}

		// Allies together: each team starts one run around the circle.
		if (bTeamsTogether && Num > 1)
		{
			TMap<int32, int32> Runs;
			for (int32 Index = 0; Index < Num; ++Index)
			{
				const int32 Team = TeamOfPlace[Sorted[Index]];
				if (Team != TeamOfPlace[Sorted[(Index + Num - 1) % Num]])
				{
					++Runs.FindOrAdd(Team);
				}
			}
			for (const TPair<int32, int32>& Run : Runs)
			{
				if (Run.Value > 1)
				{
					return FString::Printf(TEXT("team %d split in %d groups"), Run.Key, Run.Value);
				}
			}
		}
		return FString();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeTableTest, "HeroesOfLiberty.MapSize.Table", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeTableTest::RunTest(const FString& Parameters)
{
	struct FExpected { EHOLMapSize Size; const TCHAR* Key; int32 Places; int32 Side; };
	const FExpected Expected[] = {
		{ EHOLMapSize::Small, TEXT("Small"), 2, 110 },
		{ EHOLMapSize::Medium, TEXT("Medium"), 4, 154 },
		{ EHOLMapSize::Normal, TEXT("Normal"), 6, 183 },
		{ EHOLMapSize::Large, TEXT("Large"), 8, 202 },
		{ EHOLMapSize::Huge, TEXT("Huge"), 8, 220 },
	};
	const TConstArrayView<FHOLMapSizeInfo> Sizes = FHOLMapSizeRules::GetSizes();
	TestEqual(TEXT("five sizes"), Sizes.Num(), static_cast<int32>(UE_ARRAY_COUNT(Expected)));
	const double TileSize = UHOLBuildingSettings::GetTileSize();
	for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(Expected)); ++Index)
	{
		const FExpected& Line = Expected[Index];
		TestEqual(FString::Printf(TEXT("%s places"), Line.Key), FHOLMapSizeRules::GetNumPlaces(Line.Size), Line.Places);
		TestEqual(FString::Printf(TEXT("%s side"), Line.Key), FHOLMapSizeRules::GetSideTiles(Line.Size), Line.Side);
		TestTrue(FString::Printf(TEXT("%s found by key"), Line.Key), FHOLMapSizeRules::FindByKey(Line.Key) == Line.Size);
		const FHOLMapSizeInfo* Info = FHOLMapSizeRules::Find(Line.Size);
		TestTrue(FString::Printf(TEXT("%s line"), Line.Key), Info && FCString::Strcmp(Info->Key, Line.Key) == 0);
		TestTrue(FString::Printf(TEXT("%s in order"), Line.Key), Sizes.IsValidIndex(Index) && Sizes[Index].Size == Line.Size);

		// D344: within the 220 tiles (880 m) around the origin.
		TestTrue(FString::Printf(TEXT("%s within MaxSideTiles"), Line.Key), FHOLMapSizeRules::GetSideTiles(Line.Size) <= AHOLMapBounds::MaxSideTiles);
		const double Half = 0.5 * FHOLMapSizeRules::GetSideTiles(Line.Size) * TileSize;
		TestTrue(FString::Printf(TEXT("%s fits the largest map"), Line.Key), Half > 0.0 && AHOLMapBounds::FitsMaxSize(FBox2D(FVector2D(-Half), FVector2D(Half))));
		TestTrue(FString::Printf(TEXT("%s within the 8 slots"), Line.Key), Line.Places <= FHOLMatchConfig::MaxSlots);
	}
	TestEqual(TEXT("no places without a size"), FHOLMapSizeRules::GetNumPlaces(EHOLMapSize::None), 0);
	TestNull(TEXT("no line without a size"), FHOLMapSizeRules::Find(EHOLMapSize::None));
	TestTrue(TEXT("unknown key"), FHOLMapSizeRules::FindByKey(TEXT("Gigantic")) == EHOLMapSize::None);
	TestTrue(TEXT("key case ignored"), FHOLMapSizeRules::FindByKey(TEXT("large")) == EHOLMapSize::Large);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeCsvTest, "HeroesOfLiberty.MapSize.Csv", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeCsvTest::RunTest(const FString& Parameters)
{
	// Tools/Data/MapSizes.csv (read by the map scripts, TC2) says what the code says.
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools/Data/MapSizes.csv"));
	TArray<FString> Lines;
	if (!TestTrue(FString::Printf(TEXT("CSV read (%s)"), *Path), FFileHelper::LoadFileToStringArray(Lines, *Path)))
	{
		return false;
	}
	Lines.RemoveAll([](const FString& Line) { return Line.TrimStartAndEnd().IsEmpty(); });
	TestEqual(TEXT("header and five lines"), Lines.Num(), 1 + FHOLMapSizeRules::GetSizes().Num());
	TestEqual(TEXT("header"), Lines.IsEmpty() ? FString() : Lines[0].TrimStartAndEnd(), FString(TEXT("Name,DisplayName,NumPlaces,SideTiles")));
	for (int32 Index = 1; Index < Lines.Num(); ++Index)
	{
		TArray<FString> Cells;
		Lines[Index].TrimStartAndEnd().ParseIntoArray(Cells, TEXT(","), false);
		if (!TestEqual(FString::Printf(TEXT("line %d cells"), Index), Cells.Num(), 4))
		{
			continue;
		}
		const EHOLMapSize Size = FHOLMapSizeRules::FindByKey(Cells[0]);
		TestTrue(FString::Printf(TEXT("line %d known size %s"), Index, *Cells[0]), Size != EHOLMapSize::None);
		TestTrue(FString::Printf(TEXT("line %d in order"), Index), FHOLMapSizeRules::GetSizes().IsValidIndex(Index - 1)
			&& FHOLMapSizeRules::GetSizes()[Index - 1].Size == Size);
		TestEqual(FString::Printf(TEXT("%s places"), *Cells[0]), FCString::Atoi(*Cells[2]), FHOLMapSizeRules::GetNumPlaces(Size));
		TestEqual(FString::Printf(TEXT("%s side"), *Cells[0]), FCString::Atoi(*Cells[3]), FHOLMapSizeRules::GetSideTiles(Size));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeConfigTest, "HeroesOfLiberty.MapSize.Config", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeConfigTest::RunTest(const FString& Parameters)
{
	// A file without the version 3 fields (scenarios, quick_check, the lobby manager's version 2): no size, fixed places.
	const FString Json = TEXT(R"({ "version": 2, "seed": 77, "map": "/Game/Maps/Plain.Plain",
		"slots": [ { "state": "Human", "team": 1 }, { "state": "Closed" }, { "state": "AI", "team": 2 }, { "state": "AI", "team": 1 } ] })");
	FHOLMatchConfig Read;
	FString Error;
	if (!TestTrue(FString::Printf(TEXT("reads a version 2 file (%s)"), *Error), FHOLMatchConfig::FromJson(Json, Read, &Error)))
	{
		return false;
	}
	TestTrue(TEXT("no size"), Read.MapSize == EHOLMapSize::None);
	TestFalse(TEXT("places not drawn"), Read.bDrawPlaces);
	TestTrue(TEXT("teams together by default"), Read.bTeamsTogether);
	TestEqual(TEXT("the map loaded as it is"), FHOLMapSizeRules::GetMapFile(Read.Map, Read.MapSize).ToString(), FString(TEXT("/Game/Maps/Plain.Plain")));
	const TArray<int32> Fixed = { 0, INDEX_NONE, 2, 3, INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE };
	TestTrue(TEXT("fixed places: slot i at place i"), FHOLMapSizeRules::GetPlaces(Read) == Fixed);
	TestTrue(TEXT("fixed places of the slots"), FHOLMapSizeRules::GetFixedPlaces(Read.Slots) == Fixed);
	TestEqual(TEXT("upgraded to version 4"), Read.Version, 4);
	TestFalse(TEXT("a version 2 file reads the sacred sites unticked (D699)"), Read.bSacredSites);

	// The new fields go through JSON.
	FHOLMatchConfig Written = Read;
	Written.MapSize = EHOLMapSize::Large;
	Written.bDrawPlaces = true;
	Written.bTeamsTogether = false;
	FHOLMatchConfig Again;
	TestTrue(TEXT("reads its own JSON"), FHOLMatchConfig::FromJson(Written.ToJson(), Again));
	TestTrue(TEXT("identical after a round trip"), Again == Written);
	TestTrue(TEXT("the size by name in the file"), Written.ToJson().Contains(TEXT("\"Large\"")));

	// Drawn: on the 8 places of Large, from the seed, the same as the draw itself.
	Written.bTeamsTogether = true;
	TestTrue(TEXT("drawn places of a Large map"), FHOLMapSizeRules::GetPlaces(Written) == FHOLMapSizeRules::DrawPlaces(77, Written.Slots, 8, true));
	TestTrue(TEXT("drawn places valid"), UE::HeroesOfLiberty::Tests::MapSize::CheckDraw(Written.Slots, FHOLMapSizeRules::GetPlaces(Written), 8, true).IsEmpty());
	Written.MapSize = EHOLMapSize::None;
	TestTrue(TEXT("drawn without a size: on the eight slots"), FHOLMapSizeRules::GetPlaces(Written) == FHOLMapSizeRules::DrawPlaces(77, Written.Slots, FHOLMatchConfig::MaxSlots, true));

	// The default configuration (no menu, stress test): fixed places.
	const FHOLMatchConfig Default = FHOLMatchConfig::MakeDefault();
	TestTrue(TEXT("default: no size, fixed places"), Default.MapSize == EHOLMapSize::None && !Default.bDrawPlaces && Default.bTeamsTogether);
	return true;
}

#endif
