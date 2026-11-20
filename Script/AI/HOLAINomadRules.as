// Nomad option in the AI's rules (N3, NomadInProgress.md; GDD 14.1 and 14.3, D207, D354, D402, D786, D803): an AI slot that starts
// without a forum (its Peasants dispersed, the forum price in its stock) founds its first forum. Pure functions on plain inputs (the places
// of its Peasants, its stock, the forum price), tested in HOLAINomadRules_Test.as. NOT WIRED to the game today: the C++ build order of
// the AI (the brain's order to place a MainCentre foundation and send the Peasants) is the coming step (NomadInProgress.md).
// The design is not decided (Retours/Q-nomad-ia.md, [A REPONDRE]): the rule below is the simplest one, behind hol.AI.NomadForumRule.

// Which rule founds the AI's first forum (Q-nomad-ia; 1 = the recommended default, one line to change):
// 0 = the AI never founds by itself (no Nomad rule), 1 = at the largest group of Peasants, as soon as the stock covers the forum price.
const FConsoleVariable CVar_AINomadForumRule("hol.AI.NomadForumRule", 1);

// Two Peasants this close (cm) are in the same group (7 m, the reach of a starting group in the scatter, NomadInProgress.md D797).
const FConsoleVariable CVar_AINomadGroupRadius("hol.AI.NomadGroupRadius", 700);

// The most builders the AI's first forum takes at once, the one already sent included (D817 Q-nomad-ia: the other Peasants join the
// foundation; one line to change). The forum takes 150 s for one builder and less with more (D816).
const FConsoleVariable CVar_AINomadMaxBuilders("hol.AI.NomadMaxBuilders", 4);

// What the rule decides at one thought: found now or not, the group chosen (index in Groups) and the place of the foundation.
struct FHOLAINomadPlan
{
	bool bFound = false;
	int Group = -1;
	FVector Site = FVector::ZeroVector;
}

namespace HOLAINomad
{
	const int RuleNone = 0;
	const int RuleLargestGroupAtOnce = 1;

	int RuleFromCVar()
	{
		return CVar_AINomadForumRule.GetInt();
	}

	float GroupRadiusFromCVar()
	{
		return float(Math::Max(0, CVar_AINomadGroupRadius.GetInt()));
	}

	// The group of each Peasant (same order as Places): two Peasants within Radius of each other are in the same group, and so on by
	// neighbours; groups are numbered in the order of their first Peasant (the same answer on every machine).
	TArray<int> GroupsOf(const TArray<FVector>& Places, float Radius)
	{
		TArray<int> Groups;
		for (int Index = 0; Index < Places.Num(); Index++)
			Groups.Add(HOLNomad::None);
		int Next = 0;
		for (int First = 0; First < Places.Num(); First++)
		{
			if (Groups[First] != HOLNomad::None)
				continue;
			Groups[First] = Next;
			TArray<int> Open;
			Open.Add(First);
			while (Open.Num() > 0)
			{
				const int Current = Open[Open.Num() - 1];
				Open.RemoveAt(Open.Num() - 1);
				for (int Other = 0; Other < Places.Num(); Other++)
				{
					if (Groups[Other] == HOLNomad::None && FlatDistance(Places[Current], Places[Other]) <= Radius)
					{
						Groups[Other] = Next;
						Open.Add(Other);
					}
				}
			}
			Next++;
		}
		return Groups;
	}

	// The distance on the ground (cm), height left out.
	float FlatDistance(FVector A, FVector B)
	{
		return FVector(A.X - B.X, A.Y - B.Y, 0.f).Size();
	}

	// The number of Peasants in each group (index = group).
	TArray<int> GroupSizes(const TArray<int>& Groups)
	{
		TArray<int> Sizes;
		for (int Index = 0; Index < Groups.Num(); Index++)
		{
			if (Groups[Index] < 0)
				continue;
			while (Sizes.Num() <= Groups[Index])
				Sizes.Add(0);
			Sizes[Groups[Index]] += 1;
		}
		return Sizes;
	}

	// The stock covers the price, resource by resource (same order: food, wood, stone, gold).
	bool StockCovers(const TArray<int>& Stock, const TArray<int>& Price)
	{
		if (Stock.Num() < Price.Num())
			return false;
		for (int Index = 0; Index < Price.Num(); Index++)
		{
			if (Stock[Index] < Price[Index])
				return false;
		}
		return true;
	}

	// The decision: a Nomad slot before its first complete forum, with no forum foundation underway, founds at the centre of its largest
	// group of Peasants (the first one on a tie, HOLNomad::LargestGroup) as soon as its stock covers the forum price (Rule 1).
	FHOLAINomadPlan Plan(int Rule, bool bBeforeFirstForum, bool bForumUnderway, const TArray<FVector>& Places, const TArray<int>& Stock,
		const TArray<int>& Price, float Radius)
	{
		FHOLAINomadPlan Result;
		if (Rule != RuleLargestGroupAtOnce || !bBeforeFirstForum || bForumUnderway || Places.Num() == 0 || !StockCovers(Stock, Price))
			return Result;
		const TArray<int> Groups = GroupsOf(Places, Radius);
		const int Best = HOLNomad::LargestGroup(GroupSizes(Groups));
		if (Best == HOLNomad::None)
			return Result;
		FVector Sum = FVector::ZeroVector;
		int Count = 0;
		for (int Index = 0; Index < Places.Num(); Index++)
		{
			if (Groups[Index] == Best)
			{
				Sum += Places[Index];
				Count++;
			}
		}
		Result.bFound = true;
		Result.Group = Best;
		Result.Site = Sum / float(Count);
		return Result;
	}

	int MaxBuildersFromCVar()
	{
		return Math::Max(0, CVar_AINomadMaxBuilders.GetInt());
	}

	// The Peasants that join the Nomad foundation (D817 Q-nomad-ia): the idle ones, never one of Assigned (the builder already sent), the
	// nearest to Foundation first (a tie: the lower index), as many as bring the builders up to MaxBuilders (Assigned counted).
	TArray<int> HelpersFor(FVector Foundation, const TArray<FHOLAIUnit>& Peasants, const TArray<int>& Assigned, int MaxBuilders)
	{
		TArray<int> Helpers;
		int Room = MaxBuilders - Assigned.Num();
		TArray<bool> Taken;
		for (int Index = 0; Index < Peasants.Num(); Index++)
			Taken.Add(!Peasants[Index].bIdle || Assigned.Contains(Peasants[Index].Id));
		while (Room > 0)
		{
			int Best = HOLNomad::None;
			float BestDistance = 0.f;
			for (int Index = 0; Index < Peasants.Num(); Index++)
			{
				if (Taken[Index])
					continue;
				const float Distance = FlatDistance(Foundation, Peasants[Index].Location);
				if (Best == HOLNomad::None || Distance < BestDistance)
				{
					Best = Index;
					BestDistance = Distance;
				}
			}
			if (Best == HOLNomad::None)
				break;
			Taken[Best] = true;
			Helpers.Add(Peasants[Best].Id);
			Room--;
		}
		return Helpers;
	}
}
