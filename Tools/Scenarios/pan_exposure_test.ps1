# Test of Get-PanExposure (PanExposure.ps1) on the REAL scenario files (not on invented lines). Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\pan_exposure_test.ps1 [-Root <worktree>] [-Names fog_trees,...]
# E-fog_trees (2026-11-08): the user's play.ps1 run stayed "en preparation": the second LeftClickGround came 60 frames after hol.Camera.FocusNear, the
# camera drifted, "the point is not on the screen", the waitlog never came. The scenario must now keep every ground click within a few frames of the
# camera placement. Exit 1 on any offender.
param([string] $Root = "", [string[]] $Names = @("fog_trees", "neutrals_qground", "diplomacy_qground_watch", "legions_roster_gesture", "legions_roster_gesture_dawn",
    "diplomacy_qground", "diplomacy_qground_before", "diplomacy_qground_dump", "diplomacy_qground_watch_nc", "diplomacy_qground_watch_nofix",
    "fog_orders", "fog_orders_compare", "fog_orders_flicker", "fog_orders_flicker_off", "legions_roster", "build_walkers",
    "zz_edge_walk", "zz_edge_walk2", "zz_edge_walk4", "neutrals_qground_rule",
    "hero_xp", "playtest_hero_xp", "siege", "siege_combat", "siege_mangonel", "siege_raze", "siege_tech", "unit_stats_attack"))   # + ClickUnit (Glorfindel-23)   # melee_stuck* / melee_spin*: Celeborn's, covered when he fixes them
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
. (Join-Path $PSScriptRoot "PanExposure.ps1")
$Bad = 0
foreach ($Name in @($Names | ForEach-Object { $_ -split ',' } | Where-Object { $_ })) {   # -File passes "a,b" as ONE string
    $File = Join-Path $Root "Tools\Scenarios\$Name.txt"
    if (-not (Test-Path $File)) { $Bad++; Write-Host "FAIL $Name : no such scenario file $File"; continue }
    $Offenders = @(Get-PanExposure (Get-Content -Encoding UTF8 $File))
    if ($Offenders.Count -gt 0) { $Bad++; Write-Host "FAIL $Name : $($Offenders.Count) ground click(s) too long after the camera"; $Offenders | ForEach-Object { Write-Host "     $_" } }
    else { Write-Host "OK   $Name : every ground click is within 10 frames of the camera placement" }
}
# Rule 2 (2026-11-16, SAURON-1116a): no 'hol.Select.ClickUnit <N> enemy' without slot=<S>, in EVERY scenario of Tools\Scenarios (not only $Names).
$EnemyFiles = @(Get-ChildItem (Join-Path $Root "Tools\Scenarios") -Filter *.txt)
$EnemyBad = 0
foreach ($File in $EnemyFiles) {
    $Enemy = @(Get-EnemyClickWithoutSlot (Get-Content -Encoding UTF8 $File.FullName))
    if ($Enemy.Count -gt 0) { $Bad++; $EnemyBad++; Write-Host "FAIL $($File.BaseName) : $($Enemy.Count) enemy click(s) without slot="; $Enemy | ForEach-Object { Write-Host "     $_" } }
}
if ($EnemyBad -eq 0) { Write-Host "OK   enemy clicks : no 'ClickUnit <N> enemy' without slot= in $($EnemyFiles.Count) scenario files" }
$Flagged = @(Get-EnemyClickWithoutSlot @("# hol.Select.ClickUnit -1 enemy (a comment)", "hol.Select.ClickUnit -1 enemy order", "quiet hol.Select.ClickUnit 6 order enemy"))
if ($Flagged.Count -eq 2) { Write-Host "OK   control enemy_noslot : flagged (2)" } else { $Bad++; Write-Host "FAIL control enemy_noslot : $($Flagged.Count) flagged, 2 expected" }
$Clean = @(Get-EnemyClickWithoutSlot @("hol.Select.ClickUnit -1 slot=1", "hol.Select.ClickUnit -1 order slot=1", "hol.Select.ClickUnit 3 ally", "hol.Select.ClickUnit -1 enemyfoo"))
if ($Clean.Count -eq 0) { Write-Host "OK   control enemy_slot : not flagged" } else { $Bad++; Write-Host "FAIL control enemy_slot : $($Clean.Count) flagged, 0 expected" }
# Controls of the checker itself (fixtures): a ClickUnit 400 frames after the camera must be flagged, one 2 frames after must not.
$Far = @(Get-PanExposure (Get-Content -Encoding UTF8 (Join-Path $PSScriptRoot "pan_exposure_fixtures\clickunit_far.txt")))
if ($Far.Count -ge 1) { Write-Host "OK   control clickunit_far : flagged ($($Far.Count))" } else { $Bad++; Write-Host "FAIL control clickunit_far : a ClickUnit 400 frames after the camera was NOT flagged" }
$Near = @(Get-PanExposure (Get-Content -Encoding UTF8 (Join-Path $PSScriptRoot "pan_exposure_fixtures\clickunit_near.txt")))
if ($Near.Count -eq 0) { Write-Host "OK   control clickunit_near : not flagged" } else { $Bad++; Write-Host "FAIL control clickunit_near : a ClickUnit 2 frames after the camera was flagged" }
Write-Host "pan_exposure_test : $(if ($Bad -eq 0) { 'PASS' } else { "FAIL ($Bad)" })"
if ($Bad -eq 0) { exit 0 } else { exit 1 }
