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
