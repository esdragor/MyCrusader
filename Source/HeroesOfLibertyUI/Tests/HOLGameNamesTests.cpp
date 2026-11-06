#include "HUD/HOLGameNames.h"
#include "Match/HOLFactionRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// L2 (LegionsInProgress.md): the UI takes its names by type key; a faction with `DisplayName` overrides (the Legions, FactionOverrides.csv)
// has its own key per type. Every DisplayName row of the real table must read back as that faction's name, and the Dawn and the
// unfactioned call keep the base names.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameNamesFactionTest, "HeroesOfLiberty.UI.GameNames.FactionNames",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGameNamesFactionTest::RunTest(const FString& Parameters)
{
	const FName Legions(TEXT("Legions"));
	const FName Dawn(TEXT("Dawn"));
	const FName Spearman(TEXT("Spearman"));
	TestEqual(TEXT("the Legions' Spearman"), HOLGameNames::GetUnitName(Spearman, nullptr, Legions).ToString(), FString(TEXT("Legion Spearman")));
	TestEqual(TEXT("the Dawn's Spearman keeps the base name"), HOLGameNames::GetUnitName(Spearman, nullptr, Dawn).ToString(), HOLGameNames::GetUnitName(Spearman, nullptr).ToString());
	TestEqual(TEXT("no faction keeps the base name"), HOLGameNames::GetUnitName(Spearman, nullptr, NAME_None).ToString(), HOLGameNames::GetUnitName(Spearman, nullptr).ToString());
	TestNotEqual(TEXT("the Legions' name is not the Dawn's"), HOLGameNames::GetUnitName(Spearman, nullptr, Legions).ToString(), HOLGameNames::GetUnitName(Spearman, nullptr, Dawn).ToString());

	// Every name the data gives a faction is shown: the texts cannot drift from the table.
	int32 Names = 0;
	for (const HOLFactionRules::FOverrideRow& Row : HOLFactionRules::GetTable().Overrides)
	{
		if (Row.Field == TEXT("DisplayName"))
		{
			++Names;
			TestEqual(*FString::Printf(TEXT("%s of %s"), *Row.Type.ToString(), *Row.Faction.ToString()),
				HOLGameNames::GetUnitName(Row.Type, nullptr, Row.Faction).ToString(), Row.Value);
		}
	}
	TestEqual(TEXT("the Legions' seven names are in the table"), Names, 7);
	return true;
}

#endif
