// The hero duel's state and outcome, pure rules (DuelInProgress.md D1; GDD 10.1, 10.4, 10.5, D16, D18, D35, D80), tested
// alone in HOLDuelRules_Test.as. No world access: the match tells it the elapsed time and the two heroes' health, it
// answers the duel's state and what the end changes (HOLDuel::Effects). Whole numbers only (milliseconds), so that the
// server and the clients agree to the last step. Slots by index as everywhere in script.
//
// One duel: Idle -> Pending (a challenge waits for the answer) -> Active (accepted) -> Over; or Pending -> Over (refused,
// ignored, or a hero fell during the window). An Over duel never moves again.

enum EHOLDuelPhase
{
	Idle,
	Pending,
	Active,
	Over
}

// How a duel ended (None while it has not).
enum EHOLDuelEnd
{
	None,
	// The target said no (D80: final).
	Refused,
	// The target let the window run out: silence is a refusal (D80).
	Ignored,
	// One hero fell, the other one won (10.5).
	Won,
	// Time up with both standing: no winner (D18, 10.4).
	TimedOut,
	// Both fell in the same step (DU-Q1).
	DoubleKnockOut,
	// A hero fell during the response window: nobody refused, nobody fought.
	Cancelled,
	// The conditions of the challenge ceased to hold before the answer (DU-Q2 = A): the challenge falls, free for the target.
	Dropped
}

// DU-Q2, the three ways ready until the user answers: what a challenge becomes when its conditions (near, in sight, outside
// the forum zone) cease to hold before the answer.
enum EHOLDuelLostConditions
{
	// A: the challenge falls without cost for the target (no Hesitation, no Honour); the challenger's cooldown stays spent.
	DropsFree,
	// B: the challenge stands, the duel begins where the heroes are when accepted.
	Holds,
	// C: the challenge stands, but an acceptance with a false condition counts as a refusal, with its costs.
	CountsAsRefusal
}

// The numbers of the duel, one place. The GDD gives ranges marked "indicative, to be tuned in tests"; the preset takes the
// middle of each range (the longest duel: the top of the 30 to 90 s aim, 10.4).
struct FHOLDuelSettings
{
	// The target's window to answer a challenge (D80: 5 to 8 s).
	int ResponseMs = 0;

	// The challenger's cooldown, spent by the challenge itself (D16, D80: 2 to 3 min).
	int ChallengeCooldownMs = 0;

	// The longest duel; at zero with both standing the duel ends without a winner (D18, 10.4).
	int MaxDurationMs = 0;

	// Hesitation on the one who refuses (D16: 20 to 30 s).
	int HesitationMs = 0;

	// Triumph for the winner's troops and Demoralised for the loser's (D35: 30 to 45 s).
	int TriumphMs = 0;

	// The arena circle's radius, cm (10.2: "short radius"; indicative, to be tuned in tests, GDD 10.2 gives no number).
	int ArenaRadiusCm = 0;

	// DU-Q1 (both heroes fall in the same step), both ways ready until the user answers: true = A (both fall, in recovery,
	// no reward); false = B (nobody falls, both are left at one health, as for a time up).
	bool bDoubleKnockOutBothFall = true;

	// D117: the window flashes in its last instants, ms (presentation detail, indicative, to be tuned in tests).
	int WindowUrgentMs = 0;

	// DU-Q2, the three ways ready until the user answers; the preset takes A (the recommendation, not a decision).
	EHOLDuelLostConditions LostConditions = EHOLDuelLostConditions::DropsFree;

	// D403, the anti-duel zone around the MAIN forum of any player, cm. PROVISIONAL (DU-Q3): the GDD says "defence radius" without a number.
	int ForumZoneRadiusCm = 0;
}

// One duel's state, handed from call to call; every function returns a new one and leaves the one given alone.
struct FHOLDuel
{
	EHOLDuelPhase Phase = EHOLDuelPhase::Idle;
	EHOLDuelEnd End = EHOLDuelEnd::None;

	// The slots of the two heroes; the challenger is the one whose challenge it is.
	int Challenger = -1;
	int Target = -1;

	// While Pending, the time the target has left to answer; while Active, the time left to the duel and the time elapsed.
	int WindowLeftMs = 0;
	int TimeLeftMs = 0;
	int ElapsedMs = 0;

	// Set when the duel ended in a victory; -1 otherwise.
	int Winner = -1;
	int Loser = -1;

	// D120: the circle, set where the duel began (HOLDuel::BookSetArena); none before.
	bool bHasArena = false;
	FHOLDuelArena Arena;
}

// What the end of a duel changes, read from an Over duel: the match applies it (morale effects on the troops around, the
// XP, the faction's effect, the heroes' deaths, the cooldown). Nothing is changed while the duel is not over.
struct FHOLDuelEffects
{
	// A refusal (spoken or silent): the refuser hesitates; bRefusalCost is the faction's own price (Honour for the Dawn).
	int HesitationSlot = -1;
	int HesitationMs = 0;
	bool bRefusalCost = false;

	// A victory: Triumph for the winner, Demoralised for the loser, for MoraleMs; an XP bonus; the winner's faction effect.
	int TriumphSlot = -1;
	int DemoralisedSlot = -1;
	int MoraleMs = 0;
	bool bXpBonus = false;
	bool bFactionEffect = false;

	// The heroes who die: the loser of a won duel; both after a double knock-out (DU-Q1 = A).
	TArray<int> FallenSlots;

	// DU-Q1 = B: nobody falls, both are put back at one health.
	bool bLeaveAtOneHealth = false;

	// The challenger's challenge cooldown, spent by the challenge itself whatever the end (D80); -1 while Idle.
	int ChallengeCooldownSlot = -1;
	int ChallengeCooldownMs = 0;
}

namespace HOLDuel
{
	// The preset: the middle of each range of the GDD.
	FHOLDuelSettings Preset()
	{
		FHOLDuelSettings S;
		S.ResponseMs = 6500;
		S.ChallengeCooldownMs = 150000;
		S.MaxDurationMs = 90000;
		S.HesitationMs = 25000;
		S.TriumphMs = 40000;
		S.ArenaRadiusCm = 600;
		S.WindowUrgentMs = 2000;
		S.bDoubleKnockOutBothFall = true;
		S.ForumZoneRadiusCm = 2500;
		return S;
	}

	// A challenge: the target has the whole window to answer. The caller checked the conditions of the challenge and
	// spent the cooldown (Effects tells it).
	FHOLDuel Challenge(const FHOLDuelSettings& S, int Challenger, int Target)
	{
		FHOLDuel D;
		D.Phase = EHOLDuelPhase::Pending;
		D.Challenger = Challenger;
		D.Target = Target;
		D.WindowLeftMs = S.ResponseMs;
		return D;
	}

	// The target's answer (D80: final, only while Pending): accepted, the duel starts at once with its whole time;
	// refused, it is over. Any other state is returned unchanged.
	FHOLDuel Respond(const FHOLDuelSettings& S, const FHOLDuel& Duel, bool bAccept)
	{
		FHOLDuel D = Duel;
		if (D.Phase != EHOLDuelPhase::Pending)
			return D;
		D.WindowLeftMs = 0;
		if (bAccept)
		{
			D.Phase = EHOLDuelPhase::Active;
			D.TimeLeftMs = S.MaxDurationMs;
			D.ElapsedMs = 0;
		}
		else
		{
			D.Phase = EHOLDuelPhase::Over;
			D.End = EHOLDuelEnd::Refused;
		}
		return D;
	}

	// Time goes by (DeltaMs) and the two heroes' health is read at the end of it. Pending: a fallen hero cancels the
	// challenge (nobody refused); else the window runs down, and at zero the silence is a refusal. Active: the time
	// elapses; a fallen hero ends the duel (both: double knock-out); a fall on the last step beats the clock; at zero with
	// both standing it ends without a winner. Idle and Over: unchanged.
	FHOLDuel Advance(const FHOLDuelSettings& S, const FHOLDuel& Duel, int DeltaMs, int ChallengerHealth, int TargetHealth)
	{
		FHOLDuel D = Duel;
		const bool bChallengerFell = ChallengerHealth <= 0;
		const bool bTargetFell = TargetHealth <= 0;
		if (D.Phase == EHOLDuelPhase::Pending)
		{
			if (bChallengerFell || bTargetFell)
			{
				D.Phase = EHOLDuelPhase::Over;
				D.End = EHOLDuelEnd::Cancelled;
				D.WindowLeftMs = 0;
				return D;
			}
			D.WindowLeftMs = Math::Max(0, D.WindowLeftMs - DeltaMs);
			if (D.WindowLeftMs == 0)
			{
				D.Phase = EHOLDuelPhase::Over;
				D.End = EHOLDuelEnd::Ignored;
			}
			return D;
		}
		if (D.Phase != EHOLDuelPhase::Active)
			return D;
		D.ElapsedMs += DeltaMs;
		D.TimeLeftMs = Math::Max(0, D.TimeLeftMs - DeltaMs);
		if (bChallengerFell && bTargetFell)
		{
			D.Phase = EHOLDuelPhase::Over;
			D.End = EHOLDuelEnd::DoubleKnockOut;
		}
		else if (bChallengerFell)
		{
			D.Phase = EHOLDuelPhase::Over;
			D.End = EHOLDuelEnd::Won;
			D.Winner = D.Target;
			D.Loser = D.Challenger;
		}
		else if (bTargetFell)
		{
			D.Phase = EHOLDuelPhase::Over;
			D.End = EHOLDuelEnd::Won;
			D.Winner = D.Challenger;
			D.Loser = D.Target;
		}
		else if (D.TimeLeftMs == 0)
		{
			D.Phase = EHOLDuelPhase::Over;
			D.End = EHOLDuelEnd::TimedOut;
		}
		return D;
	}

	// DU-Q2: the target answers (accept or refuse) while the conditions of the challenge hold or not (bConditionsHold).
	FHOLDuel RespondChecked(const FHOLDuelSettings& S, const FHOLDuel& Duel, bool bAccept, bool bConditionsHold)
	{
		if (Duel.Phase != EHOLDuelPhase::Pending || bConditionsHold)
			return Respond(S, Duel, bAccept);
		if (S.LostConditions == EHOLDuelLostConditions::DropsFree)
			return Recheck(S, Duel, false);
		// Holds: the answer is the target's own; CountsAsRefusal: an acceptance is a refusal, with its costs.
		const bool bAcceptNow = bAccept && S.LostConditions == EHOLDuelLostConditions::Holds;
		return Respond(S, Duel, bAcceptNow);
	}

	// DU-Q2: the conditions are read again while the challenge waits; only a Pending duel can fall (DropsFree).
	FHOLDuel Recheck(const FHOLDuelSettings& S, const FHOLDuel& Duel, bool bConditionsHold)
	{
		FHOLDuel D = Duel;
		if (D.Phase != EHOLDuelPhase::Pending || bConditionsHold || S.LostConditions != EHOLDuelLostConditions::DropsFree)
			return D;
		D.Phase = EHOLDuelPhase::Over;
		D.End = EHOLDuelEnd::Dropped;
		D.WindowLeftMs = 0;
		return D;
	}

	// What the end of this duel changes (empty while the duel is not over).
	FHOLDuelEffects Effects(const FHOLDuelSettings& S, const FHOLDuel& Duel)
	{
		FHOLDuelEffects E;
		if (Duel.Phase == EHOLDuelPhase::Idle)
			return E;
		// The challenge itself spends the cooldown, whatever happens next (D80).
		E.ChallengeCooldownSlot = Duel.Challenger;
		E.ChallengeCooldownMs = S.ChallengeCooldownMs;
		if (Duel.Phase != EHOLDuelPhase::Over)
			return E;
		if (Duel.End == EHOLDuelEnd::Refused || Duel.End == EHOLDuelEnd::Ignored)
		{
			E.HesitationSlot = Duel.Target;
			E.HesitationMs = S.HesitationMs;
			E.bRefusalCost = true;
		}
		else if (Duel.End == EHOLDuelEnd::Won)
		{
			E.TriumphSlot = Duel.Winner;
			E.DemoralisedSlot = Duel.Loser;
			E.MoraleMs = S.TriumphMs;
			E.bXpBonus = true;
			E.bFactionEffect = true;
			E.FallenSlots.Add(Duel.Loser);
		}
		else if (Duel.End == EHOLDuelEnd::DoubleKnockOut)
		{
			if (S.bDoubleKnockOutBothFall)
			{
				E.FallenSlots.Add(Duel.Challenger);
				E.FallenSlots.Add(Duel.Target);
			}
			else
			{
				E.bLeaveAtOneHealth = true;
			}
		}
		return E;
	}
}

// ---- D2: the conditions of a challenge (GDD 10.1, 10.2, D16, D80, D403) ----

// What the rule needs to know about one hero; the match fills it in from the hero's state.
struct FHOLDuelHero
{
	bool bAlive = true;

	// Dead and waiting to come back, or any other recovery (10.1: a hero in recovery cannot challenge).
	bool bRecovering = false;

	// In a duel already, pending or active: see HOLDuel::InDuel.
	bool bInDuel = false;

	// The time left on the hero's own challenge cooldown (D16); it limits the challenges the hero MAKES, not the ones it receives.
	int ChallengeCooldownLeftMs = 0;
}

// What the rule needs to know about the two heroes together; the match measures it (distance, sight, the main forums'
// defence radius of every player, D403) and the rule decides.
struct FHOLDuelPlace
{
	// The two heroes are on opposite sides: a duel is against an adverse hero.
	bool bEnemies = true;

	// Close enough (the radius of "short distance" is a match matter, not a rule's) and in sight of each other.
	bool bNear = true;
	bool bSeeEachOther = true;

	// Inside the defence radius of the main forum of ANY player (D403), ruined or not; secondary centres do not count.
	bool bInMainForumRadius = false;

	// An ability or event forbids duels for the moment (10.1).
	bool bBlockedByEffect = false;
}

namespace HOLDuel
{
	// "" when the challenge can be made, else the first reason it cannot: not_enemy, challenger_unavailable,
	// target_unavailable, cooldown, challenger_in_duel, target_in_duel, too_far, not_visible, forum_zone, blocked.
	FString CanChallenge(const FHOLDuelHero& Challenger, const FHOLDuelHero& Target, const FHOLDuelPlace& Place)
	{
		if (!Place.bEnemies)
			return "not_enemy";
		if (!Challenger.bAlive || Challenger.bRecovering)
			return "challenger_unavailable";
		if (!Target.bAlive || Target.bRecovering)
			return "target_unavailable";
		if (Challenger.ChallengeCooldownLeftMs > 0)
			return "cooldown";
		if (Challenger.bInDuel)
			return "challenger_in_duel";
		if (Target.bInDuel)
			return "target_in_duel";
		if (!Place.bNear)
			return "too_far";
		if (!Place.bSeeEachOther)
			return "not_visible";
		if (Place.bInMainForumRadius)
			return "forum_zone";
		if (Place.bBlockedByEffect)
			return "blocked";
		return "";
	}

	// ---- D3: the protected duel (GDD 10.2, D18) ----

	// This hero is committed to the duel: its challenge waits for the answer, or the duel is on. A hero is in one duel at
	// a time, and no third hero can step in (no 2 against 1).
	bool InDuel(const FHOLDuel& Duel, int Slot)
	{
		if (Duel.Phase != EHOLDuelPhase::Pending && Duel.Phase != EHOLDuelPhase::Active)
			return false;
		return Slot == Duel.Challenger || Slot == Duel.Target;
	}

	// Whether a blow can land. Attacker and victim are HERO slots, -1 for anything that is not a hero (a unit, a tower, a
	// power, a world event). While the duel is ON (Active, not before the answer) the two duellists can be touched by each
	// other only, and touch nothing else; the battle around goes on for everybody else.
	bool CanHit(const FHOLDuel& Duel, int AttackerHeroSlot, int VictimHeroSlot)
	{
		if (Duel.Phase != EHOLDuelPhase::Active)
			return true;
		const bool bAttackerFights = AttackerHeroSlot >= 0 && (AttackerHeroSlot == Duel.Challenger || AttackerHeroSlot == Duel.Target);
		const bool bVictimFights = VictimHeroSlot >= 0 && (VictimHeroSlot == Duel.Challenger || VictimHeroSlot == Duel.Target);
		if (!bAttackerFights && !bVictimFights)
			return true;
		return bAttackerFights && bVictimFights && AttackerHeroSlot != VictimHeroSlot;
	}
}
// ---- D3b: the match's book of duels (DuelInProgress.md D3b) ----
// Several duels can be on at once in a match with more than two players, one per hero (10.2). The book is the one truth
// of "who is in a duel" and "whose challenge cooldown runs": the match hands it the clock and the heroes' health, asks
// it to start a duel or to take an answer, and reads the duels that ended (each reported once, then forgotten).

struct FHOLDuelBook
{
	// The duels in progress: Pending or Active (an Over duel leaves the book at once, in Tick).
	TArray<FHOLDuel> Duels;

	// By slot: the time left on the slot's challenge cooldown (D16).
	TArray<int> CooldownLeftMs;
}

// A duel that just ended, with what its end changes.
struct FHOLDuelEnded
{
	FHOLDuel Duel;
	FHOLDuelEffects Effects;
}

namespace HOLDuel
{
	// An empty book for a match of NumSlots slots.
	FHOLDuelBook NewBook(int NumSlots)
	{
		FHOLDuelBook Book;
		for (int Slot = 0; Slot < NumSlots; Slot++)
		{
			Book.CooldownLeftMs.Add(0);
		}
		return Book;
	}

	// True when the slot is in a duel of the book (pending or active).
	bool BookInDuel(const FHOLDuelBook& Book, int Slot)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (InDuel(Book.Duels[Index], Slot))
				return true;
		}
		return false;
	}

	// A challenge: "" and the duel (Pending) in the book, the challenger's cooldown spent; or the reason of CanChallenge
	// and the book untouched. The heroes' alive / recovering state and the place come from the match; whether each is in
	// a duel, and the challenger's cooldown, come from the book (whatever the hero structs say).
	FString Start(const FHOLDuelSettings& S, FHOLDuelBook& Book, int Challenger, int Target, const FHOLDuelHero& ChallengerHero, const FHOLDuelHero& TargetHero, const FHOLDuelPlace& Place)
	{
		FHOLDuelHero Mine = ChallengerHero;
		FHOLDuelHero Theirs = TargetHero;
		Mine.bInDuel = BookInDuel(Book, Challenger);
		Theirs.bInDuel = BookInDuel(Book, Target);
		Mine.ChallengeCooldownLeftMs = Challenger >= 0 && Challenger < Book.CooldownLeftMs.Num() ? Book.CooldownLeftMs[Challenger] : 0;
		FString Reason = CanChallenge(Mine, Theirs, Place);
		if (!Reason.IsEmpty())
			return Reason;
		Book.Duels.Add(Challenge(S, Challenger, Target));
		if (Challenger >= 0 && Challenger < Book.CooldownLeftMs.Num())
			Book.CooldownLeftMs[Challenger] = S.ChallengeCooldownMs;
		return "";
	}

	// The target's answer to the challenge waiting for it (D80: final). False when no pending challenge is addressed to
	// this slot (the challenger cannot answer its own challenge; a second answer finds nothing pending).
	bool Answer(const FHOLDuelSettings& S, FHOLDuelBook& Book, int TargetSlot, bool bAccept)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Pending && Book.Duels[Index].Target == TargetSlot)
			{
				Book.Duels[Index] = Respond(S, Book.Duels[Index], bAccept);
				return true;
			}
		}
		return false;
	}

	// DU-Q2: the answer of the slot with the conditions read at that instant. False when no pending challenge is addressed to it.
	bool AnswerChecked(const FHOLDuelSettings& S, FHOLDuelBook& Book, int TargetSlot, bool bAccept, bool bConditionsHold)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Pending && Book.Duels[Index].Target == TargetSlot)
			{
				Book.Duels[Index] = RespondChecked(S, Book.Duels[Index], bAccept, bConditionsHold);
				return true;
			}
		}
		return false;
	}

	// DU-Q2: the conditions read again for the challenge addressed to this slot; whether a challenge fell (it leaves the book
	// at the next Tick, reported with its effects).
	bool BookRecheck(const FHOLDuelSettings& S, FHOLDuelBook& Book, int TargetSlot, bool bConditionsHold)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (Book.Duels[Index].Phase == EHOLDuelPhase::Pending && Book.Duels[Index].Target == TargetSlot)
			{
				Book.Duels[Index] = Recheck(S, Book.Duels[Index], bConditionsHold);
				return Book.Duels[Index].Phase == EHOLDuelPhase::Over;
			}
		}
		return false;
	}

	// Health of a slot in the array the match hands over (a slot it does not list counts as standing).
	int HealthOf(const TArray<int>& Health, int Slot)
	{
		if (Slot < 0 || Slot >= Health.Num())
			return 1;
		return Health[Slot];
	}

	// Time goes by: every cooldown runs down, every duel advances with the heroes' health (by slot); the duels that ended
	// leave the book and are returned, once, with their effects.
	TArray<FHOLDuelEnded> Tick(const FHOLDuelSettings& S, FHOLDuelBook& Book, int DeltaMs, const TArray<int>& Health)
	{
		for (int Slot = 0; Slot < Book.CooldownLeftMs.Num(); Slot++)
		{
			Book.CooldownLeftMs[Slot] = Math::Max(0, Book.CooldownLeftMs[Slot] - DeltaMs);
		}
		TArray<FHOLDuelEnded> Ended;
		TArray<FHOLDuel> Kept;
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			FHOLDuel Next = Advance(S, Book.Duels[Index], DeltaMs, HealthOf(Health, Book.Duels[Index].Challenger), HealthOf(Health, Book.Duels[Index].Target));
			if (Next.Phase == EHOLDuelPhase::Over)
			{
				FHOLDuelEnded Done;
				Done.Duel = Next;
				Done.Effects = Effects(S, Next);
				Ended.Add(Done);
			}
			else
			{
				Kept.Add(Next);
			}
		}
		Book.Duels = Kept;
		return Ended;
	}

	// CanHit for the whole book: every duel on must agree (the heroes of two different duels cannot touch each other).
	bool BookCanHit(const FHOLDuelBook& Book, int AttackerHeroSlot, int VictimHeroSlot)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			if (!CanHit(Book.Duels[Index], AttackerHeroSlot, VictimHeroSlot))
				return false;
		}
		return true;
	}
}