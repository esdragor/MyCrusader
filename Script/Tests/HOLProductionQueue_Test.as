// L17-12 rest (GLORFINDEL-1117g, D830/D839): port of the rest of HOLProductionTests.cpp: HeroesOfLiberty.Production.Queue, the queue half of
// HeroesOfLiberty.Production.Advance and HeroesOfLiberty.Production.Tables, against HOLProductionRead::TryEnqueue / Cancel / Advance
// (FHOLProductionQueueState, one cost for every type) and HOLProductionTables (flat rows). Same vectors as the C++ tests, names adapted.
// These compile only once the plugin is rebuilt with the C++ wrappers (UHOLProductionReadLibrary additions, UHOLProductionTablesLibrary):
// the red (a value faked once) is to be seen after that build; the C++ tests are removed only after red then green.

FHOLResources ProdResources(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Result;
	Result.Food = Food;
	Result.Wood = Wood;
	Result.Stone = Stone;
	Result.Gold = Gold;
	return Result;
}

// 50 food, 4 steps, 1 place (MakeCost of the C++ test). EHOLProductionRefusal: 0 None, 1 NotProducible, 2 QueueFull, 3 NotEnoughResources.
// J3 / D282: the cost is taken when queued and given back in full when cancelled; a refusal changes nothing.
void Test_Production_Queue(FUnitTest& T)
{
	FHOLResources Cost = ProdResources(50, 0, 0, 0);
	FHOLProductionQueueState Queue;
	FHOLResources Stock = ProdResources(120, 0, 0, 0);

	T.AssertEquals(0, HOLProductionRead::TryEnqueue(Queue, n"Peasant", Cost, 4, 1, 5, Stock), "first queued");
	T.AssertEquals(0, HOLProductionRead::TryEnqueue(Queue, n"Peasant", Cost, 4, 1, 5, Stock), "second queued");
	T.AssertEquals(20, Stock.Food, "cost taken when queued");
	T.AssertEquals(-1, int(Queue.StartSteps[0]), "a queued item waits");
	T.AssertEquals(3, HOLProductionRead::TryEnqueue(Queue, n"Peasant", Cost, 4, 1, 5, Stock), "third refused: 20 food left");
	T.AssertEquals(2, Queue.Types.Num(), "a refusal changes nothing");
	T.AssertEquals(20, Stock.Food, "stock unchanged by a refusal");

	FHOLResources Plenty = ProdResources(1000, 0, 0, 0);
	T.AssertEquals(2, HOLProductionRead::TryEnqueue(Queue, n"Peasant", Cost, 4, 1, 2, Plenty), "queue of 2 at most");
	T.AssertEquals(1000, Plenty.Food, "nothing taken from a full queue");

	T.AssertTrue(HOLProductionRead::Cancel(Queue, 0, Cost, 4, 1, Stock), "cancel the first");
	T.AssertEquals(70, Stock.Food, "cancel gives the cost back in full");
	T.AssertEquals(1, Queue.Types.Num(), "one item left");
	T.AssertFalse(HOLProductionRead::Cancel(Queue, 3, Cost, 4, 1, Stock), "nothing to cancel past the queue");
	T.AssertEquals(70, Stock.Food, "stock unchanged by a wrong cancel");
}

// D829: the head starts whatever the population; done but no place, it stays at 100 % and the next waits; a place frees it, the next starts at once.
void Test_Production_AdvanceQueue(FUnitTest& T)
{
	FHOLResources Cost = ProdResources(50, 0, 0, 0);
	FHOLProductionQueueState Queue;
	FHOLResources Stock = ProdResources(1000, 0, 0, 0);
	HOLProductionRead::TryEnqueue(Queue, n"Peasant", Cost, 4, 1, 5, Stock);
	HOLProductionRead::TryEnqueue(Queue, n"Peasant", Cost, 4, 1, 5, Stock);
	TArray<FName> Done;

	int FreePopulation = 0;
	HOLProductionRead::Advance(Queue, 10, Cost, 4, 1, FreePopulation, Done);
	T.AssertEquals(10, int(Queue.StartSteps[0]), "head started at step 10, population full");
	T.AssertEquals(0, FreePopulation, "no place taken while in production");
	T.AssertEquals(-1, int(Queue.StartSteps[1]), "only the head is produced");

	HOLProductionRead::Advance(Queue, 14, Cost, 4, 1, FreePopulation, Done);
	T.AssertEquals(0, Done.Num(), "done but no place: nothing comes out");
	T.AssertEquals(-1, int(Queue.StartSteps[1]), "the next one waits behind it");

	FreePopulation = 5;
	HOLProductionRead::Advance(Queue, 15, Cost, 4, 1, FreePopulation, Done);
	T.AssertEquals(1, Done.Num(), "one unit done");
	T.AssertEquals(1, Queue.Types.Num(), "one item left");
	T.AssertEquals(15, int(Queue.StartSteps[0]), "the next one starts at once");
	T.AssertEquals(4, FreePopulation, "the unit out takes its place");

	HOLProductionRead::Advance(Queue, 19, Cost, 4, 1, FreePopulation, Done);
	T.AssertEquals(2, Done.Num(), "both units done");
	T.AssertEquals(0, Queue.Types.Num(), "queue empty");
}

// EHOLArmorCategory values.
const int ProdCatLightInfantry = 0;
const int ProdCatHeavyInfantry = 1;
const int ProdCatRanged = 2;
const int ProdCatCavalry = 3;
const int ProdCatPeasant = 5;
const int ProdCatGunpowder = 6;
const int ProdCatHeavy = 7;

// J3 / J7 / D283: the provisional values of the tables (units, Barracks, Archery range, main centre, starting stock).
void Test_Production_Tables(FUnitTest& T)
{
	FHOLUnitTypeSummary Peasant;
	bool bPeasant = HOLProductionTables::FindUnitType(n"Peasant", Peasant);
	T.AssertTrue(bPeasant, "Peasant in the unit type table");
	if (bPeasant)
	{
		T.AssertEquals(50, Peasant.Cost.Food, "50 food (D283)");
		T.AssertEquals(0, Peasant.Cost.Wood + Peasant.Cost.Stone + Peasant.Cost.Gold, "food only (D283)");
		T.AssertEquals(20.0f, Peasant.BuildTime, "20 s (D283)");
		T.AssertEquals(1, Peasant.Population, "one place");
		T.AssertTrue(Peasant.ArmorCategories.Num() == 1 && Peasant.ArmorCategories[0] == ProdCatPeasant, "Peasant: its own armour category only (J7-Q4)");
		T.AssertTrue(Peasant.Attack > 0 && Peasant.Attack < 8, "Peasant: a weak attack (J7-Q4)");
		T.AssertEquals(0.0f, Peasant.AggressionRadius, "Peasant: takes on nobody by itself");
	}

	FHOLUnitTypeSummary Spearman;
	bool bSpearman = HOLProductionTables::FindUnitType(n"Spearman", Spearman);
	T.AssertTrue(bSpearman, "Spearman in the unit type table");
	if (bSpearman)
	{
		T.AssertTrue(Spearman.Cost.Food == 60 && Spearman.Cost.Wood == 20 && Spearman.Cost.Stone == 0 && Spearman.Cost.Gold == 0, "Spearman: 60 food, 20 wood");
		T.AssertEquals(1, Spearman.Population, "Spearman: one place");
		T.AssertEquals(80, Spearman.MaxHealth, "Spearman: 80 health");
		T.AssertTrue(Spearman.ArmorCategories.Num() == 1 && Spearman.ArmorCategories[0] == ProdCatLightInfantry, "Spearman: light infantry only");
		T.AssertEquals(8, Spearman.Attack, "Spearman: attack 8");
		T.AssertTrue(Spearman.BonusCategories.Num() == 1 && Spearman.BonusCategories[0] == ProdCatCavalry && Spearman.BonusDamages[0] == 17, "Spearman: +17 against cavalry");
		T.AssertTrue(Spearman.AttackInterval > 0.0f && Spearman.AttackRange > 0.0f && Spearman.AttackRange < 200.0f && Spearman.AggressionRadius > 0.0f, "Spearman: a cadence, a melee reach, an aggression radius");
	}

	FHOLUnitTypeSummary ManAtArms;
	bool bManAtArms = HOLProductionTables::FindUnitType(n"ManAtArms", ManAtArms);
	T.AssertTrue(bManAtArms, "ManAtArms in the unit type table");
	if (bManAtArms)
	{
		T.AssertTrue(ManAtArms.Cost.Food == 100 && ManAtArms.Cost.Wood == 0 && ManAtArms.Cost.Stone == 0 && ManAtArms.Cost.Gold == 20, "Man-at-arms: 100 food, 20 gold");
		T.AssertEquals(1, ManAtArms.Population, "Man-at-arms: one place");
		T.AssertEquals(155, ManAtArms.MaxHealth, "Man-at-arms: 155 health");
		T.AssertTrue(ManAtArms.ArmorCategories.Num() == 2 && ManAtArms.ArmorCategories.Contains(ProdCatHeavyInfantry) && ManAtArms.ArmorCategories.Contains(ProdCatHeavy), "Man-at-arms: heavy infantry and Heavy (D428)");
		T.AssertEquals(12, ManAtArms.Attack, "Man-at-arms: attack 12");
		T.AssertEquals(4, ManAtArms.MeleeArmor, "Man-at-arms: melee armour 4");
		T.AssertEquals(4, ManAtArms.RangedArmor, "Man-at-arms: ranged armour 4");
		T.AssertEquals(0, ManAtArms.BonusCategories.Num(), "Man-at-arms: no bonus");
		T.AssertEquals(2, ManAtArms.HeroArmor, "Man-at-arms: hero armour 2 (D345)");
		T.AssertTrue(ManAtArms.Cost.Food > Spearman.Cost.Food, "Man-at-arms: costs more than the Spearman");
	}

	FHOLUnitTypeSummary Archer;
	bool bArcher = HOLProductionTables::FindUnitType(n"Archer", Archer);
	T.AssertTrue(bArcher, "Archer in the unit type table");
	if (bArcher)
	{
		T.AssertTrue(Archer.Cost.Food == 30 && Archer.Cost.Wood == 50 && Archer.Cost.Stone == 0 && Archer.Cost.Gold == 0, "Archer: 30 food, 50 wood");
		T.AssertEquals(1, Archer.Population, "Archer: one place");
		T.AssertEquals(70, Archer.MaxHealth, "Archer: 70 health");
		T.AssertTrue(Archer.ArmorCategories.Num() == 1 && Archer.ArmorCategories[0] == ProdCatRanged, "Archer: Ranged category only");
		T.AssertEquals(1, Archer.AttackType, "Archer: ranged attack (D345)");
		T.AssertTrue(Archer.BonusCategories.Num() == 2 && Archer.BonusCategories[0] == ProdCatLightInfantry && Archer.BonusDamages[0] == 5
			&& Archer.BonusCategories[1] == ProdCatGunpowder && Archer.BonusDamages[1] == 5, "Archer: +5 against light infantry and +5 against gunpowder infantry (D345)");
		T.AssertEquals(779.0f, Archer.Speed, "Archer: 779 cm/s");
	}

	FHOLBuildingProductionSummary Barracks;
	bool bBarracks = HOLProductionTables::FindBuildingType(n"Barracks", Barracks);
	T.AssertTrue(bBarracks, "Barracks in the building type table");
	if (bBarracks)
	{
		T.AssertEquals(1, Barracks.Category, "Barracks: military (F1, D30)");
		T.AssertTrue(Barracks.Produces.Num() == 4 && Barracks.Produces[0] == n"Spearman" && Barracks.Produces[1] == n"Halberdier" && Barracks.Produces[2] == n"ManAtArms" && Barracks.Produces[3] == n"DamnedWarrior", "Barracks: produces the Spearman (Legions), the Halberdier (Dawn, D858), the Man-at-arms then the Damned Warrior");
		T.AssertEquals(0, Barracks.PopulationProvided, "Barracks: gives no population");
		T.AssertEquals(0, Barracks.AcceptsCount, "Barracks: not a deposit");
	}

	FHOLBuildingProductionSummary ArcheryRange;
	bool bRange = HOLProductionTables::FindBuildingType(n"ArcheryRange", ArcheryRange);
	T.AssertTrue(bRange, "Archery range in the building type table");
	if (bRange)
	{
		T.AssertEquals(1, ArcheryRange.Category, "Archery range: military (F1, D30)");
		T.AssertTrue(ArcheryRange.Produces.Num() == 3 && ArcheryRange.Produces[0] == n"Archer" && ArcheryRange.Produces[1] == n"Crossbowman" && ArcheryRange.Produces[2] == n"Arquebusier", "Archery range: produces the Archer, the Crossbowman then the Arquebusier (G3)");
		for (int Index = 0; Index < ArcheryRange.Produces.Num(); ++Index)
		{
			FHOLUnitTypeSummary Produced;
			T.AssertTrue(HOLProductionTables::FindUnitType(ArcheryRange.Produces[Index], Produced), "Archery range: " + ArcheryRange.Produces[Index].ToString() + " in the unit type table");
		}
	}

	FHOLBuildingProductionSummary MainCentre;
	bool bCentre = HOLProductionTables::FindBuildingType(n"MainCentre", MainCentre);
	T.AssertTrue(bCentre, "main centre in the building type table");
	if (bCentre)
	{
		T.AssertEquals(10, MainCentre.PopulationProvided, "base population 10 (D283)");
		T.AssertEquals(5, MainCentre.MaxQueue, "queue of 5 (D283)");
		T.AssertTrue(MainCentre.Produces.Contains(n"Peasant"), "produces the Peasant");
	}

	FHOLResources Start = HOLProductionTables::GetStartingResources();
	T.AssertTrue(Start.Food == 200 && Start.Wood == 200 && Start.Stone == 200 && Start.Gold == 200, "200 of each at the start (D283)");
}
