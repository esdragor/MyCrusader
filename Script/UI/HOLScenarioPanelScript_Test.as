// HUD H-11b (NOT COMPILED, needs the same C++ lot as HOLScenarioPanelScript.as for the class; the rules of the namespace HOLScenarioPanelRules need none,
// except NSLOCTEXT / FText::Format in script, to verify at the first compile). Port of the former HOLScenarioPanelTests.cpp (removed, GALADRIEL-1119x).
//
// C++ test -> script equivalent:
//   FHOLScenarioPanelTextTest "HeroesOfLiberty.UI.ScenarioPanel.LocalizedOneLine":
//     - the French part (title "Scénario deploy", state "en préparation…")        -> Test_ScenarioPanelFrenchTexts
//     - "the state does not wrap while preparing / once ready, an error wraps"    -> Test_ScenarioPanelStateWrapsOnlyAnError
//     - the English and Japanese part (Game.locres read by namespace + key)       -> HOLScenarioPanelLocalizedScript_Test.as (GetLocalizedTextIn reads the .locres):
//       red seen 2026-11-19 11:34, then green; the C++ test is removed.
//   FHOLCheatPanelTextTest "HeroesOfLiberty.UI.CheatPanel.Localized": the cheats' panel, removed with this file: -> HOLCheatPanelLocalizedScript_Test.as.
// Added, one per rule of HOLScenarioPanel.cpp the C++ test did not cover: shown rule, Describe line, journal lines (read by LaunchCheck.ps1), click routing,
// anchor placement, notes rebuilt only when changed.

void Test_ScenarioPanelFrenchTexts(FUnitTest& T)
{
	// = fr part of FHOLScenarioPanelTextTest (the language of the test run is French, the source language).
	const FString Title = HOLScenarioPanelRules::TitleText("deploy").ToString();
	T.AssertEquals("Scénario deploy", Title, "fr: the title");
	const FString Preparing = HOLScenarioPanelRules::StateText(HOLScenarioPanelRules::StatePreparing(), "").ToString();
	T.AssertEquals("en préparation…", Preparing, "fr: the state while preparing");
	const FString Ready = HOLScenarioPanelRules::StateText(HOLScenarioPanelRules::StateReady(), "").ToString();
	T.AssertEquals("prêt", Ready, "fr: the state once ready");
	const FString Failed = HOLScenarioPanelRules::StateText(HOLScenarioPanelRules::StateFailed(), "étape 3 jamais venue").ToString();
	T.AssertEquals("erreur : étape 3 jamais venue", Failed, "fr: the state of a failure carries the error");
	const FString Restart = HOLScenarioPanelRules::RestartLabel().ToString();
	T.AssertEquals("Relancer", Restart, "fr: the restart button's label");
	T.AssertEquals("×", HOLScenarioPanelRules::CloseLabel(), "the close button's label");
}

void Test_ScenarioPanelStateWrapsOnlyAnError(FUnitTest& T)
{
	// = the one-line part of FHOLScenarioPanelTextTest (3 TestFalse / TestTrue on GetAutoWrapText).
	T.AssertFalse(HOLScenarioPanelRules::StateWraps(HOLScenarioPanelRules::StatePreparing()), "the state does not wrap while preparing");
	T.AssertFalse(HOLScenarioPanelRules::StateWraps(HOLScenarioPanelRules::StateReady()), "the state does not wrap once ready");
	T.AssertTrue(HOLScenarioPanelRules::StateWraps(HOLScenarioPanelRules::StateFailed()), "an error wraps");
}

void Test_ScenarioPanelShownRule(FUnitTest& T)
{
	// Refresh (cpp:96): bShown = !Name.IsEmpty() && !bClosed && !bHidden.
	T.AssertTrue(HOLScenarioPanelRules::IsShown("deploy", false, false), "a scenario, not closed, not hidden: shown");
	T.AssertFalse(HOLScenarioPanelRules::IsShown("", false, false), "before any scenario: hidden");
	T.AssertFalse(HOLScenarioPanelRules::IsShown("deploy", true, false), "closed by the player: hidden until the next scenario");
	T.AssertFalse(HOLScenarioPanelRules::IsShown("deploy", false, true), "hidden by a `panel hide` step");
}

void Test_ScenarioPanelDescribeIsTheCppLine(FUnitTest& T)
{
	// Describe (cpp:169-186), word for word: hol.HUD.Report and the journal line quick_check / LaunchCheck read.
	T.AssertEquals("no scenario", HOLScenarioPanelRules::Describe("", false, 0, 0, ""), "no scenario");
	T.AssertEquals("scenario order_lines closed", HOLScenarioPanelRules::Describe("order_lines", true, 1, 3, ""), "closed wins over the state");
	T.AssertEquals("scenario order_lines preparing, 0 notes", HOLScenarioPanelRules::Describe("order_lines", false, 0, 0, ""), "preparing");
	T.AssertEquals("scenario order_lines ready, 3 notes", HOLScenarioPanelRules::Describe("order_lines", false, 1, 3, ""), "ready, 3 notes");
	T.AssertEquals("scenario order_lines ready, 3 notes, waiting for the key", HOLScenarioPanelRules::Describe("order_lines", false, 1, 3, "Entrée"), "waiting for the key");
	T.AssertEquals("scenario legions_roster_gesture failed, 0 notes", HOLScenarioPanelRules::Describe("legions_roster_gesture", false, 2, 0, ""), "failed (the line of launch_check_test.ps1:35)");
}

void Test_ScenarioPanelJournalLinesAreTheCppOnes(FUnitTest& T)
{
	// The lines are written through UHOLGameMenuScreen::MenuLog (level Display): LaunchCheck.ps1:86 matches 'Scenario panel: scenario \S+ failed'.
	const FString Line = HOLScenarioPanelRules::RefreshLogLine(HOLScenarioPanelRules::Describe("deploy", false, 2, 1, ""));
	T.AssertEquals("Scenario panel: scenario deploy failed, 1 notes", Line, "the refresh line carries the Describe text, 'failed' included");
	T.AssertEquals("HUD: scenario panel closed", HOLScenarioPanelRules::ClosedLogLine(), "the close's line");
	T.AssertEquals("HUD: scenario deploy restarted", HOLScenarioPanelRules::RestartedLogLine("deploy"), "the restart's line");
	T.AssertEquals("hol.Scenario deploy", HOLScenarioPanelRules::RestartCommand("deploy"), "the same command as the console's");
}

void Test_ScenarioPanelClickRouting(FUnitTest& T)
{
	// ClickWidget (cpp:199-218): hol.HUD.Click ScenarioClose / ScenarioRestart; refused when collapsed or for another name.
	T.AssertEquals(2, HOLScenarioPanelRules::ClickKind(true, n"ScenarioClose"), "ScenarioClose closes");
	T.AssertEquals(1, HOLScenarioPanelRules::ClickKind(true, n"ScenarioRestart"), "ScenarioRestart restarts");
	T.AssertEquals(0, HOLScenarioPanelRules::ClickKind(true, n"AlertPanel"), "another widget's name: refused");
	T.AssertEquals(0, HOLScenarioPanelRules::ClickKind(false, n"ScenarioClose"), "collapsed panel: refused");
}

void Test_ScenarioPanelAnchorAndNotes(FUnitTest& T)
{
	// NativeTick (cpp:76-83): 16 margin, under the anchor's desired height + 6 gap; the margin alone while the top bar is absent.
	T.AssertEquals(16.0, HOLScenarioPanelRules::PanelTop(0.0), "no top bar: the margin");
	T.AssertEquals(16.0 + 72.0 + 6.0, HOLScenarioPanelRules::PanelTop(72.0), "under a 72 px top bar");
	// Refresh (cpp:147): the notes are rebuilt only when they differ.
	TArray<FString> A;
	A.Add("one");
	A.Add("two");
	TArray<FString> B;
	B.Add("one");
	B.Add("two");
	T.AssertTrue(HOLScenarioPanelRules::NotesEqual(A, B), "same notes: kept");
	B[1] = "deux";
	T.AssertFalse(HOLScenarioPanelRules::NotesEqual(A, B), "a note changed: rebuilt");
	B.RemoveAt(1);
	T.AssertFalse(HOLScenarioPanelRules::NotesEqual(A, B), "a note less: rebuilt");
	T.AssertFalse(HOLScenarioPanelRules::NotesColumnVisible(0), "no note: the column is collapsed");
	T.AssertTrue(HOLScenarioPanelRules::NotesColumnVisible(2), "notes: the column shows");
	T.AssertFalse(HOLScenarioPanelRules::WaitKeyVisible(""), "no waitkey: the line is collapsed");
	T.AssertTrue(HOLScenarioPanelRules::WaitKeyVisible("Entrée pour continuer"), "a step waits for the key: the line shows");
}
