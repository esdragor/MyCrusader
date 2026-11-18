// The New game screen's layout in script (lot NG-3, D851): the class of Script/UI/HOLNewGameScreen.as is found (FindScriptClass), and once the screen is
// built (BP_BuildNewGame from BuildLayout) its widgets exist under the NAMES the C++ and the scenarios look for. The two widget tests need a player
// controller to create the widget: without one (or if the screen does not create) they FAIL, they never pass silently.

// The screen created for a test, or nullptr (the caller then FAILS: a test never passes without having looked).
UHOLNewGameScreen NGS_CreateScreen(FUnitTest& T)
{
	const TSubclassOf<UHOLNewGameScreen> Found = UHOLNewGameScreen::FindScriptClass();
	if (!Found.IsValid())
	{
		T.AssertTrue(false, "the New game screen has a script class to create");
		return nullptr;
	}
	// A script unit test's world has no local player: the C++ test library builds the world, the player and the screen the way the game does.
	TSubclassOf<UUserWidget> Wanted = Found;
	UHOLNewGameScreen Screen = Cast<UHOLNewGameScreen>(HOLScreenTest::CreateScreenForTest(Wanted));
	if (Screen == nullptr)
		T.AssertTrue(false, "the New game screen is created");
	return Screen;
}

// A widget of the screen by its name (the widget tree's own search, through the test library).
UWidget NGS_Find(UHOLNewGameScreen Screen, FName Name)
{
	return HOLScreenTest::FindWidgetByName(Screen, Name);
}

void Test_NewGameScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLNewGameScreen> Found = UHOLNewGameScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the New game screen has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLNewGameScreenScript", "the New game screen's script class is HOLNewGameScreen.as's");
}

void Test_NewGameWidgets(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen(T);
	if (Screen == nullptr)
		return;
	// The option lists of the top (map, size, teams together, fog, mode) and of the bottom (nomad, regicide, catastrophe, sacred sites).
	TArray<FName> Lists;
	Lists.Add(n"MapDropdown");
	Lists.Add(n"SizeDropdown");
	Lists.Add(n"TeamsTogetherDropdown");
	Lists.Add(n"FogDropdown");
	Lists.Add(n"ModeDropdown");
	Lists.Add(n"NomadDropdown");
	Lists.Add(n"RegicideDropdown");
	Lists.Add(n"CatastropheDropdown");
	Lists.Add(n"SacredSitesDropdown");
	for (int Index = 0; Index < Lists.Num(); Index++)
	{
		UHOLDropdown List = Cast<UHOLDropdown>(NGS_Find(Screen, Lists[Index]));
		T.AssertTrue(List != nullptr, "the option list " + Lists[Index].ToString() + " exists");
	}
	T.AssertTrue(NGS_Find(Screen, n"Heading") != nullptr, "the heading exists");
	T.AssertTrue(NGS_Find(Screen, n"LaunchButton") != nullptr, "the launch button exists");
	T.AssertTrue(NGS_Find(Screen, n"BackButton") != nullptr, "the back button exists");
	HOLScreenTest::EndScreenTest();
}

void Test_NewGameSlotCount(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen(T);
	if (Screen == nullptr)
		return;
	// 8 lines of slots: the name of each, and the state list of the slots 1..7 (slot 0 is "Vous", SlotYou0, like the cpp's).
	int Lines = 0;
	for (int SlotIndex = 0; SlotIndex < 8; SlotIndex++)
	{
		if (NGS_Find(Screen, FName("SlotName" + SlotIndex)) != nullptr)
			Lines++;
	}
	T.AssertEquals(Lines, 8, "the screen has 8 lines of slots");
	for (int SlotIndex = 1; SlotIndex < 8; SlotIndex++)
		T.AssertTrue(NGS_Find(Screen, FName("SlotState" + SlotIndex)) != nullptr, "the slot " + SlotIndex + " has its state list");
	T.AssertTrue(NGS_Find(Screen, n"SlotYou0") != nullptr, "the slot 0 is the player's (Vous)");
	HOLScreenTest::EndScreenTest();
}

// ---- Effect tests of the SOLO gestures (NG-5): a value is chosen in a list through UHOLDropdown::Choose (it broadcasts OnChosen as a click does) and the
// effect is read on ANOTHER widget or on the configuration. The LOBBY is not tested here: it needs a manager and a read FHOLLobby (no network in a script
// test, no FHOLLobby built from script): it is looked at with two instances. ----

UHOLDropdown NGS_FindList(UHOLNewGameScreen Screen, FName Name)
{
	return Cast<UHOLDropdown>(NGS_Find(Screen, Name));
}

// A slot put on "closed" greys its faction and colour lists; put on "AI" they are active again and the state list holds the AI.
void Test_NewGameSoloStateGivesTheLineItsFields(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen(T);
	if (Screen == nullptr)
		return;
	UHOLDropdown State = NGS_FindList(Screen, n"SlotState1");
	T.AssertTrue(State != nullptr, "the slot 1 has its state list");
	if (State != nullptr)
	{
		State.Choose(0);
		T.AssertTrue(!HOLScreenTest::IsWidgetEnabled(NGS_Find(Screen, n"SlotFaction1")), "a closed slot's faction list is greyed");
		T.AssertTrue(!HOLScreenTest::IsWidgetEnabled(NGS_Find(Screen, n"SlotColor1")), "a closed slot's colour list is greyed");
		State.Choose(2);
		T.AssertEquals(HOLScreenTest::GetDropdownIndex(State), 2, "the slot 1 state list shows the AI");
		T.AssertTrue(HOLScreenTest::IsWidgetEnabled(NGS_Find(Screen, n"SlotFaction1")), "a slot put on AI has its faction list active");
		T.AssertTrue(HOLScreenTest::IsWidgetEnabled(NGS_Find(Screen, n"SlotColor1")), "a slot put on AI has its colour list active");
	}
	HOLScreenTest::EndScreenTest();
}

// ELROND-1118w: quick_check "slot 5 past the 4 places of Medium stays closed" reads the journal line the cpp wrote (HandleStateChosen); the script did not write it.
// Medium (4 places) chosen in the size list: the slots 4..7 are greyed and closed; a choice on slot 5 changes nothing and says why.
void Test_NewGameSoloSlotPastThePlacesStaysClosed(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen(T);
	if (Screen == nullptr)
		return;
	UHOLNewGameScreenScript Script = Cast<UHOLNewGameScreenScript>(Screen);
	UHOLDropdown Size = NGS_FindList(Screen, n"SizeDropdown");
	UHOLDropdown State5 = NGS_FindList(Screen, n"SlotState5");
	T.AssertTrue(Script != nullptr && Size != nullptr && State5 != nullptr, "the script screen, its size list and the slot 5 state list exist");
	if (Script != nullptr && Size != nullptr && State5 != nullptr)
	{
		Size.Choose(2);
		T.AssertEquals(HOLScreenTest::GetDropdownIndex(Size), 2, "Medium is chosen");
		T.AssertTrue(HOLScreenTest::IsWidgetEnabled(NGS_Find(Screen, n"SlotState3")), "the slot 3 is within the 4 places: its state list is active");
		T.AssertTrue(!HOLScreenTest::IsWidgetEnabled(NGS_Find(Screen, n"SlotState5")), "the slot 5 is past the 4 places: its state list is greyed");
		State5.Choose(1);
		T.AssertEquals(HOLScreenTest::GetDropdownIndex(State5), 0, "the slot 5 stays closed");
		T.AssertTrue(!HOLScreenTest::IsWidgetEnabled(NGS_Find(Screen, n"SlotFaction5")), "the slot 5 holds no faction list");
		const FString Line = Script.LastLogLine;
		T.AssertEquals(Line, "New game: slot 5 stays closed, past the 4 places of the size", "the journal says the slot 5 stays closed");
	}
	HOLScreenTest::EndScreenTest();
}

// The fog, mode and faction gestures write the cpp's journal lines (the fog and faction scenarios read them).
void Test_NewGameSoloGesturesWriteTheirJournalLine(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen(T);
	if (Screen == nullptr)
		return;
	UHOLNewGameScreenScript Script = Cast<UHOLNewGameScreenScript>(Screen);
	UHOLDropdown Fog = NGS_FindList(Screen, n"FogDropdown");
	UHOLDropdown Mode = NGS_FindList(Screen, n"ModeDropdown");
	UHOLDropdown Faction = NGS_FindList(Screen, n"SlotFaction0");
	T.AssertTrue(Script != nullptr && Fog != nullptr && Mode != nullptr && Faction != nullptr, "the script screen and its fog, mode and faction lists exist");
	if (Script != nullptr && Fog != nullptr && Mode != nullptr && Faction != nullptr)
	{
		Fog.Choose(1);
		T.AssertEquals(Script.LastLogLine, "New game: fog Explored", "fog list value 1 is Explored (2 - 1)");
		Mode.Choose(1);
		T.AssertEquals(Script.LastLogLine, "New game: mode Diplomacy", "mode list value 1 is Diplomacy");
		Faction.Choose(1); // the slot 0 opens on the faction 0: choosing 0 again changes nothing and writes nothing
		T.AssertTrue(Script.LastLogLine.StartsWith("New game: slot 0 faction "), "a faction chosen on the slot 0 is written in the journal");
	}
	HOLScreenTest::EndScreenTest();
}

void Test_NewGameJournalLines(FUnitTest& T)
{
	T.AssertEquals(HOLNewGameLog::StaysClosedLine(5, 4), "New game: slot 5 stays closed, past the 4 places of the size", "slot line");
	T.AssertEquals(HOLNewGameLog::FogLine(2), "New game: fog Normal", "fog 2");
	T.AssertEquals(HOLNewGameLog::FogLine(0), "New game: fog Disabled", "fog 0");
	T.AssertEquals(HOLNewGameLog::FogLine(9), "New game: fog Normal", "fog clamped");
	T.AssertEquals(HOLNewGameLog::ModeLine(1), "New game: mode Diplomacy", "mode 1");
	T.AssertEquals(HOLNewGameLog::ModeLine(0), "New game: mode Standard", "mode 0");
	T.AssertEquals(HOLNewGameLog::FactionLine(3, "Human"), "New game: slot 3 faction Human", "faction line");
}

// A slot that takes a seat on a colour another slot holds gives way to a free colour (HOLNewGameRules::ApplyStateChosen).
void Test_NewGameSoloTakenColourGivesWay(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen(T);
	if (Screen == nullptr)
		return;
	UHOLDropdown State = NGS_FindList(Screen, n"SlotState1");
	T.AssertTrue(State != nullptr, "the slot 1 has its state list");
	if (State != nullptr)
	{
		State.Choose(0);
		const int HeldColour = HOLScreenTest::GetDropdownIndex(NGS_Find(Screen, n"SlotColor0"));
		T.AssertTrue(HeldColour >= 0 && HeldColour < 8, "the slot 0 holds a real colour");
		// The closed slot 1 is put on the colour of the slot 0, then takes a seat: the colour must move.
		Screen.SetConfigSlot(1, -1, -1, HeldColour, -1);
		Screen.Refresh();
		State.Choose(1);
		const int NewColour = HOLScreenTest::GetDropdownIndex(NGS_Find(Screen, n"SlotColor1"));
		T.AssertTrue(NewColour >= 0 && NewColour < 8, "the slot 1 holds a real colour");
		T.AssertTrue(NewColour != HeldColour, "a colour another slot holds gives way to a free one");
	}
	HOLScreenTest::EndScreenTest();
}

// The fog and "Equipes ensemble" lists write the configuration (read back as its JSON): the fog list is ordered Normal, Explored, Disabled.
void Test_NewGameSoloGlobalsGesture(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen(T);
	if (Screen == nullptr)
		return;
	UHOLDropdown Fog = NGS_FindList(Screen, n"FogDropdown");
	UHOLDropdown Together = NGS_FindList(Screen, n"TeamsTogetherDropdown");
	T.AssertTrue(Fog != nullptr && Together != nullptr, "the fog and teams together lists exist");
	if (Fog != nullptr && Together != nullptr)
	{
		Fog.Choose(2);
		Together.Choose(0);
		const FString Json = Screen.GetConfigJson();
		int FogMode = 2;
		bool bTeamsTogether = true;
		FString Biome;
		int Peasants = 0;
		TArray<int> Difficulties;
		TArray<int> AIBrains;
		T.AssertTrue(HOLNewGameRead::DescribeGlobals(Json, FogMode, bTeamsTogether, Biome, Peasants, Difficulties, AIBrains), "the configuration is readable");
		T.AssertEquals(FogMode, 0, "the last fog entry (Disabled) is fog mode 0");
		T.AssertTrue(!bTeamsTogether, "\"Equipes ensemble\" on its first entry is off");
	}
	HOLScreenTest::EndScreenTest();
}
