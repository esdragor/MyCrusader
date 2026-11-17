// The Multiplayer screen's widget in script (MIG-CPP bascule Multiplayer, D840/D839, as HOLEndBannerScreen.as): UHOLMultiplayerScreen::Open (the
// title's Multiplayer button, hol.UI.Open Multiplayer, the menu after a match) pushes the class FindScriptClass finds, the one of this file. Its
// column, name line, list, status and buttons are built here (BuildMultiplayer, in place of BuildLayout); each lobby's line is laid out here
// (AddLobbyRow) from the view the C++ base makes (FHOLLobbyRowView: ChooseLobbyAction, CanSpectate and the texts). The widget names are the C++
// screen's (Title, NameBox, LobbyList, StatusText, RefreshButton, CreateButton, BackButton, JoinButton<i>, RejoinButton<i>, WatchButton<i>,
// SpectateButton<i>): the scenarios and hol.UI.Click use them. The manager, the name rule and what a lobby button does stay in the C++ base.
// The list's columns (the C++ screen's widths): they line up under a fixed width. MapWidth: TC3, "Plaine - Immense", the longest in French; SpectateWidth: SP1.
const float MultiplayerNameWidth = 340.0f;
const float MultiplayerMapWidth = 240.0f;
const float MultiplayerPlayersWidth = 90.0f;
const float MultiplayerStateWidth = 220.0f;
const float MultiplayerActionWidth = 200.0f;
const float MultiplayerSpectateWidth = 180.0f;

class UHOLMultiplayerScreenScript : UHOLMultiplayerScreen
{
	UFUNCTION(BlueprintOverride)
	void BuildMultiplayer()
	{
		UVerticalBox Column = MakeMultiplayerColumn();
		Title = AddMultiplayerText(Column, n"Title", UHOLMultiplayerScreen::ScreenText(n"Title"), true);
		AddNameRow(Column);
		AddLobbyListBox(Column, MultiplayerNameWidth + MultiplayerMapWidth + MultiplayerPlayersWidth + MultiplayerStateWidth + MultiplayerActionWidth + MultiplayerSpectateWidth + 40.0f, 240.0f);
		StatusText = AddMultiplayerText(Column, n"StatusText", FText(), false);
		UHorizontalBox Buttons = AddMultiplayerRow(Column);
		RefreshButton = AddMultiplayerButton(Buttons, n"RefreshButton", UHOLMultiplayerScreen::ScreenText(n"Refresh"));
		CreateButton = AddMultiplayerButton(Buttons, n"CreateButton", UHOLMultiplayerScreen::ScreenText(n"Create"));
		BackButton = AddMultiplayerButton(Buttons, n"BackButton", UHOLMultiplayerScreen::ScreenText(n"Back"));
		if (RefreshButton != nullptr)
			RefreshButton.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		if (CreateButton != nullptr)
			CreateButton.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
		if (BackButton != nullptr)
			BackButton.OnButtonBaseClicked.AddUFunction(this, n"HandleButtonClicked");
	}

	// One lobby's line: name, map, players, state, then the one button its state allows and "Observer" on a lobby gathering.
	UFUNCTION(BlueprintOverride)
	void AddLobbyRow(FHOLLobbyRowView Row)
	{
		UHorizontalBox Line = AddLobbyRowBox();
		if (Line == nullptr)
			return;
		AddLobbyCell(Line, FText::FromString(Row.Name), MultiplayerNameWidth);
		AddLobbyCell(Line, Row.Map, MultiplayerMapWidth);
		AddLobbyCell(Line, Row.PlayersText, MultiplayerPlayersWidth);
		AddLobbyCell(Line, Row.State, MultiplayerStateWidth);
		if (Row.Action != EHOLLobbyRowAction::None)
			AddLobbyButton(Line, Row.ActionName, Row.ActionLabel, Row.Index, Row.Action, false, MultiplayerActionWidth);
		if (Row.bSpectate)
			AddLobbyButton(Line, Row.SpectateName, Row.SpectateLabel, Row.Index, EHOLLobbyRowAction::None, true, MultiplayerSpectateWidth);
	}

	// Refresh, Create and Back: the lobby buttons are bound by the base (AddLobbyButton).
	UFUNCTION()
	void HandleButtonClicked(UCommonButtonBase Button)
	{
		if (Button == nullptr)
			return;
		if (Button == RefreshButton)
			RefreshLobbies();
		else if (Button == CreateButton)
			CreateLobby();
		else if (Button == BackButton)
			CloseScreen();
	}
}
