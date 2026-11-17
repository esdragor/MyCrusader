// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dev/HOLTestHooks.h"

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
// V4c (V4-Q2 = C, D608): the seed of the match makes the eruption. A scenario's .json imposes it ("seed"), the real chain reads it
// (MakeScenarioConfig, GetScenarioSeed), and the site's state follows from the configuration and the clock alone: the same seed gives the same
// eruption on any launch, another seed another one. The state half of this test moved to script (HOLVolcanoRules_Test.as,
// Test_VolcanoSiteStateSameSeedSameEruption); the .json half stays here (the real chain, HOLTestHooks).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoSeedTest, "HeroesOfLiberty.Volcano.MatchSeed", UE::HeroesOfLiberty::Tests::VolcanoFlags)
bool FHOLVolcanoSeedTest::RunTest(const FString& Parameters)
{
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
#endif
