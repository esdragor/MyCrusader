// Compiled and RED then GREEN seen 2026-11-19 11:34 (GALADRIEL-1119w; the C++ test and HOLCheatPanel.* are deleted, GALADRIEL-1119x). Script equivalent of the C++ test FHOLCheatPanelTextTest "HeroesOfLiberty.UI.CheatPanel.Localized" (HOLScenarioPanelTests.cpp:164-286),
// Needs the binding HOLScreenTest::GetLocalizedTextIn (HOLScreenTestLibrary.h): the text the SCRIPT panel
// uses (HOLCheatPanelText::*, the very FText that HOLCheatPanelScript.as gives the buttons / labels), translated through Content/Localization/Game/<culture>/Game.locres by the
// namespace and key that FText carries. A label left without a key (FromString, INVTEXT) or with another key than the C++ one gives an empty string and fails here.
// Expected values = the C++ test's, word for word. RED TO SEE: falsify "+1000 Wood" (CheatWood, en) into "+1000 Woodx", run, restore.
//
// Not covered here, as in the C++ test: French (the widgets' own text; the fr strings of HOLCheatPanelText are the source literals, read by the existing script tests).

FString CheatLocalized(const FString& Culture, const FText& Shown)
{
	return HOLScreenTest::GetLocalizedTextIn(Culture, Shown);
}

// "Vitesse des unités : x1.0" in a culture: the panel's own pattern and name (HOLCheatPanelText::SpeedLabel / UnitSpeed / GatherSpeed) each translated by its key, the value kept,
// composed the way HOLCheatPanelRules::SpeedLabelText composes them.
FString CheatSpeedLocalized(const FString& Culture, int Index)
{
	const FText NameText = Index == 0 ? HOLCheatPanelText::UnitSpeed() : HOLCheatPanelText::GatherSpeed();
	const FString Pattern = CheatLocalized(Culture, HOLCheatPanelText::SpeedLabel());
	const FString Name = CheatLocalized(Culture, NameText);
	return Pattern.Replace("{0}", Name).Replace("{1}", HOLCheatPanelRules::OneDecimal(1.0));
}

void Test_CheatPanelButtonsAreTranslatedInEnglish(FUnitTest& T)
{
	// C++ test, buttons: { widget, fr, en, ja }. The sixth button (index 6) shows FastBuildOff at start.
	T.AssertEquals("+1000 Food", CheatLocalized("en", HOLCheatPanelText::CheatLabel(0)), "en: CheatFood");
	T.AssertEquals("+1000 Wood", CheatLocalized("en", HOLCheatPanelText::CheatLabel(1)), "en: CheatWood");
	T.AssertEquals("+1000 Stone", CheatLocalized("en", HOLCheatPanelText::CheatLabel(2)), "en: CheatStone");
	T.AssertEquals("+1000 Gold", CheatLocalized("en", HOLCheatPanelText::CheatLabel(3)), "en: CheatGold");
	T.AssertEquals("+1000 of everything", CheatLocalized("en", HOLCheatPanelText::CheatLabel(4)), "en: CheatAll");
	T.AssertEquals("Population +165", CheatLocalized("en", HOLCheatPanelText::CheatLabel(5)), "en: CheatPopulation");
	T.AssertEquals("Fast build: off", CheatLocalized("en", HOLCheatPanelText::FastBuildOff()), "en: CheatFastBuild");
}

void Test_CheatPanelButtonsAreTranslatedInJapanese(FUnitTest& T)
{
	T.AssertEquals("+1000 食料", CheatLocalized("ja", HOLCheatPanelText::CheatLabel(0)), "ja: CheatFood");
	T.AssertEquals("+1000 木材", CheatLocalized("ja", HOLCheatPanelText::CheatLabel(1)), "ja: CheatWood");
	T.AssertEquals("+1000 石材", CheatLocalized("ja", HOLCheatPanelText::CheatLabel(2)), "ja: CheatStone");
	T.AssertEquals("+1000 金", CheatLocalized("ja", HOLCheatPanelText::CheatLabel(3)), "ja: CheatGold");
	T.AssertEquals("全資源 +1000", CheatLocalized("ja", HOLCheatPanelText::CheatLabel(4)), "ja: CheatAll");
	T.AssertEquals("人口 +165", CheatLocalized("ja", HOLCheatPanelText::CheatLabel(5)), "ja: CheatPopulation");
	T.AssertEquals("高速建設：オフ", CheatLocalized("ja", HOLCheatPanelText::FastBuildOff()), "ja: CheatFastBuild");
}

void Test_CheatPanelSpeedLabelsAreTranslated(FUnitTest& T)
{
	// C++ test, speeds: the pattern "{0} : x{1}" and the name by their keys, the value (1.0) kept.
	T.AssertEquals("Unit speed: x1.0", CheatSpeedLocalized("en", 0), "en: CheatUnitSpeedLabel");
	T.AssertEquals("Gather speed: x1.0", CheatSpeedLocalized("en", 1), "en: CheatGatherSpeedLabel");
	T.AssertEquals("ユニット速度：x1.0", CheatSpeedLocalized("ja", 0), "ja: CheatUnitSpeedLabel");
	T.AssertEquals("採集速度：x1.0", CheatSpeedLocalized("ja", 1), "ja: CheatGatherSpeedLabel");
}

// The six +1000 buttons' texts in a culture, joined like HOLCheatPanelRules::CheatLabelsText (which reads the CURRENT language).
FString CheatButtonsLocalized(const FString& Culture)
{
	FString Joined = "";
	for (int Index = 0; Index < HOLCheatPanelRules::FastBuildIndex(); Index++)
		Joined += CheatLocalized(Culture, HOLCheatPanelText::CheatLabel(Index)) + "|";
	return Joined;
}

// Real chain, independent of the editor's language: the panel shows the Japanese buttons, the language switches to English, only the six labels differ.
void Test_CheatPanelSignatureSeesAResourceLabelLanguageSwitch(FUnitTest& T)
{
	const FString Ja = CheatButtonsLocalized("ja");
	const FString En = CheatButtonsLocalized("en");
	T.AssertTrue(Ja != En, "the two languages give different labels");
	const FString Shown = HOLCheatPanelRules::LabelsSignature("t", "f", "u", "g", Ja);
	const bool bStaleAfterSwitch = HOLCheatPanelRules::LabelsStale(Shown, "t", "f", "u", "g", En);
	const bool bStaleSame = HOLCheatPanelRules::LabelsStale(Shown, "t", "f", "u", "g", Ja);
	T.AssertTrue(bStaleAfterSwitch, "ja -> en on the six labels alone: stale");
	T.AssertFalse(bStaleSame, "same language: not stale");
	// The panel's own joined text (current language) is what the six labels give through the rule's helper.
	T.AssertTrue(HOLCheatPanelRules::CheatLabelsText().Contains("1000") || HOLCheatPanelRules::CheatLabelsText().Contains("+165"), "the panel's joined labels carry the six buttons");
}

void Test_CheatPanelWoodStoneGoldLabelsInFrench(FUnitTest& T)
{
	// The three buttons HOLCheatPanelScript_Test.as does not cover in French (the C++ test loops on the seven): the source literals of the panel's own texts.
	const FString Wood = HOLCheatPanelText::CheatLabel(1).ToString();
	T.AssertEquals("+1000 Bois", Wood, "fr: CheatWood");
	const FString Stone = HOLCheatPanelText::CheatLabel(2).ToString();
	T.AssertEquals("+1000 Pierre", Stone, "fr: CheatStone");
	const FString Gold = HOLCheatPanelText::CheatLabel(3).ToString();
	T.AssertEquals("+1000 Or", Gold, "fr: CheatGold");
}