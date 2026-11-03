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
	FHOLAITechChoice Choice = HOLAITech::Choose(HOLAITech::DefaultSettings(), Table, Done, Buildings, AITechStock(500, 500, 0, 0), 25, 1, 3, 1);
	T.AssertTrue(Choice.bResearch, "it researches");
	T.AssertTrue(Choice.Technology == n"PloughI", "Plough I first");
	T.AssertEquals(700.f, float(Choice.BuildingAt.X), "in the Farm");
}

// The difficulty: Easy does not by the provisional setting; the setting is the one switch (3 = nobody, 0 = everybody).
void Test_AITechDifficultyIsTheOnlySwitch(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	const TArray<int> Rich = AITechStock(500, 500, 0, 0);
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 0).bResearch, "Easy does not");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 2).bResearch, "Hard does");
	Rule.MinDifficulty = 2;
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 1).bResearch, "raised to Hard: Normal does not");
	Rule.MinDifficulty = 0;
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 0).bResearch, "lowered to Easy: Easy does");
	Rule.MinDifficulty = 3;
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, Rich, 25, 1, 3, 2).bResearch, "3 = nobody");
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
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 19, 1, 3, 1).bResearch, "19 Peasants: the economy comes first");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 20, 1, 3, 1).bResearch, "20 Peasants");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Farm, Rich, 25, 0, 0, 1).bResearch, "tier 0: the technology is not open");
	TArray<FHOLAIBuilding> Site;
	Site.Add(AITechBuilding(n"Farm", 700, false, NAME_None));
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Site, Rich, 25, 1, 3, 1).bResearch, "the Farm is a site, not finished");
	TArray<FHOLAIBuilding> None;
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, None, Rich, 25, 1, 3, 1).bResearch, "no building at all");
}

// The reserve: the research is paid on top of what the economy keeps (100 food, 100 wood). Plough I costs 100 food, 100 wood: it needs 200 of each.
// The AI SAVES for the most wanted one instead of spending on a lesser one at once.
void Test_AITechKeepsTheReserveAndSaves(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	HOLAITechnology::ReadAITechnologyTable(Table);
	TArray<FName> Done;
	TArray<FHOLAIBuilding> Buildings;
	Buildings.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	Buildings.Add(AITechBuilding(n"LumberCamp", 300, true, NAME_None));
	FHOLAITechSettings Rule = HOLAITech::DefaultSettings();
	FHOLAITechChoice Short = HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(199, 500, 0, 0), 25, 1, 3, 1);
	T.AssertFalse(Short.bResearch, "199 food: 100 for the Plough and 100 kept are not there; it does not fall back on the Axes");
	T.AssertTrue(Short.Reason.Contains("saving"), "it says it is saving");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(200, 200, 0, 0), 25, 1, 3, 1).Technology == n"PloughI", "200 and 200: the Plough");
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
	FHOLAITechChoice First = HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 1, 3, 1);
	T.AssertTrue(First.Technology == n"DoubleAxesI", "no Farm: the Axes");
	Done.Add(n"DoubleAxesI");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 1, 3, 1).bResearch, "Axes I done, tier 1: the level II is not open");
	T.AssertTrue(HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 2, 6, 1).Technology == n"DoubleAxesII", "tier 2: Axes II");
	Done.Add(n"DoubleAxesII");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Camp, Rich, 25, 2, 6, 1).bResearch, "the line is finished");
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
	T.AssertFalse(HOLAITech::Choose(HOLAITech::DefaultSettings(), Table, Done, Buildings, AITechStock(900, 900, 0, 0), 25, 1, 3, 1).bResearch,
		"Plough I is being researched: nothing else starts");
	// A unit in a queue is not a research: a Barracks training a Spearman does not hold it.
	TArray<FHOLAIBuilding> Busy;
	Busy.Add(AITechBuilding(n"Barracks", 100, true, n"Spearman"));
	Busy.Add(AITechBuilding(n"Farm", 700, true, NAME_None));
	T.AssertTrue(HOLAITech::Choose(HOLAITech::DefaultSettings(), Table, Done, Busy, AITechStock(900, 900, 0, 0), 25, 1, 3, 1).bResearch, "a Spearman in a queue does not hold it");
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
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(900, 900, 900, 900), 25, 1, 3, 1).bResearch, "an empty list: nothing");
	// The Cloche (Main Centre, tier 2) has no coded effect: it stays refused with the Centre there and the tier open.
	Buildings.Add(AITechBuilding(n"MainCentre", 0, true, NAME_None));
	Rule.Order.Add(n"ImprovedAlarmBell");
	T.AssertFalse(HOLAITech::Choose(Rule, Table, Done, Buildings, AITechStock(900, 900, 900, 900), 25, 2, 6, 1).bResearch, "Implemented false: refused");
}

// The default list names rows of the real table, each of an economy line a finished building of the AI's first buildings researches and whose effect is coded.
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
				T.AssertEquals(0, Table[Row].Category, Rule.Order[Index].ToString() + " is an economy technology");
			}
		}
		T.AssertTrue(bFound, Rule.Order[Index].ToString() + " is a row of the table");
	}
}
