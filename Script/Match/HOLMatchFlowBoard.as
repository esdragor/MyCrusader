// The pause and diplomacy decisions of AHOLGameState (MIG-CPP step 5, lot S1; D836 point 4: the script is the source of the rule). Pure rules with
// parameters: nothing here reads the world, the replicated fields of AHOLGameState (PausesLeft, PausedBySlot, PausedAtSeconds, WindowPausedSeconds,
// the diplomacy book) and their replication stay in C++, as the logs ("Pause: ...", "Diplomacy: ..."), which the C++ still writes word for word.
// Same rule as the C++ FHOLPauseRules (Net/HOLPauseRules.cpp) while both copies exist; HOLMatchFlow_Test.as replays the C++ vectors on this one.
// NOT in a Dev / Editor / Examples folder: the dedicated server runs it.

/** The answers of a pause or resume request, in the order of the C++ FHOLPauseRules::EAnswer (the bridge sends them as a whole number). */
enum EHOLPauseAnswer
{
	Allowed,
	NotInMatch,
	NoPauseLeft,
	AlreadyPaused,
	NotPaused,
	TimerRunning
}

/** The gate of a stance order, before HOLDiplomacy::SetStance: Open, or the reason told in the journal (the order of the C++ ApplyStance). */
enum EHOLDiplomacyGate
{
	Open,
	NotDiplomacyMode,
	NoSuchPlayer
}

namespace HOLMatchFlow
{
	/** D513: 3 pauses per match, shared by every player. */
	int DefaultPausesPerMatch()
	{
		return 3;
	}

	/** D513: the other players wait 60 s before they may resume someone else's pause. */
	float DefaultTimerSeconds()
	{
		return 60.0;
	}

	/** Who may pause: bSolo = the solo game (free, no limit, no timer); PausedBySlot -1 = the match runs. */
	EHOLPauseAnswer CanPause(bool bSolo, int PausedBySlot, int PausesLeft, bool bRequesterInMatch)
	{
		if (PausedBySlot != -1)
			return EHOLPauseAnswer::AlreadyPaused;
		if (bSolo)
			return EHOLPauseAnswer::Allowed;
		if (!bRequesterInMatch)
			return EHOLPauseAnswer::NotInMatch;
		return PausesLeft > 0 ? EHOLPauseAnswer::Allowed : EHOLPauseAnswer::NoPauseLeft;
	}

	/** Seconds left on the timer, 0 once over and when not paused. */
	float TimerLeft(int PausedBySlot, float PausedAtSeconds, float Now)
	{
		if (PausedBySlot == -1)
			return 0.0;
		return Math::Max(DefaultTimerSeconds() - (Now - PausedAtSeconds), 0.0);
	}

	/** Who may resume: the one who paused at once, another player once the timer is over, only a player still in the match. */
	EHOLPauseAnswer CanResume(bool bSolo, int PausedBySlot, float PausedAtSeconds, int RequesterSlot, bool bRequesterInMatch, float Now)
	{
		if (PausedBySlot == -1)
			return EHOLPauseAnswer::NotPaused;
		if (bSolo)
			return EHOLPauseAnswer::Allowed;
		if (!bRequesterInMatch)
			return EHOLPauseAnswer::NotInMatch;
		if (RequesterSlot == PausedBySlot || TimerLeft(PausedBySlot, PausedAtSeconds, Now) <= 0.0)
			return EHOLPauseAnswer::Allowed;
		return EHOLPauseAnswer::TimerRunning;
	}

	/** The pauses left once an allowed pause is taken: one fewer (never below 0), none counted in the solo game. */
	int PausesLeftAfterPause(bool bSolo, int PausesLeft)
	{
		if (bSolo)
			return PausesLeft;
		return Math::Max(PausesLeft - 1, 0);
	}

	/** What a pause that ends (resumed, or lifted by a leaving player) adds to the window of the match: the seconds it ran, never negative. */
	float PausedSeconds(float PausedAtSeconds, float Now)
	{
		return Math::Max(Now - PausedAtSeconds, 0.0);
	}

	/** D720: the pause of a player who leaves is lifted at once, by the server only, and stays counted. */
	bool ShouldLiftPause(bool bAuthority, bool bPaused, int PausedBySlot, int LeavingSlot)
	{
		return bAuthority && bPaused && PausedBySlot == LeavingSlot;
	}

	/** The gate of a stance order: the server, in the Diplomacy mode, between two occupied slots (a client or a closed slot: "no such player"). */
	EHOLDiplomacyGate StanceGate(bool bAuthority, bool bDiplomacyMode, bool bFromOccupied, bool bToOccupied)
	{
		if (bAuthority && bDiplomacyMode && bFromOccupied && bToOccupied)
			return EHOLDiplomacyGate::Open;
		return bDiplomacyMode ? EHOLDiplomacyGate::NoSuchPlayer : EHOLDiplomacyGate::NotDiplomacyMode;
	}

	/** The gate of a vision order: the server, in the Diplomacy mode. */
	bool VisionGateOpen(bool bAuthority, bool bDiplomacyMode)
	{
		return bAuthority && bDiplomacyMode;
	}

	/** D373: an alliance formed (and announced) after the last defeat can end the match at that second. */
	bool ShouldEvaluateMatchEnd(bool bAnnounce, bool bFormed)
	{
		return bAnnounce && bFormed;
	}
}
