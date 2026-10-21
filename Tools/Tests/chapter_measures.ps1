# End-of-chapter measures (rule of the measures, JourneyInProgress.md): every stress test twice, off screen, at rest, in
# alternation (S1 S2 S7 S3 S4, then the same again), then S5 once. A test passes when both its runs pass its thresholds; one
# run on each side of a threshold: a third run, and the mean of the three decides. Prints one verdict line per test and
# a table, written to Saved_Tests\Logs\chapter_measures.txt. Exit code 1 on any FAIL. ~6 min without S5, ~7 min with it,
# ~10 min when the dedicated server has to be cooked again (Source, Config or Content newer than the staged server).
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths), the machine at rest, no Unreal open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\chapter_measures.ps1                 (S1, S2, S7, S3, S4, then S5)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\chapter_measures.ps1 -Tests S7:20 -NoS5   (one test again)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\chapter_measures.ps1 -Tests none          (S5 alone)
param(
    # Each test as Scenario:Seconds of measure. S3 (1000 against 1000 in melee) and S4 (the same in thirds, Archers behind)
    # measure from the moment the armies are joined (up to 40 s of warm-up), 15 s: most of both armies still fighting.
    [string[]] $Tests = @("S1:10", "S2:10", "S7:20", "S3:15", "S4:15"),
    [switch] $NoS5,
    [switch] $NoStage,
    [switch] $ForceStage,
    [string] $Tag = ""
)
$Project = (Resolve-Path "$PSScriptRoot\..\..").Path
$LogDir = Join-Path $Project "Saved_Tests\Logs"
New-Item -ItemType Directory -Force $LogDir | Out-Null
$Stamp = if ($Tag) { $Tag } else { Get-Date -Format "MMdd_HHmm" }
$Thresholds = @{ StepMs = 3.0; Fps = 144.0 }
$Clock = [Diagnostics.Stopwatch]::StartNew()

function Read-Report([string] $Log) {
    # The two verdict lines of the stress test report (UHOLStressTestSubsystem).
    $Text = if (Test-Path $Log) { Get-Content $Log -Raw } else { "" }
    $Step = [regex]::Match($Text, "Simulation: ([\d.]+) ms average per step \(max ([\d.]+) ms")
    $Fps = [regex]::Match($Text, "Rendering: ([\d.]+) fps average")
    if (-not $Step.Success -or -not $Fps.Success) { return $null }
    return [pscustomobject]@{ StepMs = [double]$Step.Groups[1].Value; MaxMs = [double]$Step.Groups[2].Value; Fps = [double]$Fps.Groups[1].Value }
}

function Invoke-Run([string] $Scenario, [int] $Seconds, [int] $Round) {
    $Name = "chapter_${Stamp}_$($Scenario.ToLower())_$Round"
    & (Join-Path $PSScriptRoot "stress_ab.ps1") -Project $Project -Test $Scenario -Seconds $Seconds -LogName $Name | Out-Host
    $Report = Read-Report (Join-Path $LogDir "$Name.log")
    if (-not $Report) { Write-Host "  $Scenario run $Round : no report (see $Name.log)"; return $null }
    $Pass = $Report.StepMs -le $Thresholds.StepMs -and $Report.Fps -ge $Thresholds.Fps
    Write-Host ("  {0} run {1} : {2:N3} ms per step (max {3:N1}), {4:N0} fps : {5}" -f $Scenario, $Round, $Report.StepMs, $Report.MaxMs, $Report.Fps, $(if ($Pass) { "PASS" } else { "FAIL" }))
    $Report | Add-Member Pass $Pass
    return $Report
}

$Plan = $Tests | Where-Object { $_ -ne "none" } | ForEach-Object { $Parts = $_.Split(":"); [pscustomobject]@{ Scenario = $Parts[0]; Seconds = [int]$Parts[1]; Runs = @() } }
# Two rounds in alternation: a drift of the machine's load spreads over every test instead of falling on one.
foreach ($Round in 1..2) {
    foreach ($Test in $Plan) { $Test.Runs += @(Invoke-Run $Test.Scenario $Test.Seconds $Round) }
}
$Lines = @()
$Failed = $false
foreach ($Test in $Plan) {
    $Runs = @($Test.Runs | Where-Object { $_ })
    # One run on each side of a threshold: a third, and the mean of the three decides.
    if ($Runs.Count -eq 2 -and $Runs[0].Pass -ne $Runs[1].Pass) {
        $Third = Invoke-Run $Test.Scenario $Test.Seconds 3
        if ($Third) { $Runs += $Third }
    }
    if ($Runs.Count -lt 2) {
        $Lines += "$($Test.Scenario) : FAIL (missing report)"
        $Failed = $true
        continue
    }
    $Step = ($Runs | Measure-Object StepMs -Average).Average
    $Fps = ($Runs | Measure-Object Fps -Average).Average
    $Pass = if ($Runs.Count -eq 3) { $Step -le $Thresholds.StepMs -and $Fps -ge $Thresholds.Fps } else { -not ($Runs | Where-Object { -not $_.Pass }) }
    $Failed = $Failed -or -not $Pass
    $Each = ($Runs | ForEach-Object { "{0:N3}" -f $_.StepMs }) -join " / "
    $Lines += ("{0} : {1} - {2} ms per step (mean {3:N3}, threshold {4}), {5:N0} fps (threshold {6}){7}" -f $Test.Scenario, $(if ($Pass) { "PASS" } else { "FAIL" }),
        $Each, $Step, $Thresholds.StepMs, $Fps, $Thresholds.Fps, $(if ($Runs.Count -eq 3) { ", decided by the mean of 3 runs" } else { "" }))
}

if (-not $NoS5) {
    # S5 once: its bandwidth moves by ~1 % from one run to the next.
    $NetArgs = @{ Units = 2000; Seconds = 40 }
    # Cooked again only when the code, the configuration or the content changed since the staged server was cooked.
    $Staged = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Binaries\Win64\HeroesOfLibertyServer.exe"
    $NeedStage = $ForceStage -or -not (Test-Path $Staged)
    if (-not $NeedStage) {
        $StagedTime = (Get-Item $Staged).LastWriteTime
        $Newer = Get-ChildItem -Recurse -File (Join-Path $Project "Source"), (Join-Path $Project "Config"), (Join-Path $Project "Content") |
            Where-Object { $_.LastWriteTime -gt $StagedTime } | Select-Object -First 1
        $NeedStage = [bool]$Newer -or (Get-Item (Join-Path $Project "HeroesOfLiberty.uproject")).LastWriteTime -gt $StagedTime
        if ($Newer) { Write-Host "S5 : $($Newer.FullName) changed since the staged server ($StagedTime) : cooked again" }
    }
    if ($NeedStage -and -not $NoStage) { $NetArgs.Stage = $true }
    elseif (-not $NeedStage) { Write-Host "S5 : nothing changed since the staged server : not cooked again" }
    $S5Out = & (Join-Path $PSScriptRoot "net_match.ps1") @NetArgs 2>&1 | Out-String
    $S5Code = $LASTEXITCODE
    Write-Host $S5Out
    $Verdict = ($S5Out -split "`r?`n" | Where-Object { $_ -match "PASS|FAIL" } | Select-Object -Last 3) -join " | "
    $Lines += "S5 : $(if ($S5Code -eq 0) { 'PASS' } else { 'FAIL' }) - $Verdict"
    $Failed = $Failed -or ($S5Code -ne 0)
}

$Summary = @("End-of-chapter measures $Stamp ($([math]::Round($Clock.Elapsed.TotalMinutes, 1)) min) : $(if ($Failed) { 'FAIL' } else { 'PASS' })") + $Lines
$Summary | Set-Content -Encoding utf8 (Join-Path $LogDir "chapter_measures.txt")
Write-Host ""
$Summary | ForEach-Object { Write-Host $_ }
exit $(if ($Failed) { 1 } else { 0 })
