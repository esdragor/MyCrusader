// The AI's garrison exit (S4-b, D696 Q2 A): which of the AI's towers let their occupants out in this think. Pure function on the
// summary, built on HOLAIGarrison::OccupantsLeave (HOLAIGarrisonRules.as). The order itself (UngarrisonAll of the tower, owner = the AI's slot)
// is NOT here: it needs a C++ binding HOLAIGarrisonOrder::IssueAIUngarrisonOrder (TYPES_A_EXPOSER of FARAMIR-1124se).

namespace HOLAIGarrisonExit
{
	// Indices into Summary.Buildings of the slot's finished towers with occupants whose calm lasted CalmSeconds. SinceLastHits is aligned to
	// Summary.Buildings (empty, short or < 0 = not given: the tower's own SecondsSinceHit is read, -1 there = never hit = calm).
	TArray<int> TowersToEmpty(int PlayerIndex, const FHOLAISummary& Summary, const TArray<float>& SinceLastHits, float CalmSeconds)
	{
		TArray<int> Out;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			const FHOLAIBuilding Building = Summary.Buildings[Index];
			if (Building.OwnerPlayer != PlayerIndex || Building.State != EHOLConstructionState::Finished || Building.GarrisonSlots <= 0 || Building.Occupants <= 0)
				continue;
			float SinceLastHit = SinceLastHits.IsValidIndex(Index) ? SinceLastHits[Index] : -1.f;
			if (SinceLastHit < 0.f)
				SinceLastHit = Building.SecondsSinceHit;
			if (HOLAIGarrison::OccupantsLeave(SinceLastHit, CalmSeconds))
				Out.Add(Index);
		}
		return Out;
	}
}
