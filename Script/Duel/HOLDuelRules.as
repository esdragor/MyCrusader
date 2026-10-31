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
	Cancelled
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

	// DU-Q1 (both heroes fall in the same step), both ways ready until the user answers: true = A (both fall, in recovery,
	// no reward); false = B (nobody falls, both are left at one health, as for a time up).
	bool bDoubleKnockOutBothFall = true;
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
		S.bDoubleKnockOutBothFall = true;
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
