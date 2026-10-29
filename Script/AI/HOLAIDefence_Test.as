// Regrouping and base defence alone (IA2.5, HOLAIArmy.as): the threat to the base, whether to meet it, the defence state with its quiet
// delay, the soldiers sent back to the assembly point, on hand-made summaries (the table numbers typed in, so that a table change does
// not move these tests). The chain (summary -> script -> attack-move orders) is tested in C++ (HeroesOfLiberty.AI.Army.Defence).

FHOLAITypeFigures DefFigures(int Health, int Damage, float Interval, float Range, int MeleeArmor, int RangedArmor, bool bRanged, float Speed, int Food, int Wood, int Gold)
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

FHOLAIArmyData DefData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Types.Empty();
	Data.Figures.Empty();
	FHOLAITypeFigures Spear = DefFigures(80, 8, 1.875f, 130.f, 0, 0, false, 777.f, 60, 20, 0);
	Spear.ArmorCategories.Add(n"LightInfantry");
	Spear.BonusCategories.Add(n"Cavalry");
	Spear.BonusDamage.Add(17.f);
	Data.Types.Add(n"Spearman");
	Data.Figures.Add(Spear);
	FHOLAITypeFigures Heavy = DefFigures(155, 12, 1.375f, 70.f, 4, 4, false, 697.f, 100, 0, 20);
	Heavy.ArmorCategories.Add(n"HeavyInfantry");
	Heavy.ArmorCategories.Add(n"Heavy");
	Data.Types.Add(n"ManAtArms");
	Data.Figures.Add(Heavy);
	FHOLAITypeFigures Bow = DefFigures(70, 5, 1.625f, 2000.f, 0, 0, true, 779.f, 30, 50, 0);
	Bow.ArmorCategories.Add(n"Ranged");
	Bow.BonusCategories.Add(n"LightInfantry");
	Bow.BonusDamage.Add(5.f);
	Data.Types.Add(n"Archer");
	Data.Figures.Add(Bow);
	return Data;
}

// The units of a list: Count of a type at a place, identifiers from FirstId.
void DefAdd(TArray<FHOLAIUnit>& List, int FirstId, FName Type, int Count, FVector Where, int Slot, bool bIdle, int TargetId)
{
	for (int Index = 0; Index < Count; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = FirstId + Index;
		Unit.OwnerSlot = Slot;
		Unit.Type = Type;
		Unit.Health = Type == n"Archer" ? 70 : (Type == n"ManAtArms" ? 155 : 80);
		Unit.bIdle = bIdle;
		Unit.TargetId = TargetId;
		Unit.Location = Where;
		List.Add(Unit);
	}
}

// A slot with its centre at the origin, a House at (1500, 0) and a Barracks at (-1500, 1500) (the enemy centre at (-8000, 0): the assembly
// point is at (-1500, 0)).
FHOLAISummary DefSummary()
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
	return Summary;
}

FVector DefPoint()
{
	return FVector(-1500, 0, 0);
}

bool DefNear(FVector A, FVector B)
{
	return (A - B).Size() < 1.0;
}

// ---------------------------------------------------------------------------------------------------------------------------
// The threat.

void Test_DefenceNoEnemyNoThreat(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Spearman", 3, DefPoint(), 1, true, -1);
	T.AssertFalse(HOLAIArmy::Threat(Summary, DefData()).bThreatened, "no enemy: no threat");
}

void Test_DefenceSpearmanNearHouseThreatens(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(3500, 0, 0), 0, false, -1);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, DefData());
	T.AssertTrue(Threat.bThreatened, "an enemy Spearman 2000 cm from the House threatens");
	T.AssertEquals(1, Threat.Count, "one threatening enemy");
	T.AssertTrue(DefNear(Threat.Location, FVector(3500, 0, 0)), "the threat is where the enemy stands");
}

void Test_DefenceSpearmanFarAwayDoesNot(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(7500, 0, 0), 0, false, -1);
	T.AssertFalse(HOLAIArmy::Threat(Summary, DefData()).bThreatened, "an enemy Spearman 6000 cm from the House does not");
}

void Test_DefenceNearTheCentreAlone(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	Summary.Buildings.Empty();
	DefAdd(Summary.Enemies, 50, n"Archer", 1, FVector(0, 2500, 0), 0, false, -1);
	T.AssertTrue(HOLAIArmy::Threat(Summary, DefData()).bThreatened, "within the radius of the centre alone: threat");
}

void Test_DefencePeasantsDoNotThreaten(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Enemies, 50, n"Peasant", 4, FVector(1500, 500, 0), 0, false, -1);
	T.AssertFalse(HOLAIArmy::Threat(Summary, DefData()).bThreatened, "enemy Peasants near the House: no threat (scouts and builders)");
}

void Test_DefenceThreatPointIsTheNearestGroup(FUnitTest& T)
{
	// Two enemies together 2500 cm east of the centre, one alone far north (still within 3000 of the House? no: 5000 cm).
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(2500, 400, 0), 0, false, -1);
	DefAdd(Summary.Enemies, 51, n"Spearman", 1, FVector(2500, -400, 0), 0, false, -1);
	DefAdd(Summary.Enemies, 52, n"Spearman", 1, FVector(0, 5000, 0), 0, false, -1);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, DefData());
	T.AssertEquals(2, Threat.Count, "two threatening enemies");
	T.AssertTrue(DefNear(Threat.Location, FVector(2500, 0, 0)), "their centroid");
}

void Test_DefenceThreatRatioFollowsTheArmies(FUnitTest& T)
{
	FHOLAISummary Strong = DefSummary();
	DefAdd(Strong.Army, 1, n"Spearman", 6, DefPoint(), 1, true, -1);
	DefAdd(Strong.Enemies, 50, n"Spearman", 1, FVector(2500, 0, 0), 0, false, -1);
	T.AssertTrue(HOLAIArmy::Threat(Strong, DefData()).Ratio > 1.f, "6 Spearmen against 1: ahead");
	FHOLAISummary Weak = DefSummary();
	DefAdd(Weak.Army, 1, n"Archer", 1, DefPoint(), 1, true, -1);
	DefAdd(Weak.Enemies, 50, n"ManAtArms", 8, FVector(2500, 0, 0), 0, false, -1);
	T.AssertTrue(HOLAIArmy::Threat(Weak, DefData()).Ratio < 0.3f, "1 Archer against 8 Men-at-arms: far behind");
}

// ---------------------------------------------------------------------------------------------------------------------------
// Whether to meet it.

void Test_DefenceStrongArmyDefends(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Spearman", 6, DefPoint(), 1, true, -1);
	DefAdd(Summary.Enemies, 50, n"Spearman", 2, FVector(3500, 0, 0), 0, false, -1);
	const FHOLAIArmyData Data = DefData();
	T.AssertTrue(HOLAIArmy::ShouldDefend(Summary, Data, HOLAIArmy::Threat(Summary, Data), DefPoint()), "ahead: meet them");
}

void Test_DefenceNoThreatNoDefence(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Spearman", 6, DefPoint(), 1, true, -1);
	const FHOLAIArmyData Data = DefData();
	T.AssertFalse(HOLAIArmy::ShouldDefend(Summary, Data, HOLAIArmy::Threat(Summary, Data), DefPoint()), "no enemy: no defence");
}

void Test_DefenceWeakArmyHoldsThePoint(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Archer", 1, DefPoint(), 1, true, -1);
	DefAdd(Summary.Enemies, 50, n"ManAtArms", 8, FVector(3500, 0, 0), 0, false, -1);
	const FHOLAIArmyData Data = DefData();
	T.AssertFalse(HOLAIArmy::ShouldDefend(Summary, Data, HOLAIArmy::Threat(Summary, Data), DefPoint()), "far behind and the enemy far from the point: no suicide");
}

void Test_DefenceWeakArmyDefendsTheAssemblyPoint(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Archer", 1, DefPoint(), 1, true, -1);
	DefAdd(Summary.Enemies, 50, n"ManAtArms", 8, FVector(-1500, 700, 0), 0, false, -1);
	const FHOLAIArmyData Data = DefData();
	T.AssertTrue(HOLAIArmy::ShouldDefend(Summary, Data, HOLAIArmy::Threat(Summary, Data), DefPoint()), "the enemy reaches the assembly point: fight there");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The state: defending until the quiet delay has passed.

void Test_DefenceStateStartsAtOnce(FUnitTest& T)
{
	FHOLAIDefence State;
	HOLAIArmy::UpdateDefence(State, true, 100.f, DefData());
	T.AssertTrue(State.bDefending, "wanted: defending");
}

void Test_DefenceStateEndsAfterTheQuietDelay(FUnitTest& T)
{
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	HOLAIArmy::UpdateDefence(State, true, 100.f, Data);
	HOLAIArmy::UpdateDefence(State, false, 102.f, Data);
	T.AssertTrue(State.bDefending, "2 s without threat: still defending");
	HOLAIArmy::UpdateDefence(State, false, 104.9f, Data);
	T.AssertTrue(State.bDefending, "4.9 s: still defending");
	HOLAIArmy::UpdateDefence(State, false, 105.f, Data);
	T.AssertFalse(State.bDefending, "5 s without threat: home again");
}

void Test_DefenceStateQuietRestartsWithTheThreat(FUnitTest& T)
{
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	HOLAIArmy::UpdateDefence(State, true, 100.f, Data);
	HOLAIArmy::UpdateDefence(State, false, 103.f, Data);
	HOLAIArmy::UpdateDefence(State, true, 104.f, Data);
	HOLAIArmy::UpdateDefence(State, false, 108.f, Data);
	T.AssertTrue(State.bDefending, "the threat came back at 104: 4 s of quiet only");
	HOLAIArmy::UpdateDefence(State, false, 109.f, Data);
	T.AssertFalse(State.bDefending, "5 s after the last threat: home");
}

void Test_DefenceNotDefendingStaysHome(FUnitTest& T)
{
	FHOLAIDefence State;
	HOLAIArmy::UpdateDefence(State, false, 10.f, DefData());
	T.AssertFalse(State.bDefending, "never wanted: home");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The defence orders: every soldier, then again every few seconds to those not striking.

void Test_DefenceOrdersEverySoldier(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Spearman", 3, DefPoint(), 1, true, -1);
	DefAdd(Summary.Army, 10, n"Archer", 2, FVector(-3000, 0, 0), 1, false, -1);
	DefAdd(Summary.Army, 20, n"Spearman", 1, FVector(-3000, 0, 0), 1, false, 60);
	DefAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(3500, 0, 0), 0, false, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, Data);
	const TArray<int> Ids = HOLAIArmy::DefenceOrders(State, Summary, Data, Threat, 100.f);
	T.AssertEquals(6, Ids.Num(), "all six soldiers: the idle, the moving (assembling) and the one striking");
	T.AssertTrue(Ids.Contains(1) && Ids.Contains(11) && Ids.Contains(20), "among them");
}

void Test_DefenceOrdersNotAgainTooSoon(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Spearman", 3, DefPoint(), 1, true, -1);
	DefAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(3500, 0, 0), 0, false, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, Data);
	HOLAIArmy::DefenceOrders(State, Summary, Data, Threat, 100.f);
	T.AssertEquals(0, HOLAIArmy::DefenceOrders(State, Summary, Data, Threat, 101.f).Num(), "1 s later: nothing");
	T.AssertEquals(0, HOLAIArmy::DefenceOrders(State, Summary, Data, Threat, 102.9f).Num(), "2.9 s later: nothing");
	T.AssertEquals(3, HOLAIArmy::DefenceOrders(State, Summary, Data, Threat, 103.f).Num(), "3 s later: again");
}

void Test_DefenceOrdersAgainSkipTheStriking(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Spearman", 2, DefPoint(), 1, true, -1);
	DefAdd(Summary.Army, 10, n"Spearman", 2, FVector(3000, 0, 0), 1, false, 50);
	DefAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(3500, 0, 0), 0, false, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, Data);
	T.AssertEquals(4, HOLAIArmy::DefenceOrders(State, Summary, Data, Threat, 100.f).Num(), "the first order: all four");
	T.AssertEquals(2, HOLAIArmy::DefenceOrders(State, Summary, Data, Threat, 104.f).Num(), "the next: the two not striking");
}

void Test_DefenceOrdersNeedAThreat(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Spearman", 3, DefPoint(), 1, true, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	T.AssertEquals(0, HOLAIArmy::DefenceOrders(State, Summary, Data, HOLAIArmy::Threat(Summary, Data), 100.f).Num(), "no threat now: no order");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The regrouping.

void Test_RegroupSendsTheIdleFarOnes(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Archer", 2, FVector(1500, 2500, 0), 1, true, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	const TArray<int> Ids = HOLAIArmy::RegroupOrders(State, Summary, Data, DefPoint(), 100.f);
	T.AssertEquals(2, Ids.Num(), "two idle Archers far from the point: both regroup");
}

void Test_RegroupLeavesTheOthersAlone(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Archer", 1, FVector(1500, 2500, 0), 1, true, 60);
	DefAdd(Summary.Army, 2, n"Archer", 1, FVector(1500, 2500, 0), 1, false, -1);
	DefAdd(Summary.Army, 3, n"Spearman", 1, FVector(-1500, 600, 0), 1, true, -1);
	DefAdd(Summary.Army, 4, n"Spearman", 1, FVector(-1500, 1000, 0), 1, true, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	T.AssertEquals(0, HOLAIArmy::RegroupOrders(State, Summary, Data, DefPoint(), 100.f).Num(),
		"fighting, moving, within 600 cm and exactly 1000 cm of the point: left alone");
}

void Test_RegroupNotAgainBeforeTheDelay(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Archer", 2, FVector(1500, 2500, 0), 1, true, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	HOLAIArmy::RegroupOrders(State, Summary, Data, DefPoint(), 100.f);
	T.AssertEquals(0, HOLAIArmy::RegroupOrders(State, Summary, Data, DefPoint(), 105.f).Num(), "5 s later: not again");
	T.AssertEquals(2, HOLAIArmy::RegroupOrders(State, Summary, Data, DefPoint(), 110.f).Num(), "10 s later: again");
}

void Test_RegroupOnlyTheNewOnesWhenSomeWereSent(FUnitTest& T)
{
	FHOLAISummary Summary = DefSummary();
	DefAdd(Summary.Army, 1, n"Archer", 1, FVector(1500, 2500, 0), 1, true, -1);
	const FHOLAIArmyData Data = DefData();
	FHOLAIDefence State;
	HOLAIArmy::RegroupOrders(State, Summary, Data, DefPoint(), 100.f);
	DefAdd(Summary.Army, 2, n"Archer", 1, FVector(1500, 2500, 0), 1, true, -1);
	const TArray<int> Ids = HOLAIArmy::RegroupOrders(State, Summary, Data, DefPoint(), 103.f);
	T.AssertEquals(1, Ids.Num(), "a new soldier is sent, the first waits");
	T.AssertTrue(Ids.Contains(2), "the new one");
}
