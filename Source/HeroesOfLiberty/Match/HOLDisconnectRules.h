// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "Technologies/HOLTechnologyRules.h"

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
	/** D719 (T0): the technologies acquired at the departure, and those whose research ends while he is away; given back on the return. */
	FHOLTechnologyState Technologies;
	/** D718 (T5): the window is over and the AI keeps the seat (GetWindowEnd = AIHolds): the record stays for his return, no longer expires. */
	bool bWindowOver = false;
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

	/** D718 (T1): what the end of the window does to the seat. */
	enum class EWindowEnd : uint8
	{
		/** The slot is defeated (reason Disconnected), as today. */
		Defeat,
		/** The AI keeps the seat to the end of the match (teams, free for all of 3 and more). */
		AIHolds,
	};
	/**
	 * D718 (T1): the end of the window of Slot, by the camps of the launch configuration (D357, HOLVictoryRules::GetCamp; only the
	 * occupied slots count, the AI seats too): Defeat when he is alone in his camp and there are only two camps in all (the 1v1, the
	 * 1 against 2); AIHolds otherwise. A closed or unknown slot: Defeat (nothing to hold).
	 */
	static EWindowEnd GetWindowEnd(const FHOLMatchConfig& Config, int32 Slot);

	/** D718 (T1): the states of a seat of a human during a match. */
	enum class ESeatState : uint8
	{
		/** Its player is connected. */
		Present,
		/** Away, in the window: the AI holds the seat for him. */
		AbsentAIHolds,
		/** Away, the window past: the AI keeps the seat to the end (GetWindowEnd = AIHolds). */
		AIHolds,
		/** Defeated: nothing to return to. */
		Defeated,
	};
	/** D718 (T1): whether the name of the seat's player may take it again: not once defeated, not once the match is over. */
	static bool MayReturn(ESeatState State, bool bMatchOver);

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

	/** T9: a seat's line for hol.Disconnect.Report, e.g. "slot 1 (Bob) away, AI holds, 162 s left" (SecondsLeft rounded up). */
	static FString FormatSeatReport(int32 Slot, const FString& Name, ESeatState State, double SecondsLeft);

	/** Index in Absent of the slot, INDEX_NONE if not absent. */
	static int32 FindAbsent(TConstArrayView<FHOLAbsentSlot> Absent, int32 Slot);

	/** The slots whose window has run out, in the order of the departures. */
	static TArray<int32> GetExpired(TConstArrayView<FHOLAbsentSlot> Absent, double Now, double WindowSeconds);

	/** The stock a player gets when he takes a slot: the one kept at his departure (R7.2), else the stock of the start. */
	static FHOLResources GetStockOnTaking(const FHOLAbsentSlot* Absent, const FHOLResources& Starting);
};
