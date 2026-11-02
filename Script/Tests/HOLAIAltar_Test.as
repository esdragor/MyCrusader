// The AI's squad at the altar of a volcano site (V11, D639, D640). HOLAIAltar (Script/AI/HOLAIAltar.as) is pure; the contest and hold sizes are the
// simulation's (HOLVolcano::DecideAltarSquad), the altar (FHOLAIAltar) and the levels are the game's own.

FHOLAISummary AltarSummary(int Soldiers)
{
	FHOLAISummary Summary;
	for (int Index = 0; Index < Soldiers; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = 200 + Index;
		Unit.OwnerSlot = 1;
		Unit.Type = n"Spearman";
		Unit.Health = 10;
		Unit.Location = FVector(Index * 100, 0, 0);
		Unit.bIdle = true;
		Summary.Army.Add(Unit);
	}
	return Summary;
}

FHOLAIAltar AltarAt(int Site, float X, float Strength, float TraceDistance)
{
	FHOLAIAltar Altar;
	Altar.SiteIndex = Site;
	Altar.Location = FVector(X, 0, 0);
	Altar.Radius = 1200.f;
	Altar.AdversaryStrength = Strength;
	Altar.bAdversary = Strength > 0.f;
	Altar.TraceDistance = TraceDistance;
	return Altar;
}

// A free altar, its nearest trace far away: one soldier in 4 (Normal, Hard), one in 8 (Easy), the nearest to the altar first.
void Test_AIAltarFreeAltarHeldByAQuarter(FUnitTest& T)
{
	const FHOLAIAltarData Data;
	TArray<FHOLAIAltar> Altars;
	Altars.Add(AltarAt(0, 10000, 0.f, 9000.f));
	FHOLAIAltarMemory Memory;
	const FHOLAIAltarStep Step = HOLAIAltar::Step(Memory, EHOLAIDifficulty::Normal, AltarSummary(8), Altars, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Step.bSent, "a squad goes to the free altar");
	T.AssertEquals(2, Step.MoveIds.Num(), "a quarter of 8 soldiers");
	T.AssertEquals(0, Step.Site, "the site");
	T.AssertTrue(Math::Abs(Step.Where.X - 10000.f) < 1.f, "toward the altar");
	T.AssertEquals(207, Step.MoveIds[0], "the soldier nearest to the altar first");

	FHOLAIAltarMemory EasyMemory;
	const FHOLAIAltarStep Easy = HOLAIAltar::Step(EasyMemory, EHOLAIDifficulty::Easy, AltarSummary(8), Altars, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertEquals(1, Easy.MoveIds.Num(), "Easy: one soldier in 8");
}

// A trace closer than 40 m, or an adversary of nothing in the zone but a trace near: no hold.
void Test_AIAltarNoHoldWithATraceNear(FUnitTest& T)
{
	const FHOLAIAltarData Data;
	TArray<FHOLAIAltar> Altars;
	Altars.Add(AltarAt(0, 10000, 0.f, 3900.f));
	FHOLAIAltarMemory Memory;
	const FHOLAIAltarStep Step = HOLAIAltar::Step(Memory, EHOLAIDifficulty::Normal, AltarSummary(8), Altars, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertFalse(Step.bSent, "the nearest trace is at 39 m: no squad");
	T.AssertEquals(0, Step.MoveIds.Num(), "no order");
}

// An adversary on the altar: more soldiers than him, the contest rule. A big army: Hard keeps its soldiers (diversion).
void Test_AIAltarContestsASmallAdversaryButNotABigArmyInHard(FUnitTest& T)
{
	const FHOLAIAltarData Data;
	TArray<FHOLAIAltar> Small;
	Small.Add(AltarAt(0, 10000, 3.f, 9000.f));
	FHOLAIAltarMemory Memory;
	const FHOLAIAltarStep ToSmall = HOLAIAltar::Step(Memory, EHOLAIDifficulty::Normal, AltarSummary(10), Small, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertEquals(5, ToSmall.MoveIds.Num(), "an adversary of 3: 1.5 times, rounded up, 5");

	TArray<FHOLAIAltar> Big;
	Big.Add(AltarAt(0, 10000, 8.f, 9000.f));
	FHOLAIAltarMemory HardMemory;
	const FHOLAIAltarStep Hard = HOLAIAltar::Step(HardMemory, EHOLAIDifficulty::Hard, AltarSummary(10), Big, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertFalse(Hard.bSent, "Hard does not throw itself at a big army");
	T.AssertTrue(Hard.bDiversion, "it strikes elsewhere instead");
}

// The squad is not recalled by a smaller need (no to-and-fro), and a forum attacked recalls it; the altar no longer lit ends it.
void Test_AIAltarSquadStaysThenIsRecalledOrEnds(FUnitTest& T)
{
	const FHOLAIAltarData Data;
	TArray<FHOLAIAltar> Altars;
	Altars.Add(AltarAt(0, 10000, 3.f, 9000.f));
	FHOLAIAltarMemory Memory;
	HOLAIAltar::Step(Memory, EHOLAIDifficulty::Normal, AltarSummary(10), Altars, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertEquals(5, Memory.Ids.Num(), "five sent");
	Altars[0].AdversaryStrength = 1.f;
	const FHOLAIAltarStep Again = HOLAIAltar::Step(Memory, EHOLAIDifficulty::Normal, AltarSummary(10), Altars, false, 110.f, FVector(0, 0, 0), Data);
	T.AssertEquals(5, Memory.Ids.Num(), "the adversary shrank: the squad stays");
	T.AssertFalse(Again.bRecalled, "no recall");

	const FHOLAISummary Without = HOLAIAltar::Without(AltarSummary(10), Memory);
	T.AssertEquals(5, Without.Army.Num(), "the other rules do not see the squad");

	FHOLAIAltarMemory Attacked = Memory;
	const FHOLAIAltarStep Recall = HOLAIAltar::Step(Attacked, EHOLAIDifficulty::Normal, AltarSummary(10), Altars, true, 120.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Recall.bRecalled, "the forum is attacked: the squad is recalled");
	T.AssertEquals(0, Attacked.Ids.Num(), "released");

	TArray<FHOLAIAltar> None;
	const FHOLAIAltarStep Ended = HOLAIAltar::Step(Memory, EHOLAIDifficulty::Normal, AltarSummary(10), None, false, 130.f, FVector(0, 0, 0), Data);
	T.AssertTrue(Ended.bEnded, "no lit altar any more (held, woken): the squad ends");
	T.AssertEquals(0, Memory.Ids.Num(), "released");
}

// Without any altar in the world nothing happens (the rule is inert until the maps have sites).
void Test_AIAltarIsInertWithoutAnAltar(FUnitTest& T)
{
	const FHOLAIAltarData Data;
	TArray<FHOLAIAltar> None;
	FHOLAIAltarMemory Memory;
	const FHOLAIAltarStep Step = HOLAIAltar::Step(Memory, EHOLAIDifficulty::Hard, AltarSummary(10), None, false, 100.f, FVector(0, 0, 0), Data);
	T.AssertFalse(Step.bSent, "nothing sent");
	T.AssertEquals(0, Step.MoveIds.Num(), "no order");
}
