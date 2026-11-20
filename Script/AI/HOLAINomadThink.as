// Nomad option in the AI's brain (N3, NomadInProgress.md): the thought of a Nomad AI seat before its first complete forum (the C++ summary's
// bNomadBeforeFirstForum). It founds that forum by HOLAINomad::Plan (HOLAINomadRules.as, hol.AI.NomadForumRule, default A of
// Retours/Q-nomad-ia.md): a MainCentre foundation at a free spot near the chosen group, ONE Peasant sent to place it, the idle ones joining it up to the cap (D817 A + A: no
// gathering first); while that foundation stands unfinished, the idle Peasants join it, nearest first. Tested on the real chain by
// HeroesOfLiberty.AI.Nomad.* (Source/HeroesOfLiberty/Tests/HOLAINomadTests.cpp).
namespace HOLAINomadThink
{
	// The Peasants already at work on the foundation: not idle, within Radius of it (Think has no memory, the ones at work near the
	// foundation are the builders it sent; a Peasant gathering elsewhere is not one).
	TArray<int> BuildersNear(FVector Foundation, const TArray<FHOLAIUnit>& Peasants, float Radius)
	{
		TArray<int> Builders;
		for (int Index = 0; Index < Peasants.Num(); Index++)
		{
			if (!Peasants[Index].bIdle && HOLAINomad::FlatDistance(Foundation, Peasants[Index].Location) <= Radius)
				Builders.Add(Peasants[Index].Id);
		}
		return Builders;
	}

	// The Peasants that join the foundation at this thought (D817 A + A): HOLAINomad::HelpersFor with the builders near the foundation
	// as Assigned, so the builders in all never pass MaxBuilders.
	TArray<int> JoinersFor(FVector Foundation, const FHOLAISummary& Summary, int MaxBuilders, float Radius)
	{
		return HOLAINomad::HelpersFor(Foundation, Summary.Peasants, BuildersNear(Foundation, Summary.Peasants, Radius), MaxBuilders);
	}

	// The Peasant that places the forum (D817 A: one builder, the others join later): the nearest to the spot, -1 without a Peasant.
	int FirstBuilderFor(FVector Spot, const TArray<FHOLAIUnit>& Peasants)
	{
		int Best = -1;
		float BestDistance = 0.f;
		for (int Index = 0; Index < Peasants.Num(); Index++)
		{
			const float Distance = HOLAINomad::FlatDistance(Spot, Peasants[Index].Location);
			if (Best == -1 || Distance < BestDistance)
			{
				Best = Peasants[Index].Id;
				BestDistance = Distance;
			}
		}
		return Best;
	}
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
			const TArray<int> Joiners = JoinersFor(Foundation, Summary, HOLAINomad::MaxBuildersFromCVar(), HOLAINomad::GroupRadiusFromCVar());
			if (Joiners.Num() > 0 && HOLAIOrder::IssueAIBuildOrder(PlayerIndex, Foundation, Joiners))
			{
				Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: {Joiners.Num()} idle Peasants join the Nomad forum");
			}
			return;
		}
		TArray<FVector> Places;
		for (int Index = 0; Index < Summary.Peasants.Num(); Index++)
		{
			Places.Add(Summary.Peasants[Index].Location);
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
		// D817 A: one builder places the forum (the nearest Peasant); the idle ones join it at the next thoughts, up to the cap.
		TArray<int> Ids;
		const int First = FirstBuilderFor(Spot, Summary.Peasants);
		if (First != -1)
			Ids.Add(First);
		if (HOLAIOrder::IssueAIPlaceOrder(PlayerIndex, n"MainCentre", Spot, Ids))
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: Nomad forum founded at ({Spot.X}, {Spot.Y}), group {Plan.Group}, builder {First} sent");
		}
		else
		{
			Log(n"LogHOLMatch", f"AI slot {PlayerIndex} step {Step}: Nomad forum refused at ({Spot.X}, {Spot.Y})");
		}
	}
}
