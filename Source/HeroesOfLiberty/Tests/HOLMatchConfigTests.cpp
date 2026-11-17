// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/GameInstance.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSubsystem.h"
#include "Match/HOLPlayerColors.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// Front-end checks (D246): the match configuration is the contract with a dedicated server, so its JSON form and the
// default used without a configuration (stress test) are pinned here.
namespace UE::HeroesOfLiberty::Tests
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FHOLMatchConfig MakeFilledConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		Config.Seed = 1234;
		Config.Map = FSoftObjectPath(TEXT("/Game/Maps/StressTest.StressTest"));
		Config.Slots[1] = { EHOLSlotState::AI, EHOLFaction::BlackLegions, 5, 2 };
		Config.Slots[3] = { EHOLSlotState::Human, EHOLFaction::Templars, 7, 1, TEXT("Chloé") };
		Config.bRegicide = true;
		Config.bCatastrophe = true;
		Config.bSacredSites = true;
		return Config;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchSubsystemFileTest, "HeroesOfLiberty.Match.SubsystemFile", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchSubsystemFileTest::RunTest(const FString& Parameters)
{
	// What a dedicated server started with -MatchConfig=<file> goes through.
	const FHOLMatchConfig Written = UE::HeroesOfLiberty::Tests::MakeFilledConfig();
	const FString Path = FPaths::Combine(FPaths::AutomationTransientDir(), TEXT("MatchConfig.json"));
	TestTrue(TEXT("file written"), FFileHelper::SaveStringToFile(Written.ToJson(), *Path));

	// A game instance subsystem must live in a game instance (ClassWithin); Initialize is not needed here.
	UGameInstance* GameInstance = NewObject<UGameInstance>(GetTransientPackage());
	UHOLMatchSubsystem* Match = NewObject<UHOLMatchSubsystem>(GameInstance);
	TestFalse(TEXT("no configuration at first"), Match->HasConfig());
	TestTrue(TEXT("default when none"), Match->GetConfigOrDefault() == FHOLMatchConfig::MakeDefault());
	TestTrue(TEXT("file read"), Match->LoadFromFile(Path));
	TestTrue(TEXT("configuration from the file"), Match->HasConfig() && Match->GetConfigOrDefault() == Written);

	AddExpectedError(TEXT("cannot read"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("missing file refused"), Match->LoadFromFile(Path + TEXT(".missing")));
	TestTrue(TEXT("configuration kept on failure"), Match->GetConfigOrDefault() == Written);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLPlayerColorsTest, "HeroesOfLiberty.Match.PlayerColors", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLPlayerColorsTest::RunTest(const FString& Parameters)
{
	TSet<uint32> Distinct;
	for (int32 Index = 0; Index < HOLPlayerColors::NumColors; ++Index)
	{
		Distinct.Add(HOLPlayerColors::GetColor(Index).DWColor());
		TestFalse(TEXT("named"), HOLPlayerColors::GetColorName(Index).IsEmpty());
	}
	TestEqual(TEXT("distinct colours"), Distinct.Num(), HOLPlayerColors::NumColors);
	TestTrue(TEXT("index wraps"), HOLPlayerColors::GetLinearColor(HOLPlayerColors::NumColors) == HOLPlayerColors::GetLinearColor(0));
	return true;
}

// Neutral units (owner 8) were drawn in the colour of the player whose palette index wraps to the same (blue, index 0):
// the colour the renderer writes for the neutral owner must be the colour of no slot, whatever the slots chose.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNeutralUnitColorTest, "HeroesOfLiberty.Match.NeutralUnitColor", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLNeutralUnitColorTest::RunTest(const FString& Parameters)
{
	constexpr int32 Slots = FHOLMatchConfig::MaxSlots;
	// The default assignment (slot i, colour i) and a shuffled one: both use the whole palette.
	for (int32 Variant = 0; Variant < 2; ++Variant)
	{
		TArray<uint8> SlotColors;
		for (int32 Slot = 0; Slot < Slots; ++Slot)
		{
			SlotColors.Add(static_cast<uint8>(Variant == 0 ? Slot : (Slot * 3 + 1) % HOLPlayerColors::NumColors));
		}
		const FVector3f Neutral = HOLPlayerColors::GetOwnerLinearColor(FHOLMatchConfig::NeutralOwner, SlotColors);
		for (int32 Color = 0; Color < HOLPlayerColors::NumColors; ++Color)
		{
			TestFalse(*FString::Printf(TEXT("variant %d: neutral colour is not palette colour %d"), Variant, Color),
				Neutral.Equals(HOLPlayerColors::GetLinearColor(Color), 0.001f));
		}
		for (int32 Slot = 0; Slot < Slots; ++Slot)
		{
			TestTrue(*FString::Printf(TEXT("variant %d: slot %d keeps its chosen colour"), Variant, Slot),
				HOLPlayerColors::GetOwnerLinearColor(Slot, SlotColors) == HOLPlayerColors::GetLinearColor(SlotColors[Slot]));
		}
	}
	return true;
}

#endif
