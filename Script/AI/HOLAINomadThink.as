// Nomad option in the AI's brain (N3, NomadInProgress.md): the thought of a Nomad AI seat before its first complete forum (the C++ summary's
// bNomadBeforeFirstForum). It founds that forum by HOLAINomad::Plan (HOLAINomadRules.as, hol.AI.NomadForumRule, default A of
// Retours/Q-nomad-ia.md): a MainCentre foundation at a free spot near the chosen group, all its Peasants sent to build it (Q2 A: no
// gathering first); while that foundation stands unfinished, its idle Peasants go and build it. Tested on the real chain by
// HeroesOfLiberty.AI.Nomad.* (Source/HeroesOfLiberty/Tests/HOLAINomadTests.cpp).
namespace HOLAINomadThink
{
	// One thought of the Nomad seat; the brain thinks of nothing else until the first forum is complete (no centre to produce from).
	void Think(int PlayerIndex, int64 Step, const FHOLAISummary& Summary, int Rings)
	{
		bool bUnderway = false;
		FVector Foundation = FVector::ZeroVector;
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			if (Summary.Buildings[Index].TypeName == n"MainCentre")
			{
				bUnderway = true;
				Foundation = Summary.Buildings[Index].Location;
				break;
			}
		}
		if (bUnderway)
		{
			if (Summary.IdlePeasants.Num() > 0 && HOLAIOrder::IssueAIBuildOrder(PlayerIndex, Foundation, Summary.IdlePeasants))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {Summary.IdlePeasants.Num()} idle Peasants build the Nomad forum");
			}
			return;
		}
		TArray<FVector> Places;
		TArray<int> Ids;
		for (int Index = 0; Index < Summary.Peasants.Num(); Index++)
		{
			Places.Add(Summary.Peasants[Index].Location);
			Ids.Add(Summary.Peasants[Index].Id);
		}
		TArray<int> Stock;
		Stock.Add(Summary.Food.Stock);
		Stock.Add(Summary.Wood.Stock);
		Stock.Add(Summary.Stone.Stock);
		Stock.Add(Summary.Gold.Stock);
		const FHOLAINomadPlan Plan = HOLAINomad::Plan(HOLAINomad::RuleFromCVar(), true, false, Places, Stock, Summary.ForumPrice,
			HOLAINomad::GroupRadiusFromCVar());
		if (!Plan.bFound)
			return;
		FVector Spot;
		if (!HOLAIOrder::FindAIPlacement(PlayerIndex, n"MainCentre", Plan.Site, Rings, Spot))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: no spot for the Nomad forum near ({Plan.Site.X}, {Plan.Site.Y})");
			return;
		}
		if (HOLAIOrder::IssueAIPlaceOrder(PlayerIndex, n"MainCentre", Spot, Ids))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: Nomad forum founded at ({Spot.X}, {Spot.Y}), group {Plan.Group}, {Ids.Num()} Peasants sent");
		}
		else
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: Nomad forum refused at ({Spot.X}, {Spot.Y})");
		}
	}
}
