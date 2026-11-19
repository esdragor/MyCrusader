// Compiled and RED then GREEN seen 2026-11-19 11:34 (GALADRIEL-1119w; the C++ test and HOLScenarioPanel.* are deleted, GALADRIEL-1119x). Script equivalent of the English and Japanese part of the C++ test FHOLScenarioPanelTextTest
// "HeroesOfLiberty.UI.ScenarioPanel.LocalizedOneLine" (HOLScenarioPanelTests.cpp:91-142), the part HOLScenarioPanelScript_Test.as:8-9 marked NOT PORTED. 
// HOLScenarioPanel.* and that C++ test are deleted. Needs the binding HOLScreenTest::GetLocalizedTextIn (HOLScreenTestLibrary.h): the FText the SCRIPT panel gives its title and state
// (HOLScenarioPanelRules::TitleText / PreparingText / ReadyText, the very ones HOLScenarioPanelScript.as's Refresh gives the widgets), translated through
// Content/Localization/Game/<culture>/Game.locres by the namespace and key it carries; a formatted title has its keyed pattern translated and its argument kept.
// Expected values = the C++ test's, word for word. RED TO SEE: falsify "preparing…" (en) into "preparing...", run, restore.

void Test_ScenarioPanelTitleAndStatesInEnglish(FUnitTest& T)
{
	T.AssertEquals("Scenario deploy", HOLScreenTest::GetLocalizedTextIn("en", HOLScenarioPanelRules::TitleText("deploy")), "en: the title");
	T.AssertEquals("preparing…", HOLScreenTest::GetLocalizedTextIn("en", HOLScenarioPanelRules::PreparingText()), "en: the state preparing");
	T.AssertEquals("ready", HOLScreenTest::GetLocalizedTextIn("en", HOLScenarioPanelRules::ReadyText()), "en: the state ready");
}

void Test_ScenarioPanelTitleAndStatesInJapanese(FUnitTest& T)
{
	T.AssertEquals("シナリオ deploy", HOLScreenTest::GetLocalizedTextIn("ja", HOLScenarioPanelRules::TitleText("deploy")), "ja: the title");
	T.AssertEquals("準備中…", HOLScreenTest::GetLocalizedTextIn("ja", HOLScenarioPanelRules::PreparingText()), "ja: the state preparing");
	T.AssertEquals("準備完了", HOLScreenTest::GetLocalizedTextIn("ja", HOLScenarioPanelRules::ReadyText()), "ja: the state ready");
}

void Test_ScenarioPanelTranslationsAreNotTheFrenchSource(FUnitTest& T)
{
	// C++ test: TestNotEqual(translated, the French source shown), per culture, for the title and each state.
	const FString FrenchTitle = HOLScenarioPanelRules::TitleText("deploy").ToString();
	const FString FrenchPreparing = HOLScenarioPanelRules::PreparingText().ToString();
	const FString FrenchReady = HOLScenarioPanelRules::ReadyText().ToString();
	TArray<FString> Cultures;
	Cultures.Add("en");
	Cultures.Add("ja");
	for (int Index = 0; Index < Cultures.Num(); Index++)
	{
		const FString Culture = Cultures[Index];
		T.AssertTrue(HOLScreenTest::GetLocalizedTextIn(Culture, HOLScenarioPanelRules::TitleText("deploy")) != FrenchTitle, Culture + ": the title is not the French source");
		T.AssertTrue(HOLScreenTest::GetLocalizedTextIn(Culture, HOLScenarioPanelRules::PreparingText()) != FrenchPreparing, Culture + ": preparing is not the French source");
		T.AssertTrue(HOLScreenTest::GetLocalizedTextIn(Culture, HOLScenarioPanelRules::ReadyText()) != FrenchReady, Culture + ": ready is not the French source");
	}
}
