// Copyright Epic Games, Inc. All Rights Reserved.

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

#endif
