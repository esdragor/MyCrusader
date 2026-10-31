// IA4, D579 and D329: the weighing of the lives by level, played on the REAL type tables (LvData loads them): the ratios, the duels
// and the losses come from the real combat figures; no number of the combat is typed in. Normal feints, Hard evaluates its loss, lets the
// life go, snipes and punishes, Evolving passes from Easy to Hard with the time. The numbers of the behaviours are settings (FHOLAILivesData).

FHOLAIUnit LvUnit(int Id, FName Type, int Health, FVector Where, int Slot)
{
	FHOLAIUnit Unit;
	Unit.Id = Id;
	Unit.OwnerSlot = Slot;
	Unit.Type = Type;
	Unit.Health = Health;
	Unit.bIdle = true;
	Unit.TargetId = -1;
	Unit.Location = Where;
	return Unit;
}

FHOLAIArmyData LvData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	HOLAIArmy::LoadFigures(Data);
	return Data;
}

int LvHealthOf(const FHOLAIArmyData& Data, FName Type)
{
	return Data.Figures[Data.Types.FindIndex(Type)].Health;
}

// A slot with its forum (and centre) at the origin, 2 lives, and Spearmen at home; Enemies Men-at-arms stand EnemyAt cm from the centre.
FHOLAISummary LvSummary(const FHOLAIArmyData& Data, int Lives, bool bDown, int Spearmen, int Enemies, float EnemyAt)
{
	FHOLAISummary Summary;
	Summary.bHasCentre = true;
	Summary.CentreLocation = FVector(0, 0, 0);
	Summary.bHasEnemyCentre = true;
	Summary.EnemyCentreLocation = FVector(-8000, 0, 0);
	Summary.bHasOwnForum = true;
	Summary.OwnForum.Slot = 0;
	Summary.OwnForum.Lives = Lives;
	Summary.OwnForum.bDown = bDown;
	Summary.OwnForum.bRuin = bDown;
	Summary.OwnForum.bHasLocation = true;
	Summary.OwnForum.Location = FVector(0, 0, 0);
	for (int Index = 0; Index < Spearmen; Index++)
	{
		Summary.Army.Add(LvUnit(1 + Index, n"Spearman", LvHealthOf(Data, n"Spearman"), FVector(-1500, 0, 0), 0));
	}
	for (int Index = 0; Index < Enemies; Index++)
	{
		Summary.Enemies.Add(LvUnit(50 + Index, n"ManAtArms", LvHealthOf(Data, n"ManAtArms"), FVector(-EnemyAt, 100.f * float(Index), 0), 1));
	}
	return Summary;
}

// Two Spearmen at home against ten Men-at-arms hitting the forum: lost beforehand (ratio far under every threshold).
FHOLAISummary LostSummary(const FHOLAIArmyData& Figures, int NumLives)
{
	return LvSummary(Figures, NumLives, false, 2, 10, 300.f);
}

// Normal that knows it loses, with a life to spare, feints; with the stronger army, or on the last life, the rules decide.
void Test_LivesNormalFeintsWhenItLoses(FUnitTest& T)
{
	const FHOLAIArmyData Data = LvData();
	const FHOLAISummary Lost = LostSummary(Data, 2);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Lost, Data);
	T.AssertTrue(Threat.Ratio < Data.Lives.NormalLoseRatio, "the real ratio is under the ratio at which Normal knows it loses");
	const FHOLAIForumSituation Normal = HOLAIForum::Judge(Lost, Data, Threat, EHOLAIDifficulty::Normal);
	T.AssertTrue(Normal.bAttacked && Normal.Call == EHOLAILifeCall::Feint, "Normal feints");
	T.AssertTrue(HOLAILives::HoldsBack(Normal), "the army is held back from the attackers");
	T.AssertFalse(HOLAIForum::DefendAnyway(Normal, Threat), "it does not give the army whatever the ratio (that is Easy)");

	// The stronger army: the rules decide (the army meets them).
	const FHOLAISummary Strong = LvSummary(Data, 2, false, 14, 2, 300.f);
	const FHOLAIThreat StrongThreat = HOLAIArmy::Threat(Strong, Data);
	T.AssertTrue(StrongThreat.Ratio >= Data.Lives.NormalLoseRatio, "the army is ahead");
	T.AssertTrue(HOLAIForum::Judge(Strong, Data, StrongThreat, EHOLAIDifficulty::Normal).Call == EHOLAILifeCall::Rules, "ahead: the rules");

	// The last life is the common rule: no feint, everything on the ruin.
	const FHOLAISummary Last = LostSummary(Data, 1);
	T.AssertTrue(HOLAIForum::Judge(Last, Data, HOLAIArmy::Threat(Last, Data), EHOLAIDifficulty::Normal).Call == EHOLAILifeCall::Rules, "last life: the rules");
}

// The feint: back to the refuge (away from the attackers), then round to their flank, then the strike (the rules meet them).
void Test_LivesNormalFeintGoesBackThenRound(FUnitTest& T)
{
	const FHOLAIArmyData Data = LvData();
	const FHOLAISummary Lost = LostSummary(Data, 2);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Lost, Data);
	const FHOLAIForumSituation Situation = HOLAIForum::Judge(Lost, Data, Threat, EHOLAIDifficulty::Normal);
	FHOLAIFeintMemory Memory;

	FHOLAIFeintStep Back = HOLAILives::FeintStep(Memory, Lost, Data, Threat, Situation, 100.f);
	T.AssertTrue(Back.bPhaseChanged && Memory.Phase == EHOLAIFeintPhase::Back, "the feint begins by falling back");
	T.AssertEquals(Lost.Army.Num(), Back.MoveIds.Num(), "every soldier is ordered");
	const FVector Refuge = HOLAILives::RefugePoint(Lost, Data, Threat);
	T.AssertEquals(Refuge.X, Back.Where.X, "to the refuge (x)");
	T.AssertEquals(Refuge.Y, Back.Where.Y, "to the refuge (y)");
	T.AssertEquals(Data.Lives.RefugeDistance, HOLAIArmy::FlatDistance(Refuge, Lost.CentreLocation), "the refuge is RefugeDistance from the centre");
	T.AssertTrue(HOLAIArmy::FlatDistance(Refuge, Threat.Location) > HOLAIArmy::FlatDistance(Lost.CentreLocation, Threat.Location), "and farther from the attackers than the centre");

	// Not again before FeintEvery, and the soldiers are idle far from the point: ordered again after it.
	FHOLAIFeintStep Same = HOLAILives::FeintStep(Memory, Lost, Data, Threat, Situation, 101.f);
	T.AssertEquals(0, Same.MoveIds.Num(), "not ordered again at once");
	FHOLAIFeintStep Again = HOLAILives::FeintStep(Memory, Lost, Data, Threat, Situation, 100.f + Data.Lives.FeintEvery + 0.1f);
	T.AssertTrue(Again.MoveIds.Num() > 0 && !Again.bPhaseChanged, "ordered again after FeintEvery");

	FHOLAIFeintStep Round = HOLAILives::FeintStep(Memory, Lost, Data, Threat, Situation, 100.f + Data.Lives.FeintBackSeconds);
	T.AssertTrue(Round.bPhaseChanged && Memory.Phase == EHOLAIFeintPhase::Around, "then goes round");
	const FVector Flank = HOLAILives::FlankPoint(Lost, Data, Threat);
	T.AssertEquals(Flank.X, Round.Where.X, "to the flank (x)");
	T.AssertEquals(Data.Lives.FlankDistance, HOLAIArmy::FlatDistance(Flank, Threat.Location), "FlankDistance from the attackers");
	const FVector Line = Threat.Location - Lost.CentreLocation;
	const FVector Side = Flank - Threat.Location;
	T.AssertTrue(Math::Abs(Line.X * Side.X + Line.Y * Side.Y) < 1.f, "at a right angle to the line from the centre to them");

	FHOLAIFeintStep Strike = HOLAILives::FeintStep(Memory, Lost, Data, Threat, Situation, 100.f + Data.Lives.FeintBackSeconds + Data.Lives.FeintAroundSeconds);
	T.AssertTrue(Strike.bStrike && Strike.MoveIds.Num() == 0 && Memory.Phase == EHOLAIFeintPhase::Strike, "then the strike: the rules meet them");

	// The decision stops being a feint (the attackers left): the memory is cleared.
	FHOLAIForumSituation Over;
	HOLAILives::FeintStep(Memory, Lost, Data, Threat, Over, 200.f);
	T.AssertTrue(Memory.Phase == EHOLAIFeintPhase::None, "no feint: the memory is cleared");
}

// Normal that loses keeps its economy: the Peasants that run go to the refuge (Withdraw) or to the attacked centre (StayAndFlee, the old way).
void Test_LivesNormalKeepsTheEconomy(FUnitTest& T)
{
	FHOLAIArmyData Data = LvData();
	const FHOLAISummary Lost = LostSummary(Data, 2);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Lost, Data);
	const FHOLAIForumSituation Normal = HOLAIForum::Judge(Lost, Data, Threat, EHOLAIDifficulty::Normal);
	const FVector Refuge = HOLAILives::RefugePoint(Lost, Data, Threat);

	T.AssertEquals(Refuge.X, HOLAILives::FleeTarget(Lost, Data, Normal, Threat).X, "Withdraw: the Peasants run to the refuge");
	T.AssertTrue(HOLAILives::FleeTarget(Lost, Data, Normal, Threat).Y == Refuge.Y, "Withdraw: (y)");
	Data.Lives.LossPlan = EHOLAILossPlan::StayAndFlee;
	T.AssertEquals(Lost.CentreLocation.X, HOLAILives::FleeTarget(Lost, Data, Normal, Threat).X, "StayAndFlee: to the centre");
	Data.Lives.LossPlan = EHOLAILossPlan::Withdraw;
	const FHOLAIForumSituation Easy = HOLAIForum::Judge(Lost, Data, Threat, EHOLAIDifficulty::Easy);
	T.AssertEquals(Lost.CentreLocation.X, HOLAILives::FleeTarget(Lost, Data, Easy, Threat).X, "Easy has no plan: the centre");
	T.AssertEquals(Lost.CentreLocation.X, HOLAILives::FleeTarget(Lost, Data, FHOLAIForumSituation(), Threat).X, "no attack: the centre");
}

// The loss of a straight fight, by the power ratio (Lanchester's square law).
void Test_LivesHardLossOfAFight(FUnitTest& T)
{
	T.AssertEquals(1.f, HOLAILives::ExpectedLoss(0.4f), "weaker: the army is lost");
	T.AssertEquals(1.f, HOLAILives::ExpectedLoss(1.f), "even: the army is lost");
	T.AssertTrue(Math::Abs(HOLAILives::ExpectedLoss(2.f) - 0.134f) < 0.001f, "twice as strong: 13% lost");
	T.AssertTrue(HOLAILives::ExpectedLoss(1.2f) > HOLAILives::ExpectedLoss(2.f), "the stronger, the less is lost");
	T.AssertTrue(HOLAILives::ExpectedLoss(1000.f) < 0.001f, "no enemy army: nothing lost");
}

// Hard lets the life go when the fight costs too much AND the army kept is enough to crush the attacker's home; otherwise the rules decide.
void Test_LivesHardLetsTheLifeGo(FUnitTest& T)
{
	FHOLAIArmyData Data = LvData();
	const FHOLAISummary Lost = LostSummary(Data, 2);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Lost, Data);
	const FHOLAIForumSituation Let = HOLAIForum::Judge(Lost, Data, Threat, EHOLAIDifficulty::Hard);
	T.AssertTrue(Let.Call == EHOLAILifeCall::Let, "lost beforehand, the attacker's home is empty: Hard lets the life go");
	T.AssertTrue(Let.Loss > Data.Lives.HardMaxLoss, "the evaluated loss is over the limit");
	T.AssertTrue(Let.HomeRatio >= Data.Lives.HardCrushRatio, "the army kept is enough to crush the home");
	T.AssertTrue(Describe_Contains(Let, EHOLAIDifficulty::Hard, "let the life go"), "the journal line says so");

	// A strong guard at the attacker's home (far from our base): the army kept is not enough, so the rules decide.
	FHOLAISummary Guarded = LostSummary(Data, 2);
	for (int Index = 0; Index < 30; Index++)
	{
		Guarded.Enemies.Add(LvUnit(200 + Index, n"ManAtArms", LvHealthOf(Data, n"ManAtArms"), FVector(-8000, 100.f * float(Index), 0), 1));
	}
	const FHOLAIForumSituation Held = HOLAIForum::Judge(Guarded, Data, HOLAIArmy::Threat(Guarded, Data), EHOLAIDifficulty::Hard);
	T.AssertTrue(Held.HomeRatio < Data.Lives.HardCrushRatio, "the home is guarded");
	T.AssertTrue(Held.Call == EHOLAILifeCall::Rules, "the army kept would not be enough: the rules");

	// A fight worth taking: the rules decide.
	const FHOLAISummary Strong = LvSummary(Data, 2, false, 40, 1, 300.f);
	const FHOLAIForumSituation Fight = HOLAIForum::Judge(Strong, Data, HOLAIArmy::Threat(Strong, Data), EHOLAIDifficulty::Hard);
	T.AssertTrue(Fight.Loss <= Data.Lives.HardMaxLoss, "the fight costs little");
	T.AssertTrue(Fight.Call == EHOLAILifeCall::Rules, "Hard fights a fight it wins cheaply");

	// The last life: the common rule.
	const FHOLAISummary Last = LostSummary(Data, 1);
	T.AssertTrue(HOLAIForum::Judge(Last, Data, HOLAIArmy::Threat(Last, Data), EHOLAIDifficulty::Hard).Call == EHOLAILifeCall::Rules, "last life: everything on the ruin");

	// The setting (IA4-Q2): without it, Hard never lets a life go.
	Data.Lives.bHardLetsLifeGo = false;
	T.AssertTrue(HOLAIForum::Judge(Lost, Data, Threat, EHOLAIDifficulty::Hard).Call == EHOLAILifeCall::Rules, "setting off: the rules");
}

bool Describe_Contains(const FHOLAIForumSituation& Situation, EHOLAIDifficulty Level, FString Part)
{
	return HOLAIForum::Describe(Situation, Level).Contains(Part);
}

// Hard snipes the weak while the life is let go: a wounded soldier alone, not a sound one, not one with a sound escort.
void Test_LivesHardSnipesTheWeak(FUnitTest& T)
{
	const FHOLAIArmyData Data = LvData();
	FHOLAISummary Summary = LvSummary(Data, 2, false, 3, 0, 300.f);
	// A Man-at-arms down to 1 health 2000 cm from the soldiers, and a sound one far away.
	Summary.Enemies.Add(LvUnit(80, n"ManAtArms", 1, FVector(-3000, 0, 0), 1));
	Summary.Enemies.Add(LvUnit(81, n"ManAtArms", LvHealthOf(Data, n"ManAtArms"), FVector(-3000, 6000, 0), 1));
	const FHOLAIUnit Soldier = Summary.Army[0];
	T.AssertTrue(HOLAILives::DuelOf(Data, Soldier, Summary.Enemies[0]) >= Data.Lives.SnipeMargin, "the wounded one is a duel won with the margin");
	T.AssertTrue(HOLAILives::DuelOf(Data, Soldier, Summary.Enemies[1]) < Data.Lives.SnipeMargin, "the sound one is not");
	T.AssertEquals(80, HOLAILives::SnipeTargetFor(Summary, Data, Soldier), "the wounded one is the target");

	// A sound escort beside him: no snipe.
	FHOLAISummary Escorted = Summary;
	Escorted.Enemies.Add(LvUnit(82, n"ManAtArms", LvHealthOf(Data, n"ManAtArms"), FVector(-3000, 200, 0), 1));
	T.AssertEquals(-1, HOLAILives::SnipeTargetFor(Escorted, Data, Soldier), "a sound soldier beside him: not isolated");

	// Out of range: no snipe.
	FHOLAISummary Far = Summary;
	Far.Enemies[0].Location = FVector(-1500.f - Data.Lives.SnipeRange - 500.f, 0, 0);
	T.AssertEquals(-1, HOLAILives::SnipeTargetFor(Far, Data, Soldier), "out of range");

	// Only while the life is let go (decision Let), throttled by SnipeEvery.
	FHOLAISnipeMemory Memory;
	FHOLAIForumSituation Let;
	Let.bAttacked = true;
	Let.Lives = 2;
	Let.Call = EHOLAILifeCall::Let;
	FHOLAIForumSituation Rules = Let;
	Rules.Call = EHOLAILifeCall::Rules;
	T.AssertEquals(0, HOLAILives::SnipeOrders(Memory, Summary, Data, Rules, 100.f).Num(), "no snipe while the rules decide");
	const TArray<FHOLAISnipe> Orders = HOLAILives::SnipeOrders(Memory, Summary, Data, Let, 100.f);
	T.AssertEquals(3, Orders.Num(), "the three idle soldiers go");
	T.AssertEquals(80, Orders[0].TargetId, "after the wounded one");
	T.AssertEquals(0, HOLAILives::SnipeOrders(Memory, Summary, Data, Let, 101.f).Num(), "not ordered again at once");
	T.AssertEquals(3, HOLAILives::SnipeOrders(Memory, Summary, Data, Let, 100.f + Data.Lives.SnipeEvery + 0.1f).Num(), "ordered again after SnipeEvery");
}

// Hard punishes: with the life let go the wave leaves at once, whatever the time and the number of soldiers, toward the attacker's centre.
void Test_LivesHardPunishesTheHome(FUnitTest& T)
{
	const FHOLAIArmyData Data = LvData();
	const FHOLAISummary Lost = LostSummary(Data, 2);
	const FHOLAIForumSituation Let = HOLAIForum::Judge(Lost, Data, HOLAIArmy::Threat(Lost, Data), EHOLAIDifficulty::Hard);
	T.AssertTrue(Let.Call == EHOLAILifeCall::Let, "the life is let go");

	FHOLAIWave Wave;
	T.AssertFalse(HOLAIArmy::CanLaunch(Wave, Lost, Data, false, 30.f), "a normal wave does not leave: too early, too few, too weak");
	Wave.bForce = true;
	T.AssertTrue(HOLAIArmy::CanLaunch(Wave, Lost, Data, false, 30.f), "the forced one does");
	const FHOLAIWaveStep Step = HOLAIArmy::UpdateWave(Wave, Lost, Data, false, false, FVector(-1500, 0, 0), 30.f);
	T.AssertTrue(Step.bLaunched, "the wave is launched");
	T.AssertEquals(Lost.EnemyCentreLocation.X, Step.AttackTarget.X, "toward the attacker's centre");
	T.AssertEquals(Lost.Army.Num(), Step.AttackIds.Num(), "with every soldier");
	T.AssertTrue(Wave.bPunishing && !Wave.bForce, "it is the punishing wave, the request is consumed");
}

// Found in a real game (probe on ai_hard, 2026-10-31): the punishing wave left the base at once, saw the attackers beside it (a low ratio),
// fell back, and was launched again five seconds later, over and over. It must not fall back on the ratio while it is still at our base
// (the enemies it sees are the ones it leaves behind), and a forced launch obeys the cooldown (twice as long after a retreat).
void Test_LivesHardPunishingWaveLeaves(FUnitTest& T)
{
	const FHOLAIArmyData Data = LvData();
	const FHOLAISummary Lost = LostSummary(Data, 2);
	FHOLAIWave Punishing;
	Punishing.bForce = true;
	T.AssertTrue(HOLAIArmy::UpdateWave(Punishing, Lost, Data, false, false, FVector(-1500, 0, 0), 30.f).bLaunched, "the punishing wave leaves");
	FHOLAIWave Plain = Punishing;
	Plain.bPunishing = false;

	const FHOLAIWaveStep Stays = HOLAIArmy::UpdateWave(Punishing, Lost, Data, false, false, FVector(-1500, 0, 0), 31.f);
	T.AssertTrue(HOLAIArmy::WaveRatio(Punishing, Lost, Data) < Data.AttackRetreatRatio, "the attackers beside it: the ratio is under the retreat ratio");
	T.AssertFalse(Stays.bRetreat, "the punishing wave does not fall back while it is at our base");
	const FHOLAIWaveStep Control = HOLAIArmy::UpdateWave(Plain, Lost, Data, false, false, FVector(-1500, 0, 0), 31.f);
	T.AssertTrue(Control.bRetreat, "a plain wave in the same place does fall back");

	// Far from the base the usual rules apply again.
	FHOLAISummary Away = Lost;
	for (int Index = 0; Index < Away.Army.Num(); Index++)
	{
		Away.Army[Index].Location = FVector(-9000, 100.f * float(Index), 0);
	}
	Away.Enemies.Add(LvUnit(300, n"ManAtArms", LvHealthOf(Data, n"ManAtArms"), FVector(-9500, 0, 0), 1));
	Away.Enemies.Add(LvUnit(301, n"ManAtArms", LvHealthOf(Data, n"ManAtArms"), FVector(-9500, 100, 0), 1));
	T.AssertTrue(HOLAIArmy::UpdateWave(Punishing, Away, Data, false, false, FVector(-1500, 0, 0), 40.f).bRetreat, "away from the base, a lost fight still calls it back");

	// The forced launch waits for the cooldown.
	FHOLAIWave Cooling;
	Cooling.bForce = true;
	Cooling.NextLaunch = 100.f;
	T.AssertFalse(HOLAIArmy::CanLaunch(Cooling, Lost, Data, false, 50.f), "forced, but the cooldown is not over");
	T.AssertTrue(HOLAIArmy::CanLaunch(Cooling, Lost, Data, false, 100.f), "forced, the cooldown is over");
}

// Evolving: Easy, then Normal, then Hard with the time; the other levels do not move. Through the real chain: the same lost forum is given
// away (Easy), feinted (Normal) and let go (Hard).
void Test_LivesEvolvingPassesFromEasyToHard(FUnitTest& T)
{
	const FHOLAIArmyData Data = LvData();
	const FHOLAILivesData Lives = Data.Lives;
	T.AssertTrue(Lives.EvolveNormalAt > 0.f && Lives.EvolveNormalAt < Lives.EvolveHardAt, "the curve rises: Normal before Hard, after the start");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Evolving, 0.f, Lives) == EHOLAIDifficulty::Easy, "starts Easy");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Evolving, Lives.EvolveNormalAt - 1.f, Lives) == EHOLAIDifficulty::Easy, "Easy until the first time");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Evolving, Lives.EvolveNormalAt, Lives) == EHOLAIDifficulty::Normal, "Normal at the first time");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Evolving, Lives.EvolveHardAt - 1.f, Lives) == EHOLAIDifficulty::Normal, "Normal until the second");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Evolving, Lives.EvolveHardAt, Lives) == EHOLAIDifficulty::Hard, "Hard at the second time");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Easy, 99999.f, Lives) == EHOLAIDifficulty::Easy, "Easy stays Easy");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Hard, 0.f, Lives) == EHOLAIDifficulty::Hard, "Hard stays Hard");
	T.AssertTrue(HOLAILives::Effective(EHOLAIDifficulty::Normal, 0.f, Lives) == EHOLAIDifficulty::Normal, "Normal stays Normal");

	const FHOLAISummary Lost = LostSummary(Data, 2);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Lost, Data);
	TArray<float> Times;
	Times.Add(0.f);
	Times.Add(Lives.EvolveNormalAt + 10.f);
	Times.Add(Lives.EvolveHardAt + 10.f);
	TArray<EHOLAILifeCall> Calls;
	Calls.Add(EHOLAILifeCall::Fight);
	Calls.Add(EHOLAILifeCall::Feint);
	Calls.Add(EHOLAILifeCall::Let);
	for (int Index = 0; Index < 3; Index++)
	{
		const EHOLAIDifficulty Level = HOLAILives::Effective(EHOLAIDifficulty::Evolving, Times[Index], Lives);
		T.AssertTrue(HOLAIForum::Judge(Lost, Data, Threat, Level).Call == Calls[Index], f"Evolving at {Times[Index]} s decides as its current level");
	}
}
