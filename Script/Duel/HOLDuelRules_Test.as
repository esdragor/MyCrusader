// The duel's state and outcome rules, alone (DuelInProgress.md D1, GDD 10.1, 10.4, 10.5, D16, D18, D35, D80): every
// expectation is read from the preset or from the duel returned, never typed in as a number.

const int DuelAlice = 0;
const int DuelBob = 1;

FHOLDuelSettings DuelSettings()
{
	return HOLDuel::Preset();
}

// A duel accepted at once: the Active state to play from.
FHOLDuel DuelActive(const FHOLDuelSettings& S)
{
	FHOLDuel Pending = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	return HOLDuel::Respond(S, Pending, true);
}

// The preset says every number the rules need; the GDD ranges bound them (5 to 8 s, 2 to 3 min, 20 to 30 s, 30 to 45 s).
void Test_DuelPresetInTheGddRanges(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	T.AssertTrue(S.ResponseMs >= 5000 && S.ResponseMs <= 8000, "response window 5 to 8 s (D80)");
	T.AssertTrue(S.ChallengeCooldownMs >= 120000 && S.ChallengeCooldownMs <= 180000, "challenge cooldown 2 to 3 min (D16)");
	T.AssertTrue(S.HesitationMs >= 20000 && S.HesitationMs <= 30000, "Hesitation 20 to 30 s (D16)");
	T.AssertTrue(S.TriumphMs >= 30000 && S.TriumphMs <= 45000, "Triumph / Demoralised 30 to 45 s (D35)");
	T.AssertTrue(S.MaxDurationMs >= 30000 && S.MaxDurationMs <= 90000, "longest duel within the 30 to 90 s aim (10.4)");
	T.AssertTrue(S.HesitationMs < S.TriumphMs, "Hesitation weaker than Demoralised (D16: shorter)");
}

// A challenge waits for the answer: Pending, the window full, nobody has won, the two slots remembered.
void Test_DuelChallengeWaitsForTheAnswer(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Pending, "pending");
	T.AssertEquals(DuelAlice, D.Challenger, "challenger");
	T.AssertEquals(DuelBob, D.Target, "target");
	T.AssertEquals(S.ResponseMs, D.WindowLeftMs, "the whole window is left");
	T.AssertEquals(-1, D.Winner, "no winner yet");
}

// Accepting starts the duel at once (D80) with the whole time to play.
void Test_DuelAcceptStartsAtOnce(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Active, "active");
	T.AssertEquals(S.MaxDurationMs, D.TimeLeftMs, "the whole time is left");
	T.AssertEquals(0, D.ElapsedMs, "nothing elapsed");
}

// Refusing is final (D80): the duel is over, nobody won, and the answer cannot be given twice.
void Test_DuelRefusalIsFinal(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel Pending = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	FHOLDuel D = HOLDuel::Respond(S, Pending, false);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Over, "over");
	T.AssertTrue(D.End == EHOLDuelEnd::Refused, "refused");
	FHOLDuel Again = HOLDuel::Respond(S, D, true);
	T.AssertTrue(Again.Phase == EHOLDuelPhase::Over, "no taking it back");
	T.AssertTrue(Again.End == EHOLDuelEnd::Refused, "still refused");
}

// Silence is a refusal (D80): the window runs down with the time, and at zero the duel is over, Ignored.
void Test_DuelSilenceIsARefusal(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	D = HOLDuel::Advance(S, D, S.ResponseMs - 1, 100, 100);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Pending, "one millisecond left: still pending");
	T.AssertEquals(1, D.WindowLeftMs, "one millisecond left");
	D = HOLDuel::Advance(S, D, 1, 100, 100);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Over, "window over");
	T.AssertTrue(D.End == EHOLDuelEnd::Ignored, "ignored");
}

// A refusal, spoken or silent, costs the refuser Hesitation (D16) and the Dawn Order its Honour; the challenger's
// cooldown is spent in any case (D80); nobody falls, nobody wins.
void Test_DuelRefusalCosts(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel Pending = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	FHOLDuel Silent = HOLDuel::Advance(S, Pending, S.ResponseMs, 100, 100);
	FHOLDuelEffects E = HOLDuel::Effects(S, Silent);
	T.AssertEquals(DuelBob, E.HesitationSlot, "the refuser hesitates");
	T.AssertEquals(S.HesitationMs, E.HesitationMs, "for the Hesitation time");
	T.AssertTrue(E.bRefusalCost, "faction cost of refusing (Honour for the Dawn)");
	T.AssertEquals(DuelAlice, E.ChallengeCooldownSlot, "the challenger's cooldown is spent");
	T.AssertEquals(S.ChallengeCooldownMs, E.ChallengeCooldownMs, "for the whole cooldown");
	T.AssertEquals(0, E.FallenSlots.Num(), "nobody falls");
	T.AssertEquals(-1, E.TriumphSlot, "no Triumph");
}

// An acceptance has no refusal cost.
void Test_DuelAcceptedHasNoRefusalCost(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	FHOLDuelEffects E = HOLDuel::Effects(S, D);
	T.AssertEquals(-1, E.HesitationSlot, "no Hesitation");
	T.AssertFalse(E.bRefusalCost, "no refusal cost");
}

// The time runs while the duel is active; when someone's health reaches zero the other one wins (10.5).
void Test_DuelWinnerWhenOneFalls(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	D = HOLDuel::Advance(S, D, 1000, 80, 100);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Active, "both stand: still going");
	T.AssertEquals(1000, D.ElapsedMs, "one second elapsed");
	T.AssertEquals(S.MaxDurationMs - 1000, D.TimeLeftMs, "the time left shrinks");
	D = HOLDuel::Advance(S, D, 50, 80, 0);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Over, "over");
	T.AssertTrue(D.End == EHOLDuelEnd::Won, "won");
	T.AssertEquals(DuelAlice, D.Winner, "the challenger wins when the target falls");
	T.AssertEquals(DuelBob, D.Loser, "the target lost");
	// And the other way round.
	FHOLDuel D2 = DuelActive(S);
	D2 = HOLDuel::Advance(S, D2, 50, 0, 40);
	T.AssertEquals(DuelBob, D2.Winner, "the target wins when the challenger falls");
	T.AssertEquals(DuelAlice, D2.Loser, "the challenger lost");
}

// The winner's rewards and the loser's fate (10.5, D35): Triumph for the winner, Demoralised for the loser, an XP bonus,
// the faction's effect, the loser falls; a won duel costs no Hesitation; the cooldown is spent.
void Test_DuelVictoryEffects(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	D = HOLDuel::Advance(S, D, 3000, 0, 40);
	FHOLDuelEffects E = HOLDuel::Effects(S, D);
	T.AssertEquals(DuelBob, E.TriumphSlot, "Triumph for the winner");
	T.AssertEquals(DuelAlice, E.DemoralisedSlot, "Demoralised for the loser");
	T.AssertEquals(S.TriumphMs, E.MoraleMs, "for the morale time");
	T.AssertTrue(E.bXpBonus, "XP bonus");
	T.AssertTrue(E.bFactionEffect, "the winner's faction effect");
	T.AssertEquals(1, E.FallenSlots.Num(), "one hero falls");
	if (E.FallenSlots.Num() == 1)
		T.AssertEquals(DuelAlice, E.FallenSlots[0], "the loser");
	T.AssertEquals(-1, E.HesitationSlot, "no Hesitation");
	T.AssertEquals(DuelAlice, E.ChallengeCooldownSlot, "the challenger's cooldown is spent");
}

// Time up with both standing: no winner, both live, no reward, the cooldown applies (D18, 10.4).
void Test_DuelTimeUpHasNoWinner(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	D = HOLDuel::Advance(S, D, S.MaxDurationMs - 1, 10, 10);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Active, "one millisecond left: still going");
	D = HOLDuel::Advance(S, D, 1, 10, 10);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Over, "over");
	T.AssertTrue(D.End == EHOLDuelEnd::TimedOut, "timed out");
	T.AssertEquals(-1, D.Winner, "no winner");
	FHOLDuelEffects E = HOLDuel::Effects(S, D);
	T.AssertEquals(0, E.FallenSlots.Num(), "both live");
	T.AssertEquals(-1, E.TriumphSlot, "no Triumph");
	T.AssertFalse(E.bXpBonus, "no XP bonus");
	T.AssertFalse(E.bFactionEffect, "no faction effect");
	T.AssertEquals(DuelAlice, E.ChallengeCooldownSlot, "the challenger's cooldown applies");
}

// A blow on the very last step still counts: health at zero beats the clock.
void Test_DuelBlowOnTheLastStepCounts(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	D = HOLDuel::Advance(S, D, S.MaxDurationMs, 10, 0);
	T.AssertTrue(D.End == EHOLDuelEnd::Won, "the fall wins over the time");
	T.AssertEquals(DuelAlice, D.Winner, "winner");
}

// Both fall in the same step (question DU-Q1, A by default): both fall, no winner, no reward; with B, nobody falls.
void Test_DuelDoubleKnockOut(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	D = HOLDuel::Advance(S, D, 40, 0, 0);
	T.AssertTrue(D.End == EHOLDuelEnd::DoubleKnockOut, "double knock-out");
	T.AssertEquals(-1, D.Winner, "no winner");
	FHOLDuelEffects A = HOLDuel::Effects(S, D);
	T.AssertEquals(2, A.FallenSlots.Num(), "A: both fall");
	T.AssertEquals(-1, A.TriumphSlot, "no Triumph");
	T.AssertFalse(A.bXpBonus, "no XP bonus");
	S.bDoubleKnockOutBothFall = false;
	FHOLDuelEffects B = HOLDuel::Effects(S, D);
	T.AssertEquals(0, B.FallenSlots.Num(), "B: nobody falls");
	T.AssertTrue(B.bLeaveAtOneHealth, "B: both left at one health");
}

// A hero who falls during the window (killed in the battle around) cancels the challenge: nobody refused, so no
// Hesitation; the cooldown stays spent (D80: the challenger cannot take it back).
void Test_DuelDeathInTheWindowCancels(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	D = HOLDuel::Advance(S, D, 100, 100, 0);
	T.AssertTrue(D.Phase == EHOLDuelPhase::Over, "over");
	T.AssertTrue(D.End == EHOLDuelEnd::Cancelled, "cancelled");
	FHOLDuelEffects E = HOLDuel::Effects(S, D);
	T.AssertEquals(-1, E.HesitationSlot, "no Hesitation: nobody refused");
	T.AssertFalse(E.bRefusalCost, "no refusal cost");
	T.AssertEquals(DuelAlice, E.ChallengeCooldownSlot, "the cooldown stays spent");
}

// An over duel does not move any more, whatever the time or the health says.
void Test_DuelOverStaysOver(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel D = DuelActive(S);
	D = HOLDuel::Advance(S, D, 40, 0, 50);
	FHOLDuel After = HOLDuel::Advance(S, D, 99999, 100, 0);
	T.AssertEquals(D.Winner, After.Winner, "the winner does not change");
	T.AssertEquals(D.ElapsedMs, After.ElapsedMs, "the clock stops");
	T.AssertTrue(After.End == EHOLDuelEnd::Won, "still won");
}
