// The AI's relay rules (R6; RQ3 not decided: three options behind hol.AI.RelayMode, default B). HOLAIRelay (Script/AI/HOLAIRelay.as) is pure; the
// setting, the relay type (FHOLAIRelay) and the levels are the game's own. Slot 1 plays; slot 0 and slot 2 are the others.

FHOLAISummary RelSummary(int Soldiers)
{
	FHOLAISummary Summary;
	for (int Index = 0; Index < Soldiers; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = 100 + Index;
		Unit.OwnerSlot = 1;
		Unit.Type = n"Spearman";
		Unit.Health = 10;
		Unit.Location = FVector(0, 0, 0);
		Summary.Army.Add(Unit);
	}
	return Summary;
}

FHOLAIRelayMemory RelMemory()
{
	FHOLAIRelayMemory Memory;
	Memory.Slot = 1;
	return Memory;
}

FHOLAIRelay RelAt(float X, int Owner)
{
	FHOLAIRelay Relay;
	Relay.Location = FVector(X, 0, 0);
	Relay.Radius = 1200.f;
	Relay.Owner = Owner;
	return Relay;
}

// The setting's default is the recommendation of the question (B), read from the game, and the options are the three of RQ3.
void Test_AIRelayModeDefaultIsTheRecommendation(FUnitTest& T)
{
	T.AssertEquals(HOLAIRelay::ModeDetachment, HOLRelayRules::GetAIRelayMode(), "default is B, a detachment by level");
	T.AssertEquals(0, HOLAIRelay::ModeIgnore, "A");
	T.AssertEquals(2, HOLAIRelay::ModeWholeArmy, "C");
}

// The share of the army by mode and level.
void Test_AIRelayShareByLevel(FUnitTest& T)
{
	const FHOLAIRelayData Data;
	T.AssertEquals(0, HOLAIRelay::SharePercent(HOLAIRelay::ModeIgnore, EHOLAIDifficulty::Hard, Data), "A: nothing, even Hard");
	T.AssertEquals(0, HOLAIRelay::SharePercent(HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Easy, Data), "B: Easy does not care");
	T.AssertEquals(25, HOLAIRelay::SharePercent(HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Data), "B: Normal a quarter");
	T.AssertEquals(25, HOLAIRelay::SharePercent(HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Hard, Data), "B: Hard a quarter");
	T.AssertEquals(100, HOLAIRelay::SharePercent(HOLAIRelay::ModeWholeArmy, EHOLAIDifficulty::Easy, Data), "C: all, whatever the level");
}

// Normal sends a quarter of the soldiers to the nearest NEUTRAL relay, not to a nearer one held by an enemy.
void Test_AIRelayNormalSendsAQuarterToTheNearestNeutral(FUnitTest& T)
{
	const FHOLAIRelayData Data;
	FHOLAIRelayMemory Memory = RelMemory();
	const FHOLAISummary Summary = RelSummary(8);
	TArray<FHOLAIRelay> Relays;
	Relays.Add(RelAt(2000, 0));
	Relays.Add(RelAt(5000, -1));
	Relays.Add(RelAt(3000, -1));
	const FHOLAIRelayStep Step = HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Step.bSent, "a detachment is sent");
	T.AssertEquals(2, Step.MoveIds.Num(), "a quarter of 8 soldiers");
	T.AssertEquals(2, Step.Relay, "the nearest neutral relay (index 2 at 3000), not the enemy's at 2000");
	T.AssertEquals(100, Step.MoveIds[0], "the first soldiers by identifier");
	T.AssertTrue(Math::Abs(Step.Where.X - 3000.f) < 1.f, "toward its centre");
	T.AssertEquals(6, HOLAIRelay::Without(Summary, Memory).Army.Num(), "the other rules see the army without the detachment");
}

// Hard also goes for the relays held by others, and takes first the one it lost.
void Test_AIRelayHardRetakesWhatWasTakenFromIt(FUnitTest& T)
{
	const FHOLAIRelayData Data;
	FHOLAIRelayMemory Memory = RelMemory();
	const FHOLAISummary Summary = RelSummary(8);
	TArray<FHOLAIRelay> Relays;
	Relays.Add(RelAt(2000, 1));
	Relays.Add(RelAt(4000, -1));
	Relays.Add(RelAt(9000, 1));
	// Thought 1: two relays are hers, the nearest neutral one is chosen.
	FHOLAIRelayStep First = HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Hard, Summary, Relays, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertEquals(1, First.Relay, "the neutral relay first");
	// The detachment ends when the relay is hers; then the far relay (9000) is taken by an enemy: Hard retakes it before the neutral one at 4000... which was taken too.
	Relays[1].Owner = 1;
	FHOLAIRelayStep Ended = HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Hard, Summary, Relays, false, 110.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Ended.bEnded, "the detachment ends on a relay that is hers");
	Relays[2].Owner = 0;
	Relays.Add(RelAt(3000, 2));
	FHOLAIRelayStep Retake = HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Hard, Summary, Relays, false, 120.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Retake.bSent, "Hard sends again");
	T.AssertEquals(2, Retake.Relay, "the relay taken from it (9000) before the nearer enemy relay (3000)");
	T.AssertTrue(Retake.Reason.Contains("lost"), "the journal says it is one it lost");
}

// Easy, mode A and a small army send nothing (B); the whole army goes whatever its size (C).
void Test_AIRelayNothingSentWhenTheOptionOrTheLevelSaysSo(FUnitTest& T)
{
	const FHOLAIRelayData Data;
	TArray<FHOLAIRelay> Relays;
	Relays.Add(RelAt(3000, -1));
	FHOLAIRelayMemory Easy = RelMemory();
	T.AssertFalse(HOLAIRelay::Step(Easy, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Easy, RelSummary(8), Relays, false, 100.f, FVector(0, 0, 0), Data).bSent, "B: Easy sends nothing");
	FHOLAIRelayMemory Ignoring = RelMemory();
	T.AssertFalse(HOLAIRelay::Step(Ignoring, HOLAIRelay::ModeIgnore, EHOLAIDifficulty::Hard, RelSummary(8), Relays, false, 100.f, FVector(0, 0, 0), Data).bSent, "A: nothing");
	FHOLAIRelayMemory Small = RelMemory();
	T.AssertFalse(HOLAIRelay::Step(Small, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, RelSummary(3), Relays, false, 100.f, FVector(0, 0, 0), Data).bSent, "B: 3 soldiers are too few");
	FHOLAIRelayMemory Whole = RelMemory();
	const FHOLAIRelayStep Step = HOLAIRelay::Step(Whole, HOLAIRelay::ModeWholeArmy, EHOLAIDifficulty::Easy, RelSummary(3), Relays, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Step.bSent, "C: the whole army goes");
	T.AssertEquals(3, Step.MoveIds.Num(), "all three soldiers");
	FHOLAIRelayMemory Quiet = RelMemory();
	T.AssertFalse(HOLAIRelay::Step(Quiet, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, RelSummary(8), Relays, true, 100.f, FVector(0, 0, 0), Data).bSent, "nothing is sent while the forum is attacked");
}

// A relay of an ally is ours and is not a target.
void Test_AIRelayAnAlliesRelayIsNotATarget(FUnitTest& T)
{
	const FHOLAIRelayData Data;
	FHOLAIRelayMemory Memory = RelMemory();
	FHOLAISummary Summary = RelSummary(8);
	FHOLAIForum Ally;
	Ally.Slot = 3;
	Summary.AllyForums.Add(Ally);
	TArray<FHOLAIRelay> Relays;
	Relays.Add(RelAt(2000, 3));
	T.AssertTrue(HOLAIRelay::IsOurs(Summary, 1, 3), "an ally's relay is ours");
	T.AssertFalse(HOLAIRelay::Step(Memory, HOLAIRelay::ModeWholeArmy, EHOLAIDifficulty::Hard, Summary, Relays, false, 100.f, FVector(0, 0, 0), Data).bSent, "nothing to take");
}

// The forum attacked recalls the detachment, which is not sent again for the pause; soldiers killed leave it.
void Test_AIRelayRecallWhenTheForumIsAttacked(FUnitTest& T)
{
	const FHOLAIRelayData Data;
	FHOLAIRelayMemory Memory = RelMemory();
	FHOLAISummary Summary = RelSummary(8);
	TArray<FHOLAIRelay> Relays;
	Relays.Add(RelAt(3000, -1));
	T.AssertTrue(HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 100.f, FVector(0, 0, 0), Data).bSent, "sent");
	T.AssertEquals(2, Memory.Ids.Num(), "two soldiers detached");
	const FHOLAIRelayStep Recall = HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, true, 110.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Recall.bRecalled, "recalled");
	T.AssertEquals(0, Memory.Ids.Num(), "nobody is detached any more");
	T.AssertEquals(8, HOLAIRelay::Without(Summary, Memory).Army.Num(), "the whole army is back in the other rules' sight");
	T.AssertFalse(HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 120.f, FVector(0, 0, 0), Data).bSent, "not sent again during the pause");
	T.AssertTrue(HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 141.f, FVector(0, 0, 0), Data).bSent, "sent again after it");
	Summary.Army[0].Health = 0;
	Summary.Army[1].Health = 0;
	const FHOLAIRelayStep Lost = HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 142.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Lost.bEnded, "a detachment with no soldier alive ends");
}

// On its way: only the idle soldiers away from the zone are ordered again, and not before the delay.
void Test_AIRelayOrdersAgainTheIdleOnesAway(FUnitTest& T)
{
	const FHOLAIRelayData Data;
	FHOLAIRelayMemory Memory = RelMemory();
	FHOLAISummary Summary = RelSummary(8);
	TArray<FHOLAIRelay> Relays;
	Relays.Add(RelAt(3000, -1));
	HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 100.f, FVector(0, 0, 0), Data);
	Summary.Army[0].bIdle = true;
	Summary.Army[1].bIdle = true;
	Summary.Army[1].Location = FVector(3000, 0, 0);
	T.AssertEquals(0, HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 102.f, FVector(0, 0, 0), Data).MoveIds.Num(), "not before the delay");
	const FHOLAIRelayStep Again = HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 106.f, FVector(0, 0, 0), Data);
	T.AssertEquals(1, Again.MoveIds.Num(), "one idle soldier away from the zone");
	T.AssertEquals(100, Again.MoveIds[0], "the one that is away");
	Relays[0].Owner = 1;
	T.AssertTrue(HOLAIRelay::Step(Memory, HOLAIRelay::ModeDetachment, EHOLAIDifficulty::Normal, Summary, Relays, false, 120.f, FVector(0, 0, 0), Data).bEnded, "the relay is ours: the detachment ends");
}
