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

#endif
