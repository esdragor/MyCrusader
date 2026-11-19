# HALDIR-1119u - script corrector, selection block (H-9), reprise 14

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branch selection. Start 09:37:07.

## Passes
1. 09:37 : tests 0/0, scripts did not compile (3 errors, listed below). 18 s.
2. 09:42:38 : tests 1640/1640 PASS (1614 + 26) after the three fixes. 174 s.
3. 09:45:45 : RED SEEN, one value falsified in each of the 3 files: 1637/1640 (N-3), failing:
   - HOLSelection_Test.Test_SelectionNameFollowsConstructionState (line 6, "Maison" -> "MaisonX")
   - HOLSelectionChips_Test.Test_SelectionChipsAllChipFirstThenTheTypesStayInTheControllersOrder (line 20, "Tous (8)" -> "Tous (9)")
   - HOLSelectionUnit_Test.Test_SelectionUnitSortsSoldiersThenWorkersEachByName (line 28, 4 -> 99)
   Values restored with git checkout (nothing falsified remains).
4. 09:48:57 : tests 1640/1640 PASS again. 178 s.

## Errors fixed (commit 3be6faf)
- Script\UI\HOLSelection.as:314:16 "Multiple matching signatures to UCommonTextBlock::SetAutoWrapText(const bool)": the setter is a BlueprintSetter declared in both UTextLayoutWidget and UTextBlock, so the call is ambiguous. Fix: `ActivityText.AutoWrapText = true;` (property, uses the setter).
- Script\UI\HOLSelection.as:499:38 "No matching signatures to HOLSelectionRules::BuildingDetail(..., FString, FText)": FHOLBuildingSelView::ForumText is an FText, the script function takes an FString. Fix: pass `View.ForumText.ToString()`.
- Script\UI\HOLSelectionChips.as:118:9 "No matching signatures to UWrapBox::SetExplicitWrapSize(const bool)": SetExplicitWrapSize / SetWrapSize are not UFUNCTIONs. Fix: `Chips.bExplicitWrapSize = true; Chips.WrapSize = ...;` (SetWrapSize on the next line would have failed the same way).

## Existing tests / C++
No existing test (HUD layout, HUD stats, HUD selection name, C++ HUD tests) went red: 1640/1640. No C++ change needed.

## Remaining errors
None. Last gesture: stray_processes -Stop, no detached process.

## Notes
quick_check script errors are in Saved_Tests\Logs\quick_tests.log of the worktree (not Saved\Logs). Not run: game scenarios (ecran_hud_selection_*), only `-Parts tests`.
