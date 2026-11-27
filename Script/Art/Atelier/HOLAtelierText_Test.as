// HALDIR-1127ar3: the atelier's messages are built from a LOCALIZED pattern ("{0} : {1}" fr, "{0}: {1}" en, "{0}：{1}" ja; "Envoi écrit dans {0}"),
// never with " : " or a space glued in script (HOLAtelierScreen.as ChooseType, LoadModel, PickModel, Save). Through the chain: the pattern's
// translation read from Content/Localization/Game/<culture>/Game.locres by its namespace and key (HOLScreenTest::GetLocalizedTextIn), filled by
// HOLAtelierText::Fill, the function the screen's messages use.
// RED while the key "Pair" does not exist and "Saved" has no {0}. To SEE RED again: make HOLAtelierText::Get("Pair") fall through to FromString.

// HeroesOfLiberty.Atelier.MessagePatterns
void Test_AtelierMessagePatterns(FUnitTest& T)
{
	const FString PairEn = HOLAtelierText::Fill(HOLScreenTest::GetLocalizedTextIn("en", HOLAtelierText::Get("Pair")), "House", "x");
	T.AssertEquals("House: x", PairEn, "en: name and reason, no space before the colon");
	const FString PairJa = HOLAtelierText::Fill(HOLScreenTest::GetLocalizedTextIn("ja", HOLAtelierText::Get("Pair")), "House", "x");
	T.AssertEquals("House：x", PairJa, "ja: full-width colon, no space after it");
	const FString SavedJa = HOLAtelierText::Fill(HOLScreenTest::GetLocalizedTextIn("ja", HOLAtelierText::Get("Saved")), "C:/Envois/House");
	T.AssertEquals("送付フォルダーを書き出しました：C:/Envois/House", SavedJa, "ja: the folder right after the colon");
	const FString SavedEn = HOLAtelierText::Fill(HOLScreenTest::GetLocalizedTextIn("en", HOLAtelierText::Get("Saved")), "C:/Envois/House");
	T.AssertEquals("Submission written to C:/Envois/House", SavedEn, "en: the folder after the words");
	// The game's culture in the tests (French, the source): the French separator, through the function the screen calls.
	const FString PairFr = HOLAtelierText::Format("Pair", "House", "x");
	T.AssertEquals("House : x", PairFr, "fr: the French separator");
}

// HALDIR-1127ar6: the atelier's status row shows a word of the culture, not the raw manifest value ("provisional" read in French and
// Japanese before). Through the chain: HOLAtelierText::StatusLabel (the function ShowValues and the status button call), its
// translation read from Content/Localization/Game/<culture>/Game.locres. The manifest keeps the raw value (Test_AtelierManifest).
// RED while StatusLabel returns the raw value. To SEE RED again: make StatusLabel return FText::FromString(Status).
// HeroesOfLiberty.Atelier.StatusLabels
void Test_AtelierStatusLabels(FUnitTest& T)
{
	const FString ProvisionalFr = HOLAtelierText::StatusLabel("provisional").ToString();
	T.AssertEquals("provisoire", ProvisionalFr, "fr (the tests' culture): the French word");
	const FString FinalFr = HOLAtelierText::StatusLabel("final").ToString();
	T.AssertEquals("final", FinalFr, "fr: final");
	const FString ProvisionalEn = HOLScreenTest::GetLocalizedTextIn("en", HOLAtelierText::StatusLabel("provisional"));
	T.AssertEquals("provisional", ProvisionalEn, "en: provisional");
	const FString ProvisionalJa = HOLScreenTest::GetLocalizedTextIn("ja", HOLAtelierText::StatusLabel("provisional"));
	T.AssertEquals("暫定", ProvisionalJa, "ja: provisional");
	const FString FinalJa = HOLScreenTest::GetLocalizedTextIn("ja", HOLAtelierText::StatusLabel("final"));
	T.AssertEquals("確定", FinalJa, "ja: final");
	const FString Other = HOLAtelierText::StatusLabel("draft").ToString();
	T.AssertEquals("draft", Other, "another value: shown as it is");
}
