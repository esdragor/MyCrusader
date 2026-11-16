// The in-game menu's rules in script (D356, D357, V1-Q1, M6-Q7 revised, M6-Q10; migration D830/D835, pilot 2 MIG-P2a): the seven the menu's
// screen reads. TEMPORARY TWIN of Source/HeroesOfLiberty/Player/HOLGameMenuRules.cpp (CanSurrender, CanQuitToMenu, OffersReturnToLobby,
// QuitsToList, ShowsStatsButton, GetEscapeAction, KeepsMatchMenuInput): the C++ copy stays while HOLPlayerController, HOLGameHUD, HOLGameScreen, HOLEndBanner, HOLStatsScreen,
// HOLUIOpen and HOLManagerClientRules call it; both copies are held to the same vectors (HOLGameMenuTests.cpp and HOLGameMenuRules_Test.as).
// The C++ enumerations are no UENUM: the script has its own, value for value, until they are exposed (TYPES_A_EXPOSER of the pilot).

// EHOLEndBanner's values (D357, V1-Q2, M6-Q10).
enum EHOLGameMenuBanner
{
	None,
	Defeat,
	Victory,
	// The match over, seen by a spectator (no slot).
	MatchOver,
}

// EHOLMatchOrigin's values (M6 R4.7, M6-Q10).
enum EHOLGameMenuOrigin
{
	// New game, the console's "open", a test: back to the main menu.
	Local,
	// From the manager without a lobby: the Multiplayer screen's « Regarder », a spectator.
	Manager,
	// From a lobby, as one of its players.
	Lobby,
}

// EHOLEscapeAction's values (V1.3a, D313, V1-Q1).
enum EHOLGameMenuEscape
{
	CloseMenu,
	CancelMode,
	Deselect,
	OpenMenu,
}

namespace HOLGameMenuRules
{
	// What Escape does in a match: the open menu first, then a mode armed (the selection kept, D313), then the selection, else the menu.
	EHOLGameMenuEscape GetEscapeAction(bool bMenuOpen, bool bModeArmed, bool bHasSelection)
	{
		if (bMenuOpen)
		{
			return EHOLGameMenuEscape::CloseMenu;
		}
		if (bModeArmed)
		{
			return EHOLGameMenuEscape::CancelMode;
		}
		return bHasSelection ? EHOLGameMenuEscape::Deselect : EHOLGameMenuEscape::OpenMenu;
	}

	// Menu input stays on while a match screen opens or another one is still open (V1.4a: Escape must still close the banner).
	bool KeepsMatchMenuInput(bool bOpening, bool bOtherScreenOpen)
	{
		return bOpening || bOtherScreenOpen;
	}

	// « Abandonner » is offered only to a player still standing, the match not over.
	bool CanSurrender(bool bDefeated, bool bMatchOver)
	{
		return !bDefeated && !bMatchOver;
	}

	// « Quitter vers le menu » is offered in its place: a spectator or a match over has nothing left to lose (V1 trial).
	bool CanQuitToMenu(bool bDefeated, bool bMatchOver)
	{
		return !CanSurrender(bDefeated, bMatchOver);
	}

	// « Retour au salon » once the match is over: a lobby's match, for its players alone (a spectator is in no lobby).
	bool OffersReturnToLobby(EHOLGameMenuBanner Banner, EHOLGameMenuOrigin Origin)
	{
		return Origin == EHOLGameMenuOrigin::Lobby && (Banner == EHOLGameMenuBanner::Defeat || Banner == EHOLGameMenuBanner::Victory);
	}

	// The quit goes to the Multiplayer screen's list for any match opened from the manager; else to the menu.
	bool QuitsToList(EHOLGameMenuOrigin Origin)
	{
		return Origin != EHOLGameMenuOrigin::Local;
	}

	// « Statistiques » on the game screen: a spectator's during the match; everyone's once it is over.
	bool ShowsStatsButton(bool bSpectator, bool bMatchOver)
	{
		return bSpectator || bMatchOver;
	}
}
