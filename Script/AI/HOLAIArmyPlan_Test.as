// The army in the AI's needs, buildings and production (IA2.4), pure rules over summaries built by hand: the army's share of the spending,
// the resources it asks of the Peasants, the Barracks and the Archery Range the composition calls for, the units queued per building
// and the soldiers' rally point. The chain (summary -> script -> orders -> queues) is tested in C++ (HeroesOfLiberty.AI.Army.Production).
// The figures are the hand-made ones of HOLAIArmyRules_Test.as (the table numbers typed in).

// The figures of HOLAIArmyRules_Test.as, typed in again (each test file is a module of its own).
FHOLAITypeFigures PlanFigures(int Health, int Damage, float Interval, float Range, int MeleeArmor, int RangedArmor, bool bRanged, float Speed, int Food, int Wood, int Gold)
{
	FHOLAITypeFigures Figures;
	Figures.Health = Health;
	Figures.Damage = Damage;
	Figures.AttackInterval = Interval;
	Figures.AttackRange = Range;
	Figures.MeleeArmor = MeleeArmor;
	Figures.RangedArmor = RangedArmor;
	Figures.bRanged = bRanged;
	Figures.Speed = Speed;
	Figures.CostFood = Food;
	Figures.CostWood = Wood;
	Figures.CostGold = Gold;
	return Figures;
}

FHOLAITypeFigures PlanSpearman()
{
	FHOLAITypeFigures Figures = PlanFigures(80, 8, 1.875f, 130.f, 0, 0, false, 777.f, 60, 20, 0);
	Figures.ArmorCategories.Add(n"LightInfantry");
	Figures.BonusCategories.Add(n"Cavalry");
	Figures.BonusDamage.Add(17.f);
	return Figures;
}

FHOLAITypeFigures PlanManAtArms()
{
	FHOLAITypeFigures Figures = PlanFigures(155, 12, 1.375f, 70.f, 4, 4, false, 697.f, 100, 0, 20);
	Figures.ArmorCategories.Add(n"HeavyInfantry");
	Figures.ArmorCategories.Add(n"Heavy");
	return Figures;
}

FHOLAITypeFigures PlanArcher()
{
	FHOLAITypeFigures Figures = PlanFigures(70, 5, 1.625f, 2000.f, 0, 0, true, 779.f, 30, 50, 0);
	Figures.ArmorCategories.Add(n"Ranged");
	Figures.BonusCategories.Add(n"LightInfantry");
	Figures.BonusDamage.Add(5.f);
	Figures.BonusCategories.Add(n"GunpowderInfantry");
	Figures.BonusDamage.Add(5.f);
	return Figures;
}

FHOLAITypeFigures PlanCrossbowman()
{
	FHOLAITypeFigures Figures = PlanFigures(80, 12, 2.125f, 2000.f, 0, 0, true, 720.f, 80, 0, 40);
	Figures.ArmorCategories.Add(n"Ranged");
	Figures.BonusCategories.Add(n"Heavy");
	Figures.BonusDamage.Add(10.f);
	return Figures;
}

// The default data with the hand-made figures.
FHOLAIArmyData PlanHandData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Types.Empty();
	Data.Figures.Empty();
	Data.Types.Add(n"Spearman");
	Data.Figures.Add(PlanSpearman());
	Data.Types.Add(n"ManAtArms");
	Data.Figures.Add(PlanManAtArms());
	Data.Types.Add(n"Archer");
	Data.Figures.Add(PlanArcher());
	Data.Types.Add(n"Crossbowman");
	Data.Figures.Add(PlanCrossbowman());
	return Data;
}

void PlanAddUnits(TArray<FHOLAIUnit>& List, FName Type, int Count, int Health, int Slot)
{
	for (int Index = 0; Index < Count; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = 100 + List.Num();
		Unit.OwnerSlot = Slot;
		Unit.Type = Type;
		Unit.Health = Health;
		Unit.bIdle = true;
		List.Add(Unit);
	}
}

bool PlanNear(float A, float B)
{
	return Math::Abs(A - B) < 0.001f;
}

FHOLAISourceNear PlanSource(int SourceIndex, FVector Where)
{
	FHOLAISourceNear Source;
	Source.SourceIndex = SourceIndex;
	Source.Location = Where;
	Source.Remaining = 1000;
	return Source;
}

void PlanAddBuilding(FHOLAISummary& Summary, FName Type, EHOLConstructionState State, FVector Where)
{
	FHOLAIBuilding Building;
	Building.TypeName = Type;
	Building.State = State;
	Building.Location = Where;
	Summary.Buildings.Add(Building);
}

// A slot with 20 Peasants working (ids 1 to 20, none idle), its centre at the origin, a house, a lumber camp and a mill finished, 500 of
// each resource, pop 25 of 60, sources of every resource on the +X side, the enemy centre at (-8000, 0).
FHOLAISummary PlanSummary()
{
	FHOLAISummary Summary;
	Summary.Food.Stock = 500;
	Summary.Wood.Stock = 500;
	Summary.Stone.Stock = 500;
	Summary.Gold.Stock = 500;
	for (int Id = 1; Id <= 20; Id++)
	{
		Summary.PeasantIds.Add(Id);
	}
	Summary.Food.Sources.Add(PlanSource(10, FVector(1000, 0, 0)));
	Summary.Wood.Sources.Add(PlanSource(11, FVector(1000, 500, 0)));
	Summary.Stone.Sources.Add(PlanSource(12, FVector(1000, 900, 0)));
	Summary.Gold.Sources.Add(PlanSource(13, FVector(1000, -900, 0)));
	Summary.bHasPopulation = true;
	Summary.PopulationUsed = 25;
	Summary.PopulationCap = 60;
	Summary.bHasCentre = true;
	Summary.CentreLocation = FVector(0, 0, 0);
	Summary.bHasEnemyCentre = true;
	Summary.EnemyCentreLocation = FVector(-8000, 0, 0);
	PlanAddBuilding(Summary, n"MainCentre", EHOLConstructionState::Finished, FVector(0, 0, 0));
	PlanAddBuilding(Summary, n"House", EHOLConstructionState::Finished, FVector(1500, 0, 0));
	PlanAddBuilding(Summary, n"LumberCamp", EHOLConstructionState::Finished, FVector(2000, 500, 0));
	PlanAddBuilding(Summary, n"Mill", EHOLConstructionState::Finished, FVector(2000, -500, 0));
	return Summary;
}

FHOLAIOpeningData PlanData()
{
	FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	Data.Army = PlanHandData();
	return Data;
}

void PlanEnemies(FHOLAISummary& Summary, FName Type, int Count)
{
	PlanAddUnits(Summary.Enemies, Type, Count, 100000, 0);
	for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
	{
		Summary.Enemies[Index].Health = PlanHandData().Figures[PlanHandData().Types.FindIndex(Summary.Enemies[Index].Type)].Health;
	}
}

void PlanAddSoldiers(FHOLAISummary& Summary, FName Type, int Count)
{
	PlanAddUnits(Summary.Army, Type, Count, 1000, 1);
	const FHOLAIArmyData Hand = PlanHandData();
	for (int Index = 0; Index < Summary.Army.Num(); Index++)
	{
		Summary.Army[Index].Health = Hand.Figures[Hand.Types.FindIndex(Summary.Army[Index].Type)].Health;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------
// The army's share of the spending.

void Test_ArmyShareNoneBeforeTheEconomyStands(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	FHOLAIOpeningData Data = PlanData();
	Data.GameSeconds = 400.f;
	Summary.PeasantIds.Empty();
	for (int Id = 1; Id <= Data.ArmyMinPeasants; Id++)
	{
		Summary.PeasantIds.Add(Id);
	}
	T.AssertTrue(HOLAINeeds::ArmyEstablished(Summary, Data), "enough Peasants: the economy stands");
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.15f + 0.30f * 400.f / 600.f), "the base plus two thirds of the ramp");
	Summary.PeasantIds.RemoveAt(Data.ArmyMinPeasants - 1);
	T.AssertFalse(HOLAINeeds::ArmyEstablished(Summary, Data), "one Peasant short: not yet");
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.f), "no share before the economy stands, however late");
	// Peasants waiting in the centre's queue count (alive + queued).
	Summary.Buildings[0].Queue.Add(n"Peasant");
	T.AssertTrue(HOLAINeeds::ArmyEstablished(Summary, Data), "one short + 1 queued");
}

void Test_ArmyShareRisesWithTimeAndThreat(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	FHOLAIOpeningData Data = PlanData();
	Data.GameSeconds = 0.f;
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.15f), "the base at once");
	Data.GameSeconds = 300.f;
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.30f), "half way up the ramp");
	Data.GameSeconds = 5000.f;
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.45f), "the max, no more");

	// The enemy's army is stronger than ours (we have none): the bump, within the cap.
	PlanEnemies(Summary, n"ManAtArms", 5);
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.60f), "max 0.45 + bump 0.15 = the cap 0.6");
	Data.GameSeconds = 0.f;
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.30f), "base 0.15 + bump 0.15");

	// Our army is stronger: no bump.
	PlanAddSoldiers(Summary, n"Crossbowman", 20);
	T.AssertTrue(PlanNear(HOLAINeeds::ArmyShareOf(Summary, Data), 0.15f), "ahead of the enemy: the time share only");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The spending: the army asks for the resources of the units the composition calls for.

void Test_ArmyAsksTheResourcesOfItsUnits(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	FHOLAIOpeningData Data = PlanData();
	Data.PeasantTarget = 19;

	// No enemy seen: the fixed mix starts with Spearman and Archer, which cost no gold and no stone.
	FHOLAINeeds Needs = HOLAINeeds::Compute(Summary, Data);
	T.AssertTrue(Needs.Spend[0] > 0.f && Needs.Spend[1] > 0.f, "food and wood for Spearmen and Archers");
	T.AssertTrue(PlanNear(Needs.Spend[3], 0.f), "no gold in the Spearman and Archer mix");
	T.AssertTrue(PlanNear(Needs.Share[3], 0.f), "so no Peasant for the gold");

	// The same slot with no army share: the spending of the economy alone, smaller.
	FHOLAIOpeningData NoArmyData = Data;
	NoArmyData.ArmyMinPeasants = 99;
	FHOLAINeeds Economy = HOLAINeeds::Compute(Summary, NoArmyData);
	T.AssertTrue(Needs.Spend[0] + Needs.Spend[1] > Economy.Spend[0] + Economy.Spend[1], "the army adds to the spending");

	// Enemy Archers: Man-at-arms (20 gold) at the Barracks and Crossbowman (40 gold) at the Archery Range: gold is asked of the Peasants.
	PlanEnemies(Summary, n"Archer", 6);
	Needs = HOLAINeeds::Compute(Summary, Data);
	T.AssertTrue(Needs.Spend[3] > 0.f, "gold for the counters");
	T.AssertTrue(Needs.Share[3] > 0.f, "a share of the Peasants goes to the gold");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The buildings.

void Test_BarracksComesAfterHouseCampAndMill(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	FHOLAIOpeningData Data = PlanData();
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, Data);
	T.AssertTrue(Choice.bBuild, "a building is chosen");
	T.AssertTrue(Choice.Type == n"Barracks", "the fixed mix starts with the Spearman: the Barracks");
	T.AssertTrue(Choice.Anchor.X < 0.f && Math::Abs(Choice.Anchor.Y) < 1.f, "its spot is sought on the enemy side of the centre (-X)");
	T.AssertTrue(PlanNear(Choice.Anchor.Size(), Data.MilitaryDistance), "at the military distance from the centre");

	// No camp yet: the camp comes first.
	FHOLAISummary NoCamp = PlanSummary();
	NoCamp.Buildings.RemoveAt(2);
	T.AssertTrue(HOLAIOpening::ChooseBuild(NoCamp, Data).Type == n"LumberCamp", "the lumber camp before the Barracks");
	FHOLAISummary NoHouse = PlanSummary();
	NoHouse.Buildings.RemoveAt(1);
	T.AssertFalse(HOLAIOpening::ChooseBuild(NoHouse, Data).Type == n"Barracks", "no Barracks before the first house");
}

void Test_BarracksWaitsForTheWood(FUnitTest& T)
{
	FHOLAIOpeningData Data = PlanData();
	FHOLAISummary Poor = PlanSummary();
	Poor.Wood.Stock = 149;
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Poor, Data);
	T.AssertFalse(Choice.bBuild && Choice.Type == n"Barracks", "149 wood: not yet");
	Poor.Wood.Stock = 150;
	T.AssertTrue(HOLAIOpening::ChooseBuild(Poor, Data).Type == n"Barracks", "150 wood: the Barracks");

}

void Test_NoBarracksUnderFourteenPeasants(FUnitTest& T)
{
	FHOLAIOpeningData Data = PlanData();
	T.AssertEquals(14, Data.ArmyBuildPeasants, "the Barracks from 14 Peasants");
	FHOLAISummary Few = PlanSummary();
	while (Few.PeasantIds.Num() > Data.ArmyBuildPeasants - 1)
	{
		Few.PeasantIds.RemoveAt(Few.PeasantIds.Num() - 1);
	}
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Few, Data);
	T.AssertFalse(Choice.bBuild && (Choice.Type == n"Barracks" || Choice.Type == n"ArcheryRange"), "under 14 Peasants: no military building");
	Few.PeasantIds.Add(99);
	T.AssertTrue(HOLAIOpening::ChooseBuild(Few, Data).Type == n"Barracks", "14 Peasants: the Barracks");
}

// The wood is kept for the military building (the farms of the block would spend it 75 at a time, the Barracks needing 150 never came):
// with the food comfortable (stock >= SaveForArmyFood), no farm is ordered while the Barracks is wanted and the wood short; with the food
// short, the farm goes on.
void Test_FarmsWaitWhileTheWoodIsKeptForTheBarracks(FUnitTest& T)
{
	FHOLAIOpeningData Data = PlanData();
	FHOLAISummary Summary = PlanSummary();
	Summary.Wood.Stock = 100;
	Summary.Food.Sources[0].Remaining = 50;
	TArray<int> NoBusy;
	FHOLAIFarmPlan Plan;
	Plan.bActive = true;
	Plan.Mill = FVector(0, 4000, 0);
	Plan.Farms.Add(FVector(-3500, 5000, 0));
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, Data, Plan, NoBusy);
	T.AssertFalse(Choice.bBuild, "food in stock, the Barracks wanted, 100 wood: saving, no mill or farm");
	T.AssertEquals("saving wood", Choice.Nothing, "saving wood");
	Summary.Wood.Stock = 150;
	Choice = HOLAIOpening::ChooseBuild(Summary, Data, Plan, NoBusy);
	T.AssertTrue(Choice.bBuild && Choice.Type == n"Barracks", "150 wood: the Barracks");
	Summary.Wood.Stock = 100;
	Summary.Food.Stock = 50;
	Choice = HOLAIOpening::ChooseBuild(Summary, Data, Plan, NoBusy);
	T.AssertTrue(Choice.bBuild && (Choice.Type == n"Mill" || Choice.Type == n"Farm"), "the food is short: the block goes on");
}

void Test_ArcheryRangeWhenTheCompositionWantsArchers(FUnitTest& T)
{
	FHOLAIOpeningData Data = PlanData();
	FHOLAISummary Summary = PlanSummary();
	PlanAddBuilding(Summary, n"Barracks", EHOLConstructionState::Finished, FVector(-1500, 0, 0));

	// Enemy Men-at-arms: the Crossbowman counters them, so the Archery Range it needs.
	PlanEnemies(Summary, n"ManAtArms", 5);
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, Data);
	T.AssertTrue(Choice.bBuild && Choice.Type == n"ArcheryRange", "Crossbowmen against Men-at-arms: the Archery Range");

	// Enemy Archers: the Man-at-arms counters them; the Barracks stands: nothing to build.
	FHOLAISummary Archers = PlanSummary();
	PlanAddBuilding(Archers, n"Barracks", EHOLConstructionState::Foundation, FVector(-1500, 0, 0));
	PlanEnemies(Archers, n"Archer", 5);
	FHOLAIBuildChoice None = HOLAIOpening::ChooseBuild(Archers, Data);
	T.AssertFalse(None.bBuild, "Men-at-arms against Archers: the Barracks stands (a foundation counts), nothing to build");

	// Enemy Archers and no Barracks yet: the Barracks.
	FHOLAISummary Fresh = PlanSummary();
	PlanEnemies(Fresh, n"Archer", 5);
	T.AssertTrue(HOLAIOpening::ChooseBuild(Fresh, Data).Type == n"Barracks", "Men-at-arms against Archers: the Barracks");

	// A Range standing already when the mix calls for a Spearman: still the Barracks.
	FHOLAISummary Range = PlanSummary();
	PlanAddBuilding(Range, n"ArcheryRange", EHOLConstructionState::Finished, FVector(-1500, 0, 0));
	T.AssertTrue(HOLAIOpening::ChooseBuild(Range, Data).Type == n"Barracks", "the first of the mix is a Spearman: the Barracks");
}

void Test_NoMilitaryBuildingWhenBothStand(FUnitTest& T)
{
	FHOLAIOpeningData Data = PlanData();
	FHOLAISummary Summary = PlanSummary();
	PlanAddBuilding(Summary, n"Barracks", EHOLConstructionState::Finished, FVector(-1500, 0, 0));
	PlanAddBuilding(Summary, n"ArcheryRange", EHOLConstructionState::UnderConstruction, FVector(-1500, 1500, 0));
	FHOLAIBuildChoice Choice = HOLAIOpening::ChooseBuild(Summary, Data);
	T.AssertFalse(Choice.bBuild && (Choice.Type == n"Barracks" || Choice.Type == n"ArcheryRange"), "one of each: no second one now");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The rally point.

void Test_RallyPointInFrontOfTheCentre(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	FHOLAIOpeningData Data = PlanData();
	FVector Point = HOLAIOpening::RallyPointOf(Summary, Data);
	T.AssertTrue(PlanNear(Point.X, -1500.f) && PlanNear(Point.Y, 0.f), "1500 cm toward the enemy centre");
	Summary.bHasEnemyCentre = false;
	Point = HOLAIOpening::RallyPointOf(Summary, Data);
	T.AssertTrue(PlanNear(Point.X, 1500.f) && PlanNear(Point.Y, 0.f), "+X when the enemy centre is unknown");
	Summary.CentreLocation = FVector(1000, 2000, 0);
	Summary.bHasEnemyCentre = true;
	Summary.EnemyCentreLocation = FVector(1000, -3000, 0);
	Point = HOLAIOpening::RallyPointOf(Summary, Data);
	T.AssertTrue(PlanNear(Point.X, 1000.f) && PlanNear(Point.Y, 500.f), "from the centre, along the line to the enemy");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The production.

FHOLAISummary PlanWithBothBuildings()
{
	FHOLAISummary Summary = PlanSummary();
	PlanAddBuilding(Summary, n"Barracks", EHOLConstructionState::Finished, FVector(-1500, 0, 0));
	PlanAddBuilding(Summary, n"ArcheryRange", EHOLConstructionState::Finished, FVector(-1500, 1500, 0));
	return Summary;
}

int PlanOrderAt(const FHOLAIMilitaryChoice& Choice, FName Building)
{
	for (int Index = 0; Index < Choice.Orders.Num(); Index++)
	{
		if (Choice.Orders[Index].Building == Building)
		{
			return Index;
		}
	}
	return -1;
}

void Test_EachBuildingQueuesItsOwnUnit(FUnitTest& T)
{
	FHOLAISummary Summary = PlanWithBothBuildings();
	FHOLAIOpeningData Data = PlanData();
	FHOLAIMilitaryChoice Choice = HOLAIOpening::ChooseMilitary(Summary, Data, 0);
	T.AssertEquals(2, Choice.Orders.Num(), "an order per military building");
	const int Barracks = PlanOrderAt(Choice, n"Barracks");
	const int Range = PlanOrderAt(Choice, n"ArcheryRange");
	if (Barracks >= 0 && Range >= 0)
	{
		T.AssertTrue(Choice.Orders[Barracks].bQueue && Choice.Orders[Barracks].Unit == n"Spearman", "the Barracks: a Spearman (the mix's first)");
		T.AssertTrue(Choice.Orders[Range].bQueue && Choice.Orders[Range].Unit == n"Archer", "the Archery Range: an Archer, never a unit of the Barracks");
		T.AssertTrue(Choice.Orders[Barracks].Location.Equals(FVector(-1500, 0, 0)), "the Barracks named by its point");
	}
	T.AssertTrue(PlanNear(Choice.RallyPoint.X, -1500.f), "the rally point comes with the orders");
}

void Test_ProductionRestrictedByBuilding(FUnitTest& T)
{
	// Enemy Archers: the Man-at-arms (Barracks) and the Crossbowman (Archery Range) counter them.
	FHOLAISummary Summary = PlanWithBothBuildings();
	PlanEnemies(Summary, n"Archer", 6);
	FHOLAIMilitaryChoice Choice = HOLAIOpening::ChooseMilitary(Summary, PlanData(), 0);
	const int Barracks = PlanOrderAt(Choice, n"Barracks");
	const int Range = PlanOrderAt(Choice, n"ArcheryRange");
	T.AssertTrue(Barracks >= 0 && Range >= 0, "an order per building");
	if (Barracks >= 0 && Range >= 0)
	{
		T.AssertTrue(Choice.Orders[Barracks].Unit == n"ManAtArms", "Barracks: Man-at-arms against Archers");
		T.AssertTrue(Choice.Orders[Range].Unit == n"Crossbowman", "Archery Range: its own best, a Crossbowman, never a Man-at-arms");
	}
}

void Test_QueueLengthLimitsProduction(FUnitTest& T)
{
	FHOLAISummary Summary = PlanWithBothBuildings();
	Summary.Buildings[4].Queue.Add(n"Spearman");
	FHOLAIMilitaryChoice Choice = HOLAIOpening::ChooseMilitary(Summary, PlanData(), 0);
	const int Barracks = PlanOrderAt(Choice, n"Barracks");
	T.AssertTrue(Barracks >= 0, "the Barracks is in the orders");
	if (Barracks >= 0)
	{
		T.AssertTrue(Choice.Orders[Barracks].bQueue, "one waiting: a second may follow (2 at most)");
	}
	Summary.Buildings[4].Queue.Add(n"Spearman");
	Choice = HOLAIOpening::ChooseMilitary(Summary, PlanData(), 0);
	const int Full = PlanOrderAt(Choice, n"Barracks");
	T.AssertTrue(Full < 0 || !Choice.Orders[Full].bQueue, "two waiting: no more");
	const int Range = PlanOrderAt(Choice, n"ArcheryRange");
	T.AssertTrue(Range >= 0 && Choice.Orders[Range].bQueue, "the other building is not held back");
}

void Test_ProductionNeedsStockAndRoom(FUnitTest& T)
{
	FHOLAIOpeningData Data = PlanData();
	FHOLAISummary Poor = PlanWithBothBuildings();
	Poor.Food.Stock = 59;
	FHOLAIMilitaryChoice Choice = HOLAIOpening::ChooseMilitary(Poor, Data, 0);
	int Barracks = PlanOrderAt(Choice, n"Barracks");
	T.AssertTrue(Barracks < 0 || !Choice.Orders[Barracks].bQueue, "59 food: no Spearman (60)");
	const int Range = PlanOrderAt(Choice, n"ArcheryRange");
	T.AssertTrue(Range >= 0 && Choice.Orders[Range].bQueue, "an Archer (30 food, 50 wood) still pays");

	// The first order's cost is taken from the stock before the second: 100 food pays a Spearman (60) but then not an Archer (30)? it does; 80 food: not both.
	FHOLAISummary Short = PlanWithBothBuildings();
	Short.Food.Stock = 80;
	Choice = HOLAIOpening::ChooseMilitary(Short, Data, 0);
	int Queued = 0;
	for (int Index = 0; Index < Choice.Orders.Num(); Index++)
	{
		Queued += Choice.Orders[Index].bQueue ? 1 : 0;
	}
	T.AssertEquals(1, Queued, "80 food pays one of the two (60 + 30), not both");

	// Places: one left, a Peasant just queued by the thought: none for the army.
	FHOLAISummary Full = PlanWithBothBuildings();
	Full.PopulationUsed = 59;
	Choice = HOLAIOpening::ChooseMilitary(Full, Data, 0);
	Queued = 0;
	for (int Index = 0; Index < Choice.Orders.Num(); Index++)
	{
		Queued += Choice.Orders[Index].bQueue ? 1 : 0;
	}
	T.AssertEquals(1, Queued, "one place left: one unit");
	Choice = HOLAIOpening::ChooseMilitary(Full, Data, 1);
	Queued = 0;
	for (int Index = 0; Index < Choice.Orders.Num(); Index++)
	{
		Queued += Choice.Orders[Index].bQueue ? 1 : 0;
	}
	T.AssertEquals(0, Queued, "the place is taken by the Peasant just ordered: none");
}

void Test_NoProductionBeforeTheEconomyStands(FUnitTest& T)
{
	FHOLAISummary Summary = PlanWithBothBuildings();
	while (Summary.PeasantIds.Num() >= PlanData().ArmyMinPeasants)
	{
		Summary.PeasantIds.RemoveAt(Summary.PeasantIds.Num() - 1);
	}
	FHOLAIMilitaryChoice Choice = HOLAIOpening::ChooseMilitary(Summary, PlanData(), 0);
	for (int Index = 0; Index < Choice.Orders.Num(); Index++)
	{
		T.AssertFalse(Choice.Orders[Index].bQueue, "under the minimum: no soldier yet");
	}
	T.AssertEquals(2, Choice.Orders.Num(), "the buildings still get their rally");
}

void Test_UnfinishedMilitaryBuildingMakesNothing(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	PlanAddBuilding(Summary, n"Barracks", EHOLConstructionState::UnderConstruction, FVector(-1500, 0, 0));
	T.AssertEquals(0, HOLAIOpening::ChooseMilitary(Summary, PlanData(), 0).Orders.Num(), "a site gets neither a unit nor a rally");
}

void Test_WoodKeptForTheHouseWhenOneIsWanted(FUnitTest& T)
{
	FHOLAISummary Summary = PlanWithBothBuildings();
	Summary.PopulationUsed = 58;
	Summary.Wood.Stock = 60;
	FHOLAIOpeningData Data = PlanData();
	T.AssertTrue(HOLAIOpening::HouseWanted(Summary, Data), "the places are short: a house is wanted");
	FHOLAIMilitaryChoice Choice = HOLAIOpening::ChooseMilitary(Summary, Data, 0);
	const int Range = PlanOrderAt(Choice, n"ArcheryRange");
	T.AssertTrue(Range < 0 || !Choice.Orders[Range].bQueue, "the Archer (50 wood) would leave under the house's 50: kept");
}

// The margin of places counts the military buildings as producers (they fill the places like the centre does) once the economy stands.
void Test_HouseMarginCountsMilitaryProducers(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	FHOLAIOpeningData Data = PlanData();
	const int Alone = HOLAIOpening::HouseMargin(Summary, Data);
	PlanAddBuilding(Summary, n"Barracks", EHOLConstructionState::Finished, FVector(-1500, 0, 0));
	PlanAddBuilding(Summary, n"ArcheryRange", EHOLConstructionState::Finished, FVector(-1500, 1500, 0));
	T.AssertEquals(Alone * 2, HOLAIOpening::HouseMargin(Summary, Data), "the Barracks and the Range are two producers (the centre alone before)");
	while (Summary.PeasantIds.Num() >= Data.ArmyMinPeasants)
	{
		Summary.PeasantIds.RemoveAt(Summary.PeasantIds.Num() - 1);
	}
	T.AssertEquals(Alone, HOLAIOpening::HouseMargin(Summary, Data), "before the economy stands they produce nothing: the centre alone");
}

// The composition restricted to one building's units (IA2.4): the same rules among the candidates it makes.
void Test_CompositionRestrictedToABuilding(FUnitTest& T)
{
	FHOLAISummary Summary = PlanSummary();
	FHOLAIArmyData Data = PlanHandData();
	FHOLAIComposition Barracks = HOLAIArmy::ChooseComposition(Summary, Data, n"Barracks");
	FHOLAIComposition Range = HOLAIArmy::ChooseComposition(Summary, Data, n"ArcheryRange");
	T.AssertTrue(Barracks.bValid && Barracks.Type == n"Spearman" && Barracks.Building == n"Barracks", "fixed mix at the Barracks: a Spearman");
	T.AssertTrue(Range.bValid && Range.Type == n"Archer" && Range.Building == n"ArcheryRange", "fixed mix at the Range: an Archer");
	T.AssertTrue(HOLAIArmy::ChooseComposition(Summary, Data).Type == HOLAIArmy::ChooseComposition(Summary, Data, NAME_None).Type, "no filter = all");
	PlanEnemies(Summary, n"ManAtArms", 5);
	T.AssertTrue(HOLAIArmy::ChooseComposition(Summary, Data).Type == n"Crossbowman", "unrestricted: the Crossbowman against Men-at-arms");
	T.AssertTrue(HOLAIArmy::ChooseComposition(Summary, Data, n"Barracks").Building == n"Barracks", "restricted to the Barracks: one of its own");
	T.AssertFalse(HOLAIArmy::ChooseComposition(Summary, Data, n"Forum").bValid, "a building that makes nothing of the data: no choice");
}
