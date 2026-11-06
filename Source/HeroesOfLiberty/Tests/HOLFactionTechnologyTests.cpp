// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLFactionRules.h"
#include "Misc/AutomationTest.h"
#include "Technologies/HOLTechnologyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// L2 (LegionsInProgress.md): a technology of the Dawn (TemperedHalberds, `Faction` = Dawn) is not offered to the Legions, on the real
// technology table and the real faction tables; the technologies of every faction (`Common`) stay in both grids. The grid reads
// GetBuildingTechnologies with the building owner's faction key, the server refuses the order with IsTechnologyOpenTo.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLFactionTechnologyGridTest, "HeroesOfLiberty.Match.Factions.TechnologyGrid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLFactionTechnologyGridTest::RunTest(const FString& Parameters)
{
	const FName Halberds(TEXT("TemperedHalberds"));
	const FName Dawn(TEXT("Dawn"));
	const FName Legions(TEXT("Legions"));
	if (!TestNotNull(TEXT("the technology table is loaded"), UHOLTechnologySettings::GetTable()) || !TestNotNull(TEXT("TemperedHalberds is a row"), UHOLTechnologySettings::FindTechnology(Halberds)))
	{
		return false;
	}
	TArray<FName> Names;
	UHOLTechnologySettings::GetBuildingTechnologies(TEXT("Barracks"), Dawn, Names);
	TestTrue(TEXT("the Dawn's Barracks research the halberds"), Names.Contains(Halberds));
	UHOLTechnologySettings::GetBuildingTechnologies(TEXT("Barracks"), Legions, Names);
	TestFalse(TEXT("the Legions' Barracks do not offer the halberds (a technology of the Dawn)"), Names.Contains(Halberds));
	UHOLTechnologySettings::GetBuildingTechnologies(TEXT("Barracks"), NAME_None, Names);
	TestTrue(TEXT("no faction: nothing is filtered"), Names.Contains(Halberds));

	// A technology of every faction stays in the grid of both.
	TArray<FName> DawnForge;
	TArray<FName> LegionsForge;
	UHOLTechnologySettings::GetBuildingTechnologies(TEXT("Forge"), Dawn, DawnForge);
	UHOLTechnologySettings::GetBuildingTechnologies(TEXT("Forge"), Legions, LegionsForge);
	TestTrue(TEXT("the Forge has technologies"), DawnForge.Num() > 0);
	TestEqual(TEXT("the Forge's technologies are the same for both factions (Common)"), LegionsForge, DawnForge);

	// The server's question: the same answer on the same table.
	TestTrue(TEXT("the Dawn may research the halberds"), HOLFactionRules::IsTechnologyOpenTo(Halberds, Dawn));
	TestFalse(TEXT("the Legions may not"), HOLFactionRules::IsTechnologyOpenTo(Halberds, Legions));
	TestTrue(TEXT("a Common technology is open to the Legions"), DawnForge.Num() > 0 && HOLFactionRules::IsTechnologyOpenTo(DawnForge[0], Legions));
	TestTrue(TEXT("a technology the table does not know is left open"), HOLFactionRules::IsTechnologyOpenTo(TEXT("NoSuchTechnology"), Legions));
	return true;
}

#endif
