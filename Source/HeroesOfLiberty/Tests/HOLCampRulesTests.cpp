// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLMatchConfig.h"
#include "Match/HOLVictoryRules.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLCampSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// N3: the neutral owner is out of the slots and hostile to every slot of any launch (D401, FHOLCampTable); no slot index reaches it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNeutralOwnerTest, "HeroesOfLiberty.Neutrals.Owner",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHOLNeutralOwnerTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("the first number out of the slots"), FHOLMatchConfig::NeutralOwner, FHOLMatchConfig::MaxSlots);
	TestTrue(TEXT("recognised"), FHOLMatchConfig::IsNeutralOwner(FHOLMatchConfig::NeutralOwner));
	TestFalse(TEXT("a slot is not neutral"), FHOLMatchConfig::IsNeutralOwner(FHOLMatchConfig::MaxSlots - 1));

	FHOLMatchConfig Config;
	Config.Normalize();
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		TestTrue(*FString::Printf(TEXT("slot %d is hostile to the neutral owner"), Slot), HOLVictoryRules::AreHostile(Config, Slot, FHOLMatchConfig::NeutralOwner));
	}
	// Teams: allies are one camp, the neutral owner is alone in its own.
	Config.Slots[0].State = EHOLSlotState::AI;
	Config.Slots[1].State = EHOLSlotState::AI;
	Config.Slots[0].Team = 1;
	Config.Slots[1].Team = 1;
	TestFalse(TEXT("allies are not hostile"), HOLVictoryRules::AreHostile(Config, 0, 1));
	TestTrue(TEXT("a team is hostile to the neutral owner"), HOLVictoryRules::AreHostile(Config, 1, FHOLMatchConfig::NeutralOwner));

	// The combat processor's two questions of the camps' statics: nobody is passive until a camp says so, and a hit with no camp is harmless.
	TestFalse(TEXT("no passive unit by default"), HOLCamps::IsPassive(123456));
	HOLCamps::NoteHit(123456, 0, 10.f);
	HOLCamps::NoteHit(123456, FHOLMatchConfig::NeutralOwner, 10.f);
	return true;
}

#endif
