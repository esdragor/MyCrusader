# The lines check.ps1 prints from a scenario's log (dot-sourced by check.ps1; test: check_log_test.ps1).
# Select-CheckLogLines <lines of the log>: the scenario's own lines and its errors, the last 40.
# The engine's smoke tests run at frame 0 of every uncooked -game start (LaunchEngineLoop.cpp RunSmokeTests). Under -culture=fr
# (check.ps1, play.ps1) the [Core][SmokeFilter] tests of Engine\Source\Runtime\Core\Tests\Experimental\UnifiedError\UnifiedErrorTests.cpp
# compare their localized messages ("Erreur vide") to English text ("Empty error") and write 20 "LogAutomationTest: Error:
# Condition failed" (0 under -culture=en, SAURON-1116d 2026-11-16). They say nothing of the scenario or of the game: one line
# names them instead of 20 (preflight.ps1 does not count them either); every other error line is kept.
function Select-CheckLogLines([string[]] $Lines) {
    $Engine = @($Lines | Where-Object { $_ -match "LogAutomationTest: Error: Condition failed" }).Count
    $Kept = @($Lines | Where-Object { $_ -match "Scenario |Test sequence|Production:|Technology:|LogHOLTest: Error|Error:" -and $_ -notmatch "LogAutomationTest: Error: Condition failed" } | Select-Object -Last 40)
    if ($Engine -gt 0) { $Kept = @("engine smoke tests: $Engine 'Condition failed' at start (UnifiedErrorTests.cpp under -culture=fr, English messages compared; not the scenario, not counted)") + $Kept }
    $Kept
}
