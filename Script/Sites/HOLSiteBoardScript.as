// The server's sacred-site board (MIG-CPP-Site, D836 point 3), the script side of UHOLSiteBoardSubsystem: the C++ part (UHOLSiteSubsystem::OnStepEnded)
// counts the units of each site in its Mass walk (HOLSite::ResetCounts / AddUnit stay C++: per unit, in the Mass walk) and calls this board ONCE per site and per step with the
// counts already aggregated, then once per step for the victory count. The rules are HOLSiteRules.as (HOLSite::Step) and HOLSiteVictory.as
// (HOLSiteVictory::Step), the script is the source. The structs of the bridge are the C++ ones (global FHOLSiteState, FHOLSiteVictoryState); the
// rules' own are HOLSite::FHOLSiteState and HOLSiteVictory::FHOLSiteVictoryState. Server and solo only. Tested by HeroesOfLiberty.Sites.ServerBoard.
class UHOLSiteBoardScript : UHOLSiteBoardSubsystem
{
	UFUNCTION(BlueprintOverride)
	FHOLSiteState StepSiteRule(const FHOLSiteState& State, const TArray<int>& Takers, const TArray<int>& Disturbers, const TArray<int>& TeamBySlot, int HoldSteps, int DecaySteps)
	{
		HOLSite::FHOLSiteState In;
		In.Owner = State.Owner;
		In.Taker = State.Taker;
		In.Held = State.Held;
		const HOLSite::FHOLSiteState Out = HOLSite::Step(In, Takers, Disturbers, TeamBySlot, HoldSteps, DecaySteps);
		FHOLSiteState Result;
		Result.Owner = Out.Owner;
		Result.Taker = Out.Taker;
		Result.Held = Out.Held;
		return Result;
	}

	UFUNCTION(BlueprintOverride)
	FHOLSiteVictoryState VictoryStepRule(const FHOLSiteVictoryState& State, const TArray<int>& OwnerBySite, const TArray<int>& ContestedBySite, const TArray<int>& TeamBySlot, int VictorySteps)
	{
		HOLSiteVictory::FHOLSiteVictoryState In;
		In.Leader = State.Leader;
		In.Held = State.Held;
		In.Winner = State.Winner;
		In.LastOwners = State.LastOwners;
		const HOLSiteVictory::FHOLSiteVictoryState Out = HOLSiteVictory::Step(In, OwnerBySite, ContestedBySite, TeamBySlot, VictorySteps);
		FHOLSiteVictoryState Result;
		Result.Leader = Out.Leader;
		Result.Held = Out.Held;
		Result.Winner = Out.Winner;
		Result.LastOwners = Out.LastOwners;
		return Result;
	}

	UFUNCTION(BlueprintOverride)
	int GoldStepRule(int& Carry, int Sites, int RatePerMinute)
	{
		return HOLSiteVictory::GoldStepWith(Carry, Sites, RatePerMinute);
	}
}
