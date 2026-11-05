// Sacred sites, victory count and gold by the minute (S10, S6). STUB written with the red test (HOLSiteVictory_Test.as): the signatures, no
// behaviour yet.
namespace HOLSiteVictory
{
	const int None = -1;
	const int VictorySeconds = 300;

	int VictorySteps()
	{
		return 0;
	}

	struct FHOLSiteVictoryState
	{
		int Leader = -1;
		int Held = 0;
		int Winner = -1;
		TArray<int> LastOwners;
	}

	FHOLSiteVictoryState Step(const FHOLSiteVictoryState& State, const TArray<int>& OwnerBySite, const TArray<int>& ContestedBySite, const TArray<int>& TeamBySlot)
	{
		return State;
	}

	struct FHOLGoldCarry
	{
		int Carry = 0;
	}

	int GoldStep(FHOLGoldCarry& Memory, int Sites, int RatePerMinute)
	{
		return 0;
	}
}
