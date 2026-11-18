// The New game screen in script (D851, PLAN-ECRANS-1118.md). Lot NG-3: the layout (UHOLNewGameScreen::BuildLayout calls BuildNewGame for a derived class;
// the C++ layout stays the one of the class itself until the switch, NG-6). Lot NG-4: the SOLO behaviour (the gestures of the lists).
// This file lays out what HOLNewGameScreen.cpp BuildLayout lays out (224-345), in the same order, with the same widths (GetNewGameWidth, read from the
// cpp's constants) and the same widget NAMES (the scenarios, quick_check and the C++ Refresh / ShowLobby find the widgets by name): Heading,
// MapDropdown, FogDropdown, SizeDropdown, TeamsTogetherDropdown, HeadName, HeadSlot, HeadLevel, then per slot n = 0..7 SlotName<n>, SlotYou0 (slot 0
// only) or SlotState<n> (slots 1..7), SlotLevel<n>, SlotFaction<n>, SlotSwatch<n>, SlotColor<n>, SlotTeam<n>; ModeDropdown, NomadDropdown,
// RegicideDropdown, CatastropheDropdown, SacredSitesDropdown, SpectatorsText, Note (the cpp's name of the note: it is "Note", not "NoteText"),
// ErrorText, BackButton, ReadyButton, SpectateButton, LaunchButton.
// NG-4: `default bScriptOwnsGestures = true` makes the base NOT subscribe its Handle*Chosen; this class subscribes UHOLDropdown::OnChosen itself
// (BuildNewGame, with the lists it has just created) and applies each gesture: read the configuration (GetConfigJson + HOLMatchConfigRead::Describe,
// HOLNewGameRead::DescribeGlobals), apply the pure rule (HOLNewGameRules::), write by the base's SetConfig* / ChooseNewGameSize, then Refresh() (C++,
// the widgets). Launch, ShowError, Back, focus stay the base's.
// NG-5: the LOBBY (IsLobby()): each gesture sends to the manager (SendSettings, SendSlotAIChoice, SendSlotOpen, SendMyChoice) instead of only writing the
// local configuration, by the rule HOLNewGameRules::LobbyStateAction; ShowLobby() shows a read lobby (GetLobbyView) on the widgets. EnterLobby, ReadLobby,
// JoinMatch, the network sends and the Ready / Spectator buttons (wired by the base to ToggleReady / ToggleSpectator) stay the base's.
class UHOLNewGameScreenScript : UHOLNewGameScreen
{
	default bScriptOwnsGestures = true;
	// NG-5: the lobby display is this class's (the base then calls ShowLobby() below instead of its own ShowLobbyWidgets()).
	default bScriptOwnsLobbyDisplay = true;

	// The cpp's FHOLMatchConfig::MaxSlots (one line per slot).
	const int SlotCount = 8;

	// The lists created by BuildNewGame (null where the base refused to create one), to know which list a choice comes from.
	UHOLDropdown MapList;
	UHOLDropdown SizeList;
	UHOLDropdown TeamsTogetherList;
	UHOLDropdown FogList;
	UHOLDropdown ModeList;
	UHOLDropdown NomadList;
	UHOLDropdown RegicideList;
	UHOLDropdown CatastropheList;
	UHOLDropdown SacredSitesList;
	TArray<UHOLDropdown> SlotStateLists;
	TArray<UHOLDropdown> SlotFactionLists;
	TArray<UHOLDropdown> SlotColorLists;
	TArray<UHOLDropdown> SlotTeamLists;

	// NG-5: the widgets the lobby display writes (the base's own members are protected C++ fields; the script keeps what AddNewGame* returned).
	UCommonTextBlock LobbyHeading;
	UCommonTextBlock LobbySpectators;
	TArray<UCommonTextBlock> LobbySlotNames;
	UCommonTextBlock LobbyYouText;
	UHOLButton LobbyReadyButton;
	UHOLButton LobbySpectateButton;
	UHOLButton LobbyLaunchButton;

	// The configuration as ReadConfig last read it (numbers are the enums': state 0 Closed 1 Human 2 AI; size 0 None..5 Huge; fog 0 Disabled..2 Normal).
	int CfgVersion = 0;
	int CfgSeed = 0;
	FString CfgMap;
	int CfgMapSize = 0;
	bool bCfgDrawPlaces = false;
	int CfgMode = 0;
	bool bCfgNomad = false;
	bool bCfgRegicide = false;
	bool bCfgCatastrophe = false;
	bool bCfgSacredSites = false;
	TArray<int> CfgStates;
	TArray<int> CfgFactions;
	TArray<int> CfgColors;
	TArray<int> CfgTeams;
	TArray<FString> CfgNames;
	int CfgFogMode = 2;
	bool bCfgTeamsTogether = true;

	// The name of a per-slot widget: the cpp's SlotWidgetName (Prefix + index, "SlotState1").
	FName SlotWidgetName(FString Prefix, int SlotIndex)
	{
		return FName(Prefix + SlotIndex);
	}

	// ---- Reading the configuration (the base gives it as JSON text) ----

	// False (nothing read, the members unchanged or partly set) when the JSON is unreadable: the gesture then does nothing.
	bool ReadConfig()
	{
		const FString Json = GetConfigJson();
		if (!HOLMatchConfigRead::Describe(Json, CfgVersion, CfgSeed, CfgMap, CfgMapSize, bCfgDrawPlaces, CfgMode, bCfgNomad, bCfgRegicide,
			bCfgCatastrophe, bCfgSacredSites, CfgStates, CfgFactions, CfgColors, CfgTeams, CfgNames))
			return false;
		FString Biome;
		int Peasants = 0;
		TArray<int> Difficulties;
		TArray<int> AIBrains;
		return HOLNewGameRead::DescribeGlobals(Json, CfgFogMode, bCfgTeamsTogether, Biome, Peasants, Difficulties, AIBrains);
	}

	// The places of each size, indexed by the size's number (0 None = 0, then Small..Huge): the table HOLNewGameRules:: takes as a parameter.
	TArray<int> PlacesBySize()
	{
		TArray<int> Result;
		Result.Add(0);
		const int NumSizes = HOLMapSizeRead::GetNumSizes();
		for (int SizeIndex = 0; SizeIndex < NumSizes; SizeIndex++)
			Result.Add(HOLMapSizeRead::GetNumPlaces(HOLMapSizeRead::GetSizeAt(SizeIndex)));
		return Result;
	}

	// The slot a list belongs to (-1: none of this array).
	int SlotOfList(const TArray<UHOLDropdown>& Lists, UHOLDropdown List)
	{
		for (int SlotIndex = 0; SlotIndex < Lists.Num(); SlotIndex++)
		{
			if (Lists[SlotIndex] != nullptr && Lists[SlotIndex] == List)
				return SlotIndex;
		}
		return -1;
	}

	// The choice event of a list (UHOLDropdown::OnChosen, (UHOLDropdown, int32)); a null list (the base made none) is skipped.
	void Subscribe(UHOLDropdown List, FName Handler)
	{
		if (List != nullptr)
			List.OnChosen.AddUFunction(this, Handler);
	}

	UFUNCTION(BlueprintOverride)
	void BuildNewGame()
	{
		// Widths and lists come from the base (the cpp's constants and name lists), never copied here.
		const float NameWidth = UHOLNewGameScreen::GetNewGameWidth(n"Name");
		const float StateWidth = UHOLNewGameScreen::GetNewGameWidth(n"State");
		const float FactionWidth = UHOLNewGameScreen::GetNewGameWidth(n"Faction");
		const float SwatchSize = UHOLNewGameScreen::GetNewGameWidth(n"Swatch");
		const float ColorWidth = UHOLNewGameScreen::GetNewGameWidth(n"Color");
		const float TeamWidth = UHOLNewGameScreen::GetNewGameWidth(n"Team");
		const float LevelWidth = UHOLNewGameScreen::GetNewGameWidth(n"Level");
		const float OptionWidth = UHOLNewGameScreen::GetNewGameWidth(n"Option");
		const float ToggleWidth = UHOLNewGameScreen::GetNewGameWidth(n"Toggle");
		const float OptionLabelWidth = UHOLNewGameScreen::GetNewGameWidth(n"OptionLabel");
		const float TeamsTogetherLabelWidth = UHOLNewGameScreen::GetNewGameWidth(n"TeamsTogetherLabel");
		const float MapListWidth = UHOLNewGameScreen::GetNewGameWidth(n"MapList");
		const float FogListWidth = UHOLNewGameScreen::GetNewGameWidth(n"FogList");

		const TArray<FText> OffOn = UHOLNewGameScreen::GetNewGameOptions(n"OffOn");

		// A screen built once per object; the arrays start empty (slot n at index n).
		SlotStateLists.Empty();
		SlotFactionLists.Empty();
		SlotColorLists.Empty();
		SlotTeamLists.Empty();
		LobbySlotNames.Empty();

		// Text styles of AddNewGameText / AddNewGameCellText: 0 heading, 1 note, 2 body.
		UVerticalBox Column = MakeNewGameColumn();
		LobbyHeading = AddNewGameText(Column, n"Heading", UHOLNewGameScreen::GetNewGameText(n"Heading"), 0);

		// The map line: the map, then the fog (F8, over the "Equipes ensemble" of the line below).
		UHorizontalBox MapLine = AddNewGameRow(Column, 0.0);
		AddNewGameCellText(MapLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Map"), 2, NameWidth);
		MapList = AddNewGameDropdown(MapLine, n"MapDropdown", GetMapNames(), 0, MapListWidth);
		AddNewGameCellText(MapLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Fog"), 2, TeamsTogetherLabelWidth);
		FogList = AddNewGameDropdown(MapLine, n"FogDropdown", UHOLNewGameScreen::GetNewGameOptions(n"Fog"), 0, FogListWidth);

		// TC3: the size under the map, then "Equipes ensemble" (the places are drawn at the launch).
		UHorizontalBox SizeLine = AddNewGameRow(Column, 0.0);
		AddNewGameCellText(SizeLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Size"), 2, NameWidth);
		SizeList = AddNewGameDropdown(SizeLine, n"SizeDropdown", UHOLNewGameScreen::GetNewGameOptions(n"Size"), 0, MapListWidth);
		AddNewGameCellText(SizeLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"TeamsTogether"), 2, TeamsTogetherLabelWidth);
		TeamsTogetherList = AddNewGameDropdown(SizeLine, n"TeamsTogetherDropdown", OffOn, 1, TeamWidth);

		// The column headings (IA0.6 S2: HeadName / HeadSlot / HeadLevel are shown or hidden by Refresh, solo or lobby).
		UHorizontalBox Headings = AddNewGameRow(Column, 16.0);
		AddNewGameCellText(Headings, n"HeadName", UHOLNewGameScreen::GetNewGameText(n"Player"), 1, NameWidth);
		AddNewGameCellText(Headings, n"HeadSlot", UHOLNewGameScreen::GetNewGameText(n"Player"), 1, StateWidth);
		AddNewGameCellText(Headings, n"HeadLevel", UHOLNewGameScreen::GetNewGameText(n"Level"), 1, LevelWidth);
		AddNewGameCellText(Headings, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Faction"), 1, FactionWidth);
		AddNewGameCellText(Headings, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Colour"), 1, SwatchSize + 8.0 + ColorWidth);
		AddNewGameCellText(Headings, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Team"), 1, TeamWidth);

		// One line per slot: name, state ("Vous" for slot 0), AI level, faction, swatch, colour, team.
		const TArray<FText> StateNames = UHOLNewGameScreen::GetNewGameOptions(n"SlotState");
		const TArray<FText> LevelNames = UHOLNewGameScreen::GetNewGameOptions(n"Level");
		const TArray<FText> FactionNames = UHOLNewGameScreen::GetNewGameOptions(n"Faction");
		const TArray<FText> ColorNames = UHOLNewGameScreen::GetNewGameOptions(n"Color");
		const TArray<FText> TeamNames = UHOLNewGameScreen::GetNewGameOptions(n"Team");
		const TArray<FLinearColor> Swatches = UHOLNewGameScreen::GetNewGameColorSwatches();
		for (int SlotIndex = 0; SlotIndex < SlotCount; SlotIndex++)
		{
			UHorizontalBox Row = AddNewGameRow(Column, 0.0);
			// "Joueur n" comes from the base (PlayerN, SlotIndex 0-based).
			LobbySlotNames.Add(AddNewGameCellText(Row, SlotWidgetName("SlotName", SlotIndex), UHOLNewGameScreen::GetNewGamePlayerName(SlotIndex), 2, NameWidth));
			UHOLDropdown StateList = nullptr;
			if (SlotIndex == 0)
				LobbyYouText = AddNewGameCellText(Row, n"SlotYou0", UHOLNewGameScreen::GetNewGameText(n"You"), 2, StateWidth);
			else
				StateList = AddNewGameDropdown(Row, SlotWidgetName("SlotState", SlotIndex), StateNames, 0, StateWidth);
			SlotStateLists.Add(StateList);
			AddNewGameDropdown(Row, SlotWidgetName("SlotLevel", SlotIndex), LevelNames, int(EHOLAIDifficulty::Normal), LevelWidth);
			SlotFactionLists.Add(AddNewGameDropdown(Row, SlotWidgetName("SlotFaction", SlotIndex), FactionNames, 0, FactionWidth));
			AddNewGameSwatch(Row, SlotWidgetName("SlotSwatch", SlotIndex));
			UHOLDropdown ColorList = AddNewGameDropdown(Row, SlotWidgetName("SlotColor", SlotIndex), ColorNames, 0, ColorWidth);
			if (ColorList != nullptr)
				ColorList.SetOptionSwatches(Swatches);
			SlotColorLists.Add(ColorList);
			SlotTeamLists.Add(AddNewGameDropdown(Row, SlotWidgetName("SlotTeam", SlotIndex), TeamNames, 0, TeamWidth));
		}

		// The mode and its options (S7, D699: sacred sites, off by default).
		UHorizontalBox Options = AddNewGameRow(Column, 16.0);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Mode"), 2, NameWidth);
		ModeList = AddNewGameDropdown(Options, n"ModeDropdown", UHOLNewGameScreen::GetNewGameOptions(n"Mode"), 0, OptionWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Nomad"), 2, OptionLabelWidth);
		NomadList = AddNewGameDropdown(Options, n"NomadDropdown", OffOn, 0, ToggleWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Regicide"), 2, OptionLabelWidth);
		RegicideList = AddNewGameDropdown(Options, n"RegicideDropdown", OffOn, 0, ToggleWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Catastrophe"), 2, OptionLabelWidth);
		CatastropheList = AddNewGameDropdown(Options, n"CatastropheDropdown", OffOn, 0, ToggleWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"SacredSites"), 2, OptionLabelWidth);
		SacredSitesList = AddNewGameDropdown(Options, n"SacredSitesDropdown", OffOn, 0, ToggleWidth);

		// The texts under the grid: the lobby's spectators (hidden), the note, the error (hidden).
		LobbySpectators = AddNewGameText(Column, n"SpectatorsText", FText(), 1);
		if (LobbySpectators != nullptr)
			LobbySpectators.SetVisibility(ESlateVisibility::Collapsed);
		AddNewGameText(Column, n"Note", UHOLNewGameScreen::GetNewGameText(n"GreyedTeamsOpen"), 1);
		UCommonTextBlock Error = AddNewGameText(Column, n"ErrorText", FText(), 2);
		// The error's colour (HOLUIStyle::Missing) is set by the base's AddNewGameText for the name "ErrorText" (ARWEN-1118q 2).
		if (Error != nullptr)
			Error.SetVisibility(ESlateVisibility::Collapsed);

		AddNewGameHint(Column);
		UHorizontalBox Buttons = AddNewGameButtonRow(Column);
		AddNewGameButton(Buttons, n"BackButton", UHOLNewGameScreen::GetNewGameText(n"Back"));
		UHOLButton Ready = AddNewGameButton(Buttons, n"ReadyButton", UHOLNewGameScreen::GetNewGameText(n"ReadyAction"));
		if (Ready != nullptr)
			Ready.SetVisibility(ESlateVisibility::Collapsed);
		UHOLButton Spectate = AddNewGameButton(Buttons, n"SpectateButton", UHOLNewGameScreen::GetNewGameText(n"BecomeSpectator"));
		if (Spectate != nullptr)
			Spectate.SetVisibility(ESlateVisibility::Collapsed);
		// The Launch / Back / Ready / Spectate buttons stay wired by the base (Launch(), Close, ToggleReady, ToggleSpectator).
		LobbyLaunchButton = AddNewGameButton(Buttons, n"LaunchButton", UHOLNewGameScreen::GetNewGameText(n"Launch"));
		LobbyReadyButton = Ready;
		LobbySpectateButton = Spectate;

		// NG-4: the gestures. Each list's OnChosen is broadcast just after the value is taken (never by Refresh's SetSelectedIndex).
		Subscribe(MapList, n"OnMapChosen");
		Subscribe(SizeList, n"OnSizeChosen");
		Subscribe(TeamsTogetherList, n"OnTeamsTogetherChosen");
		Subscribe(FogList, n"OnFogChosen");
		Subscribe(ModeList, n"OnModeChosen");
		Subscribe(NomadList, n"OnOptionChosen");
		Subscribe(RegicideList, n"OnOptionChosen");
		Subscribe(CatastropheList, n"OnOptionChosen");
		Subscribe(SacredSitesList, n"OnOptionChosen");
		for (int SlotIndex = 0; SlotIndex < SlotCount; SlotIndex++)
		{
			Subscribe(SlotStateLists[SlotIndex], n"OnSlotStateChosen");
			Subscribe(SlotFactionLists[SlotIndex], n"OnSlotFactionChosen");
			Subscribe(SlotColorLists[SlotIndex], n"OnSlotColorChosen");
			Subscribe(SlotTeamLists[SlotIndex], n"OnSlotTeamChosen");
		}
	}

	// ---- The gestures (HandleMapChosen ... HandleTeamChosen of HOLNewGameScreen.cpp, solo and lobby branches) ----
	// Solo: the configuration is written and the widgets refreshed. Lobby (IsLobby()): the same local write, and the choice is SENT to the manager
	// (SendSettings, SendSlotAIChoice, SendSlotOpen, SendMyChoice; they do nothing without a client); the next read of the lobby puts back the manager's value.

	// A list of on / off (index 1 is "Oui" / "On"); a missing list is off.
	bool ListIsOn(UHOLDropdown List)
	{
		return List != nullptr && List.GetSelectedIndex() == 1;
	}

	// HandleMapChosen: the map; solo: a configuration with no size takes the smallest size that holds its players (places drawn); Refresh; lobby: SendSettings
	// (after Refresh, as the cpp: the lobby never draws a default size).
	UFUNCTION()
	void OnMapChosen(UHOLDropdown Dropdown, int Index)
	{
		const bool bInLobby = IsLobby();
		SetConfigMapIndex(Index);
		if (bInLobby)
		{
			Refresh();
			SendSettings();
			return;
		}
		if (ReadConfig() && HOLNewGameRules::MapChoiceNeedsDefaultSize(false, CfgMapSize))
		{
			const int DefaultSize = HOLNewGameRules::DefaultSize(PlacesBySize(), HOLNewGameRules::NumOccupied(CfgStates));
			ChooseNewGameSize(DefaultSize);
		}
		Refresh();
	}

	// HandleSizeChosen: solo: ChooseSize (false and nothing changed when the players do not fit, the list greys those sizes already); lobby: the size of the
	// table as it is (a number that is no size is not taken), sent (the manager closes the slots past it); Refresh.
	UFUNCTION()
	void OnSizeChosen(UHOLDropdown Dropdown, int Index)
	{
		if (IsLobby())
		{
			const TArray<int> Places = PlacesBySize();
			if (Index >= 1 && Index < Places.Num())
			{
				SetConfigMapSize(Index);
				SendSettings();
			}
			Refresh();
			return;
		}
		ChooseNewGameSize(Index);
		Refresh();
	}

	// HandleTeamsTogetherChosen: the list's 1 is "Oui"; lobby: SendSettings; Refresh.
	UFUNCTION()
	void OnTeamsTogetherChosen(UHOLDropdown Dropdown, int Index)
	{
		if (!ReadConfig())
			return;
		SetConfigGlobals(Index == 1, CfgFogMode, CfgMode);
		if (IsLobby())
			SendSettings();
		Refresh();
	}

	// HandleFogChosen: the list is ordered Normal, Explored, Disabled, so FogMode = 2 - Index (clamped 0..2); lobby: SendSettings; Refresh.
	UFUNCTION()
	void OnFogChosen(UHOLDropdown Dropdown, int Index)
	{
		if (!ReadConfig())
			return;
		int Fog = 2 - Index;
		if (Fog < 0)
			Fog = 0;
		if (Fog > 2)
			Fog = 2;
		SetConfigGlobals(bCfgTeamsTogether, Fog, CfgMode);
		if (IsLobby())
			SendSettings();
		Refresh();
	}

	// HandleModeChosen: 1 is Diplomacy, any other value Standard; lobby: SendSettings; Refresh.
	UFUNCTION()
	void OnModeChosen(UHOLDropdown Dropdown, int Index)
	{
		if (!ReadConfig())
			return;
		const int NewMode = Index == 1 ? 1 : 0;
		SetConfigGlobals(bCfgTeamsTogether, CfgFogMode, NewMode);
		if (IsLobby())
			SendSettings();
		Refresh();
	}

	// HandleOptionChosen: solo: only the sacred sites are taken (Nomad, Regicide, Catastrophe stay greyed and unchanged), no Refresh, as in the cpp; lobby: the
	// four options as the lists show them (the sacred sites first, as the cpp), sent.
	UFUNCTION()
	void OnOptionChosen(UHOLDropdown Dropdown, int Index)
	{
		const bool bSacredSites = ListIsOn(SacredSitesList);
		if (IsLobby())
		{
			SetConfigOptions(ListIsOn(NomadList), ListIsOn(RegicideList), ListIsOn(CatastropheList), bSacredSites);
			SendSettings();
			return;
		}
		if (!ReadConfig())
			return;
		SetConfigOptions(bCfgNomad, bCfgRegicide, bCfgCatastrophe, bSacredSites);
	}

	// HandleStateChosen: lobby (the leader's list: Open, Closed, Not ready, Ready, AI): HOLNewGameRules::LobbyStateAction says what is sent (1 the AI, 2 the slot
	// open, 3 the slot closed, 0 nothing), no Refresh (the next read puts back the manager's value). Solo: a slot past the places of the size stays closed; else the slot
	// takes the state, and a colour another slot holds gives way to the first free one (HOLNewGameRules::ApplyStateChosen); Refresh.
	UFUNCTION()
	void OnSlotStateChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotStateLists, Dropdown);
		if (SlotIndex < 0)
			return;
		if (IsLobby())
		{
			const int Action = HOLNewGameRules::LobbyStateAction(Index, HasClient());
			if (Action == 1)
				SendSlotAIChoice(SlotIndex, -1, -1, "");
			else if (Action == 2)
				SendSlotOpen(SlotIndex, true);
			else if (Action == 3)
				SendSlotOpen(SlotIndex, false);
			return;
		}
		if (!ReadConfig())
			return;
		const int NumPlaces = HOLNewGameRules::NumPlacesOfConfig(PlacesBySize(), CfgMapSize);
		if (HOLNewGameRules::ApplyStateChosen(CfgStates, CfgColors, SlotIndex, Index, NumPlaces))
			SetConfigSlot(SlotIndex, CfgStates[SlotIndex], -1, CfgColors[SlotIndex], -1);
		Refresh();
	}

	// HandleFactionChosen: a faction the list does not offer (greyed, the Templars without the cheat) is never taken; in a lobby only my line and the leader's AIs
	// are changed; else the slot's faction, Refresh, and (lobby) the faction is sent: the leader's AI by SendSlotAIChoice, my line by SendMyChoice.
	UFUNCTION()
	void OnSlotFactionChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotFactionLists, Dropdown);
		if (SlotIndex < 0)
			return;
		if (!HOLNewGameRules::IsFactionChoosable(Index, HOLFactionChoiceRead::IsCheatOn()))
		{
			Refresh();
			return;
		}
		const bool bInLobby = IsLobby();
		if (bInLobby)
		{
			const FHOLNewGameLobbyView View = GetLobbyView();
			if (!HOLNewGameRules::LobbyLineEditable(SlotIndex, View.YourSlot, CanEditLobbyAISlot(SlotIndex)))
			{
				Refresh();
				return;
			}
		}
		SetConfigSlot(SlotIndex, -1, Index, -1, -1);
		Refresh();
		if (!bInLobby)
			return;
		const FString Name = UHOLNewGameScreen::GetNewGameFactionName(Index);
		if (IsLobbyAI(SlotIndex))
			SendSlotAIChoice(SlotIndex, -1, -1, Name);
		else
			SendMyChoice(-1, -1, -1, -1, Name);
	}

	// HandleColorChosen: the colour (a taken one is greyed in the list; "Aleatoire" is 8); Refresh; lobby: sent (an AI's by SendSlotAIChoice, mine by SendMyChoice).
	UFUNCTION()
	void OnSlotColorChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotColorLists, Dropdown);
		if (SlotIndex < 0)
			return;
		SetConfigSlot(SlotIndex, -1, -1, Index, -1);
		Refresh();
		if (!IsLobby())
			return;
		if (IsLobbyAI(SlotIndex))
			SendSlotAIChoice(SlotIndex, Index, -1, "");
		else
			SendMyChoice(Index, -1, -1, -1, "");
	}

	// HandleTeamChosen: the team; no Refresh, as in the cpp; lobby: sent (an AI's by SendSlotAIChoice, mine by SendMyChoice).
	UFUNCTION()
	void OnSlotTeamChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotTeamLists, Dropdown);
		if (SlotIndex < 0)
			return;
		SetConfigSlot(SlotIndex, -1, -1, -1, Index);
		if (!IsLobby())
			return;
		if (IsLobbyAI(SlotIndex))
			SendSlotAIChoice(SlotIndex, -1, Index, "");
		else
			SendMyChoice(-1, Index, -1, -1, "");
	}

	// ---- The lobby display (ShowLobbyWidgets of HOLNewGameScreen.cpp, moved to the script) ----
	// Called by the base's ShowLobby after the lobby is read, the configuration copied and Refresh() done (bScriptOwnsLobbyDisplay). The base keeps the Starting
	// state, JoinMatch, the journal and the Ready / Spectator wiring (ToggleReady / ToggleSpectator): this only shows.

	// One line of the lobby: the name, the state list (or "Vous" for the slot 0), the faction / colour / team lists greyed but for my line and the leader's AIs.
	void ShowLobbyLine(const FHOLNewGameLobbyView& View, int SlotIndex, const TArray<FText>& LobbyStates)
	{
		const bool bRowExists = SlotIndex < View.Slots.Num();
		int RowState = 0;
		bool bRowReady = false;
		if (bRowExists)
		{
			RowState = View.Slots[SlotIndex].State;
			bRowReady = View.Slots[SlotIndex].bReady;
		}
		const bool bAI = bRowExists && RowState == 3;
		if (SlotIndex < LobbySlotNames.Num() && LobbySlotNames[SlotIndex] != nullptr)
			LobbySlotNames[SlotIndex].SetText(GetNewGameLobbyLineName(SlotIndex));
		// Open, closed, not ready, ready, AI (the lobby's state list).
		const int State = HOLNewGameRules::LobbyListIndex(bRowExists, RowState, bRowReady);
		FText AIName;
		if (bAI)
			AIName = GetNewGameLobbyAIName(SlotIndex);
		UHOLDropdown StateList = nullptr;
		if (SlotIndex < SlotStateLists.Num())
			StateList = SlotStateLists[SlotIndex];
		if (StateList != nullptr)
		{
			// The leader opens and closes the slots no player holds, or puts an AI there: Open, Closed and AI (the others are the manager's).
			StateList.SetSelectedIndex(State);
			StateList.SetIsEnabled(CanToggleLobbySlot(SlotIndex));
			StateList.SetOptionEnabled(2, false);
			StateList.SetOptionEnabled(3, false);
			if (bAI)
				StateList.SetShownText(AIName);
		}
		else if (SlotIndex == 0 && LobbyYouText != nullptr)
		{
			LobbyYouText.SetText(bAI ? AIName : LobbyStates[State]);
		}
		// Only my line is mine, and an AI's is the leader's; the faction too.
		const bool bEditable = HOLNewGameRules::LobbyLineEditable(SlotIndex, View.YourSlot, CanEditLobbyAISlot(SlotIndex));
		if (SlotIndex < SlotFactionLists.Num() && SlotFactionLists[SlotIndex] != nullptr)
			SlotFactionLists[SlotIndex].SetIsEnabled(bEditable);
		if (SlotIndex < SlotColorLists.Num() && SlotColorLists[SlotIndex] != nullptr)
			SlotColorLists[SlotIndex].SetIsEnabled(bEditable);
		if (SlotIndex < SlotTeamLists.Num() && SlotTeamLists[SlotIndex] != nullptr)
			SlotTeamLists[SlotIndex].SetIsEnabled(bEditable);
	}

	UFUNCTION(BlueprintOverride)
	void ShowLobby()
	{
		const FHOLNewGameLobbyView View = GetLobbyView();
		const TArray<FText> LobbyStates = UHOLNewGameScreen::GetNewGameOptions(n"LobbyState");
		if (LobbyHeading != nullptr)
			LobbyHeading.SetText(FText::FromString(View.Name));
		for (int SlotIndex = 0; SlotIndex < SlotCount; SlotIndex++)
			ShowLobbyLine(View, SlotIndex, LobbyStates);

		// The map, its size, "Equipes ensemble", the fog, the mode and the options are the leader's (the sacred sites are not in the cpp's list: not added).
		const bool bLeader = IsLobbyYouLeader();
		TArray<UHOLDropdown> Settings;
		Settings.Add(MapList);
		Settings.Add(SizeList);
		Settings.Add(TeamsTogetherList);
		Settings.Add(FogList);
		Settings.Add(ModeList);
		Settings.Add(NomadList);
		Settings.Add(RegicideList);
		Settings.Add(CatastropheList);
		for (int Setting = 0; Setting < Settings.Num(); Setting++)
		{
			if (Settings[Setting] != nullptr)
				Settings[Setting].SetIsEnabled(bLeader);
		}

		// Ready: "Plus pret" when ready; changeable with a slot while the lobby has not started; no Ready for a spectator.
		const bool bLobbyOpen = View.State == 0;
		const bool bSpectator = IsLobbyYouSpectator();
		TArray<bool> ReadyRows;
		for (int Row = 0; Row < View.Slots.Num(); Row++)
			ReadyRows.Add(View.Slots[Row].bReady);
		const bool bReady = HOLNewGameRules::IsYouReady(ReadyRows, View.YourSlot);
		if (LobbyReadyButton != nullptr)
		{
			SetNewGameButtonLabel(n"ReadyButton", UHOLNewGameScreen::GetNewGameText(HOLNewGameRules::ReadyButtonShowsUndo(bReady) ? n"NotReadyAction" : n"ReadyAction"));
			LobbyReadyButton.SetIsEnabled(HOLNewGameRules::ReadyButtonEnabled(View.YourSlot, bLobbyOpen));
			LobbyReadyButton.SetVisibility(HOLNewGameRules::ReadyButtonVisible(bSpectator) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
		// Spectator (SP1, D642): a spectator's button takes a slot back (only if one is open), a player's leaves his.
		if (LobbySpectateButton != nullptr)
		{
			SetNewGameButtonLabel(n"SpectateButton", UHOLNewGameScreen::GetNewGameText(HOLNewGameRules::SpectateButtonShowsPlay(bSpectator) ? n"BecomePlayer" : n"BecomeSpectator"));
			LobbySpectateButton.SetIsEnabled(HOLNewGameRules::SpectateButtonEnabled(bLobbyOpen, bSpectator, LobbyHasOpenSlot()));
		}
		// The spectators ("Spectateurs : A, couronne B"), hidden when there is none.
		if (LobbySpectators != nullptr)
		{
			const TArray<FString> Spectators = View.Spectators;
			LobbySpectators.SetText(UHOLNewGameScreen::GetNewGameSpectatorsText(Spectators, View.Leader));
			LobbySpectators.SetVisibility(Spectators.Num() == 0 ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		}
		// Launch: the leader's only, active when all are ready (the manager still decides).
		if (LobbyLaunchButton != nullptr)
		{
			LobbyLaunchButton.SetVisibility(bLeader ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			LobbyLaunchButton.SetIsEnabled(CanStartLobby() && bLobbyOpen);
		}
	}
}
