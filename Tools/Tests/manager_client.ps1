# The game's client of the lobby manager against the real manager (M6 R4.1): the published HOLManager.exe
# (Tools\Manager\build_manager.ps1) on a free port, one lobby created by hand, then the game off screen with a test
# servers.json (-ServersFile=) asks for the status and the lobbies (hol.Manager.Status, hol.Manager.Lobbies).
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\manager_client.ps1 [-EngineDir <...\Engine\Binaries\Win64>] [-Screen]
# -Screen (R4.2): the Multiplayer screen instead, off screen (-RenderOffScreen): opened from the menu, its list of 1 lobby
# waited for and captured (Saved_Tests\Screenshots\...\HOL\manager_client_multiplayer.png), then the row's Join pressed
# (its intention logged).
# -Case Off / OtherVersion (R4.3, the screen too): no manager on the port of servers.json, or a manager serving another
# game version (--GameVersion=); the screen's red status line is waited for in the log ("Multiplayer: error ...") and
# captured (manager_client_off.png, manager_client_version.png). The game off screen is "dev" (HOLVersion), so the
# normal cases serve "dev".
# -Case Lobby (R4.4): the lobby is created by "Alice" (kept alive), the game ("Bob", -PlayerName=) joins it from the
# list, waits for "2 players" in its log and captures the lobby screen (manager_client_lobby.png); the manager is asked
# whether Bob is in, then the game presses Back and the manager is asked whether Bob is out.
# -Case Ready (R4.5): as Lobby, then Bob takes the colour 5 in his line's list (hol.UI.Choose SlotColor1 5) and presses
# Ready; the lobby is captured (manager_client_ready.png) and the manager is asked whether Bob has the colour 5 and is
# ready; Bob, not the leader, must log "launch hidden".
# -Case Leader (R4.5): the game ("Bob") creates the lobby and leads it; "Alice" joins by HTTP; Bob closes the free slot 2
# (hol.UI.Choose SlotState2 1), then Alice is made ready by HTTP, Bob presses Ready, the lobby is captured
# (manager_client_leader.png) and Bob presses Launch. Without a server (ServerExe of the test manager missing) the manager
# answers 500 server_start_failed and the lobby stays; with one, 202 and "starting": either proves Launch went out.
# -Case Match (R4.7): as Leader, with the staged dedicated server (Saved\StagedBuilds\WindowsServer, net_match.ps1 -Stage)
# as the test manager's ServerExe: after Launch the lobby goes in_game and the game logs "Lobby: joining match M... at
# 127.0.0.1:<port> as Bob" with the port the manager gives; the server's own log is printed (arrival not required yet).
# -Case Return (R4.8): as Match, then Bob gives up once the start wait for the absent Alice is over (hol.Victory.Surrender,
# no cheat needed; Alice's team wins, the server sends finished): the end banner with « Retour au salon (n s) » and
# « Quitter vers la liste » is captured (manager_client_return.png), « Retour au salon » pressed; the game logs "Menu:
# back in lobby" and the manager has Bob in the lobby, state "lobby", on a new line (M6-Q14: not ready, Alice not kept,
# Bob leader). No HeroesOfLibertyServer of this copy is left.
# -Case Killed (R4.9): as Match until the server logs "Bob holds slot 0", then the match's server (this copy's
# HeroesOfLibertyServer processes, by their PIDs) is killed: the game goes back to the menu and opens the Multiplayer
# list with the red reason ("Multiplayer: lobby lost: ...", manager_client_killed.png); the time from the kill to the
# game's "Manager: match lost" is printed. No HeroesOfLibertyServer of this copy is left.
# -Case NameTaken: the game joins under the name "Alice": the red line "Ce nom est deja pris" is waited for and captured.
# Prints one line PASS / FAIL; exit code 0 / 1. The manager is always stopped.
param([string] $EngineDir = "", [switch] $Screen, [ValidateSet("", "Off", "OtherVersion", "Lobby", "NameTaken", "Ready", "Leader", "Match", "Return", "Killed")] [string] $Case = "")
if ($Case) { $Screen = $true }
$ErrorCase = $Case -in @("Off", "OtherVersion")
$LobbyCase = $Case -in @("Lobby", "NameTaken", "Ready")

$Project = (Resolve-Path "$PSScriptRoot\..\..").Path
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $Root = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $Root) { Write-Host "manager_client : FAIL (engine '$Association' not found; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $Root "Engine\Binaries\Win64"
}
$Exe = Join-Path $Project "Saved\Manager\HOLManager.exe"
if (-not (Test-Path $Exe)) { Write-Host "manager_client : FAIL (no $Exe; run Tools\Manager\build_manager.ps1)"; exit 1 }
$TestSaved = Join-Path $Project "Saved_Tests"
$Work = Join-Path $TestSaved "ManagerClient"
New-Item -ItemType Directory -Force $Work | Out-Null
$GameLog = Join-Path $TestSaved "Logs\manager_client_game.log"
$ManagerLog = Join-Path $Work "manager.log"
New-Item -ItemType Directory -Force (Split-Path $GameLog) | Out-Null
Remove-Item $GameLog, $ManagerLog -ErrorAction SilentlyContinue

# A free TCP port from 7790 (another copy's manager may hold 7700).
$Port = 7790
while ($Port -lt 7890) {
    try { $Probe = New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback, $Port); $Probe.Start(); $Probe.Stop(); break } catch { $Port++ }
}
$Url = "http://127.0.0.1:${Port}"
$LobbyName = "Salon R4 test"
$Failure = $null
$OtherVersion = "2026-10-24-other"
$Served = if ($Case -eq "OtherVersion") { $OtherVersion } else { "dev" }
$Manager = $null
$ManagerArguments = "--urls $Url --GameVersion=$Served"
if ($Case -in @("Match", "Return", "Killed")) {
    $ServerExe = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLibertyServer.exe"
    if (-not (Test-Path $ServerExe)) { Write-Host "manager_client : FAIL (no $ServerExe; run Tools\Tests\net_match.ps1 -Stage)"; exit 1 }
    $ManagerArguments += " --ServerExe=`"$ServerExe`" --ManagerUrl=$Url"
    # A staged server without Version.txt is "unknown" and refuses everyone (HOLVersion): "dev" for the time of the
    # test, as the game in the editor (package.ps1 writes the real one into a package).
    $ServerVersion = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Version.txt"
    $ServerVersionMade = -not (Test-Path $ServerVersion)
    if ($ServerVersionMade) { [IO.File]::WriteAllText($ServerVersion, "dev", (New-Object Text.UTF8Encoding $false)) }
}
if ($Case -ne "Off") {
    $Manager = Start-Process -FilePath $Exe -ArgumentList $ManagerArguments -WorkingDirectory $Work -PassThru -WindowStyle Hidden -RedirectStandardOutput $ManagerLog
}
try {
    $Servers = Join-Path $Work "servers.json"
    "{ `"servers`": [ { `"name`": `"Test`", `"url`": `"$Url`" } ] }" | Set-Content -Encoding ascii $Servers
    $Created = $null
    if ($ErrorCase) {
        # The error cases: the screen opened, its red status line waited for and captured; no lobby needed.
        $Shot = if ($Case -eq "Off") { "manager_client_off" } else { "manager_client_version" }
        $Sequence = Join-Path $Work "multiplayer_error.seq"
        @("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: error", "frames 10",
            "capture $Shot", "quit") | Set-Content -Encoding utf8 $Sequence
        $Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -unattended -nosplash -LiveCoding=0 -culture=fr -saveddirsuffix=Tests " +
            "-ServersFile=`"$Servers`" -ExecCmds=`"hol.Test.Run $Sequence`" -abslog=`"$GameLog`""
        $Game = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
        if (-not $Game.WaitForExit(180000)) { Stop-Process -Id $Game.Id -Force -ErrorAction SilentlyContinue }
        $Lines = if (Test-Path $GameLog) { @(Select-String -Path $GameLog -Encoding utf8 -Pattern "Multiplayer|Test capture|Test run" | ForEach-Object { $_.Line }) } else { @() }
        $Lines | ForEach-Object { Write-Host "  $_" }
        # Accented letters matched by "." (this file is read as ANSI by Windows PowerShell).
        $Expected = if ($Case -eq "Off") { "Multiplayer: error Le serveur de parties ne r.pond pas\." }
            else { "Multiplayer: error Version diff.rente : le serveur a $([regex]::Escape($OtherVersion)), vous avez dev\." }
        if (-not ($Lines -match "LogHOLUI: Display: $Expected")) { throw "no '$Expected'" }
        if ($Lines -match "Multiplayer: \d+ lobbies shown") { throw "lobbies shown despite the error" }
        if (-not ($Lines -match "LogHOLTest: Display: Test capture: $Shot")) { throw "no capture" }
    }
    else {
        $Status = $null
        for ($i = 0; $i -lt 40 -and -not $Status; $i++) {
            try { $Status = Invoke-RestMethod "${Url}/status" -TimeoutSec 2 } catch { Start-Sleep -Milliseconds 250 }
        }
        if (-not $Status) { throw "the manager does not answer on $Url" }
    if ($Case -in @("Leader", "Match", "Return", "Killed")) {
        $IsMatch = $Case -in @("Match", "Return", "Killed")
        $AfterMatch = if ($Case -eq "Return") { @("wait 30", "wait 30", "wait 8", "hol.Victory.Surrender", "waitlog End banner: lobby match", "wait 1.5", "capture manager_client_return", "hol.UI.Focus ReturnButton", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15", "waitmap MainMenu", "waitlog Menu: back in lobby", "wait 3") }
            elseif ($Case -eq "Killed") { @("waitmap MainMenu", "waitui", "waitlog Multiplayer: lobby lost", "frames 10", "capture manager_client_killed", "wait 1") } else { @() }
        $Sequence = Join-Path $Work "leader.seq"
        @("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 0 lobbies shown", "frames 3",
            "hol.UI.Focus CreateButton", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15", "waitlog : 2 players (", "waitui", "wait 1",
            "hol.UI.Choose SlotState2 1", "waitlog Lobby: slot 2 closed accepted", "waitlog 1/2 ready", "wait 0.5",
            "hol.UI.Focus ReadyButton", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15", "waitlog 2/2 ready", "wait 1",
            "capture manager_client_leader", "hol.UI.Focus LaunchButton", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15",
            "waitlog Lobby: start sent", $(if ($IsMatch) { "waitlog Lobby: joining match" } else { "wait 2" }), $(if ($IsMatch) { "waitmap Plain" } else { "wait 0" }), "wait 3") + $AfterMatch + @("quit") | Set-Content -Encoding utf8 $Sequence
        $Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -unattended -nosplash -LiveCoding=0 -culture=fr -saveddirsuffix=Tests " +
            "-PlayerName=Bob -ServersFile=`"$Servers`" -ExecCmds=`"hol.Test.Run $Sequence`" -abslog=`"$GameLog`""
        $Game = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
        $Deadline = (Get-Date).AddSeconds($(if ($Case -in @("Return", "Killed")) { 300 } else { 180 }))
        $Alice = $null; $AliceReady = $false; $Read = $null
        while (-not $Game.HasExited -and (Get-Date) -lt $Deadline) {
            Start-Sleep -Milliseconds 500
            if (-not $Alice) {
                $Found = @((Invoke-RestMethod "${Url}/lobbies" -TimeoutSec 2).lobbies)
                if ($Found.Count -gt 0) {
                    $Body = @{ playerName = "Alice"; gameVersion = $Status.gameVersion } | ConvertTo-Json
                    $Alice = Invoke-RestMethod "${Url}/lobbies/$($Found[0].id)/join" -Method Post -Body $Body -ContentType "application/json; charset=utf-8"
                    Write-Host "Alice joined $($Found[0].id)"
                }
                continue
            }
            if ($Case -eq "Killed" -and $MatchRead) {
                $KillServerLog = Join-Path $Project "Saved\Manager\Saved\Matches\$($MatchRead.matchId)\server.log"
                if (-not $KilledAt -and (Test-Path $KillServerLog) -and (Select-String -Path $KillServerLog -Pattern "LogHOLMatch: Bob holds slot 0" -Quiet)) {
                    Start-Sleep 2
                    # Only this copy's servers (the test manager's), by PID: never another worktree's.
                    $Killed = @(Get-Process HeroesOfLibertyServer* -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path.StartsWith($Project) })
                    $Killed | ForEach-Object { Write-Host "killing server PID $($_.Id) ($($_.Path))"; Stop-Process -Id $_.Id -Force }
                    $KilledAt = Get-Date
                }
                if ($KilledAt -and -not $LostAfter -and (Test-Path $GameLog) -and (Select-String -Path $GameLog -Pattern "LogHOLNet: Display: Manager: match lost" -Quiet)) {
                    $LostAfter = ((Get-Date) - $KilledAt).TotalSeconds
                    Write-Host ("match lost seen {0:N1} s after the kill" -f $LostAfter)
                }
                continue
            }
            if ($Case -eq "Return" -and $MatchRead) {
                # Alice never comes to the match; Bob's way back is read without a token.
                if ((Test-Path $GameLog) -and (Select-String -Path $GameLog -Pattern "LogHOLUI: Display: Menu: back in lobby" -Quiet)) {
                    $Back = Invoke-RestMethod "${Url}/lobbies/$($Alice.lobby.id)" -TimeoutSec 2
                    break
                }
                continue
            }
            $Read = Invoke-RestMethod "${Url}/lobbies/$($Alice.lobby.id)" -Headers @{ "X-Player-Token" = $Alice.token } -TimeoutSec 2
            if (-not $AliceReady -and $Read.slots[2].state -eq "closed") {
                Invoke-RestMethod "${Url}/lobbies/$($Alice.lobby.id)/me" -Method Put -Body '{ "ready": true }' -ContentType "application/json" -Headers @{ "X-Player-Token" = $Alice.token } | Out-Null
                $AliceReady = $true
                Write-Host "Alice ready"
            }
            if ($IsMatch -and $Read.match) { $MatchRead = $Read.match }
            $Until = if ($Case -in @("Return", "Killed")) { "(?!)" } elseif ($IsMatch) { "LogHOLUI: Display: Lobby: (joining match|start refused)" } else { "LogHOLUI: Display: Lobby: start (accepted|refused)" }
            if ((Test-Path $GameLog) -and (Select-String -Path $GameLog -Pattern $Until -Quiet)) {
                $Read = Invoke-RestMethod "${Url}/lobbies/$($Alice.lobby.id)" -Headers @{ "X-Player-Token" = $Alice.token } -TimeoutSec 2
                Start-Sleep 2; break
            }
        }
        if (-not $Game.HasExited) { Stop-Process -Id $Game.Id -Force -ErrorAction SilentlyContinue }
        $Lines = if (Test-Path $GameLog) { @(Select-String -Path $GameLog -Encoding utf8 -Pattern "Multiplayer|LogHOLUI: Display: (Lobby|End banner|Menu)|Test capture|Test run|LogHOLNet: (Display: )?Manager" | ForEach-Object { $_.Line }) } else { @() }
        $Lines | ForEach-Object { Write-Host "  $_" }
        Write-Host "  manager: state $($Read.state), slot 2 $($Read.slots[2].state)"
        if (-not $Alice) { $Failure = "no lobby created by the game" }
        elseif (-not ($Lines -match "you Bob, leader Bob, .* launch shown")) { $Failure = "Bob, the leader, does not log 'launch shown'" }
        elseif (-not ($Lines -match "LogHOLUI: Display: Lobby: slot 2 closed accepted")) { $Failure = "no 'Lobby: slot 2 closed accepted'" }
        elseif (-not ($Lines -match "LogHOLTest: Display: Test capture: manager_client_leader")) { $Failure = "no capture" }
        elseif (-not ($Lines -match "LogHOLUI: Display: Lobby: start sent")) { $Failure = "no 'Lobby: start sent'" }
        elseif ($Lines -match "LogHOLUI: Display: Lobby: start accepted \(HTTP 202\)") { $Started = "202, lobby $($Read.state)" }
        elseif ($Lines -match "LogHOLUI: Display: Lobby: start refused \(HTTP 500 server_start_failed") { $Started = "500 server_start_failed (no server), lobby $($Read.state)" }
        else { $Failure = "Launch neither accepted (202) nor refused for want of a server" }
        if ($IsMatch -and -not $Failure) {
            $ServerLog = if ($MatchRead) { Get-Item (Join-Path $Project "Saved\Manager\Saved\Matches\$($MatchRead.matchId)\server.log") -ErrorAction SilentlyContinue }
            if ($ServerLog) { Write-Host "  server log $($ServerLog.FullName):"; Select-String -Path $ServerLog.FullName -Pattern "LogHOL\w+: .*(Bob|Alice|slot|Version|Manager|listening|Login)" | Select-Object -Last 15 | ForEach-Object { Write-Host "    $($_.Line)" } }
            Write-Host "  manager: match $($MatchRead.matchId) port $($MatchRead.port) as $($MatchRead.playerName)"
            if (-not $Started.StartsWith("202")) { $Failure = "Launch not accepted ($Started)" }
            elseif (-not ($Lines -match "LogHOLUI: Display: Lobby: joining match \S+ at 127\.0\.0\.1:\d+ as Bob")) { $Failure = "no 'Lobby: joining match ... at 127.0.0.1:<port> as Bob'" }
            elseif ($MatchRead -and -not ($Lines -match "joining match $([regex]::Escape($MatchRead.matchId)) at 127\.0\.0\.1:$($MatchRead.port) as Bob")) { $Failure = "joined another address than the manager's match ($($MatchRead.port))" }
            elseif (-not ($Lines -match "LogHOLNet: Display: Manager: open 127\.0\.0\.1:\d+\?Name=Bob\?Version=")) { $Failure = "no 'Manager: open 127.0.0.1:<port>?Name=Bob?Version=...'" }
            elseif (-not $ServerLog -or -not (Select-String -Path $ServerLog.FullName -Pattern "LogHOLMatch: Bob holds slot 0" -Quiet)) { $Failure = "the match's server has no 'Bob holds slot 0'" }
            if ($Case -eq "Killed" -and -not $Failure) {
                if (-not $KilledAt -or -not $Killed.Count) { $Failure = "no server of this copy killed" }
                elseif (-not ($Lines -match "LogHOLNet: Display: Manager: match lost \(match_(lost|closed)")) { $Failure = "no 'Manager: match lost (match_lost|match_closed ...)'" }
                elseif (-not ($Lines -match "LogHOLUI: Display: Multiplayer: lobby lost: La (connexion . la partie est perdue|partie a .t. ferm.e)")) { $Failure = "no red reason over the list" }
                elseif (-not ($Lines -match "LogHOLTest: Display: Test capture: manager_client_killed")) { $Failure = "no capture" }
                $Left = @(Get-Process HeroesOfLibertyServer* -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path.StartsWith($Project) })
                Write-Host "  servers of this copy left: $($Left.Count)"
                if ($Left.Count) { if (-not $Failure) { $Failure = "$($Left.Count) server(s) of this copy still running" }; $Left | Stop-Process -Force }
            }
            if ($Case -eq "Return" -and -not $Failure) {
                $BackBob = if ($Back) { @($Back.slots | Where-Object { $_.playerName -eq "Bob" }).Count } else { 0 }
                $BackLine = if ($Back) { @($Back.slots | Where-Object { $_.playerName -eq "Bob" }) | Select-Object -First 1 }
                $BackAlice = if ($Back) { @($Back.slots | Where-Object { $_.playerName -eq "Alice" }).Count } else { 0 }
                Write-Host "  manager after the match: state $($Back.state), Bob in it: $BackBob (ready $($BackLine.ready), color $($BackLine.colorIndex), team $($BackLine.team)), Alice in it: $BackAlice, leader $($Back.leader)"
                if (-not ($Lines -match "LogHOLUI: Display: End banner: lobby match, back to lobby offered, no countdown")) { $Failure = "no lobby end banner" }
                elseif (-not ($Lines -match "LogHOLTest: Display: Test capture: manager_client_return")) { $Failure = "no capture" }
                elseif (-not ($Lines -match "LogHOLUI: Display: End banner: back to lobby")) { $Failure = "« Retour au salon » not pressed" }
                elseif (-not ($Lines -match "LogHOLUI: Display: Menu: back in lobby $([regex]::Escape($Alice.lobby.id)) as Bob")) { $Failure = "no 'Menu: back in lobby ... as Bob'" }
                elseif (-not $Back -or $Back.state -ne "lobby" -or $BackBob -ne 1) { $Failure = "the manager does not have Bob back in its lobby (state $($Back.state))" }
                # M6-Q14: nobody kept after finished; Bob's is a new line (not ready), Alice holds nothing, Bob leads.
                elseif ($BackLine.ready) { $Failure = "Bob's line came back ready: his old line was kept, not a new one" }
                elseif ($BackAlice -ne 0) { $Failure = "Alice still in the lobby after finished" }
                elseif ($Back.leader -ne "Bob") { $Failure = "Bob is not the leader on his return (leader $($Back.leader))" }
                else {
                    # The server quits by itself once empty (R3.8); its launcher process with it.
                    for ($i = 0; $i -lt 40; $i++) {
                        $Left = @(Get-Process HeroesOfLibertyServer -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path.StartsWith($Project) })
                        if (-not $Left.Count) { break }
                        Start-Sleep 1
                    }
                    Write-Host "  servers of this copy left after $i s: $($Left.Count)"
                    if ($Left.Count) { $Failure = "$($Left.Count) server(s) of this copy still running"; $Left | Stop-Process -Force }
                }
            }
        }
    }
    else {
        $Body = @{ name = $LobbyName; playerName = $(if ($LobbyCase) { "Alice" } else { "Bob" }); gameVersion = $Status.gameVersion } | ConvertTo-Json
        $Created = Invoke-RestMethod "${Url}/lobbies" -Method Post -Body $Body -ContentType "application/json; charset=utf-8"
        Write-Host "lobby $($Created.lobby.id) created on $Url"

        # The lobby's creator stays in it only while it reads it (5 s, PROTOCOL.md): read twice a second until the game has asked.
        $BobIn = $null; $BobOut = $null
        $HasBob = { (@((Invoke-RestMethod "${Url}/lobbies/$($Created.lobby.id)" -TimeoutSec 2).slots | Where-Object { $_.playerName -eq "Bob" })).Count -gt 0 }
        if ($LobbyCase) {
            $Sequence = Join-Path $Work "lobby.seq"
            $Join = @("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 3",
                "hol.UI.Focus JoinButton0", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15")
            if ($Case -eq "Lobby") {
                $Join + @("waitlog : 2 players (", "waitui", "wait 1", "capture manager_client_lobby", "wait 3", "hol.UI.Focus BackButton", "frames 3",
                    "hol.UI.Key Enter 0 0.1", "wait 0.15", "waitlog Lobby: left", "wait 3", "quit") | Set-Content -Encoding utf8 $Sequence
                $DonePattern = "LogHOLUI: Display: Lobby: left"
                $GameName = "Bob"
            } elseif ($Case -eq "Ready") {
                $Join + @("waitlog : 2 players (", "waitui", "wait 1", "hol.UI.Choose SlotColor1 5", "waitlog Lobby: colour 5 accepted", "wait 0.5",
                    "hol.UI.Focus ReadyButton", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15", "waitlog Lobby: ready on accepted", "wait 1",
                    "capture manager_client_ready", "wait 3", "quit") | Set-Content -Encoding utf8 $Sequence
                $DonePattern = "Test capture: manager_client_ready"
                $GameName = "Bob"
            } else {
                $Join + @("waitlog Multiplayer: error", "frames 10", "capture manager_client_name_taken", "quit") | Set-Content -Encoding utf8 $Sequence
                $DonePattern = "Test capture: manager_client_name_taken"
                $GameName = "Alice"
            }
            $Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -unattended -nosplash -LiveCoding=0 -culture=fr -saveddirsuffix=Tests " +
                "-PlayerName=$GameName -ServersFile=`"$Servers`" -ExecCmds=`"hol.Test.Run $Sequence`" -abslog=`"$GameLog`""
        }
        elseif ($Screen) {
            $Sequence = Join-Path $Work "multiplayer.seq"
            @("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 10",
                "capture manager_client_multiplayer", "hol.UI.Focus JoinButton0", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15",
                "waitlog Multiplayer: join", "quit") | Set-Content -Encoding utf8 $Sequence
            $Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -unattended -nosplash -LiveCoding=0 -culture=fr -saveddirsuffix=Tests " +
                "-ServersFile=`"$Servers`" -ExecCmds=`"hol.Test.Run $Sequence`" -abslog=`"$GameLog`""
            $DonePattern = "LogHOLUI: Display: Multiplayer: join"
        } else {
            $Arguments = "`"$UProject`" -game -nullrhi -nosound -unattended -nosplash -LiveCoding=0 -saveddirsuffix=Tests -ServersFile=`"$Servers`" " +
                "-ExecCmds=`"hol.Manager.Status,hol.Manager.Lobbies,hol.Delay 3 quit`" -abslog=`"$GameLog`""
            $DonePattern = "Manager lobbies:"
        }
        $Game = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
        $Deadline = (Get-Date).AddSeconds(180)
        while (-not $Game.HasExited -and (Get-Date) -lt $Deadline) {
            Start-Sleep -Milliseconds 500
            Invoke-RestMethod "${Url}/lobbies/$($Created.lobby.id)" -Headers @{ "X-Player-Token" = $Created.token } -TimeoutSec 2 | Out-Null
            if ($Case -eq "Lobby" -and $null -eq $BobIn -and (Test-Path $GameLog) -and (Select-String -Path $GameLog -Pattern "Test capture: manager_client_lobby" -Quiet)) {
                $BobIn = & $HasBob
            }
            if ((Test-Path $GameLog) -and (Select-String -Path $GameLog -Pattern $DonePattern -Quiet)) {
                if ($Case -eq "Ready") { $BobRow = @((Invoke-RestMethod "${Url}/lobbies/$($Created.lobby.id)" -TimeoutSec 2).slots | Where-Object { $_.playerName -eq "Bob" })[0] }
                Start-Sleep 2; break
            }
        }
        if ($Case -eq "Lobby") { $BobOut = -not (& $HasBob) }
        if (-not $Game.HasExited) { Stop-Process -Id $Game.Id -Force -ErrorAction SilentlyContinue }
        if ($LobbyCase) {
            $Lines = if (Test-Path $GameLog) { @(Select-String -Path $GameLog -Encoding utf8 -Pattern "Multiplayer|LogHOLUI: Display: Lobby:|Test capture|Test run|LogHOLNet: Manager:" | ForEach-Object { $_.Line }) } else { @() }
            $Lines | ForEach-Object { Write-Host "  $_" }
            if ($Case -eq "Lobby") {
                if (-not ($Lines -match "LogHOLUI: Display: Lobby: $($Created.lobby.id) .*: 2 players \(Alice, Bob\), you Bob, leader Alice")) { $Failure = "no 'Lobby: ... 2 players (Alice, Bob), you Bob, leader Alice'" }
                elseif (-not ($Lines -match "LogHOLTest: Display: Test capture: manager_client_lobby")) { $Failure = "no capture" }
                elseif (-not $BobIn) { $Failure = "the manager does not have Bob in the lobby" }
                elseif (-not ($Lines -match "LogHOLUI: Display: Lobby: left")) { $Failure = "no 'Lobby: left'" }
                elseif (-not $BobOut) { $Failure = "Bob still in the lobby after Back" }
            } elseif ($Case -eq "Ready") {
                Write-Host "  manager: Bob colorIndex $($BobRow.colorIndex) ready $($BobRow.ready)"
                if (-not ($Lines -match "LogHOLUI: Display: Lobby: colour 5 accepted")) { $Failure = "no 'Lobby: colour 5 accepted'" }
                elseif (-not ($Lines -match "LogHOLUI: Display: Lobby: ready on accepted")) { $Failure = "no 'Lobby: ready on accepted'" }
                elseif (-not ($Lines -match "you Bob, leader Alice, .* launch hidden")) { $Failure = "Bob (not the leader) does not log 'launch hidden'" }
                elseif ($Lines -match "you Bob, .* launch shown") { $Failure = "Launch shown to Bob, not the leader" }
                elseif (-not $BobRow -or $BobRow.colorIndex -ne 5 -or $BobRow.ready -ne $true) { $Failure = "the manager does not have Bob in colour 5 and ready" }
                elseif (-not ($Lines -match "LogHOLTest: Display: Test capture: manager_client_ready")) { $Failure = "no capture" }
            } else {
                if (-not ($Lines -match "LogHOLUI: Display: Multiplayer: error Ce nom est d.j. pris dans ce salon\.")) { $Failure = "no 'Multiplayer: error Ce nom est deja pris dans ce salon.'" }
                elseif ($Lines -match "Multiplayer: in lobby") { $Failure = "entered despite the name" }
                elseif (-not ($Lines -match "LogHOLTest: Display: Test capture: manager_client_name_taken")) { $Failure = "no capture" }
            }
        }
        elseif ($Screen) {
            $Lines = if (Test-Path $GameLog) { @(Select-String -Path $GameLog -Pattern "Multiplayer|Test capture|Test run|hol.UI.Open" | ForEach-Object { $_.Line }) } else { @() }
            $Lines | ForEach-Object { Write-Host "  $_" }
            if (-not ($Lines -match "LogHOLUI: Display: Multiplayer: 1 lobbies shown")) { $Failure = "no 'Multiplayer: 1 lobbies shown'" }
            elseif (-not ($Lines -match ([regex]::Escape("LogHOLUI: Display: Multiplayer: lobby $($Created.lobby.id) `"$LobbyName`" 1/")))) { $Failure = "no row for the lobby '$LobbyName'" }
            elseif (-not ($Lines -match "LogHOLTest: Display: Test capture: manager_client_multiplayer")) { $Failure = "no capture" }
            elseif (-not ($Lines -match ([regex]::Escape("LogHOLUI: Display: Multiplayer: join $($Created.lobby.id) as ")))) { $Failure = "no 'Multiplayer: join $($Created.lobby.id) as ...'" }
        } else {
            $Lines = if (Test-Path $GameLog) { @(Select-String -Path $GameLog -Pattern "LogHOLNet" | ForEach-Object { $_.Line }) } else { @() }
            $Lines | Where-Object { $_ -match "Manager" } | ForEach-Object { Write-Host "  $_" }
            if (-not ($Lines -match "Manager status: protocol 1, game \S+, 1 lobbies")) { $Failure = "no 'Manager status: protocol 1, ..., 1 lobbies'" }
            elseif (-not ($Lines -match "Manager lobbies: 1")) { $Failure = "no 'Manager lobbies: 1'" }
            elseif (-not ($Lines -match [regex]::Escape("`"$LobbyName`""))) { $Failure = "no lobby named '$LobbyName'" }
        }
    }
    }
}
catch { $Failure = "$_" }
finally {
    if ($Manager) { Stop-Process -Id $Manager.Id -Force -ErrorAction SilentlyContinue }
    if ($ServerVersionMade) { Remove-Item $ServerVersion -ErrorAction SilentlyContinue }
}
if ($Failure) { Write-Host "manager_client : FAIL ($Failure; game log $GameLog)"; exit 1 }
if ($Case -eq "Lobby") { Write-Host "manager_client -Case Lobby : PASS (Bob joined Alice's lobby, 2 players shown and captured, in then out of the manager's lobby after Back; $Url)"; exit 0 }
if ($Case -eq "Match") { Write-Host "manager_client -Case Match : PASS (Launch $Started, the game opened the match's server: $(@($Lines -match 'joining match')[0] -replace '.*Lobby: ', ''); $Url)"; exit 0 }
if ($Case -eq "Return") { Write-Host "manager_client -Case Return : PASS (match from the lobby, Bob gave up, end banner captured, « Retour au salon »: Bob back in lobby $($Alice.lobby.id), state $($Back.state); no server left; $Url)"; exit 0 }
if ($Case -eq "Killed") { Write-Host ("manager_client -Case Killed : PASS (match's server killed, the game back on the Multiplayer list with the red reason in {0:N1} s, captured; no server left; $Url)" -f $LostAfter); exit 0 }
if ($Case -eq "Leader") { Write-Host "manager_client -Case Leader : PASS (Bob created and led the lobby, closed slot 2, Ready, captured, Launch sent: $Started; $Url)"; exit 0 }
if ($Case -eq "Ready") { Write-Host "manager_client -Case Ready : PASS (Bob took colour 5 and Ready, the manager has both, Launch hidden from him, captured; $Url)"; exit 0 }
if ($Case -eq "NameTaken") { Write-Host "manager_client -Case NameTaken : PASS (joining as Alice refused in red, captured; $Url)"; exit 0 }
if ($Case) { Write-Host "manager_client -Case ${Case} : PASS (Multiplayer screen: the error shown in red, captured, no list; $Url)"; exit 0 }
if ($Screen) { Write-Host "manager_client -Screen : PASS (Multiplayer screen: 1 lobby shown, captured, Join pressed; manager on $Url)"; exit 0 }
Write-Host "manager_client : PASS (status and 1 lobby read from the manager on $Url)"
exit 0
