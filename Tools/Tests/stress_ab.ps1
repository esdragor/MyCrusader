# One short stress test run, off screen, on any copy of the project (main copy or a worktree), to compare two builds
# A / B in alternation (A B A B A B) on the same machine load. Prints the CPU load before the run and the report lines
# (Simulation, Rendering, Frame, Draw, gather rate, S3 and S4 survivors and arrows).
# Refuses to start while another HeroesOfLiberty Unreal / UBT runs.
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths); announce timed runs to the other sessions:
#   & .\Tools\Tests\stress_ab.ps1 -Project C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -Test S7 -Seconds 20 -LogName a1
#   & .\Tools\Tests\stress_ab.ps1 -Project C:\Users\baudo\Documents\HeroesOfLiberty-J5c -Test S7 -Seconds 20 -LogName b1
# A clean side A: a detached worktree at the base commit, built (git worktree add --detach <dir> <commit>; Build.bat).
# -Trace also records an Insights trace (<LogName>.utrace; the trace costs time: compare traced runs only with traced
# runs), to read with: python Tools\Tests\trace_report.py Saved_Tests\Logs\<LogName>.utrace
param(
    [Parameter(Mandatory)] [string] $Project,
    [string] $Test = "S7",
    [int] $Seconds = 20,
    [Parameter(Mandatory)] [string] $LogName,
    [string] $Extra = "",
    [string] $ExtraExec = "",
    [switch] $Trace
)
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$Root = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds").$Association
$Exe = Join-Path $Root "Engine\Binaries\Win64\UnrealEditor.exe"
$LogDir = Join-Path $Project "Saved_Tests\Logs"
New-Item -ItemType Directory -Force $LogDir | Out-Null
$Log = Join-Path $LogDir "$LogName.log"
if ($Trace) {
    $TraceFile = Join-Path $LogDir "$LogName.utrace"
    Remove-Item $TraceFile -ErrorAction SilentlyContinue
    $Extra = "$Extra -trace=cpu,frame,bookmark,region,log -tracefile=`"$TraceFile`""
}
$Others = Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -match 'HeroesOfLiberty' -and $_.Name -match 'Unreal|UnrealBuildTool|dotnet' }
if ($Others) { Write-Host "BUSY: $($Others | ForEach-Object { $_.ProcessId })"; exit 1 }
$ArgList = "`"$UProject`" /Game/Maps/StressTest -game -RenderOffScreen -ResX=888 -ResY=500 -saveddirsuffix=Tests -nosplash -nosound -LiveCoding=0 -as-exit-on-error $Extra -abslog=`"$Log`" -ExecCmds=`"r.VSync 0,t.MaxFPS 0,scalability 2,hol.Units.SpawnBudget -1,$ExtraExec hol.StressTest $Test 2000 $Seconds quit`""
$Cpu = (Get-Counter '\Processor(_Total)\% Processor Time' -SampleInterval 2 -MaxSamples 3).CounterSamples | ForEach-Object { [math]::Round($_.CookedValue, 1) }
Write-Host "project: $Project ; cpu before: $($Cpu -join ', ')"
$Clock = [Diagnostics.Stopwatch]::StartNew()
$P = Start-Process -FilePath $Exe -ArgumentList $ArgList -PassThru
$P.WaitForExit(600000) | Out-Null
Write-Host "exit: $($P.HasExited) code $($P.ExitCode) in $([math]::Round($Clock.Elapsed.TotalSeconds)) s ; log $Log"
Select-String -Path $Log -Pattern '^  (Simulation|Rendering|Frame|Draw):|wood unloaded during|^  (Battle|Arrows during)' | ForEach-Object { $_.Line.Trim() }
