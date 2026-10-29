// The opening rules alone (IA1.5): summaries built by hand, HOLAIOpening::ChooseBuild, ChooseProduction and SplitGather
// asked. The chain (C++ summary -> script -> orders -> queue) is tested in C++ (HeroesOfLiberty.AI.Opening.*).

FHOLAISourceNear OpeningSource(int SourceIndex, FVector Where)
{
	FHOLAISourceNear Source;
	Source.SourceIndex = SourceIndex;
	Source.Location = Where;
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

// The default data is the opening of the game: house, lumber camp, mill, 3 food for 2 wood.
void Test_DefaultDataIsTheOpening(FUnitTest& T)
{
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	T.AssertEquals(3, Data.Steps.Num(), "three buildings");
	if (Data.Steps.Num() == 3)
	{
		T.AssertTrue(Data.Steps[0].Type == n"House" && Data.Steps[0].bWhenRoomLow, "first the house, when the room is low");
		T.AssertTrue(Data.Steps[1].Type == n"LumberCamp" && Data.Steps[1].NearResource == 1, "then the lumber camp near the wood");
		T.AssertTrue(Data.Steps[2].Type == n"Mill" && Data.Steps[2].NearResource == 0, "then the mill near the food");
	}
	T.AssertEquals(3, Data.WeightFood, "food weight");
	T.AssertEquals(2, Data.WeightWood, "wood weight");
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

// The house waits until the places left fall to the margin; the places left count the Peasants waiting in a queue.
void Test_HouseWaitsForLowRoom(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertFalse(Choice.bBuild, "5 places left: no house yet");
	T.AssertEquals("not yet", Choice.Nothing, "why");

	// 6 used (one in production), one more waiting: 10 - 6 - 1 = 3 left.
	Summary.PopulationUsed = 6;
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].Queue.Add(n"Peasant");
	Summary.Buildings[0].bProducing = true;
	Choice = HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps());
	T.AssertTrue(Choice.bBuild, "3 places left: the house");
	T.AssertTrue(Choice.Type == n"House", "a house");
	T.AssertEquals(1, Choice.Builder, "built by the first idle Peasant");
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

// With a house (a foundation counts) the lumber camp is next, beside the wood; then the mill, beside the food; then done.
void Test_OpeningGoesInOrder(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	AddBuilding(Summary, n"House", EHOLConstructionState::Foundation);
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

// A camp with no source of its resource, or not enough wood, or nobody to build: nothing, with the reason.
void Test_BuildNothingReasons(FUnitTest& T)
{
	FHOLAISummary Summary = OpeningSummary();
	AddBuilding(Summary, n"House", EHOLConstructionState::Finished);
	Summary.Wood.Sources.Empty();
	T.AssertEquals("no source", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "no wood source");

	Summary = OpeningSummary();
	AddBuilding(Summary, n"House", EHOLConstructionState::Finished);
	Summary.Wood.Stock = 99;
	T.AssertEquals("no wood", HOLAIOpening::ChooseBuild(Summary, OpeningDataWithSteps()).Nothing, "99 wood for a camp");

	Summary = OpeningSummary();
	AddBuilding(Summary, n"House", EHOLConstructionState::Finished);
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
	AddBuilding(Summary, n"House", EHOLConstructionState::Finished);
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
