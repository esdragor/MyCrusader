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
// the widgets). Launch, ShowError, Back, focus stay the base's. The LOBBY (IsLobby()) is the next lot: its gestures do nothing here.
class UHOLNewGameScreenScript : UHOLNewGameScreen
{
	default bScriptOwnsGestures = true;

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

		// Text styles of AddNewGameText / AddNewGameCellText: 0 heading, 1 note, 2 body.
		UVerticalBox Column = MakeNewGameColumn();
		AddNewGameText(Column, n"Heading", UHOLNewGameScreen::GetNewGameText(n"Heading"), 0);

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
			AddNewGameCellText(Row, SlotWidgetName("SlotName", SlotIndex), UHOLNewGameScreen::GetNewGamePlayerName(SlotIndex), 2, NameWidth);
			UHOLDropdown StateList = nullptr;
			if (SlotIndex == 0)
				AddNewGameCellText(Row, n"SlotYou0", UHOLNewGameScreen::GetNewGameText(n"You"), 2, StateWidth);
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
		UCommonTextBlock Spectators = AddNewGameText(Column, n"SpectatorsText", FText(), 1);
		if (Spectators != nullptr)
			Spectators.SetVisibility(ESlateVisibility::Collapsed);
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
		AddNewGameButton(Buttons, n"LaunchButton", UHOLNewGameScreen::GetNewGameText(n"Launch"));

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

	// ---- The gestures, solo (HandleMapChosen ... HandleTeamChosen of HOLNewGameScreen.cpp 971-1049, 1097, 1220, 1289, 1327) ----
	// In a lobby (IsLobby()) none of them does anything yet: the lobby is the next lot (SendSettings, SendSlotAIChoice, SendMyChoice, SendSlotOpen).

	// HandleMapChosen: the map; a configuration with no size takes the smallest size that holds its players (places drawn); Refresh.
	UFUNCTION()
	void OnMapChosen(UHOLDropdown Dropdown, int Index)
	{
		if (IsLobby())
			return;
		SetConfigMapIndex(Index);
		if (ReadConfig() && HOLNewGameRules::MapChoiceNeedsDefaultSize(false, CfgMapSize))
		{
			const int DefaultSize = HOLNewGameRules::DefaultSize(PlacesBySize(), HOLNewGameRules::NumOccupied(CfgStates));
			ChooseNewGameSize(DefaultSize);
		}
		Refresh();
	}

	// HandleSizeChosen (solo branch): ChooseSize (false and nothing changed when the players do not fit, the list greys those sizes already); Refresh.
	UFUNCTION()
	void OnSizeChosen(UHOLDropdown Dropdown, int Index)
	{
		if (IsLobby())
			return;
		ChooseNewGameSize(Index);
		Refresh();
	}

	// HandleTeamsTogetherChosen: the list's 1 is "Oui"; Refresh.
	UFUNCTION()
	void OnTeamsTogetherChosen(UHOLDropdown Dropdown, int Index)
	{
		if (IsLobby() || !ReadConfig())
			return;
		SetConfigGlobals(Index == 1, CfgFogMode, CfgMode);
		Refresh();
	}

	// HandleFogChosen: the list is ordered Normal, Explored, Disabled, so FogMode = 2 - Index (clamped 0..2); Refresh.
	UFUNCTION()
	void OnFogChosen(UHOLDropdown Dropdown, int Index)
	{
		if (IsLobby() || !ReadConfig())
			return;
		int Fog = 2 - Index;
		if (Fog < 0)
			Fog = 0;
		if (Fog > 2)
			Fog = 2;
		SetConfigGlobals(bCfgTeamsTogether, Fog, CfgMode);
		Refresh();
	}

	// HandleModeChosen: 1 is Diplomacy, any other value Standard; Refresh.
	UFUNCTION()
	void OnModeChosen(UHOLDropdown Dropdown, int Index)
	{
		if (IsLobby() || !ReadConfig())
			return;
		const int NewMode = Index == 1 ? 1 : 0;
		SetConfigGlobals(bCfgTeamsTogether, CfgFogMode, NewMode);
		Refresh();
	}

	// HandleOptionChosen (solo branch): only the sacred sites are taken (Nomad, Regicide, Catastrophe stay greyed and unchanged); no Refresh, as in the cpp.
	UFUNCTION()
	void OnOptionChosen(UHOLDropdown Dropdown, int Index)
	{
		if (IsLobby() || !ReadConfig())
			return;
		const bool bSacredSites = SacredSitesList != nullptr && SacredSitesList.GetSelectedIndex() == 1;
		SetConfigOptions(bCfgNomad, bCfgRegicide, bCfgCatastrophe, bSacredSites);
	}

	// HandleStateChosen (solo branch): a slot past the places of the size stays closed; else the slot takes the state, and a colour another slot holds
	// gives way to the first free one (HOLNewGameRules::ApplyStateChosen); Refresh.
	UFUNCTION()
	void OnSlotStateChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotStateLists, Dropdown);
		if (IsLobby() || SlotIndex < 0 || !ReadConfig())
			return;
		const int NumPlaces = HOLNewGameRules::NumPlacesOfConfig(PlacesBySize(), CfgMapSize);
		if (HOLNewGameRules::ApplyStateChosen(CfgStates, CfgColors, SlotIndex, Index, NumPlaces))
			SetConfigSlot(SlotIndex, CfgStates[SlotIndex], -1, CfgColors[SlotIndex], -1);
		Refresh();
	}

	// HandleFactionChosen (solo): a faction the list does not offer (greyed, the Templars without the cheat) is never taken; else the slot's faction; Refresh.
	UFUNCTION()
	void OnSlotFactionChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotFactionLists, Dropdown);
		if (IsLobby() || SlotIndex < 0)
			return;
		if (HOLNewGameRules::IsFactionChoosable(Index, HOLFactionChoiceRead::IsCheatOn()))
			SetConfigSlot(SlotIndex, -1, Index, -1, -1);
		Refresh();
	}

	// HandleColorChosen (solo): the colour (a taken one is greyed in the list; "Aleatoire" is 8); Refresh.
	UFUNCTION()
	void OnSlotColorChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotColorLists, Dropdown);
		if (IsLobby() || SlotIndex < 0)
			return;
		SetConfigSlot(SlotIndex, -1, -1, Index, -1);
		Refresh();
	}

	// HandleTeamChosen (solo): the team; nothing else (no Refresh, as in the cpp).
	UFUNCTION()
	void OnSlotTeamChosen(UHOLDropdown Dropdown, int Index)
	{
		const int SlotIndex = SlotOfList(SlotTeamLists, Dropdown);
		if (IsLobby() || SlotIndex < 0)
			return;
		SetConfigSlot(SlotIndex, -1, -1, -1, Index);
	}
}
