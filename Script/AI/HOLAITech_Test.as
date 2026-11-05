// The AI's research of technologies, alone (HOLAITech.as, T7): the real table (Tools/Data/Technologies.csv, read as the simulation reads it),
// buildings and stocks built by hand.

FHOLAIBuilding AITechBuilding(FName Type, float X, bool bFinished, FName Queued)
{
	FHOLAIBuilding Building;
	Building.TypeName = Type;
	Building.State = bFinished ? EHOLConstructionState::Finished : EHOLConstructionState::UnderConstruction;
	Building.Location = FVector(X, 0, 0);
	if (!Queued.IsNone())
	{
		Building.Queue.Add(Queued);
	}
	return Building;
}

TArray<int> AITechStock(int Food, int Wood, int Stone, int Gold)
{
	TArray<int> Stock;
	Stock.Add(Food);
	Stock.Add(Wood);
	Stock.Add(Stone);
	Stock.Add(Gold);
	return Stock;
}

// Normal, tier 1, a finished Farm, the purse full, 25 Peasants: Plough I, in the Farm.
void Test_AITechResearchesTheFirstOfTheOrder(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	T.AssertTrue(HOLAITechnology::ReadAITechnologyTable(Table), "the real table is there");
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	FHOLAITechChoice Choice = HOLAITech::Choose(HOLAITech::DefaultSettings(), Table, Done, Buildings, AITechStock(500, 500, 0, 0), 25, 1, 3, 2);
	T.AssertTrue(Choice.bResearch, "it researches");
	T.AssertTrue(Choice.Technology == n"PloughI", "Plough I first");
	T.AssertEquals(700.f, float(Choice.BuildingAt.X), "in the Farm");
}

// D648: every level researches, the DELAY is what differs: nothing before it (game seconds), then the same rule. The levels by their numbers 0 Easy, 1 Normal, 2 Hard.
void Test_AITechEveryLevelResearchesAfterItsDelay(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	const TArray<int> Rich = AITechStock(500, 500, 0, 0);
	const FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	T.AssertEquals(3, Rule.DelaySeconds.Num(), "a delay per level");
	for (int Level = 0; Level < 3; Level++)
	{
		FHOLAITechMemory Memory;
		const float Delay = Rule.DelaySeconds[Level];
		FHOLAITechChoice Early = HOLAITech::Think(Rule, Memory, Delay - 1.f, Table, Done, Buildings, Rich, 25, 1, 3, Level);
		if (Delay > 0.f)
		{
			T.AssertFalse(Early.bResearch, f"level {Level}: not a second before its delay");
			T.AssertTrue(Early.Reason.Contains("too early"), f"level {Level}: it says why");
		}
		FHOLAITechChoice Late = HOLAITech::Think(Rule, Memory, Delay, Table, Done, Buildings, Rich, 25, 1, 3, Level);
		T.AssertTrue(Late.bResearch, f"level {Level}: researches at its delay");
		T.AssertTrue(Late.Technology == n"PloughI", f"level {Level}: the same first technology");
	}
}

// D648: Hard first, then Normal, Easy MUCH later (not a few seconds: at least twice Normal's delay and a few minutes).
void Test_AITechDelaysAreOrderedHardNormalEasy(FUnitTest& T)
{
	const FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	T.AssertTrue(Rule.DelaySeconds[2] < Rule.DelaySeconds[1], "Hard before Normal");
	T.AssertTrue(Rule.DelaySeconds[1] < Rule.DelaySeconds[0], "Normal before Easy");
	T.AssertTrue(Rule.DelaySeconds[0] >= 2.f * Rule.DelaySeconds[1], "Easy much later than Normal");
	T.AssertTrue(Rule.DelaySeconds[0] >= 300.f, "Easy at least five minutes in");
}

// An Evolving AI plays Easy, then Normal, then Hard (D329): its delay is the one of the level it plays now, so it researches when the stage it is in allows.
void Test_AITechEvolvingFollowsTheLevelItPlays(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	const TArray<int> Rich = AITechStock(500, 500, 0, 0);
	const FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	const FHOLAILivesData Data;
	// Just before Normal's stage the Evolving AI is Easy: it waits for Easy's delay.
	float Now = Data.EvolveNormalAt - 1.f;
	int Level = int(HOLAILives::Effective(EHOLAIDifficulty::Evolving, Now, Data));
	T.AssertEquals(0, Level, "Easy before Normal's stage");
	FHOLAITechMemory Memory;
	T.AssertEquals(Now >= Rule.DelaySeconds[0], HOLAITech::Think(Rule, Memory, Now, Table, Done, Buildings, Rich, 25, 1, 3, Level).bResearch, "Easy's delay applies");
	// At Hard's stage it is Hard: Hard's delay (the shortest) applies whatever Easy's was.
	Now = Data.EvolveHardAt;
	Level = int(HOLAILives::Effective(EHOLAIDifficulty::Evolving, Now, Data));
	T.AssertEquals(2, Level, "Hard after Hard's stage");
	FHOLAITechMemory Later;
	T.AssertTrue(HOLAITech::Think(Rule, Later, Now, Table, Done, Buildings, Rich, 25, 1, 3, Level).bResearch, "researches as a Hard AI");
}

// Each condition holds it back alone: the economy (20 Peasants), the tier, the building (finished), the Farm busy.
void Test_AITechEachConditionHoldsItBack(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	TArray<FHOLAIBuilding> Farm;
	Farm.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	const TArray<int> Rich = AITechStock(500, 500, 0, 0);
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 19, 1, 3, 2).bResearch, "19 Peasants: the economy comes first");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 20, 1, 3, 2).bResearch, "20 Peasants");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 25, 0, 0, 2).bResearch, "tier 0: the technology is not open");
	TArray<FHOLAIBuilding> Site;
	Site.Add(AITechBuilding(n"Farm", 700, false, NAME_None));
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Site, Rich, 25, 1, 3, 2).bResearch, "the Farm is a site, not finished");
	TArray<FHOLAIBuilding> None;
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, None, Rich, 25, 1, 3, 2).bResearch, "no building at all");
}

// The purse: the research is paid out of the stock when it is queued, and the reserve a rule keeps comes on top (the default keeps none: the AI spends
// all it has, a stock of 200 never builds up, so a reserve would never be met in a real game). Plough I costs 100 food, 100 wood. The AI SAVES for
// the most wanted open one instead of spending on a lesser one at once.
void Test_AITechKeepsTheReserveAndSaves(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	Buildings.Add(AITechBuilding(n"LumberCamp", 300, true, NAME_None));
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	FHOLAITechChoice Short = HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(99, 500, 0, 0), 25, 1, 3, 2);
	T.AssertFalse(Short.bResearch, "99 food: the Plough is not paid; it does not fall back on the Axes");
	T.AssertTrue(Short.Reason.Contains("saving"), "it says it is saving");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(100, 100, 0, 0), 25, 1, 3, 2).Technology == n"PloughI", "100 and 100: the Plough");
	// A rule that keeps a reserve (100 food, 100 wood) needs the cost on top.
	Rule.Reserve[0] = 100;
	Rule.Reserve[1] = 100;
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(199, 500, 0, 0), 25, 1, 3, 2).bResearch, "199 food with 100 kept");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(200, 200, 0, 0), 25, 1, 3, 2).Technology == n"PloughI", "200 and 200 with 100 kept");
}

// What is not there goes on to the next: no Farm, a Lumber Camp: the Axes. Researched ones are left, the level II asks the tier 2.
void Test_AITechSkipsWhatIsNotThereAndFollowsTheLines(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Camp;
	Camp.Add(AITechBuilding(n"LumberCamp", 300, true, NAME_None));
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	const TArray<int> Rich = AITechStock(900, 900, 0, 0);
	FHOLAITechChoice First = HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 1, 3, 2);
	T.AssertTrue(First.Technology == n"DoubleAxesI", "no Farm: the Axes");
	Done.Add(n"DoubleAxesI");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 1, 3, 2).bResearch, "Axes I done, tier 1: the level II is not open");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 2, 6, 2).Technology == n"DoubleAxesII", "tier 2: Axes II");
	Done.Add(n"DoubleAxesII");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 2, 6, 2).bResearch, "the line is finished");
}

// One research at a time: a technology in any queue of the slot holds the next back (the purse is spared, a Farm is not a second hand).
void Test_AITechOneAtATime(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, n"PloughI"));
	Buildings.Add(AITechBuilding(n"LumberCamp", 300, true, NAME_None));
	T.AssertFalse(HOLAITech::Choose(HOLAITech::DefaultSettings(), Table, Done, Buildings, AITechStock(900, 900, 0, 0), 25, 1, 3, 2).bResearch,
		"Plough I is being researched: nothing else starts");
	// A unit in a queue is not a research: a Barracks training a Spearman does not hold it.
	TArray<FHOLAIBuilding> Busy;
	Busy.Add(AITechBuilding(n"Barracks", 100, true, n"Spearman"));
	Busy.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	T.AssertTrue(HOLAITech::Choose(HOLAITech::DefaultSettings(), Table, Done, Busy, AITechStock(900, 900, 0, 0), 25, 1, 3, 2).bResearch, "a Spearman in a queue does not hold it");
}

// The order of the settings is the only list: a technology that is not in it is never chosen, and one whose effect is not coded (Implemented false) is refused.
void Test_AITechOnlyTheListedAndImplemented(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	Rule.Order.Empty();
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(900, 900, 900, 900), 25, 1, 3, 2).bResearch, "an empty list: nothing");
	// The Cloche (Main Centre, tier 2) has no coded effect: it stays refused with the Centre there and the tier open.
	Buildings.Add(AITechBuilding(n"MainCentre", 0, true, NAME_None));
	Rule.Order.Add(n"ImprovedAlarmBell");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(900, 900, 900, 900), 25, 2, 6, 2).bResearch, "Implemented false: refused");
}

// F1 (Fortifications, D696): with a finished Forge the AI researches the attack lines after the economy ones: the economy lines open at tier 1 first (the Forge is
// asked nothing while one of them is open), then the level I of the melee attack, of the ranged attack, then the level II of each at tier 2. It builds no Forge here
// (F14): the rule only uses the one it has.
void Test_AIResearchesTheForgeAttacksAfterTheEconomyLines(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	Buildings.Add(AITechBuilding(n"Forge", 900, true, NAME_None));
	const FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	const TArray<int> Rich = AITechStock(900, 900, 0, 900);
	TArray<FName> Done;
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 1).Technology == n"PloughI", "an economy line still open comes first, the Forge waits");
	Done.Add(n"PloughI");
	Done.Add(n"WheelbarrowI");
	FHOLAITechChoice Melee = HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 1);
	T.AssertTrue(Melee.bResearch, "the economy line of the Farm done: the Forge's turn");
	T.AssertTrue(Melee.Technology == n"MeleeAttackI", "the melee attack first");
	T.AssertEquals(900.f, float(Melee.BuildingAt.X), "in the Forge");
	Done.Add(n"MeleeAttackI");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 1).Technology == n"RangedAttackI", "then the ranged attack");
	Done.Add(n"RangedAttackI");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 1).bResearch, "tier 1: both level I done, the level II are not open");
	Done.Add(n"PloughII");
	Done.Add(n"WheelbarrowII");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 2, 6, 1).Technology == n"MeleeAttackII", "tier 2: the melee level II");
	Done.Add(n"MeleeAttackII");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 2, 6, 1).Technology == n"RangedAttackII", "then the ranged level II");
	Done.Add(n"RangedAttackII");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 2, 6, 1).bResearch, "the four attack lines are done: nothing left (the armour is not coded, F2)");
}

// Without a finished Forge nothing changes for the AI (it builds none until F14): the attacks are passed over, as a technology of an absent building is.
void Test_AIForgeAttacksNeedAFinishedForge(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	Done.Add(n"PloughI");
	Done.Add(n"WheelbarrowI");
	const FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	const TArray<int> Rich = AITechStock(900, 900, 0, 900);
	TArray<FHOLAIBuilding> Farm;
	Farm.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 25, 1, 3, 1).bResearch, "no Forge: no attack line");
	Farm.Add(AITechBuilding(n"Forge", 900, false, NAME_None));
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 25, 1, 3, 1).bResearch, "a Forge still a site: no attack line");
	Farm.Add(AITechBuilding(n"Forge", 950, true, n"MeleeAttackI"));
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 25, 1, 3, 1).bResearch, "one research at a time: Melee I is in the Forge's queue");
}

// The default list names rows of the real table, each a line a finished building of the AI's first buildings researches (an economy line, or an attack line of the
// Forge, F1) and whose effect is coded.
void Test_AITechDefaultOrderIsInTheTable(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	T.AssertTrue(Rule.Order.Num() > 0, "a list");
	for (int Index = 0; Index < Rule.Order.Num(); Index++)
	{
		bool bFound = false;
		for (int Row = 0; Row < Table.Num(); Row++)
		{
			if (Table[Row].Name == Rule.Order[Index])
			{
				bFound = true;
				T.AssertTrue(Table[Row].bImplemented, Rule.Order[Index].ToString() + " has a coded effect");
				const bool bEconomy = Table[Row].Category == 0;
				const bool bForgeAttack = Table[Row].Category == 1 && Table[Row].Building == n"Forge";
				T.AssertTrue(bEconomy || bForgeAttack, Rule.Order[Index].ToString() + " is an economy technology or an attack line of the Forge");
			}
		}
		T.AssertTrue(bFound, Rule.Order[Index].ToString() + " is a row of the table");
	}
}

// Saving: the cost the stock already holds is earmarked (so that the other purchases leave it), no more than the stock has.
void Test_AITechEarmarksWhatIsHeldOfTheCost(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	FHOLAITechMemory Memory;
	FHOLAITechChoice Saving = HOLAITech::Think(Rule, Memory, 100.f, Table, Done, Buildings, AITechStock(130, 40, 7, 9), 25, 1, 3, 2);
	T.AssertTrue(Saving.bSaving, "saving for the Plough");
	T.AssertEquals(4, Saving.Earmark.Num(), "one amount per resource");
	T.AssertEquals(100, Saving.Earmark[0], "food: the 100 of the cost, not the 130 held");
	T.AssertEquals(40, Saving.Earmark[1], "wood: the 40 held of the 100");
	T.AssertEquals(0, Saving.Earmark[2], "stone: not in the cost");
	T.AssertEquals(0, Saving.Earmark[3], "gold: not in the cost");
	// Paid: not saving, nothing earmarked.
	FHOLAITechChoice Paid = HOLAITech::Think(Rule, Memory, 101.f, Table, Done, Buildings, AITechStock(130, 130, 0, 0), 25, 1, 3, 2);
	T.AssertTrue(Paid.bResearch, "it researches");
	T.AssertFalse(Paid.bSaving, "not saving");
	T.AssertEquals(100, Paid.Earmark[1], "the cost of the research is set aside for the other purchases of the thought");
}

// A save that lasts longer than SaveSeconds is given up: no earmark for GiveUpSeconds, then it saves again.
void Test_AITechGivesUpAfterSaveSeconds(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	FHOLAITechMemory Memory;
	const TArray<int> Poor = AITechStock(130, 40, 0, 0);
	T.AssertTrue(HOLAITech::Think(Rule, Memory, 100.f, Table, Done, Buildings, Poor, 25, 1, 3, 2).bSaving, "starts saving at 100");
	T.AssertTrue(HOLAITech::Think(Rule, Memory, 100.f + Rule.SaveSeconds, Table, Done, Buildings, Poor, 25, 1, 3, 2).bSaving, "still saving at the limit");
	FHOLAITechChoice Gave = HOLAITech::Think(Rule, Memory, 100.f + Rule.SaveSeconds + 1.f, Table, Done, Buildings, Poor, 25, 1, 3, 2);
	T.AssertFalse(Gave.bSaving, "given up");
	T.AssertEquals(0, Gave.Earmark.Num(), "nothing earmarked");
	T.AssertFalse(HOLAITech::Think(Rule, Memory, 200.f, Table, Done, Buildings, Poor, 25, 1, 3, 2).bSaving, "still given up");
	T.AssertTrue(HOLAITech::Think(Rule, Memory, 100.f + Rule.SaveSeconds + 1.f + Rule.GiveUpSeconds + 1.f, Table, Done, Buildings, Poor, 25, 1, 3, 2).bSaving, "saves again after the pause");
}
