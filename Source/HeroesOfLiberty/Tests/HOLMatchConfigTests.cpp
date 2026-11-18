// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/GameInstance.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

// Front-end checks (D246): the match configuration is the contract with a dedicated server, so its JSON form and the
// default used without a configuration (stress test) are pinned here. PlayerColors and NeutralUnitColor moved to ScriptTestsHOLMatchConfig_Test.as (CELEBORN-1118m); SubsystemFile stays for the refusal of a missing file (the C++ test declares the expected Error, a script cannot).
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

#endif
