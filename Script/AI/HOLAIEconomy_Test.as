// The economy rule of the AI brain, alone (IA0.4 B3, IA0.4-Q2 A): summaries built by hand, HOLAIEconomy::ChooseGather
// asked. The chain (C++ summary -> script -> order -> queue -> processors) is tested in C++ (HeroesOfLiberty.AI.Brain.*).

// A source of the summary, by its index in the source subsystem.
FHOLAISourceNear MakeSourceNear(int SourceIndex)
{
	FHOLAISourceNear Source;
	Source.SourceIndex = SourceIndex;
	return Source;
}

// A summary with these stocks (food, wood, stone, gold), the given idle Peasants and no source yet.
FHOLAISummary MakeEconomySummary(int Food, int Wood, int Stone, int Gold, const TArray<int>& IdlePeasants)
{
	FHOLAISummary Summary;
	Summary.Food.Stock = Food;
	Summary.Wood.Stock = Wood;
	Summary.Stone.Stock = Stone;
	Summary.Gold.Stock = Gold;
	Summary.IdlePeasants = IdlePeasants;
	return Summary;
}

// One source per resource: food 10, wood 11, stone 12, gold 13.
void AddOneSourceEach(FHOLAISummary& Summary)
{
	Summary.Food.Sources.Add(MakeSourceNear(10));
	Summary.Wood.Sources.Add(MakeSourceNear(11));
	Summary.Stone.Sources.Add(MakeSourceNear(12));
	Summary.Gold.Sources.Add(MakeSourceNear(13));
}

TArray<int> OneIdlePeasant()
{
	TArray<int> Idle;
	Idle.Add(5);
	return Idle;
}

void AssertGather(FUnitTest& T, const FHOLAIGatherChoice& Choice, int Resource, FString ResourceName, int SourceIndex,
	FString Case)
{
	T.AssertTrue(Choice.bGather, f"{Case}: an order");
	T.AssertEquals(Resource, Choice.Resource, f"{Case}: resource");
	T.AssertEquals(ResourceName, Choice.ResourceName, f"{Case}: resource name");
	T.AssertEquals(SourceIndex, Choice.SourceIndex, f"{Case}: source");
	T.AssertTrue(Choice.Nothing.IsEmpty(), f"{Case}: no reason for nothing");
}

void AssertNothing(FUnitTest& T, const FHOLAIGatherChoice& Choice, FString Reason, FString Case)
{
	T.AssertFalse(Choice.bGather, f"{Case}: no order");
	T.AssertEquals(Reason, Choice.Nothing, f"{Case}: why nothing");
	T.AssertEquals(-1, Choice.Resource, f"{Case}: no resource");
	T.AssertEquals(-1, Choice.SourceIndex, f"{Case}: no source");
	T.AssertEquals(0, Choice.Peasants.Num(), f"{Case}: no Peasant sent");
}

// The lowest stock among the resources with a source is chosen.
void Test_LowestStockChosen(FUnitTest& T)
{
	FHOLAISummary Summary = MakeEconomySummary(200, 100, 300, 400, OneIdlePeasant());
	AddOneSourceEach(Summary);
	AssertGather(T, HOLAIEconomy::ChooseGather(Summary), 1, "Wood", 11, "wood lowest");

	Summary = MakeEconomySummary(200, 300, 400, 50, OneIdlePeasant());
	AddOneSourceEach(Summary);
	AssertGather(T, HOLAIEconomy::ChooseGather(Summary), 3, "Gold", 13, "gold lowest");
}

// A tie goes to the first in the order food, wood, stone, gold.
void Test_TieInOrderFoodWoodStoneGold(FUnitTest& T)
{
	FHOLAISummary Summary = MakeEconomySummary(200, 200, 200, 200, OneIdlePeasant());
	AddOneSourceEach(Summary);
	AssertGather(T, HOLAIEconomy::ChooseGather(Summary), 0, "Food", 10, "all equal");

	Summary = MakeEconomySummary(300, 100, 200, 100, OneIdlePeasant());
	AddOneSourceEach(Summary);
	AssertGather(T, HOLAIEconomy::ChooseGather(Summary), 1, "Wood", 11, "wood and gold tied");

	Summary = MakeEconomySummary(300, 300, 100, 100, OneIdlePeasant());
	AddOneSourceEach(Summary);
	AssertGather(T, HOLAIEconomy::ChooseGather(Summary), 2, "Stone", 12, "stone and gold tied");
}

// The lowest stock without a source is passed over for the next lowest that has one.
void Test_LowestWithoutSourceSkipped(FUnitTest& T)
{
	FHOLAISummary Summary = MakeEconomySummary(200, 50, 100, 300, OneIdlePeasant());
	Summary.Food.Sources.Add(MakeSourceNear(10));
	Summary.Stone.Sources.Add(MakeSourceNear(12));
	Summary.Gold.Sources.Add(MakeSourceNear(13));
	AssertGather(T, HOLAIEconomy::ChooseGather(Summary), 2, "Stone", 12, "wood lowest without a source");
}

// Idle Peasants but no source at all: nothing.
void Test_NoSourceNothing(FUnitTest& T)
{
	TArray<int> Idle;
	Idle.Add(1);
	Idle.Add(2);
	FHOLAISummary Summary = MakeEconomySummary(200, 200, 200, 200, Idle);
	AssertNothing(T, HOLAIEconomy::ChooseGather(Summary), "no source", "no source");
}

// Sources but no idle Peasant: nothing.
void Test_NoIdleNothing(FUnitTest& T)
{
	TArray<int> Idle;
	FHOLAISummary Summary = MakeEconomySummary(200, 100, 200, 200, Idle);
	AddOneSourceEach(Summary);
	AssertNothing(T, HOLAIEconomy::ChooseGather(Summary), "no idle", "no idle Peasant");
}

// All the idle Peasants go, in one order, to the resource's first source (its nearest).
void Test_AllIdleToFirstSource(FUnitTest& T)
{
	TArray<int> Idle;
	Idle.Add(3);
	Idle.Add(7);
	Idle.Add(9);
	FHOLAISummary Summary = MakeEconomySummary(100, 200, 200, 200, Idle);
	Summary.Food.Sources.Add(MakeSourceNear(21));
	Summary.Food.Sources.Add(MakeSourceNear(22));
	Summary.Food.Sources.Add(MakeSourceNear(23));
	Summary.Wood.Sources.Add(MakeSourceNear(11));

	FHOLAIGatherChoice Choice = HOLAIEconomy::ChooseGather(Summary);
	AssertGather(T, Choice, 0, "Food", 21, "three idle, three food sources");
	T.AssertEquals(3, Choice.Peasants.Num(), "every idle Peasant sent");
	if (Choice.Peasants.Num() == 3)
	{
		T.AssertEquals(3, Choice.Peasants[0], "first idle Peasant");
		T.AssertEquals(7, Choice.Peasants[1], "second idle Peasant");
		T.AssertEquals(9, Choice.Peasants[2], "third idle Peasant");
	}
}
