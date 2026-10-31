// The AI's duel decisions (DuelInProgress.md D9; GDD 10.3, D421-D423): pure rules, whole numbers, no world access. The brain
// reads the match (the two heroes' level and health, the kit's recharges, the battle around, the faction) into FHOLAIDuelView
// and asks one of two questions: ANSWER a challenge (accept or refuse) or CHALLENGE (which hero, when). Each answer comes
// with a reason line (the decision journal). The AI estimates its chances, then weighs them by the moment (D423): what a
// refusal would cost now and what a victory would bring here. Its reaction time is a human's (never within the window's
// first instants), and the "choice of duels" cursor (D422) sets how far its estimate can be wrong: Easy is often wrong.
// The numbers are a starting point, in one place (Preset), "to be tuned in tests"; nothing here is a design decision.

// What the AI sees of the duel proposed or possible (percent and counts: whole numbers).
struct FHOLAIDuelView
{
	// Its hero and the other one: level (1 to 10), health in percent of the maximum, and the part of the duel kit ready in percent
	// (parry, own abilities: what is not on recharge). The enemy's kit is known to the AI as a player knows it: by sight.
	int MyLevel = 1;
	int EnemyLevel = 1;
	int MyHealthPercent = 100;
	int EnemyHealthPercent = 100;
	int MyKitReadyPercent = 100;
	int EnemyKitReadyPercent = 100;

	// The moment: a battle is on (or about to be) near its hero, so that a refusal's Hesitation would fall at the worst time;
	// the troops of its side close to its hero (a victory's Triumph lands on them); the Order of the Dawn loses Honour on a refusal.
	bool bBattleNear = false;
	int TroopsNearMe = 0;
	bool bDawn = false;

	// Its hero is in danger (low health under attack): a duel protects him (D18, D423).
	bool bMyHeroInDanger = false;
}

// The "choice of duels" cursor (D422) and the AI's own numbers, by level.
struct FHOLAIDuelSettings
{
	// How far the estimate can be wrong, percent points of the odds either way (the caller hands a Noise of -100..100).
	int EstimateErrorPercent = 0;

	// Reaction time before the answer is given (a human's), ms.
	int ReactionMs = 0;

	// The odds (percent) needed to accept with no pressure, and the floor under it however great the pressure.
	int BaseAcceptPercent = 0;
	int MinAcceptPercent = 0;

	// Pressure that lowers the bar: a battle near (the refusal's Hesitation lands at the worst time), the Dawn's Honour,
	// and each troop near (a victory's Triumph lands on them), capped.
	int BattlePressure = 0;
	int DawnPressure = 0;
	int TroopPressureEach = 0;
	int TroopPressureMax = 0;

	// To challenge: the odds needed in a battle, and the lowest odds a protective challenge is worth (the hero in danger).
	int ChallengeAcceptPercent = 0;
	int ProtectionFloorPercent = 0;
}

// An answer or a challenge decided, with the numbers it rests on.
struct FHOLAIDuelDecision
{
	bool bYes = false;
	int Odds = 0;
	int EstimatedOdds = 0;
	int Threshold = 0;
	int Pressure = 0;
	int ReactionMs = 0;
	FString Reason;
}

namespace HOLAIDuel
{
	FHOLAIDuelSettings Preset(EHOLAIDifficulty Level)
	{
		FHOLAIDuelSettings S;
		S.BaseAcceptPercent = 60;
		S.MinAcceptPercent = 35;
		S.BattlePressure = 12;
		S.DawnPressure = 6;
		S.TroopPressureEach = 1;
		S.TroopPressureMax = 8;
		S.ChallengeAcceptPercent = 65;
		S.ProtectionFloorPercent = 30;
		S.EstimateErrorPercent = 12;
		S.ReactionMs = 2000;
		if (Level == EHOLAIDifficulty::Easy)
		{
			S.EstimateErrorPercent = 30;
			S.ReactionMs = 3200;
		}
		else if (Level == EHOLAIDifficulty::Hard)
		{
			S.EstimateErrorPercent = 4;
			S.ReactionMs = 1200;
		}
		return S;
	}

	// The strength of one hero: his health, his level, the part of his kit ready. Level counts as a stat edge (D43: statistics only).
	int Strength(int Level, int HealthPercent, int KitReadyPercent)
	{
		// Level 1 counts 10 and each level above adds 1 (a stat edge only, D43); the kit counts from half (never ready) to the whole.
		return HealthPercent * (9 + Level) * (50 + KitReadyPercent / 2);
	}

	// The true chance of winning, percent (0..100), from the two heroes.
	int Odds(const FHOLAIDuelView& View)
	{
		const int Mine = Strength(View.MyLevel, View.MyHealthPercent, View.MyKitReadyPercent);
		const int Theirs = Strength(View.EnemyLevel, View.EnemyHealthPercent, View.EnemyKitReadyPercent);
		if (Mine + Theirs <= 0)
			return 50;
		// Rounded to the nearest percent (half up), so that the two sides' odds add up to 100.
		return Math::IntegerDivisionTrunc(200 * Mine + Mine + Theirs, 2 * (Mine + Theirs));
	}

	// What the AI believes the odds are: the truth moved by its error, Noise -100..100 (the caller draws it: pure here).
	int EstimatedOdds(const FHOLAIDuelSettings& S, const FHOLAIDuelView& View, int Noise)
	{
		const int Shift = Math::IntegerDivisionTrunc(Math::Clamp(Noise, -100, 100) * S.EstimateErrorPercent, 100);
		return Math::Clamp(Odds(View) + Shift, 0, 100);
	}

	// How much the moment lowers the bar for accepting.
	int Pressure(const FHOLAIDuelSettings& S, const FHOLAIDuelView& View)
	{
		int Total = 0;
		if (View.bBattleNear)
		{
			Total += S.BattlePressure;
			Total += Math::Min(S.TroopPressureMax, View.TroopsNearMe * S.TroopPressureEach);
		}
		if (View.bDawn)
			Total += S.DawnPressure;
		return Total;
	}

	// A challenge was received: accept or refuse, after the reaction time.
	FHOLAIDuelDecision Answer(const FHOLAIDuelSettings& S, const FHOLAIDuelView& View, int Noise)
	{
		FHOLAIDuelDecision D;
		D.Odds = Odds(View);
		D.EstimatedOdds = EstimatedOdds(S, View, Noise);
		D.Pressure = Pressure(S, View);
		D.Threshold = Math::Max(S.MinAcceptPercent, S.BaseAcceptPercent - D.Pressure);
		D.ReactionMs = S.ReactionMs;
		D.bYes = D.EstimatedOdds >= D.Threshold;
		if (D.bYes)
			D.Reason = D.Pressure > 0 ? "the odds reach the bar lowered by the moment" : "the odds reach the bar";
		else
			D.Reason = D.EstimatedOdds < S.MinAcceptPercent ? "too likely to lose, whatever the moment" : "the odds do not reach the bar";
		return D;
	}

	// Whether to challenge now (bAllowed: HOLDuel::CanChallenge said "" for this pair).
	FHOLAIDuelDecision Challenge(const FHOLAIDuelSettings& S, const FHOLAIDuelView& View, bool bAllowed, int Noise)
	{
		FHOLAIDuelDecision D;
		D.Odds = Odds(View);
		D.EstimatedOdds = EstimatedOdds(S, View, Noise);
		D.Pressure = Pressure(S, View);
		D.ReactionMs = S.ReactionMs;
		if (!bAllowed)
		{
			D.Threshold = S.ChallengeAcceptPercent;
			D.Reason = "the challenge is not allowed";
			return D;
		}
		if (View.bMyHeroInDanger)
		{
			// A protection (D18, D423): worth it unless the duel is surely lost.
			D.Threshold = S.ProtectionFloorPercent;
			D.bYes = D.EstimatedOdds >= D.Threshold;
			D.Reason = D.bYes ? "the hero is in danger and the duel protects him" : "the hero is in danger but the duel would be lost";
			return D;
		}
		D.Threshold = S.ChallengeAcceptPercent;
		if (!View.bBattleNear)
		{
			D.Reason = "no fight near: it challenges just before or during one";
			return D;
		}
		D.bYes = D.EstimatedOdds >= D.Threshold;
		D.Reason = D.bYes ? "a fight is near and the odds are good" : "the odds are not good enough to seek the duel";
		return D;
	}

	// The journal line of a decision.
	FString Describe(const FString& What, const FHOLAIDuelDecision& D)
	{
		return "AI duel " + What + ": " + (D.bYes ? "accept" : "refuse") + " (odds " + D.Odds + ", believed " + D.EstimatedOdds + ", bar " + D.Threshold + ", pressure " + D.Pressure + ", reaction " + D.ReactionMs + " ms): " + D.Reason;
	}
}