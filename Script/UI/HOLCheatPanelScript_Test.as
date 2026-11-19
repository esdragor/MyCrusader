// NOT COMPILED YET (GALADRIEL-1119m, H-11a; needs the C++ lot of the recipe for HOLCheatPanelScript.as, same module). Rules of the cheats' panel (HOLCheatPanelRules), pure, in literals copied from
// the behaviour of HOLCheatPanel.cpp: the seven buttons in order, the command of each, the plan that command gives through the REAL controller commands (HOLControllerCommands::PlanOf, the
// route each click takes), the labels, the journal and report lines, the greyed and server-followed states. RED TO SEE once the module compiles: falsify one expected value, run, restore.

FString CheatPlanRoute(int Index, bool bFastBuildBefore)
{
	const FHOLCheatPlan Plan = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(Index), HOLCheatPanelRules::ArgsOf(Index, bFastBuildBefore));
	return Plan.Route;
}

void Test_CheatPanelButtonsAreTheSevenInTheCppOrder(FUnitTest& T)
{
	T.AssertEquals(HOLCheatPanelRules::NumButtons(), 7, "seven cheat buttons under the toggle");
	T.AssertEquals(HOLCheatPanelRules::ButtonName(0), "CheatFood", "first: food");
	T.AssertEquals(HOLCheatPanelRules::ButtonName(1), "CheatWood", "wood");
	T.AssertEquals(HOLCheatPanelRules::ButtonName(2), "CheatStone", "stone");
	T.AssertEquals(HOLCheatPanelRules::ButtonName(3), "CheatGold", "gold");
	T.AssertEquals(HOLCheatPanelRules::ButtonName(4), "CheatAll", "all");
	T.AssertEquals(HOLCheatPanelRules::ButtonName(5), "CheatPopulation", "population");
	T.AssertEquals(HOLCheatPanelRules::ButtonName(6), "CheatFastBuild", "last: fast build");
	T.AssertEquals(HOLCheatPanelRules::ToggleName(), "CheatToggle", "the toggle's name (scenarios click it)");
	T.AssertEquals(HOLCheatPanelRules::IndexOfButton("CheatGold"), 3, "hol.HUD.Click finds a button by its name");
	T.AssertEquals(HOLCheatPanelRules::IndexOfButton("CheatToggle"), -1, "the toggle is not a cheat button");
	T.AssertEquals(HOLCheatPanelRules::IndexOfButton("MenuButton"), -1, "another panel's widget is not ours");
	T.AssertEquals(HOLCheatPanelRules::SpeedName(0), "CheatUnitSpeed", "first slider");
	T.AssertEquals(HOLCheatPanelRules::SpeedName(1), "CheatGatherSpeed", "second slider");
}

void Test_CheatPanelEveryButtonPlansTheRouteOfTheCppRequest(FUnitTest& T)
{
	// The click goes through the controller commands' plan: the five resource buttons and nothing else take ServerCheatResources with the C++ amounts.
	T.AssertEquals(CheatPlanRoute(0, false), "ServerCheatResources", "food: resources route");
	T.AssertEquals(CheatPlanRoute(4, false), "ServerCheatResources", "all: resources route");
	T.AssertEquals(CheatPlanRoute(5, false), "ServerCheatPopulation", "population route");
	T.AssertEquals(CheatPlanRoute(6, false), "ServerCheatFastBuild", "fast build route");

	const FHOLCheatPlan Food = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(0), HOLCheatPanelRules::ArgsOf(0, false));
	T.AssertEquals(Food.Ints.Num(), 4, "four amounts");
	const int FoodFood = Food.Ints[0];
	const int FoodWood = Food.Ints[1];
	T.AssertEquals(FoodFood, 1000, "food button: +1000 food (FHOLResources(1000,0,0,0))");
	T.AssertEquals(FoodWood, 0, "food button: no wood");

	const FHOLCheatPlan Gold = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(3), HOLCheatPanelRules::ArgsOf(3, false));
	const int GoldFood = Gold.Ints[0];
	const int GoldGold = Gold.Ints[3];
	T.AssertEquals(GoldFood, 0, "gold button: no food");
	T.AssertEquals(GoldGold, 1000, "gold button: +1000 gold (FHOLResources(0,0,0,1000))");

	const FHOLCheatPlan All = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(4), HOLCheatPanelRules::ArgsOf(4, false));
	const int AllFood = All.Ints[0];
	const int AllStone = All.Ints[2];
	const int AllGold = All.Ints[3];
	T.AssertEquals(AllFood, 1000, "all button: food");
	T.AssertEquals(AllStone, 1000, "all button: stone");
	T.AssertEquals(AllGold, 1000, "all button: gold");

	const FHOLCheatPlan Population = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(5), HOLCheatPanelRules::ArgsOf(5, false));
	const int Places = Population.Ints[0];
	T.AssertEquals(Places, 165, "population button: +165 places (PopulationPlaces)");
}

void Test_CheatPanelFastBuildClickAsksTheOppositeState(FUnitTest& T)
{
	const FHOLCheatPlan TurnOn = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(6), HOLCheatPanelRules::ArgsOf(6, false));
	const int On = TurnOn.Ints[0];
	T.AssertEquals(On, 1, "off before the click: it asks on (1)");
	const FHOLCheatPlan TurnOff = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(6), HOLCheatPanelRules::ArgsOf(6, true));
	const int Off = TurnOff.Ints[0];
	T.AssertEquals(Off, 0, "on before the click: it asks off (0)");
}

void Test_CheatPanelLabelsAreTheCppOnes(FUnitTest& T)
{
	T.AssertEquals(HOLCheatPanelText::CheatLabel(0).ToString(), "+1000 Nourriture", "food label");
	T.AssertEquals(HOLCheatPanelText::CheatLabel(4).ToString(), "+1000 de tout", "all label");
	T.AssertEquals(HOLCheatPanelText::CheatLabel(5).ToString(), "Population +165", "population label");
	T.AssertEquals(HOLCheatPanelRules::ToggleLabel(false).ToString(), "Triches  +", "folded toggle (two spaces)");
	T.AssertEquals(HOLCheatPanelRules::ToggleLabel(true).ToString(), "Triches  -", "open toggle");
	T.AssertEquals(HOLCheatPanelRules::FastBuildLabel(true).ToString(), "Construction rapide : oui", "fast build on");
	T.AssertEquals(HOLCheatPanelRules::FastBuildLabel(false).ToString(), "Construction rapide : non", "fast build off");
	T.AssertEquals(HOLCheatPanelRules::SpeedLabelText(0, 1.0), "Vitesse des unités : x1.0", "unit speed label, one decimal");
	T.AssertEquals(HOLCheatPanelRules::SpeedLabelText(1, 2.5), "Vitesse de collecte : x2.5", "gather speed label");
}

void Test_CheatPanelJournalAndReportLinesAreTheCppOnes(FUnitTest& T)
{
	T.AssertEquals(HOLCheatPanelRules::ClickLine(0), "HUD: cheat CheatFood clicked", "a button's click line");
	T.AssertEquals(HOLCheatPanelRules::ClickLine(6), "HUD: cheat CheatFastBuild clicked", "the fast build's");
	T.AssertEquals(HOLCheatPanelRules::ToggleLine(true), "HUD: cheats open", "open line");
	T.AssertEquals(HOLCheatPanelRules::ToggleLine(false), "HUD: cheats folded", "folded line");
	T.AssertEquals(HOLCheatPanelRules::FoldLine(600.0), "HUD: cheats folded (window 600 px high, the hero's block opened)", "the low-window fold line");
	T.AssertEquals(HOLCheatPanelRules::SpeedLine(0, 1.5), "HUD: cheat unit speed x1.5", "unit speed line");
	T.AssertEquals(HOLCheatPanelRules::SpeedLine(1, 3.0), "HUD: cheat gather speed x3.0", "gather speed line");
	T.AssertEquals(HOLCheatPanelRules::Describe(false, true, 2.0, 2.0), "cheats folded", "folded: nothing else (heros_triches_600 waits for it)");
	T.AssertEquals(HOLCheatPanelRules::Describe(true, false, 1.0, 1.0), "cheats open, fast build off, unit speed x1.0, gather speed x1.0", "open, defaults");
	T.AssertEquals(HOLCheatPanelRules::Describe(true, true, 1.5, 5.0), "cheats open, fast build on, unit speed x1.5, gather speed x5.0", "open, changed");
}

void Test_CheatPanelSpeedsMoveByStepsAndClamp(FUnitTest& T)
{
	T.AssertEquals(HOLCheatPanelRules::OneDecimal(HOLCheatPanelRules::SnapSpeed(1.2)), "1.0", "1.2 snaps to 1.0");
	T.AssertEquals(HOLCheatPanelRules::OneDecimal(HOLCheatPanelRules::SnapSpeed(1.3)), "1.5", "1.3 snaps to 1.5");
	T.AssertEquals(HOLCheatPanelRules::OneDecimal(HOLCheatPanelRules::SnapSpeed(0.1)), "0.5", "under the range: 0.5");
	T.AssertEquals(HOLCheatPanelRules::OneDecimal(HOLCheatPanelRules::SnapSpeed(9.0)), "5.0", "over the range: 5.0");
	T.AssertFalse(HOLCheatPanelRules::SpeedChanged(1.0, 1.2), "the drag stays inside the same step: no request");
	T.AssertTrue(HOLCheatPanelRules::SpeedChanged(1.0, 1.4), "the step changed: one request");
	T.AssertFalse(HOLCheatPanelRules::SpeedChanged(5.0, 7.0), "already at the top: no request");
}

void Test_CheatPanelGreyedAndServerFollowedStates(FUnitTest& T)
{
	T.AssertFalse(HOLCheatPanelRules::ButtonClickable(false), "folded: a cheat button click is refused (hol.HUD.Click: hidden)");
	T.AssertTrue(HOLCheatPanelRules::ButtonClickable(true), "open: it answers");
	T.AssertTrue(HOLCheatPanelRules::FastBuildFollows(true, 0.0, true), "solo, the server's build time is 0 but the panel says on: it follows to off");
	T.AssertTrue(HOLCheatPanelRules::FastBuildFollows(true, 0.5, false), "solo, build time above 0 and the panel says off: it follows to on");
	T.AssertFalse(HOLCheatPanelRules::FastBuildFollows(true, 0.5, true), "agreement: nothing to follow");
	T.AssertFalse(HOLCheatPanelRules::FastBuildFollows(false, 0.5, false), "a client never learns another player's change");
	T.AssertTrue(HOLCheatPanelRules::SpeedFollows(true, 2.0, 1.0), "solo: the console changed the speed, the slider follows");
	T.AssertFalse(HOLCheatPanelRules::SpeedFollows(true, 1.0, 1.0), "same speed: nothing");
	T.AssertFalse(HOLCheatPanelRules::SpeedFollows(false, 2.0, 1.0), "client: it keeps what it asked");
}

void Test_CheatPanelStaysUnderTheTopBar(FUnitTest& T)
{
	T.AssertEquals(Math::RoundToInt(HOLCheatPanelRules::TopOf(0.0)), 16, "no top bar yet: at the margin");
	T.AssertEquals(Math::RoundToInt(HOLCheatPanelRules::TopOf(40.0)), 62, "margin 16 + bar 40 + gap 6");
	T.AssertEquals(Math::RoundToInt(HOLCheatPanelRules::BottomOf(40.0, 200.0, true)), 262, "the AI level line starts under 62 + 200");
	T.AssertEquals(Math::RoundToInt(HOLCheatPanelRules::BottomOf(40.0, 200.0, false)), 0, "without the panel: 0");
}
