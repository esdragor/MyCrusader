// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatProcessor.h"
#include "Simulation/HOLMoraleProcessor.h"
#include "Simulation/HOLSimulationSettings.h"

#if WITH_DEV_AUTOMATION_TESTS

// PLAN-MORAL-1120 (S2-a rest): the morale processor lays and ticks the levers of the step, the combat reads them the same step, so Morale runs
// BEFORE Combat in the configured list (Config\DefaultGame.ini, UHOLSimulationSettings::SimulationProcessors). The test reads the list the
// simulation really runs (the settings object loaded from the ini), never a copy of it.
// RED while the ini lists HOLCombatProcessor before HOLMoraleProcessor (lines 41 and 43 before the fix).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMoraleProcessorRunsBeforeCombatTest, "HeroesOfLiberty.Morale.ProcessorRunsBeforeCombat", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLMoraleProcessorRunsBeforeCombatTest::RunTest(const FString& Parameters)
{
	const UHOLSimulationSettings* Settings = GetDefault<UHOLSimulationSettings>();
	if (!TestNotNull(TEXT("the simulation settings"), Settings))
	{
		return true;
	}
	int32 MoraleIndex = INDEX_NONE;
	int32 CombatIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Settings->SimulationProcessors.Num(); ++Index)
	{
		const UClass* ProcessorClass = Settings->SimulationProcessors[Index].Get();
		if (ProcessorClass == UHOLMoraleProcessor::StaticClass())
		{
			MoraleIndex = Index;
		}
		else if (ProcessorClass == UHOLCombatProcessor::StaticClass())
		{
			CombatIndex = Index;
		}
	}
	if (TestTrue(TEXT("the morale processor is in the configured list"), MoraleIndex != INDEX_NONE) && TestTrue(TEXT("the combat processor is in the configured list"), CombatIndex != INDEX_NONE))
	{
		TestTrue(*FString::Printf(TEXT("Morale (position %d) runs before Combat (position %d)"), MoraleIndex, CombatIndex), MoraleIndex < CombatIndex);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
