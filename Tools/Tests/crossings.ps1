# The ridge crossed again and again (the user's trials of ridge_army): one army of the player, made between its main
# centre and its ridge on Plain, sent across the ridge to a random point, then back across it, Count times
# (hol.Test.Crossings, Source/HeroesOfLiberty/Dev/HOLCrossingTest.cpp). Each crossing checked every frame: no unit in a
# rock, no stall (half the walking units not moving), every unit standing near the point in time.
# Off screen, no window (the user works on the machine meanwhile). Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\crossings.ps1 [-Count 100] [-Seed 1] [-Units 50] [-Type ManAtArms] [-Root <copy>]
# Prints each failed crossing, then one line PASS / FAIL; exit code 0 / 1. Log: <copy>\Saved_Tests\Logs\crossings.log
# (~7 s of game a crossing).
param(
    [int] $Count = 100,
    [int] $Seed = 1,
    [int] $Units = 50,
    [string] $Type = "ManAtArms",
    [string] $Root = "",
    [string] $EngineDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "crossings : FAIL (engine '$Association' of the .uproject not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Saved = Join-Path $Project "Saved_Tests"
$Log = Join-Path $Saved "Logs\crossings.log"
$Sequence = Join-Path $Saved "crossings.seq"
$Config = Join-Path $Saved "crossings.json"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
Remove-Item $Log -ErrorAction SilentlyContinue
'{ "map": "/Game/Maps/Plain.Plain", "slots": [ { "state": "Human", "colorIndex": 0 } ] }' | Set-Content -Encoding ascii $Config
@(
    "waitmap MainMenu", "waitui", "hol.UI.NewGame.Launch $($Config -replace '\\', '/') 0", "waitmap Plain",
    "waitlog Camera above the start of slot", "frames 5", "hol.Test.Crossings $Count $Seed $Units $Type quit"
) | Set-Content -Encoding ascii $Sequence

$Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -nosplash -culture=fr -LiveCoding=0 -saveddirsuffix=Tests -abslog=`"$Log`" -ExecCmds=`"hol.Test.Run $Sequence`""
$Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru
# A crossing takes ~7 s of game; 30 s each at most, then the game is stopped.
if (-not $Process.WaitForExit([Math]::Max(120, 60 + 30 * $Count) * 1000)) { $Process.Kill() }

$Lines = if (Test-Path $Log) { Get-Content $Log } else { @() }
$Lines | Select-String -Pattern "Crossing \d+/\d+: arrived .* FAIL" | ForEach-Object { Write-Host ($_.Line -replace "^.*Display: ", "  ") }
$Done = $Lines | Select-String -Pattern "Crossings: done, (\d+) crossings, (\d+) failed" | Select-Object -Last 1
if (-not $Done) { Write-Host "crossings : FAIL (no end line; log $Log)"; exit 1 }
$Failed = [int]$Done.Matches[0].Groups[2].Value
Write-Host "crossings : $(if ($Failed -eq 0) { 'PASS' } else { 'FAIL' }) ($($Done.Matches[0].Groups[1].Value) crossings, $Failed failed, seed $Seed, $Units $Type)"
exit $(if ($Failed -eq 0) { 0 } else { 1 })
