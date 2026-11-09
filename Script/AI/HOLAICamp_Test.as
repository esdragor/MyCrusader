// N6 (NeutralsInProgress.md): the AI's reading of a neutral camp (HOLAIArmy.as: CampUnits, AdversaryView, CampRatio, CampPolicy), on
// hand-made summaries (the helpers of HOLAIAttack_Test.as, copied: each test file is its own module). The camp is the units of the
// neutral owner (slot 8) in Summary.Enemies, as the summary builds them in a real game. The effect asserted is the launch of the
// wave (CanLaunch).

FHOLAITypeFigures CmpFigures(int Health, int Damage, float Interval, float Range, int MeleeArmor, int RangedArmor, float Speed, int Food, int Wood, int Gold)
{
	FHOLAITypeFigures Figures;
	Figures.Health = Health;
	Figures.Damage = Damage;
	Figures.AttackInterval = Interval;
	Figures.AttackRange = Range;
	Figures.MeleeArmor = MeleeArmor;
	Figures.RangedArmor = RangedArmor;
	Figures.bRanged = false;
	Figures.Speed = Speed;
	Figures.CostFood = Food;
	Figures.CostWood = Wood;
	Figures.CostGold = Gold;
	return Figures;
}

FHOLAIArmyData CmpData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Types.Empty();
	Data.Figures.Empty();
	FHOLAITypeFigures Spear = CmpFigures(80, 8, 1.875f, 130.f, 0, 0, 777.f, 60, 20, 0);
	Spear.ArmorCategories.Add(n"LightInfantry");
	Spear.BonusCategories.Add(n"Cavalry");
	Spear.BonusDamage.Add(17.f);
	Data.Types.Add(n"Spearman");
	Data.Figures.Add(Spear);
	FHOLAITypeFigures Heavy = CmpFigures(155, 12, 1.375f, 70.f, 4, 4, 697.f, 100, 0, 20);
	Heavy.ArmorCategories.Add(n"HeavyInfantry");
	Heavy.ArmorCategories.Add(n"Heavy");
	Data.Types.Add(n"ManAtArms");
	Data.Figures.Add(Heavy);
	return Data;
}

void CmpAdd(TArray<FHOLAIUnit>& List, int FirstId, FName Type, int Count, FVector Where, int Slot, bool bIdle)
{
	for (int Index = 0; Index < Count; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = FirstId + Index;
		Unit.OwnerSlot = Slot;
		Unit.Type = Type;
		Unit.Health = Type == n"ManAtArms" ? 155 : 80;
		Unit.bIdle = bIdle;
		Unit.TargetId = -1;
		Unit.Location = Where;
		List.Add(Unit);
	}
}

// Ten idle Spearmen at the assembly point (-1500, 0), two enemy Spearmen (slot 0) at the enemy centre (-8000, 0).
FHOLAISummary CmpSummary()
{
	FHOLAISummary Summary;
	Summary.bHasCentre = true;
	Summary.CentreLocation = FVector(0, 0, 0);
	Summary.bHasEnemyCentre = true;
	Summary.EnemyCentreLocation = FVector(-8000, 0, 0);
	FHOLAIBuilding House;
	House.TypeName = n"House";
	House.State = EHOLConstructionState::Finished;
	House.Location = FVector(1500, 0, 0);
	Summary.Buildings.Add(House);
	FHOLAIBuilding Barracks;
	Barracks.TypeName = n"Barracks";
	Barracks.State = EHOLConstructionState::Finished;
	Barracks.Location = FVector(-1500, 1500, 0);
	Summary.Buildings.Add(Barracks);
	CmpAdd(Summary.Army, 1, n"Spearman", 10, FVector(-1500, 0, 0), 1, true);
	CmpAdd(Summary.Enemies, 50, n"Spearman", 2, FVector(-8000, 500, 0), 0, false);
	return Summary;
}

// A camp of Count units of a type, slot 8, on the way to the enemy centre.
void CmpCamp(FHOLAISummary& Summary, int FirstId, FName Type, int Count)
{
	const int Slot = HOLAIArmy::NeutralOwnerSlot;
	CmpAdd(Summary.Enemies, FirstId, Type, Count, FVector(-4000, 0, 0), Slot, false);
}

void Test_CampUnitsAreTheNeutralOwnerOnly(FUnitTest& T)
{
	FHOLAISummary Summary = CmpSummary();
	CmpCamp(Summary, 200, n"Spearman", 3);
	CmpAdd(Summary.Enemies, 300, n"Spearman", 1, FVector(-8000, 800, 0), 5, false);
	T.AssertEquals(3, HOLAIArmy::CampUnits(Summary).Num(), "the three units of slot 8 are the camp, the unit of slot 5 is not");
	FHOLAISummary Seen = HOLAIArmy::AdversaryView(Summary);
	T.AssertEquals(3, Seen.Enemies.Num(), "the adversaries alone: the 2 of the base summary and the one of slot 5");
	T.AssertEquals(6, Summary.Enemies.Num(), "the summary itself is not changed");
	T.AssertEquals(10, Seen.Army.Num(), "our army is kept");
}

void Test_CampRatioIsTheCampAlone(FUnitTest& T)
{
	const FHOLAIArmyData Data = CmpData();
	FHOLAISummary Bare = CmpSummary();
	T.AssertTrue(HOLAIArmy::CampRatio(Data, Bare) >= Data.LargeRatio, "no camp seen: the large value");
	FHOLAISummary Weak = CmpSummary();
	CmpCamp(Weak, 200, n"Spearman", 2);
	T.AssertTrue(HOLAIArmy::CampRatio(Data, Weak) >= Data.AttackLaunchRatio, "10 Spearmen against a camp of 2 Spearmen: ahead by the margin");
	FHOLAISummary Strong = CmpSummary();
	CmpCamp(Strong, 200, n"ManAtArms", 12);
	T.AssertTrue(HOLAIArmy::CampRatio(Data, Strong) < Data.AttackLaunchRatio, "10 Spearmen against a camp of 12 Men-at-arms: not ahead");
	T.AssertTrue(HOLAIArmy::CampRatio(Data, Strong) > 0.f, "the ratio is a value, not the stub's zero");
}

// Policy 1 (default, today): the camp counts like any enemy, the wave does not leave while it could not beat it.
void Test_CampCountsInTheLaunchByDefault(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = CmpSummary();
	CmpCamp(Summary, 200, n"ManAtArms", 12);
	T.AssertEquals(1, CmpData().CampPolicy, "the default policy keeps today's behaviour");
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, CmpData(), false, 500.f), "10 Spearmen, 2 enemy Spearmen and a camp of 12 Men-at-arms: under the margin");
}

// Policy 0: the camp is left out of the ratio, the wave leaves against the adversaries.
void Test_CampLeftOutWhenPolicyZero(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = CmpSummary();
	CmpCamp(Summary, 200, n"ManAtArms", 12);
	FHOLAIArmyData Data = CmpData();
	Data.CampPolicy = 0;
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, Summary, Data, false, 500.f), "policy 0: only the 2 enemy Spearmen weigh, far ahead");
}

// Leaving the camp out must not hide an adversary: policy 0 with a strong adversary still holds the wave.
void Test_CampPolicyZeroStillSeesTheAdversary(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = CmpSummary();
	CmpAdd(Summary.Enemies, 70, n"ManAtArms", 12, FVector(-8000, -500, 0), 1, false);
	CmpCamp(Summary, 200, n"Spearman", 2);
	FHOLAIArmyData Data = CmpData();
	Data.CampPolicy = 0;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, Data, false, 500.f), "10 Spearmen against 12 Men-at-arms of slot 1: under the margin whatever the camp");
}
