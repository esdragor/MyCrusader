// The AI's garrison call (S4-b, D811): which of the AI's towers call which of its units in this think, built on the rule of
// HOLAIGarrisonRules.as (TowerCalls, Entrants). Pure function on plain inputs; the order itself is HOLPlayerOrders::Garrison (script).
// NOT WIRED to the brain yet: the summary carries no tower places, occupants or hit times, and the AI has no garrison order
// (TYPES_A_EXPOSER in Docs/Rapports/ELROND-1123c.md).

// One call: the tower (index into the towers given to Plan) and the ids of the units sent in, nearest first.
struct FHOLAIGarrisonCall
{
	int TowerIndex = -1;
	TArray<int> UnitIds;
}

namespace HOLAIGarrisonCall
{
	// STUB (red first): the plan read from the summary. TowerIndex is an index into Summary.Buildings.
	TArray<FHOLAIGarrisonCall> PlanFromSummary(int PlayerIndex, int Rule, EHOLAIDifficulty Level, const FHOLAISummary& Summary,
		const TArray<float>& SinceFirstHits, const TArray<float>& SinceLastHits, float Radius)
	{
		TArray<FHOLAIGarrisonCall> Calls;
		return Calls;
	}
	// For each tower that calls (HOLAIGarrison::TowerCalls inside Entrants), the units that enter (HOLAIGarrison::Entrants: in reach, of a kind
	// the rule calls, nearest first, admitted by the garrison rule). A unit that enters one tower is no longer offered to the next ones.
	TArray<FHOLAIGarrisonCall> Plan(int Rule, EHOLAIDifficulty Level, const TArray<FHOLAIGarrisonTower>& Towers, const TArray<int>& UnitIds,
		const TArray<int>& UnitSlots, const TArray<int>& UnitKinds, const TArray<FVector>& UnitPlaces, float Radius)
	{
		TArray<FHOLAIGarrisonCall> Calls;
		TArray<int> LeftIds = UnitIds;
		TArray<int> LeftSlots = UnitSlots;
		TArray<int> LeftKinds = UnitKinds;
		TArray<FVector> LeftPlaces = UnitPlaces;
		for (int TowerIndex = 0; TowerIndex < Towers.Num(); TowerIndex++)
		{
			const TArray<int> In = HOLAIGarrison::Entrants(Rule, Level, Towers[TowerIndex], LeftSlots, LeftKinds, LeftPlaces, Radius);
			if (In.Num() == 0)
				continue;
			FHOLAIGarrisonCall Call;
			Call.TowerIndex = TowerIndex;
			TArray<int> NextIds;
			TArray<int> NextSlots;
			TArray<int> NextKinds;
			TArray<FVector> NextPlaces;
			for (int Index = 0; Index < LeftIds.Num(); Index++)
			{
				if (!LeftSlots.IsValidIndex(Index) || !LeftKinds.IsValidIndex(Index) || !LeftPlaces.IsValidIndex(Index))
					continue;
				if (In.Contains(Index))
					continue;
				NextIds.Add(LeftIds[Index]);
				NextSlots.Add(LeftSlots[Index]);
				NextKinds.Add(LeftKinds[Index]);
				NextPlaces.Add(LeftPlaces[Index]);
			}
			// Nearest first: the order Entrants gave.
			for (int Each = 0; Each < In.Num(); Each++)
				Call.UnitIds.Add(LeftIds[In[Each]]);
			Calls.Add(Call);
			LeftIds = NextIds;
			LeftSlots = NextSlots;
			LeftKinds = NextKinds;
			LeftPlaces = NextPlaces;
		}
		return Calls;
	}
}
