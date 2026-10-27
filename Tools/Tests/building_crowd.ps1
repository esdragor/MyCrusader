# A group on an enemy House, again and again (the user's trial of 2026-10-25: Men-at-arms melted into one mass at one
# point of the House's wall). Plain, the player and an AI (slot 1, its Houses made by hol.Cheat.SpawnBuilding beyond the
# player's ridge); each time a group of 6 to 12 born at a random angle and distance, ordered on the House as a right
# click does (hol.Test.BuildingCrowd, Source/HeroesOfLiberty/Dev/HOLBuildingCrowdTest.cpp). Checked once all strike: in
# reach of the wall, no two closer than 1.5 radii, spread along the wall.
# Off screen, no window. Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\building_crowd.ps1 [-Count 20] [-Seed 1] [-Type ManAtArms] [-Root <copy>]
# Prints each failed run, then one line PASS / FAIL; exit code 0 / 1. Log: <copy>\Saved_Tests\Logs\building_crowd.log
param(
    [int] $Count = 20,
    [int] $Seed = 1,
    [string] $Type = "ManAtArms",
    [string] $Root = "",
    [string] $EngineDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "building_crowd : FAIL (engine '$Association' of the .uproject not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Saved = Join-Path $Project "Saved_Tests"
$Log = Join-Path $Saved "Logs\building_crowd.log"
$Sequence = Join-Path $Saved "building_crowd.seq"
$Config = Join-Path $Saved "building_crowd.json"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
Remove-Item $Log -ErrorAction SilentlyContinue
'{ "map": "/Game/Maps/Plain.Plain", "startingPeasants": 0, "startingHero": 0, "slots": [ { "state": "Human", "colorIndex": 0 }, { "state": "AI", "colorIndex": 1 } ] }' | Set-Content -Encoding ascii $Config
@(
    "waitmap MainMenu", "waitui", "hol.UI.NewGame.Launch $($Config -replace '\\', '/') 0", "waitmap Plain",
    "waitlog Camera above the start of slot", "frames 5", "hol.Test.BuildingCrowd $Count $Seed $Type quit"
) | Set-Content -Encoding ascii $Sequence

$Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -nosplash -culture=fr -LiveCoding=0 -as-exit-on-error -saveddirsuffix=Tests -abslog=`"$Log`" -ExecCmds=`"hol.Test.Run $Sequence`""
$Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru
# A run takes ~10 s of game; 40 s each at most, then the game is stopped.
if (-not $Process.WaitForExit([Math]::Max(120, 60 + 40 * $Count) * 1000)) { $Process.Kill() }

$Lines = if (Test-Path $Log) { Get-Content $Log } else { @() }
$Lines | Select-String -Pattern "BuildingCrowd \d+/\d+: .*" | ForEach-Object { Write-Host ($_.Line -replace "^.*Display: ", "  ") }
$Done = $Lines | Select-String -Pattern "BuildingCrowd: done, (\d+) runs, (\d+) failed" | Select-Object -Last 1
if (-not $Done) { Write-Host "building_crowd : FAIL (no end line; log $Log)"; exit 1 }
$Failed = [int]$Done.Matches[0].Groups[2].Value
Write-Host "building_crowd : $(if ($Failed -eq 0) { 'PASS' } else { 'FAIL' }) ($($Done.Matches[0].Groups[1].Value) runs, $Failed failed, seed $Seed, $Type)"
exit $(if ($Failed -eq 0) { 0 } else { 1 })
