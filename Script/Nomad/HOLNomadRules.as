// Nomad option, pure rules (N1, NomadInProgress.md; GDD 14.1 and 14.3, D207, D354, D402). With the option (the menu box,
// FHOLMatchConfig::bNomad), a player starts without a main forum: his Peasants are dispersed in small groups at random places of the map
// (D207), the hero appears with one group, and the price of a forum is added to his starting stock (D402: no forum given, he places it
// where and when he wants or spends otherwise at his own risk). As long as none of his forums is complete, he loses when he has no Peasant
// left (D354); a forum destroyed while being built costs no life (its resources are lost); from the first complete forum on, the normal
// rules apply (the forum with lives, D347); before it, the hero comes back beside the largest group of Peasants (D354).
// The defeat itself goes through the existing AHOLGameState::DefeatSlot (C++, step N2, a new EHOLDefeatReason); nothing here reads the
// world: the counts are the caller's input. Tested in Script/Tests/HOLNomadRules_Test.as.
// Provisional (Retours/Q-nomad.md): the size of the groups (Q1) and the minimal distance to an enemy group (Q2), one line each below.
struct FHOLNomadState
{
	// The Nomad option of the match (FHOLMatchConfig::bNomad).
	bool bNomad = false;
	// One forum of the player was complete once: the normal rules apply for good (D354).
	bool bForumCompleteOnce = false;
}

namespace HOLNomad
{
	const int None = -1;

	// D786 (Q-nomad Q1 = B, 2026-11-12; D207 says "small groups"): one Peasant per starting group (5 Peasants alone, AoE2's Nomad).
	const int GroupSize = 1;

	// D786 (Q-nomad Q2 = A, 2026-11-12; D207 "vigilance: minimal distance between players"): the least distance between a starting
	// group and a group of another camp (cm; 1 cell = 100 cm; 40 cells).
	const int MinEnemyDistanceCm = 4000;

	FHOLNomadState NewState(bool bNomad)
	{
		FHOLNomadState State;
		State.bNomad = bNomad;
		return State;
	}

	// The first complete forum of the player (the end of its construction): noted once for all.
	void NoteForumComplete(FHOLNomadState& State)
	{
		State.bForumCompleteOnce = true;
	}

	// Nomad and no forum complete yet: the rules of D354 apply instead of the forum with lives.
	bool IsBeforeFirstForum(const FHOLNomadState& State)
	{
		return State.bNomad && !State.bForumCompleteOnce;
	}

	// One forum of the player is (or was) complete: false for a Nomad before his first (the input of HOLSpecialization::IsOpen and Start).
	bool HasCompleteForum(const FHOLNomadState& State)
	{
		return !IsBeforeFirstForum(State);
	}

	// D354: no Peasant left before the first complete forum is the defeat.
	bool IsDefeated(const FHOLNomadState& State, int PeasantCount)
	{
		return IsBeforeFirstForum(State) && PeasantCount <= 0;
	}

	// D354, D402: a forum falling while being built costs no life (its resources are lost); a complete one costs a life (D347).
	bool FallCostsLife(bool bForumWasComplete)
	{
		return bForumWasComplete;
	}

	// D402: one resource of the starting stock; the forum price is added with the option (no forum given).
	int StartingAmount(int StandardAmount, int ForumPrice, bool bNomad)
	{
		return bNomad ? StandardAmount + ForumPrice : StandardAmount;
	}

	// D354: the hero comes back beside the largest group of Peasants before the first forum, at the forum afterwards.
	bool HeroBackAtLargestGroup(const FHOLNomadState& State)
	{
		return IsBeforeFirstForum(State);
	}

	// The index of the largest group (the first of them on a tie: the same answer on every machine), None when no Peasant is left.
	int LargestGroup(const TArray<int>& GroupPeasants)
	{
		int Best = None;
		int BestCount = 0;
		for (int Index = 0; Index < GroupPeasants.Num(); Index++)
		{
			if (GroupPeasants[Index] > BestCount)
			{
				Best = Index;
				BestCount = GroupPeasants[Index];
			}
		}
		return Best;
	}

	// D207: the starting Peasants split in small groups of at most GroupSize (the last one smaller).
	TArray<int> SplitIntoGroups(int Peasants)
	{
		TArray<int> Sizes;
		int Left = Peasants;
		while (Left > 0)
		{
			int Size = Left < GroupSize ? Left : GroupSize;
			Sizes.Add(Size);
			Left -= Size;
		}
		return Sizes;
	}

	// D207 vigilance: a starting group is far enough from every enemy group (the distance to the closest one, cm).
	bool FarEnough(int DistanceToClosestEnemyCm)
	{
		return DistanceToClosestEnemyCm >= MinEnemyDistanceCm;
	}
}
