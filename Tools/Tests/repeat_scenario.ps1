# Repeats a scenario N times (the repeated measure : `neutrals` x 12, a flaky check, a sweep) FROM A COPY OF THE BINARIES, so that
# Tools\build.ps1 of the same worktree can compile while the passes run (see BinaryCopy.ps1 for the why and the layout).
#   powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\repeat_scenario.ps1 <name> [-Runs 12] [-Root <worktree>] [-TimeoutSeconds 240]
# Each pass is Tools\Scenarios\check.ps1 on the shadow project (same checks, same PASS / FAIL line). The log of pass k is kept as
# <Root>\Saved_Tests\Logs\scenario_<name>_run<k>.log (check.ps1 overwrites scenario_<name>.log every pass).
# Prints one line per pass, then "repeat <name> : PASS n/N (min / median / max s)". Exit code = number of failed passes.
# The binaries copied are the ones present at the start: a build finishing during the passes does not change them (a new call
# takes a new copy). -Keep leaves the shadow directory (named in the output) for a look.
param(
    [Parameter(Position = 0, Mandatory = $true)] [string] $Name,
    [int] $Runs = 12,
    [string] $Root = "",
    [int] $TimeoutSeconds = 240,
    [string] $EngineDir = "",
    [switch] $Keep
)
. (Join-Path $PSScriptRoot 'BinaryCopy.ps1')
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Check = Join-Path $Root 'Tools\Scenarios\check.ps1'
$Shadow = New-BinaryCopy -Root $Root
Write-Host "repeat $Name : $Runs pass(es) from the binary copy $Shadow (build.ps1 of $(Split-Path $Root -Leaf) stays free)"
$Seconds = @(); $Failed = 0
try {
    for ($K = 1; $K -le $Runs; $K++) {
        $Extra = if ($EngineDir) { @('-EngineDir', $EngineDir) } else { @() }
        $Out = & powershell -NoProfile -ExecutionPolicy Bypass -File $Check $Name -Root $Shadow -TimeoutSeconds $TimeoutSeconds @Extra 2>&1 | Out-String
        $Code = $LASTEXITCODE
        $Line = @($Out -split "`r?`n" | Where-Object { $_ -match '^check ' } | Select-Object -Last 1)
        $Log = Join-Path $Root "Saved_Tests\Logs\scenario_$Name.log"
        if (Test-Path $Log) { Copy-Item $Log (Join-Path $Root "Saved_Tests\Logs\scenario_${Name}_run$K.log") -Force }
        if ($Code -ne 0) { $Failed++ }
        if ($Line.Count -and $Line[0] -match '\((\d+) s\)') { $Seconds += [int]$Matches[1] }
        Write-Host ("  pass {0}/{1} : {2}" -f $K, $Runs, $(if ($Line.Count) { $Line[0] } else { "exit $Code, no result line" }))
    }
} finally {
    if ($Keep) { Write-Host "shadow kept : $Shadow" } else { Remove-BinaryCopy $Shadow }
}
$Sorted = @($Seconds | Sort-Object)
$Spread = if ($Sorted.Count) { "{0} / {1} / {2} s" -f $Sorted[0], $Sorted[[int][math]::Floor(($Sorted.Count - 1) / 2)], $Sorted[-1] } else { "no timing" }
Write-Host ("repeat $Name : {0} {1}/{2} ({3})" -f $(if ($Failed -eq 0) { 'PASS' } else { 'FAIL' }), ($Runs - $Failed), $Runs, $Spread)
exit $Failed
