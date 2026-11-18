// The New game screen's layout in script (lot NG-3, D851): the class of Script/UI/HOLNewGameScreen.as is found (FindScriptClass), and once the screen is
// built (BP_BuildNewGame from BuildLayout) its widgets exist under the NAMES the C++ and the scenarios look for. The two widget tests need a player
// controller to create the widget: without one (no world in the test run) they return, like the settings test without its settings object.

UHOLNewGameScreen NGS_CreateScreen()
{
	const TSubclassOf<UHOLNewGameScreen> Found = UHOLNewGameScreen::FindScriptClass();
	APlayerController Controller = Gameplay::GetPlayerController(0);
	if (!Found.IsValid() || Controller == nullptr)
		return nullptr;
	return Cast<UHOLNewGameScreen>(Widget::CreateWidget(Controller, Found));
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
	UHOLNewGameScreen Screen = NGS_CreateScreen();
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
		UHOLDropdown List = Cast<UHOLDropdown>(Screen.GetWidgetFromName(Lists[Index]));
		T.AssertTrue(List != nullptr, "the option list " + Lists[Index].ToString() + " exists");
	}
	T.AssertTrue(Screen.GetWidgetFromName(n"Heading") != nullptr, "the heading exists");
	T.AssertTrue(Screen.GetWidgetFromName(n"LaunchButton") != nullptr, "the launch button exists");
	T.AssertTrue(Screen.GetWidgetFromName(n"BackButton") != nullptr, "the back button exists");
}

void Test_NewGameSlotCount(FUnitTest& T)
{
	UHOLNewGameScreen Screen = NGS_CreateScreen();
	if (Screen == nullptr)
		return;
	// 8 lines of slots: the name of each, and the state list of the slots 1..7 (slot 0 is "Vous", SlotYou0, like the cpp's).
	int Lines = 0;
	for (int SlotIndex = 0; SlotIndex < 8; SlotIndex++)
	{
		if (Screen.GetWidgetFromName(FName("SlotName" + SlotIndex)) != nullptr)
			Lines++;
	}
	T.AssertEquals(Lines, 8, "the screen has 8 lines of slots");
	for (int SlotIndex = 1; SlotIndex < 8; SlotIndex++)
		T.AssertTrue(Screen.GetWidgetFromName(FName("SlotState" + SlotIndex)) != nullptr, "the slot " + SlotIndex + " has its state list");
	T.AssertTrue(Screen.GetWidgetFromName(n"SlotYou0") != nullptr, "the slot 0 is the player's (Vous)");
}
