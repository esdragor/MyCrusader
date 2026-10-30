// The needs rules alone (IA1.6): summaries built by hand, HOLAINeeds::Compute and the opening's SplitGather / ChooseProduction
// asked. The chain (C++ summary and type tables -> script -> orders) is tested in C++ (HeroesOfLiberty.AI.Needs.*).

bool NeedsNear(float A, float B)
{
	return Math::Abs(A - B) < 0.01f;
}

FHOLAISourceNear NeedsSource(int SourceIndex, FVector Where)
{
	FHOLAISourceNear Source;
	Source.SourceIndex = SourceIndex;
	Source.Location = Where;
	Source.Remaining = 200;
	return Source;
}

// A slot with its finished centre at the origin, 5 Peasants (ids 1 to 5, all idle), pop 5 of 10, 200 of each resource, one
// source of food (#10) and of wood (#11) on the +X side.
FHOLAISummary NeedsSummary()
{
	FHOLAISummary Summary;
	Summary.Food.Stock = 200;
	Summary.Wood.Stock = 200;
	Summary.Stone.Stock = 200;
	Summary.Gold.Stock = 200;
	for (int Id = 1; Id <= 5; Id++)
	{
		Summary.IdlePeasants.Add(Id);
		Summary.PeasantIds.Add(Id);
	}
	Summary.Food.Sources.Add(NeedsSource(10, FVector(1000, 0, 0)));
	Summary.Wood.Sources.Add(NeedsSource(11, FVector(1000, 500, 0)));
	Summary.bHasPopulation = true;
	Summary.PopulationUsed = 5;
	Summary.PopulationCap = 10;
	Summary.bHasCentre = true;
	Summary.CentreLocation = FVector(0, 0, 0);
	FHOLAIBuilding Centre;
	Centre.TypeName = n"MainCentre";
	Centre.State = EHOLConstructionState::Finished;
	Centre.Location = FVector(0, 0, 0);
	Summary.Buildings.Add(Centre);
	return Summary;
}

void NeedsAddBuilding(FHOLAISummary& Summary, FName Type, EHOLConstructionState State)
{
	FHOLAIBuilding Building;
	Building.TypeName = Type;
	Building.State = State;
	Building.Location = FVector(-3000, 0, 0);
	Summary.Buildings.Add(Building);
}

// The centre makes a Peasant every 20 s (150 food over the minute); 3 Peasants a minute need 3 / 10 of a House (15 wood); the
// next building of the list is the lumber camp (100); plus the reserve (30).
void Test_SpendOverTheMinute(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData());
	T.AssertTrue(NeedsNear(Needs.Spend[0], 150.f), "food: 3 Peasants of 50");
	T.AssertTrue(NeedsNear(Needs.Spend[1], 145.f), "wood: 15 for the houses + 100 for the camp + 30 reserve");
	T.AssertTrue(NeedsNear(Needs.Spend[2], 0.f), "no stone");
	T.AssertTrue(NeedsNear(Needs.Spend[3], 0.f), "no gold");

	// With the camp placed, the mill is the next one; with the opening done, nothing more of the list.
	NeedsAddBuilding(Summary, n"House", EHOLConstructionState::Finished);
	NeedsAddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Foundation);
	T.AssertTrue(NeedsNear(HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData()).Spend[1], 145.f), "the mill is next: 100 as well");
	NeedsAddBuilding(Summary, n"Mill", EHOLConstructionState::Foundation);
	T.AssertTrue(NeedsNear(HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData()).Spend[1], 45.f), "opening done: houses and reserve");
}

// At the Peasant target the centre stops: no more food for Peasants, no more wood for their houses.
void Test_NoSpendAtTheTarget(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	Data.PeasantTarget = 5;
	FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, Data);
	T.AssertTrue(NeedsNear(Needs.Spend[0], 0.f), "no food for Peasants");
	T.AssertTrue(NeedsNear(Needs.Spend[1], 130.f), "wood: the camp and the reserve only");
	T.AssertTrue(NeedsNear(Needs.Share[1], 1.f), "all the Peasants on the wood");
}

// A stock in hand pays part of the spending, but never more than leaves a quarter of it to the sources.
void Test_StockLowersTheDemand(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	Summary.Food.Stock = 0;
	T.AssertTrue(NeedsNear(HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData()).Demand[0], 150.f), "no stock: the whole spending");
	Summary.Food.Stock = 100;
	T.AssertTrue(NeedsNear(HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData()).Demand[0], 100.f), "100 in stock: half of it counts");
	Summary.Food.Stock = 10000;
	T.AssertTrue(NeedsNear(HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData()).Demand[0], 37.5f), "a big stock: a quarter of the spending");
}

// What a Peasant brings home: its rate when the deposit is at the source, less when the round trip is long.
void Test_NetRateCountsTheTrip(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	T.AssertTrue(NeedsNear(HOLAINeeds::NetRate(Data, Summary, 0), 0.7f), "deposit at the source: 0.7 per second");
	Summary.Food.Sources[0].DepositDistance = 8000.f;
	// 10 carried: 10 / 0.7 = 14.29 s to gather + 20 s of walk: 0.2916 per second.
	T.AssertTrue(Math::Abs(HOLAINeeds::NetRate(Data, Summary, 0) - 0.2916f) < 0.001f, "a far deposit: 0.29 per second");
	T.AssertTrue(NeedsNear(HOLAINeeds::NetRate(Data, Summary, 2), 0.f), "no stone source: 0");
}

// A far food source asks for more Peasants than a near one for the same spending.
void Test_FarSourceAsksMorePeasants(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	const float Close = HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData()).Wanted[0];
	Summary.Food.Sources[0].DepositDistance = 8000.f;
	const float Far = HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData()).Wanted[0];
	T.AssertTrue(Far > Close * 2.f, "far food: more than twice the Peasants");
}

// The shares sum to 1 over the resources with a source and follow the needs.
void Test_SharesFollowTheNeeds(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData());
	T.AssertFalse(Needs.bFromWeights, "from the needs");
	T.AssertTrue(NeedsNear(Needs.Share[0] + Needs.Share[1], 1.f), "food and wood share the Peasants");
	T.AssertTrue(Needs.Share[0] > Needs.Share[1], "more on the food");
	T.AssertTrue(NeedsNear(Needs.Share[2], 0.f) && NeedsNear(Needs.Share[3], 0.f), "none without a source");

	// No wood in stock, plenty of food: the wood asks the Peasants.
	Summary.Wood.Stock = 0;
	Summary.Food.Stock = 1000;
	Needs = HOLAINeeds::Compute(Summary, HOLAIOpening::DefaultData());
	T.AssertTrue(Needs.Share[1] > Needs.Share[0] * 2.f, "wood short, food in plenty: the wood");
}

// The five starting Peasants: 3 on the food, 2 on the wood with the stocks of the start (as the opening's weights did) ...
void Test_StartingPeasantsByTheNeeds(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Summary.IdlePeasants);
	T.AssertEquals(2, Groups.Num(), "two groups");
	if (Groups.Num() == 2)
	{
		T.AssertEquals(3, Groups[0].Peasants.Num(), "three on food");
		T.AssertEquals(2, Groups[1].Peasants.Num(), "two on wood");
	}
}

// ... and the spread follows the lack: no wood, plenty of food: 1 on the food, 4 on the wood.
void Test_SplitFollowsTheLack(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	Summary.Wood.Stock = 0;
	Summary.Food.Stock = 1000;
	TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Summary.IdlePeasants);
	T.AssertEquals(2, Groups.Num(), "two groups");
	if (Groups.Num() == 2)
	{
		T.AssertEquals(0, Groups[0].Resource, "food first");
		T.AssertEquals(1, Groups[0].Peasants.Num(), "one on food");
		T.AssertEquals(4, Groups[1].Peasants.Num(), "four on wood");
	}
}

// A source used up leaves the summary: its Peasants, idle, go to what is left.
void Test_UsedUpSourceSendsItsPeasantsOn(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	Summary.Food.Sources.Empty();
	Summary.Food.Peasants = 3;
	TArray<int> Free;
	Free.Add(1);
	Free.Add(2);
	Free.Add(3);
	TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Free);
	T.AssertEquals(1, Groups.Num(), "one group");
	if (Groups.Num() == 1)
	{
		T.AssertEquals(1, Groups[0].Resource, "the wood, the only source left");
		T.AssertEquals(3, Groups[0].Peasants.Num(), "all three");
	}
}

// With nothing asked of any source (no centre, nothing planned, no reserve), the weights of the data decide.
void Test_WeightsWhenNothingIsAsked(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	Summary.bHasCentre = false;
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	Data.ReserveWood = 0.f;
	Data.Steps.Empty();
	FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, Data);
	T.AssertTrue(Needs.bFromWeights, "weights");
	T.AssertTrue(NeedsNear(Needs.Share[0], 0.6f) && NeedsNear(Needs.Share[1], 0.4f), "3 food for 2 wood");
}

// The rally of the new Peasants follows the lack: the wood when the wood is short.
void Test_RallyFollowsTheLack(FUnitTest& T)
{
	FHOLAISummary Summary = NeedsSummary();
	Summary.Wood.Stock = 0;
	Summary.Food.Stock = 1000;
	Summary.Food.Peasants = 1;
	FHOLAIProduceChoice Choice = HOLAIOpening::ChooseProduction(Summary, HOLAIOpening::DefaultData());
	T.AssertTrue(Choice.bProduce, "a Peasant");
	T.AssertEquals(1, Choice.RallyResource, "the wood");
	T.AssertEquals(11, Choice.RallySource, "its source");
}

// The Peasants are produced up to the target (80 here, GDD 5.3: 70 to 90), counting the ones in a queue.
void Test_ProductionUpToTheTarget(FUnitTest& T)
{
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	T.AssertTrue(Data.PeasantTarget >= 70 && Data.PeasantTarget <= 90, "the target is in the GDD's range");
	FHOLAISummary Summary = NeedsSummary();
	Summary.PeasantIds.Empty();
	for (int Id = 1; Id <= 79; Id++)
	{
		Summary.PeasantIds.Add(Id);
	}
	Summary.PopulationCap = 175;
	Summary.PopulationUsed = 79;
	T.AssertTrue(HOLAIOpening::ChooseProduction(Summary, Data).bProduce, "79 Peasants: one more");
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].bProducing = true;
	T.AssertEquals("target reached", HOLAIOpening::ChooseProduction(Summary, Data).Nothing, "79 alive and one in production: 80");
}
