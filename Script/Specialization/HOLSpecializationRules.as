// Tier specialization choice, common mechanism, pure rules (SP1, SpecializationInProgress.md; GDD 9.5, D07, D332, D683, D684, with D131,
// D690): each tier (1, 2, 3) opens a common trunk and the choice of one specialization among 2 (one commandery among 3 for the Templars,
// D131), final for the match (D07). When the tier opens, a button lights up; the player picks when he decides, for free (D332); the
// unlock runs at the Forum beside its queue and the choice is applied after 30 / 45 / 60 s (tiers 1 / 2 / 3, D684); it can be cancelled
// at no cost before the end and the choice reopens (nothing to give back: nothing was paid); once applied it is final. What depends on the
// choice (a technology's effect, a building unlocked then paid as any building, D683; a Templar unit and its contingent, D334) arrives
// when Advance returns the tier; the faction chapters (Dawn A8, Legions L9) plug their options there (D690).
// The player's tier is the caller's input (AHOLGameState::GetTier in C++, from the hero's level, HOLHeroXP::GetTier; it never drops).
// Times are integer milliseconds of match time (D468 determinism); nothing here reads the world. Tested in
// Script/Tests/HOLSpecializationRules_Test.as.
// Provisional (Retours/Q-specialization.md): two tiers at once (Q1) and the Forum down (Q2), one line each below.
struct FHOLSpecializationState
{
	// The options of each tier: 2 (D07), 3 for the Templars' commanderies (D131).
	int OptionCount = 2;
	// Per tier (index = tier, 1 to 3; index 0 unused): the applied option, or None.
	TArray<int> Chosen;
	// Per tier: the option whose unlock runs at the Forum, or None; and the match time it has run.
	TArray<int> RunningOption;
	TArray<int> RunningMs;
	// The lowest tier whose unlock runs (the one the bar shows), 0 when none runs.
	int PendingTier = 0;
}

namespace HOLSpecialization
{
	const int None = -1;
	const int DefaultOptionCount = 2;
	const int FirstTier = 1;
	const int LastTier = 3;

	// PROVISIONAL (D684, "values to tune in test"): the unlock delay of tiers 1, 2 and 3.
	const int Tier1DelayMs = 30000;
	const int Tier2DelayMs = 45000;
	const int Tier3DelayMs = 60000;

	// D785 (Q-specialization Q1 = C, 2026-11-12): tier 1 first: a tier's button lights up only once the tier before is applied (the
	// order is imposed, one bar, so one unlock at a time); false = each reached tier its own unlock at once.
	const bool PreviousTierFirst = true;

	// D785 (Q-specialization Q2 = A, 2026-11-12; D684 leaves it to the chapter): the countdown while the Forum is down pauses.
	const int ForumDownPauses = 0;
	const int ForumDownRuns = 1;
	const int ForumDownCancels = 2;
	const int ForumDown = ForumDownPauses;

	// The answers of Start and Cancel.
	const int Ok = 0;
	const int BadTier = 1;
	const int TierNotReached = 2;
	const int BadOption = 3;
	const int AlreadyChosen = 4;
	const int Busy = 5;
	const int NothingToCancel = 6;
	const int PreviousTierNotApplied = 7;

	FHOLSpecializationState NewState(int OptionCount)
	{
		FHOLSpecializationState State;
		State.OptionCount = OptionCount;
		for (int Tier = 0; Tier <= LastTier; Tier++)
		{
			State.Chosen.Add(None);
			State.RunningOption.Add(None);
			State.RunningMs.Add(0);
		}
		return State;
	}

	bool IsValidTier(int Tier)
	{
		return Tier >= FirstTier && Tier <= LastTier;
	}

	// The unlock delay of a tier (D684); 0 outside tiers 1 to 3.
	int DelayMs(int Tier)
	{
		if (Tier == 1)
			return Tier1DelayMs;
		if (Tier == 2)
			return Tier2DelayMs;
		if (Tier == 3)
			return Tier3DelayMs;
		return 0;
	}

	// The applied option of a tier, or None.
	int ChosenOption(const FHOLSpecializationState& State, int Tier)
	{
		if (!IsValidTier(Tier) || Tier >= State.Chosen.Num())
			return None;
		return State.Chosen[Tier];
	}

	bool IsRunning(const FHOLSpecializationState& State, int Tier)
	{
		return IsValidTier(Tier) && Tier < State.RunningOption.Num() && State.RunningOption[Tier] != None;
	}

	// The tier before is applied, or the order is not imposed (D785).
	bool PreviousApplied(const FHOLSpecializationState& State, int Tier)
	{
		return !PreviousTierFirst || Tier <= FirstTier || ChosenOption(State, Tier - 1) != None;
	}

	// The tier's button is lit: reached, the tier before applied (D785) and not applied yet (D332: no deadline). It stays lit while its
	// unlock runs (the bar shows it).
	bool IsOpen(const FHOLSpecializationState& State, int Tier, int PlayerTier)
	{
		return IsValidTier(Tier) && PlayerTier >= Tier && PreviousApplied(State, Tier) && ChosenOption(State, Tier) == None;
	}

	void RefreshPending(FHOLSpecializationState& State)
	{
		State.PendingTier = 0;
		for (int Tier = FirstTier; Tier <= LastTier; Tier++)
		{
			if (IsRunning(State, Tier))
			{
				State.PendingTier = Tier;
				return;
			}
		}
	}

	// The player clicks Option of Tier: the unlock starts at the Forum (D684). Ok: it runs; any other answer: nothing changed. A running
	// unlock is not restarted in place: cancel it first.
	int Start(FHOLSpecializationState& State, int Tier, int Option, int PlayerTier)
	{
		if (!IsValidTier(Tier))
			return BadTier;
		if (PlayerTier < Tier)
			return TierNotReached;
		if (Option < 0 || Option >= State.OptionCount)
			return BadOption;
		if (ChosenOption(State, Tier) != None)
			return AlreadyChosen;
		if (IsRunning(State, Tier))
			return Busy;
		if (!PreviousApplied(State, Tier))
			return PreviousTierNotApplied;
		State.RunningOption[Tier] = Option;
		State.RunningMs[Tier] = 0;
		RefreshPending(State);
		return Ok;
	}

	// Cancels the running unlock of Tier (0: the one the bar shows) at no cost (D684): nothing chosen, the choice reopens. An applied
	// choice is final (D07): nothing to cancel.
	int Cancel(FHOLSpecializationState& State, int Tier = 0)
	{
		const int Target = Tier == 0 ? State.PendingTier : Tier;
		if (!IsRunning(State, Target))
			return NothingToCancel;
		State.RunningOption[Target] = None;
		State.RunningMs[Target] = 0;
		RefreshPending(State);
		return Ok;
	}

	// The time left on the bar of Tier (0: the one the bar shows) (D684); 0 when nothing runs.
	int RemainingMs(const FHOLSpecializationState& State, int Tier = 0)
	{
		const int Target = Tier == 0 ? State.PendingTier : Tier;
		if (!IsRunning(State, Target))
			return 0;
		const int Left = DelayMs(Target) - State.RunningMs[Target];
		return Left > 0 ? Left : 0;
	}

	// Lets ElapsedMs of match time pass on the running unlocks. Returns the tier whose choice is applied now (what depends on it arrives,
	// D332), or 0. One tier per call: should two land together (Q1 B), call again with 0 ms until it returns 0.
	int Advance(FHOLSpecializationState& State, int ElapsedMs, bool bForumStanding)
	{
		if (State.PendingTier == 0)
			return 0;
		if (!bForumStanding && ForumDown == ForumDownCancels)
		{
			for (int Tier = FirstTier; Tier <= LastTier; Tier++)
				Cancel(State, Tier);
			return 0;
		}
		const bool bRuns = bForumStanding || ForumDown == ForumDownRuns;
		int Applied = 0;
		for (int Tier = FirstTier; Tier <= LastTier; Tier++)
		{
			if (!IsRunning(State, Tier))
				continue;
			if (bRuns && ElapsedMs > 0)
				State.RunningMs[Tier] += ElapsedMs;
			if (Applied == 0 && State.RunningMs[Tier] >= DelayMs(Tier))
				Applied = Tier;
		}
		if (Applied != 0)
		{
			State.Chosen[Applied] = State.RunningOption[Applied];
			State.RunningOption[Applied] = None;
			State.RunningMs[Applied] = 0;
			RefreshPending(State);
		}
		return Applied;
	}
}
