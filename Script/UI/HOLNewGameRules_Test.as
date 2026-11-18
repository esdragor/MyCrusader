// The New game screen's pure rules (HOLNewGameRules.as, lot NG-2, D851). Expected values are literals, read off the C++ they come from.
// Helpers are prefixed NGR_ (a function name twice in the script folder drops the whole compilation).

void NGR_Fill8(TArray<int>& Out, int A, int B, int C, int D, int E, int F, int G, int H)
{
	Out.Empty();
	Out.Add(A);
	Out.Add(B);
	Out.Add(C);
	Out.Add(D);
	Out.Add(E);
	Out.Add(F);
	Out.Add(G);
	Out.Add(H);
}

// The places by size (HOLMapSizeRules.cpp): None 0, Small 2, Medium 4, Normal 6, Large 8, Huge 8.
void NGR_Places(TArray<int>& Out)
{
	Out.Empty();
	Out.Add(0);
	Out.Add(2);
	Out.Add(4);
	Out.Add(6);
	Out.Add(8);
	Out.Add(8);
}

void Test_ColorTakenIgnoresOwnSlot(FUnitTest& T)
{
	TArray<int> States;
	TArray<int> Colors;
	NGR_Fill8(States, 1, 1, 0, 0, 0, 0, 0, 0);
	NGR_Fill8(Colors, 3, 5, 3, 0, 0, 0, 0, 0);
	// Slot 0 holds colour 3 itself; slot 2 is closed with colour 3 too: neither takes it from slot 0.
	T.AssertFalse(HOLNewGameRules::IsColorTaken(States, Colors, 0, 3), "my own colour is not taken by me, nor by a closed slot");
	T.AssertTrue(HOLNewGameRules::IsColorTaken(States, Colors, 0, 5), "colour 5 is held by the open slot 1");
	T.AssertFalse(HOLNewGameRules::IsColorTaken(States, Colors, 1, 5), "slot 1's own colour is free for slot 1");
	T.AssertTrue(HOLNewGameRules::IsColorTaken(States, Colors, 1, 3), "colour 3 is held by slot 0 for slot 1");
	// Aleatoire is never taken, even when another slot is on it (E2-2).
	NGR_Fill8(Colors, 3, 8, 3, 0, 0, 0, 0, 0);
	T.AssertFalse(HOLNewGameRules::IsColorTaken(States, Colors, 0, 8), "Aleatoire is never taken");
	// An AI holds its colour like a human.
	NGR_Fill8(States, 1, 2, 0, 0, 0, 0, 0, 0);
	NGR_Fill8(Colors, 3, 6, 3, 0, 0, 0, 0, 0);
	T.AssertTrue(HOLNewGameRules::IsColorTaken(States, Colors, 0, 6), "an AI slot holds colour 6");
}

void Test_FindFreeColorSkipsTaken(FUnitTest& T)
{
	TArray<int> States;
	TArray<int> Colors;
	NGR_Fill8(States, 1, 1, 1, 1, 0, 0, 0, 0);
	NGR_Fill8(Colors, 0, 1, 2, 3, 0, 0, 0, 0);
	T.AssertEquals(HOLNewGameRules::FindFreeColor(States, Colors, 0, 1), 4, "from 1: 1, 2, 3 are held, 4 is the first free one");
	T.AssertEquals(HOLNewGameRules::FindFreeColor(States, Colors, 0, 0), 0, "from 0: my own colour is free for me");
	// Wrap: every slot occupied on colours 0..7, slot 3 from Aleatoire (8 % 8 = 0): 0, 1, 2 are held, 3 is its own.
	NGR_Fill8(States, 1, 1, 1, 1, 1, 1, 1, 1);
	NGR_Fill8(Colors, 0, 1, 2, 3, 4, 5, 6, 7);
	T.AssertEquals(HOLNewGameRules::FindFreeColor(States, Colors, 3, 8), 3, "from Aleatoire (8): the search starts at colour 0 and stops at my own 3");
	T.AssertEquals(HOLNewGameRules::FindFreeColor(States, Colors, 0, 6), 0, "from 6: 6 and 7 are held, the wrap reaches my own 0");
	// Everything held by others (a slot 8 that is none of the 8): the colour asked is kept.
	T.AssertEquals(HOLNewGameRules::FindFreeColor(States, Colors, 8, 5), 5, "no free colour: From itself");
}

void Test_FactionListHidesTemplarsWithoutCheat(FUnitTest& T)
{
	TArray<int> Plain = HOLNewGameRules::ListedFactions(false);
	TArray<int> Cheat = HOLNewGameRules::ListedFactions(true);
	T.AssertEquals(Plain.Num(), 5, "without the cheat the list holds five factions");
	T.AssertEquals(Plain[Plain.Num() - 1], 4, "the last listed one is the Fire Heirs, not the Templars");
	T.AssertEquals(Cheat.Num(), 6, "with the cheat the list holds six factions");
	T.AssertEquals(Cheat[Cheat.Num() - 1], 5, "the Templars close the list");
	T.AssertEquals(Plain[0], 0, "the Dawn first");
	T.AssertFalse(HOLNewGameRules::IsFactionChoosable(5, false), "the Templars are not choosable without the cheat");
	T.AssertTrue(HOLNewGameRules::IsFactionChoosable(5, true), "the Templars are choosable with the cheat");
	T.AssertTrue(HOLNewGameRules::IsFactionChoosable(0, false), "the Dawn is choosable");
	T.AssertTrue(HOLNewGameRules::IsFactionChoosable(1, false), "the Black Legions are choosable");
	T.AssertFalse(HOLNewGameRules::IsFactionChoosable(2, true), "the Dragon is listed but greyed, even with the cheat");
	T.AssertEquals(HOLNewGameRules::SanitizeFaction(5, false), 0, "a Templars slot without the cheat is played as the Dawn");
	T.AssertEquals(HOLNewGameRules::SanitizeFaction(5, true), 5, "with the cheat the Templars stay");
	T.AssertEquals(HOLNewGameRules::SanitizeFaction(4, false), 4, "another faction is untouched");
	T.AssertTrue(HOLNewGameRules::FactionNeedsShownText(5, false), "a Templars line without the cheat says its name all the same");
	T.AssertFalse(HOLNewGameRules::FactionNeedsShownText(5, true), "with the cheat the list holds it");
	T.AssertTrue(HOLNewGameRules::FactionListsNeedRebuild(false, false, false), "the first time the lists are built");
	T.AssertFalse(HOLNewGameRules::FactionListsNeedRebuild(true, false, false), "same cheat as last time: nothing to rebuild");
	T.AssertTrue(HOLNewGameRules::FactionListsNeedRebuild(true, false, true), "the cheat changed: rebuilt");
}

void Test_SizeFitsPlayers(FUnitTest& T)
{
	TArray<int> Places;
	NGR_Places(Places);
	T.AssertEquals(HOLNewGameRules::DefaultSize(Places, 6), 3, "6 players: Normal (6 places) is the smallest size that holds them");
	T.AssertEquals(HOLNewGameRules::DefaultSize(Places, 2), 1, "2 players: Small");
	T.AssertEquals(HOLNewGameRules::DefaultSize(Places, 5), 3, "5 players: Normal");
	T.AssertEquals(HOLNewGameRules::DefaultSize(Places, 7), 4, "7 players: Large");
	T.AssertEquals(HOLNewGameRules::DefaultSize(Places, 9), 0, "9 players: no size holds them, None");
	TArray<int> Allowed = HOLNewGameRules::AllowedSizes(Places, 6);
	T.AssertEquals(Allowed.Num(), 3, "6 players: Normal, Large and Huge are allowed");
	T.AssertFalse(HOLNewGameRules::IsSizeAllowed(Places, 2, 6), "Medium (4 places) is greyed for 6 players");
	T.AssertTrue(HOLNewGameRules::IsSizeAllowed(Places, 3, 6), "Normal (6 places) holds 6 players");
	T.AssertFalse(HOLNewGameRules::IsSizeAllowed(Places, 0, 0), "None is never chosen");
	T.AssertEquals(HOLNewGameRules::NumPlacesOfConfig(Places, 0), 8, "no size: all 8 slots");
	T.AssertEquals(HOLNewGameRules::NumPlacesOfConfig(Places, 2), 4, "Medium: 4 places");

	// ChooseSize: 4 players on slots 0, 1, 3, 5.
	TArray<int> States;
	TArray<int> Moves;
	NGR_Fill8(States, 1, 1, 0, 1, 0, 1, 0, 0);
	T.AssertFalse(HOLNewGameRules::ChooseSize(States, Places, 1, Moves), "Small (2 places) cannot hold 4 players");
	T.AssertEquals(States[5], 1, "a refused size leaves the slots as they were");
	T.AssertFalse(HOLNewGameRules::ChooseSize(States, Places, 0, Moves), "None is refused");
	T.AssertTrue(HOLNewGameRules::ChooseSize(States, Places, 2, Moves), "Medium (4 places) holds 4 players");
	T.AssertEquals(States[2], 1, "the player of slot 5 moved onto the first closed slot, 2");
	T.AssertEquals(States[5], 0, "slot 5 is closed");
	T.AssertEquals(Moves[5], 2, "the move says slot 5 goes to slot 2");
	T.AssertEquals(Moves[1], 1, "a slot that stays moves nowhere");
	T.AssertEquals(HOLNewGameRules::NumOccupied(States), 4, "still 4 players");
	TArray<int> Six;
	NGR_Fill8(Six, 1, 1, 1, 1, 1, 1, 0, 0);
	T.AssertTrue(HOLNewGameRules::ChooseSize(Six, Places, 3, Moves), "6 players fit Normal");
	T.AssertEquals(Six[5], 1, "nothing moved");
	T.AssertTrue(HOLNewGameRules::IsKnownSize(Places, 5), "Huge is a size of the table");
	T.AssertFalse(HOLNewGameRules::IsKnownSize(Places, 0), "None is not");
	T.AssertFalse(HOLNewGameRules::IsKnownSize(Places, 6), "a value past the table is not");
	T.AssertTrue(HOLNewGameRules::MapChoiceNeedsDefaultSize(false, 0), "solo, no size: the default one is taken");
	T.AssertFalse(HOLNewGameRules::MapChoiceNeedsDefaultSize(true, 0), "lobby: the manager's size is kept");
}

void Test_StateAIEnablesLevel(FUnitTest& T)
{
	TArray<int> States;
	TArray<int> Colors;
	NGR_Fill8(States, 1, 1, 0, 0, 0, 0, 0, 0);
	NGR_Fill8(Colors, 0, 1, 0, 0, 0, 0, 0, 0);
	T.AssertFalse(HOLNewGameRules::LevelColumnShown(true, States), "a lobby without an AI keeps its width: no level column");
	T.AssertTrue(HOLNewGameRules::LevelColumnShown(false, States), "solo: the level column is always there");
	// Slot 1 becomes an AI (state 2).
	T.AssertTrue(HOLNewGameRules::ApplyStateChosen(States, Colors, 1, 2, 4), "slot 1 is inside the 4 places");
	T.AssertEquals(States[1], 2, "slot 1 is an AI");
	T.AssertTrue(HOLNewGameRules::LevelColumnShown(true, States), "a lobby with an AI shows the level column");
	T.AssertTrue(HOLNewGameRules::LevelCellShown(true, States[1]), "the AI's level list shows");
	T.AssertFalse(HOLNewGameRules::LevelCellShown(true, States[0]), "a human's level list does not");
	T.AssertFalse(HOLNewGameRules::LevelCellShown(false, States[1]), "no level column, no cell");
	T.AssertEquals(Colors[1], 1, "slot 1 keeps its colour: nobody else holds it");
	// Slot 2 opens on a colour (0) held by slot 0: it takes the first free one (1 is slot 1's, 2 is free).
	T.AssertTrue(HOLNewGameRules::ApplyStateChosen(States, Colors, 2, 1, 4), "slot 2 is inside the 4 places");
	T.AssertEquals(Colors[2], 2, "slot 2 left colour 0 for the first free colour, 2");
	// A slot past the places of the size stays closed.
	T.AssertFalse(HOLNewGameRules::ApplyStateChosen(States, Colors, 5, 1, 4), "slot 5 is past the 4 places");
	T.AssertEquals(States[5], 0, "slot 5 stays closed");
	T.AssertFalse(HOLNewGameRules::SlotStateListEnabled(5, 4), "its list is greyed");
	T.AssertTrue(HOLNewGameRules::SlotStateListEnabled(3, 4), "slot 3's list is enabled");
	// Closing keeps the colour.
	T.AssertTrue(HOLNewGameRules::ApplyStateChosen(States, Colors, 1, 0, 4), "slot 1 closes");
	T.AssertEquals(Colors[1], 1, "a closed slot keeps its colour");
	T.AssertFalse(HOLNewGameRules::SlotFieldsActive(States[1]), "a closed slot's faction, colour and team fields are off");
	// The lobby: what a state list choice sends.
	T.AssertEquals(HOLNewGameRules::LobbyStateAction(4, true), 1, "AI chosen: SetSlotAI");
	T.AssertEquals(HOLNewGameRules::LobbyStateAction(4, false), 1, "AI chosen: tested before the client");
	T.AssertEquals(HOLNewGameRules::LobbyStateAction(0, true), 2, "Open: SetSlot open");
	T.AssertEquals(HOLNewGameRules::LobbyStateAction(1, true), 3, "Closed: SetSlot closed");
	T.AssertEquals(HOLNewGameRules::LobbyStateAction(1, false), 0, "no client: nothing sent");
	T.AssertEquals(HOLNewGameRules::LobbyStateAction(3, true), 0, "Ready is not chosen by hand");
	TArray<int> Rows;
	NGR_Fill8(Rows, 2, 3, 0, 1, 0, 0, 0, 0);
	T.AssertTrue(HOLNewGameRules::IsLobbyAI(true, Rows, 1), "row 1 is an AI in a lobby");
	T.AssertFalse(HOLNewGameRules::IsLobbyAI(true, Rows, 0), "row 0 is a player");
	T.AssertFalse(HOLNewGameRules::IsLobbyAI(false, Rows, 1), "solo: never a lobby AI");
	T.AssertFalse(HOLNewGameRules::IsLobbyAI(true, Rows, 9), "an index past the rows is none");
	T.AssertFalse(HOLNewGameRules::IsLobbyAI(true, Rows, -1), "a negative index is none");
}

void Test_LobbyButtonsAndRows(FUnitTest& T)
{
	T.AssertFalse(HOLNewGameRules::ReadyButtonEnabled(-1, true), "no slot of mine: no Ready");
	T.AssertTrue(HOLNewGameRules::ReadyButtonEnabled(2, true), "my slot, lobby not started: Ready enabled");
	T.AssertFalse(HOLNewGameRules::ReadyButtonEnabled(2, false), "lobby started: Ready off");
	T.AssertFalse(HOLNewGameRules::ReadyButtonVisible(true), "a spectator has no Ready");
	T.AssertTrue(HOLNewGameRules::ReadyButtonVisible(false), "a player has it");
	T.AssertFalse(HOLNewGameRules::SpectateButtonEnabled(true, true, false), "a spectator with no open slot cannot take one back");
	T.AssertTrue(HOLNewGameRules::SpectateButtonEnabled(true, true, true), "a spectator with an open slot can play");
	T.AssertTrue(HOLNewGameRules::SpectateButtonEnabled(true, false, false), "a player can always leave his slot");
	T.AssertFalse(HOLNewGameRules::SpectateButtonEnabled(false, false, true), "lobby started: off");
	T.AssertTrue(HOLNewGameRules::SpectateButtonShowsPlay(true), "a spectator's button says Jouer");
	T.AssertTrue(HOLNewGameRules::ReadyButtonShowsUndo(true), "ready: the button says Plus pret");
	TArray<bool> Ready;
	Ready.Add(false);
	Ready.Add(true);
	T.AssertTrue(HOLNewGameRules::IsYouReady(Ready, 1), "my row (1) is ready");
	T.AssertFalse(HOLNewGameRules::IsYouReady(Ready, 0), "my row (0) is not");
	T.AssertFalse(HOLNewGameRules::IsYouReady(Ready, -1), "no row of mine: not ready");
	T.AssertFalse(HOLNewGameRules::IsYouReady(Ready, 5), "a row past the lobby: not ready");
	T.AssertTrue(HOLNewGameRules::LobbyLineEditable(3, 3, false), "my line is mine");
	T.AssertTrue(HOLNewGameRules::LobbyLineEditable(4, 3, true), "an AI's line is the leader's");
	T.AssertFalse(HOLNewGameRules::LobbyLineEditable(4, 3, false), "another player's line is greyed");
	// The state list entry of a lobby row: 0 open, 1 closed, 2 not ready, 3 ready, 4 AI.
	T.AssertEquals(HOLNewGameRules::LobbyListIndex(false, 2, true), 0, "no row: open");
	T.AssertEquals(HOLNewGameRules::LobbyListIndex(true, 0, false), 0, "an open row");
	T.AssertEquals(HOLNewGameRules::LobbyListIndex(true, 1, false), 1, "a closed row");
	T.AssertEquals(HOLNewGameRules::LobbyListIndex(true, 2, false), 2, "a player not ready");
	T.AssertEquals(HOLNewGameRules::LobbyListIndex(true, 2, true), 3, "a player ready");
	T.AssertEquals(HOLNewGameRules::LobbyListIndex(true, 3, true), 4, "an AI has no Ready: AI");
	T.AssertEquals(HOLNewGameRules::SlotStateOfLobbyRow(false, 2), 0, "no row: closed");
	T.AssertEquals(HOLNewGameRules::SlotStateOfLobbyRow(true, 2), 1, "a player: human");
	T.AssertEquals(HOLNewGameRules::SlotStateOfLobbyRow(true, 3), 2, "an AI: AI");
	T.AssertEquals(HOLNewGameRules::SlotStateOfLobbyRow(true, 0), 0, "an open row: closed");
	T.AssertEquals(HOLNewGameRules::ClampLobbyColor(12), 8, "a colour past Aleatoire is clamped to it");
	T.AssertEquals(HOLNewGameRules::ClampLobbyColor(-3), 0, "a negative colour is 0");
	T.AssertEquals(HOLNewGameRules::ClampLobbyTeam(9, 4), 4, "a team past the maximum is clamped");
}
