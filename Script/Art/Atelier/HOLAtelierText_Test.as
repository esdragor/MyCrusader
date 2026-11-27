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
