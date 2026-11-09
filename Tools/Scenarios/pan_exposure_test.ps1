# Test of Get-PanExposure (PanExposure.ps1) on the REAL scenario files (not on invented lines). Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\pan_exposure_test.ps1 [-Root <worktree>] [-Names fog_trees,...]
# E-fog_trees (2026-11-08): the user's play.ps1 run stayed "en preparation": the second LeftClickGround came 60 frames after hol.Camera.FocusNear, the
# camera drifted, "the point is not on the screen", the waitlog never came. The scenario must now keep every ground click within a few frames of the
# camera placement. Exit 1 on any offender.
param([string] $Root = "", [string[]] $Names = @("fog_trees", "neutrals_qground", "diplomacy_qground_watch", "legions_roster_gesture", "legions_roster_gesture_dawn"))
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
Write-Host "pan_exposure_test : $(if ($Bad -eq 0) { 'PASS' } else { "FAIL ($Bad)" })"
if ($Bad -eq 0) { exit 0 } else { exit 1 }
