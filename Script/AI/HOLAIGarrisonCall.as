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
	// The plan read from the summary: the slot's finished buildings with places are the towers, its units with a read garrison kind the
	// candidates. TowerIndex of a call is an index into Summary.Buildings. SinceFirstHits / SinceLastHits are aligned to Summary.Buildings
	// (empty or short = -1 = not given: the tower's own Building.SecondsSinceHit is read instead, -1 there = never hit = calm).
	TArray<FHOLAIGarrisonCall> PlanFromSummary(int PlayerIndex, int Rule, EHOLAIDifficulty Level, const FHOLAISummary& Summary,
		const TArray<float>& SinceFirstHits, const TArray<float>& SinceLastHits, float Radius)
	{
		TArray<FHOLAIGarrisonTower> Towers;
		TArray<int> BuildingIndices;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			const FHOLAIBuilding Building = Summary.Buildings[Index];
			if (Building.OwnerPlayer != PlayerIndex || Building.State != EHOLConstructionState::Finished || Building.GarrisonSlots <= 0)
				continue;
			FHOLAIGarrisonTower Tower;
			Tower.Slot = PlayerIndex;
			Tower.Place = Building.Location;
			Tower.bFinished = true;
			Tower.Slots = Building.GarrisonSlots;
			Tower.Occupants = Building.Occupants;
			Tower.SinceFirstHit = SinceFirstHits.IsValidIndex(Index) ? SinceFirstHits[Index] : -1.f;
			Tower.SinceLastHit = SinceLastHits.IsValidIndex(Index) ? SinceLastHits[Index] : -1.f;
				// S4-b read: no hit time given, the summary's own reader (AHOLBuilding last hit) says when the tower was last hit. The AI has
				// no reader of the FIRST hit yet, so the reaction delay of the level (HOLAIGarrison::ReactSeconds) is not measurable and
				// counts as elapsed: a tower hit within hol.AI.GarrisonAttackedSeconds calls at once.
				if (Tower.SinceLastHit < 0.f && Building.SecondsSinceHit >= 0.f)
				{
					Tower.SinceLastHit = Building.SecondsSinceHit;
					if (Tower.SinceFirstHit < 0.f)
						Tower.SinceFirstHit = Math::Max(Building.SecondsSinceHit, HOLAIGarrison::ReactSeconds(Level));
				}
			Towers.Add(Tower);
			BuildingIndices.Add(Index);
		}
		TArray<int> UnitIds;
		TArray<int> UnitSlots;
		TArray<int> UnitKinds;
		TArray<FVector> UnitPlaces;
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			const FHOLAIUnit Unit = Summary.Army[Index];
			if (Unit.GarrisonKind == -1)
				continue;
			UnitIds.Add(Unit.Id);
			UnitSlots.Add(Unit.OwnerSlot);
			UnitKinds.Add(Unit.GarrisonKind);
			UnitPlaces.Add(Unit.Location);
		}
		TArray<FHOLAIGarrisonCall> Calls = Plan(Rule, Level, Towers, UnitIds, UnitSlots, UnitKinds, UnitPlaces, Radius);
		for (int Each = 0; Each < Calls.Num(); Each++)
			Calls[Each].TowerIndex = BuildingIndices[Calls[Each].TowerIndex];
		return Calls;
	}

	// S4-b wired: the AI's garrison orders of this think (HOLAIGarrisonOrder::IssueAIGarrisonOrder, the server checks owner, kind and places);
	// returns how many were queued.
	int Issue(int PlayerIndex, int Rule, EHOLAIDifficulty Level, const FHOLAISummary& Summary, const TArray<float>& SinceFirstHits,
		const TArray<float>& SinceLastHits)
	{
		const TArray<FHOLAIGarrisonCall> Calls = PlanFromSummary(PlayerIndex, Rule, Level, Summary, SinceFirstHits, SinceLastHits, HOLAIGarrison::RadiusFromCVar());
		int Issued = 0;
		for (int Each = 0; Each < Calls.Num(); Each++)
		{
			AHOLBuilding Tower = Summary.Buildings[Calls[Each].TowerIndex].Actor;
			if (Tower == nullptr)
				continue;
			if (HOLAIGarrisonOrder::IssueAIGarrisonOrder(PlayerIndex, Calls[Each].UnitIds, Tower))
				Issued++;
		}
		return Issued;
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
