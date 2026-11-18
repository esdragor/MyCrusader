// Lot MIG-C-MapSize (D830, D836, D839): the script port of the tests of HOLMapSizeTests.cpp that read only the pure rule of the map sizes
// (FHOLMapSizeRules, C++, stays), run against its read library (HOLMapSizeReadLibrary, namespace HOLMapSizeRead). Same vectors as the C++
// tests (Table, Allowed, MapFile, DrawPlaces, CheckPlaces, FitSlots, and the places part of Config). A size is its number (0 None, 1 Small,
// 2 Medium, 3 Normal, 4 Large, 5 Huge); the slots are States (0 Closed, 1 Human, 2 AI) and Teams (see the library). The C++ twins of Allowed, MapFile,
// DrawPlaces, CheckPlaces and FitSlots are removed (red then green seen, SAURON-1117i); Table and Config stay in C++ (FitsMaxSize, JSON). Not ported (see Docs\Rapports\ARWEN-1117i.md): the CSV test (reads Tools/Data/MapSizes.csv), the
// FromJson/ToJson round trip of Config, the Lobby tests (HOLManagerClientRules), the AHOLMapBounds checks of Table (the 220-tile limit of
// D344 is written as a constant below).

const int HOLMapSizeNone = 0;
const int HOLMapSizeSmall = 1;
const int HOLMapSizeMedium = 2;
const int HOLMapSizeNormal = 3;
const int HOLMapSizeLarge = 4;
const int HOLMapSizeHuge = 5;
const int HOLMapSizeMaxSlots = 8;

// Up to eight ints as an array (-99 = not given): the script has no array literal here.
TArray<int> HOLMapSizeInts(int A = -99, int B = -99, int C = -99, int D = -99, int E = -99, int F = -99, int G = -99, int H = -99)
{
	TArray<int> Result;
	if (A != -99) Result.Add(A);
	if (B != -99) Result.Add(B);
	if (C != -99) Result.Add(C);
	if (D != -99) Result.Add(D);
	if (E != -99) Result.Add(E);
	if (F != -99) Result.Add(F);
	if (G != -99) Result.Add(G);
	if (H != -99) Result.Add(H);
	return Result;
}

// Rows of ints in one flat array (Angelscript refuses TArray<TArray<int>>): Lens holds each row's length.
void HOLMapSizeAddRow(TArray<int>& Flat, TArray<int>& Lens, const TArray<int>& Row)
{
	Lens.Add(Row.Num());
	for (int Index = 0; Index < Row.Num(); ++Index)
	{
		Flat.Add(Row[Index]);
	}
}

TArray<int> HOLMapSizeGetRow(const TArray<int>& Flat, const TArray<int>& Lens, int RowIndex)
{
	int Start = 0;
	for (int Index = 0; Index < RowIndex; ++Index)
	{
		Start += Lens[Index];
	}
	TArray<int> Row;
	for (int Index = 0; Index < Lens[RowIndex]; ++Index)
	{
		Row.Add(Flat[Start + Index]);
	}
	return Row;
}

bool HOLMapSizeAreSame(const TArray<int>& A, const TArray<int>& B)
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int Index = 0; Index < A.Num(); ++Index)
	{
		if (A[Index] != B[Index])
		{
			return false;
		}
	}
	return true;
}

// MakeSlots of the C++ test: eight slots, Teams[i] for slot i (odd = AI, even = Human), the slots past Teams closed.
void HOLMapSizeMakeSlots(const TArray<int>& Teams, TArray<int>& OutStates, TArray<int>& OutTeams)
{
	OutStates.Reset();
	OutTeams.Reset();
	for (int Index = 0; Index < HOLMapSizeMaxSlots; ++Index)
	{
		if (Index < Teams.Num())
		{
			OutStates.Add(Index % 2 == 1 ? 2 : 1);
			OutTeams.Add(Teams[Index]);
		}
		else
		{
			OutStates.Add(0);
			OutTeams.Add(0);
		}
	}
}

// CheckDraw of the C++ test: what is wrong with a draw, or empty: one place per slot, distinct, on the map, evenly spread, allies together.
FString HOLMapSizeCheckDraw(const TArray<int>& States, const TArray<int>& Teams, const TArray<int>& Places, int NumPlaces, bool bTeamsTogether)
{
	if (Places.Num() != States.Num())
	{
		return "" + Places.Num() + " places for " + States.Num() + " slots";
	}
	TArray<int> PlaceTeam;	// by place: the team key (a slot without a team is its own team), -1 when free
	for (int Place = 0; Place < NumPlaces; ++Place)
	{
		PlaceTeam.Add(-1);
	}
	for (int Index = 0; Index < States.Num(); ++Index)
	{
		if (States[Index] == 0)
		{
			if (Places[Index] != -1)
			{
				return "closed slot " + Index + " at place " + Places[Index];
			}
			continue;
		}
		if (Places[Index] < 0 || Places[Index] >= NumPlaces)
		{
			return "slot " + Index + " at place " + Places[Index] + ", off the " + NumPlaces + " places";
		}
		if (PlaceTeam[Places[Index]] != -1)
		{
			return "place " + Places[Index] + " given twice";
		}
		PlaceTeam[Places[Index]] = Teams[Index] > 0 ? Teams[Index] : 100 + Index;
	}

	// Around the circle (the used places in order): the gaps between two neighbouring players differ by one at most.
	TArray<int> Sorted;
	for (int Place = 0; Place < NumPlaces; ++Place)
	{
		if (PlaceTeam[Place] != -1)
		{
			Sorted.Add(Place);
		}
	}
	const int Num = Sorted.Num();
	int MinGap = 1000000;
	int MaxGap = 0;
	for (int Index = 0; Index < Num; ++Index)
	{
		const int Gap = Index + 1 < Num ? Sorted[Index + 1] - Sorted[Index] : Sorted[0] + NumPlaces - Sorted[Index];
		MinGap = Math::Min(MinGap, Gap);
		MaxGap = Math::Max(MaxGap, Gap);
	}
	if (Num > 1 && MaxGap - MinGap > 1)
	{
		return "uneven places (gaps " + MinGap + " to " + MaxGap + ")";
	}

	// Allies together: each team starts one run around the circle.
	if (bTeamsTogether && Num > 1)
	{
		TArray<int> RunTeams;
		TArray<int> RunCounts;
		for (int Index = 0; Index < Num; ++Index)
		{
			const int Team = PlaceTeam[Sorted[Index]];
			if (Team != PlaceTeam[Sorted[(Index + Num - 1) % Num]])
			{
				int At = RunTeams.FindIndex(Team);
				if (At < 0)
				{
					RunTeams.Add(Team);
					RunCounts.Add(0);
					At = RunTeams.Num() - 1;
				}
				RunCounts[At] += 1;
			}
		}
		for (int Run = 0; Run < RunTeams.Num(); ++Run)
		{
			if (RunCounts[Run] > 1)
			{
				return "team " + RunTeams[Run] + " split in " + RunCounts[Run] + " groups";
			}
		}
	}
	return "";
}

// HeroesOfLiberty.MapSize.Table
void Test_MapSizeTable(FUnitTest& T)
{
	TArray<FString> Keys;
	Keys.Add("Small"); Keys.Add("Medium"); Keys.Add("Normal"); Keys.Add("Large"); Keys.Add("Huge");
	const TArray<int> Places = HOLMapSizeInts(2, 4, 6, 8, 8);
	const TArray<int> Sides = HOLMapSizeInts(110, 154, 183, 202, 220);
	T.AssertEquals(5, HOLMapSizeRead::GetNumSizes(), "five sizes");
	for (int Index = 0; Index < 5; ++Index)
	{
		const int Size = HOLMapSizeSmall + Index;
		T.AssertEquals(Places[Index], HOLMapSizeRead::GetNumPlaces(Size), Keys[Index] + " places");
		T.AssertEquals(Sides[Index], HOLMapSizeRead::GetSideTiles(Size), Keys[Index] + " side");
		T.AssertEquals(Size, HOLMapSizeRead::FindByKey(Keys[Index]), Keys[Index] + " found by key");
		FString Key;
		int NumPlaces = 0;
		int SideTiles = 0;
		T.AssertTrue(HOLMapSizeRead::FindSize(Size, Key, NumPlaces, SideTiles), Keys[Index] + " line");
		T.AssertTrue(Key == Keys[Index], Keys[Index] + " line key");
		T.AssertEquals(Places[Index], NumPlaces, Keys[Index] + " line places");
		T.AssertEquals(Sides[Index], SideTiles, Keys[Index] + " line side");
		T.AssertEquals(Size, HOLMapSizeRead::GetSizeAt(Index), Keys[Index] + " in order");
		T.AssertTrue(HOLMapSizeRead::GetSideTiles(Size) <= 220, Keys[Index] + " within MaxSideTiles (D344: 220 tiles, 880 m)");
		T.AssertTrue(Places[Index] <= HOLMapSizeMaxSlots, Keys[Index] + " within the 8 slots");
	}
	T.AssertEquals(0, HOLMapSizeRead::GetNumPlaces(HOLMapSizeNone), "no places without a size");
	FString Key;
	int NumPlaces = 0;
	int SideTiles = 0;
	T.AssertFalse(HOLMapSizeRead::FindSize(HOLMapSizeNone, Key, NumPlaces, SideTiles), "no line without a size");
	T.AssertEquals(HOLMapSizeNone, HOLMapSizeRead::FindByKey("Gigantic"), "unknown key");
	T.AssertEquals(HOLMapSizeLarge, HOLMapSizeRead::FindByKey("large"), "key case ignored");
}

// HeroesOfLiberty.MapSize.Allowed
void Test_MapSizeAllowed(FUnitTest& T)
{
	TArray<int> AllowedFlat;
	TArray<int> AllowedLens;
	for (int Players = 1; Players <= 8; ++Players)
	{
		TArray<int> Row;
		const int First = Players <= 2 ? HOLMapSizeSmall : Players <= 4 ? HOLMapSizeMedium : Players <= 6 ? HOLMapSizeNormal : HOLMapSizeLarge;
		for (int Size = First; Size <= HOLMapSizeHuge; ++Size)
		{
			Row.Add(Size);
		}
		HOLMapSizeAddRow(AllowedFlat, AllowedLens, Row);
	}
	const TArray<int> Default = HOLMapSizeInts(1, 1, 2, 2, 3, 3, 4, 4);
	for (int Players = 1; Players <= 8; ++Players)
	{
		const TArray<int> Got = HOLMapSizeRead::GetAllowedSizes(Players);
		const TArray<int> Allowed = HOLMapSizeGetRow(AllowedFlat, AllowedLens, Players - 1);
		T.AssertEquals(Allowed.Num(), Got.Num(), "number of sizes allowed for " + Players + " players");
		for (int Index = 0; Index < Got.Num() && Index < Allowed.Num(); ++Index)
		{
			T.AssertEquals(Allowed[Index], Got[Index], "size " + Index + " allowed for " + Players + " players");
		}
		T.AssertEquals(Default[Players - 1], HOLMapSizeRead::GetDefaultSize(Players), "default size for " + Players + " players");
		for (int Size = HOLMapSizeSmall; Size <= HOLMapSizeHuge; ++Size)
		{
			T.AssertTrue(Allowed.Contains(Size) == HOLMapSizeRead::IsAllowed(Size, Players), "size " + Size + " allowed for " + Players + " players");
		}
	}
	T.AssertTrue(HOLMapSizeRead::GetAllowedSizes(9).Num() == 0 && HOLMapSizeRead::GetDefaultSize(9) == HOLMapSizeNone, "no size past eight players");
	T.AssertEquals(HOLMapSizeSmall, HOLMapSizeRead::GetDefaultSize(0), "nobody seated yet: the smallest");
	T.AssertFalse(HOLMapSizeRead::IsAllowed(HOLMapSizeNone, 1), "None never allowed");
}

// HeroesOfLiberty.MapSize.MapFile
void Test_MapSizeMapFile(FUnitTest& T)
{
	const FString Plain = "/Game/Maps/Plain.Plain";
	T.AssertTrue(HOLMapSizeRead::GetMapFile(Plain, HOLMapSizeLarge) == "/Game/Maps/Plain_Large.Plain_Large", "Plain at Large");
	T.AssertTrue(HOLMapSizeRead::GetMapFile(Plain, HOLMapSizeSmall) == "/Game/Maps/Plain_Small.Plain_Small", "Plain at Small");
	T.AssertTrue(HOLMapSizeRead::GetMapFile(Plain, HOLMapSizeHuge) == "/Game/Maps/Plain_Huge.Plain_Huge", "Plain at Huge");
	T.AssertTrue(HOLMapSizeRead::GetMapFile("/Game/Maps/Plain", HOLMapSizeMedium) == "/Game/Maps/Plain_Medium.Plain_Medium", "a map given without its object name");
	T.AssertTrue(HOLMapSizeRead::GetMapFile(Plain, HOLMapSizeNone) == Plain, "without a size, the map as it is");
	T.AssertTrue(HOLMapSizeRead::GetMapFile("", HOLMapSizeLarge) == "", "no map, no file");
}

// HeroesOfLiberty.MapSize.DrawPlaces
void Test_MapSizeDrawPlaces(FUnitTest& T)
{
	TArray<int> CaseTeamsFlat;
	TArray<int> CaseTeamsLens;
	TArray<int> CasePlaces;
	TArray<FString> CaseNames;
	CaseNames.Add("1 player on Small");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(0));  CasePlaces.Add(2);
	CaseNames.Add("1 player on Large");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(0));  CasePlaces.Add(8);
	CaseNames.Add("1v1 on Small");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(0, 0));  CasePlaces.Add(2);
	CaseNames.Add("2v2 on Medium");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(1, 2, 1, 2));  CasePlaces.Add(4);
	CaseNames.Add("2v2 on Large, with gaps");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(1, 1, 2, 2));  CasePlaces.Add(8);
	CaseNames.Add("3v3 on Normal");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(1, 2, 1, 2, 1, 2));  CasePlaces.Add(6);
	CaseNames.Add("3v3 on Large");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(2, 2, 2, 1, 1, 1));  CasePlaces.Add(8);
	CaseNames.Add("4v4 on Large");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(1, 2, 1, 2, 1, 2, 1, 2));  CasePlaces.Add(8);
	CaseNames.Add("2v2v2v2 on Huge");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(1, 2, 3, 4, 4, 3, 2, 1));  CasePlaces.Add(8);
	CaseNames.Add("free for all, 4 on Medium");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(0, 0, 0, 0));  CasePlaces.Add(4);
	CaseNames.Add("free for all, 5 on Large");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(0, 0, 0, 0, 0));  CasePlaces.Add(8);
	CaseNames.Add("a team and two alone on Normal");  HOLMapSizeAddRow(CaseTeamsFlat, CaseTeamsLens, HOLMapSizeInts(1, 0, 1, 0));  CasePlaces.Add(6);

	for (int Case = 0; Case < CaseNames.Num(); ++Case)
	{
		const TArray<int> CaseTeams = HOLMapSizeGetRow(CaseTeamsFlat, CaseTeamsLens, Case);
		TArray<int> States;
		TArray<int> Teams;
		HOLMapSizeMakeSlots(CaseTeams, States, Teams);
		for (int Mode = 0; Mode < 2; ++Mode)
		{
			const bool bTogether = Mode == 0;
			const FString Label = CaseNames[Case] + ", " + (bTogether ? "together" : "anywhere");
			TArray<int> FirstPlaces;
			bool bSplitOnce = false;
			bool bFailed = false;
			for (int Seed = 0; Seed < 200 && !bFailed; ++Seed)
			{
				const TArray<int> Places = HOLMapSizeRead::DrawPlaces(Seed, States, Teams, CasePlaces[Case], bTogether);
				const FString Error = HOLMapSizeCheckDraw(States, Teams, Places, CasePlaces[Case], bTogether);
				T.AssertTrue(Error == "", Label + ", seed " + Seed + ": " + Error);
				if (Error != "")
				{
					bFailed = true;
					break;
				}
				const TArray<int> Again = HOLMapSizeRead::DrawPlaces(Seed, States, Teams, CasePlaces[Case], bTogether);
				bool bSame = Again.Num() == Places.Num();
				for (int Index = 0; bSame && Index < Places.Num(); ++Index)
				{
					bSame = Places[Index] == Again[Index];
				}
				T.AssertTrue(bSame, Label + ", seed " + Seed + ": same seed, same places");
				if (!FirstPlaces.Contains(Places[0]))
				{
					FirstPlaces.Add(Places[0]);
				}
				bSplitOnce = bSplitOnce || HOLMapSizeCheckDraw(States, Teams, Places, CasePlaces[Case], true) != "";
			}
			// The draw turns: over the seeds, the first slot gets every place of the map.
			T.AssertEquals(CasePlaces[Case], FirstPlaces.Num(), Label + ": the first slot's places over the seeds");
			// Without "Equipes ensemble", two allies end apart sometimes (2v2 on Medium, 4v4...).
			if (!bTogether && CaseTeams.Num() >= 4 && CaseTeams.Contains(2) && CaseTeams.Num() == CasePlaces[Case])
			{
				T.AssertTrue(bSplitOnce, Label + ": allies apart sometimes");
			}
		}
	}

	// More players than places: no draw.
	TArray<int> States3;
	TArray<int> Teams3;
	HOLMapSizeMakeSlots(HOLMapSizeInts(0, 0, 0), States3, Teams3);
	T.AssertTrue(HOLMapSizeRead::DrawPlaces(1, States3, Teams3, 2, true).Num() == 0, "3 players on Small: no draw");
}

// HeroesOfLiberty.MapSize.Config, the places part (the JSON part stays in C++).
void Test_MapSizeConfigPlaces(FUnitTest& T)
{
	// The version 2 file of the C++ test: slots Human team 1, Closed, AI team 2, AI team 1.
	TArray<int> States = HOLMapSizeInts(1, 0, 2, 2);
	TArray<int> Teams = HOLMapSizeInts(1, 0, 2, 1);
	const TArray<int> Fixed = HOLMapSizeInts(0, -1, 2, 3, -1, -1, -1, -1);
	TArray<int> Got = HOLMapSizeRead::GetConfigPlaces(77, HOLMapSizeNone, false, true, States, Teams);
	T.AssertTrue(HOLMapSizeAreSame(Got, Fixed), "fixed places: slot i at place i");
	T.AssertTrue(HOLMapSizeAreSame(HOLMapSizeRead::GetFixedPlaces(States, Teams), Fixed), "fixed places of the slots");

	// Drawn: on the 8 places of Large, from the seed, the same as the draw itself; without a size, on the eight slots.
	TArray<int> LargeStates;
	TArray<int> LargeTeams;
	HOLMapSizeMakeSlots(HOLMapSizeInts(1, 2, 1, 2), LargeStates, LargeTeams);
	const TArray<int> Drawn = HOLMapSizeRead::GetConfigPlaces(77, HOLMapSizeLarge, true, true, LargeStates, LargeTeams);
	T.AssertTrue(HOLMapSizeAreSame(Drawn, HOLMapSizeRead::DrawPlaces(77, LargeStates, LargeTeams, 8, true)), "drawn places of a Large map");
	T.AssertTrue(HOLMapSizeCheckDraw(LargeStates, LargeTeams, Drawn, 8, true) == "", "drawn places valid");
	const TArray<int> DrawnNone = HOLMapSizeRead::GetConfigPlaces(77, HOLMapSizeNone, true, true, LargeStates, LargeTeams);
	T.AssertTrue(HOLMapSizeAreSame(DrawnNone, HOLMapSizeRead::DrawPlaces(77, LargeStates, LargeTeams, HOLMapSizeMaxSlots, true)), "drawn without a size: on the eight slots");
}

// HeroesOfLiberty.MapSize.CheckPlaces
void Test_MapSizeCheckPlaces(FUnitTest& T)
{
	TArray<int> States;
	TArray<int> Teams;
	FString Error;
	HOLMapSizeMakeSlots(HOLMapSizeInts(0, 0), States, Teams);

	// The test path (scenarios, quick_check): no size, fixed places on the eight slots of Plain.
	T.AssertEquals(HOLMapSizeMaxSlots, HOLMapSizeRead::GetConfigNumPlaces(HOLMapSizeNone), "no size: the eight places of a test map");
	T.AssertTrue(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeNone, false, true, States, Teams, Error), "no size, fixed places: accepted");
	HOLMapSizeMakeSlots(HOLMapSizeInts(0, 0, 0, 0, 0, 0, 0, 0), States, Teams);
	T.AssertTrue(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeNone, false, true, States, Teams, Error), "no size, eight fixed places: accepted");

	// The menu: drawn places on the places of the size.
	HOLMapSizeMakeSlots(HOLMapSizeInts(0, 0), States, Teams);
	T.AssertEquals(2, HOLMapSizeRead::GetConfigNumPlaces(HOLMapSizeSmall), "Small: two places");
	T.AssertTrue(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeSmall, true, true, States, Teams, Error), "two players drawn on Small: accepted");
	HOLMapSizeMakeSlots(HOLMapSizeInts(0, 0, 0), States, Teams);
	T.AssertFalse(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeSmall, true, true, States, Teams, Error), "three players drawn on Small: refused");
	T.AssertTrue(Error.Contains("3 players") && Error.Contains("2 places"), "the reason names the players and the places");
	T.AssertTrue(HOLMapSizeRead::GetConfigPlaces(5, HOLMapSizeSmall, true, true, States, Teams).Num() == 0, "its places: none");

	// Fixed places on a size: each slot is its place, which must be on the map.
	HOLMapSizeMakeSlots(HOLMapSizeInts(0, 0), States, Teams);
	T.AssertTrue(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeSmall, false, true, States, Teams, Error), "slots 0 and 1 fixed on Small: accepted");
	States[1] = 0;
	States[3] = 2;
	T.AssertFalse(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeSmall, false, true, States, Teams, Error), "slot 3 fixed on Small (2 places): refused");
	T.AssertTrue(Error.Contains("slot 3"), "the reason names the slot");
	T.AssertTrue(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeMedium, false, true, States, Teams, Error), "slot 3 fixed on Medium (4 places): accepted");
	T.AssertTrue(HOLMapSizeRead::CheckConfigPlaces(5, HOLMapSizeSmall, true, true, States, Teams, Error), "slots 0 and 3 drawn on Small: accepted (any slot, a drawn place)");
}

// HeroesOfLiberty.MapSize.FitSlots
void Test_MapSizeFitSlots(FUnitTest& T)
{
	// Slots 0, 3 and 6 held, 6 a coloured AI: on Medium (4 places), 6 moves onto slot 1, slots 4 to 7 closed.
	TArray<int> States = HOLMapSizeInts(1, 0, 0, 2, 0, 0, 2, 0);
	TArray<int> Teams = HOLMapSizeInts(0, 0, 0, 0, 0, 0, 2, 0);
	TArray<int> Colors = HOLMapSizeInts(0, 0, 0, 3, 0, 0, 6, 0);
	T.AssertTrue(HOLMapSizeRead::FitSlotsToSize(States, Teams, Colors, HOLMapSizeMedium), "three players fit Medium");
	T.AssertTrue(States[1] == 2 && Teams[1] == 2 && Colors[1] == 6, "slot 6 moved onto slot 1, as it was");
	T.AssertTrue(States[0] == 1 && States[3] == 2 && Colors[3] == 3, "slots 0 and 3 stay");
	bool bClosed = true;
	for (int Index = 4; Index < States.Num(); ++Index)
	{
		bClosed = bClosed && States[Index] == 0;
	}
	T.AssertTrue(bClosed, "slots 4 to 7 closed");
	T.AssertEquals(HOLMapSizeMaxSlots, States.Num(), "eight slots still");

	// Three players do not fit Small: nothing changes.
	const TArray<int> BeforeStates = States;
	const TArray<int> BeforeTeams = Teams;
	const TArray<int> BeforeColors = Colors;
	T.AssertFalse(HOLMapSizeRead::FitSlotsToSize(States, Teams, Colors, HOLMapSizeSmall), "three players do not fit Small");
	T.AssertTrue(HOLMapSizeAreSame(States, BeforeStates) && HOLMapSizeAreSame(Teams, BeforeTeams) && HOLMapSizeAreSame(Colors, BeforeColors), "nothing changed");

	// Large: every slot open, nothing moves.
	T.AssertTrue(HOLMapSizeRead::FitSlotsToSize(States, Teams, Colors, HOLMapSizeLarge), "Large fits");
	T.AssertTrue(HOLMapSizeAreSame(States, BeforeStates) && HOLMapSizeAreSame(Teams, BeforeTeams) && HOLMapSizeAreSame(Colors, BeforeColors), "nothing moved on Large");

	// The default size of the screen: the smallest that holds the players; the others greyed.
	T.AssertEquals(HOLMapSizeMedium, HOLMapSizeRead::GetDefaultSize(3), "three players: Medium by default");
	T.AssertFalse(HOLMapSizeRead::IsAllowed(HOLMapSizeSmall, 3), "Small greyed for three");
}
