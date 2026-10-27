# Two-client match on a dedicated server (D268, M2.5): the dedicated server (HeroesOfLibertyServer, D231) on Plain with
# a match configuration of two humans (-MatchConfig=, D243), then two game clients side by side on the second screen,
# joining by address. The server needs its cooked, staged build (-Stage, a few minutes; again after a change of code,
# content or configuration the server uses). Logs: Saved_Tests\Logs\net_server.log, net_client1.log, net_client2.log.
# Each client, once in the match: a capture net_client<N> (from the game, Saved_Tests_Net<N>\Screenshots\WindowsEditor\HOL),
# then a click in the middle of its screen, on its own centre, and a selection report.
# With -Units N (S5, D273): the server runs on StressTest and starts S2 (or -Test) with N units once the clients are in;
# server and clients log a unit replication report every 5 s and the positions of a sample of units (hol.Net.Sample); once it has
# reported, the server quits, the clients are closed and s5_report.py prints the S5 report (bandwidth, sends, step
# times, frame rates, displayed error) with its PASS / FAIL. -Keep leaves everything running for a look, without report.
# S5 knobs: -UpdateFrequency (updates per second of the shared actor), -Threshold (send threshold, cm), -RefreshSteps
# (cycle of the slow refresh, in steps), -Lag (ms added to each packet, both ways), -Loss (% of packets
# lost, both ways) and -ServerSettings (console settings of the server, for A / B measures).
# -Test picks the server's stress test: S2 (the walk, the S5 of D273, by default), S3 (S5 in combat, J7: 1000 against
# 1000 in melee, the worst case for the health sent per hit, ~590 hits per second; its measure begins once the armies
# are joined, ~5 s after the charge, up to 40 s of warm-up, and lasts -Seconds, 20 by default) or S4 (mixed armies:
# the arrows are deduced by the clients, no extra bits). S3 and S4 turn the combat on themselves (hol.Combat.Enabled 1).
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\net_match.ps1 [-Stage] [-Clients 2] [-Units 2000 [-Test S2|S3|S4] [-Seconds 40] [-Keep]]
param(
    [switch] $Stage,
    [int] $Clients = 2,
    [string] $Map = "/Game/Maps/Plain",
    [int] $Units = 0,
    [ValidateSet("S2", "S3", "S4")] [string] $Test = "S2",
    [int] $Seconds = 40,
    [float] $UpdateFrequency = 0,
    [float] $Threshold = 0,
    [int] $RefreshSteps = 0,
    [int] $Lag = 0,
    [int] $Loss = 0,
    [string[]] $ServerSettings = @(),
    # Extra command-line arguments of the server (e.g. a network trace: -ServerArgs "-trace=net -NetTrace=3 -tracefile=...").
    [string] $ServerArgs = "",
    [switch] $Keep,
    # Several matches at once (R5.5): the server's port (0 = the engine's 7777) and a tag that keeps each run's logs,
    # match config and client settings apart (net_server<Tag>.log, Saved_Tests_Net<Tag><N>).
    [int] $Port = 0,
    [string] $Tag = "",
    [string] $EngineDir = ""
)
if ($Units -gt 0 -and -not $PSBoundParameters.ContainsKey('Map')) { $Map = "/Game/Maps/StressTest" }
# A battle (S3, S4): 20 s of melee by default (~10 % of the units fall in 15 s), three whole replication reports.
$Battle = $Test -ne "S2"
if ($Battle -and -not $PSBoundParameters.ContainsKey('Seconds')) { $Seconds = 20 }

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
    # Server only, Plain, its five sizes (TC3: a lobby plays Plain_<Size>) and StressTest (S5); UAT's own log goes to Saved_Tests\Logs\net_stage.log. -pak: the cooked
    # data is staged with the server; without it the server streams it from the Zen server that ran at staging time, and
    # waits forever once another Zen instance has replaced it (another editor started).
    $StageLog = Join-Path $Logs "net_stage.log"
    & (Join-Path $EngineDir "Build\BatchFiles\RunUAT.bat") BuildCookRun -project="$UProject" -noP4 -server -serverplatform=Win64 `
        -noclient -build -cook -stage -pak -map=/Game/Maps/Plain+/Game/Maps/Plain_Small+/Game/Maps/Plain_Medium+/Game/Maps/Plain_Normal+/Game/Maps/Plain_Large+/Game/Maps/Plain_Huge+/Game/Maps/StressTest -serverconfig=Development -unattended -utf8output *> $StageLog
    if ($LASTEXITCODE -ne 0) { Write-Host "net_match : staging FAILED (see $StageLog)"; exit 1 }
}
$Server = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Binaries\Win64\HeroesOfLibertyServer.exe"
if (-not (Test-Path $Server)) { Write-Host "net_match : no staged server ($Server); run with -Stage"; exit 1 }
# A staged server without Version.txt is "unknown" and refuses everyone (HOLVersion, R3.3): this test stage serves
# "dev", as the clients from the editor (package.ps1 writes the real version into a package).
[IO.File]::WriteAllText((Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Version.txt"), "dev", (New-Object Text.UTF8Encoding $false))

# Two humans in slots 0 and 1, the colours of their slots; the clients take the free human slots in order.
$Slots = for ($i = 0; $i -lt 8; $i++) { "{ `"state`": `"$(if ($i -lt $Clients) { 'Human' } else { 'Closed' })`", `"colorIndex`": $i }" }
$Config = Join-Path $TestSaved "net_match$Tag.json"
"{ `"map`": `"$Map.$(Split-Path $Map -Leaf)`", `"slots`": [ $($Slots -join ', ') ] }" | Set-Content -Encoding ascii $Config

$ServerLog = Join-Path $Logs "net_server$Tag.log"
# The logs of the last run go first: the waits below read the new ones.
Remove-Item -ErrorAction SilentlyContinue $ServerLog, (Join-Path $Logs "net_client$Tag*.log")
# -useperfthreads (J7c.4, S5 in combat): a dedicated server runs every ParallelFor on one thread by the engine's default
# (FApp::ShouldUseThreadingForPerformance is false there), so the parallel processors of D302 did not run in parallel
# (server step 14 ms in combat against ~6 ms in the game).
# -HOLCheats (R3.5, R-Q7): a test server keeps the clients' cheats; the manager's servers refuse them.
$ServerArguments = "$Map -log -unattended -as-exit-on-error -useperfthreads -HOLCheats -MatchConfig=`"$Config`" -abslog=`"$ServerLog`""
if ($ServerArgs) { $ServerArguments += " $ServerArgs" }
if ($Port -gt 0) { $ServerArguments += " -port=$Port" }
$Address = if ($Port -gt 0) { "127.0.0.1:$Port" } else { "127.0.0.1" }
# Commands both sides run at start: reports and samples of S5, network emulation.
$NetCommands = @()
if ($Units -gt 0) { $NetCommands += "hol.Net.ReportInterval 5", "hol.Net.Sample 100" }
if ($Lag -gt 0) { $NetCommands += "NetEmulation.PktLag $Lag" }
if ($Loss -gt 0) { $NetCommands += "NetEmulation.PktLoss $Loss" }
$ServerCommands = $NetCommands
if ($Units -gt 0) {
    # S2 starts once the clients have joined (hol.Net.WhenClients), so they receive the births as they happen and none
    # is still loading during the measure; the server quits after S2's report unless the match is kept for a look.
    if ($UpdateFrequency -gt 0) { $ServerCommands += "hol.Net.UpdateFrequency $UpdateFrequency" }
    if ($Threshold -gt 0) { $ServerCommands += "hol.Net.ErrorThreshold $Threshold" }
    if ($RefreshSteps -gt 0) { $ServerCommands += "hol.Net.RefreshSteps $RefreshSteps" }
    # Other console settings of the server, for A / B measures (e.g. -ServerSettings "hol.Units.Avoid 0").
    $ServerCommands += $ServerSettings
    $ServerCommands += "hol.Net.WhenClients $Clients hol.StressTest $Test $Units $Seconds$(if (-not $Keep) { ' quit' })"
}
if ($ServerCommands) { $ServerArguments += " -ExecCmds=`"$($ServerCommands -join ',')`"" }
$ServerProcess = Start-Process -FilePath $Server -ArgumentList $ServerArguments `
    -PassThru -WindowStyle Minimized
Write-Host "server: pid $($ServerProcess.Id), log $ServerLog"

# The clients wait for the server to listen, then join; side by side on the second screen (1920 x 1080 from x = 3840).
$Deadline = (Get-Date).AddSeconds(60)
while (-not ((Test-Path $ServerLog) -and (Select-String -Path $ServerLog -Pattern "listening on port|Listening on port" -Quiet))) {
    if ((Get-Date) -gt $Deadline -or $ServerProcess.HasExited) { Write-Host "net_match : the server did not start listening (see $ServerLog)"; exit 1 }
    Start-Sleep -Milliseconds 250
}
$Width = [Math]::Floor(1920 / $Clients)
$ClientProcesses = @()
$ClientLogs = @()
for ($Client = 1; $Client -le $Clients; $Client++) {
    $ClientLog = Join-Path $Logs "net_client$Tag$Client.log"
    $Sequence = Join-Path $TestSaved "net_client$Tag$Client.seq"
    if ($Units -gt 0) {
        # The stress test starts once both clients are in: a capture while the units walk (S2) or fight (S3, S4: the
        # armies are joined ~5 s after their birth).
        @("waitmap $(Split-Path $Map -Leaf)", "wait 20", "capture net_client$Client") | Set-Content -Encoding utf8 $Sequence
    } else {
        @("waitmap $(Split-Path $Map -Leaf)", "waitlog Camera above the start", "frames 60", "capture net_client$Client",
            "hol.Camera.TestCursor 0.5 0.5", "hol.Select.Click 0.5 0.5", "waitlog Selection:", "hol.Select.Report",
            "hol.Camera.TestCursor off") | Set-Content -Encoding utf8 $Sequence
    }
    # Each client its own settings (Saved_Tests_Net<N>): the engine re-centres a window on the display saved in the
    # settings when it applies the resolution, which stacked the clients; with no display saved (-1) it keeps -WinX.
    $ClientSettings = Join-Path $Project "Saved_Tests_Net$Tag$Client\Config\WindowsEditor\GameUserSettings.ini"
    New-Item -ItemType Directory -Force (Split-Path $ClientSettings) | Out-Null
    (Get-Content (Join-Path $PSScriptRoot "GameUserSettings.ini")) `
        -replace '^(ResolutionSizeX|LastUserConfirmedResolutionSizeX)=.*', "`$1=$($Width - 20)" `
        -replace '^(ResolutionSizeY|LastUserConfirmedResolutionSizeY)=.*', '$1=1000' `
        -replace '^(DisplayIndex|LastUserConfirmedDisplayIndex)=.*', '$1=-1' | Set-Content -Encoding ascii $ClientSettings
    $X = 3840 + ($Client - 1) * $Width
    $Arguments = "`"$UProject`" $Address -game -windowed -ResX=$($Width - 20) -ResY=1000 -WinX=$X -WinY=40 -saveddirsuffix=Tests_Net$Tag$Client -nosplash -LiveCoding=0 -as-exit-on-error -abslog=`"$ClientLog`" -ExecCmds=`"$(($NetCommands + "hol.Test.Run $Sequence") -join ',')`""
    $ClientProcess = Start-Process -FilePath (Join-Path $EngineDir "Binaries\Win64\UnrealEditor.exe") -ArgumentList $Arguments -PassThru
    $ClientProcesses += $ClientProcess
    $ClientLogs += $ClientLog
    Write-Host "client $Client : pid $($ClientProcess.Id), log $ClientLog"
}
if ($Units -le 0 -or $Keep) { exit 0 }

# S5: wait for the stress test's report (the server quits after it), close the clients, then the report. A battle
# waits up to 40 s more for its armies to join.
$Deadline = (Get-Date).AddSeconds(60 + $(if ($Battle) { 40 } else { 0 }) + $Seconds + 120)
while (-not (Select-String -Path $ServerLog -Pattern "Stress test $Test \([^)]*\), \d+ units" -Quiet)) {
    if ((Get-Date) -gt $Deadline -or $ServerProcess.HasExited) { Write-Host "net_match : $Test did not report (see $ServerLog)"; break }
    Start-Sleep -Seconds 1
}
Start-Sleep -Seconds 2
$ClientProcesses | Where-Object { -not $_.HasExited } | Stop-Process -Force
if (-not $ServerProcess.WaitForExit(30000)) { Stop-Process -Id $ServerProcess.Id -Force }
& python (Join-Path $PSScriptRoot "s5_report.py") $ServerLog @ClientLogs
exit $LASTEXITCODE
