// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionChoice.h"
#include "Misc/AutomationTest.h"
#include "Net/HOLManagerClientRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// FactionChoiceInProgress.md: the faction list of "Nouvelle partie" and the cheat that adds the Templars.
namespace UE::HeroesOfLiberty::Tests::FactionChoice
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionChoiceListTest, "HeroesOfLiberty.Match.FactionChoice.List", UE::HeroesOfLiberty::Tests::FactionChoice::Flags)
bool FHOLFactionChoiceListTest::RunTest(const FString& Parameters)
{
	for (const bool bCheat : { false, true })
	{
		TestTrue(TEXT("the Dawn is chosen"), HOLFactionChoice::IsChoosable(EHOLFaction::DawnOrder, bCheat));
		TestTrue(TEXT("the Legions are chosen"), HOLFactionChoice::IsChoosable(EHOLFaction::BlackLegions, bCheat));
		for (const EHOLFaction Greyed : { EHOLFaction::DragonChildren, EHOLFaction::ShadowCircle, EHOLFaction::FireHeirs })
		{
			TestTrue(TEXT("Dragon, Shadow, Fire are listed..."), HOLFactionChoice::IsListed(Greyed, bCheat));
			TestFalse(TEXT("... but greyed, cheat or not"), HOLFactionChoice::IsChoosable(Greyed, bCheat));
		}
	}
	TestFalse(TEXT("without the cheat the Templars are not listed (not greyed: the cheat stays secret)"), HOLFactionChoice::IsListed(EHOLFaction::Templars, false));
	TestFalse(TEXT("... nor chosen"), HOLFactionChoice::IsChoosable(EHOLFaction::Templars, false));
	TestTrue(TEXT("with the cheat they are listed"), HOLFactionChoice::IsListed(EHOLFaction::Templars, true));
	TestTrue(TEXT("... and chosen"), HOLFactionChoice::IsChoosable(EHOLFaction::Templars, true));

	const TArray<EHOLFaction> Without = HOLFactionChoice::GetListed(false);
	TestEqual(TEXT("five entries without the cheat"), Without.Num(), 5);
	const TArray<EHOLFaction> With = HOLFactionChoice::GetListed(true);
	TestEqual(TEXT("six with it"), With.Num(), 6);
	for (int32 Index = 0; Index < With.Num(); ++Index)
	{
		TestEqual(TEXT("the drop-down's index is the enum's value"), static_cast<int32>(With[Index]), Index);
	}

	TestTrue(TEXT("a Templar slot without the cheat plays the Dawn"), HOLFactionChoice::Sanitize(EHOLFaction::Templars, false) == EHOLFaction::DawnOrder);
	TestTrue(TEXT("with the cheat it stays"), HOLFactionChoice::Sanitize(EHOLFaction::Templars, true) == EHOLFaction::Templars);
	TestTrue(TEXT("the Legions are never touched"), HOLFactionChoice::Sanitize(EHOLFaction::BlackLegions, false) == EHOLFaction::BlackLegions);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionChoiceNameTest, "HeroesOfLiberty.Match.FactionChoice.Names", UE::HeroesOfLiberty::Tests::FactionChoice::Flags)
bool FHOLFactionChoiceNameTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("the manager's name of the Templars"), HOLFactionChoice::GetName(EHOLFaction::Templars), FString(TEXT("Templars")));
	TestEqual(TEXT("... of the Dawn"), HOLFactionChoice::GetName(EHOLFaction::DawnOrder), FString(TEXT("DawnOrder")));
	for (int32 Value = 0; Value <= static_cast<int32>(EHOLFaction::Templars); ++Value)
	{
		const EHOLFaction Faction = static_cast<EHOLFaction>(Value);
		EHOLFaction Back = EHOLFaction::Templars;
		TestTrue(TEXT("every name reads back"), HOLFactionChoice::TryFindByName(HOLFactionChoice::GetName(Faction), Back));
		TestTrue(TEXT("... to the same faction"), Back == Faction);
	}
	EHOLFaction Unknown = EHOLFaction::BlackLegions;
	TestFalse(TEXT("an unknown name is refused"), HOLFactionChoice::TryFindByName(TEXT("Pirates"), Unknown));
	TestFalse(TEXT("an empty name too"), HOLFactionChoice::TryFindByName(FString(), Unknown));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionChoiceFlagTest, "HeroesOfLiberty.Match.FactionChoice.TripleClick", UE::HeroesOfLiberty::Tests::FactionChoice::Flags)
bool FHOLFactionChoiceFlagTest::RunTest(const FString& Parameters)
{
	const bool bWas = HOLFactionChoice::IsCheatOn();
	HOLFactionChoice::SetCheatOn(false);
	TestFalse(TEXT("off by default"), HOLFactionChoice::IsCheatOn());
	HOLFactionChoice::SetCheatOn(true);
	TestTrue(TEXT("on"), HOLFactionChoice::IsCheatOn());
	HOLFactionChoice::SetCheatOn(bWas);

	HOLFactionChoice::FClickCounter Clicks;
	TestFalse(TEXT("one click"), Clicks.Click(0.0));
	TestFalse(TEXT("two clicks"), Clicks.Click(0.3));
	TestTrue(TEXT("three quick clicks are a triple"), Clicks.Click(0.6));
	TestFalse(TEXT("a fourth does not make another"), Clicks.Click(0.7));

	HOLFactionChoice::FClickCounter Slow;
	TestFalse(TEXT("slow: 1"), Slow.Click(0.0));
	TestFalse(TEXT("slow: 2 (the gap 0.9 s is over the window)"), Slow.Click(0.9));
	TestFalse(TEXT("slow: 3 (counted from the second)"), Slow.Click(1.2));
	TestTrue(TEXT("slow: three within the window after the restart"), Slow.Click(1.5));

	HOLFactionChoice::FClickCounter Six;
	int32 Triples = 0;
	for (int32 Click = 0; Click < 6; ++Click)
	{
		Triples += Six.Click(0.1 * Click) ? 1 : 0;
	}
	TestEqual(TEXT("six quick clicks are two triples"), Triples, 2);

	HOLFactionChoice::FClickCounter Edge;
	Edge.Click(0.0);
	Edge.Click(0.8);
	TestTrue(TEXT("a gap of exactly the window still counts"), Edge.Click(1.6));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionChoiceBodyTest, "HeroesOfLiberty.Net.FactionChoice.Bodies", UE::HeroesOfLiberty::Tests::FactionChoice::Flags)
bool FHOLFactionChoiceBodyTest::RunTest(const FString& Parameters)
{
	// The lobby's lines carry the faction when asked, as the manager's contract says (a string, the enum's name).
	TestTrue(TEXT("my line: faction only"), FHOLManagerClientRules::MakeMeBody({}, {}, {}, {}, TEXT("Templars")).Contains(TEXT("\"faction\":\"Templars\"")));
	TestFalse(TEXT("my line: no faction when none is given"), FHOLManagerClientRules::MakeMeBody(4, 2, true).Contains(TEXT("faction")));
	TestTrue(TEXT("an AI's line: faction"), FHOLManagerClientRules::MakeAISlotBody({}, {}, TEXT("BlackLegions")).Contains(TEXT("\"faction\":\"BlackLegions\"")));
	TestFalse(TEXT("an AI's line: none"), FHOLManagerClientRules::MakeAISlotBody(5, 2).Contains(TEXT("faction")));

	FHOLLobby Lobby;
	const FString Json = TEXT("{\"id\":\"l1\",\"name\":\"n\",\"slots\":[{\"index\":0,\"state\":\"player\",\"playerName\":\"Bob\",\"colorIndex\":0,\"team\":0,\"faction\":\"Templars\",\"ready\":false}]}");
	if (FHOLManagerClientRules::ParseLobby(Json, Lobby) && Lobby.Slots.Num() > 0)
	{
		EHOLFaction Read = EHOLFaction::DawnOrder;
		TestTrue(TEXT("the lobby's faction string reads back"), HOLFactionChoice::TryFindByName(Lobby.Slots[0].Faction, Read) && Read == EHOLFaction::Templars);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
