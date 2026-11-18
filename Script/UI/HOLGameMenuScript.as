// The in-game menu as a view (D356, D357, D513, V1-Q1, M6-Q7 revised, M6-Q10; migration D830/D835, pilot 2 MIG-P2b): which of its buttons
// show, their labels, what a click on each does, from what the machine knows of the match (pure, no world), as UHOLGameMenu's
// RefreshSurrender, RefreshPause and SetOpen do in C++ (Source/HeroesOfLibertyUI/Screens/HOLGameMenu.cpp). The widget applying it waits for
// the bindings listed in AngelscriptInProgress.md (UHOLGameMenu derives from UHOLScreen, a UCommonActivatableWidget, whose helpers, the
// root layout, UHOLButton and the controller's calls are no UFUNCTION). Describe is the stable line of the tests.

// What a click on one of the menu's buttons asks for (the C++ handlers' names).
enum EHOLGameMenuAction
{
	None,
	// « Reprendre »: the menu closes.
	Resume,
	// « Pause »: the menu closes, the server pauses (AHOLPlayerController::RequestPause).
	Pause,
	// « Abandonner »: the confirmation over the menu (UHOLGameMenu::AskToSurrender).
	Surrender,
	// « Quitter vers le menu »: the MainMenu map (UHOLEndBannerScreen::LeaveMatch).
	QuitToMenu,
	// « Quitter vers la liste »: the lobby forgotten, the Multiplayer screen's list after the match.
	QuitToList,
	// « Retour au salon »: the lobby after the match.
	ReturnToLobby,
	// « Résultat »: the end banner again (AHOLGameHUD::ReopenEndBanner).
	ShowResult,
}

struct FHOLGameMenuView
{
	// False without a game or player state: « Reprendre » alone.
	bool bKnown = false;
	bool bPause = false;
	bool bPauseEnabled = false;
	bool bPauseFree = false;
	int PausesLeft = 0;
	bool bSurrender = false;
	bool bQuit = false;
	bool bQuitToList = false;
	bool bReturn = false;
	bool bResult = false;
	// "menu resume[ pause[ <left>][ greyed]][ surrender][ return][ quit list|menu][ result]".
	FString Describe = "menu resume";
}

namespace HOLGameMenuScript
{
	// MySlot -1: a spectator. Banner: the end banner due to this player (EHOLEndBanner's value). bCanPauseNow: UHOLPauseLibrary::CanPauseNow.
	FHOLGameMenuView Make(bool bKnown, int MySlot, bool bDefeated, bool bMatchOver, bool bPaused, bool bPauseFree, int PausesLeft,
		bool bCanPauseNow, EHOLGameMenuBanner Banner, EHOLGameMenuOrigin Origin)
	{
		FHOLGameMenuView View;
		View.bKnown = bKnown;
		// The quit's way follows the match's origin, the same all the match long (SetOpen).
		View.bQuitToList = HOLGameMenuRules::QuitsToList(Origin);
		if (bKnown)
		{
			// D513: a player still in the match while it runs; greyed with none left (RefreshPause).
			View.bPause = MySlot >= 0 && !bDefeated && !bMatchOver && !bPaused;
			View.bPauseEnabled = View.bPause && bCanPauseNow;
			View.bPauseFree = bPauseFree;
			View.PausesLeft = Math::Max(0, PausesLeft);
			// D357: « Abandonner » or the ways out; once over, the end banner's (RefreshSurrender).
			View.bSurrender = HOLGameMenuRules::CanSurrender(bDefeated, bMatchOver);
			View.bQuit = HOLGameMenuRules::CanQuitToMenu(bDefeated, bMatchOver);
			View.bReturn = bMatchOver && HOLGameMenuRules::OffersReturnToLobby(Banner, Origin);
			View.bResult = bMatchOver;
		}
		FString Line = "menu resume";
		if (View.bPause)
			Line += (View.bPauseFree ? " pause" : f" pause {View.PausesLeft}") + (View.bPauseEnabled ? "" : " greyed");
		if (View.bSurrender)
			Line += " surrender";
		if (View.bReturn)
			Line += " return";
		if (View.bQuit)
			Line += View.bQuitToList ? " quit list" : " quit menu";
		if (View.bResult)
			Line += " result";
		View.Describe = Line;
		return View;
	}

	// What a click on the widget named Name does; None: not the menu's, hidden or greyed.
	EHOLGameMenuAction Click(FHOLGameMenuView View, FName Name)
	{
		if (Name == n"ResumeButton")
			return EHOLGameMenuAction::Resume;
		if (Name == n"PauseButton" && View.bPauseEnabled)
			return EHOLGameMenuAction::Pause;
		if (Name == n"SurrenderButton" && View.bSurrender)
			return EHOLGameMenuAction::Surrender;
		if (Name == n"QuitButton" && View.bQuit)
			return View.bQuitToList ? EHOLGameMenuAction::QuitToList : EHOLGameMenuAction::QuitToMenu;
		if (Name == n"ReturnButton" && View.bReturn)
			return EHOLGameMenuAction::ReturnToLobby;
		if (Name == n"ResultButton" && View.bResult)
			return EHOLGameMenuAction::ShowResult;
		return EHOLGameMenuAction::None;
	}

	// The label of the widget named Name, the C++ menu's keys (already translated); "" for none.
	FString Label(FHOLGameMenuView View, FName Name)
	{
		if (Name == n"Title")
			return NSLOCTEXT("HOLGameMenu", "Title", "Menu").ToString();
		if (Name == n"ResumeButton")
			return NSLOCTEXT("HOLGameMenu", "Resume", "Reprendre").ToString();
		if (Name == n"PauseButton")
			return View.bPauseFree ? NSLOCTEXT("HOLGameMenu", "Pause", "Pause").ToString()
				: NSLOCTEXT("HOLGameMenu", "PauseLeft", "Pause ({0})").ToString().Replace("{0}", f"{View.PausesLeft}");
		if (Name == n"SurrenderButton")
			return NSLOCTEXT("HOLGameMenu", "Surrender", "Abandonner").ToString();
		if (Name == n"ReturnButton")
			return NSLOCTEXT("HOLEndBanner", "ReturnToLobby", "Retour au salon").ToString();
		if (Name == n"QuitButton")
			return View.bQuitToList ? NSLOCTEXT("HOLEndBanner", "QuitToList", "Quitter vers la liste").ToString()
				: NSLOCTEXT("HOLEndBanner", "QuitToMenu", "Quitter vers le menu").ToString();
		if (Name == n"ResultButton")
			return NSLOCTEXT("HOLGameMenu", "Result", "Résultat").ToString();
		return "";
	}
}
