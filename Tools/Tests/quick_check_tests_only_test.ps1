# Checks of the tool quick_check.ps1 (2026-11-09, Auto_Journey_File line 55): "-Parts tests" alone must run the automation
# tests only, NOT the main game's menu/camera sequence (7 frames, a "sequence" report). Static, a second to run:
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\quick_check_tests_only_test.ps1 [-Path <quick_check.ps1 to judge>]
# Red on the version before 64376782 (git show 64376782^:Tools/Tests/quick_check.ps1 > <file>; -Path <file>), green on the current one.
# The end-to-end proof (2026-11-09): "-Parts tests" ends "tests 1107/1107 : PASS / quick_check : PASS in 239 s (game launch done at 1 s)".
param([string] $Path = (Join-Path $PSScriptRoot "quick_check.ps1"))

$Text = Get-Content $Path -Raw
$Checks = [ordered]@{}
# 1. The need of the main game is derived from the parts: any part other than "tests".
$Checks["GameNeeded is true only for a part other than tests"] = $Text -match '\$GameNeeded\s*=\s*@\(\$Parts \| Where-Object \{ \$_ -ne "tests" \}\)\.Count -gt 0'
# 2. The game is launched only when needed.
$Checks["the main game (UnrealEditor.exe) starts only if GameNeeded"] = $Text -match '\$GameProcess\s*=\s*if \(\$GameNeeded\) \{ Start-Process -FilePath \(Join-Path \$EngineDir "UnrealEditor\.exe"\)'
# 3. Its wait and its "sequence" report are guarded as well.
$Checks["the wait for the game is guarded by GameNeeded"] = $Text -match '\$GameTimedOut\s*=\s*if \(\$GameNeeded\)'
$Checks["the sequence report is guarded by GameNeeded"] = $Text -match 'if \(\$GameNeeded\) \{ Report "sequence" \$SequenceChecks \}'

$Failed = @($Checks.Keys | Where-Object { -not $Checks[$_] })
foreach ($Name in $Checks.Keys) { Write-Host "$(if ($Checks[$Name]) { 'ok ' } else { 'KO ' }) $Name" }
Write-Host "quick_check_tests_only_test : $(if ($Failed.Count -eq 0) { 'PASS' } else { 'FAIL' }) ($Path)"
if ($Failed.Count -gt 0) { exit 1 }
exit 0
