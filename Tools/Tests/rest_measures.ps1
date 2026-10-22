# Every rest measure in one go, for a moment when nothing else runs on the machine (asked 2026-10-21):
#   main copy : S1, S2, S7, S3, S4 (each twice, chapter_measures.ps1), then S5 and S5 in combat
#   (S3 / S4 live on main since the melee merge 48ffa00: the Elrond copy is no longer needed)
# Checks first that no Unreal / build runs and that the processor is calm, builds the main copy, then runs the measures
# and writes one summary: Saved_Tests\Logs\rest_measures_<date>.txt. ~10-15 min (more when the dedicated server has to
# be cooked again). Run from Windows PowerShell (never Git Bash), from the main copy:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\rest_measures.ps1            (everything)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\rest_measures.ps1 -DryRun    (checks and plan only)
param(
    [switch] $DryRun,
    [switch] $NoBuild,
    # Runs even when the processor is above MaxCpu before the start (the numbers are then worth less).
    [switch] $Force,
    [double] $MaxCpu = 15
)
$Main = (Resolve-Path "$PSScriptRoot\..\..").Path
$Stamp = Get-Date -Format "MMdd_HHmm"
$SummaryFile = Join-Path $Main "Saved_Tests\Logs\rest_measures_$Stamp.txt"
New-Item -ItemType Directory -Force (Split-Path $SummaryFile) | Out-Null
$Clock = [Diagnostics.Stopwatch]::StartNew()
$Summary = @()

function Say([string] $Text) { Write-Host $Text; $script:Summary += $Text }

function Describe-Copy([string] $Dir) {
    $Head = "$(git -C $Dir log -1 --format='%h %s' 2>$null)"
    if ($Head.Length -gt 80) { $Head = $Head.Substring(0, 77) + "..." }
    $Dirty = @(git -C $Dir status --short --untracked-files=no 2>$null).Count
    return "$Head$(if ($Dirty) { " (+ $Dirty uncommitted files)" } else { '' })"
}

function Build-Copy([string] $Dir) {
    $UProject = Join-Path $Dir "HeroesOfLiberty.uproject"
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $Root = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds").$Association
    Write-Host "Build of $Dir ..."
    $Out = & (Join-Path $Root "Engine\Build\BatchFiles\Build.bat") HeroesOfLibertyEditor Win64 Development "-Project=$UProject" -WaitMutex 2>&1 | Out-String
    $Ok = $Out -match "Result: Succeeded"
    if (-not $Ok) { Write-Host ($Out -split "`r?`n" | Select-String "error" | Select-Object -First 10 | Out-String) }
    return $Ok
}

# 1. The machine at rest: no Unreal, build or other game open, the processor calm.
$Busy = Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(UnrealEditor|UnrealEditor-Cmd|UnrealBuildTool|UnrealInsights|ShaderCompileWorker|HeroesOfLiberty)' }
if ($Busy) {
    Write-Host "STOP: Unreal or a build is running (close it first):"
    $Busy | ForEach-Object { Write-Host "  $($_.ProcessId) $($_.Name)" }
    exit 2
}
$Cpu = (Get-Counter '\Processor(_Total)\% Processor Time' -SampleInterval 2 -MaxSamples 5).CounterSamples | ForEach-Object { $_.CookedValue }
$CpuMean = [math]::Round(($Cpu | Measure-Object -Average).Average, 1)
$Heavy = Get-Process | Where-Object { $_.Name -match '^(Discord|opera|chrome|msedge|firefox|Notepad|GitKraken|Battle\.net|steam|RelicCardinal|AoE\w*|ShivaGet|IMF|DriverBooster)$' } |
    Select-Object -ExpandProperty Name -Unique
Write-Host "Processor: $CpuMean % on average over 10 s (limit $MaxCpu %)"
if ($Heavy) { Write-Host "Open and worth closing: $($Heavy -join ', ')" }
if ($CpuMean -gt $MaxCpu -and -not $Force) {
    Write-Host "STOP: processor above $MaxCpu % : close the applications above, or run again with -Force"
    exit 2
}

Say "Rest measures $Stamp - processor $CpuMean % before the start$(if ($Heavy) { " ; open: $($Heavy -join ', ')" })"
Say "  main : $(Describe-Copy $Main) -> S1, S2, S7, S3, S4 (x2), S5, S5 combat"
if ($DryRun) { Write-Host "Dry run: nothing built or measured."; exit 0 }

# 2. The main copy built (the measures run its editor binaries).
if (-not $NoBuild) {
    if (-not (Build-Copy $Main)) { Say "main : BUILD FAILED, nothing measured"; $Summary | Set-Content -Encoding utf8 $SummaryFile; exit 1 }
}

# 3. The measures.
$Failed = $false
& (Join-Path $Main "Tools\Tests\chapter_measures.ps1") -Tag "rest_${Stamp}_main"
$Failed = $Failed -or ($LASTEXITCODE -ne 0)
Say ""
Say "== main"
Get-Content (Join-Path $Main "Saved_Tests\Logs\chapter_measures.txt") | ForEach-Object { Say $_ }

Say ""
Say "All done in $([math]::Round($Clock.Elapsed.TotalMinutes, 1)) min : $(if ($Failed) { 'FAIL' } else { 'PASS' }) - summary in $SummaryFile"
$Summary | Set-Content -Encoding utf8 $SummaryFile
exit $(if ($Failed) { 1 } else { 0 })
