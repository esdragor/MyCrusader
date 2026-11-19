# GALADRIEL-1119u : cheat panel, script made identical to C++ (2026-11-19)

Commit 02bdbaf on galadriel-tab (Script\UI\HOLCheatPanelScript.as only, no C++, no build).

## Causes and fixes
1. Panel 8 px wider, buttons and slider tracks / labels shifted 4 px right, folded button frame 8 px wider: a single cause. The script panel is made by Screen.MakePanel
   (Source\HeroesOfLibertyUI\HUD\HOLScriptScreen.cpp:401, padding FMargin(12, 8)) while the C++ sets Panel->SetPadding(FMargin(8.f)) (HOLCheatPanel.cpp:425). Fix: after MakePanel,
   `Panel.SetPadding(FMargin(8.0));` in Build (HOLCheatPanelScript.as, line after `Panel = Screen.MakePanel(n"CheatPanel")`). The common helper is untouched.
2. Slider handle white instead of ochre: the C++ calls SetSliderBarColor(HOLUIStyle::TextMuted) and SetSliderHandleColor(HOLUIStyle::Accent) (HOLCheatPanel.cpp:474-475); the script did not.
   Fix in MakeSlider: `Slider.SetSliderBarColor(UHOLScriptScreen::GetMutedColor()); Slider.SetSliderHandleColor(UHOLScriptScreen::GetAccentColor());`
   Both bindings already exist (HOLScriptScreen.h:160 and 253) and USlider's setters are script-callable: no missing binding.

## Verification
preflight ecran_hud_triche : the scenario PASSES (the only "FAIL" of the run was the "uncommitted file" guard, before the commit). PNGs copied to scratchpad\apres\ecran_hud_triche\.
Rectangle x 16-232, y 70-440 AVANT vs APRES (recette counted 17 728 on triche_ouverte at threshold 0):
- triche_ouverte: 3 770 pixels differ at threshold 0, none above the recette's threshold 12; max channel difference 4 (translucent panel over the 3D scene, not a layout difference).
- triche_pliee: 13 882 at threshold 0 (rectangle is mostly 3D decor), none above 12, max channel difference 4.
Side-by-side image reopened: C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\compare\ecran_hud_triche\triche_ouverte_apres_correction.png
: panel right edge 224, buttons 24-216, ochre handles, indistinguishable.

Tests: quick_check -Parts tests 1622/1622 PASS.

## What remains
Nothing visible. Not set from the script (not seen in the capture, and I did not check they are bound): Slider.MouseUsesStep = true and IsFocusable = false of the C++ (HOLCheatPanel.cpp:471-472).
The first preflight of the session ran before my edits applied (wrong indentation in the first Edit calls): no consequence.
