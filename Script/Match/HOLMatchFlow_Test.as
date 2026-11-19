// HOLMatchFlow, the pause and diplomacy decisions of AHOLGameState (MIG-CPP step 5, lot S1; D836 point 4: the script is the source of the rule).
// Pure rules with parameters (HOLMatchFlowBoard.as): the limited pause (D513: 3 pauses per match shared by all, 60 s timer, only the players still in the
// match, solo free), the lift of the pause of a player who leaves (D720), the gates of ApplyStance / ApplyVision (HOLDiplomacy::SetStance / SetVision
// stay the rule of the stances themselves). RED until HOLMatchFlowBoard.as exists (compiles after the build).
//
// The pause cases are the vectors of the former C++ test HeroesOfLiberty.Net.PauseLimits (removed with FHOLPauseRules: this file is now the only test of
// the rule). The lift replays HeroesOfLiberty.Match.DisconnectPause
// (FHOLDisconnectPauseTest, Tests/HOLDisconnectTests.cpp:800, LiftPauseOf: only the pause of the leaving slot, counted pauses untouched).

// Pause: who may pause. A dedicated or listen server (bSolo false) counts the pauses; the solo game does not.
void Test_MatchFlowPauseAllowedAndRefused(FUnitTest& T)
{
	T.AssertEquals(int(EHOLPauseAnswer::Allowed), int(HOLMatchFlow::CanPause(false, -1, 3, true)), "a player in the match pauses");
	T.AssertEquals(int(EHOLPauseAnswer::NotInMatch), int(HOLMatchFlow::CanPause(false, -1, 3, false)), "a defeated player or a spectator does not");
	T.AssertEquals(int(EHOLPauseAnswer::AlreadyPaused), int(HOLMatchFlow::CanPause(false, 1, 2, true)), "no second pause on a paused match");
	T.AssertEquals(int(EHOLPauseAnswer::NoPauseLeft), int(HOLMatchFlow::CanPause(false, -1, 0, true)), "no fourth pause (a listen server counts the same)");
	T.AssertEquals(int(EHOLPauseAnswer::Allowed), int(HOLMatchFlow::CanPause(true, -1, 0, true)), "solo: no limit");
	T.AssertEquals(int(EHOLPauseAnswer::Allowed), int(HOLMatchFlow::CanPause(true, -1, 0, false)), "solo: even a requester not in the match");
	T.AssertEquals(int(EHOLPauseAnswer::AlreadyPaused), int(HOLMatchFlow::CanPause(true, 0, 3, true)), "solo: still not twice");
}

// Pause: resume. The one who paused at once, another player once the 60 s are over, never a spectator.
void Test_MatchFlowResume(FUnitTest& T)
{
	T.AssertEquals(int(EHOLPauseAnswer::NotPaused), int(HOLMatchFlow::CanResume(false, -1, 0.0, 0, true, 10.0)), "nothing to resume while the match runs");
	T.AssertEquals(int(EHOLPauseAnswer::Allowed), int(HOLMatchFlow::CanResume(false, 1, 100.0, 1, true, 101.0)), "the one who paused resumes at once");
	T.AssertEquals(int(EHOLPauseAnswer::TimerRunning), int(HOLMatchFlow::CanResume(false, 1, 100.0, 0, true, 159.0)), "another player waits for the timer");
	T.AssertEquals(int(EHOLPauseAnswer::Allowed), int(HOLMatchFlow::CanResume(false, 1, 100.0, 0, true, 160.0)), "another player resumes once it is over");
	T.AssertEquals(int(EHOLPauseAnswer::NotInMatch), int(HOLMatchFlow::CanResume(false, 1, 100.0, 2, false, 500.0)), "a spectator never resumes");
	T.AssertEquals(int(EHOLPauseAnswer::NotInMatch), int(HOLMatchFlow::CanResume(false, 1, 100.0, 1, false, 101.0)), "the one who paused, once defeated, does not either");
	T.AssertEquals(int(EHOLPauseAnswer::Allowed), int(HOLMatchFlow::CanResume(true, 0, 10.0, 0, true, 11.0)), "solo: no timer");
	T.AssertEquals(int(EHOLPauseAnswer::Allowed), int(HOLMatchFlow::CanResume(true, 0, 10.0, 3, false, 11.0)), "solo: anyone resumes");
}

// Pause: the timer (60 s) and the count of the pauses.
void Test_MatchFlowTimerAndCount(FUnitTest& T)
{
	T.AssertEquals(60.0, HOLMatchFlow::DefaultTimerSeconds(), "the timer is 60 s");
	T.AssertEquals(3, HOLMatchFlow::DefaultPausesPerMatch(), "3 pauses per match by default");
	T.AssertEquals(60.0, HOLMatchFlow::TimerLeft(1, 100.0, 100.0), "timer: 60 s at the start");
	T.AssertEquals(15.0, HOLMatchFlow::TimerLeft(1, 100.0, 145.0), "timer: 15 s left after 45 s");
	T.AssertEquals(0.0, HOLMatchFlow::TimerLeft(1, 100.0, 200.0), "timer: 0 once over");
	T.AssertEquals(0.0, HOLMatchFlow::TimerLeft(-1, 100.0, 120.0), "timer: 0 when not paused");
	T.AssertEquals(2, HOLMatchFlow::PausesLeftAfterPause(false, 3), "one pause used, shared by all");
	T.AssertEquals(0, HOLMatchFlow::PausesLeftAfterPause(false, 1), "the last one");
	T.AssertEquals(0, HOLMatchFlow::PausesLeftAfterPause(false, 0), "never below 0");
	T.AssertEquals(0, HOLMatchFlow::PausesLeftAfterPause(true, 0), "solo: nothing counted");
	T.AssertEquals(3, HOLMatchFlow::PausesLeftAfterPause(true, 3), "solo: the count is untouched");
	// Three pauses in a row use the three (the C++ test: 3 pauses used, then the fourth is refused).
	int Left = HOLMatchFlow::DefaultPausesPerMatch();
	for (int Round = 0; Round < 3; Round++)
		Left = HOLMatchFlow::PausesLeftAfterPause(false, Left);
	T.AssertEquals(0, Left, "3 pauses used");
	T.AssertEquals(int(EHOLPauseAnswer::NoPauseLeft), int(HOLMatchFlow::CanPause(false, -1, Left, true)), "then no fourth pause");
	T.AssertEquals(40.0, HOLMatchFlow::PausedSeconds(100.0, 140.0), "the window counts the pause that ran");
	T.AssertEquals(0.0, HOLMatchFlow::PausedSeconds(100.0, 90.0), "never negative");
}

// Pause: the whole numbers the bridge sends are the order the C++ journal reads (HOLGameState.cpp: the "Pause: refused to ..."
// reason is picked by this number: allowed, not in the match, no pause left, already paused, not paused, timer running). Changing the order breaks the journal lines.
void Test_MatchFlowAnswerOrder(FUnitTest& T)
{
	T.AssertEquals(0, int(EHOLPauseAnswer::Allowed), "answer 0: allowed");
	T.AssertEquals(1, int(EHOLPauseAnswer::NotInMatch), "answer 1: not in the match");
	T.AssertEquals(2, int(EHOLPauseAnswer::NoPauseLeft), "answer 2: no pause left");
	T.AssertEquals(3, int(EHOLPauseAnswer::AlreadyPaused), "answer 3: already paused");
	T.AssertEquals(4, int(EHOLPauseAnswer::NotPaused), "answer 4: not paused");
	T.AssertEquals(5, int(EHOLPauseAnswer::TimerRunning), "answer 5: timer running");
	// The bridge (UHOLMatchFlowBoardScript) sends exactly these numbers.
	T.AssertEquals(5, HOLMatchFlow::CanResume(false, 1, 100.0, 0, true, 159.5) == EHOLPauseAnswer::TimerRunning ? 5 : -1, "159.5 s: still the timer");
	T.AssertEquals(0, int(HOLMatchFlow::CanResume(false, 1, 100.0, 0, true, 160.5)), "160.5 s: allowed (answer 0)");
}

// Pause lifted at once when the one who paused leaves (D720); the count is not touched (PausesLeftAfterPause is not called).
void Test_MatchFlowLiftPauseOfLeaver(FUnitTest& T)
{
	T.AssertTrue(HOLMatchFlow::ShouldLiftPause(true, true, 1, 1), "the pause of the leaving slot is lifted");
	T.AssertFalse(HOLMatchFlow::ShouldLiftPause(true, true, 1, 0), "another slot leaves: the pause stays");
	T.AssertFalse(HOLMatchFlow::ShouldLiftPause(true, false, -1, 1), "no pause: nothing to lift");
	T.AssertFalse(HOLMatchFlow::ShouldLiftPause(false, true, 1, 1), "a client never lifts");
}

// Diplomacy: the gate of a stance order (authority, Diplomacy mode, both slots occupied), then SetStance decides.
void Test_MatchFlowStanceGate(FUnitTest& T)
{
	T.AssertEquals(int(EHOLDiplomacyGate::Open), int(HOLMatchFlow::StanceGate(true, true, true, true)), "server, Diplomacy mode, two players: open");
	T.AssertEquals(int(EHOLDiplomacyGate::NotDiplomacyMode), int(HOLMatchFlow::StanceGate(true, false, true, true)), "standard mode: 'not the Diplomacy mode'");
	T.AssertEquals(int(EHOLDiplomacyGate::NotDiplomacyMode), int(HOLMatchFlow::StanceGate(true, false, false, true)), "standard mode told first, even with a closed slot");
	T.AssertEquals(int(EHOLDiplomacyGate::NoSuchPlayer), int(HOLMatchFlow::StanceGate(true, true, false, true)), "the sender slot closed: 'no such player'");
	T.AssertEquals(int(EHOLDiplomacyGate::NoSuchPlayer), int(HOLMatchFlow::StanceGate(true, true, true, false)), "the target slot closed: 'no such player'");
	T.AssertEquals(int(EHOLDiplomacyGate::NoSuchPlayer), int(HOLMatchFlow::StanceGate(false, true, true, true)), "a client: refused, told 'no such player' as the C++ does");
	T.AssertEquals(int(EHOLDiplomacyGate::NotDiplomacyMode), int(HOLMatchFlow::StanceGate(false, false, true, true)), "a client in standard mode: 'not the Diplomacy mode'");
}

// Diplomacy: vision (authority and Diplomacy mode only), and when a formed alliance asks the match end to be evaluated again (D373).
void Test_MatchFlowVisionAndAllianceEnd(FUnitTest& T)
{
	T.AssertTrue(HOLMatchFlow::VisionGateOpen(true, true), "server in Diplomacy mode: open");
	T.AssertFalse(HOLMatchFlow::VisionGateOpen(true, false), "standard mode: shut");
	T.AssertFalse(HOLMatchFlow::VisionGateOpen(false, true), "a client: shut");
	T.AssertTrue(HOLMatchFlow::ShouldEvaluateMatchEnd(true, true), "an alliance formed and announced: the end is evaluated");
	T.AssertFalse(HOLMatchFlow::ShouldEvaluateMatchEnd(true, false), "announced, not formed (a break or a neutral): no");
	T.AssertFalse(HOLMatchFlow::ShouldEvaluateMatchEnd(false, true), "nothing announced: no");
}

// ---- S2a (MIG-CPP step 5, lot S2a reduced): the gate of DefeatSlot and the verdict of EvaluateMatchEnd (HOLMatchFlowBoard.as). RED until the board has
// DefeatGateOpen / MatchEndVerdict / MatchNotOver. Cases drawn from AHOLGameState::DefeatSlot (HOLGameState.cpp:650) and EvaluateMatchEnd (:694-717).

/** A list of the first N of A, B, C, D (the slots, their states or their teams). */
TArray<int> MatchFlowList(int N, int A, int B, int C, int D)
{
	TArray<int> Result;
	if (N > 0) Result.Add(A);
	if (N > 1) Result.Add(B);
	if (N > 2) Result.Add(C);
	if (N > 3) Result.Add(D);
	return Result;
}

// DefeatSlot: refused without authority, once the match is over, without a reason, for an invalid or already defeated slot, for a closed camp.
void Test_MatchFlowDefeatGate(FUnitTest& T)
{
	T.AssertTrue(HOLMatchFlow::DefeatGateOpen(true, false, false, true, false, true), "the server, a running match, a reason, a live slot of an occupied camp: open");
	T.AssertFalse(HOLMatchFlow::DefeatGateOpen(false, false, false, true, false, true), "a client never defeats a slot");
	T.AssertFalse(HOLMatchFlow::DefeatGateOpen(true, true, false, true, false, true), "the match is over: no more defeat");
	T.AssertFalse(HOLMatchFlow::DefeatGateOpen(true, false, true, true, false, true), "reason None: refused");
	T.AssertFalse(HOLMatchFlow::DefeatGateOpen(true, false, false, false, false, true), "a slot outside the table: refused");
	T.AssertFalse(HOLMatchFlow::DefeatGateOpen(true, false, false, true, true, true), "a slot already defeated: refused (never twice)");
	T.AssertFalse(HOLMatchFlow::DefeatGateOpen(true, false, false, true, false, false), "a closed camp: refused");
}

// EvaluateMatchEnd: one against one, free for all, one camp alone, teams, the Diplomacy mode.
void Test_MatchFlowMatchEndVerdict(FUnitTest& T)
{
	int NotOver = HOLMatchFlow::MatchNotOver();
	TArray<int> TwoStates = MatchFlowList(2, 1, 2, 0, 0);
	TArray<int> TwoTeams = MatchFlowList(2, 0, 0, 0, 0);
	int Verdict = HOLMatchFlow::MatchEndVerdict(TwoStates, TwoTeams, 0, false, -1);
	T.AssertEquals(NotOver, Verdict, "1 against 1, nobody defeated: goes on");
	Verdict = HOLMatchFlow::MatchEndVerdict(TwoStates, TwoTeams, 1, false, -1);
	T.AssertEquals(2, Verdict, "1 against 1, slot 0 defeated: slot 1 wins (mask 2)");
	Verdict = HOLMatchFlow::MatchEndVerdict(TwoStates, TwoTeams, 2, false, -1);
	T.AssertEquals(1, Verdict, "1 against 1, slot 1 defeated: slot 0 wins (mask 1)");
	Verdict = HOLMatchFlow::MatchEndVerdict(TwoStates, TwoTeams, 3, false, -1);
	T.AssertEquals(0, Verdict, "1 against 1, both defeated: over, nobody wins (mask 0, not 'goes on')");

	TArray<int> OneState = MatchFlowList(1, 1, 0, 0, 0);
	Verdict = HOLMatchFlow::MatchEndVerdict(OneState, TwoTeams, 0, false, -1);
	T.AssertEquals(NotOver, Verdict, "a camp alone: never wins by itself");
	Verdict = HOLMatchFlow::MatchEndVerdict(OneState, TwoTeams, 1, false, -1);
	T.AssertEquals(0, Verdict, "a camp alone, fallen: over, nobody wins");

	TArray<int> FourStates = MatchFlowList(4, 1, 2, 1, 2);
	TArray<int> FourTeams = MatchFlowList(4, 1, 1, 2, 2);
	Verdict = HOLMatchFlow::MatchEndVerdict(FourStates, FourTeams, 1, false, -1);
	T.AssertEquals(NotOver, Verdict, "teams 1+2: one member of a team fell, the team stands");
	Verdict = HOLMatchFlow::MatchEndVerdict(FourStates, FourTeams, 3, false, -1);
	T.AssertEquals(12, Verdict, "teams 1+2: team 1 fell, team 2 wins (slots 2 and 3, mask 12)");
	Verdict = HOLMatchFlow::MatchEndVerdict(FourStates, FourTeams, 12, false, -1);
	T.AssertEquals(3, Verdict, "teams 1+2: team 2 fell, team 1 wins (mask 3)");

	// The Diplomacy mode (the C++ gives every slot its own camp: Teams all 0): the survivors allied to each other win together.
	TArray<int> DipStates = MatchFlowList(3, 1, 2, 1, 0);
	TArray<int> DipTeams = MatchFlowList(3, 0, 0, 0, 0);
	Verdict = HOLMatchFlow::MatchEndVerdict(DipStates, DipTeams, 4, true, 3);
	T.AssertEquals(3, Verdict, "Diplomacy: slot 2 beaten, survivors 0 and 1 allied: they win together (mask 3)");
	Verdict = HOLMatchFlow::MatchEndVerdict(DipStates, DipTeams, 4, true, -1);
	T.AssertEquals(NotOver, Verdict, "Diplomacy: slot 2 beaten, a hostile survivor: goes on");
	Verdict = HOLMatchFlow::MatchEndVerdict(DipStates, DipTeams, 0, true, 3);
	T.AssertEquals(NotOver, Verdict, "Diplomacy: nobody beaten, an alliance alone ends nothing");
	Verdict = HOLMatchFlow::MatchEndVerdict(DipStates, DipTeams, 0, false, 3);
	T.AssertEquals(NotOver, Verdict, "standard mode: the alliance winners are never read");
	Verdict = HOLMatchFlow::MatchEndVerdict(DipStates, DipTeams, 5, true, 3);
	T.AssertEquals(2, Verdict, "Diplomacy: the camps rule first (slots 0 and 2 fell, slot 1 alone wins) before the alliance");
}

// Agreement (two copies of the gate while the C++ inline refusal stays): the 64 combinations against the former refusal, written as it was (a chain of ||).
void Test_MatchFlowDefeatGateAgreement(FUnitTest& T)
{
	int Wrong = 0;
	for (int Bits = 0; Bits < 64; ++Bits)
	{
		bool bAuthority = (Bits & 1) != 0;
		bool bMatchOver = (Bits & 2) != 0;
		bool bReasonNone = (Bits & 4) != 0;
		bool bSlotValid = (Bits & 8) != 0;
		bool bDefeated = (Bits & 16) != 0;
		bool bCampOccupied = (Bits & 32) != 0;
		bool bRefused = !bAuthority || bMatchOver || bReasonNone || !bSlotValid || bDefeated || !bCampOccupied;
		if (HOLMatchFlow::DefeatGateOpen(bAuthority, bMatchOver, bReasonNone, bSlotValid, bDefeated, bCampOccupied) == bRefused)
			Wrong += 1;
	}
	T.AssertEquals(0, Wrong, "the script gate answers as the C++ inline refusal on the 64 combinations");
}

// Agreement of the verdict with the C++ rule (HOLVictoryRules::Evaluate through HOLVictoryRead): every defeated mask, three configurations, both modes.
void MatchFlowVerdictSweep(FUnitTest& T, const TArray<int>& States, const TArray<int>& Teams, const FString& Label)
{
	int Wrong = 0;
	for (int Mask = 0; Mask < 256; ++Mask)
	{
		int RuleMask = 0;
		TArray<int> Winners;
		bool bOver = HOLVictoryRead::Evaluate(States, Teams, Mask, RuleMask, Winners);
		int Expected = bOver ? RuleMask : HOLMatchFlow::MatchNotOver();
		if (HOLMatchFlow::MatchEndVerdict(States, Teams, Mask, false, -1) != Expected)
			Wrong += 1;
		if (HOLMatchFlow::MatchEndVerdict(States, Teams, Mask, true, -1) != Expected)
			Wrong += 1;
		// With an alliance that ended: the rule still goes first; else the alliance winners, only with a beaten slot.
		int ExpectedAllied = Expected;
		if (!bOver && Mask != 0)
			ExpectedAllied = 5;
		if (HOLMatchFlow::MatchEndVerdict(States, Teams, Mask, true, 5) != ExpectedAllied)
			Wrong += 1;
	}
	T.AssertEquals(0, Wrong, Label + ": the script verdict equals the C++ rule on the 256 masks (both modes, with and without an ended alliance)");
}

void Test_MatchFlowMatchEndVerdictAgreement(FUnitTest& T)
{
	MatchFlowVerdictSweep(T, MatchFlowList(2, 1, 2, 0, 0), MatchFlowList(2, 0, 0, 0, 0), "1 against 1");
	MatchFlowVerdictSweep(T, MatchFlowList(4, 1, 2, 1, 2), MatchFlowList(4, 1, 1, 2, 2), "teams 1+2");
	MatchFlowVerdictSweep(T, MatchFlowList(4, 1, 2, 2, 1), MatchFlowList(4, 0, 0, 0, 0), "free for all of 4");
}