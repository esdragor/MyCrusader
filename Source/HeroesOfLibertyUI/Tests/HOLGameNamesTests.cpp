#include "HUD/HOLGameNames.h"
#include "Match/HOLFactionRules.h"
#include "Internationalization/TextKey.h"
#include "Internationalization/Text.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/** The game's language for a test, the machine's put back after. */
	struct FScopedLanguage
	{
		FString Previous;
		explicit FScopedLanguage(const TCHAR* Culture)
		{
			Previous = FInternationalization::Get().GetCurrentCulture()->GetName();
			FInternationalization::Get().SetCurrentCulture(Culture);
		}
		~FScopedLanguage() { FInternationalization::Get().SetCurrentCulture(Previous); }
	};
}

// L2 (LegionsInProgress.md): the UI takes its names by type key; a faction with `DisplayName` overrides (the Legions, FactionOverrides.csv)
// has its own key per type. Every DisplayName row of the real table must read back as that faction's name, and the Dawn and the
// unfactioned call keep the base names.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameNamesFactionTest, "HeroesOfLiberty.UI.GameNames.FactionNames",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGameNamesFactionTest::RunTest(const FString& Parameters)
{
	// The table's DisplayName is the French source text: the check reads the French names (the language of the machine put back after).
	const FScopedLanguage French(TEXT("fr"));
	const FName Legions(TEXT("Legions"));
	const FName Dawn(TEXT("Dawn"));
	const FName Spearman(TEXT("Spearman"));
	TestEqual(TEXT("the Legions' Spearman"), HOLGameNames::GetUnitName(Spearman, nullptr, Legions).ToString(), FString(TEXT("Lancier squelette")));
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

// The Legions' seven names in French (source), English and Japanese (LegionsInProgress.md, user answers of 2026-11-07): the text the
// screen shows follows the language; the Dawn keeps its own names in French (Lancier, Homme d'armes, Archer, Arbalétrier), and in each
// language the Legions' name differs from the Dawn's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameNamesLegionsLanguagesTest, "HeroesOfLiberty.UI.GameNames.LegionsLanguages",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGameNamesLegionsLanguagesTest::RunTest(const FString& Parameters)
{
	const FName Legions(TEXT("Legions"));
	const FName Dawn(TEXT("Dawn"));
	const TCHAR* const Types[] = { TEXT("Spearman"), TEXT("ManAtArms"), TEXT("Archer"), TEXT("Crossbowman"), TEXT("LightCavalry"), TEXT("Ram"), TEXT("Mangonel") };
	const TCHAR* const French[] = { TEXT("Lancier squelette"), TEXT("Garde noire"), TEXT("Archer squelette"), TEXT("Arbalétrier squelette"),
		TEXT("Cavalier funeste"), TEXT("Bélier d'os"), TEXT("Mangonneau d'os") };
	const TCHAR* const English[] = { TEXT("Skeleton Spearman"), TEXT("Black Guard"), TEXT("Skeleton Archer"), TEXT("Skeleton Crossbowman"),
		TEXT("Dread Rider"), TEXT("Bone Ram"), TEXT("Bone Mangonel") };
	const TCHAR* const Japanese[] = { TEXT("スケルトン槍兵"), TEXT("黒衛兵"), TEXT("スケルトン弓兵"),
		TEXT("スケルトン弩兵"), TEXT("恐怖の騎兵"), TEXT("骨の破城槌"),
		TEXT("骨のマンゴネル") };
	const TCHAR* const* const Expected[] = { French, English, Japanese };
	const TCHAR* const Cultures[] = { TEXT("fr"), TEXT("en"), TEXT("ja") };
	for (int32 Language = 0; Language < 3; ++Language)
	{
		const FScopedLanguage Scoped(Cultures[Language]);
		for (int32 Index = 0; Index < 7; ++Index)
		{
			const FName Type(Types[Index]);
			const FString Own = HOLGameNames::GetUnitName(Type, nullptr, Legions).ToString();
			TestEqual(*FString::Printf(TEXT("%s of the Legions in %s"), Types[Index], Cultures[Language]), Own, FString(Expected[Language][Index]));
			TestNotEqual(*FString::Printf(TEXT("%s: the Legions' name is not the Dawn's in %s"), Types[Index], Cultures[Language]),
				Own, HOLGameNames::GetUnitName(Type, nullptr, Dawn).ToString());
		}
	}
	// The Dawn keeps its names (French).
	{
		const FScopedLanguage Scoped(TEXT("fr"));
		const TCHAR* const DawnNames[][2] = { { TEXT("Spearman"), TEXT("Lancier") }, { TEXT("ManAtArms"), TEXT("Homme d'armes") },
			{ TEXT("Archer"), TEXT("Archer") }, { TEXT("Crossbowman"), TEXT("Arbalétrier") } };
		for (const TCHAR* const* Pair : DawnNames)
		{
			TestEqual(*FString::Printf(TEXT("%s of the Dawn keeps its name"), Pair[0]), HOLGameNames::GetUnitName(FName(Pair[0]), nullptr, Dawn).ToString(), FString(Pair[1]));
		}
	}
	return true;
}

// F1: the Forge has its own localization key (the .po files carry `HOLGameNames,Forge`), so its name follows the language; with no row, the
// name still comes from the code, not from the table's French DisplayName.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLGameNamesForgeTest, "HeroesOfLiberty.UI.GameNames.ForgeKey",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLGameNamesForgeTest::RunTest(const FString& Parameters)
{
	const FText Name = HOLGameNames::GetBuildingName(TEXT("Forge"), nullptr);
	TestEqual(TEXT("the Forge's name"), Name.ToString(), FString(TEXT("Forge")));
	TestEqual(TEXT("its localization key"), FTextInspector::GetKey(Name).Get(FString()), FString(TEXT("Forge")));
	TestEqual(TEXT("its localization namespace"), FTextInspector::GetNamespace(Name).Get(FString()), FString(TEXT("HOLGameNames")));
	return true;
}

#endif
