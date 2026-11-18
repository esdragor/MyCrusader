// The New game screen's layout in script (lot NG-3, D851, PLAN-ECRANS-1118.md): UHOLNewGameScreen::BuildLayout calls BP_BuildNewGame for a derived class
// (the C++ layout stays the one of the class itself until the switch, NG-6). This file lays out what HOLNewGameScreen.cpp BuildLayout lays out (224-345),
// in the same order, with the same widths (GetNewGameWidth, read from the cpp's constants) and the same widget NAMES (the scenarios, quick_check and the
// C++ Refresh / ShowLobby find the widgets by name): Heading, MapDropdown, FogDropdown, SizeDropdown, TeamsTogetherDropdown, HeadName, HeadSlot, HeadLevel,
// then per slot n = 0..7 SlotName<n>, SlotYou0 (slot 0 only) or SlotState<n> (slots 1..7), SlotLevel<n>, SlotFaction<n>, SlotSwatch<n>, SlotColor<n>,
// SlotTeam<n>; ModeDropdown, NomadDropdown, RegicideDropdown, CatastropheDropdown, SacredSitesDropdown, SpectatorsText, Note (the cpp's name of the note:
// it is "Note", not "NoteText"), ErrorText, BackButton, ReadyButton, SpectateButton, LaunchButton.
// Construction only: no gesture, no Refresh, no lobby here (the next lots of this same file). No event is subscribed.
class UHOLNewGameScreenScript : UHOLNewGameScreen
{
	// The cpp's FHOLMatchConfig::MaxSlots (one line per slot).
	const int SlotCount = 8;

	// The name of a per-slot widget: the cpp's SlotWidgetName (Prefix + index, "SlotState1").
	FName SlotWidgetName(FString Prefix, int SlotIndex)
	{
		return FName(Prefix + SlotIndex);
	}

	UFUNCTION(BlueprintOverride)
	void BP_BuildNewGame()
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

		// Text styles of AddNewGameText / AddNewGameCellText: 0 heading, 1 note, 2 body.
		UVerticalBox Column = MakeNewGameColumn();
		AddNewGameText(Column, n"Heading", UHOLNewGameScreen::GetNewGameText(n"Heading"), 0);

		// The map line: the map, then the fog (F8, over the "Equipes ensemble" of the line below).
		UHorizontalBox MapLine = AddNewGameRow(Column, 0.0);
		AddNewGameCellText(MapLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Map"), 2, NameWidth);
		AddNewGameDropdown(MapLine, n"MapDropdown", GetMapNames(), 0, MapListWidth);
		AddNewGameCellText(MapLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Fog"), 2, TeamsTogetherLabelWidth);
		AddNewGameDropdown(MapLine, n"FogDropdown", UHOLNewGameScreen::GetNewGameOptions(n"Fog"), 0, FogListWidth);

		// TC3: the size under the map, then "Equipes ensemble" (the places are drawn at the launch).
		UHorizontalBox SizeLine = AddNewGameRow(Column, 0.0);
		AddNewGameCellText(SizeLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Size"), 2, NameWidth);
		AddNewGameDropdown(SizeLine, n"SizeDropdown", UHOLNewGameScreen::GetNewGameOptions(n"Size"), 0, MapListWidth);
		AddNewGameCellText(SizeLine, NAME_None, UHOLNewGameScreen::GetNewGameText(n"TeamsTogether"), 2, TeamsTogetherLabelWidth);
		AddNewGameDropdown(SizeLine, n"TeamsTogetherDropdown", OffOn, 1, TeamWidth);

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
			if (SlotIndex == 0)
				AddNewGameCellText(Row, n"SlotYou0", UHOLNewGameScreen::GetNewGameText(n"You"), 2, StateWidth);
			else
				AddNewGameDropdown(Row, SlotWidgetName("SlotState", SlotIndex), StateNames, 0, StateWidth);
			AddNewGameDropdown(Row, SlotWidgetName("SlotLevel", SlotIndex), LevelNames, int(EHOLAIDifficulty::Normal), LevelWidth);
			AddNewGameDropdown(Row, SlotWidgetName("SlotFaction", SlotIndex), FactionNames, 0, FactionWidth);
			AddNewGameSwatch(Row, SlotWidgetName("SlotSwatch", SlotIndex));
			UHOLDropdown ColorList = AddNewGameDropdown(Row, SlotWidgetName("SlotColor", SlotIndex), ColorNames, 0, ColorWidth);
			if (ColorList != nullptr)
				ColorList.SetOptionSwatches(Swatches);
			AddNewGameDropdown(Row, SlotWidgetName("SlotTeam", SlotIndex), TeamNames, 0, TeamWidth);
		}

		// The mode and its options (S7, D699: sacred sites, off by default).
		UHorizontalBox Options = AddNewGameRow(Column, 16.0);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Mode"), 2, NameWidth);
		AddNewGameDropdown(Options, n"ModeDropdown", UHOLNewGameScreen::GetNewGameOptions(n"Mode"), 0, OptionWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Nomad"), 2, OptionLabelWidth);
		AddNewGameDropdown(Options, n"NomadDropdown", OffOn, 0, ToggleWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Regicide"), 2, OptionLabelWidth);
		AddNewGameDropdown(Options, n"RegicideDropdown", OffOn, 0, ToggleWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"Catastrophe"), 2, OptionLabelWidth);
		AddNewGameDropdown(Options, n"CatastropheDropdown", OffOn, 0, ToggleWidth);
		AddNewGameCellText(Options, NAME_None, UHOLNewGameScreen::GetNewGameText(n"SacredSites"), 2, OptionLabelWidth);
		AddNewGameDropdown(Options, n"SacredSitesDropdown", OffOn, 0, ToggleWidth);

		// The texts under the grid: the lobby's spectators (hidden), the note, the error (hidden).
		UCommonTextBlock Spectators = AddNewGameText(Column, n"SpectatorsText", FText(), 1);
		if (Spectators != nullptr)
			Spectators.SetVisibility(ESlateVisibility::Collapsed);
		AddNewGameText(Column, n"Note", UHOLNewGameScreen::GetNewGameText(n"GreyedTeamsOpen"), 1);
		UCommonTextBlock Error = AddNewGameText(Column, n"ErrorText", FText(), 2);
		// MANQUE: the cpp colours the error HOLUIStyle::Missing (SetColorAndOpacity); no exposed colour for it (see the report).
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
		AddNewGameButton(Buttons, n"LaunchButton", UHOLNewGameScreen::GetNewGameText(n"Launch"));
	}
}
