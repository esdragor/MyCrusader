// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** What Escape does in a match (D356), from the most to the least urgent. */
enum class EHOLEscapeAction : uint8
{
	/** The in-game menu is open: Escape closes it (Back). */
	CloseMenu,
	/** Placement, a chosen age or the attack-move mode is armed: Escape leaves it, the selection kept (D313, J7b.3). */
	CancelMode,
	/** Units or a building selected: Escape selects nothing (D262). */
	Deselect,
	/** Nothing else to undo: Escape opens the in-game menu. */
	OpenMenu,
};

/** The end banner over the match (D357, V1-Q2): none, the player's own result, or the spectator's (M6-Q10). */
enum class EHOLEndBanner : uint8
{
	None,
	Defeat,
	Victory,
	/** The match over, seen by a spectator (no slot): a neutral title, the winner under it (M6-Q10). */
	MatchOver,
};

/** Where a match was opened from (M6 R4.7, M6-Q10): its end banner's way out follows. */
enum class EHOLMatchOrigin : uint8
{
	/** New game, the console's "open", a test: back to the main menu. */
	Local,
	/** From the manager without a lobby: the Multiplayer screen's « Regarder », a spectator (M6-Q9). */
	Manager,
	/** From a lobby, as one of its players (R4.7): the way back to it (R4.8). */
	Lobby,
};

/** The in-game menu's rules (D356, V1-Q1), pure: no world, no actor. */
namespace HOLGameMenuRules
{
	HEROESOFLIBERTY_API EHOLEscapeAction GetEscapeAction(bool bMenuOpen, bool bModeArmed, bool bHasSelection);

	/** « Abandonner » is offered only to a player still standing, the match not over. */
	HEROESOFLIBERTY_API bool CanSurrender(bool bDefeated, bool bMatchOver);

	/** « Quitter vers le menu » is offered in its place: a spectator or a match over has nothing left to lose (V1 trial). */
	HEROESOFLIBERTY_API bool CanQuitToMenu(bool bDefeated, bool bMatchOver);

	/**
	 * The banner of a player (D357): the match over, Victory when its slot won (fallen before or not: its camp won),
	 * Defeat otherwise (no winner too); the match going on, Defeat once its slot is defeated, else None. Masks: bit Slot.
	 * Without a slot (a spectator, M6-Q10): MatchOver once the match is over, else None.
	 */
	HEROESOFLIBERTY_API EHOLEndBanner GetEndBanner(int32 MySlot, uint8 DefeatedMask, bool bMatchOver, uint8 WinnerMask);

	/**
	 * Whether « Retour au salon » is offered once the match is over (M6-Q7 revised), by the end banner and the in-game
	 * menu: a lobby's match, for its players alone (a spectator is in no lobby, M6-Q10). Nobody is sent back by
	 * himself: everyone stays in the finished match as long as he likes and leaves when he chooses.
	 */
	HEROESOFLIBERTY_API bool OffersReturnToLobby(EHOLEndBanner Banner, EHOLMatchOrigin Origin);

	/** Whether the banner's quit goes to the Multiplayer screen's list (« Quitter vers la liste », R4.8, M6-Q10): any
	 *  match opened from the manager, a lobby's or a spectator's; else « Quitter vers le menu ». */
	HEROESOFLIBERTY_API bool QuitsToList(EHOLMatchOrigin Origin);

	/**
	 * Whether to open the banner (V1-Q2): one is due, not already shown, and not the one the player closed by
	 * « Continuer à regarder » (a new result, e.g. Defeat then the team's Victory, opens again).
	 */
	HEROESOFLIBERTY_API bool ShouldOpenEndBanner(EHOLEndBanner Due, EHOLEndBanner Shown, EHOLEndBanner Closed);

	/**
	 * Whether the match keeps its menu input (IMC_UI, Escape as Back) when a match screen (the in-game menu, the end
	 * banner) opens or closes (V1.4a): open, yes; closed, only while another match screen is still open, e.g. the
	 * menu closed over the banner, whose Escape is then « Continuer à regarder ».
	 */
	HEROESOFLIBERTY_API bool KeepsMatchMenuInput(bool bOpening, bool bOtherScreenOpen);

	/**
	 * Whether the game screen shows its « Statistiques » button (M6-Q10, C6): a spectator's, during the match; a player
	 * sees no statistics before the end; once the match is over, everyone's (the end banner closed, still at hand).
	 */
	HEROESOFLIBERTY_API bool ShowsStatsButton(bool bSpectator, bool bMatchOver);
}
