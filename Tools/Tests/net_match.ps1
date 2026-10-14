# Two-client match on a dedicated server (D268, M2.5): the dedicated server (HeroesOfLibertyServer, D231) on Plain with
# a match configuration of two humans (-MatchConfig=, D243), then two game clients side by side on the second screen,
# joining by address. The server needs its cooked, staged build (-Stage, a few minutes; again after a change of code,
# content or configuration the server uses). Logs: Saved_Tests\Logs\net_server.log, net_client1.log, net_client2.log.
# Each client, once in the match: a capture net_client<N> (from the game, Saved_Tests_Net<N>\Screenshots\WindowsEditor\HOL),
# then a click in the middle of its screen, on its own centre, and a selection report.
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\net_match.ps1 [-Stage] [-Clients 2]
param(
    [switch] $Stage,
    [int] $Clients = 2,
    [string] $Map = "/Game/Maps/Plain",
    [string] $EngineDir = ""
)

$Project = Resolve-Path "$PSScriptRoot\..\.."
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    # An installed build registered under its name (HKCU\Software\Epic Games\Unreal Engine\Builds).
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $Root = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $Root) { Write-Host "net_match : engine '$Association' of the .uproject not found in the registry; pass -EngineDir"; exit 1 }
    $EngineDir = Join-Path $Root "Engine"
}
$TestSaved = Join-Path $Project "Saved_Tests"
$Logs = Join-Path $TestSaved "Logs"
New-Item -ItemType Directory -Force $Logs | Out-Null

if ($Stage) {
    # Server only, Plain and StressTest (S5); UAT's own log goes to Saved_Tests\Logs\net_stage.log.
    $StageLog = Join-Path $Logs "net_stage.log"
    & (Join-Path $EngineDir "Build\BatchFiles\RunUAT.bat") BuildCookRun -project="$UProject" -noP4 -server -serverplatform=Win64 `
        -noclient -build -cook -stage -map=/Game/Maps/Plain+/Game/Maps/StressTest -serverconfig=Development -unattended -utf8output *> $StageLog
    if ($LASTEXITCODE -ne 0) { Write-Host "net_match : staging FAILED (see $StageLog)"; exit 1 }
}
$Server = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Binaries\Win64\HeroesOfLibertyServer.exe"
if (-not (Test-Path $Server)) { Write-Host "net_match : no staged server ($Server); run with -Stage"; exit 1 }

# Two humans in slots 0 and 1, the colours of their slots; the clients take the free human slots in order.
$Slots = for ($i = 0; $i -lt 8; $i++) { "{ `"state`": `"$(if ($i -lt $Clients) { 'Human' } else { 'Closed' })`", `"colorIndex`": $i }" }
$Config = Join-Path $TestSaved "net_match.json"
"{ `"map`": `"$Map.$(Split-Path $Map -Leaf)`", `"slots`": [ $($Slots -join ', ') ] }" | Set-Content -Encoding ascii $Config

$ServerLog = Join-Path $Logs "net_server.log"
$ServerProcess = Start-Process -FilePath $Server -ArgumentList "$Map -log -unattended -MatchConfig=`"$Config`" -abslog=`"$ServerLog`"" `
    -PassThru -WindowStyle Minimized
Write-Host "server: pid $($ServerProcess.Id), log $ServerLog"

# The clients wait for the server to listen, then join; side by side on the second screen (1920 x 1080 from x = 3840).
$Deadline = (Get-Date).AddSeconds(60)
while (-not ((Test-Path $ServerLog) -and (Select-String -Path $ServerLog -Pattern "listening on port|Listening on port" -Quiet))) {
    if ((Get-Date) -gt $Deadline -or $ServerProcess.HasExited) { Write-Host "net_match : the server did not start listening (see $ServerLog)"; exit 1 }
    Start-Sleep -Milliseconds 250
}
$Width = [Math]::Floor(1920 / $Clients)
for ($Client = 1; $Client -le $Clients; $Client++) {
    $ClientLog = Join-Path $Logs "net_client$Client.log"
    $Sequence = Join-Path $TestSaved "net_client$Client.seq"
    @("waitmap $(Split-Path $Map -Leaf)", "waitlog Camera above the start", "frames 60", "capture net_client$Client",
        "hol.Camera.TestCursor 0.5 0.5", "hol.Select.Click 0.5 0.5", "waitlog Selection:", "hol.Select.Report",
        "hol.Camera.TestCursor off") | Set-Content -Encoding utf8 $Sequence
    # Each client its own settings (Saved_Tests_Net<N>): the engine re-centres a window on the display saved in the
    # settings when it applies the resolution, which stacked the clients; with no display saved (-1) it keeps -WinX.
    $ClientSettings = Join-Path $Project "Saved_Tests_Net$Client\Config\WindowsEditor\GameUserSettings.ini"
    New-Item -ItemType Directory -Force (Split-Path $ClientSettings) | Out-Null
    (Get-Content (Join-Path $PSScriptRoot "GameUserSettings.ini")) `
        -replace '^(ResolutionSizeX|LastUserConfirmedResolutionSizeX)=.*', "`$1=$($Width - 20)" `
        -replace '^(ResolutionSizeY|LastUserConfirmedResolutionSizeY)=.*', '$1=1000' `
        -replace '^(DisplayIndex|LastUserConfirmedDisplayIndex)=.*', '$1=-1' | Set-Content -Encoding ascii $ClientSettings
    $X = 3840 + ($Client - 1) * $Width
    $Arguments = "`"$UProject`" 127.0.0.1 -game -windowed -ResX=$($Width - 20) -ResY=1000 -WinX=$X -WinY=40 -saveddirsuffix=Tests_Net$Client -nosplash -abslog=`"$ClientLog`" -ExecCmds=`"hol.Test.Run $Sequence`""
    $ClientProcess = Start-Process -FilePath (Join-Path $EngineDir "Binaries\Win64\UnrealEditor.exe") -ArgumentList $Arguments -PassThru
    Write-Host "client $Client : pid $($ClientProcess.Id), log $ClientLog"
}
