// The pause and diplomacy decisions of AHOLGameState (MIG-CPP step 5, lot S1; D836 point 4: the script is the source of the rule). Pure rules with
// parameters: nothing here reads the world, the replicated fields of AHOLGameState (PausesLeft, PausedBySlot, PausedAtSeconds, WindowPausedSeconds,
// the diplomacy book) and their replication stay in C++, as the logs ("Pause: ...", "Diplomacy: ..."), which the C++ still writes word for word.
// The one source of the pause rule (the C++ FHOLPauseRules and its test Net.PauseLimits are removed); HOLMatchFlow_Test.as holds its vectors.
// NOT in a Dev / Editor / Examples folder: the dedicated server runs it.

/** The answers of a pause or resume request, the bridge sends them as a whole number and the C++ journal reads the order (Test_MatchFlowAnswerOrder). */
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

	/** S2a (MIG-CPP step 5): the answer of MatchEndVerdict when the match is not over (a winner mask is 0..255, MaxSlots = 8 bits). */
	int MatchNotOver()
	{
		return 256;
	}

	/**
	 * S2a: the gate of AHOLGameState::DefeatSlot, the server only, a match still running, a real reason, a valid slot not yet defeated, in an occupied camp.
	 * Same as the former inline refusal (!HasAuthority || bMatchOver || Reason == None || !IsValidIndex || IsSlotDefeated || camp == INDEX_NONE).
	 */
	bool DefeatGateOpen(bool bAuthority, bool bMatchOver, bool bReasonNone, bool bSlotValid, bool bDefeated, bool bCampOccupied)
	{
		return bAuthority && !bMatchOver && !bReasonNone && bSlotValid && !bDefeated && bCampOccupied;
	}

	/**
	 * S2c: the gate of AHOLGameState::HandleForumFall, the server only, for a main forum of a valid slot not yet defeated and whose defeat is not already
	 * pending, in an occupied camp. Same as the former inline refusal (!HasAuthority || !IsPrimaryForum || !IsValidIndex || IsSlotDefeated || Pending || camp == INDEX_NONE).
	 * Game thread only (DamageBuilding, called by the combat processor outside its ParallelFor, the cheats and the volcano).
	 */
	bool FallGateOpen(bool bAuthority, bool bPrimaryForum, bool bSlotValid, bool bDefeated, bool bPendingForum, bool bCampOccupied)
	{
		return bAuthority && bPrimaryForum && bSlotValid && !bDefeated && !bPendingForum && bCampOccupied;
	}

	/**
	 * S2c: the gate of AHOLGameState::HandleKingDeath, the server only, a match still running, a valid slot not yet defeated and whose king's defeat is not
	 * already pending, in an occupied camp. Same as the former inline refusal (!HasAuthority || bMatchOver || !IsValidIndex || IsSlotDefeated || Pending || camp == INDEX_NONE).
	 */
	bool KingGateOpen(bool bAuthority, bool bMatchOver, bool bSlotValid, bool bDefeated, bool bPendingKing, bool bCampOccupied)
	{
		return bAuthority && !bMatchOver && bSlotValid && !bDefeated && !bPendingKing && bCampOccupied;
	}

	/**
	 * S2a: the verdict of AHOLGameState::EvaluateMatchEnd. The winner mask when the match is over, else MatchNotOver(). The camps rule is HOLVictoryRules (C++,
	 * read through HOLVictoryRead, States / Teams as the C++ gives them: Teams all 0 in the Diplomacy mode); in the Diplomacy mode with at least one beaten
	 * slot, an alliance of survivors ends it (D373): AlliedWinnerMask is the C++ HOLDiplomacy::AlliedVictory winners (bits), -1 when it did not end.
	 */
	int MatchEndVerdict(const TArray<int>& States, const TArray<int>& Teams, int DefeatedMask, bool bDiplomacyMode, int AlliedWinnerMask)
	{
		int WinnerMask = 0;
		TArray<int> Winners;
		if (HOLVictoryRead::Evaluate(States, Teams, DefeatedMask, WinnerMask, Winners))
			return WinnerMask;
		if (bDiplomacyMode && DefeatedMask != 0 && AlliedWinnerMask >= 0)
			return AlliedWinnerMask;
		return MatchNotOver();
	}
}

/** The script side of UHOLMatchFlowSubsystem (the C++ part, AHOLGameState, gives the replicated state and keeps its journal lines; the rule is HOLMatchFlow). */
class UHOLMatchFlowBoardScript : UHOLMatchFlowSubsystem
{
	UFUNCTION(BlueprintOverride)
	int PauseAnswer(bool bSolo, int PausedBySlot, int PausesLeft, bool bRequesterInMatch)
	{
		return int(HOLMatchFlow::CanPause(bSolo, PausedBySlot, PausesLeft, bRequesterInMatch));
	}

	UFUNCTION(BlueprintOverride)
	int ResumeAnswer(bool bSolo, int PausedBySlot, float64 PausedAtSeconds, int RequesterSlot, bool bRequesterInMatch, float64 Now)
	{
		return int(HOLMatchFlow::CanResume(bSolo, PausedBySlot, PausedAtSeconds, RequesterSlot, bRequesterInMatch, Now));
	}

	UFUNCTION(BlueprintOverride)
	int PausesAfterPause(bool bSolo, int PausesLeft)
	{
		return HOLMatchFlow::PausesLeftAfterPause(bSolo, PausesLeft);
	}

	UFUNCTION(BlueprintOverride)
	int StanceGateAnswer(bool bAuthority, bool bDiplomacyMode, bool bFromOccupied, bool bToOccupied)
	{
		return int(HOLMatchFlow::StanceGate(bAuthority, bDiplomacyMode, bFromOccupied, bToOccupied));
	}

	UFUNCTION(BlueprintOverride)
	int VisionGateAnswer(bool bAuthority, bool bDiplomacyMode)
	{
		return HOLMatchFlow::VisionGateOpen(bAuthority, bDiplomacyMode) ? 1 : 0;
	}

	UFUNCTION(BlueprintOverride)
	int EvaluateEndAnswer(bool bAnnounce, bool bFormed)
	{
		return HOLMatchFlow::ShouldEvaluateMatchEnd(bAnnounce, bFormed) ? 1 : 0;
	}

	UFUNCTION(BlueprintOverride)
	int DefeatGateAnswer(bool bAuthority, bool bMatchOver, bool bReasonNone, bool bSlotValid, bool bDefeated, bool bCampOccupied)
	{
		return HOLMatchFlow::DefeatGateOpen(bAuthority, bMatchOver, bReasonNone, bSlotValid, bDefeated, bCampOccupied) ? 1 : 0;
	}

	UFUNCTION(BlueprintOverride)
	int FallGateAnswer(bool bAuthority, bool bPrimaryForum, bool bSlotValid, bool bDefeated, bool bPendingForum, bool bCampOccupied)
	{
		return HOLMatchFlow::FallGateOpen(bAuthority, bPrimaryForum, bSlotValid, bDefeated, bPendingForum, bCampOccupied) ? 1 : 0;
	}

	UFUNCTION(BlueprintOverride)
	int KingGateAnswer(bool bAuthority, bool bMatchOver, bool bSlotValid, bool bDefeated, bool bPendingKing, bool bCampOccupied)
	{
		return HOLMatchFlow::KingGateOpen(bAuthority, bMatchOver, bSlotValid, bDefeated, bPendingKing, bCampOccupied) ? 1 : 0;
	}

	UFUNCTION(BlueprintOverride)
	int MatchEndAnswer(const TArray<int>& States, const TArray<int>& Teams, int DefeatedMask, bool bDiplomacyMode, int AlliedWinnerMask)
	{
		return HOLMatchFlow::MatchEndVerdict(States, Teams, DefeatedMask, bDiplomacyMode, AlliedWinnerMask);
	}
}