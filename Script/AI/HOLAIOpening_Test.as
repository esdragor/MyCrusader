// The opening rules alone (IA1.5): summaries built by hand, HOLAIOpening::ChooseBuild, ChooseProduction and SplitGather
// asked. The chain (C++ summary -> script -> orders -> queue) is tested in C++ (HeroesOfLiberty.AI.Opening.*).

FHOLAISourceNear OpeningSource(int SourceIndex, FVector Where)
{
	FHOLAISourceNear Source;
	Source.SourceIndex = SourceIndex;
	Source.Location = Where;
	Source.Remaining = 200;
	return Source;
}

// A slot with its finished centre at the origin, 5 Peasants (ids 1 to 5, all idle), pop 5 of 10, 200 of each resource, one
// source of food (#10) and of wood (#11) on the +X side.
FHOLAISummary OpeningSummary()
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
	Summary.Food.Sources.Add(OpeningSource(10, FVector(1000, 0, 0)));
	Summary.Wood.Sources.Add(OpeningSource(11, FVector(1000, 500, 0)));
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

FHOLAIOpeningData OpeningDataWithSteps()
{
	return HOLAIOpening::DefaultData();
}

void AddBuilding(FHOLAISummary& Summary, FName Type, EHOLConstructionState State)
{
	FHOLAIBuilding Building;
	Building.TypeName = Type;
	Building.State = State;
	Building.Location = FVector(-3000, 0, 0);
	Summary.Buildings.Add(Building);
}

// The default data is the opening of the game: lumber camp then mill, 3 food for 2 wood; the houses are a rule of their own.
void Test_DefaultDataIsTheOpening(FUnitTest& T)
{
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	T.AssertEquals(2, Data.Steps.Num(), "two buildings (the houses are repeated by their rule)");
	if (Data.Steps.Num() == 2)
	{
		T.AssertTrue(Data.Steps[0].Type == n"LumberCamp" && Data.Steps[0].NearResource == 1, "first the lumber camp near the wood");
		T.AssertTrue(Data.Steps[1].Type == n"Mill" && Data.Steps[1].NearResource == 0, "then the mill near the food");
	}
	T.AssertEquals(3, Data.WeightFood, "food weight");
	T.AssertEquals(2, Data.WeightWood, "wood weight");
	T.AssertEquals(8, Data.FarmCount, "a block of eight farms");
	T.AssertEquals(75, Data.Farm.CostWood, "the farm's wood, as the table has it");
}

// The 5 starting Peasants spread 3 on the food, 2 on the wood, each group to its resource's nearest source.
void Test_StartingPeasantsSpread(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Summary.IdlePeasants);
	T.AssertEquals(2, Groups.Num(), "two groups");
	if (Groups.Num() == 2)
	{
		T.AssertEquals(0, Groups[0].Resource, "first group: food");
		T.AssertEquals("Food", Groups[0].ResourceName, "first group's name");
		T.AssertEquals(10, Groups[0].SourceIndex, "first group: the food source");
		T.AssertEquals(3, Groups[0].Peasants.Num(), "three on food");
		T.AssertEquals(1, Groups[1].Resource, "second group: wood");
		T.AssertEquals(11, Groups[1].SourceIndex, "second group: the wood source");
		T.AssertEquals(2, Groups[1].Peasants.Num(), "two on wood");
		int Sent = 0;
		for (int Index = 0; Index < Groups.Num(); Index++)
		{
			Sent += Groups[Index].Peasants.Num();
		}
		T.AssertEquals(5, Sent, "every Peasant sent once");
	}
}

// Peasants already gathering count: with 3 on food and none on wood, two more go to the wood.
void Test_SplitCountsGatherers(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.Food.Peasants = 3;
	TArray<int> Free;
	Free.Add(4);
	Free.Add(5);
	TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Free);
	T.AssertEquals(1, Groups.Num(), "one group");
	if (Groups.Num() == 1)
	{
		T.AssertEquals(1, Groups[0].Resource, "wood");
		T.AssertEquals(2, Groups[0].Peasants.Num(), "both");
	}
}

// No source for the weighted resources: the lowest stock among those with a source (the IA0.4 rule).
void Test_SplitFallsBackToLowestStock(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.Food.Sources.Empty();
	Summary.Wood.Sources.Empty();
	Summary.Stone.Sources.Add(OpeningSource(12, FVector(500, 0, 0)));
	Summary.Gold.Sources.Add(OpeningSource(13, FVector(600, 0, 0)));
	Summary.Gold.Stock = 100;
	TArray<FHOLAIGatherGroup> Groups = HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Summary.IdlePeasants);
	T.AssertEquals(1, Groups.Num(), "one group");
	if (Groups.Num() == 1)
	{
		T.AssertEquals(3, Groups[0].Resource, "gold, the lowest stock");
		T.AssertEquals(5, Groups[0].Peasants.Num(), "all five");
	}
}

void Test_SplitNothingWithoutFreePeasant(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	TArray<int> Free;
	T.AssertEquals(0, HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Free).Num(), "no Peasant, no group");
	Summary.Food.Sources.Empty();
	Summary.Wood.Sources.Empty();
	T.AssertEquals(0, HOLAIOpening::SplitGather(Summary, HOLAIOpening::DefaultData(), Summary.IdlePeasants).Num(), "no source, no group");
}

// The margin is what the production brings in during the house's building: one producer, 20 s of house and 5 s of walk at
// 20 s a Peasant = 2 places; two producers, 4.
void Test_HouseMarginFollowsProduction(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	T.AssertEquals(2, HOLAIOpening::HouseMargin(Summary, HOLAIOpening::DefaultData()), "the centre alone: 2");
	Summary.Buildings[0].bProducing = true;
	T.AssertEquals(2, HOLAIOpening::HouseMargin(Summary, HOLAIOpening::DefaultData()), "the centre producing: 2");
	AddBuilding(Summary, n"Barracks", EHOLConstructionState::Finished);
	Summary.Buildings[1].bProducing = true;
	T.AssertEquals(4, HOLAIOpening::HouseMargin(Summary, HOLAIOpening::DefaultData()), "two producing: 4");
}

// The house waits until the places left fall to the margin (2); the places left count the Peasants waiting in a queue.
void Test_HouseWaitsForLowRoom(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.PopulationUsed = 7;
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild && Choice.Type == n"House", "3 places left: no house yet");

	// 8 used (one in production), none waiting: 2 left.
	Summary.PopulationUsed = 8;
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild, "2 places left: the house");
	T.AssertTrue(Choice.Type == n"House", "a house");
	T.AssertEquals(1, Choice.Builder, "built by the first idle Peasant");

	// 7 used, one more waiting in the queue behind the one in production: 10 - 7 - 1 = 2 left.
	Summary.PopulationUsed = 7;
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].bProducing = true;
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild && Choice.Type == n"House", "the Peasant waiting takes a place: the house");
}

// The houses come again: a finished house does not end the rule, a house under construction counts its places (no second one
// while they come), and none above the population's cap of the game.
void Test_HousesRepeat(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	AddBuilding(Summary, n"House", EHOLConstructionState::Finished);
	Summary.PopulationCap = 20;
	Summary.PopulationUsed = 18;
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild && Choice.Type == n"House", "a second house when the room is low again");

	AddBuilding(Summary, n"House", EHOLConstructionState::UnderConstruction);
	T.AssertEquals(10, HOLAIOpening::PlacesComing(Summary, OpeningDataWithSteps()), "a house under construction: 10 places coming");
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild && Choice.Type == n"House", "no third house while the second comes");

	Summary = OpeningSummary();
	Summary.PopulationCap = 175;
	Summary.PopulationUsed = 174;
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild && Choice.Type == n"House", "no house at the cap of 175");

	Summary.PopulationCap = 170;
	Summary.PopulationUsed = 169;
	AddBuilding(Summary, n"House", EHOLConstructionState::Foundation);
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild && Choice.Type == n"House", "170 + 10 coming reach the cap: no more");
}

// The house goes on the side of the centre away from the nearest food and wood, 1500 cm out.
void Test_HouseAwayFromSources(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.Food.Sources[0].Location = FVector(1000, 0, 0);
	Summary.Wood.Sources[0].Location = FVector(1000, 0, 0);
	Summary.PopulationUsed = 8;
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild, "the house");
	T.AssertTrue(Math::Abs(Choice.Anchor.X + 1500.0) < 1.0, "1500 cm to the -X side");
	T.AssertTrue(Math::Abs(Choice.Anchor.Y) < 1.0, "on the axis");

	// A centre elsewhere: the anchor is measured from it.
	Summary.CentreLocation = FVector(5000, 2000, 0);
	Summary.Food.Sources[0].Location = FVector(5000, 3000, 0);
	Summary.Wood.Sources[0].Location = FVector(5000, 3000, 0);
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Math::Abs(Choice.Anchor.X - 5000.0) < 1.0 && Math::Abs(Choice.Anchor.Y - 500.0) < 1.0, "1500 cm to the -Y side of the centre");
}

// Not enough wood: nothing, and the house is not skipped for the next building.
void Test_HouseNeedsWood(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.PopulationUsed = 8;
	Summary.Wood.Stock = 49;
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild, "49 wood: no house");
	T.AssertEquals("no wood", Choice.Nothing, "why");
}

// The lumber camp is first, beside the wood; then the mill, beside the food; then done.
void Test_OpeningGoesInOrder(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild, "the lumber camp");
	T.AssertTrue(Choice.Type == n"LumberCamp", "a lumber camp");
	T.AssertTrue(Math::Abs(Choice.Anchor.X - 1000.0) < 1.0 && Math::Abs(Choice.Anchor.Y - 500.0) < 1.0, "beside the nearest wood");

	AddBuilding(Summary, n"LumberCamp", EHOLConstructionState::UnderConstruction);
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild && Choice.Type == n"Mill", "the mill");
	T.AssertTrue(Math::Abs(Choice.Anchor.X - 1000.0) < 1.0 && Math::Abs(Choice.Anchor.Y) < 1.0, "beside the nearest food");

	AddBuilding(Summary, n"Mill", EHOLConstructionState::Finished);
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild, "opening done");
	T.AssertEquals("done", Choice.Nothing, "why");
}

// A camp with no source of its resource, or not enough wood, or nobody to build: nothing, with the reason. A step with no
// source is skipped (the next one may have its source).
void Test_BuildNothingReasons(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.Wood.Sources.Empty();
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild && Choice.Type == n"Mill", "no wood source: the mill goes first");

	AddBuilding(Summary, n"Mill", EHOLConstructionState::Finished);
	T.AssertEquals("no source", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "mill built, no wood source: nothing");

	Summary = OpeningSummary();
	Summary.Wood.Stock = 99;
	T.AssertEquals("no wood", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "99 wood for a camp");

	Summary = OpeningSummary();
	Summary.IdlePeasants.Empty();
	Summary.PeasantIds.Empty();
	T.AssertEquals("no builder", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "no Peasant");

	Summary = OpeningSummary();
	Summary.bHasCentre = false;
	T.AssertEquals("no centre", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "no centre");
}

// None idle: the builder is the Peasant with the highest identifier (the latest born, nearest the centre).
void Test_BuilderWithoutIdle(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.IdlePeasants.Empty();
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild, "a camp");
	T.AssertEquals(5, Choice.Builder, "the latest Peasant");
}

// The centre queues a Peasant when it has room in its queue, in the population and the food.
void Test_CentreProducesPeasants(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	FHOLAIProduceChoice Choice = HOLAIOpening::ChooseProduction(Summary, HOLAIOpening::DefaultData());
	T.AssertTrue(Choice.bProduce, "a Peasant");
	T.AssertTrue(Choice.Centre.Equals(FVector(0, 0, 0), 1.0), "at the centre");
	T.AssertEquals(0, Choice.RallyResource, "the rally: food, the biggest deficit");
	T.AssertEquals(10, Choice.RallySource, "the rally: the food source");

	// The rally follows the deficit: 3 on food, none on wood.
	Summary.Food.Peasants = 3;
	Choice = HOLAIOpening::ChooseProduction(Summary, HOLAIOpening::DefaultData());
	T.AssertEquals(1, Choice.RallyResource, "the rally: wood");
	T.AssertEquals(11, Choice.RallySource, "the rally: the wood source");
}

void Test_ProductionStops(FUnitTest& T)
{
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();

	// Queue of two (one in production): full.
	FHOLAISummary Summary = OpeningSummary();
	Summary.PopulationUsed = 6;
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].bProducing = true;
	T.AssertEquals("queue full", HOLAIOpening::ChooseProduction(Summary, Data).Nothing, "two in the queue");

	// No room: 10 of 10.
	Summary = OpeningSummary();
	Summary.PopulationUsed = 10;
	T.AssertEquals("no room", HOLAIOpening::ChooseProduction(Summary, Data).Nothing, "population full");

	// The Peasant waiting in the queue takes the last place: 9 used (one in production) + 1 waiting = 10.
	Summary = OpeningSummary();
	Summary.PopulationUsed = 9;
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].bProducing = true;
	Data.QueueLength = 3;
	T.AssertEquals("no room", HOLAIOpening::ChooseProduction(Summary, Data).Nothing, "the waiting Peasant takes the last place");
	Data.QueueLength = 2;

	// No food.
	Summary = OpeningSummary();
	Summary.Food.Stock = 49;
	T.AssertEquals("no food", HOLAIOpening::ChooseProduction(Summary, Data).Nothing, "49 food");

	// Target reached: 5 Peasants alive, target 5.
	Summary = OpeningSummary();
	Data.PeasantTarget = 5;
	T.AssertEquals("target reached", HOLAIOpening::ChooseProduction(Summary, Data).Nothing, "target");
	Data.PeasantTarget = 80;

	// Centre not finished.
	Summary = OpeningSummary();
	Summary.Buildings[0].State = EHOLConstructionState::UnderConstruction;
	T.AssertEquals("no centre", HOLAIOpening::ChooseProduction(Summary, Data).Nothing, "centre not finished");
}

// The sites under way are few: at the limit of the data (3) nothing new but a house, and a house is not stopped by it.
void Test_SitesAreLimited(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	AddBuilding(Summary, n"House", EHOLConstructionState::UnderConstruction);
	AddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Foundation);
	T.AssertEquals(2, HOLAIOpening::SitesUnderWay(Summary), "two sites under way");
	T.AssertTrue(HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).bBuild, "two sites: the mill still goes");
	AddBuilding(Summary, n"Farm", EHOLConstructionState::Foundation);
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild, "three sites: nothing more");
	T.AssertEquals("busy", Choice.Nothing, "why");

	// The house is not held back: room low, no house coming (the sites under way are not houses).
	Summary = OpeningSummary();
	AddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Foundation);
	AddBuilding(Summary, n"Mill", EHOLConstructionState::Foundation);
	AddBuilding(Summary, n"Farm", EHOLConstructionState::Foundation);
	Summary.PopulationUsed = 9;
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild && Choice.Type == n"House", "the house goes whatever the sites");
}

// The builders already sent to a site are not sent again: the next idle one, else the next Peasant from the latest.
void Test_BuilderSkipsTheBusyOnes(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	TArray<int> Busy;
	Busy.Add(1);
	Busy.Add(2);
	FHOLAIFarmPlan Plan;
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, Busy);
	T.AssertTrue(Choice.bBuild, "a camp");
	T.AssertEquals(3, Choice.Builder, "the first idle one not busy");

	Summary.IdlePeasants.Empty();
	Busy.Empty();
	Busy.Add(5);
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, Busy);
	T.AssertEquals(4, Choice.Builder, "none idle: the latest born that is not busy");

	Busy.Empty();
	for (int Id = 1; Id <= 5; Id++)
	{
		Busy.Add(Id);
	}
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, Busy);
	T.AssertEquals("no builder", Choice.Nothing, "all busy");
}

// With the camp and the mill built, a camp comes again when the nearest wood is far from every camp (the first massif is cut):
// beside the nearest tree, so at the edge of the massif; not while it is near, not for a resource nobody needs.
void Test_FarCampAtTheEdgeOfTheMassif(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	AddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Finished);
	AddBuilding(Summary, n"Mill", EHOLConstructionState::Finished);
	T.AssertEquals("done", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "wood near its camp: nothing");

	Summary.Wood.Sources[0].DepositDistance = 3000.f;
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild && Choice.Type == n"LumberCamp", "wood far from the camp: a second camp");
	T.AssertTrue(Math::Abs(Choice.Anchor.X - 1000.0) < 1.0 && Math::Abs(Choice.Anchor.Y - 500.0) < 1.0, "beside the nearest wood");

	// Stone far from the centre but nobody asks for stone (no spending on it): no mining camp.
	Summary.Wood.Sources[0].DepositDistance = 300.f;
	Summary.Stone.Sources.Add(OpeningSource(12, FVector(5000, 0, 0)));
	Summary.Stone.Sources[0].DepositDistance = 5000.f;
	T.AssertEquals("done", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "no mining camp for stone nobody needs");

	// A camp under construction already: the far wood waits for it.
	Summary.Wood.Sources[0].DepositDistance = 3000.f;
	AddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Foundation);
	T.AssertFalse(HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).bBuild, "a camp is coming: no other");
}

// The berries last less than the lead (240 s at the pace of their gatherers): the farm block is to be planned, from the centre.
void Test_FarmsWhenTheBerriesGoDown(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	AddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Finished);
	AddBuilding(Summary, n"Mill", EHOLConstructionState::Finished);
	Summary.Food.Peasants = 3;
	Summary.Food.Sources[0].Remaining = 100000;
	T.AssertFalse(HOLAIOpening::FarmsWanted(Summary, HOLAIOpening::DefaultData()), "plenty of berries: no farms");
	T.AssertEquals("done", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "plenty of berries: done");

	Summary.Food.Sources[0].Remaining = 100;
	T.AssertTrue(HOLAIOpening::FarmsWanted(Summary, HOLAIOpening::DefaultData()), "berries low: farms");
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild, "no order before the block is planned");
	T.AssertTrue(Choice.bPlanFarms, "the block is to be planned");

	// No berries at all: farms too.
	Summary.Food.Sources.Empty();
	T.AssertTrue(HOLAIOpening::FarmsWanted(Summary, HOLAIOpening::DefaultData()), "no food source: farms");
	// Not without the wood to pay a mill: the plan waits.
	Summary.Wood.Stock = 40;
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bPlanFarms, "no plan without wood");
}

FHOLAIFarmPlan MakePlan()
{
	FHOLAIFarmPlan Plan;
	Plan.bActive = true;
	Plan.Mill = FVector(0, 4000, 0);
	for (int Index = 0; Index < 8; Index++)
	{
		Plan.Farms.Add(FVector(float(Index) * 1000.f - 3500.f, 5000, 0));
	}
	return Plan;
}

void AddBuildingAt(FHOLAISummary& Summary, FName Type, EHOLConstructionState State, FVector Where)
{
	FHOLAIBuilding Building;
	Building.TypeName = Type;
	Building.State = State;
	Building.Location = Where;
	Summary.Buildings.Add(Building);
}

// The plan in order: its mill first, then its farms one by one at the planned spots; a farm built (any state) is skipped; all
// nine built: done. The wood is the farm's (75).
void Test_FarmBlockInOrder(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	AddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Finished);
	AddBuilding(Summary, n"Mill", EHOLConstructionState::Finished);
	Summary.Food.Peasants = 3;
	Summary.Food.Sources[0].Remaining = 100;
	TArray<int> NoBusy;
	FHOLAIFarmPlan Plan = MakePlan();
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, NoBusy);
	T.AssertTrue(Choice.bBuild && Choice.Type == n"Mill" && Choice.bAtSpot, "the block's mill first, at its spot");
	T.AssertTrue(Choice.Spot.Equals(Plan.Mill, 1.0), "at the planned spot");

	AddBuildingAt(Summary, n"Mill", EHOLConstructionState::Finished, Plan.Mill);
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, NoBusy);
	T.AssertTrue(Choice.bBuild && Choice.Type == n"Farm" && Choice.bAtSpot, "then a farm");
	T.AssertTrue(Choice.Spot.Equals(Plan.Farms[0], 1.0), "the first planned farm");

	AddBuildingAt(Summary, n"Farm", EHOLConstructionState::Finished, Plan.Farms[0]);
	AddBuildingAt(Summary, n"Farm", EHOLConstructionState::Foundation, Plan.Farms[1]);
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, NoBusy);
	T.AssertTrue(Choice.bBuild && Choice.Spot.Equals(Plan.Farms[2], 1.0), "the third farm after two (a foundation counts)");

	Summary.Wood.Stock = 74;
	T.AssertEquals("no wood", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, NoBusy).Nothing, "75 wood a farm");
	Summary.Wood.Stock = 500;

	// The foundation of the second farm done: only three sites... the plan goes on to the end.
	Summary.Buildings[Summary.Buildings.Num() - 1].State = EHOLConstructionState::Finished;
	for (int Index = 2; Index < 8; Index++)
	{
		AddBuildingAt(Summary, n"Farm", EHOLConstructionState::Finished, Plan.Farms[Index]);
	}
	T.AssertEquals("done", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps(), Plan, NoBusy).Nothing, "all nine: done");
}

// The farms the plan wants count in the spending (75 wood each) while the berries are low, and not when they last.
void Test_FarmsCountInTheSpending(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.Food.Sources[0].Remaining = 100000;
	TArray<float> Plenty = HOLAINeeds::SpendOf(Summary, HOLAIOpening::DefaultData());
	Summary.Food.Sources[0].Remaining = 100;
	TArray<float> Low = HOLAINeeds::SpendOf(Summary, HOLAIOpening::DefaultData());
	T.AssertTrue(OpeningNear(Low[1] - Plenty[1], 600.f), "8 farms of 75 wood more");
	AddBuilding(Summary, n"Farm", EHOLConstructionState::Finished);
	AddBuilding(Summary, n"Farm", EHOLConstructionState::Foundation);
	TArray<float> Some = HOLAINeeds::SpendOf(Summary, HOLAIOpening::DefaultData());
	T.AssertTrue(OpeningNear(Low[1] - Some[1], 150.f), "two built: 6 farms left to pay");
}

bool OpeningNear(float A, float B)
{
	return Math::Abs(A - B) < 0.01f;
}

// A lone Peasant does not leave its work to build: below two Peasants nobody is sent, whatever the site.
void Test_NobodyBuildsWithTooFewPeasants(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.IdlePeasants.Empty();
	Summary.PeasantIds.Empty();
	Summary.IdlePeasants.Add(1);
	Summary.PeasantIds.Add(1);
	T.AssertEquals("no builder", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "one Peasant: the camp waits");
	Summary.PopulationUsed = 9;
	T.AssertEquals("no builder", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "one Peasant: not even the house");
	Summary.IdlePeasants.Add(2);
	Summary.PeasantIds.Add(2);
	T.AssertTrue(HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).bBuild, "two Peasants: the house");
}
