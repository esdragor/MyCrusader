// MIG-C-Volcano (CELEBORN-1117j): the pure volcano rules of HOLVolcanoRules.h, tested through HOLVolcanoRules:: (UHOLVolcanoRulesLibrary, envelopes only).
// Port of HOLVolcanoTests.cpp: StandardStaysDormant, CatastropheCycle, LavaSpeed, FlowFront, LavaReach, ObstacleBoxes, EruptionFollowsTheFlow, DrawFlow,
// SiteState, EvacuationPoint, AltarAdvance, AltarHold, AltarWake, FreshDeposit. The AltarSend / AltarHoldSquad cases are HOLVolcanoAltarSquad_Test.as.
// NEEDS THE C++ LOT (Source/HeroesOfLiberty/Simulation/HOLVolcanoRulesLibrary.*): compile only after the build. MatchSeed's .json half (MakeScenarioConfig,
// GetScenarioSeed: HOLTestHooks) stays C++; its state half (same seed, same eruption) is Test_VolcanoSiteStateSameSeedSameEruption.

bool VolNear(double A, double B, double Eps)
{
	return Math::Abs(A - B) <= Eps;
}

TArray<FVector> VolLFlow()
{
	TArray<FVector> Points;
	Points.Add(FVector(0, 0, 0));
	Points.Add(FVector(3000, 0, 50));
	Points.Add(FVector(3000, 4000, 0));
	return Points;
}

TArray<FVector> VolStraight(double X, double Y)
{
	TArray<FVector> Points;
	Points.Add(FVector(0, 0, 0));
	Points.Add(FVector(X, Y, 0));
	return Points;
}

FHOLVolcanoSiteLayout VolThreeFlowSite()
{
	FHOLVolcanoSiteLayout Site;
	FHOLVolcanoFlowLine East;
	East.Points = VolStraight(4000, 0);
	FHOLVolcanoFlowLine North;
	North.Points = VolStraight(0, 7000);
	FHOLVolcanoFlowLine West;
	West.Points = VolStraight(-9000, 0);
	Site.Flows.Add(East);
	Site.Flows.Add(North);
	Site.Flows.Add(West);
	return Site;
}

void Test_VolcanoStandardStaysDormant(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	bool bAlwaysDormant = true;
	for (float Now = 0.f; Now <= 3600.f; Now += 5.f)
	{
		if (HOLVolcanoRules::GetPhase(false, 7, Now, Tuning, 0.f) != 0)
			bAlwaysDormant = false;
	}
	T.AssertTrue(bAlwaysDormant, "without Catastrophe the site never wakes in 60 min (D398)");
}

void Test_VolcanoCatastropheCycle(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	for (int Seed = 1; Seed <= 20; Seed++)
	{
		const FHOLVolcanoCycleView First = HOLVolcanoRules::MakeCycle(Seed, 0, 0.f, Tuning, 0.f);
		const FString Tag = "seed " + Seed + ": ";
		T.AssertTrue(First.WarningAt >= Tuning.WindowOpensAfter && First.WarningAt <= Tuning.WindowOpensAfter + Tuning.WindowLength, Tag + "the alert starts inside the window");
		const float Alert = First.EruptAt - First.WarningAt;
		T.AssertTrue(Alert >= 60.f && Alert <= 90.f, Tag + "alert 60 to 90 s (D28)");
		const float Cooling = First.CooledAt - First.EruptEndAt;
		T.AssertTrue(Cooling >= 120.f && Cooling <= 180.f, Tag + "cooling 2 to 3 min (D379)");
		const float Rest = First.ReArmAt - First.CooledAt;
		T.AssertTrue(Rest >= 360.f && Rest <= 480.f, Tag + "rest 6 to 8 min, then the next window (D400)");

		T.AssertEquals(1, HOLVolcanoRules::GetPhase(true, Seed, 1.f, Tuning, 0.f), Tag + "armed before the alert");
		T.AssertEquals(2, HOLVolcanoRules::GetPhase(true, Seed, First.WarningAt + 1.f, Tuning, 0.f), Tag + "warning");
		T.AssertEquals(3, HOLVolcanoRules::GetPhase(true, Seed, First.EruptAt + 1.f, Tuning, 0.f), Tag + "erupting");
		T.AssertEquals(4, HOLVolcanoRules::GetPhase(true, Seed, First.EruptEndAt + 1.f, Tuning, 0.f), Tag + "cooling");
		T.AssertEquals(5, HOLVolcanoRules::GetPhase(true, Seed, First.CooledAt + 1.f, Tuning, 0.f), Tag + "resting on basalt");
		T.AssertEquals(1, HOLVolcanoRules::GetPhase(true, Seed, First.ReArmAt + 1.f, Tuning, 0.f), Tag + "armed again");
		T.AssertTrue(HOLVolcanoRules::MakeCycle(Seed, 0, 0.f, Tuning, 0.f).EruptAt == First.EruptAt, Tag + "same seed, same dates");
	}
}

void Test_VolcanoLavaSpeedIs70PercentOfTheManAtArms(FUnitTest& T)
{
	const float Man = HOLVolcanoRules::ManAtArmsSpeed();
	T.AssertTrue(Man > 0.f, "the table has a Man-at-arms");
	const FHOLVolcanoTuning Tuning;
	const float Speed = Man * Tuning.LavaSpeedFactor;
	T.AssertTrue(VolNear(Speed, Man * 0.7f, 0.01), "the lava walks at 70 % of the Man-at-arms' speed");
	T.AssertTrue(Speed > 480.f && Speed < 495.f, "about 490 cm/s");
	T.AssertTrue(Speed < Man, "slower than the Man-at-arms, so a unit that runs escapes it");
}

void Test_VolcanoFlowFront(FUnitTest& T)
{
	const TArray<FVector> Flow = VolLFlow();
	const TArray<FVector> Empty;
	const float Speed = 488.f;
	const float EruptAt = 500.f;
	T.AssertTrue(VolNear(HOLVolcanoRules::FlowLength(Flow), 7000.0, 0.01), "length: the legs added (Z ignored)");
	const float Duration = HOLVolcanoRules::EruptionDuration(Flow, Speed);
	T.AssertTrue(VolNear(Duration, 7000.0 / Speed, 0.001), "it reaches the end after length over speed");
	T.AssertEquals(0.f, HOLVolcanoRules::FrontDistance(Flow, EruptAt - 10.f, EruptAt, Speed), "not yet erupting: the front is at the crater");
	T.AssertEquals(0.f, HOLVolcanoRules::FrontDistance(Flow, EruptAt, EruptAt, Speed), "at the first instant: still at the crater");
	T.AssertTrue(VolNear(HOLVolcanoRules::FrontDistance(Flow, EruptAt + 5.f, EruptAt, Speed), 5.0 * Speed, 0.01), "5 s later: 5 x speed");
	T.AssertTrue(VolNear(HOLVolcanoRules::FrontDistance(Flow, EruptAt + Duration, EruptAt, Speed), 7000.0, 0.5), "at the end instant: the full length");
	T.AssertEquals(7000.f, HOLVolcanoRules::FrontDistance(Flow, EruptAt + 1000.f, EruptAt, Speed), "long after: it stays at the end");
	T.AssertTrue(HOLVolcanoRules::FrontDistance(Flow, EruptAt + Duration - 1.f, EruptAt, Speed) < 7000.f, "one second before the end it has not arrived");

	float Last = 0.f;
	bool bNeverBack = true;
	for (float Now = 400.f; Now < 700.f; Now += 0.1f)
	{
		const float Distance = HOLVolcanoRules::FrontDistance(Flow, Now, EruptAt, Speed);
		if (Distance < Last)
			bNeverBack = false;
		Last = Distance;
	}
	T.AssertTrue(bNeverBack, "the front never goes backward");

	T.AssertTrue(HOLVolcanoRules::PointAt(Flow, 0.f).Equals(FVector(0, 0, 0), 0.01), "at 0: the crater");
	T.AssertTrue(HOLVolcanoRules::PointAt(Flow, 1500.f).Equals(FVector(1500, 0, 25), 0.01), "at 1500: halfway along the first leg");
	T.AssertTrue(HOLVolcanoRules::PointAt(Flow, 3000.f).Equals(FVector(3000, 0, 50), 0.01), "at 3000: the corner");
	T.AssertTrue(HOLVolcanoRules::PointAt(Flow, 5000.f).Equals(FVector(3000, 2000, 25), 0.01), "at 5000: 2000 up the second leg");
	T.AssertTrue(HOLVolcanoRules::PointAt(Flow, 99999.f).Equals(FVector(3000, 4000, 0), 0.01), "beyond the end: the end");
	T.AssertTrue(HOLVolcanoRules::PointAt(Flow, -5.f).Equals(FVector(0, 0, 0), 0.01), "before the start: the crater");

	T.AssertEquals(0.f, HOLVolcanoRules::FlowLength(Empty), "no point: no length");
	T.AssertEquals(0.f, HOLVolcanoRules::EruptionDuration(Empty, Speed), "no length: no duration");
	T.AssertEquals(0.f, HOLVolcanoRules::FrontDistance(Empty, EruptAt + 10.f, EruptAt, Speed), "no length: the front stays at 0");
	T.AssertEquals(0.f, HOLVolcanoRules::EruptionDuration(Flow, 0.f), "no speed: no duration");
	T.AssertEquals(0.f, HOLVolcanoRules::FrontDistance(Flow, EruptAt + 10.f, EruptAt, 0.f), "no speed: the front does not move");
	T.AssertTrue(HOLVolcanoRules::PointAt(Empty, 10.f).IsZero(), "no point: the origin");
}

void Test_VolcanoLavaReach(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	const double H = Tuning.LavaHalfWidth;
	const float W = Tuning.LavaHalfWidth;
	T.AssertEquals(800.f, Tuning.LavaHalfWidth, "a flow is 16 m wide: half-width 800 cm (V5-Q1 = B, the user)");
	const TArray<FVector> Flow = VolLFlow();
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, 0.f, W, FVector(0, 0, 0), 0.f), "front at 0: the crater itself is safe");

	const float Front = 2000.f;
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(1000, 0, 0), 0.f), "on the line behind the front: burnt");
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(0, 0, 0), 0.f), "on the crater once it erupts: burnt");
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(1000, H - 10, 0), 0.f), "just inside the width: burnt");
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(1000, H + 10, 0), 0.f), "just outside the width: safe");
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(1000, -(H + 10), 0), 0.f), "outside the width on the other side: safe");
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(1000, H + 10, 0), 50.f), "a body's radius counts: outside by 10 with radius 50 touches it");
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(1000, H + 100, 0), 50.f), "a body far enough even with its radius: safe");
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(Front + H - 10, 0, 0), 0.f), "the head of the front is rounded: a little ahead of it, burnt");
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(Front + H + 10, 0, 0), 0.f), "well ahead of the front: safe");
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(3000, 2000, 0), 0.f), "ahead on the flow's second leg: safe");
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Front, W, FVector(1000, 0, 99999), 0.f), "Z is ignored");

	const float Turned = 5000.f;
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Turned, W, FVector(3000, 1500, 0), 0.f), "second leg, behind the front: burnt");
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, Turned, W, FVector(3000 + H - 10, 1500, 0), 0.f), "second leg, beside the line: burnt");
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, Turned, W, FVector(3000, 3500, 0), 0.f), "second leg, beyond the front: safe");
	T.AssertFalse(HOLVolcanoRules::IsInLava(Flow, Turned, W, FVector(0, 3000, 0), 0.f), "away from both legs: safe");
	T.AssertTrue(HOLVolcanoRules::IsInLava(Flow, HOLVolcanoRules::FlowLength(Flow), W, FVector(3000, 4000, 0), 0.f), "front at the end: the far end is burnt");

	const TArray<FVector> Empty;
	TArray<FVector> OnePoint;
	OnePoint.Add(FVector(0, 0, 0));
	T.AssertFalse(HOLVolcanoRules::IsInLava(Empty, 100.f, W, FVector(0, 0, 0), 0.f), "no point: no lava");
	T.AssertFalse(HOLVolcanoRules::IsInLava(OnePoint, 100.f, W, FVector(0, 0, 0), 0.f), "one point: no lava");

	T.AssertTrue(HOLVolcanoRules::LavaDestroys(false, false), "a unit is destroyed");
	T.AssertTrue(HOLVolcanoRules::LavaDestroys(true, false), "a hero outside a duel is destroyed");
	T.AssertFalse(HOLVolcanoRules::LavaDestroys(true, true), "a hero in a duel is not");
}

bool VolInBox(const FHOLVolcanoBoxView& Box, double X, double Y)
{
	const double Yaw = -Math::DegreesToRadians(Box.YawDegrees);
	const double DX = X - Box.Centre.X;
	const double DY = Y - Box.Centre.Y;
	const double LocalX = DX * Math::Cos(Yaw) - DY * Math::Sin(Yaw);
	const double LocalY = DX * Math::Sin(Yaw) + DY * Math::Cos(Yaw);
	return Math::Abs(LocalX) <= Box.HalfLength + 0.01 && Math::Abs(LocalY) <= Box.HalfWidth + 0.01;
}

void Test_VolcanoObstacleBoxesCoverTheLavaReach(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	const float H = Tuning.LavaHalfWidth;
	const TArray<FVector> Flow = VolLFlow();
	const TArray<FHOLVolcanoBoxView> Boxes = HOLVolcanoRules::GetObstacleBoxes(Flow, H);
	T.AssertEquals(2, Boxes.Num(), "one box per segment");
	if (Boxes.Num() != 2)
		return;
	T.AssertTrue(VolNear(Boxes[0].Centre.X, 1500.0, 0.01), "first box: centre");
	T.AssertTrue(VolNear(Boxes[0].HalfLength, 1500.0 + H, 0.01), "first box: half length = half the segment + the half width at each end");
	T.AssertTrue(VolNear(Boxes[0].HalfWidth, H, 0.01), "first box: half width");
	T.AssertTrue(VolNear(Boxes[0].YawDegrees, 0.0, 0.01), "first box: along +X");
	T.AssertTrue(VolNear(Boxes[1].YawDegrees, 90.0, 0.01), "second box: along +Y");
	T.AssertTrue(VolNear(Boxes[1].Centre.Y, 2000.0, 0.01), "second box: centre");

	const float Length = HOLVolcanoRules::FlowLength(Flow);
	int Reach = 0;
	int Missed = 0;
	int FarCount = 0;
	int FarInBox = 0;
	for (double X = -1500.0; X <= 4500.0; X += 50.0)
	{
		for (double Y = -1500.0; Y <= 5500.0; Y += 50.0)
		{
			const bool bInBoxes = VolInBox(Boxes[0], X, Y) || VolInBox(Boxes[1], X, Y);
			if (HOLVolcanoRules::IsInLava(Flow, Length, H, FVector(X, Y, 0), 0.f))
			{
				Reach++;
				if (!bInBoxes)
					Missed++;
			}
			else if (!HOLVolcanoRules::IsInLava(Flow, Length, 2.f * H, FVector(X, Y, 0), 0.f))
			{
				FarCount++;
				if (bInBoxes)
					FarInBox++;
			}
		}
	}
	T.AssertTrue(Reach > 1000 && FarCount > 1000, "the sample has points under the lava and far from it");
	T.AssertEquals(0, Missed, "every point the lava reaches is in the boxes");
	T.AssertEquals(0, FarInBox, "no point farther than twice the half width is");
	const TArray<FVector> Empty;
	T.AssertEquals(0, HOLVolcanoRules::GetObstacleBoxes(Empty, H).Num(), "no flow, no box");
}

void Test_VolcanoEruptionFollowsTheFlow(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	const float Duration = HOLVolcanoRules::EruptionDuration(VolStraight(6000, 0), 488.f);
	for (int Seed = 1; Seed <= 10; Seed++)
	{
		const FHOLVolcanoCycleView Cycle = HOLVolcanoRules::MakeCycle(Seed, 0, 0.f, Tuning, Duration);
		const FString Tag = "seed " + Seed + ": ";
		T.AssertTrue(VolNear(Cycle.EruptEndAt - Cycle.EruptAt, Duration, 0.001), Tag + "the eruption lasts the flow's duration");
		T.AssertEquals(3, HOLVolcanoRules::GetPhase(true, Seed, Cycle.EruptEndAt - 0.5f, Tuning, Duration), Tag + "erupting just before the lava arrives");
		T.AssertEquals(4, HOLVolcanoRules::GetPhase(true, Seed, Cycle.EruptEndAt + 0.5f, Tuning, Duration), Tag + "cooling once it has");
		T.AssertTrue(Cycle.EruptAt == HOLVolcanoRules::MakeCycle(Seed, 0, 0.f, Tuning, 0.f).EruptAt, Tag + "the alert and the eruption's start do not move with the flow");
	}
}

void Test_VolcanoDrawFlow(FUnitTest& T)
{
	T.AssertEquals(-1, HOLVolcanoRules::DrawFlow(3, 0, 0), "no flow: nothing to draw");
	T.AssertEquals(0, HOLVolcanoRules::DrawFlow(3, 5, 1), "one flow: always that one");
	const int FlowCount = 4;
	TArray<int> Counts;
	for (int Flow = 0; Flow < FlowCount; Flow++)
		Counts.Add(0);
	int Repeats = 0;
	int Pairs = 0;
	bool bInRange = true;
	bool bStable = true;
	for (int Seed = 1; Seed <= 1000; Seed++)
	{
		int Previous = -1;
		for (int Index = 0; Index < 4; Index++)
		{
			const int Flow = HOLVolcanoRules::DrawFlow(Seed, Index, FlowCount);
			if (Flow < 0 || Flow >= FlowCount)
			{
				bInRange = false;
				continue;
			}
			if (Flow != HOLVolcanoRules::DrawFlow(Seed, Index, FlowCount))
				bStable = false;
			Counts[Flow] = Counts[Flow] + 1;
			if (Previous != -1)
			{
				Pairs++;
				if (Flow == Previous)
					Repeats++;
			}
			Previous = Flow;
		}
	}
	T.AssertTrue(bInRange, "always one of the site's flows");
	T.AssertTrue(bStable, "same seed and cycle, same flow (every machine agrees)");
	for (int Flow = 0; Flow < FlowCount; Flow++)
		T.AssertTrue(Counts[Flow] > 850 && Counts[Flow] < 1150, "flow " + Flow + " is drawn about a quarter of the time (got " + Counts[Flow] + " of 4000)");
	T.AssertTrue(Repeats > Pairs * 15 / 100 && Repeats < Pairs * 35 / 100, "no memory: the same flow twice in a row about once in four (" + Repeats + " of " + Pairs + ")");

	const FHOLVolcanoTuning Tuning;
	const FHOLVolcanoCycleView Before = HOLVolcanoRules::MakeCycle(9, 1, 100.f, Tuning, 0.f);
	HOLVolcanoRules::DrawFlow(9, 1, FlowCount);
	T.AssertTrue(HOLVolcanoRules::MakeCycle(9, 1, 100.f, Tuning, 0.f).EruptAt == Before.EruptAt, "the dates of a cycle do not depend on the draw");
}

void Test_VolcanoSiteStateFollowsTheCycles(FUnitTest& T)
{
	const float Man = HOLVolcanoRules::ManAtArmsSpeed();
	T.AssertTrue(Man > 0.f, "the table gives a Man-at-arms' speed");
	const FHOLVolcanoTuning Tuning;
	const float Speed = Man * Tuning.LavaSpeedFactor;
	const FHOLVolcanoSiteLayout Site = VolThreeFlowSite();

	bool bDormant = true;
	for (float Now = 0.f; Now <= 3600.f; Now += 7.f)
	{
		const FHOLVolcanoStateView State = HOLVolcanoRules::SiteStateAt(false, 5, 0, Site, Now, Tuning, Man, -1, 0.f);
		if (State.Phase != 0 || State.FlowIndex != -1 || State.FrontDistance != 0.f)
			bDormant = false;
	}
	T.AssertTrue(bDormant, "no Catastrophe: dormant for an hour, no flow, no front");

	for (int MatchSeed = 1; MatchSeed <= 12; MatchSeed++)
	{
		const FString Tag = "seed " + MatchSeed + ": ";
		const int SeedOfSite = HOLVolcanoRules::SiteSeed(MatchSeed, 0);
		const int Flow0 = HOLVolcanoRules::DrawFlow(SeedOfSite, 0, Site.Flows.Num());
		const float Length0 = HOLVolcanoRules::EruptionDuration(Site.Flows[Flow0].Points, Speed);
		const FHOLVolcanoCycleView Cycle0 = HOLVolcanoRules::MakeCycle(SeedOfSite, 0, 0.f, Tuning, Length0);

		const FHOLVolcanoStateView Armed = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, 1.f, Tuning, Man, -1, 0.f);
		T.AssertTrue(Armed.Phase == 1 && Armed.CycleIndex == 0 && Armed.FlowIndex == Flow0, Tag + "armed at the start, cycle 0, the flow of the draw");

		const FHOLVolcanoStateView Warning = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.WarningAt + 5.f, Tuning, Man, -1, 0.f);
		T.AssertTrue(Warning.Phase == 2 && Warning.FlowIndex == Flow0, Tag + "alert");
		T.AssertTrue(VolNear(Warning.SecondsToErupt, Cycle0.EruptAt - (Cycle0.WarningAt + 5.f), 0.01), Tag + "the countdown to the lava");
		T.AssertEquals(0.f, Warning.FrontDistance, Tag + "the front waits at the crater");

		const float Probe = Length0 * 0.5f;
		const FHOLVolcanoStateView Erupting = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.EruptAt + Probe, Tuning, Man, -1, 0.f);
		T.AssertEquals(3, Erupting.Phase, Tag + "erupting");
		T.AssertTrue(VolNear(Erupting.FrontDistance, Probe * Speed, 0.5), Tag + "half the flow's time of lava");
		T.AssertTrue(Erupting.FrontPoint.Equals(HOLVolcanoRules::PointAt(Site.Flows[Flow0].Points, Probe * Speed), 1.0), Tag + "the front point is on the drawn flow");
		T.AssertEquals(-1.f, Erupting.SecondsToErupt, Tag + "no countdown once it flows");

		const FHOLVolcanoStateView Cooling = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.EruptEndAt + 1.f, Tuning, Man, -1, 0.f);
		T.AssertTrue(Cooling.Phase == 4 && VolNear(Cooling.FrontDistance, HOLVolcanoRules::FlowLength(Site.Flows[Flow0].Points), 0.5), Tag + "cooling, the lava at the end of its flow");
		T.AssertEquals(5, HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.CooledAt + 1.f, Tuning, Man, -1, 0.f).Phase, Tag + "basalt, resting");

		const int Flow1 = HOLVolcanoRules::DrawFlow(SeedOfSite, 1, Site.Flows.Num());
		const FHOLVolcanoCycleView Cycle1 = HOLVolcanoRules::MakeCycle(SeedOfSite, 1, Cycle0.ReArmAt, Tuning, HOLVolcanoRules::EruptionDuration(Site.Flows[Flow1].Points, Speed));
		const FHOLVolcanoStateView Next = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, Cycle1.EruptAt + 3.f, Tuning, Man, -1, 0.f);
		T.AssertTrue(Next.Phase == 3 && Next.CycleIndex == 1 && Next.FlowIndex == Flow1, Tag + "the second eruption, cycle 1, its own flow");
		T.AssertTrue(Cycle1.WarningAt > Cycle0.ReArmAt, Tag + "the second window opens after the first rest");
	}

	int Different = 0;
	for (int MatchSeed = 1; MatchSeed <= 12; MatchSeed++)
	{
		if (HOLVolcanoRules::SiteSeed(MatchSeed, 0) != HOLVolcanoRules::SiteSeed(MatchSeed, 1))
			Different++;
	}
	T.AssertEquals(12, Different, "the seed of site 1 is never site 0's");
	T.AssertTrue(HOLVolcanoRules::SiteSeed(1, 0) != HOLVolcanoRules::SiteSeed(2, 0), "another match, another seed");

	const FHOLVolcanoSiteLayout Bare;
	const FHOLVolcanoCycleView BareCycle = HOLVolcanoRules::MakeCycle(HOLVolcanoRules::SiteSeed(3, 0), 0, 0.f, Tuning, 0.f);
	const FHOLVolcanoStateView BareState = HOLVolcanoRules::SiteStateAt(true, 3, 0, Bare, BareCycle.EruptAt + 5.f, Tuning, Man, -1, 0.f);
	T.AssertTrue(BareState.Phase == 3 && BareState.FlowIndex == -1 && BareState.FrontDistance == 0.f, "no flow: erupting by the placeholder, nothing drawn, the front stays");
}

// MatchSeed's state half: the same seed gives the same eruption at every instant, another seed another one, no Catastrophe sleeps.
void Test_VolcanoSiteStateSameSeedSameEruption(FUnitTest& T)
{
	const float Man = HOLVolcanoRules::ManAtArmsSpeed();
	const FHOLVolcanoTuning Tuning;
	const FHOLVolcanoSiteLayout Site = VolThreeFlowSite();
	int SameCount = 0;
	for (float Now = 0.f; Now <= 1800.f; Now += 9.f)
	{
		const FHOLVolcanoStateView A = HOLVolcanoRules::SiteStateAt(true, 4242, 0, Site, Now, Tuning, Man, -1, 0.f);
		const FHOLVolcanoStateView B = HOLVolcanoRules::SiteStateAt(true, 4242, 0, Site, Now, Tuning, Man, -1, 0.f);
		if (A.Phase == B.Phase && A.CycleIndex == B.CycleIndex && A.FlowIndex == B.FlowIndex && A.FrontPoint.Equals(B.FrontPoint, 0.001))
			SameCount++;
	}
	T.AssertEquals(201, SameCount, "same seed, same eruption at each of the 201 instants");
	bool bDiffers = false;
	for (float Now = 0.f; Now <= 3600.f && !bDiffers; Now += 3.f)
	{
		const FHOLVolcanoStateView A = HOLVolcanoRules::SiteStateAt(true, 4242, 0, Site, Now, Tuning, Man, -1, 0.f);
		const FHOLVolcanoStateView C = HOLVolcanoRules::SiteStateAt(true, 4243, 0, Site, Now, Tuning, Man, -1, 0.f);
		bDiffers = A.Phase != C.Phase || A.FlowIndex != C.FlowIndex;
	}
	T.AssertTrue(bDiffers, "another seed, another eruption");
	T.AssertEquals(0, HOLVolcanoRules::SiteStateAt(false, 4242, 0, Site, 700.f, Tuning, Man, -1, 0.f).Phase, "no Catastrophe: dormant");
}

void Test_VolcanoEvacuationPoint(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	const float W = Tuning.LavaHalfWidth;
	const TArray<FVector> Flow = VolStraight(8000, 0);
	T.AssertEquals(W + 60.f, HOLVolcanoRules::EvacuationReach(W, 60.f), "the reach is the half width plus the radius");
	T.AssertEquals(360.f, HOLVolcanoRules::EvacuationReach(300.f, 60.f), "... of the setting, whatever it is");
	T.AssertTrue(HOLVolcanoRules::IsInCorridor(Flow, W, FVector(4000, 700, 0), 60.f), "700 cm aside, radius 60: inside the 16 m corridor");
	T.AssertFalse(HOLVolcanoRules::IsInCorridor(Flow, 400.f, FVector(4000, 700, 0), 60.f), "... but outside an 8 m corridor");
	T.AssertFalse(HOLVolcanoRules::IsInCorridor(Flow, W, FVector(4000, W + 61, 0), 60.f), "the radius counts: 800 + 60 + 1 cm aside is out");
	T.AssertTrue(HOLVolcanoRules::IsInCorridor(Flow, W, FVector(4000, W + 59, 0), 60.f), "... 800 + 60 - 1 cm is in");
	T.AssertFalse(HOLVolcanoRules::IsInCorridor(Flow, W, FVector(4000, 3000, 0), 60.f), "far from the line: out");

	const FVector Out = HOLVolcanoRules::EvacuationPoint(Flow, W, FVector(4000, 700, 25), 60.f, 200.f);
	T.AssertFalse(HOLVolcanoRules::IsInCorridor(Flow, W, Out, 60.f), "the point is out of the corridor");
	T.AssertTrue(Out.Y > 0.0, "... on the same side");
	T.AssertTrue(Out.Y >= W + 60.f + 199.f, "... with a margin beyond the reach");
	T.AssertTrue(Math::Abs(Out.X - 4000.0) < 1.0, "... straight away from the line");
	T.AssertTrue(VolNear(Out.Z, 25.0, 0.01), "... at its height");
	T.AssertTrue(HOLVolcanoRules::EvacuationPoint(Flow, W, FVector(4000, 3000, 0), 60.f, 200.f).Equals(FVector(4000, 3000, 0), 0.001), "a body already out stays where it is");
	T.AssertFalse(HOLVolcanoRules::IsInCorridor(Flow, W, HOLVolcanoRules::EvacuationPoint(Flow, W, FVector(2000, 0, 0), 60.f, 200.f), 60.f), "a body on the line itself is led out as well");
	const FVector OtherSide = HOLVolcanoRules::EvacuationPoint(Flow, W, FVector(4000, -300, 0), 60.f, 200.f);
	T.AssertTrue(OtherSide.Y < 0.0 && !HOLVolcanoRules::IsInCorridor(Flow, W, OtherSide, 60.f), "a body on the other side goes to the other side");

	int Caught = 0;
	int Left = 0;
	for (double X = -500.0; X <= 8500.0; X += 500.0)
	{
		for (double Y = -1500.0; Y <= 1500.0; Y += 100.0)
		{
			const FVector P(X, Y, 0);
			if (HOLVolcanoRules::IsInCorridor(Flow, W, P, 60.f))
			{
				Caught++;
				if (!HOLVolcanoRules::IsInCorridor(Flow, W, HOLVolcanoRules::EvacuationPoint(Flow, W, P, 60.f, 200.f), 60.f))
					Left++;
			}
		}
	}
	T.AssertTrue(Caught > 100, "the grid has bodies in the corridor");
	T.AssertEquals(Caught, Left, "every one of them is led out");

	const TArray<FVector> Line = VolStraight(10000, 0);
	T.AssertTrue(VolNear(HOLVolcanoRules::DistanceToFlow(Line, FVector(5000, 3000, 0)), 3000.0, 0.5), "the distance to a line: 3000 cm beside its middle");
	T.AssertTrue(VolNear(HOLVolcanoRules::DistanceToFlow(Line, FVector(13000, 4000, 0)), 5000.0, 0.5), "... past its end: to the end point");
}

void Test_VolcanoAltarAdvance(FUnitTest& T)
{
	FHOLVolcanoTuning Tuning;
	const FHOLVolcanoCycleView Base = HOLVolcanoRules::MakeCycle(4242, 0, 0.f, Tuning, 60.f);
	T.AssertTrue(Tuning.AltarAdvance > 0.f, "the setting is a positive advance");
	const FHOLVolcanoCycleView Early = HOLVolcanoRules::AdvanceForAltar(Base, 10.f, Tuning);
	T.AssertEquals(10.f, Early.WarningAt, "the alert starts the instant the altar is held");
	T.AssertTrue(VolNear(Early.EruptAt - Early.WarningAt, Base.EruptAt - Base.WarningAt, 0.001), "the alert keeps its length (D380)");
	T.AssertTrue(VolNear(Early.EruptAt, 10.0 + (Base.EruptAt - Base.WarningAt), 0.001), "the eruption follows the alert");
	T.AssertTrue(VolNear(Base.ReArmAt - Early.ReArmAt, Base.WarningAt - 10.f, 0.001), "the cooling and the re-arming follow by the same shift");
	const float Late = Base.WarningAt - 30.f;
	T.AssertTrue(VolNear(HOLVolcanoRules::AdvanceForAltar(Base, Late, Tuning).WarningAt, Late, 0.001), "held 30 s before the alert: the alert comes at once");
	T.AssertTrue(HOLVolcanoRules::AdvanceForAltar(Base, Base.WarningAt + 1.f, Tuning).WarningAt == Base.WarningAt, "held during the alert: unchanged");
	FHOLVolcanoTuning None = Tuning;
	None.AltarAdvance = 0.f;
	T.AssertTrue(HOLVolcanoRules::AdvanceForAltar(Base, 10.f, None).WarningAt == Base.WarningAt, "no advance: unchanged");
	FHOLVolcanoTuning Bounded = Tuning;
	Bounded.AltarAdvance = 120.f;
	T.AssertTrue(VolNear(HOLVolcanoRules::AdvanceForAltar(Base, 10.f, Bounded).WarningAt, Base.WarningAt - 120.f, 0.001), "a bounded advance moves the alert by that much");
}

void Test_VolcanoAltarHoldCount(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	T.AssertEquals(60.f, Tuning.AltarHoldSeconds, "held ~60 s (D380)");
	float Held = 0.f;
	T.AssertEquals(1, HOLVolcanoRules::StepAltar(Held, 1, 1.f, Tuning), "one camp: holding");
	T.AssertEquals(1.f, Held, "... the count goes up by the step");
	for (int Second = 0; Second < 19; Second++)
		HOLVolcanoRules::StepAltar(Held, 1, 1.f, Tuning);
	T.AssertEquals(20.f, Held, "20 s held");
	T.AssertEquals(2, HOLVolcanoRules::StepAltar(Held, 2, 1.f, Tuning), "two camps: contested");
	T.AssertEquals(2, HOLVolcanoRules::StepAltar(Held, 3, 5.f, Tuning), "... three camps too");
	T.AssertEquals(20.f, Held, "... the count is frozen");
	T.AssertEquals(0, HOLVolcanoRules::StepAltar(Held, 0, 1.f, Tuning), "nobody: idle");
	T.AssertEquals(20.f - Tuning.AltarDecayRate, Held, "... the count goes down by the decay rate");
	HOLVolcanoRules::StepAltar(Held, 0, 1000.f, Tuning);
	T.AssertEquals(0.f, Held, "... never below zero");

	float Owned = 0.f;
	int Holder = -1;
	for (int Second = 0; Second < 29; Second++)
		HOLVolcanoRules::StepAltarOwned(Owned, Holder, 1, 0, 1.f, Tuning);
	T.AssertTrue(Holder == 0 && VolNear(Owned, 29.0, 0.001), "camp 0 holds 29 s");
	HOLVolcanoRules::StepAltarOwned(Owned, Holder, 2, -1, 1.f, Tuning);
	T.AssertEquals(29.f, Owned, "contested: frozen, still camp 0's");
	HOLVolcanoRules::StepAltarOwned(Owned, Holder, 1, 0, 1.f, Tuning);
	T.AssertEquals(30.f, Owned, "camp 0 alone again: goes on");
	HOLVolcanoRules::StepAltarOwned(Owned, Holder, 1, 1, 1.f, Tuning);
	T.AssertTrue(Holder == 1 && Owned == 1.f, "camp 1 alone after camp 0's 30 s: starts again, 1 s, not 31");

	float Count = 0.f;
	for (int Second = 0; Second < 59; Second++)
		HOLVolcanoRules::StepAltar(Count, 1, 1.f, Tuning);
	T.AssertTrue(Count < Tuning.AltarHoldSeconds, "59 s: not held yet");
	HOLVolcanoRules::StepAltar(Count, 1, 1.f, Tuning);
	T.AssertTrue(Count >= Tuning.AltarHoldSeconds, "60 s: held");
}

void Test_VolcanoAltarWakeStartsTheAlert(FUnitTest& T)
{
	const FHOLVolcanoTuning Tuning;
	const float Man = 697.f;
	const int MatchSeed = 4242;
	FHOLVolcanoSiteLayout Site;
	FHOLVolcanoFlowLine Line;
	Line.Points = VolStraight(9000, 0);
	Site.Flows.Add(Line);
	Site.bAltarPresent = true;
	Site.AltarLocation = FVector(200, 0, 0);

	const FHOLVolcanoStateView Plain = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, 30.f, Tuning, Man, -1, 0.f);
	T.AssertEquals(1, Plain.Phase, "armed at 30 s");
	T.AssertTrue(Plain.bAltarLit, "the altar is lit while armed");
	T.AssertFalse(Plain.bWoken, "... and the cycle is not woken");

	const FHOLVolcanoStateView Woken = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, 31.f, Tuning, Man, Plain.CycleIndex, 30.f);
	T.AssertEquals(2, Woken.Phase, "one second after the altar is held: the alert");
	T.AssertTrue(Woken.bWoken, "... woken");
	T.AssertFalse(Woken.bAltarLit, "... the altar is no longer lit");
	T.AssertEquals(30.f, Woken.WarningAt, "... the alert began when the altar was held");
	T.AssertTrue(Woken.EruptAt - Woken.WarningAt >= Tuning.WarningMin && Woken.EruptAt - Woken.WarningAt <= Tuning.WarningMax, "... the alert has its 60 to 90 s (D28)");
	T.AssertTrue(VolNear(Woken.SecondsToErupt, Woken.EruptAt - 31.f, 0.001), "... the countdown is the alert's length less the second spent");
	T.AssertTrue(Woken.FlowIndex == Plain.FlowIndex, "same flow as the plain cycle (the draw does not move)");
	T.AssertTrue(HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, 31.f, Tuning, Man, -1, 0.f).Phase != 2, "without the wake the same instant is not the alert");
	T.AssertEquals(3, HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, Woken.EruptAt + 2.f, Tuning, Man, Plain.CycleIndex, 30.f).Phase, "erupting two seconds after the woken date");
	const float ProbeAt = Plain.EruptAt + 2000.f;
	T.AssertTrue(HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, ProbeAt, Tuning, Man, Plain.CycleIndex, 30.f).CycleIndex >= HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, ProbeAt, Tuning, Man, -1, 0.f).CycleIndex, "the later cycles come sooner than without the altar (D400)");
	T.AssertEquals(0, Plain.CycleIndex, "... and the woken cycle is the first one");
	const FHOLVolcanoStateView Later = HOLVolcanoRules::SiteStateAt(true, MatchSeed, 0, Site, Woken.EruptAt + 1000.f, Tuning, Man, Plain.CycleIndex, 30.f);
	T.AssertEquals(Plain.CycleIndex + 1, Later.CycleIndex, "a cycle later: a new cycle");
	T.AssertTrue(Later.Phase != 1 || Later.bAltarLit, "... the altar is lit again (D400) whenever that cycle is armed");
	T.AssertFalse(HOLVolcanoRules::SiteStateAt(false, MatchSeed, 0, Site, 30.f, Tuning, Man, -1, 0.f).bAltarLit, "no Catastrophe: no altar lit");
}

void Test_VolcanoFreshDeposit(FUnitTest& T)
{
	FHOLVolcanoTuning Tuning;
	TArray<FVector> Flow;
	Flow.Add(FVector(0, 0, 0));
	Flow.Add(FVector(6000, 0, 0));
	Flow.Add(FVector(6000, 4000, 0));
	const float Length = HOLVolcanoRules::FlowLength(Flow);
	const int SiteSeedValue = HOLVolcanoRules::SiteSeed(4242, 0);

	const FHOLVolcanoFreshView First = HOLVolcanoRules::FreshDeposit(SiteSeedValue, 0, Flow, Tuning);
	T.AssertTrue(First.bPresent, "a flow with a length leaves a deposit");
	T.AssertTrue(First.Location.Equals(HOLVolcanoRules::PointAt(Flow, First.Along * Length), 1.0), "it stands where its drawn fraction says");
	T.AssertTrue(HOLVolcanoRules::DistanceToFlow(Flow, First.Location) < 1.f, "... which is on the flow's line");
	FName Expected = First.Kind == 0 ? (First.bBig ? Tuning.FreshDepositGoldBigType : Tuning.FreshDepositGoldSmallType) : (First.bBig ? Tuning.FreshDepositStoneBigType : Tuning.FreshDepositStoneSmallType);
	T.AssertTrue(First.TypeName == Expected, "... named by the setting of its kind and its size");
	const FHOLVolcanoFreshView Again = HOLVolcanoRules::FreshDeposit(SiteSeedValue, 0, Flow, Tuning);
	T.AssertTrue(Again.Kind == First.Kind && Again.Location == First.Location && Again.bBig == First.bBig, "the same call gives the same deposit");

	float LowestAlong = 2.f;
	float HighestAlong = -1.f;
	int Big = 0;
	int GoldSmall = 0;
	int GoldBig = 0;
	int StoneSmall = 0;
	int StoneBig = 0;
	bool bAllNamed = true;
	for (int Cycle = 0; Cycle < 400; Cycle++)
	{
		const FHOLVolcanoFreshView Drawn = HOLVolcanoRules::FreshDeposit(SiteSeedValue, Cycle, Flow, Tuning);
		LowestAlong = Math::Min(LowestAlong, Drawn.Along);
		HighestAlong = Math::Max(HighestAlong, Drawn.Along);
		if (Drawn.bBig)
			Big++;
		if (Drawn.Kind == 0)
		{
			if (Drawn.bBig)
				GoldBig++;
			else
				GoldSmall++;
		}
		else
		{
			if (Drawn.bBig)
				StoneBig++;
			else
				StoneSmall++;
		}
		if (!Drawn.Location.Equals(HOLVolcanoRules::PointAt(Flow, Drawn.Along * Length), 1.0) || Drawn.TypeName.IsNone())
			bAllNamed = false;
	}
	const float Range = Tuning.FreshDepositAlongMax - Tuning.FreshDepositAlongMin;
	T.AssertTrue(LowestAlong >= Tuning.FreshDepositAlongMin - 0.0001f && HighestAlong <= Tuning.FreshDepositAlongMax + 0.0001f, "the place is always between the minimum and the maximum of the settings");
	T.AssertTrue(LowestAlong < Tuning.FreshDepositAlongMin + 0.1f * Range && HighestAlong > Tuning.FreshDepositAlongMax - 0.1f * Range, "... and it varies: the 400 cycles cover most of the range");
	T.AssertTrue(Big > 140 && Big < 260, "both sizes come, about half big over 400 cycles");
	T.AssertTrue(GoldSmall > 20 && GoldBig > 20 && StoneSmall > 20 && StoneBig > 20, "all four combinations come");
	T.AssertTrue(bAllNamed, "every draw stands on the flow and has a type");

	Tuning.FreshDepositBigShare = 0.f;
	Big = 0;
	for (int Cycle = 0; Cycle < 50; Cycle++)
		if (HOLVolcanoRules::FreshDeposit(SiteSeedValue, Cycle, Flow, Tuning).bBig)
			Big++;
	T.AssertEquals(0, Big, "big share 0: always small");
	Tuning.FreshDepositBigShare = 1.f;
	for (int Cycle = 0; Cycle < 50; Cycle++)
		if (HOLVolcanoRules::FreshDeposit(SiteSeedValue, Cycle, Flow, Tuning).bBig)
			Big++;
	T.AssertEquals(50, Big, "big share 1: always big");
	Tuning.FreshDepositBigShare = 0.5f;
	Tuning.FreshDepositAlongMin = 0.25f;
	Tuning.FreshDepositAlongMax = 0.25f;
	T.AssertTrue(HOLVolcanoRules::FreshDeposit(SiteSeedValue, 0, Flow, Tuning).Location.Equals(HOLVolcanoRules::PointAt(Flow, 0.25f * Length), 1.0), "a range reduced to one fraction puts it there");
	Tuning.FreshDepositAlongMin = 0.3f;
	Tuning.FreshDepositAlongMax = 0.7f;

	int Gold = 0;
	for (int Cycle = 0; Cycle < 200; Cycle++)
		if (HOLVolcanoRules::FreshDeposit(SiteSeedValue, Cycle, Flow, Tuning).Kind == 0)
			Gold++;
	T.AssertTrue(Gold > 70 && Gold < 130, "share 0.5: both kinds come, about half gold over 200 cycles");
	Tuning.FreshDepositGoldShare = 0.f;
	Gold = 0;
	for (int Cycle = 0; Cycle < 50; Cycle++)
		if (HOLVolcanoRules::FreshDeposit(SiteSeedValue, Cycle, Flow, Tuning).Kind == 0)
			Gold++;
	T.AssertEquals(0, Gold, "share 0: always stone");
	Tuning.FreshDepositGoldShare = 1.f;
	for (int Cycle = 0; Cycle < 50; Cycle++)
		if (HOLVolcanoRules::FreshDeposit(SiteSeedValue, Cycle, Flow, Tuning).Kind == 0)
			Gold++;
	T.AssertEquals(50, Gold, "share 1: always gold");
	Tuning.FreshDepositGoldShare = 0.5f;

	int Differ = 0;
	for (int Cycle = 0; Cycle < 40; Cycle++)
		if (HOLVolcanoRules::FreshDeposit(HOLVolcanoRules::SiteSeed(4242, 0), Cycle, Flow, Tuning).Kind != HOLVolcanoRules::FreshDeposit(HOLVolcanoRules::SiteSeed(4242, 1), Cycle, Flow, Tuning).Kind)
			Differ++;
	T.AssertTrue(Differ > 5, "two sites of a match draw their own kinds");

	TArray<FVector> Degenerate;
	Degenerate.Add(FVector(100, 100, 0));
	Degenerate.Add(FVector(100, 100, 0));
	const TArray<FVector> NoPoint;
	T.AssertFalse(HOLVolcanoRules::FreshDeposit(SiteSeedValue, 0, Degenerate, Tuning).bPresent, "a flow without length leaves nothing");
	T.AssertFalse(HOLVolcanoRules::FreshDeposit(SiteSeedValue, 0, NoPoint, Tuning).bPresent, "... nor a flow without point");

	FHOLVolcanoSiteLayout Site;
	FHOLVolcanoFlowLine Line;
	Line.Points = Flow;
	Site.Flows.Add(Line);
	int RestSteps = 0;
	bool bAllMatch = true;
	for (float Now = 0.f; Now < 3000.f; Now += 1.f)
	{
		const FHOLVolcanoStateView State = HOLVolcanoRules::SiteStateAt(true, 4242, 0, Site, Now, Tuning, 697.f, -1, 0.f);
		const bool bRest = State.Phase == 5;
		if (bRest)
			RestSteps++;
		if (HOLVolcanoRules::FreshDepositDue(State.Phase, State.FlowIndex) != bRest)
			bAllMatch = false;
	}
	T.AssertTrue(bAllMatch && RestSteps > 300, "the deposit is due in every second of the rest, and only then");
	const FHOLVolcanoStateView Dormant = HOLVolcanoRules::SiteStateAt(false, 4242, 0, Site, 1500.f, Tuning, 697.f, -1, 0.f);
	T.AssertFalse(HOLVolcanoRules::FreshDepositDue(Dormant.Phase, Dormant.FlowIndex), "a dormant site (no Catastrophe) leaves nothing");
	T.AssertFalse(HOLVolcanoRules::FreshDepositDue(5, -1), "a rest without a drawn flow leaves nothing");
}
