# Screenshots of what F7 / F8 compiled but never showed (FogInProgress.md F9), off screen, no window. Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\fog_shots.ps1 -What Menu|Client [-Root <copy>]
# Menu  : the New game screen with its "Brouillard" list (Normal, then "Carte explorée" chosen), captures newgame_fog_a / _b
#         (Saved_Tests_Shots\Screenshots\WindowsEditor\HOL), and hol.UI.Text FogDropdown read back in the log.
# Client: the server and two clients of the "netfog" part of quick_check (Alice, Bob 300 m apart, Normal fog); Alice's units go and see
#         Bob's centre and come home, the server then cuts trees along her march while she does not see them; Alice's client
#         captures Bob's centre as a ghost (fogclient_ghost) and reports "Sources (client): N standing ... shown M" before and after the
#         cut (shown stays above standing = trees kept drawn). Her saved folder: Saved_Tests_Net2.
# Prints the lines that matter; exit code 0 when the captures exist.
param(
    [ValidateSet("Menu", "Client")] [string] $What = "Menu",
    [string] $Root = "",
    [string] $EngineDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "fog_shots : FAIL (engine '$Association' not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Editor = Join-Path $EngineDir "UnrealEditor.exe"
# Another copy's build makes every launch wait on UBT's platform check for minutes (the clients then miss the server): skip it, as play.ps1.
$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Work = Join-Path $Project "Saved_Tests"
New-Item -ItemType Directory -Force (Join-Path $Work "Logs") | Out-Null

# A saved folder reused off screen sometimes gives a black world: a new one each time.
function New-ShotFolder([string] $Suffix) {
    $Dir = Join-Path $Project "Saved_$Suffix"
    if (Test-Path $Dir) { Remove-Item -Recurse -Force $Dir -ErrorAction SilentlyContinue }
    $Dir
}

if ($What -eq "Menu") {
    $Suffix = "Tests_Shots"
    $Dir = New-ShotFolder $Suffix
    $Log = Join-Path $Work "Logs\fog_shots_menu.log"
    $Sequence = Join-Path $Work "fog_shots_menu.seq"
    Remove-Item $Log -ErrorAction SilentlyContinue
    @("waitmap MainMenu", "waitui", "hol.UI.Open NewGame", "waitui", "frames 20", "hol.UI.Text FogDropdown", "capture newgame_fog_a",
        "hol.UI.Choose FogDropdown 1", "frames 10", "hol.UI.Text FogDropdown", "capture newgame_fog_b", "quit") | Set-Content -Encoding ascii $Sequence
    $Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -nosplash -unattended -culture=fr -LiveCoding=0 -as-exit-on-error -asdebugport=0 -saveddirsuffix=$Suffix -abslog=`"$Log`" -ExecCmds=`"hol.Test.Run $Sequence`""
    $Process = Start-Process -FilePath $Editor -ArgumentList $Arguments -PassThru
    if (-not $Process.WaitForExit(180000)) { $Process.Kill() }
    if (Test-Path $Log) { Get-Content $Log | Select-String -Pattern "Text FogDropdown|New game: fog|Test capture|Error:" | ForEach-Object { $_.Line -replace "^.*?\]", "" } }
    $Shots = Join-Path $Dir "Screenshots\WindowsEditor\HOL"
    $Made = @("newgame_fog_a.png", "newgame_fog_b.png") | Where-Object { (Test-Path (Join-Path $Shots $_)) -and (Get-Item (Join-Path $Shots $_)).Length -gt 0 }
    Write-Host "fog_shots Menu : $(if ($Made.Count -eq 2) { 'PASS' } else { 'FAIL' }) ($Shots)"
    exit $(if ($Made.Count -eq 2) { 0 } else { 1 })
}

# ---- Client
$Config = Join-Path $Work "fog_shots_client.json"
'{ "map": "/Game/Maps/Plain.Plain", "startingPeasants": 5, "slots": [ { "state": "Human", "colorIndex": 0, "playerName": "Alice" }, { "state": "Human", "colorIndex": 1, "playerName": "Bob" } ] }' | Set-Content -Encoding ascii $Config
$ServerLog = Join-Path $Work "Logs\fog_shots_server.log"
$ServerSequence = Join-Path $Work "fog_shots_server.seq"
$Done = Join-Path $Work "fog_shots_done.flag"
Remove-Item $ServerLog, (Join-Path $Work "Logs\fog_shots_client*.log"), $Done -ErrorAction SilentlyContinue
# Only beyond Alice's home sight (64 m from -14800) and inside the band her march explored (y = 0).
$Cuts = @(-4000, -2000, 0, 2000, 4000, 6000, 8000, 10000, 12000) | ForEach-Object { "hol.Sources.TakeNear $_ 0" }
@("waitplayers 2", "mark fogshots", "wait 8", "hol.Units.SpeedScale 10", "hol.Units.Move 12500 0 0", "wait 14", "hol.Units.Move -15200 0 0", "wait 20") + $Cuts +
    @("hol.Sources.Report", "waitfile $Done 240", "wait 1", "quit") | Set-Content -Encoding utf8 $ServerSequence
$Port = 17800 + (Get-Random -Maximum 200)
$ServerArguments = "`"$UProject`" /Game/Maps/Plain -server -Port=$Port -useperfthreads -unattended -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -nosound -saveddirsuffix=Tests_NetSrv -HOLCheats -MatchConfig=`"$Config`" -abslog=`"$ServerLog`" -ExecCmds=`"hol.Fog.Mode 2,hol.Test.Run $ServerSequence`""
$Server = Start-Process -FilePath $Editor -ArgumentList $ServerArguments -PassThru -WindowStyle Hidden
$Listening = $false
$Deadline = (Get-Date).AddSeconds(90)
while (-not $Listening -and (Get-Date) -lt $Deadline -and -not $Server.HasExited) {
    Start-Sleep -Milliseconds 250
    $Listening = (Test-Path $ServerLog) -and (Select-String -Path $ServerLog -Pattern "listening on port" -Quiet)
}
if (-not $Listening) { Write-Host "fog_shots Client : FAIL (server not listening)"; if (-not $Server.HasExited) { $Server.Kill() }; exit 1 }
$Clients = @()
foreach ($Client in 1, 2) {
    $Suffix = "Tests_Net$Client"
    $Settings = Join-Path $Project "Saved_$Suffix\Config\WindowsEditor\GameUserSettings.ini"
    if (-not (Test-Path $Settings)) {
        New-Item -ItemType Directory -Force (Split-Path $Settings) | Out-Null
        (Get-Content (Join-Path $PSScriptRoot "GameUserSettings.ini")) -replace '^(DisplayIndex|LastUserConfirmedDisplayIndex)=.*', '$1=-1' | Set-Content -Encoding ascii $Settings
    }
    $Sequence = Join-Path $Work "fog_shots_client$Client.seq"
    # Alice (client 2): sources before the cut (the server cuts at ~42 s after its mark), then Bob's centre seen as a ghost, then after the cut.
    $Lines = if ($Client -eq 2) {
        @("waitmap Plain", "waitlog Camera above the start", "waitplayers 2", "mark fogshots", "hol.Fog.Render 1", "wait 34", "hol.Sources.Report", "hol.Fog.Report",
          "hol.Camera.FocusSlot 1", "hol.Camera.Zoom -2", "frames 30", "capture fogclient_ghost", "hol.Buildings.Report", "wait 14", "hol.Sources.Report", "hol.Fog.Report",
          "hol.Camera.FocusSlot 0", "frames 30", "capture fogclient_home", "signal $Done", "mark end", "quit")
    } else {
        @("waitmap Plain", "waitlog Camera above the start", "waitplayers 2", "mark fogshots", "waitfile $Done 240", "mark end", "quit")
    }
    $Lines | Set-Content -Encoding utf8 $Sequence
    $ClientLog = Join-Path $Work "Logs\fog_shots_client$Client.log"
    $Url = if ($Client -eq 1) { "127.0.0.1:${Port} -PlayerName=Bob" } else { "127.0.0.1:${Port}?Name=Alice" }
    $Arguments = "`"$UProject`" $Url -game -RenderOffScreen -ResX=960 -ResY=540 -nosound -saveddirsuffix=$Suffix -nosplash -unattended -LiveCoding=0 -asdebugport=0 -as-exit-on-error -culture=fr -abslog=`"$ClientLog`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Test.Run $Sequence`""
    $Clients += @{ Client = $Client; Log = $ClientLog; Process = (Start-Process -FilePath $Editor -ArgumentList $Arguments -PassThru) }
}
foreach ($C in $Clients) { if (-not $C.Process.WaitForExit(240000)) { $C.Process.Kill() } }
if (-not $Server.WaitForExit(30000)) { $Server.Kill() }
$Alice = (Get-Content (Join-Path $Work "Logs\fog_shots_client2.log")) | Select-String -Pattern "Sources \(client\)|Fog: mode|Building fog|Test capture|Test mark"
$Alice | ForEach-Object { $_.Line -replace "^.*?\]", "" }
(Get-Content $ServerLog) | Select-String -Pattern "Sources \(server\)|took" | Select-Object -Last 4 | ForEach-Object { $_.Line -replace "^.*?\]", "" }
$Shots = Join-Path $Project "Saved_Tests_Net2\Screenshots\WindowsEditor\HOL"
$Made = @("fogclient_ghost.png", "fogclient_home.png") | Where-Object { (Test-Path (Join-Path $Shots $_)) -and (Get-Item (Join-Path $Shots $_)).Length -gt 0 }
Write-Host "fog_shots Client : $(if ($Made.Count -eq 2) { 'PASS' } else { 'FAIL' }) ($Shots)"
exit $(if ($Made.Count -eq 2) { 0 } else { 1 })