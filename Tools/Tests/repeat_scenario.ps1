# Repeats a scenario N times (the repeated measure : `neutrals` x 12, a flaky check, a sweep) FROM A COPY OF THE BINARIES, so that
# Tools\build.ps1 of the same worktree can compile while the passes run (see BinaryCopy.ps1 for the why and the layout).
#   powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\repeat_scenario.ps1 <name> [-Runs 12] [-Root <worktree>] [-TimeoutSeconds 240]
#   powershell -ExecutionPolicy Bypass -File ...\repeat_scenario.ps1 -Names a,b -Runs 8     (several scenarios, one after the other, ONE binary copy)
# Each pass is Tools\Scenarios\check.ps1 on the shadow project (same checks, same PASS / FAIL line). The log of pass k is kept as
# <Root>\Saved_Tests\Logs\scenario_<name>_run<k>.log (check.ps1 overwrites scenario_<name>.log every pass).
# Prints one line per pass, then "repeat <name> : PASS k/N (min / median / max s), verdict OK|RED" (k = passes that passed, N = -Runs;
# "verdict OK" only if k = N; ab_matrix.ps1 reads this line). Exit code = number of failed passes + invalid passes.
# A pass is VALID only if it ran on a log of its own : before each pass the tool waits (-MaxWaitSeconds, default 60) until the editor
# of the previous pass is gone and the scenario log can be removed (2026-11-07 : the previous editor, killed, still held the log, check.ps1
# gave up after 5 s with "log of an earlier pass is held by another process", the tool counted that as a failed pass and kept the held
# log as run<k>.log : 7 passes out of 8 were copies of the previous one). A pass that still says "held" (or has no result line, or no
# log written after its start) is INVALID : "pass k/N : INVALID (reason)", retried up to -Retries (default 2) times, never counted
# as a PASS or as a FAIL of the scenario, and no run<k>.log is kept for it.
# The binaries copied are the ones present at the start: a build finishing during the passes does not change them (a new call
# takes a new copy). -Keep leaves the shadow directory (named in the output) for a look.
param(
    [Parameter(Position = 0)] [string] $Name = "",
    [string[]] $Names = @(),                # also accepted as one string "a,b" (powershell -File passes a,b as ONE string)
    [int] $Runs = 12,
    [string] $Root = "",
    [int] $TimeoutSeconds = 240,
    [string] $EngineDir = "",
    [int] $MaxWaitSeconds = 60,
    [int] $Retries = 2,
    [switch] $Keep
)
. (Join-Path $PSScriptRoot 'BinaryCopy.ps1')
$All = @(@($Name) + @($Names) | ForEach-Object { "$_" -split '[,;\s]+' } | Where-Object { $_ } | Select-Object -Unique)
if (-not $All.Count) { Write-Host "repeat : give a scenario name (<name> or -Names a,b)"; exit 1 }
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Check = Join-Path $Root 'Tools\Scenarios\check.ps1'
$Shadow = New-BinaryCopy -Root $Root

# True when <Path> is absent or can be opened with no sharing (nobody holds it, so check.ps1 can remove it).
function Test-Free([string] $Path) {
    if (-not (Test-Path $Path)) { return $true }
    try { $S = [IO.File]::Open($Path, 'Open', 'ReadWrite', 'None'); $S.Dispose(); return $true } catch { return $false }
}
# Waits until no editor of the shadow project is left (kills the stragglers after a third of the wait) and the log is free.
function Wait-PassReady([string] $Log) {
    $Start = Get-Date; $Killed = $false
    while ($true) {
        $Editors = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($Shadow) })
        $Free = Test-Free $Log
        if (-not $Editors.Count -and $Free) { return $true }
        $Elapsed = ((Get-Date) - $Start).TotalSeconds
        if ($Elapsed -gt $MaxWaitSeconds) { return $false }
        if ($Editors.Count -and -not $Killed -and $Elapsed -gt ($MaxWaitSeconds / 3)) { $Editors | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }; $Killed = $true }
        Start-Sleep -Milliseconds 500
    }
}

$Extra = if ($EngineDir) { @('-EngineDir', $EngineDir) } else { @() }
$Bad = 0
Write-Host "repeat $($All -join ', ') : $Runs pass(es) each from the binary copy $Shadow (build.ps1 of $(Split-Path $Root -Leaf) stays free)"
try {
    foreach ($Scenario in $All) {
        $Seconds = @(); $Failed = 0; $Invalid = 0
        $Log = Join-Path $Root "Saved_Tests\Logs\scenario_$Scenario.log"
        for ($K = 1; $K -le $Runs; $K++) {
            $Kept = Join-Path $Root "Saved_Tests\Logs\scenario_${Scenario}_run$K.log"
            Remove-Item $Kept -Force -ErrorAction SilentlyContinue   # never a log of an older call under this name
            $Done = $false
            for ($Try = 0; $Try -le $Retries -and -not $Done; $Try++) {
                $Why = ""
                if (-not (Wait-PassReady $Log)) { $Why = "the log $Log is still held after $MaxWaitSeconds s (an editor of an earlier pass?)" }
                else {
                    $PassStart = Get-Date
                    $Out = & powershell -NoProfile -ExecutionPolicy Bypass -File $Check $Scenario -Root $Shadow -TimeoutSeconds $TimeoutSeconds @Extra 2>&1 | Out-String
                    $Code = $LASTEXITCODE
                    $Line = @($Out -split "`r?`n" | Where-Object { $_ -match '^check ' } | Select-Object -Last 1)
                    $Text = if ($Line.Count) { $Line[0] } else { "" }
                    if (-not $Text) { $Why = "no result line (exit $Code)" }
                    elseif ($Text -match 'held by another process') { $Why = "log held by another process" }
                    elseif ($Text -match 'no editor slot') { $Why = "no editor slot" }
                    elseif (-not (Test-Path $Log) -or (Get-Item $Log).LastWriteTime -lt $PassStart.AddSeconds(-2)) { $Why = "no log written by this pass" }
                }
                if ($Why) {
                    Write-Host ("  pass {0}/{1} : INVALID ({2}){3}" -f $K, $Runs, $Why, $(if ($Try -lt $Retries) { " : retry $($Try + 1)/$Retries" } else { "" }))
                    continue
                }
                $Done = $true
                Copy-Item $Log $Kept -Force
                if ($Code -ne 0) { $Failed++ }
                if ($Text -match '\((\d+) s\)') { $Seconds += [int]$Matches[1] }
                Write-Host ("  pass {0}/{1} : {2}" -f $K, $Runs, $Text)
            }
            if (-not $Done) { $Invalid++ }
        }
        $Sorted = @($Seconds | Sort-Object)
        $Spread = if ($Sorted.Count) { "{0} / {1} / {2} s" -f $Sorted[0], $Sorted[[int][math]::Floor(($Sorted.Count - 1) / 2)], $Sorted[-1] } else { "no timing" }
        # 2026-11-08 (L9-7) : the line always reads "PASS k/N" with k = passes that PASSED (it used to read "FAIL k/N" with k = the PASS count,
        # read as "k failures"); the verdict is its own word : "verdict OK" (all N passed) or "verdict RED" (a pass failed or was invalid).
        $Verdict = if ($Failed -eq 0 -and $Invalid -eq 0) { 'OK' } else { 'RED' }
        Write-Host ("repeat $Scenario : PASS {0}/{1} ({2}){3}, verdict {4}" -f ($Runs - $Failed - $Invalid), $Runs, $Spread, $(if ($Invalid) { ", $Invalid INVALID pass(es) not counted as a result of the scenario" } else { "" }), $Verdict)
        $Bad += $Failed + $Invalid
    }
} finally {
    if ($Keep) { Write-Host "shadow kept : $Shadow" } else { Remove-BinaryCopy $Shadow }
}
exit $Bad
