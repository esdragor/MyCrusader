# Screenshots and log lines of the faction list of "Nouvelle partie" (FactionChoiceInProgress.md), off screen, no window.
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\faction_shots.ps1 [-Root <copy>]
# On the menu map: Nouvelle partie, an AI on line 2, then the faction list of line 2 opened
#   A) without the cheat: five entries, Dragon / Ombre / Feu greyed, no Templars      -> faction_A_without
#   B) with the cheat (hol.Faction.Cheat 1, as a triple click on the title would): six entries -> faction_B_with
# and read back (hol.UI.Text): Legions chosen, a greyed Dragon refused, the Templars chosen with the cheat, the cheat switched off with a Templar
# line still saying "Templiers". The triple click itself is not injected (the console has no mouse): its counter is a unit test.
# Prints the lines that matter; exit code 0 when the captures exist.
param(
    [string] $Root = "",
    [string] $EngineDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "faction_shots : FAIL (engine '$Association' not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Editor = Join-Path $EngineDir "UnrealEditor.exe"
$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Work = Join-Path $Project "Saved_Tests"
New-Item -ItemType Directory -Force (Join-Path $Work "Logs") | Out-Null

$Suffix = "Tests_FactionShots"
$Dir = Join-Path $Project "Saved_$Suffix"
if (Test-Path $Dir) { Remove-Item -Recurse -Force $Dir -ErrorAction SilentlyContinue }
$Log = Join-Path $Work "Logs\faction_shots.log"
$Sequence = Join-Path $Work "faction_shots.seq"
Remove-Item $Log -ErrorAction SilentlyContinue
@(
    "waitmap MainMenu", "waitui", "hol.Faction.Cheat 0", "hol.UI.Open NewGame", "waitui", "frames 20",
    "hol.UI.Choose SlotState1 2", "frames 10", "hol.UI.Text SlotFaction0 SlotFaction1",
    "hol.UI.Focus SlotFaction1", "hol.UI.Key Enter 0.3", "wait 2", "frames 10", "capture faction_A_without",
    "hol.UI.Key Escape 0.3", "wait 1.5", "frames 10",
    "hol.UI.Choose SlotFaction1 2", "frames 5", "hol.UI.Text SlotFaction1",
    "hol.UI.Choose SlotFaction1 1", "frames 5", "hol.UI.Text SlotFaction1",
    "hol.Faction.Cheat 1", "frames 20",
    "hol.UI.Focus SlotFaction1", "hol.UI.Key Enter 0.3", "wait 2", "frames 10", "capture faction_B_with",
    "hol.UI.Key Escape 0.3", "wait 1.5", "frames 10",
    "hol.UI.Choose SlotFaction1 5", "frames 5", "hol.UI.Text SlotFaction1",
    "hol.Faction.Cheat 0", "frames 10", "hol.UI.Text SlotFaction1",
    "quit"
) | Set-Content -Encoding ascii $Sequence
$Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -nosplash -unattended -culture=fr -LiveCoding=0 -as-exit-on-error -asdebugport=0 -saveddirsuffix=$Suffix -abslog=`"$Log`" -ExecCmds=`"hol.Test.Run $Sequence`""
$Process = Start-Process -FilePath $Editor -ArgumentList $Arguments -PassThru -WindowStyle Hidden
if (-not $Process.WaitForExit(180000)) { $Process.Kill() }
if (Test-Path $Log) { Get-Content $Log | Select-String -Pattern "Text Slot|Faction cheat|New game: slot|Test capture|Error:" | ForEach-Object { $_.Line -replace "^.*?\]", "" } }
$Shots = Join-Path $Dir "Screenshots\WindowsEditor\HOL"
$Made = @("faction_A_without.png", "faction_B_with.png") | Where-Object { (Test-Path (Join-Path $Shots $_)) -and (Get-Item (Join-Path $Shots $_)).Length -gt 0 }
Write-Host "faction_shots : $(if ($Made.Count -eq 2) { 'PASS' } else { 'FAIL' }) ($Shots)"
exit $(if ($Made.Count -eq 2) { 0 } else { 1 })
