// The attack wave and the retreat alone (IA2.6, HOLAIArmy.as): when a wave is launched (each condition failing alone), what it
// orders, when it falls back (the ratio collapses, the losses pass the share, hysteresis between the two ratios), when it is home or
// over, on hand-made summaries (AtkData / AtkSummary / AtkAdd of HOLAIDefence_Test.as: the table numbers typed in). The chain
// (summary -> script -> attack-move orders) is tested in C++ (HeroesOfLiberty.AI.Army.Attack).

FHOLAITypeFigures AtkFigures(int Health, int Damage, float Interval, float Range, int MeleeArmor, int RangedArmor, bool bRanged, float Speed, int Food, int Wood, int Gold)
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

FHOLAIArmyData AtkData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Types.Empty();
	Data.Figures.Empty();
	FHOLAITypeFigures Spear = AtkFigures(80, 8, 1.875f, 130.f, 0, 0, false, 777.f, 60, 20, 0);
	Spear.ArmorCategories.Add(n"LightInfantry");
	Spear.BonusCategories.Add(n"Cavalry");
	Spear.BonusDamage.Add(17.f);
	Data.Types.Add(n"Spearman");
	Data.Figures.Add(Spear);
	FHOLAITypeFigures Heavy = AtkFigures(155, 12, 1.375f, 70.f, 4, 4, false, 697.f, 100, 0, 20);
	Heavy.ArmorCategories.Add(n"HeavyInfantry");
	Heavy.ArmorCategories.Add(n"Heavy");
	Data.Types.Add(n"ManAtArms");
	Data.Figures.Add(Heavy);
	FHOLAITypeFigures Bow = AtkFigures(70, 5, 1.625f, 2000.f, 0, 0, true, 779.f, 30, 50, 0);
	Bow.ArmorCategories.Add(n"Ranged");
	Bow.BonusCategories.Add(n"LightInfantry");
	Bow.BonusDamage.Add(5.f);
	Data.Types.Add(n"Archer");
	Data.Figures.Add(Bow);
	return Data;
}

// The units of a list: Count of a type at a place, identifiers from FirstId.
void AtkAdd(TArray<FHOLAIUnit>& List, int FirstId, FName Type, int Count, FVector Where, int Slot, bool bIdle, int TargetId)
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
FHOLAISummary AtkBase()
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

FVector AtkPoint()
{
	return FVector(-1500, 0, 0);
}

bool AtkNear(FVector A, FVector B)
{
	return (A - B).Size() < 1.0;
}

// The summary of a slot ready to attack: ten idle Spearmen at the assembly point (-1500, 0), two enemy Spearmen at the enemy centre
// (-8000, 0). Ratio far above 1.5, game time 500 s.
FHOLAISummary AtkSummary(int Soldiers)
{
	FHOLAISummary Summary = AtkBase();
	AtkAdd(Summary.Army, 1, n"Spearman", Soldiers, AtkPoint(), 1, true, -1);
	AtkAdd(Summary.Enemies, 50, n"Spearman", 2, FVector(-8000, 500, 0), 0, false, -1);
	return Summary;
}

// A wave that is out with the soldiers of the summary (as the launch makes it).
FHOLAIWave AtkWaveOut(const FHOLAISummary& Summary, float Now)
{
	FHOLAIWave Wave;
	Wave.Phase = EHOLAIWavePhase::Out;
	Wave.Ids = HOLAIArmy::SoldierIds(Summary);
	Wave.LaunchedSize = Wave.Ids.Num();
	Wave.PhaseStart = Now;
	Wave.LastOrder = Now;
	Wave.NextLaunch = Now + 90.f;
	return Wave;
}

// ---------------------------------------------------------------------------------------------------------------------------
// The launch, each condition failing alone.

void Test_AttackLaunchesWhenAllHold(FUnitTest& T)
{
	FHOLAIWave Wave;
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, AtkSummary(10), AtkData(), false, 500.f), "late enough, 10 soldiers, far ahead, at home, no cooldown, enemy centre known");
}

void Test_AttackTooEarly(FUnitTest& T)
{
	FHOLAIWave Wave;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, AtkSummary(10), AtkData(), false, 300.f), "enemy soldiers seen and 300 s < 420 s: too early");
}

void Test_AttackTooFewSoldiers(FUnitTest& T)
{
	FHOLAIWave Wave;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, AtkSummary(7), AtkData(), false, 500.f), "7 soldiers < 8");
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, AtkSummary(8), AtkData(), false, 500.f), "8 soldiers are enough");
}

void Test_AttackPeasantsAreNotSoldiers(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = AtkSummary(7);
	AtkAdd(Summary.Army, 100, n"Peasant", 5, AtkPoint(), 1, true, -1);
	T.AssertEquals(7, HOLAIArmy::SoldierIds(Summary).Num(), "the Peasants of the list are not soldiers");
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, AtkData(), false, 500.f), "7 soldiers and 5 Peasants: too few");
}

void Test_AttackRatioTooLow(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = AtkSummary(10);
	AtkAdd(Summary.Enemies, 70, n"Spearman", 10, FVector(-8000, -500, 0), 0, false, -1);
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, AtkData(), false, 500.f), "10 Spearmen against 12: under the 1.5 margin");
}

void Test_AttackNotWhileDefending(FUnitTest& T)
{
	FHOLAIWave Wave;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, AtkSummary(10), AtkData(), true, 500.f), "defending: no launch");
}

void Test_AttackCooldown(FUnitTest& T)
{
	FHOLAIWave Wave;
	Wave.NextLaunch = 510.f;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, AtkSummary(10), AtkData(), false, 500.f), "cooldown not over");
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, AtkSummary(10), AtkData(), false, 510.f), "cooldown over");
}

void Test_AttackNoEnemyCentre(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = AtkSummary(10);
	Summary.bHasEnemyCentre = false;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, AtkData(), false, 500.f), "no enemy centre known: nowhere to go");
}

void Test_AttackNotWhileOut(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 100.f);
	Wave.NextLaunch = 0.f;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, AtkData(), false, 500.f), "a wave already out: no second launch");
}

void Test_AttackUnseenEnemyNeedsASmallerTime(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = AtkBase();
	AtkAdd(Summary.Army, 1, n"Spearman", 10, AtkPoint(), 1, true, -1);
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, Summary, AtkData(), false, 350.f), "no enemy soldier seen, centre known: ratio large, 350 s >= 300 s");
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, AtkData(), false, 250.f), "...but not at 250 s");
	AtkAdd(Summary.Enemies, 50, n"Peasant", 4, FVector(-8000, 0, 0), 0, false, -1);
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, Summary, AtkData(), false, 350.f), "enemy Peasants only: still no soldier seen");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The wave's life.

void Test_AttackLaunchOrdersEveryoneToTheEnemyCentre(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	AtkAdd(Summary.Army, 100, n"Peasant", 3, AtkPoint(), 1, true, -1);
	FHOLAIWave Wave;
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 500.f);
	T.AssertTrue(Step.bLaunched, "launched");
	T.AssertEquals(10, Step.AttackIds.Num(), "all ten soldiers, no Peasant");
	T.AssertTrue(AtkNear(Step.AttackTarget, FVector(-8000, 0, 0)), "toward the enemy centre");
	T.AssertTrue(Wave.Phase == EHOLAIWavePhase::Out, "the wave is out");
	T.AssertEquals(10, Wave.Ids.Num(), "it keeps the ids it launched");
	T.AssertEquals(10, Wave.LaunchedSize, "and its size");
	T.AssertTrue(Wave.NextLaunch >= 590.f - 0.01f, "the cooldown starts");
	T.AssertTrue(Step.Ratio > 1.5f, "the ratio is reported");
}

void Test_AttackDefendingBlocksTheLaunchInTheUpdate(FUnitTest& T)
{
	FHOLAIWave Wave;
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, AtkSummary(10), AtkData(), true, false, AtkPoint(), 500.f);
	T.AssertFalse(Step.bLaunched, "defending: not launched");
	T.AssertTrue(Wave.Phase == EHOLAIWavePhase::Home, "the wave stays home");
}

void Test_AttackIdleSoldiersOrderedAgainEveryFewSeconds(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	const FHOLAIWaveStep Soon = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 500.5f);
	T.AssertEquals(0, Soon.AttackIds.Num(), "half a second after the last order: nothing (D751: the wave is looked at every second)");
	T.AssertFalse(Soon.bRetreat || Soon.bOver, "the wave goes on");
	const FHOLAIWaveStep Later = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 506.f);
	T.AssertEquals(10, Later.AttackIds.Num(), "6 s later the idle soldiers are ordered again");
	T.AssertTrue(AtkNear(Later.AttackTarget, FVector(-8000, 0, 0)), "toward the enemy centre");
	// Soldiers that strike are left alone.
	Summary.Army[0].TargetId = 50;
	Summary.Army[0].bIdle = false;
	const FHOLAIWaveStep Next = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 512.f);
	T.AssertEquals(9, Next.AttackIds.Num(), "the one that strikes is not ordered");
}

void Test_AttackCentreGoneAimsAtTheNearestEnemy(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	Summary.bHasEnemyCentre = false;
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 510.f);
	T.AssertFalse(Step.bOver, "enemies are still seen: the wave goes on");
	T.AssertEquals(10, Step.AttackIds.Num(), "the idle soldiers are ordered");
	T.AssertTrue(AtkNear(Step.AttackTarget, FVector(-8000, 500, 0)), "toward the nearest enemy unit");
}

void Test_AttackRetreatWhenTheRatioCollapses(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	AtkAdd(Summary.Enemies, 70, n"ManAtArms", 20, AtkPoint() + FVector(-2000, 0, 0), 0, false, -1);
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 510.f);
	T.AssertTrue(Step.bRetreat, "20 Men-at-arms against 10 Spearmen: fall back");
	T.AssertEquals(10, Step.MoveIds.Num(), "every wave soldier is ordered to move");
	T.AssertEquals(0, Step.AttackIds.Num(), "not on an attack-move");
	T.AssertTrue(Wave.Phase == EHOLAIWavePhase::Back, "the wave is falling back");
	T.AssertTrue(Step.Ratio < 0.6f, "the ratio that decided is reported");
	T.AssertEquals(10, Step.Left, "ten left");
}

void Test_AttackHysteresisKeepsTheWaveOut(FUnitTest& T)
{
	// 10 Spearmen against 10 Spearmen next to them: the ratio is about 1, between 0.6 and 1.5.
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	AtkAdd(Summary.Enemies, 70, n"Spearman", 8, AtkPoint() + FVector(-2000, 0, 0), 0, false, -1);
	const FHOLAIArmyData Data = AtkData();
	FHOLAISummary Near = Summary;
	Near.Enemies.Empty();
	AtkAdd(Near.Enemies, 70, n"Spearman", 10, AtkPoint() + FVector(-2000, 0, 0), 0, false, -1);
	const float Ratio = HOLAIArmy::PowerRatio(Data, Near);
	T.AssertTrue(Ratio > Data.AttackRetreatRatio && Ratio < Data.AttackLaunchRatio, "the situation is between the two thresholds");
	FHOLAIWave Equal = AtkWaveOut(Near, 500.f);
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Equal, Near, Data, false, false, AtkPoint(), 510.f);
	T.AssertFalse(Step.bRetreat, "between the two ratios the wave stays out");
	T.AssertTrue(Equal.Phase == EHOLAIWavePhase::Out, "still out");
	// And the same ratio does not launch a wave at home.
	FHOLAIWave Home;
	T.AssertFalse(HOLAIArmy::CanLaunch(Home, Near, Data, false, 510.f), "the same ratio does not launch one");
}

void Test_AttackRetreatWhenLossesPassTheShare(FUnitTest& T)
{
	FHOLAISummary Launch = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Launch, 500.f);
	// 4 of 10 left: 60 % lost is not more than the share, 3 left is.
	FHOLAISummary Four = AtkSummary(4);
	FHOLAIWave Same = Wave;
	T.AssertFalse(HOLAIArmy::UpdateWave(Same, Four, AtkData(), false, false, AtkPoint(), 510.f).bRetreat, "60 % lost: the share is not passed");
	FHOLAISummary Three = AtkSummary(3);
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Three, AtkData(), false, false, AtkPoint(), 510.f);
	T.AssertTrue(Step.bRetreat, "70 % lost: fall back");
	T.AssertEquals(3, Step.MoveIds.Num(), "the three left are ordered");
	T.AssertEquals(3, Step.Left, "three left");
}

void Test_AttackRecallByTheDefence(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, true, AtkPoint(), 510.f);
	T.AssertTrue(Step.bRetreat, "recalled: fall back");
	T.AssertTrue(Wave.Phase == EHOLAIWavePhase::Back, "back");
}

void Test_AttackRetreatRestartsTheCooldownLonger(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	Wave.Phase = EHOLAIWavePhase::Back;
	Wave.PhaseStart = 510.f;
	// All at the assembly point: home.
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 520.f);
	T.AssertTrue(Step.bHome, "most are at the assembly point: home");
	T.AssertTrue(Wave.Phase == EHOLAIWavePhase::Home, "home");
	T.AssertTrue(Wave.NextLaunch >= 520.f + 180.f - 0.01f, "twice the cooldown after a retreat");
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, AtkSummary(10), AtkData(), false, 600.f), "no new wave 80 s after a retreat");
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, AtkSummary(10), AtkData(), false, 701.f), "...but one after 180 s");
}

void Test_AttackBackStaysBackWhileFarThenTimesOut(FUnitTest& T)
{
	FHOLAISummary Summary = AtkBase();
	AtkAdd(Summary.Army, 1, n"Spearman", 6, FVector(-6000, 0, 0), 1, false, -1);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	Wave.Phase = EHOLAIWavePhase::Back;
	Wave.PhaseStart = 510.f;
	T.AssertFalse(HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 530.f).bHome, "far from the point after 20 s: still falling back");
	T.AssertTrue(Wave.Phase == EHOLAIWavePhase::Back, "back");
	T.AssertTrue(HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 551.f).bHome, "after 40 s: home anyway");
}

void Test_AttackOverWhenNothingIsLeftToAttack(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	Summary.bHasEnemyCentre = false;
	Summary.Enemies.Empty();
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, AtkData(), false, false, AtkPoint(), 510.f);
	T.AssertTrue(Step.bOver, "no enemy centre, no enemy seen: the wave is over");
	T.AssertTrue(Wave.Phase == EHOLAIWavePhase::Home, "home");
	T.AssertEquals(0, Wave.Ids.Num(), "the wave forgets its soldiers");
	T.AssertTrue(Wave.NextLaunch >= 510.f + 90.f - 0.01f, "the cooldown restarts");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The soldiers the defence and the regrouping may order.

void Test_AttackWithoutWaveLeavesTheHomeSoldiers(FUnitTest& T)
{
	FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Wave = AtkWaveOut(Summary, 500.f);
	AtkAdd(Summary.Army, 200, n"Archer", 2, AtkPoint(), 1, true, -1);
	T.AssertEquals(2, HOLAIArmy::WithoutWave(Summary, Wave).Army.Num(), "a wave out: only the two new Archers are left");
	T.AssertEquals(10, HOLAIArmy::WaveSize(Wave, Summary), "the wave has ten alive");
	Wave.Phase = EHOLAIWavePhase::Home;
	T.AssertEquals(12, HOLAIArmy::WithoutWave(Summary, Wave).Army.Num(), "no wave out: everyone");
}
