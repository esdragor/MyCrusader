// MIG2-S4 (ELROND-1118n, D830, D839): port of Tests/HOLTechnologyTableTests.cpp, HeroesOfLiberty.Technologies.Playable and
// HeroesOfLiberty.Technologies.Rules, against HOLTechnologyCheck:: (UHOLTechnologyCheckLibrary additions: wrappers of HOLTechnology::, the rows
// being the real table, Tools/Data/Technologies.csv + BuildingTypes.csv read through the settings tables, never a typed copy of a row). The
// availability vectors of Rules (the 14 Check calls) were already ported: Test_TechnologyRulesVectors (HOLTechnologyRules_Test.as). Expected values are literals
// of the C++ test (the percents 10 / 20 are the CSV's). EHOLProductionRefusal: 0 None, 2 QueueFull. EHOLAttackType: 0 Melee, 1 Ranged, 2 Siege, 3 Hero.
// Armour category bits: 0 Light, 1 HeavyInf, 2 Ranged, 3 Cavalry, 4 Siege, 5 Peasant, 6 Gunpowder, 7 Heavy, 9 Hero.

namespace TechnologyTableTest
{
	FHOLResources Stock(int Food, int Wood, int Stone, int Gold)
	{
		FHOLResources Result;
		Result.Food = Food;
		Result.Wood = Wood;
		Result.Stone = Stone;
		Result.Gold = Gold;
		return Result;
	}

	int Bonus1(const FName& Technology, const FName& Stat)
	{
		TArray<FName> Researched;
		Researched.Add(Technology);
		return HOLTechnologyCheck::GetBonusPercent(Researched, Stat);
	}
}

// T3/T4: of the 26 technologies, 17 are playable (the 8 economy ones, the Vow of Poverty, the Forge's 4 attack and 4 armour lines).
void Test_TechnologyTable_Playable(FUnitTest& T)
{
	TArray<FName> Expected;
	Expected.Add(n"DoubleAxesI");
	Expected.Add(n"DoubleAxesII");
	Expected.Add(n"MinerPicksI");
	Expected.Add(n"MinerPicksII");
	Expected.Add(n"PloughI");
	Expected.Add(n"PloughII");
	Expected.Add(n"WheelbarrowI");
	Expected.Add(n"WheelbarrowII");
	Expected.Add(n"VowOfPoverty");
	Expected.Add(n"MeleeAttackI");
	Expected.Add(n"MeleeAttackII");
	Expected.Add(n"RangedAttackI");
	Expected.Add(n"RangedAttackII");
	Expected.Add(n"InfantryArmorI");
	Expected.Add(n"InfantryArmorII");
	Expected.Add(n"CavalryArmorI");
	Expected.Add(n"CavalryArmorII");

	TArray<FName> Playable = HOLTechnologyCheck::GetPlayableTechnologies();
	T.AssertEquals(17, Playable.Num(), "17 technologies are playable");

	TArray<FHOLAITechnology> Table;
	T.AssertTrue(HOLAITechnology::ReadAITechnologyTable(Table), "the technology table is loaded");
	TArray<FName> Soon;
	Soon.Add(n"ImprovedAlarmBell");
	Soon.Add(n"TemperedHalberds");
	Soon.Add(n"ReinforcedRam");
	Soon.Add(n"HeavyMunitions");
	for (int Index = 0; Index < Soon.Num(); Index++)
	{
		bool bInTable = false;
		for (int Row = 0; Row < Table.Num(); Row++)
		{
			if (Table[Row].Name == Soon[Index])
			{
				bInTable = true;
			}
		}
		T.AssertTrue(bInTable, Soon[Index].ToString() + " is in the table");
		int Queue = HOLTechnologyCheck::GetResearchQueueSize(Soon[Index]);
		T.AssertTrue(Queue >= 0, Soon[Index].ToString() + ": its building exists (it is greyed Soon, not missing)");
		bool bPlayable = HOLTechnologyCheck::IsPlayable(Soon[Index]);
		T.AssertFalse(bPlayable, Soon[Index].ToString() + " is not playable: no effect coded (D601)");
	}

	for (int Index = 0; Index < Expected.Num(); Index++)
	{
		T.AssertTrue(Playable.Contains(Expected[Index]), Expected[Index].ToString() + " is playable");
		bool bPlayable = HOLTechnologyCheck::IsPlayable(Expected[Index]);
		T.AssertTrue(bPlayable, Expected[Index].ToString() + " is playable (IsPlayable)");
		// T-Q6 = A: the file of 5 (D283) of every building that researches one is the production queue's.
		int Queue = HOLTechnologyCheck::GetResearchQueueSize(Expected[Index]);
		T.AssertEquals(5, Queue, Expected[Index].ToString() + " researches in a queue of 5");
	}
	for (int Index = 0; Index < Playable.Num(); Index++)
	{
		T.AssertTrue(Expected.Contains(Playable[Index]), Playable[Index].ToString() + " is expected to be playable");
	}
}

// T3/T4 (the part of Rules the vectors do not hold): the word of the log.
void Test_TechnologyTable_DescribeBlock(FUnitTest& T)
{
	FString Word = HOLTechnologyCheck::DescribeBlock(8);
	T.AssertEquals("soon", Word, "the log says soon");
}

// A research is an item of the production queue (T-Q6 = A): cost taken at the order, given back in full at the cancel, no population.
void Test_TechnologyTable_ResearchQueue(FUnitTest& T)
{
	FHOLResources Cost;
	int Steps = 0;
	int Pop = -1;
	T.AssertTrue(HOLTechnologyCheck::GetResearchCost(n"PloughI", 0.05f, Cost, Steps, Pop), "PloughI is in the table");
	T.AssertEquals(100, Cost.Food, "the cost is the row's: food");
	T.AssertEquals(100, Cost.Wood, "the cost is the row's: wood");
	T.AssertEquals(0, Cost.Stone, "the cost is the row's: stone");
	T.AssertEquals(0, Cost.Gold, "the cost is the row's: gold");
	T.AssertEquals(600, Steps, "30 s = 600 steps of 0.05 s");
	T.AssertEquals(0, Pop, "no population place");

	FHOLProductionQueueState Queue;
	FHOLResources Stock = TechnologyTableTest::Stock(1000, 1000, 1000, 1000);
	bool bQueuedBefore = HOLTechnologyCheck::IsQueued(Queue, n"PloughI");
	T.AssertFalse(bQueuedBefore, "not queued yet");
	for (int Index = 0; Index < 5; Index++)
	{
		FName Name = n"PloughI";
		if (Index == 1) Name = n"Other1";
		if (Index == 2) Name = n"Other2";
		if (Index == 3) Name = n"Other3";
		if (Index == 4) Name = n"Other4";
		int Refusal = HOLProductionRead::TryEnqueue(Queue, Name, Cost, Steps, Pop, 5, Stock);
		T.AssertEquals(0, Refusal, "each of the five researches is queued");
	}
	T.AssertEquals(2, HOLProductionRead::TryEnqueue(Queue, n"PloughII", Cost, Steps, Pop, 5, Stock), "the sixth is refused: a file of 5");
	bool bQueued = HOLTechnologyCheck::IsQueued(Queue, n"PloughI");
	T.AssertTrue(bQueued, "queued");
	bool bOther = HOLTechnologyCheck::IsQueued(Queue, n"PloughII");
	T.AssertFalse(bOther, "another one is not");
	T.AssertEquals(500, Stock.Food, "five costs taken");
	T.AssertTrue(HOLProductionRead::Cancel(Queue, 0, Cost, Steps, Pop, Stock), "cancel gives it back in full");
	T.AssertEquals(600, Stock.Food, "four costs taken");
	bool bQueuedAfter = HOLTechnologyCheck::IsQueued(Queue, n"PloughI");
	T.AssertFalse(bQueuedAfter, "no longer queued");

	// A research needs no population place: a player over his cap (a house lost) still finishes it.
	FHOLProductionQueueState Solo;
	FHOLResources Rich = TechnologyTableTest::Stock(1000, 1000, 1000, 1000);
	HOLProductionRead::TryEnqueue(Solo, n"PloughI", Cost, Steps, Pop, 5, Rich);
	int FreePopulation = -3;
	TArray<FName> Done;
	HOLProductionRead::Advance(Solo, 0, Cost, Steps, Pop, FreePopulation, Done);
	T.AssertEquals(1, Solo.Types.Num(), "one item in the queue");
	T.AssertTrue(Solo.StartSteps[0] != -1, "started although the population is over its cap");
	T.AssertEquals(-3, FreePopulation, "the population is left alone");
	HOLProductionRead::Advance(Solo, 600, Cost, Steps, Pop, FreePopulation, Done);
	T.AssertEquals(1, Done.Num(), "one research done after 600 steps");
	T.AssertTrue(Done.Num() == 1 && Done[0] == n"PloughI", "it is PloughI");
	T.AssertEquals(0, Solo.Types.Num(), "the queue is empty");
}

// T5: the effects. A line's percent is its TOTAL (T-Q2): I then II gives II's, not I + II. Other lines on a statistic add up.
void Test_TechnologyTable_Bonus(FUnitTest& T)
{
	TArray<FName> Researched;
	T.AssertEquals(0, HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherFood"), "nothing researched: no bonus");
	Researched.Add(n"PloughI");
	T.AssertEquals(10, HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherFood"), "Plough I: +10 % food");
	T.AssertEquals(0, HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherWood"), "Plough I: nothing for wood");
	int One = HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherFood");
	Researched.Add(n"PloughII");
	int Both = HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherFood");
	T.AssertEquals(20, Both, "Plough I and II: II's total, not the sum");
	T.AssertTrue(Both > One, "II gives more than I");
	Researched.Add(n"MinerPicksI");
	T.AssertEquals(10, HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherStone"), "Miner picks: stone");
	T.AssertEquals(10, HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherGold"), "Miner picks: gold");
	T.AssertEquals(20, HOLTechnologyCheck::GetBonusPercent(Researched, n"GatherFood"), "the plough still gives food only");
	T.AssertEquals(0, TechnologyTableTest::Bonus1(n"Nothing", n"GatherFood"), "an unknown technology gives nothing");

	// T5: the Wheelbarrow raises what a worker carries (a whole quantity, 10 at the Peasant): both of its stats are the line's.
	T.AssertEquals(10, TechnologyTableTest::Bonus1(n"WheelbarrowI", n"WorkerCarry"), "Wheelbarrow I: carry +10 %");
	TArray<FName> Barrows;
	Barrows.Add(n"WheelbarrowI");
	Barrows.Add(n"WheelbarrowII");
	T.AssertEquals(20, HOLTechnologyCheck::GetBonusPercent(Barrows, n"WorkerSpeed"), "Wheelbarrow I and II: speed +20 % (the line's total)");
	T.AssertEquals(10, HOLTechnologyCheck::ScaleByPercent(10, 0), "10 carried, +0 %: 10");
	T.AssertEquals(11, HOLTechnologyCheck::ScaleByPercent(10, 10), "10 carried, +10 %: 11");
	T.AssertEquals(12, HOLTechnologyCheck::ScaleByPercent(10, 20), "10 carried, +20 %: 12");
	T.AssertEquals(3, HOLTechnologyCheck::ScaleByPercent(3, 10), "3 carried, +10 %: still 3 (a third of a unit is not one)");
	T.AssertEquals(0, HOLTechnologyCheck::ScaleByPercent(0, 20), "nothing carried: nothing");
	T.AssertEquals(10, HOLTechnologyCheck::ScaleByPercent(10, -50), "a negative percent never lowers it");
}

// T6: the Forge's attack lines raise a hit's base attack by the attack's type; melee and ranged are separate lines.
void Test_TechnologyTable_AttackLines(FUnitTest& T)
{
	T.AssertTrue(HOLTechnologyCheck::GetAttackStat(0) == n"MeleeAttack", "melee hits are raised by MeleeAttack");
	T.AssertTrue(HOLTechnologyCheck::GetAttackStat(1) == n"RangedAttack", "ranged hits are raised by RangedAttack");
	T.AssertTrue(HOLTechnologyCheck::GetAttackStat(2) == n"None", "siege hits: no technology yet");
	T.AssertTrue(HOLTechnologyCheck::GetAttackStat(3) == n"None", "hero hits: no technology");
	T.AssertEquals(10, TechnologyTableTest::Bonus1(n"MeleeAttackI", n"MeleeAttack"), "Lames affutees I: melee +10 %");
	T.AssertEquals(0, TechnologyTableTest::Bonus1(n"MeleeAttackI", n"RangedAttack"), "Lames affutees I: nothing for the ranged");
	TArray<FName> Blades;
	Blades.Add(n"MeleeAttackI");
	Blades.Add(n"MeleeAttackII");
	T.AssertEquals(20, HOLTechnologyCheck::GetBonusPercent(Blades, n"MeleeAttack"), "Lames affutees I and II: II's total");
	T.AssertEquals(10, TechnologyTableTest::Bonus1(n"RangedAttackI", n"RangedAttack"), "Empennage I: ranged +10 %");
	T.AssertEquals(0, TechnologyTableTest::Bonus1(n"RangedAttackI", n"MeleeAttack"), "Empennage I: nothing for the melee");
}

// F2 (Q-forge-armure, option B): Maille covers every infantry on foot (and the Ranged shooters), Barde the cavalry; not the Peasant, the heroes, the siege.
void Test_TechnologyTable_ArmorLines(FUnitTest& T)
{
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat(1 << 0) == n"InfantryArmor", "light infantry: Maille");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat((1 << 1) | (1 << 7)) == n"InfantryArmor", "heavy infantry (with the Heavy tag): Maille");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat(1 << 6) == n"InfantryArmor", "gunpowder infantry: Maille");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat((1 << 3) | (1 << 7)) == n"CavalryArmor", "cavalry (with the Heavy tag): Barde");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat(1 << 2) == n"InfantryArmor", "archers and crossbowmen: Maille (option B)");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat((1 << 9) | (1 << 2)) == n"None", "a hero with the Ranged category: no armour");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat(1 << 5) == n"None", "Peasant: no armour");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat(1 << 9) == n"None", "hero: no armour");
	T.AssertTrue(HOLTechnologyCheck::GetArmorStat(1 << 4) == n"None", "siege: no armour");
	T.AssertTrue(HOLTechnologyCheck::ArmorCoversAttack(0), "the armours lower melee hits");
	T.AssertTrue(HOLTechnologyCheck::ArmorCoversAttack(1), "the armours lower ranged hits");
	T.AssertFalse(HOLTechnologyCheck::ArmorCoversAttack(2), "not siege hits");
	T.AssertFalse(HOLTechnologyCheck::ArmorCoversAttack(3), "not hero hits");
	T.AssertEquals(20, TechnologyTableTest::Bonus1(n"InfantryArmorII", n"InfantryArmor"), "Maille II: infantry armour 20 %");
	T.AssertEquals(0, TechnologyTableTest::Bonus1(n"InfantryArmorII", n"CavalryArmor"), "Maille II: cavalry 0");
}

// The player's state: a technology is researched once.
void Test_TechnologyTable_State(FUnitTest& T)
{
	TArray<FName> Done;
	bool bNone = HOLTechnologyCheck::StateHas(Done, n"PloughI");
	T.AssertFalse(bNone, "none yet");
	T.AssertTrue(HOLTechnologyCheck::StateAdd(Done, n"PloughI"), "added");
	bool bHas = HOLTechnologyCheck::StateHas(Done, n"PloughI");
	T.AssertTrue(bHas, "has it");
	T.AssertFalse(HOLTechnologyCheck::StateAdd(Done, n"PloughI"), "added once");
	T.AssertEquals(1, Done.Num(), "one entry");
}
