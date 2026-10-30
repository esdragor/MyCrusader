// The weighing of the lives when the forum is attacked (IA4, D579): the Easy level always gives the army ("sacrifices the army, even lost
// beforehand"), and every situation is written down (lives, power ratio, decision). Played on the REAL type tables (HOLAIArmy::LoadFigures):
// the ratios come from the real combat figures, no number of the combat is typed in. Normal and Hard are not written yet: they keep the
// ratio rules (decision "Rules").

FHOLAIUnit LivesUnit(int Id, FName Type, int Health, FVector Where, int Slot)
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

FHOLAIArmyData LivesData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	HOLAIArmy::LoadFigures(Data);
	return Data;
}

int LivesHealthOf(const FHOLAIArmyData& Data, FName Type)
{
	return Data.Figures[Data.Types.FindIndex(Type)].Health;
}

// A slot with its forum (and centre) at the origin, 2 lives, and Spearmen at home; Enemies Men-at-arms stand EnemyAt cm from the centre.
FHOLAISummary LivesSummary(const FHOLAIArmyData& Data, int Lives, bool bDown, int Spearmen, int Enemies, float EnemyAt)
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
		Summary.Army.Add(LivesUnit(1 + Index, n"Spearman", LivesHealthOf(Data, n"Spearman"), FVector(-1500, 0, 0), 0));
	}
	for (int Index = 0; Index < Enemies; Index++)
	{
		Summary.Enemies.Add(LivesUnit(50 + Index, n"ManAtArms", LivesHealthOf(Data, n"ManAtArms"), FVector(-EnemyAt, 100.f * float(Index), 0), 1));
	}
	return Summary;
}

// Easy gives the army at a ratio the ratio rules refuse (the real figures: 2 Spearmen against 10 Men-at-arms hitting the forum); Normal and
// Hard keep the rules for now.
void Test_LivesEasyAlwaysGivesTheArmy(FUnitTest& T)
{
	const FHOLAIArmyData Data = LivesData();
	const FHOLAISummary Summary = LivesSummary(Data, 2, false, 2, 10, 300.f);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, Data);
	T.AssertTrue(Threat.bThreatened && Threat.bAttackingBuilding, "the Men-at-arms are at the forum");
	T.AssertTrue(Threat.Ratio < Data.DefenceMinRatioAttacked, "lost beforehand: the ratio rules would not send the army");
	T.AssertFalse(HOLAIArmy::ShouldDefend(Summary, Data, Threat, FVector(-1500, 0, 0)), "the ratio rules keep the army home");

	const FHOLAIForumSituation Easy = HOLAIForum::Judge(Summary, Data, Threat, EHOLAIDifficulty::Easy);
	T.AssertTrue(Easy.bAttacked, "the forum is attacked");
	T.AssertEquals(2, Easy.Lives, "two lives left");
	T.AssertTrue(Easy.Call == EHOLAILifeCall::Fight, "Easy: the army fights whatever the ratio");
	T.AssertTrue(HOLAIForum::DefendAnyway(Easy, Threat), "Easy sends the army");
	T.AssertEquals(Threat.Ratio, Easy.Ratio, "the situation carries the real power ratio");

	T.AssertTrue(HOLAIForum::Judge(Summary, Data, Threat, EHOLAIDifficulty::Normal).Call == EHOLAILifeCall::Rules, "Normal: the ratio rules, for now");
	T.AssertTrue(HOLAIForum::Judge(Summary, Data, Threat, EHOLAIDifficulty::Hard).Call == EHOLAILifeCall::Rules, "Hard: the ratio rules, for now");
	T.AssertFalse(HOLAIForum::DefendAnyway(HOLAIForum::Judge(Summary, Data, Threat, EHOLAIDifficulty::Normal), Threat), "Normal adds nothing");
}

// Not attacked: enemies near the base but far from the forum, or nobody: no situation, Easy changes nothing.
void Test_LivesNoAttackNoSituation(FUnitTest& T)
{
	const FHOLAIArmyData Data = LivesData();
	const FHOLAISummary Quiet = LivesSummary(Data, 2, false, 2, 0, 300.f);
	const FHOLAIThreat None = HOLAIArmy::Threat(Quiet, Data);
	const FHOLAIForumSituation Calm = HOLAIForum::Judge(Quiet, Data, None, EHOLAIDifficulty::Easy);
	T.AssertFalse(Calm.bAttacked, "nobody: not attacked");
	T.AssertFalse(HOLAIForum::DefendAnyway(Calm, None), "nothing to send the army to");

	const FHOLAISummary Far = LivesSummary(Data, 2, false, 2, 3, Data.AttackedRadius + 1500.f);
	const FHOLAIThreat Near = HOLAIArmy::Threat(Far, Data);
	T.AssertTrue(Near.bThreatened, "enemies near the base");
	const FHOLAIForumSituation Away = HOLAIForum::Judge(Far, Data, Near, EHOLAIDifficulty::Easy);
	T.AssertFalse(Away.bAttacked, "but not at the forum: the forum is not attacked");
	T.AssertFalse(HOLAIForum::DefendAnyway(Away, Near), "Easy keeps to the ratio rules while the forum is not hit");
}

// A fallen forum is an attacked forum too (the last life: the rules already send everything; the situation says so).
void Test_LivesFallenForumIsASituation(FUnitTest& T)
{
	const FHOLAIArmyData Data = LivesData();
	const FHOLAISummary Summary = LivesSummary(Data, 1, true, 3, 6, 500.f);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, Data);
	const FHOLAIForumSituation Situation = HOLAIForum::Judge(Summary, Data, Threat, EHOLAIDifficulty::Easy);
	T.AssertTrue(Situation.bAttacked, "a fallen forum with enemies at it");
	T.AssertEquals(1, Situation.Lives, "last life");
	T.AssertTrue(Situation.Call == EHOLAILifeCall::Fight, "Easy fights");
}

// The journal line: every situation (lives, ratio, decision, level), written again only when it changes.
void Test_LivesTheJournalLine(FUnitTest& T)
{
	const FHOLAIArmyData Data = LivesData();
	const FHOLAISummary Summary = LivesSummary(Data, 2, false, 2, 10, 300.f);
	const FHOLAIThreat Threat = HOLAIArmy::Threat(Summary, Data);
	const FHOLAIForumSituation Easy = HOLAIForum::Judge(Summary, Data, Threat, EHOLAIDifficulty::Easy);
	const FString Line = HOLAIForum::Describe(Easy, EHOLAIDifficulty::Easy);
	T.AssertTrue(Line.Contains("forum attacked"), "says what it is: " + Line);
	T.AssertTrue(Line.Contains("lives 2"), "the lives: " + Line);
	T.AssertTrue(Line.Contains("ratio"), "the ratio: " + Line);
	T.AssertTrue(Line.Contains("Easy"), "the level: " + Line);
	T.AssertTrue(Line.Contains("fight"), "the decision: " + Line);

	T.AssertTrue(HOLAIForum::Describe(HOLAIForum::Judge(Summary, Data, Threat, EHOLAIDifficulty::Normal), EHOLAIDifficulty::Normal).Contains("rules"), "Normal: the decision is the rules");

	// Written when it begins, not again while it is the same, again when the lives or the decision change, once more when it ends.
	FHOLAIForumLogged Logged;
	T.AssertTrue(HOLAIForum::ShouldLog(Logged, Easy), "a new situation");
	T.AssertFalse(HOLAIForum::ShouldLog(Logged, Easy), "the same: not again");
	FHOLAIForumSituation Fewer = Easy;
	Fewer.Lives = 1;
	T.AssertTrue(HOLAIForum::ShouldLog(Logged, Fewer), "a life less: written");
	FHOLAIForumSituation Over;
	T.AssertTrue(HOLAIForum::ShouldLog(Logged, Over), "the attack ends: written");
	T.AssertFalse(HOLAIForum::ShouldLog(Logged, Over), "quiet: nothing");
}
