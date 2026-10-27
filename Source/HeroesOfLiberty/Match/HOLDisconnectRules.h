// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"

struct FHOLMatchConfig;

/** R7 (R-Q5, D71): a slot whose player left during the match, kept for him until the end of the window. Server only. */
struct FHOLAbsentSlot
{
	int32 Slot = INDEX_NONE;
	/** The slot's name in the launch configuration. */
	FString Name;
	/** Server world time of the departure. */
	double LeftAt = 0.;
	/** The stock at the departure, given back on the return (R7.2). */
	FHOLResources Resources;
};

/**
 * R7 (R-Q5, D71): a named player who leaves a match launched by the lobby manager keeps his slot (units, buildings and
 * stock) for a window (~3 min); he takes it back by joining again under the same name; at the end of the window, his
 * slot is defeated as by a surrender (reason Disconnected). Pure rules, for the tests.
 */
struct HEROESOFLIBERTY_API FHOLDisconnectRules
{
	/** The window by default (R-Q5: ~3 min). */
	static constexpr double DefaultWindowSeconds = 180.;

	/**
	 * Whether a player leaving Slot keeps it for the window: a named human slot of the configuration (a match of the lobby
	 * manager), the match started (not waiting for its players) and not over, the slot not defeated.
	 */
	static bool KeepsSlot(const FHOLMatchConfig& Config, int32 Slot, bool bWaitingForPlayers, bool bMatchOver, bool bSlotDefeated);

	/** The window has run out (at its end exactly too). */
	static bool IsWindowOver(double LeftAt, double Now, double WindowSeconds);

	/** R7.5: how long « Bob : de retour » stays in the banner. */
	static constexpr double BackNoticeSeconds = 5.;

	/**
	 * R7.5: the banner's line of a player gone, SecondsLeft before his defeat (rounded up): « Bob : connexion perdue — 2:41 »;
	 * never more than WindowSeconds when given (> 0).
	 */
	static FText FormatAbsentLine(const FString& Name, double SecondsLeft, double WindowSeconds = 0.);
	/** R7.5: the banner's line of a player back: « Bob : de retour ». */
	static FText FormatBackLine(const FString& Name);

	/** Seconds left in the window, 0 at its end (the banner of R7.5). */
	static double GetSecondsLeft(double LeftAt, double Now, double WindowSeconds);

	/** Index in Absent of the slot, INDEX_NONE if not absent. */
	static int32 FindAbsent(TConstArrayView<FHOLAbsentSlot> Absent, int32 Slot);

	/** The slots whose window has run out, in the order of the departures. */
	static TArray<int32> GetExpired(TConstArrayView<FHOLAbsentSlot> Absent, double Now, double WindowSeconds);

	/** The stock a player gets when he takes a slot: the one kept at his departure (R7.2), else the stock of the start. */
	static FHOLResources GetStockOnTaking(const FHOLAbsentSlot* Absent, const FHOLResources& Starting);
};
