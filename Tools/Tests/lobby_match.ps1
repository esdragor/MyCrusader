# A lobby match end to end with two game clients (M6 R5.1, R5.2): the published test manager (HOLManager.exe,
# Tools\Manager\build_manager.ps1) with the staged dedicated server (net_match.ps1 -Stage) as its ServerExe, as in
# manager_client.ps1 -Case Match, then two games off screen (-RenderOffScreen), "Alice" and "Bob" (-PlayerName=), each
# with its own Saved folder (-saveddirsuffix=Tests_Alice / Tests_Bob) and log, driven by sequences (hol.Test.Run):
# Alice opens Multiplayer and creates a lobby; once the manager lists it, Bob is started, sees the list and joins; each
# takes a colour (Alice 2, Bob 5) and presses Ready; Bob captures the lobby with both ready (lobby_match_lobby.png);
# Alice, the leader, presses Launch; both reach Plain (the server logs "Alice holds slot 0" and "Bob holds slot 1") and
# capture it (lobby_match_alice.png, lobby_match_bob.png); Bob gives up (hol.Victory.Surrender, no cheat), both get the
# end banner of a lobby match; M6-Q14: nobody is kept in the lobby after the end; Bob presses « Retour au salon » first
# and the manager has the lobby "lobby" with Bob alone, not leader, Launch hidden (Alice, the leader, still in the
# finished match, stays leader; no line kept for her); then Alice goes back by the in-game menu: Alice and Bob on new
# lines (default colours), nobody ready, Alice leader again. The clients only wait on their own logs (waitlog).
# R5.3: once the match is under way (the manager says "in_game"), a third game, "Carol" (Saved_Tests_Carol), opens
# Multiplayer, sees the lobby "en partie" with « Regarder », presses it and comes in as a spectator (no Name: the server
# logs "Carol joined as a spectator"), captures it (lobby_match_watchlist.png, lobby_match_watch.png), signals Bob, who
# only then gives up; she stays 25 s more (lobby_match_watch_end.png) and quits.
# M6-Q12: each player's table of players shows the lobby's names (Alice: "Vous, Bob", Bob: "Alice, Vous", Carol: "Alice, Bob").
# M6-Q10: Carol gets the spectator's banner (MatchOver, « Vainqueur : Alice », no return to a lobby) and stays; meanwhile a
# fourth game, "Dave" (Saved_Tests_Dave), opens the match's port directly without a name once it is over (« Regarder » is
# gone from the list: the lobby is back to "lobby") and is refused (server: "Login refused from ... (spectator)");
# lobby_match_refused.png shows what he gets; Carol then presses « Quitter vers la liste » (her banner's way out, as
# the players') and must land on the Multiplayer list (lobby_match_watch_back.png) before she quits.
# Captures: Saved_Tests_Alice / Saved_Tests_Bob / Saved_Tests_Carol \Screenshots\WindowsEditor\HOL.
# Prints one line PASS / FAIL; exit code 0 / 1. The manager, both games and this copy's servers are always stopped
# (by PID; never another worktree's).
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\lobby_match.ps1 [-EngineDir <...\Engine\Binaries\Win64>]
param([string] $EngineDir = "")
$Began = Get-Date
$Project = (Resolve-Path "$PSScriptRoot\..\..").Path
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $Root = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $Root) { Write-Host "lobby_match : FAIL (engine '$Association' not found; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $Root "Engine\Binaries\Win64"
}
$Exe = Join-Path $Project "Saved\Manager\HOLManager.exe"
if (-not (Test-Path $Exe)) { Write-Host "lobby_match : FAIL (no $Exe; run Tools\Manager\build_manager.ps1)"; exit 1 }
$ServerExe = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLibertyServer.exe"
if (-not (Test-Path $ServerExe)) { Write-Host "lobby_match : FAIL (no $ServerExe; run Tools\Tests\net_match.ps1 -Stage)"; exit 1 }
$TestSaved = Join-Path $Project "Saved_Tests"
$Work = Join-Path $TestSaved "LobbyMatch"
$Logs = Join-Path $TestSaved "Logs"
New-Item -ItemType Directory -Force $Work, $Logs | Out-Null
$ManagerLog = Join-Path $Work "manager.log"
$GameLogs = @{ Alice = (Join-Path $Logs "lobby_match_alice.log"); Bob = (Join-Path $Logs "lobby_match_bob.log"); Carol = (Join-Path $Logs "lobby_match_carol.log"); Dave = (Join-Path $Logs "lobby_match_dave.log") }
# R5.3: Bob gives up only once Carol, the spectator, has captured the match (this file, signalled by her sequence).
$CarolIn = Join-Path $Work "carol_in.signal"
# M6-Q10: Dave tries once the match is over (written here when Carol has her banner); Carol quits once he is done.
$MatchOver = Join-Path $Work "match_over.signal"
$DaveDone = Join-Path $Work "dave_done.signal"
# M6-Q7 revised: Alice comes back late (she stays in the finished match past 60 s): Bob, back first, waits for her in the lobby.
$AliceBack = Join-Path $Work "alice_back.signal"
# M6-Q14: Bob signals once back; the lobby is read (Bob alone, Alice still leader from the match); only then Alice goes back.
$BobBack = Join-Path $Work "bob_back.signal"
$BobChecked = Join-Path $Work "bob_checked.signal"
# Bob gives up only once Carol is in (up to 180 s on a loaded machine): Alice waits for this signal before her end
# banner, a waitlog lasting 60 s at most (2026-10-26: Alice timed out 15 s before Bob's surrender).
$BobGivesUp = Join-Path $Work "bob_gives_up.signal"
Remove-Item $ManagerLog, $GameLogs.Alice, $GameLogs.Bob, $GameLogs.Carol, $GameLogs.Dave, $CarolIn, $MatchOver, $DaveDone, $AliceBack, $BobBack, $BobChecked, $BobGivesUp -ErrorAction SilentlyContinue
$Colours = @{ Alice = 2; Bob = 5 }

# A free TCP port from 7790 (another copy's manager may hold 7700).
$Port = 7790
while ($Port -lt 7890) {
    try { $Probe = New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback, $Port); $Probe.Start(); $Probe.Stop(); break } catch { $Port++ }
}
$Url = "http://127.0.0.1:${Port}"
# A staged server without Version.txt is "unknown" and refuses everyone (HOLVersion): "dev" for the time of the test.
$ServerVersion = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Version.txt"
$ServerVersionMade = -not (Test-Path $ServerVersion)
if ($ServerVersionMade) { [IO.File]::WriteAllText($ServerVersion, "dev", (New-Object Text.UTF8Encoding $false)) }
$Manager = Start-Process -FilePath $Exe -ArgumentList "--urls $Url --GameVersion=dev --ServerExe=`"$ServerExe`" --ManagerUrl=$Url" `
    -WorkingDirectory $Work -PassThru -WindowStyle Hidden -RedirectStandardOutput $ManagerLog
$Games = @{}
$Failure = $null
$Matches0 = Join-Path $Project "Saved\Manager\Saved\Matches"
$Servers = Join-Path $Work "servers.json"
"{ `"servers`": [ { `"name`": `"Test`", `"url`": `"$Url`" } ] }" | Set-Content -Encoding ascii $Servers

# This copy's match servers (the test manager's), by path: never another worktree's.
function Get-OurServers { @(Get-Process HeroesOfLibertyServer* -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path.StartsWith($Project) }) }
function Test-Log([string] $Who, [string] $Pattern) { (Test-Path $GameLogs[$Who]) -and (Select-String -Path $GameLogs[$Who] -Pattern $Pattern -Quiet) }
function Start-Game([string] $Who, [string[]] $Steps) {
    $Sequence = Join-Path $Work "$($Who.ToLower()).seq"
    $Steps | Set-Content -Encoding utf8 $Sequence
    $Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -unattended -nosplash -LiveCoding=0 -as-exit-on-error -culture=fr " +
        "-saveddirsuffix=Tests_$Who -PlayerName=$Who -ServersFile=`"$Servers`" -ExecCmds=`"hol.Test.Run $Sequence`" -abslog=`"$($GameLogs[$Who])`""
    $Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
    Write-Host "$Who : pid $($Process.Id), log $($GameLogs[$Who])"
    $Process
}
$Press = { param($Button) @("hol.UI.Focus $Button", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15") }
# Both, once the lobby is full: their colour and Ready; in the match: a capture; then the end banner and « Retour au salon ».
$Pick = { param($Who, $Line) @("waitlog : 2 players (", "waitui", "wait 1", "hol.UI.Choose SlotColor$Line $($Colours[$Who])", "waitlog Lobby: colour $($Colours[$Who]) accepted",
    "wait 0.5") + (& $Press ReadyButton) + @("waitlog Lobby: ready on accepted") }
$InMatch = { param($Who) @("waitlog Lobby: joining match", "waitmap Plain", "waitlog Camera above the start", "frames 60", "capture lobby_match_$($Who.ToLower())") }
$Back = @("waitlog End banner: lobby match", "wait 1.5") + (& $Press ReturnButton) + @("waitmap MainMenu", "waitlog Menu: back in lobby", "wait 3", "quit")
# C3: once the match is over, the statistics from the server (hol.Stats.Dump on a client logs the copy once complete),
# then $Back from its third step.
$StatsAfter = @("waitlog End banner: lobby match", "hol.Stats.Dump", "waitlog Stats: client copy received", "frames 3", "wait 1.5")
# M6-Q7 revised: nobody is sent back. Alice stays in the finished match past 60 s (two waits, a step lasts 60 s at most;
# nobody keeps a slot after the end, M6-Q14; she goes back once Bob is back and checked), the banner still there with no countdown; she closes it
# (« Continuer à regarder »), opens the statistics from the game screen's « Statistiques », closes them, then opens the
# in-game menu (« Menu ») and goes back by its « Retour au salon » ($Back from its third step).
$StayAfterEnd = @("wait 35", "wait 36", "frames 3", "capture lobby_match_end_late") + (& $Press ContinueButton) + @("waitlog End banner: closed", "frames 3",
    "hol.HUD.Click StatsButton", "waitlog Stats screen: opened", "frames 5") + (& $Press BackButton) + @("waitlog Stats screen: closed", "frames 3",
    "hol.HUD.Click MenuButton", "waitlog Game menu: opened", "waitui", "frames 3", "capture lobby_match_game_menu_end")

# C6 (M6-Q10): Carol, the spectator, opens the statistics during the match from the game screen's « Statistiques »,
# stays on « Chronologie » past one of the screen's 10 s requests (the curves grow), captures it, and « Retour » gives
# the match back. « Stock » too, the same for both players (lobby_match_stats_stock: the curves side by side, none hidden).
$StatsLive = @("hol.HUD.Click StatsButton", "waitlog Stats screen: opened", "wait 1") + (& $Press TimelineTab) +
    @("wait 12", "frames 3", "capture lobby_match_stats_live") + (& $Press SeriesStock) + @("frames 3", "capture lobby_match_stats_stock") +
    (& $Press BackButton) + @("waitlog Stats screen: closed", "frames 3")
# C3 (M6-Q11): Carol gets the statistics during the match and after, Bob is refused during it, Alice and Bob get them after;
# every table a client dumped is one the server sent; Bob's defeat is among the events. An empty string when all holds.
function Test-Stats([string] $ServerFile) {
    $Server = Get-Content $ServerFile
    $Sent = @($Server | ForEach-Object { if ($_ -match "Stats: sent table (slot \d+: .*)$") { $Matches[1] } })
    if (-not ($Server -match "Stats: refused to Bob \(a player during the match")) { return "the server has no 'Stats: refused to Bob'" }
    if (-not ($Server -match "Stats: sent to Carol \(spectator, match on\)")) { return "the server sent nothing to Carol during the match" }
    foreach ($Who in "Alice", "Bob", "Carol") {
        if (-not ($Server -match "Stats: sent to $Who \(\w+, match over\)")) { return "the server sent nothing to $Who after the end" }
        $Lines = Get-Content $GameLogs[$Who]
        $Copies = @($Lines -match "Stats: client copy received").Count
        if ($Copies -lt ($(if ($Who -eq "Carol") { 2 } else { 1 }))) { return "$Who : $Copies copy of the statistics received" }
        $Tables = @($Lines | ForEach-Object { if ($_ -match "LogHOLMatch: Display: Stats: (slot \d+: .*)$") { $Matches[1] } })
        if ($Tables.Count -lt 2) { return "$Who : no table of both players in the copy" }
        foreach ($Table in $Tables) { if ($Sent -notcontains $Table) { return "$Who : table '$Table' not one the server sent" } }
        if ($Who -ne "Carol" -or $Copies -ge 2) { if (-not ($Lines -match "Stats: event at \d+ s: slot 1 Defeated")) { return "$Who : no 'slot 1 Defeated' event in the copy" } }
    }
    if (-not (@(Get-Content $GameLogs.Bob) -match "Stats: the server refused")) { return "Bob : no refusal in his log" }
    return ""
}

try {
    $Status = $null
    for ($i = 0; $i -lt 40 -and -not $Status; $i++) {
        try { $Status = Invoke-RestMethod "${Url}/status" -TimeoutSec 2 } catch { Start-Sleep -Milliseconds 250 }
    }
    if (-not $Status) { throw "the manager does not answer on $Url" }

    $Games.Alice = Start-Game Alice (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 0 lobbies shown", "frames 3") +
        (& $Press CreateButton) + (& $Pick Alice 0) + @("waitlog 2/2 ready", "wait 3") + (& $Press LaunchButton) + @("waitlog Lobby: start sent") +
        (& $InMatch Alice) + @("waitfile $BobGivesUp 240") + $StatsAfter + $StayAfterEnd + @("waitfile $BobChecked 240") + $Back[2..($Back.Count - 3)] + @("signal $AliceBack") + $Back[($Back.Count - 2)..($Back.Count - 1)])
    # Bob only once the manager lists Alice's lobby: the list is read once, when the screen opens.
    $Deadline = (Get-Date).AddSeconds(180)
    $Lobby = $null
    while (-not $Lobby) {
        if ($Games.Alice.HasExited -or (Get-Date) -gt $Deadline) { throw "no lobby created by Alice" }
        Start-Sleep -Milliseconds 500
        $Lobby = @((Invoke-RestMethod "${Url}/lobbies" -TimeoutSec 2).lobbies)[0]
    }
    Write-Host "lobby $($Lobby.id) created by Alice"
    $Games.Bob = Start-Game Bob (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 3") +
        (& $Press JoinButton0) + (& $Pick Bob 1) + @("waitlog 2/2 ready", "wait 0.5", "capture lobby_match_lobby") +
        (& $InMatch Bob) + @("waitfile $CarolIn 180", "hol.Stats.Dump", "waitlog Stats: the server refused", "wait 2", "signal $BobGivesUp", "hol.Victory.Surrender") +
        $StatsAfter + $Back[2..($Back.Count - 3)] + @("signal $BobBack", "waitfile $AliceBack 240") + $Back[($Back.Count - 2)..($Back.Count - 1)])

    # R5.3: Carol, once the match is under way (state "in_game" at the manager), sees the lobby in the list, presses
    # « Regarder », reaches the match as a spectator and captures it; she stays until well after the end (what she gets is noted).
    $Deadline = (Get-Date).AddSeconds(180)
    while ((Invoke-RestMethod "${Url}/lobbies/$($Lobby.id)" -TimeoutSec 2).state -ne "in_game") {
        if ($Games.Bob.HasExited -or (Get-Date) -gt $Deadline) { throw "the lobby never in_game" }
        Start-Sleep -Milliseconds 500
    }
    Write-Host "lobby $($Lobby.id) in_game: Carol started"
    $Games.Carol = Start-Game Carol (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 3",
        "capture lobby_match_watchlist") + (& $Press WatchButton0) + @("waitlog Multiplayer: watching match", "waitmap Plain", "frames 120", "capture lobby_match_watch") + $StatsLive + @(
        "hol.Stats.Dump", "waitlog Stats: client copy received", "frames 3", "signal $CarolIn", "waitlog End banner: MatchOver",
        "hol.Stats.Dump", "waitlog Stats: client copy received", "frames 3", "wait 2", "capture lobby_match_watch_end") +
        (& $Press StatsButton) + @("waitlog Stats screen: opened", "frames 5", "capture lobby_match_stats") + (& $Press BackButton) +
        @("waitlog Stats screen: closed", "frames 3", "waitfile $DaveDone 150", "wait 1") +
        (& $Press QuitButton) + @("waitmap MainMenu", "waitlog lobbies shown", "frames 30", "capture lobby_match_watch_back", "wait 1", "quit"))
    # M6-Q10: Dave, started meanwhile, opens the match's port without a name once it is over.
    # "open": an absolute travel, as « Regarder » (UHOLManagerClient::OpenMatch), the menu's Name=Player not carried.
    $Deadline = (Get-Date).AddSeconds(240)
    while (-not (Test-Path $CarolIn)) {
        if ($Games.Carol.HasExited -or (Get-Date) -gt $Deadline) { throw "Carol never in the match" }
        Start-Sleep -Milliseconds 500
    }
    $MatchPort = if ((Select-String -Path $GameLogs.Carol -Pattern "watching match \S+ of \S+ on port (\d+)" | Select-Object -First 1).Line -match "on port (\d+)") { $Matches[1] } else { throw "no port in Carol's log" }
    $Games.Dave = Start-Game Dave @("waitmap MainMenu", "waitui", "waitfile $MatchOver 240", "open 127.0.0.1:${MatchPort}?Version=dev", "wait 8", "waitui",
        "capture lobby_match_refused", "signal $DaveDone", "wait 1", "quit")

    # Both back in the lobby (or a game gone): the manager's lobby is read.
    $Deadline = (Get-Date).AddSeconds(420)
    $After = $null
    $BobFirst = $null
    while ((Get-Date) -lt $Deadline) {
        Start-Sleep -Seconds 1
        if (-not (Test-Path $MatchOver) -and (Test-Log Carol "End banner: MatchOver")) { "over" | Set-Content -Encoding ascii $MatchOver }
        # M6-Q14: Bob back first, Alice (the leader) still in the finished match: Bob alone, no line kept for Alice, Alice still leader.
        if (-not $BobFirst -and (Test-Path $BobBack)) {
            $BobFirst = Invoke-RestMethod "${Url}/lobbies/$($Lobby.id)" -TimeoutSec 2
            $FirstNames = @($BobFirst.slots | Where-Object { $_.playerName } | ForEach-Object { $_.playerName }) -join ", "
            Write-Host "  Bob back first: state $($BobFirst.state), players ($FirstNames), leader $($BobFirst.leader)"
            "checked" | Set-Content -Encoding ascii $BobChecked
        }
        if ((Test-Log Alice "Menu: back in lobby") -and (Test-Log Bob "Menu: back in lobby")) {
            Start-Sleep 2
            $After = Invoke-RestMethod "${Url}/lobbies/$($Lobby.id)" -TimeoutSec 2
            break
        }
        if ($Games.Alice.HasExited -and $Games.Bob.HasExited) { break }
    }
    # Carol, the spectator, quits by herself once Dave is done (her sequence): what she got at the end is in her log.
    if (-not (Test-Path $MatchOver) -and (Test-Log Carol "End banner: MatchOver")) { "over" | Set-Content -Encoding ascii $MatchOver }
    if ($Games.Dave -and -not $Games.Dave.WaitForExit(120000)) { Write-Host "  Dave still there 120 s after the end" }
    if ($Games.Carol -and -not $Games.Carol.WaitForExit(30000)) { Write-Host "  Carol still there 30 s after Dave" }
    $Lines = @{}
    foreach ($Who in "Alice", "Bob", "Carol", "Dave") {
        $Lines[$Who] = if (Test-Path $GameLogs[$Who]) { @(Select-String -Path $GameLogs[$Who] -Encoding utf8 -Pattern "Multiplayer:|LogHOLUI: Display: (Lobby|End banner|Menu|Game menu|Players table|Stats screen: (opened|closed))|The match is over|PendingConnectionFailure|Test capture|Test run|Test sequence|LogHOLNet: Display: Manager|LogLoad: LoadMap|Camera above|Victory:|NetworkFailure|TravelFailure|Connection closed|Game HUD" | ForEach-Object { $_.Line }) } else { @() }
        Write-Host "  $Who :"; $Lines[$Who] | ForEach-Object { Write-Host "    $_" }
    }
    $ServerLog = Get-ChildItem $Matches0 -Recurse -Filter server.log -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -gt $Began } | Sort-Object LastWriteTime | Select-Object -Last 1
    if ($ServerLog) { Write-Host "  server log $($ServerLog.FullName):"; Select-String -Path $ServerLog.FullName -Pattern "LogHOL\w+: .*(Bob|Alice|holds slot|spectator|Surrender|finished|winner|quit|empty|refused)" | Select-Object -Last 20 | ForEach-Object { Write-Host "    $($_.Line)" } }
    if ($After) { $After.slots | Where-Object { $_.playerName } | ForEach-Object { Write-Host "  manager after: $($_.playerName) colour $($_.colorIndex) ready $($_.ready)" }; Write-Host "  manager after: state $($After.state)" }

    foreach ($Who in "Alice", "Bob") {
        if ($Failure) { break }
        $L = $Lines[$Who]
        if (-not ($L -match "Lobby: colour $($Colours[$Who]) accepted")) { $Failure = "$Who : no 'Lobby: colour $($Colours[$Who]) accepted'" }
        elseif (-not ($L -match "Lobby: ready on accepted")) { $Failure = "$Who : no 'Lobby: ready on accepted'" }
        elseif (-not ($L -match "2 players \(Alice, Bob\), you $Who, leader Alice, 2/2 ready")) { $Failure = "$Who : no '2 players (Alice, Bob), you $Who, leader Alice, 2/2 ready'" }
        elseif (-not ($L -match "LogHOLUI: Display: Lobby: joining match \S+ at 127\.0\.0\.1:\d+ as $Who")) { $Failure = "$Who : no 'Lobby: joining match ... as $Who'" }
        elseif (-not ($L -match "Test capture: lobby_match_$($Who.ToLower())")) { $Failure = "$Who : no capture in the match" }
        elseif (-not ($L -match "Players table: $(if ($Who -eq 'Alice') { 'Vous, Bob' } else { 'Alice, Vous' })$")) { $Failure = "$Who : the table of players without the lobby's names" }
        elseif (-not ($L -match "End banner: lobby match, back to lobby offered, no countdown")) { $Failure = "$Who : no lobby end banner" }
        elseif ($L -match "back to lobby in \d+ s") { $Failure = "$Who : a countdown on « Retour au salon »" }
        elseif (-not ($L -match "$(if ($Who -eq 'Alice') { 'Game menu' } else { 'End banner' }): back to lobby$")) { $Failure = "$Who : « Retour au salon » not pressed" }
        elseif (-not ($L -match "Menu: back in lobby $([regex]::Escape($Lobby.id)) as $Who")) { $Failure = "$Who : no 'Menu: back in lobby $($Lobby.id) as $Who'" }
    }
    # M6-Q7 revised: Alice stayed past 60 s without being sent back, closed the banner, opened the statistics from the
    # game screen, then went back by the in-game menu's « Retour au salon ».
    if (-not $Failure) {
        $L = $Lines.Alice
        $Stamp = { param($Pattern) $Line = @($L | Where-Object { $_ -match $Pattern }) | Select-Object -First 1
            if ($Line -match "^\[(\d{4})\.(\d\d)\.(\d\d)-(\d\d)\.(\d\d)\.(\d\d):(\d{3})\]") { [datetime]::new([int]$Matches[1], [int]$Matches[2], [int]$Matches[3], [int]$Matches[4], [int]$Matches[5], [int]$Matches[6], [int]$Matches[7]) } else { $null } }
        $Shown = & $Stamp "End banner: lobby match, back to lobby offered"
        $Left = & $Stamp "Game menu: back to lobby$"
        $Index = { param($Pattern) [array]::FindIndex([string[]]$L, [Predicate[string]] { param($Line) $Line -match $Pattern }) }
        if (-not $Shown -or -not $Left) { $Failure = "Alice : no time for the banner or the way back" }
        elseif (($Left - $Shown).TotalSeconds -lt 65) { $Failure = "Alice : back after $([int]($Left - $Shown).TotalSeconds) s (past 60 s wanted)" }
        elseif ((& $Index "End banner: back to lobby$") -ge 0) { $Failure = "Alice : sent back by the banner" }
        elseif ((& $Index "End banner: closed") -lt 0) { $Failure = "Alice : the banner not closed" }
        elseif ((& $Index "Stats screen: opened") -lt (& $Index "End banner: closed")) { $Failure = "Alice : no statistics from the game screen after the banner" }
        elseif ((& $Index "Game menu: match over, ways out lobby, list, result") -lt 0) { $Failure = "Alice : the in-game menu without the ways out" }
        else { Write-Host "  Alice : stayed $([int]($Left - $Shown).TotalSeconds) s in the finished match, statistics from the game screen, back by the in-game menu" }
    }
    # R5.3: Carol saw the lobby "en partie" with « Regarder », pressed it, came in as a spectator, captured the match.
    if (-not $Failure) {
        $L = $Lines.Carol
        if (-not ($L -match "Multiplayer: lobby $([regex]::Escape($Lobby.id)) .* en partie .*, watch")) { $Failure = "Carol : the lobby not shown in a match with « Regarder »" }
        elseif (-not ($L -match "Multiplayer: watching match \S+ of $([regex]::Escape($Lobby.id)) on port \d+")) { $Failure = "Carol : no 'Multiplayer: watching match'" }
        elseif (-not ($L -match "LoadMap: 127\.0\.0\.1/Game/Maps/Plain\?Version=")) { $Failure = "Carol : Plain not loaded from the server (or with a Name)" }
        elseif (-not ($L -match "Test capture: lobby_match_watch \(hud=HOLGameScreen\)")) { $Failure = "Carol : no capture of the match" }
        elseif (-not ($L -match "Players table: Alice, Bob$")) { $Failure = "Carol : the table of players without 'Alice, Bob'" }
        elseif (-not ($L -match "Test capture: lobby_match_stats_live \(hud=HOLGameScreen menu=HOLStatsScreen\)")) { $Failure = "Carol : no capture of the statistics during the match" }
        elseif (-not ($L -match "End banner: MatchOver")) { $Failure = "Carol : no MatchOver banner" }
        elseif (-not ($L -match 'End banner: detail "Vainqueur : Alice"')) { $Failure = "Carol : no 'Vainqueur : Alice' on her banner" }
        elseif ($L -match "End banner: (lobby match|back to lobby)") { $Failure = "Carol : a lobby end banner (she has no lobby)" }
        elseif (-not ($L -match "Test capture: lobby_match_watch_end")) { $Failure = "Carol : no capture of her banner" }
        elseif (-not ($L -match "Test capture: lobby_match_stats \(hud=HOLGameScreen menu=HOLStatsScreen\)")) { $Failure = "Carol : no capture of the statistics screen" }
        elseif (-not ($L -match "Stats screen: closed")) { $Failure = "Carol : the statistics screen not closed by « Retour »" }
        elseif (-not ($L -match "End banner: way out list")) { $Failure = "Carol : her banner without « Quitter vers la liste »" }
        elseif (-not ($L -match "End banner: quit to list")) { $Failure = "Carol : « Quitter vers la liste » not pressed" }
        elseif (-not ($L -match "Test capture: lobby_match_watch_back")) { $Failure = "Carol : not back on the Multiplayer list after « Quitter vers la liste »" }
        elseif ($L -match "Test sequence: timeout") { $Failure = "Carol : her sequence timed out" }
        elseif ($Lines.Dave -match "Test sequence: timeout") { $Failure = "Dave : his sequence timed out" }
        elseif (-not ($Lines.Dave -match "Test capture: lobby_match_refused")) { $Failure = "Dave : no capture after his attempt" }
        elseif ($Lines.Dave -match "LoadMap: 127\.0\.0\.1") { $Failure = "Dave : let into the finished match" }
    }
    if (-not $Failure) {
        if ($ServerLog -and -not (Select-String -Path $ServerLog.FullName -Pattern "LogHOLMatch: Carol joined as a spectator" -Quiet)) { $Failure = "the server has no 'Carol joined as a spectator'" }
        elseif ($ServerLog -and -not (Select-String -Path $ServerLog.FullName -Pattern "LogHOLMatch: (Warning: )?Login refused from .*: The match is over \(spectator\)" -Quiet)) { $Failure = "the server has no 'Login refused ... (spectator)' for Dave" }
        elseif (-not ($Lines.Alice -match "Lobby: start accepted \(HTTP 202\)")) { $Failure = "Alice's Launch not accepted" }
        elseif (-not ($Lines.Bob -match "Test capture: lobby_match_lobby")) { $Failure = "no capture of the lobby at Bob's" }
        elseif (-not ($Lines.Alice -match "End banner: Victory")) { $Failure = "Alice has no Victory banner" }
        elseif (-not ($Lines.Bob -match "End banner: Defeat")) { $Failure = "Bob has no Defeat banner" }
        elseif (-not $ServerLog) { $Failure = "no server log of the match" }
        elseif (-not (Select-String -Path $ServerLog.FullName -Pattern "LogHOLMatch: Alice holds slot 0" -Quiet)) { $Failure = "the server has no 'Alice holds slot 0'" }
        elseif (-not (Select-String -Path $ServerLog.FullName -Pattern "LogHOLMatch: Bob holds slot 1" -Quiet)) { $Failure = "the server has no 'Bob holds slot 1'" }
        # The surrender over the network (V1.4c, out of the "net" part since V5.5a): Bob's hol.Victory.Surrender is his
        # controller's ServerSurrender, accepted by the dedicated server for his own slot, the match then over for Alice.
        elseif (Select-String -Path $ServerLog.FullName -Pattern "Victory: surrender of slot -?\d+ refused" -Quiet) { $Failure = "the server refused a surrender" }
        elseif (-not (Select-String -Path $ServerLog.FullName -SimpleMatch "Victory: slot 1 defeated (Surrender)" -Quiet)) { $Failure = "the server has no 'Victory: slot 1 defeated (Surrender)' (Bob's ServerSurrender)" }
        elseif (-not (Select-String -Path $ServerLog.FullName -SimpleMatch "Victory: match over, winners 0" -Quiet)) { $Failure = "the server has no 'Victory: match over, winners 0' after Bob's surrender" }
        elseif (-not $After) { $Failure = "the manager's lobby not read after the match" }
        elseif ($Failure = Test-Stats $ServerLog.FullName) { }
        elseif (-not $BobFirst) { $Failure = "the lobby not read when Bob came back first" }
        elseif ($BobFirst.state -ne "lobby") { $Failure = "Bob back first: the lobby is $($BobFirst.state), not 'lobby'" }
        elseif ($FirstNames -ne "Bob") { $Failure = "Bob back first: players ($FirstNames), not Bob alone (a line kept for Alice?)" }
        elseif ($BobFirst.leader -ne "Alice") { $Failure = "Bob back first: leader $($BobFirst.leader), not Alice (still in the match)" }
        elseif (-not ($Lines.Bob -match "1 players? \(Bob\), you Bob, leader Alice, .*launch hidden")) { $Failure = "Bob back first: his lobby screen does not hide Launch (Alice still leader)" }
        elseif ($After.state -ne "lobby") { $Failure = "the lobby is $($After.state), not 'lobby'" }
        else {
            $Players = @($After.slots | Where-Object { $_.playerName })
            $Alice = @($Players | Where-Object { $_.playerName -eq "Alice" })[0]
            $Bob = @($Players | Where-Object { $_.playerName -eq "Bob" })[0]
            if ($Players.Count -ne 2 -or -not $Alice -or -not $Bob) { $Failure = "the lobby has $($Players.Count) player(s), not Alice and Bob" }
            # M6-Q14: new lines; E2-1: each one given back the colour of the match (free: Bob 5 back first, then Alice 2).
            elseif ($Alice.colorIndex -ne $Colours.Alice -or $Bob.colorIndex -ne $Colours.Bob) { $Failure = "colours of the match not given back (Alice $($Alice.colorIndex), Bob $($Bob.colorIndex); E2-1)" }
            elseif ($After.leader -ne "Alice") { $Failure = "leader $($After.leader), not Alice (the leader back from the match)" }
            elseif ($Alice.ready -or $Bob.ready) { $Failure = "still ready after the match (Alice $($Alice.ready), Bob $($Bob.ready))" }
        }
    }
}
catch { $Failure = "$_" }
finally {
    foreach ($Game in $Games.Values) { if ($Game -and -not $Game.HasExited) { Stop-Process -Id $Game.Id -Force -ErrorAction SilentlyContinue } }
    # The server quits by itself once empty (R3.8); left after 30 s, it is stopped (and the test fails).
    for ($i = 0; $i -lt 30 -and (Get-OurServers).Count; $i++) { Start-Sleep 1 }
    $Left = Get-OurServers
    if ($Left.Count) { $Left | ForEach-Object { Write-Host "  stopping server PID $($_.Id)"; Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue }; if (-not $Failure) { $Failure = "$($Left.Count) server(s) of this copy still running" } }
    if ($Manager) { Stop-Process -Id $Manager.Id -Force -ErrorAction SilentlyContinue }
    if ($ServerVersionMade) { Remove-Item $ServerVersion -ErrorAction SilentlyContinue }
}
$Took = "{0:N0} s" -f ((Get-Date) - $Began).TotalSeconds
if ($Failure) { Write-Host "lobby_match : FAIL ($Failure; logs $($GameLogs.Alice), $($GameLogs.Bob); $Took)"; exit 1 }
Write-Host "lobby_match : PASS (Alice created lobby $($Lobby.id), Bob joined from the list, colours $($Colours.Alice) / $($Colours.Bob), both ready, Launch: both in the match (slots 0 / 1), tables with the names, Carol watched (spectator), Bob gave up, Carol got « Partie terminée / Vainqueur : Alice », Dave refused after the end, Carol back to the list by « Quitter vers la liste », Bob back first alone, not leader (Alice still leader in the match), then Alice by the in-game menu, leader again: both on new lines with their match colours (E2-1), not ready; captured; no server left; $Took)"
exit 0
