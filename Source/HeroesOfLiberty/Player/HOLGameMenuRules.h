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

/** The end banner over the match (D357, V1-Q2): none, or the player's own result. */
enum class EHOLEndBanner : uint8
{
	None,
	Defeat,
	Victory,
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
	 */
	HEROESOFLIBERTY_API EHOLEndBanner GetEndBanner(int32 MySlot, uint8 DefeatedMask, bool bMatchOver, uint8 WinnerMask);

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
}
