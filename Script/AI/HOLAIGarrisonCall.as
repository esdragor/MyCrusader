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
	// BOUCHON (red commit): nobody is ever called.
	TArray<FHOLAIGarrisonCall> Plan(int Rule, EHOLAIDifficulty Level, const TArray<FHOLAIGarrisonTower>& Towers, const TArray<int>& UnitIds,
		const TArray<int>& UnitSlots, const TArray<int>& UnitKinds, const TArray<FVector>& UnitPlaces, float Radius)
	{
		TArray<FHOLAIGarrisonCall> Calls;
		return Calls;
	}
}
