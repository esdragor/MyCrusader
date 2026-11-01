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

// The three questions of the user (GDD D590 to D592, answered 2026-11-01, all A) fix the game's defaults: the numbers are typed
// in on purpose here, a change of one of them is a change of a decision.
void Test_DuelDecidedDefaults(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	T.AssertTrue(S.LostConditions == EHOLDuelLostConditions::DropsFree, "DU-Q2 = A: a challenge whose conditions are lost falls free (D591)");
	T.AssertEquals(2500, S.ForumZoneRadiusCm, "DU-Q3 = A: 25 m around a main forum (D592)");
	T.AssertTrue(S.bDoubleKnockOutBothFall, "DU-Q1 = A: two simultaneous falls, both heroes fall (D590)");
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

// Both fall in the same step (DU-Q1 = A, GDD D590): both fall, no winner, no reward; with B, nobody falls.
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

// ---- D2: the conditions of a challenge (GDD 10.1, 10.2, D16, D18, D80, D403) ----

FHOLDuelHero DuelReadyHero()
{
	FHOLDuelHero H;
	return H;
}

FHOLDuelPlace DuelFairPlace()
{
	FHOLDuelPlace P;
	return P;
}

// Two heroes alive, free, enemies, near, in sight, away from every main forum: the challenge goes (empty reason).
void Test_DuelChallengeAllowed(FUnitTest& T)
{
	T.AssertTrue(HOLDuel::CanChallenge(DuelReadyHero(), DuelReadyHero(), DuelFairPlace()).IsEmpty(), "allowed");
}

// Each condition alone refuses, with its own reason.
void Test_DuelChallengeRefusals(FUnitTest& T)
{
	FHOLDuelHero Ok = DuelReadyHero();
	FHOLDuelPlace Fair = DuelFairPlace();

	FHOLDuelPlace Allies = Fair;
	Allies.bEnemies = false;
	T.AssertEquals("not_enemy", HOLDuel::CanChallenge(Ok, Ok, Allies), "only an adverse hero");

	FHOLDuelHero Dead = Ok;
	Dead.bAlive = false;
	T.AssertEquals("challenger_unavailable", HOLDuel::CanChallenge(Dead, Ok, Fair), "dead challenger");
	T.AssertEquals("target_unavailable", HOLDuel::CanChallenge(Ok, Dead, Fair), "dead target");

	FHOLDuelHero Recovering = Ok;
	Recovering.bRecovering = true;
	T.AssertEquals("challenger_unavailable", HOLDuel::CanChallenge(Recovering, Ok, Fair), "challenger in recovery");
	T.AssertEquals("target_unavailable", HOLDuel::CanChallenge(Ok, Recovering, Fair), "target in recovery");

	FHOLDuelHero Tired = Ok;
	Tired.ChallengeCooldownLeftMs = 1;
	T.AssertEquals("cooldown", HOLDuel::CanChallenge(Tired, Ok, Fair), "challenge cooldown not over (D16)");
	// The target's own cooldown does not matter: it only limits the challenges one makes.
	T.AssertTrue(HOLDuel::CanChallenge(Ok, Tired, Fair).IsEmpty(), "the target's cooldown is not in the way");

	FHOLDuelHero Busy = Ok;
	Busy.bInDuel = true;
	T.AssertEquals("challenger_in_duel", HOLDuel::CanChallenge(Busy, Ok, Fair), "challenger already in a duel");
	T.AssertEquals("target_in_duel", HOLDuel::CanChallenge(Ok, Busy, Fair), "target already in a duel (no 2 against 1)");

	FHOLDuelPlace Far = Fair;
	Far.bNear = false;
	T.AssertEquals("too_far", HOLDuel::CanChallenge(Ok, Ok, Far), "too far");

	FHOLDuelPlace Hidden = Fair;
	Hidden.bSeeEachOther = false;
	T.AssertEquals("not_visible", HOLDuel::CanChallenge(Ok, Ok, Hidden), "not in sight of each other");

	FHOLDuelPlace Forum = Fair;
	Forum.bInMainForumRadius = true;
	T.AssertEquals("forum_zone", HOLDuel::CanChallenge(Ok, Ok, Forum), "inside a main forum's radius (D403)");

	FHOLDuelPlace Blocked = Fair;
	Blocked.bBlockedByEffect = true;
	T.AssertEquals("blocked", HOLDuel::CanChallenge(Ok, Ok, Blocked), "an effect forbids duels");
}

// A challenge spends the cooldown: with the effects of the pending duel, the challenger cannot challenge again until
// it is over; this is what the match writes into FHOLDuelHero.ChallengeCooldownLeftMs.
void Test_DuelCooldownBlocksTheNextChallenge(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel Pending = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	FHOLDuelEffects E = HOLDuel::Effects(S, Pending);
	FHOLDuelHero Alice = DuelReadyHero();
	Alice.ChallengeCooldownLeftMs = E.ChallengeCooldownMs;
	T.AssertEquals("cooldown", HOLDuel::CanChallenge(Alice, DuelReadyHero(), DuelFairPlace()), "cooldown spent by the challenge");
	Alice.ChallengeCooldownLeftMs = Math::Max(0, Alice.ChallengeCooldownLeftMs - S.ChallengeCooldownMs);
	T.AssertTrue(HOLDuel::CanChallenge(Alice, DuelReadyHero(), DuelFairPlace()).IsEmpty(), "challenge again once it is over");
}

// ---- D3: the protected duel (GDD 10.2, D18) ----

// A pending challenge protects nobody; an active duel does; an over duel no longer.
void Test_DuelInDuelOnlyWhenPendingOrActive(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel Idle;
	T.AssertFalse(HOLDuel::InDuel(Idle, DuelAlice), "idle: nobody");
	FHOLDuel Pending = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	T.AssertTrue(HOLDuel::InDuel(Pending, DuelAlice), "pending: the challenger is committed (one duel at a time)");
	T.AssertTrue(HOLDuel::InDuel(Pending, DuelBob), "pending: the target too");
	T.AssertFalse(HOLDuel::InDuel(Pending, 2), "a third hero is not");
	FHOLDuel Active = HOLDuel::Respond(S, Pending, true);
	T.AssertTrue(HOLDuel::InDuel(Active, DuelBob), "active: yes");
	FHOLDuel Over = HOLDuel::Respond(S, Pending, false);
	T.AssertFalse(HOLDuel::InDuel(Over, DuelAlice), "over: no longer");
}

// Only the two duellists can hit each other; nothing else can touch them, and they can touch nothing else.
// Attacker / victim are hero slots, -1 for what is not a hero (a unit, a tower, a power, a world event).
void Test_DuelProtectedHits(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel Active = DuelActive(S);
	const int Third = 2;
	T.AssertTrue(HOLDuel::CanHit(Active, DuelAlice, DuelBob), "challenger hits target");
	T.AssertTrue(HOLDuel::CanHit(Active, DuelBob, DuelAlice), "target hits challenger");
	T.AssertFalse(HOLDuel::CanHit(Active, -1, DuelBob), "a unit, tower, power or event cannot touch a duellist");
	T.AssertFalse(HOLDuel::CanHit(Active, -1, DuelAlice), "same for the challenger");
	T.AssertFalse(HOLDuel::CanHit(Active, Third, DuelBob), "a third hero cannot touch a duellist (no 2 against 1)");
	T.AssertFalse(HOLDuel::CanHit(Active, DuelAlice, -1), "a duellist cannot hit a unit meanwhile");
	T.AssertFalse(HOLDuel::CanHit(Active, DuelBob, Third), "nor a third hero");
	T.AssertTrue(HOLDuel::CanHit(Active, Third, -1), "the battle goes on around: a third hero hits units");
	T.AssertTrue(HOLDuel::CanHit(Active, -1, Third), "a unit hits a third hero");
	T.AssertTrue(HOLDuel::CanHit(Active, Third, 3), "two other heroes fight on");
}

// Before the answer and after the end nothing is protected (the protection is the duel's, D18).
void Test_DuelNoProtectionOutsideTheDuel(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuel Pending = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	T.AssertTrue(HOLDuel::CanHit(Pending, -1, DuelBob), "pending: the target can still be hit");
	T.AssertTrue(HOLDuel::CanHit(Pending, DuelAlice, -1), "pending: the challenger can still fight");
	FHOLDuel Over = HOLDuel::Respond(S, Pending, false);
	T.AssertTrue(HOLDuel::CanHit(Over, -1, DuelBob), "over: hittable again");
	FHOLDuel Idle;
	T.AssertTrue(HOLDuel::CanHit(Idle, -1, DuelBob), "idle: hittable");
}
// ---- D3b: the match's book of duels (several duels at once, one per hero, cooldowns, the ones that end) ----

// Health of the four slots, all standing, as the match would hand it over.
TArray<int> DuelHealths(int A, int B, int C, int D)
{
	TArray<int> H;
	H.Add(A);
	H.Add(B);
	H.Add(C);
	H.Add(D);
	return H;
}

// Starting a duel through the book: the book holds the pending duel, the cooldown is spent, the two heroes are committed.
void Test_DuelBookStartSpendsTheCooldown(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	T.AssertEquals(4, Book.CooldownLeftMs.Num(), "one cooldown per slot");
	FString Reason = HOLDuel::Start(S, Book, 0, 1, DuelReadyHero(), DuelReadyHero(), DuelFairPlace());
	T.AssertTrue(Reason.IsEmpty(), "started");
	T.AssertEquals(1, Book.Duels.Num(), "one duel in the book");
	T.AssertEquals(S.ChallengeCooldownMs, Book.CooldownLeftMs[0], "the challenger's cooldown is spent");
	T.AssertEquals(0, Book.CooldownLeftMs[1], "the target's is not");
	T.AssertTrue(HOLDuel::BookInDuel(Book, 0), "challenger committed");
	T.AssertTrue(HOLDuel::BookInDuel(Book, 1), "target committed");
	T.AssertFalse(HOLDuel::BookInDuel(Book, 2), "a third hero is free");
}

// The book fills the hero's duel state and cooldown itself: whatever the caller says, a committed hero or a hero in
// cooldown is refused (the book is the one truth).
void Test_DuelBookRefusesWhatItKnows(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	HOLDuel::Start(S, Book, 0, 1, DuelReadyHero(), DuelReadyHero(), DuelFairPlace());
	T.AssertEquals("cooldown", HOLDuel::Start(S, Book, 0, 2, DuelReadyHero(), DuelReadyHero(), DuelFairPlace()), "same challenger again");
	T.AssertEquals("target_in_duel", HOLDuel::Start(S, Book, 2, 1, DuelReadyHero(), DuelReadyHero(), DuelFairPlace()), "a third hero cannot challenge a duellist");
	T.AssertEquals("challenger_in_duel", HOLDuel::Start(S, Book, 1, 2, DuelReadyHero(), DuelReadyHero(), DuelFairPlace()), "a duellist cannot challenge a third hero");
	T.AssertEquals(1, Book.Duels.Num(), "nothing was added by the refusals");
	// Another pair can duel meanwhile.
	T.AssertTrue(HOLDuel::Start(S, Book, 2, 3, DuelReadyHero(), DuelReadyHero(), DuelFairPlace()).IsEmpty(), "a second duel elsewhere");
	T.AssertEquals(2, Book.Duels.Num(), "two duels at once");
	// The place's refusals pass through.
	FHOLDuelBook Other = HOLDuel::NewBook(4);
	FHOLDuelPlace Forum = DuelFairPlace();
	Forum.bInMainForumRadius = true;
	T.AssertEquals("forum_zone", HOLDuel::Start(S, Other, 0, 1, DuelReadyHero(), DuelReadyHero(), Forum), "forum zone");
	T.AssertEquals(0, Other.Duels.Num(), "no duel, and no cooldown spent by a refused challenge");
	T.AssertEquals(0, Other.CooldownLeftMs[0], "cooldown untouched");
}

// The answer is the target's, to a pending challenge: accepting makes the duel active; a wrong slot or a second answer does nothing.
void Test_DuelBookAnswer(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	HOLDuel::Start(S, Book, 0, 1, DuelReadyHero(), DuelReadyHero(), DuelFairPlace());
	T.AssertFalse(HOLDuel::Answer(S, Book, 0, true), "the challenger cannot answer its own challenge");
	T.AssertFalse(HOLDuel::Answer(S, Book, 2, true), "a stranger cannot");
	T.AssertTrue(HOLDuel::Answer(S, Book, 1, true), "the target accepts");
	T.AssertTrue(Book.Duels[0].Phase == EHOLDuelPhase::Active, "active");
	T.AssertFalse(HOLDuel::Answer(S, Book, 1, false), "no taking it back (D80)");
	T.AssertTrue(Book.Duels[0].Phase == EHOLDuelPhase::Active, "still active");
}

// Tick: cooldowns run down, a duel that ends leaves the book once with its effects, the heroes are free again.
void Test_DuelBookTickEndsDuels(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	HOLDuel::Start(S, Book, 0, 1, DuelReadyHero(), DuelReadyHero(), DuelFairPlace());
	HOLDuel::Answer(S, Book, 1, true);
	TArray<FHOLDuelEnded> Ended = HOLDuel::Tick(S, Book, 1000, DuelHealths(50, 50, 50, 50));
	T.AssertEquals(0, Ended.Num(), "nobody ended");
	T.AssertEquals(S.ChallengeCooldownMs - 1000, Book.CooldownLeftMs[0], "the cooldown runs down");
	Ended = HOLDuel::Tick(S, Book, 50, DuelHealths(50, 0, 50, 50));
	T.AssertEquals(1, Ended.Num(), "one duel ended");
	if (Ended.Num() == 1)
	{
		T.AssertTrue(Ended[0].Duel.End == EHOLDuelEnd::Won, "won");
		T.AssertEquals(0, Ended[0].Duel.Winner, "by the challenger");
		T.AssertEquals(1, Ended[0].Effects.FallenSlots.Num(), "one hero falls");
	}
	T.AssertEquals(0, Book.Duels.Num(), "the book let it go");
	T.AssertFalse(HOLDuel::BookInDuel(Book, 0), "the heroes are free");
	TArray<FHOLDuelEnded> Again = HOLDuel::Tick(S, Book, 50, DuelHealths(50, 0, 50, 50));
	T.AssertEquals(0, Again.Num(), "an ended duel is reported once");
	T.AssertTrue(Book.CooldownLeftMs[0] > 0, "the cooldown still runs after the duel (D16)");
}

// A silent target: the window closes inside Tick and the effects name the refuser.
void Test_DuelBookSilenceInTick(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	HOLDuel::Start(S, Book, 2, 3, DuelReadyHero(), DuelReadyHero(), DuelFairPlace());
	TArray<FHOLDuelEnded> Ended = HOLDuel::Tick(S, Book, S.ResponseMs, DuelHealths(50, 50, 50, 50));
	T.AssertEquals(1, Ended.Num(), "ended");
	if (Ended.Num() == 1)
	{
		T.AssertTrue(Ended[0].Duel.End == EHOLDuelEnd::Ignored, "ignored");
		T.AssertEquals(3, Ended[0].Effects.HesitationSlot, "the silent one hesitates");
	}
}

// The book's protection: with a duel on, its two heroes touch each other only; with another duel on elsewhere, the
// four heroes are paired; a pending challenge protects nobody.
void Test_DuelBookProtectedHits(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettings();
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	HOLDuel::Start(S, Book, 0, 1, DuelReadyHero(), DuelReadyHero(), DuelFairPlace());
	T.AssertTrue(HOLDuel::BookCanHit(Book, -1, 1), "pending: no protection yet");
	HOLDuel::Answer(S, Book, 1, true);
	T.AssertFalse(HOLDuel::BookCanHit(Book, -1, 1), "active: a unit cannot hit a duellist");
	T.AssertTrue(HOLDuel::BookCanHit(Book, 0, 1), "the duellists hit each other");
	T.AssertFalse(HOLDuel::BookCanHit(Book, 2, 1), "a third hero cannot");
	HOLDuel::Start(S, Book, 2, 3, DuelReadyHero(), DuelReadyHero(), DuelFairPlace());
	HOLDuel::Answer(S, Book, 3, true);
	T.AssertFalse(HOLDuel::BookCanHit(Book, 2, 1), "heroes of two different duels cannot touch each other");
	T.AssertTrue(HOLDuel::BookCanHit(Book, 2, 3), "each pair hits its own");
	T.AssertTrue(HOLDuel::BookCanHit(Book, -1, -1), "the rest of the battle goes on");
}

// ---- DU-Q2: what a challenge becomes when its conditions cease to hold before the answer (three ways, one setting) ----

FHOLDuelSettings DuelSettingsLost(EHOLDuelLostConditions Mode)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	S.LostConditions = Mode;
	return S;
}

// A: the conditions lost, the challenge falls at once, free for the target; the challenger's cooldown stays spent.
void Test_DuelLostConditionsDropsFree(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettingsLost(EHOLDuelLostConditions::DropsFree);
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	T.AssertTrue(HOLDuel::Recheck(S, D, true).Phase == EHOLDuelPhase::Pending, "the conditions hold: the challenge waits");
	FHOLDuel Fallen = HOLDuel::Recheck(S, D, false);
	T.AssertTrue(Fallen.Phase == EHOLDuelPhase::Over, "the conditions lost: over");
	T.AssertTrue(Fallen.End == EHOLDuelEnd::Dropped, "dropped");
	FHOLDuelEffects E = HOLDuel::Effects(S, Fallen);
	T.AssertEquals(-1, E.HesitationSlot, "no Hesitation for the target");
	T.AssertFalse(E.bRefusalCost, "no Honour lost");
	T.AssertEquals(DuelAlice, E.ChallengeCooldownSlot, "the challenger's cooldown stays spent");
	T.AssertEquals(S.ChallengeCooldownMs, E.ChallengeCooldownMs, "the whole cooldown");
	T.AssertTrue(HOLDuel::Respond(S, Fallen, true).Phase == EHOLDuelPhase::Over, "no accepting a fallen challenge");
	// An answer given at the instant the conditions are false falls the same way, whatever the answer.
	T.AssertTrue(HOLDuel::RespondChecked(S, D, true, false).End == EHOLDuelEnd::Dropped, "accepted with the conditions lost: dropped");
	T.AssertTrue(HOLDuel::RespondChecked(S, D, false, false).End == EHOLDuelEnd::Dropped, "refused with the conditions lost: dropped, free");
	// With the conditions true, it is the plain answer.
	T.AssertTrue(HOLDuel::RespondChecked(S, D, true, true).Phase == EHOLDuelPhase::Active, "accepted with the conditions true: the duel begins");
	T.AssertTrue(HOLDuel::RespondChecked(S, D, false, true).End == EHOLDuelEnd::Refused, "refused with the conditions true: refused");
}

// Only a challenge waiting can fall: an Active duel is not touched by the conditions (the arena is set at the acceptance).
void Test_DuelLostConditionsOnlyPendingFalls(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettingsLost(EHOLDuelLostConditions::DropsFree);
	FHOLDuel Active = DuelActive(S);
	T.AssertTrue(HOLDuel::Recheck(S, Active, false).Phase == EHOLDuelPhase::Active, "an active duel stays active");
	FHOLDuel Over = HOLDuel::Respond(S, HOLDuel::Challenge(S, DuelAlice, DuelBob), false);
	T.AssertTrue(HOLDuel::Recheck(S, Over, false).End == EHOLDuelEnd::Refused, "an over duel is unchanged");
}

// B: the challenge stands whatever the conditions; the duel begins at the acceptance.
void Test_DuelLostConditionsHolds(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettingsLost(EHOLDuelLostConditions::Holds);
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	T.AssertTrue(HOLDuel::Recheck(S, D, false).Phase == EHOLDuelPhase::Pending, "the challenge stands");
	T.AssertTrue(HOLDuel::RespondChecked(S, D, true, false).Phase == EHOLDuelPhase::Active, "accepted from shelter: the duel begins");
	FHOLDuel Refused = HOLDuel::RespondChecked(S, D, false, false);
	T.AssertTrue(Refused.End == EHOLDuelEnd::Refused, "refused from shelter: a plain refusal");
	T.AssertTrue(HOLDuel::Effects(S, Refused).bRefusalCost, "with its costs");
}

// C: the challenge stands; an acceptance with a false condition is counted as a refusal, with its costs.
void Test_DuelLostConditionsCountsAsRefusal(FUnitTest& T)
{
	FHOLDuelSettings S = DuelSettingsLost(EHOLDuelLostConditions::CountsAsRefusal);
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	T.AssertTrue(HOLDuel::Recheck(S, D, false).Phase == EHOLDuelPhase::Pending, "the challenge stands until the answer");
	FHOLDuel Counted = HOLDuel::RespondChecked(S, D, true, false);
	T.AssertTrue(Counted.Phase == EHOLDuelPhase::Over, "accepted from shelter: no duel");
	T.AssertTrue(Counted.End == EHOLDuelEnd::Refused, "counted as a refusal");
	FHOLDuelEffects E = HOLDuel::Effects(S, Counted);
	T.AssertEquals(DuelBob, E.HesitationSlot, "Hesitation on the target");
	T.AssertTrue(E.bRefusalCost, "with the faction's price");
	T.AssertTrue(HOLDuel::RespondChecked(S, D, true, true).Phase == EHOLDuelPhase::Active, "accepted with the conditions true: the duel begins");
}

// In the book: the same three ways, by the target's slot; a fallen challenge leaves at the next Tick with its effects, once.
void Test_DuelBookLostConditions(FUnitTest& T)
{
	FHOLDuelHero Hero;
	FHOLDuelPlace Place;
	TArray<int> Health;
	Health.Add(100);
	Health.Add(100);
	// A: Recheck drops it; the Tick reports it with no refusal cost and the cooldown spent.
	FHOLDuelSettings A = DuelSettingsLost(EHOLDuelLostConditions::DropsFree);
	FHOLDuelBook Book = HOLDuel::NewBook(2);
	T.AssertEquals("", HOLDuel::Start(A, Book, DuelAlice, DuelBob, Hero, Hero, Place), "the challenge starts");
	T.AssertFalse(HOLDuel::BookRecheck(A, Book, DuelAlice, false), "nothing is addressed to the challenger");
	T.AssertTrue(HOLDuel::BookRecheck(A, Book, DuelBob, false), "the conditions lost: the challenge fell");
	TArray<FHOLDuelEnded> Ended = HOLDuel::Tick(A, Book, 50, Health);
	T.AssertEquals(1, Ended.Num(), "reported once");
	if (Ended.Num() == 1)
	{
		T.AssertTrue(Ended[0].Duel.End == EHOLDuelEnd::Dropped, "dropped");
		T.AssertFalse(Ended[0].Effects.bRefusalCost, "free");
	}
	T.AssertEquals(0, HOLDuel::Tick(A, Book, 50, Health).Num(), "not reported again");
	T.AssertFalse(HOLDuel::BookInDuel(Book, DuelBob), "nobody is in a duel any more");
	// C: the answer with a false condition is a refusal with costs.
	FHOLDuelSettings C = DuelSettingsLost(EHOLDuelLostConditions::CountsAsRefusal);
	FHOLDuelBook BookC = HOLDuel::NewBook(2);
	HOLDuel::Start(C, BookC, DuelAlice, DuelBob, Hero, Hero, Place);
	T.AssertFalse(HOLDuel::BookRecheck(C, BookC, DuelBob, false), "C: the challenge stands");
	T.AssertTrue(HOLDuel::AnswerChecked(C, BookC, DuelBob, true, false), "the answer is taken");
	TArray<FHOLDuelEnded> EndedC = HOLDuel::Tick(C, BookC, 50, Health);
	T.AssertEquals(1, EndedC.Num(), "C: reported once");
	if (EndedC.Num() == 1)
		T.AssertTrue(EndedC[0].Effects.bRefusalCost, "C: with its costs");
	// B: the answer from shelter starts the duel.
	FHOLDuelSettings B = DuelSettingsLost(EHOLDuelLostConditions::Holds);
	FHOLDuelBook BookB = HOLDuel::NewBook(2);
	HOLDuel::Start(B, BookB, DuelAlice, DuelBob, Hero, Hero, Place);
	T.AssertTrue(HOLDuel::AnswerChecked(B, BookB, DuelBob, true, false), "B: taken");
	T.AssertTrue(HOLDuel::BookCanHit(BookB, DuelAlice, DuelBob), "B: the duel is on");
	T.AssertFalse(HOLDuel::BookCanHit(BookB, DuelAlice, 5), "B: protected");
}

// ---- D120: contact and hold in the circle ----

// The adversary of a duellist, only while the duel is on.
void Test_DuelAdversary(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel Pending = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	T.AssertEquals(-1, HOLDuel::AdversaryOf(Pending, DuelAlice), "pending: no contact yet");
	FHOLDuel Active = HOLDuel::Respond(S, Pending, true);
	T.AssertEquals(DuelBob, HOLDuel::AdversaryOf(Active, DuelAlice), "Alice fights Bob");
	T.AssertEquals(DuelAlice, HOLDuel::AdversaryOf(Active, DuelBob), "Bob fights Alice");
	T.AssertEquals(-1, HOLDuel::AdversaryOf(Active, 5), "a third hero fights nobody");
	T.AssertEquals(-1, HOLDuel::AdversaryOf(HOLDuel::Advance(S, Active, S.MaxDurationMs, 100, 100), DuelAlice), "over: no contact");
}

// A point outside is brought back to the nearest point of the circle; a point inside or on the edge is left alone.
void Test_DuelClampToArena(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuelArena Arena = HOLDuel::MakeArena(S, -100, 0, 100, 0);
	FHOLDuelPoint Inside = HOLDuel::ClampToArena(Arena, 50, 20);
	T.AssertEquals(50, Inside.X, "inside: X unchanged");
	T.AssertEquals(20, Inside.Y, "inside: Y unchanged");
	FHOLDuelPoint Edge = HOLDuel::ClampToArena(Arena, Arena.CenterX + Arena.RadiusCm, Arena.CenterY);
	T.AssertEquals(Arena.CenterX + Arena.RadiusCm, Edge.X, "on the edge: unchanged");
	FHOLDuelPoint Far = HOLDuel::ClampToArena(Arena, Arena.CenterX + 10 * Arena.RadiusCm, Arena.CenterY);
	T.AssertEquals(Arena.CenterX + Arena.RadiusCm, Far.X, "far on the right: brought to the edge");
	T.AssertEquals(Arena.CenterY, Far.Y, "same line");
	FHOLDuelPoint Diagonal = HOLDuel::ClampToArena(Arena, Arena.CenterX + 5 * Arena.RadiusCm, Arena.CenterY + 5 * Arena.RadiusCm);
	T.AssertTrue(HOLDuel::InArena(Arena, Diagonal.X, Diagonal.Y) || HOLDuel::InArena(Arena, Diagonal.X - 1, Diagonal.Y - 1), "diagonal: back at the circle (rounding aside)");
	T.AssertTrue(Diagonal.X > Arena.CenterX && Diagonal.Y > Arena.CenterY, "diagonal: the same side");
	T.AssertEquals(Diagonal.X - Arena.CenterX, Diagonal.Y - Arena.CenterY, "diagonal: the same direction");
}

// The book keeps the circle of each duel on, by the duellists' slots.
void Test_DuelBookArena(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuelHero Hero;
	FHOLDuelPlace Place;
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	HOLDuel::Start(S, Book, DuelAlice, DuelBob, Hero, Hero, Place);
	FHOLDuelArena Arena = HOLDuel::MakeArena(S, 0, 0, 200, 0);
	T.AssertFalse(HOLDuel::BookSetArena(Book, DuelAlice, DuelBob, Arena), "a challenge waiting has no circle yet");
	T.AssertTrue(HOLDuel::Answer(S, Book, DuelBob, true), "accepted");
	T.AssertTrue(HOLDuel::BookSetArena(Book, DuelAlice, DuelBob, Arena), "the circle is set at the acceptance");
	FHOLDuelArena Got;
	T.AssertTrue(HOLDuel::BookArenaOf(Book, DuelBob, Got), "Bob finds it");
	T.AssertEquals(Arena.CenterX, Got.CenterX, "same centre X");
	T.AssertEquals(Arena.RadiusCm, Got.RadiusCm, "same radius");
	T.AssertTrue(HOLDuel::BookArenaOf(Book, DuelAlice, Got), "Alice finds it");
	T.AssertFalse(HOLDuel::BookArenaOf(Book, 2, Got), "a third slot has none");
	T.AssertEquals(DuelBob, HOLDuel::BookAdversaryOf(Book, DuelAlice), "the book's adversary");
	T.AssertEquals(-1, HOLDuel::BookAdversaryOf(Book, 3), "none for a bystander");
}
// The challenge window's view, alone (DuelInProgress.md D8; GDD D117): every number is read from the preset or the duel.

// The target sees the window that asks; the challenger the indicator; a third hero nothing; nobody once it is answered.
void Test_DuelWindowWhoSeesWhat(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	FHOLDuelWindowView Target = HOLDuel::WindowView(S, D, DuelBob);
	T.AssertTrue(Target.Kind == EHOLDuelWindowKind::Incoming, "the target gets the window");
	T.AssertEquals(DuelAlice, Target.OpponentSlot, "it shows the challenger");
	T.AssertTrue(Target.bCanAnswer, "with live buttons");
	FHOLDuelWindowView Challenger = HOLDuel::WindowView(S, D, DuelAlice);
	T.AssertTrue(Challenger.Kind == EHOLDuelWindowKind::Sent, "the challenger gets the indicator");
	T.AssertEquals(DuelBob, Challenger.OpponentSlot, "it shows the target");
	T.AssertFalse(Challenger.bCanAnswer, "no button: the challenger cannot cancel (D80)");
	T.AssertTrue(HOLDuel::WindowView(S, D, 5).Kind == EHOLDuelWindowKind::None, "a third hero sees nothing");
	T.AssertTrue(HOLDuel::WindowView(S, HOLDuel::Respond(S, D, true), DuelBob).Kind == EHOLDuelWindowKind::None, "answered: closed");
	T.AssertTrue(HOLDuel::WindowView(S, HOLDuel::Respond(S, D, false), DuelBob).Kind == EHOLDuelWindowKind::None, "refused: closed");
}

// The countdown: the whole window at the start, seconds rounded up, the bar emptying to zero, urgent at the end.
void Test_DuelWindowCountdown(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel D = HOLDuel::Challenge(S, DuelAlice, DuelBob);
	FHOLDuelWindowView Start = HOLDuel::WindowView(S, D, DuelBob);
	T.AssertEquals(1000, Start.FillPermille, "full bar at the start");
	T.AssertEquals(Math::IntegerDivisionTrunc(S.ResponseMs + 999, 1000), Start.SecondsLeft, "seconds left, rounded up");
	T.AssertFalse(Start.bUrgent, "not urgent at the start");
	FHOLDuel Half = HOLDuel::Advance(S, D, S.ResponseMs / 2, 100, 100);
	FHOLDuelWindowView Mid = HOLDuel::WindowView(S, Half, DuelBob);
	T.AssertTrue(Mid.FillPermille < Start.FillPermille && Mid.FillPermille > 0, "the bar empties");
	T.AssertTrue(Mid.SecondsLeft < Start.SecondsLeft, "the seconds go down");
	FHOLDuel Last = HOLDuel::Advance(S, D, S.ResponseMs - 1, 100, 100);
	FHOLDuelWindowView End = HOLDuel::WindowView(S, Last, DuelBob);
	T.AssertEquals(1, End.SecondsLeft, "the last second reads 1 until the window ends");
	T.AssertTrue(End.bUrgent, "the last instants are urgent");
	T.AssertTrue(End.FillPermille >= 0 && End.FillPermille < Mid.FillPermille, "the bar is nearly empty");
	T.AssertTrue(HOLDuel::WindowView(S, HOLDuel::Advance(S, D, S.ResponseMs, 100, 100), DuelBob).Kind == EHOLDuelWindowKind::None, "the window ran out: closed (silence is a refusal)");
	// The sent indicator counts down the same.
	T.AssertEquals(Mid.SecondsLeft, HOLDuel::WindowView(S, Half, DuelAlice).SecondsLeft, "the challenger sees the same countdown");
}

// In the book: the window of the viewer's own challenge; none once the duel is on.
void Test_DuelBookWindow(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuelHero Hero;
	FHOLDuelPlace Place;
	FHOLDuelBook Book = HOLDuel::NewBook(4);
	T.AssertTrue(HOLDuel::BookWindowView(S, Book, DuelBob).Kind == EHOLDuelWindowKind::None, "an empty book: nothing");
	HOLDuel::Start(S, Book, DuelAlice, DuelBob, Hero, Hero, Place);
	T.AssertTrue(HOLDuel::BookWindowView(S, Book, DuelBob).Kind == EHOLDuelWindowKind::Incoming, "the target's window");
	T.AssertTrue(HOLDuel::BookWindowView(S, Book, DuelAlice).Kind == EHOLDuelWindowKind::Sent, "the challenger's indicator");
	T.AssertTrue(HOLDuel::BookWindowView(S, Book, 3).Kind == EHOLDuelWindowKind::None, "nothing for a bystander");
	HOLDuel::Answer(S, Book, DuelBob, true);
	T.AssertTrue(HOLDuel::BookWindowView(S, Book, DuelBob).Kind == EHOLDuelWindowKind::None, "the duel is on: the window is closed");
}