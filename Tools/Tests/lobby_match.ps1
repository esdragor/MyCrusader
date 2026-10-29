# A lobby match end to end with two game clients (M6 R5.1, R5.2): the published test manager (HOLManager.exe,
# Tools\Manager\build_manager.ps1) with the staged dedicated server (net_match.ps1 -Stage) as its ServerExe, as in
# manager_client.ps1 -Case Match, then two games off screen (-RenderOffScreen), "Alice" and "Bob" (-PlayerName=), each
# with its own Saved folder (-saveddirsuffix=Tests_Alice / Tests_Bob) and log, driven by sequences (hol.Test.Run):
# Alice opens Multiplayer and creates a lobby (Grande by default), chooses the size Petite (TC3: 2 places, slots 2 to 7
# closed, the server opens Plain_Small); once the manager lists it, Bob is started, sees the list ("Plaine — Petite 1/2") and joins; each
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
#   -Rejoin (R7.8): Alice and Bob in a lobby match, Bob closes his game and comes back by « Revenir » (see below).
#   -AI (IA0.6 L5): Alice alone on Petite with an AI put by her slot list, the editor as the server (as -Rejoin); the
#     server's brain thinks for slot 1 and sends its 5 Peasants to gather; Alice gives up (see below).
param([string] $EngineDir = "", [switch] $Rejoin, [switch] $AI, [int] $Window = 0)
$Began = Get-Date
# Every Unreal started from here (games, the editor as a server) skips the SDK check at start (UnrealBuildTool
# -Mode=ValidatePlatforms): it waits behind any build of another copy on this engine, up to minutes (games stuck at
# start, 2026-10-26 and 27). The engine reads UE_SKIP_UBT_SDK_SETUP (TargetPlatform); nothing here needs the SDKs.
$env:UE_SKIP_UBT_SDK_SETUP = "1"
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
# R7.8: -Rejoin has the manager start the editor in server mode through a .cmd (CreateProcess runs it): the code of
# this copy as built, with no staging (net_match.ps1 -Stage takes minutes). IA0.6 L5: -AI the same.
if ($Rejoin -or $AI) {
    $ServerExe = Join-Path $Project "Saved_Tests\LobbyMatch\editor_server.cmd"
    New-Item -ItemType Directory -Force (Split-Path $ServerExe) | Out-Null
    $WindowArg = if ($Window -gt 0) { " -HOLDisconnectWindow=$Window" } else { "" }
    "@`"$EngineDir\UnrealEditor.exe`" `"$UProject`" %* -server -nullrhi -nosound -nosplash -LiveCoding=0 -asdebugport=0 -saveddirsuffix=Tests_LobbySrv$WindowArg" |
        Set-Content -Encoding ascii $ServerExe
}
if (-not (Test-Path $ServerExe)) { Write-Host "lobby_match : FAIL (no $ServerExe; run Tools\Tests\net_match.ps1 -Stage)"; exit 1 }
$TestSaved = Join-Path $Project "Saved_Tests"
$Work = Join-Path $TestSaved "LobbyMatch"
# 2026-10-28: after the end, the players stay in the finished match as long as they want; the server quits only once
# empty. The staged server is started through a .cmd (the manager takes no extra server arguments) that sets the old
# "quit N s after the end" cvar to 5 s, if it still exists: Alice, staying ~70 s, must still find the server running.
if (-not $Rejoin) {
    $StagedServerExe = $ServerExe
    $ServerExe = Join-Path $Work "staged_server.cmd"
    New-Item -ItemType Directory -Force $Work | Out-Null
    "@`"$StagedServerExe`" %* -ExecCmds=`"hol.Net.QuitAfterFinished 5`"" | Set-Content -Encoding ascii $ServerExe
}
$Logs = Join-Path $TestSaved "Logs"
New-Item -ItemType Directory -Force $Work, $Logs | Out-Null
$ManagerLog = Join-Path $Work "manager.log"
$GameLogs = @{ Bob2 = (Join-Path $Logs "lobby_match_bob2.log"); Alice = (Join-Path $Logs "lobby_match_alice.log"); Bob = (Join-Path $Logs "lobby_match_bob.log"); Carol = (Join-Path $Logs "lobby_match_carol.log"); Dave = (Join-Path $Logs "lobby_match_dave.log") }
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
# 2026-10-28: Alice signals once she has stayed ~70 s in the finished match (still connected): the server must still run.
$AliceStayed = Join-Path $Work "alice_stayed.signal"
Remove-Item $ManagerLog, $GameLogs.Bob2, $GameLogs.Alice, $GameLogs.Bob, $GameLogs.Carol, $GameLogs.Dave, $CarolIn, $MatchOver, $DaveDone, $AliceBack, $BobBack, $BobChecked, $BobGivesUp, $AliceStayed -ErrorAction SilentlyContinue
$Colours = @{ Alice = 2; Bob = 5 }

# This copy's own ports (copy_ports.ps1): its test manager on a free TCP port of its range (another copy's manager may
# hold 7700, another copy's test runs in its own range), its match servers from its own UDP range.
. (Join-Path $PSScriptRoot "copy_ports.ps1")
$CopyIndex = Get-CopyIndex $Project
$Port = Get-FreeTcpPort (7790 + 10 * $CopyIndex)
$FirstServerPort = 7900 + 10 * $CopyIndex
$Url = "http://127.0.0.1:${Port}"
# A staged server without Version.txt is "unknown" and refuses everyone (HOLVersion): "dev" for the time of the test.
$ServerVersion = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Version.txt"
# -Rejoin and -AI run the editor as the server: no staged server to mark.
$ServerVersionMade = (-not $Rejoin) -and (-not $AI) -and -not (Test-Path $ServerVersion)
if ($ServerVersionMade) { [IO.File]::WriteAllText($ServerVersion, "dev", (New-Object Text.UTF8Encoding $false)) }
# Servers from this copy's UDP range (7900 and up): out of quick_check's net range (7800-7899), whose clients would join a
# server of ours still starting (seen 2026-10-27: a second « Alice » and « Bob » from another copy in the match), and
# out of the other copies' lobby tests (seen 2026-10-29: a « Carol » of another copy in Arwen's match).
$Manager = Start-Process -FilePath $Exe -ArgumentList "--urls $Url --GameVersion=dev --ServerExe=`"$ServerExe`" --ManagerUrl=$Url --FirstServerPort=$FirstServerPort" `
    -WorkingDirectory $Work -PassThru -WindowStyle Hidden -RedirectStandardOutput $ManagerLog
$Games = @{}
$Failure = $null
$Matches0 = Join-Path $Project "Saved\Manager\Saved\Matches"
$Servers = Join-Path $Work "servers.json"
"{ `"servers`": [ { `"name`": `"Test`", `"url`": `"$Url`" } ] }" | Set-Content -Encoding ascii $Servers

# This copy's match servers (the test manager's), by path: never another worktree's.
function Get-OurServers { @(Get-Process HeroesOfLibertyServer* -ErrorAction SilentlyContinue | Where-Object { $_.Path -and $_.Path.StartsWith($Project) }) +
    @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($Project) -and $_.CommandLine.Contains(" -server ") } | ForEach-Object { Get-Process -Id $_.ProcessId -ErrorAction SilentlyContinue }) }
function Test-Log([string] $Who, [string] $Pattern) { (Test-Path $GameLogs[$Who]) -and (Select-String -Path $GameLogs[$Who] -Pattern $Pattern -Quiet) }
function Start-Game([string] $Who, [string[]] $Steps, [string] $Name = $Who) {
    $Sequence = Join-Path $Work "$($Who.ToLower()).seq"
    $Steps | Set-Content -Encoding utf8 $Sequence
    $Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -unattended -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -culture=fr " +
        "-saveddirsuffix=Tests_$Name -PlayerName=$Name -ServersFile=`"$Servers`" -ExecCmds=`"hol.Test.Run $Sequence`" -abslog=`"$($GameLogs[$Who])`""
    $Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
    Write-Host "$Who : pid $($Process.Id), log $($GameLogs[$Who])"
    $Process
}
$Press = { param($Button) @("hol.UI.Focus $Button", "frames 3", "hol.UI.Key Enter 0 0.1", "wait 0.15") }
# Both, once the lobby is full: their colour and Ready; in the match: a capture; then the end banner and « Retour au salon ».
$Pick = { param($Who, $Line) @("waitlog : 2 players (", "waitui", "wait 1", "hol.UI.Choose SlotColor$Line $($Colours[$Who])", "waitlog Lobby: colour $($Colours[$Who]) accepted",
    "wait 0.5") + (& $Press ReadyButton) + @("waitlog Lobby: ready on accepted") }
# TC3: the leader turns the new lobby (Large) to Small: two places, Plain_Small.
$ToSmall = @("waitlog map Plain Large teams together 1:", "waitui", "hol.UI.Choose SizeDropdown 1",
    "waitlog settings Plain size Small teams together 1 nomad 0 regicide 0 catastrophe 0 accepted")
$InMatch = { param($Who) @("waitlog Lobby: joining match", "waitmap Plain_Small", "waitlog Camera above the start", "frames 60", "capture lobby_match_$($Who.ToLower())") }
$Back = @("waitlog End banner: lobby match", "wait 1.5") + (& $Press ReturnButton) + @("waitmap MainMenu", "waitlog Menu: back in lobby", "wait 3", "quit")
# C3: once the match is over, the statistics from the server (hol.Stats.Dump on a client logs the copy once complete),
# then $Back from its third step.
$StatsAfter = @("waitlog End banner: lobby match", "hol.Stats.Dump", "waitlog Stats: client copy received", "frames 3", "wait 1.5")
# M6-Q7 revised: nobody is sent back. Alice stays in the finished match past 60 s (two waits, a step lasts 60 s at most;
# nobody keeps a slot after the end, M6-Q14; she goes back once Bob is back and checked), the banner still there with no countdown; she closes it
# (« Continuer à regarder »), opens the statistics from the game screen's « Statistiques », closes them, then opens the
# in-game menu (« Menu ») and goes back by its « Retour au salon » ($Back from its third step).
$StayAfterEnd = @("wait 35", "wait 36", "signal $AliceStayed", "frames 3", "capture lobby_match_end_late") + (& $Press ContinueButton) + @("waitlog End banner: closed", "frames 3",
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

# IA0.6 L5 (-AI): a lobby match against an AI, end to end. Alice (launched as -PlayerName=Alice) types "Alicia" in the
# name box of the Multiplayer screen (the user's trial, 2026-10-29: the typed name must win at the login), creates the lobby, turns it to Petite (2 places), takes
# the colour 2, puts an AI on slot 1 by its list (hol.UI.Choose SlotState1 4: "IA Normal 2"; an AI counts in the 2
# players, IA0.6-Q3 A), presses Ready (1/1) and Launch; she reaches Plain_Small, the AI's Peasants seen at work from her
# side (hol.Units.Where 1); she gives up (hol.Victory.Surrender), gets her end banner and quits; the server quits once
# empty. The server (the editor, as -Rejoin): match.json with slot 1 "AI" Normal, the brain thinking for slot 1, its 5
# starting Peasants sent to gather. Captures lobby_match_ai_lobby / _start / _match / _end (Saved_Tests_Alice).
if ($AI) {
    try {
        $Status = $null
        for ($i = 0; $i -lt 40 -and -not $Status; $i++) { try { $Status = Invoke-RestMethod "${Url}/status" -TimeoutSec 2 } catch { Start-Sleep -Milliseconds 250 } }
        if (-not $Status) { throw "the manager does not answer on $Url" }
        $Games.Alice = Start-Game Alice (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 0 lobbies shown", "frames 3", "hol.UI.Type NameBox Alicia", "frames 2") +
            (& $Press CreateButton) + $ToSmall + @("waitui", "wait 1", "hol.UI.Choose SlotColor0 $($Colours.Alice)", "waitlog Lobby: colour $($Colours.Alice) accepted", "wait 0.5",
            "hol.UI.Choose SlotState1 4", "waitlog Lobby: slot 1 ai accepted", "wait 0.5", "hol.UI.Text SlotState1", "frames 2") +
            (& $Press ReadyButton) + @("waitlog Lobby: ready on accepted", "waitlog 1/1 ready", "wait 1", "capture lobby_match_ai_lobby") +
            (& $Press LaunchButton) + @("waitlog Lobby: start sent", "waitlog Lobby: joining match", "waitmap Plain_Small", "waitlog Camera above the start", "frames 60",
            "capture lobby_match_ai_start", "wait 20", "hol.Units.Where 1", "waitlog Units where:", "frames 3", "capture lobby_match_ai_match",
            "hol.Victory.Surrender", "waitlog End banner: lobby match", "wait 1.5", "capture lobby_match_ai_end", "wait 1", "quit"))
        if (-not $Games.Alice.WaitForExit(360000)) { throw "Alice still there after 360 s" }
    }
    catch { $Failure = "$_" }
    finally {
        foreach ($Game in $Games.Values) { if ($Game -and -not $Game.HasExited) { Stop-Process -Id $Game.Id -Force -ErrorAction SilentlyContinue } }
        # The server quits by itself once empty (R3.8); left after 30 s, it is stopped (and the test fails).
        for ($i = 0; $i -lt 30 -and (Get-OurServers).Count; $i++) { Start-Sleep 1 }
        $Left = Get-OurServers
        if ($Left.Count) { $Left | ForEach-Object { Write-Host "  stopping server PID $($_.Id)"; Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue }; if (-not $Failure) { $Failure = "$($Left.Count) server(s) of this copy still running (after Alice left, the AI alone)" } }
        if ($Manager) { Stop-Process -Id $Manager.Id -Force -ErrorAction SilentlyContinue }
    }
    $ServerLog = Get-ChildItem $Matches0 -Recurse -Filter server.log -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -gt $Began } | Sort-Object LastWriteTime | Select-Object -Last 1
    $S = if ($ServerLog) { @(Get-Content $ServerLog.FullName) } else { @() }
    $MatchJson = if ($ServerLog) { Get-Content (Join-Path $ServerLog.DirectoryName "match.json") -Raw -ErrorAction SilentlyContinue | ConvertFrom-Json } else { $null }
    $AISlot = if ($MatchJson) { $MatchJson.slots[1] } else { $null }
    $Start = @(Get-Content $ManagerLog -ErrorAction SilentlyContinue | Select-String -Pattern "Starting the server of" | ForEach-Object { $_.Line }) | Select-Object -First 1
    $L = if (Test-Path $GameLogs.Alice) { @(Get-Content $GameLogs.Alice -Encoding utf8) } else { @() }
    if ($ServerLog) { Write-Host "  server log $($ServerLog.FullName):"; $S -match "Match on |AI brain|AI slot 1 step|Gather order of slot 1|Starting Peasants|holds slot|Victory: (slot|match)|empty" | Select-Object -First 25 | ForEach-Object { Write-Host "    $_" } }
    Write-Host "  match.json slot 1: $(if ($AISlot) { "$($AISlot.state) $($AISlot.difficulty) colour $($AISlot.colorIndex)" } else { 'not read' }); size $($MatchJson.mapSize)"
    Write-Host "  manager: $Start"
    Write-Host "  Alice :"; $L -match "Lobby: (slot|colour|ready|start|joining)|ready, launch|Text SlotState1|Players table|Units where|End banner|Test capture|Test sequence: timeout" | ForEach-Object { Write-Host "    $_" }
    $Where = @($L | ForEach-Object { if ($_ -match "Units where: (.*)$") { $Matches[1] } }) | Select-Object -Last 1
    $AIBusy = @([regex]::Matches("$Where", "#\d+ slot 1 at \([^)]*\) (\w+)") | Where-Object { $_.Groups[1].Value -in "GatherTrip", "Gathering", "DepositTrip", "Depositing" }).Count
    if (-not $Failure) {
        if (-not ($L -match "LogHOLUI: Display: Lobby: slot 1 ai accepted")) { $Failure = "Alice : no 'Lobby: slot 1 ai accepted'" }
        elseif (-not ($L -match "Text SlotState1: IA Normal 2$")) { $Failure = "Alice : the slot list does not show 'IA Normal 2'" }
        elseif (-not ($L -match "1 players \(Alicia\), you Alicia, leader Alicia, 1/1 ready, launch shown, slots c$($Colours.Alice)r a\d x x x x x x$")) { $Failure = "Alice : no lobby line '1 players (Alicia) ... 1/1 ready ... slots c$($Colours.Alice)r a<n> x x x x x x'" }
        elseif (-not ($L -match "Lobby: start accepted \(HTTP 202\)")) { $Failure = "Alice's Launch not accepted" }
        elseif (-not ($L -match "LogHOLUI: Display: Lobby: joining match \S+ at 127\.0\.0\.1:\d+ as Alicia")) { $Failure = "Alice : no 'Lobby: joining match ... as Alicia'" }
        elseif (-not ($L -match "Players table: Vous, IA Normal 2$")) { $Failure = "Alice : the table of players is not 'Vous, IA Normal 2'" }
        elseif (-not ($L -match "Test capture: lobby_match_ai_match")) { $Failure = "Alice : no capture in the match" }
        elseif ($AIBusy -lt 5) { $Failure = "Alice : $AIBusy of the AI's Peasants seen at work, 5 wanted ($Where)" }
        elseif (-not ($L -match "End banner: Defeat")) { $Failure = "Alice : no Defeat banner after her surrender" }
        elseif (-not ($L -match "End banner: lobby match")) { $Failure = "Alice : no lobby end banner" }
        elseif ($L -match "Test sequence: timeout") { $Failure = "Alice : her sequence timed out" }
        elseif (-not $ServerLog) { $Failure = "no server log of the match" }
        elseif (-not $AISlot -or $AISlot.state -ne "AI" -or $AISlot.difficulty -ne "Normal") { $Failure = "match.json without slot 1 'AI' Normal" }
        elseif ($Start -match "StartingPeasants") { $Failure = "the server started with StartingPeasants on its command line" }
        elseif (-not ($S -match "Match on \S+: given configuration, 2 occupied slots")) { $Failure = "the server has no 'given configuration, 2 occupied slots'" }
        elseif (-not ($S -match "AI brain \S+ awake, net mode DedicatedServer, thinking slots \[1\]")) { $Failure = "the server has no 'AI brain ... DedicatedServer, thinking slots [1]'" }
        elseif (-not ($S -match "AI slot 1 step \d+: 5 idle -> gather")) { $Failure = "the server has no 'AI slot 1 step N: 5 idle -> gather'" }
        elseif (-not ($S -match "Gather order of slot 1: 5 units to source")) { $Failure = "the server has no 'Gather order of slot 1: 5 units to source'" }
        elseif (-not ($S -match "LogHOLMatch: Alicia holds slot 0")) { $Failure = "the server has no 'Alicia holds slot 0' (the name typed on the Multiplayer screen, over -PlayerName=Alice$(if ($S -match 'joined as a spectator') { '; she joined as a spectator' }))" }
        elseif (-not ($S -match [regex]::Escape("Victory: slot 0 defeated (Surrender)"))) { $Failure = "the server has no 'Victory: slot 0 defeated (Surrender)'" }
        elseif (@($S -match "LogNet: Join succeeded: ").Count -ne 1) { $Failure = "$(@($S -match 'LogNet: Join succeeded: ').Count) joins on the server, 1 expected" }
    }
    $Took = "{0:N0} s" -f ((Get-Date) - $Began).TotalSeconds
    if ($Failure) { Write-Host "lobby_match : FAIL (ai: $Failure; log $($GameLogs.Alice); $Took)"; exit 1 }
    Write-Host "lobby_match : PASS (ai: Alice, renamed Alicia on the screen, created a Petite lobby, put an AI on slot 1 (IA Normal 2), 1/1 ready, Launch; match.json slot 1 AI Normal; the server's brain thought for slot 1 and sent its 5 Peasants to gather, seen at work by Alice ($AIBusy); table 'Vous, IA Normal 2'; Alice gave up, Defeat banner; no server left; $Took)"
    exit 0
}

# R7.8 (-Rejoin): Alice creates, Bob joins, both ready, Launch; Bob closes his game ("quit"); Alice gets « Bob : connexion
# perdue », the server keeps slot 1 (and its stock), the manager gets "left" and lists Bob away; Bob starts again, sees
# « Revenir » on the lobby's line, presses it, takes slot 1 back with the same stock (the server: "back", "Manager: Bob
# back"), Alice gets « Bob : de retour »; Bob queues a Peasant at his centre (R7.4: the server accepts it); both quit.
# -Window <s> (R7.8, second round): the server's window cut to <s>; Bob does not come back in time: the server defeats
# slot 1 (Disconnected), Alice, alone left, wins (End banner: Victory); Bob started again finds no « Revenir » (the
# match is over: with two players his defeat ends it, so his return as a spectator is left to Match.DisconnectGameState).
if ($Rejoin) {
    $Late = $Window -gt 0
    $RejoinDone = Join-Path $Work "rejoin_done.signal"
    Remove-Item $RejoinDone -ErrorAction SilentlyContinue
    $Away = $null
    try {
        $Status = $null
        for ($i = 0; $i -lt 40 -and -not $Status; $i++) { try { $Status = Invoke-RestMethod "${Url}/status" -TimeoutSec 2 } catch { Start-Sleep -Milliseconds 250 } }
        if (-not $Status) { throw "the manager does not answer on $Url" }
        $AliceEnd = if ($Late) { @("waitlog End banner: Victory", "waitlog Absence banner: hidden", "frames 3", "capture lobby_match_rejoin_win") } else { @("waitlog de retour", "frames 3", "capture lobby_match_rejoin_back") }
        $Games.Alice = Start-Game Alice (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 0 lobbies shown", "frames 3") +
            (& $Press CreateButton) + $ToSmall + (& $Pick Alice 0) + @("waitlog 2/2 ready", "wait 3") + (& $Press LaunchButton) + @("waitlog Lobby: start sent") +
            (& $InMatch Alice) + @("waitlog Absence banner: shown, Bob", "frames 3", "capture lobby_match_rejoin_away") + $AliceEnd +
            @("waitfile $RejoinDone 300", "wait 1", "quit"))
        $Deadline = (Get-Date).AddSeconds(180)
        $Lobby = $null
        while (-not $Lobby) {
            if ($Games.Alice.HasExited -or (Get-Date) -gt $Deadline) { throw "no lobby created by Alice" }
            Start-Sleep -Milliseconds 500
            $Lobby = @((Invoke-RestMethod "${Url}/lobbies" -TimeoutSec 2).lobbies)[0]
        }
        Write-Host "lobby $($Lobby.id) created by Alice"
        $Games.Bob = Start-Game Bob (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 3") +
            (& $Press JoinButton0) + (& $Pick Bob 1) + @("waitlog 2/2 ready") + (& $InMatch Bob) + @("hol.HUD.Report", "wait 3", "quit"))
        if (-not $Games.Bob.WaitForExit(300000)) { throw "the first game of Bob still there after 300 s" }
        Write-Host "first game of Bob closed"
        if ($Late) {
            # The window runs out: the match over, the lobby no longer in_game at the manager.
            $Deadline = (Get-Date).AddSeconds($Window + 60)
            while ((Invoke-RestMethod "${Url}/lobbies/$($Lobby.id)" -TimeoutSec 2).state -eq "in_game") {
                if ((Get-Date) -gt $Deadline) { throw "the lobby still in_game $($Window + 60) s after Bob left" }
                Start-Sleep -Milliseconds 500
            }
            Write-Host "manager: match of $($Lobby.id) over"
            $Games.Bob2 = Start-Game Bob2 (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog lobbies shown", "wait 2", "frames 3",
                "capture lobby_match_rejoin_late", "signal $RejoinDone", "wait 1", "quit")) Bob
            if (-not $Games.Bob2.WaitForExit(300000)) { throw "the second game of Bob still there after 300 s" }
            if (-not $Games.Alice.WaitForExit(60000)) { throw "Alice still there 60 s after Bob" }
        }
        else {
            # The manager: Bob away (the server's "left").
            $Deadline = (Get-Date).AddSeconds(30)
            while ((Get-Date) -lt $Deadline -and -not $Away) {
                Start-Sleep -Milliseconds 500
                $Listed = @((Invoke-RestMethod "${Url}/lobbies" -TimeoutSec 2).lobbies | Where-Object { $_.id -eq $Lobby.id })[0]
                if ($Listed -and @($Listed.away) -contains "Bob") { $Away = $Listed }
            }
            if (-not $Away) { throw "the manager never lists Bob away" }
            Write-Host "manager: away $(@($Away.away) -join ', ')"
            $Games.Bob2 = Start-Game Bob2 (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 3",
                "capture lobby_match_rejoin_list") + (& $Press RejoinButton0) + @("waitlog Multiplayer: back in match", "waitmap Plain_Small", "waitlog Camera above the start",
                "frames 30", "hol.HUD.Report", "hol.Prod.Queue", "wait 3", "capture lobby_match_rejoin_bob", "signal $RejoinDone", "wait 1", "quit")) Bob
            if (-not $Games.Bob2.WaitForExit(300000)) { throw "the second game of Bob still there after 300 s" }
            if (-not $Games.Alice.WaitForExit(60000)) { throw "Alice still there 60 s after Bob" }
        }
    }
    catch { $Failure = "$_" }
    finally {
        foreach ($Game in $Games.Values) { if ($Game -and -not $Game.HasExited) { Stop-Process -Id $Game.Id -Force -ErrorAction SilentlyContinue } }
        for ($i = 0; $i -lt 30 -and (Get-OurServers).Count; $i++) { Start-Sleep 1 }
        $Left = Get-OurServers
        if ($Left.Count) { $Left | ForEach-Object { Write-Host "  stopping server PID $($_.Id)"; Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue }; if (-not $Failure) { $Failure = "$($Left.Count) server(s) of this copy still running" } }
        if ($Manager) { Stop-Process -Id $Manager.Id -Force -ErrorAction SilentlyContinue }
        if ($ServerVersionMade) { Remove-Item $ServerVersion -ErrorAction SilentlyContinue }
    }
    $ServerLog = Get-ChildItem $Matches0 -Recurse -Filter server.log -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -gt $Began } | Sort-Object LastWriteTime | Select-Object -Last 1
    $S = if ($ServerLog) { @(Get-Content $ServerLog.FullName) } else { @() }
    $Lines = @{}
    foreach ($Who in "Alice", "Bob", "Bob2") { $Lines[$Who] = if (Test-Path $GameLogs[$Who]) { @(Get-Content $GameLogs[$Who] -Encoding utf8) } else { @() } }
    if ($ServerLog) { Write-Host "  server log $($ServerLog.FullName):"; $S -match "holds slot|Disconnect:|Manager: Bob|Production: slot 1|spectator|Victory: (slot|match)" | ForEach-Object { Write-Host "    $_" } }
    foreach ($Who in "Alice", "Bob2") { Write-Host "  $Who :"; $Lines[$Who] -match "Absence banner|End banner: (Victory|Defeat|MatchOver)|Multiplayer: (lobby|back|rejoin)|Test sequence: timeout|LogHOL\w+: .*Production" | ForEach-Object { Write-Host "    $_" } }
    # Only our games in the match: Alice, Bob, and Bob again after « Revenir » (never in the -Window round).
    $Joins = @($S -match "LogNet: Join succeeded: ").Count
    $JoinsExpected = if ($Late) { 2 } else { 3 }
    if (-not $Failure -and $ServerLog -and $Joins -ne $JoinsExpected) { $Failure = "$Joins joins on the server, $JoinsExpected expected (another copy's game in the match?)" }
    if (-not $Failure -and $Late) {
        $LobbyLine = @($Lines.Bob2 -match "Multiplayer: lobby $([regex]::Escape($Lobby.id)) ")
        if (-not $ServerLog) { $Failure = "no server log of the match" }
        elseif (-not ($S -match "Disconnect: slot 1 \(Bob\) kept")) { $Failure = "the server has no 'Disconnect: slot 1 (Bob) kept'" }
        elseif (-not ($S -match "Disconnect: slot 1 not back within $Window s")) { $Failure = "the server has no 'Disconnect: slot 1 not back within $Window s'" }
        elseif (-not ($S -match "Victory: slot 1 defeated \(Disconnected\)")) { $Failure = "the server has no 'Victory: slot 1 defeated (Disconnected)'" }
        elseif (-not ($S -match "Victory: match over")) { $Failure = "the server has no 'Victory: match over'" }
        elseif (-not ($Lines.Alice -match "Absence banner: shown, Bob : connexion perdue — 0:(0\d|10)")) { $Failure = "Alice : no banner with at most 0:$Window" }
        elseif (-not ($Lines.Alice -match "End banner: Victory")) { $Failure = "Alice : no victory banner" }
        elseif ($LobbyLine -match ", rejoin") { $Failure = "Bob : « Revenir » offered after his defeat" }
        elseif ($S -match "Manager: Bob back") { $Failure = "the server took Bob back" }
        elseif (($Lines.Alice + $Lines.Bob + $Lines.Bob2) -match "Test sequence: timeout") { $Failure = "a sequence timed out" }
        $Took = "{0:N0} s" -f ((Get-Date) - $Began).TotalSeconds
        if ($Failure) { Write-Host "lobby_match : FAIL (rejoin -Window ${Window}: $Failure; logs $($GameLogs.Alice), $($GameLogs.Bob), $($GameLogs.Bob2); $Took)"; exit 1 }
        Write-Host "lobby_match : PASS (rejoin -Window ${Window}: Bob left, not back within $Window s: slot 1 defeated (Disconnected), Alice wins; Bob started again gets no « Revenir » ($(if ($LobbyLine) { 'lobby listed' } else { 'lobby not listed' })); $Took)"
        exit 0
    }
    if (-not $Failure) {
        $Kept = @($S | ForEach-Object { if ($_ -match "Disconnect: slot 1 \(Bob\) kept, F (\d+) W (\d+) S (\d+) G (\d+)") { "$($Matches[1]) $($Matches[2]) $($Matches[3]) $($Matches[4])" } })[0]
        $BackLine = @($S -match "Disconnect: slot 1 \(Bob\) back, ")[0]
        $BackStock = if ($BackLine -match "back, (.*), population") { (@([regex]::Matches($Matches[1], "\d+") | ForEach-Object { $_.Value }) -join " ") } else { $null }
        if (-not $ServerLog) { $Failure = "no server log of the match" }
        elseif (-not ($S -match "LogHOLMatch: Bob holds slot 1")) { $Failure = "the server has no 'Bob holds slot 1'" }
        elseif (-not $Kept) { $Failure = "the server has no 'Disconnect: slot 1 (Bob) kept'" }
        elseif (-not ($S -match "Manager: Bob left")) { $Failure = "the server has no 'Manager: Bob left'" }
        elseif (-not ($Lines.Alice -match "Absence banner: shown, Bob")) { $Failure = "Alice : no absence banner for Bob" }
        elseif (-not ($Lines.Bob2 -match "Multiplayer: lobby $([regex]::Escape($Lobby.id)) .*, rejoin")) { $Failure = "Bob : the line of the lobby without « Revenir » (', rejoin')" }
        elseif (-not ($Lines.Bob2 -match "Multiplayer: back in match \S+ of $([regex]::Escape($Lobby.id)) on port \d+")) { $Failure = "Bob : « Revenir » did not open the match" }
        elseif (@($S -match "LogHOLMatch: Bob holds slot 1").Count -lt 2) { $Failure = "the server: Bob not back in slot 1" }
        elseif (-not $BackLine) { $Failure = "the server has no 'Disconnect: slot 1 (Bob) back'" }
        elseif ($BackStock -ne $Kept) { $Failure = "Bob back with stock ($BackStock), not the one kept ($Kept)" }
        elseif (-not ($S -match "Manager: Bob back")) { $Failure = "the server has no 'Manager: Bob back'" }
        elseif (-not ($Lines.Alice -match "Absence banner: shown, .*Bob.*de retour")) { $Failure = "Alice : no « Bob : de retour »" }
        elseif (-not ($S -match "Production: slot 1 queued Peasant")) { $Failure = "the server did not accept the Peasant of Bob after his return (R7.4)" }
        elseif (($Lines.Alice + $Lines.Bob + $Lines.Bob2) -match "Test sequence: timeout") { $Failure = "a sequence timed out" }
        elseif ($S -match "Disconnect: slot \d+ not back within") { $Failure = "a slot defeated by its absence" }
    }
    $Took = "{0:N0} s" -f ((Get-Date) - $Began).TotalSeconds
    if ($Failure) { Write-Host "lobby_match : FAIL (rejoin: $Failure; logs $($GameLogs.Alice), $($GameLogs.Bob), $($GameLogs.Bob2); $Took)"; exit 1 }
    Write-Host "lobby_match : PASS (rejoin: Bob left the match of lobby $($Lobby.id), slot 1 kept with F W S G $Kept, banner at Alice, the manager listed him away; « Revenir »: slot 1 back, same stock, manager told, « de retour » at Alice, his Peasant accepted; $Took)"
    exit 0
}

try {
    $Status = $null
    for ($i = 0; $i -lt 40 -and -not $Status; $i++) {
        try { $Status = Invoke-RestMethod "${Url}/status" -TimeoutSec 2 } catch { Start-Sleep -Milliseconds 250 }
    }
    if (-not $Status) { throw "the manager does not answer on $Url" }

    $Games.Alice = Start-Game Alice (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 0 lobbies shown", "frames 3") +
        (& $Press CreateButton) + $ToSmall + (& $Pick Alice 0) + @("waitlog 2/2 ready", "wait 3") + (& $Press LaunchButton) + @("waitlog Lobby: start sent") +
        (& $InMatch Alice) + @("waitfile $BobGivesUp 240") + $StatsAfter + $StayAfterEnd + @("waitfile $BobChecked 240") + $Back[2..($Back.Count - 3)] + @("signal $AliceBack") + $Back[($Back.Count - 2)..($Back.Count - 1)])
    # Bob only once the manager lists Alice's lobby: the list is read once, when the screen opens.
    $Deadline = (Get-Date).AddSeconds(180)
    $Lobby = $null
    # TC3: listed once Alice has chosen Petite (2 places).
    while (-not $Lobby -or $Lobby.size -ne "Small") {
        if ($Games.Alice.HasExited -or (Get-Date) -gt $Deadline) { throw "no lobby created by Alice on Petite" }
        Start-Sleep -Milliseconds 500
        $Lobby = @((Invoke-RestMethod "${Url}/lobbies" -TimeoutSec 2).lobbies)[0]
    }
    if ($Lobby.maxPlayers -ne 2) { throw "lobby on Petite with maxPlayers $($Lobby.maxPlayers), 2 wanted" }
    Write-Host "lobby $($Lobby.id) created by Alice, $($Lobby.map) $($Lobby.size), $($Lobby.maxPlayers) places"
    $Games.Bob = Start-Game Bob (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 3") +
        (& $Press JoinButton0) + (& $Pick Bob 1) + @("waitlog 2/2 ready", "wait 0.5", "capture lobby_match_lobby") +
        (& $InMatch Bob) + @("waitfile $CarolIn 180", "hol.Stats.Dump", "waitlog Stats: the server refused", "wait 2", "signal $BobGivesUp", "hol.Victory.Surrender") +
        # 2026-10-28: Bob stays 10 s in the lobby once Alice is back (3 s raced the read of the lobby below, 2 s after both are back).
        $StatsAfter + $Back[2..($Back.Count - 3)] + @("signal $BobBack", "waitfile $AliceBack 240", "wait 10", "quit"))

    # R5.3: Carol, once the match is under way (state "in_game" at the manager), sees the lobby in the list, presses
    # « Regarder », reaches the match as a spectator and captures it; she stays until well after the end (what she gets is noted).
    $Deadline = (Get-Date).AddSeconds(180)
    while ((Invoke-RestMethod "${Url}/lobbies/$($Lobby.id)" -TimeoutSec 2).state -ne "in_game") {
        if ($Games.Bob.HasExited -or (Get-Date) -gt $Deadline) { throw "the lobby never in_game" }
        Start-Sleep -Milliseconds 500
    }
    Write-Host "lobby $($Lobby.id) in_game: Carol started"
    $Games.Carol = Start-Game Carol (@("waitmap MainMenu", "waitui", "hol.UI.Open Multiplayer", "waitui", "waitlog Multiplayer: 1 lobbies shown", "frames 3",
        "capture lobby_match_watchlist") + (& $Press WatchButton0) + @("waitlog Multiplayer: watching match", "waitmap Plain_Small", "frames 120", "capture lobby_match_watch") + $StatsLive + @(
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
    $ServersAfterStay = $null
    while ((Get-Date) -lt $Deadline) {
        Start-Sleep -Seconds 1
        # 2026-10-28: Alice still in the finished match ~70 s after its end (past the old 5 s set by staged_server.cmd).
        if ($null -eq $ServersAfterStay -and (Test-Path $AliceStayed)) {
            $ServersAfterStay = (Get-OurServers).Count
            Write-Host "  Alice stayed in the finished match: $ServersAfterStay server(s) of this copy running"
        }
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

    # 2026-10-28: the server quits only once empty, never N s after the end with players still in.
    if ($ServerLog -and (Select-String -Path $ServerLog.FullName -Pattern "Server quits: .*after the end of the match" -Quiet)) { $Failure = "the server quit after the end of the match with Alice still in ('" + (Select-String -Path $ServerLog.FullName -Pattern "Server quits: " | Select-Object -First 1).Line + "')" }
    elseif ($null -eq $ServersAfterStay) { $Failure = "Alice never signalled her stay in the finished match" }
    elseif ($ServersAfterStay -lt 1) { $Failure = "no server running once Alice stayed ~70 s in the finished match (still connected)" }
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
        elseif (-not ($L -match "LoadMap: 127\.0\.0\.1(:\d+)?/Game/Maps/Plain_Small\?Version=")) { $Failure = "Carol : Plain_Small not loaded from the server (or with a Name)" }
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
        elseif (-not ($Lines.Bob -match "Multiplayer: lobby \S+ .* Plaine .{1,3} Petite 1/2 au salon, join")) { $Failure = "Bob's list without 'Plaine — Petite 1/2' (TC3)" }
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
