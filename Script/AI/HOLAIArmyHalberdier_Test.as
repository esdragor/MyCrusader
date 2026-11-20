// D858 step 2b: the Halberdier replaces the Spearman for the Dawn's AI (the Spearman is the Legions' alone since 6d4b82b). The effect is read on the
// real chain: the fixed mix is asked again and again (HOLAIArmy::ChooseComposition, which reads the Faction column of the unit table), each choice
// joining the army, and the units planned are counted by type. Nothing here is typed in from the candidate list.

int HalbPicks(FHOLAIArmyData Data, FName Type, int Picks)
{
	FHOLAISummary Summary;
	Summary.Tier = 3;
	int Made = 0;
	for (int Pick = 0; Pick < Picks; Pick++)
	{
		const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
		if (!Choice.bValid)
		{
			break;
		}
		FHOLAIUnit Unit;
		Unit.Id = 100 + Pick;
		Unit.Type = Choice.Type;
		Unit.Health = 60;
		Unit.bIdle = true;
		Summary.Army.Add(Unit);
		if (Choice.Type == Type)
		{
			Made++;
		}
	}
	return Made;
}

void Test_ArmyDawnPlansTheHalberdierNeverTheSpearman(FUnitTest& T)
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Faction = n"Dawn";
	const int Halberdiers = HalbPicks(Data, n"Halberdier", 28);
	const int Spearmen = HalbPicks(Data, n"Spearman", 28);
	T.AssertTrue(Halberdiers > 0, "Dawn AI: the fixed mix plans the Halberdier");
	T.AssertEquals(0, Spearmen, "Dawn AI: the fixed mix never plans the Spearman");
	T.AssertTrue(Halberdiers >= 6, "Dawn AI: the Halberdier takes the Spearman's share of 2 in 7 (28 picks: at least 6)");
}

void Test_ArmyLegionsPlansTheSpearmanNeverTheHalberdier(FUnitTest& T)
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Faction = n"Legions";
	const int Spearmen = HalbPicks(Data, n"Spearman", 28);
	const int Halberdiers = HalbPicks(Data, n"Halberdier", 28);
	T.AssertTrue(Spearmen > 0, "Legions AI: the fixed mix plans the Spearman");
	T.AssertEquals(0, Halberdiers, "Legions AI: the fixed mix never plans the Halberdier");
}

void Test_ArmyDawnBarracksOrdersTheHalberdier(FUnitTest& T)
{
	// The first unit of the mix for each faction is the one its Barracks makes: asked at the Barracks only.
	FHOLAISummary Summary;
	Summary.Tier = 3;
	FHOLAIArmyData Dawn = HOLAIArmy::DefaultData();
	Dawn.Faction = n"Dawn";
	const FHOLAIComposition DawnChoice = HOLAIArmy::ChooseComposition(Summary, Dawn, n"Barracks");
	T.AssertTrue(DawnChoice.bValid && DawnChoice.Type == n"Halberdier", "Dawn AI: the Barracks is asked for the Halberdier first");
	FHOLAIArmyData Legions = HOLAIArmy::DefaultData();
	Legions.Faction = n"Legions";
	const FHOLAIComposition LegionsChoice = HOLAIArmy::ChooseComposition(Summary, Legions, n"Barracks");
	T.AssertTrue(LegionsChoice.bValid && LegionsChoice.Type == n"Spearman", "Legions AI: the Barracks is asked for the Spearman first");
}
