// GALADRIEL-1121sg: the cheats' labels ("Vitesse des unités : x1.0", "Triches  +", ...) were written only on a click, so after a live language switch they kept the old language.
// The rule HOLCheatPanelRules::LabelsStale compares what is shown with what the current language would show; the texts are passed in, so no test depends on the editor's real language.

void Test_CheatPanelLabelsAreStaleWhenOnlyTheLanguageChanged(FUnitTest& T)
{
	// The same state (toggle folded, fast build off, speed x1.0) shown in French, then asked in English: nothing but the text changed.
	const FString Shown = HOLCheatPanelRules::LabelsSignature("Triches  +", "Construction rapide : non", "Vitesse des unités : x1.0", "Vitesse de collecte : x1.0");
	const bool bStale = HOLCheatPanelRules::LabelsStale(Shown, "Cheats  +", "Fast build: off", "Unit speed: x1.0", "Gather speed: x1.0");
	T.AssertTrue(bStale, "a language switch alone makes the labels stale");
}

void Test_CheatPanelLabelsAreNotStaleWhenNothingChanged(FUnitTest& T)
{
	const FString Shown = HOLCheatPanelRules::LabelsSignature("Cheats  +", "Fast build: off", "Unit speed: x1.0", "Gather speed: x1.0");
	const bool bStale = HOLCheatPanelRules::LabelsStale(Shown, "Cheats  +", "Fast build: off", "Unit speed: x1.0", "Gather speed: x1.0");
	T.AssertFalse(bStale, "the same texts: no rewrite");
}

void Test_CheatPanelLabelsAreStaleWhenTheScaleChanged(FUnitTest& T)
{
	const FString Shown = HOLCheatPanelRules::LabelsSignature("Cheats  +", "Fast build: off", "Unit speed: x1.0", "Gather speed: x1.0");
	const bool bStale = HOLCheatPanelRules::LabelsStale(Shown, "Cheats  +", "Fast build: off", "Unit speed: x1.0", "Gather speed: x2.5");
	T.AssertTrue(bStale, "a changed value is stale too");
}

// The real chain: the panel's own texts (HOLCheatPanelRules::SpeedLabelText) joined by the rule give a signature that moves when the scale moves.
void Test_CheatPanelSignatureFollowsTheRealSpeedLabel(FUnitTest& T)
{
	const FString Unit1 = HOLCheatPanelRules::SpeedLabelText(0, 1.0);
	const FString Unit2 = HOLCheatPanelRules::SpeedLabelText(0, 2.5);
	const FString Gather = HOLCheatPanelRules::SpeedLabelText(1, 1.0);
	const FString Before = HOLCheatPanelRules::LabelsSignature("t", "f", Unit1, Gather);
	const bool bStale = HOLCheatPanelRules::LabelsStale(Before, "t", "f", Unit2, Gather);
	T.AssertTrue(bStale, "the real label for x2.5 differs from the one for x1.0");
}
