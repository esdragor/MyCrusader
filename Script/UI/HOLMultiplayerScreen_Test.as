// The Multiplayer screen in script (MIG-CPP bascule Multiplayer, D840/D839): UHOLMultiplayerScreen::Open (title, hol.UI.Open Multiplayer, the menu
// after a match) pushes the class FindScriptClass finds, the one of Script/UI/HOLMultiplayerScreen.as. The name rule and the row view stay in C++
// (the values below are CALCULATED by the base: IsValidPlayerName, ChooseLobbyAction and CanSpectate through MakeLobbyRowView).

// HeroesOfLiberty.UI.MultiplayerScreenScriptClass: the screen the menu opens is the script's, not the abstract base.
void Test_MultiplayerScreenScriptClass(FUnitTest& T)
{
	const TSubclassOf<UHOLMultiplayerScreen> Found = UHOLMultiplayerScreen::FindScriptClass();
	T.AssertTrue(Found.IsValid(), "the multiplayer screen opened by the menu has a script class");
	if (Found.IsValid())
		T.AssertEquals(Found.Get().GetName().ToString(), "HOLMultiplayerScreenScript", "the multiplayer screen is HOLMultiplayerScreen.as's");
}

// HeroesOfLiberty.UI.MultiplayerPlayerName: 1 to 16 letters, digits, spaces, - or _ once trimmed (Manager/PROTOCOL.md, 8).
void Test_MultiplayerPlayerName(FUnitTest& T)
{
	T.AssertTrue(UHOLMultiplayerScreen::IsValidPlayerName("Chef"), "a plain name");
	T.AssertTrue(UHOLMultiplayerScreen::IsValidPlayerName("  Le chef-2_b  "), "spaces, - and _ are kept, the ends trimmed");
	T.AssertTrue(UHOLMultiplayerScreen::IsValidPlayerName("1234567890123456"), "16 characters");
	T.AssertFalse(UHOLMultiplayerScreen::IsValidPlayerName("12345678901234567"), "17 characters");
	T.AssertFalse(UHOLMultiplayerScreen::IsValidPlayerName("   "), "only spaces");
	T.AssertFalse(UHOLMultiplayerScreen::IsValidPlayerName(""), "empty");
	T.AssertFalse(UHOLMultiplayerScreen::IsValidPlayerName("Chef!"), "a punctuation mark");
}

FHOLLobbyRowView MultiplayerScreen_Row(int Index, int State, int Minutes, bool bCanJoin, bool bCanWatch, const TArray<FString>& Away, bool bCanSpectate, const FString& MyName)
{
	return UHOLMultiplayerScreen::MakeLobbyRowView(Index, "Salon " + Index, "Plain", "Large", 1, 4, State, Minutes, bCanJoin, bCanWatch, Away, bCanSpectate, MyName);
}

// HeroesOfLiberty.UI.MultiplayerLobbyRows: Join on a lobby that gathers, Watch on a match under way, Rejoin on the one I left; "Observer" only
// on a lobby that gathers; the buttons are named as the scenarios click them.
void Test_MultiplayerLobbyRows(FUnitTest& T)
{
	TArray<FString> None;
	TArray<FString> Away;
	Away.Add("Chef");

	FHOLLobbyRowView Gathering = MultiplayerScreen_Row(0, 0, -1, true, false, None, true, "Chef");
	T.AssertEquals(int(EHOLLobbyRowAction::Join), int(Gathering.Action), "a lobby that gathers offers Join");
	T.AssertEquals("JoinButton0", Gathering.ActionName.ToString(), "the Join button's name ends with the lobby's index");
	T.AssertTrue(Gathering.bSpectate, "a lobby that gathers offers Observer");
	T.AssertEquals("SpectateButton0", Gathering.SpectateName.ToString(), "the Observer button's name");
	T.AssertEquals("1/4", Gathering.PlayersText.ToString(), "players over places");

	FHOLLobbyRowView UnderWay = MultiplayerScreen_Row(1, 2, 12, false, true, None, false, "Chef");
	T.AssertEquals(int(EHOLLobbyRowAction::Watch), int(UnderWay.Action), "a match under way offers Watch");
	T.AssertEquals("WatchButton1", UnderWay.ActionName.ToString(), "the Watch button's name");
	T.AssertFalse(UnderWay.bSpectate, "a match under way offers no Observer");

	FHOLLobbyRowView Left = MultiplayerScreen_Row(2, 2, 12, false, true, Away, false, "chef");
	T.AssertEquals(int(EHOLLobbyRowAction::Rejoin), int(Left.Action), "the match I left offers Rejoin (name compared case ignored)");
	T.AssertEquals("RejoinButton2", Left.ActionName.ToString(), "the Rejoin button's name");

	FHOLLobbyRowView Starting = MultiplayerScreen_Row(3, 1, -1, false, false, None, false, "Chef");
	T.AssertEquals(int(EHOLLobbyRowAction::None), int(Starting.Action), "a lobby that starts offers nothing");
	T.AssertFalse(Starting.bSpectate, "a lobby that starts offers no Observer");
}

// HeroesOfLiberty.UI.MultiplayerVeil: the Multiplayer screen lays a dark veil (HOLUIStyle::Veil, D843 Q2 B) behind its column so the red line reads over the
// scene; the base gives its colour (VeilColor) to the script, which asks for the veil in MakeMultiplayerColumn(true). RED until the C++ lot is built.
void Test_MultiplayerVeil(FUnitTest& T)
{
	const FLinearColor Veil = UHOLMultiplayerScreen::VeilColor();
	T.AssertTrue(Veil.A >= 0.5f, "the veil is dark enough to read the status line over a scene (alpha 0.6)");
	T.AssertTrue(Veil.R <= 0.05f && Veil.G <= 0.05f && Veil.B <= 0.05f, "the veil is black, not tinted");
}
