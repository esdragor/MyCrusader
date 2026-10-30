// The forum rules alone (IA3.2, HOLAIForum.as): the Peasants that rebuild a fallen forum, the last life, the defence at any ratio, on
// hand-made summaries. The chain (summary -> script -> build orders) is played by the scenario ai_forum.

FHOLAITypeFigures FrmFigures(int Health, int Damage, float Interval, float Range, int MeleeArmor, int RangedArmor, bool bRanged, float Speed, int Food, int Wood, int Gold)
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

FHOLAIArmyData FrmData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Types.Empty();
	Data.Figures.Empty();
	FHOLAITypeFigures Spear = FrmFigures(80, 8, 1.875f, 130.f, 0, 0, false, 777.f, 60, 20, 0);
	Spear.ArmorCategories.Add(n"LightInfantry");
	Spear.BonusCategories.Add(n"Cavalry");
	Spear.BonusDamage.Add(17.f);
	Data.Types.Add(n"Spearman");
	Data.Figures.Add(Spear);
	FHOLAITypeFigures Heavy = FrmFigures(155, 12, 1.375f, 70.f, 4, 4, false, 697.f, 100, 0, 20);
	Heavy.ArmorCategories.Add(n"HeavyInfantry");
	Heavy.ArmorCategories.Add(n"Heavy");
	Data.Types.Add(n"ManAtArms");
	Data.Figures.Add(Heavy);
	FHOLAITypeFigures Bow = FrmFigures(70, 5, 1.625f, 2000.f, 0, 0, true, 779.f, 30, 50, 0);
	Bow.ArmorCategories.Add(n"Ranged");
	Bow.BonusCategories.Add(n"LightInfantry");
	Bow.BonusDamage.Add(5.f);
	Data.Types.Add(n"Archer");
	Data.Figures.Add(Bow);
	return Data;
}

// The units of a list: Count of a type at a place, identifiers from FirstId.
void FrmAdd(TArray<FHOLAIUnit>& List, int FirstId, FName Type, int Count, FVector Where, int Slot, bool bIdle, int TargetId)
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
FHOLAISummary FrmBase()
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

FVector FrmPoint()
{
	return FVector(-1500, 0, 0);
}

FHOLAISummary FrmSummary(int Lives, bool bDown, int Peasants)
{
	FHOLAISummary Summary = FrmBase();
	Summary.bHasOwnForum = true;
	Summary.OwnForum.Slot = 0;
	Summary.OwnForum.Lives = Lives;
	Summary.OwnForum.bDown = bDown;
	Summary.OwnForum.bRuin = bDown;
	Summary.OwnForum.bHasLocation = true;
	Summary.OwnForum.Location = FVector(0, 0, 0);
	for (int Index = 0; Index < Peasants; Index++)
	{
		FHOLAIUnit Peasant;
		Peasant.Id = 100 + Index;
		Peasant.OwnerSlot = 0;
		Peasant.Type = n"Peasant";
		Peasant.Health = 25;
		Peasant.Location = FVector(1000.f * float(Index + 1), 0, 0);
		Summary.Peasants.Add(Peasant);
		Summary.PeasantIds.Add(Peasant.Id);
	}
	return Summary;
}

void Test_ForumStandingNothingToRebuild(FUnitTest& T)
{
	FHOLAIForumMemory Memory;
	Memory.Sent.Add(100);
	const FHOLAISummary Summary = FrmSummary(2, false, 8);
	T.AssertEquals(0, HOLAIForum::RebuildOrders(Memory, Summary, HOLAIForum::DefaultData(), 50.f).Num(), "a standing forum: no rebuilder");
	T.AssertEquals(0, Memory.Sent.Num(), "the memory is cleared");
	T.AssertFalse(HOLAIForum::OwnDown(Summary), "not down");
	T.AssertFalse(HOLAIForum::Emergency(Summary), "no emergency");
}

void Test_ForumFewRebuildWithSeveralLives(FUnitTest& T)
{
	FHOLAIForumMemory Memory;
	const FHOLAIForumData Data = HOLAIForum::DefaultData();
	const FHOLAISummary Summary = FrmSummary(2, true, 8);
	T.AssertTrue(HOLAIForum::OwnDown(Summary), "down");
	T.AssertFalse(HOLAIForum::Emergency(Summary), "two lives left: no emergency");
	const TArray<int> Ids = HOLAIForum::RebuildOrders(Memory, Summary, Data, 50.f);
	T.AssertEquals(Data.RebuildersSpare, Ids.Num(), "RebuildersSpare Peasants go");
	T.AssertTrue(Ids.Contains(100) && Ids.Contains(104) && !Ids.Contains(105), "the nearest ones to the forum");
	T.AssertEquals(Data.RebuildersSpare, Memory.Sent.Num(), "remembered");
}

void Test_ForumAllRebuildOnTheLastLife(FUnitTest& T)
{
	FHOLAIForumMemory Memory;
	const FHOLAISummary Summary = FrmSummary(1, true, 8);
	T.AssertTrue(HOLAIForum::Emergency(Summary), "the last life, down: emergency");
	T.AssertEquals(8, HOLAIForum::RebuildOrders(Memory, Summary, HOLAIForum::DefaultData(), 50.f).Num(), "all the Peasants go");
}

void Test_ForumRebuildersAreNotSentAgain(FUnitTest& T)
{
	FHOLAIForumMemory Memory;
	const FHOLAIForumData Data = HOLAIForum::DefaultData();
	FHOLAISummary Summary = FrmSummary(1, true, 8);
	HOLAIForum::RebuildOrders(Memory, Summary, Data, 50.f);
	T.AssertEquals(0, HOLAIForum::RebuildOrders(Memory, Summary, Data, 50.f + Data.OrderEvery).Num(), "all sent already: nobody more");

	// Two Peasants are born: sent after the delay, not before. One of the first dies: not counted any more.
	FHOLAISummary Later = FrmSummary(1, true, 10);
	T.AssertEquals(0, HOLAIForum::RebuildOrders(Memory, Later, Data, 50.f + 0.5f * Data.OrderEvery).Num(), "not within the delay of the last choice");
	T.AssertEquals(2, HOLAIForum::RebuildOrders(Memory, Later, Data, 50.f + 2.f * Data.OrderEvery).Num(), "the two new ones go");
	T.AssertEquals(10, Memory.Sent.Num(), "ten are sent");

	FHOLAISummary Fewer = FrmSummary(1, true, 7);
	HOLAIForum::RebuildOrders(Memory, Fewer, Data, 100.f);
	T.AssertEquals(7, Memory.Sent.Num(), "the dead ones are forgotten");
}

void Test_ForumRisenClearsTheMemory(FUnitTest& T)
{
	FHOLAIForumMemory Memory;
	const FHOLAIForumData Data = HOLAIForum::DefaultData();
	HOLAIForum::RebuildOrders(Memory, FrmSummary(1, true, 8), Data, 50.f);
	T.AssertEquals(8, Memory.Sent.Num(), "eight sent");
	HOLAIForum::RebuildOrders(Memory, FrmSummary(1, false, 8), Data, 60.f);
	T.AssertEquals(0, Memory.Sent.Num(), "the forum stands again: the memory is empty, the Peasants are free");
}

void Test_ForumLastLifeDefendsAtAnyRatio(FUnitTest& T)
{
	const FHOLAIArmyData Army = FrmData();
	FHOLAISummary Last = FrmSummary(1, true, 3);
	FrmAdd(Last.Army, 1, n"Spearman", 1, FrmPoint(), 0, true, -1);
	FrmAdd(Last.Enemies, 50, n"ManAtArms", 9, FVector(500, 0, 0), 1, false, -1);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Last, Army);
	T.AssertTrue(Threat.bThreatened && Threat.Ratio < Army.DefenceMinRatioAttacked, "a crushing threat at the ruin");
	T.AssertTrue(HOLAIArmy::ShouldDefend(Last, Army, Threat, FrmPoint()), "the last life: the army fights whatever the ratio");

	FHOLAISummary Spare = FrmSummary(2, true, 3);
	Spare.Army = Last.Army;
	Spare.Enemies = Last.Enemies;
	T.AssertFalse(HOLAIArmy::ShouldDefend(Spare, Army, HOLAIArmy::Threat(Spare, Army), FrmPoint()), "two lives left: the weak army keeps away");
}

void Test_ForumLastLifeRebuildersDoNotFlee(FUnitTest& T)
{
	const FHOLAIArmyData Army = FrmData();
	FHOLAIForumMemory Memory;
	FHOLAISummary Summary = FrmSummary(1, true, 3);
	FrmAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(1500, 0, 0), 1, false, -1);
	FHOLAIFlight Flight;
	T.AssertTrue(HOLAIArmy::FleeOrders(Flight, Summary, Army, 10.f).Num() > 0, "without the rebuilding, the Peasants near the Spearman run");
	HOLAIForum::RebuildOrders(Memory, Summary, HOLAIForum::DefaultData(), 10.f);
	FHOLAIFlight Fresh;
	T.AssertEquals(0, HOLAIArmy::FleeOrders(Fresh, HOLAIForum::Calm(Summary, Memory), Army, 10.f).Num(), "the rebuilders stay on the last life");

	FHOLAISummary Spare = FrmSummary(2, true, 3);
	FrmAdd(Spare.Enemies, 50, n"Spearman", 1, FVector(1500, 0, 0), 1, false, -1);
	FHOLAIForumMemory SpareMemory;
	HOLAIForum::RebuildOrders(SpareMemory, Spare, HOLAIForum::DefaultData(), 10.f);
	FHOLAIFlight Other;
	T.AssertTrue(HOLAIArmy::FleeOrders(Other, HOLAIForum::Calm(Spare, SpareMemory), Army, 10.f).Num() > 0, "with lives to spare they run as the others");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The enemy ruin (IA3.3): the wave goes and holds a fallen enemy forum.

FHOLAIForum FrmEnemyForum(int Slot, int Lives, bool bDown, FVector Where)
{
	FHOLAIForum Forum;
	Forum.Slot = Slot;
	Forum.Lives = Lives;
	Forum.bDown = bDown;
	Forum.bRuin = bDown;
	Forum.bHasLocation = true;
	Forum.Location = Where;
	return Forum;
}

// Ten idle Spearmen at the assembly point, two enemy Spearmen near the enemy centre (-8000, 0).
FHOLAISummary FrmReady()
{
	FHOLAISummary Summary = FrmBase();
	FrmAdd(Summary.Army, 1, n"Spearman", 10, FrmPoint(), 0, true, -1);
	FrmAdd(Summary.Enemies, 50, n"Spearman", 2, FVector(-8000, 500, 0), 1, false, -1);
	Summary.EnemyForums.Add(FrmEnemyForum(1, 2, false, FVector(-8000, 0, 0)));
	return Summary;
}

void Test_RuinTargetIsTheNearestFallenEnemyForum(FUnitTest& T)
{
	FHOLAISummary Summary = FrmReady();
	bool bFound = true;
	HOLAIForum::RuinTarget(Summary, FVector(0, 0, 0), bFound);
	T.AssertFalse(bFound, "every enemy forum stands: no ruin to hold");
	T.AssertFalse(HOLAIForum::EnemyRuin(Summary), "no enemy ruin");

	Summary.EnemyForums.Add(FrmEnemyForum(2, 1, true, FVector(9000, 0, 0)));
	Summary.EnemyForums.Add(FrmEnemyForum(3, 1, true, FVector(3000, 3000, 0)));
	const FVector Target = HOLAIForum::RuinTarget(Summary, FVector(0, 0, 0), bFound);
	T.AssertTrue(bFound && HOLAIForum::EnemyRuin(Summary), "a ruin");
	T.AssertTrue((Target - FVector(3000, 3000, 0)).Size() < 1.f, "the nearest fallen forum to the wave");
}

void Test_AttackStartsEarlyOnARuin(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Summary = FrmReady();
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, FrmData(), false, 100.f), "no ruin: 100 s is too early");
	Summary.EnemyForums[0].bDown = true;
	Summary.EnemyForums[0].bRuin = true;
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, Summary, FrmData(), false, 100.f), "a ruin to hold: the minimum time does not count");
}

void Test_AttackOnARuinStillNeedsTheRatioAndTheSoldiers(FUnitTest& T)
{
	FHOLAIWave Wave;
	FHOLAISummary Weak = FrmReady();
	Weak.EnemyForums[0].bDown = true;
	FrmAdd(Weak.Enemies, 70, n"ManAtArms", 9, FVector(-8000, 0, 0), 1, false, -1);
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Weak, FrmData(), false, 100.f), "a crushing enemy at the ruin: the army does not go");

	FHOLAISummary Few = FrmBase();
	FrmAdd(Few.Army, 1, n"Spearman", 5, FrmPoint(), 0, true, -1);
	Few.EnemyForums.Add(FrmEnemyForum(1, 1, true, FVector(-8000, 0, 0)));
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Few, FrmData(), false, 100.f), "too few soldiers");
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, FrmReady(), FrmData(), true, 500.f), "not while defending");
}

void Test_WaveGoesToTheRuinNotTheCentre(FUnitTest& T)
{
	FHOLAISummary Summary = FrmReady();
	Summary.EnemyForums.Add(FrmEnemyForum(2, 1, true, FVector(3000, 9000, 0)));
	FHOLAIWave Wave;
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, FrmData(), false, false, FrmPoint(), 100.f);
	T.AssertTrue(Step.bLaunched, "launched at 100 s: a ruin is there");
	T.AssertTrue((Step.AttackTarget - FVector(3000, 9000, 0)).Size() < 1.f, "toward the ruin, not the nearest centre");

	// On its way, the idle soldiers are ordered again toward the ruin.
	const FHOLAIWaveStep Later = HOLAIArmy::UpdateWave(Wave, Summary, FrmData(), false, false, FrmPoint(), 106.f);
	T.AssertEquals(10, Later.AttackIds.Num(), "the idle soldiers are ordered again");
	T.AssertTrue((Later.AttackTarget - FVector(3000, 9000, 0)).Size() < 1.f, "still toward the ruin");

	// The ruin risen: the centre again.
	Summary.EnemyForums[1].bDown = false;
	const FHOLAIWaveStep Risen = HOLAIArmy::UpdateWave(Wave, Summary, FrmData(), false, false, FrmPoint(), 112.f);
	T.AssertTrue((Risen.AttackTarget - FVector(-8000, 0, 0)).Size() < 1.f, "the forum stands again: the enemy centre");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The King (IA3.4, Regicide).

FHOLAISummary FrmKingSummary(FVector Where, bool bIdle)
{
	FHOLAISummary Summary = FrmBase();
	Summary.bRegicide = true;
	Summary.bHasKing = true;
	Summary.King.Id = 20;
	Summary.King.OwnerSlot = 0;
	Summary.King.Type = n"King";
	Summary.King.Health = 250;
	Summary.King.Location = Where;
	Summary.King.bIdle = bIdle;
	return Summary;
}

void Test_KingOnlyInRegicide(FUnitTest& T)
{
	FHOLAIKingMemory Memory;
	FHOLAISummary Summary = FrmKingSummary(FVector(-450, 0, 0), true);
	Summary.bRegicide = false;
	FVector Where;
	T.AssertFalse(HOLAIForum::KingOrder(Memory, Summary, FHOLAIKingData(), 100.f, Where), "no Regicide: the order is not given");
	Summary = FrmKingSummary(FVector(-450, 0, 0), true);
	Summary.bHasKing = false;
	T.AssertFalse(HOLAIForum::KingOrder(Memory, Summary, FHOLAIKingData(), 100.f, Where), "no King: nothing");
}

void Test_KingGoesBehindTheForum(FUnitTest& T)
{
	FHOLAIKingMemory Memory;
	const FHOLAIKingData Data;
	const FHOLAISummary Summary = FrmKingSummary(FVector(-450, 0, 0), true);
	T.AssertTrue((HOLAIForum::KingPost(Summary, Data) - FVector(2500, 0, 0)).Size() < 1.f, "the post: behind the forum, away from the enemy centre (-8000, 0)");
	FVector Where;
	T.AssertTrue(HOLAIForum::KingOrder(Memory, Summary, Data, 100.f, Where), "the King in front of the forum goes to his post");
	T.AssertTrue((Where - FVector(2500, 0, 0)).Size() < 1.f, "to the post");
	T.AssertFalse(HOLAIForum::KingOrder(Memory, Summary, Data, 101.f, Where), "not again within OrderEvery");
}

void Test_KingAtHisPostStays(FUnitTest& T)
{
	FHOLAIKingMemory Memory;
	FVector Where;
	T.AssertFalse(HOLAIForum::KingOrder(Memory, FrmKingSummary(FVector(2300, 100, 0), true), FHOLAIKingData(), 100.f, Where), "at his post: he stays");
	T.AssertFalse(HOLAIForum::KingOrder(Memory, FrmKingSummary(FVector(-450, 0, 0), false), FHOLAIKingData(), 200.f, Where), "on the move: not ordered over his walk");
}

void Test_KingFallsBackToHisPostFromTheEnemySoldier(FUnitTest& T)
{
	FHOLAIKingMemory Memory;
	FHOLAISummary Summary = FrmKingSummary(FVector(-450, 0, 0), false);
	FrmAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(-1500, 0, 0), 1, false, -1);
	FVector Where;
	T.AssertTrue(HOLAIForum::KingOrder(Memory, Summary, FHOLAIKingData(), 100.f, Where), "an enemy soldier 1050 cm away, the post on the safe side: he falls back");
	T.AssertTrue((Where - FVector(2500, 0, 0)).Size() < 1.f, "to his post, not away into the open");

	FHOLAIKingMemory Other;
	FHOLAISummary Peasants = FrmKingSummary(FVector(-450, 0, 0), false);
	FrmAdd(Peasants.Enemies, 50, n"Peasant", 3, FVector(-1500, 0, 0), 1, false, -1);
	T.AssertFalse(HOLAIForum::KingOrder(Other, Peasants, FHOLAIKingData(), 100.f, Where), "enemy Peasants frighten nobody (he is on the move)");
}

void Test_KingStaysWhenCorneredOrAtHisPost(FUnitTest& T)
{
	FVector Where;
	FHOLAIKingMemory AtPost;
	FHOLAISummary Summary = FrmKingSummary(FVector(2500, 0, 0), true);
	FrmAdd(Summary.Enemies, 50, n"Spearman", 1, FVector(1000, 0, 0), 1, false, -1);
	T.AssertFalse(HOLAIForum::KingOrder(AtPost, Summary, FHOLAIKingData(), 100.f, Where), "at his post with a soldier near: he stays (no running into the open)");

	FHOLAIKingMemory Between;
	FHOLAISummary Cut = FrmKingSummary(FVector(-450, 0, 0), false);
	FrmAdd(Cut.Enemies, 50, n"Spearman", 1, FVector(800, 0, 0), 1, false, -1);
	T.AssertFalse(HOLAIForum::KingOrder(Between, Cut, FHOLAIKingData(), 100.f, Where), "the soldier is between him and the post: he does not run into him");
}

void Test_ArmyDefendsTheKingInRegicide(FUnitTest& T)
{
	const FHOLAIArmyData Army = FrmData();
	FHOLAISummary Summary = FrmKingSummary(FVector(0, 9000, 0), true);
	FrmAdd(Summary.Army, 1, n"Spearman", 8, FrmPoint(), 0, true, -1);
	FrmAdd(Summary.Enemies, 50, n"Spearman", 2, FVector(500, 9000, 0), 1, false, -1);
	T.AssertTrue(HOLAIArmy::Threat(Summary, Army).bThreatened, "Regicide: two soldiers next to the King are a threat");
	Summary.bRegicide = false;
	T.AssertFalse(HOLAIArmy::Threat(Summary, Army).bThreatened, "no Regicide: far from every guarded place, no threat");
}

void Test_PeasantsKeepWorkingDuringTheRuin(FUnitTest& T)
{
	FHOLAIForumMemory Memory;
	const FHOLAIForumData Data = HOLAIForum::DefaultData();
	FHOLAISummary Summary = FrmSummary(1, true, 4);
	// The Peasants stand 1000, 2000, 3000 and 4000 cm from the forum: with 800 cm/s and 1.5 s of margin they are called 2.75, 4, 5.25 and 6.5 s (walk plus margin) before the end.
	Summary.OwnForum.RuinSecondsLeft = 60.f;
	T.AssertEquals(0, HOLAIForum::RebuildOrders(Memory, Summary, Data, 50.f).Num(), "a long ruin ahead: nobody leaves his work");
	Summary.OwnForum.RuinSecondsLeft = 5.f;
	const TArray<int> Early = HOLAIForum::RebuildOrders(Memory, Summary, Data, 60.f);
	T.AssertTrue(Early.Num() == 2 && Early.Contains(102) && Early.Contains(103), "5 s left: only the two whose walk takes that long go");
	Summary.OwnForum.RuinSecondsLeft = 0.f;
	Summary.OwnForum.bRuin = false;
	Summary.OwnForum.bRebuilding = true;
	T.AssertEquals(2, HOLAIForum::RebuildOrders(Memory, Summary, Data, 70.f).Num(), "the rebuilding begun: the two others go");
}

void Test_WaveAimsAtTheEnemyKingInRegicide(FUnitTest& T)
{
	FHOLAISummary Summary = FrmReady();
	FrmAdd(Summary.Enemies, 70, n"King", 1, FVector(-9500, 1500, 0), 1, false, -1);
	bool bFound = false;
	HOLAIForum::KingTarget(Summary, FVector(0, 0, 0), bFound);
	T.AssertTrue(bFound, "the enemy King is known");

	FHOLAIWave Wave;
	FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, FrmData(), false, false, FrmPoint(), 500.f);
	T.AssertTrue(Step.bLaunched && (Step.AttackTarget - FVector(-8000, 0, 0)).Size() < 1.f, "no Regicide: the enemy centre");

	Summary.bRegicide = true;
	FHOLAIWave Other;
	Step = HOLAIArmy::UpdateWave(Other, Summary, FrmData(), false, false, FrmPoint(), 500.f);
	T.AssertTrue(Step.bLaunched && (Step.AttackTarget - FVector(-9500, 1500, 0)).Size() < 1.f, "Regicide: the enemy King");

	// A fallen forum comes first.
	Summary.EnemyForums.Add(FrmEnemyForum(2, 1, true, FVector(3000, 9000, 0)));
	FHOLAIWave Third;
	Step = HOLAIArmy::UpdateWave(Third, Summary, FrmData(), false, false, FrmPoint(), 500.f);
	T.AssertTrue(Step.bLaunched && (Step.AttackTarget - FVector(3000, 9000, 0)).Size() < 1.f, "a ruin to hold before the King");
}

// ---------------------------------------------------------------------------------------------------------------------------
// Helping a teammate (IA4, D571): the army goes and holds a teammate's fallen forum when it dominates on the spot (ratio at least 1).

void Test_TeammateRuinIsHelpedWhenTheArmyHoldsItsOwnThere(FUnitTest& T)
{
	FHOLAISummary Summary = FrmReady();
	bool bFound = true;
	HOLAIArmy::AllyHelpTarget(Summary, FrmData(), FVector(0, 0, 0), bFound);
	T.AssertFalse(bFound, "no teammate: nowhere to help");
	Summary.AllyForums.Add(FrmEnemyForum(2, 2, false, FVector(3000, 9000, 0)));
	HOLAIArmy::AllyHelpTarget(Summary, FrmData(), FVector(0, 0, 0), bFound);
	T.AssertFalse(bFound, "the teammate's forum stands: nothing to help");
	Summary.AllyForums[0].bDown = true;
	Summary.AllyForums[0].bRuin = true;
	const FVector Target = HOLAIArmy::AllyHelpTarget(Summary, FrmData(), FVector(0, 0, 0), bFound);
	T.AssertTrue(bFound && (Target - FVector(3000, 9000, 0)).Size() < 1.f, "the teammate's ruin, nobody there: the army can hold it");

	FHOLAIWave Wave;
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, Summary, FrmData(), false, 100.f), "a teammate's ruin to hold: the minimum time does not count");
	Summary.AllyForums[0].bDown = false;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, FrmData(), false, 100.f), "the same, the forum standing: too early as before");
}

void Test_TeammateRuinWithAStrongEnemyOnTheSpotIsLeftAlone(FUnitTest& T)
{
	FHOLAISummary Summary = FrmReady();
	Summary.AllyForums.Add(FrmEnemyForum(2, 1, true, FVector(3000, 9000, 0)));
	// Nine Men-at-arms stand at the ruin against our ten Spearmen: under 1 on the spot.
	FrmAdd(Summary.Enemies, 70, n"ManAtArms", 9, FVector(3000, 8500, 0), 1, false, -1);
	bool bFound = true;
	HOLAIArmy::AllyHelpTarget(Summary, FrmData(), FVector(0, 0, 0), bFound);
	T.AssertTrue(HOLAIArmy::RatioAt(Summary, FrmData(), FVector(3000, 9000, 0)) < 1.f, "the ratio on the spot is under 1");
	T.AssertFalse(bFound, "a crushing enemy at the teammate's ruin: the army does not go");
	FHOLAIWave Wave;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Summary, FrmData(), false, 100.f), "and no launch for it (the minimum time still counts)");

	// The same enemy far from the ruin does not count there.
	Summary.Enemies.Empty();
	FrmAdd(Summary.Enemies, 70, n"ManAtArms", 9, FVector(-8000, 0, 0), 1, false, -1);
	HOLAIArmy::AllyHelpTarget(Summary, FrmData(), FVector(0, 0, 0), bFound);
	T.AssertTrue(bFound, "the enemy army is elsewhere: only the soldiers on the spot count");
}

void Test_TeammateRuinStillNeedsTheSoldiersAndNoDefence(FUnitTest& T)
{
	FHOLAISummary Few = FrmBase();
	FrmAdd(Few.Army, 1, n"Spearman", 5, FrmPoint(), 0, true, -1);
	Few.AllyForums.Add(FrmEnemyForum(2, 1, true, FVector(3000, 9000, 0)));
	FHOLAIWave Wave;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Few, FrmData(), false, 100.f), "too few soldiers");
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, FrmReady(), FrmData(), true, 100.f), "not while defending");

	// Without an enemy centre known, a teammate's ruin is still a place to go.
	FHOLAISummary NoCentre = FrmReady();
	NoCentre.bHasEnemyCentre = false;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, NoCentre, FrmData(), false, 100.f), "no enemy centre and no ruin: nowhere to go");
	NoCentre.AllyForums.Add(FrmEnemyForum(2, 1, true, FVector(3000, 9000, 0)));
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, NoCentre, FrmData(), false, 100.f), "a teammate's ruin to help hold");
}

void Test_WaveGoesToTheTeammateRuinBeforeTheEnemyRuin(FUnitTest& T)
{
	FHOLAISummary Summary = FrmReady();
	Summary.EnemyForums.Add(FrmEnemyForum(3, 1, true, FVector(-9000, 3000, 0)));
	Summary.AllyForums.Add(FrmEnemyForum(2, 1, true, FVector(3000, 9000, 0)));
	FHOLAIWave Wave;
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Summary, FrmData(), false, false, FrmPoint(), 100.f);
	T.AssertTrue(Step.bLaunched, "launched at 100 s");
	T.AssertTrue((Step.AttackTarget - FVector(3000, 9000, 0)).Size() < 1.f, "to the teammate's ruin, before the enemy's");

	// With a strong enemy on the teammate's ruin the wave takes the enemy's ruin as before.
	FrmAdd(Summary.Enemies, 70, n"ManAtArms", 9, FVector(3000, 8500, 0), 1, false, -1);
	bool bFound = false;
	const FVector Where = HOLAIArmy::WaveTarget(Summary, FrmData(), FrmPoint(), bFound);
	T.AssertTrue(bFound && (Where - FVector(-9000, 3000, 0)).Size() < 1.f, "the teammate's ruin is out of reach: the enemy's ruin");
}
