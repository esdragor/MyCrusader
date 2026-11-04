// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestHooks.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLVolcanoRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// The volcano site's cycle (V1, GDD 12, D28, D398, D400): a pure function of the time and a seed.
namespace UE::HeroesOfLiberty::Tests
{
	constexpr EAutomationTestFlags VolcanoFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoStandardTest, "HeroesOfLiberty.Volcano.StandardStaysDormant", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoStandardTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	bool bAlwaysDormant = true;
	for (float Now = 0.f; Now <= 3600.f; Now += 5.f)
	{
		bAlwaysDormant &= HOLVolcano::GetPhase(false, 7, Now, Settings) == EHOLVolcanoPhase::Dormant;
	}
	TestTrue(TEXT("without Catastrophe the site never wakes in 60 min (D398)"), bAlwaysDormant);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoCycleTest, "HeroesOfLiberty.Volcano.CatastropheCycle", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoCycleTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	for (int32 Seed = 1; Seed <= 20; ++Seed)
	{
		const HOLVolcano::FCycle First = HOLVolcano::MakeCycle(Seed, 0, 0.f, Settings);
		const FString Tag = FString::Printf(TEXT("seed %d"), Seed);
		TestTrue(Tag + TEXT(": the alert starts inside the window"),
			First.WarningAt >= Settings.WindowOpensAfter && First.WarningAt <= Settings.WindowOpensAfter + Settings.WindowLength);
		const float Alert = First.EruptAt - First.WarningAt;
		TestTrue(Tag + TEXT(": alert 60 to 90 s (D28)"), Alert >= 60.f && Alert <= 90.f);
		const float Cooling = First.CooledAt - First.EruptEndAt;
		TestTrue(Tag + TEXT(": cooling 2 to 3 min (D379)"), Cooling >= 120.f && Cooling <= 180.f);
		TestTrue(Tag + TEXT(": rest 6 to 8 min, then the next window (D400)"),
			First.ReArmAt - First.CooledAt >= 360.f && First.ReArmAt - First.CooledAt <= 480.f);

		TestTrue(Tag + TEXT(": armed before the alert"), HOLVolcano::GetPhase(true, Seed, 1.f, Settings) == EHOLVolcanoPhase::Armed);
		TestTrue(Tag + TEXT(": warning"), HOLVolcano::GetPhase(true, Seed, First.WarningAt + 1.f, Settings) == EHOLVolcanoPhase::Warning);
		TestTrue(Tag + TEXT(": erupting"), HOLVolcano::GetPhase(true, Seed, First.EruptAt + 1.f, Settings) == EHOLVolcanoPhase::Erupting);
		TestTrue(Tag + TEXT(": cooling"), HOLVolcano::GetPhase(true, Seed, First.EruptEndAt + 1.f, Settings) == EHOLVolcanoPhase::Cooling);
		TestTrue(Tag + TEXT(": resting on basalt"), HOLVolcano::GetPhase(true, Seed, First.CooledAt + 1.f, Settings) == EHOLVolcanoPhase::Rest);
		TestTrue(Tag + TEXT(": armed again"), HOLVolcano::GetPhase(true, Seed, First.ReArmAt + 1.f, Settings) == EHOLVolcanoPhase::Armed);
		TestTrue(Tag + TEXT(": same seed, same dates"), HOLVolcano::MakeCycle(Seed, 0, 0.f, Settings).EruptAt == First.EruptAt);
	}
	return true;
}

// The lava on a flow (V2, D377, D575): the front's place is a function of the time, the flow and the speed.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoLavaSpeedTest, "HeroesOfLiberty.Volcano.LavaSpeed", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoLavaSpeedTest::RunTest(const FString& Parameters)
{
	// The real chain: the Man-at-arms' speed read from the unit type table, less 30 % (V-Q4 = A, D575), about 490 cm/s.
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* ManAtArms = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("ManAtArms"), TEXT("Volcano test"), false) : nullptr;
	if (!TestNotNull(TEXT("the table has a Man-at-arms"), ManAtArms))
	{
		return false;
	}
	const FHOLVolcanoSettings Settings;
	const float Speed = Settings.LavaSpeed(ManAtArms->Speed);
	TestEqual(TEXT("the lava walks at 70 % of the Man-at-arms' speed"), Speed, ManAtArms->Speed * 0.7f, 0.01f);
	TestTrue(TEXT("about 490 cm/s"), Speed > 480.f && Speed < 495.f);
	TestTrue(TEXT("slower than the Man-at-arms, so a unit that runs escapes it"), Speed < ManAtArms->Speed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoFlowTest, "HeroesOfLiberty.Volcano.FlowFront", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoFlowTest::RunTest(const FString& Parameters)
{
	// An L-shaped flow: 3000 cm east then 4000 cm north, 7000 cm in all.
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(3000.0, 0.0, 50.0), FVector(3000.0, 4000.0, 0.0) };
	const float Speed = 488.f;
	const float EruptAt = 500.f;
	TestEqual(TEXT("length: the legs added (Z ignored)"), Flow.Length(), 7000.f, 0.01f);
	const float Duration = HOLVolcano::EruptionDuration(Flow, Speed);
	TestEqual(TEXT("it reaches the end after length over speed"), Duration, 7000.f / Speed, 0.001f);

	TestEqual(TEXT("not yet erupting: the front is at the crater"), HOLVolcano::FrontDistance(Flow, EruptAt - 10.f, EruptAt, Speed), 0.f);
	TestEqual(TEXT("at the first instant: still at the crater"), HOLVolcano::FrontDistance(Flow, EruptAt, EruptAt, Speed), 0.f);
	TestEqual(TEXT("5 s later: 5 x speed"), HOLVolcano::FrontDistance(Flow, EruptAt + 5.f, EruptAt, Speed), 5.f * Speed, 0.01f);
	TestEqual(TEXT("at the end instant: the full length"), HOLVolcano::FrontDistance(Flow, EruptAt + Duration, EruptAt, Speed), 7000.f, 0.5f);
	TestEqual(TEXT("long after: it stays at the end"), HOLVolcano::FrontDistance(Flow, EruptAt + 1000.f, EruptAt, Speed), 7000.f);
	TestTrue(TEXT("one second before the end it has not arrived"), HOLVolcano::FrontDistance(Flow, EruptAt + Duration - 1.f, EruptAt, Speed) < 7000.f);

	// The front never goes back, whatever the instants asked in (a front read every 0.1 s).
	float Last = 0.f;
	bool bNeverBack = true;
	for (float Now = 400.f; Now < 700.f; Now += 0.1f)
	{
		const float Distance = HOLVolcano::FrontDistance(Flow, Now, EruptAt, Speed);
		bNeverBack &= Distance >= Last;
		Last = Distance;
	}
	TestTrue(TEXT("the front never goes backward"), bNeverBack);

	// Places along the polyline.
	TestTrue(TEXT("at 0: the crater"), HOLVolcano::PointAt(Flow, 0.f).Equals(FVector(0.0, 0.0, 0.0), 0.01));
	TestTrue(TEXT("at 1500: halfway along the first leg"), HOLVolcano::PointAt(Flow, 1500.f).Equals(FVector(1500.0, 0.0, 25.0), 0.01));
	TestTrue(TEXT("at 3000: the corner"), HOLVolcano::PointAt(Flow, 3000.f).Equals(FVector(3000.0, 0.0, 50.0), 0.01));
	TestTrue(TEXT("at 5000: 2000 up the second leg"), HOLVolcano::PointAt(Flow, 5000.f).Equals(FVector(3000.0, 2000.0, 25.0), 0.01));
	TestTrue(TEXT("beyond the end: the end"), HOLVolcano::PointAt(Flow, 99999.f).Equals(FVector(3000.0, 4000.0, 0.0), 0.01));
	TestTrue(TEXT("before the start: the crater"), HOLVolcano::PointAt(Flow, -5.f).Equals(FVector(0.0, 0.0, 0.0), 0.01));

	// Degenerate flows do not break anything.
	const HOLVolcano::FFlow Empty;
	TestEqual(TEXT("no point: no length"), Empty.Length(), 0.f);
	TestEqual(TEXT("no length: no duration"), HOLVolcano::EruptionDuration(Empty, Speed), 0.f);
	TestEqual(TEXT("no length: the front stays at 0"), HOLVolcano::FrontDistance(Empty, EruptAt + 10.f, EruptAt, Speed), 0.f);
	TestEqual(TEXT("no speed: no duration"), HOLVolcano::EruptionDuration(Flow, 0.f), 0.f);
	TestEqual(TEXT("no speed: the front does not move"), HOLVolcano::FrontDistance(Flow, EruptAt + 10.f, EruptAt, 0.f), 0.f);
	TestTrue(TEXT("no point: the origin"), HOLVolcano::PointAt(Empty, 10.f).IsZero());
	return true;
}

// The lava's reach (V5, GDD 12.2, D377): what is under a flow whose front has gone some way is destroyed.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoReachTest, "HeroesOfLiberty.Volcano.LavaReach", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoReachTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	const double H = Settings.LavaHalfWidth;
	TestEqual(TEXT("a flow is 16 m wide: half-width 800 cm (V5-Q1 = B, the user)"), Settings.LavaHalfWidth, 800.f);
	// An L-shaped flow: 3000 cm east then 4000 cm north.
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(3000.0, 0.0, 50.0), FVector(3000.0, 4000.0, 0.0) };

	// Before the eruption (front at the crater) nothing is burnt, not even the crater.
	TestFalse(TEXT("front at 0: the crater itself is safe"), HOLVolcano::IsInLava(Flow, 0.f, Settings.LavaHalfWidth, FVector(0.0, 0.0, 0.0)));
	// Front at 2000 (first leg).
	const float Front = 2000.f;
	const float W = Settings.LavaHalfWidth;
	TestTrue(TEXT("on the line behind the front: burnt"), HOLVolcano::IsInLava(Flow, Front, W, FVector(1000.0, 0.0, 0.0)));
	TestTrue(TEXT("on the crater once it erupts: burnt"), HOLVolcano::IsInLava(Flow, Front, W, FVector(0.0, 0.0, 0.0)));
	TestTrue(TEXT("just inside the width: burnt"), HOLVolcano::IsInLava(Flow, Front, W, FVector(1000.0, H - 10.0, 0.0)));
	TestFalse(TEXT("just outside the width: safe"), HOLVolcano::IsInLava(Flow, Front, W, FVector(1000.0, H + 10.0, 0.0)));
	TestFalse(TEXT("outside the width on the other side: safe"), HOLVolcano::IsInLava(Flow, Front, W, FVector(1000.0, -(H + 10.0), 0.0)));
	TestTrue(TEXT("a body's radius counts: outside by 10 with radius 50 touches it"), HOLVolcano::IsInLava(Flow, Front, W, FVector(1000.0, H + 10.0, 0.0), 50.f));
	TestFalse(TEXT("a body far enough even with its radius: safe"), HOLVolcano::IsInLava(Flow, Front, W, FVector(1000.0, H + 100.0, 0.0), 50.f));
	TestTrue(TEXT("the head of the front is rounded: a little ahead of it, burnt"), HOLVolcano::IsInLava(Flow, Front, W, FVector(Front + H - 10.0, 0.0, 0.0)));
	TestFalse(TEXT("well ahead of the front: safe, the lava has not come yet"), HOLVolcano::IsInLava(Flow, Front, W, FVector(Front + H + 10.0, 0.0, 0.0)));
	TestFalse(TEXT("ahead on the flow's second leg: safe"), HOLVolcano::IsInLava(Flow, Front, W, FVector(3000.0, 2000.0, 0.0)));
	TestTrue(TEXT("Z is ignored: far above the line but inside its width, burnt"), HOLVolcano::IsInLava(Flow, Front, W, FVector(1000.0, 0.0, 99999.0)));

	// The front has turned the corner (3000 + 2000 up the second leg).
	const float Turned = 5000.f;
	TestTrue(TEXT("second leg, behind the front: burnt"), HOLVolcano::IsInLava(Flow, Turned, W, FVector(3000.0, 1500.0, 0.0)));
	TestTrue(TEXT("second leg, beside the line: burnt"), HOLVolcano::IsInLava(Flow, Turned, W, FVector(3000.0 + H - 10.0, 1500.0, 0.0)));
	TestFalse(TEXT("second leg, beyond the front: safe"), HOLVolcano::IsInLava(Flow, Turned, W, FVector(3000.0, 3500.0, 0.0)));
	TestFalse(TEXT("away from both legs: safe"), HOLVolcano::IsInLava(Flow, Turned, W, FVector(0.0, 3000.0, 0.0)));

	// Once the lava has arrived, the whole flow is burnt, its far end too.
	TestTrue(TEXT("front at the end: the far end is burnt"), HOLVolcano::IsInLava(Flow, Flow.Length(), W, FVector(3000.0, 4000.0, 0.0)));

	// A flow that is not one burns nothing.
	const HOLVolcano::FFlow Empty;
	TestFalse(TEXT("no point: no lava"), HOLVolcano::IsInLava(Empty, 100.f, W, FVector::ZeroVector));
	HOLVolcano::FFlow OnePoint;
	OnePoint.Points = { FVector::ZeroVector };
	TestFalse(TEXT("one point: no lava"), HOLVolcano::IsInLava(OnePoint, 100.f, W, FVector::ZeroVector));

	// What the lava destroys: all but a hero in a duel (GDD 9.8, 10.2).
	TestTrue(TEXT("a unit is destroyed"), HOLVolcano::LavaDestroys(false, false));
	TestTrue(TEXT("a hero outside a duel is destroyed"), HOLVolcano::LavaDestroys(true, false));
	TestFalse(TEXT("a hero in a duel is not"), HOLVolcano::LavaDestroys(true, true));
	return true;
}
// The ground a cooled flow forbids (V6, D379): boxes that cover the whole lava reach and not much more.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoObstacleTest, "HeroesOfLiberty.Volcano.ObstacleBoxes", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoObstacleTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	const float H = Settings.LavaHalfWidth;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(3000.0, 0.0, 50.0), FVector(3000.0, 4000.0, 0.0) };
	const TArray<HOLVolcano::FObstacleBox> Boxes = HOLVolcano::GetObstacleBoxes(Flow, H);
	if (!TestEqual(TEXT("one box per segment"), Boxes.Num(), 2))
	{
		return false;
	}
	TestEqual(TEXT("first box: centre"), Boxes[0].Centre.X, 1500.0, 0.01);
	TestEqual(TEXT("first box: half length = half the segment + the half width at each end"), Boxes[0].HalfLength, 1500.f + H, 0.01f);
	TestEqual(TEXT("first box: half width"), Boxes[0].HalfWidth, H, 0.01f);
	TestEqual(TEXT("first box: along +X"), Boxes[0].YawDegrees, 0.f, 0.01f);
	TestEqual(TEXT("second box: along +Y"), Boxes[1].YawDegrees, 90.f, 0.01f);
	TestEqual(TEXT("second box: centre"), Boxes[1].Centre.Y, 2000.0, 0.01);

	const auto InBox = [](const HOLVolcano::FObstacleBox& Box, const FVector2D& P)
	{
		const FVector2D Local = (P - Box.Centre).GetRotated(-Box.YawDegrees);
		return FMath::Abs(Local.X) <= Box.HalfLength + 0.01 && FMath::Abs(Local.Y) <= Box.HalfWidth + 0.01;
	};
	int32 Reach = 0;
	int32 Missed = 0;
	int32 Far = 0;
	int32 FarInBox = 0;
	for (double X = -1500.0; X <= 4500.0; X += 50.0)
	{
		for (double Y = -1500.0; Y <= 5500.0; Y += 50.0)
		{
			const FVector2D P(X, Y);
			const bool bInBoxes = InBox(Boxes[0], P) || InBox(Boxes[1], P);
			if (HOLVolcano::IsInLava(Flow, Flow.Length(), H, FVector(X, Y, 0.0)))
			{
				++Reach;
				Missed += bInBoxes ? 0 : 1;
			}
			else if (!HOLVolcano::IsInLava(Flow, Flow.Length(), 2.f * H, FVector(X, Y, 0.0)))
			{
				++Far;
				FarInBox += bInBoxes ? 1 : 0;
			}
		}
	}
	TestTrue(TEXT("the sample has points under the lava and far from it"), Reach > 1000 && Far > 1000);
	TestEqual(TEXT("every point the lava reaches is in the boxes"), Missed, 0);
	TestEqual(TEXT("no point farther than twice the half width is"), FarInBox, 0);

	TestEqual(TEXT("no flow, no box"), HOLVolcano::GetObstacleBoxes(HOLVolcano::FFlow(), H).Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoEruptionLengthTest, "HeroesOfLiberty.Volcano.EruptionFollowsTheFlow", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoEruptionLengthTest::RunTest(const FString& Parameters)
{
	// The eruption lasts as long as the lava needs to reach the end of its flow: the cycle's dates follow the flow.
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(6000.0, 0.0, 0.0) };
	const FHOLVolcanoSettings Settings;
	const float Duration = HOLVolcano::EruptionDuration(Flow, 488.f);
	for (int32 Seed = 1; Seed <= 10; ++Seed)
	{
		const HOLVolcano::FCycle Cycle = HOLVolcano::MakeCycle(Seed, 0, 0.f, Settings, Duration);
		const FString Tag = FString::Printf(TEXT("seed %d"), Seed);
		TestEqual(Tag + TEXT(": the eruption lasts the flow's duration"), Cycle.EruptEndAt - Cycle.EruptAt, Duration, 0.001f);
		TestTrue(Tag + TEXT(": erupting just before the lava arrives"),
			HOLVolcano::GetPhase(true, Seed, Cycle.EruptEndAt - 0.5f, Settings, Duration) == EHOLVolcanoPhase::Erupting);
		TestTrue(Tag + TEXT(": cooling once it has"),
			HOLVolcano::GetPhase(true, Seed, Cycle.EruptEndAt + 0.5f, Settings, Duration) == EHOLVolcanoPhase::Cooling);
		TestEqual(Tag + TEXT(": the alert and the eruption's start do not move with the flow"),
			Cycle.EruptAt, HOLVolcano::MakeCycle(Seed, 0, 0.f, Settings).EruptAt);
	}
	return true;
}

// The direction of an eruption (V3, D379, V-Q2 = B): drawn freely among the site's flows, no memory, no side favoured.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoDrawFlowTest, "HeroesOfLiberty.Volcano.DrawFlow", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoDrawFlowTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("no flow: nothing to draw"), HOLVolcano::DrawFlow(3, 0, 0), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("one flow: always that one"), HOLVolcano::DrawFlow(3, 5, 1), 0);

	// Four flows (two symmetrical pairs, say): 4000 draws over many seeds and cycles, each flow about a quarter of them.
	const int32 FlowCount = 4;
	int32 Counts[FlowCount] = {};
	int32 Repeats = 0;
	int32 Pairs = 0;
	bool bInRange = true;
	bool bStable = true;
	for (int32 Seed = 1; Seed <= 1000; ++Seed)
	{
		int32 Previous = INDEX_NONE;
		for (int32 Index = 0; Index < 4; ++Index)
		{
			const int32 Flow = HOLVolcano::DrawFlow(Seed, Index, FlowCount);
			bInRange &= Flow >= 0 && Flow < FlowCount;
			bStable &= Flow == HOLVolcano::DrawFlow(Seed, Index, FlowCount);
			if (Flow >= 0 && Flow < FlowCount)
			{
				++Counts[Flow];
			}
			if (Previous != INDEX_NONE)
			{
				++Pairs;
				Repeats += Flow == Previous ? 1 : 0;
			}
			Previous = Flow;
		}
	}
	TestTrue(TEXT("always one of the site's flows"), bInRange);
	TestTrue(TEXT("same seed and cycle, same flow (every machine agrees)"), bStable);
	for (int32 Flow = 0; Flow < FlowCount; ++Flow)
	{
		TestTrue(FString::Printf(TEXT("flow %d is drawn about a quarter of the time (got %d of 4000)"), Flow, Counts[Flow]),
			Counts[Flow] > 850 && Counts[Flow] < 1150);
	}
	// V-Q2 = B: no memory, so the same flow twice in a row happens about once in four.
	TestTrue(FString::Printf(TEXT("no memory: the same flow twice in a row happens (%d of %d pairs)"), Repeats, Pairs),
		Repeats > Pairs * 15 / 100 && Repeats < Pairs * 35 / 100);

	// The draw does not move the cycle's dates (a stream of its own).
	const FHOLVolcanoSettings Settings;
	const HOLVolcano::FCycle Before = HOLVolcano::MakeCycle(9, 1, 100.f, Settings);
	HOLVolcano::DrawFlow(9, 1, FlowCount);
	TestEqual(TEXT("the dates of a cycle do not depend on the draw"), HOLVolcano::MakeCycle(9, 1, 100.f, Settings).EruptAt, Before.EruptAt);
	return true;
}

// The whole state of a site at an instant (V4): flows drawn per cycle, lava per flow, no replication but the clock.
namespace UE::HeroesOfLiberty::Tests
{
	/** The Man-at-arms' speed from the unit type table (the real chain), 0 if the table has none. */
	float ManAtArmsSpeedFromTable()
	{
		const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
		const FHOLUnitTypeRow* ManAtArms = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("ManAtArms"), TEXT("Volcano test"), false) : nullptr;
		return ManAtArms ? ManAtArms->Speed : 0.f;
	}

	/** A site with three straight flows from the crater (0, 0): east 4000 cm, north 7000 cm, west 9000 cm. */
	HOLVolcano::FSite MakeThreeFlowSite()
	{
		HOLVolcano::FSite Site;
		const FVector Ends[3] = { FVector(4000.0, 0.0, 0.0), FVector(0.0, 7000.0, 0.0), FVector(-9000.0, 0.0, 0.0) };
		for (const FVector& End : Ends)
		{
			HOLVolcano::FFlow Flow;
			Flow.Points = { FVector::ZeroVector, End };
			Site.Flows.Add(Flow);
		}
		return Site;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoSiteStateTest, "HeroesOfLiberty.Volcano.SiteState", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoSiteStateTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests;
	const float ManSpeed = ManAtArmsSpeedFromTable();
	if (!TestTrue(TEXT("the table gives a Man-at-arms' speed"), ManSpeed > 0.f))
	{
		return false;
	}
	const FHOLVolcanoSettings Settings;
	const float Speed = Settings.LavaSpeed(ManSpeed);
	const HOLVolcano::FSite Site = MakeThreeFlowSite();

	// Without Catastrophe the site never moves (D398), whatever the time.
	bool bDormant = true;
	for (float Now = 0.f; Now <= 3600.f; Now += 7.f)
	{
		const HOLVolcano::FSiteState State = HOLVolcano::SiteStateAt(false, 5, 0, Site, Now, Settings, ManSpeed);
		bDormant &= State.Phase == EHOLVolcanoPhase::Dormant && State.FlowIndex == INDEX_NONE && State.FrontDistance == 0.f;
	}
	TestTrue(TEXT("no Catastrophe: dormant for an hour, no flow, no front"), bDormant);

	for (int32 MatchSeed = 1; MatchSeed <= 12; ++MatchSeed)
	{
		const FString Tag = FString::Printf(TEXT("seed %d"), MatchSeed);
		const int32 SeedOfSite = HOLVolcano::SiteSeed(MatchSeed, 0);
		const int32 Flow0 = HOLVolcano::DrawFlow(SeedOfSite, 0, Site.Flows.Num());
		const float Length0 = HOLVolcano::EruptionDuration(Site.Flows[Flow0], Speed);
		const HOLVolcano::FCycle Cycle0 = HOLVolcano::MakeCycle(SeedOfSite, 0, 0.f, Settings, Length0);

		const HOLVolcano::FSiteState Armed = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, 1.f, Settings, ManSpeed);
		TestTrue(Tag + TEXT(": armed at the start, cycle 0, the flow of the draw"), Armed.Phase == EHOLVolcanoPhase::Armed && Armed.CycleIndex == 0 && Armed.FlowIndex == Flow0);

		const HOLVolcano::FSiteState Warning = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.WarningAt + 5.f, Settings, ManSpeed);
		TestTrue(Tag + TEXT(": alert"), Warning.Phase == EHOLVolcanoPhase::Warning && Warning.FlowIndex == Flow0);
		TestEqual(Tag + TEXT(": the countdown to the lava"), Warning.SecondsToErupt, Cycle0.EruptAt - (Cycle0.WarningAt + 5.f), 0.01f);
		TestEqual(Tag + TEXT(": the front waits at the crater"), Warning.FrontDistance, 0.f);

		const float Probe = Length0 * 0.5f; // the shortest flow lasts 8 s: half of the drawn flow's time, whichever it is
		const HOLVolcano::FSiteState Erupting = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.EruptAt + Probe, Settings, ManSpeed);
		TestTrue(Tag + TEXT(": erupting"), Erupting.Phase == EHOLVolcanoPhase::Erupting);
		TestEqual(Tag + TEXT(": half the flow's time of lava"), Erupting.FrontDistance, Probe * Speed, 0.5f);
		TestTrue(Tag + TEXT(": the front point is on the drawn flow"), Erupting.FrontPoint.Equals(HOLVolcano::PointAt(Site.Flows[Flow0], Probe * Speed), 1.0));
		TestEqual(Tag + TEXT(": no countdown once it flows"), Erupting.SecondsToErupt, -1.f);

		const HOLVolcano::FSiteState Cooling = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.EruptEndAt + 1.f, Settings, ManSpeed);
		TestTrue(Tag + TEXT(": cooling, the lava at the end of its flow"), Cooling.Phase == EHOLVolcanoPhase::Cooling && FMath::IsNearlyEqual(Cooling.FrontDistance, Site.Flows[Flow0].Length(), 0.5f));

		const HOLVolcano::FSiteState Rest = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.CooledAt + 1.f, Settings, ManSpeed);
		TestTrue(Tag + TEXT(": basalt, resting"), Rest.Phase == EHOLVolcanoPhase::Rest);

		// The next cycle: its own flow, its window after the first one re-armed.
		const int32 Flow1 = HOLVolcano::DrawFlow(SeedOfSite, 1, Site.Flows.Num());
		const HOLVolcano::FCycle Cycle1 = HOLVolcano::MakeCycle(SeedOfSite, 1, Cycle0.ReArmAt, Settings, HOLVolcano::EruptionDuration(Site.Flows[Flow1], Speed));
		const HOLVolcano::FSiteState Next = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Cycle1.EruptAt + 3.f, Settings, ManSpeed);
		TestTrue(Tag + TEXT(": the second eruption, cycle 1, its own flow"), Next.Phase == EHOLVolcanoPhase::Erupting && Next.CycleIndex == 1 && Next.FlowIndex == Flow1);
		TestTrue(Tag + TEXT(": the second window opens after the first rest"), Cycle1.WarningAt > Cycle0.ReArmAt);

		// The phase agrees with the single-cycle function where the flow is the same length for all cycles.
		TestTrue(Tag + TEXT(": same machine, same state"), HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Cycle0.EruptAt + Probe, Settings, ManSpeed).FrontPoint.Equals(Erupting.FrontPoint, 0.001));
	}

	// Two sites of one match do not erupt on the same dates (their seeds differ), and the seed depends on both numbers.
	int32 Different = 0;
	for (int32 MatchSeed = 1; MatchSeed <= 12; ++MatchSeed)
	{
		Different += HOLVolcano::SiteSeed(MatchSeed, 0) != HOLVolcano::SiteSeed(MatchSeed, 1) ? 1 : 0;
	}
	TestEqual(TEXT("the seed of site 1 is never site 0's"), Different, 12);
	TestTrue(TEXT("another match, another seed"), HOLVolcano::SiteSeed(1, 0) != HOLVolcano::SiteSeed(2, 0));

	// A site without any flow still goes through its cycle (the placeholder length), with nothing to draw.
	const HOLVolcano::FSite Bare;
	const HOLVolcano::FCycle BareCycle = HOLVolcano::MakeCycle(HOLVolcano::SiteSeed(3, 0), 0, 0.f, Settings);
	const HOLVolcano::FSiteState BareState = HOLVolcano::SiteStateAt(true, 3, 0, Bare, BareCycle.EruptAt + 5.f, Settings, ManSpeed);
	TestTrue(TEXT("no flow: erupting by the placeholder, nothing drawn, the front stays"), BareState.Phase == EHOLVolcanoPhase::Erupting && BareState.FlowIndex == INDEX_NONE && BareState.FrontDistance == 0.f);
	return true;
}

// V4c (V4-Q2 = C, D608): the seed of the match makes the eruption. A scenario's .json imposes it ("seed"), the real chain reads it
// (MakeScenarioConfig, GetScenarioSeed), and the site's state follows from the configuration and the clock alone: the same seed gives the same
// eruption on any launch, another seed another one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoSeedTest, "HeroesOfLiberty.Volcano.MatchSeed", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoSeedTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests;
	const float ManSpeed = ManAtArmsSpeedFromTable();
	if (!TestTrue(TEXT("the table gives a Man-at-arms' speed"), ManSpeed > 0.f))
	{
		return false;
	}
	const FHOLVolcanoSettings Settings;
	const HOLVolcano::FSite Site = MakeThreeFlowSite();

	// The scenario's .json, by the real chain: with a seed (imposed), without (drawn).
	const auto ConfigOf = [this](const FString& Json, const int32 Drawn, FHOLMatchConfig& Out)
	{
		if (!TestTrue(TEXT(".json read"), HOLTestHooks::MakeScenarioConfig(&Json, Out)))
		{
			return false;
		}
		Out.Seed = HOLTestHooks::GetScenarioSeed(&Json, Drawn);
		return true;
	};
	FHOLMatchConfig First, Again, Other, Drawn;
	if (!ConfigOf(TEXT("{ \"seed\": 4242, \"bCatastrophe\": true }"), 111, First)
		|| !ConfigOf(TEXT("{ \"seed\": 4242, \"bCatastrophe\": true }"), 987654, Again)
		|| !ConfigOf(TEXT("{ \"seed\": 4243, \"bCatastrophe\": true }"), 111, Other)
		|| !ConfigOf(TEXT("{ \"bCatastrophe\": true }"), 31337, Drawn))
	{
		return false;
	}
	TestEqual(TEXT("the .json's seed is the match's, whatever the draw"), First.Seed, 4242);
	TestEqual(TEXT("again, another launch"), Again.Seed, 4242);
	TestEqual(TEXT("another seed"), Other.Seed, 4243);
	TestEqual(TEXT("no seed in the .json: the drawn one"), Drawn.Seed, 31337);
	TestTrue(TEXT("the Catastrophe option comes from the .json"), First.bCatastrophe);

	// The same configuration gives the same eruption at every instant; the state is the one the seed makes.
	const HOLVolcano::FCycle Cycle0 = HOLVolcano::MakeCycle(HOLVolcano::SiteSeed(4242, 0), 0, 0.f, Settings,
		HOLVolcano::EruptionDuration(Site.Flows[HOLVolcano::DrawFlow(HOLVolcano::SiteSeed(4242, 0), 0, Site.Flows.Num())], Settings.LavaSpeed(ManSpeed)));
	int32 SameCount = 0;
	for (float Now = 0.f; Now <= 1800.f; Now += 9.f)
	{
		const HOLVolcano::FSiteState A = HOLVolcano::SiteStateOfMatch(First, 0, Site, Now, Settings, ManSpeed);
		const HOLVolcano::FSiteState B = HOLVolcano::SiteStateOfMatch(Again, 0, Site, Now, Settings, ManSpeed);
		const HOLVolcano::FSiteState Truth = HOLVolcano::SiteStateAt(true, 4242, 0, Site, Now, Settings, ManSpeed);
		SameCount += (A.Phase == B.Phase && A.CycleIndex == B.CycleIndex && A.FlowIndex == B.FlowIndex && A.FrontPoint.Equals(B.FrontPoint, 0.001)
			&& A.Phase == Truth.Phase && A.FlowIndex == Truth.FlowIndex && A.FrontPoint.Equals(Truth.FrontPoint, 0.001)) ? 1 : 0;
	}
	TestEqual(TEXT("same seed, same eruption at each of the 201 instants"), SameCount, 201);
	const HOLVolcano::FSiteState Erupting = HOLVolcano::SiteStateOfMatch(First, 0, Site, Cycle0.EruptAt + 4.f, Settings, ManSpeed);
	TestTrue(TEXT("the eruption of the seed's first cycle is on at its date"), Erupting.Phase == EHOLVolcanoPhase::Erupting && Erupting.CycleIndex == 0);

	// Another seed does not give the same eruption: over the whole hour, some instant tells them apart.
	bool bDiffers = false;
	for (float Now = 0.f; Now <= 3600.f && !bDiffers; Now += 3.f)
	{
		const HOLVolcano::FSiteState A = HOLVolcano::SiteStateOfMatch(First, 0, Site, Now, Settings, ManSpeed);
		const HOLVolcano::FSiteState C = HOLVolcano::SiteStateOfMatch(Other, 0, Site, Now, Settings, ManSpeed);
		bDiffers = A.Phase != C.Phase || A.FlowIndex != C.FlowIndex;
	}
	TestTrue(TEXT("another seed, another eruption"), bDiffers);

	// Without the Catastrophe option the site sleeps, the seed or not (D398).
	FHOLMatchConfig Standard = First;
	Standard.bCatastrophe = false;
	TestTrue(TEXT("no Catastrophe: dormant"), HOLVolcano::SiteStateOfMatch(Standard, 0, Site, Cycle0.EruptAt + 4.f, Settings, ManSpeed).Phase == EHOLVolcanoPhase::Dormant);
	return true;
}

// The AI's way out (V11, D427, V5-Q1 = B): the corridor is the flow's half width plus the body's radius, the whole length of the flow.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoEvacuationTest, "HeroesOfLiberty.Volcano.EvacuationPoint", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoEvacuationTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	const float W = Settings.LavaHalfWidth;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(8000.0, 0.0, 0.0) };

	TestEqual(TEXT("the reach is the half width plus the radius"), HOLVolcano::EvacuationReach(W, 60.f), W + 60.f);
	// The reach follows the settings (never typed): a narrower flow, a shorter reach.
	TestEqual(TEXT("... of the setting, whatever it is"), HOLVolcano::EvacuationReach(300.f, 60.f), 360.f);

	// A body that the old 8 m flow (half width 400) spared is now caught, along the whole corridor even before the front moves.
	const FVector Beside(4000.0, 700.0, 0.0);
	TestTrue(TEXT("700 cm aside, radius 60: inside the 16 m corridor"), HOLVolcano::IsInCorridor(Flow, W, Beside, 60.f));
	TestFalse(TEXT("... but outside an 8 m corridor"), HOLVolcano::IsInCorridor(Flow, 400.f, Beside, 60.f));
	TestFalse(TEXT("the radius counts: 800 + 60 + 1 cm aside is out"), HOLVolcano::IsInCorridor(Flow, W, FVector(4000.0, W + 61.0, 0.0), 60.f));
	TestTrue(TEXT("... 800 + 60 - 1 cm is in"), HOLVolcano::IsInCorridor(Flow, W, FVector(4000.0, W + 59.0, 0.0), 60.f));
	TestFalse(TEXT("far from the line: out"), HOLVolcano::IsInCorridor(Flow, W, FVector(4000.0, 3000.0, 0.0), 60.f));

	// The way out: out of the corridor with a margin, on the side it stood on, at its height.
	const FVector Out = HOLVolcano::EvacuationPoint(Flow, W, FVector(4000.0, 700.0, 25.0), 60.f);
	TestFalse(TEXT("the point is out of the corridor"), HOLVolcano::IsInCorridor(Flow, W, Out, 60.f));
	TestTrue(TEXT("... on the same side"), Out.Y > 0.0);
	TestTrue(TEXT("... with a margin beyond the reach"), Out.Y >= W + 60.f + 199.f);
	TestTrue(TEXT("... straight away from the line"), FMath::Abs(Out.X - 4000.0) < 1.0);
	TestEqual(TEXT("... at its height"), static_cast<float>(Out.Z), 25.f);
	TestTrue(TEXT("a body already out stays where it is"), HOLVolcano::EvacuationPoint(Flow, W, FVector(4000.0, 3000.0, 0.0), 60.f).Equals(FVector(4000.0, 3000.0, 0.0), 0.001));
	const FVector OnLine = HOLVolcano::EvacuationPoint(Flow, W, FVector(2000.0, 0.0, 0.0), 60.f);
	TestFalse(TEXT("a body on the line itself is led out as well"), HOLVolcano::IsInCorridor(Flow, W, OnLine, 60.f));
	const FVector OtherSide = HOLVolcano::EvacuationPoint(Flow, W, FVector(4000.0, -300.0, 0.0), 60.f);
	TestTrue(TEXT("a body on the other side goes to the other side"), OtherSide.Y < 0.0 && !HOLVolcano::IsInCorridor(Flow, W, OtherSide, 60.f));

	// Every body of a grid across the corridor ends outside it.
	int32 Caught = 0;
	int32 Left = 0;
	for (double X = -500.0; X <= 8500.0; X += 500.0)
	{
		for (double Y = -1500.0; Y <= 1500.0; Y += 100.0)
		{
			const FVector P(X, Y, 0.0);
			if (HOLVolcano::IsInCorridor(Flow, W, P, 60.f))
			{
				++Caught;
				Left += HOLVolcano::IsInCorridor(Flow, W, HOLVolcano::EvacuationPoint(Flow, W, P, 60.f), 60.f) ? 0 : 1;
			}
		}
	}
	TestTrue(TEXT("the grid has bodies in the corridor"), Caught > 100);
	TestEqual(TEXT("every one of them is led out"), Left, Caught);
	return true;
}

// The altar held starts the alert AT ONCE (V8, D380; Q13 = B of the user, 2026-11-02): the default AltarAdvance is "all the time left before the alert"; a smaller
// setting still moves the alert by that much (the setting rules).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarTest, "HeroesOfLiberty.Volcano.AltarAdvance", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoAltarTest::RunTest(const FString& Parameters)
{
	FHOLVolcanoSettings Settings;
	const HOLVolcano::FCycle Base = HOLVolcano::MakeCycle(4242, 0, 0.f, Settings, 60.f);
	TestTrue(TEXT("the setting is a positive advance"), Settings.AltarAdvance > 0.f);

	// Held long before the alert: the alert starts at that very instant (option B), as long as before, the rest of the cycle following.
	const HOLVolcano::FCycle Early = HOLVolcano::AdvanceForAltar(Base, 10.f, Settings);
	TestEqual(TEXT("the alert starts the instant the altar is held"), Early.WarningAt, 10.f);
	TestEqual(TEXT("... the alert keeps its length (D380)"), Early.EruptAt - Early.WarningAt, Base.EruptAt - Base.WarningAt);
	TestEqual(TEXT("... the eruption follows the alert"), Early.EruptAt, 10.f + (Base.EruptAt - Base.WarningAt));
	const float Moved = Base.WarningAt - 10.f;
	TestEqual(TEXT("... the cooling and the re-arming follow by the same shift"), Base.ReArmAt - Early.ReArmAt, Moved);

	// Held just before the alert: the alert starts at once, not before.
	const float Late = Base.WarningAt - 30.f;
	TestEqual(TEXT("held 30 s before the alert: the alert comes at once"), HOLVolcano::AdvanceForAltar(Base, Late, Settings).WarningAt, Late);

	// Held once the alert has begun: nothing changes.
	const HOLVolcano::FCycle After = HOLVolcano::AdvanceForAltar(Base, Base.WarningAt + 1.f, Settings);
	TestEqual(TEXT("held during the alert: unchanged"), After.WarningAt, Base.WarningAt);

	// The setting rules: no advance, no change; a bounded one (option A's 120 s) moves by that much only.
	FHOLVolcanoSettings None = Settings;
	None.AltarAdvance = 0.f;
	TestEqual(TEXT("no advance: unchanged"), HOLVolcano::AdvanceForAltar(Base, 10.f, None).WarningAt, Base.WarningAt);
	FHOLVolcanoSettings Bounded = Settings;
	Bounded.AltarAdvance = 120.f;
	TestEqual(TEXT("a bounded advance moves the alert by that much"), HOLVolcano::AdvanceForAltar(Base, 10.f, Bounded).WarningAt, Base.WarningAt - 120.f);
	return true;
}

// How many soldiers the AI sends to the altar an adversary holds (V11, D427, the user's answer of 2026-11-02): a small adversary meets a bigger squad,
// a big army is not met head on (Hard: a diversion), the size does not oscillate over the reflections.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarSendTest, "HeroesOfLiberty.Volcano.AltarSend", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoAltarSendTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	using namespace HOLVolcano;
	// 10 free soldiers, the reserve (40 %) keeps 4 at home: 6 can go.
	FAltarSendInput Small;
	Small.FreeSoldiers = 10;
	Small.AdversaryStrength = 1.f;
	const FAltarSend ToSmall = AltarSend(Small, Settings);
	TestTrue(TEXT("a small adversary (1): a squad bigger than him"), ToSmall.Squad > 1);
	TestEqual(TEXT("... the minimum of the settings (needs 1.5, at least 2)"), ToSmall.Squad, Settings.AiAltarMinSend);
	TestFalse(TEXT("... no diversion"), ToSmall.bDiversion);

	FAltarSendInput Middle = Small;
	Middle.AdversaryStrength = 3.f;
	TestEqual(TEXT("an adversary of 3: 1.5 times, rounded up, 5"), AltarSend(Middle, Settings).Squad, 5);

	FAltarSendInput Big = Small;
	Big.AdversaryStrength = 8.f;
	Big.Level = EAltarLevel::Hard;
	const FAltarSend ToBig = AltarSend(Big, Settings);
	TestEqual(TEXT("a big army (8, needs 12 of the 6 spared), Hard: no massive send"), ToBig.Squad, 0);
	TestTrue(TEXT("... a diversion instead (its camp is bare)"), ToBig.bDiversion);
	Big.Level = EAltarLevel::Normal;
	TestEqual(TEXT("... Normal sends all it can spare (6)"), AltarSend(Big, Settings).Squad, 6);
	TestFalse(TEXT("... without diversion"), AltarSend(Big, Settings).bDiversion);
	Big.Level = EAltarLevel::Hard;
	Big.AlreadySent = 4;
	TestEqual(TEXT("Hard with 4 already there: they stay, no recall"), AltarSend(Big, Settings).Squad, 4);

	// Ten reflections with an adversary whose strength wobbles between 3 and 4: the squad never shrinks and settles.
	FAltarSendInput Wobble = Small;
	int32 Previous = 0;
	bool bNeverShrinks = true;
	TArray<int32> Squads;
	for (int32 Reflection = 0; Reflection < 10; ++Reflection)
	{
		Wobble.AdversaryStrength = (Reflection % 2 == 0) ? 3.f : 4.f;
		Wobble.AlreadySent = Previous;
		const int32 Squad = AltarSend(Wobble, Settings).Squad;
		bNeverShrinks = bNeverShrinks && Squad >= Previous;
		Previous = Squad;
		Squads.Add(Squad);
	}
	TestTrue(TEXT("ten reflections: the squad never shrinks"), bNeverShrinks);
	TestEqual(TEXT("... and holds the same size over the last 6"), Squads.Last(), Squads[4]);
	TestEqual(TEXT("... the size of the bigger need (4 x 1.5 = 6)"), Squads.Last(), 6);
	return true;
}

// The altar's count (V8, D380, D348's rule): one camp in the zone counts up, nobody makes it go down, two camps freeze it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarHoldTest, "HeroesOfLiberty.Volcano.AltarHold", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoAltarHoldTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	TestEqual(TEXT("held ~60 s (D380)"), Settings.AltarHoldSeconds, 60.f);

	float Held = 0.f;
	TestTrue(TEXT("one camp: holding"), HOLVolcano::StepAltar(Held, 1, 1.f, Settings) == HOLVolcano::EAltarHold::Holding);
	TestEqual(TEXT("... the count goes up by the step"), Held, 1.f);
	for (int32 Second = 0; Second < 19; ++Second)
	{
		HOLVolcano::StepAltar(Held, 1, 1.f, Settings);
	}
	TestEqual(TEXT("20 s held"), Held, 20.f);

	TestTrue(TEXT("two camps: contested"), HOLVolcano::StepAltar(Held, 2, 1.f, Settings) == HOLVolcano::EAltarHold::Contested);
	TestTrue(TEXT("... three camps too"), HOLVolcano::StepAltar(Held, 3, 5.f, Settings) == HOLVolcano::EAltarHold::Contested);
	TestEqual(TEXT("... the count is frozen"), Held, 20.f);

	TestTrue(TEXT("nobody: idle"), HOLVolcano::StepAltar(Held, 0, 1.f, Settings) == HOLVolcano::EAltarHold::Idle);
	TestEqual(TEXT("... the count goes down by the decay rate"), Held, 20.f - Settings.AltarDecayRate);
	HOLVolcano::StepAltar(Held, 0, 1000.f, Settings);
	TestEqual(TEXT("... never below zero"), Held, 0.f);

	// The count belongs to its holder: another camp alone in the zone starts from 0, the same camp goes on from its own count.
	float Owned = 0.f;
	int32 Holder = INDEX_NONE;
	for (int32 Second = 0; Second < 29; ++Second)
	{
		HOLVolcano::StepAltarOwned(Owned, Holder, 1, 0, 1.f, Settings);
	}
	TestTrue(TEXT("camp 0 holds 29 s"), Holder == 0 && FMath::IsNearlyEqual(Owned, 29.f));
	HOLVolcano::StepAltarOwned(Owned, Holder, 2, INDEX_NONE, 1.f, Settings);
	TestEqual(TEXT("contested: frozen, still camp 0's"), Owned, 29.f);
	HOLVolcano::StepAltarOwned(Owned, Holder, 1, 0, 1.f, Settings);
	TestEqual(TEXT("camp 0 alone again: goes on"), Owned, 30.f);
	HOLVolcano::StepAltarOwned(Owned, Holder, 1, 1, 1.f, Settings);
	TestTrue(TEXT("camp 1 alone after camp 0's 30 s: starts again, 1 s, not 31"), Holder == 1 && Owned == 1.f);

	// Held 60 s in a row reaches the threshold, 59 s does not.
	float Count = 0.f;
	for (int32 Second = 0; Second < 59; ++Second)
	{
		HOLVolcano::StepAltar(Count, 1, 1.f, Settings);
	}
	TestTrue(TEXT("59 s: not held yet"), Count < Settings.AltarHoldSeconds);
	HOLVolcano::StepAltar(Count, 1, 1.f, Settings);
	TestTrue(TEXT("60 s: held"), Count >= Settings.AltarHoldSeconds);
	return true;
}

// The woken cycle in a site's state (V8): the altar held starts that cycle's alert at once; the site is lit while Armed and not woken.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarWakeTest, "HeroesOfLiberty.Volcano.AltarWake", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoAltarWakeTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	constexpr float ManSpeed = 697.f;
	constexpr int32 MatchSeed = 4242;
	HOLVolcano::FSite Site;
	HOLVolcano::FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(9000.0, 0.0, 0.0) };
	Site.Flows = { Flow };
	Site.Altar.bPresent = true;
	Site.Altar.Location = FVector(200.0, 0.0, 0.0);

	const HOLVolcano::FSiteState Plain = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, 30.f, Settings, ManSpeed);
	TestTrue(TEXT("armed at 30 s"), Plain.Phase == EHOLVolcanoPhase::Armed);
	TestTrue(TEXT("the altar is lit while armed"), Plain.bAltarLit);
	TestFalse(TEXT("... and the cycle is not woken"), Plain.bWoken);

	// Held at 30 s: the alert starts then.
	HOLVolcano::FWakes Wakes;
	Wakes.Add(Plain.CycleIndex, 30.f);
	const HOLVolcano::FSiteState Woken = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, 31.f, Settings, ManSpeed, &Wakes);
	TestTrue(TEXT("one second after the altar is held: the alert"), Woken.Phase == EHOLVolcanoPhase::Warning);
	TestTrue(TEXT("... woken"), Woken.bWoken);
	TestFalse(TEXT("... the altar is no longer lit"), Woken.bAltarLit);
	TestEqual(TEXT("... the alert began when the altar was held"), Woken.WarningAt, 30.f);
	TestTrue(TEXT("... the alert has its 60 to 90 s (D28)"), Woken.EruptAt - Woken.WarningAt >= Settings.WarningMin && Woken.EruptAt - Woken.WarningAt <= Settings.WarningMax);
	TestTrue(TEXT("... the countdown is the alert's length less the second spent"), FMath::IsNearlyEqual(Woken.SecondsToErupt, Woken.EruptAt - 31.f, 0.001f));
	TestTrue(TEXT("same flow as the plain cycle (the draw does not move)"), Woken.FlowIndex == Plain.FlowIndex);
	TestFalse(TEXT("without the wake the same instant is still armed"), HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, 31.f, Settings, ManSpeed).Phase == EHOLVolcanoPhase::Warning);

	// The eruption comes at the woken date and the rest of the cycle follows; the next cycle opens earlier than it would have.
	const HOLVolcano::FSiteState Erupting = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Woken.EruptAt + 2.f, Settings, ManSpeed, &Wakes);
	TestTrue(TEXT("erupting two seconds after the woken date"), Erupting.Phase == EHOLVolcanoPhase::Erupting);
	const float ProbeAt = Plain.EruptAt + 2000.f;
	const HOLVolcano::FSiteState PlainNext = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, ProbeAt, Settings, ManSpeed);
	const HOLVolcano::FSiteState WokenNext = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, ProbeAt, Settings, ManSpeed, &Wakes);
	TestTrue(TEXT("the later cycles come sooner than without the altar (the cycle restarts from the provoked eruption, D400)"), WokenNext.CycleIndex >= PlainNext.CycleIndex);
	TestTrue(TEXT("... and the woken cycle is the first one"), Plain.CycleIndex == 0);

	// Lit again at the re-arming: the next cycle is Armed and lit, whatever woke the last.
	const HOLVolcano::FSiteState Later = HOLVolcano::SiteStateAt(true, MatchSeed, 0, Site, Woken.EruptAt + 1000.f, Settings, ManSpeed, &Wakes);
	TestTrue(TEXT("a cycle later: a new cycle"), Later.CycleIndex == Plain.CycleIndex + 1);
	TestTrue(TEXT("... the altar is lit again (D400) whenever that cycle is armed"), Later.Phase != EHOLVolcanoPhase::Armed || Later.bAltarLit);

	// Without Catastrophe nothing is lit.
	TestFalse(TEXT("no Catastrophe: no altar lit"), HOLVolcano::SiteStateAt(false, MatchSeed, 0, Site, 30.f, Settings, ManSpeed).bAltarLit);
	return true;
}

// The free altar the AI holds itself (V11, D640): one free soldier in 4 (one in 8 in Easy) when no adversary is in the zone and the nearest trace is
// farther than 40 m from its positions; never a recall.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoAltarHoldSquadTest, "HeroesOfLiberty.Volcano.AltarHoldSquad", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoAltarHoldSquadTest::RunTest(const FString& Parameters)
{
	const FHOLVolcanoSettings Settings;
	using namespace HOLVolcano;
	FAltarHoldInput Input;
	Input.FreeSoldiers = 8;
	Input.NearestTraceDistance = 9000.f;
	TestEqual(TEXT("40 m kept as the default"), Settings.AiAltarSafeDistance, 4000.f);
	TestEqual(TEXT("Normal: one soldier in 4 of 8"), AltarHoldSquad(Input, Settings), 2);
	Input.Level = EAltarLevel::Hard;
	TestEqual(TEXT("Hard: the same quarter"), AltarHoldSquad(Input, Settings), 2);
	Input.Level = EAltarLevel::Easy;
	TestEqual(TEXT("Easy: one in 8"), AltarHoldSquad(Input, Settings), 1);
	Input.Level = EAltarLevel::Normal;
	Input.FreeSoldiers = 5;
	TestEqual(TEXT("5 free soldiers: the quarter rounded up, 2"), AltarHoldSquad(Input, Settings), 2);
	Input.FreeSoldiers = 0;
	TestEqual(TEXT("no free soldier: none"), AltarHoldSquad(Input, Settings), 0);

	Input.FreeSoldiers = 8;
	Input.NearestTraceDistance = 4000.f;
	TestEqual(TEXT("a trace at exactly 40 m is too near"), AltarHoldSquad(Input, Settings), 0);
	Input.NearestTraceDistance = 9000.f;
	Input.bAdversaryInZone = true;
	TestEqual(TEXT("an adversary in the zone: not a free altar (the contest rule applies)"), AltarHoldSquad(Input, Settings), 0);
	Input.bAdversaryInZone = false;
	Input.AlreadySent = 3;
	TestEqual(TEXT("3 already there: they stay"), AltarHoldSquad(Input, Settings), 3);
	Input.NearestTraceDistance = 100.f;
	TestEqual(TEXT("... even if the trace were near: no recall"), AltarHoldSquad(Input, Settings), 3);

	FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(10000.0, 0.0, 0.0) };
	TestTrue(TEXT("the distance to a line: 3000 cm beside its middle"), FMath::IsNearlyEqual(DistanceToFlow(Flow, FVector(5000.0, 3000.0, 0.0)), 3000.f, 0.5f));
	TestTrue(TEXT("... past its end: to the end point"), FMath::IsNearlyEqual(DistanceToFlow(Flow, FVector(13000.0, 4000.0, 0.0)), 5000.f, 0.5f));
	return true;
}

// The fresh deposit of a cooled flow (V7, D379): gold or stone, on the flow's line, a function of the seed and the cycle, there when the basalt is passable.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoFreshDepositTest, "HeroesOfLiberty.Volcano.FreshDeposit", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoFreshDepositTest::RunTest(const FString& Parameters)
{
	using namespace HOLVolcano;
	FHOLVolcanoSettings Settings;
	FFlow Flow;
	Flow.Points = { FVector(0.0, 0.0, 0.0), FVector(6000.0, 0.0, 0.0), FVector(6000.0, 4000.0, 0.0) };
	const int32 SiteSeedValue = SiteSeed(4242, 0);

	const FFreshDeposit First = FreshDeposit(SiteSeedValue, 0, Flow, Settings);
	if (!TestTrue(TEXT("a flow with a length leaves a deposit"), First.bPresent))
	{
		return false;
	}
	// D649: the place is drawn from the seed between the two settings, on the flow's line, and is not a fixed value.
	TestTrue(TEXT("it stands where its drawn fraction says"), First.Location.Equals(PointAt(Flow, First.Along * Flow.Length()), 1.0));
	TestTrue(TEXT("... which is on the flow's line"), DistanceToFlow(Flow, First.Location) < 1.f);
	TestTrue(TEXT("... named by the setting of its kind and its size"), First.TypeName == (First.Kind == EFreshKind::Gold ? (First.bBig ? Settings.FreshDepositGoldBigType : Settings.FreshDepositGoldSmallType)
		: (First.bBig ? Settings.FreshDepositStoneBigType : Settings.FreshDepositStoneSmallType)));
	TestTrue(TEXT("the same call gives the same deposit (a function of seed, cycle and flow)"),
		FreshDeposit(SiteSeedValue, 0, Flow, Settings).Kind == First.Kind && FreshDeposit(SiteSeedValue, 0, Flow, Settings).Location == First.Location && FreshDeposit(SiteSeedValue, 0, Flow, Settings).bBig == First.bBig);

	// The place varies from cycle to cycle, always inside the settings' range; both sizes come; every kind and size pairs with its own type.
	float LowestAlong = 2.f;
	float HighestAlong = -1.f;
	int32 Big = 0;
	int32 Combos[2][2] = { { 0, 0 }, { 0, 0 } };
	bool bAllNamed = true;
	for (int32 Cycle = 0; Cycle < 400; ++Cycle)
	{
		const FFreshDeposit Drawn = FreshDeposit(SiteSeedValue, Cycle, Flow, Settings);
		LowestAlong = FMath::Min(LowestAlong, Drawn.Along);
		HighestAlong = FMath::Max(HighestAlong, Drawn.Along);
		Big += Drawn.bBig ? 1 : 0;
		Combos[Drawn.Kind == EFreshKind::Gold ? 1 : 0][Drawn.bBig ? 1 : 0]++;
		bAllNamed &= Drawn.Location.Equals(PointAt(Flow, Drawn.Along * Flow.Length()), 1.0) && !Drawn.TypeName.IsNone();
	}
	TestTrue(TEXT("the place is always between the minimum and the maximum of the settings"), LowestAlong >= Settings.FreshDepositAlongMin - 0.0001f && HighestAlong <= Settings.FreshDepositAlongMax + 0.0001f);
	TestTrue(TEXT("... and it varies (not one fixed place): the 400 cycles cover most of the range"),
		LowestAlong < Settings.FreshDepositAlongMin + 0.1f * (Settings.FreshDepositAlongMax - Settings.FreshDepositAlongMin)
		&& HighestAlong > Settings.FreshDepositAlongMax - 0.1f * (Settings.FreshDepositAlongMax - Settings.FreshDepositAlongMin));
	TestTrue(TEXT("both sizes come, about half big over 400 cycles"), Big > 140 && Big < 260);
	TestTrue(TEXT("all four combinations (small/big gold/stone) come"), Combos[0][0] > 20 && Combos[0][1] > 20 && Combos[1][0] > 20 && Combos[1][1] > 20);
	TestTrue(TEXT("every draw stands on the flow and has a type"), bAllNamed);

	Settings.FreshDepositBigShare = 0.f;
	Big = 0;
	for (int32 Cycle = 0; Cycle < 50; ++Cycle)
	{
		Big += FreshDeposit(SiteSeedValue, Cycle, Flow, Settings).bBig ? 1 : 0;
	}
	TestEqual(TEXT("big share 0: always small"), Big, 0);
	Settings.FreshDepositBigShare = 1.f;
	for (int32 Cycle = 0; Cycle < 50; ++Cycle)
	{
		Big += FreshDeposit(SiteSeedValue, Cycle, Flow, Settings).bBig ? 1 : 0;
	}
	TestEqual(TEXT("big share 1: always big"), Big, 50);
	Settings.FreshDepositBigShare = 0.5f;
	Settings.FreshDepositAlongMin = 0.25f;
	Settings.FreshDepositAlongMax = 0.25f;
	TestTrue(TEXT("a range reduced to one fraction puts it there"), FreshDeposit(SiteSeedValue, 0, Flow, Settings).Location.Equals(PointAt(Flow, 0.25f * Flow.Length()), 1.0));
	Settings.FreshDepositAlongMin = 0.3f;
	Settings.FreshDepositAlongMax = 0.7f;

	// Both kinds come, about as often as the share says; no memory (a cycle does not depend on the previous one).
	int32 Gold = 0;
	for (int32 Cycle = 0; Cycle < 200; ++Cycle)
	{
		Gold += FreshDeposit(SiteSeedValue, Cycle, Flow, Settings).Kind == EFreshKind::Gold ? 1 : 0;
	}
	TestTrue(TEXT("share 0.5: both kinds come, about half gold over 200 cycles"), Gold > 70 && Gold < 130);
	Settings.FreshDepositGoldShare = 0.f;
	Gold = 0;
	for (int32 Cycle = 0; Cycle < 50; ++Cycle)
	{
		Gold += FreshDeposit(SiteSeedValue, Cycle, Flow, Settings).Kind == EFreshKind::Gold ? 1 : 0;
	}
	TestEqual(TEXT("share 0: always stone"), Gold, 0);
	Settings.FreshDepositGoldShare = 1.f;
	for (int32 Cycle = 0; Cycle < 50; ++Cycle)
	{
		Gold += FreshDeposit(SiteSeedValue, Cycle, Flow, Settings).Kind == EFreshKind::Gold ? 1 : 0;
	}
	TestEqual(TEXT("share 1: always gold"), Gold, 50);
	Settings.FreshDepositGoldShare = 0.5f;

	// Two sites of one match do not leave the same kinds.
	int32 Differ = 0;
	for (int32 Cycle = 0; Cycle < 40; ++Cycle)
	{
		Differ += FreshDeposit(SiteSeed(4242, 0), Cycle, Flow, Settings).Kind != FreshDeposit(SiteSeed(4242, 1), Cycle, Flow, Settings).Kind ? 1 : 0;
	}
	TestTrue(TEXT("two sites of a match draw their own kinds"), Differ > 5);

	FFlow Empty;
	Empty.Points = { FVector(100.0, 100.0, 0.0), FVector(100.0, 100.0, 0.0) };
	TestFalse(TEXT("a flow without length leaves nothing"), FreshDeposit(SiteSeedValue, 0, Empty, Settings).bPresent);
	TestFalse(TEXT("... nor a flow without point"), FreshDeposit(SiteSeedValue, 0, FFlow(), Settings).bPresent);

	// Due exactly while the site rests on its basalt, on the real chain of states.
	FSite Site;
	Site.Flows = { Flow };
	constexpr float ManSpeed = 697.f;
	int32 RestSteps = 0;
	bool bAllMatch = true;
	for (float Now = 0.f; Now < 3000.f; Now += 1.f)
	{
		const FSiteState State = SiteStateAt(true, 4242, 0, Site, Now, Settings, ManSpeed);
		const bool bRest = State.Phase == EHOLVolcanoPhase::Rest;
		RestSteps += bRest ? 1 : 0;
		bAllMatch &= FreshDepositDue(State) == bRest;
	}
	TestTrue(TEXT("the deposit is due in every second of the rest, and only then (not in the alert, the lava, the cooling)"), bAllMatch && RestSteps > 300);
	TestFalse(TEXT("a dormant site (no Catastrophe) leaves nothing"), FreshDepositDue(SiteStateAt(false, 4242, 0, Site, 1500.f, Settings, ManSpeed)));
	FSiteState NoFlow;
	NoFlow.Phase = EHOLVolcanoPhase::Rest;
	NoFlow.FlowIndex = INDEX_NONE;
	TestFalse(TEXT("a rest without a drawn flow leaves nothing"), FreshDepositDue(NoFlow));
	return true;
}

#endif
