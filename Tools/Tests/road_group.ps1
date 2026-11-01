# Red-first test of the road speed on a GROUP (map_forest retour: "same speed at 1 % or 10 000 %"): runs the scenario road_group
# (10 Men-at-arms ordered by the console order of one slot = the player's group move order, formation march) and reads, from the
# log, how far the group's mean x went in 3 s for each setting of hol.Terrain.RoadSpeedPercent (1 -> 100 and 10000 -> 300 are the
# cvar's clamps). Exit 0 only if the distance follows the setting.
#   powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-<wt>\Tools\Tests\road_group.ps1 [-Root <wt>] [-NoRun]
param([string] $Root = "", [switch] $NoRun)
if (-not $Root) { $Root = (Resolve-Path "$PSScriptRoot\..\..").Path }
$Log = Join-Path $Root "Saved_Tests\Logs\scenario_road_group.log"
if (-not $NoRun) {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $Root "Tools\Scenarios\check.ps1") road_group -Root $Root -TimeoutSeconds 300 | Out-Null
}
if (-not (Test-Path $Log)) { Write-Host "road_group: no log"; exit 1 }
$Lines = Get-Content $Log
# One "Units where:" line = one reading of the 10 units: their mean x. A "PHASE route=N" note opens a phase.
$Phases = @(); $Current = $null
foreach ($Line in $Lines) {
    if ($Line -match "Scenario note: PHASE route=(\d+)") { $Current = @{ Percent = [int]$Matches[1]; Means = @() }; $Phases += $Current; continue }
    if ($Current -and $Line -match "Units where: \d+ units: (.*)$") {
        $Xs = [regex]::Matches($Matches[1], "slot 0 at \((-?\d+),") | ForEach-Object { [double]$_.Groups[1].Value }
        if ($Xs.Count -gt 0) { $Current.Means += ($Xs | Measure-Object -Average).Average }
    }
}
$Rows = @()
foreach ($P in $Phases) {
    if ($P.Means.Count -ge 4) { $Rows += [pscustomobject]@{ Route = $P.Percent; Start = [int]$P.Means[0]; After1 = [int]$P.Means[1]; After2 = [int]$P.Means[2]; After3 = [int]$P.Means[3]; Distance3s = [int]($P.Means[3] - $P.Means[0]); Back = $(if ($P.Means.Count -ge 5) { [int]$P.Means[4] } else { 0 }) } }
}
$Rows | Format-Table -AutoSize | Out-String | Write-Host
if ($Rows.Count -lt 4) { Write-Host "road_group FAIL: $($Rows.Count) phases read (scenario incomplete)"; exit 1 }
$D = @{}; foreach ($R in $Rows) { $D[$R.Route] = $R.Distance3s }
$Ok = $true
function Check($Name, $Cond) { if ($Cond) { Write-Host "  ok   $Name" } else { Write-Host "  RED  $Name"; $script:Ok = $false } }
Check "1 % (clamped to 100) equals 100 % within 10 %" ([Math]::Abs($D[1] - $D[100]) -le 0.1 * $D[100])
Check "130 % walks 1.2 to 1.4 times the distance of 100 %" ($D[130] -ge 1.2 * $D[100] -and $D[130] -le 1.4 * $D[100])
Check "10000 % (clamped to 300) walks more than 2 times the distance of 100 %" ($D[10000] -gt 2.0 * $D[100])
if ($Ok) { Write-Host "road_group PASS"; exit 0 } else { Write-Host "road_group FAIL"; exit 1 }
