// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
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

#endif
