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

	/** Eight slots: Teams[i] for slot i (0 = no team), the slots past Teams closed. */
	TArray<FHOLMatchSlot> MakeSlots(const TArray<int32>& Teams)
	{
		TArray<FHOLMatchSlot> Slots;
		Slots.SetNum(FHOLMatchConfig::MaxSlots);
		for (int32 Index = 0; Index < Teams.Num(); ++Index)
		{
			Slots[Index].State = Index % 2 ? EHOLSlotState::AI : EHOLSlotState::Human;
			Slots[Index].Team = Teams[Index];
		}
		return Slots;
	}

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

	/** Whether a draw keeps every team on neighbouring places (CheckDraw with bTeamsTogether). */
	bool AreTeamsTogether(const TArray<FHOLMatchSlot>& Slots, const TArray<int32>& Places, const int32 NumPlaces)
	{
		return CheckDraw(Slots, Places, NumPlaces, true).IsEmpty();
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeAllowedTest, "HeroesOfLiberty.MapSize.Allowed", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeAllowedTest::RunTest(const FString& Parameters)
{
	using enum EHOLMapSize;
	const TArray<EHOLMapSize> Allowed[] = {
		{ Small, Medium, Normal, Large, Huge },	// 1 player
		{ Small, Medium, Normal, Large, Huge },	// 2
		{ Medium, Normal, Large, Huge },		// 3
		{ Medium, Normal, Large, Huge },		// 4
		{ Normal, Large, Huge },				// 5
		{ Normal, Large, Huge },				// 6
		{ Large, Huge },						// 7
		{ Large, Huge },						// 8
	};
	const EHOLMapSize Default[] = { Small, Small, Medium, Medium, Normal, Normal, Large, Large };
	for (int32 Players = 1; Players <= 8; ++Players)
	{
		TestTrue(FString::Printf(TEXT("sizes allowed for %d players"), Players), FHOLMapSizeRules::GetAllowedSizes(Players) == Allowed[Players - 1]);
		TestTrue(FString::Printf(TEXT("default size for %d players"), Players), FHOLMapSizeRules::GetDefaultSize(Players) == Default[Players - 1]);
		for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
		{
			TestEqual(FString::Printf(TEXT("%s allowed for %d players"), Info.Key, Players), FHOLMapSizeRules::IsAllowed(Info.Size, Players),
				Allowed[Players - 1].Contains(Info.Size));
		}
	}
	TestTrue(TEXT("no size past eight players"), FHOLMapSizeRules::GetAllowedSizes(9).IsEmpty() && FHOLMapSizeRules::GetDefaultSize(9) == None);
	TestTrue(TEXT("nobody seated yet: the smallest"), FHOLMapSizeRules::GetDefaultSize(0) == Small);
	TestFalse(TEXT("None never allowed"), FHOLMapSizeRules::IsAllowed(None, 1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeMapFileTest, "HeroesOfLiberty.MapSize.MapFile", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeMapFileTest::RunTest(const FString& Parameters)
{
	const FSoftObjectPath Plain(TEXT("/Game/Maps/Plain.Plain"));
	TestEqual(TEXT("Plain at Large"), FHOLMapSizeRules::GetMapFile(Plain, EHOLMapSize::Large).ToString(), FString(TEXT("/Game/Maps/Plain_Large.Plain_Large")));
	TestEqual(TEXT("Plain at Small"), FHOLMapSizeRules::GetMapFile(Plain, EHOLMapSize::Small).ToString(), FString(TEXT("/Game/Maps/Plain_Small.Plain_Small")));
	TestEqual(TEXT("Plain at Huge"), FHOLMapSizeRules::GetMapFile(Plain, EHOLMapSize::Huge).ToString(), FString(TEXT("/Game/Maps/Plain_Huge.Plain_Huge")));
	TestEqual(TEXT("a map given without its object name"), FHOLMapSizeRules::GetMapFile(FSoftObjectPath(TEXT("/Game/Maps/Plain")), EHOLMapSize::Medium).ToString(),
		FString(TEXT("/Game/Maps/Plain_Medium.Plain_Medium")));
	TestEqual(TEXT("without a size, the map as it is"), FHOLMapSizeRules::GetMapFile(Plain, EHOLMapSize::None).ToString(), Plain.ToString());
	TestTrue(TEXT("no map, no file"), FHOLMapSizeRules::GetMapFile(FSoftObjectPath(), EHOLMapSize::Large).IsNull());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeDrawTest, "HeroesOfLiberty.MapSize.DrawPlaces", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeDrawTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSize;
	struct FCase { const TCHAR* Name; TArray<int32> Teams; int32 NumPlaces; };
	const FCase Cases[] = {
		{ TEXT("1 player on Small"), { 0 }, 2 },
		{ TEXT("1 player on Large"), { 0 }, 8 },
		{ TEXT("1v1 on Small"), { 0, 0 }, 2 },
		{ TEXT("2v2 on Medium"), { 1, 2, 1, 2 }, 4 },
		{ TEXT("2v2 on Large, with gaps"), { 1, 1, 2, 2 }, 8 },
		{ TEXT("3v3 on Normal"), { 1, 2, 1, 2, 1, 2 }, 6 },
		{ TEXT("3v3 on Large"), { 2, 2, 2, 1, 1, 1 }, 8 },
		{ TEXT("4v4 on Large"), { 1, 2, 1, 2, 1, 2, 1, 2 }, 8 },
		{ TEXT("2v2v2v2 on Huge"), { 1, 2, 3, 4, 4, 3, 2, 1 }, 8 },
		{ TEXT("free for all, 4 on Medium"), { 0, 0, 0, 0 }, 4 },
		{ TEXT("free for all, 5 on Large"), { 0, 0, 0, 0, 0 }, 8 },
		{ TEXT("a team and two alone on Normal"), { 1, 0, 1, 0 }, 6 },
	};
	for (const FCase& Case : Cases)
	{
		const TArray<FHOLMatchSlot> Slots = MakeSlots(Case.Teams);
		for (const bool bTogether : { true, false })
		{
			TSet<int32> FirstPlaces;
			bool bSplitOnce = false;
			for (int32 Seed = 0; Seed < 200; ++Seed)
			{
				const TArray<int32> Places = FHOLMapSizeRules::DrawPlaces(Seed, Slots, Case.NumPlaces, bTogether);
				const FString Error = CheckDraw(Slots, Places, Case.NumPlaces, bTogether);
				if (!TestTrue(FString::Printf(TEXT("%s, %s, seed %d: %s"), Case.Name, bTogether ? TEXT("together") : TEXT("anywhere"), Seed, *Error), Error.IsEmpty()))
				{
					break;
				}
				TestTrue(FString::Printf(TEXT("%s, seed %d: same seed, same places"), Case.Name, Seed),
					Places == FHOLMapSizeRules::DrawPlaces(Seed, Slots, Case.NumPlaces, bTogether));
				FirstPlaces.Add(Places[0]);
				bSplitOnce |= !AreTeamsTogether(Slots, Places, Case.NumPlaces);
			}
			// The draw turns: over the seeds, the first slot gets every place of the map.
			TestEqual(FString::Printf(TEXT("%s, %s: the first slot's places over the seeds"), Case.Name, bTogether ? TEXT("together") : TEXT("anywhere")),
				FirstPlaces.Num(), Case.NumPlaces);
			// Without "Équipes ensemble", two allies end apart sometimes (2v2 on Medium, 4v4...).
			if (!bTogether && Case.Teams.Num() >= 4 && Case.Teams.Contains(2) && Case.Teams.Num() == Case.NumPlaces)
			{
				TestTrue(FString::Printf(TEXT("%s, anywhere: allies apart sometimes"), Case.Name), bSplitOnce);
			}
		}
	}

	// More players than places: no draw.
	TestTrue(TEXT("3 players on Small: no draw"), FHOLMapSizeRules::DrawPlaces(1, MakeSlots({ 0, 0, 0 }), 2, true).IsEmpty());
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
	TestEqual(TEXT("upgraded to version 3"), Read.Version, 3);

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

// TC3: what the game mode refuses at the launch (the players past the places of the size, fixed places off the map) and
// what the New game screen does to the slots when a size is chosen (the slots past its places closed, the players kept).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeCheckPlacesTest, "HeroesOfLiberty.MapSize.CheckPlaces", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeCheckPlacesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSize;

	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	Config.Slots = MakeSlots({ 0, 0 });
	Config.Seed = 5;

	// The test path (scenarios, quick_check): no size, fixed places on the eight slots of Plain.
	FString Error;
	TestEqual(TEXT("no size: the eight places of a test map"), FHOLMapSizeRules::GetNumPlaces(Config), FHOLMatchConfig::MaxSlots);
	TestTrue(TEXT("no size, fixed places: accepted"), FHOLMapSizeRules::CheckPlaces(Config, &Error));
	Config.Slots = MakeSlots({ 0, 0, 0, 0, 0, 0, 0, 0 });
	TestTrue(TEXT("no size, eight fixed places: accepted"), FHOLMapSizeRules::CheckPlaces(Config, &Error));

	// The menu: drawn places on the places of the size.
	Config.MapSize = EHOLMapSize::Small;
	Config.bDrawPlaces = true;
	Config.Slots = MakeSlots({ 0, 0 });
	TestEqual(TEXT("Small: two places"), FHOLMapSizeRules::GetNumPlaces(Config), 2);
	TestTrue(TEXT("two players drawn on Small: accepted"), FHOLMapSizeRules::CheckPlaces(Config, &Error));
	Config.Slots = MakeSlots({ 0, 0, 0 });
	Error.Reset();
	TestFalse(TEXT("three players drawn on Small: refused"), FHOLMapSizeRules::CheckPlaces(Config, &Error));
	TestTrue(TEXT("the reason names the players and the places"), Error.Contains(TEXT("3 players")) && Error.Contains(TEXT("2 places")));
	TestTrue(TEXT("its places: none"), FHOLMapSizeRules::GetPlaces(Config).IsEmpty());

	// Fixed places on a size: each slot is its place, which must be on the map.
	Config.bDrawPlaces = false;
	Config.Slots = MakeSlots({ 0, 0 });
	TestTrue(TEXT("slots 0 and 1 fixed on Small: accepted"), FHOLMapSizeRules::CheckPlaces(Config, &Error));
	Config.Slots[1].State = EHOLSlotState::Closed;
	Config.Slots[3].State = EHOLSlotState::AI;
	Error.Reset();
	TestFalse(TEXT("slot 3 fixed on Small (2 places): refused"), FHOLMapSizeRules::CheckPlaces(Config, &Error));
	TestTrue(TEXT("the reason names the slot"), Error.Contains(TEXT("slot 3")));
	Config.MapSize = EHOLMapSize::Medium;
	TestTrue(TEXT("slot 3 fixed on Medium (4 places): accepted"), FHOLMapSizeRules::CheckPlaces(Config, &Error));
	Config.MapSize = EHOLMapSize::Small;
	Config.bDrawPlaces = true;
	TestTrue(TEXT("slots 0 and 3 drawn on Small: accepted (any slot, a drawn place)"), FHOLMapSizeRules::CheckPlaces(Config, &Error));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMapSizeFitSlotsTest, "HeroesOfLiberty.MapSize.FitSlots", UE::HeroesOfLiberty::Tests::MapSize::Flags)
bool FHOLMapSizeFitSlotsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::MapSize;

	// Slots 0, 3 and 6 held, 6 a coloured AI: on Medium (4 places), 6 moves onto slot 1, slots 4 to 7 closed.
	TArray<FHOLMatchSlot> Slots = MakeSlots({});
	Slots[0].State = EHOLSlotState::Human;
	Slots[3].State = EHOLSlotState::AI;
	Slots[3].ColorIndex = 3;
	Slots[6].State = EHOLSlotState::AI;
	Slots[6].ColorIndex = 6;
	Slots[6].Team = 2;
	const FHOLMatchSlot Moved = Slots[6];
	TestTrue(TEXT("three players fit Medium"), FHOLMapSizeRules::FitSlotsToSize(Slots, EHOLMapSize::Medium));
	TestTrue(TEXT("slot 6 moved onto slot 1, as it was"), Slots[1] == Moved);
	TestTrue(TEXT("slots 0 and 3 stay"), Slots[0].State == EHOLSlotState::Human && Slots[3].State == EHOLSlotState::AI && Slots[3].ColorIndex == 3);
	bool bClosed = true;
	for (int32 Index = 4; Index < Slots.Num(); ++Index)
	{
		bClosed &= Slots[Index].State == EHOLSlotState::Closed;
	}
	TestTrue(TEXT("slots 4 to 7 closed"), bClosed);
	TestEqual(TEXT("eight slots still"), Slots.Num(), FHOLMatchConfig::MaxSlots);

	// Three players do not fit Small: nothing changes.
	const TArray<FHOLMatchSlot> Before = Slots;
	TestFalse(TEXT("three players do not fit Small"), FHOLMapSizeRules::FitSlotsToSize(Slots, EHOLMapSize::Small));
	TestTrue(TEXT("nothing changed"), Slots == Before);

	// Large: every slot open, nothing moves.
	TestTrue(TEXT("Large fits"), FHOLMapSizeRules::FitSlotsToSize(Slots, EHOLMapSize::Large));
	TestTrue(TEXT("nothing moved on Large"), Slots == Before);

	// The default size of the screen: the smallest that holds the players; the others greyed.
	TestTrue(TEXT("three players: Medium by default"), FHOLMapSizeRules::GetDefaultSize(3) == EHOLMapSize::Medium);
	TestFalse(TEXT("Small greyed for three"), FHOLMapSizeRules::IsAllowed(EHOLMapSize::Small, 3));
	return true;
}

#endif
