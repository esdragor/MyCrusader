// Palier 3 G14 (Q-ai-arquebusier, answer A): the Arquebusier enters the AI's fixed mix of the tier 3 with a share of 1 in 7, when the
// tier is open AND the AI's faction makes it (the Legions have none, D710: the server would refuse it in a loop). The counter rule is unchanged.
// The effect is tested on the real chain: the composition is asked again and again, each choice joining the army, and the units made are counted.

int ArquebusierPicks(FHOLAIArmyData Data, int Tier, int Picks)
{
	FHOLAISummary Summary;
	Summary.Tier = Tier;
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
		if (Choice.Type == n"Arquebusier")
		{
			Made++;
		}
	}
	return Made;
}

void Test_ArmyArquebusierEntersTheTierThreeMix(FUnitTest& T)
{
	const FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	T.AssertEquals(4, ArquebusierPicks(Data, 3, 28), "tier 3, Dawn AI: 28 units of the fixed mix hold 4 Arquebusiers (1 in 7)");
	T.AssertEquals(0, ArquebusierPicks(Data, 2, 28), "tier 2: the Arquebusier is not open, none is planned");
}

void Test_ArmyArquebusierIsMadeAtTheArcheryRange(FUnitTest& T)
{
	const FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	FHOLAISummary Summary;
	Summary.Tier = 3;
	// Everything else is already at its share: the Arquebusier is the one lacking.
	for (int Index = 0; Index < 14; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = 100 + Index;
		Unit.Health = 60;
		Unit.Type = Index < 4 ? n"Spearman" : Index < 6 ? n"ManAtArms" : Index < 10 ? n"Archer" : n"Crossbowman";
		Summary.Army.Add(Unit);
	}
	const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.bValid, "a choice");
	T.AssertTrue(Choice.Type == n"Arquebusier", "the type lacking in the mix is the Arquebusier");
	T.AssertTrue(Choice.Building == n"ArcheryRange", "made at the Archery range");
}
