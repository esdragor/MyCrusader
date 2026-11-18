// Lot MIG-C-MatchConfig (CELEBORN-1117n; D830, D836, D839): the script port of eight of the eleven tests of HOLMatchConfigTests.cpp (Default,
// JsonRoundTrip, OlderJson, ManagerReference, Rejects, HumanSlots, SlotForName, RandomColors), run against the read library of FHOLMatchConfig
// (HOLMatchConfigReadLibrary, namespace HOLMatchConfigRead): a configuration is its JSON text, numbers are the enums' (state 0 Closed 1 Human 2 AI;
// faction 0 DawnOrder 1 BlackLegions 5 Templars; mode 0 Standard; map size 0 None). Same vectors as the C++ tests. Differences, all stated:
// the C++ fixture MakeFilledConfig is rebuilt here from MakeDefaultJson + SetSlotJson / SetOptionsJson; "reads its own JSON" and "identical after
// a round trip" go through RoundTripsLosslessly (FromJson, ToJson, FromJson, same configuration) since the script holds no struct; "output
// untouched on failure" is ReadLeavesOutputUntouchedOnFailure. SubsystemFile, PlayerColors and NeutralUnitColor are ported below (CELEBORN-1117o) through new library functions (RunSubsystemFile,
// GetPaletteDWColors, GetPaletteColorName, PaletteLinearColorsEqual, OwnerColorIsPaletteColor, GetNeutralOwner); not ported: the refusal of a
// missing file in SubsystemFile (it logs an Error that only the C++ test declares expected), so the C++ SubsystemFile test stays (CELEBORN-1118m: the C++ PlayerColors and NeutralUnitColor tests are removed, to be reverted if their mutation is not seen red). This file compiles
// only after the C++ lot of the library is built.

struct FMCFGView
{
	bool bOk = false;
	int Version = 0;
	int Seed = 0;
	FString Map;
	int MapSize = 0;
	bool bDrawPlaces = false;
	int Mode = 0;
	bool bNomad = false;
	bool bRegicide = false;
	bool bCatastrophe = false;
	bool bSacredSites = false;
	TArray<int> States;
	TArray<int> Factions;
	TArray<int> Colors;
	TArray<int> Teams;
	TArray<FString> Names;
}

FMCFGView MCFG_Describe(const FString& Json)
{
	FMCFGView V;
	V.bOk = HOLMatchConfigRead::Describe(Json, V.Version, V.Seed, V.Map, V.MapSize, V.bDrawPlaces, V.Mode, V.bNomad, V.bRegicide, V.bCatastrophe,
		V.bSacredSites, V.States, V.Factions, V.Colors, V.Teams, V.Names);
	return V;
}

// MakeFilledConfig of the C++ test: seed 1234, StressTest, slot 1 AI BlackLegions colour 5 team 2, slot 3 Human Templars colour 7 team 1 "Chloe",
// Regicide, Catastrophe and SacredSites ticked.
FString MCFG_Filled()
{
	FString Json = HOLMatchConfigRead::MakeDefaultJson();
	Json = HOLMatchConfigRead::SetOptionsJson(Json, 1234, "/Game/Maps/StressTest.StressTest", false, true, true, true);
	Json = HOLMatchConfigRead::SetSlotJson(Json, 1, 2, 1, 5, 2, "");
	Json = HOLMatchConfigRead::SetSlotJson(Json, 3, 1, 5, 7, 1, "Chloé");
	return Json;
}

TArray<int> MCFG_Taken(int A, int B)
{
	TArray<int> Taken;
	if (A >= 0)
	{
		Taken.Add(A);
	}
	if (B >= 0)
	{
		Taken.Add(B);
	}
	return Taken;
}

void Test_MatchConfigDefault(FUnitTest& T)
{
	FString Json = HOLMatchConfigRead::MakeDefaultJson();
	FMCFGView V = MCFG_Describe(Json);
	T.AssertTrue(V.bOk, "the default reads");
	T.AssertEquals(HOLMatchConfigRead::GetMaxSlots(), V.States.Num(), "eight slots");
	T.AssertEquals(8, V.States.Num(), "eight slots (the number itself)");
	int Occupied = HOLMatchConfigRead::GetNumOccupiedSlots(Json);
	T.AssertEquals(1, Occupied, "one occupied slot");
	T.AssertEquals(1, V.States[0], "slot 0 is human");
	T.AssertEquals(HOLMatchConfigRead::GetCurrentVersion(), V.Version, "current version");
	T.AssertTrue(V.Map.Len() > 0, "a map is set");
	T.AssertTrue(V.Mode == 0 && !V.bNomad && !V.bRegicide && !V.bCatastrophe, "Standard without options");
	T.AssertFalse(V.bSacredSites, "sacred sites unticked by default (D699)");

	TArray<int> Distinct;
	for (int Index = 0; Index < V.Colors.Num(); Index++)
	{
		if (!Distinct.Contains(V.Colors[Index]))
		{
			Distinct.Add(V.Colors[Index]);
		}
	}
	T.AssertEquals(HOLMatchConfigRead::GetMaxSlots(), Distinct.Num(), "distinct default colours");
}

void Test_MatchConfigJsonRoundTrip(FUnitTest& T)
{
	FString Written = MCFG_Filled();
	FString Error;
	T.AssertTrue(HOLMatchConfigRead::CanRead(Written, Error), "reads its own JSON (" + Error + ")");
	T.AssertTrue(HOLMatchConfigRead::RoundTripsLosslessly(Written), "identical after a round trip");
	T.AssertEquals(3, HOLMatchConfigRead::GetNumOccupiedSlots(Written), "three occupied slots");
	FMCFGView V = MCFG_Describe(Written);
	T.AssertEquals(1234, V.Seed, "the seed survives");
	T.AssertEquals(5, V.Colors[1], "slot 1 colour survives");
	T.AssertEquals(2, V.Teams[1], "slot 1 team survives");
	T.AssertEquals(5, V.Factions[3], "slot 3 faction survives (Templars)");
	T.AssertTrue(V.bRegicide && V.bCatastrophe && V.bSacredSites && !V.bNomad, "the options survive");
}

void Test_MatchConfigOlderJson(FUnitTest& T)
{
	// No version, two slots only, no options: what a file written before a field existed looks like.
	FString Json = "{ \"map\": \"/Game/Level/BattleMap.BattleMap\", \"slots\": [ { \"state\": \"Human\", \"colorIndex\": 3 }, { \"state\": \"AI\", \"faction\": \"BlackLegions\", \"team\": 9 } ] }";
	FString Error;
	T.AssertTrue(HOLMatchConfigRead::CanRead(Json, Error), "reads an older file (" + Error + ")");
	FMCFGView Read = MCFG_Describe(Json);
	T.AssertEquals(HOLMatchConfigRead::GetMaxSlots(), Read.States.Num(), "padded to eight slots");
	T.AssertEquals(2, HOLMatchConfigRead::GetNumOccupiedSlots(Json), "two occupied slots");
	T.AssertEquals(3, Read.Colors[0], "colour kept");
	T.AssertEquals(0, Read.Teams[1], "team out of range back to none");
	T.AssertEquals(1, Read.Factions[1], "faction read by name (BlackLegions)");
	T.AssertFalse(Read.bCatastrophe, "missing option keeps its default");
	T.AssertEquals(HOLMatchConfigRead::GetCurrentVersion(), Read.Version, "upgraded to the current version");
	T.AssertTrue(Read.Names[0].IsEmpty(), "no player name before version 2");

	FString V1 = "{ \"version\": 1, \"slots\": [ { \"state\": \"Human\" } ] }";
	T.AssertTrue(HOLMatchConfigRead::CanRead(V1, Error), "reads a version 1 file");
	FMCFGView ReadV1 = MCFG_Describe(V1);
	T.AssertEquals(HOLMatchConfigRead::GetCurrentVersion(), ReadV1.Version, "version 1 upgraded");
}

void Test_MatchConfigManagerReference(FUnitTest& T)
{
	// The match.json the lobby manager writes (R3.1, Manager\PROTOCOL.md), from the manager's own reference file.
	FString Path = "Manager/HOLManager.Tests/Data/match_v2_reference.json";
	FString Json;
	bool bLoaded = HOLMatchConfigRead::LoadProjectFileText(Path, Json);
	T.AssertTrue(bLoaded, "reference file read (" + Path + ")");
	if (!bLoaded)
	{
		return;
	}
	FString Error;
	bool bCanRead = HOLMatchConfigRead::CanRead(Json, Error);
	T.AssertTrue(bCanRead, "reference parsed (" + Error + ")");
	if (!bCanRead)
	{
		return;
	}
	FMCFGView Read = MCFG_Describe(Json);
	// A version 2 file, read and upgraded (TC1, version 3): no map size, fixed places.
	T.AssertEquals(HOLMatchConfigRead::GetCurrentVersion(), Read.Version, "version 2 upgraded to the current version");
	T.AssertTrue(Read.MapSize == 0 && !Read.bDrawPlaces, "no map size, fixed places");
	T.AssertEquals(184467, Read.Seed, "seed");
	T.AssertEquals("/Game/Maps/Plain.Plain", Read.Map, "map");
	T.AssertTrue(Read.Mode == 0 && Read.bNomad && !Read.bRegicide && !Read.bCatastrophe, "Standard, nomad only");
	T.AssertEquals(HOLMatchConfigRead::GetMaxSlots(), Read.States.Num(), "eight slots");
	T.AssertEquals(3, HOLMatchConfigRead::GetNumOccupiedSlots(Json), "three humans");

	// State (0 Closed, 1 Human), colour, team, name of each slot.
	TArray<int> States;
	States.Add(1); States.Add(0); States.Add(1); States.Add(1); States.Add(0); States.Add(0); States.Add(0); States.Add(0);
	TArray<int> Colors;
	Colors.Add(0); Colors.Add(1); Colors.Add(3); Colors.Add(5); Colors.Add(4); Colors.Add(5); Colors.Add(6); Colors.Add(7);
	TArray<int> Teams;
	Teams.Add(1); Teams.Add(0); Teams.Add(2); Teams.Add(1); Teams.Add(0); Teams.Add(0); Teams.Add(0); Teams.Add(0);
	TArray<FString> Names;
	Names.Add("Bob"); Names.Add(""); Names.Add("Alice"); Names.Add("Chloé"); Names.Add(""); Names.Add(""); Names.Add(""); Names.Add("");
	for (int Index = 0; Index < States.Num() && Index < Read.States.Num(); Index++)
	{
		T.AssertEquals(States[Index], Read.States[Index], "slot " + Index + " state");
		T.AssertEquals(0, Read.Factions[Index], "slot " + Index + " faction (DawnOrder)");
		T.AssertEquals(Colors[Index], Read.Colors[Index], "slot " + Index + " colour");
		T.AssertEquals(Teams[Index], Read.Teams[Index], "slot " + Index + " team");
		T.AssertEquals(Names[Index], Read.Names[Index], "slot " + Index + " name");
	}
}

void Test_MatchConfigRejects(FUnitTest& T)
{
	FString Base = HOLMatchConfigRead::MakeDefaultJson();
	bool bParsed = true;
	T.AssertTrue(HOLMatchConfigRead::ReadLeavesOutputUntouchedOnFailure(Base, "{ \"version\": 99 }", bParsed), "output untouched on a newer version");
	T.AssertFalse(bParsed, "newer version refused");
	bParsed = true;
	T.AssertTrue(HOLMatchConfigRead::ReadLeavesOutputUntouchedOnFailure(Base, "{ not json", bParsed), "output untouched on malformed JSON");
	T.AssertFalse(bParsed, "malformed JSON refused");
	FString Error;
	T.AssertFalse(HOLMatchConfigRead::CanRead("{ \"version\": 99 }", Error), "newer version refused (fresh read)");
	T.AssertFalse(Error.IsEmpty(), "the refusal says why");
}

void Test_MatchConfigHumanSlots(FUnitTest& T)
{
	FString Config = MCFG_Filled();
	T.AssertEquals(0, HOLMatchConfigRead::FindFreeHumanSlot(Config, MCFG_Taken(-1, -1)), "first human slot");
	T.AssertEquals(3, HOLMatchConfigRead::FindFreeHumanSlot(Config, MCFG_Taken(0, -1)), "AI slot skipped");
	T.AssertEquals(-1, HOLMatchConfigRead::FindFreeHumanSlot(Config, MCFG_Taken(0, 3)), "no human slot left");
}

void Test_MatchConfigSlotForName(FUnitTest& T)
{
	// R3.2: the lobby manager names the human slots, each player gets the one of his name.
	FString Named = HOLMatchConfigRead::MakeDefaultJson();
	Named = HOLMatchConfigRead::SetSlotJson(Named, 0, 1, 0, 0, 0, "Alice");
	Named = HOLMatchConfigRead::SetSlotJson(Named, 1, 2, 1, 1, 0, "");
	Named = HOLMatchConfigRead::SetSlotJson(Named, 2, 1, 0, 2, 0, "Bob");
	T.AssertTrue(HOLMatchConfigRead::HasPlayerNames(Named), "names given");
	T.AssertEquals(2, HOLMatchConfigRead::FindSlotForName(Named, "Bob", MCFG_Taken(-1, -1)), "known name, whatever the arrival");
	T.AssertEquals(0, HOLMatchConfigRead::FindSlotForName(Named, "Alice", MCFG_Taken(2, -1)), "first named slot");
	T.AssertEquals(2, HOLMatchConfigRead::FindSlotForName(Named, "bOB", MCFG_Taken(-1, -1)), "case ignored");
	T.AssertEquals(2, HOLMatchConfigRead::FindSlotForName(Named, " Bob ", MCFG_Taken(-1, -1)), "spaces around ignored");
	T.AssertEquals(-1, HOLMatchConfigRead::FindSlotForName(Named, "Chloé", MCFG_Taken(-1, -1)), "unknown name: spectator");
	T.AssertEquals(-1, HOLMatchConfigRead::FindSlotForName(Named, "", MCFG_Taken(-1, -1)), "no name: spectator");
	T.AssertEquals(-1, HOLMatchConfigRead::FindSlotForName(Named, "Bob", MCFG_Taken(2, -1)), "name already held: spectator");

	// Without names (solo, tests, net_match.ps1, version 1): by arrival, as before.
	FString NoNames = HOLMatchConfigRead::SetSlotJson(MCFG_Filled(), 3, 1, 5, 7, 1, "");
	T.AssertFalse(HOLMatchConfigRead::HasPlayerNames(NoNames), "no names");
	T.AssertEquals(0, HOLMatchConfigRead::FindSlotForName(NoNames, "Bob", MCFG_Taken(-1, -1)), "no names: first free slot");
	T.AssertEquals(3, HOLMatchConfigRead::FindSlotForName(NoNames, "", MCFG_Taken(0, -1)), "no names, no name: first free slot");
	T.AssertEquals(-1, HOLMatchConfigRead::FindSlotForName(NoNames, "Bob", MCFG_Taken(0, 3)), "no names, all taken");
}

// E2-2: "Aleatoire" in the menu; at launch each such slot gets a colour no other occupied slot has, chosen or drawn.
void Test_MatchConfigRandomColors(FUnitTest& T)
{
	int Random = HOLMatchConfigRead::GetRandomColor();
	int NumColors = HOLMatchConfigRead::GetNumColors();
	TArray<int> States;
	States.Add(1); States.Add(2); States.Add(1); States.Add(0); States.Add(0);
	TArray<int> Colors;
	Colors.Add(Random); Colors.Add(3); Colors.Add(Random); Colors.Add(3); Colors.Add(Random);
	TArray<int> FirstDrawn;
	for (int Seed = 0; Seed < 50; Seed++)
	{
		TArray<int> Result = HOLMatchConfigRead::DrawRandomColors(Seed, States, Colors);
		T.AssertEquals(HOLMatchConfigRead::GetMaxSlots(), Result.Num(), "seed " + Seed + ": eight colours back");
		if (Result.Num() < 5)
		{
			return;
		}
		TArray<int> Distinct;
		for (int Index = 0; Index < 3; Index++)
		{
			int Color = Result[Index];
			T.AssertTrue(Color >= 0 && Color < NumColors, "seed " + Seed + " slot " + Index + ": a palette colour (" + Color + ")");
			if (!Distinct.Contains(Color))
			{
				Distinct.Add(Color);
			}
		}
		T.AssertEquals(3, Distinct.Num(), "seed " + Seed + ": distinct colours");
		T.AssertEquals(3, Result[1], "seed " + Seed + ": a chosen colour is kept");
		T.AssertEquals(4, Result[4], "seed " + Seed + ": a closed slot gets its index");
		if (!FirstDrawn.Contains(Result[0]))
		{
			FirstDrawn.Add(Result[0]);
		}
	}
	T.AssertTrue(FirstDrawn.Num() > 1, "the draw varies with the seed");
}

// SubsystemFile: what a dedicated server started with -MatchConfig=<file> goes through (a UHOLMatchSubsystem fed from a file).
void Test_MatchConfigSubsystemFile(FUnitTest& T)
{
	bool bNoConfigAtFirst = false;
	bool bDefaultWhenNone = false;
	bool bLoaded = false;
	bool bConfigFromFile = false;
	bool bRan = HOLMatchConfigRead::RunSubsystemFile(MCFG_Filled(), bNoConfigAtFirst, bDefaultWhenNone, bLoaded, bConfigFromFile);
	T.AssertTrue(bRan, "file written");
	T.AssertTrue(bNoConfigAtFirst, "no configuration at first");
	T.AssertTrue(bDefaultWhenNone, "default when none");
	T.AssertTrue(bLoaded, "file read");
	T.AssertTrue(bConfigFromFile, "configuration from the file");
}

// PlayerColors: the palette is eight distinct named colours and an index past the end wraps.
void Test_MatchConfigPlayerColors(FUnitTest& T)
{
	int NumColors = HOLMatchConfigRead::GetNumColors();
	TArray<int> Palette = HOLMatchConfigRead::GetPaletteDWColors();
	T.AssertEquals(NumColors, Palette.Num(), "one colour per palette index");
	TArray<int> Distinct;
	for (int Index = 0; Index < NumColors; Index++)
	{
		if (Index < Palette.Num() && !Distinct.Contains(Palette[Index]))
		{
			Distinct.Add(Palette[Index]);
		}
		T.AssertFalse(HOLMatchConfigRead::GetPaletteColorName(Index).IsEmpty(), "named (" + Index + ")");
	}
	T.AssertEquals(NumColors, Distinct.Num(), "distinct colours");
	T.AssertTrue(HOLMatchConfigRead::PaletteLinearColorsEqual(NumColors, 0), "index wraps");
}

// NeutralUnitColor: neutral units (owner 8) were drawn in the colour of palette index 0 (blue); the colour written for the neutral owner must be
// the colour of no slot, whatever the slots chose, and every slot keeps its chosen colour.
void Test_MatchConfigNeutralUnitColor(FUnitTest& T)
{
	int Slots = HOLMatchConfigRead::GetMaxSlots();
	int NumColors = HOLMatchConfigRead::GetNumColors();
	int Neutral = HOLMatchConfigRead::GetNeutralOwner();
	// The default assignment (slot i, colour i) and a shuffled one: both use the whole palette.
	for (int Variant = 0; Variant < 2; Variant++)
	{
		TArray<int> SlotColors;
		for (int Slot = 0; Slot < Slots; Slot++)
		{
			SlotColors.Add(Variant == 0 ? Slot : (Slot * 3 + 1) % NumColors);
		}
		for (int Color = 0; Color < NumColors; Color++)
		{
			T.AssertFalse(HOLMatchConfigRead::OwnerColorIsPaletteColor(Neutral, SlotColors, Color, 0.001f), "variant " + Variant + ": neutral colour is not palette colour " + Color);
		}
		for (int Slot = 0; Slot < Slots; Slot++)
		{
			T.AssertTrue(HOLMatchConfigRead::OwnerColorIsPaletteColor(Slot, SlotColors, SlotColors[Slot], 0.0f), "variant " + Variant + ": slot " + Slot + " keeps its chosen colour");
		}
	}
}
