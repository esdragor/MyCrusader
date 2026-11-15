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
		return Groups;
	}

	// The number of Peasants in each group (index = group).
	TArray<int> GroupSizes(const TArray<int>& Groups)
	{
		TArray<int> Sizes;
		return Sizes;
	}

	// The stock covers the price, resource by resource (same order: food, wood, stone, gold).
	bool StockCovers(const TArray<int>& Stock, const TArray<int>& Price)
	{
		return false;
	}

	// The decision: a Nomad slot before its first complete forum, with no forum foundation underway, founds at the centre of its largest
	// group of Peasants (the first one on a tie, HOLNomad::LargestGroup) as soon as its stock covers the forum price (Rule 1).
	FHOLAINomadPlan Plan(int Rule, bool bBeforeFirstForum, bool bForumUnderway, const TArray<FVector>& Places, const TArray<int>& Stock,
		const TArray<int>& Price, float Radius)
	{
		FHOLAINomadPlan Result;
		return Result;
	}
}
