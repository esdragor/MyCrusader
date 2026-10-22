# Every rest measure in one go, for a moment when nothing else runs on the machine (asked 2026-10-21):
#   main copy  : S1, S2, S7 (each twice, chapter_measures.ps1), then S5
#   Elrond     : S3, S4 (each twice, only written there for now), then S5 and S5 in combat
# Checks first that no Unreal / build runs and that the processor is calm, builds both copies, then runs the measures
# and writes one summary: Saved_Tests\Logs\rest_measures_<date>.txt (main copy). ~25-30 min (more when a dedicated
# server has to be cooked again). Run from Windows PowerShell (never Git Bash), from the main copy:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\rest_measures.ps1            (everything)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\rest_measures.ps1 -DryRun    (checks and plan only)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\rest_measures.ps1 -NoElrond  (main copy only, ~10 min)
param(
    [switch] $DryRun,
    [switch] $NoElrond,
    [switch] $NoBuild,
    # Runs even when the processor is above MaxCpu before the start (the numbers are then worth less).
    [switch] $Force,
    [double] $MaxCpu = 15
)
$Main = (Resolve-Path "$PSScriptRoot\..\..").Path
$Elrond = Join-Path (Split-Path $Main) "HeroesOfLiberty-Elrond"
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

$RunElrond = -not $NoElrond -and (Test-Path (Join-Path $Elrond "HeroesOfLiberty.uproject"))
Say "Rest measures $Stamp - processor $CpuMean % before the start$(if ($Heavy) { " ; open: $($Heavy -join ', ')" })"
Say "  main   : $(Describe-Copy $Main) -> S1, S2, S7 (x2), S5"
if ($RunElrond) { Say "  Elrond : $(Describe-Copy $Elrond) -> S3, S4 (x2), S5, S5 combat" }
elseif (-not $NoElrond) { Say "  Elrond : not found, S3 / S4 skipped" }
if ($DryRun) { Write-Host "Dry run: nothing built or measured."; exit 0 }

# 2. Both copies built (the measures run the editor binaries of each copy).
if (-not $NoBuild) {
    if (-not (Build-Copy $Main)) { Say "main : BUILD FAILED, nothing measured"; $Summary | Set-Content -Encoding utf8 $SummaryFile; exit 1 }
    if ($RunElrond -and -not (Build-Copy $Elrond)) { Say "Elrond : BUILD FAILED, S3 / S4 skipped"; $RunElrond = $false }
}

# 3. The measures, one copy after the other (each script refuses to start while another Unreal runs).
$Failed = $false
& (Join-Path $Main "Tools\Tests\chapter_measures.ps1") -Tag "rest_${Stamp}_main"
$Failed = $Failed -or ($LASTEXITCODE -ne 0)
Say ""
Say "== main"
Get-Content (Join-Path $Main "Saved_Tests\Logs\chapter_measures.txt") | ForEach-Object { Say $_ }
if ($RunElrond) {
    & (Join-Path $Elrond "Tools\Tests\chapter_measures.ps1") -Tests "S3:15", "S4:15" -Tag "rest_${Stamp}_elrond"
    $Failed = $Failed -or ($LASTEXITCODE -ne 0)
    Say ""
    Say "== Elrond"
    Get-Content (Join-Path $Elrond "Saved_Tests\Logs\chapter_measures.txt") | ForEach-Object { Say $_ }
}

Say ""
Say "All done in $([math]::Round($Clock.Elapsed.TotalMinutes, 1)) min : $(if ($Failed) { 'FAIL' } else { 'PASS' }) - summary in $SummaryFile"
$Summary | Set-Content -Encoding utf8 $SummaryFile
exit $(if ($Failed) { 1 } else { 0 })
