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

