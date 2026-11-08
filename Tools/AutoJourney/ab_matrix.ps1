# A/B matrix (L9-7, 2026-11-08) : configs x scenarios x N passes -> one "PASS k/N" line per config/scenario, written to a FILE.
# Why : seven fresh agents in a row measured the melee with small series (N = 4 to 8), each re-reading ten calls of context to rebuild the
# same loop (Celeborn-13 to 19). One call of this tool replaces that loop ; the table file is the shared, resumable record.
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\ab_matrix.ps1 -Configs "s120=_s120,s150=_s150,default=" `
#       -Scenarios "melee_stuck,melee_stuck_old" -N 8 [-Root <worktree>] [-Out <file>]
# A CONFIG is "label=suffix" : the scenario run for (config, scenario) is <scenario><suffix> (Tools\Scenarios\<scenario><suffix>.txt, e.g.
# melee_stuck_s120 ; empty suffix = the scenario as it is). A bare word "x" means label x, suffix "_x". The scenarios of the configs are
# written by whoever owns the measure (a cvar line in each .txt) ; this tool only repeats them and tabulates.
# Each cell = ONE Tools\Tests\repeat_scenario.ps1 call (N valid passes from a copy of the binaries, so build.ps1 stays free ; INVALID passes
# are retried there and never counted). Scenario outer, config inner : the A and the B of one scenario run next to each other in time.
# N must be >= 8 (a small series is noise : no conclusion from 4) ; below 8 only with -Exploratory, and the file then says "NOT A CONCLUSION".
# The FILE (default <Root>\Saved\ab_matrix\matrix_<stamp>.txt) is rewritten after every cell, so a stopped run leaves the cells already done.
#   <config> | <scenario> | PASS k/N | verdict OK|RED | invalid I | min / median / max s        (k = passes that PASSED, never a failure count)
#   <config> | <scenario> | NO RESULT (exit x) | ...      (the repeat tool printed no summary line : read cell_<config>_<scenario>.txt)
# The raw output of each cell is kept next to the table as cell_<config>_<scenario>.txt (the run<k> logs stay in Saved_Tests\Logs).
# Exit code : 2 = refused (N < 8, bad arguments) ; else the number of NO RESULT cells (0 = every cell gave a line ; RED cells are results).
param(
    [string[]] $Configs = @(),
    [string[]] $Scenarios = @(),
    [int] $N = 8,
    [string] $Root = "",
    [string] $Out = "",
    [string] $RepeatTool = "",
    [switch] $Exploratory
)
$ConfigList = @($Configs | ForEach-Object { "$_" -split '[,;]' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })   # powershell -File passes a,b as ONE string
$ScenarioList = @($Scenarios | ForEach-Object { "$_" -split '[,;\s]+' } | Where-Object { $_ })
if (-not $ConfigList.Count -or -not $ScenarioList.Count) { Write-Host "ab_matrix : give -Configs ""label=suffix,..."" and -Scenarios ""a,b"""; exit 2 }
if ($N -lt 8 -and -not $Exploratory) { Write-Host "ab_matrix : N = $N is below 8 : a small series is noise, no conclusion from it. Use -N 8 (or more), or -Exploratory to look without concluding."; exit 2 }
if (-not $Root) { $Root = Join-Path $PSScriptRoot "..\.." }
$Root = (Resolve-Path $Root).Path
if (-not $RepeatTool) { $RepeatTool = Join-Path $PSScriptRoot "..\Tests\repeat_scenario.ps1" }
$RepeatTool = (Resolve-Path $RepeatTool).Path
if (-not $Out) { $Out = Join-Path $Root ("Saved\ab_matrix\matrix_" + (Get-Date -Format "yyyyMMdd_HHmmss") + ".txt") }
$Dir = Split-Path -Parent $Out
New-Item -ItemType Directory -Force $Dir | Out-Null

$Cfg = @(foreach ($C in $ConfigList) {
    if ($C -match '^([^=]+)=(.*)$') { [pscustomobject]@{ Label = $Matches[1].Trim(); Suffix = $Matches[2].Trim() } }
    else { [pscustomobject]@{ Label = $C; Suffix = "_$C" } }
})
$Cells = @()   # one object per finished cell
$Started = Get-Date

function Write-Table {
    $Head = @("ab_matrix : $($Cfg.Count) config(s) x $($ScenarioList.Count) scenario(s), N = $N$(if ($N -lt 8) { ' : NOT A CONCLUSION (N < 8, -Exploratory)' })",
              "started $($Started.ToString('yyyy-MM-dd HH:mm:ss')), written $((Get-Date).ToString('HH:mm:ss')), $($Cells.Count)/$($Cfg.Count * $ScenarioList.Count) cell(s) done, root $Root",
              "config | scenario | PASS k/N | verdict | invalid | min / median / max s", "")
    $Body = @($Cells | ForEach-Object { $_.Line })
    $Pivot = @("", ("pivot (k/N passed) : scenario | " + (($Cfg | ForEach-Object { $_.Label }) -join " | ")))   # parentheses : the comma binds tighter than +
    foreach ($S in $ScenarioList) {
        $Row = @(foreach ($C in $Cfg) {
            $Cell = $Cells | Where-Object { $_.Config -eq $C.Label -and $_.Scenario -eq $S } | Select-Object -First 1
            if ($Cell) { $Cell.Short } else { "-" }
        })
        $Pivot += "pivot : $S | " + ($Row -join " | ")
    }
    Set-Content -Path $Out -Value ($Head + $Body + $Pivot) -Encoding UTF8
}

$NoResult = 0
foreach ($S in $ScenarioList) {
    foreach ($C in $Cfg) {
        $Name = "$S$($C.Suffix)"
        $CellFile = Join-Path $Dir "cell_$($C.Label)_$S.txt"
        Write-Host ("ab_matrix : {0} / {1} -> {2} x {3} ..." -f $C.Label, $S, $Name, $N)
        $Text = (& powershell -NoProfile -ExecutionPolicy Bypass -File $RepeatTool $Name -Runs $N -Root $Root 2>&1 | Out-String)
        $Code = $LASTEXITCODE
        Set-Content -Path $CellFile -Value $Text -Encoding UTF8
        $Line = @($Text -split "`r?`n" | Where-Object { $_ -match "^repeat \S+ : PASS \d+/\d+" } | Select-Object -Last 1)
        if ($Line.Count -and $Line[0] -match '^repeat \S+ : PASS (\d+)/(\d+)') {
            $K = $Matches[1]; $Of = $Matches[2]
            $Spread = if ($Line[0] -match '\(([^)]*\bs)\)') { $Matches[1] } else { "no timing" }
            $Invalid = if ($Line[0] -match '(\d+) INVALID') { $Matches[1] } else { "0" }
            $Verdict = if ($Line[0] -match 'verdict (OK|RED)') { $Matches[1] } elseif ($K -eq $Of) { "OK" } else { "RED" }
            $Cells += [pscustomobject]@{ Config = $C.Label; Scenario = $S; Short = "$K/$Of"; Line = "$($C.Label) | $S | PASS $K/$Of | verdict $Verdict | invalid $Invalid | $Spread" }
        } else {
            $NoResult++
            $Cells += [pscustomobject]@{ Config = $C.Label; Scenario = $S; Short = "NO RESULT"; Line = "$($C.Label) | $S | NO RESULT (exit $Code) | see $CellFile" }
        }
        Write-Table
    }
}
Get-Content $Out | ForEach-Object { Write-Host $_ }
Write-Host "ab_matrix : table in $Out ($NoResult cell(s) without result)"
exit $NoResult
