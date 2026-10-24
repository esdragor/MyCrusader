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
		return Config;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchConfigDefaultTest, "HeroesOfLiberty.Match.Default", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchConfigDefaultTest::RunTest(const FString& Parameters)
{
	const FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	TestEqual(TEXT("eight slots"), Config.Slots.Num(), FHOLMatchConfig::MaxSlots);
	TestEqual(TEXT("one occupied slot"), Config.GetNumOccupiedSlots(), 1);
	TestTrue(TEXT("slot 0 is human"), Config.Slots[0].State == EHOLSlotState::Human);
	TestEqual(TEXT("current version"), Config.Version, FHOLMatchConfig::CurrentVersion);
	TestFalse(TEXT("a map is set"), Config.Map.IsNull());
	TestTrue(TEXT("Standard without options"), Config.Mode == EHOLMatchMode::Standard && !Config.bNomad
		&& !Config.bRegicide && !Config.bCatastrophe);

	TSet<int32> Colors;
	for (const FHOLMatchSlot& Slot : Config.Slots)
	{
		Colors.Add(Slot.ColorIndex);
	}
	TestEqual(TEXT("distinct default colours"), Colors.Num(), FHOLMatchConfig::MaxSlots);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchConfigJsonRoundTripTest, "HeroesOfLiberty.Match.JsonRoundTrip", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchConfigJsonRoundTripTest::RunTest(const FString& Parameters)
{
	const FHOLMatchConfig Written = UE::HeroesOfLiberty::Tests::MakeFilledConfig();
	FHOLMatchConfig Read;
	FString Error;
	TestTrue(TEXT("reads its own JSON"), FHOLMatchConfig::FromJson(Written.ToJson(), Read, &Error));
	TestTrue(FString::Printf(TEXT("identical after a round trip (%s)"), *Error), Read == Written);
	TestEqual(TEXT("three occupied slots"), Read.GetNumOccupiedSlots(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchConfigOlderJsonTest, "HeroesOfLiberty.Match.OlderJson", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchConfigOlderJsonTest::RunTest(const FString& Parameters)
{
	// No version, two slots only, no options: what a file written before a field existed looks like.
	const FString Json = TEXT(R"({ "map": "/Game/Level/BattleMap.BattleMap",
		"slots": [ { "state": "Human", "colorIndex": 3 }, { "state": "AI", "faction": "BlackLegions", "team": 9 } ] })");
	FHOLMatchConfig Read;
	FString Error;
	TestTrue(FString::Printf(TEXT("reads an older file (%s)"), *Error), FHOLMatchConfig::FromJson(Json, Read, &Error));
	TestEqual(TEXT("padded to eight slots"), Read.Slots.Num(), FHOLMatchConfig::MaxSlots);
	TestEqual(TEXT("two occupied slots"), Read.GetNumOccupiedSlots(), 2);
	TestEqual(TEXT("colour kept"), Read.Slots[0].ColorIndex, 3);
	TestEqual(TEXT("team out of range back to none"), Read.Slots[1].Team, 0);
	TestTrue(TEXT("faction read by name"), Read.Slots[1].Faction == EHOLFaction::BlackLegions);
	TestFalse(TEXT("missing option keeps its default"), Read.bCatastrophe);
	TestEqual(TEXT("upgraded to the current version"), Read.Version, FHOLMatchConfig::CurrentVersion);
	TestTrue(TEXT("no player name before version 2"), Read.Slots[0].PlayerName.IsEmpty());

	FHOLMatchConfig ReadV1;
	TestTrue(TEXT("reads a version 1 file"), FHOLMatchConfig::FromJson(TEXT(R"({ "version": 1, "slots": [ { "state": "Human" } ] })"), ReadV1));
	TestEqual(TEXT("version 1 upgraded"), ReadV1.Version, FHOLMatchConfig::CurrentVersion);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchConfigManagerReferenceTest, "HeroesOfLiberty.Match.ManagerReference", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchConfigManagerReferenceTest::RunTest(const FString& Parameters)
{
	// The match.json the lobby manager writes (R3.1, Manager\PROTOCOL.md), from the manager's own reference file.
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Manager/HOLManager.Tests/Data/match_v2_reference.json"));
	FString Json;
	if (!TestTrue(FString::Printf(TEXT("reference file read (%s)"), *Path), FFileHelper::LoadFileToString(Json, *Path)))
	{
		return false;
	}
	FHOLMatchConfig Read;
	FString Error;
	const bool bParsed = FHOLMatchConfig::FromJson(Json, Read, &Error);
	if (!TestTrue(FString::Printf(TEXT("reference parsed (%s)"), *Error), bParsed))
	{
		return false;
	}
	TestEqual(TEXT("version 2"), Read.Version, 2);
	TestEqual(TEXT("seed"), Read.Seed, 184467);
	TestEqual(TEXT("map"), Read.Map.ToString(), FString(TEXT("/Game/Maps/Plain.Plain")));
	TestTrue(TEXT("Standard, nomad only"), Read.Mode == EHOLMatchMode::Standard && Read.bNomad && !Read.bRegicide
		&& !Read.bCatastrophe);
	TestEqual(TEXT("eight slots"), Read.Slots.Num(), FHOLMatchConfig::MaxSlots);
	TestEqual(TEXT("three humans"), Read.GetNumOccupiedSlots(), 3);

	struct FExpected { EHOLSlotState State; int32 ColorIndex; int32 Team; const TCHAR* Name; };
	const FExpected Expected[] = {
		{ EHOLSlotState::Human, 0, 1, TEXT("Bob") },
		{ EHOLSlotState::Closed, 1, 0, TEXT("") },
		{ EHOLSlotState::Human, 3, 2, TEXT("Alice") },
		{ EHOLSlotState::Human, 5, 1, TEXT("Chloé") },
		{ EHOLSlotState::Closed, 4, 0, TEXT("") },
		{ EHOLSlotState::Closed, 5, 0, TEXT("") },
		{ EHOLSlotState::Closed, 6, 0, TEXT("") },
		{ EHOLSlotState::Closed, 7, 0, TEXT("") },
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Expected) && Index < Read.Slots.Num(); ++Index)
	{
		const FHOLMatchSlot& Slot = Read.Slots[Index];
		TestTrue(FString::Printf(TEXT("slot %d state"), Index), Slot.State == Expected[Index].State);
		TestTrue(FString::Printf(TEXT("slot %d faction"), Index), Slot.Faction == EHOLFaction::DawnOrder);
		TestEqual(FString::Printf(TEXT("slot %d colour"), Index), Slot.ColorIndex, Expected[Index].ColorIndex);
		TestEqual(FString::Printf(TEXT("slot %d team"), Index), Slot.Team, Expected[Index].Team);
		TestEqual(FString::Printf(TEXT("slot %d name"), Index), Slot.PlayerName, FString(Expected[Index].Name));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchConfigRejectsTest, "HeroesOfLiberty.Match.Rejects", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchConfigRejectsTest::RunTest(const FString& Parameters)
{
	FHOLMatchConfig Read = FHOLMatchConfig::MakeDefault();
	const FHOLMatchConfig Before = Read;
	TestFalse(TEXT("newer version refused"), FHOLMatchConfig::FromJson(TEXT(R"({ "version": 99 })"), Read));
	TestFalse(TEXT("malformed JSON refused"), FHOLMatchConfig::FromJson(TEXT("{ not json"), Read));
	TestTrue(TEXT("output untouched on failure"), Read == Before);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchConfigSlotsTest, "HeroesOfLiberty.Match.HumanSlots", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchConfigSlotsTest::RunTest(const FString& Parameters)
{
	const FHOLMatchConfig Config = UE::HeroesOfLiberty::Tests::MakeFilledConfig();
	TestEqual(TEXT("first human slot"), Config.FindFreeHumanSlot({}), 0);
	TestEqual(TEXT("AI slot skipped"), Config.FindFreeHumanSlot({ 0 }), 3);
	TestEqual(TEXT("no human slot left"), Config.FindFreeHumanSlot({ 0, 3 }), INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMatchConfigSlotForNameTest, "HeroesOfLiberty.Match.SlotForName", UE::HeroesOfLiberty::Tests::Flags)
bool FHOLMatchConfigSlotForNameTest::RunTest(const FString& Parameters)
{
	// R3.2: the lobby manager names the human slots, each player gets the one of his name.
	FHOLMatchConfig Named = FHOLMatchConfig::MakeDefault();
	Named.Slots[0].PlayerName = TEXT("Alice");
	Named.Slots[1] = { EHOLSlotState::AI, EHOLFaction::BlackLegions, 1, 0 };
	Named.Slots[2] = { EHOLSlotState::Human, EHOLFaction::DawnOrder, 2, 0, TEXT("Bob") };
	TestTrue(TEXT("names given"), Named.HasPlayerNames());
	TestEqual(TEXT("known name, whatever the arrival"), Named.FindSlotForName(TEXT("Bob"), {}), 2);
	TestEqual(TEXT("first named slot"), Named.FindSlotForName(TEXT("Alice"), { 2 }), 0);
	TestEqual(TEXT("case ignored"), Named.FindSlotForName(TEXT("bOB"), {}), 2);
	TestEqual(TEXT("spaces around ignored"), Named.FindSlotForName(TEXT(" Bob "), {}), 2);
	TestEqual(TEXT("unknown name: spectator"), Named.FindSlotForName(TEXT("Chloé"), {}), INDEX_NONE);
	TestEqual(TEXT("no name: spectator"), Named.FindSlotForName(FString(), {}), INDEX_NONE);
	TestEqual(TEXT("name already held: spectator"), Named.FindSlotForName(TEXT("Bob"), { 2 }), INDEX_NONE);

	// Without names (solo, tests, net_match.ps1, version 1): by arrival, as before.
	const FHOLMatchConfig Unnamed = UE::HeroesOfLiberty::Tests::MakeFilledConfig();
	FHOLMatchConfig NoNames = Unnamed;
	NoNames.Slots[3].PlayerName.Reset();
	TestFalse(TEXT("no names"), NoNames.HasPlayerNames());
	TestEqual(TEXT("no names: first free slot"), NoNames.FindSlotForName(TEXT("Bob"), {}), 0);
	TestEqual(TEXT("no names, no name: first free slot"), NoNames.FindSlotForName(FString(), { 0 }), 3);
	TestEqual(TEXT("no names, all taken"), NoNames.FindSlotForName(TEXT("Bob"), { 0, 3 }), INDEX_NONE);
	return true;
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

#endif
