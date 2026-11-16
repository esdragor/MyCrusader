// The end banner in script (D357, V1-Q2, M6-Q7 revised, M6-Q10; migration D830/D835, pilot 2 MIG-P2d): which banner is due and when it
// opens (TEMPORARY TWINS of HOLGameMenuRules::GetEndBanner and ShouldOpenEndBanner, Source/HeroesOfLiberty/Player/HOLGameMenuRules.cpp,
// held to the vectors of Victory.EndBanner.Which and .Closed), and the banner as a view (title, buttons, labels, clicks), as UHOLEndBanner's
// SetResult and RefreshWayBack do in C++ (Source/HeroesOfLibertyUI/Screens/HOLEndBanner.cpp). The widget applying it waits for the same
// bindings as the in-game menu's (AngelscriptInProgress.md, pilot 2). Describe is the stable line of the tests.

// What a click on one of the banner's buttons asks for.
enum EHOLEndBannerAction
{
	None,
	// « Continuer » / « Continuer à regarder », or Back: the banner closes, the match watched.
	Continue,
	QuitToMenu,
	QuitToList,
	ReturnToLobby,
	// « Statistiques »: the statistics screen over the banner (C4).
	Stats,
}

struct FHOLEndBannerView
{
	EHOLGameMenuBanner Result = EHOLGameMenuBanner::None;
	bool bVisible = false;
	bool bReturn = false;
	bool bQuitToList = false;
	// "hidden"; or "<victory|defeat|over> continue[ return] quit list|menu stats".
	FString Describe = "hidden";
}

namespace HOLEndBannerScript
{
	// The banner of a player (D357): the match over, Victory when its slot won, Defeat otherwise (no winner too); the match going on,
	// Defeat once its slot is defeated, else None. Masks: bit Slot. Without a slot (a spectator, M6-Q10): MatchOver once over, else None.
	EHOLGameMenuBanner GetEndBanner(int MySlot, int DefeatedMask, bool bMatchOver, int WinnerMask)
	{
		if (MySlot < 0 || MySlot >= 8)
			return bMatchOver ? EHOLGameMenuBanner::MatchOver : EHOLGameMenuBanner::None;
		const int Me = 1 << MySlot;
		// D357: a camp wins with its fallen members; no winner is everybody's defeat.
		if (bMatchOver)
			return (WinnerMask & Me) != 0 ? EHOLGameMenuBanner::Victory : EHOLGameMenuBanner::Defeat;
		return (DefeatedMask & Me) != 0 ? EHOLGameMenuBanner::Defeat : EHOLGameMenuBanner::None;
	}

	// Whether to open the banner (V1-Q2): one is due, not already shown, and not the one the player closed.
	bool ShouldOpenEndBanner(EHOLGameMenuBanner Due, EHOLGameMenuBanner Shown, EHOLGameMenuBanner Closed)
	{
		return Due != EHOLGameMenuBanner::None && Due != Shown && Due != Closed;
	}

	FHOLEndBannerView Make(EHOLGameMenuBanner Result, EHOLGameMenuOrigin Origin)
	{
		FHOLEndBannerView View;
		View.Result = Result;
		if (Result == EHOLGameMenuBanner::None)
			return View;
		View.bVisible = true;
		// RefreshWayBack: the lobby for its players, the quit's way by the match's origin.
		View.bReturn = HOLGameMenuRules::OffersReturnToLobby(Result, Origin);
		View.bQuitToList = HOLGameMenuRules::QuitsToList(Origin);
		const FString Which = Result == EHOLGameMenuBanner::Victory ? "victory" : (Result == EHOLGameMenuBanner::MatchOver ? "over" : "defeat");
		View.Describe = Which + " continue" + (View.bReturn ? " return" : "") + (View.bQuitToList ? " quit list" : " quit menu") + " stats";
		return View;
	}

	// What a click on the widget named Name does; None: not the banner's or hidden.
	EHOLEndBannerAction Click(FHOLEndBannerView View, FName Name)
	{
		if (!View.bVisible)
			return EHOLEndBannerAction::None;
		if (Name == n"ContinueButton")
			return EHOLEndBannerAction::Continue;
		if (Name == n"QuitButton")
			return View.bQuitToList ? EHOLEndBannerAction::QuitToList : EHOLEndBannerAction::QuitToMenu;
		if (Name == n"ReturnButton" && View.bReturn)
			return EHOLEndBannerAction::ReturnToLobby;
		if (Name == n"StatsButton")
			return EHOLEndBannerAction::Stats;
		return EHOLEndBannerAction::None;
	}

	// The label of the widget named Name, the C++ banner's keys (already translated); "" for none.
	FString Label(FHOLEndBannerView View, FName Name)
	{
		const bool bVictory = View.Result == EHOLGameMenuBanner::Victory;
		if (Name == n"Title")
			return bVictory ? NSLOCTEXT("HOLEndBanner", "Victory", "Victoire").ToString()
				: (View.Result == EHOLGameMenuBanner::MatchOver ? NSLOCTEXT("HOLEndBanner", "MatchOver", "Partie terminée").ToString()
				: NSLOCTEXT("HOLEndBanner", "Defeat", "Défaite").ToString());
		if (Name == n"ContinueButton")
			return bVictory ? NSLOCTEXT("HOLEndBanner", "Continue", "Continuer").ToString()
				: NSLOCTEXT("HOLEndBanner", "ContinueWatching", "Continuer à regarder").ToString();
		if (Name == n"QuitButton")
			return View.bQuitToList ? NSLOCTEXT("HOLEndBanner", "QuitToList", "Quitter vers la liste").ToString()
				: NSLOCTEXT("HOLEndBanner", "QuitToMenu", "Quitter vers le menu").ToString();
		if (Name == n"ReturnButton")
			return NSLOCTEXT("HOLEndBanner", "ReturnToLobby", "Retour au salon").ToString();
		if (Name == n"StatsButton")
			return NSLOCTEXT("HOLEndBanner", "Stats", "Statistiques").ToString();
		return "";
	}
}
