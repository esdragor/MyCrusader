// Sacred sites, pure rule (S1; GDD 14.3 Sacred sites, D697 to D703). STUB written with the red test (HOLSiteRules_Test.as): the signatures,
// no behaviour yet.
namespace HOLSite
{
	const int StepsPerSecond = HOLHero::GetSimulationRateHz();
	const int None = -1;
	const int HoldSeconds = 60;
	const int DecaySteps = 2;

	int HoldSteps()
	{
		return HoldSeconds * StepsPerSecond;
	}

	struct FHOLSiteState
	{
		int Owner = -1;
		int Taker = -1;
		int Held = 0;
	}

	void ResetCounts(TArray<int>& Takers, TArray<int>& Disturbers, int Slots)
	{
		Takers.Reset();
		Disturbers.Reset();
		for (int Slot = 0; Slot < Slots; Slot++)
		{
			Takers.Add(0);
			Disturbers.Add(0);
		}
	}

	void AddUnit(TArray<int>& Takers, TArray<int>& Disturbers, int Slot, bool bCanTake, bool bIsPeasant)
	{
	}

	FHOLSiteState Step(const FHOLSiteState& State, const TArray<int>& Takers, const TArray<int>& Disturbers, const TArray<int>& TeamBySlot)
	{
		return State;
	}
}
