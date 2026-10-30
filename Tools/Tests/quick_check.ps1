# Quick check of the project (D267), on two levels (D327):
#   - by default (no argument, under a minute): the automation tests in their own process WHILE a single short game goes
#     through the "smoke" part: the title and New game screens in French, a 2-player match launched from there, the
#     camera moved by the arrow key, its own centre selected then nothing, 3 Men-at-arms boxed and sent beside an enemy
#     Peasant, which they kill;
#   - -Full (~530 s): every part below, the default before D327 (at a closure, or when a part's ground changed);
#   - -Parts a,b,c: those parts alone (tests, smoke, menu, camera, production, move, sources, gather, build, anims,
#     formation, combat, bodies, rocks, victory, sides, sizes, net, lobby, aiserver, netai, hero; bodies, rocks, lobby and hero only when asked, not in -Full;
#     "hero" (J8a.6, ~3 min): build:hero in solo and the net part cut to its hero phases (sheets, return, block);
#     "aiserver" (IA0.5 B1, ~40 s, last): the AI's brain on a dedicated server without client (slot 1 AI, 5 starting Peasants);
#     "netai" (IA0.5 B2, ~60 s, after "aiserver"): the same server with Alice and Bob joined and the AI in slot 2, its
#     Peasants seen gathering on each client;
#     "sizes" (TC4, ~15 s): Plain at its five sizes and a refused configuration, in the menu's game launch). The solo parts (production to combat) run in the camera part's 2-player match, so the
#     camera part comes with them; each one counts what the parts before it left (Peasants, sources used up). "net" runs
#     alone (~95 s: its camera, centres, production, army and fight); with "move", "sources" or "build" its clients also
#     go through their orders, gathering, houses and camps (the server's commands for those come with them); with
#     "victory", it ends with the end of the match (~10 s more, see below);
#   - -Parts part:case (EtapeDefini F): only that case of the part, its steps and its checks (build:cut, build:gap,
#     build:hero; several with commas: build:cut,build:gap). "build", "formation" and "combat" have cases ($SubCaseTable): the build cases before it are
#     skipped (a short set-up instead: its wood, its logs), the camera part only plays its 2-player match (no 8-player).
# The full run: a single game launch goes through the menu in French, English and Japanese (language switched live), a
# drop-down list, then a match at 2 and at 8 players with the camera checks of D257 and the selection checks of D264 (own
# centre, another's, void, Escape).
# Every step starts as soon as the one before is done (hol.Test.Run), no fixed delays. Replaced capture_screens.ps1,
# check_camera.ps1 and the automation command for everyday checks; the short stress tests S1 and S2 stay apart (only
# when units, the simulation or rendering change, and at a closure).
# Alongside, the "net" part (D273, M2.5.6): a two-client match on a dedicated server, the editor in server mode (D275),
# each client checking its camera, the two centres in their colours and a click on each.
# With "victory" (V1.4c, D356-D358), last in the net part (the match is over after it): client 1 loses (its
# King killed on the server since V5.5a, its surrender before); the server defeats its slot, collapses it and ends
# the match, client 2's slot the winner; the banner Defeat on client 1, Victory on client 2 (the game state's replication
# and OnRep); client 2 sees client 1's units go (Units report each second, none left after 5 s).
# The "sources" part (J5a.2, D297): the wood beside slot 0, forbidden on the navmesh tree by tree; a tree used up
# leaves the map on the server and on both clients. With "net" (J5a.5, D299, D303): each client sends its Peasant to
# the tree nearest its start, the server having left 5 wood in it; the tree is used up on both clients, the wood rises
# at the deposit, the exact load reaches its owner alone, the other client only knows that it carries wood; J5b.3: the
# HUD's counts of gatherers and idle workers are each player's own, slot 0's tree selected on both clients shows what is
# left going down, each client sees the other's Peasant on its round trip.
# The "gather" part (J5b.3, D299, D307, D309), in solo: a rally point on the berries (the Peasant produced gathers there,
# the food rises at the deposit only), twelve Peasants on one tree (the others gather the trees beside it), the berries
# used up (all idle, the HUD's counter, the "." key), no two Peasants at rest closer than 90 cm. J5c.3 (D298): each unload
# shows a +10 to its owner alone, with what was unloaded (solo: captured; 2 clients: never on the other client).
# J6a.1 (D310): no Peasant's body in a building's footprint, watched at every step (hol.Nav.Footprints) over the gather
# part, then an order onto the centre itself and one around it (solo), and over the round trips of the two clients
# (server); the navmesh ends 60 cm (agent) + 25 cm (BodyClearance) at least from the centre's sides.
# J5c.1: each client knows what the other's Peasant gathers (wood), received in the activity's bits.
# The "build" part (J6a.3, D311), in solo: placement mode, the ghost green on free ground, a house placed by a click,
# another one touching it with Shift (still in the mode), the ghost red near the enemy centre, Escape; refused (nothing
# taken): on a house, on a tree, near the enemy centre, without enough wood; the second foundation selected by a click and cancelled
# (refunded). The user's trial of J7b, feedback 3: the ghost on house A goes beside it (green, ring 0, nearest the
# cursor), a click places it there (then cancelled, refunded). With "net": each client places a house behind its centre, sees the other's, pays alone, and a house near
# the enemy centre is refused. The two clients wait for each other before quitting (each one's last report done).
# J6b.3, with "net": each client's Peasant builds a lumber camp by the wood beside its centre and unloads there; the
# other's camp reaches a client at its first work, then finished; a camp asked on a tree is refused by the server.
# J6a.4 (D312), solo: a Peasant sent by a right click on house A (a ghost until then): its first work blocks the navmesh,
# it rises, finished alone in the table's time (hol.Build.Speed 2), the population cap 10 -> 20, the builder idle; three
# Peasants place house C, then D with Shift: C built by the three (about twice as fast), then D; the first Peasant helps
# on D (a right click on the site), D cancelled after its first work (the share not built refunded, D345: 1 to 49 of its 50 wood), its builders idle. With "net": each
# client's Peasant builds its house; the other's foundation never reaches a client before its first work, then it does.
# The user's trial of J6a.4, solo: the centre's rally point on a ghost house E (the flag on it), every site finished at
# its first blow (hol.Cheat.FastBuild 1): the Peasant produced builds E, goes on to F 8 m away (hol.Build.NextSiteRange,
# 20 m), not to G 40 m away, then stands idle; Shift + place H, Shift + place I, Shift + right click on a tree: H built,
# then I, then it gathers wood.
# J6a.5 (D313), solo: every Peasant selected without wood, the age panels I to IV (II to IV locked), panel I's house, lumber camp, mining camp and
# barracks on Q, S, D, F greyed, their tooltips; a click on the lumber camp: no ghost, the message "Pas assez de bois" (captured in French,
# English and Japanese); with wood, the key Z starts the lumber camp's ghost; two Peasants place house J by the key A,
# the site selected shows its progress, its builders and "Annuler" (V), clicked: refunded; the population full, the key
# A at the centre: the message "Population maximale". The user's trial of J6a.5: the cells write the name alone (the cost
# in the tooltip only); F1 and F3: no building, the message "Aucun bâtiment militaire", nothing selected; F2: every
# economic building of the player selected (centre, houses, sites; not its ghosts), the camera where it was; F2 three
# times again: the centre, then the houses, the camera on each, the selection kept.
# The user's request of 2026-10-19, solo: a barracks' ghost (no builder) selected, its rally point by a right click (the
# flag); built at its first blow by a Peasant: the Homme d'armes it produces walks to the rally point kept since the ghost.
# The "anims" part (J5c.4), in solo: what the presentation shows, read from hol.Units.AnimReport (sequence and prop of
# each Peasant drawn, per state). Two Peasants build a house; three each chop wood, mine stone, mine gold, pick berries;
# three stand idle: each gesture with its tool (axe, pickaxe, nothing for the berries), the hammer, the plain idle. Then
# the four groups of gatherers walk away with their loads (the carrying walk of the load, the load shown), and stand
# there (the loaded idle, the load still shown). Every Peasant drawn, whatever it does, must show what its state calls for.
# J6b.1 (D312), solo: a lumber camp placed by the wood and built by two Peasants: once finished, they gather the nearest
# wood and unload at the camp (a gatherer nearby unloads at the centre until then); the centre's rally point on the camp:
# the Peasant born once it is finished gathers there too; a mining camp by the stone vein: its builders gather the stone;
# a mill by slot 7's berries (D346): its builders gather the berries and unload the food there;
# a camp 30 m from any wood: its builders stand idle.
# J6b.2 (D314), solo: six Peasants on round trips between the vein north-east of slot 0 and its centre, 50 m (their kept
# paths, D305); house K placed across their way and started: the kept paths across it dropped, those walking across it
# given a new path once its tiles are rebuilt (one log line), no body in a footprint after; six Peasants sent 90 m
# across open ground, house N started across their way: they go round it and arrive.
# J6b.2b (D324), solo: houses P and Q whose tiles touch, their bodies inset 115 cm in them (2.3 m apart): the navmesh's
# paths go through the way between them; one Peasant sent across passes through it, on its middle line; two sent together
# pass one after the other; none put back by a wall, none given a new path, all arrive.
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths), with no editor of the project open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1 [-Full | -Parts net,sources,move] [-EngineDir <dir>] [-Visible] [-GameFps <n> | -RealTime]
# (-File passes a list as one string: the script splits it at the commas.)
# The game renders off screen, with no window (the user works on the machine meanwhile); -Visible shows it on the
# second screen instead, windowed 1600 x 900, and the two clients side by side under it.
# The solo game runs on a fixed time step (Docs/Archive/TestSpeedInProgress.md): each frame moves the game on by 1/GameFps s, whatever
# the time it took, so the game's time runs ahead of the clock (the walks, sites and fights of the parts go by faster); the
# simulation keeps its own 20 Hz steps, the same as in real time, and the sequence's "wait" counts the game's seconds.
# 20 (the default): one simulation step per frame, as in a real game (never fewer frames than steps); -Parts build 306 s
# -> 72 s. 10 (two steps a frame) is faster but breaks "build cut" (house K: the navmesh's rebuild comes late in frames).
# -RealTime: the clock's time, as before. The net part's server and clients always run in real time.
# The engine is the project's (EngineAssociation of the .uproject, found in the registry), unless -EngineDir is given.
# Results: a PASS / FAIL line per part, captures in Saved_Tests\Screenshots\WindowsEditor\HOL (qc_*, cam_*, sel_*),
# contact sheets qc_<culture>.png (Python with Pillow), logs Saved_Tests\Logs\quick_game.log and quick_tests.log.
param(
    [string[]] $Parts = @("tests", "smoke"),
    [switch] $Full,
    [string] $EngineDir = "",
    [switch] $Visible,
    [double] $GameFps = 20,
    [switch] $RealTime,
    [string] $GameExec = "",
    [int] $DuelSeconds = 120
)
# -Full (D327): every part, the default before D327 (~530 s).
if ($Full) { $Parts = @("tests", "menu", "camera", "production", "move", "sources", "gather", "build", "anims", "formation", "combat", "victory", "sides", "sizes", "net", "aiserver", "netai") }
# A list passed by -File arrives as one string ("net,sources"): split it.
$Parts = @($Parts | ForEach-Object { $_ -split "," } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
# "hero" (J8a.6): the hero's checks alone, solo and 2 clients: the build part's hero cases (build:hero) and the net part
# cut to its hero phases (each client goes from its start to the hero sheets, the server spawns the heroes once both
# are there; the "net 2 clients" report keeps its join and server checks only). With "net" asked too, the net part whole.
$NetHeroOnly = ($Parts -contains "hero") -and -not ($Parts -contains "net")
if ($Parts -contains "hero") {
    $Parts = @($Parts | Where-Object { $_ -ne "hero" }) + @("net")
    if (-not ($Parts -contains "build")) { $Parts += "build:hero" }
}
# part:case (EtapeDefini F): the cases each part can play alone. Each case: the marks after "mark <part>" it plays (its
# section and the ones its report reads) and the set-up that stands for the cases skipped before it (the state it needs:
# wood, logs; then a selection, so that its Escape logs "Selection: none"). "build gap" reads "build wall" too (the
# walkers taken away, the houses' capture).
$SubCaseTable = @{
    build = [ordered]@{
        cut = @{ Marks = @("cut", "cut move"); Setup = @("hol.Cheat.Wood 1000", "hol.Gather.LogDeposits 1") }
        gap = @{ Marks = @("gap", "gap one", "gap two", "wall"); Setup = @("hol.Cheat.Wood 1000") }
        hero = @{ Marks = @("hero", "hero return", "hero texts", "hero xp", "hero aura", "hero grid"); Setup = @() }
        dropoff = @{ Marks = @("dropoff camp", "dropoff", "dropoff order"); Setup = @("hol.Cheat.Wood 1000", "hol.Gather.LogDeposits 1") }
        dropoffshift = @{ Marks = @("dropoff camp", "dropoffshift", "dropoffshift order"); Setup = @("hol.Cheat.Wood 1000", "hol.Gather.LogDeposits 1") }
        dropoffmove = @{ Marks = @("dropoff camp", "dropoffmove", "dropoffmove order"); Setup = @("hol.Cheat.Wood 1000", "hol.Gather.LogDeposits 1") }
        shiftwork = @{ Marks = @("shiftwork", "shiftwork order", "shiftwork site", "shiftwork after"); Setup = @("hol.Cheat.Wood 1000", "hol.Gather.LogDeposits 1") }
        shiftplace = @{ Marks = @("shiftplace", "shiftplace order"); Setup = @("hol.Cheat.Wood 1000") }
        shiftsite = @{ Marks = @("shiftsite", "shiftsite order", "shiftsite later"); Setup = @("hol.Cheat.Wood 1000") }
        rallysteps = @{ Marks = @("rallysteps", "rallysteps source", "rallysteps replace", "rallysteps nine"); Setup = @() }
        corner = @{ Marks = @("corner"); Setup = @("hol.Cheat.Wood 1000") }
        barwidth = @{ Marks = @("barwidth"); Setup = @() }
    }
    formation = [ordered]@{
        regroup = @{ Marks = @("regroup", "regroup end", "regroup spear", "regroup spear end", "regroup formed", "regroup formed end",
            "regroup formed spear", "regroup formed spear end", "regroup formed centre", "regroup formed centre end"); Setup = @() }
    }
    # The melee is off until "mark combat" (hol.Combat.Enabled 0 at the launch): a combat case alone turns it on.
    combat = [ordered]@{
        forget = @{ Marks = @("forget", "forget end"); Setup = @("hol.Combat.Enabled 1") }
    }
}
$SubCases = @{}
foreach ($Asked in @($Parts | Where-Object { $_ -match ":" })) {
    $Part, $Case = $Asked -split ":", 2
    if (-not $SubCaseTable.ContainsKey($Part)) { Write-Host "quick_check : FAIL (sub-cases not supported for $Part)"; exit 1 }
    if (-not $SubCaseTable[$Part].Contains($Case)) {
        Write-Host "quick_check : FAIL (unknown case '$Case' of $Part; cases: $(@($SubCaseTable[$Part].Keys) -join ', '))"; exit 1
    }
    if (-not $SubCases.ContainsKey($Part)) { $SubCases[$Part] = @() }
    $SubCases[$Part] += $Case
}
# A part asked whole as well as by case (build,build:cut) is played whole.
foreach ($Part in @($SubCases.Keys)) { if ($Parts -contains $Part) { $SubCases.Remove($Part) } }
$Parts = @($Parts | ForEach-Object { ($_ -split ":")[0] } | Select-Object -Unique)
# The solo parts run in the camera part's 2-player match: asked alone, the camera part comes with them.
if (@($Parts | Where-Object { $_ -in "production", "move", "bodies", "rocks", "sources", "gather", "build", "anims", "formation", "combat" }).Count -and -not ($Parts -contains "camera")) {
    $Parts += "camera"
}
Write-Host "parts: $($Parts -join ', ')"

# The army round the ridge (the part "rocks", the user's trials ridge_army): cases from every side of slot 0's ridge (three
# boulders welded together at world x -12000, y -1000 to +1000, a no-navmesh zone 1 m round them, create_match_map.py) to
# its other side, in world coordinates (slot 0's centre at -15000, 0; the veins at y +-4592 and the houses of "bodies"
# at x -9000 kept clear). Each: Name, Count, Type, spawn X, Y, Radius, target X, Y.
$RockCases = @(
    @{ Name = "front to back"; Count = 50; Type = "ManAtArms"; X = -13400; Y = 0; R = 600; TX = -10000; TY = 0 },
    @{ Name = "back to front"; Count = 50; Type = "ManAtArms"; X = -10500; Y = 0; R = 600; TX = -13700; TY = 0 },
    @{ Name = "back right to front left"; Count = 50; Type = "ManAtArms"; X = -10900; Y = 1900; R = 600; TX = -13300; TY = -700 },
    @{ Name = "back left to front right"; Count = 50; Type = "ManAtArms"; X = -10900; Y = -1900; R = 600; TX = -13300; TY = 700 },
    @{ Name = "front left to back right"; Count = 50; Type = "ManAtArms"; X = -13300; Y = -1500; R = 600; TX = -10300; TY = 1200 },
    @{ Name = "pressed to the rock"; Count = 30; Type = "ManAtArms"; X = -13000; Y = 200; R = 400; TX = -10600; TY = 300 },
    @{ Name = "spearmen front to back"; Count = 12; Type = "Spearman"; X = -13300; Y = 400; R = 300; TX = -10500; TY = -300 },
    @{ Name = "archers back to front"; Count = 20; Type = "Archer"; X = -10600; Y = -500; R = 400; TX = -13500; TY = 500 },
    @{ Name = "end to end"; Count = 50; Type = "ManAtArms"; X = -12900; Y = -2300; R = 600; TX = -11000; TY = 2200 },
    # The same army formed, ordered again and again (the user's 2nd trial of ridge_army: after the scenario's order, the
    # army sent back past the rock's end stood pressed against it): Chain, no spawn, the units standing round the last target.
    @{ Name = "army formed behind"; Count = 50; Type = "ManAtArms"; X = -13400; Y = 0; R = 600; TX = -10000; TY = 0 },
    @{ Name = "chain to front left"; Count = 50; Type = "ManAtArms"; Chain = $true; X = -10000; Y = 0; R = 1200; TX = -13300; TY = -700 },
    @{ Name = "chain to back right"; Count = 50; Type = "ManAtArms"; Chain = $true; X = -13300; Y = -700; R = 1200; TX = -10300; TY = 1500 },
    @{ Name = "chain to front right"; Count = 50; Type = "ManAtArms"; Chain = $true; X = -10300; Y = 1500; R = 1200; TX = -13300; TY = 800 },
    @{ Name = "chain to back left"; Count = 50; Type = "ManAtArms"; Chain = $true; X = -13300; Y = 800; R = 1200; TX = -10000; TY = -1200 },
    @{ Name = "chain to front"; Count = 50; Type = "ManAtArms"; Chain = $true; X = -10000; Y = -1200; R = 1200; TX = -13600; TY = 0 },
    # Through the grove at the map's middle (12 trunks on a disc of 14 m, create_match_map.py): Zone, where a unit off the
    # navmesh is in a trunk (the ridge's otherwise).
    @{ Name = "grove west to east"; Count = 50; Type = "ManAtArms"; X = -2600; Y = 0; R = 600; TX = 2600; TY = 0; Zone = @(-1700, 1700, -1700, 1700) },
    @{ Name = "grove south to north"; Count = 30; Type = "Spearman"; X = 300; Y = -2600; R = 500; TX = -300; TY = 2600; Zone = @(-1700, 1700, -1700, 1700) }
)

$Clock = [Diagnostics.Stopwatch]::StartNew()
$Project = Resolve-Path "$PSScriptRoot\..\.."
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
# Every editor or game launched here skips the engine's AutoSDK setup through UnrealBuildTool (-Mode=ValidatePlatforms,
# 2026-10-27): one at a time on the machine, it queued behind the other sessions' builds for up to 4.5 min, and the net
# part's server started after its clients had given up. No SDK is needed by these checks (no packaging here).
$env:UE_SKIP_UBT_SDK_SETUP = "1"
if (-not $EngineDir) {
    # An installed build registered under its name (HKCU\Software\Epic Games\Unreal Engine\Builds).
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $Root = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $Root) { Write-Host "quick_check : FAIL (engine '$Association' of the .uproject not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $Root "Engine\Binaries\Win64"
}
Write-Host "engine: $EngineDir"
$TestSaved = Join-Path $Project "Saved_Tests"
$Shots = Join-Path $TestSaved "Screenshots\WindowsEditor\HOL"
$GameLog = Join-Path $TestSaved "Logs\quick_game.log"
$TestsLog = Join-Path $TestSaved "Logs\quick_tests.log"
$Cultures = @("fr", "en", "ja")
$MenuScreens = @("title", "quit", "newgame", "video", "sound", "controls", "language")

# The test settings, once: windowed 1600 x 900 on the second screen.
$TestSettings = Join-Path $TestSaved "Config\WindowsEditor\GameUserSettings.ini"
if (-not (Test-Path $TestSettings)) {
    New-Item -ItemType Directory -Force (Split-Path $TestSettings) | Out-Null
    Copy-Item (Join-Path $PSScriptRoot "GameUserSettings.ini") $TestSettings
}
New-Item -ItemType Directory -Force (Split-Path $GameLog) | Out-Null
Remove-Item $GameLog, $TestsLog -ErrorAction SilentlyContinue

# 1. Automation tests, in the background (no window).
# IA1.1: the matches of quick_check make their own Peasants (none at the start, as the scenarios); the automation
# tests keep the rule of the game (5). J8a.2: nor their hero (bStartingHero, the same -ini).
$NoStartingPeasants = "-ini:Game:[/Script/HeroesOfLiberty.HOLMatchSettings]:StartingPeasants=0,[/Script/HeroesOfLiberty.HOLMatchSettings]:bStartingHero=False"
$TestsProcess = $null
if ($Parts -contains "tests") {
    $TestsArguments = "`"$UProject`" -ExecCmds=`"Automation RunTests HeroesOfLiberty+Angelscript.UnitTests;SoftQuit`" -saveddirsuffix=Tests -unattended -nullrhi -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -nosound -abslog=`"$TestsLog`""
    $TestsProcess = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor-Cmd.exe") -ArgumentList $TestsArguments -PassThru -WindowStyle Hidden
}

# 3. (started now, the clients join further down) Two-client match on a dedicated server (D273): the editor in dedicated
# server mode on Plain (D275: no cooking, always the code of the moment; the cooked server of net_match.ps1 stays for
# S5 and the user's trials), two humans in slots 0 and 1 at the colours of their slots.
$NetServerProcess = $null
$ManagerListener = $null
$NetServerLog = Join-Path $TestSaved "Logs\quick_net_server.log"
if ($Parts -contains "net") {
    $NetConfig = Join-Path $TestSaved "quick_net.json"
    # The slots named by the lobby manager (R3.2), in the reverse of the clients' order: client 1 (Bob, -PlayerName=) holds
    # slot 1, client 2 (Alice, ?Name= in the URL it opens) slot 0, whoever arrives first.
    $NetNames = @("Alice", "Bob")
    $NetSlots = for ($i = 0; $i -lt 8; $i++) {
        $NetSlotName = if ($i -lt 2) { ", `"playerName`": `"$($NetNames[$i])`"" } else { "" }
        "{ `"state`": `"$(if ($i -lt 2) { 'Human' } else { 'Closed' })`", `"colorIndex`": $i$NetSlotName }"
    }
    "{ `"version`": 2, `"map`": `"/Game/Maps/Plain.Plain`", `"slots`": [ $($NetSlots -join ', ') ] }" | Set-Content -Encoding ascii $NetConfig
    Remove-Item $NetServerLog, (Join-Path $TestSaved "Logs\quick_net_client*.log"), (Join-Path $TestSaved "quick_net_*.flag") -ErrorAction SilentlyContinue
    # Sources (J5a.2): 5 s after both clients joined, the server uses up the first tree of the wood beside slot 0.
    # Gathering (J5a.5): a second later, it leaves 5 wood in the tree nearest each start (the one each client's Peasant
    # will gather), so that the gathering uses it up within seconds; every unload logged. The starts are the main centres'
    # middles, (-14800, 0) and (15200, 0) (Get-SlotStart, on the 4 m grid), where hol.Select.OrderSourceNear looks: from
    # (15000, 0), the tree nearest was another one than slot 1's Peasant's (43, not 45), never used up.
    $NetSourceCommand = if ($Parts -contains "sources") {
        ",hol.Net.WhenClients 2 hol.Delay 5 hol.Sources.Take 0" +
        ",hol.Net.WhenClients 2 hol.Delay 6 hol.Sources.TakeNear -14800 0 95,hol.Net.WhenClients 2 hol.Delay 6 hol.Sources.TakeNear 15200 0 95" +
        ",hol.Gather.LogDeposits 1" +
        # No body in a footprint (J6a.1, D310): the Peasants' round trips to both centres watched at every step.
        ",hol.Net.WhenClients 2 hol.Nav.Footprints on,hol.Net.WhenClients 2 hol.Delay 30 hol.Nav.Footprints report,hol.Net.WhenClients 2 hol.Delay 45 hol.Nav.Footprints report"
    } else { "" }
    # The camps (J6b.3): built 4 times faster, every unload logged (at the camp).
    if ($Parts -contains "build") { $NetSourceCommand += ",hol.Build.Speed 4,hol.Gather.LogDeposits 1" }
    # The army (J7a.6): where the server has the units, every 5 s (the units stand still after their march).
    $NetSourceCommand += (@(for ($t = 20; $t -le 500; $t += 5) { ",hol.Net.WhenClients 2 hol.Delay $t hol.Units.Where" }) -join "")
    # The fight seen by a client (J7b.4): once client 1 frames the solo combat part's ground (its signal), the server makes
    # 2 Men-at-arms of slot 0 and 2 Spearmen of slot 1 there (6 m apart, in each other's aggression radius), then says
    # where every unit is each second (the client's positions compared with them); every hit logged with the health it
    # leaves (J7-Q5 revised: the client's health values compared with them).
    # The arrows (J7c.2b2): once client 1's fight window is closed (its second signal, its bodies counted), 3 Archers of
    # slot 0 and 2 Spearmen of slot 1, 40 m off the fight, shoot for 12 s, then all removed (the server's signal: each
    # client's arrow report once they have landed); no Units where meanwhile (the fight's comparisons stay on its own).
    $NetFightFlag = Join-Path $TestSaved "quick_net_fight1.flag"
    $NetArrowsFlag = Join-Path $TestSaved "quick_net_arrows1.flag"
    $NetArrowsEndFlag = Join-Path $TestSaved "quick_net_arrows_end.flag"
    $NetHeroWoundedFlag = Join-Path $TestSaved "quick_net_hero_wounded.flag"
    $NetHeroKilledFlag = Join-Path $TestSaved "quick_net_hero_killed.flag"
    $NetHeroReadyFlag = Join-Path $TestSaved "quick_net_hero_ready.flag"
    $NetHeroBackFlag = Join-Path $TestSaved "quick_net_hero_back.flag"
    $NetHeroBornFlag = Join-Path $TestSaved "quick_net_hero_born.flag"
    $NetHeroXPFlag = Join-Path $TestSaved "quick_net_hero_xp.flag"
    $NetHeroAbilitiesFlag = Join-Path $TestSaved "quick_net_hero_abilities.flag"
    $NetHeroCastFlag = Join-Path $TestSaved "quick_net_hero_cast.flag"
    $NetServerSequence = Join-Path $TestSaved "quick_net_server.seq"
    # A refused cheat (R3.4, R-Q7): the server, launched with -HOLCheats as every test server, refuses the clients' cheats
    # from the start (hol.Cheat.Refuse 1 in its -ExecCmds); client 1 asks for 500 wood, then signals; the server allows
    # them again for the rest (the army's resources) and answers.
    $NetCheatTriedFlag = Join-Path $TestSaved "quick_net_cheat_tried.flag"
    $NetCheatBackFlag = Join-Path $TestSaved "quick_net_cheat_back.flag"
    # The limited pause (D513, P5): Bob (client 1) pauses; Alice (client 2) asks to resume, refused while the timer runs;
    # Bob asks a second pause, refused (already paused); the server reports twice 1 s apart (paused by slot 1, 2 left, the
    # step unchanged); Bob resumes at once (his own pause); the server reports twice again (running, stepping on).
    $NetPausedFlag = Join-Path $TestSaved "quick_net_paused.flag"
    $NetAliceResumeTriedFlag = Join-Path $TestSaved "quick_net_alice_resume_tried.flag"
    $NetPauseCheckFlag = Join-Path $TestSaved "quick_net_pause_check.flag"
    $NetPauseCheckedFlag = Join-Path $TestSaved "quick_net_pause_checked.flag"
    $NetResumedFlag = Join-Path $TestSaved "quick_net_resumed.flag"
    $NetKingAskFlag = Join-Path $TestSaved "quick_net_king_ask.flag"
    $NetKingBornFlag = Join-Path $TestSaved "quick_net_king_born.flag"
    $NetKingKillFlag = Join-Path $TestSaved "quick_net_king_kill.flag"
    # The refused pause (D361): client 1 has typed "pause" in its console before its signal; the server must not be
    # paused and its simulation must step on (two reports 1 s apart).
    # "hero" alone (J8a.6): the clients' cheats allowed at once (Bob's gold), the heroes made once both clients are done
    # with their start (their "done" signals).
    $(if ($NetHeroOnly) { @("hol.Cheat.Refuse 0", "waitfile $(Join-Path $TestSaved "quick_net_done1.flag") 300", "waitfile $(Join-Path $TestSaved "quick_net_done2.flag") 300") }
    else { @("waitfile $NetCheatTriedFlag 300", "hol.Match.PauseReport", "wait 1", "hol.Match.PauseReport", "hol.Cheat.Refuse 0", "signal $NetCheatBackFlag",
        "waitfile $NetPauseCheckFlag 120", "hol.Match.PauseReport", "wait 1", "hol.Match.PauseReport", "signal $NetPauseCheckedFlag",
        "waitfile $NetResumedFlag 60", "hol.Match.PauseReport", "wait 1", "hol.Match.PauseReport", "waitfile $NetFightFlag 900", "mark net fight", "hol.Combat.LogHits 1", "hol.Units.Spawn 2 -6300 -5500 0 ManAtArms 150",
        "hol.Units.Spawn 2 -5700 -5500 1 Spearman 150") + @(for ($i = 0; $i -lt 30; $i++) { "wait 1"; "hol.Units.Where" }) +
        @("waitfile $NetArrowsFlag 300", "mark net arrows", "hol.Units.Spawn 3 -2000 -5000 0 Archer 150", "hol.Units.Spawn 2 -2000 -3600 1 Spearman 150", "wait 12",
        "hol.Units.Remove -2000 -4300 1500 1", "hol.Units.Remove -2000 -4300 1500 0", "mark net arrows end", "signal $NetArrowsEndFlag", "hol.Units.CorpseReport") }) +
        # The hero sheets (J8a.3, J8-Q5): a hero for each client's slot (Alice slot 0, Bob slot 1, R3.2), born here (none at
        # the start: bStartingHero off), 40 m apart; their XP set (5, 7); 2 Men-at-arms of slot 0 strike Bob's hero for 10 s,
        # then removed: once still, the server's report, then each client's (its signal); then Bob's hero killed, the
        # server's report again, then each client's.
        # J8a.5 loop 6: once born (their XP set), the server waits for each client's hero block (its signal) before the strike.
        # The return (J8a.4 loop 5), its times at 20 % (hol.Hero.ReturnTimePercent: ready 152 steps after death, the price
        # from 150 to 0 in 240 steps): once Bob's hero is ready (Bob's too early order refused meanwhile), Alice's killed,
        # the clients' signal; Bob's paid return then Alice's by itself at price 0 (the server's own lines), its report,
        # the clients' signal; both heroes removed at the end, the times back to the rules (the next parts as before).
        @("mark net hero", "hol.Hero.ReturnTimePercent 20", "hol.Units.Spawn 1 2000 -5000 1 Hero 150", "hol.Units.Spawn 1 6000 -5000 0 Hero 150", "wait 1",
            "hol.Hero.SetXP 0 5", "hol.Hero.SetXP 1 7", "signal $NetHeroBornFlag",
            "waitfile $(Join-Path $TestSaved "quick_net_hero_born1.flag") 180", "waitfile $(Join-Path $TestSaved "quick_net_hero_born2.flag") 180",
            "hol.Units.Spawn 2 2600 -5000 0 ManAtArms 150", "wait 10",
            "hol.Units.Remove 2300 -5000 2000 0 ManAtArms", "wait 2", "hol.Hero.Report", "signal $NetHeroWoundedFlag",
            "waitfile $(Join-Path $TestSaved "quick_net_hero_seen1.flag") 120", "waitfile $(Join-Path $TestSaved "quick_net_hero_seen2.flag") 120",
            "hol.Hero.Kill 1", "wait 2", "hol.Hero.Report", "signal $NetHeroKilledFlag",
            "waitfile $(Join-Path $TestSaved "quick_net_hero_done1.flag") 120", "waitfile $(Join-Path $TestSaved "quick_net_hero_done2.flag") 120",
            "waitlog Hero return: slot 1 ready", "mark net hero return", "hol.Hero.Kill 0", "signal $NetHeroReadyFlag",
            "waitlog Hero return: slot 1 by its main forum", "waitlog Hero return: slot 0 by its main forum", "wait 1", "hol.Hero.Report",
            "signal $NetHeroBackFlag", "waitfile $(Join-Path $TestSaved "quick_net_hero_back1.flag") 120", "waitfile $(Join-Path $TestSaved "quick_net_hero_back2.flag") 120",
            # J8b.3: both back, Bob's XP to 220 (level 2: his body at 486 / 486); a Man-at-arms of slot 0 made by him (back
            # by his main forum, ~(14065, 299)) for 3 s, then removed: wounded, the clients show his health of 486; the
            # server's report, the clients' signal.
            "mark net hero xp", "hol.Hero.SetXP 1 220", "wait 1", "hol.Units.Spawn 1 14300 300 0 ManAtArms 10", "wait 3",
            "hol.Units.Remove 14200 300 2000 0 ManAtArms", "wait 2", "hol.Hero.Report", "signal $NetHeroXPFlag",
            "waitfile $(Join-Path $TestSaved "quick_net_hero_xp1.flag") 120", "waitfile $(Join-Path $TestSaved "quick_net_hero_xp2.flag") 120",
            # J8c.3: Bob's XP to 1 300 (level 4: the banner open); Bob casts his cry twice (the second refused, its cool-down)
            # and plants his banner by his forum (hol.Hero.Cast); the server's report once he has, the clients' signal; his
            # abilities reset after.
            "mark net hero abilities", "hol.Hero.SetXP 1 1300", "wait 1", "signal $NetHeroAbilitiesFlag",
            "waitfile $(Join-Path $TestSaved "quick_net_hero_cast1.flag") 120", "wait 1", "hol.Hero.Report", "signal $NetHeroCastFlag",
            "waitfile $(Join-Path $TestSaved "quick_net_hero_abilities1.flag") 120", "waitfile $(Join-Path $TestSaved "quick_net_hero_abilities2.flag") 120",
            "hol.Hero.ResetAbilities 1",
            "hol.Hero.ReturnTimePercent 100", "hol.Units.Remove 0 0 60000 0 Hero", "hol.Units.Remove 0 0 60000 1 Hero") +
        # V5.5a (Regicide), with "victory": once client 1 asks (its forum fallen and held, V2.5-V4.5), a King for each slot
        # (client 2 Alice in slot 0, client 1 Bob in slot 1, R3.2), then, once both clients have shown them, client 1's King
        # (slot 1) killed: its defeat (King) ends the match in place of V1.4c's surrender.
        @(if ($Parts -contains "victory") { "waitfile $NetKingAskFlag 900", "hol.Victory.SpawnKing 0", "hol.Victory.SpawnKing 1", "wait 1", "signal $NetKingBornFlag",
            "waitfile $NetKingKillFlag 120", "hol.Victory.KillKing 1" }) |
        Set-Content -Encoding utf8 $NetServerSequence
    $NetSourceCommand += ",hol.Test.Run $NetServerSequence"
    # -useperfthreads (J7c.4): a dedicated server, the editor's -server too, runs every ParallelFor on one thread by the
    # engine's default (FApp::ShouldUseThreadingForPerformance); the parallel processors of D302 run as in the game.
    # A free UDP port in a range of 10 of this copy's own (M6): copies probing from the same 7777 at the same moment
    # took the same port, the engine moved one server to 7778 and its clients joined the other copy's server (seen
    # 2026-10-27, Glorfindel and the main copy). 7800-7899: clear of the lobby manager's servers (7777-7799) and of
    # lobby_match's (7900 and up).
    $CopyIndex = @{ "MyCrusader" = 0; "HeroesOfLiberty-Elrond" = 1; "HeroesOfLiberty-Arwen" = 2; "HeroesOfLiberty-Glorfindel" = 3;
        "HeroesOfLiberty-Haldir" = 4; "HeroesOfLiberty-Galadriel" = 5; "HeroesOfLiberty-Celeborn" = 6 }[(Split-Path -Leaf $Project)]
    if ($null -eq $CopyIndex) { $CopyIndex = 7; foreach ($Char in (Split-Path -Leaf $Project).ToCharArray()) { $CopyIndex = 7 + ($CopyIndex + [int]$Char) % 3 } }
    $NetPortFirst = 7800 + 10 * $CopyIndex
    $NetPort = $NetPortFirst
    while ($NetPort -lt $NetPortFirst + 10) {
        try { $Probe = New-Object Net.Sockets.UdpClient($NetPort); $Probe.Close(); break } catch { $NetPort++ }
    }
    # The lobby manager (R3.6, R3.7): a test listener on a free TCP port of 127.0.0.1 answers 204 to every call and writes
    # each one (method, path, body) in a file; the server gets its address and a match id, as from the manager.
    $ManagerCallsFile = Join-Path $TestSaved "quick_net_manager.txt"
    Remove-Item $ManagerCallsFile -ErrorAction SilentlyContinue
    $ManagerProbe = New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback, 0)
    $ManagerProbe.Start(); $ManagerPort = $ManagerProbe.LocalEndpoint.Port; $ManagerProbe.Stop()
    $ManagerListener = New-Object Net.HttpListener
    $ManagerListener.Prefixes.Add("http://127.0.0.1:${ManagerPort}/")
    $ManagerListener.Start()
    $ManagerRunspace = [PowerShell]::Create()
    $null = $ManagerRunspace.AddScript({
        param($Listener, $File)
        while ($Listener.IsListening) {
            try { $Context = $Listener.GetContext() } catch { break }
            $Body = (New-Object IO.StreamReader($Context.Request.InputStream)).ReadToEnd()
            Add-Content -Path $File -Encoding utf8 -Value "$($Context.Request.HttpMethod) $($Context.Request.Url.AbsolutePath) $Body"
            $Context.Response.StatusCode = 204
            $Context.Response.Close()
        }
    }).AddArgument($ManagerListener).AddArgument($ManagerCallsFile)
    $null = $ManagerRunspace.BeginInvoke()
    $NetServerArguments = "`"$UProject`" /Game/Maps/Plain -server -Port=$NetPort -ManagerUrl=http://127.0.0.1:${ManagerPort} -MatchId=QC1 -useperfthreads -unattended -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -nosound -saveddirsuffix=Tests_NetSrv -HOLCheats $NoStartingPeasants -MatchConfig=`"$NetConfig`" -abslog=`"$NetServerLog`" -ExecCmds=`"hol.Prod.BuildTime 2,hol.Cheat.Refuse 1,hol.Match.StartWaitSeconds 120$NetSourceCommand`""
    $NetServerProcess = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $NetServerArguments -PassThru -WindowStyle Hidden
}

# 2. One game launch for the menu and the camera.
$Steps = New-Object System.Collections.Generic.List[string]
# Inside a part played by cases (Start-CaseFilter): each "mark <part> <name>" starts a case, kept if it is one of the
# marks asked, else skipped with every step and key up to the next mark; the first kept after a skip gets its set-up.
$CaseFilter = $null
function Start-CaseFilter([string] $Part) {
    if (-not $SubCases.ContainsKey($Part)) { return }
    $Marks = @{}
    foreach ($Case in $SubCases[$Part]) { foreach ($Mark in $SubCaseTable[$Part][$Case].Marks) { $Marks["$Part $Mark"] = $SubCaseTable[$Part][$Case].Setup } }
    $script:CaseFilter = @{ Part = $Part; Marks = $Marks; Skipping = $true }
}
function Stop-CaseFilter { $script:CaseFilter = $null }
function Step([string[]] $Lines) {
    foreach ($Line in $Lines) {
        $Filter = $script:CaseFilter
        if ($Filter -and $Line -match "^mark ($([regex]::Escape($Filter.Part))(?: .*)?)$") {
            $Kept = $Filter.Marks.ContainsKey($Matches[1])
            if ($Kept -and $Filter.Skipping) {
                # The set-up, then the centre selected and Escape: the case's "waitlog Selection: none" finds its line.
                $Filter.Marks[$Matches[1]] + @("hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)",
                    "hol.UI.Key Escape 0 0.1", "wait 0.15") | ForEach-Object { $Steps.Add($_) }
            }
            $Filter.Skipping = -not $Kept
        }
        if (-not ($Filter -and $Filter.Skipping)) { $Steps.Add($Line) }
    }
}
# A key press and its release (a tenth of a second).
function Key([string] $Key, [string] $Hold = "0.1") { Step "hol.UI.Key $Key 0 $Hold", "wait $([double]$Hold + 0.05)" }
# A menu screen opened by the same path as its button, once it has finished fading in.
function Open([string] $Screen) { Step "hol.UI.Open $Screen", "waitui" }
# Back, once the screen below is shown again.
function Back { Key Escape; Step "waitui" }

Step "waitmap MainMenu", "waitui"
if ($Parts -contains "menu") {
    foreach ($Culture in $Cultures) {
        Step "mark menu $Culture", "hol.UI.Language $Culture", "waitui", "capture qc_${Culture}_title"
        Open Quit; Step "capture qc_${Culture}_quit"; Back
        Open NewGame; Step "capture qc_${Culture}_newgame"
        if ($Culture -eq "fr") {
            # D266: the colour list opened and a value taken with the keyboard (the keyboard first, see the journal).
            Key Up
            Step "hol.UI.Focus SlotColor0", "frames 2"; Key Enter
            Step "waitlog Dropdown SlotColor0: list opened", "frames 3", "capture qc_fr_dropdown"
            Key Down; Key Enter
            Step "waitlog Dropdown SlotColor0: value 1 chosen"
            # E2-4: a line nobody holds (slot 3, closed) shows no faction, swatch, colour nor team; taken (Human), they come back.
            Step "mark menu emptyrow", "hol.UI.Shown SlotFaction3 SlotSwatch3 SlotColor3 SlotTeam3 SlotFaction0 SlotSwatch0 SlotColor0 SlotTeam0",
                "capture qc_fr_emptyrow", "mark menu takenrow", "hol.UI.Choose SizeDropdown 2", "frames 3", "hol.UI.Choose SlotState3 1", "frames 3",
                "hol.UI.Shown SlotFaction3 SlotSwatch3 SlotColor3 SlotTeam3", "capture qc_fr_takenrow", "hol.UI.Choose SlotState3 0", "frames 3"
            # TC3: the size under the map (Small by default for one player, opened above), "Équipes ensemble" beside it;
            # three players on Medium: Small greyed (not chosen), slot 5 past the 4 places stays closed; back to Small.
            # TC-Q12: in solo the teams can be chosen (the factions stay greyed), "Équipes ensemble" too.
            Step "mark menu sizes", "hol.UI.Shown SizeDropdown TeamsTogetherDropdown SlotTeam0 SlotFaction0", "hol.UI.Choose SlotTeam0 1", "frames 2",
                "hol.UI.Choose SlotTeam0 0", "hol.UI.Choose SlotState1 1", "hol.UI.Choose SlotState2 1",
                "frames 3", "hol.UI.Choose SizeDropdown 1", "hol.UI.Choose SlotState5 1", "frames 3", "capture qc_fr_sizes",
                "hol.UI.Choose SlotState1 0", "hol.UI.Choose SlotState2 0", "frames 3", "hol.UI.Choose SizeDropdown 1", "frames 3"
            # IA0.6 S1: in solo "IA" is open in the state list: slot 1 taken by an AI shows its colour and team; closed again.
            # IA0.6 S2 (Q1): one "Joueur" column, the list shows the name the slot will have (the game's, GetAIName: "IA Normal 2"
            # for slot index 1); its level after it, Normal and greyed, on an AI line only (none on the closed slot 2).
            Step "mark menu ai", "hol.UI.Choose SlotState1 2", "frames 3", "hol.UI.Shown SlotColor1 SlotTeam1", "capture qc_fr_ai",
                "hol.UI.Choose SlotState2 0", "hol.UI.Text SlotState1 SlotLevel1", "hol.UI.Shown SlotLevel1 SlotName1 SlotLevel2",
                "hol.UI.Choose SlotState1 0", "frames 3"
        }
        Back
        if ($Culture -eq "fr") {
            # E2-3: a screen pushed while the one before is still fading in (the menu's BeginPlay back from a lobby's
            # match: title, Multiplayer, then the lobby), then both closed: the title again, not an empty layer.
            Step "mark menu stack", "hol.UI.Open Multiplayer", "frames 2", "hol.UI.Open NewGame", "waitui"
            Back; Back
            Step "capture qc_fr_stackback"
        }
        for ($Tab = 0; $Tab -lt 4; $Tab++) {
            Open "Settings:$Tab"; Step "capture qc_${Culture}_$($MenuScreens[3 + $Tab])"
        }
        Back
    }
    Step "hol.UI.Language fr", "waitui"
    # IA0.6 S3: from the menu to the match, no file: "IA" chosen in the list of slot 1, Launch (what the screen shows);
    # the brain thinks for slot 1 and sends its 5 Peasants to gather. quick_check runs without starting Peasants
    # ($NoStartingPeasants, the game's rule is 5): the AI's 5 are made once its centre is down. Back to the main menu.
    Open NewGame
    Step "mark menu ailaunch", "hol.UI.Choose SlotState1 2", "frames 3", "hol.UI.NewGame.Launch - 0", "waitlog Main centre of slot 1",
        "hol.Units.Spawn 5 0 0 1 Peasant", "waitlog AI slot 1 step",
        "open /Game/Maps/MainMenu", "waitmap MainMenu", "waitui"
}

# The "victory" part (V1.4a, D356-D358), in solo on Plain, a human (slot 0) against an AI (slot 1): the surrender by
# its real path (Escape with nothing selected, the menu, "Abandonner", "Oui"), the end banner (Defeat), the menu opened
# over it then closed by Escape, Escape again closes the banner (V1.4a: the menu input kept under the closing menu);
# an order of the fallen player refused by the server (D358); then a second match where the AI is defeated (hol.Victory.Defeat 1): Victory, "Quitter vers le menu", the menu again.
if ($Parts -contains "victory") {
    $VictoryConfig = Join-Path $TestSaved "victory_1v1.json"
    '{ "map": "/Game/Maps/Plain.Plain", "slots": [ { "state": "Human", "colorIndex": 0 }, { "state": "AI", "colorIndex": 1 } ] }' | Set-Content -Encoding ascii $VictoryConfig
    Step "mark victory surrender", "hol.UI.NewGame.Launch $VictoryConfig 0", "waitmap Plain", "frames 60"
    Key Escape
    Step "waitlog Game menu: opened", "frames 5", "hol.UI.Focus SurrenderButton", "frames 3"
    Key Enter
    Step "waitui", "frames 5", "hol.UI.Focus ConfirmButton", "frames 3"
    Key Enter
    Step "waitlog End banner: Defeat", "waitui", "frames 10"
    # D358: an order of the fallen player (a foundation asked by the server's path, its own centre gone) is refused.
    Step "mark victory order", "hol.Build.PlaceNear House -6000 0 enemy", "waitlog Victory: orders of defeated slot 0 refused", "frames 5"
    Step "mark victory escape", "hol.UI.Open GameMenu", "waitui", "frames 5"
    Key Escape
    Step "waitlog Game menu: closed", "waitui", "frames 10"
    Key Escape
    Step "waitlog End banner: closed", "waitui", "frames 10"
    # C5: a sample every 2 s in this short match (~35 s), so that the timeline's curves have points enough.
    Step "mark victory win", "open /Game/Maps/MainMenu", "waitmap MainMenu", "waitui", "hol.Stats.SampleSeconds 2", "hol.UI.NewGame.Launch $VictoryConfig 0", "waitmap Plain", "frames 60"
    # C4c: something to count before the end: a Peasant produced (its rally point on the berries, where it gathers until
    # a first deposit), then 3 Men-at-arms of slot 0 against 3 Spearmen of slot 1 (the combat part's ground), fought out.
    Step "hol.Prod.BuildTime 2", "hol.Gather.LogPopups 1", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5",
        "waitlog Selection: MainCentre of slot 0 (own)", "hol.Camera.Focus -15000 1600", "frames 3",
        "hol.Select.OrderSource -15000 2000", "waitlog Rally: slot 0 set the rally point of MainCentre", "hol.Prod.Queue",
        "waitlog Production: Peasant of slot 0 done", "waitlog Deposit popup: +",
        "hol.Units.SpawnSeed 122", "hol.Combat.Enabled 1", "hol.Units.Spawn 3 -6300 -5500 0 ManAtArms 150", "hol.Units.Spawn 3 -5700 -5500 1 Spearman 150",
        "wait 15", "hol.Combat.Report", "hol.Combat.Enabled 0", "hol.Stats.Dump"
    Step "hol.Victory.Defeat 1", "waitlog End banner: Victory", "waitui", "frames 5", "hol.Stats.Dump"
    # C4: the statistics screen from the banner's « Statistiques », its tabs captured, Back to the banner.
    Step "mark victory stats", "hol.UI.Focus StatsButton", "frames 3"
    Key Enter
    Step "waitlog Stats screen: row", "waitui", "frames 5", "capture qc_stats_summary", "hol.UI.Focus MilitaryTab", "frames 3"
    Key Enter
    Step "frames 5", "capture qc_stats_military", "hol.UI.Focus EconomyTab", "frames 3"
    Key Enter
    Step "frames 5", "capture qc_stats_economy", "hol.UI.Focus TimelineTab", "frames 3"
    Key Enter
    # C5: the timeline, the workers' curves (its first series) then the army's, the defeat of slot 1 marked.
    Step "waitlog Stats screen: tab 3", "frames 5", "capture qc_stats_curve_workers", "hol.UI.Focus SeriesMilitary", "frames 3"
    Key Enter
    Step "waitlog Stats screen: timeline Military", "frames 5", "capture qc_stats_curve_army", "hol.Stats.SampleSeconds 10"
    Back
    Step "waitlog Stats screen: closed", "frames 5", "hol.UI.Focus QuitButton", "frames 3"
    Key Enter
    Step "waitlog End banner: quit to menu", "waitmap MainMenu", "waitui", "mark victory end"
    # V2.5 (D349-D353), a third match: the main forum's lives. Slot 0's forum falls (2 lives -> 1, in ruin): the table and
    # its selection block say so, a production on it is refused by the server; 2 Peasants sent on it (a right click on
    # the centre), its ruin skipped: rebuilding, its builders at work; skipped again: rebuilt; it falls again,
    # its last life: defeated (Forum), the banner Defeat, the table's row defeated. Back to the menu.
    Step "mark victory forum fall", "hol.UI.NewGame.Launch $VictoryConfig 0", "waitmap Plain", "frames 60",
        "hol.Victory.FallForum 0", "waitlog Victory: forum of slot 0 fell", "frames 3",
        "hol.Prod.Queue", "waitlog Production order of slot 0 ignored: its forum is down", "frames 3", "hol.HUD.Report"
    # V4.5 (D353, D202): the message of its fall with the major alert (banner 4 s, the thread's line 8 s); a click on the
    # banner: the camera above its forum; after 4.5 s no alert, after 9 s the thread empty.
    Step "mark victory forum alert", "hol.HUD.Report", "hol.HUD.Click AlertPanel", "waitlog Camera above the forum of slot 0", "frames 3",
        "wait 4.5", "hol.HUD.Report", "wait 4.5", "hol.HUD.Report"
    # V3.5 (D348), the hold by the real presence (the units alive 12 m from the forum's edge; Plain's slot 0 centre at
    # -14800 0, 16 m wide): 3 Spearmen of slot 1 made 5 m from its edge: held by slot 1; a Peasant of mine in the zone:
    # contested; the Spearmen taken away: released, the count recedes, no longer held; my Peasant taken away after.
    Step "mark victory forum held", "hol.Units.Spawn 3 -14800 1300 1 Spearman 150", "waitlog Victory: forum of slot 0 held by slot 1", "wait 3", "hol.HUD.Report"
    Step "mark victory forum contested", "hol.Units.Spawn 1 -14800 -1300 0 Peasant 0", "waitlog Victory: forum of slot 0 contested", "frames 3", "hol.HUD.Report"
    # 0.6 s at ForumHoldDecay 2 s a second: the shown whole seconds always drop by 1 (0.4 s did only when the fraction allowed).
    Step "mark victory forum released", "hol.Units.Remove -14800 1300 800 1", "waitlog Victory: forum of slot 0 released", "wait 0.6", "hol.HUD.Report",
        "waitlog Victory: forum of slot 0 no longer held", "frames 3", "hol.HUD.Report", "hol.Units.Remove -14800 -1300 800 0", "frames 3", "mark victory forum hold end"
    # The match starts with no unit: 2 Peasants of slot 0 made 20 m beside its centre (Plain's slot 0, -14800 0).
    Step "mark victory forum build", "hol.Units.Spawn 2 -14800 2000 0 Peasant 150", "waitunits 2", "frames 3", "hol.Select.Own", "waitlog Select own: 2 units", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.OrderNear 0 0", "wait 1",
        "hol.Victory.SkipRuin 0", "waitlog Victory: forum of slot 0 ruin -> rebuilding", "waitlog Construction: slot 0 rebuilds its forum",
        # The units unselected by hol.Prod.Queue (the centre selected first, its order refused again): the block read.
        "frames 3", "hol.Select.Report", "hol.Prod.Queue", "frames 3", "hol.HUD.Report"
    Step "mark victory forum rebuilt", "hol.Victory.SkipRuin 0", "waitlog Victory: forum of slot 0 rebuilt", "frames 3", "hol.HUD.Report"
    # V4.5: the AI's forum falls: the message alone for slot 0 (another camp), no alert.
    Step "mark victory forum other", "hol.Victory.FallForum 1", "waitlog Victory: forum of slot 1 fell", "frames 3", "hol.HUD.Report"
    Step "mark victory forum defeat", "hol.Victory.FallForum 0", "waitlog End banner: Defeat", "waitui", "frames 3", "hol.HUD.Report",
        "open /Game/Maps/MainMenu", "waitmap MainMenu", "waitui", "mark victory forum end"
    # V5.5a (Regicide, D206), a fourth match with the option (bRegicide in the configuration): a King born for each slot
    # (Plain's slot 0 centre at -14800 0, its King 4.5 m beyond its west edge, about -16050 0). Selected (Ctrl + A: it is
    # no Peasant): its block, its grid the stop alone, the population without it. Combat on: an enemy Peasant 5.5 m off
    # (no attack of its own): the King does not attack; an enemy Man-at-arms there (it attacks him): the King ripostes.
    # Its aggressor removed (combat turned off at once): no longer Attacking (EtapeDefini I). Sent on an enemy House (right
    # click): refused, the log saying why (no building attack, not "not its own", EtapeDefini I).
    # Then the AI's King killed (hol.Victory.KillKing 1): its death, the message for another camp (no alert), its
    # defeat (King), the banner Victory.
    $RegicideConfig = Join-Path $TestSaved "victory_regicide.json"
    '{ "map": "/Game/Maps/Plain.Plain", "bRegicide": true, "slots": [ { "state": "Human", "colorIndex": 0 }, { "state": "AI", "colorIndex": 1, "aiBrain": false } ] }' | Set-Content -Encoding ascii $RegicideConfig
    Step "mark victory regicide", "hol.UI.NewGame.Launch $RegicideConfig 0", "waitmap Plain", "frames 60", "hol.Select.Army", "frames 3", "hol.HUD.Report"
    Step "mark victory regicide pass", "hol.Combat.LogHits 1", "hol.Combat.Enabled 1", "hol.Units.Spawn 1 -16600 0 1 Peasant 0", "wait 4", "hol.Units.Remove -16600 0 1500 1", "frames 3"
    Step "mark victory regicide riposte", "hol.Units.Spawn 1 -16600 0 1 ManAtArms 0", "wait 5", "hol.Units.Remove -16600 0 1500 1", "hol.Combat.Enabled 0",
        "hol.Combat.LogHits 0", "frames 3", "hol.Select.Army", "frames 3", "hol.HUD.Report"
    Step "mark victory regicide refusal", "hol.Cheat.SpawnBuilding House 1 -1800 -1600", "waitlog Cheat: spawned House of slot 1", "hol.Camera.FocusNear -1800 -1600",
        "frames 5", "hol.Select.Army", "frames 3", "hol.Select.OrderNear -1800 -1600", "wait 1"
    Step "mark victory regicide death", "hol.Victory.KillKing 1", "waitlog End banner: Victory", "waitui", "frames 3", "hol.HUD.Report",
        "open /Game/Maps/MainMenu", "waitmap MainMenu", "waitui", "mark victory regicide end"
}

# The "sides" part (EtapeDefini D.3), in solo on Plain, three players: me (slot 0, team 1), an enemy AI (slot 1, team
# 2), an allied AI (slot 2, team 1). Groups in front of my centre (combat off, as the whole launch): my 4 Men-at-arms,
# slot 1's 5 Spearmen and 3 Archers on one side, slot 2's 3 Spearmen and 2 Archers on the other. The eight cases of the
# priority Own > Ally > Enemy, each judged by "Select: box, N units (side)", hol.Select.Report, hol.Units.Bars and
# hol.HUD.Report: a box on enemies, on allies (their panel "other" without commands, their full bars shown), on mine and
# enemies, on mine and allies, on allies and enemies (the allies alone), Shift + click on an enemy with allies selected,
# then on an enemy and an ally with mine selected, a double click on an ally and on an enemy (its owner's units of
# its type on the screen).
$SidesMine = @(-10800, 0); $SidesEnemy = @(-8000, -1900); $SidesEnemyArchers = @(-7400, -1900)
$SidesAlly = @(-8000, 1900); $SidesAllyArchers = @(-7400, 1900)
if ($Parts -contains "sides") {
    $SidesConfig = Join-Path $TestSaved "sides_3p.json"
    '{ "map": "/Game/Maps/Plain.Plain", "slots": [ { "state": "Human", "colorIndex": 0, "team": 1 }, { "state": "AI", "colorIndex": 1, "team": 2 }, { "state": "AI", "colorIndex": 2, "team": 1 } ] }' |
        Set-Content -Encoding ascii $SidesConfig
    # A ground box around the points (cm), and the reports after it.
    function SidesBox([double] $X1, [double] $Y1, [double] $X2, [double] $Y2, [string] $Add = "") { Step "hol.Select.BoxGround $X1 $Y1 $X2 $Y2 $Add".Trim() }
    function SidesReport { Step "frames 3", "hol.Select.Report", "hol.Units.Bars", "hol.HUD.Report", "frames 2" }
    $EnemyBox = @(-8700, -2600, -6900, -1100); $AllyBox = @(-8700, 1100, -6900, 2600); $MineBox = @(-11300, -500, -10300, 500)
    Step "mark sides start", "hol.Camera.TestCursor 0.5 0.5", "hol.UI.NewGame.Launch $SidesConfig 0", "waitmap Plain", "frames 30",
        "hol.Units.Spawn 4 $($SidesMine -join ' ') 0 ManAtArms 250", "hol.Units.Spawn 5 $($SidesEnemy -join ' ') 1 Spearman 350",
        "hol.Units.Spawn 3 $($SidesEnemyArchers -join ' ') 1 Archer 250", "hol.Units.Spawn 3 $($SidesAlly -join ' ') 2 Spearman 250",
        "hol.Units.Spawn 2 $($SidesAllyArchers -join ' ') 2 Archer 200", "waitunits 17", "hol.Camera.Focus -9000 0", "hol.Camera.Zoom -2", "frames 10",
        "capture qc_sides_view"
    # Each case from nothing selected (a click on the void): its box logged even when it takes the case before's units.
    function SidesCase([string] $Name) { Step "mark sides $Name", "hol.Select.Click 0.03 0.05", "frames 2" }
    SidesCase "1"; SidesBox $EnemyBox[0] $EnemyBox[1] $EnemyBox[2] $EnemyBox[3]; SidesReport
    SidesCase "2"; SidesBox $AllyBox[0] $AllyBox[1] $AllyBox[2] $AllyBox[3]; SidesReport
    SidesCase "3"; SidesBox $MineBox[0] $EnemyBox[1] $EnemyBox[2] $MineBox[3]; SidesReport
    SidesCase "4"; SidesBox $MineBox[0] $MineBox[1] $AllyBox[2] $AllyBox[3]; SidesReport
    SidesCase "5"; SidesBox $EnemyBox[0] $EnemyBox[1] $AllyBox[2] $AllyBox[3]; SidesReport
    SidesCase "6"; SidesBox $AllyBox[0] $AllyBox[1] $AllyBox[2] $AllyBox[3]; Step "hol.Select.ClickUnit 0 add slot=1"; SidesReport
    SidesCase "7"; SidesBox $MineBox[0] $MineBox[1] $MineBox[2] $MineBox[3]; Step "hol.Select.ClickUnit 0 add slot=1"; SidesReport
    Step "hol.Select.ClickUnit 0 add slot=2"; SidesReport
    SidesCase "8 ally"; Step "hol.Select.ClickUnit 0 double slot=2"; SidesReport
    SidesCase "8 enemy"; Step "hol.Select.ClickUnit 0 double slot=1"; SidesReport
    # Allied combat (bug: the camps were ignored): the groups above removed, my 4 Men-at-arms and the ally's 4 Spearmen
    # side by side before my centre, combat on: nobody fights in Auto, on Hold (mine), on an attack-move of mine onto
    # the ally's group (each judged by hol.Combat.Report: no hit struck, every body full, no target of the same camp);
    # then 3 enemy Spearmen come beside them: mine and the ally's fight them (both struck), still no target of the camp.
    Step "mark sides fight setup", "hol.Units.Remove -9000 0 6000 0", "hol.Units.Remove -9000 0 6000 1", "hol.Units.Remove -9000 0 6000 2", "frames 5",
        "hol.Units.Spawn 4 -10800 0 0 ManAtArms 200", "hol.Units.Spawn 4 -10250 0 2 Spearman 200", "frames 10", "hol.Combat.Enabled 1", "hol.Combat.Report"
    Step "mark sides fight auto", "wait 3", "hol.Combat.Report"
    Step "mark sides fight hold"; SidesBox $MineBox[0] $MineBox[1] -10600 $MineBox[3]; Step "hol.Select.Hold", "wait 2", "hol.Combat.Report"
    Step "mark sides fight attackmove", "hol.Select.AttackMove -10250 0", "wait 3", "hol.Combat.Report"
    # Case 3: an attack order of mine on the ally's unit, refused by the server (no hit); case 4: a right click on the
    # ally's unit, a move as on my own (no attack order sent). Mine boxed with the ally's (mine kept, own first).
    Step "mark sides fight order"; SidesBox $MineBox[0] $MineBox[1] -9700 $MineBox[3]; Step "hol.Select.Attack slot=2", "wait 3", "hol.Combat.Report"
    Step "mark sides fight rightclick"; SidesBox $MineBox[0] $MineBox[1] -9700 $MineBox[3]; Step "hol.Select.ClickUnit 0 order slot=2", "wait 3", "hol.Combat.Report"
    Step "mark sides fight enemy", "hol.Units.Spawn 3 -10500 900 1 Spearman 150", "wait 6", "hol.Combat.Report", "hol.Combat.Enabled 0"
    # Allied avoidance (the camps ignored by the avoidance and the pushes): my 4 Men-at-arms sent straight through a
    # standing wall of 25 allied Spearmen, combat off: they pass as through my own (the ally's pushed aside), not round a
    # wall of enemies: as through a wall of mine (hol.Units.Where 0 after 2 s and 4 s: where my 4 are, the ids last spawned).
    foreach ($Wall in @(0, 2)) {
        # Same seed for both walls: the same shape, only the owner changes (the gap between them was the draw).
        Step "mark sides avoid $Wall", "hol.Units.Remove -9000 0 6000 0", "hol.Units.Remove -9000 0 6000 1", "hol.Units.Remove -9000 0 6000 2", "frames 5", "hol.Units.SpawnSeed 1234",
            "hol.Units.Spawn 5 -9600 -800 $Wall Spearman 60", "hol.Units.Spawn 5 -9600 -400 $Wall Spearman 60", "hol.Units.Spawn 5 -9600 0 $Wall Spearman 60",
            "hol.Units.Spawn 5 -9600 400 $Wall Spearman 60", "hol.Units.Spawn 5 -9600 800 $Wall Spearman 60", "hol.Units.Spawn 4 -10800 0 0 ManAtArms 150", "frames 20", "hol.Units.Where $Wall"
        SidesBox -11100 -450 -10500 450; Step "hol.Select.OrderGround -8400 0", "wait 2", "hol.Units.Where 0", "wait 2", "hol.Units.Where 0", "hol.Units.Where $Wall"
    }
    Step "mark sides end", "open /Game/Maps/MainMenu", "waitmap MainMenu", "waitui"
}

# The "sizes" part (TC4, map sizes): Plain at each size, opened from a v3 configuration file by hol.Match.Open (as a
# server given -MatchConfig=: the game mode alone checks it), places drawn from a fixed seed. Each: the map of the size
# loaded, one main centre per player on its drawn place (the circle of the size, radius 0.75 of the half side, place 0 at
# 180 degrees, counter-clockwise), the camera above slot 0's centre, then sent past both corners: held on the size's
# bounds. Large: 4 against 4, "Équipes ensemble" (each team on 4 neighbouring places); Huge: 5 players on 8 places,
# spread round the circle. Then a refused one: fixed places past the size (slot 2 on the 2 places of Small): "Match
# config refused", no centre, no crash, back to the menu.
# Each size: key, side in tiles (Tools/Data/MapSizes.csv), places, slots taken, seed, teams (one per slot, or none), teams together.
$SizeCases = @(
    @{ Key = "Small"; Side = 110; Places = 2; Players = 2; Seed = 11; Teams = $null; Together = $true },
    @{ Key = "Medium"; Side = 154; Places = 4; Players = 4; Seed = 12; Teams = $null; Together = $true },
    @{ Key = "Normal"; Side = 183; Places = 6; Players = 6; Seed = 13; Teams = $null; Together = $true },
    @{ Key = "Large"; Side = 202; Places = 8; Players = 8; Seed = 14; Teams = @(1, 2, 1, 2, 1, 2, 1, 2); Together = $true },
    @{ Key = "Huge"; Side = 220; Places = 8; Players = 5; Seed = 15; Teams = $null; Together = $false }
)
function Write-SizeConfig([string] $Path, [string] $Size, [int] $Seed, [bool] $Draw, [bool] $Together, [string[]] $States, $Teams) {
    $Slots = for ($i = 0; $i -lt 8; $i++) {
        $Team = if ($Teams) { ", `"team`": $($Teams[$i])" } else { "" }
        "{ `"state`": `"$($States[$i])`", `"colorIndex`": $i$Team }"
    }
    "{ `"version`": 3, `"seed`": $Seed, `"map`": `"/Game/Maps/Plain.Plain`", `"mapSize`": `"$Size`", `"bDrawPlaces`": $($Draw.ToString().ToLower()), `"bTeamsTogether`": $($Together.ToString().ToLower()), `"slots`": [ $($Slots -join ', ') ] }" |
        Set-Content -Encoding ascii $Path
}
if ($Parts -contains "sizes") {
    Step "mark sizes start", "hol.Camera.TestCursor 0.5 0.5"
    foreach ($Size in $SizeCases) {
        $States = for ($i = 0; $i -lt 8; $i++) { if ($i -eq 0) { "Human" } elseif ($i -lt $Size.Players) { "AI" } else { "Closed" } }
        $Config = Join-Path $TestSaved "sizes_$($Size.Key).json"
        Write-SizeConfig $Config $Size.Key $Size.Seed $true $Size.Together $States $Size.Teams
        Step "mark sizes $($Size.Key)", "hol.Match.Open $Config", "waitmap Plain_$($Size.Key)", "waitlog Camera above the start of slot 0", "frames 5", "hol.Camera.Report",
            "hol.Camera.Focus 9999999 9999999", "frames 2", "hol.Camera.Report", "hol.Camera.Focus -9999999 -9999999", "frames 2", "hol.Camera.Report"
    }
    $Config = Join-Path $TestSaved "sizes_refused.json"
    Write-SizeConfig $Config "Small" 16 $false $true @("Human", "Closed", "AI", "Closed", "Closed", "Closed", "Closed", "Closed") $null
    Step "mark sizes refused", "hol.Match.Open $Config", "waitmap Plain_Small", "frames 30",
        "mark sizes end", "open /Game/Maps/MainMenu", "waitmap MainMenu", "waitui", "mark sizes menu"
}

# Start slots of /Game/Maps/Plain (Tools/Editor/create_match_map.py): 150 m from the centre, at these angles; each main
# centre (4 x 4 tiles, 16 m) stands on the 4 m tile grid (D311, D344), its middle on the grid point nearest its slot,
# halves rounded up as FMath::RoundToDouble does (slot 0: (-14800, 0), slot 1: (15200, 0); diagonal slots: 1.93 m off on
# each axis), where the camera starts and the scripted checks aim.
$SlotAngles = @(180, 0, 270, 90, 225, 45, 315, 135)
$Tile = 400
function Get-SlotStart([int] $Slot) {
    $Radians = $SlotAngles[$Slot] * [Math]::PI / 180
    return @(([Math]::Floor(15000 * [Math]::Cos($Radians) / $Tile + 0.5) * $Tile), ([Math]::Floor(15000 * [Math]::Sin($Radians) / $Tile + 0.5) * $Tile))
}
# Where hol.Build.PlaceNear <Type> <Dx> <Dy> puts a building of Tiles x Tiles tiles: slot's main centre plus (Dx, Dy) in
# its frame (X toward the map's middle), the footprint's corner then snapped to the nearest grid point
# (HOLConstruction::SnapToGrid). The two centres of the 2-player map are not mirrors of each other on the 4 m grid
# ((-14800, 0) and (15200, 0)): the same offset gives spots that are not mirrors either.
function Get-NearSpot([int] $Slot, [double] $Dx, [double] $Dy, [int] $Tiles) {
    $At = Get-SlotStart $Slot
    $Yaw = ($SlotAngles[$Slot] + 180) * [Math]::PI / 180
    $Half = $Tiles * $Tile / 2
    $Point = @(($At[0] + $Dx * [Math]::Cos($Yaw) - $Dy * [Math]::Sin($Yaw)), ($At[1] + $Dx * [Math]::Sin($Yaw) + $Dy * [Math]::Cos($Yaw)))
    return @($Point | ForEach-Object { [Math]::Floor(($_ - $Half) / $Tile + 0.5) * $Tile + $Half })
}
# The "net" part's buildings, in each client's centre's frame: its house 24 m behind (12 m behind its back side), the
# house asked near the enemy centre 32 m behind it (touching that centre's house, 28 m from its middle: within the
# anti-rush 40 m), its lumber camp by its wood, its barracks (slot 0's: the solo part's).
$NetHouseNear = @(-2400, 0)
$NetEnemyNear = @(-3200, 0)
$NetCampNear = @(1500, -2100)
$NetBarracksNear = @(-3800, 4600)
# The local player takes the first human slot (D243): 2 players -> slot 0; 8 players, seven AI first -> slot 7.
$Cases = @(
    @{ Name = "2p"; Slot = 0; Other = 1; States = @("Human", "Human", "Closed", "Closed", "Closed", "Closed", "Closed", "Closed") },
    @{ Name = "8p"; Slot = 7; Other = 0; States = @("AI", "AI", "AI", "AI", "AI", "AI", "AI", "Human") }
)
# A part played by cases (part:case) only needs the camera part's 2-player match, where the solo parts run: no 8-player
# match (its launch and checks, ~20 s), nothing the cases read.
if ($SubCases.Count -gt 0) { $Cases = @($Cases[0]) }
# Gathering in solo (J5b.3): the Peasants of slot 0 left by the parts before (2 produced, 3 more with the orders), the
# tree clicked (source 3, the 4th of the first row of slot 0's wood) and the berry bush (the middle one of the front row
# of slot 0's patch), where the two groups are made.
$GatherBefore = $(if ($Parts -contains "production") { 2 } else { 0 }) + $(if (($Parts -contains "production") -and ($Parts -contains "move")) { 3 } else { 0 })
$GatherTrees = 12
$GatherBerries = 4
$GatherTree = @(-14885, -1781)
$GatherTreeSpawn = @(-14885, -1250)
$GatherBerry = @(-15000, 2000)
$GatherBerrySpawn = @(-15000, 1350)
# Placement in solo (J6a.3): house A 16 m behind slot 0's centre's middle, a tile (4 m) behind its back side (its middle
# on a grid point, 2 x 2 tiles of 4 m: 8 m), house B touching it, a spot across both. Behind slot 0's centre, the map
# ends 44 m from its back side (x -20000): the buildings behind it stand within 11 tiles.
$BuildA = @(-16400, 0)
$BuildB = @(-16400, 800)
$BuildOver = @(-16400, 400)
# Building in solo (J6a.4): the Peasant sent to A, then three Peasants for house C and house D touching it (Shift), all
# behind slot 0's centre; the navmesh probed across A (16 m, through its middle).
$BuildPeasant = @(-18400, -400)
$BuildThree = @(-19400, 800)
$BuildC = @(-17600, 800)
$BuildD = @(-17600, 1600)
$BuildAcross = @(-16400, -800, -16400, 800)
# The user's trial of J6a.4 (solo): house E behind slot 0's centre, on the wood's side, its rally point on it; F 8 m from
# E (between their tiles), G 40 m from it on the other side; H and I behind F; a tree of the wood's back rows.
$BuildE = @(-17200, -1600)
$BuildF = @(-17200, -3200)
$BuildG = @(-17200, 3200)
$BuildH = @(-18400, -3200)
$BuildI = @(-19200, -3200)
$BuildTree = @(-15770, -2400)
# The construction HUD (J6a.5): house J behind slot 0's centre, away from the others, two Peasants made beside it (south).
$BuildJ = @(-19200, 2400)
$BuildJSpawn = @(-19200, 1400)
# F4 (J8a.2, J8a-Q4): slot 0's hero behind its centre, away from the houses of the build part; slot 1's behind the enemy centre.
$BuildHero = @(-18800, 3600)
$BuildEnemyHero = @(18800, 3600)
# J8b.2 loop 3: the House finished while the hero is dead, north of his place, and its builder made east of it.
$HeroXPHouse = @(-17600, 5200)
$HeroXPBuilder = @(-16400, 5200)
# The rally flags of a group (the user's 3rd trial of J6a.5): a right click on the ground behind slot 0's centre.
$BuildRally = @(-18400, -1600)
# The animations (J5c.4), solo: house K behind slot 0's centre and its wood, its builders made beside it; each group of
# gatherers made a few metres from its source (the wood's edge, the stone and gold veins on either side of slot 0, slot 4's
# berries: slot 0's are used up by the gather part), then sent ~20 m away with its load; three idle Peasants on their own.
# Look: where its close-up at work is taken from (the wood's edge, not under the crowns).
$AnimHouse = @(-19200, -5200)
$AnimBuilders = @(-18000, -5200)
$AnimIdle = @(-9000, 2000)
# The soldiers (J7a.3): 3 Spearmen and 3 Men-at-arms made apart (east of the idle Peasants, north of the gap's walls),
# shown at rest, then sent 12 m east together: their armed idle and walk.
$AnimSpearmen = @(-7800, 3000)
$AnimMenAtArms = @(-7800, 4000)
$AnimSoldiersTo = @(-6600, 3500)
# The colours (J7a.4): a Peasant, a Spearman and a Man-at-arms of slot 1 (no player, red by default) west of slot 0's
# soldiers, never selected nor ordered: both colours side by side (body, pennant, shield face) in anims_colors.
$AnimOthers = @(-8900, 3500)
$AnimGroups = @(
    @{ Name = "wood"; Spawn = @(-13700, -1500); Source = @(-14221, -1900); Target = @(-12500, -3000); Look = @(-14200, -1450) },
    @{ Name = "stone"; Spawn = @(-11087, -5400); Source = @(-11087, -4592); Target = @(-11087, -6600); Look = @(-11087, -4592) },
    @{ Name = "gold"; Spawn = @(-11087, 5400); Source = @(-11087, 4592); Target = @(-11087, 6600); Look = @(-11087, 4592) },
    @{ Name = "berries"; Spawn = @(-11668, -9546); Source = @(-12021, -9193); Target = @(-10200, -8000); Look = @(-12021, -9193) }
)
# The camps (J6b.1): every Peasant of slot 0 sent behind its centre; lumber camp L (2 x 2 tiles, 8 m) 2 m off the edge
# of slot 0's wood (on the map's side: nearer than the centre to the first trees of its front rows, so that the Peasant
# gathering there unloads at L once it is finished), a Peasant made beside the wood sent to its tree nearest L, two
# builders made beyond L; mining camp M 5 m off the stone vein between slots 0 and 4 (at (-11087, -4592)), its two
# builders made beside it; a lumber camp 30 m from every wood, behind the centre.
$CampCrowd = @(-19200, 1400)
$CampL = @(-13600, -2000)
$CampLSpawn = @(-12000, -2000)
$CampEarly = @(-13800, -1100)
$CampTree = @(-14230, -1800)
$CampM = @(-12000, -3600)
$CampMSpawn = @(-12000, -2600)
# Mill Mi (D346) 4 m off slot 7's berries (a closed slot: its patch is never used up, unlike slot 0's), 146 m from slot
# 0's centre; its two builders made beyond it.
$CampMill = @(-8400, 13200)
$CampMillSpawn = @(-7400, 14000)
# Farms (IA1.2 F9) 16-24 m east of mill Mi (its food deposit; slot 7's berries are west of Mi, beyond the 20 m of a nearby
# grove): B built first by one Peasant then sent away (its field free), A touching it on the north (centres 8 m apart: one
# massif) by two; a third Peasant's right click on A; a walker across both, north to south through their centres.
$FarmA = @(-6000, 13200)
$FarmB = @(-6000, 12400)
$FarmBBuilder = @(-4800, 12400)
$FarmAway = @(-4400, 10800)
$FarmABuilders = @(-4800, 13600)
$FarmCSpawn = @(-4800, 14400)
$FarmClick = @(-5750, 13350)
$FarmCrossFrom = @(-6000, 15000)
$FarmCrossTo = @(-6000, 10600)
$CampFar = @(-19200, -800)
$CampFarSpawn = @(-19200, -2000)
# A right click on a deposit (L.1, D485): lumber camp D south-west of the forum, clear of the houses of the earlier cases (y = -3200), its builder made east of it; three
# Peasants made by the tree nearest CampTree (the forum, at (-15000, 0), much nearer them than D), a fourth beside them.
$DropD = @(-18400, -6000)
$DropBuilder = @(-17200, -6000)
$DropGatherers = @(-13700, -1300)
$DropEmpty = @(-13700, -900)
# Shift + right click on a deposit (L.3, D485): house S between the wood and D, built by a carrier and an empty Peasant.
$DropShiftHouse = @(-17600, -4400)
# Shift + right click on a deposit with Shift moves (L.5, D485): point P south-east of the wood (17 m from its first tree,
# 9 m off the way from the wood to D), point B 15 m south of D; both on free ground.
$DropMoveP = @(-13400, -3300)
$DropMoveB = @(-18400, -7500)
# Shift + ground then Shift + work (L.6a): (a) a Peasant made 20 m north of Pa, the tree nearest CampTree ~64 m north
# of Pa; (b) house H, its builder made 8 m east of it and sent 24 m east (Away), the Peasant made 24 m north of H, Pb
# 24 m east of that Peasant; (c) a Peasant made north-west of H, Q 33 m west of H. All south of D's row, on free ground.
$ShiftWorkA = @(-15800, -6000)
$ShiftWorkPa = @(-15800, -8000)
$ShiftWorkH = @(-15600, -9600)
$ShiftWorkHBuilder = @(-14800, -9600)
$ShiftWorkAway = @(-13200, -9600)
$ShiftWorkB = @(-15600, -7200)
$ShiftWorkPb = @(-13200, -7200)
$ShiftWorkC = @(-16400, -7200)
$ShiftWorkQ = @(-18800, -10400)
# Shift + ground then Shift + placement (EtapeDefini M.1): a Peasant made at A, P 20 m east of it, house H 20 m south of
# P, Q 24 m west of H. Shift + site then Shift + ground (M.1): sites S and S2 (their builder made east of S and sent
# east, Away), (a) a Peasant made north-west of S, Q2 south-west of S, then slot 0's berries; (b) a Peasant made east of
# S2, Q3 west of S2. All south of the shiftwork row, on free ground.
$ShiftPlaceA = @(-17200, -12000)
$ShiftPlaceP = @(-15200, -12000)
$ShiftPlaceH = @(-15200, -14000)
$ShiftPlaceQ = @(-17600, -14400)
$ShiftSiteS = @(-12800, -12000)
$ShiftSiteS2 = @(-12800, -14800)
$ShiftSiteBuilder = @(-12000, -12000)
$ShiftSiteAway = @(-10800, -13200)
$ShiftSiteC = @(-13600, -10800)
$ShiftSiteQ2 = @(-14400, -13200)
$ShiftSiteBerries = @(-12021, -9193)
$ShiftSiteD = @(-11200, -15600)
$ShiftSiteQ3 = @(-14400, -16000)
# A rally point in several steps (EtapeDefini N.1): slot 0's centre selected; R1 20 m north-east of it, R2 28 m north-west
# of R1 (each ~18 m off the straight way from the centre to the next aim, so a step skipped shows); the gold vein north-east
# of slot 0 (a source after R1); R3 for a plain right click after the steps; nine points in a row for the limit (8 steps).
$RallyStepsR1 = @(-13600, 3600)
$RallyStepsR2 = @(-15600, 5600)
$RallyStepsGold = @(-11087, 4592)
$RallyStepsR3 = @(-14000, 5200)
$RallyStepsRow = @(for ($k = 0; $k -lt 9; $k++) { , @((-15200 + 300 * $k), 6800) })
# A unit standing still in a corner of a house's visible wall (EtapeDefini C.1): house X on free ground north of slot 0's
# centre, clear of every building of the cases before (the barracks at (-18600, 4600) and (-18600, 7400) wedged a corner's unit
# between two walls at (-18000, 5600): C.3, it stays) and of the rally steps (y 6800); its builder made east of it; a Man-at-arms
# made 8.5 m beyond each of its corners on the diagonal (the signs of the four corners), sent to the diagonal beyond that corner.
$CornerHouse = @(-16400, 9600)
$CornerBuilder = @(-15400, 9600)
# The window sizes the bottom bar must fit at (EtapeDefini T, build:barwidth): the common 16:9 and 16:10 ones, a 4:3 one, and
# 1500 wide at 1080 high (the HUD's scale 1, where it fell short before).
$HudSizes = @("1280x720", "1280x1024", "1366x768", "1440x900", "1500x1080", "1920x1080")
$CornerSigns = @(@{ X = -1; Y = -1 }, @{ X = 1; Y = -1 }, @{ X = -1; Y = 1 }, @{ X = 1; Y = 1 })
# Paths cut by a footprint (J6b.2): six Peasants made beside the vein north-east of slot 0 (at (-11087, 4592); a vein is
# not used up in a minute, as a tree of the wood is: each tree felled forgets the kept paths around it) gather it and
# unload at the centre, 50 m away (M is farther); house K across their way, its builder made west of it. Then six Peasants
# sent from 50 m north of house N to 40 m south of it, its builder made east of it.
$CutSpawn = @(-11800, 3900)
$CutVein = @(-11087, 4592)
$CutK = @(-12800, 2400)
$CutKBuilder = @(-13700, 2400)
$CutN = @(-8800, 0)
$CutNBuilder = @(-7800, 0)
$CutFrom = @(-8800, 5000)
$CutTo = @(-8800, -4000)
# The way between two houses (J6b.2b, D324): houses P and Q whose tiles touch (Q north of P) on open ground east of house N,
# their builders made south of them; walkers made 15 m west of the way, sent 15 m east of it (the way runs along X at
# y = 400, between bodies 2.3 m apart (2 x BodyInset 115), from x -6285 to -5715: 8 m tiles less 1.15 m on each side).
# The formation (J7a.4), solo: 3 Peasants and 3 Men-at-arms made side by side (the soldiers south of them) on open
# ground between slot 0's ridge, slot 4's and the grove, sent 35 m east together: one pace (the slowest's), rows, the
# soldiers in front. Then a Ctrl + right-button drag, an arrow of 3.6 m north-east of them (its front of 3 columns
# centred on its base, facing along it, yaw 45; the user's 2nd trial), then a Peasant made north-west of the front sent through it: those pushed walk back to their places.
$FormPeasants = @(-6000, -5200)
$FormMenAtArms = @(-6000, -5800)
$FormTo = @(-2500, -5500)
$FormLine = @(-1627, -3627, -1373, -3373)
$FormWalker = @(-2300, -2700)
$FormWalkerTo = @(-700, -4300)
# Then (the user's 1st trial of J7a.4) a plain right-button drag of 11 m back at the march's end (orientation only: the default rows, not 6
# columns) and, with Shift, a move queued while they walk there, 26 m north: the six go there once that walk is over.
$FormPlainLine = @(-2900, -5900, -2100, -5100)
$FormQueuedTo = @(-2200, -2900)
# Then the block march (the user's 1st trial of J7a.4): 3 Peasants, 3 Spearmen and 3 Men-at-arms (800, 777, 697 cm/s since D345) on
# the six's first ground (gone by then), sent to the same point 35 m east.
$FormBlockPeasants = @(-6000, -5000)
$FormBlockSpearmen = @(-6000, -5500)
$FormBlockMenAtArms = @(-6000, -6000)
# Then the straggler (the user's 1st trial of J7b), after the groups: 5 Men-at-arms close together on the six's first
# ground (gone by then) and a 6th 10 m behind them (west), sent 37 m east-north-east (between the block's end and the
# six's queued point, more than 16 m from every enemy of the combat parts): the five leave at once, the 6th walks on its
# own path to its place. Then the same with an Archer (779 cm/s against 697) as the 6th, sent 35 m north-east.
$StragFrom = @(-6000, -5500)
$StragLate = @(-7000, -5500)
$StragTo = @(-2500, -4200)
$StragArcherTo = @(-3500, -3000)
# Then the scattered group (the user's trial of playtest note 1 of J7c): 2 groups of 3 Men-at-arms 10 m apart, side by side
# across the order (north and south of the six's first ground, gone by then), sent 37 m east-south-east (14 m south of the
# block of nine's end): no straggler (the body is half the group), the block forms its rows on the move at once (playtest note 1 of J7b).
$ScatterNorth = @(-6000, -5000)
$ScatterSouth = @(-6000, -6000)
$ScatterTo = @(-2500, -6900)
# Then the close straggler (the user's trial of playtest note 1 of J7b): 5 Men-at-arms in a file along the order (1.5 m
# apart, on the six's first ground, gone by then) and a 6th 3 m behind and beside the last (within the body's 4.5 m, far
# from its slot), sent 38 m east-south-east, between the block of nine's end and the scattered group's (9 m from each):
# no straggler, the block forms its rows on the move at once, nobody held nor left; the six removed at the end (the parts after unchanged).
$NearFile = @(-6000, -5500)
$NearLate = @(-6200, -5750)
$NearTo = @(-1900, -6200)
# Then two blocks meeting (the user's rule for playtest note 1 of J7b: the blocks merge when they meet, else at the arrival):
# 2 files of 3 Men-at-arms along the order (1.5 m apart), 5 m apart across it (two packets), on the six's first ground (gone
# by then), sent 28 m east to the point between them (west of the block of nine's end, 8 m from it): the two blocks' paths
# converge, their points meet on the way, the smaller merges into the main block; removed at the end.
$MeetNorth = @(-6200, -5200)
$MeetSouth = @(-6200, -5700)
$MeetTo = @(-3300, -5450)
# Formed up in place inside a mixed army (the user's trial of select_type_q: "elles peinent"), where the two blocks met
# (removed by then): one type picked by a double click on one of its units (ClickUnit's index from the last made, the
# click's point found by trial: in the crowd it may fall on a neighbour's body) and sent Dx east of the army's middle.
$RegroupAt = @(-3300, -5450)
$RegroupCases = @(
    @{ Name = "regroup"; Type = "ManAtArms"; Others = @("Spearman", "Archer"); Dx = 100; Group = 1 },
    @{ Name = "regroup spear"; Type = "Spearman"; Others = @("ManAtArms", "Archer"); Dx = 100; Group = 2 },
    @{ Name = "regroup formed"; Type = "ManAtArms"; Others = @("Spearman", "Archer"); Dx = 100; Group = 3; Formed = 1 },
    @{ Name = "regroup formed spear"; Type = "Spearman"; Others = @("ManAtArms", "Archer"); Dx = 100; Group = 4; Formed = 1 },
    @{ Name = "regroup formed centre"; Type = "ManAtArms"; Others = @("Spearman", "Archer"); Dx = 0; Group = 5; Formed = 1 }
)
$AidBack = 1850
# The melee (J7b.2), solo: 3 Men-at-arms of slot 0 and 3 Spearmen of slot 1, 6 m apart on the formation's first ground.
$CombatMenAtArms = @(-6300, -5500)
$CombatSpearmen = @(-5700, -5500)
# The attack-move and hold position (J7b.3), on ground clear of the parts before (more than 16 m from their soldiers).
# D345 (aggression 16 m): the Men-at-arms start 10 m further west and the Spearman 8 m north of their way (4 m before),
# more than 16 m edge to edge from them and from the Peasants left at (-5900, -2500) whatever the spawn's draw, so that
# only the attack-move (within 16 m of their way) takes it on.
$OrdersMoveFrom = @(-7500, -1500)
$OrdersMoveTo = @(-3500, -1500)
$OrdersMoveSpearman = @(-5000, -700)
$OrdersHold = @(-4500, 2500)
# (Spawn places a unit up to ~1.5 m off its point: the Peasant 5.5 m away, out of reach whatever the draw, within 16 m.)
$OrdersHoldPeasant = @(-3950, 2500)
$OrdersHoldSpearman = @(-4500, 3300)
# Two enemies never push each other back (the user's trial of J7b), on the attack-move's start (left by then): 3 Men-at-arms
# of slot 0 ordered on a Peasant of slot 1 standing behind 12 others of slot 1 (no aggression: they stand, never strike).
$PushMenAtArms = @(-6500, -1500)
$PushWall = @(-5700, -1500)
$PushTarget = @(-5000, -1500)
# A queue after an ordered target (the review of J7b): 3 Men-at-arms made for it on clear ground (more than 16 m from
# every unit left by the parts before: the push's wall is still being fought, the hold's Peasant still stands) ordered on
# a Peasant of slot 1 15 m north (A), then with Shift on another 20 m further north (B, out of their 16 m aggression once A
# is dead, D345: only the queue takes them there): B is attacked once A is killed; then a Peasant made 6 m beyond B (C) is
# taken on by their own aggression.
$QueueMenAtArms = @(-2000, 0)
$QueueA = @(-2000, 1500)
$QueueB = @(-2000, 3500)
$QueueC = @(-2000, 4100)
# The shot (J7c.2a): 4 Archers of slot 0 on clear ground south (more than 16 m from the units left), 3 Spearmen of slot 1
# 14 m north of them and 2 Peasants 10 m north-east.
$ArrowArchers = @(-2000, -5000)
$ArrowSpearmen = @(-2000, -3600)
$ArrowPeasants = @(-1300, -4300)
# The target that flees (J7c.2b): on the shot's ground once cleared, the Spearman 20 m north of the 3 Archers (their reach
# since D345; 15 m before, within its own 16 m aggression since D345, it would charge them), sent 50 m north.
$FleeArchers = @(-2000, -5000)
$FleeSpearman = @(-2000, -3000)
$FleeTo = @(-2000, 2000)
# The last attacker forgotten (EtapeDefini S): a Man-at-arms of slot 0 on clear ground south of the shot's, an Archer of slot 1
# 19 m south of it (within its 20 m reach, beyond the Man-at-arms' 16 m aggression), sent 51 m further south once its arrow
# has landed; the ground cleared at the end (3600 cm around ForgetGround: short of the reinforcements at MeetTo, 42 m off).
$ForgetManAtArms = @(-2000, -6000)
$ForgetArcher = @(-2000, -7900)
$ForgetTo = @(-2000, -13000)
$ForgetGround = @(-2000, -9500)
$GapP = @(-6000, 0)
$GapQ = @(-6000, 800)
$GapBuilders = @(-6000, -1500)
$GapFrom = @(-7500, 400)
# Half the width of a house's body (D324): 8 m tiles less BodyInset (115 cm) on each side; the way between P and Q spans
# x GapP[0] -+ this.
$GapHalfBody = 285
# The wall walk (measure only): a line 50 cm inside P's tiles on its south side (65 cm off its body).
$GapWallY = $GapP[1] - 350
# The barracks (J7a.2), solo: its middle on a tile's middle (3 x 3 tiles, 12 m) behind slot 0's centre, north-west of
# house G; its three builders made south of it; its rally point 17 m north-east; the soldiers then sent onto the vein of
# the cut part, then back to the rally point (the anims part boxes Peasants beside that vein). The same spot, in each
# centre's frame, for the "net" part's barracks ($NetBarracksNear).
$BarracksAt = @(-18600, 4600)
$BarracksSpawn = @(-18600, 3400)
$BarracksRally = @(-17400, 5800)
# The rally point of a barracks' ghost (the user's request of 2026-10-19), solo: a second barracks north of the first one,
# placed with nothing selected (a ghost, no builder); its rally point 18 m north-east of it; its builder made east of it.
$BarracksGhostAt = @(-18600, 7400)
$BarracksGhostSpawn = @(-17400, 7400)
$BarracksGhostRally = @(-17000, 8600)
$GapTo = @(-4500, 400)
# The smoke part (D327), the default's single short game: the title screen and New game in French, a 2-player match
# launched from it, the camera moved by the arrow, its own centre selected then nothing; 3 Men-at-arms of slot 0 made 18 m
# from a Peasant of slot 1 (beyond their 16 m aggression with this seed's draw), boxed and sent 4 m short of it: once there they take it on and
# kill it (the melee on for this step alone). Every step waits for its event, no fixed delays.
$SmokeMenAtArms = @(-6500, -1500)
$SmokeEnemy = @(-4700, -1500)
# The Shift queue kept in order (the user's trial of J7b): 4 Peasants on clear ground north of the hold, P1, P2, P3.
$FifoFrom = @(-5500, 6000)
$FifoP1 = @(-5500, 7800)
$FifoP2 = @(-4300, 6000)
$FifoP3 = @(-3500, 6500)
# The order lines (D328), on the fifo's ground (left by then): 3 Men-at-arms sent 10 m north (L1), then with Shift 10 m
# east (L2), then with Shift an attack-move 10 m south (L3).
$LinesFrom = @(-5500, 6000)
$LinesL1 = @(-5500, 7000)
$LinesL2 = @(-4500, 7000)
$LinesL3 = @(-4500, 6000)
# Born on a rock (EtapeDefini, the user's trial melee_reinforce): the middle boulder of slot 0's ridge (create_match_map.py:
# 8 x 5 x 3.6 m, half buried, its long side across), a Man-at-arms sent from it 17 m towards the middle, beside its axis.
$RockAt = @(-12000, 0)
$RockGoal = @(-10300, -1500)
$SmokeTo = @(-5100, -1500)
if ($Parts -contains "smoke") {
    $SmokeConfig = Join-Path $TestSaved "smoke_2p.json"
    $SmokeSlots = for ($i = 0; $i -lt 8; $i++) { "{ `"state`": `"$($Cases[0].States[$i])`", `"colorIndex`": $i }" }
    "{ `"map`": `"/Game/Maps/Plain.Plain`", `"slots`": [ $($SmokeSlots -join ', ') ] }" | Set-Content -Encoding ascii $SmokeConfig
    Step "mark smoke menu", "capture qc_smoke_title"
    Open NewGame; Step "capture qc_smoke_newgame"
    Step "mark smoke camera", "hol.Camera.TestCursor 0.5 0.5", "hol.UI.NewGame.Launch $SmokeConfig 0", "waitmap Plain", "frames 10", "hol.Camera.Report"
    Key Up "0.5"
    Step "hol.Camera.Report", "capture qc_smoke_play"
    Step "mark smoke selection", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)",
        "hol.Select.Click 0.03 0.05", "waitlog Selection: none"
    Step "mark smoke move", "hol.Units.Spawn 3 $($SmokeMenAtArms -join ' ') 0 ManAtArms 150", "hol.Units.Spawn 1 $($SmokeEnemy -join ' ') 1 Peasant 10",
        "waitunits 4", "hol.Camera.Focus $(($SmokeMenAtArms[0] + $SmokeEnemy[0]) / 2) $($SmokeMenAtArms[1])", "frames 3",
        "hol.Select.BoxGround $($SmokeMenAtArms[0] - 350) $($SmokeMenAtArms[1] - 350) $($SmokeMenAtArms[0] + 350) $($SmokeMenAtArms[1] + 350)",
        "waitlog Selection: 3 units (own): ManAtArms x3", "hol.Combat.Enabled 1", "hol.Select.OrderGround $($SmokeTo -join ' ')",
        "waitlog Peasant of slot 1 killed by", "mark smoke combat", "hol.Units.Where 0", "hol.Combat.Report", "capture qc_smoke_melee",
        "hol.Combat.Enabled 0", "mark smoke dying", "wait 1.5", "hol.Units.Report 200", "hol.Stats.Dump", "mark smoke end"
}
if ($Parts -contains "camera") {
    # After the smoke part, the match it launched is left for the main menu first.
    $First = -not ($Parts -contains "smoke")
    foreach ($Case in $Cases) {
        $Slots = for ($i = 0; $i -lt 8; $i++) { "{ `"state`": `"$($Case.States[$i])`", `"colorIndex`": $i }" }
        $Config = Join-Path $TestSaved "camera_$($Case.Name).json"
        "{ `"map`": `"/Game/Maps/Plain.Plain`", `"slots`": [ $($Slots -join ', ') ] }" | Set-Content -Encoding ascii $Config
        if (-not $First) { Step "open /Game/Maps/MainMenu", "waitmap MainMenu", "waitui" }
        $First = $false
        $Name = "cam_$($Case.Name)"
        # Launched from the New game screen; then arrow up held 1 s, one wheel step out, the cursor 1 s on the right
        # edge (simulated inside the game), a report after each.
        Step "mark camera $($Case.Name)", "hol.Camera.TestCursor 0.5 0.5", "hol.UI.NewGame.Launch $Config 0", "waitmap Plain", "frames 10"
        Step "capture ${Name}_start", "hol.Camera.Report"
        Key Up "1.0"
        Step "hol.Camera.Report", "hol.UI.Key MouseScrollDown 0", "wait 0.5", "hol.Camera.Report"
        Step "hol.Camera.TestCursor 0.999 0.5", "wait 1", "hol.Camera.TestCursor 0.5 0.5", "hol.Camera.Report", "capture ${Name}_end"
        # Selection (D262, D264), the cursor in the middle of the screen: its own centre, another player's, the void,
        # then Escape; a report says what is selected.
        Step "mark selection $($Case.Name)", "hol.Camera.FocusSlot $($Case.Slot)", "frames 3", "hol.Select.Click 0.5 0.5",
            "waitlog Selection: MainCentre of slot $($Case.Slot) (own)", "frames 2", "hol.Select.Report", "capture sel_$($Case.Name)_own"
        Step "hol.Camera.FocusSlot $($Case.Other)", "frames 3", "hol.Select.Click 0.5 0.5",
            "waitlog Selection: MainCentre of slot $($Case.Other) (other)", "frames 2", "hol.Select.Report", "capture sel_$($Case.Name)_other"
        Step "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "hol.Select.Report"
        Step "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre"
        Key Escape
        Step "waitlog Selection: none", "hol.Select.Report"
        if (($Parts -contains "production") -and $Case.Name -eq "2p") {
            # Production (J3, D284), a Peasant in 2 s instead of 20: its centre selected, one Peasant by a click on the
            # grid's cell, two by its key A, the third one cancelled by a click on the queue, the two others born.
            Step "mark production", "hol.Prod.BuildTime 2", "hol.Camera.FocusSlot $($Case.Slot)", "frames 3", "hol.Select.Click 0.5 0.5",
                "waitlog Selection: MainCentre of slot $($Case.Slot) (own)", "frames 2", "hol.HUD.Report"
            # A garbage collection first: the unit type table read by the HUD and the production must survive it.
            Step "obj gc", "frames 5", "hol.HUD.Click GridCell00", "waitlog queued Peasant (1 in"
            Key A
            Step "waitlog queued Peasant (2 in"
            Key A
            Step "waitlog queued Peasant (3 in", "frames 5", "hol.HUD.Report", "capture prod_queue"
            Step "hol.HUD.Click QueueItem2", "waitlog cancelled Peasant", "waitlog (2 born here)", "frames 10", "hol.HUD.Report", "hol.Prod.Report", "capture prod_done",
                "hol.Prod.BuildTime 0"
            # Its centre let go (Escape with it selected: no menu): the next part's click on it logs a new selection, which
            # a click on the centre already selected never writes (-Parts production,build without -Full stopped there).
            Key Escape
            Step "waitlog Selection: none"
        }
        if (($Parts -contains "move") -and $Case.Name -eq "2p") {
            # Paths (J4, D286), on the navmesh of Plain with the centre of slot 0 made at run time: through that
            # centre (bottom to top on the screen), beside it, through the ridge in front of it, across the grove.
            Step "mark move", "hol.Nav.Path -16500 0 -13500 0", "hol.Nav.Path -16500 3000 -13500 3000", "hol.Nav.Path -13000 0 -11000 0",
                "hol.Nav.Path -2000 0 2000 0"
            if ($Parts -contains "production") {
                # Orders (J4.7, D289), after the two Peasants of the production: a rally point between the centre and the
                # ridge, three Peasants produced walk to it; a box around them, Shift + click takes one out and back in;
                # a right click behind the ridge: they go around it, arrive without overlapping, none in a rock; Escape.
                Step "mark move solo", "hol.Cheat.Food 1000", "hol.Prod.BuildTime 2", "hol.Prod.Rally -13500 0",
                    "waitlog Rally: slot 0 set the rally point of MainCentre at (-13500, 0)",
                    "hol.Prod.Queue", "hol.Prod.Queue", "hol.Prod.Queue", "waitlog (5 born here)", "wait 0.5", "hol.Orders.Chains 0", "hol.Prod.BuildTime 0", "wait 7",
                    "hol.Units.Where 0", "hol.Units.Orders 0", "hol.Orders.Chains 0"
                Step "hol.Camera.Focus -12000 0", "frames 5", "hol.Select.BoxGround -14000 -500 -13000 500", "waitlog Selection: 3 units (own): Peasant x3",
                    "frames 3", "hol.Select.Report", "capture move_box"
                Step "hol.Select.ClickUnit -1 add", "waitlog Selection: 2 units (own): Peasant x2", "hol.Select.ClickUnit -1 add",
                    "waitlog Selection: 3 units (own): Peasant x3", "hol.Select.OrderGround -10500 0", "mark move walk"
                for ($i = 0; $i -lt 44; $i++) { Step "hol.Units.Where 0", "wait 0.25" }
                Step "mark move arrived", "hol.Units.Where 0", "hol.Nav.Units", "hol.Select.Report", "capture move_arrived"
                Key Escape
                Step "waitlog Selection: none", "hol.Select.Report"
            }
            # Shift queue in the order given (the user's trial of J7b): 4 Peasants standing, Shift on P1 (18 m north, goes at
            # once), P2 (12 m east of the start, nearer than P1) and P3: P1, then P2, then P3, P2 never skipped.
            Step "hol.Units.SpawnSeed 101", "hol.Units.Spawn 4 $($FifoFrom -join ' ') 0 Peasant 150", "wait 0.5", "hol.Camera.Focus $($FifoFrom -join ' ')", "frames 3",
                "hol.Select.BoxGround $($FifoFrom[0] - 400) $($FifoFrom[1] - 400) $($FifoFrom[0] + 400) $($FifoFrom[1] + 400)",
                "waitlog Selection: 4 units (own): Peasant x4", "mark move fifo", "hol.Units.Where 0",
                "hol.Select.OrderGround $($FifoP1 -join ' ') shift", "frames 2", "hol.Select.OrderGround $($FifoP2 -join ' ') shift", "frames 2",
                "hol.Select.OrderGround $($FifoP3 -join ' ') shift"
            for ($i = 0; $i -lt 16; $i++) { Step "wait 1", "hol.Units.Where 0" }
            # Taken away at the end (hol.Units.Remove), around P3 where they stand: the parts after it (gather's counts, the
            # build gap's way) see none of them. Waited for without its count (checked by the analysis: a wrong count must
            # not stop the rest of the sequence).
            Step "mark move fifo end", "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "hol.Units.Remove $($FifoP3 -join ' ') 800 0",
                "waitlog units of slot 0 within 800 of ($($FifoP3 -join ', '))", "frames 3"
            # The order lines (D328): 3 Men-at-arms boxed, a move, then Shift + a move, then Shift + an attack-move: one chain
            # Move -> Move -> AttackMove (server and received), its 3 lines and 3 points drawn while selected, none once the
            # selection is emptied (the chain still there); followed every half second, it shortens, then none on arrival.
            Step "hol.Units.SpawnSeed 102", "hol.Units.Spawn 3 $($LinesFrom -join ' ') 0 ManAtArms 150", "wait 0.5", "hol.Camera.Focus $($LinesFrom -join ' ')", "frames 3",
                "hol.Select.BoxGround $($LinesFrom[0] - 400) $($LinesFrom[1] - 400) $($LinesFrom[0] + 400) $($LinesFrom[1] + 400)",
                "waitlog Selection: 3 units (own): ManAtArms x3", "mark move lines", "hol.Camera.Focus $($LinesL2 -join ' ')", "frames 3",
                "hol.Select.OrderGround $($LinesL1 -join ' ')", "frames 2", "hol.Select.OrderGround $($LinesL2 -join ' ') shift", "frames 2",
                "hol.Select.AttackMove $($LinesL3 -join ' ') shift", "wait 0.5", "hol.Orders.Chains 0", "hol.Select.Report",
                "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "frames 2", "hol.Select.Report", "hol.Orders.Chains 0"
            for ($i = 0; $i -lt 24; $i++) { Step "wait 0.5", "hol.Orders.Chains 0" }
            # Taken away at the end, around L3 (waited for without its count, checked by the analysis).
            Step "mark move lines end", "hol.Units.Remove $($LinesL3 -join ' ') 800 0", "waitlog units of slot 0 within 800 of ($($LinesL3 -join ', '))", "frames 3"
            # Born on a rock (the user's trial melee_reinforce): a Man-at-arms made at the middle boulder of slot 0's ridge
            # (-12000, 0), boxed and sent 17 m beyond it; it must leave the rock and reach the point (sampled every 0.5 s).
            Step "hol.Units.SpawnSeed 103", "hol.Units.Spawn 1 $($RockAt -join ' ') 0 ManAtArms 30", "wait 0.5", "mark move rock", "hol.Units.Where 0", "hol.Nav.Units",
                "hol.Nav.Path $($RockAt -join ' ') $($RockGoal -join ' ')", "hol.Camera.Focus $($RockAt -join ' ')", "frames 3",
                "hol.Select.BoxGround $($RockAt[0] - 600) $($RockAt[1] - 600) $($RockAt[0] + 600) $($RockAt[1] + 600)",
                "waitlog Selection: ManAtArms #", "hol.Select.OrderGround $($RockGoal -join ' ')"
            for ($i = 0; $i -lt 24; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            Step "mark move rock end", "hol.Select.Click 0.03 0.05", "hol.Units.Remove $($RockGoal -join ' ') 800 0", "hol.Units.Remove $($RockAt -join ' ') 600 0", "frames 3"
        }
        if (($Parts -contains "bodies") -and $Case.Name -eq "2p") {
            # Walkers blocked by bodies (MV.4, the user's trial blocked_bodies), in world coordinates beyond the ridge (slot 0's
            # centre at -15000, 0): two houses touching (centres 8 m apart, as "build gap") built at once by 5 Peasants taken
            # away after; then 5 Peasants abreast (a rank, 1.5 m apart) 9 m before their narrow way, 3 Men-at-arms before a tight wall of 12 Peasants of
            # ours, 3 Peasants before a tight wall of 12 enemy Peasants (aggression radius 0: no fight). The three orders given
            # one after the other (the narrow way last: its time is the exact one), then a sample every 0.5 s for 13 s.
            Step "mark bodies", "hol.Units.Remove -9500 0 2500 0", "hol.Units.Remove -9500 -3000 1500 0", "hol.Units.Remove -9500 3000 1500 0", "frames 3",
                "hol.Cheat.Resources 5000", "hol.Units.SpawnSeed 121",
                "hol.Build.PlaceNear House 6000 -3400", "waitlog Construction: slot 0 placed House", "hol.Build.PlaceNear House 6000 -2600", "waitlog Construction: slot 0 placed House",
                "hol.Units.Spawn 5 -10000 -4300 0 Peasant 150", "wait 0.5", "hol.Camera.Focus -9500 -3100", "frames 3",
                "hol.Select.BoxGround -10600 -4900 -9400 -3700", "waitlog Selection: 5 units (own): Peasant x5", "hol.Build.Instant 1",
                "hol.Select.OrderNear 6000 -3200", "waitlog Construction: slot 0 finished House", "hol.Select.OrderNear 6000 -2400", "waitlog Construction: slot 0 finished House",
                "hol.Build.Instant 0", "hol.Units.Remove -9000 -3000 1500 0", "waitlog units of slot 0 within 1500 of (-9000, -3000)", "frames 3",
                "hol.Units.Spawn 1 -10000 -3100 0 Peasant 0", "hol.Units.Spawn 1 -10000 -2950 0 Peasant 0", "hol.Units.Spawn 1 -10000 -2800 0 Peasant 0", "hol.Units.Spawn 1 -10000 -2650 0 Peasant 0", "hol.Units.Spawn 1 -10000 -2500 0 Peasant 0", "hol.Units.Spawn 12 -9000 0 0 Peasant 300", "hol.Units.Spawn 3 -10400 0 0 ManAtArms 150",
                "hol.Units.Spawn 3 -10400 3000 0 Peasant 150", "hol.Units.Spawn 5 -10800 6000 0 Peasant 150", "hol.Units.Spawn 5 -9200 6000 0 Peasant 150"
            # The enemy wall posed one by one, two ranks of 6 staggered, bodies touching (1.25 m apart): no gap to walk through.
            $Wall = @(); for ($w = 0; $w -lt 12; $w++) { $Wall += "hol.Units.Spawn 1 $(if ($w -lt 6) { -9000 } else { -8880 }) $(3000 + (($w % 6) - 2.5) * 125 + $(if ($w -lt 6) { 0 } else { 62.5 })) 1 Peasant 0" }
            # The contacts with still bodies counted from the orders (MV.4.4, hol.Units.ContactReport: the count reset here).
            Step (@($Wall) + @("wait 1", "hol.Units.ContactReport",
                                "hol.Camera.Focus -9700 3000", "frames 3", "hol.Select.BoxGround -11000 2300 -9800 3700", "waitlog Selection: 3 units (own): Peasant x3", "hol.Select.OrderGround -7600 3000",
                "hol.Camera.Focus -9700 0", "frames 3", "hol.Select.BoxGround -11000 -700 -9800 700", "waitlog Selection: 3 units (own): ManAtArms x3", "hol.Select.OrderGround -7600 0",
                "hol.Camera.Focus -9700 -2800", "frames 3", "hol.Select.BoxGround -10900 -3300 -9800 -2300", "waitlog Selection: 5 units (own): Peasant x5", "hol.Select.OrderGround -7500 -2800",
                "mark bodies walk"))
            for ($i = 0; $i -lt 26; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            Step "hol.Units.ContactReport"
            # The crossing alone, once the others stand (the heading flips are counted over all walkers, MV.4.3): the count
            # reset, the two orders, a sample every 0.5 s for 8 s, the count.
            Step "hol.Units.TurnReport", "mark bodies crossing",
                "hol.Camera.Focus -10800 6000", "frames 3", "hol.Select.BoxGround -11400 5400 -10200 6600", "waitlog Selection: 5 units (own): Peasant x5", "hol.Select.OrderGround -9200 6000",
                "hol.Camera.Focus -9200 6000", "frames 3", "hol.Select.BoxGround -9800 5400 -8600 6600", "waitlog Selection: 5 units (own): Peasant x5", "hol.Select.OrderGround -10800 6000"
            for ($i = 0; $i -lt 16; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            Step "hol.Units.TurnReport"
            # Taken away at the end (the parts after see none of them). The two houses too, destroyed by the cheat (D345.2a):
            # left standing, they raised slot 0's population cap by 20 for the parts after ("build site" read 30 -> 40).
            Step "mark bodies end", "hol.Select.Click 0.03 0.05", "hol.Units.Remove -9000 0 5000 0", "hol.Units.Remove -9000 3000 2500 1", "hol.Units.Remove -10000 6000 2000 0",
                "hol.Cheat.DamageNear 99999 6000 -3400 own melee", "waitlog Building destroyed: House of slot 0 at (-8800, -3200)",
                "hol.Cheat.DamageNear 99999 6000 -2600 own melee", "waitlog Building destroyed: House of slot 0 at (-8800, -2400)", "frames 3"
        }
        if (($Parts -contains "rocks") -and $Case.Name -eq "2p") {
            # No body may go into a rock, no army stuck at one (the user's trials ridge_army): each case of $RockCases in turn,
            # the units off the navmesh near the ridge logged every 0.1 s (hol.Nav.Units), the positions every 0.5 s, for 16 s.
            $c = 0
            foreach ($R in $RockCases) {
                $c++
                # The ground of the case cleared (its start and its target, not the centre's Peasants), the box short of the centre (x -14400).
                if ($R.Chain) { Step "mark rocks $c", "wait 0.5", "hol.Select.Click 0.03 0.05", "waitlog Selection: none" }
                else { Step "mark rocks $c", "hol.Units.Remove $($R.X) $($R.Y) $($R.R + 900) 0 $($R.Type)", "hol.Units.Remove $($R.TX) $($R.TY) 1800 0 $($R.Type)", "frames 3", "hol.Units.SpawnSeed $(210 + $c)", "hol.Units.Spawn $($R.Count) $($R.X) $($R.Y) 0 $($R.Type) $($R.R)", "wait 0.5" }
                # The box wholly on the screen (the camera over the start), then the target (the camera over it).
                Step "hol.Camera.Focus $($R.X) $($R.Y)", "frames 3", "hol.Select.BoxGround $([Math]::Max($R.X - $R.R - 500, -14300)) $($R.Y - $R.R - 500) $($R.X + $R.R + 500) $($R.Y + $R.R + 500)", "waitlog Selection: $($R.Count) units (own)",
                    "hol.Camera.Focus $($R.TX) $($R.TY)", "frames 3", "hol.Select.OrderGround $($R.TX) $($R.TY)", "mark rocks walk $c"
                # 25 cm (hol.Nav.Units): a unit as drawn between two steps cuts the navmesh's voxel corner round a trunk (27 cm clear of it).
                for ($i = 0; $i -lt 160; $i++) { Step "wait 0.1", "hol.Nav.Units 25"; if ($i % 5 -eq 4) { Step "hol.Units.Where 0" } }
                # Taken away, unless the next case orders it again.
                if (-not ($c -lt $RockCases.Count -and $RockCases[$c].Chain)) { Step "hol.Units.Remove $($R.TX) $($R.TY) 1800 0 $($R.Type)", "frames 3" }
            }
            Step "mark rocks end", "hol.Select.Click 0.03 0.05"
            foreach ($R in $RockCases) { Step "hol.Units.Remove $($R.TX) $($R.TY) 1800 0 $($R.Type)" }
            Step "frames 3"
        }
        if (($Parts -contains "sources") -and $Case.Name -eq "2p") {
            # Resource sources (J5a.2, D297): the 376 sources placed by the map (create_match_map.py: 40 trees beside each slot,
            # then 6 berry bushes on its other side and 8 veins between neighbours, J5b.1), the
            # first one of slot 0's wood at (-14221, -1816) and the one behind it at (-14217, -2023). A standing tree is
            # forbidden on the navmesh; used up (hol.Sources.Take, a development tool until the gathering of J5a.3), it
            # leaves the map and the navmesh opens where it stood, the tree behind still closing its own place.
            Step "mark sources", "hol.Sources.Report", "hol.Sources.Nearest -14221 -1300", "hol.Nav.Path -14221 -1300 -14221 -1816",
                "hol.Sources.Take 0", "waitlog Sources: source 0 used up (server)", "wait 1", "hol.Sources.Report",
                "hol.Nav.Path -14221 -1300 -14221 -1816", "hol.Nav.Path -14221 -1300 -14217 -2023",
                "hol.Camera.Focus -15000 -1500", "frames 5", "capture src_wood"
        }
        if (($Parts -contains "gather") -and $Case.Name -eq "2p") {
            # Gathering in solo (J5b.3, D299, D307, D309), Peasants of slot 0 made by hol.Units.Spawn (free) beside those
            # of the parts before ($GatherBefore, idle): twelve in a box, a right click on one tree of the wood's edge,
            # the 4th of its first row (more than its free spots: the others go to trees beside it); four in a box, a
            # right click on the berries; the centre's rally point on the same bush, one Peasant produced, which gathers
            # there. Then the six bushes used up (hol.Sources.TakeNear): the carriers unload, the stock rises then
            # only, all stand idle; the "." key and the HUD's counter go through them.
            Step "mark gather", "hol.Units.SpawnSeed 102", "hol.Nav.Footprints navmesh", "hol.Nav.Footprints on", "hol.Gather.LogDeposits 1", "hol.Gather.LogPopups 1", "hol.Cheat.Food 1000", "hol.Cheat.Population 165", "hol.Prod.BuildTime 2", "hol.Units.Where 0",
                "hol.Units.Spawn $GatherTrees $($GatherTreeSpawn -join ' ') 0 Peasant 250", "hol.Units.Spawn $GatherBerries $($GatherBerrySpawn -join ' ') 0 Peasant 200",
                "waitunits $($GatherBefore + $GatherTrees + $GatherBerries)"
            Step "hol.Camera.Focus $($GatherTreeSpawn[0]) $($GatherTreeSpawn[1] + 150)", "frames 5", "hol.Select.BoxGround -15450 -1650 -14300 -850",
                "waitlog Selection: $GatherTrees units (own): Peasant x$GatherTrees", "hol.Select.OrderSource $($GatherTree -join ' ')",
                "waitlog Gather order of slot 0: $GatherTrees units to source"
            Step "hol.Camera.Focus $($GatherBerry[0]) $($GatherBerry[1] - 400)", "frames 5", "hol.Select.BoxGround -15350 1000 -14650 1700",
                "waitlog Selection: $GatherBerries units (own): Peasant x$GatherBerries", "hol.Select.OrderSource $($GatherBerry -join ' ')",
                "waitlog Gather order of slot 0: $GatherBerries units to source"
            Step "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)",
                "hol.Camera.Focus $($GatherBerry[0]) $($GatherBerry[1] - 400)", "frames 3", "hol.Select.OrderSource $($GatherBerry -join ' ')",
                "waitlog Rally: slot 0 set the rally point of MainCentre", "hol.Prod.Queue", "waitlog Production: Peasant of slot 0 done", "hol.Prod.Report"
            # Born on the rally point's side (D303), ~10 m from its bush, a first food in ~1.4 s: carrying when looked at.
            Step "wait 5", "mark gather working", "hol.Units.Where 0", "hol.Prod.Report", "hol.HUD.Report", "frames 2", "capture gather_berries",
                "hol.Camera.Focus $($GatherTree[0]) $($GatherTree[1] + 300)", "frames 5", "capture gather_wood"
            # The camera on the centre's side facing the berries, where they unload: the first +10 captured (J5c.3).
            Step "mark gather used up", "hol.Camera.Focus $($GatherBerry[0]) 500", "frames 3"
            for ($i = 0; $i -lt 6; $i++) { Step "hol.Sources.TakeNear $($GatherBerry[0]) $($GatherBerry[1] + 150)" }
            Step "waitlog Deposit popup: +", "frames 6", "capture gather_popup"
            # A second after the last one stopped (J7a.6): those stopped at the deposit side by side part over a few steps
            # (UHOLGatherProcessor::Stop), the last ones 5 steps before the look stood 70 cm apart.
            Step "waitidle $($GatherBefore + $GatherBerries + 1)", "wait 1", "frames 2", "hol.Units.Where 0", "hol.Prod.Report", "hol.HUD.Report", "hol.Stats.Dump"
            # The "." key three times, then a click on the HUD's counter: each time the next idle one, the camera above it.
            Step "mark gather idle", "hol.Camera.FocusSlot 0", "frames 2"
            for ($i = 0; $i -lt 3; $i++) {
                Key Period
                Step "waitlog Next idle worker:", "frames 2", "hol.Camera.Report", "hol.Select.Report"
                if ($i -eq 0) { Step "hol.HUD.Report", "capture gather_idle" }
            }
            Step "hol.HUD.Click IdleWorkers", "waitlog Next idle worker:", "frames 2", "hol.Camera.Report", "hol.Select.Report", "hol.Prod.BuildTime 0"
            # No body in a footprint (J6a.1, D310): every Peasant of slot 0 sent onto its centre itself (the point inside
            # the footprint: they crowd against its wall), then around it to its far corner; watched since the mark gather.
            Step "mark footprints", "hol.Units.Move $((Get-SlotStart 0) -join ' ') 0", "wait 5", "hol.Units.Move $((Get-SlotStart 0)[0] - 1900) -1500 0", "wait 5",
                "hol.Nav.Footprints off", "hol.Nav.Units"
        }
        if (($Parts -contains "build") -and $Case.Name -eq "2p") {
            # -Parts build:<case>: only the marks of that case (Step), the others skipped.
            Start-CaseFilter "build"
            # Nothing selected: the placements of J6a.3 send no builder (J6a.4). The own centre selected, then Escape: never
            # Escape with nothing selected, which opens the in-game menu (D356) and took the placement's Escape below. The
            # report says it, after a mark (the selection part counts its reports).
            Step "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)"
            Key Escape
            Step "mark build none", "hol.Select.Report", "waitlog Selection report: selected none"
            # Placement (J6a.3, D311), 120 wood exactly (two houses of 50): the cursor stays in the middle of the screen
            # (hol.Camera.TestCursor), the camera brings the ghost where wanted. House A behind slot 0's centre, B touching
            # it (Shift: still in the mode), then the ghost near the enemy centre: red (on a house it now goes beside it, the
            # user's trial of J7b, feedback 3); Escape leaves the mode.
            Step "mark build", "hol.Cheat.Wood -100000", "hol.Cheat.Wood 120", "hol.Build.Report",
                "hol.Camera.Focus $($BuildA -join ' ')", "frames 3", "hol.Build.Place House", "waitlog Placement ghost: House at ($($BuildA -join ', ')) valid",
                "frames 3", "hol.Select.Report", "capture build_ghost_green", "hol.Build.Click shift",
                "waitlog Construction: slot 0 placed House foundation at ($($BuildA -join ', '))"
            Step "hol.Camera.Focus $($BuildB -join ' ')", "frames 3", "hol.Build.Click shift", "waitlog Construction: slot 0 placed House foundation at ($($BuildB -join ', '))",
                "hol.Camera.FocusNear -2400 0 enemy", "waitlog Placement ghost: House at", "frames 3", "hol.Select.Report",
                "capture build_ghost_red", "hol.Build.Click", "waitlog Placement: not here"
            Key Escape
            Step "waitlog Placement: ended", "hol.Select.Report"
            # Refused by the server, nothing taken (20 wood left): on house A and B, on a tree of slot 0's wood, near the
            # enemy centre (slot 1), and a free spot without the wood.
            Step "mark build refused", "hol.Build.PlaceNear House $($BuildOver[0] - (Get-SlotStart 0)[0]) $($BuildOver[1])",
                "waitlog Construction: slot 0 cannot place", "hol.Build.PlaceAtSource House -14217 -2023", "waitlog Construction: slot 0 cannot place",
                "hol.Build.PlaceNear House -2400 0 enemy", "waitlog Construction: slot 0 cannot place",
                "hol.Build.PlaceNear House -2400 0", "waitlog Construction: slot 0 cannot place", "hol.Build.Report"
            # House B selected by a click on it (its square, its block in the HUD), then cancelled: refunded, gone.
            Step "mark build cancel", "hol.Camera.Focus $($BuildB[0]) $($BuildB[1] - 300)", "frames 3", "hol.Select.ClickGround $($BuildB -join ' ')",
                "waitlog Selection: House of slot 0 (own), foundation", "frames 3", "hol.Select.Report", "hol.HUD.Report", "capture build_foundations",
                "hol.Build.CancelSelected", "waitlog Construction: slot 0 cancelled House", "waitlog Selection: none", "frames 3", "hol.Build.Report", "hol.Select.Report"
            # The user's trial of J7b, feedback 3 (70 wood): the cursor on house A, 2 m north of its middle: the ghost goes
            # beside it, touching (ring 0), north (B's spot, now free: nearest the cursor), green; a click places it there.
            # Then selected and cancelled (refunded): the parts below find the ground and the stock as before.
            Step "mark build beside", "hol.Camera.Focus $($BuildA[0]) $($BuildA[1] + 200)", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($BuildB -join ', ')) valid, moved beside the building (ring 0)", "frames 3", "hol.Select.Report",
                "capture build_ghost_beside", "hol.Build.Click", "waitlog Construction: slot 0 placed House foundation at ($($BuildB -join ', '))", "hol.Build.Report",
                "hol.Select.ClickGround $($BuildB -join ' ')", "waitlog Selection: House of slot 0 (own), foundation", "hol.Build.CancelSelected",
                "waitlog Construction: slot 0 cancelled House", "waitlog Selection: none", "hol.Build.Report"
            # Building (J6a.4, D312), twice as fast (hol.Build.Speed 2: house 10 s for one builder). The population cap of the
            # centre alone (the gather part's cheat taken back), 100 wood more (170). The navmesh across house A, a ghost:
            # straight. A Peasant made beside it, selected, a right click on A: it walks there, works, A blocks the navmesh
            # (a detour now), rises (captured), is finished alone in 200 steps, the cap 10 -> 20, the Peasant idle.
            $Uncheat = if ($Parts -contains "gather") { "hol.Cheat.Population -165" } else { "hol.Build.Report" }
            Step "mark build site", "hol.Units.SpawnSeed 103", "hol.Build.Speed 2", $Uncheat, "hol.Cheat.Wood 100", "wait 0.3", "hol.Prod.Report", "hol.HUD.Report",
                "hol.Nav.Path $($BuildAcross -join ' ')", "hol.Units.Spawn 1 $($BuildPeasant -join ' ') 0 Peasant 10", "wait 0.5",
                "hol.Camera.Focus -17000 0", "frames 3", "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Select.OrderGround $($BuildA -join ' ')",
                "waitlog Build order of slot 0: 1 units to House at ($($BuildA -join ', '))", "waitlog Construction: slot 0 first work on House at ($($BuildA -join ', '))",
                "wait 2", "hol.Nav.Path $($BuildAcross -join ' ')", "wait 2", "hol.Build.Report", "frames 2", "capture build_site",
                "waitlog Construction: slot 0 finished House at ($($BuildA -join ', '))", "waitlog Build: a builder of slot 0 stops (no site left), idle", "wait 0.3",
                "hol.Prod.Report", "hol.HUD.Report", "hol.Build.Report"
            # Three Peasants made behind, selected by a box; house C placed by Shift + click, D touching it by another one
            # (queued in that order), Escape. The three build C (about twice as fast as one alone), then D.
            Step "mark build three", "hol.Units.SpawnSeed 104", "hol.Units.Spawn 3 $($BuildThree -join ' ') 0 Peasant 150", "wait 0.5", "hol.Camera.Focus $($BuildThree[0] + 600) $($BuildThree[1])", "frames 3",
                "hol.Select.BoxGround $($BuildThree[0] - 400) $($BuildThree[1] - 400) $($BuildThree[0] + 400) $($BuildThree[1] + 400)", "waitlog Selection: 3 units (own): Peasant x3",
                "hol.Camera.Focus $($BuildC -join ' ')", "frames 3", "hol.Build.Place House", "waitlog Placement ghost: House at ($($BuildC -join ', ')) valid",
                "hol.Build.Click shift", "waitlog Build order of slot 0: 3 units to House at ($($BuildC -join ', ')), queued",
                "hol.Camera.Focus $($BuildD -join ' ')", "frames 3", "waitlog Placement ghost: House at ($($BuildD -join ', ')) valid",
                "hol.Build.Click shift", "waitlog Build order of slot 0: 3 units to House at ($($BuildD -join ', ')), queued"
            Key Escape
            Step "waitlog Placement: ended", "hol.Camera.Focus -17400 300", "waitlog Construction: slot 0 first work on House at ($($BuildC -join ', '))", "wait 2", "frames 2",
                "capture build_three", "waitlog Construction: slot 0 finished House at ($($BuildC -join ', '))", "hol.Build.Speed 1",
                "waitlog Construction: slot 0 first work on House at ($($BuildD -join ', '))", "hol.Prod.Report", "hol.HUD.Report"
            # The first Peasant (idle at A) helps on D: a right click on the site; D cancelled once it works there: half
            # refunded, its four builders idle.
            Step "mark build help", "hol.Select.ClickUnit -4", "waitlog Selection: Peasant #", "hol.Select.OrderGround $($BuildD -join ' ')",
                "waitlog Build order of slot 0: 1 units to House at ($($BuildD -join ', '))", "waitlog House at ($($BuildD -join ', ')), 4 builders at work", "hol.Build.Report",
                "hol.Select.ClickGround $($BuildD -join ' ')", "waitlog Selection: House of slot 0 (own), under construction", "hol.Build.CancelSelected",
                "waitlog Construction: slot 0 cancelled House", "wait 1", "hol.Build.Speed 1", "hol.Camera.Focus -17000 300", "frames 5", "hol.Build.Report", "hol.Prod.Report",
                "hol.HUD.Report", "capture build_finished"
            # The user's trial of J6a.4. With the centre selected (no builder), houses E, F and G placed (Shift: still in the
            # mode), every site finished at its first blow (hol.Cheat.FastBuild 1, production in one step too); a right click
            # on the ghost E puts the centre's rally point on it (the flag on its middle, captured); a Peasant produced builds
            # E, goes on to F (8 m), not to G (40 m), then stands idle.
            Step "mark build rally", "hol.Cheat.FastBuild 1", "hol.Cheat.Wood 300", "hol.Cheat.Food 100", "hol.Cheat.Population 20",
                "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "hol.Select.Report", "waitlog Selection report: selected MainCentre of slot 0 (own)",
                "hol.Camera.Focus $($BuildE -join ' ')", "frames 3", "hol.Build.Place House", "waitlog Placement ghost: House at ($($BuildE -join ', ')) valid",
                "hol.Build.Click shift", "waitlog Construction: slot 0 placed House foundation at ($($BuildE -join ', '))",
                "hol.Camera.Focus $($BuildF -join ' ')", "frames 3", "waitlog Placement ghost: House at ($($BuildF -join ', ')) valid",
                "hol.Build.Click shift", "waitlog Construction: slot 0 placed House foundation at ($($BuildF -join ', '))",
                "hol.Camera.Focus $($BuildG -join ' ')", "frames 3", "waitlog Placement ghost: House at ($($BuildG -join ', ')) valid",
                "hol.Build.Click shift", "waitlog Construction: slot 0 placed House foundation at ($($BuildG -join ', '))"
            Key Escape
            Step "waitlog Placement: ended", "hol.Select.Report", "waitlog Selection report: selected MainCentre of slot 0 (own)",
                "hol.Camera.Focus $($BuildE -join ' ')", "frames 3", "hol.Select.OrderGround $($BuildE -join ' ')",
                "waitlog Rally: slot 0 set the rally point of MainCentre at ($($BuildE -join ', ')), on site House", "frames 3", "hol.Select.Report",
                "capture build_rally", "hol.Prod.Queue Peasant", "waitlog Construction: slot 0 finished House at ($($BuildE -join ', '))",
                "waitlog Construction: slot 0 finished House at ($($BuildF -join ', '))", "waitlog Build: a builder of slot 0 stops (no site left), idle", "wait 1",
                "hol.Build.Report"
            # Shift + place H, Shift + place I, Shift + right click on a tree, the newborn selected (construction 4 times as
            # fast: 5 s a house): H built, then I, then it gathers wood.
            Step "mark build queue", "hol.Cheat.FastBuild 0", "hol.Build.Speed 4", "hol.Camera.Focus $($BuildF -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Camera.Focus $($BuildH -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($BuildH -join ', ')) valid", "hol.Build.Click shift",
                "waitlog Build order of slot 0: 1 units to House at ($($BuildH -join ', ')), queued", "hol.Camera.Focus $($BuildI -join ' ')", "frames 3",
                "waitlog Placement ghost: House at ($($BuildI -join ', ')) valid", "hol.Build.Click shift",
                "waitlog Build order of slot 0: 1 units to House at ($($BuildI -join ', ')), queued"
            Key Escape
            Step "waitlog Placement: ended", "hol.Camera.Focus $($BuildTree[0]) $($BuildTree[1] + 300)", "frames 3", "hol.Select.OrderSource $($BuildTree -join ' ') shift",
                "waitlog units to gather source", "waitlog Construction: slot 0 finished House at ($($BuildH -join ', '))",
                "waitlog Construction: slot 0 finished House at ($($BuildI -join ', '))", "waitlog Build: a builder of slot 0 goes on to gather source", "wait 4",
                "hol.HUD.Report", "hol.Build.Speed 1", "hol.Build.Report", "hol.Prod.Report"
            # The construction HUD (J6a.5, D313). Every Peasant selected, no wood: the builders' grid greyed, its tooltips; a
            # click on Z (lumber camp): no ghost, "Pas assez de bois", captured in the three languages.
            Step "mark build hud", "hol.Cheat.Wood -100000", "hol.Camera.Focus $($BuildJ -join ' ')", "frames 3", "hol.Select.Own", "waitlog Select own:", "frames 3",
                "hol.HUD.Report", "capture build_hud_greyed", "hol.HUD.Click AgeCell001", "waitlog HUD message: NotEnoughWood", "frames 3", "hol.HUD.Report",
                "capture build_hud_fr"
            foreach ($Culture in "en", "ja") {
                Step "hol.UI.Language $Culture", "frames 5", "hol.HUD.Click AgeCell001", "waitlog HUD message: NotEnoughWood", "frames 3", "capture build_hud_$Culture"
            }
            Step "hol.UI.Language fr", "frames 5"
            # With wood, the key Z: the lumber camp's ghost; Escape.
            Step "mark build hud ghost", "hol.Cheat.Wood 500", "wait 0.3", "hol.HUD.Report"
            Key A
            Key Z
            Step "waitlog Placement: LumberCamp (cost", "frames 3", "hol.HUD.Report", "capture build_hud_ghost"
            Key Escape
            Step "waitlog Placement: ended"
            # The age panels and the command panel (G.1c, G.1d, G.1f): the key Z (age II, not reached) does nothing; A then A
            # by their keys (hol.Select.Grid 0, 0): the house's ghost, age I kept; then E: the mining camp's ghost instead;
            # then Z, age I chosen: the lumber camp's (never age II); Escape: the placement ended and age I closed; a click
            # on S of the command panel (the Peasants' orders): stop.
            Step "mark build hud ages", "hol.Select.Grid 1", "waitlog Grid: age 1 not reached", "frames 3", "hol.HUD.Report",
                "hol.Select.Grid 0", "hol.Select.Grid 0", "waitlog Placement: House (cost", "frames 3", "hol.HUD.Report",
                "hol.Select.Grid 2", "waitlog Placement: MiningCamp (cost", "frames 3", "hol.HUD.Report",
                "hol.Select.Grid 1", "waitlog Placement: LumberCamp (cost", "frames 3", "hol.HUD.Report"
            Key Escape
            Step "waitlog Placement: ended", "frames 3", "hol.HUD.Report", "hol.HUD.Click GridCell05", "waitlog Order: ", "frames 3"
            # Two Peasants made behind, selected by a box; the key A, a click: house J placed, they build it; the site selected:
            # its block (progress, builders, "Annuler" on V), the click on it refunds half.
            Step "mark build hud site", "hol.Units.SpawnSeed 105", "hol.Units.Spawn 2 $($BuildJSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($BuildJSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($BuildJSpawn[0] - 400) $($BuildJSpawn[1] - 400) $($BuildJSpawn[0] + 400) $($BuildJSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($BuildJ -join ' ')", "frames 3"
            Key A
            Key A
            Step "waitlog Placement ghost: House at ($($BuildJ -join ', ')) valid", "hol.Build.Click", "waitlog Build order of slot 0: 2 units to House at ($($BuildJ -join ', '))",
                "waitlog Construction: slot 0 first work on House at ($($BuildJ -join ', '))", "wait 2", "hol.Camera.Focus $($BuildJ[0]) $($BuildJ[1] - 300)", "frames 3",
                "hol.Select.ClickGround $($BuildJ -join ' ')", "waitlog Selection: House of slot 0 (own), under construction", "frames 3", "hol.HUD.Report",
                "capture build_hud_block", "hol.HUD.Click GridCell11", "waitlog Construction: slot 0 cancelled House", "waitlog Selection: none", "hol.Build.Report"
            # The population full (the cheat's places taken back): the key A at the centre, the Peasant queued waits, the message.
            Step "mark build hud population", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)",
                "hol.Cheat.Population -300", "hol.Cheat.Food 100", "wait 0.5", "hol.Prod.Report"
            Key A
            Step "waitlog Command feedback: population full", "waitlog HUD message: PopulationFull", "frames 3", "hol.HUD.Report", "capture build_hud_popfull",
                "hol.HUD.Click QueueItem0", "waitlog cancelled Peasant", "hol.Cheat.Population 300"
            # The F keys (the user's trial of J6a.5), the camera elsewhere. F1 and F3: no building of theirs, the message.
            Key Escape
            Step "mark build hud f1", "waitlog Selection: none", "hol.Build.Report", "hol.Camera.Focus 0 0", "frames 2", "hol.Camera.Report"
            Key F1
            Step "waitlog Category Military: no building", "waitlog HUD message: NoMilitaryBuilding", "frames 3", "hol.Select.Report", "hol.HUD.Report",
                "capture build_hud_f1"
            Key F3
            Step "waitlog Category Technological: no building", "waitlog HUD message: NoTechnologicalBuilding", "hol.Select.Report"
            # F2: every economic building selected, the camera kept; then three times: the centre, a house, another one.
            Step "mark build hud f2"
            Key F2
            Step "waitlog Category Economic: ", "frames 3", "hol.Camera.Report", "hol.Select.Report", "hol.HUD.Report", "capture build_hud_f2"
            for ($i = 0; $i -lt 3; $i++) {
                Key F2
                Step "waitlog Category Economic: next building", "frames 3", "hol.Camera.Report"
                if ($i -eq 1) { Step "hol.Select.Report", "capture build_hud_next" }
            }
            # Tab (the user's 2nd trial of J6a.5): the group's subgroups. « Tous » first: the centre's Peasant on A, the key A
            # queues it there; then the centre alone (the others' frames dimmer); then the houses, their ghost G among them:
            # nothing to produce. Escape, F2 anew: « Tous » again.
            Step "mark build hud tab", "hol.Cheat.Food 200", "wait 0.3", "hol.HUD.Report"
            Key A
            Step "waitlog Grid: Peasant queued at MainCentre", "frames 3", "hol.HUD.Report"
            Key Tab
            Step "waitlog Subgroup: MainCentre", "frames 3", "hol.HUD.Report", "hol.Select.Report", "capture build_hud_tab_centre"
            Key Tab
            Step "waitlog Subgroup: House x", "frames 3", "hol.HUD.Report", "hol.Select.Report", "capture build_hud_tab_houses"
            Key Escape
            Step "waitlog Selection: none"
            Key F2
            Step "waitlog Category Economic: ", "frames 3", "hol.HUD.Report"
            # The user's 3rd trial of J6a.5: on « Tous », the centre's flag (on E); a right click on the ground: the
            # group's rally point there, the flag with it.
            Step "mark build hud rally", "hol.Select.Report", "hol.Camera.Focus $($BuildRally[0] + 600) $($BuildRally[1])", "frames 3",
                "hol.Select.OrderGround $($BuildRally -join ' ')", "waitlog Rally: slot 0 set the rally point of MainCentre at (", "frames 3", "hol.Select.Report",
                "capture build_hud_rally"
            # The camps (J6b.1, D312): every Peasant of slot 0 sent behind its centre (no gatherer left), the centre's queue
            # emptied; construction twice as fast, gathering 5 times. A Peasant sent to the tree nearest lumber camp L, then
            # two Peasants place L by the wood's edge and build it; the centre's rally point on L's site, a Peasant queued
            # (12 s): L finished, its builders gather the wood and unload at L (the first Peasant at the centre until then, at
            # L after), the Peasant born walks to gather there too (its rally point on a camp finished meanwhile).
            Key Escape
            Step "mark build camp", "waitlog Selection: none", "hol.Units.SpawnSeed 106", "hol.Units.Move $($CampCrowd -join ' ') 0", "hol.Cheat.Wood 1000", "hol.Cheat.Food 500",
                "hol.Build.Speed 2", "hol.Gather.SpeedScale 5", "hol.Gather.LogDeposits 1", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5",
                "waitlog Selection: MainCentre of slot 0 (own)", "hol.Prod.Cancel 0", "hol.Prod.Cancel 0", "wait 0.5", "hol.Prod.Report",
                "hol.Units.Spawn 1 $($CampEarly -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($CampEarly -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Select.OrderSource $($CampTree -join ' ')", "waitlog Gather order of slot 0: 1 units to source",
                "hol.Units.Spawn 2 $($CampLSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($CampLSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CampLSpawn[0] - 400) $($CampLSpawn[1] - 400) $($CampLSpawn[0] + 400) $($CampLSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($CampL -join ' ')", "frames 3", "hol.Build.Place LumberCamp", "waitlog Placement ghost: LumberCamp at ($($CampL -join ', ')) valid",
                "hol.Build.Click", "waitlog Build order of slot 0: 2 units to LumberCamp at ($($CampL -join ', '))"
            Step "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)", "hol.Camera.Focus $($CampL -join ' ')", "frames 3",
                "hol.Select.OrderGround $($CampL -join ' ')", "waitlog Rally: slot 0 set the rally point of MainCentre at ($($CampL -join ', ')), on site LumberCamp",
                "hol.Prod.BuildTime 12", "hol.Prod.Queue Peasant", "waitlog Construction: slot 0 finished LumberCamp at ($($CampL -join ', '))",
                "waitlog Construction: slot 0 camp LumberCamp at ($($CampL -join ', ')) finished: ", "waitlog Production: Peasant of slot 0 done",
                "waitlog Rally: 1 units of slot 0 born for the finished camp LumberCamp", "hol.Prod.BuildTime 0", "wait 8", "hol.Units.Where 0",
                "hol.Camera.Focus $($CampL[0] + 200) $($CampL[1])", "frames 5", "hol.HUD.Report", "capture build_camp_wood"
            # Mining camp M by the stone vein between slots 0 and 4: its two builders gather the stone (the gold vein is 60 m
            # away) and unload at M.
            Key Escape
            Step "mark build camp mining", "waitlog Selection: none", "hol.Units.SpawnSeed 107", "hol.Units.Spawn 2 $($CampMSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($CampMSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CampMSpawn[0] - 400) $($CampMSpawn[1] - 400) $($CampMSpawn[0] + 400) $($CampMSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($CampM -join ' ')", "frames 3", "hol.Build.Place MiningCamp", "waitlog Placement ghost: MiningCamp at ($($CampM -join ', ')) valid",
                "hol.Build.Click", "waitlog Build order of slot 0: 2 units to MiningCamp at ($($CampM -join ', '))", "waitlog Construction: slot 0 finished MiningCamp at ($($CampM -join ', '))",
                "waitlog Construction: slot 0 camp MiningCamp at ($($CampM -join ', ')) finished: ", "wait 9", "hol.Units.Where 0",
                "hol.Camera.Focus $($CampM[0] + 400) $($CampM[1] - 300)", "frames 5", "capture build_camp_stone"
            # Mill Mi (D346) by slot 7's berries: its two builders gather the berries and unload the food at Mi.
            Key Escape
            Step "mark build camp mill", "waitlog Selection: none", "hol.Units.SpawnSeed 110", "hol.Units.Spawn 2 $($CampMillSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($CampMillSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CampMillSpawn[0] - 400) $($CampMillSpawn[1] - 400) $($CampMillSpawn[0] + 400) $($CampMillSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($CampMill -join ' ')", "frames 3", "hol.Build.Place Mill", "waitlog Placement ghost: Mill at ($($CampMill -join ', ')) valid",
                "hol.Build.Click", "waitlog Build order of slot 0: 2 units to Mill at ($($CampMill -join ', '))", "waitlog Construction: slot 0 finished Mill at ($($CampMill -join ', '))",
                "waitlog Construction: slot 0 camp Mill at ($($CampMill -join ', ')) finished: ", "wait 9", "hol.Units.Where 0",
                "hol.Camera.Focus $($CampMill[0] - 400) $($CampMill[1] - 400)", "frames 5", "capture build_camp_food"
            # A lumber camp far from every wood (30 m): no accepted source within 25 m, its two builders stand idle.
            Key Escape
            Step "mark build camp far", "waitlog Selection: none", "hol.Units.SpawnSeed 108", "hol.Units.Spawn 2 $($CampFarSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($CampFarSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CampFarSpawn[0] - 400) $($CampFarSpawn[1] - 400) $($CampFarSpawn[0] + 400) $($CampFarSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($CampFar -join ' ')", "frames 3", "hol.Build.Place LumberCamp", "waitlog Placement ghost: LumberCamp at ($($CampFar -join ', ')) valid",
                "hol.Build.Click", "waitlog Build order of slot 0: 2 units to LumberCamp at ($($CampFar -join ', '))", "waitlog Construction: slot 0 finished LumberCamp at ($($CampFar -join ', '))",
                "waitlog Construction: slot 0 camp LumberCamp at ($($CampFar -join ', ')) finished: no accepted source", "wait 1", "hol.Units.Where 0", "hol.HUD.Report",
                "hol.Camera.Focus $($CampFar[0]) $($CampFar[1] - 300)", "frames 5", "capture build_camp_idle", "hol.Build.Speed 1", "hol.Gather.SpeedScale 1", "hol.Build.Report"
            # Paths cut by a footprint (J6b.2, D314). Six Peasants gather the vein (5 times as fast: they walk most of the time)
            # and unload at the centre; 25 s later (a round trip each: their paths kept), house K placed across their way by a
            # Peasant made beside it: at its first work, the kept paths across it are dropped; once its tiles are rebuilt, the
            # walkers crossing it get a new path (the log line); watched 17 s (every body put back by a footprint logged),
            # longer than a round trip (~15 s at the D345 speed) so a batch of unloads falls in it whatever its phase.
            Key Escape
            Step "mark build cut", "waitlog Selection: none", "hol.Units.SpawnSeed 109", "hol.Gather.SpeedScale 5", "hol.Units.Spawn 6 $($CutSpawn -join ' ') 0 Peasant 100", "wait 0.5",
                "hol.Camera.Focus $($CutSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CutSpawn[0] - 350) $($CutSpawn[1] - 300) $($CutSpawn[0] + 350) $($CutSpawn[1] + 300)", "waitlog Selection: 6 units (own): Peasant x6",
                "hol.Select.OrderSource $($CutVein -join ' ')", "waitlog Gather order of slot 0: 6 units to source", "wait 25",
                "hol.Units.Spawn 1 $($CutKBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($CutK -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Build.Place House", "waitlog Placement ghost: House at ($($CutK -join ', ')) valid", "hol.Build.Click",
                "waitlog Build order of slot 0: 1 units to House at ($($CutK -join ', '))", "waitlog Construction: slot 0 first work on House at ($($CutK -join ', '))",
                "waitlog Buildings (D310): footprint House at ($($CutK -join ', '))", "hol.Nav.Footprints on", "hol.Units.KeepOut 2", "wait 1.5",
                "hol.Camera.Focus $($CutK[0]) $($CutK[1] + 200)", "frames 3", "capture build_cut_gather", "wait 15", "hol.Nav.Footprints off", "hol.Units.KeepOut 1",
                "hol.Units.Where 0"
            # Six Peasants made 50 m north of house N, sent 90 m south (straight on open ground); house N started across their
            # way by a Peasant made beside it: they go round it, arrive, none put back by a footprint. The waits are on the
            # clock, not the steps: ~16.5 s of walk, 20.5 s given (J6b.3: 10 s after the capture left 2 s, 4 of 6 arrived once).
            Key Escape
            Step "mark build cut move", "waitlog Selection: none", "hol.Units.SpawnSeed 110", "hol.Gather.SpeedScale 1", "hol.Units.LogDetours 1", "hol.Units.Spawn 6 $($CutFrom -join ' ') 0 Peasant 100", "wait 0.5",
                "hol.Camera.Focus $($CutFrom -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CutFrom[0] - 350) $($CutFrom[1] - 300) $($CutFrom[0] + 350) $($CutFrom[1] + 300)", "waitlog Selection: 6 units (own): Peasant x6",
                "hol.Camera.Focus $($CutTo -join ' ')", "frames 3", "hol.Select.OrderGround $($CutTo -join ' ')", "hol.Units.Spawn 1 $($CutNBuilder -join ' ') 0 Peasant 10", "wait 0.5",
                "hol.Camera.Focus $($CutN -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Build.Place House", "waitlog Placement ghost: House at ($($CutN -join ', ')) valid", "hol.Build.Click",
                "waitlog Build order of slot 0: 1 units to House at ($($CutN -join ', '))", "waitlog Construction: slot 0 first work on House at ($($CutN -join ', '))",
                "waitlog Buildings (D310): footprint House at ($($CutN -join ', '))", "hol.Nav.Footprints on", "hol.Units.KeepOut 2", "hol.Units.Where 0", "wait 6.5",
                "hol.Camera.Focus $($CutN -join ' ')", "frames 3", "capture build_cut_move", "wait 14", "hol.Nav.Footprints off", "hol.Units.KeepOut 1",
                "hol.Units.Where 0", "hol.Units.Orders 0", "hol.Build.Report", "hol.Units.LogDetours 0"
            # The way between two houses (J6b.2b, D324): P and Q placed touching and finished at their first blow, their two
            # builders sent away; the navmesh's paths through the way (straight, and slanting); one Peasant sent across, then
            # two together: they pass through the way, one after the other, none put back by a wall.
            Key Escape
            Step "mark build gap", "waitlog Selection: none", "hol.Units.SpawnSeed 111", "hol.Cheat.FastBuild 1", "hol.Units.Spawn 2 $($GapBuilders -join ' ') 0 Peasant 100", "wait 0.5",
                "hol.Camera.Focus $($GapBuilders -join ' ')", "frames 3",
                "hol.Select.BoxGround $($GapBuilders[0] - 400) $($GapBuilders[1] - 400) $($GapBuilders[0] + 400) $($GapBuilders[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($GapP -join ' ')", "frames 3", "hol.Build.Place House", "waitlog Placement ghost: House at ($($GapP -join ', ')) valid", "hol.Build.Click shift",
                "hol.Camera.Focus $($GapQ -join ' ')", "frames 3", "waitlog Placement ghost: House at ($($GapQ -join ', ')) valid", "hol.Build.Click shift"
            Key Escape
            Step "waitlog Construction: slot 0 finished House at ($($GapP -join ', '))", "waitlog Construction: slot 0 finished House at ($($GapQ -join ', '))",
                "waitlog Buildings (D310): footprint House at ($($GapQ -join ', '))", "hol.Cheat.FastBuild 0", "hol.Camera.Focus $($GapBuilders[0]) $($GapBuilders[1] - 1000)", "frames 3",
                "hol.Select.OrderGround $($GapBuilders[0]) $($GapBuilders[1] - 1000)", "wait 3", "hol.Nav.Path $($GapFrom -join ' ') $($GapTo -join ' ')",
                "hol.Nav.Path $($GapFrom[0]) $($GapFrom[1] - 200) $($GapTo[0]) $($GapTo[1] + 200)"
            Key Escape
            Step "mark build gap one", "waitlog Selection: none", "hol.Units.SpawnSeed 112", "hol.Units.Spawn 1 $($GapFrom -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($GapFrom -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Camera.Focus $($GapP[0]) $($GapTo[1])", "frames 3", "hol.Nav.Footprints on", "hol.Units.KeepOut 2",
                "hol.Select.OrderGround $($GapTo -join ' ')"
            for ($i = 0; $i -lt 14; $i++) { Step "wait 0.2", "hol.Units.Where 0"; if ($i -eq 6) { Step "capture build_gap_one" } }
            Step "wait 4", "hol.Units.Where 0", "hol.Nav.Footprints off"
            Key Escape
            Step "mark build gap two", "waitlog Selection: none", "hol.Units.SpawnSeed 113", "hol.Units.Spawn 2 $($GapFrom -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($GapFrom -join ' ')", "frames 3",
                "hol.Select.BoxGround $($GapFrom[0] - 400) $($GapFrom[1] - 400) $($GapFrom[0] + 400) $($GapFrom[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($GapP[0]) $($GapTo[1])", "frames 3", "hol.Nav.Footprints on", "hol.Select.OrderGround $($GapTo -join ' ')"
            for ($i = 0; $i -lt 16; $i++) { Step "wait 0.2", "hol.Units.Where 0"; if ($i -eq 7) { Step "capture build_gap_two" } }
            Step "wait 4", "hol.Units.Where 0", "hol.Nav.Footprints off", "hol.Units.KeepOut 1"
            # Measure only (no check): one Peasant sent along P's south wall, 20 cm inside its tiles; the footprint watch logs how
            # close its body comes to P's body (the middle of a Peasant at the visible wall: 85 - 60 = 25 cm at BodyInset 115).
            Key Escape
            Step "mark build wall", "waitlog Selection: none", "hol.Units.SpawnSeed 114", "hol.Units.Spawn 1 $($GapP[0] - 1000) $GapWallY 0 Peasant 10", "wait 0.5",
                "hol.Camera.Focus $($GapP[0] - 1000) $GapWallY", "frames 3", "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #",
                "hol.Camera.Focus $($GapP[0]) $GapWallY", "frames 3", "hol.Nav.Footprints on", "hol.Select.OrderGround $($GapP[0] + 1000) $GapWallY"
            for ($i = 0; $i -lt 10; $i++) { Step "wait 0.25", "hol.Units.Where 0"; if ($i -eq 4) { Step "hol.Camera.Zoom 12", "frames 2", "capture build_wall_one", "hol.Camera.Zoom -12" } }
            Step "wait 3", "hol.Units.Where 0", "hol.Nav.Footprints off"
            # The walkers taken away (hol.Units.Remove): the wall's Peasant stood 8.5 m from the attack-move's Spearman (its
            # aggression drew it off the Men-at-arms' way), the gap's three 15 m from it. Waited for without their counts
            # (checked by the analysis of "build gap": a wrong count must not stop the rest of the sequence).
            # Escape first, while the wall's Peasant is still selected (taken away selected, the selection's "none" may come
            # before the Escape that waits for it).
            Key Escape
            Step "waitlog Selection: none", "hol.Units.Remove $($GapP[0] + 1000) $GapWallY 300 0", "waitlog units of slot 0 within 300 of ($($GapP[0] + 1000), $GapWallY)",
                "hol.Units.Remove $($GapTo -join ' ') 400 0", "waitlog units of slot 0 within 400 of ($($GapTo -join ', '))", "frames 3"
            # Close up: the two houses in their tiles, and the ghost of a third one touching Q (its tiles, its body inside them).
            Step "hol.Camera.Zoom 12", "hol.Build.Place House", "hol.Camera.Focus $($GapQ[0]) $($GapQ[1] + 800)",
                "waitlog Placement ghost: House at ($($GapQ[0]), $($GapQ[1] + 800)) valid", "frames 3", "capture build_gap_houses"
            Key Escape
            Step "waitlog Placement: ended", "hol.Camera.Zoom -12"
            # The barracks (J7a.2). Three Peasants made beside its spot, selected by a box: R on their grid (captured); the key R,
            # the ghost, a click: they build it (4 times as fast). The barracks selected: Lancier on A, Homme d'armes on Z
            # (captured); its rally point by a right click; A, Z then A again (2 s each; the Archer went to the Archery range, AR-Q2):
            # all three born, they walk to the rally point
            # (captured). All three sent onto the stone vein: they walk there (they do not gather). F1: the barracks selected.
            Step "mark build barracks", "hol.Units.SpawnSeed 115", "hol.Cheat.Wood 300", "hol.Cheat.Food 500", "hol.Cheat.Gold 100", "hol.Cheat.Population 20", "hol.Build.Speed 4",
                "hol.Units.Spawn 3 $($BarracksSpawn -join ' ') 0 Peasant 150", "wait 0.5", "hol.Camera.Focus $($BarracksSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($BarracksSpawn[0] - 400) $($BarracksSpawn[1] - 400) $($BarracksSpawn[0] + 400) $($BarracksSpawn[1] + 400)", "waitlog Selection: 3 units (own): Peasant x3",
                "hol.Camera.Focus $($BarracksAt -join ' ')", "frames 3", "hol.HUD.Report", "capture build_barracks_peasants"
            Key A
            Key Q
            Step "waitlog Placement ghost: Barracks at ($($BarracksAt -join ', ')) valid", "frames 3", "capture build_barracks_ghost", "hol.Build.Click",
                "waitlog Build order of slot 0: 3 units to Barracks at ($($BarracksAt -join ', '))", "waitlog Construction: slot 0 finished Barracks at ($($BarracksAt -join ', '))",
                "wait 1", "hol.Build.Speed 1", "hol.Build.Report", "hol.Camera.Focus $($BarracksAt[0]) $($BarracksAt[1] - 300)", "frames 3",
                "hol.Select.ClickGround $($BarracksAt -join ' ')", "waitlog Selection: Barracks of slot 0 (own)", "frames 3", "hol.HUD.Report", "capture build_barracks_finished",
                "hol.Camera.Focus $($BarracksRally -join ' ')", "frames 3", "hol.Select.OrderGround $($BarracksRally -join ' ')",
                "waitlog Rally: slot 0 set the rally point of Barracks at (", "hol.Prod.BuildTime 2", "hol.Prod.Report"
            Key A
            Step "waitlog Production: Spearman of slot 0 done"
            Key Z
            Step "waitlog Production: ManAtArms of slot 0 done"
            Key A
            Step "waitlog Production: Spearman of slot 0 done", "hol.Prod.BuildTime 0", "wait 9", "hol.Units.Where 0", "hol.Prod.Report", "frames 3", "capture build_barracks_soldiers"
            Key Escape
            Step "mark build barracks orders", "waitlog Selection: none",
                "hol.Select.BoxGround $($BarracksRally[0] - 500) $($BarracksRally[1] - 500) $($BarracksRally[0] + 500) $($BarracksRally[1] + 500)", "waitlog Selection: 3 units (own)",
                "hol.Camera.Focus $($CutVein[0]) $($CutVein[1] - 300)", "frames 3", "hol.Select.OrderSource $($CutVein -join ' ')", "waitlog units that do not work walk to", "wait 1",
                "hol.Camera.Focus $($BarracksRally -join ' ')", "frames 3", "hol.Select.OrderGround $($BarracksRally -join ' ')", "wait 1"
            Key Escape
            Step "mark build barracks f1", "waitlog Selection: none", "hol.Camera.Focus 0 0", "frames 2"
            Key F1
            Step "waitlog Category Military: ", "frames 3", "hol.Select.Report", "hol.HUD.Report", "capture build_barracks_f1"
            Key Escape
            Step "waitlog Selection: none"
            # The rally point of a ghost (the user's request of 2026-10-19). A barracks placed with nothing selected: a ghost,
            # selected by a click; a right click on the ground puts its rally point there (the flag, captured); every site
            # finished at its first blow and production in one step (hol.Cheat.FastBuild 1): a Peasant made west of it builds
            # it; a Homme d'armes produced there walks to the rally point kept since the ghost.
            Step "mark build barracks ghost", "hol.Units.SpawnSeed 116", "hol.Cheat.Wood 300", "hol.Cheat.Food 500", "hol.Cheat.Gold 100", "hol.Cheat.Population 20",
                "hol.Camera.Focus $($BarracksGhostAt -join ' ')", "frames 3", "hol.Build.Place Barracks",
                "waitlog Placement ghost: Barracks at ($($BarracksGhostAt -join ', ')) valid", "hol.Build.Click",
                "waitlog Construction: slot 0 placed Barracks foundation at ($($BarracksGhostAt -join ', '))", "wait 0.5",
                "hol.Camera.Focus $($BarracksGhostAt[0]) $($BarracksGhostAt[1] - 300)", "frames 3", "hol.Select.ClickGround $($BarracksGhostAt -join ' ')",
                "waitlog Selection: Barracks of slot 0 (own)", "hol.Camera.Focus $($BarracksGhostRally -join ' ')", "frames 3",
                "hol.Select.OrderGround $($BarracksGhostRally -join ' ')", "waitlog Rally: slot 0 set the rally point of Barracks at (", "frames 3",
                "hol.Select.Report", "capture build_barracks_ghost_rally", "hol.Cheat.FastBuild 1",
                "hol.Units.Spawn 1 $($BarracksGhostSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($BarracksGhostSpawn -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Camera.Focus $($BarracksGhostAt[0]) $($BarracksGhostAt[1] - 300)", "frames 3",
                "hol.Select.OrderGround $($BarracksGhostAt -join ' ')", "waitlog Build order of slot 0: 1 units to Barracks at ($($BarracksGhostAt -join ', '))",
                "waitlog Construction: slot 0 finished Barracks at ($($BarracksGhostAt -join ', '))", "wait 0.5",
                "hol.Select.ClickGround $($BarracksGhostAt -join ' ')", "waitlog Selection: Barracks of slot 0 (own)", "hol.Prod.Queue ManAtArms",
                "waitlog Production: ManAtArms of slot 0 done", "hol.Cheat.FastBuild 0", "wait 9", "hol.Units.Where 0", "hol.Prod.Report",
                "hol.Camera.Focus $($BarracksGhostRally -join ' ')", "frames 3", "capture build_barracks_ghost_soldier"
            Key Escape
            # The farm (IA1.2 F9): B built by one Peasant (its field born, it gathers it), then sent away: B's field free.
            Step "mark build farm", "waitlog Selection: none", "hol.Units.SpawnSeed 142", "hol.Cheat.Wood 150", "hol.Build.Speed 4", "hol.Gather.SpeedScale 5",
                "hol.Gather.LogDeposits 1", "hol.Units.Spawn 1 $($FarmBBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($FarmBBuilder -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Camera.Focus $($FarmB -join ' ')", "frames 3", "hol.Build.Place Farm",
                "waitlog Placement ghost: Farm at ($($FarmB -join ', ')) valid", "hol.Build.Click", "waitlog Build order of slot 0: 1 units to Farm at ($($FarmB -join ', '))",
                "waitlog Construction: slot 0 finished Farm at ($($FarmB -join ', '))", "wait 1",
                # Its builder still selected (a click on it again logs no new selection): sent away.
                "hol.Camera.Focus $($FarmAway -join ' ')", "frames 3", "hol.Select.OrderGround $($FarmAway -join ' ')", "wait 1"
            Key Escape
            # A built by two Peasants: the first gathers its field, the second B's (the free field nearest, F6); both unload
            # their food at Mi (24 m).
            Step "mark build farm a", "waitlog Selection: none", "hol.Camera.Focus $($FarmABuilders -join ' ')", "frames 3",
                # One by one, 3 m apart: two born together overlap and a click on one picks the other.
                "hol.Units.Spawn 1 $($FarmABuilders -join ' ') 0 Peasant 10", "wait 0.5", "frames 3", "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #",
                "hol.Units.Spawn 1 $($FarmABuilders[0] + 300) $($FarmABuilders[1]) 0 Peasant 10", "wait 0.5", "frames 3", "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #",
                "hol.Select.ClickUnit -2 add", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($FarmA -join ' ')", "frames 3", "hol.Build.Place Farm", "waitlog Placement ghost: Farm at ($($FarmA -join ', ')) valid",
                "hol.Build.Click", "waitlog Build order of slot 0: 2 units to Farm at ($($FarmA -join ', '))", "waitlog Construction: slot 0 finished Farm at ($($FarmA -join ', '))",
                "wait 12", "hol.Units.Where 0", "hol.Camera.Focus $($FarmA -join ' ')", "frames 5", "capture build_farm_gather"
            Key Escape
            # A third Peasant, a right click on A (OrderAt): its field ordered; its one place and B's taken, no food within 20 m:
            # it stands idle in front of it (F3: FindMassifSpot finds nothing, StandBeforeSource).
            Step "mark build farm click", "waitlog Selection: none", "hol.Units.Spawn 1 $($FarmCSpawn -join ' ') 0 Peasant 10", "wait 0.5",
                "hol.Camera.Focus $($FarmCSpawn -join ' ')", "frames 3", "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Camera.Focus $($FarmA -join ' ')", "frames 3",
                "hol.Select.OrderGround $($FarmClick -join ' ')", "waitlog Order: on farm ", "wait 5", "hol.Units.Where 0", "frames 3", "capture build_farm_click"
            Key Escape
            # A walker across both farms (D312: walked over): the navmesh's path straight, no detour, it arrives (44 m, ~5.5 s).
            Step "mark build farm cross", "waitlog Selection: none", "hol.Units.LogDetours 1", "hol.Nav.Path $($FarmCrossFrom -join ' ') $($FarmCrossTo -join ' ')",
                "hol.Units.Spawn 1 $($FarmCrossFrom -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($FarmCrossFrom -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Camera.Focus $($FarmA[0]) $($FarmA[1] - 1200)", "frames 3", "hol.Select.OrderGround $($FarmCrossTo -join ' ')", "hol.Units.Where 0"
            for ($i = 0; $i -lt 8; $i++) { Step "wait 1", "hol.Units.Where 0" }
            Key Escape
            # Put back as before: the speeds; the farms' units taken away (the farms stay, their fields with no Peasant).
            Step "waitlog Selection: none", "hol.Units.LogDetours 0", "hol.Build.Speed 1", "hol.Gather.SpeedScale 1",
                "hol.Units.Remove $($FarmCrossTo -join ' ') 400 0", "waitlog units of slot 0 within 400 of ($($FarmCrossTo -join ', '))",
                "hol.Units.Remove $($FarmAway -join ' ') 400 0", "waitlog units of slot 0 within 400 of ($($FarmAway -join ', '))",
                "hol.Units.Remove $($FarmA[0]) $($FarmA[1] - 400) 1500 0", "waitlog units of slot 0 within 1500 of ($($FarmA[0]), $($FarmA[1] - 400))", "frames 3"
            # F4 (D30, J8a-Q4): the player's hero alone. None yet: nothing selected, the camera kept. Then a hero of slot 0
            # behind its centre and one of slot 1 behind the enemy's: F4 selects slot 0's alone, the camera kept; again,
            # the camera on him. Both removed after (the parts after count slot 0's units).
            Step "mark build hero", "hol.Camera.Focus 0 0", "frames 2", "hol.Camera.Report"
            Key F4
            # J8a.5 loop 2: the hero's block of the game screen in script, hidden without a hero, then his level and HP.
            Step "waitlog Select hero: none", "frames 2", "hol.Select.Report", "hol.Camera.Report", "hol.HUD.Report",
                "hol.Units.Spawn 1 $($BuildHero -join ' ') 0 Hero 100", "hol.Units.Spawn 1 $($BuildEnemyHero -join ' ') 1 Hero 100", "wait 0.5"
            Key F4
            Step "waitlog Select hero: selected", "frames 3", "hol.Select.Report", "hol.Camera.Report", "hol.HUD.Report", "frames 3", "capture build_hero_block"
            Key F4
            Step "waitlog Select hero: camera on", "frames 3", "hol.Camera.Report", "hol.Select.Report"
            Key Escape
            # J8a.5 loop 4: the block during the death and the return, the times at 20 % (hol.Hero.ReturnTimePercent: ready
            # 152 steps after a level 1 death, the price from 150 to 0 in 240 steps), the gold set to 500 (hol.Cheat.Gold adds:
            # -1000000 empties it). Dead: the delay, twice (going down); ready: the price buyable, no gold with the stock
            # emptied, buyable again; the return's button clicked: back (bought; without the button, by himself at price 0 at
            # worst 12 s later), the gold lowered by the price the server took (hol.Hero.Report before and after). Then the
            # portrait clicked: selects him; again: the camera on him (as F4). Both heroes removed (the reborn one by the forum),
            # the times back to the rules, the centre selected then Escape (a known "Selection: none" for the cases after).
            Step "waitlog Selection: none", "mark build hero return", "hol.Hero.ReturnTimePercent 20", "hol.Cheat.Gold -1000000", "hol.Cheat.Gold 500",
                "hol.Hero.Kill 0", "waitlog hol.Hero.Kill: slot 0", "wait 0.5", "hol.HUD.Report", "frames 3", "capture build_hero_dead", "wait 1", "hol.HUD.Report",
                "waitlog Hero return: slot 0 ready", "frames 3", "hol.HUD.Report", "capture build_hero_ready",
                "hol.Cheat.Gold -1000000", "frames 3", "hol.HUD.Report", "hol.Cheat.Gold 500", "frames 3", "hol.HUD.Report", "hol.Hero.Report",
                "hol.HUD.Click HeroReturnButton", "waitlog Hero return: slot 0 by its main forum", "wait 0.5", "hol.HUD.Report", "hol.Hero.Report",
                "hol.HUD.Click HeroPortrait", "frames 3", "hol.Select.Report", "hol.Camera.Report", "hol.HUD.Click HeroPortrait", "frames 3", "hol.Camera.Report", "hol.Select.Report",
                "hol.Hero.ReturnTimePercent 100", "hol.Units.Remove 0 0 60000 0 Hero", "hol.Units.Remove $($BuildEnemyHero -join ' ') 1000 1 Hero", "wait 0.3",
                "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)"
            Key Escape
            Step "waitlog Selection: none"
            # J8a.5 loop 7b: the block's texts in English and Japanese (NSLOCTEXT of the script, gathered from Script/ by
            # GatherText, translated by fill_po.py): his hero again, alive (level, HP) in en then ja; killed (times at 20 %):
            # the delay in en then ja; ready: the button in en, the gold emptied (its tooltip) in en then ja, given back in ja.
            # Back to French, the return bought, the hero removed, the centre selected then Escape (as the case before).
            Step "mark build hero texts", "hol.Units.Spawn 1 $($BuildHero -join ' ') 0 Hero 100", "wait 0.5",
                "hol.UI.Language en", "frames 3", "hol.HUD.Report", "hol.UI.Language ja", "frames 3", "hol.HUD.Report", "capture build_hero_block_ja",
                "hol.Hero.ReturnTimePercent 20", "hol.Cheat.Gold -1000000", "hol.Cheat.Gold 500", "hol.Hero.Kill 0", "waitlog hol.Hero.Kill: slot 0", "wait 0.5",
                "hol.UI.Language en", "frames 3", "hol.HUD.Report", "hol.UI.Language ja", "frames 3", "hol.HUD.Report",
                "waitlog Hero return: slot 0 ready", "hol.UI.Language en", "frames 3", "hol.HUD.Report", "capture build_hero_ready_en",
                "hol.Cheat.Gold -1000000", "frames 3", "hol.HUD.Report", "hol.UI.Language ja", "frames 3", "hol.HUD.Report",
                "hol.Cheat.Gold 500", "frames 3", "hol.HUD.Report", "hol.UI.Language fr", "frames 3",
                "hol.HUD.Click HeroReturnButton", "waitlog Hero return: slot 0 by its main forum", "wait 0.5",
                "hol.Hero.ReturnTimePercent 100", "hol.Units.Remove 0 0 60000 0 Hero", "wait 0.3",
                "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)"
            Key Escape
            Step "waitlog Selection: none"
            # J8b.2 loop 3: the hero's XP in a match (J8b-Q1, Q2, Q5). His hero again at 150 XP (hol.Hero.SetXP), a Peasant of
            # slot 1 made beside him: killed by him (combat +50, 200), level 2, the HP gained added. Killed (times at 20 %:
            # ready 184 steps after a level 2 death), a House finished meanwhile by a Peasant (FastBuild): development +25 though
            # dead (D06). The return bought: back at level 2, full health (486). Then the hero and the builder removed, his XP
            # back to 0, the times and the combat as before, the centre selected then Escape (as the cases before).
            # J8b.4: the camera on him, his golden flash captured at his level up, the HUD's reports (his XP's bar, the thread's
            # alerts); once back, his XP to 600: level 3, the alert of tier 1, captured.
            Step "mark build hero xp", "hol.Combat.Enabled 1", "hol.Cheat.Wood 1000", "hol.Camera.Focus $($BuildHero -join ' ')", "frames 3",
                "hol.Units.Spawn 1 $($BuildHero -join ' ') 0 Hero 100", "wait 0.5",
                "hol.Hero.SetXP 0 150", "hol.Units.Spawn 1 $($BuildHero[0] + 200) $($BuildHero[1]) 1 Peasant 10", "waitlog Hero level: slot 0 level 1 -> 2",
                "waitlog Hero flash: slot 0 level 1 -> 2", "wait 0.25", "capture build_hero_flash", "wait 0.3", "hol.Hero.Report", "hol.HUD.Report",
                "hol.Hero.ReturnTimePercent 20", "hol.Cheat.Gold -1000000", "hol.Cheat.Gold 500", "hol.Hero.Kill 0", "waitlog hol.Hero.Kill: slot 0", "wait 0.5",
                "hol.Cheat.FastBuild 1", "hol.Units.Spawn 1 $($HeroXPBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($HeroXPBuilder -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Camera.Focus $($HeroXPHouse -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($HeroXPHouse -join ', ')) valid", "hol.Build.Click", "waitlog Construction: slot 0 finished House at ($($HeroXPHouse -join ', '))",
                "hol.Cheat.FastBuild 0", "wait 0.3", "hol.Hero.Report",
                "waitlog Hero return: slot 0 ready", "hol.HUD.Click HeroReturnButton", "waitlog Hero return: slot 0 by its main forum", "wait 0.5", "hol.Hero.Report",
                "hol.HUD.Report", "hol.Hero.SetXP 0 600", "waitlog Hero flash: slot 0 level 2 -> 3", "wait 0.5", "hol.HUD.Report", "frames 3", "capture build_hero_tier",
                "hol.UI.Language en", "hol.Hero.SetXP 0 1300", "waitlog Hero flash: slot 0 level 3 -> 4", "wait 0.3", "hol.HUD.Report",
                "hol.UI.Language ja", "hol.Hero.SetXP 0 3300", "waitlog Hero flash: slot 0 level 4 -> 6", "wait 0.3", "hol.HUD.Report", "frames 3", "capture build_hero_tier_ja",
                "hol.UI.Language fr", "frames 3",
                "hol.Hero.ReturnTimePercent 100", "hol.Units.Remove 0 0 60000 0 Hero", "hol.Units.Remove $($HeroXPHouse -join ' ') 1500 0", "hol.Hero.SetXP 0 0",
                "hol.Combat.Enabled 0", "wait 0.3", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)"
            Key Escape
            Step "waitlog Selection: none"
            # J8c.2 loop 2: the hero's aura, cry and banner in a match (J8c-Q1 to Q3), the hits logged (hol.Combat.LogHits). His
            # hero again, a Spearman of slot 0 6 m from him, a Man-at-arms of slot 1 beside it and an Archer of slot 1 15 m off
            # (21 m from him): -15 %, the Man-at-arms' 12 on the Spearman 10.2 (on the hero, D17, 3.06), the Archer's 10 on it
            # (5 and its bonus against light infantry) 8.5. Those gone, a new Spearman cried over with him (Serrez les rangs),
            # the cry refused again (its cool-down), then hit: -40 %, 7.2 (on the hero 2.16) and arrows of 6. His XP to
            # level 4, his banner refused 17 m from him, planted 16 m away; a Spearman 6 m beyond it (22 m from him) hit by a
            # Man-at-arms: 10.2. Then all removed, his abilities reset (the banner gone), his XP back to 0, the centre selected
            # then Escape (as the cases before).
            $AuraHero = $BuildHero
            $AuraSpot = @($AuraHero[0], ($AuraHero[1] + 600))
            Step "mark build hero aura", "hol.Combat.Enabled 1", "hol.Combat.LogHits 1", "hol.Camera.Focus $($AuraHero -join ' ')", "frames 3",
                "hol.Units.Spawn 1 $($AuraHero -join ' ') 0 Hero 100", "wait 0.5",
                "hol.Units.Spawn 1 $($AuraSpot -join ' ') 0 Spearman 10", "hol.Units.Spawn 1 $($AuraSpot[0] + 150) $($AuraSpot[1]) 1 ManAtArms 10",
                "hol.Units.Spawn 1 $($AuraSpot[0]) $($AuraSpot[1] + 1500) 1 Archer 10", "waitlog ManAtArms hits #", "wait 3",
                "hol.Units.Remove $($AuraHero -join ' ') 4000 1", "hol.Units.Remove $($AuraHero -join ' ') 4000 0 Spearman", "wait 0.3",
                "hol.Units.Spawn 1 $($AuraSpot -join ' ') 0 Spearman 10", "wait 0.3", "hol.Hero.CloseRanks 0", "waitlog cries Serrez les rangs,",
                "hol.Hero.CloseRanks 0", "hol.Units.Spawn 1 $($AuraSpot[0] + 150) $($AuraSpot[1]) 1 ManAtArms 10",
                "hol.Units.Spawn 1 $($AuraSpot[0]) $($AuraSpot[1] + 1500) 1 Archer 10", "waitlog ManAtArms hits #", "wait 3",
                "hol.Units.Remove $($AuraHero -join ' ') 4000 1", "hol.Units.Remove $($AuraHero -join ' ') 4000 0 Spearman", "wait 0.3",
                "hol.Hero.SetXP 0 1300", "waitlog Hero level: slot 0 level 1 -> 4",
                "hol.Hero.Banner 0 $($AuraHero[0] + 1700) $($AuraHero[1])", "hol.Hero.Banner 0 $($AuraHero[0] + 1600) $($AuraHero[1])", "waitlog plants his banner",
                "hol.Units.Spawn 1 $($AuraHero[0] + 2200) $($AuraHero[1]) 0 Spearman 10", "hol.Units.Spawn 1 $($AuraHero[0] + 2350) $($AuraHero[1]) 1 ManAtArms 10",
                "waitlog Spearman for 10.2", "wait 0.5",
                "hol.Units.Remove $($AuraHero -join ' ') 5000 1", "hol.Units.Remove $($AuraHero -join ' ') 5000 0 Spearman", "hol.Units.Remove 0 0 60000 0 Hero",
                "hol.Hero.ResetAbilities 0", "hol.Hero.SetXP 0 0", "hol.Combat.LogHits 0", "hol.Combat.Enabled 0", "wait 0.3",
                "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)"
            Key Escape
            Step "waitlog Selection: none"
            # J8c.4 loop 1: the hero's abilities from the grid (J8c-Q4), as the keys. His hero again, selected (F4): Z refused
            # by the grid at level 1 (the banner opens at 4), A cries, A again refused by the grid (its cool-down, seconds
            # told); at level 4 Z then a left click 10 m from him plants the banner (hol.Select.Banner). Then removed, his
            # abilities reset, his XP back to 0, the centre selected then Escape. J8c.4 loop 2: the HUD read (hol.HUD.Report) once
            # he is selected, after the cry, at level 4 and once the banner is planted: the grid's A and Z cells, their
            # tooltips, the hero's block's three boxes. J8c.4 loop 3: the aura circles drawn for the player (hol.Select.Report,
            # "Aura rings:"): his hero's once selected; at level 4 the banner aimed (Z) with the test cursor 10 m from him (its
            # reach and its aura there, in reach), then 18 m (out of reach); once planted, the hero's and the banner's.
            Step "mark build hero grid", "hol.Camera.Focus $($AuraHero -join ' ')", "frames 3",
                "hol.Units.Spawn 1 $($AuraHero -join ' ') 0 Hero 100", "wait 0.5", "hol.Select.Hero", "waitlog Select hero: selected #", "wait 0.3", "hol.HUD.Report",
                "hol.Select.Report",
                "hol.Select.Grid 1", "hol.Camera.Zoom 12", "frames 3", "hol.Select.Grid 0", "waitlog Cry flash: slot 0,", "frames 2", "capture build_hero_cry", "hol.Camera.Zoom -12", "waitlog cries Serrez les rangs,", "wait 0.3", "hol.HUD.Report", "hol.Select.Grid 0",
                "hol.Hero.SetXP 0 1300", "waitlog Hero level: slot 0 level 1 -> 4", "wait 0.3", "hol.HUD.Report",
                "hol.Select.Grid 1", "hol.Camera.TestCursorGround $($AuraHero[0] + 1000) $($AuraHero[1])", "frames 3", "hol.Select.Report", "capture build_hero_aim",
                "hol.Camera.TestCursorGround $($AuraHero[0] + 1800) $($AuraHero[1])", "frames 3", "hol.Select.Report", "capture build_hero_aim_far", "hol.Camera.TestCursor off",
                "hol.Select.Banner $($AuraHero[0] + 1000) $($AuraHero[1])", "waitlog plants his banner", "wait 0.3", "hol.HUD.Report", "hol.Select.Report", "hol.Hero.BannerDisplay", "capture build_hero_banner_rings", "hol.Camera.Focus $($AuraHero[0] + 1000) $($AuraHero[1])", "hol.Camera.Zoom 12", "frames 3", "capture build_hero_banner", "hol.Camera.Zoom -12", "hol.Camera.Focus $($AuraHero -join ' ')",
                "hol.Units.Remove 0 0 60000 0 Hero", "hol.Hero.ResetAbilities 0", "hol.Hero.SetXP 0 0", "wait 0.3",
                "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)"
            Key Escape
            Step "waitlog Selection: none"
            # A right click on a finished deposit (L.1, D485): lumber camp D built at its first blow; three Peasants gather
            # the tree nearest CampTree for a few seconds (partly loaded, the forum nearer them than D), a fourth made empty
            # beside them; the four selected through group 3 (a box near the wood would take the camp case's gatherers too), a right click on D's centre: the three unload their wood at D (not at the
            # forum), then stand idle in front of it; the empty one walks to the point clicked, idle. Then all removed.
            # D, shared by the cases dropoff, dropoffshift and dropoffmove (each played alone builds it).
            Step "mark build dropoff camp", "hol.Units.SpawnSeed 118", "hol.Cheat.Wood 1000", "hol.Gather.LogDeposits 1", "hol.Gather.SpeedScale 1", "hol.Cheat.FastBuild 1",
                "hol.Units.Spawn 1 $($DropBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($DropBuilder -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Camera.Focus $($DropD -join ' ')", "frames 3", "hol.Build.Place LumberCamp",
                "waitlog Placement ghost: LumberCamp at ($($DropD -join ', ')) valid", "hol.Build.Click", "waitlog Construction: slot 0 finished LumberCamp at ($($DropD -join ', '))",
                "hol.Cheat.FastBuild 0"
            Step "mark build dropoff", "hol.Units.SpawnSeed 118", "hol.Gather.SpeedScale 1", "hol.Cheat.FastBuild 0", "hol.Units.Spawn 3 $($DropGatherers -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($DropGatherers -join ' ')", "frames 3",
                "hol.Select.BoxGround $($DropGatherers[0] - 400) $($DropGatherers[1] - 400) $($DropGatherers[0] + 400) $($DropGatherers[1] + 400)", "waitlog Selection: 3 units (own): Peasant x3", "hol.Select.Group Add 3", "waitlog Group 3: created, 3 units",
                "hol.Select.OrderSource $($CampTree -join ' ')", "waitlog Gather order of slot 0: 3 units to source", "wait 5",
                "hol.Units.Spawn 1 $($DropEmpty -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($DropGatherers -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Select.Group Add 3", "waitlog Group 3: added, 4 units",
                "hol.Select.Group Select 3", "waitlog Group 3: selected, 4 units"
            # The order, then where they are 12 s later (~66 m to walk) and 3 s after that (still idle in front of D).
            Step "mark build dropoff order", "hol.Units.Where 0", "hol.Camera.Focus $($DropD -join ' ')", "frames 3", "hol.Select.OrderGround $($DropD -join ' ')", "wait 12",
                "hol.Units.Where 0", "wait 3", "hol.Units.Where 0", "hol.Units.Remove $($DropD -join ' ') 1500 0", "wait 0.5"
            # Shift + right click on D (L.3, D485): a Peasant gathers the tree nearest CampTree for 5 s (partly loaded), a
            # second made empty beside it; both selected, house S placed (no FastBuild: it lasts), then Shift + right click
            # on D: the carrier builds S with the other, and only then unloads its wood at D (queued after S), idle there;
            # the empty one finishes S and is idle (Shift on D for it: queued walk to the point clicked). Then all removed.
            Step "mark build dropoffshift", "hol.Units.SpawnSeed 119", "hol.Gather.SpeedScale 1", "hol.Cheat.FastBuild 0",
                "hol.Units.Spawn 1 $($DropGatherers -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($DropGatherers -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Select.Group Add 4", "waitlog Group 4: created, 1 units",
                "hol.Select.OrderSource $($CampTree -join ' ')", "waitlog Gather order of slot 0: 1 units to source", "wait 5",
                "hol.Units.Spawn 1 $($DropEmpty -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($DropGatherers -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Select.Group Add 4", "waitlog Group 4: added, 2 units",
                "hol.Select.Group Select 4", "waitlog Group 4: selected, 2 units"
            # S placed, Shift + right click on D; where they are at S's first work, when it is finished, then 10 s and 13 s after.
            Step "mark build dropoffshift order", "hol.Units.Where 0", "hol.Camera.Focus $($DropShiftHouse -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($DropShiftHouse -join ', ')) valid", "hol.Build.Click",
                "waitlog Build order of slot 0: 2 units to House at ($($DropShiftHouse -join ', '))", "hol.Camera.Focus $($DropD -join ' ')", "frames 3",
                "hol.Select.OrderGround $($DropD -join ' ') shift", "waitlog Construction: slot 0 first work on House at ($($DropShiftHouse -join ', '))", "wait 1",
                "hol.Units.Where 0", "waitlog Construction: slot 0 finished House at ($($DropShiftHouse -join ', '))", "hol.Units.Where 0", "wait 10", "hol.Units.Where 0",
                "wait 3", "hol.Units.Where 0", "hol.Units.Remove $($DropD -join ' ') 1500 0", "hol.Units.Remove $($DropShiftHouse -join ' ') 1500 0", "wait 0.5"
            # Shift moves round a Shift + right click on D (L.5, D485, the user's trial): a Peasant gathers the tree nearest
            # CampTree for 5 s (partly loaded), alone in group 5. A: Shift + right click on the ground at P, then Shift +
            # right click on D: it walks to P first, then unloads at D, idle there. B: it gathers again, then Shift + D, then
            # Shift + ground at B: it unloads at D first, then walks to B, idle there. Then all removed.
            Step "mark build dropoffmove", "hol.Units.SpawnSeed 120", "hol.Gather.SpeedScale 1", "hol.Cheat.FastBuild 0",
                "hol.Units.Spawn 1 $($DropGatherers -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($DropGatherers -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Select.Group Add 5", "waitlog Group 5: created, 1 units",
                "hol.Select.OrderSource $($CampTree -join ' ')", "waitlog Gather order of slot 0: 1 units to source", "wait 5"
            # A: where it is before the orders, every 0.25 s for 6 s after them (P is ~3 s away), then 18 s and 21 s after.
            Step "mark build dropoffmove order", "hol.Units.Where 0", "hol.Camera.Focus $($DropMoveP -join ' ')", "frames 3", "hol.Select.OrderGround $($DropMoveP -join ' ') shift",
                "hol.Camera.Focus $($DropD -join ' ')", "frames 3", "hol.Select.OrderGround $($DropD -join ' ') shift"
            for ($i = 0; $i -lt 24; $i++) { Step "wait 0.25", "hol.Units.Where 0" }
            Step "wait 12", "hol.Units.Where 0", "wait 3", "hol.Units.Where 0"
            # B: back to the tree (~60 m) and 5-8 s of gathering, then Shift + D and Shift + B; where it is every second for
            # 22 s (D ~60 m away, B 15 m beyond), then 3 s after.
            Step "hol.Camera.Focus $($DropD -join ' ')", "frames 3", "hol.Select.Group Select 5", "waitlog Group 5: selected, 1 units",
                "hol.Select.OrderSource $($CampTree -join ' ')", "waitlog Gather order of slot 0: 1 units to source", "wait 16",
                "hol.Units.Where 0", "hol.Camera.Focus $($DropD -join ' ')", "frames 3", "hol.Select.OrderGround $($DropD -join ' ') shift",
                "hol.Camera.Focus $($DropMoveB -join ' ')", "frames 3", "hol.Select.OrderGround $($DropMoveB -join ' ') shift"
            for ($i = 0; $i -lt 22; $i++) { Step "wait 1", "hol.Units.Where 0" }
            Step "wait 3", "hol.Units.Where 0"
            # C (L.6a, the user's 2nd trial): back to the tree (~70 m from B) and a few seconds of gathering, then Shift + P,
            # Shift + D, Shift + B: it walks to P, unloads at D, then walks to B, idle there. Where it is every 0.25 s for 8 s,
            # then every second for 26 s (P ~17 m away, D 57 m beyond, B 15 m beyond D), then 3 s after.
            Step "hol.Camera.Focus $($DropD -join ' ')", "frames 3", "hol.Select.Group Select 5", "waitlog Group 5: selected, 1 units",
                "hol.Select.OrderSource $($CampTree -join ' ')", "waitlog Gather order of slot 0: 1 units to source", "wait 19",
                "hol.Units.Where 0", "hol.Camera.Focus $($DropMoveP -join ' ')", "frames 3", "hol.Select.OrderGround $($DropMoveP -join ' ') shift",
                "hol.Camera.Focus $($DropD -join ' ')", "frames 3", "hol.Select.OrderGround $($DropD -join ' ') shift",
                "hol.Camera.Focus $($DropMoveB -join ' ')", "frames 3", "hol.Select.OrderGround $($DropMoveB -join ' ') shift"
            for ($i = 0; $i -lt 32; $i++) { Step "wait 0.25", "hol.Units.Where 0" }
            for ($i = 0; $i -lt 26; $i++) { Step "wait 1", "hol.Units.Where 0" }
            Step "wait 3", "hol.Units.Where 0", "hol.Units.Remove $($DropD -join ' ') 1500 0", "hol.Units.Remove $($DropMoveB -join ' ') 1500 0",
                "hol.Units.Remove $($DropMoveP -join ' ') 1500 0", "wait 0.5"
            # Shift + ground then Shift + work (L.6a, the user's 2nd trial): (a) an empty Peasant, Shift + right click on the
            # ground at Pa (south), then Shift + right click on the tree nearest CampTree (north): it walks to Pa first, then
            # gathers. (b) house H placed by a builder sent away at once (H stays unbuilt); an empty Peasant, Shift + ground
            # at Pb (east), then Shift + right click on H: it walks to Pb first, then builds H. (c) seen only: a third empty
            # Peasant, Shift + right click on H then Shift + ground at Q: does it build H first or walk to Q at once?
            Step "mark build shiftwork", "hol.Units.SpawnSeed 121", "hol.Gather.SpeedScale 1", "hol.Cheat.FastBuild 0",
                "hol.Units.Spawn 1 $($ShiftWorkA -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftWorkA -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #"
            # (a): where it is before the orders, then every 0.5 s for 24 s (Pa 20 m away, the tree ~64 m beyond).
            Step "mark build shiftwork order", "hol.Units.Where 0", "hol.Camera.Focus $($ShiftWorkPa -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftWorkPa -join ' ') shift",
                "hol.Camera.Focus $($CampTree -join ' ')", "frames 3", "hol.Select.OrderSource $($CampTree -join ' ') shift"
            for ($i = 0; $i -lt 48; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            # (b): H placed by its builder, the builder sent away; the Peasant made, Shift + Pb, Shift + H; every 0.5 s for 22 s.
            Step "mark build shiftwork site", "hol.Units.Spawn 1 $($ShiftWorkHBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftWorkHBuilder -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Camera.Focus $($ShiftWorkH -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($ShiftWorkH -join ', ')) valid", "hol.Build.Click", "waitlog Build order of slot 0: 1 units to House at ($($ShiftWorkH -join ', '))",
                "hol.Camera.Focus $($ShiftWorkAway -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftWorkAway -join ' ')", "wait 0.5",
                "hol.Units.Spawn 1 $($ShiftWorkB -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftWorkB -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Units.Where 0", "hol.Camera.Focus $($ShiftWorkPb -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftWorkPb -join ' ') shift",
                "hol.Camera.Focus $($ShiftWorkH -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftWorkH -join ' ') shift"
            for ($i = 0; $i -lt 44; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            # (c): seen only; every second for 25 s.
            Step "mark build shiftwork after", "hol.Units.Spawn 1 $($ShiftWorkC -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftWorkC -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Units.Where 0", "hol.Camera.Focus $($ShiftWorkH -join ' ')", "frames 3",
                "hol.Select.OrderGround $($ShiftWorkH -join ' ') shift", "hol.Camera.Focus $($ShiftWorkQ -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftWorkQ -join ' ') shift"
            for ($i = 0; $i -lt 25; $i++) { Step "wait 1", "hol.Units.Where 0" }
            Step "hol.Units.Remove $($ShiftWorkH -join ' ') 3000 0", "hol.Units.Remove $($ShiftWorkQ -join ' ') 1000 0", "hol.Units.Remove $($ShiftWorkAway -join ' ') 600 0",
                "hol.Units.Remove $($ShiftWorkPb -join ' ') 1000 0", "wait 0.5"
            # Shift + ground then Shift + placement (EtapeDefini M.1, the user's 3rd trial of L): an empty Peasant, Shift +
            # right click on the ground at P, then house H placed with Shift (Escape leaves the placement mode, the
            # selection kept), then Shift + ground at Q: it walks to P, then builds H (FastBuild: finished at its first
            # blow), then walks to Q, idle there.
            Step "mark build shiftplace", "hol.Units.SpawnSeed 122", "hol.Gather.SpeedScale 1", "hol.Cheat.FastBuild 1",
                "hol.Units.Spawn 1 $($ShiftPlaceA -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftPlaceA -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #"
            # Where it is before the orders, then every 0.5 s for 26 s (P 20 m away, H 20 m beyond, Q 24 m beyond H).
            Step "mark build shiftplace order", "hol.Units.Where 0", "hol.Camera.Focus $($ShiftPlaceP -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftPlaceP -join ' ') shift",
                "hol.Camera.Focus $($ShiftPlaceH -join ' ')", "frames 3", "hol.Build.Place House", "waitlog Placement ghost: House at ($($ShiftPlaceH -join ', ')) valid",
                "hol.Build.Click shift", "waitlog Construction: slot 0 placed House foundation at ($($ShiftPlaceH -join ', '))"
            Key Escape
            Step "waitlog Placement: ended", "hol.Camera.Focus $($ShiftPlaceQ -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftPlaceQ -join ' ') shift"
            for ($i = 0; $i -lt 52; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            Step "hol.Units.Remove $($ShiftPlaceQ -join ' ') 1000 0", "hol.Cheat.FastBuild 0", "wait 0.5"
            # Shift + site then Shift + ground (EtapeDefini M.1, the user's 3rd trial of L): sites S and S2 placed by a builder
            # sent away at once (both unbuilt). (a) an empty Peasant, Shift + right click on S, Shift + ground at Q2, Shift +
            # right click on slot 0's berries: it builds S (FastBuild), then walks to Q2, then gathers food. (b) another,
            # Shift + right click on S2, then 0.5 s later (on its way to S2) Shift + ground at Q3: it builds S2, then walks
            # to Q3, idle there.
            Step "mark build shiftsite", "hol.Units.SpawnSeed 123", "hol.Gather.SpeedScale 1", "hol.Cheat.FastBuild 0",
                "hol.Units.Spawn 1 $($ShiftSiteBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftSiteBuilder -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Camera.Focus $($ShiftSiteS -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($ShiftSiteS -join ', ')) valid", "hol.Build.Click", "waitlog Build order of slot 0: 1 units to House at ($($ShiftSiteS -join ', '))",
                "hol.Camera.Focus $($ShiftSiteS2 -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($ShiftSiteS2 -join ', ')) valid", "hol.Build.Click", "waitlog Build order of slot 0: 1 units to House at ($($ShiftSiteS2 -join ', '))",
                "hol.Camera.Focus $($ShiftSiteAway -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftSiteAway -join ' ')", "wait 0.5", "hol.Cheat.FastBuild 1",
                "hol.Units.Spawn 1 $($ShiftSiteC -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftSiteC -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #"
            # (a): where it is before the orders, then every 0.5 s for 30 s (S 13 m away, Q2 18 m beyond, the berries 47 m beyond Q2).
            Step "mark build shiftsite order", "hol.Units.Where 0", "hol.Camera.Focus $($ShiftSiteS -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftSiteS -join ' ') shift",
                "hol.Camera.Focus $($ShiftSiteQ2 -join ' ')", "frames 3", "hol.Select.OrderGround $($ShiftSiteQ2 -join ' ') shift",
                "hol.Camera.Focus $($ShiftSiteBerries -join ' ')", "frames 3", "hol.Select.OrderSource $($ShiftSiteBerries -join ' ') shift"
            for ($i = 0; $i -lt 60; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            # (b): every 0.5 s for 20 s after the second order (S2 ~18 m away, Q3 25 m beyond it).
            Step "mark build shiftsite later", "hol.Units.Spawn 1 $($ShiftSiteD -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($ShiftSiteD -join ' ')", "frames 3",
                "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Units.Where 0", "hol.Camera.Focus $($ShiftSiteS2 -join ' ')", "frames 3",
                "hol.Select.OrderGround $($ShiftSiteS2 -join ' ') shift", "wait 0.5", "hol.Units.Where 0", "hol.Camera.Focus $($ShiftSiteQ3 -join ' ')", "frames 3",
                "hol.Select.OrderGround $($ShiftSiteQ3 -join ' ') shift"
            for ($i = 0; $i -lt 40; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            Step "hol.Units.Remove $($ShiftSiteBerries -join ' ') 1500 0", "hol.Units.Remove $($ShiftSiteQ3 -join ' ') 1000 0", "hol.Units.Remove $($ShiftSiteQ2 -join ' ') 1000 0",
                "hol.Units.Remove $($ShiftSiteAway -join ' ') 600 0", "hol.Cheat.FastBuild 0", "wait 0.5"
            # A rally point in several steps (EtapeDefini N.1, AoE4): slot 0's centre selected. (a) a right click at R1, then
            # Shift + right click at R2: the flag at R2, two segments (centre, R1, R2); a Peasant produced passes R1, then
            # stops at R2, its chain two white Rally steps. (b) a right click at R1, then Shift + right click on the gold vein:
            # a Peasant produced passes R1, then mines. (c) a plain right click at R3 after the steps: R3 alone. (d) a right
            # click then 8 Shift + right clicks in a row: the 9th refused (8 steps at most). The rally point cleared at the end.
            $RallyStepsWaitRally = "waitlog Rally: slot 0 "
            Step "mark build rallysteps", "hol.Cheat.Food 1000", "hol.Cheat.Population 50", "hol.Prod.BuildTime 1", "hol.Gather.SpeedScale 1",
                "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "hol.Select.Report", "waitlog Selection report: selected MainCentre of slot 0 (own)",
                "hol.Camera.Focus $($RallyStepsR1 -join ' ')", "frames 3", "hol.Select.OrderGround $($RallyStepsR1 -join ' ')", $RallyStepsWaitRally,
                "hol.Camera.Focus $($RallyStepsR2 -join ' ')", "frames 3", "hol.Select.OrderGround $($RallyStepsR2 -join ' ') shift", $RallyStepsWaitRally,
                "frames 3", "hol.Select.Report", "hol.Prod.Queue Peasant", "waitlog Production: Peasant of slot 0 done", "hol.Units.Where 0", "wait 0.3", "hol.Orders.Chains"
            for ($i = 0; $i -lt 44; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            Step "mark build rallysteps source", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "hol.Select.Report",
                "waitlog Selection report: selected MainCentre of slot 0 (own)", "hol.Camera.Focus $($RallyStepsR1 -join ' ')", "frames 3",
                "hol.Select.OrderGround $($RallyStepsR1 -join ' ')", $RallyStepsWaitRally, "hol.Camera.Focus $($RallyStepsGold -join ' ')", "frames 3",
                "hol.Select.OrderSource $($RallyStepsGold -join ' ') shift", $RallyStepsWaitRally, "frames 3", "hol.Select.Report", "hol.Prod.Queue Peasant",
                "waitlog Production: Peasant of slot 0 done", "hol.Units.Where 0"
            for ($i = 0; $i -lt 60; $i++) { Step "wait 0.5", "hol.Units.Where 0" }
            Step "mark build rallysteps replace", "hol.Camera.Focus $($RallyStepsR3 -join ' ')", "frames 3", "hol.Select.OrderGround $($RallyStepsR3 -join ' ')", $RallyStepsWaitRally,
                "frames 3", "hol.Select.Report"
            Step "mark build rallysteps nine", "hol.Camera.Focus $(-14000) $($RallyStepsRow[0][1])", "frames 3", "hol.Select.OrderGround $($RallyStepsRow[0] -join ' ')", $RallyStepsWaitRally
            for ($k = 1; $k -lt 9; $k++) { Step "hol.Select.OrderGround $($RallyStepsRow[$k] -join ' ') shift", $RallyStepsWaitRally }
            Step "frames 3", "hol.Select.Report", "hol.Prod.Rally -15000 0", $RallyStepsWaitRally, "hol.Prod.BuildTime 0", "hol.Units.Remove $($RallyStepsR2 -join ' ') 600 0"
            Key Escape
            Step "waitlog Selection: none"
            # A unit standing still in a corner of a house's visible wall (EtapeDefini C.1): house X built at once by a Peasant
            # (removed after); a Man-at-arms made beyond each corner on the diagonal, selected, sent by a right click onto the diagonal
            # just beyond that corner of X's body (centre +- 380: the navmesh's edge, 85 cm off the body; sent onto the body's corner
            # itself, the right click's ground point 13 cm off puts it against a side, not in the corner): it stops in the slab's corner, stands idle,
            # and the still-unit rule eases it out of the visible wall. Where they are 4 s and 8 s after the last order, the
            # footprint watch. The four removed and X destroyed (the parts after count slot 0's units and its population cap).
            Step "mark build corner", "hol.Units.SpawnSeed 143", "hol.Cheat.Wood 1000", "hol.Cheat.FastBuild 1", "hol.Nav.Footprints on",
                "hol.Units.Spawn 1 $($CornerBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($CornerBuilder -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Camera.Focus $($CornerHouse -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($CornerHouse -join ', ')) valid", "hol.Build.Click", "waitlog Construction: slot 0 finished House at ($($CornerHouse -join ', '))",
                "hol.Cheat.FastBuild 0", "wait 0.3", "hol.Units.Remove $($CornerHouse -join ' ') 2000 0", "waitlog units of slot 0 within 2000 of ($($CornerHouse -join ', '))"
            foreach ($S in $CornerSigns) {
                $CornerFrom = @(($CornerHouse[0] + 850 * $S.X), ($CornerHouse[1] + 850 * $S.Y))
                $CornerTo = @(($CornerHouse[0] + 380 * $S.X), ($CornerHouse[1] + 380 * $S.Y))
                Step "hol.Units.Spawn 1 $($CornerFrom -join ' ') 0 ManAtArms 10", "wait 0.5", "hol.Camera.Focus $($CornerFrom -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                    "waitlog Selection: ManAtArms #", "hol.Camera.Focus $($CornerTo -join ' ')", "frames 3", "hol.Select.OrderGround $($CornerTo -join ' ')", "waitlog Order: 1 units to ("
            }
            Step "wait 4", "hol.Units.Where 0", "wait 4", "hol.Units.Where 0", "hol.Nav.Footprints", "hol.Nav.Footprints off",
                "hol.Units.Remove $($CornerHouse -join ' ') 1500 0 ManAtArms", "waitlog units of slot 0 within 1500 of ($($CornerHouse -join ', '))",
                "hol.Cheat.DamageNear 99999 $($CornerHouse[0] - (Get-SlotStart 0)[0]) $($CornerHouse[1] - (Get-SlotStart 0)[1]) own melee",
                "waitlog Building destroyed: House of slot 0 at ($($CornerHouse -join ', '))", "frames 3",
                "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)"
            Key Escape
            Step "waitlog Selection: none"
            # The bottom bar's width (EtapeDefini T): a Peasant selected (the widest bar: selection, commands, the four age
            # panels), the window set to each size (r.SetRes: the HUD's DPI scale follows the shortest side), the layout
            # reported (bar against the minimap's place); captured at 1366x768, 1500x1080 (the bar drawn smaller) and 1920x1080;
            # the window back to its size.
            Step "mark build barwidth", "hol.Units.Spawn 1 $($CornerBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($CornerBuilder -join ' ')",
                "frames 3", "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "frames 3"
            foreach ($Size in $HudSizes) {
                Step "r.SetRes ${Size}w", "frames 8", "hol.HUD.Report"
                if ($Size -in @("1366x768", "1500x1080", "1920x1080")) { Step "capture qc_hud_$(($Size -split 'x')[0])" }
            }
            Step "r.SetRes 1600x900w", "frames 8", "hol.HUD.Report", "hol.Units.Remove $($CornerBuilder -join ' ') 600 0 Peasant",
                "waitlog units of slot 0 within 600 of ($($CornerBuilder -join ', '))"
            Key Escape
            Step "waitlog Selection: none"
            Stop-CaseFilter
        }
        if (($Parts -contains "anims") -and $Case.Name -eq "2p") {
            # The animations (J5c.4), at the table's rates of construction (two builders: ~10 s a house) and gathering.
            # Two Peasants made beside K, selected by a box, K placed by a click: they walk there and hammer.
            # First the wood's edge cleared: Peasants of the earlier parts (gather, build camp) at work between slot 0's
            # wood and the lumber camp L would enter the wood group's box; all units there sent to the cut's far point.
            Step "mark clear wood", "hol.Camera.Focus $($AnimGroups[0].Spawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($AnimGroups[0].Spawn[0] - 1900) $($AnimGroups[0].Spawn[1] - 1500) $($AnimGroups[0].Spawn[0] + 1300) $($AnimGroups[0].Spawn[1] + 800)",
                "frames 3", "hol.Select.OrderGround $($CutTo -join ' ')", "frames 3", "wait 6"
            Step "mark anims", "hol.Units.SpawnSeed 117","hol.Cheat.FastBuild 0", "hol.Build.Speed 1", "hol.Cheat.GatherSpeed 1", "hol.Cheat.Wood 100",
                "hol.Units.Spawn 2 $($AnimBuilders -join ' ') 0 Peasant 100", "hol.Units.Spawn 3 $($AnimIdle -join ' ') 0 Peasant 150"
            foreach ($Group in $AnimGroups) { Step "hol.Units.Spawn 3 $($Group.Spawn -join ' ') 0 Peasant 150" }
            Step "hol.Units.SpawnSeed 118", "hol.Units.Spawn 3 $($AnimSpearmen -join ' ') 0 Spearman 150", "hol.Units.Spawn 3 $($AnimMenAtArms -join ' ') 0 ManAtArms 150",
                "hol.Units.Spawn 1 $($AnimOthers[0]) $($AnimOthers[1] - 400) 1 Spearman 10", "hol.Units.Spawn 1 $($AnimOthers -join ' ') 1 ManAtArms 10",
                "hol.Units.Spawn 1 $($AnimOthers[0]) $($AnimOthers[1] + 400) 1 Peasant 10"
            Step "wait 0.5", "hol.Camera.Focus $($AnimBuilders -join ' ')", "frames 3",
                "hol.Select.BoxGround $($AnimBuilders[0] - 400) $($AnimBuilders[1] - 400) $($AnimBuilders[0] + 400) $($AnimBuilders[1] + 400)",
                "waitlog Selection: 2 units (own): Peasant x2", "hol.Camera.Focus $($AnimHouse -join ' ')", "frames 3", "hol.Build.Place House",
                "waitlog Placement ghost: House at ($($AnimHouse -join ', ')) valid", "hol.Build.Click", "waitlog Build order of slot 0: 2 units to House at ($($AnimHouse -join ', '))",
                "waitlog Construction: slot 0 first work on House at ($($AnimHouse -join ', '))"
            # Each group selected by a box around it, a right click on its source. Each also kept as control group 6 to 9
            # (emptied at the end, the groups part reads the bar): sent away later by its digit, not by a box around its
            # source (after gather and build, the lumber camp's gatherers fell trees there and the wood group's own
            # Peasants may fell a tree outside such a box).
            for ($i = 0; $i -lt $AnimGroups.Count; $i++) {
                $Group = $AnimGroups[$i]
                Step "hol.Camera.Focus $($Group.Spawn -join ' ')", "frames 3",
                    "hol.Select.BoxGround $($Group.Spawn[0] - 400) $($Group.Spawn[1] - 400) $($Group.Spawn[0] + 400) $($Group.Spawn[1] + 400)",
                    "waitlog Selection: 3 units (own): Peasant x3", "hol.Select.Group Add $(6 + $i)", "waitlog Group $(6 + $i): created, 3 units",
                    "hol.Select.OrderSource $($Group.Source -join ' ')", "waitlog Gather order of slot 0: 3 units to source"
            }
            # At their sources within ~1.5 s (spawned 5 to 8 m away): all at work, the report, close-ups.
            Step "wait 3", "mark anims working", "hol.Units.AnimReport", "hol.Units.Where 0", "hol.Camera.Zoom 14"
            foreach ($Group in $AnimGroups) { Step "hol.Camera.Focus $($Group.Look -join ' ')", "frames 3", "capture anims_work_$($Group.Name)" }
            Step "hol.Camera.Focus $($AnimHouse[0] - 350) $($AnimHouse[1])", "frames 3", "capture anims_work_build",
                "hol.Camera.Focus $($AnimIdle -join ' ')", "frames 3", "capture anims_work_idle",
                "hol.Camera.Focus $($AnimSpearmen[0]) $(($AnimSpearmen[1] + $AnimMenAtArms[1]) / 2)", "frames 3", "capture anims_soldiers_idle",
                "hol.Camera.Focus $(($AnimOthers[0] + $AnimSpearmen[0]) / 2) $($AnimOthers[1])", "frames 3", "capture anims_colors", "hol.Camera.Zoom -14", "wait 3"
            # Each group (a few wood, stone, gold or food each by now: 0.7 a second) selected by its control group (its three
            # Peasants and only them), sent ~20 m away: it keeps its load (D296). Reported on the way, then once they stand there.
            for ($i = 0; $i -lt $AnimGroups.Count; $i++) {
                $Group = $AnimGroups[$i]
                Step "hol.Camera.Focus $($Group.Source -join ' ')", "frames 3", "hol.Select.Group Select $(6 + $i)",
                    "waitlog Group $(6 + $i): selected, 3 units", "waitlog Selection: 3 units (own): Peasant x3", "hol.Select.OrderGround $($Group.Target -join ' ')", "waitlog Order: "
            }
            # The soldiers, selected by a box around both groups, sent last (on their way at the report).
            Step "hol.Camera.Focus $($AnimSpearmen[0]) $(($AnimSpearmen[1] + $AnimMenAtArms[1]) / 2)", "frames 3",
                "hol.Select.BoxGround $($AnimSpearmen[0] - 500) $($AnimSpearmen[1] - 500) $($AnimMenAtArms[0] + 500) $($AnimMenAtArms[1] + 500)",
                "waitlog Selection: 6 units (own)", "hol.Select.OrderGround $($AnimSoldiersTo -join ' ')", "waitlog Order: "
            Step "wait 0.5", "mark anims walking", "hol.Units.AnimReport", "hol.Units.Where 0", "hol.Camera.Zoom 14"
            $Last = $AnimGroups[-1]
            # The last group sent, a few metres on its way.
            Step "hol.Camera.Focus $($Last.Source[0] + ($Last.Target[0] - $Last.Source[0]) / 5) $($Last.Source[1] + ($Last.Target[1] - $Last.Source[1]) / 5)", "frames 2", "capture anims_walk_$($Last.Name)",
                "hol.Camera.Focus $($AnimSpearmen[0] + 300) $(($AnimSpearmen[1] + $AnimMenAtArms[1]) / 2)", "frames 2", "capture anims_soldiers_walk",
                "hol.Camera.Zoom -14", "wait 4", "mark anims standing", "hol.Units.AnimReport", "hol.Units.Where 0", "hol.Camera.Zoom 14"
            foreach ($Group in $AnimGroups) { Step "hol.Camera.Focus $($Group.Target -join ' ')", "frames 3", "capture anims_stand_$($Group.Name)" }
            # Nothing left selected (as in "fifo"): with -Parts ...,anims,combat the combat's "nothing selected" bars
            # would otherwise see the last anims selection (in -Full, groups ends with Escape).
            Step "hol.Camera.Zoom -14", "hol.Build.Report"
            # Control groups 6 to 9 emptied (the groups part reads the whole bar).
            for ($i = 0; $i -lt $AnimGroups.Count; $i++) {
                Step "hol.Select.Group Select $(6 + $i)", "hol.Select.Group Remove $(6 + $i)", "waitlog Group $(6 + $i): removed, 0 units"
            }
            Step "hol.Select.Click 0.03 0.05", "frames 3"
        }
        if (($Parts -contains "formation") -and $Case.Name -eq "2p") {
            Start-CaseFilter "formation"
            # The formation (J7a.4): the units of slot 0 before (the new ones are the others), the six made, a box
            # around them, a right click 35 m east; reported every half second on the way, then at rest.
            Step "mark formation", "hol.Units.SpawnSeed 119", "hol.Units.Formation 0", "hol.Camera.Focus $($FormPeasants[0]) $(($FormPeasants[1] + $FormMenAtArms[1]) / 2)",
                "hol.Units.Spawn 3 $($FormPeasants -join ' ') 0 Peasant 150", "hol.Units.Spawn 3 $($FormMenAtArms -join ' ') 0 ManAtArms 150", "wait 0.5", "frames 3",
                "hol.Select.BoxGround $($FormPeasants[0] - 800) $($FormMenAtArms[1] - 500) $($FormPeasants[0] + 800) $($FormPeasants[1] + 500)",
                "waitlog Selection: 6 units (own)", "hol.Units.Formation 0", "hol.Camera.Focus $($FormTo -join ' ')", "frames 3",
                "hol.Select.OrderGround $($FormTo -join ' ')", "waitlog Order: ", "mark formation march"
            for ($i = 0; $i -lt 20; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
            Step "wait 2", "mark formation arrived", "hol.Units.Formation 0", "hol.Camera.Zoom 14", "frames 3", "capture formation_mixed", "hol.Camera.Zoom -14"
            # The drag (the six still selected), then at rest.
            Step "mark formation front", "hol.Camera.Focus $(($FormLine[0] + $FormLine[2]) / 2) $(($FormLine[1] + $FormLine[3]) / 2)", "frames 3",
                "hol.Select.OrderLineGround $($FormLine -join ' ') ctrl", "waitlog Order: ", "wait 7", "hol.Units.Formation 0",
                "hol.Camera.Zoom 14", "frames 3", "capture formation_front", "hol.Camera.Zoom -14"
            # A Peasant sent through the front, alone: the pushed ones walk back to their places.
            Step "mark formation push", "hol.Units.SpawnSeed 120", "hol.Units.Formation 0", "hol.Units.Spawn 1 $($FormWalker -join ' ') 0 Peasant 150", "wait 0.3", "frames 3",
                "hol.Select.BoxGround $($FormWalker[0] - 300) $($FormWalker[1] - 300) $($FormWalker[0] + 300) $($FormWalker[1] + 300)",
                "waitlog Selection: Peasant #", "hol.Select.OrderGround $($FormWalkerTo -join ' ')", "waitlog Order: "
            for ($i = 0; $i -lt 18; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
            Step "mark formation back", "hol.Units.Formation 0", "hol.Camera.Zoom 14", "frames 3", "capture formation_back", "hol.Camera.Zoom -14"
            # A plain drag (orientation only), then a Shift move queued after it (the walker first taken out of the selection).
            Step "mark formation plain", "hol.Select.BoxGround $($FormLine[0] - 600) $($FormLine[1] - 600) $($FormLine[2] + 600) $($FormLine[3] + 600)", "waitlog Selection: ",
                "hol.Camera.Focus $(($FormPlainLine[0] + $FormPlainLine[2]) / 2) $(($FormPlainLine[1] + $FormPlainLine[3]) / 2)", "frames 3",
                "hol.Select.OrderLineGround $($FormPlainLine -join ' ')", "waitlog Order: ", "wait 0.5",
                "hol.Camera.Focus $($FormQueuedTo -join ' ')", "frames 3", "hol.Select.OrderGround $($FormQueuedTo -join ' ') shift", "waitlog Order: ", "wait 1", "hol.Units.Formation 0"
            for ($i = 0; $i -lt 46; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
            Step "mark formation queued", "hol.Units.Formation 0", "hol.Camera.Focus $($FormQueuedTo -join ' ')", "frames 3", "capture formation_queued"
            # The block march (J7a.4, the user's 1st trial): 3 Peasants, 3 Spearmen, 3 Men-at-arms on the six's first ground,
            # sent 35 m east: form-up in place, then one block on one path at the slowest's pace, reported every half second.
            Step "mark formation block", "hol.Units.SpawnSeed 121", "hol.Units.Formation 0", "hol.Camera.Focus $($FormPeasants[0]) $($FormBlockSpearmen[1])",
                "hol.Units.Spawn 3 $($FormBlockPeasants -join ' ') 0 Peasant 150", "hol.Units.Spawn 3 $($FormBlockSpearmen -join ' ') 0 Spearman 150",
                "hol.Units.Spawn 3 $($FormBlockMenAtArms -join ' ') 0 ManAtArms 150", "wait 0.5", "frames 3",
                "hol.Select.BoxGround $($FormBlockPeasants[0] - 800) $($FormBlockMenAtArms[1] - 400) $($FormBlockPeasants[0] + 800) $($FormBlockPeasants[1] + 400)",
                "waitlog Selection: 9 units (own)", "hol.Units.Formation 0", "hol.Select.OrderGround $($FormTo -join ' ')", "waitlog Order: "
            for ($i = 0; $i -lt 24; $i++) { Step "wait 0.25", "hol.Units.Formation 0" }
            Step "hol.Camera.Focus $($FormTo[0] - 1200) $($FormTo[1])", "frames 3", "capture formation_block"
            for ($i = 0; $i -lt 20; $i++) { Step "wait 0.25", "hol.Units.Formation 0" }
            Step "wait 2", "mark formation block end", "hol.Units.Formation 0", "hol.Camera.Focus $($FormTo -join ' ')", "frames 3", "capture formation_block_end"
            # The control groups and Ctrl + A (J7a.5), the nine of the block selected: real keys (the AZERTY names of the
            # top row, the QWERTY one, Ctrl and Shift held around them) and the console commands. Shift + digit creates an
            # empty group; Ctrl + digit alone does nothing (the user's trial: no assign).
            Step "mark groups", "hol.Units.Formation 0",
                "hol.UI.Key LeftShift 0 0.5", "hol.UI.Key Ampersand 0.2 0.1", "wait 0.6", "waitlog Group 1: created", "waitlog HUD groups: 1=9",
                "hol.UI.Key LeftControl 0 0.5", "hol.UI.Key A 0.2 0.1", "wait 0.6", "waitlog Select army: ",
                "hol.UI.Key LeftControl 0 0.5", "hol.UI.Key Ampersand 0.2 0.1", "wait 0.6", "hol.Select.Report",
                "hol.Select.Group Add 2", "hol.Select.Group Remove 1", "hol.Select.Group Select 2", "waitlog Group 2: selected", "waitlog HUD groups: 1=3 2=",
                "hol.UI.Key LeftShift 0 0.5", "hol.UI.Key One 0.2 0.1", "wait 0.6", "waitlog Group 1: added", "frames 3", "capture groups_bar",
                "hol.Camera.Focus 0 0", "frames 2", "hol.UI.Key Ampersand 0 0.05", "hol.UI.Key Ampersand 0.15 0.05", "wait 0.4", "hol.Camera.Report",
                "hol.Select.Army", "hol.UI.Key LeftControl 0 0.6", "hol.UI.Key LeftShift 0 0.6", "hol.UI.Key Ampersand 0.25 0.1", "wait 0.7",
                "waitlog Group 1: removed", "wait 0.4", "hol.HUD.Click ControlGroup2", "waitlog Group 2: selected", "hol.Select.Group Select 0", "hol.Select.Group Double 2", "hol.Camera.Report", "mark groups end"
            Key Escape
            Step "waitlog Selection: none"
            # The straggler (the user's 1st trial of J7b): 5 Men-at-arms and a 6th 10 m behind, a box around the six, an
            # order 37 m away; then 5 Men-at-arms and an Archer 10 m behind. Each reported every half second, then at rest.
            foreach ($Strag in @(@{ Mark = "straggler"; Seed = 131; Late = "ManAtArms"; To = $StragTo }, @{ Mark = "straggler archer"; Seed = 132; Late = "Archer"; To = $StragArcherTo })) {
                Step "mark $($Strag.Mark)", "hol.Units.SpawnSeed $($Strag.Seed)", "hol.Units.Formation 0", "hol.Camera.Focus $(($StragFrom[0] + $StragLate[0]) / 2) $($StragFrom[1])", "frames 3",
                    "hol.Units.Spawn 5 $($StragFrom -join ' ') 0 ManAtArms 150", "hol.Units.Spawn 1 $($StragLate -join ' ') 0 $($Strag.Late) 10", "wait 0.5", "frames 3",
                    "hol.Select.BoxGround $($StragLate[0] - 400) $($StragFrom[1] - 500) $($StragFrom[0] + 500) $($StragFrom[1] + 500)",
                    "waitlog Selection: 6 units (own)", "hol.Units.Formation 0", "hol.Select.OrderGround $($Strag.To -join ' ')", "waitlog Order: "
                for ($i = 0; $i -lt 24; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
                Step "wait 2", "mark $($Strag.Mark) end", "hol.Units.Formation 0"
                Key Escape
                Step "waitlog Selection: none"
            }
            # The scattered group (the user's trial of playtest note 1 of J7c): 2 groups of 3 Men-at-arms 10 m apart, a box
            # around the six, an order 37 m away; reported every half second, then at rest.
            Step "mark scattered", "hol.Units.SpawnSeed 133", "hol.Units.Formation 0", "hol.Camera.Focus $($ScatterNorth[0]) $(($ScatterNorth[1] + $ScatterSouth[1]) / 2)", "frames 3",
                "hol.Units.Spawn 3 $($ScatterNorth -join ' ') 0 ManAtArms 150", "hol.Units.Spawn 3 $($ScatterSouth -join ' ') 0 ManAtArms 150", "wait 0.5", "frames 3",
                "hol.Select.BoxGround $($ScatterNorth[0] - 500) $($ScatterSouth[1] - 500) $($ScatterNorth[0] + 500) $($ScatterNorth[1] + 500)",
                "waitlog Selection: 6 units (own)", "hol.Units.Formation 0", "hol.Select.OrderGround $($ScatterTo -join ' ')", "waitlog Order: "
            for ($i = 0; $i -lt 24; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
            Step "wait 2", "mark scattered end", "hol.Units.Formation 0"
            Key Escape
            Step "waitlog Selection: none"
            # The close straggler (the user's trial of playtest note 1 of J7b): 5 Men-at-arms in a file and a 6th 3 m behind
            # and beside, a box around the six, an order 38 m away; reported every half second, then at rest, then removed.
            Step "mark straggler near", "hol.Units.SpawnSeed 134", "hol.Units.Formation 0", "hol.Camera.Focus $($NearFile[0] + 300) $($NearFile[1])", "frames 3"
            for ($i = 0; $i -lt 5; $i++) { Step "hol.Units.Spawn 1 $($NearFile[0] + 150 * $i) $($NearFile[1]) 0 ManAtArms 10" }
            Step "hol.Units.Spawn 1 $($NearLate -join ' ') 0 ManAtArms 10", "wait 0.5", "frames 3",
                "hol.Select.BoxGround $($NearLate[0] - 300) $($NearLate[1] - 300) $($NearFile[0] + 900) $($NearFile[1] + 300)",
                "waitlog Selection: 6 units (own)", "hol.Units.Formation 0", "hol.Select.OrderGround $($NearTo -join ' ')", "waitlog Order: "
            for ($i = 0; $i -lt 24; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
            Step "wait 2", "mark straggler near end", "hol.Units.Formation 0"
            Key Escape
            Step "waitlog Selection: none", "hol.Units.Remove $($NearTo -join ' ') 500 0", "wait 0.3"
            # Two blocks meeting: 2 files of 3 Men-at-arms 5 m apart across the order, a box around the six, an order 28 m
            # east between them; reported every half second, then at rest, then removed.
            Step "mark blocks meet", "hol.Units.SpawnSeed 135", "hol.Units.Formation 0", "hol.Camera.Focus $($MeetNorth[0] + 150) $(($MeetNorth[1] + $MeetSouth[1]) / 2)", "frames 3"
            for ($i = 0; $i -lt 3; $i++) { Step "hol.Units.Spawn 1 $($MeetNorth[0] + 150 * $i) $($MeetNorth[1]) 0 ManAtArms 10", "hol.Units.Spawn 1 $($MeetSouth[0] + 150 * $i) $($MeetSouth[1]) 0 ManAtArms 10" }
            Step "wait 0.5", "frames 3", "hol.Select.BoxGround $($MeetNorth[0] - 300) $($MeetSouth[1] - 300) $($MeetNorth[0] + 600) $($MeetNorth[1] + 300)",
                "waitlog Selection: 6 units (own)", "hol.Units.Formation 0", "hol.Select.OrderGround $($MeetTo -join ' ')", "waitlog Order: "
            for ($i = 0; $i -lt 20; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
            Step "wait 2", "mark blocks meet end", "hol.Units.Formation 0"
            Key Escape
            Step "waitlog Selection: none", "hol.Units.Remove $($MeetTo -join ' ') 500 0", "wait 0.3"
            # Reinforcements onto friends standing on their places (MV.4.6): a line of 6 Men-at-arms standing exactly on the
            # formation's places, 6 more sent onto it from 29 m west; friends at rest give way (D292), every one sent sets down.
            # Reported at 12 s (hol.Units.Orders: the 6 sent are the 6 last identifiers), then all removed.
            Step "mark reinforce", "hol.Units.SpawnSeed 136", "hol.Camera.Focus $($MeetTo[0] - 1400) $($MeetTo[1])", "frames 3"
            for ($i = 0; $i -lt 6; $i++) { Step "hol.Units.Spawn 1 $($MeetTo[0] - 175 * [Math]::Floor($i / 3)) $($MeetTo[1] - 150 + 150 * ($i % 3)) 0 ManAtArms 0" }
            Step "hol.Units.Spawn 6 $($MeetNorth[0]) $($MeetTo[1]) 0 ManAtArms 150", "wait 0.5", "frames 3",
                "hol.Select.BoxGround $($MeetNorth[0] - 500) $($MeetTo[1] - 500) $($MeetNorth[0] + 500) $($MeetTo[1] + 500)",
                "waitlog Selection: 6 units (own)", "hol.Select.OrderGround $($MeetTo -join ' ')", "waitlog Order: ", "wait 12", "hol.Units.Orders 0", "mark reinforce end"
            Key Escape
            Step "waitlog Selection: none", "hol.Units.Remove $($MeetTo -join ' ') 1200 0", "wait 0.3"
            # Formed up in place inside a mixed army (the user's trial of select_type_q, "elles peinent"): 6 Men-at-arms,
            # 6 Spearmen and 4 Archers made within 3.5 m and at rest, one type picked by a double click (its units
            # scattered among the others) and sent 1 m east of the army's middle; reported every half second for 8 s.
            foreach ($Regroup in $RegroupCases) {
                # The type made first and alone, picked by a double click then kept as a control group (a click in the
                # mixed army lands on whichever body is drawn on top: not the same type from one run of parts to another).
                $OtherCounts = @{ ManAtArms = 6; Spearman = 6; Archer = 4 }
                # The ground cleared first (the double click takes every unit of the type on the screen: the formation
                # cases before leave Men-at-arms around).
                Step "mark formation $($Regroup.Name)", "hol.Units.Remove $($RegroupAt -join ' ') 3500 0", "wait 0.3",
                    # Its control group emptied first (the part groups leaves units in group 2, far from here: Add would keep them).
                    "hol.Select.Group Select $($Regroup.Group)", "hol.Select.Group Remove $($Regroup.Group)", "wait 0.2", "hol.Units.SpawnSeed 137", "hol.Camera.Focus $($RegroupAt -join ' ')", "frames 3",
                    "hol.Units.Spawn 6 $($RegroupAt -join ' ') 0 $($Regroup.Type) 350", "wait 1", "frames 3",
                    "hol.Select.ClickUnit -1 double", "waitlog Selection: 6 units (own): $($Regroup.Type) x6", "hol.Select.Group Add $($Regroup.Group)", "wait 0.2"
                Key Escape
                Step "waitlog Selection: none", "hol.Units.Spawn $($OtherCounts[$Regroup.Others[0]]) $($RegroupAt -join ' ') 0 $($Regroup.Others[0]) 350",
                    "hol.Units.Spawn $($OtherCounts[$Regroup.Others[1]]) $($RegroupAt -join ' ') 0 $($Regroup.Others[1]) 350", "wait 2", "frames 3"
                if ($Regroup.Formed) {
                    # The army first formed up as one (each one then holds its place).
                    Step "hol.Select.BoxGround $($RegroupAt[0] - 600) $($RegroupAt[1] - 600) $($RegroupAt[0] + 600) $($RegroupAt[1] + 600)",
                        "waitlog Selection: 16 units (own)", "hol.Select.OrderGround $($RegroupAt -join ' ')", "waitlog Order: ", "wait 8"
                    Key Escape
                    Step "waitlog Selection: none"
                }
                Step "hol.Select.Group Select $($Regroup.Group)", "waitlog Selection: 6 units (own): $($Regroup.Type) x6"
                Step "wait 1", "hol.Units.Formation 0", "hol.Select.OrderGround $($RegroupAt[0] + $Regroup.Dx) $($RegroupAt[1])", "waitlog Order: "
                for ($i = 0; $i -lt 16; $i++) { Step "wait 0.5", "hol.Units.Formation 0" }
                Step "mark formation $($Regroup.Name) end", "hol.Units.Formation 0"
                Key Escape
                Step "waitlog Selection: none", "hol.Units.Remove $($RegroupAt -join ' ') 1200 0", "wait 0.3"
            }
            Stop-CaseFilter
        }
        if (($Parts -contains "combat") -and $Case.Name -eq "2p") {
            Start-CaseFilter "combat"
            # The melee (J7b.2), solo: 3 Men-at-arms of slot 0 and 3 Spearmen of slot 1 made 6 m apart on the formation's
            # first ground (left by then): they take each other on by themselves (aggression 16 m), walk into reach and strike.
            Step "mark combat", "hol.Units.SpawnSeed 122", "hol.Combat.Enabled 1", "hol.Combat.Report", "hol.Units.Spawn 3 $($CombatMenAtArms -join ' ') 0 ManAtArms 150", "hol.Units.Spawn 3 $($CombatSpearmen -join ' ') 1 Spearman 150",
                "hol.Camera.Focus $(($CombatMenAtArms[0] + $CombatSpearmen[0]) / 2) $($CombatMenAtArms[1])", "wait 0.2", "hol.Combat.Report"
            for ($i = 0; $i -lt 3; $i++) { Step "wait 1", "hol.Combat.Report" }
            # The strikes (J7b.5): each side's fighters play their strike (hol.Units.AnimReport per slot), a close-up of the melee.
            Step "hol.Units.AnimReport 0", "hol.Units.AnimReport 1", "hol.Camera.Zoom 14", "frames 3", "capture combat_melee", "hol.Camera.Zoom -14", "frames 2"
            # A player's order prevails (J7b.2): the 3 Men-at-arms, in the melee, ordered 20 m away (west): they stop striking
            # and walk off; the Spearmen chase their last attackers, taken on again once the Men-at-arms have arrived.
            # The health bars (J7b.5): the wounded alone while nothing is selected, then the 3 Men-at-arms selected as well.
            Step "capture combat_fight", "hol.Combat.Report", "hol.Units.Bars", "hol.Select.BoxGround -6800 -5900 -5200 -5100", "waitlog Selection: 3 units (own): ManAtArms x3",
                "frames 2", "hol.Combat.Report", "hol.Units.Bars", "capture combat_bars",
                "mark combat away", "hol.Select.OrderGround $($CombatMenAtArms[0] - 2000) $($CombatMenAtArms[1])", "hol.Combat.Report",
                "hol.Camera.Focus $($CombatMenAtArms[0] - 1000) $($CombatMenAtArms[1])"
            for ($i = 0; $i -lt 30; $i++) { Step "wait 1", "hol.Combat.Report" }
            Step "capture combat_away", "hol.Select.Click 0.03 0.05"
            Step "mark combat end", "hol.Combat.Report", "frames 3", "capture combat_end"
            # The orders (J7b.3): 4 Peasants of slot 0 (no aggression of their own) and a Spearman of slot 1, 6 m apart where
            # the melee was; the Peasants selected, a right click on the Spearman: they attack it. Then the grid's stop (S).
            Step "mark orders attack", "hol.Units.SpawnSeed 123", "hol.Units.Spawn 4 $($CombatMenAtArms -join ' ') 0 Peasant 150", "hol.Units.Spawn 1 $($CombatSpearmen -join ' ') 1 Spearman 10",
                "hol.Camera.Focus $(($CombatMenAtArms[0] + $CombatSpearmen[0]) / 2) $($CombatMenAtArms[1])", "wait 0.3",
                "hol.Select.BoxGround $($CombatMenAtArms[0] - 350) $($CombatMenAtArms[1] - 350) $($CombatMenAtArms[0] + 350) $($CombatMenAtArms[1] + 350)",
                "waitlog Selection: 4 units (own): Peasant x4", "hol.Select.OrderGround $($CombatSpearmen -join ' ')", "hol.Combat.Report"
            for ($i = 0; $i -lt 4; $i++) { Step "wait 1", "hol.Combat.Report" }
            # The Peasant's strike (J7b.5): the ordered Peasants, striking by now, play theirs with the knife (the report of
            # slot 0), a close-up of them (taken once the order is applied: the click and the order stay as they are). Their
            # selection block (J7b trial feedback): fighting, none idle (hol.HUD.Report).
            Step "hol.Units.AnimReport 0", "hol.HUD.Report", "hol.Camera.Zoom 14", "frames 3", "capture combat_peasant_strike", "hol.Camera.Zoom -14", "frames 2"
            Step "capture orders_attack", "mark orders stop", "hol.Select.Stop", "wait 0.2", "hol.Combat.Report", "wait 2", "hol.Combat.Report"
            Step "mark orders end", "hol.Select.Click 0.03 0.05"
            # The attack-move (J7b.3): 3 Men-at-arms of slot 0 attack-move 30 m east, a Spearman of slot 1 standing 4 m off
            # their way (within the attack-move's 16 m): they take it on, kill it, then walk on to their destination.
            Step "mark orders attackmove", "hol.Units.SpawnSeed 124", "hol.Units.Spawn 3 $($OrdersMoveFrom -join ' ') 0 ManAtArms 150", "hol.Units.Spawn 1 $($OrdersMoveSpearman -join ' ') 1 Spearman 150",
                "hol.Camera.Focus $($OrdersMoveFrom -join ' ')", "wait 0.3", "hol.Combat.Report",
                "hol.Select.BoxGround $($OrdersMoveFrom[0] - 350) $($OrdersMoveFrom[1] - 350) $($OrdersMoveFrom[0] + 350) $($OrdersMoveFrom[1] + 350)",
                "waitlog Selection: 3 units (own): ManAtArms x3", "hol.Camera.Focus $(($OrdersMoveFrom[0] + $OrdersMoveTo[0]) / 2) $($OrdersMoveTo[1])", "wait 0.2",
                "hol.Select.AttackMove $($OrdersMoveTo -join ' ')", "hol.Combat.Report"
            for ($i = 0; $i -lt 22; $i++) { Step "wait 1", "hol.Combat.Report" }
            Step "capture orders_attackmove", "hol.Select.Click 0.03 0.05"
            # Hold position (J7b.3): a Man-at-arms of slot 0 told to hold, an enemy Peasant (no aggression) just out of its
            # reach: it does not chase; then an enemy Man-at-arms made 8 m away (same 70 cm reach: a longer reach would strike
            # it from out of its own, J7c.4) walks into its reach and is struck.
            # (The Peasant made once the hold is applied: the box takes a body's middle, above its feet, so a box tight
            # enough to leave the Peasant out would miss the Man-at-arms.)
            Step "mark orders hold", "hol.Units.SpawnSeed 125", "hol.Units.Spawn 1 $($OrdersHold -join ' ') 0 ManAtArms 150",
                "hol.Camera.Focus $($OrdersHold -join ' ')", "wait 0.3",
                "hol.Select.BoxGround $($OrdersHold[0] - 350) $($OrdersHold[1] - 350) $($OrdersHold[0] + 350) $($OrdersHold[1] + 350)",
                "waitlog Selection: ManAtArms #", "hol.Select.Hold", "waitlog Hold order of slot 0 for 1 units",
                "hol.Units.Spawn 1 $($OrdersHoldPeasant -join ' ') 1 Peasant 150", "wait 0.2", "hol.Combat.Report"
            for ($i = 0; $i -lt 3; $i++) { Step "wait 1", "hol.Combat.Report" }
            Step "mark orders hold reach", "hol.Units.SpawnSeed 126", "hol.Units.Spawn 1 $($OrdersHoldSpearman -join ' ') 1 ManAtArms 150", "hol.Combat.Report"
            for ($i = 0; $i -lt 8; $i++) { Step "wait 1", "hol.Combat.Report" }
            Step "capture orders_hold", "mark orders hold end", "hol.Select.Click 0.03 0.05"
            # The fall (J7b.5): a Peasant of slot 1 made beside the 3 Men-at-arms standing at the attack-move's end (their own
            # aggression takes it on); once killed it plays its fall (the report of slot 1), captured falling (dying 1 s).
            # Its body (J7c, bodies kept 30 s): 5 s later still on the ground, its fall ended (hol.Units.CorpseReport).
            # (Before the push: no other Peasant of slot 1 can die meanwhile, so the wait catches this one's death; after
            # it, the push's target and its wall die all along, the Men-at-arms going on from one to the next.)
            Step "mark combat fall", "hol.Units.SpawnSeed 127", "hol.Camera.Focus $($OrdersMoveTo -join ' ')", "frames 2", "hol.Units.Spawn 1 $($OrdersMoveTo[0] + 450) $($OrdersMoveTo[1]) 1 Peasant 10",
                "waitlog Peasant of slot 1 killed by", "frames 2", "hol.Units.AnimReport 1", "hol.Camera.Zoom 14", "frames 2", "capture combat_fall",
                "wait 5", "hol.Units.CorpseReport", "capture combat_corpse", "hol.Camera.Zoom -14", "hol.Units.AnimReport 0", "mark combat fall end"
            # Two enemies never push each other back (the user's trial of J7b): the Men-at-arms walk to their target through
            # the standing Peasants, which do not give way; they go round them and strike it all the same.
            Step "hol.Units.SpawnSeed 128", "hol.Units.Spawn 12 $($PushWall -join ' ') 1 Peasant 300", "hol.Units.Spawn 1 $($PushTarget -join ' ') 1 Peasant 10",
                "hol.Units.Spawn 3 $($PushMenAtArms -join ' ') 0 ManAtArms 150", "hol.Camera.Focus $($PushWall -join ' ')", "wait 0.3",
                "hol.Select.BoxGround $($PushMenAtArms[0] - 350) $($PushMenAtArms[1] - 350) $($PushMenAtArms[0] + 350) $($PushMenAtArms[1] + 350)",
                "waitlog Selection: 3 units (own): ManAtArms x3", "mark orders push", "hol.Select.OrderGround $($PushTarget -join ' ')", "hol.Combat.Report"
            for ($i = 0; $i -lt 10; $i++) { Step "wait 1", "hol.Combat.Report" }
            Step "capture orders_push", "mark orders push end", "hol.Select.Click 0.03 0.05"
            # A queue after an ordered target (the review of J7b): A by a right click, B with Shift; then C by their aggression.
            Step "mark orders queue", "hol.Units.SpawnSeed 129", "hol.Units.Spawn 3 $($QueueMenAtArms -join ' ') 0 ManAtArms 150",
                "hol.Units.Spawn 1 $($QueueA -join ' ') 1 Peasant 10", "hol.Units.Spawn 1 $($QueueB -join ' ') 1 Peasant 10",
                "hol.Camera.Focus $($QueueMenAtArms -join ' ')", "wait 0.3", "hol.Combat.Report",
                "hol.Select.BoxGround $($QueueMenAtArms[0] - 350) $($QueueMenAtArms[1] - 350) $($QueueMenAtArms[0] + 350) $($QueueMenAtArms[1] + 350)",
                "waitlog Selection: 3 units (own): ManAtArms x3", "hol.Camera.Focus $($QueueA[0]) $(($QueueA[1] + $QueueB[1]) / 2)", "wait 0.2",
                "hol.Select.OrderGround $($QueueA -join ' ')", "hol.Select.OrderGround $($QueueB -join ' ') shift", "hol.Combat.Report"
            for ($i = 0; $i -lt 20; $i++) { Step "wait 1", "hol.Combat.Report" }
            # (The report a few frames after the spawn: one in the same step does not have C yet.)
            Step "mark orders queue seek", "hol.Units.SpawnSeed 130", "hol.Units.Spawn 1 $($QueueC -join ' ') 1 Peasant 10", "wait 0.3", "hol.Combat.Report"
            for ($i = 0; $i -lt 8; $i++) { Step "wait 1", "hol.Combat.Report" }
            # The bodies gone (J7c, bodies kept 30 s): ~40 s after the fall's body was laid, it has sunk and gone at 31 s.
            Step "mark combat corpse expiry", "hol.Units.CorpseReport", "mark orders queue end", "hol.Select.Click 0.03 0.05"
            # The shot (J7c.2a): 4 Archers of slot 0, 3 Spearmen and 2 Peasants of slot 1 14 m and 10 m off (within the Archers'
            # 16 m aggression): they shoot by themselves; each arrow lands after its flight (distance at release / 37.5 m/s),
            # lost when its target died meanwhile; a Peasant killed by an arrow dies as in melee (it falls, Dying).
            # The arrows drawn (J7c.2b): a close-up 0.15 s after a release, the arrows in the air; the display's report
            # (hol.Combat.ArrowReport) before and after, everyone of the shot taken away at the end: none left in flight.
            Step "mark combat arrows", "hol.Combat.ArrowReport", "hol.Units.SpawnSeed 131", "hol.Combat.LogHits 1", "hol.Camera.Focus $($ArrowArchers[0]) $(($ArrowArchers[1] + $ArrowSpearmen[1]) / 2)",
                "hol.Units.Spawn 4 $($ArrowArchers -join ' ') 0 Archer 150", "hol.Units.Spawn 3 $($ArrowSpearmen -join ' ') 1 Spearman 150",
                "hol.Units.Spawn 2 $($ArrowPeasants -join ' ') 1 Peasant 150", "wait 0.3", "hol.Combat.Report", "hol.Camera.Zoom 12",
                "waitlog releases an arrow", "wait 0.15", "capture combat_arrows_1", "wait 0.1", "capture combat_arrows_2", "hol.Camera.Zoom -12",
                "waitlog hits struck, by arrow", "frames 2", "hol.Units.AnimReport 1", "mark combat arrows fight"
            for ($i = 0; $i -lt 12; $i++) { Step "wait 1", "hol.Combat.Report" }
            Step "hol.Units.Remove $($ArrowArchers[0]) $(($ArrowArchers[1] + $ArrowSpearmen[1]) / 2) 1500 1",
                "hol.Units.Remove $($ArrowArchers[0]) $(($ArrowArchers[1] + $ArrowSpearmen[1]) / 2) 1500 0", "wait 1", "hol.Combat.ArrowReport",
                "mark combat arrows end"
            # The target that flees (J7c.2b, the user's case): every unit 1.5 times as fast (the Spearman ~8.7 m/s, an AoE4 light cavalry), 3 Archers
            # and a Spearman of slot 1 20 m off (D345); as soon as the first arrow is released the Spearman runs north: the arrow
            # still lands on it (damage at the landing), the drawn arrow follows it and is gone on it.
            Step "mark combat arrows flee", "hol.Units.Remove $($FleeArchers[0]) $($FleeArchers[1]) 12000 1", "hol.Units.Remove $($FleeSpearman[0]) $($FleeSpearman[1]) 4000 0", "wait 0.2", "hol.Cheat.UnitSpeed 1.5", "hol.Units.SpawnSeed 132", "hol.Camera.Focus $($FleeArchers[0]) $($FleeArchers[1] + 900)",
                "hol.Units.Spawn 3 $($FleeArchers -join ' ') 0 Archer 150", "hol.Units.Spawn 1 $($FleeSpearman -join ' ') 1 Spearman 150", "hol.Camera.Zoom 12",
                "waitlog releases an arrow", "hol.Units.Move $($FleeTo -join ' ') 1", "hol.Units.Where 1", "hol.Units.Where 0", "wait 0.1", "capture combat_flee_1",
                "hol.Units.Where 1", "capture combat_flee_2", "hol.Units.Where 1", "hol.Units.Where 0", "wait 0.25", "hol.Units.Where 1", "hol.Units.Where 0", "hol.Camera.Zoom -12", "wait 2", "hol.Units.Where 1", "hol.Units.Where 0",
                "hol.Combat.ArrowReport", "hol.Units.Remove $($FleeArchers[0]) $($FleeArchers[1]) 6000 1", "hol.Units.Remove $($FleeArchers[0]) $($FleeArchers[1]) 6000 0",
                "hol.Cheat.UnitSpeed 1", "wait 1", "hol.Combat.ArrowReport", "hol.Combat.LogHits 0", "mark combat arrows flee end"
            # Reinforcements behind their friends in the fight (MV.4): 12 Men-at-arms of slot 0 attack-move in formation onto 12 of
            # slot 1; 6 s on, 6 more Men-at-arms made where they started and sent by a plain move to the ground 7 m behind the
            # line's point (on the point itself the click is an attack on the enemy there); 20 s more, then all removed.
            # Coming to aid (MV.4, choice C): sent 17 m behind the point, they stand out of the enemies' 16 m aggression but
            # within 20 m of their friends fighting (hol.Combat.AidReport each second; hol.Combat.AidRadius 0: they stay idle).
            # Control first (MV.4, choice C): 6 against 6 Men-at-arms made in contact at the meeting point, 6 more of slot 0 made idle
            # 18.5 m behind: out of the enemies' 16 m aggression, within 20 m of their friends fighting. Aid off (hol.Combat.AidRadius 0)
            # they stay idle there (the case is meaningful); aid back on (2000 cm) none is left idle near the fight within 3 s.
            Step "mark combat aid", "hol.Units.Remove $($MeetTo -join ' ') 4000 0", "hol.Units.Remove $($MeetTo -join ' ') 4000 1", "wait 0.2",
                "hol.Units.SpawnSeed 141", "hol.Camera.Focus $($MeetTo[0] - 900) $($MeetTo[1])", "frames 3", "hol.Combat.AidRadius 0",
                "hol.Units.Spawn 6 $($MeetTo -join ' ') 0 ManAtArms 150", "hol.Units.Spawn 6 $($MeetTo[0] + 300) $($MeetTo[1]) 1 ManAtArms 150", "wait 1",
                "hol.Units.Spawn 6 $($MeetTo[0] - $AidBack) $($MeetTo[1]) 0 ManAtArms 150", "wait 1"
            for ($i = 0; $i -lt 3; $i++) { Step "wait 1", "hol.Combat.AidReport 0 2000 $($MeetTo[0] - $AidBack) $($MeetTo[1]) 1000" }
            Step "mark combat aid on", "hol.Combat.AidRadius 2000"
            for ($i = 0; $i -lt 4; $i++) { Step "wait 1", "hol.Combat.AidReport 0 2000 $($MeetTo[0] - $AidBack) $($MeetTo[1]) 1000" }
            Step "mark combat aid end"
            Step "mark combat reinforce", "hol.Units.Remove $($MeetTo -join ' ') 4000 0", "hol.Units.Remove $($MeetTo -join ' ') 4000 1", "wait 0.2",
                "hol.Units.SpawnSeed 139", "hol.Camera.Focus $($MeetTo[0] - 1400) $($MeetTo[1])", "frames 3",
                "hol.Units.Spawn 12 $($MeetNorth[0]) $($MeetTo[1]) 0 ManAtArms 150", "hol.Units.Spawn 12 $($MeetTo[0] + 600) $($MeetTo[1]) 1 ManAtArms 150", "wait 0.5", "frames 3",
                "hol.Select.BoxGround $($MeetNorth[0] - 900) $($MeetTo[1] - 900) $($MeetNorth[0] + 900) $($MeetTo[1] + 900)",
                "waitlog Selection: 12 units (own)", "hol.Select.AttackMove $($MeetTo -join ' ')", "wait 6",
                "hol.Camera.Focus $($MeetNorth[0]) $($MeetTo[1])", "frames 3", "mark combat reinforce sent",
                "hol.Units.Spawn 6 $($MeetNorth[0]) $($MeetTo[1]) 0 ManAtArms 150", "wait 0.5", "frames 3",
                "hol.Select.BoxGround $($MeetNorth[0] - 700) $($MeetTo[1] - 700) $($MeetNorth[0] + 700) $($MeetTo[1] + 700)",
                "waitlog Selection: 6 units (own)", "hol.Camera.Focus $($MeetTo[0] - 1400) $($MeetTo[1])", "frames 3", "hol.Select.OrderGround $($MeetTo[0] - 1700) $($MeetTo[1])", "waitlog Order: 6 units to ("
            for ($i = 0; $i -lt 24; $i++) { Step "wait 1", "hol.Combat.AidReport 0" }
            Step "mark combat reinforce end"
            Key Escape
            Step "waitlog Selection: none", "hol.Units.Remove $($MeetTo -join ' ') 4000 0", "hol.Units.Remove $($MeetTo -join ' ') 4000 1", "wait 0.3"
            # The last attacker forgotten (EtapeDefini S): a Man-at-arms hit once by an Archer which then walks away (a move order
            # on slot 1; 7.8 m/s against 7 m/s: never caught up with), out of its 16 m aggression. It ripostes (chases) for 5 s
            # after the hit, then forgets it: no target, standing still, idle (no chase across the map). The hit's step from the
            # arrow's landing line (hol.Combat.LogHits), then a combat report and the units' places every 0.5 s for 11 s.
            Step "mark combat forget", "hol.Units.SpawnSeed 142", "hol.Combat.LogHits 1", "hol.Camera.Focus $($ForgetArcher -join ' ')", "frames 2",
                "hol.Units.Spawn 1 $($ForgetManAtArms -join ' ') 0 ManAtArms 10", "hol.Units.Spawn 1 $($ForgetArcher -join ' ') 1 Archer 10", "wait 0.2", "hol.Units.Where",
                "waitlog Archer lands on #", "hol.Units.Move $($ForgetTo -join ' ') 1", "waitlog Move order: ", "hol.Combat.Report", "hol.Units.Where"
            for ($i = 0; $i -lt 22; $i++) { Step "wait 0.5", "hol.Combat.Report", "hol.Units.Where" }
            Step "mark combat forget end", "hol.Combat.LogHits 0", "hol.Units.Remove $($ForgetGround -join ' ') 3600 0", "hol.Units.Remove $($ForgetGround -join ' ') 3600 1", "wait 0.3"
            Stop-CaseFilter
        }
    }
}
Step "mark end", "quit"
$SequenceFile = Join-Path $TestSaved "quick_check.seq"
$Steps | Set-Content -Encoding utf8 $SequenceFile

Get-ChildItem $Shots -Filter "qc_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "cam_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "sel_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "move_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "src_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "gather_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "build_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "anims_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "formation_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "combat_*.png" -ErrorAction SilentlyContinue | Remove-Item
$Display = if ($Visible) { "-windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100" } else { "-RenderOffScreen -ResX=1600 -ResY=900 -nosound" }
# The melee off (J7b.2) until the combat part: the parts before it make units of several slots side by side.
# The Multiplayer screen of the menu part asks a manager nobody runs (port 7799), never the player's one (7700).
$TestServers = Join-Path $TestSaved "servers_none.json"
Set-Content -Path $TestServers -Value '{ "servers": [ { "name": "None", "url": "http://127.0.0.1:7799" } ] }' -Encoding ascii
$TimeStep = if ($RealTime) { "" } else { "-UseFixedTimeStep -FPS=$GameFps" }
# Off screen on a fixed step, the world is not drawn while the sequence waits for the game (each frame ~2 ms cheaper);
# drawn for the commands and the captures (Docs/Archive/TestSpeedInProgress.md). -Visible: always drawn.
if (-not $RealTime -and -not $Visible) { $GameExec = (@("hol.Test.SkipWaitRendering 1") + @($GameExec | Where-Object { $_ })) -join "," }
$GameArguments = "`"$UProject`" -game $Display $TimeStep $NoStartingPeasants -saveddirsuffix=Tests -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -culture=fr -ServersFile=`"$TestServers`" -abslog=`"$GameLog`" -ExecCmds=`"hol.Combat.Enabled 0,$(if ($GameExec) { "$GameExec," })hol.Test.Run $SequenceFile`""
$GameProcess = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $GameArguments -PassThru

# 3 (continued). Once the server listens, the two clients join it, each with its own saved folder (Saved_Tests_Net<N>,
# as net_match.ps1). Each one: camera above the slot it holds, the buildings it has received (the two centres in their
# colours), a click on the centre of slot 0, then of slot 1 (one its own, the other not), then one in the void.
$NetClients = @()
$NetListening = $false
if ($NetServerProcess) {
    $Deadline = (Get-Date).AddSeconds(90)
    while (-not $NetListening -and (Get-Date) -lt $Deadline -and -not $NetServerProcess.HasExited) {
        Start-Sleep -Milliseconds 250
        $NetListening = (Test-Path $NetServerLog) -and (Select-String -Path $NetServerLog -Pattern "listening on port" -Quiet)
    }
}
if ($NetListening) {
    Remove-Item (Join-Path $TestSaved "quick_net_*.flag") -ErrorAction SilentlyContinue
    # R3.10: Alice (client 2) first, Bob (client 1) once she has captured her wait banner (her signal below): she alone
    # waits, for Bob. Step U (2026-10-29): her "Camera above the start" line gated Bob's launch, for 60 s at most, then Bob
    # was launched anyway: an Alice slow to start (another copy's compilation holds every launch) came in after him and
    # never saw the banner (her waitlog timed out, her sequence stopped). Now the gate is her capture itself and lasts
    # 120 s (the server's wait, hol.Match.StartWaitSeconds 120 in its -ExecCmds, lasts as long: a banner for her up to
    # ~115 s after the server's start); past it, Bob is launched anyway and the check below says so.
    $NetAliceReady = $false
    $NetAliceWaitFlag = Join-Path $TestSaved "quick_net_alice_wait.flag"
    foreach ($Client in 2, 1) {
        $ClientSaved = Join-Path $Project "Saved_Tests_Net$Client"
        $ClientShots = Join-Path $ClientSaved "Screenshots\WindowsEditor\HOL"
        Get-ChildItem $ClientShots -Filter "qcnet_*.png" -ErrorAction SilentlyContinue | Remove-Item
        $ClientSettings = Join-Path $ClientSaved "Config\WindowsEditor\GameUserSettings.ini"
        if (-not (Test-Path $ClientSettings)) {
            # No display saved (-1): the engine keeps -WinX instead of re-centring the window (net_match.ps1).
            New-Item -ItemType Directory -Force (Split-Path $ClientSettings) | Out-Null
            (Get-Content (Join-Path $PSScriptRoot "GameUserSettings.ini")) `
                -replace '^(DisplayIndex|LastUserConfirmedDisplayIndex)=.*', '$1=-1' | Set-Content -Encoding ascii $ClientSettings
        }
        # waitlog reads the lines since the last command: nothing between waitmap and it. The cursor is held in the middle
        # from the start (-ExecCmds below), or the camera scrolls at the edge before the first step. Each client waits for
        # the other one (a client done and gone frees its slot for the one still joining).
        # R3.10: Alice, here first, captures her wait banner before waiting for Bob (Bob waits for nobody), then signals it:
        # the script launches Bob on that signal (the banner cannot be missed whatever her start takes).
        $NetSteps = @("waitmap Plain", "waitlog Camera above the start")
        if ($Client -eq 2) { $NetSteps += "waitlog Wait banner: shown", "frames 5", "capture qcnet_${Client}_wait", "signal $NetAliceWaitFlag" }
        $NetSteps += "waitplayers 2", "mark net", "frames 30", "hol.Camera.Report", "hol.Buildings.Report", "capture qcnet_${Client}_start"
        # "hero" alone (J8a.6): the steps from here to the "done" signal dropped below.
        $NetStartSteps = $NetSteps.Count
        # The refused cheat (R3.4): client 1's 500 wood, refused by the server (hol.Cheat.Refuse 1 until its signal).
        # The refused pause (D361): client 1 types "pause" in its console, as a player would (the engine's ServerPause).
        if ($Client -eq 1) { $NetSteps += "hol.Cheat.Wood 500", "console pause", "wait 1", "hol.Prod.Report", "signal $NetCheatTriedFlag", "waitfile $NetCheatBackFlag 60" }
        # The limited pause (D513): Bob pauses, Alice's resume is refused (timer running), Bob's second pause too.
        if ($Client -eq 1) {
            $NetSteps += "hol.Match.Pause", "wait 1", "hol.HUD.Report", "capture qcnet_1_paused", "signal $NetPausedFlag", "waitfile $NetAliceResumeTriedFlag 60", "hol.Match.Pause", "wait 1",
                "signal $NetPauseCheckFlag", "waitfile $NetPauseCheckedFlag 60", "hol.Match.Resume", "wait 1", "hol.HUD.Report", "signal $NetResumedFlag"
        } else {
            $NetSteps += "waitfile $NetPausedFlag 300", "hol.Match.Resume", "wait 1", "hol.HUD.Report", "capture qcnet_2_paused", "signal $NetAliceResumeTriedFlag", "waitfile $NetResumedFlag 120"
        }
        foreach ($Slot in 0, 1) {
            $NetSteps += "hol.Camera.FocusSlot $Slot", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot $Slot",
                "frames 2", "hol.Select.Report", "capture qcnet_${Client}_slot$Slot"
        }
        $NetSteps += "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "hol.Select.Report"
        # Production (D284): each client has its centre make a Peasant (2 s, the server's hol.Prod.BuildTime), then both
        # Peasants must be shown, and the other player's resources must not have reached this client (D282).
        $NetSteps += "hol.Prod.Queue", "waitunits 2", "wait 1", "hol.Units.Report", "hol.Prod.Report", "capture qcnet_${Client}_peasants"
        # A rally point in several steps seen by its owner (EtapeDefini N.1): its centre selected, a right click at P1, then
        # Shift + right click at P2 (16 m toward the map's middle, 24 and 40 m aside): the flag at P2, two segments and a
        # point at P1, from what the server replicated to this client; then cleared (a right click on the centre).
        $RallySlot = 2 - $Client
        $RallyStart = Get-SlotStart $RallySlot
        $RallySign = if ($RallyStart[0] -lt 0) { 1 } else { -1 }
        $NetRallyP1 = @(($RallyStart[0] + $RallySign * 1600), ($RallyStart[1] + 2400))
        $NetRallyP2 = @(($RallyStart[0] + $RallySign * 1600), ($RallyStart[1] + 4000))
        # (hol.Prod.Queue selected the centre already: emptied first, so that the click logs its selection.)
        $NetSteps += "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "hol.Camera.FocusSlot own", "frames 3", "hol.Select.Click 0.5 0.5",
            "waitlog Selection: MainCentre of slot $RallySlot (own)",
            "hol.Camera.Focus $($NetRallyP1[0]) $($NetRallyP1[1] + 800)", "frames 3", "hol.Select.OrderGround $($NetRallyP1 -join ' ')", "wait 1",
            "hol.Select.OrderGround $($NetRallyP2 -join ' ') shift", "wait 1", "frames 3", "hol.Select.Report", "hol.Camera.FocusSlot own", "frames 3",
            "hol.Select.OrderGround $($RallyStart -join ' ')", "wait 1", "hol.Select.Click 0.03 0.05", "waitlog Selection: none"
        # Orders (J4, D288, D289): each client sends every unit it knows, the other player's Peasant too, 25 m beside its
        # start; the server moves its own alone; then each client must see both Peasants where their owners sent them.
        if ($Parts -contains "move") {
            $NetSteps += "hol.Units.OrderNear -1000 2500 all", "wait 12", "hol.Units.Where", "capture qcnet_${Client}_moved"
        }
        # Sources (J5a.2): the 376 sources received, the tree the server used up gone (5 s after both joined, long done).
        # Without the orders' 12 s, the clients get here ~5 s after joining: 4 s more, the server's takes done (5 and 6 s).
        if ($Parts -contains "sources") {
            if (-not ($Parts -contains "move")) { $NetSteps += "wait 4" }
            $NetSteps += "hol.Sources.Report"
            # Gathering (J5a.5, D299, D303): its Peasant selected alone, a right click on the tree nearest its start (5
            # wood left by the server): it gathers it, the tree is used up, it goes on with a neighbour, unloads 10 at
            # its centre. Every 2 s, what the HUD shows (its exact load, sent to this client alone) and what this client
            # knows of each unit (the other's activity, never its amount).
            # J5b.3 (D307): the HUD's counts first, its Peasant idle. From the 3rd report to the 8th, both clients select the
            # tree nearest slot 0's start (slot 0's 5 wood: the other's for one of them), its remaining amount followed;
            # then their own Peasant again.
            $NetSteps += "mark gather", "hol.HUD.Report", "hol.Camera.FocusSlot own", "frames 3", "hol.Select.ClickUnit 0", "waitlog Selection: Peasant", "frames 2",
                "hol.Select.OrderSourceNear", "frames 2"
            for ($i = 0; $i -lt 16; $i++) {
                if ($i -eq 2) { $NetSteps += "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.ClickSource $((Get-SlotStart 0) -join ' ')", "waitlog Selection:" }
                if ($i -eq 8) { $NetSteps += "hol.Camera.FocusSlot own", "frames 3", "hol.Select.ClickUnit 0", "waitlog Selection:" }
                $NetSteps += "wait 2", "hol.HUD.Report", "hol.Units.Where"
            }
            $NetSteps += "hol.Sources.Report", "hol.Prod.Report", "capture qcnet_${Client}_gather"
        }
        # Placement (J6a.3, D311): its Peasant sent aside first, 15 m in front of its centre (no unload meanwhile; born on
        # the -X side of every centre, behind slot 0's, it would else reach its house before the other client's report), then selected, a house 24 m behind its
        # centre, which its Peasant builds (J6a.4); once both placed (each waits for the other), its own foundation seen here
        # and not the other's (a ghost for its owner alone until its first work), its own wood 50 less, the other's stock
        # never received; then the other's site arrives at its first work (captured); a house 32 m behind the enemy centre,
        # refused by the server (anti-rush radius, D22).
        # Step U (2026-10-29): the two clients place within a frame of each other (the "placing" flags: until then the
        # phases since the pause ran unsynchronised, one client's Peasant could be seconds ahead on its 39 m walk), and
        # "waitlog Building arrived" comes right after the reports it must follow (waitlog reads the lines since the last
        # command: with the two captures between them, an arrival during the first capture was missed and the sequence
        # stopped). The captures before the reports show the same state (the other's house not there yet).
        $Other = 3 - $Client
        if ($Parts -contains "build") {
            $NetSteps += "mark build", "hol.Units.OrderNear 1500 2500", "wait 3", "hol.Prod.Report", "hol.Select.Own", "waitlog Select own: 1 units",
                "signal $(Join-Path $TestSaved "quick_net_placing$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_placing$Other.flag")", "hol.Build.PlaceNear House $($NetHouseNear -join ' ')",
                "signal $(Join-Path $TestSaved "quick_net_placed$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_placed$Other.flag")", "wait 2",
                "hol.Camera.FocusSlot 0", "frames 5", "capture qcnet_${Client}_build0", "hol.Camera.FocusSlot 1", "frames 5", "capture qcnet_${Client}_build1",
                "hol.Build.Report", "hol.Prod.Report", "waitlog Building arrived: House of slot", "wait 1", "hol.Build.Report", "hol.Camera.FocusSlot 0", "frames 5",
                "capture qcnet_${Client}_site0", "hol.Camera.FocusSlot 1", "frames 5", "capture qcnet_${Client}_site1", "hol.Build.PlaceNear House $($NetEnemyNear -join ' ') enemy", "wait 2", "hol.Build.Report", "hol.Prod.Report"
            # The camps (J6b.3, D312): once both houses are seen, its Peasant places a lumber camp 2 to 6 m off the wood beside its
            # centre ($NetCampNear: slot 0's 6 m off, 4 m farther out than the solo part's L; slot 1's, the same offset in its frame, 2 m off: the centres are not mirrors on the 4 m grid) and builds it (hol.Build.Speed 4 on the server); once
            # its first unload at the camp shows (the +10), each waits for the other's, then sees both camps finished; then,
            # its selection emptied, a camp asked on the tree nearest slot 0's L (a cheating client): refused by the server.
            $NetSteps += "signal $(Join-Path $TestSaved "quick_net_built$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_built$Other.flag")",
                "mark camp", "hol.Select.Own", "waitlog Select own: 1 units", "hol.Build.PlaceNear LumberCamp $($NetCampNear -join ' ')", "wait 2", "hol.Prod.Report",
                "waitlog Deposit popup: +", "signal $(Join-Path $TestSaved "quick_net_camp$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_camp$Other.flag")",
                "wait 1", "hol.Build.Report", "hol.Prod.Report", "hol.Select.Click 0.03 0.05", "waitlog Selection: none",
                "hol.Build.PlaceAtSource LumberCamp $($CampTree -join ' ')", "wait 2", "hol.Build.Report", "hol.Prod.Report"
        }
        # The army (J7a.6): resources, places and fast building cheated (every player); its Peasant places a barracks behind
        # its centre (the solo part's spot, in its frame; its own foundation arrives at once, the other's when finished, at its
        # first blow); once the other's has arrived here and both clients have seen theirs, 2 Spearmen and 2 Men-at-arms
        # queued there; once all 10 units are shown, the other's army framed: its type, mesh and colour logged (Units looks).
        # Then its 5 units (Peasant included) in control group 1 (Shift + &), the selection emptied, & selects them again,
        # a right click 30 m beside its centre: the group arrives in rows; each waits for the other's, then reports where.
        $NetSteps += "mark army", "hol.Cheat.Resources 2000", "hol.Cheat.Population 20", "hol.Cheat.FastBuild 1", "wait 1",
            "hol.Select.Own", "waitlog Select own: 1 units", "hol.Build.PlaceNear Barracks $($NetBarracksNear -join ' ')", "waitlog Building arrived: Barracks of slot", "waitlog , finished",
            "signal $(Join-Path $TestSaved "quick_net_barracks$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_barracks$Other.flag")", "wait 3",
            "hol.Prod.Queue Spearman Barracks", "hol.Prod.Queue Spearman Barracks", "hol.Prod.Queue ManAtArms Barracks", "hol.Prod.Queue ManAtArms Barracks",
            "waitunits 10", "wait 8", "signal $(Join-Path $TestSaved "quick_net_soldiers$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_soldiers$Other.flag")",
            "hol.Cheat.FastBuild 0", "hol.Camera.FocusNear $($NetBarracksNear -join ' ') enemy", "frames 10", "hol.Units.Report", "capture qcnet_${Client}_army_other",
            "hol.Camera.FocusNear -1500 3500", "frames 5", "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "hol.Select.Own", "waitlog Select own: 5 units",
            "hol.UI.Key LeftShift 0 0.5", "hol.UI.Key Ampersand 0.2 0.1", "wait 0.6", "waitlog Group 1: created", "hol.Select.Click 0.03 0.05", "waitlog Selection: none",
            "hol.UI.Key Ampersand 0 0.05", "wait 0.4", "hol.Select.Report"
        # The order lines (D328): client 1 orders first and signals it; client 2, a second later, has received no chain
        # (client 1's never reaches it), then orders; each one's chain (one Move, its 5 units) reaches its owner alone.
        $ChainsFlag = Join-Path $TestSaved "quick_net_ordered1.flag"
        if ($Client -eq 1) { $NetSteps += "hol.Select.OrderNear 0 3000", "signal $ChainsFlag", "wait 1", "hol.Orders.Chains" }
        else { $NetSteps += "waitfile $ChainsFlag", "wait 1", "hol.Orders.Chains", "hol.Select.OrderNear 0 3000", "wait 1", "hol.Orders.Chains" }
        $NetSteps += "wait 18",
            "signal $(Join-Path $TestSaved "quick_net_marched$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_marched$Other.flag")", "wait 1",
            "hol.Units.Where", "hol.Camera.FocusNear 0 3000", "frames 5", "capture net_army_c$Client"
        # The fight (J7b.4), on client 1 alone: the solo combat part's ground framed, its signal to the server, which makes 2
        # Men-at-arms of slot 0 and 2 Spearmen of slot 1 there; once shown, its newest own unit (one of them) selected, then
        # every half second where it has the units (hp level, target, activity) and what the HUD shows of the selection.
        if ($Client -eq 1) {
            $NetSteps += "mark combat", "hol.Camera.Focus $(($CombatMenAtArms[0] + $CombatSpearmen[0]) / 2) $($CombatMenAtArms[1])", "frames 5",
                "signal $NetFightFlag", "waitunits 14", "wait 0.5", "hol.Select.ClickUnit -1", "waitlog Selection:"
            for ($i = 0; $i -lt 48; $i++) {
                $NetSteps += "wait 0.5", "hol.Units.Where", "hol.HUD.Report", "hol.Units.Bars"
                if ($i -eq 6) { $NetSteps += "capture net_combat_c1" }
            }
            # The bodies (J7c, bodies kept 30 s): the fight's Spearmen killed, this client keeps theirs (the server none).
            # Then the fight's window closed (its own mark): the server's arrows fight starts, this client waits for its end
            # and the last arrows' landing.
            $NetSteps += "hol.Units.CorpseReport", "mark arrows", "signal $NetArrowsFlag", "waitfile $NetArrowsEndFlag 120", "wait 3"
        }
        # Each client waits for the other one's last report before quitting: a player gone leaves the other's reports (D282's
        # "others []", the flake of 2026-10-16).
        # The arrows (J7c.2b2): deduced by each client from the Attacking state of the server's Archers (net arrows), all
        # landed by now (client 1 signals its end once they have).
        if ($NetHeroOnly) { $NetSteps = @($NetSteps | Select-Object -First $NetStartSteps) }
        $NetSteps += "signal $(Join-Path $TestSaved "quick_net_done$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_done$Other.flag")", "hol.Combat.ArrowReport"
        # The hero sheets (J8a.3): once the server's heroes are wounded and still, the sheets as this client has them and
        # the units it shows (the wounded hero's health drawn back from the share received); then once Bob's hero is dead.
        # The return (J8a.4 loop 5), the times at 20 % on every machine (hol.Hero.ReturnTimePercent): Bob (client 1) orders his
        # hero's return as soon as it is dead, too early (refused on the server). Once it is ready (the server's signal),
        # each client asks its own return decision to act (refused: a client decides nothing), reports; Bob's gold to 0,
        # his order (cannot pay), 1 000 gold, his report, his order (bought), his report; once both heroes are back (the
        # server's signal), the last report.
        # The hero's block (J8a.5 loop 6), hol.HUD.Report at each moment: born (its own hero alive), wounded, Bob's dead (twice,
        # 1 s apart), ready (Alice's own just killed), Bob without gold (his button greyed: its click sends nothing), with
        # 1 000 gold (the price shown, then his paid return by the button, in the same frame), after it, both back.
        $NetSteps += "mark hero", "hol.Hero.ReturnTimePercent 20", "waitfile $NetHeroBornFlag 180", "wait 1", "hol.HUD.Report",
            "signal $(Join-Path $TestSaved "quick_net_hero_born$Client.flag")", "waitfile $NetHeroWoundedFlag 180", "wait 1", "hol.Hero.Report", "hol.Units.Where", "hol.HUD.Report",
            "signal $(Join-Path $TestSaved "quick_net_hero_seen$Client.flag")", "waitfile $NetHeroKilledFlag 120", "hol.HUD.Report"
        if ($Client -eq 1) { $NetSteps += "hol.Hero.Return" }
        $NetSteps += "wait 1", "hol.Hero.Report", "hol.HUD.Report", "signal $(Join-Path $TestSaved "quick_net_hero_done$Client.flag")",
            "mark hero return", "waitfile $NetHeroReadyFlag 120", "wait 1", "hol.Hero.ReturnProbe $(if ($Client -eq 1) { 1 } else { 0 })", "hol.Hero.Report", "hol.HUD.Report"
        if ($Client -eq 1) {
            $NetSteps += "hol.Cheat.Gold -1000000", "wait 1", "hol.HUD.Report", "hol.HUD.Click HeroReturnButton", "hol.Hero.Return", "wait 1", "hol.Cheat.Gold 1000", "wait 1", "hol.Hero.Report",
                "hol.HUD.Report", "hol.HUD.Click HeroReturnButton", "wait 1.5", "hol.Hero.Report", "hol.HUD.Report"
        }
        $NetSteps += "waitfile $NetHeroBackFlag 120", "wait 1", "hol.Hero.Report", "hol.HUD.Report", "signal $(Join-Path $TestSaved "quick_net_hero_back$Client.flag")"
        # J8b.3: Bob's hero at level 2 (the server's signal): the sheets, the units and the block as this client has them.
        $NetSteps += "mark hero xp", "waitfile $NetHeroXPFlag 120", "wait 1", "hol.Hero.Report", "hol.Units.Where", "hol.HUD.Report",
            "signal $(Join-Path $TestSaved "quick_net_hero_xp$Client.flag")"
        # J8c.3: Bob (client 1) at level 4 casts his cry twice (the second refused at the server) and plants his banner 6 m
        # from his forum's return place (~(14065, 299)); then both report the sheets as they have them.
        $NetSteps += "mark hero abilities", "waitfile $NetHeroAbilitiesFlag 120"
        if ($Client -eq 1) { $NetSteps += "hol.Hero.Cast CloseRanks", "wait 0.5", "hol.Hero.Cast CloseRanks", "hol.Hero.Cast Banner 14665 299", "wait 0.5",
            "signal $(Join-Path $TestSaved "quick_net_hero_cast1.flag")" }
        $NetSteps += "waitfile $NetHeroCastFlag 120", "wait 1", "hol.Hero.Report", "hol.Hero.BannerDisplay", "signal $(Join-Path $TestSaved "quick_net_hero_abilities$Client.flag")"
        # The end of the match (V1.4c, D356-D358), last (the match is over after it): client 2 counts the units it shows,
        # then signals; client 1's King dies (V5.5a, in place of its surrender: the network surrender is the "lobby" part's,
        # Bob's ServerSurrender checked on the server log by lobby_match.ps1), its banner Defeat; client 2's banner Victory (the game state's OnRep), then the units it shows each second for 5 s (client 1's fall and go).
        # Client 1 waits for client 2's last report before quitting.
        if ($Parts -contains "victory") {
            $VictoryReady = Join-Path $TestSaved "quick_net_victory_ready.flag"
            $VictoryDone = Join-Path $TestSaved "quick_net_victory_done.flag"
            # V2.5 (D353, the lives public): first client 1's main forum falls on the server (its own cheat of damage, the
            # server's path: a dedicated server has no local controller for hol.Victory.FallForum); client 2's players'
            # table then shows that slot at 1 life, in ruin.
            $ForumFell = Join-Path $TestSaved "quick_net_forum_fell.flag"
            $ForumSeen = Join-Path $TestSaved "quick_net_forum_seen.flag"
            $KingSeen = Join-Path $TestSaved "quick_net_king_seen2.flag"
            if ($Client -eq 1) {
                $NetSteps += "mark victory", "waitfile $VictoryReady", "hol.Cheat.DamageNear 99999 0 0 own melee", "wait 0.5",
                    # V3.5: its forum down held by the other slot (the presence fixed on both slots, slots known only once joined:
                    # the standing forum ignores it); client 2 sees the hold in its table too.
                    "hol.Victory.HoldPresence 0 1 0 1", "hol.Victory.HoldPresence 1 1 0 0", "wait 1", "hol.HUD.Report", "signal $ForumFell",
                    # V5.5a: no surrender any more: the Kings born on the server, client 1 shows the other's (slot 0), then its own
                    # King killed (the server's KillKing 1): its message without alert (its own loss), its banner Defeat.
                    "waitfile $ForumSeen", "signal $NetKingAskFlag", "waitfile $NetKingBornFlag 120", "wait 1.5", "hol.Units.Report",
                    "waitfile $KingSeen 120", "signal $NetKingKillFlag", "waitlog End banner: Defeat", "waitfile $VictoryDone"
            }
            else {
                $NetSteps += "mark victory", "hol.Units.Report", "signal $VictoryReady", "waitfile $ForumFell", "wait 1", "hol.HUD.Report", "signal $ForumSeen",
                    "waitfile $NetKingBornFlag 300", "wait 1.5", "hol.Units.Report", "signal $KingSeen", "waitlog End banner: Victory"
                for ($i = 0; $i -lt 5; $i++) { $NetSteps += "wait 1", "hol.Units.Report" }
                $NetSteps += "signal $VictoryDone"
            }
        }
        $NetSteps += "mark end", "quit"
        $ClientSequence = Join-Path $TestSaved "quick_net_client$Client.seq"
        $NetSteps | Set-Content -Encoding utf8 $ClientSequence
        $ClientLog = Join-Path $TestSaved "Logs\quick_net_client$Client.log"
        $ClientDisplay = if ($Visible) { "-windowed -ResX=940 -ResY=530 -WinX=$(3940 + ($Client - 1) * 960) -WinY=1020" } else { "-RenderOffScreen -ResX=960 -ResY=540 -nosound" }
        $ClientUrl = if ($Client -eq 1) { "127.0.0.1:${NetPort} -PlayerName=Bob" } else { "127.0.0.1:${NetPort}?Name=Alice" }
        $ClientArguments = "`"$UProject`" $ClientUrl -game $ClientDisplay -saveddirsuffix=Tests_Net$Client -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -culture=fr -abslog=`"$ClientLog`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Prod.BuildTime 2,hol.Gather.LogPopups 1,hol.Test.Run $ClientSequence`""
        Remove-Item $ClientLog -ErrorAction SilentlyContinue
        $ClientProcess = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $ClientArguments -PassThru
        $NetClients += @{ Client = $Client; Log = $ClientLog; Shots = $ClientShots; Process = $ClientProcess }
        if ($Client -eq 2) {
            # Her wait banner captured (its signal), 120 s at most (the server waits for her as long): Bob's launch waits for it.
            $Deadline = (Get-Date).AddSeconds(120)
            while (-not $NetAliceReady -and (Get-Date) -lt $Deadline -and -not $ClientProcess.HasExited) {
                Start-Sleep -Milliseconds 250
                $NetAliceReady = Test-Path $NetAliceWaitFlag
            }
            if (-not $NetAliceReady) { Write-Host "net: client 2 (Alice) has not captured her wait banner within 120 s; client 1 (Bob) launched anyway" }
        }
    }
    # Client 1 first again for the reports below.
    $NetClients = @($NetClients | Sort-Object { $_.Client })
}

# 780 s with the camera's parts (-Full): the sequence takes ~285 s since J6b.2b (the build gap part), ~30 s more with the
# barracks (J7a.2), the start ~20 s; past 480 s with the combat and orders parts (J7b.3: attack-move and hold, ~45 s more);
# ~660 s with the straggler's groups (J7b trial feedback 1, ~105 s more), when 660 s cut the last parts off.
# 120 s otherwise: the default's smoke part is done ~40 s after the launch (D327), the menu part ~15 s.
$GameLimitSeconds = if ($Parts -contains "camera") { 960 } else { 120 }
$GameTimedOut = -not $GameProcess.WaitForExit($GameLimitSeconds * 1000)
if ($GameTimedOut) { $GameProcess.Kill() }
$GameSeconds = [Math]::Round($Clock.Elapsed.TotalSeconds)

$AllOk = $true
# The first script compile errors of a log (Angelscript, D484), at most three lines: file, then line and message. A script
# test's failed assertion is written "Angelscript: Error: ::<test>" too, its next line "Assert failed ...": not a compile
# error (the test's own failure says it, J8a.4 loop 5). The script's Log() lines ("Angelscript: Hero return refused ...")
# never match.
function Get-ScriptErrors([string] $Path) {
    if (-not (Test-Path $Path)) { return @() }
    return @(Select-String -Path $Path -Pattern "Angelscript: Error: (.*)$" -Context 0, 1 |
        Where-Object { -not ($_.Context.PostContext.Count -and $_.Context.PostContext[0] -match "^\s*Assert failed") } |
        ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() } | Select-Object -First 3)
}
function Report([string] $Part, [System.Collections.Specialized.OrderedDictionary] $Checks) {
    # A part played by cases: only the reports of the cases asked ("build cut" for build:cut), the others not played
    # (their checks still read their empty sections: [regex]::Match on "$X", never on null). A case's own further reports
    # too ("build hero return" for build:hero).
    foreach ($Played in @($SubCases.Keys)) {
        if ($Part -match "^$([regex]::Escape($Played))( |$)" -and
            -not @($SubCases[$Played] | Where-Object { $Part -eq "$Played $_" -or $Part.StartsWith("$Played $_ ") }).Count) { return $true }
    }
    $Failed = @($Checks.Keys | Where-Object { -not $Checks[$_] })
    Write-Host "$Part : $(if ($Failed.Count -eq 0) { 'PASS' } else { 'FAIL' })"
    $Checks.Keys | Where-Object { -not $Checks[$_] } | ForEach-Object { Write-Host "  [KO] $_" }
    return $Failed.Count -eq 0
}

$Log = if (Test-Path $GameLog) { Get-Content $GameLog } else { @() }
# The game's time against the clock's (fixed time step, Docs/Archive/TestSpeedInProgress.md).
$SpeedLine = @($Log | Select-String -Pattern "Test sequence: done in .*" | ForEach-Object { $_.Matches[0].Value }) | Select-Object -Last 1
if ($SpeedLine) { Write-Host "game speed : $SpeedLine" }
# The log lines of a section, from its mark to the next one.
function Get-Section([string] $Mark) {
    $Start = -1
    for ($i = 0; $i -lt $Log.Count; $i++) {
        if ($Log[$i] -match "Test mark: (.*)$") {
            if ($Start -ge 0) { return $Log[$Start..$i] }
            if ($Matches[1] -eq $Mark) { $Start = $i }
        }
    }
    if ($Start -ge 0) { return $Log[$Start..($Log.Count - 1)] }
    return @()
}

# The reports of hol.Orders.Chains (D328) in some log lines, in order: the server's ("Order chains of slot S: N" and its N
# chains) or, with -Received, the local player's ("Order chains (received) of slot S: N"). Each report: its count, its
# slot, and per chain its number of units, its kinds ("Move -> AttackMove") and its steps as logged.
function Get-ChainReports($Lines, [switch] $Received) {
    $Head = if ($Received) { "Order chains \(received\) of slot (-?\d+): (\d+)$" } else { "Order chains of slot (-?\d+): (\d+)$" }
    $Body = if ($Received) { "Order chain \(received\) \d+: units ([^;]*); steps (.*)$" } else { "Order chain \d+: units ([^;]*); steps (.*)$" }
    $Reports = @(); $Report = $null
    foreach ($Line in $Lines) {
        if ($Line -match $Head) {
            $Report = [pscustomobject]@{ Slot = [int]$Matches[1]; Count = [int]$Matches[2]; Chains = @() }; $Reports += $Report
        } elseif ($Report -and $Line -match $Body) {
            $Steps = $Matches[2]
            $Report.Chains += [pscustomobject]@{ Units = @($Matches[1] -split " " | Where-Object { $_ }).Count; Steps = $Steps
                Kinds = (@([regex]::Matches($Steps, "(?:^| -> )(\w+)") | ForEach-Object { $_.Groups[1].Value }) -join " -> ") }
        }
    }
    return , $Reports
}
# A report in a few words: "none", or each chain's kinds and units ("Move -> AttackMove (3 units)").
function Format-ChainReport($Report) {
    if (-not $Report -or $Report.Count -eq 0) { return "none" }
    return (@($Report.Chains | ForEach-Object { "$($_.Kinds) ($($_.Units) units)" }) -join " | ")
}

$SequenceChecks = [ordered]@{}
$SequenceChecks["game launch ended by itself"] = -not $GameTimedOut
# The game quits at once on a script that does not compile (-as-exit-on-error): its first errors, by name.
$GameScriptErrors = @(Get-ScriptErrors $GameLog)
$SequenceChecks["scripts compiled ($(if ($GameScriptErrors.Count) { $GameScriptErrors -join ' ' } else { 'no error' }))"] = $GameScriptErrors.Count -eq 0
# Each capture shows the screen it is named after (the screens on top of the layers when it was taken; in a match,
# the game HUD of J3).
$Expected = @{ title = "menu=HOLTitleScreen modal=none"; stackback = "menu=HOLTitleScreen modal=none"; quit = "menu=HOLTitleScreen modal=HOLConfirmDialog";
    newgame = "menu=HOLNewGameScreen modal=none"; dropdown = "menu=HOLNewGameScreen modal=none"; video = "menu=HOLSettingsScreen modal=none";
    sound = "menu=HOLSettingsScreen modal=none"; controls = "menu=HOLSettingsScreen modal=none"; language = "menu=HOLSettingsScreen modal=none";
    start = "hud=HOLGameScreen"; end = "hud=HOLGameScreen"; own = "hud=HOLGameScreen"; other = "hud=HOLGameScreen";
    queue = "hud=HOLGameScreen"; done = "hud=HOLGameScreen"; berries = "hud=HOLGameScreen"; wood = "hud=HOLGameScreen"; idle = "hud=HOLGameScreen";
    popup = "hud=HOLGameScreen"; green = "hud=HOLGameScreen"; red = "hud=HOLGameScreen"; foundations = "hud=HOLGameScreen";
    site = "hud=HOLGameScreen"; three = "hud=HOLGameScreen"; finished = "hud=HOLGameScreen"; rally = "hud=HOLGameScreen";
    greyed = "hud=HOLGameScreen"; fr = "hud=HOLGameScreen"; en = "hud=HOLGameScreen"; ja = "hud=HOLGameScreen"; ghost = "hud=HOLGameScreen";
    block = "hud=HOLGameScreen"; popfull = "hud=HOLGameScreen"; f1 = "hud=HOLGameScreen"; f2 = "hud=HOLGameScreen"; next = "hud=HOLGameScreen";
    stone = "hud=HOLGameScreen"; play = "hud=HOLGameScreen"; melee = "hud=HOLGameScreen";
    summary = "hud=HOLGameScreen menu=HOLStatsScreen"; military = "hud=HOLGameScreen menu=HOLStatsScreen"; economy = "hud=HOLGameScreen menu=HOLStatsScreen";
    workers = "hud=HOLGameScreen menu=HOLStatsScreen"; army = "hud=HOLGameScreen menu=HOLStatsScreen" }
$WrongScreens = @($Log | Select-String -Pattern "Test capture: (\S+) \((.*)\)" | ForEach-Object {
    $Capture = $_.Matches[0].Groups[1].Value; $Shown = $_.Matches[0].Groups[2].Value
    $Want = $Expected[($Capture -split "_")[-1]]
    if ($Want -and $Shown -ne $Want) { "$Capture shows $Shown" } })
$SequenceChecks["captures show the right screen ($($WrongScreens -join '; '))"] = $WrongScreens.Count -eq 0
if ($Parts -contains "victory") {
    # C5: the timeline's curves of the second match: two players, several points each (a sample every 2 s), the defeat marked.
    $Timeline = @($Log | Select-String -Pattern "Stats screen: timeline (Workers|Military), (\d+) curves, (\d+) points, (\d+) marks, (.*)$" | ForEach-Object {
        [pscustomobject]@{ Series = $_.Matches[0].Groups[1].Value; Curves = [int]$_.Matches[0].Groups[2].Value; Points = [int]$_.Matches[0].Groups[3].Value
            Marks = [int]$_.Matches[0].Groups[4].Value; Axes = $_.Matches[0].Groups[5].Value } })
    $TimelineShown = @($Timeline | Where-Object { $_.Curves -eq 2 -and $_.Points -ge 12 -and $_.Marks -ge 1 } | ForEach-Object { $_.Series } | Select-Object -Unique)
    $TimelineText = (@($Timeline | Select-Object -Last 2 | ForEach-Object { "$($_.Series) $($_.Curves) curves $($_.Points) points $($_.Marks) marks, $($_.Axes)" }) -join "; ")
    $SequenceChecks["C5 timeline: workers and army curves of 2 players, the defeat marked ($TimelineText)"] = $TimelineShown.Count -eq 2
}
$SequenceChecks["sequence done (no step timed out)"] = [bool]($Log | Select-String -Pattern "Test mark: end" -Quiet)
$Errors = @($Log | Select-String -Pattern "LogHOLUI: (Error|Warning)|LogHOLMatch: (Error|Warning)|LogHOLTest: Error" | ForEach-Object { $_.Line })
# The "sizes" part's refused configuration logs its refusal on purpose (checked by the part itself).
if ($Parts -contains "sizes") {
    $Refused = @(Get-Section "sizes refused")
    $Errors = @($Errors | Where-Object { $Refused -notcontains $_ })
}
$SequenceChecks["no interface, match or sequence error ($($Errors.Count))"] = $Errors.Count -eq 0
$AllOk = (Report "sequence" $SequenceChecks) -and $AllOk
$Errors | Select-Object -First 5 | ForEach-Object { Write-Host "  $_" }

if ($Parts -contains "victory") {
    $Checks = [ordered]@{}
    $Section = Get-Section "victory surrender"
    foreach ($Want in "Game menu: opened", "Game menu: Surrender", "Victory: slot 0 defeated (Surrender)", "Victory: slot 0 collapsed", "Victory: match over, winners 1", "End banner: Defeat") {
        $Checks["surrender by Escape, menu, Abandonner, Oui: $Want"] = [bool]($Section | Select-String -SimpleMatch $Want -Quiet)
    }
    $Section = Get-Section "victory order"
    $Checks["an order after the defeat refused by the server (D358)"] = [bool]($Section | Select-String -SimpleMatch "Victory: orders of defeated slot 0 refused (D358)" -Quiet) -and
        -not ($Section | Select-String -SimpleMatch "Construction: slot 0 placed" -Quiet)
    $Section = Get-Section "victory escape"
    $Checks["menu over the banner opened then closed by Escape"] = [bool]($Section | Select-String -SimpleMatch "Game menu: opened" -Quiet) -and
        [bool]($Section | Select-String -SimpleMatch "Game menu: closed" -Quiet)
    # V1.4a: Escape still the banner's Back (Continuer a regarder) once the menu over it is closed, not the menu again.
    $Reopened = @($Section | Select-String -SimpleMatch "Game menu: opened").Count
    $Checks["then Escape closes the banner (End banner: closed, menu opened $Reopened time(s))"] = [bool]($Section | Select-String -SimpleMatch "End banner: closed" -Quiet) -and $Reopened -eq 1
    $Section = Get-Section "victory win"
    $Checks["AI defeated: match over, winners 0"] = [bool]($Section | Select-String -SimpleMatch "Victory: match over, winners 0" -Quiet)
    $Checks["End banner: Victory"] = [bool]($Section | Select-String -SimpleMatch "End banner: Victory" -Quiet)
    # C4: the statistics screen opened from the banner, closed by Back; the quit to the menu follows in this section
    # (the "victory win" section ends at this mark).
    $Section = Get-Section "victory stats"
    $Checks["Statistiques: the stats screen opened, then closed"] = [bool]($Section | Select-String -SimpleMatch "Stats screen: opened" -Quiet) -and
        [bool]($Section | Select-String -SimpleMatch "Stats screen: closed" -Quiet)
    # The menu's map loaded after the click.
    $Quit = @($Section | Select-String -SimpleMatch "End banner: quit to menu" | Select-Object -First 1)
    $After = if ($Quit.Count) { $Section[($Quit[0].LineNumber - 1)..($Section.Count - 1)] } else { @() }
    $Checks["Quitter vers le menu: MainMenu loaded"] = [bool]($After | Select-String -Pattern "LoadMap: /Game/Maps/MainMenu" -Quiet)
    $AllOk = (Report "victory" $Checks) -and $AllOk

    # V2.5: the main forum's lives (the third match). The last HUD report of a section: its players' table and the rest.
    function ForumHud([string] $Mark) {
        $Line = @(Get-Section $Mark | Select-String -Pattern "HUD forum: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
        $Table = if ("$Line" -match ", players \[([^\]]*)\]") { $Matches[1] } else { "" }
        $Row0 = @($Table -split " \| " | Where-Object { $_ -match "^slot 0 " }) | Select-Object -First 1
        return [pscustomobject]@{ Line = "$Line"; Row0 = "$Row0" }
    }
    $Checks = [ordered]@{}
    $Section = Get-Section "victory forum fall"
    $Checks["FallForum 0: the forum fell, lives 1"] = [bool]($Section | Select-String -SimpleMatch "Victory: forum of slot 0 fell, lives 1 " -Quiet)
    $Checks["a production on the forum down refused by the server"] = [bool]($Section | Select-String -SimpleMatch "Production order of slot 0 ignored: its forum is down" -Quiet)
    $Hud = ForumHud "victory forum fall"
    $Checks["table: slot 0 you, lives 1, ruin ($($Hud.Row0))"] = $Hud.Row0 -match "^slot 0 you colour \d+ lives 1 ruin$"
    $Checks["selection block: in ruin, lives 1 ($([regex]::Match($Hud.Line, ', in ruin[^,]*, lives \d+').Value))"] = $Hud.Line -match ", in ruin \d+ s, lives 1"
    # V4.5 (D353, D202): its own fall, the message with the alert (its camp), the banner, the sound, the thread's line.
    $OwnFell = "Votre forum est tombé — 1 vie"
    $Checks["message: $OwnFell (alert)"] = [bool]($Section | Select-String -SimpleMatch "Message: $OwnFell (alert)" -Quiet)
    $Checks["alert: $OwnFell, with its sound"] = [bool]($Section | Select-String -Pattern "Alert: $([regex]::Escape($OwnFell)) \(sound /Engine/[^,)]+\)" -Quiet)
    $AlertSection = @(Get-Section "victory forum alert")
    $Alerts = @($AlertSection | Select-String -Pattern "HUD alert: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Threads = @($AlertSection | Select-String -Pattern "HUD messages: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["HUD alert: shown, then none after 4.5 s ($($Alerts -join ' -> '))"] = ($Alerts.Count -eq 3) -and $Alerts[0] -eq "$OwnFell (slot 0)" -and $Alerts[1] -eq "none" -and $Alerts[2] -eq "none"
    $Checks["HUD messages: the line, still after 4.5 s, then gone after 9 s ($($Threads -join ' -> '))"] = ($Threads.Count -eq 3) -and $Threads[0] -eq "[$OwnFell]" -and $Threads[1] -eq "[$OwnFell]" -and $Threads[2] -eq "[]"
    $Checks["a click on the alert: the camera above the forum of slot 0"] = [bool]($AlertSection | Select-String -SimpleMatch "Camera above the forum of slot 0" -Quiet)
    # V3.5 (D348): the hold by the real presence; the HUD's table row "... ruin hold N s <state> by slot A".
    $Section = Get-Section "victory forum held"
    $Checks["3 Spearmen of slot 1 at 5 m: held by slot 1"] = [bool]($Section | Select-String -Pattern "Victory: forum of slot 0 held by slot 1 \(\d+ s\)" -Quiet)
    $Hud = ForumHud "victory forum held"
    $Held = [regex]::Match($Hud.Row0, " hold (\d+) s holding by slot 1$")
    $Checks["table: slot 0 hold N s holding by slot 1 ($($Hud.Row0))"] = $Held.Success -and [int]$Held.Groups[1].Value -ge 1
    $Checks["selection block: hold holding by slot 1 ($([regex]::Match($Hud.Line, ', hold [^,]*').Value))"] = $Hud.Line -match ", hold \d+ s holding by slot 1, lives 1"
    $Section = Get-Section "victory forum contested"
    $Checks["a Peasant of mine in the zone: contested"] = [bool]($Section | Select-String -Pattern "Victory: forum of slot 0 contested \(\d+ s\)" -Quiet)
    $Hud = ForumHud "victory forum contested"
    $Checks["table: slot 0 hold contested ($($Hud.Row0))"] = $Hud.Row0 -match " hold \d+ s contested( by slot 1)?$"
    $Section = Get-Section "victory forum released"
    $Released = @($Section | Select-String -Pattern "Victory: forum of slot 0 released \((\d+) s\)" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }) | Select-Object -First 1
    $Receding = @($Section | Select-String -Pattern "HUD forum: .*\[slot 0 [^|\]]* hold (\d+) s receding" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }) | Select-Object -First 1
    $Checks["the Spearmen taken away: released ($Released s), the count receding ($Receding s)"] = ($null -ne $Released) -and ($null -ne $Receding) -and $Receding -lt $Released
    $Checks["then no longer held"] = [bool]($Section | Select-String -SimpleMatch "Victory: forum of slot 0 no longer held" -Quiet)
    $Hud = ForumHud "victory forum released"
    $Checks["table: slot 0 ruin, no hold shown ($($Hud.Row0))"] = $Hud.Row0 -match "^slot 0 you colour \d+ lives 1 ruin$"
    # V4.5: the hold announced once (held, contested, released: one hold by slot 1), with the alert.
    $HeldMessages = @(@(Get-Section "victory forum held") + @(Get-Section "victory forum contested") + @(Get-Section "victory forum released") |
        Select-String -Pattern "Message: .*tient.*$" | ForEach-Object { $_.Matches[0].Value })
    $Checks["message once: IA Normal 2 tient votre ruine (alert) ($($HeldMessages -join '; '))"] = ($HeldMessages -join '|') -eq "Message: IA Normal 2 tient votre ruine (alert)"
    $Section = Get-Section "victory forum build"
    $Checks["SkipRuin 0: ruin -> rebuilding"] = [bool]($Section | Select-String -SimpleMatch "Victory: forum of slot 0 ruin -> rebuilding" -Quiet)
    $AtWork = @($Section | Select-String -Pattern "Construction: slot 0 rebuilds its forum at .*, (\d+) builders at work" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $Checks["the units sent on it rebuild it ($($AtWork -join ', ') builders at work)"] = $AtWork.Count -gt 0 -and ($AtWork | Measure-Object -Maximum).Maximum -ge 1
    $Hud = ForumHud "victory forum build"
    $Builders = [regex]::Match($Hud.Line, ", rebuilding \d+%, builders (\d+), lives 1")
    $Checks["selection block: rebuilding, builders $($Builders.Groups[1].Value), lives 1"] = $Builders.Success -and [int]$Builders.Groups[1].Value -ge 1
    $Checks["table: slot 0 rebuilding ($($Hud.Row0))"] = $Hud.Row0 -match "^slot 0 you colour \d+ lives 1 rebuilding \d+%$"
    $Section = Get-Section "victory forum rebuilt"
    $Checks["SkipRuin 0: rebuilt"] = [bool]($Section | Select-String -SimpleMatch "Victory: forum of slot 0 rebuilt" -Quiet)
    $Hud = ForumHud "victory forum rebuilt"
    $Checks["table: slot 0 standing, lives 1 ($($Hud.Row0))"] = $Hud.Row0 -match "^slot 0 you colour \d+ lives 1 standing$"
    # V4.5: the AI's fall, the message alone (another camp): no alert.
    $Section = @(Get-Section "victory forum other")
    $OtherFell = @($Section | Select-String -Pattern "Message: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["FallForum 1: message alone ($($OtherFell -join '; '))"] = ($OtherFell -join '|') -eq "Le forum de IA Normal 2 est tombé — 1 vie"
    $OtherAlert = @($Section | Select-String -Pattern "HUD alert: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
    $Checks["FallForum 1: no Alert, HUD alert: none ($OtherAlert)"] = -not ($Section | Select-String -CaseSensitive -SimpleMatch "Alert: " -Quiet) -and "$OtherAlert" -eq "none"
    $Section = Get-Section "victory forum defeat"
    # V4.5: the message of its defeat; no alert banner under the banner Defeat (neither the defeat's nor its last fall's).
    # V4.5b: its own loss (its defeat, its last life) is no alert (GetMessage): the thread's lines alone.
    $Checks["message: Vous êtes vaincu, no (alert)"] = [bool]($Section | Select-String -Pattern "Message: Vous êtes vaincu$" -Quiet)
    $Checks["message: Votre forum est tombé — 0 vies, no (alert)"] = [bool]($Section | Select-String -Pattern "Message: Votre forum est tombé — 0 vies$" -Quiet)
    $DefeatAlert = @($Section | Select-String -Pattern "HUD alert: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
    $Checks["no alert of the defeat nor of its last fall (HUD alert: $DefeatAlert)"] = -not ($Section | Select-String -CaseSensitive -SimpleMatch "Alert: " -Quiet) -and "$DefeatAlert" -eq "none"
    foreach ($Want in "Victory: forum of slot 0 fell, lives 0 ", "Victory: slot 0 defeated (Forum)", "Victory: match over, winners 1", "End banner: Defeat") {
        $Checks["its last life: $($Want.Trim())"] = [bool]($Section | Select-String -SimpleMatch $Want -Quiet)
    }
    $Hud = ForumHud "victory forum defeat"
    $Checks["table: slot 0 defeated ($($Hud.Row0))"] = $Hud.Row0 -match "^slot 0 you colour \d+ lives \d+ defeated greyed struck$"
    $AllOk = (Report "victory forum" $Checks) -and $AllOk

    # V5.5a (Regicide): the fourth match, with the option.
    $Checks = [ordered]@{}
    $Section = @(Get-Section "victory regicide")
    foreach ($KingSlot in 0, 1) {
        $Born = @($Section | Select-String -Pattern "Regicide: king of slot $KingSlot spawned at \((-?\d+), (-?\d+)\)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value), $($_.Matches[0].Groups[2].Value)" })
        $Checks["the King of slot $KingSlot born ($($Born -join '; '))"] = $Born.Count -eq 1
    }
    $Hud = @($Section | Select-String -Pattern "HUD report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
    $Checks["selected: King (own), hp 250/250, the stop alone ($([regex]::Match("$Hud", 'selection [^,]*, hp [^,]*, grid [^,]*, orders [^,]*').Value))"] =
        "$Hud" -match "selection King \(own\), hp 250/250, grid hidden, orders S=Stop,"
    $Checks["population without the King ($([regex]::Match("$Hud", 'population \d+/\d+').Value))"] = "$Hud" -match "population 0/\d+"
    $Section = @(Get-Section "victory regicide pass")
    $KingHits = @($Section | Select-String -Pattern "Combat: step \d+ #\d+ King hits")
    $Checks["an enemy Peasant 5.5 m off: the King does not attack ($($KingHits.Count) hits)"] = $KingHits.Count -eq 0 -and [bool]($Section | Select-String -SimpleMatch "Spawn: 1 Peasant of slot 1" -Quiet)
    # IA0.4 B2b: the Peasant stayed by the King all along (the AI's brain off for this slot, "aiBrain": false), so the case
    # above was really tested; one sent to gather elsewhere would leave the 15 m and not be removed (a silent false green).
    $PassRemoved = @($Section | Select-String -Pattern "Remove: (\d+) units of slot 1 within 1500 of \(-16600, 0\)" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $Checks["the enemy Peasant still there after 4 s, removed ($($PassRemoved -join ', ') units)"] = ($PassRemoved -join ',') -eq "1"
    $Section = @(Get-Section "victory regicide riposte")
    $Struck = @($Section | Select-String -Pattern "Combat: step \d+ #\d+ ManAtArms hits #\d+ King for [\d.]+, (-?[\d.]+) HP left")
    $Riposte = @($Section | Select-String -Pattern "Combat: step \d+ #\d+ King hits #\d+ ManAtArms for ([\d.]+)")
    $Checks["an enemy Man-at-arms strikes the King ($($Struck.Count) hits), the King ripostes ($($Riposte.Count) hits, $(@($Riposte | ForEach-Object { $_.Matches[0].Groups[1].Value }) -join ', '))"] =
        $Struck.Count -ge 1 -and $Riposte.Count -ge 1
    # EtapeDefini I: its aggressor removed, the King no longer Attacking (combat turned off in the same frame); under the banner neither.
    $Hud = @($Section | Select-String -Pattern "HUD report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
    $Checks["its aggressor removed: the King no longer Attacking ($([regex]::Match("$Hud", 'selection [^,]*, [^,]*').Value))"] =
        "$Hud" -match "selection King \(own\), hp " -and "$Hud" -notmatch "Attacking"
    $Section = @(Get-Section "victory regicide refusal")
    $Refusals = @($Section | Select-String -Pattern "units ignored, .*$" | ForEach-Object { $_.Matches[0].Value })
    $Checks["sent on an enemy House: refused for its want of building attack ($($Refusals -join '; '))"] =
        [bool]($Section | Select-String -SimpleMatch "Order: 1 units attack House of slot 1" -Quiet) -and ($Refusals -join '|') -eq "units ignored, no building attack"
    $Section = @(Get-Section "victory regicide death")
    $Hud = @($Section | Select-String -Pattern "HUD report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
    $Checks["under the banner: the King not Attacking ($([regex]::Match("$Hud", 'selection [^,]*, [^,]*').Value))"] =
        "$Hud" -match "selection King \(own\), hp " -and "$Hud" -notmatch "Attacking"
    foreach ($Want in "hol.Victory.KillKing: slot 1, 1 King(s) at 0 health", "Regicide: king of slot 1 died", "Victory: slot 1 defeated (King)", "Victory: match over, winners 0", "End banner: Victory") {
        $Checks["KillKing 1: $Want"] = [bool]($Section | Select-String -SimpleMatch $Want -Quiet)
    }
    $Checks["my King alive (no death of slot 0)"] = -not (@(Get-Section "victory regicide") + @(Get-Section "victory regicide pass") + @(Get-Section "victory regicide riposte") + $Section |
        Select-String -SimpleMatch "Regicide: king of slot 0 died" -Quiet)
    $KingMessages = @($Section | Select-String -Pattern "Message: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["messages: the King's death, then the defeat, no (alert) ($($KingMessages -join '; '))"] = ($KingMessages -join '|') -eq "Le Roi de IA Normal 2 est mort|IA Normal 2 est vaincu"
    $KingAlert = @($Section | Select-String -Pattern "HUD alert: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
    $Checks["no alert (HUD alert: $KingAlert)"] = -not ($Section | Select-String -CaseSensitive -SimpleMatch "Alert: " -Quiet) -and "$KingAlert" -eq "none"
    $AllOk = (Report "victory regicide" $Checks) -and $AllOk
}

if ($Parts -contains "sides") {
    $Checks = [ordered]@{}
    # A section's box line, its reports (selection, bars, HUD), each the n-th of the section (-1: the last).
    function SidesLine($Section, [string] $Pattern, [int] $Index = -1) {
        $Found = @($Section | Select-String -Pattern $Pattern | ForEach-Object { $_.Matches[0].Groups[1].Value })
        if ($Found.Count -eq 0) { return "" }; return $Found[$Index]
    }
    # Every bar of the line selected and full ("#12 140/140 selected"), and as many as N.
    function SidesFullBars([string] $Bars, [int] $N) {
        $Items = @([regex]::Matches($Bars, "#\d+ (\d+)/(\d+)( selected)?"))
        return ($Items.Count -eq $N) -and -not ($Items | Where-Object { $_.Groups[1].Value -ne $_.Groups[2].Value -or -not $_.Groups[3].Success })
    }
    $Wanted = [ordered]@{
        "1" = @{ What = "box on enemies alone: the enemies"; Box = "8 units (enemy)"; Sel = "8 units (other): Archer x3, Spearman x5"; Bars = 8 }
        "2" = @{ What = "box on allies alone: the allies"; Box = "5 units (ally)"; Sel = "5 units (other): Archer x2, Spearman x3"; Bars = 5 }
        "3" = @{ What = "box on mine and enemies: mine"; Box = "4 units (own)"; Sel = "4 units (own): ManAtArms x4" }
        "4" = @{ What = "box on mine and allies: mine"; Box = "4 units (own)"; Sel = "4 units (own): ManAtArms x4" }
        "5" = @{ What = "box on allies and enemies: the allies alone"; Box = "5 units (ally)"; Sel = "5 units (other): Archer x2, Spearman x3"; Bars = 5 }
        "6" = @{ What = "allies selected, Shift + click on an enemy: the allies stay"; Sel = "5 units (other): Archer x2, Spearman x3" }
        "7" = @{ What = "mine selected, Shift + click on an enemy then an ally: mine stay"; Sel = "4 units (own): ManAtArms x4"; Twice = $true }
        "8 ally" = @{ What = "double click on an ally Spearman: its owner's 3"; Type = "3 units of type Spearman of slot 2"; Sel = "3 units (other): Spearman x3" }
        "8 enemy" = @{ What = "double click on an enemy Spearman: its owner's 5"; Type = "5 units of type Spearman of slot 1"; Sel = "5 units (other): Spearman x5" }
    }
    foreach ($Case in $Wanted.Keys) {
        $Want = $Wanted[$Case]; $Section = Get-Section "sides $Case"
        $Sel = SidesLine $Section "Selection report: selected (.*)$"
        $Seen = "selection '$Sel'"
        $Ok = $Sel -eq $Want.Sel
        if ($Want.Twice) { $First = SidesLine $Section "Selection report: selected (.*)$" 0; $Seen = "selections '$First', '$Sel'"; $Ok = $Ok -and $First -eq $Want.Sel }
        if ($Want.Box) { $Box = SidesLine $Section "Select: box, (.*)$" 0; $Seen += ", box '$Box'"; $Ok = $Ok -and $Box -eq $Want.Box }
        if ($Want.Type) { $Type = SidesLine $Section "Select: (\d+ units of type \w+ of slot \d+) on the screen"; $Seen += ", '$Type'"; $Ok = $Ok -and $Type -eq $Want.Type }
        if ($Want.Bars) {
            $Bars = SidesLine $Section "Health bars: (.*)$"; $Hud = SidesLine $Section "HUD report: (.*)$"
            # Another player's units: no command (the grid hidden, no order), their count in the panel's chips.
            $Full = SidesFullBars $Bars $Want.Bars; $Hidden = $Hud -match "\(other\), grid hidden, orders none,"
            $Seen += ", $($Want.Bars) full bars $Full, no command $Hidden"; $Ok = $Ok -and $Full -and $Hidden
        }
        $Checks["$Case. $($Want.What) ($Seen)"] = $Ok
    }
    # Allied combat: each report of a section (its last), "#id Type slot S hp H/M target T ... hits N damage D".
    function SidesFight([string] $Case) {
        $Section = Get-Section "sides fight $Case"
        $Units = @{}; $Head = $null
        foreach ($Line in $Section) {
            if ($Line -match "Combat report: step \d+ units \d+ fighting \d+ hits (\d+)") { $Head = [int64]$Matches[1]; $Units = @{} }
            elseif ($Line -match "  #(\d+) \w+ slot (\d+) hp (-?[\d.]+)/(\d+) target (\d+) attacking \d+ hits (\d+) damage ([\d.]+)") {
                $Units[$Matches[1]] = @{ Slot = [int]$Matches[2]; Hp = [double]$Matches[3]; Max = [int]$Matches[4]; Target = $Matches[5]; Hits = [int]$Matches[6]; Damage = [double]$Matches[7] }
            }
        }
        $Camp = @{ 0 = "A"; 2 = "A"; 1 = "B" }
        $SameCamp = @($Units.Keys | Where-Object { $T = $Units[$_].Target; $T -ne "0" -and $Units.ContainsKey($T) -and $Camp[$Units[$T].Slot] -eq $Camp[$Units[$_].Slot] })
        $Hurt = @($Units.Keys | Where-Object { $Units[$_].Slot -ne 1 -and $Units[$_].Hp -lt $Units[$_].Max })
        $Dealt = @{}; foreach ($Id in $Units.Keys) { $Dealt[$Units[$Id].Slot] += $Units[$Id].Damage }
        return @{ Hits = $Head; SameCamp = $SameCamp.Count; Hurt = $Hurt.Count; Dealt0 = [int]$Dealt[0]; Dealt2 = [int]$Dealt[2]; Units = $Units.Count }
    }
    $FightStart = SidesFight "setup"
    foreach ($Case in @("auto", "hold", "attackmove", "order", "rightclick")) {
        $F = SidesFight $Case
        $Checks["fight $Case. me and my ally side by side: nobody fights (hits $($FightStart.Hits) -> $($F.Hits), hurt $($F.Hurt), targets in the camp $($F.SameCamp), units $($F.Units))"] =
            $F.Hits -eq $FightStart.Hits -and $F.Hurt -eq 0 -and $F.SameCamp -eq 0
    }
    $Order = Get-Section "sides fight order"
    $Refused = @($Order | Select-String -Pattern "Attack order of slot 0 on #\d+ refused").Count
    $Checks["fight order. an attack order of mine on the ally's unit: refused by the server ($Refused refusal)"] = $Refused -ge 1
    $Right = Get-Section "sides fight rightclick"
    $Clicked = @($Right | Select-String -Pattern "Right click on unit #\d+").Count
    $Attacks = @($Right | Select-String -Pattern "Order: \d+ units attack").Count
    $Moves = @($Right | Select-String -Pattern "Order: \d+ units to \(").Count
    $Checks["fight rightclick. a right click on the ally's unit: a move as on mine, no attack (clicks $Clicked, attack orders $Attacks, moves $Moves)"] =
        $Clicked -eq 1 -and $Attacks -eq 0 -and $Moves -eq 1
    $F = SidesFight "enemy"
    $Checks["fight enemy. an enemy comes: mine and the ally's fight it (damage by slot 0 $($F.Dealt0), by slot 2 $($F.Dealt2), targets in the camp $($F.SameCamp))"] =
        $F.Dealt0 -gt 0 -and $F.Dealt2 -gt 0 -and $F.SameCamp -eq 0
    # Allied avoidance: my 4's distances to the order's point (the farthest, the sum) after 2 s and 4 s, and the wall's
    # units moved (mine excluded). The Units where reports: the wall's, mine after 2 s and 4 s, the wall's.
    function SidesAvoid([int] $Wall) {
        $Avoid = Get-Section "sides avoid $Wall"
        $Wheres = @($Avoid | Where-Object { $_ -match "Units where: " })
        $Left = @(); $Moved = 0
        if ($Wheres.Count -eq 4) {
            $Mine = @([regex]::Matches($Wheres[1], "#(\d+) slot 0 at") | ForEach-Object { [int]$_.Groups[1].Value } | Sort-Object -Descending | Select-Object -First 4)
            $Left = @($Wheres[1], $Wheres[2] | ForEach-Object { $D = @([regex]::Matches($_, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\)") | Where-Object { $Mine -contains [int]$_.Groups[1].Value } | ForEach-Object { [int][math]::Sqrt([math]::Pow([double]$_.Groups[2].Value + 8400, 2) + [math]::Pow([double]$_.Groups[3].Value, 2)) }); @{ Max = ($D | Measure-Object -Maximum).Maximum; Sum = ($D | Measure-Object -Sum).Sum; N = $D.Count } })
            $Before = @{}; foreach ($M in [regex]::Matches($Wheres[0], "#(\d+) slot $Wall at \((-?\d+), (-?\d+)\)")) { $Before[$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }
            foreach ($M in [regex]::Matches($Wheres[3], "#(\d+) slot $Wall at \((-?\d+), (-?\d+)\)")) { $B = $Before[$M.Groups[1].Value]; if ($B -and $Mine -notcontains [int]$M.Groups[1].Value -and [math]::Sqrt([math]::Pow($B[0] - [double]$M.Groups[2].Value, 2) + [math]::Pow($B[1] - [double]$M.Groups[3].Value, 2)) -gt 30) { ++$Moved } }
        }
        return @{ Left = $Left; Moved = $Moved; Text = (($Left | ForEach-Object { "max $($_.Max) sum $($_.Sum) of $($_.N)" }) -join " then ") + ", wall moved $Moved" }
    }
    $Own = SidesAvoid 0; $Ally = SidesAvoid 2
    $Checks["avoid. mine sent through a wall of the ally's: as through a wall of mine (mine: $($Own.Text); the ally's: $($Ally.Text))"] =
        $Own.Left.Count -eq 2 -and $Ally.Left.Count -eq 2 -and $Own.Left[1].N -eq 4 -and $Ally.Left[1].N -eq 4 -and [math]::Abs($Ally.Left[1].Sum - $Own.Left[1].Sum) -le 200 -and [math]::Abs($Ally.Moved - $Own.Moved) -le 2
    $AllOk = (Report "sides" $Checks) -and $AllOk
}

if ($Parts -contains "sizes") {
    foreach ($Size in $SizeCases) {
        $Section = Get-Section "sizes $($Size.Key)"
        $Checks = [ordered]@{}
        $Checks["map Plain_$($Size.Key) loaded, $($Size.Players) players"] = [bool]($Section | Select-String -SimpleMatch "Match on Plain_$($Size.Key): given configuration, $($Size.Players) occupied slots" -Quiet)
        $Checks["no Match config refused"] = -not ($Section | Select-String -SimpleMatch "Match config refused" -Quiet)
        # The places drawn: one per player, distinct, on the places of the size.
        $Drawn = $Section | Select-String -Pattern "Places drawn \(seed $($Size.Seed), teams together \d\): (.*)$" | Select-Object -First 1
        $Places = if ($Drawn) { @($Drawn.Matches[0].Groups[1].Value -split " " | ForEach-Object { [int]$_ }) } else { @() }
        $Taken = @($Places | Where-Object { $_ -ge 0 })
        $Checks["places drawn: $($Size.Players) distinct of $($Size.Places) ($($Places -join ' '))"] = ($Taken.Count -eq $Size.Players) -and
            (@($Taken | Select-Object -Unique).Count -eq $Size.Players) -and (@($Taken | Where-Object { $_ -ge $Size.Places }).Count -eq 0)
        # One main centre per player, on its place: the circle of the size (0.75 of the half side), at its place's angle.
        $Half = $Size.Side * $Tile / 2
        $Radius = 0.75 * $Half
        $Centres = @($Section | Select-String -Pattern "Main centre of slot (\d) \(colour \d\) at \((-?\d+), (-?\d+)\), yaw (-?\d+)$" | ForEach-Object {
            $g = $_.Matches[0].Groups; [pscustomobject]@{ Slot = [int]$g[1].Value; X = [int]$g[2].Value; Y = [int]$g[3].Value } })
        $Misplaced = @($Centres | Where-Object {
            $Place = if ($_.Slot -lt $Places.Count) { $Places[$_.Slot] } else { -1 }
            $Angle = (180 + $Place * 360 / $Size.Places) * [Math]::PI / 180
            $Want = @(($Radius * [Math]::Cos($Angle)), ($Radius * [Math]::Sin($Angle)))
            ($Place -lt 0) -or ([Math]::Sqrt([Math]::Pow($_.X - $Want[0], 2) + [Math]::Pow($_.Y - $Want[1], 2)) -gt $Tile * 1.5) } | ForEach-Object { "slot $($_.Slot) at ($($_.X), $($_.Y))" })
        $Points = @($Centres | ForEach-Object { "$($_.X),$($_.Y)" } | Select-Object -Unique)
        $Checks["$($Size.Players) main centres on $($Size.Players) distinct points, each on its place (r $([Math]::Round($Radius / 100)) m; $($Centres.Count) centres; off: $($Misplaced -join '; '))"] =
            ($Centres.Count -eq $Size.Players) -and ($Points.Count -eq $Size.Players) -and ($Misplaced.Count -eq 0)
        # The camera above slot 0's centre, then held on the bounds of the size past both corners.
        $Own = @($Centres | Where-Object { $_.Slot -eq 0 }) | Select-Object -First 1
        $Above = $Section | Select-String -Pattern "Camera above the start of slot 0 \((-?\d+), (-?\d+)\)" | Select-Object -First 1
        $Reports = @($Section | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), (-?\d+)\), height (\d+)" | ForEach-Object {
            $g = $_.Matches[0].Groups; @{ X = [int]$g[1].Value; Y = [int]$g[2].Value } })
        $Checks["camera above slot 0's centre ($(if ($Reports.Count) { "$($Reports[0].X), $($Reports[0].Y)" }))"] = $Own -and $Above -and ($Reports.Count -ge 1) -and
            ([int]$Above.Matches[0].Groups[1].Value -eq $Own.X) -and ([int]$Above.Matches[0].Groups[2].Value -eq $Own.Y) -and
            ([Math]::Abs($Reports[0].X - $Own.X) -le 2) -and ([Math]::Abs($Reports[0].Y - $Own.Y) -le 2)
        $Corners = if ($Reports.Count -eq 3) { "($($Reports[1].X), $($Reports[1].Y)) ($($Reports[2].X), $($Reports[2].Y))" } else { "$($Reports.Count) reports" }
        $Checks["camera held on the bounds of the size, +-$([Math]::Round($Half / 100)) m ($Corners)"] = ($Reports.Count -eq 3) -and
            ([Math]::Abs($Reports[1].X - $Half) -le 1) -and ([Math]::Abs($Reports[1].Y - $Half) -le 1) -and
            ([Math]::Abs($Reports[2].X + $Half) -le 1) -and ([Math]::Abs($Reports[2].Y + $Half) -le 1)
        if ($Size.Teams) {
            # "Équipes ensemble": each team's places one run of neighbours round the circle.
            foreach ($Team in @($Size.Teams | Select-Object -Unique)) {
                $Run = @(for ($i = 0; $i -lt $Size.Players; $i++) { if ($Size.Teams[$i] -eq $Team -and $i -lt $Places.Count) { $Places[$i] } })
                $Together = @(0..($Size.Places - 1) | Where-Object { $From = $_
                    @(0..($Run.Count - 1) | Where-Object { $Run -contains (($From + $_) % $Size.Places) }).Count -eq $Run.Count })
                $Checks["team $Team on neighbouring places ($($Run -join ' '))"] = ($Run.Count -eq 4) -and ($Together.Count -ge 1)
            }
        }
        if ($Size.Players -lt $Size.Places) {
            # Fewer players than places: spread round the circle, never more than one empty place between two neighbours.
            $Sorted = @($Taken | Sort-Object)
            $Gaps = @(for ($i = 0; $i -lt $Sorted.Count; $i++) { (($Sorted[($i + 1) % $Sorted.Count] - $Sorted[$i] + $Size.Places - 1) % $Size.Places) + 1 })
            $Checks["places spread round the circle (gaps $($Gaps -join ' '))"] = ($Gaps.Count -eq $Size.Players) -and (@($Gaps | Where-Object { $_ -gt 2 }).Count -eq 0)
        }
        $AllOk = (Report "sizes $($Size.Key)" $Checks) -and $AllOk
    }
    $Section = Get-Section "sizes refused"
    $Checks = [ordered]@{}
    $Checks["fixed places past the size: Match config refused on Plain_Small"] = [bool]($Section | Select-String -SimpleMatch "Match config refused on Plain_Small" -Quiet)
    $Checks["no centre, no place drawn"] = -not ($Section | Select-String -Pattern "Main centre of slot|Places drawn" -Quiet)
    $Checks["no crash: back to the menu"] = [bool]((Get-Section "sizes end") | Select-String -Pattern "Test mark: sizes menu" -Quiet)
    $AllOk = (Report "sizes refused" $Checks) -and $AllOk
}

if ($Parts -contains "smoke") {
    $Checks = [ordered]@{}
    $Missing = @("title", "newgame", "play", "melee" | Where-Object { -not (Test-Path (Join-Path $Shots "qc_smoke_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $Section = Get-Section "smoke camera"
    # Every unit type built while the map loads, nothing loaded at a first birth in the match.
    $Preload = $Section | Select-String -Pattern "Unit types preloaded: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
    $Late = @($Log | Select-String -Pattern "built after the preload" | ForEach-Object { $_.Line -replace '^.*Unit type ', '' })
    $Checks["unit types preloaded ($Preload), none built in the match ($($Late -join '; '))"] = ($null -ne $Preload) -and ($Late.Count -eq 0)
    $Checks["match launched from the screen, slot 0 held"] = [bool]($Section | Select-String -Pattern "Match on \w+: given configuration" -Quiet) -and
        [bool]($Section | Select-String -Pattern "holds slot 0 " -Quiet)
    $Reports = @($Section | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), (-?\d+)\), height (\d+)" | ForEach-Object {
        $g = $_.Matches[0].Groups; @{ X = [int]$g[1].Value; Y = [int]$g[2].Value } })
    $Start = Get-SlotStart 0
    $Up = if ($Reports.Count -eq 2) { $Reports[1].X - $Reports[0].X } else { 0 }
    $Checks["camera above slot 0, arrow up 0.5 s: +X ~20 m (got $([Math]::Round($Up / 100)) m)"] = ($Reports.Count -eq 2) -and
        ([Math]::Abs($Reports[0].X - $Start[0]) -le 2) -and ([Math]::Abs($Reports[0].Y - $Start[1]) -le 2) -and ($Up -ge 1000) -and ($Up -le 3000)
    $Section = Get-Section "smoke selection"
    $Checks["its centre selected, then nothing"] = [bool]($Section | Select-String -SimpleMatch "Selection: MainCentre of slot 0 (own)" -Quiet) -and
        [bool]($Section | Select-String -SimpleMatch "Selection: none" -Quiet)
    $Section = Get-Section "smoke move"
    $Checks["3 Men-at-arms boxed"] = [bool]($Section | Select-String -SimpleMatch "Selection: 3 units (own): ManAtArms x3" -Quiet)
    $Kill = $Section | Select-String -Pattern "Combat: step \d+ #\d+ Peasant of slot 1 killed by #\d+ ManAtArms of slot 0 \(\d+ hits struck\)" | Select-Object -First 1
    $Checks["sent beside the enemy Peasant, they kill it ($(if ($Kill) { $Kill.Line -replace '^.*Combat: ', '' }))"] = $null -ne $Kill
    $Where = (Get-Section "smoke combat") | Select-String -Pattern "Units where: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
    $Walked = @([regex]::Matches([string]$Where, "slot 0 at \((-?\d+), (-?\d+)\)") | ForEach-Object {
        [Math]::Round([Math]::Sqrt([Math]::Pow([double]$_.Groups[1].Value - $SmokeTo[0], 2) + [Math]::Pow([double]$_.Groups[2].Value - $SmokeTo[1], 2)) / 100, 1) })
    $Checks["the 3 walked there (m from the point: $($Walked -join ', '))"] = ($Walked.Count -eq 3) -and (@($Walked | Where-Object { $_ -le 6 }).Count -eq 3)
    # The melee off right after the kill: the dying Peasant is destroyed all the same, 1 s after its death.
    $Report = (Get-Section "smoke dying") | Select-String -Pattern "Units report: (\d+) units: (.*)$" | Select-Object -First 1
    $Dying = if ($Report) { @([regex]::Matches($Report.Matches[0].Groups[2].Value, "#\d+ \w+ slot \d+ [^;]*, Dying(;|$)")).Count } else { -1 }
    $Checks["combat off: the dead Peasant destroyed all the same 1.5 s later ($Dying left dying)"] = $Dying -eq 0
    $AllOk = (Report "smoke" $Checks) -and $AllOk
}

if ($Parts -contains "menu") {
    foreach ($Culture in $Cultures) {
        $Checks = [ordered]@{}
        $Missing = @($MenuScreens | Where-Object { -not (Test-Path (Join-Path $Shots "qc_${Culture}_$_.png")) })
        $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
        $Checks["language switched"] = [bool]((Get-Section "menu $Culture") | Select-String -Pattern "Language $Culture$" -Quiet)
        if ($Culture -eq "fr") {
            $Checks["colour list opened and a value taken"] = (Test-Path (Join-Path $Shots "qc_fr_dropdown.png")) -and
                [bool]((Get-Section "menu fr") | Select-String -Pattern "SlotColor0: value 1 chosen" -Quiet)
            # E2-4: a closed line empty (faction, swatch, colour, team hidden), the player's shown; slot 3 taken: shown again.
            $Empty = Get-Section "menu emptyrow"
            $Taken = Get-Section "menu takenrow"
            $Fields = "SlotFaction", "SlotSwatch", "SlotColor", "SlotTeam"
            $Checks["closed line empty, player's line shown"] = (@($Fields | Where-Object {
                -not ($Empty | Select-String -SimpleMatch "Shown ${_}3: no" -Quiet) -or -not ($Empty | Select-String -SimpleMatch "Shown ${_}0: yes" -Quiet) }).Count -eq 0) -and
                (Test-Path (Join-Path $Shots "qc_fr_emptyrow.png"))
            $Checks["line taken (Human): fields shown again"] = (@($Fields | Where-Object {
                -not ($Taken | Select-String -SimpleMatch "Shown ${_}3: yes" -Quiet) }).Count -eq 0) -and
                [bool]($Taken | Select-String -SimpleMatch "SlotState3: value 1 chosen" -Quiet)
            # TC3: the size list (Small by default for one player), Medium for slot 3, Small greyed for three players, the
            # slots past the places closed, Small again for one.
            $Sizes = Get-Section "menu sizes"
            $Checks["size: Small by default for one player"] = [bool]((Get-Section "menu fr") | Select-String -SimpleMatch "New game: size Small, 2 places" -Quiet)
            $Checks["size: Medium chosen before taking slot 3"] = [bool]($Taken | Select-String -SimpleMatch "New game: size Medium, 4 places" -Quiet)
            $Checks["size and Équipes ensemble shown"] = [bool]($Sizes | Select-String -SimpleMatch "Shown SizeDropdown: yes" -Quiet) -and
                [bool]($Sizes | Select-String -SimpleMatch "Shown TeamsTogetherDropdown: yes" -Quiet)
            $SmallLines = @($Sizes | Select-String -SimpleMatch "New game: size Small, 2 places")
            $Checks["size: Small greyed for three players, chosen again for one ($($SmallLines.Count) time(s))"] = $SmallLines.Count -eq 1 -and
                @($Sizes | Select-String -SimpleMatch "SizeDropdown: value 1 chosen").Count -eq 1
            $Checks["teams open in solo, factions greyed (TC-Q12)"] = [bool]($Sizes | Select-String -Pattern "Shown SlotTeam0: yes$" -Quiet) -and
                [bool]($Sizes | Select-String -SimpleMatch "Shown SlotFaction0: yes, greyed" -Quiet) -and
                [bool]($Sizes | Select-String -Pattern "Shown TeamsTogetherDropdown: yes$" -Quiet)
            $Checks["slot 5 past the 4 places of Medium stays closed"] = [bool]($Sizes | Select-String -SimpleMatch "New game: slot 5 stays closed, past the 4 places of the size" -Quiet)
            # IA0.6 S1: "IA" chosen on slot 1 in solo (a greyed option is ignored by Choose: no "value 2 chosen"), colour and team shown.
            $AI = Get-Section "menu ai"
            $Checks["AI open in solo: slot 1 taken by an AI, colour and team shown"] = [bool]($AI | Select-String -SimpleMatch "SlotState1: value 2 chosen" -Quiet) -and
                [bool]($AI | Select-String -Pattern "Shown SlotColor1: yes$" -Quiet) -and [bool]($AI | Select-String -Pattern "Shown SlotTeam1: yes$" -Quiet)
            # IA0.6 S2: the name shown, the level (Normal, shown and greyed), no separate name column, no level on a closed line.
            $SeenName = (@($AI | Select-String -Pattern "Text SlotState1: (.*)$") | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
            $SeenLevel = (@($AI | Select-String -Pattern "Text SlotLevel1: (.*)$") | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
            $LevelShown = [bool]($AI | Select-String -Pattern "Shown SlotLevel1: yes, greyed$" -Quiet)
            $NameGone = [bool]($AI | Select-String -Pattern "Shown SlotName1: (no|missing)$" -Quiet)
            $ClosedNoLevel = [bool]($AI | Select-String -Pattern "Shown SlotLevel2: no(, greyed)?$" -Quiet)
            $Checks["AI line merged (IA0.6 S2): name '$SeenName' (want 'IA Normal 2'), level '$SeenLevel' shown+greyed=$LevelShown (want 'Normal', True), no name column=$NameGone, closed line without level=$ClosedNoLevel"] =
                $SeenName -eq "IA Normal 2" -and $SeenLevel -eq "Normal" -and $LevelShown -and $NameGone -and $ClosedNoLevel
        }
        $AllOk = (Report "menu $Culture" $Checks) -and $AllOk
        if (Get-Command python -ErrorAction SilentlyContinue) {
            python (Join-Path $PSScriptRoot "make_sheet.py") $Shots "qc_$Culture" $MenuScreens | Out-Null
        }
    }
    # IA0.6 S3: the AI chosen in the list goes to the match: an AI slot in the given configuration, the brain thinking
    # for slot 1 (not the main menu's "thinking slots []"), its 5 starting Peasants sent to gather.
    $Checks = [ordered]@{}
    $Launch = Get-Section "menu ailaunch"
    $Checks["IA chosen in the list of slot 1"] = [bool]($Launch | Select-String -SimpleMatch "SlotState1: value 2 chosen" -Quiet)
    $Checks["launched from the screen (given configuration, 2 occupied slots)"] = [bool]($Launch | Select-String -Pattern "Match on \w+: given configuration, 2 occupied slots" -Quiet)
    $Checks["brain thinking for slot 1"] = [bool]($Launch | Select-String -SimpleMatch "thinking slots [1]" -Quiet)
    $Gather = @($Launch | Select-String -Pattern "AI slot 1 step \d+: (\d+) idle -> gather \w+ #\d+$") | Select-Object -First 1
    $Idle = if ($Gather) { [int]$Gather.Matches[0].Groups[1].Value } else { 0 }
    $Checks["its 5 Peasants sent to gather ($Idle)"] = $Idle -eq 5
    $AllOk = (Report "menu ailaunch" $Checks) -and $AllOk
}

if ($Parts -contains "camera") {
    foreach ($Case in $Cases) {
        $Section = Get-Section "camera $($Case.Name)"
        $Reports = @($Section | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), (-?\d+)\), height (\d+)" | ForEach-Object {
            $g = $_.Matches[0].Groups; @{ X = [int]$g[1].Value; Y = [int]$g[2].Value; Height = [int]$g[4].Value } })
        $Expected = Get-SlotStart $Case.Slot
        $Checks = [ordered]@{}
        $Checks["launched from the screen"] = [bool]($Section | Select-String -Pattern "Match on \w+: given configuration" -Quiet)
        $Checks["match map, no pawn"] = [bool]($Section | Select-String -Pattern "Match map: RTS camera, no pawn" -Quiet)
        $Checks["holds slot $($Case.Slot)"] = [bool]($Section | Select-String -Pattern "holds slot $($Case.Slot) " -Quiet)
        $Checks["four reports"] = $Reports.Count -eq 4
        if ($Reports.Count -eq 4) {
            $Checks["starts above slot $($Case.Slot) ($($Expected -join ', '))"] = ([Math]::Abs($Reports[0].X - $Expected[0]) -le 2) -and ([Math]::Abs($Reports[0].Y - $Expected[1]) -le 2)
            $Up = $Reports[1].X - $Reports[0].X
            $Checks["arrow up 1 s: +X ~40 m (got $([Math]::Round($Up / 100)) m)"] = ($Up -ge 3000) -and ($Up -le 5000) -and ($Reports[1].Y -eq $Reports[0].Y)
            $Checks["wheel down: 35 -> 40 m (got $($Reports[2].Height / 100) m)"] = $Reports[2].Height -eq 4000
            $Edge = $Reports[3].Y - $Reports[2].Y
            $Checks["right edge 1 s: +Y (got $([Math]::Round($Edge / 100)) m)"] = ($Edge -ge 3000) -and ($Edge -le 6500)
        }
        $Checks["captures"] = (Test-Path (Join-Path $Shots "cam_$($Case.Name)_start.png")) -and (Test-Path (Join-Path $Shots "cam_$($Case.Name)_end.png"))
        # The main centres on the tile grid (D311): one per open slot, at its slot's grid point, along the grid lines.
        $Centres = @($Section | Select-String -Pattern "Main centre of slot (\d) \(colour \d\) at \((-?\d+), (-?\d+)\), yaw (-?\d+)$" | ForEach-Object { $_.Matches[0] })
        $OffGrid = @($Centres | Where-Object { $G = $_.Groups; $At = Get-SlotStart ([int]$G[1].Value)
            ([int]$G[2].Value -ne $At[0]) -or ([int]$G[3].Value -ne $At[1]) -or ([int]$G[2].Value % $Tile -ne 0) -or ([int]$G[3].Value % $Tile -ne 0) -or ([int]$G[4].Value % 90 -ne 0) })
        $OpenSlots = @($Case.States | Where-Object { $_ -ne "Closed" }).Count
        $Checks["main centres on the 4 m grid, along its lines ($($Centres.Count) of $OpenSlots; $(($Centres | ForEach-Object { "$($_.Groups[1].Value): ($($_.Groups[2].Value), $($_.Groups[3].Value)) yaw $($_.Groups[4].Value)" }) -join '; '))"] =
            ($Centres.Count -eq $OpenSlots) -and ($OffGrid.Count -eq 0)

        # Selection: four reports, in the order of the steps.
        $Selection = @((Get-Section "selection $($Case.Name)") | Select-String -Pattern "Selection report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $Own = "MainCentre of slot $($Case.Slot) (own)"
        $Other = "MainCentre of slot $($Case.Other) (other)"
        $Checks["selection: four reports ($($Selection.Count))"] = $Selection.Count -eq 4
        if ($Selection.Count -eq 4) {
            $Checks["click on its centre ($($Selection[0]))"] = $Selection[0] -eq "selected $Own"
            $Checks["click on another player's centre ($($Selection[1]))"] = $Selection[1] -eq "selected $Other"
            $Checks["click in the void: nothing ($($Selection[2]))"] = $Selection[2] -eq "selected none"
            $Checks["Escape: nothing ($($Selection[3]))"] = $Selection[3] -eq "selected none"
        }
        $Checks["selection captures"] = (Test-Path (Join-Path $Shots "sel_$($Case.Name)_own.png")) -and (Test-Path (Join-Path $Shots "sel_$($Case.Name)_other.png"))
        $AllOk = (Report "camera $($Case.Name)" $Checks) -and $AllOk
    }
}

if (($Parts -contains "production") -and ($Parts -contains "camera")) {
    $Section = Get-Section "production"
    $Hud = @($Section | Select-String -Pattern "HUD report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks = [ordered]@{}
    $Checks["HUD: centre selected, Peasant in the grid ($($Hud | Select-Object -First 1))"] = ($Hud.Count -ge 1) -and
        ($Hud[0] -match "resources F 200 W 200 S 200 G 200, population 0/10, selection MainCentre \(own\), grid A=Peasant enabled")
    $Checks["three queued (click, key A, key A)"] = [bool]($Section | Select-String -Pattern "queued Peasant \(3 in the queue, F 50 W 200 S 200 G 200\)" -Quiet)
    $Checks["HUD: 50 food, queue of 3 ($(if ($Hud.Count -ge 2) { $Hud[1] }))"] = ($Hud.Count -ge 2) -and ($Hud[1] -match "resources F 50 .* queue 3 ")
    $Checks["click on the queue cancels and refunds"] = [bool]($Section | Select-String -Pattern "cancelled Peasant \(2 in the queue, F 100 W 200 S 200 G 200\)" -Quiet)
    $Checks["two Peasants born"] = @($Section | Select-String -Pattern "Peasant of slot 0 done").Count -eq 2
    $Report = $Section | Select-String -Pattern "Production report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value } | Select-Object -Last 1
    $Checks["population 2/10, queue empty ($Report)"] = [bool]($Report -match "F 100 W 200 S 200 G 200, population 2/10, .* queue 0 ")
    $Checks["captures"] = (Test-Path (Join-Path $Shots "prod_queue.png")) -and (Test-Path (Join-Path $Shots "prod_done.png"))
    $AllOk = (Report "production" $Checks) -and $AllOk
}

if (($Parts -contains "move") -and ($Parts -contains "camera")) {
    $Paths = @((Get-Section "move") | Select-String -Pattern "Nav path: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Ratio = { param($Line) if ($Line -match "ratio ([0-9.]+)") { [double]$Matches[1] } else { 0.0 } }
    $Checks = [ordered]@{}
    $Checks["four paths ($($Paths.Count))"] = $Paths.Count -eq 4
    if ($Paths.Count -eq 4) {
        $Checks["navmesh: modifiers only, agent 60 cm ($($Paths[0]))"] = $Paths[0] -match "\(navmesh modifiers only, agent 60 cm\)$"
        # D324: around the centre's body (9.9 m of its 12 m of tiles), 1.12 (1.2 around its tiles before).
        $Checks["around the centre made at run time ($($Paths[0]))"] = (& $Ratio $Paths[0]) -gt 1.06
        $Checks["beside the centre: straight ($($Paths[1]))"] = ((& $Ratio $Paths[1]) -gt 0) -and ((& $Ratio $Paths[1]) -lt 1.02)
        $Checks["around the ridge ($($Paths[2]))"] = (& $Ratio $Paths[2]) -gt 1.2
        $Checks["across the grove ($($Paths[3]))"] = ((& $Ratio $Paths[3]) -gt 0) -and ($Paths[3] -notmatch "partial")
    }
    if ($Parts -contains "production") {
        # Orders (D289): unit positions of slot 0 from the "Units where" lines, the three produced last followed.
        $ReadWhere = { param($Line) $Units = @{}; foreach ($M in [regex]::Matches([string]$Line, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\)")) {
            $Units[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }; $Units }
        $Distance = { param($A, $B) if ($null -eq $A -or $null -eq $B) { return 1e9 }; [Math]::Sqrt([Math]::Pow($A[0] - $B[0], 2) + [Math]::Pow($A[1] - $B[1], 2)) }
        $Solo = Get-Section "move solo"
        $AtRally = & $ReadWhere ($Solo | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1)
        $Three = @($AtRally.Keys | Sort-Object | Select-Object -Last 3)
        $RallyGaps = @($Three | ForEach-Object { [Math]::Round((& $Distance $AtRally[$_] @(-13500, 0))) })
        $Checks["rally point set, three Peasants walked to it ($($RallyGaps -join ', ') cm)"] = ($AtRally.Count -eq 5) -and
            (@($RallyGaps | Where-Object { $_ -le 300 }).Count -eq 3)
        $Wanted = "3 units (own): Peasant x3"
        $Checks["box around them selects the three ($($Solo | Select-String -Pattern 'Selection report: selected (.*)$' | ForEach-Object { $_.Matches[0].Groups[1].Value }))"] =
            [bool]($Solo | Select-String -Pattern "Selection report: selected $([regex]::Escape($Wanted))" -Quiet) -and
            [bool]($Solo | Select-String -Pattern "Selection markers: rings 3" -Quiet)
        $Checks["Shift + click takes one out, then back in"] = [bool]($Solo | Select-String -Pattern "Selection: 2 units \(own\): Peasant x2" -Quiet)
        # The rally line (D328): the Peasant(s) born and still walking have a chain "Rally" to the rally point (server and
        # received), none once all arrived.
        $RallyServer = Get-ChainReports $Solo
        $RallyReceived = Get-ChainReports $Solo -Received
        $IsRally = { param($Report) ($Report.Count -ge 1) -and ($Report.Chains.Count -eq $Report.Count) -and
            (@($Report.Chains | Where-Object { -not ($_.Steps -match "^Rally at \((-?\d+), (-?\d+)\)$" -and [Math]::Abs([int]$Matches[1] + 13500) -le 300 -and [Math]::Abs([int]$Matches[2]) -le 300) }).Count -eq 0) }
        $Checks["rally: a chain Rally to (-13500, 0) for the Peasant born walking, server and received ($(Format-ChainReport $RallyServer[0]); $(Format-ChainReport $RallyReceived[0]))"] =
            ($RallyServer.Count -eq 2) -and ($RallyReceived.Count -eq 2) -and (& $IsRally $RallyServer[0]) -and (& $IsRally $RallyReceived[0])
        $Checks["rally: no chain once arrived, server and received ($(Format-ChainReport $RallyServer[-1]); $(Format-ChainReport $RallyReceived[-1]))"] =
            ($RallyServer.Count -eq 2) -and ($RallyReceived.Count -eq 2) -and ($RallyServer[1].Count -eq 0) -and ($RallyReceived[1].Count -eq 0)
        $Walk = @((Get-Section "move walk") | Select-String -Pattern "Units where: (.*)$" | ForEach-Object { & $ReadWhere $_.Matches[0].Groups[1].Value })
        $Arrived = Get-Section "move arrived"
        $End = & $ReadWhere ($Arrived | Select-String -Pattern "Units where: (.*)$" | Select-Object -First 1)
        $Ratios = @(); $Closest = [double]::MaxValue
        foreach ($Id in $Three) {
            $Points = @($Walk | Where-Object { $_.ContainsKey($Id) } | ForEach-Object { , $_[$Id] }) + @(, $End[$Id])
            $Length = 0.0; for ($i = 1; $i -lt $Points.Count; $i++) { $Length += & $Distance $Points[$i - 1] $Points[$i] }
            $Straight = & $Distance $Points[0] $Points[-1]
            $Ratios += if ($Straight -gt 0) { [Math]::Round($Length / $Straight, 2) } else { 0 }
            foreach ($Other in $Three) { if ($Other -gt $Id) { $Closest = [Math]::Min($Closest, (& $Distance $End[$Id] $End[$Other])) } }
        }
        $TargetGaps = @($Three | ForEach-Object { [Math]::Round((& $Distance $End[$_] @(-10500, 0))) })
        $Checks["right click behind the ridge: arrived ($($TargetGaps -join ', ') cm off the point)"] = (@($TargetGaps | Where-Object { $_ -le 300 }).Count -eq 3)
        $Checks["around the ridge (walk / straight $($Ratios -join ', '))"] = (@($Ratios | Where-Object { $_ -gt 1.1 }).Count -eq 3)
        $Checks["no overlap on arrival (closest $([Math]::Round($Closest)) cm, bodies 120 cm)"] = $Closest -ge 110
        $NavUnits = $Arrived | Select-String -Pattern "Nav units: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value } | Select-Object -First 1
        $Checks["no Peasant in a rock or a centre ($NavUnits)"] = [bool]($NavUnits -match "^5 units, 0 off the navmesh$")
        # The selection reported just before the Escape of "move arrived" (the three) and just after it (none).
        $EscapeAt = -1; for ($i = 0; $i -lt $Arrived.Count; $i++) { if ($Arrived[$i] -match "Key pressed: Escape") { $EscapeAt = $i; break } }
        $BeforeEscape = @(if ($EscapeAt -gt 0) { $Arrived[0..($EscapeAt - 1)] | Select-String -Pattern "Selection report: selected (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value } })
        $AfterEscape = @(if ($EscapeAt -ge 0) { $Arrived[$EscapeAt..($Arrived.Count - 1)] | Select-String -Pattern "Selection report: selected (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value } })
        $Checks["Escape clears the selection ($(if ($BeforeEscape.Count) { $BeforeEscape[-1] }) -> $(if ($AfterEscape.Count) { $AfterEscape[0] }))"] =
            ($BeforeEscape.Count -ge 1) -and ($BeforeEscape[-1] -eq $Wanted) -and ($AfterEscape.Count -ge 1) -and ($AfterEscape[0] -eq "none")
        $Checks["captures"] = (Test-Path (Join-Path $Shots "move_box.png")) -and (Test-Path (Join-Path $Shots "move_arrived.png"))
    }
    # The Shift queue kept in order: the 4 Peasants' middle reaches P1, then P2, then P3 (P2 started on its own step, not
    # overridden by P3 started in the same pass).
    $Fifo = @(Get-Section "move fifo")
    $FifoWhere = @($Fifo | Select-String -Pattern "Units where: (.*)$" | ForEach-Object { $Units = @{}; foreach ($M in [regex]::Matches($_.Matches[0].Groups[1].Value, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\)")) {
        $Units[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }; , $Units })
    if ($FifoWhere.Count -ge 10) {
        $Mine = @($FifoWhere[0].Keys | Where-Object { [Math]::Abs($FifoWhere[0][$_][0] - $FifoFrom[0]) -le 500 -and [Math]::Abs($FifoWhere[0][$_][1] - $FifoFrom[1]) -le 500 })
        $Checks["fifo: 4 Peasants made ($($Mine.Count))"] = $Mine.Count -eq 4
        $Reached = @()
        foreach ($Point in @(, $FifoP1) + @(, $FifoP2) + @(, $FifoP3)) {
            $At = -1
            for ($i = [Math]::Max(0, $(if ($Reached.Count) { $Reached[-1] } else { 0 })); $i -lt $FifoWhere.Count -and $At -lt 0; $i++) {
                $R = $FifoWhere[$i]; $In = @($Mine | Where-Object { $R.ContainsKey($_) })
                if ($In.Count -eq 4) {
                    $X = ($In | ForEach-Object { $R[$_][0] } | Measure-Object -Average).Average; $Y = ($In | ForEach-Object { $R[$_][1] } | Measure-Object -Average).Average
                    if ([Math]::Sqrt([Math]::Pow($X - $Point[0], 2) + [Math]::Pow($Y - $Point[1], 2)) -le 350) { $At = $i }
                }
            }
            $Reached += $At
        }
        $Checks["fifo: P1, then P2, then P3 reached in order (reports $($Reached -join ', '))"] = ($Reached[0] -ge 0) -and ($Reached[1] -gt $Reached[0]) -and ($Reached[2] -gt $Reached[1])
        Write-Host "  fifo: P1, P2, P3 reached at reports $($Reached -join ', ')"
    } else {
        $Checks["fifo reports ($($FifoWhere.Count))"] = $false
    }
    # Taken away at the end: the 4 of the scenario, no other unit of slot 0.
    $FifoRemoved = @(Get-Section "move fifo end" | Select-String -Pattern "Remove: (\d+) units of slot 0 within 800 of" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $Checks["fifo: its 4 Peasants taken away at the end ($($FifoRemoved -join ', '))"] = ($FifoRemoved.Count -eq 1) -and ($FifoRemoved[0] -eq 4)
    # The order lines (D328): the first report a chain Move -> Move -> AttackMove of the 3 Men-at-arms, server and received;
    # then (repeats merged) Move -> AttackMove, AttackMove, none; 3 lines and 3 points drawn while selected, none after.
    $LinesLog = @(Get-Section "move lines")
    $LinesServer = Get-ChainReports $LinesLog
    $LinesReceived = Get-ChainReports $LinesLog -Received
    $LinesFull = "Move -> Move -> AttackMove (3 units)"
    $Checks["lines: one chain Move -> Move -> AttackMove on the server ($(Format-ChainReport $LinesServer[0]))"] = ($LinesServer.Count -ge 1) -and ((Format-ChainReport $LinesServer[0]) -eq $LinesFull)
    $Checks["lines: the same chain received by its owner ($(Format-ChainReport $LinesReceived[0]))"] = ($LinesReceived.Count -ge 1) -and ((Format-ChainReport $LinesReceived[0]) -eq $LinesFull)
    $Merge = { param($Reports) $Out = @(); foreach ($R in $Reports) { $T = Format-ChainReport $R; if (-not $Out.Count -or $Out[-1] -ne $T) { $Out += $T } }; , $Out }
    $Shortened = @("Move -> Move -> AttackMove", "Move -> AttackMove", "AttackMove" | ForEach-Object { "$_ (3 units)" }) + "none"
    $ServerSteps = & $Merge $LinesServer
    $Checks["lines: shortened step by step, then none on arrival, server ($($ServerSteps -join ' / '))"] = ($ServerSteps -join "|") -eq ($Shortened -join "|")
    $ReceivedSteps = & $Merge $LinesReceived
    $Checks["lines: the same received ($($ReceivedSteps -join ' / '))"] = ($ReceivedSteps -join "|") -eq ($Shortened -join "|")
    $LinesMarkers = @($LinesLog | Select-String -Pattern "Selection markers: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["lines: 3 segments and 3 points drawn, the group selected ($($LinesMarkers | Select-Object -First 1))"] =
        ($LinesMarkers.Count -eq 2) -and ($LinesMarkers[0] -match "^rings 3,.*, order lines 3, order points 3,")
    $Checks["lines: none drawn without selection, the chain still received ($($LinesMarkers | Select-Object -Skip 1); $(Format-ChainReport $LinesReceived[1]))"] =
        ($LinesMarkers.Count -eq 2) -and ($LinesMarkers[1] -match "^rings 0,.*, order lines 0, order points 0,") -and ($LinesReceived.Count -ge 2) -and ($LinesReceived[1].Count -eq 1)
    $LinesRemoved = @(Get-Section "move lines end" | Select-String -Pattern "Remove: (\d+) units of slot 0 within 800 of" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $Checks["lines: its 3 Men-at-arms taken away at the end ($($LinesRemoved -join ', '))"] = ($LinesRemoved.Count -eq 1) -and ($LinesRemoved[0] -eq 3)
    # Born on a rock: the Man-at-arms made at the boulder (put on the ground beside it, within 6 m) leaves it and reaches its point (within 3 m) in 12 s.
    $RockWhere = @(Get-Section "move rock" | Select-String -Pattern "Units where: (.*)$" | ForEach-Object { $Units = @{}; foreach ($M in [regex]::Matches($_.Matches[0].Groups[1].Value, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\)")) {
        $Units[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }; , $Units })
    $RockUnit = @($(if ($RockWhere.Count) { $RockWhere[0].Keys | Where-Object { [Math]::Abs($RockWhere[0][$_][0] - $RockAt[0]) -le 600 -and [Math]::Abs($RockWhere[0][$_][1] - $RockAt[1]) -le 600 } }))
    $Checks["rock: 1 Man-at-arms made on the boulder ($($RockUnit.Count))"] = $RockUnit.Count -eq 1
    if ($RockUnit.Count -eq 1 -and $RockWhere.Count -ge 2) {
        $RockFirst = $RockWhere[0][$RockUnit[0]]; $RockLast = $RockWhere[-1][$RockUnit[0]]
        $RockLeft = [Math]::Sqrt([Math]::Pow($RockLast[0] - $RockGoal[0], 2) + [Math]::Pow($RockLast[1] - $RockGoal[1], 2))
        $RockMoved = [Math]::Sqrt([Math]::Pow($RockLast[0] - $RockFirst[0], 2) + [Math]::Pow($RockLast[1] - $RockFirst[1], 2))
        $Checks["rock: it reaches its point from the boulder (born at ($($RockFirst -join ', ')), at ($($RockLast -join ', ')) after 12 s, moved $([Math]::Round($RockMoved / 100, 1)) m, $([Math]::Round($RockLeft / 100, 1)) m left)"] = $RockLeft -le 300
    }
    $RockPath = @(Get-Section "move rock" | Select-String -Pattern "Nav (path|units): (.*)$" | ForEach-Object { $_.Matches[0].Value })
    Write-Host "  rock: $($RockPath -join ' | ')"
    $AllOk = (Report "move" $Checks) -and $AllOk
}

if (($Parts -contains "rocks") -and ($Parts -contains "camera")) {
    # Each case of $RockCases: near the ridge (x -12800 to -11200, y -1900 to 1900) a unit's centre off the navmesh is in a
    # rock or its zone; all the units standing (not Moving) within 12 m of the target (the point ordered is the block's
    # front, its rows behind it) by the end; the block held at most 10 steps while it re-forms.
    $Checks = [ordered]@{}
    $Summary = @()
    for ($c = 1; $c -le $RockCases.Count; $c++) {
        $R = $RockCases[$c - 1]
        $Section = @(Get-Section "rocks walk $c")
        $Navs = @($Section | Select-String -Pattern "Nav units: \d+ units, \d+ off the navmesh(.*)$")
        $InRock = 0; $MostInRock = 0
        $Zone = if ($R.Zone) { $R.Zone } else { @(-12800, -11200, -1900, 1900) }
        foreach ($N in $Navs) { $Here = 0
            foreach ($M in [regex]::Matches($N.Matches[0].Groups[1].Value, "#(\d+) at \((-?\d+), (-?\d+)\)")) { $X = [double]$M.Groups[2].Value; $Y = [double]$M.Groups[3].Value
                if ($X -ge $Zone[0] -and $X -le $Zone[1] -and $Y -ge $Zone[2] -and $Y -le $Zone[3]) { $Here++ } }
            $InRock += $Here; $MostInRock = [Math]::Max($MostInRock, $Here) }
        $Samples = @($Section | Select-String -Pattern "Units where: (.*)$" | ForEach-Object { $Units = @{}
            foreach ($M in [regex]::Matches($_.Matches[0].Groups[1].Value, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\) (\w+)")) { $Units[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value, $M.Groups[4].Value) }
            , $Units })
        $Army = if ($Samples.Count) { @($Samples[0].Keys | Where-Object { $P = $Samples[0][$_]; [Math]::Sqrt([Math]::Pow($P[0] - $R.X, 2) + [Math]::Pow($P[1] - $R.Y, 2)) -le $R.R + 600 }) } else { @() }
        $ArriveTime = 99.0; $Late = 0
        for ($i = 0; $i -lt $Samples.Count; $i++) { $S = $Samples[$i]
            $Out = @($Army | Where-Object { -not ($S.ContainsKey($_) -and $S[$_][2] -ne "Moving" -and [Math]::Sqrt([Math]::Pow($S[$_][0] - $R.TX, 2) + [Math]::Pow($S[$_][1] - $R.TY, 2)) -le 1200) })
            $Late = $Out.Count
            if ($Army.Count -gt 0 -and $Out.Count -eq 0) { $ArriveTime = ($i + 1) * 0.5; break } }
        $Arrived = @($Section | Select-String -Pattern "Block march: (\S+ )?arrived after \d+ steps of form-up and \d+ of march \((\d+) held\)")
        $HeldSteps = 0; foreach ($A in $Arrived) { $HeldSteps = [Math]::Max($HeldSteps, [int]$A.Matches[0].Groups[2].Value) }
        $Stuck = @($Section | Select-String -Pattern "Block march: (\S+ )?unit \d+ held the block").Count
        $Summary += "$c $($R.Name): rock $InRock ($MostInRock at once), $($Army.Count)/$($R.Count) arrived in $ArriveTime s ($Late out), held $HeldSteps, stuck $Stuck"
        $Checks["$c $($R.Name): $InRock unit-samples in a rock ($MostInRock at once, 160 samples: $($Navs.Count))"] = $Navs.Count -eq 160 -and $InRock -eq 0
        $Checks["$c $($R.Name): $($Army.Count) of $($R.Count) standing at the target in $ArriveTime s (all, at most 16 s; $Late still out at the end)"] = $Army.Count -eq $R.Count -and $ArriveTime -le 16.0
        # A unit holding it 0.5 s leaves it (MV.4.4's safety net, BlockMemberStuckSteps): counted, not a failure.
        $Checks["$c $($R.Name): block held $HeldSteps steps at most (at most 10), $Stuck units left it held"] = $HeldSteps -le 10
    }
    $Summary | ForEach-Object { Write-Host "rocks: $_" }
    $AllOk = (Report "rocks" $Checks) -and $AllOk
}

if (($Parts -contains "bodies") -and ($Parts -contains "camera")) {
    # MV.4: walkers blocked by bodies. The samples after the last order (every 0.5 s); the walkers of each case told by
    # where they stand in the first one; the time of a case: the first sample where all its walkers are there.
    $Checks = [ordered]@{}
    $Samples = @(Get-Section "bodies walk" | Select-String -Pattern "Units where: (.*)$" | ForEach-Object { $Units = @{}
        foreach ($M in [regex]::Matches($_.Matches[0].Groups[1].Value, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\) (\w+)")) { $Units[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }
        , $Units })
    $Checks["samples ($($Samples.Count) of 26)"] = $Samples.Count -eq 26
    $First = if ($Samples.Count -gt 0) { $Samples[0] } else { @{} }
    $Pick = { param($Y) @($First.Keys | Where-Object { $First[$_][0] -lt -9400 -and [Math]::Abs($First[$_][1] - $Y) -le 700 }) }
    $TimeAll = { param($Units, $There) $Units = @($Units | Where-Object { $null -ne $_ }); if ($Units.Count -eq 0) { return 99.0 }; for ($i = 0; $i -lt $Samples.Count; $i++) { $S = $Samples[$i]
            if (@($Units | Where-Object { $S.ContainsKey($_) -and (& $There $S[$_]) }).Count -eq $Units.Count) { return ($i + 1) * 0.5 } }
        return 99.0 }
    # The narrow way (the houses snapped at x -8800, y -3200 and -2400): past their bodies (x -9085 to -8515) on the far side.
    $Narrow = & $Pick -2800
    $NarrowTime = & $TimeAll $Narrow { param($P) $P[0] -ge -8400 }
    $Checks["narrow way: $($Narrow.Count) of 5 Peasants through in $NarrowTime s (at most 4 s)"] = ($Narrow.Count -eq 5) -and ($NarrowTime -le 4.0)
    # The enemy wall: the 3 Peasants arrived (within 4 m of the point) behind it.
    $Enemy = & $Pick 3000
    $EnemyTime = & $TimeAll $Enemy { param($P) [Math]::Abs($P[0] + 7600) -le 400 -and [Math]::Abs($P[1] - 3000) -le 400 }
    $Checks["enemy wall: $($Enemy.Count) of 3 Peasants behind it in $EnemyTime s (at most 12 s)"] = ($Enemy.Count -eq 3) -and ($EnemyTime -le 12.0)
    # Our wall: the 3 Men-at-arms arrived within the first sample's 0.5 s + the distance left from there / speed x 1.5. Their
    # order was given before the narrow way's, the samples timed from the latter: the distance and the time both from the
    # first sample (once over the whole 28 m with the walk before it counted as half a second: the speed seen ~3 times too
    # high, the limit 4 s instead of ~6). Their speed: the best half second of their mean advance along x.
    $Ours = & $Pick 0
    $Speed = 1.0
    for ($i = 1; $i -lt $Samples.Count -and $Ours.Count -gt 0; $i++) { $S0 = $Samples[$i - 1]; $S1 = $Samples[$i]
        if (@($Ours | Where-Object { $S0.ContainsKey($_) -and $S1.ContainsKey($_) }).Count -ne $Ours.Count) { continue }
        $Speed = [Math]::Max($Speed, (($Ours | ForEach-Object { $S1[$_][0] - $S0[$_][0] }) | Measure-Object -Average).Average / 0.5) }
    $OursLeft = if ($Ours.Count -gt 0) { [Math]::Max(0.0, (($Ours | ForEach-Object { -7600 - $First[$_][0] }) | Measure-Object -Maximum).Maximum) } else { 2800.0 }
    $OursLimit = [Math]::Round(0.5 + $OursLeft / $Speed * 1.5, 1)
    $OursTime = & $TimeAll $Ours { param($P) [Math]::Abs($P[0] + 7600) -le 400 -and [Math]::Abs($P[1]) -le 400 }
    $Checks["our wall: $($Ours.Count) of 3 Men-at-arms behind it in $OursTime s (at most $OursLimit s, $([Math]::Round($Speed)) cm/s)"] = ($Ours.Count -eq 3) -and ($OursTime -le $OursLimit)
    # MV.4.4 (the user's trial blocked_bodies: "they hit the still ones a little, then go round them"): the detour begun
    # soon enough that no walker touches a still body (closer than the two radii, hol.Units.ContactReport, every step,
    # out of a fight) over the three cases; a tolerance of 3 cm (a step's rounding).
    $ContactTolerance = 3
    $Contact = @(Get-Section "bodies walk" | Select-String -Pattern "Contact report: (\d+) contacts with still bodies in (\d+) walker-steps, closest gap (-?\d+) cm at \((-?\d+), (-?\d+)\)" | Select-Object -Last 1)
    $StillContacts = if ($Contact.Count) { [int]$Contact[0].Matches[0].Groups[1].Value } else { -1 }
    $StillGap = if ($Contact.Count) { [int]$Contact[0].Matches[0].Groups[3].Value } else { -9999 }
    $StillAt = if ($Contact.Count) { "($($Contact[0].Matches[0].Groups[4].Value), $($Contact[0].Matches[0].Groups[5].Value))" } else { "none" }
    Write-Host "bodies: $StillContacts contacts with still bodies in $(if ($Contact.Count) { $Contact[0].Matches[0].Groups[2].Value } else { '?' }) walker-steps, closest gap $StillGap cm at $StillAt"
    $Checks["no still body touched: closest gap $StillGap cm at $StillAt, $StillContacts walker-steps in contact (at least -$ContactTolerance cm)"] = ($Contact.Count -eq 1) -and ($StillGap -ge -$ContactTolerance)
    # The crossing (the user's trial crossing), on its own after the others: two groups of 5 Peasants face to face, 16 m apart,
    # each to the other's place. All arrived (within 4 m), and the heading flips counted by the game at every step (MV.4.3,
    # hol.Units.TurnReport: turns of more than 25 degrees between two steps) at most $FlipLimit: without the remembered detour
    # (hol.Units.DetourMemory 0, before MV.2) 72 in 403 walker-steps, with it 6 in 431 (MV.4.3; the samples every 0.5 s missed them).
    $FlipLimit = 20
    $CrossSection = @(Get-Section "bodies crossing")
    $Samples = @($CrossSection | Select-String -Pattern "Units where: (.*)$" | ForEach-Object { $Units = @{}
        foreach ($M in [regex]::Matches($_.Matches[0].Groups[1].Value, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\) (\w+)")) { $Units[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }
        , $Units })
    $Checks["crossing samples ($($Samples.Count) of 16)"] = $Samples.Count -eq 16
    $First = if ($Samples.Count -gt 0) { $Samples[0] } else { @{} }
    $CrossA = @($First.Keys | Where-Object { $First[$_][0] -lt -10000 -and [Math]::Abs($First[$_][1] - 6000) -le 700 })
    $CrossB = @($First.Keys | Where-Object { $First[$_][0] -ge -10000 -and [Math]::Abs($First[$_][1] - 6000) -le 700 })
    $CrossTime = [Math]::Max((& $TimeAll $CrossA { param($P) [Math]::Abs($P[0] + 9200) -le 400 -and [Math]::Abs($P[1] - 6000) -le 400 }),
        (& $TimeAll $CrossB { param($P) [Math]::Abs($P[0] + 10800) -le 400 -and [Math]::Abs($P[1] - 6000) -le 400 }))
    $Turn = @($CrossSection | Select-String -Pattern "Turn report: (\d+) flips over 25 deg in (\d+) walker-steps" | Select-Object -Last 1)
    $Flips = if ($Turn.Count) { [int]$Turn[0].Matches[0].Groups[1].Value } else { -1 }
    $WalkerSteps = if ($Turn.Count) { [int]$Turn[0].Matches[0].Groups[2].Value } else { -1 }
    Write-Host "bodies: narrow way $NarrowTime s, enemy wall $EnemyTime s, our wall $OursTime s (limit $OursLimit s); crossing arrived in $CrossTime s, $Flips heading flips in $WalkerSteps walker-steps"
    $Checks["crossing: $($CrossA.Count) + $($CrossB.Count) Peasants arrived in $CrossTime s (all, at most 8 s)"] = ($CrossA.Count -eq 5) -and ($CrossB.Count -eq 5) -and ($CrossTime -le 8.0)
    $Checks["crossing: $Flips heading flips over 25 degrees in $WalkerSteps walker-steps (at most $FlipLimit)"] = ($Flips -ge 0) -and ($WalkerSteps -gt 0) -and ($Flips -le $FlipLimit)
    $AllOk = (Report "bodies" $Checks) -and $AllOk
}

if (($Parts -contains "sources") -and ($Parts -contains "camera")) {
    $Section = Get-Section "sources"
    $Reports = @($Section | Select-String -Pattern "Sources \(server\): (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Paths = @($Section | Select-String -Pattern "Nav path: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks = [ordered]@{}
    $Checks["376 sources placed, shown, forbidden on the navmesh ($($Reports | Select-Object -First 1))"] =
        ($Reports.Count -eq 2) -and ($Reports[0] -eq "376 standing of 376 (Tree 320: 32000 left, BerryBush 48: 9600 left, StoneVein 4: 10000 left, GoldVein 4: 10000 left); shown 376, navmesh 376")
    $Checks["nearest tree is the first one ($($Section | Select-String -Pattern 'Sources: nearest (.*)$' | ForEach-Object { $_.Matches[0].Groups[1].Value }))"] =
        [bool]($Section | Select-String -Pattern "Sources: nearest 0 Tree at \(-14221, -1816\), 100 left" -Quiet)
    $Checks["three paths ($($Paths.Count))"] = $Paths.Count -eq 3
    if ($Paths.Count -eq 3) {
        $Checks["a standing tree closes its place ($($Paths[0]))"] = $Paths[0] -match "off the navmesh"
        $Checks["used up, its place opens ($($Paths[1]))"] = ($Paths[1] -notmatch "off the navmesh") -and ($Paths[1] -match "ratio") -and ($Paths[1] -notmatch "partial")
        $Checks["the tree behind still closes its place ($($Paths[2]))"] = $Paths[2] -match "off the navmesh"
    }
    $Checks["used up: 375 left, shown and forbidden ($($Reports | Select-Object -Last 1))"] =
        ($Reports.Count -eq 2) -and ($Reports[1] -eq "375 standing of 376 (Tree 319: 31900 left, BerryBush 48: 9600 left, StoneVein 4: 10000 left, GoldVein 4: 10000 left); shown 375, navmesh 375")
    $Checks["capture"] = Test-Path (Join-Path $Shots "src_wood.png")
    $AllOk = (Report "sources" $Checks) -and $AllOk
}

if (($Parts -contains "gather") -and ($Parts -contains "camera")) {
    # Units of slot 0 from a "Units where" line: identifier -> X, Y, activity, what it carries.
    $ReadUnits = { param($Line) $Units = @{}
        foreach ($M in [regex]::Matches([string]$Line, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\) (\w+)(?:, carries (\w+) (\d+|\?))?")) {
            $Units[[int]$M.Groups[1].Value] = @{ X = [double]$M.Groups[2].Value; Y = [double]$M.Groups[3].Value; Activity = $M.Groups[4].Value;
                Resource = $M.Groups[5].Value; Amount = $(if ($M.Groups[6].Value -match '^\d+$') { [int]$M.Groups[6].Value } else { 0 }) } }
        $Units }
    $Gap = { param($A, $B) [Math]::Sqrt([Math]::Pow($A.X - $B.X, 2) + [Math]::Pow($A.Y - $B.Y, 2)) }
    # The closest two units standing (gathering at their spot or idle), with the pair.
    $Closest = { param($Units) $Best = @(1e9, ""); $Ids = @($Units.Keys | Where-Object { $Units[$_].Activity -in "Gathering", "Idle" })
        for ($i = 0; $i -lt $Ids.Count; $i++) { for ($j = $i + 1; $j -lt $Ids.Count; $j++) {
            $D = & $Gap $Units[$Ids[$i]] $Units[$Ids[$j]]; if ($D -lt $Best[0]) { $Best = @($D, "#$($Ids[$i]) and #$($Ids[$j])") } } }
        $Best }
    $Food = { param($Line) if ([string]$Line -match "^F (\d+) ") { [int]$Matches[1] } else { -1 } }
    $Value = { param($Lines, [string] $Pattern) $Lines | Select-String -Pattern $Pattern | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value } }

    $Start = Get-Section "gather"
    $Working = Get-Section "gather working"
    $UsedUp = Get-Section "gather used up"
    $Idle = Get-Section "gather idle"
    $Checks = [ordered]@{}
    $Old = @((& $ReadUnits (& $Value $Start "Units where: (.*)$")).Keys)
    $AtWork = & $ReadUnits (& $Value $Working "Units where: (.*)$")
    $New = @($AtWork.Keys | Where-Object { $_ -notin $Old } | Sort-Object)
    $Checks["$GatherBefore Peasants before, $GatherTrees + $GatherBerries made, one produced ($($Old.Count) + $($New.Count))"] =
        ($Old.Count -eq $GatherBefore) -and ($New.Count -eq $GatherTrees + $GatherBerries + 1)
    $Trees = @($New | Select-Object -First $GatherTrees)
    $Berries = @($New | Select-Object -Skip $GatherTrees -First $GatherBerries)
    $Newborn = $New | Select-Object -Last 1

    # Rally point on a bush (D298): the Peasant produced gathers the patch (that bush or one beside it, D309); the food
    # rises only at the deposit (D296).
    $Rally = & $Value $Start "(Rally: slot 0 set the rally point of MainCentre at .*)$"
    $Checks["rally point on the berry bush ($Rally)"] = [bool]($Rally -match "at \($($GatherBerry -join ', ')\), on source \d+$")
    $Born = if ($null -ne $Newborn) { $AtWork[$Newborn] } else { $null }
    $Checks["the Peasant produced gathers the berries (#$Newborn $(if ($Born) { "$($Born.Activity) $($Born.Resource) $($Born.Amount)" }))"] =
        ($null -ne $Born) -and ($Born.Activity -eq "Gathering") -and ($Born.Resource -eq "Food") -and ($Born.Amount -ge 1) -and
        ((& $Gap $Born @{ X = $GatherBerry[0]; Y = $GatherBerry[1] }) -le 800)
    $Carried = 0; foreach ($Id in @($Berries) + @($Newborn)) { if ($null -ne $Id -and $AtWork.Contains($Id) -and $AtWork[$Id].Resource -eq "Food") { $Carried += $AtWork[$Id].Amount } }
    $FoodBorn = & $Food (& $Value $Start "Production report: (.*)$")
    $FoodWorking = & $Food (& $Value $Working "Production report: (.*)$")
    $Unloads = @($UsedUp | Select-String -Pattern "Gather: slot 0 unloaded (\d+) Food \(stock F (\d+) " | ForEach-Object { , @([int]$_.Matches[0].Groups[1].Value, [int]$_.Matches[0].Groups[2].Value) })
    $Unloaded = 0; $Rises = $true
    foreach ($U in $Unloads) { $Unloaded += $U[0]; if ($U[1] -ne $FoodBorn + $Unloaded) { $Rises = $false } }
    $FoodAfter = & $Food (& $Value $UsedUp "Production report: (.*)$")
    $Checks["food unchanged while carried ($FoodBorn -> $FoodWorking, $Carried carried)"] = ($FoodBorn -gt 0) -and ($FoodWorking -eq $FoodBorn) -and ($Carried -ge 1)
    $Checks["food rises at the deposit only, by what was carried ($($Unloads.Count) unloads, +$Unloaded, $FoodBorn -> $FoodAfter)"] =
        ($Unloads.Count -ge 1) -and $Rises -and ($Unloaded -ge $Carried) -and ($FoodAfter -eq $FoodBorn + $Unloaded) -and
        -not ($Start + $Working | Select-String -Pattern "Gather: slot 0 unloaded" -Quiet)

    # The HUD's counts (D307): gatherers per resource, idle workers.
    $HudWorking = & $Value $Working "HUD report: (.*)$"
    $Checks["HUD while gathering: F $($GatherBerries + 1), W $GatherTrees, idle $GatherBefore ($([regex]::Match("$HudWorking", 'gatherers .*$').Value))"] =
        [bool]($HudWorking -match ", gatherers F $($GatherBerries + 1) W $GatherTrees S 0 G 0, idle $GatherBefore$")

    # An order on a tree with more Peasants than its free spots (D309): the others gather the trees beside it.
    $Clicked = $Start | Select-String -Pattern "Order: $GatherTrees units gather source (\d+) \(Tree\) at \((-?\d+), (-?\d+)\)" | Select-Object -Last 1
    $TreeAt = if ($Clicked) { @{ X = [double]$Clicked.Matches[0].Groups[2].Value; Y = [double]$Clicked.Matches[0].Groups[3].Value } } else { @{ X = 1e9; Y = 1e9 } }
    $OnIt = @($Trees | Where-Object { $AtWork.Contains($_) -and $AtWork[$_].Activity -eq "Gathering" -and (& $Gap $AtWork[$_] $TreeAt) -le 200 })
    $Beside = @($Trees | Where-Object { $AtWork.Contains($_) -and $AtWork[$_].Activity -eq "Gathering" -and (& $Gap $AtWork[$_] $TreeAt) -gt 200 })
    $Farthest = ($Beside | ForEach-Object { & $Gap $AtWork[$_] $TreeAt } | Measure-Object -Maximum).Maximum
    $Checks["order on one tree: $($OnIt.Count) gather it, $($Beside.Count) the trees beside it (up to $([Math]::Round($Farthest)) cm), of $GatherTrees"] =
        ($null -ne $Clicked) -and ($OnIt.Count -ge 1) -and ($Beside.Count -ge 1) -and ($OnIt.Count + $Beside.Count -eq $GatherTrees) -and ($Farthest -le 2000)

    # The berries used up: every Peasant of the berries idle, the counter and the "." key (D307, D30).
    $AfterUse = & $ReadUnits (& $Value $UsedUp "Units where: (.*)$")
    $IdleIds = @($AfterUse.Keys | Where-Object { $AfterUse[$_].Activity -eq "Idle" } | Sort-Object)
    $WantIdle = $GatherBefore + $GatherBerries + 1
    $GoneBushes = @($UsedUp | Select-String -Pattern "Sources: source (\d+) used up" | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique)
    $Checks["six bushes used up, the berry Peasants idle ($($GoneBushes.Count) used up, $($IdleIds.Count) idle)"] = ($GoneBushes.Count -eq 6) -and
        ($IdleIds.Count -eq $WantIdle) -and (@(@($Berries) + @($Newborn) | Where-Object { $_ -notin $IdleIds }).Count -eq 0)
    $HudIdle = & $Value $UsedUp "HUD report: (.*)$"
    $Checks["HUD: idle $WantIdle, F 0, W $GatherTrees ($([regex]::Match("$HudIdle", 'gatherers .*$').Value))"] =
        [bool]($HudIdle -match ", gatherers F 0 W $GatherTrees S 0 G 0, idle $WantIdle$")
    $Picks = @($Idle | Select-String -Pattern "Next idle worker: #(\d+) \((\d+) idle\) at \((-?\d+), (-?\d+)\)" | ForEach-Object { $_.Matches[0] })
    $Cameras = @($Idle | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), " | ForEach-Object { @{ X = [double]$_.Matches[0].Groups[1].Value; Y = [double]$_.Matches[0].Groups[2].Value } })
    $Selected = @($Idle | Select-String -Pattern "Selection report: selected (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $PickIds = @($Picks | ForEach-Object { [int]$_.Groups[1].Value })
    $Centred = $true; $Followed = $true
    for ($i = 0; $i -lt $Picks.Count; $i++) {
        $At = @{ X = [double]$Picks[$i].Groups[3].Value; Y = [double]$Picks[$i].Groups[4].Value }
        if ($i -ge $Cameras.Count -or (& $Gap $Cameras[$i] $At) -gt 5) { $Centred = $false }
        if ($i -ge $Selected.Count -or $Selected[$i] -ne "Peasant #$($PickIds[$i]) of slot 0 (own)") { $Followed = $false }
    }
    $Checks["'.' three times, then the counter: the next idle one each time (#$($PickIds -join ', #'))"] = ($Picks.Count -eq 4) -and
        (@($PickIds | Sort-Object -Unique).Count -eq 4) -and (@($PickIds | Where-Object { $_ -notin $IdleIds }).Count -eq 0) -and
        (@($Picks | Where-Object { [int]$_.Groups[2].Value -ne $WantIdle }).Count -eq 0) -and
        ((($PickIds | Select-Object -First 3) -join ',') -eq (($PickIds | Select-Object -First 3 | Sort-Object) -join ','))
    $Checks["camera above it, it alone selected"] = ($Picks.Count -ge 1) -and $Centred -and $Followed
    $HudPick = & $Value $Idle "HUD report: (.*)$"
    $Checks["HUD: the idle one selected, the counter at $WantIdle"] = [bool]($HudPick -match "selection Peasant \(own\), .*, idle $WantIdle$")

    # Units at rest never on one another (D309: spots 100 cm apart, idle points 140 cm).
    $Near = & $Closest $AtWork
    $NearAfter = & $Closest $AfterUse
    $Checks["no two Peasants at rest closer than 90 cm ($([Math]::Round($Near[0])) cm $($Near[1]) gathering; $([Math]::Round($NearAfter[0])) cm $($NearAfter[1]) after)"] =
        ($Near[0] -ge 90) -and ($NearAfter[0] -ge 90)
    # The +10 (J5c.3, D298): one per unload of slot 0, with its amount and resource, at the centre (its deposit); from
    # the mark on, every unload logged by the simulation and every +10 by the controller, in the same step; up to the camps
    # of J6b.1 (their unloads are at the camps).
    $GatherMark = 0; for ($i = 0; $i -lt $Log.Count; $i++) { if ($Log[$i] -match "Test mark: gather$") { $GatherMark = $i; break } }
    $CampMark = $Log.Count - 1; for ($i = $GatherMark; $i -lt $Log.Count; $i++) { if ($Log[$i] -match "Test mark: build camp$") { $CampMark = $i; break } }
    $FromGather = $Log[$GatherMark..$CampMark]
    $UnloadKinds = @($FromGather | Select-String -Pattern "Gather: slot 0 unloaded ([1-9]\d*) (\w+) " | ForEach-Object { "+$($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" } | Sort-Object)
    $PopupLines = @($FromGather | Select-String -Pattern "Deposit popup: (\+\d+ \w+) at \((-?\d+), (-?\d+)\)" | ForEach-Object { $_.Matches[0] })
    $PopupKinds = @($PopupLines | ForEach-Object { $_.Groups[1].Value } | Sort-Object)
    $SlotStart = Get-SlotStart 0
    $PopupFar = @($PopupLines | Where-Object { [Math]::Sqrt([Math]::Pow([double]$_.Groups[2].Value - $SlotStart[0], 2) + [Math]::Pow([double]$_.Groups[3].Value - $SlotStart[1], 2)) -gt 1200 })
    $UsedUpPopups = @($UsedUp | Select-String -Pattern "Deposit popup: \+\d+ Food at" )
    $Checks["a +10 per unload, its amount and resource ($($PopupKinds.Count) shown: $(($PopupKinds | Group-Object | ForEach-Object { "$($_.Count) x $($_.Name)" }) -join ', '); $($UnloadKinds.Count) unloads)"] =
        ($UsedUpPopups.Count -ge 1) -and ($PopupKinds.Count -eq $UnloadKinds.Count) -and (($PopupKinds -join '|') -eq ($UnloadKinds -join '|'))
    $Checks["each +10 at the centre ($($PopupFar.Count) farther than 12 m)"] = ($PopupLines.Count -ge 1) -and ($PopupFar.Count -eq 0)
    $Missing = @("berries", "wood", "idle", "popup" | Where-Object { -not (Test-Path (Join-Path $Shots "gather_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "gather" $Checks) -and $AllOk

    # No body in a footprint (J6a.1, D310): the navmesh ends far enough from the centre's sides; over the gather part,
    # the order onto the centre and the one around it, every step, no Peasant's body entered a footprint.
    $Checks = [ordered]@{}
    $NavLine = $Start | Select-String -Pattern "Footprint navmesh: MainCentre of slot 0, (\d+) rays: sides (\d+) to \d+ cm, corners (\d+) to \d+ cm" | Select-Object -First 1
    $Checks["navmesh at 85 cm at least from the centre (agent 60 + clearance 25): $(if ($NavLine) { $NavLine.Matches[0].Value -replace '^Footprint navmesh: ', '' })"] =
        ($null -ne $NavLine) -and ([int]$NavLine.Matches[0].Groups[1].Value -ge 20) -and ([int]$NavLine.Matches[0].Groups[2].Value -ge 85) -and ([int]$NavLine.Matches[0].Groups[3].Value -ge 85)
    $Watch = (Get-Section "footprints") | Select-String -Pattern "Footprint watch: (\d+) steps, \d+ samples, closest body gap (-?\d+) cm \((.*)\); bodies in a footprint: (\d+) unit-steps, (\d+) units; .*; put back out: (\d+), new paths: (\d+)$" | Select-Object -Last 1
    $W = if ($Watch) { $Watch.Matches[0].Groups } else { $null }
    $Checks["no Peasant's body in a footprint ($(if ($W) { "$($W[1].Value) steps, closest gap $($W[2].Value) cm: $($W[3].Value); $($W[4].Value) unit-steps inside; put back out $($W[6].Value), new paths $($W[7].Value)" }))"] =
        ($null -ne $W) -and ([int]$W[1].Value -ge 200) -and ([int]$W[2].Value -ge 0) -and ([int]$W[4].Value -eq 0)
    $AllOk = (Report "footprints" $Checks) -and $AllOk
}

if (($Parts -contains "build") -and ($Parts -contains "camera")) {
    $Build = Get-Section "build"
    $Refused = Get-Section "build refused"
    $Cancel = Get-Section "build cancel"
    $Value = { param($Lines, [string] $Pattern) $Lines | Select-String -Pattern $Pattern | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value } }
    $Wood = { param($Line) if ([string]$Line -match "W (\d+) S") { [int]$Matches[1] } else { -1 } }
    $Checks = [ordered]@{}
    $Checks["120 wood to start ($(& $Value $Build 'Build report: (.*)$'))"] = (& $Wood (& $Value $Build "Build report: (F \d+ W \d+ S)")) -eq 120
    $Checks["placement mode: ghost green on free ground, shown"] = [bool]($Build | Select-String -Pattern "Placement: House \(cost F 0 W 50 S 0 G 0\)" -Quiet) -and
        [bool]($Build | Select-String -Pattern "Placement ghost: House at \($($BuildA -join ', ')\) valid" -Quiet) -and
        [bool]($Build | Select-String -Pattern "Selection markers: .*, ghost valid" -Quiet)
    $PlacedA = & $Value $Build "Construction: slot 0 placed House foundation at \($($BuildA -join ', ')\), (.*)$"
    $PlacedB = & $Value $Build "Construction: slot 0 placed House foundation at \($($BuildB -join ', ')\), (.*)$"
    $Checks["click: house A placed, 50 wood taken ($PlacedA)"] = [bool]($PlacedA -match "^paid F 0 W 50 S 0 G 0 \(F \d+ W 70 ")
    $Checks["Shift + click: house B touching it ($PlacedB)"] = [bool]($PlacedB -match "^paid F 0 W 50 S 0 G 0 \(F \d+ W 20 ")
    $Ended = @($Build | Select-String -Pattern "Placement: ended|Construction: slot 0 placed" | ForEach-Object { if ($_.Line -match "ended") { "ended" } else { "placed" } })
    $Checks["the mode left only by Escape, after both ($($Ended -join ', '))"] = ($Ended -join ',') -eq "placed,placed,ended"
    $Checks["ghost red near the enemy centre, its click places nothing, shown"] = [bool]($Build | Select-String -Pattern "Placement ghost: House at \(-?\d+, -?\d+\) refused \(too near an enemy main centre\)" -Quiet) -and
        [bool]($Build | Select-String -Pattern "Placement: not here \(too near an enemy main centre\)" -Quiet) -and
        [bool]($Build | Select-String -Pattern "Selection markers: .*, ghost refused" -Quiet) -and
        @($Build | Select-String -Pattern "Construction: slot 0 placed").Count -eq 2
    $Checks["clicks in placement mode select nothing"] = -not ($Build | Select-String -Pattern "Selection: " -Quiet)
    $Reasons = @($Refused | Select-String -Pattern "Construction: slot 0 cannot place House at \((-?\d+), (-?\d+)\): ([^(]+) \((.*)\)$" | ForEach-Object { $_.Matches[0] })
    $Said = ($Reasons | ForEach-Object { $_.Groups[3].Value.Trim() }) -join '; '
    $Checks["refused by the server: on a house, on a tree, near the enemy centre, not enough wood ($Said)"] = $Said -eq "on a building; on a resource source; too near an enemy main centre; not enough resources"
    $Checks["refused: nothing taken (wood $(($Reasons | ForEach-Object { & $Wood $_.Groups[4].Value }) -join ', '))"] = ($Reasons.Count -eq 4) -and
        (@($Reasons | Where-Object { (& $Wood $_.Groups[4].Value) -ne 20 }).Count -eq 0) -and ((& $Wood (& $Value $Refused "Build report: (F \d+ W \d+ S)")) -eq 20)
    $Selected = & $Value $Cancel "Selection report: selected (.*)$"
    $Markers = $Cancel | Select-String -Pattern "Selection markers: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
    $Hud = & $Value $Cancel "HUD report: (.*)$"
    $Checks["a click selects the foundation, its square, its block ($((@($Cancel | Select-String -Pattern 'Selection report: selected (.*)$' | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -First 1)); $Markers)"] =
        [bool](($Cancel | Select-String -Pattern "Selection report: selected House of slot 0 \(own\), foundation$" -Quiet)) -and [bool]($Markers -match "frame 1") -and
        [bool]($Hud -match "selection House \(own\), foundation")
    $Cancelled = & $Value $Cancel "(Construction: slot 0 cancelled House .*)$"
    $Report = & $Value $Cancel "Build report: (.*)$"
    $Checks["cancelled: 50 back, gone ($Cancelled)"] = [bool]($Cancelled -match "at \($($BuildB -join ', ')\), refund F 0 W 50 S 0 G 0 \(F \d+ W 70 ") -and
        [bool]($Report -match "W 70 ") -and [bool]($Report -match "House of slot 0 \(own\) foundation at \($($BuildA -join ', ')\)") -and
        -not ($Report -match "at \($($BuildB -join ', ')\)") -and ($Selected -eq "none")
    # The user's trial of J7b, feedback 3: on house A the ghost goes beside it, green; a click places it there.
    $Beside = Get-Section "build beside"
    $Moved = & $Value $Beside "Placement ghost: House at (.*moved.*)$"
    $Checks["the ghost on house A goes beside it, touching, nearest the cursor, green ($Moved)"] = $Moved -eq "($($BuildB -join ', ')) valid, moved beside the building (ring 0)"
    $PlacedBeside = & $Value $Beside "Construction: slot 0 placed House foundation at \($($BuildB -join ', ')\), (.*)$"
    $Checks["a click on house A places it beside ($PlacedBeside), then cancelled: refunded"] = [bool]($PlacedBeside -match "^paid F 0 W 50 S 0 G 0 \(F \d+ W 20 ") -and
        [bool]($Beside | Select-String -Pattern "Construction: slot 0 cancelled House at \($($BuildB -join ', ')\), refund F 0 W 50 S 0 G 0 \(F \d+ W 70 " -Quiet)
    $Missing = @("ghost_green", "ghost_red", "ghost_beside", "foundations" | Where-Object { -not (Test-Path (Join-Path $Shots "build_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build" $Checks) -and $AllOk

    # J6a.4 (D312): the site.
    $Site = Get-Section "build site"
    $Three = Get-Section "build three"
    $Help = Get-Section "build help"
    $Checks = [ordered]@{}
    $Cap = { param($Line) if ([string]$Line -match "population \d+/(\d+)") { [int]$Matches[1] } else { -1 } }
    $Paths = @($Site | Select-String -Pattern "Nav path: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Ratio = { param($Text) if ([string]$Text -match "ratio (\d+\.\d+)") { [double]$Matches[1] } elseif ([string]$Text -match "^none") { 99.0 } else { -1.0 } }
    $Checks["the ghost blocks nothing, the site blocks the navmesh after its first work ($($Paths -join ' / '))"] = ($Paths.Count -eq 2) -and
        ((& $Ratio $Paths[0]) -ge 0) -and ((& $Ratio $Paths[0]) -le 1.02) -and ((& $Ratio $Paths[1]) -ge 1.08)
    $Checks["a right click on the ghost: its Peasant sent to build it"] = [bool]($Site | Select-String -Pattern "Order: 1 units build House of slot 0 \(own\), foundation" -Quiet) -and
        [bool]($Site | Select-String -Pattern "Build order of slot 0: 1 units to House at \($($BuildA -join ', ')\)$" -Quiet)
    $SiteReport = & $Value $Site "Build report: (.*House of slot 0 \(own\) under construction.*)$"
    $Checks["A rises with its work ($([regex]::Match("$SiteReport", 'House of slot 0 \(own\) under construction \d+%').Value))"] = [bool]($SiteReport -match "House of slot 0 \(own\) under construction (\d+)% at \($($BuildA -join ', ')\)") -and
        ([int]$Matches[1] -ge 10) -and ([int]$Matches[1] -le 90)
    $FinishedA = & $Value $Site "(Construction: slot 0 finished House at \($($BuildA -join ', ')\) .*)$"
    $StepsA = if ([string]$FinishedA -match "after (\d+) steps") { [int]$Matches[1] } else { -1 }
    $Checks["A finished by one builder in the table's time, 20 s at speed 2: 200 steps ($FinishedA)"] = [bool]($FinishedA -match "after 200 steps of work \(by builders: 1 x200\), speed 2$")
    $Prods = @($Site | Select-String -Pattern "Production report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Huds = @($Site | Select-String -Pattern "HUD report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["population cap 10 -> 20 when A is finished, the HUD too ($(($Prods | ForEach-Object { & $Cap $_ }) -join ' -> '); HUD $(($Huds | ForEach-Object { & $Cap $_ }) -join ' -> '))"] =
        ($Prods.Count -eq 2) -and ((& $Cap $Prods[0]) -eq 10) -and ((& $Cap $Prods[1]) -eq 20) -and ($Huds.Count -eq 2) -and ((& $Cap $Huds[0]) -eq 10) -and ((& $Cap $Huds[1]) -eq 20)
    $Checks["its builder idle once A is finished"] = [bool]($Site | Select-String -Pattern "Build: a builder of slot 0 stops \(no site left\), idle" -Quiet)
    $ThreeLines = @($Three | ForEach-Object { $_ })
    $Order = @($ThreeLines | Select-String -Pattern "Construction: slot 0 (finished|first work on) House at \((-?\d+), (-?\d+)\)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" })
    $Checks["three Peasants: C then D (Shift) placed with them, C built, then D ($($Order -join '; '))"] = (@($ThreeLines | Select-String -Pattern "Build order of slot 0: 3 units to House at .*, queued$").Count -eq 2) -and
        (($Order -join '|') -eq "first work on ($($BuildC -join ', '))|finished ($($BuildC -join ', '))|first work on ($($BuildD -join ', '))")
    $FinishedC = & $Value $Three "(Construction: slot 0 finished House at \($($BuildC -join ', ')\) .*)$"
    $StepsC = if ([string]$FinishedC -match "after (\d+) steps") { [int]$Matches[1] } else { -1 }
    $ByThree = if ([string]$FinishedC -match "3 x(\d+)") { [int]$Matches[1] } else { 0 }
    $Checks["C by three builders about twice as fast as A by one ($StepsC steps against $StepsA; $FinishedC)"] = ($StepsA -gt 0) -and ($StepsC -gt 0) -and
        ($StepsC -le 0.6 * $StepsA) -and ($StepsC -ge 0.45 * $StepsA) -and ($ByThree -ge 0.8 * $StepsC)
    $Prods = @($Three | Select-String -Pattern "Production report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["population cap 30 with A and C ($(($Prods | ForEach-Object { & $Cap $_ }) -join ', '))"] = ($Prods.Count -eq 1) -and ((& $Cap $Prods[0]) -eq 30)
    $Checks["a right click on the site D: the first Peasant helps, 4 builders at work"] = [bool]($Help | Select-String -Pattern "Order: 1 units build House of slot 0 \(own\), under construction" -Quiet) -and
        [bool]($Help | Select-String -Pattern "House at \($($BuildD -join ', ')\), 4 builders at work" -Quiet)
    $CancelledD = & $Value $Help "(Construction: slot 0 cancelled House .*)$"
    # D345: the share of its 50 wood not built (1 to 49 once some work is done), added to the 70 wood left.
    $Checks["D cancelled after its first work: the share not built refunded ($CancelledD)"] = [bool]($CancelledD -match "at \($($BuildD -join ', ')\), refund F 0 W (\d+) S 0 G 0 \(F \d+ W (\d+) ") -and
        ([int]$Matches[1] -ge 1) -and ([int]$Matches[1] -le 49) -and ([int]$Matches[2] -eq 70 + [int]$Matches[1])
    $Idle = @($Help | Select-String -Pattern "Build: a builder of slot 0 stops \(no site left\), idle").Count
    $Checks["its four builders idle ($Idle)"] = $Idle -eq 4
    $FinalReport = & $Value $Help "Build report: (.*)$"
    $Prods = @($Help | Select-String -Pattern "Production report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["A and C finished, D gone, cap 30 ($(& $Cap $Prods[-1]))"] = [bool]($FinalReport -match "House of slot 0 \(own\) finished at \($($BuildA -join ', ')\)") -and
        [bool]($FinalReport -match "House of slot 0 \(own\) finished at \($($BuildC -join ', ')\)") -and -not ($FinalReport -match "at \($($BuildD -join ', ')\)") -and
        ($Prods.Count -eq 1) -and ((& $Cap $Prods[0]) -eq 30)
    $Missing = @("site", "three", "finished" | Where-Object { -not (Test-Path (Join-Path $Shots "build_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build site" $Checks) -and $AllOk

    # The user's trial of J6a.4: the rally point on a site, the nearby site, the first blow, a queue of sites and a source.
    $Rally = Get-Section "build rally"
    $Queued = Get-Section "build queue"
    $Checks = [ordered]@{}
    $Checks["a right click on the ghost E, the centre selected: its rally point on the site"] =
        [bool]($Rally | Select-String -Pattern "Order: rally point of MainCentre of slot 0 \(own\) at \($($BuildE -join ', ')\) on site House of slot 0 \(own\), foundation" -Quiet) -and
        [bool]($Rally | Select-String -Pattern "Rally: slot 0 set the rally point of MainCentre at \($($BuildE -join ', ')\), on site House$" -Quiet)
    $Flag = & $Value $Rally "Selection markers: (.*)$"
    $Checks["the flag shown on it ($Flag)"] = [bool]($Flag -match "flag 1")
    $Checks["the Peasant produced sent to build E"] = [bool]($Rally | Select-String -Pattern "Build order of slot -1: 1 units to House at \($($BuildE -join ', ')\)$" -Quiet)
    $FinishedE = & $Value $Rally "(Construction: slot 0 finished House at \($($BuildE -join ', ')\) .*)$"
    $Checks["E finished at its first blow, one step after contact ($FinishedE)"] = [bool]($FinishedE -match "after 1 steps of work \(by builders: 1 x1\), speed 1, instant$")
    $Nearby = @($Rally | Select-String -Pattern "Build: a builder of slot 0 goes on to the nearby site (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    # D324: measured between the bodies, 8 m between their tiles + 2 x 115 cm (BodyInset).
    $Checks["E finished, its builder goes on to F 8 m away ($($Nearby -join '; '))"] = ($Nearby.Count -eq 1) -and
        [bool]($Nearby[0] -match "^House at \($($BuildF -join ', ')\), 1030 cm from the one finished$")
    $Sites = @($Rally | Select-String -Pattern "Construction: slot 0 (finished|first work on) House at \((-?\d+), (-?\d+)\)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" })
    $Checks["E then F, each at its first blow, G (40 m) never started ($($Sites -join '; '))"] = ($Sites -join '|') -eq
        "first work on ($($BuildE -join ', '))|finished ($($BuildE -join ', '))|first work on ($($BuildF -join ', '))|finished ($($BuildF -join ', '))"
    $FinishedF = & $Value $Rally "(Construction: slot 0 finished House at \($($BuildF -join ', ')\) .*)$"
    $Checks["F at its first blow too ($FinishedF)"] = [bool]($FinishedF -match "after 1 steps of work \(by builders: 1 x1\), speed 1, instant$")
    $Idle = @($Rally | Select-String -Pattern "Build: a builder of slot 0 stops \(no site left\), idle").Count
    $RallyReport = & $Value $Rally "Build report: (.*)$"
    $Checks["then idle, G still a foundation ($Idle; $RallyReport)"] = ($Idle -eq 1) -and [bool]($RallyReport -match "House of slot 0 \(own\) foundation at \($($BuildG -join ', ')\)")
    $Checks["Shift + right click on a tree: queued"] = [bool]($Queued | Select-String -Pattern "Order: 1 units gather source \d+ \(Tree\) at .*, queued$" -Quiet) -and
        [bool]($Queued | Select-String -Pattern "Build order of slot 0: 1 units to gather source \d+ \(Wood\) at .*, queued$" -Quiet)
    $Tokens = @($Queued | ForEach-Object {
        if ([string]$_ -match "Construction: slot 0 finished House at \((-?\d+), (-?\d+)\)") { "finished ($($Matches[1]), $($Matches[2]))" }
        elseif ([string]$_ -match "Build: a builder of slot 0 goes on to gather source \d+ \((\w+)\)") { "gather $($Matches[1])" } })
    $Checks["H, then I, then the wood ($($Tokens -join '; '))"] = ($Tokens -join '|') -eq "finished ($($BuildH -join ', '))|finished ($($BuildI -join ', '))|gather Wood"
    $QueueHud = & $Value $Queued "HUD report: (.*)$"
    $Checks["the Peasant on its gathering's round trip ($([regex]::Match("$QueueHud", 'EHOLUnitActivity::\w+( EHOLResource::\w+ \d+/\d+)?').Value))"] =
        [bool]($QueueHud -match "EHOLUnitActivity::(Gathering|GatherTrip)")
    $Checks["capture (rally)"] = Test-Path (Join-Path $Shots "build_rally.png")
    $AllOk = (Report "build trial" $Checks) -and $AllOk

    # J6a.5 (D313): the construction HUD.
    $Hud = Get-Section "build hud"
    $HudGhost = Get-Section "build hud ghost"
    $HudSite = Get-Section "build hud site"
    $HudPopulation = Get-Section "build hud population"
    $HudF1 = Get-Section "build hud f1"
    $HudF2 = Get-Section "build hud f2"
    $HudTab = Get-Section "build hud tab"
    $HudAges = Get-Section "build hud ages"
    $Checks = [ordered]@{}
    $Grid = & $Value $Hud "HUD report: .*, grid (.*), queue "
    $Checks["Peasants without wood: the age panels, I open (house, lumber camp, mining camp, mill on A, Z, E, R, barracks and archery range on Q, S, farm on W, greyed), II to IV locked, none chosen; orders S, V ($Grid)"] = (@($Hud | Select-String -Pattern "HUD report: ").Count -ge 1) -and
        [bool](($Hud | Select-String -Pattern "HUD report: " | Select-Object -First 1).Line -match "grid ages I=open \[A=House greyed Z=LumberCamp greyed E=MiningCamp greyed R=Mill greyed Q=Barracks greyed S=ArcheryRange greyed W=Farm greyed\] II=locked \[\] III=locked \[\] IV=locked \[\] \(chosen none\), orders S=Stop V=Hold, queue ")
    # The user's trial of J6a.5: a cell writes the name alone, its cost is the tooltip's.
    $Cells = ($Hud | Select-String -Pattern "HUD cells: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["the cells write the name alone, no cost ($Cells)"] = [bool]($Cells -match "^S: Arr.+t; V: Tenir; I A: Maison; I Z: Camp de b.+cherons; I E: Camp de mineurs; I R: Moulin; I Q: Caserne; I S: Champ de tir; I W: Ferme$") -and -not ($Cells -match "\d")
    $Tooltips = ($Hud | Select-String -Pattern "HUD tooltips: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["tooltips: name, cost missing, effect ($Tooltips)"] = [bool]($Tooltips -match "^S: Arr.+t \| Annule les ordres et le combat; V: Tenir la position \| N'attaque .+; I A: Palier I \S+ Fondation; I A: Maison \| W 50 missing \| \+10 population; I Z: Camp de b.+cherons \| W 100 missing \| D.+p.+t : bois; I E: Camp de mineurs \| W 100 missing \| D.+p.+t : pierre, or; I R: Moulin \| W 100 missing \| D.+p.+t : nourriture; I Q: Caserne \| W 150 missing \| Produit : Lancier, Homme d'armes; I S: Champ de tir \| W 150 missing \| Produit : Archer, Arbal.+trier; I W: Ferme \| W 75 missing; II Z: Palier II \S+ Essor \| Palier non atteint; III E: Palier III \S+ Puissance \| Palier non atteint; IV R: Palier IV \S+ L.+gende \| Palier non atteint$")
    $Checks["a click on a greyed cell: no ghost, the message (not enough wood)"] = [bool]($Hud | Select-String -Pattern "Command feedback: not enough Wood" -Quiet) -and
        [bool]($Hud | Select-String -Pattern 'HUD message: NotEnoughWood "Pas assez de bois"' -Quiet) -and -not ($Hud | Select-String -Pattern "Placement: LumberCamp" -Quiet) -and
        [bool](($Hud | Select-String -Pattern "HUD report: " | Select-Object -Last 1).Line -match ", message NotEnoughWood")
    $Checks["the message in the three languages (HUD message lines: $(@($Hud | Select-String -Pattern 'HUD message: NotEnoughWood').Count))"] = @($Hud | Select-String -Pattern "HUD message: NotEnoughWood").Count -eq 3
    $Checks["with wood, the key Z: the lumber camp's ghost, the cells ready"] = [bool]($HudGhost | Select-String -Pattern "Placement: LumberCamp \(cost F 0 W 100 S 0 G 0\)" -Quiet) -and
        [bool]($HudGhost | Select-String -Pattern "A=House enabled Z=LumberCamp enabled E=MiningCamp enabled" -Quiet) -and [bool]($HudGhost | Select-String -Pattern "Placement: ended" -Quiet)
    $AgesReports = @($HudAges | Select-String -Pattern "HUD report: " | ForEach-Object { $_.Line })
    $Checks["the key Z (II not reached): nothing; A then A: the house's ghost, I chosen; then E: the mining camp's instead; then Z: the lumber camp's, I kept; Escape: ended, I closed; S of the command panel: stop ($(& $Value $HudAges '(Order: .*)$'))"] =
        [bool]($HudAges | Select-String -Pattern "Grid: age 1 not reached" -Quiet) -and -not ($HudAges | Select-String -Pattern "Grid: age -1" -Quiet) -and
        -not ($HudAges | Select-String -Pattern "Grid: age 1 chosen" -Quiet) -and ($AgesReports.Count -ge 5) -and ($AgesReports[0] -match "\(chosen none\)") -and
        [bool]($HudAges | Select-String -Pattern "Placement: House \(cost" -Quiet) -and ($AgesReports[1] -match "\(chosen I\)") -and
        [bool]($HudAges | Select-String -Pattern "Placement: MiningCamp \(cost" -Quiet) -and ($AgesReports[2] -match "\(chosen I\)") -and
        [bool]($HudAges | Select-String -Pattern "Placement: LumberCamp \(cost" -Quiet) -and ($AgesReports[3] -match "\(chosen I\)") -and
        [bool]($HudAges | Select-String -Pattern "Grid: age 0 closed" -Quiet) -and ($AgesReports[4] -match "\(chosen none\)") -and
        [bool]($HudAges | Select-String -Pattern "Order: \d+ units stop" -Quiet)
    $Checks["the key A with two Peasants: house J placed, they build it"] = [bool]($HudSite | Select-String -Pattern "Placement: House \(cost" -Quiet) -and
        [bool]($HudSite | Select-String -Pattern "Build order of slot 0: 2 units to House at \($($BuildJ -join ', ')\)$" -Quiet)
    $Block = & $Value $HudSite "HUD report: (.*)$"
    $Checks["the site's block: progress, builders, Annuler on V ($([regex]::Match("$Block", 'selection House[^,]*, under construction \d+%, builders \d+').Value); $([regex]::Match("$Block", 'V=Cancel \w+').Value))"] =
        [bool]($Block -match "selection House \(own\), under construction (\d+)%, builders (\d+), grid V=Cancel enabled") -and ([int]$Matches[1] -ge 1) -and ([int]$Matches[1] -le 60) -and ([int]$Matches[2] -ge 1)
    $CancelledJ = & $Value $HudSite "(Construction: slot 0 cancelled House .*)$"
    $Checks["Annuler clicked: the share not built refunded ($CancelledJ)"] = [bool]($CancelledJ -match "at \($($BuildJ -join ', ')\), refund F 0 W (\d+) S 0 G 0 ") -and
        ([int]$Matches[1] -ge 1) -and ([int]$Matches[1] -le 49)
    $Checks["population full: the key A queues, the message"] = [bool]($HudPopulation | Select-String -Pattern "Command feedback: population full" -Quiet) -and
        [bool]($HudPopulation | Select-String -Pattern "HUD message: PopulationFull" -Quiet) -and [bool]($HudPopulation | Select-String -Pattern "queued Peasant" -Quiet) -and
        [bool](($HudPopulation | Select-String -Pattern "HUD report: " | Select-Object -Last 1).Line -match "message PopulationFull")
    # F1, F3: no building of theirs, the message, nothing selected.
    $Checks["F1 and F3: no building, the message, nothing selected"] = [bool]($HudF1 | Select-String -Pattern "Category Military: no building" -Quiet) -and
        [bool]($HudF1 | Select-String -Pattern 'HUD message: NoMilitaryBuilding "Aucun b.+timent militaire"' -Quiet) -and
        [bool]($HudF1 | Select-String -Pattern "Category Technological: no building" -Quiet) -and [bool]($HudF1 | Select-String -Pattern "HUD message: NoTechnologicalBuilding" -Quiet) -and
        (@($HudF1 | Select-String -Pattern "Selection report: selected none").Count -eq 2) -and
        [bool](($HudF1 | Select-String -Pattern "HUD report: " | Select-Object -Last 1).Line -match "selection none, grid hidden, orders none, queue hidden, message NoMilitaryBuilding")
    # F2: every economic building of slot 0 (finished, sites and its ghosts: the user's 2nd trial of J6a.5) as the build
    # report lists them; the ghost G of the build trial among them.
    $Owned = @(([string](& $Value $HudF1 "(Build report: .*)$")) -split "; " | Where-Object { $_ -match "of slot 0 \(own\) (finished|under construction|foundation)" })
    $Ghosts = @($Owned | Where-Object { $_ -match "of slot 0 \(own\) foundation" })
    $Selected = & $Value $HudF2 "Category Economic: (\d+) buildings selected"
    $SelReport = & $Value $HudF2 "Selection report: selected (.*)$"
    $Checks["F2: every economic building selected, its ghosts too ($Selected; $($Owned.Count) in the build report, $($Ghosts.Count) ghosts: $SelReport)"] =
        ($Owned.Count -ge 3) -and ($Ghosts.Count -ge 1) -and ([int]$Selected -eq $Owned.Count) -and
        [bool]($SelReport -match "^$($Owned.Count) buildings \(own\): House x\d+.*MainCentre x1") -and
        [bool]($HudF2 | Select-String -Pattern "Selection markers: rings 0, frame $($Owned.Count)," -Quiet) -and
        [bool]($HudF2 | Select-String -Pattern "HUD report: .*selection $($Owned.Count) buildings \(own\): " -Quiet)
    $Cams = @($HudF2 | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), " | ForEach-Object { $_.Matches[0] })
    $Before = $HudF1 | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), " | Select-Object -Last 1 | ForEach-Object { $_.Matches[0] }
    $Checks["F2: the camera where it was ($($Before.Value) -> $(if ($Cams.Count -gt 0) { $Cams[0].Value }))"] = $Before -and ($Cams.Count -ge 1) -and
        ($Cams[0].Groups[1].Value -eq $Before.Groups[1].Value) -and ($Cams[0].Groups[2].Value -eq $Before.Groups[2].Value)
    $Next = @($HudF2 | Select-String -Pattern "Category Economic: next building (\w+) of slot 0 \(own\)[^(]*at \((-?\d+), (-?\d+)\) \((\d+) selected\)" | ForEach-Object { $_.Matches[0] })
    $Start0 = Get-SlotStart 0
    $Said = ($Next | ForEach-Object { "$($_.Groups[1].Value) ($($_.Groups[2].Value), $($_.Groups[3].Value))" }) -join '; '
    $Checks["F2 again: the centre, then two houses, the selection kept ($Said)"] = ($Next.Count -eq 3) -and ($Next[0].Groups[1].Value -eq "MainCentre") -and
        ([Math]::Abs([int]$Next[0].Groups[2].Value - $Start0[0]) -le 2) -and ([Math]::Abs([int]$Next[0].Groups[3].Value - $Start0[1]) -le 2) -and
        ($Next[1].Groups[1].Value -eq "House") -and ($Next[2].Groups[1].Value -eq "House") -and ("$($Next[1].Groups[2].Value),$($Next[1].Groups[3].Value)" -ne "$($Next[2].Groups[2].Value),$($Next[2].Groups[3].Value)") -and
        (@($Next | Where-Object { [int]$_.Groups[4].Value -eq $Owned.Count }).Count -eq 3) -and
        (@($HudF2 | Select-String -Pattern "Selection report: selected $($Owned.Count) buildings \(own\)").Count -eq 2)
    $Moves = @($Cams | Select-Object -Skip 1)
    $Checks["F2 again: the camera above each ($(($Moves | ForEach-Object { "($($_.Groups[1].Value), $($_.Groups[2].Value))" }) -join '; '))"] = ($Moves.Count -eq 3) -and ($Next.Count -eq 3) -and
        (@(0..2 | Where-Object { ([Math]::Abs([int]$Moves[$_].Groups[1].Value - [int]$Next[$_].Groups[2].Value) -le 2) -and ([Math]::Abs([int]$Moves[$_].Groups[2].Value - [int]$Next[$_].Groups[3].Value) -le 2) }).Count -eq 3)
    # Tab (the user's 2nd trial of J6a.5): « Tous » (A = the centre's Peasant, queued there), the centre alone, the houses.
    $TabHud = @($HudTab | Select-String -Pattern "HUD report: .*selection (\d+) buildings \(own\): [^,]*, subgroup ([^,]+), " | ForEach-Object { $_.Matches[0] })
    $TabCells = @($HudTab | Select-String -Pattern "HUD cells: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $TabSaid = ($TabHud | ForEach-Object { $_.Groups[2].Value }) -join ' -> '
    $Checks["Tab: All, then the centre, then the houses, then F2 anew on All ($TabSaid; cells $($TabCells -join ' | '))"] = ($TabHud.Count -eq 5) -and ($TabCells.Count -eq 5) -and
        ($TabHud[0].Groups[2].Value -eq "All ($($Owned.Count))") -and ($TabHud[1].Groups[2].Value -eq "All ($($Owned.Count))") -and
        ($TabHud[2].Groups[2].Value -eq "MainCentre") -and ($TabHud[3].Groups[2].Value -match "^House x\d+$") -and ($TabHud[4].Groups[2].Value -eq "All ($($Owned.Count))") -and
        ($TabCells[0] -eq "A: Paysan") -and ($TabCells[2] -eq "A: Paysan") -and ($TabCells[3] -eq "none") -and ($TabCells[4] -eq "A: Paysan")
    $Queued = & $Value $HudTab "(Grid: Peasant queued at .*)$"
    $Checks["Tous: the key A queues the Peasant at the centre ($Queued)"] = [bool]($Queued -match "^Grid: Peasant queued at MainCentre of slot 0 \(own\).*, subgroup All \(\d+\)$")
    $TabMarkers = @($HudTab | Select-String -Pattern "Selection markers: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Houses = if ($TabHud.Count -ge 4 -and $TabHud[3].Groups[2].Value -match "^House x(\d+)$") { [int]$Matches[1] } else { 0 }
    $Checks["the frames out of the active subgroup dimmer ($($TabMarkers -join ' | '))"] = ($TabMarkers.Count -eq 2) -and ($Houses -ge 1) -and
        ($TabMarkers[0] -match "frame $($Owned.Count),.*frames dimmed $($Owned.Count - 1)$") -and ($TabMarkers[1] -match "frame $($Owned.Count),.*frames dimmed $($Owned.Count - $Houses)$")
    # The user's 3rd trial of J6a.5: the centre's rally flag (on E) shown on « Tous » and on its subgroup, not on the houses'.
    $HudRally = Get-Section "build hud rally"
    $RallyMarkers = @($HudRally | Select-String -Pattern "Selection markers: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $FlagE = [regex]::Escape("flag 1 at ($($BuildE -join ', ')),")
    $Checks["the centre's flag on E: on « Tous » and on the centre's subgroup, none on the houses' ($(@($RallyMarkers | Select-Object -First 1) + $TabMarkers -join ' | '))"] =
        ($RallyMarkers.Count -eq 2) -and ($TabMarkers.Count -eq 2) -and ($RallyMarkers[0] -match $FlagE) -and ($TabMarkers[0] -match $FlagE) -and ($TabMarkers[1] -match "flag 0,")
    $GroupOrder = & $Value $HudRally "(Order: rally point of .*)$"
    $Moved = if ($RallyMarkers.Count -ge 2 -and $RallyMarkers[1] -match "flag 1 at \((-?\d+), (-?\d+)\),") { @([int]$Matches[1], [int]$Matches[2]) } else { @() }
    $Checks["a right click on « Tous »: the group's rally point, the flag moved there ($GroupOrder; $(if ($RallyMarkers.Count -ge 2) { $RallyMarkers[1] }))"] =
        [bool]($GroupOrder -match "^Order: rally point of 1 buildings of the group at ") -and ($Moved.Count -eq 2) -and
        ([Math]::Abs($Moved[0] - $BuildRally[0]) -le 50) -and ([Math]::Abs($Moved[1] - $BuildRally[1]) -le 50)
    $Missing = @("greyed", "fr", "en", "ja", "ghost", "block", "popfull", "f1", "f2", "next", "tab_centre", "tab_houses", "rally" | Where-Object { -not (Test-Path (Join-Path $Shots "build_hud_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build hud" $Checks) -and $AllOk

    # J6b.1 (D312): the camps. Each unload: what, where, who (hol.Gather.LogDeposits).
    $Camp = @(Get-Section "build camp" | ForEach-Object { $_ })
    $Mining = @(Get-Section "build camp mining" | ForEach-Object { $_ })
    $MillLines = @(Get-Section "build camp mill" | ForEach-Object { $_ })
    $Far = @(Get-Section "build camp far" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $ReadUnloads = { param($Lines) @($Lines | Select-String -Pattern "Gather: slot 0 unloaded (\d+) (\w+) \(stock [^)]*\) at (\w+) \((-?\d+), (-?\d+)\), unit #(\d+)" | ForEach-Object {
        $G = $_.Matches[0].Groups; [pscustomobject]@{ Amount = [int]$G[1].Value; Resource = $G[2].Value; At = "$($G[3].Value) ($($G[4].Value), $($G[5].Value))"; Unit = $G[6].Value; Line = $_.LineNumber } }) }
    $AtL = "LumberCamp ($($CampL -join ', '))"
    $AtM = "MiningCamp ($($CampM -join ', '))"
    $Early = & $Value $Camp "Selection: Peasant #(\d+) of slot 0 \(own\)"
    $FinishedL = @($Camp | Select-String -Pattern "Construction: slot 0 finished LumberCamp at \($($CampL -join ', ')\)" | Select-Object -First 1)
    $EndL = if ($FinishedL.Count -eq 1) { $FinishedL[0].LineNumber } else { [int]::MaxValue }
    $Unloads = & $ReadUnloads $Camp
    $Before = @($Unloads | Where-Object { $_.Line -lt $EndL })
    $After = @($Unloads | Where-Object { $_.Line -gt $EndL })
    $SentL = & $Value $Camp "(camp LumberCamp at \($($CampL -join ', ')\) finished: .*)$"
    $Checks["L built by its two Peasants, its builders sent to gather the wood within 25 m ($SentL)"] = [bool]($Camp | Select-String -Pattern "Build order of slot 0: 2 units to LumberCamp at \($($CampL -join ', ')\)$" -Quiet) -and
        ($FinishedL.Count -eq 1) -and [bool]($SentL -match "finished: (\d+) builders sent to gather Wood \(source \d+, (\d+\.\d) m\)$") -and ([int]$Matches[1] -ge 2) -and ([double]$Matches[2] -le 25)
    $Builders = @($After | Where-Object { $_.Unit -ne $Early })
    $BuilderUnits = @($Builders | ForEach-Object { $_.Unit } | Sort-Object -Unique)
    $Checks["they unload their wood at L, none at the centre ($($Builders.Count) unloads by $($BuilderUnits.Count) units: $(($Builders | ForEach-Object { "#$($_.Unit) $($_.Amount) $($_.Resource) at $($_.At)" } | Select-Object -First 4) -join '; '))"] =
        ($BuilderUnits.Count -ge 2) -and (@($Builders | Where-Object { $_.At -ne $AtL -or $_.Resource -ne "Wood" }).Count -eq 0)
    $EarlyBefore = @($Before | Where-Object { $_.Unit -eq $Early })
    $EarlyAfter = @($After | Where-Object { $_.Unit -eq $Early })
    $Checks["L unfinished takes nothing: the first Peasant (#$Early) unloads at the centre until its end, at L after ($(($EarlyBefore | ForEach-Object { $_.At }) -join ', ') | $(($EarlyAfter | ForEach-Object { $_.At }) -join ', '))"] =
        ($EarlyBefore.Count -ge 1) -and (@($Before | Where-Object { $_.At -eq $AtL }).Count -eq 0) -and (@($EarlyBefore | Where-Object { $_.At -notmatch "^MainCentre " }).Count -eq 0) -and
        (@($EarlyAfter | Where-Object { $_.At -eq $AtL }).Count -ge 1)
    $Checks["the centre's rally point on L's site, the Peasant born once L is finished gathers by it"] =
        [bool]($Camp | Select-String -Pattern "Rally: slot 0 set the rally point of MainCentre at \($($CampL -join ', ')\), on site LumberCamp$" -Quiet) -and
        [bool]($Camp | Select-String -Pattern "Rally: 1 units of slot 0 born for the finished camp LumberCamp at \($($CampL -join ', ')\) sent to gather Wood \(source \d+, \d+\.\d m\)$" -Quiet)
    $SentM = & $Value $Mining "(camp MiningCamp at \($($CampM -join ', ')\) finished: .*)$"
    $Checks["M built by its two Peasants, its builders sent to the stone vein, not to the gold one ($SentM)"] = [bool]($Mining | Select-String -Pattern "Build order of slot 0: 2 units to MiningCamp at \($($CampM -join ', ')\)$" -Quiet) -and
        [bool]($SentM -match "finished: (\d+) builders sent to gather Stone \(source \d+, (\d+\.\d) m\)$") -and ([int]$Matches[1] -ge 2) -and ([double]$Matches[2] -le 25)
    $Stone = @(& $ReadUnloads $Mining | Where-Object { $_.Resource -eq "Stone" })
    $StoneUnits = @($Stone | ForEach-Object { $_.Unit } | Sort-Object -Unique)
    $Checks["they unload their stone at M ($($Stone.Count) unloads by $($StoneUnits.Count) units: $(($Stone | ForEach-Object { $_.At } | Sort-Object -Unique) -join ', '))"] =
        ($StoneUnits.Count -ge 2) -and (@($Stone | Where-Object { $_.At -ne $AtM }).Count -eq 0)
    $AtMill = "Mill ($($CampMill -join ', '))"
    $SentMill = & $Value $MillLines "(camp Mill at \($($CampMill -join ', ')\) finished: .*)$"
    $Checks["Mi (D346) built by its two Peasants, its builders sent to the berries ($SentMill)"] = [bool]($MillLines | Select-String -Pattern "Build order of slot 0: 2 units to Mill at \($($CampMill -join ', ')\)$" -Quiet) -and
        [bool]($SentMill -match "finished: (\d+) builders sent to gather Food \(source \d+, (\d+\.\d) m\)$") -and ([int]$Matches[1] -ge 2) -and ([double]$Matches[2] -le 25)
    $Food = @(& $ReadUnloads $MillLines | Where-Object { $_.Resource -eq "Food" })
    $FoodUnits = @($Food | ForEach-Object { $_.Unit } | Sort-Object -Unique)
    $Checks["they unload their food at Mi ($($Food.Count) unloads by $($FoodUnits.Count) units: $(($Food | ForEach-Object { $_.At } | Sort-Object -Unique) -join ', '))"] =
        ($FoodUnits.Count -ge 2) -and (@($Food | Where-Object { $_.At -ne $AtMill }).Count -eq 0)
    $SentFar = & $Value $Far "(camp LumberCamp at \($($CampFar -join ', ')\) finished: .*)$"
    $IdleFar = @($Far | Select-String -Pattern "Build: a builder of slot 0 stops \(no site left\), idle").Count
    $Checks["a camp 30 m from any wood: no source within 25 m, its two builders idle ($SentFar; $IdleFar idle)"] =
        [bool]($SentFar -match "finished: no accepted source within 25 m, 2 builders go on as after a house$") -and ($IdleFar -eq 2) -and
        -not ($Far | Select-String -Pattern "goes on to the nearby site" -Quiet)
    $Missing = @("wood", "stone", "food", "idle" | Where-Object { -not (Test-Path (Join-Path $Shots "build_camp_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build camp" $Checks) -and $AllOk

    # J6b.2 (D314): paths cut by a footprint. The footprint's line; the watch of the bodies over the seconds after it.
    $Cut = @(Get-Section "build cut" | ForEach-Object { $_ })
    $CutMove = @(Get-Section "build cut move" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $ReadCut = { param($Lines, $At) $Line = $Lines | Select-String -Pattern "Buildings \(D310\): footprint House at \($($At -join ', ')\): (\d+) walkers given a new path, (\d+) reused gather paths dropped(.*)\((.*)\)$" | Select-Object -First 1
        if ($Line) { $G = $Line.Matches[0].Groups; [pscustomobject]@{ Walkers = [int]$G[1].Value; Dropped = [int]$G[2].Value; Text = ($Line.Matches[0].Value -replace '^Buildings \(D310\): ', ''); Rebuilt = ($G[4].Value -match '^tiles rebuilt') } } }
    # C.2: a unit standing still eased out of a visible wall (C.1) is counted apart, not as put back.
    $ReadWatch = { param($Lines) $W = $Lines | Select-String -Pattern "Footprint watch: (\d+) steps, .*; bodies in a footprint: (\d+) unit-steps, .*; eased out of a visible wall: (\d+); put back out: (\d+), new paths: (\d+)$" | Select-Object -Last 1
        if ($W) { [pscustomobject]@{ Steps = [int]$W.Matches[0].Groups[1].Value; Inside = [int]$W.Matches[0].Groups[2].Value; Eased = [int]$W.Matches[0].Groups[3].Value; Out = [int]$W.Matches[0].Groups[4].Value; Repaths = [int]$W.Matches[0].Groups[5].Value } } }
    $K = & $ReadCut $Cut $CutK
    $Checks["house K across the round trips: its kept paths dropped, its walkers given a new path once its tiles were rebuilt ($(if ($K) { $K.Text }))"] =
        ($null -ne $K) -and ($K.Walkers -ge 1) -and ($K.Dropped -ge 1) -and $K.Rebuilt
    $WatchK = & $ReadWatch $Cut
    $PushedK = @($Cut | Select-String -Pattern "Keep-out: .* put back .*$" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 2)
    # Same rule as house N below (J6b.3): a walker brushing K's wall is put back by the guard (a few cm a step, D310), 10 in
    # a row give it one new path; none in the footprint, a few put back, a new path at most.
    $Checks["none in it over 17 s after it, a few put back, a new path at most ($(if ($WatchK) { "$($WatchK.Steps) steps, $($WatchK.Inside) unit-steps in it, $($WatchK.Out) put back, $($WatchK.Repaths) new paths for it, $($WatchK.Eased) eased out of a visible wall" }) $($PushedK -join '; '))"] =
        ($null -ne $WatchK) -and ($WatchK.Steps -ge 200) -and ($WatchK.Inside -eq 0) -and ($WatchK.Out -le 20) -and ($WatchK.Repaths -le 1)
    $KLine = -1; for ($i = 0; $i -lt $Cut.Count; $i++) { if ($Cut[$i] -match "Buildings \(D310\): footprint House at \($($CutK -join ', ')\)") { $KLine = $i; break } }
    $UnloadsK = if ($KLine -ge 0) { @($Cut[$KLine..($Cut.Count - 1)] | Select-String -Pattern "Gather: slot 0 unloaded \d+ \w+ \(stock [^)]*\) at MainCentre ").Count } else { 0 }
    $Checks["the round trips go on around it ($UnloadsK unloads at the centre after it)"] = $UnloadsK -ge 3
    $N = & $ReadCut $CutMove $CutN
    $Checks["house N across a move order: its walkers given a new path ($(if ($N) { $N.Text }))"] = ($null -ne $N) -and ($N.Walkers -ge 6) -and $N.Rebuilt
    $WatchN = & $ReadWatch $CutMove
    $PushedN = @($CutMove | Select-String -Pattern "Keep-out: .* put back .*$" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 2)
    # D324: round N's body (5.7 m; its 8 m tiles before D324), the group of six squeezes its inner Peasant against the west wall, which the
    # footprints' guard keeps out (a few cm a step, D310): a slide along the wall, not a walk into the building. A slide of 10
    # steps in a row (half a second) gives that walker one new path from where it stands (RepathBuildingPushes, D310), as
    # #48 at the full quick_check of J6b.3: one at most, and no body in the footprint after the guard.
    # D344: with 4 m tiles the wall it slides along is 5.7 m (3.7 m before): 25 to 40 put back measured, limit 50 (20 before).
    $Checks["no walk into it: none in it, a few slides along its wall, a new path at most ($(if ($WatchN) { "$($WatchN.Steps) steps, $($WatchN.Inside) unit-steps in it, $($WatchN.Out) put back, $($WatchN.Repaths) new paths for it, $($WatchN.Eased) eased out of a visible wall" }) $($PushedN -join '; '))"] =
        ($null -ne $WatchN) -and ($WatchN.Inside -eq 0) -and ($WatchN.Out -le 50) -and ($WatchN.Repaths -le 1)
    $Arrived = @($CutMove | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1 | ForEach-Object {
        [regex]::Matches($_.Matches[0].Groups[1].Value, "#\d+ slot 0 at \((-?\d+), (-?\d+)\) (\w+)") } |
        Where-Object { [Math]::Sqrt([Math]::Pow([double]$_.Groups[1].Value - $CutTo[0], 2) + [Math]::Pow([double]$_.Groups[2].Value - $CutTo[1], 2)) -le 600 -and $_.Groups[3].Value -eq "Idle" })
    $Checks["they arrive ($($Arrived.Count) standing within 6 m of the point)"] = $Arrived.Count -ge 6
    $Missing = @("gather", "move" | Where-Object { -not (Test-Path (Join-Path $Shots "build_cut_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build cut" $Checks) -and $AllOk

    # J6b.2b (D324): the way between two houses whose tiles touch. The navmesh's paths through it; the walkers' samples (every
    # 0.2 s) while they are between the two bodies (x -6285 to -5715): in the way (within 60 cm of its middle line), one at
    # a time; none put back by a wall, no new path; all arrived.
    $Gap = @(Get-Section "build gap" | ForEach-Object { $_ })
    $GapOne = @(Get-Section "build gap one" | ForEach-Object { $_ })
    $GapTwo = @(Get-Section "build gap two" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $GapPaths = @($Gap | Select-String -Pattern "Nav path: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $GapRatio = { param($Text) if ([string]$Text -match "ratio (\d+\.\d+)") { [double]$Matches[1] } else { 99.0 } }
    $Checks["two houses touching, finished"] = [bool]($Gap | Select-String -Pattern "Construction: slot 0 finished House at \($($GapQ -join ', ')\)" -Quiet)
    $Checks["the navmesh's paths through the way: straight ($($GapPaths -join ' / '))"] = ($GapPaths.Count -eq 2) -and ((& $GapRatio $GapPaths[0]) -le 1.01) -and ((& $GapRatio $GapPaths[1]) -le 1.03)
    # The walkers: the units of the section whose first sample is within 4 m of the start.
    $ReadSamples = { param($Lines)
        $Samples = @($Lines | Select-String -Pattern "Units where: (.*)$" | ForEach-Object { $Units = @{}
            foreach ($M in [regex]::Matches($_.Matches[0].Groups[1].Value, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\) (\w+)")) { $Units[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value, $M.Groups[4].Value) }
            , $Units })
        $Walkers = if ($Samples.Count -gt 0) { @($Samples[0].Keys | Where-Object { $P = $Samples[0][$_]; [Math]::Abs($P[0] - $GapFrom[0]) -le 400 -and [Math]::Abs($P[1] - $GapFrom[1]) -le 400 }) } else { @() }
        [pscustomobject]@{ Samples = $Samples; Walkers = $Walkers } }
    $InWay = { param($P) $P[0] -ge $GapP[0] - $GapHalfBody -and $P[0] -le $GapP[0] + $GapHalfBody }
    foreach ($GapCase in @(@{ Name = "one"; Lines = $GapOne; Count = 1 }, @{ Name = "two"; Lines = $GapTwo; Count = 2 })) {
        $R = & $ReadSamples $GapCase.Lines
        $Between = @(); $Abreast = 0; $Off = @()
        foreach ($S in $R.Samples) {
            $Here = @($R.Walkers | Where-Object { $S.ContainsKey($_) -and (& $InWay $S[$_]) })
            foreach ($U in $Here) { $Between += "#$U ($($S[$U][0]), $($S[$U][1]))"; if ([Math]::Abs($S[$U][1] - $GapFrom[1]) -gt 60) { $Off += "#$U ($($S[$U][0]), $($S[$U][1]))" } }
            # Two in the way at once are a file (one behind the other, a body apart), not abreast.
            if ($Here.Count -ge 2 -and [Math]::Abs($S[$Here[0]][0] - $S[$Here[1]][0]) -lt 100) { $Abreast++ }
        }
        $Last = if ($R.Samples.Count -gt 0) { $R.Samples[-1] } else { @{} }
        $Arrived = @($R.Walkers | Where-Object { $Last.ContainsKey($_) -and [Math]::Abs($Last[$_][0] - $GapTo[0]) -le 400 -and [Math]::Abs($Last[$_][1] - $GapTo[1]) -le 400 -and $Last[$_][2] -eq "Idle" })
        $Watch = & $ReadWatch $GapCase.Lines
        $Pushed = @($GapCase.Lines | Select-String -Pattern "Keep-out: .* put back .*$" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 2)
        $Checks["$($GapCase.Name): $($R.Walkers.Count) walker(s) seen in the way ($($Between.Count) samples: $(($Between | Select-Object -First 4) -join ', '))"] = ($R.Walkers.Count -eq $GapCase.Count) -and ($Between.Count -ge $GapCase.Count)
        $Checks["$($GapCase.Name): on the way's middle line, within 60 cm (off: $($Off -join ', '))"] = $Off.Count -eq 0
        if ($GapCase.Count -ge 2) { $Checks["two: one after the other ($Abreast samples with both in the way less than 1 m apart along it)"] = $Abreast -eq 0 }
        $Checks["$($GapCase.Name): none put back by a wall ($(if ($Watch) { "$($Watch.Steps) steps, $($Watch.Out) put back, $($Watch.Repaths) new paths, $($Watch.Eased) eased out of a visible wall" }) $($Pushed -join '; '))"] =
            ($null -ne $Watch) -and ($Watch.Out -eq 0) -and ($Watch.Repaths -eq 0)
        $Checks["$($GapCase.Name): arrived ($($Arrived.Count) standing within 4 m of the point)"] = $Arrived.Count -eq $GapCase.Count
    }
    # The walkers taken away after the wall (hol.Units.Remove): the wall's one, the way's three, no other unit of slot 0.
    $WallSection = @(Get-Section "build wall" | ForEach-Object { $_ })
    $Removed = foreach ($Cut in @(@{ At = "300 of \($($GapP[0] + 1000), $GapWallY\)"; Count = 1 }, @{ At = "400 of \($($GapTo[0]), $($GapTo[1])\)"; Count = 3 })) {
        $N = @($WallSection | Select-String -Pattern "Remove: (\d+) units of slot 0 within $($Cut.At)" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
        @{ Count = $Cut.Count; Got = $N }
    }
    $Checks["walkers taken away: wall $($Removed[0].Got -join ', ') of 1, way $($Removed[1].Got -join ', ') of 3"] =
        @($Removed | Where-Object { $_.Got.Count -eq 1 -and $_.Got[0] -eq $_.Count }).Count -eq 2
    $Missing = @("one", "two", "houses" | Where-Object { -not (Test-Path (Join-Path $Shots "build_gap_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build gap" $Checks) -and $AllOk

    # J7a.2: the barracks.
    $Barracks = @(Get-Section "build barracks" | ForEach-Object { $_ })
    $BarracksOrders = @(Get-Section "build barracks orders" | ForEach-Object { $_ })
    $BarracksF1 = @(Get-Section "build barracks f1" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $PeasantCells = ($Barracks | Select-String -Pattern "HUD cells: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["the Peasants' grid: Q = Caserne, S = Champ de tir, W = Ferme ($PeasantCells)"] = [bool]($PeasantCells -match "; I Q: Caserne; I S: Champ de tir; I W: Ferme$") -and
        [bool](($Barracks | Select-String -Pattern "HUD report: " | Select-Object -First 1).Line -match "Q=Barracks enabled")
    $At = [regex]::Escape("Barracks at ($($BarracksAt -join ', '))")
    $Checks["the keys A, Q: the ghost, placed, built by the three, finished"] = [bool]($Barracks | Select-String -Pattern "Placement: Barracks \(cost F 0 W 150 S 0 G 0\)" -Quiet) -and
        [bool]($Barracks | Select-String -Pattern "Build order of slot 0: 3 units to $At$" -Quiet) -and [bool]($Barracks | Select-String -Pattern "Construction: slot 0 finished $At" -Quiet)
    $BarracksCells = @($Barracks | Select-String -Pattern "HUD cells: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["the barracks selected: A Lancier, Z Homme d'armes, no Archer (AR-Q2) ($(@($BarracksCells | Select-Object -Last 1)))"] = ($BarracksCells.Count -ge 2) -and
        [bool]($BarracksCells[-1] -match "^A: Lancier; Z: Homme d'armes$")
    # The ground point under the cursor, a few cm off the camera's focus (as the group's rally point of J6a.5).
    $RallyAt = if ([string](& $Value $Barracks "(Rally: slot 0 set the rally point of Barracks at .*)$") -match "at \((-?\d+), (-?\d+)\)") { @([int]$Matches[1], [int]$Matches[2]) } else { @() }
    $Checks["its rally point by a right click ($($RallyAt -join ', '))"] = ($RallyAt.Count -eq 2) -and ([Math]::Abs($RallyAt[0] - $BarracksRally[0]) -le 50) -and
        ([Math]::Abs($RallyAt[1] - $BarracksRally[1]) -le 50)
    $Born = @($Barracks | Select-String -Pattern "Production: (Spearman|ManAtArms|Archer) of slot 0 done" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["a Lancier, a Homme d'armes and a second Lancier born ($($Born -join ', '))"] = ($Born -join ',') -eq "Spearman,ManAtArms,Spearman"
    $WhereLine = [string]($Barracks | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1)
    $Soldiers = @{}
    foreach ($M in [regex]::Matches([string]$WhereLine, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\)")) { $Soldiers[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }
    $Last3 = @($Soldiers.Keys | Sort-Object | Select-Object -Last 3)
    $Gaps = @($Last3 | ForEach-Object { [Math]::Round([Math]::Sqrt([Math]::Pow($Soldiers[$_][0] - $BarracksRally[0], 2) + [Math]::Pow($Soldiers[$_][1] - $BarracksRally[1], 2))) })
    $Checks["all three at the rally point, within 4 m ($($Gaps -join ', ') cm)"] = ($Gaps.Count -eq 3) -and (@($Gaps | Where-Object { $_ -le 400 }).Count -eq 3)
    $Checks["all three sent onto the vein: they walk there, no gather order"] = [bool]($BarracksOrders | Select-String -Pattern "Order of slot 0: 3 units that do not work walk to" -Quiet) -and
        -not ($BarracksOrders | Select-String -Pattern "Gather order of slot 0" -Quiet)
    $Checks["F1: the barracks selected"] = [bool]($BarracksF1 | Select-String -Pattern "Category Military: 1 buildings selected" -Quiet) -and
        [bool]($BarracksF1 | Select-String -Pattern "Selection report: selected .*Barracks" -Quiet)
    $Missing = @("peasants", "ghost", "finished", "soldiers", "f1" | Where-Object { -not (Test-Path (Join-Path $Shots "build_barracks_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build barracks" $Checks) -and $AllOk

    # The user's request of 2026-10-19: the rally point of a barracks' ghost.
    $Ghost = @(Get-Section "build barracks ghost" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $GhostAt = [regex]::Escape("Barracks at ($($BarracksGhostAt -join ', '))")
    $GhostLines = @($Ghost | Select-String -Pattern "Rally: slot 0 set the rally point of Barracks at|first work on $GhostAt|finished $GhostAt" | ForEach-Object { $_.Line })
    $Checks["the ghost placed, its rally point set before its first work"] = [bool]($Ghost | Select-String -Pattern "placed $([regex]::Escape("Barracks foundation at ($($BarracksGhostAt -join ', '))"))" -Quiet) -and
        ($GhostLines.Count -ge 2) -and [bool]($GhostLines[0] -match "Rally: ") -and -not ($Ghost | Select-String -Pattern "Production order of slot 0 ignored" -Quiet)
    $GhostRallyAt = if ([string](& $Value $Ghost "(Rally: slot 0 set the rally point of Barracks at .*)$") -match "at \((-?\d+), (-?\d+)\)") { @([int]$Matches[1], [int]$Matches[2]) } else { @() }
    $Checks["its rally point by a right click ($($GhostRallyAt -join ', '))"] = ($GhostRallyAt.Count -eq 2) -and ([Math]::Abs($GhostRallyAt[0] - $BarracksGhostRally[0]) -le 50) -and
        ([Math]::Abs($GhostRallyAt[1] - $BarracksGhostRally[1]) -le 50)
    $Checks["built by the Peasant, a Homme d'armes born"] = [bool]($Ghost | Select-String -Pattern "Construction: slot 0 finished $GhostAt" -Quiet) -and
        [bool]($Ghost | Select-String -Pattern "Production: ManAtArms of slot 0 done" -Quiet)
    $WhereLine = [string]($Ghost | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1)
    $GhostUnits = @{}
    foreach ($M in [regex]::Matches([string]$WhereLine, "#(\d+) slot 0 at \((-?\d+), (-?\d+)\)")) { $GhostUnits[[int]$M.Groups[1].Value] = @([double]$M.Groups[2].Value, [double]$M.Groups[3].Value) }
    $GhostNewest = @($GhostUnits.Keys | Sort-Object | Select-Object -Last 1)
    $GhostGap = if ($GhostNewest.Count -eq 1) { [Math]::Round([Math]::Sqrt([Math]::Pow($GhostUnits[$GhostNewest[0]][0] - $BarracksGhostRally[0], 2) + [Math]::Pow($GhostUnits[$GhostNewest[0]][1] - $BarracksGhostRally[1], 2))) } else { -1 }
    $Checks["it walks to the rally point kept since the ghost, within 4 m ($GhostGap cm)"] = ($GhostGap -ge 0) -and ($GhostGap -le 400)
    $Missing = @("rally", "soldier" | Where-Object { -not (Test-Path (Join-Path $Shots "build_barracks_ghost_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build barracks ghost" $Checks) -and $AllOk

    # IA1.2 F9: the farm. Its field born at its end, its builders on the fields, their food unloaded at Mi, a right click
    # on A, a walker across both.
    $FarmB0 = @(Get-Section "build farm" | ForEach-Object { $_ })
    $FarmA0 = @(Get-Section "build farm a" | ForEach-Object { $_ })
    $FarmClickLines = @(Get-Section "build farm click" | ForEach-Object { $_ })
    $FarmCross = @(Get-Section "build farm cross" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $FieldOf = { param($Lines, $At) if ([string](& $Value $Lines "(Building Farm of slot 0 finished: its FarmField is source \d+ at \($($At -join ', ')\))$") -match "is source (\d+) at") { [int]$Matches[1] } else { -1 } }
    $FieldB = & $FieldOf $FarmB0 $FarmB
    $FieldA = & $FieldOf $FarmA0 $FarmA
    $Checks["B built by its Peasant, its field born (source $FieldB), its builder on it, then sent away"] = ($FieldB -ge 0) -and
        [bool]($FarmB0 | Select-String -Pattern "Build order of slot 0: 1 units to Farm at \($($FarmB -join ', ')\)$" -Quiet) -and
        [bool]($FarmB0 | Select-String -Pattern "Build: a builder of slot 0 goes on to gather its farm's field \(source $FieldB\)" -Quiet)
    $SentA = @($FarmA0 | Select-String -Pattern "Build: a builder of slot 0 goes on to gather (its farm's|a free) field \(source (\d+)\)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" })
    $Checks["A built by its two Peasants (75 wood each), its field born (source $FieldA); one builder on it, the other on B's free field ($($SentA -join ', '))"] = ($FieldA -ge 0) -and ($FieldA -ne $FieldB) -and
        [bool]($FarmA0 | Select-String -Pattern "Build order of slot 0: 2 units to Farm at \($($FarmA -join ', ')\)$" -Quiet) -and
        ($SentA.Count -eq 2) -and ($SentA -contains "its farm's $FieldA") -and ($SentA -contains "a free $FieldB") -and
        -not ($FarmA0 | Select-String -Pattern "finds its farm's field taken" -Quiet)
    # Its two builders: the two Peasants clicked before the box (#ids); their food unloaded at Mi (Mi's berry gatherers apart).
    $FarmBuilderIds = @($FarmA0 | Select-String -Pattern "Selection: Peasant #(\d+) of slot 0 \(own\)" | ForEach-Object { $_.Matches[0].Groups[1].Value } | Select-Object -Unique)
    $FarmFood = @(& $ReadUnloads $FarmA0 | Where-Object { $_.Resource -eq "Food" -and $FarmBuilderIds -contains $_.Unit })
    $FarmFoodUnits = @($FarmFood | ForEach-Object { $_.Unit } | Sort-Object -Unique)
    $Checks["they unload their food at Mi ($($FarmFood.Count) unloads by $($FarmFoodUnits.Count) of #$($FarmBuilderIds -join ', #'): $(($FarmFood | ForEach-Object { "$($_.Amount) at $($_.At)" } | Select-Object -First 4) -join '; '))"] =
        ($FarmBuilderIds.Count -eq 2) -and ($FarmFoodUnits.Count -eq 2) -and (@($FarmFood | Where-Object { $_.At -ne $AtMill }).Count -eq 0)
    $OnFarm = & $Value $FarmClickLines "(Order: on farm .*)$"
    $ClickId = & $Value $FarmClickLines "Selection: Peasant #(\d+) of slot 0 \(own\)"
    $Checks["a right click on A: its field ordered ($OnFarm)"] = [bool]($OnFarm -match "^Order: on farm Farm of slot 0 \(own\).*: its field, source $FieldA$") -and
        [bool]($FarmClickLines | Select-String -Pattern "Order: 1 units gather source $FieldA \(" -Quiet) -and
        [bool]($FarmClickLines | Select-String -Pattern "Gather order of slot 0: 1 units to source $FieldA \(Food\)" -Quiet)
    # F3: one place; B's taken too, no food within 20 m: no room in the massif, it stands in front of A; it unloads nothing.
    $NoRoom = & $Value $FarmClickLines "(Gather: slot 0 found no room in its massif, it stands idle in front of it at .*)$"
    $ClickFood = @(& $ReadUnloads $FarmClickLines | Where-Object { $_.Unit -eq $ClickId })
    $Checks["one Peasant on a field: the third finds no room and stands in front of A, no unload ($NoRoom; $($ClickFood.Count) unloads by #$ClickId)"] =
        [bool]$NoRoom -and ($ClickFood.Count -eq 0) -and -not ($FarmClickLines | Select-String -Pattern "found no room in the massif of source $FieldA, sent to" -Quiet)
    $CrossPath = & $Value $FarmCross "Nav path: (.*)$"
    $CrossRatio = if ([string]$CrossPath -match "ratio (\d+\.\d+)") { [double]$Matches[1] } else { -1.0 }
    $Checks["across both farms, the navmesh's path straight ($CrossPath)"] = ($CrossRatio -ge 0) -and ($CrossRatio -le 1.02)
    $CrossId = & $Value $FarmCross "Selection: Peasant #(\d+) of slot 0 \(own\)"
    $CrossLast = [string]($FarmCross | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1)
    $CrossAt = if ($CrossId -and $CrossLast -match "#$CrossId slot 0 at \((-?\d+), (-?\d+)\)") { @([double]$Matches[1], [double]$Matches[2]) } else { $null }
    $CrossGap = if ($CrossAt) { [Math]::Round([Math]::Sqrt([Math]::Pow($CrossAt[0] - $FarmCrossTo[0], 2) + [Math]::Pow($CrossAt[1] - $FarmCrossTo[1], 2))) } else { -1 }
    $CrossDetours = @($FarmCross | Select-String -Pattern "Detour: .*" | ForEach-Object { $_.Matches[0].Value })
    $Checks["a walker across them, no detour, arrived within 4 m in 8 s for 44 m ($CrossGap cm; $($CrossDetours -join '; '))"] = ($CrossGap -ge 0) -and ($CrossGap -le 400) -and ($CrossDetours.Count -eq 0)
    $Missing = @("gather", "click" | Where-Object { -not (Test-Path (Join-Path $Shots "build_farm_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build farm" $Checks) -and $AllOk

    # F4 (D30, J8a-Q4): the player's hero alone, not slot 1's; again, the camera on him.
    $Hero = @(Get-Section "build hero" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $HeroCams = @($Hero | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), " | ForEach-Object { $_.Matches[0] })
    $HeroSel = @($Hero | Select-String -Pattern "Selection report: selected (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["no hero: F4 selects nothing, the camera kept ($($HeroSel | Select-Object -First 1))"] = [bool]($Hero | Select-String -Pattern "Select hero: none" -Quiet) -and
        ($HeroSel.Count -ge 1) -and ($HeroSel[0] -eq "none") -and ($HeroCams.Count -ge 2) -and ($HeroCams[1].Value -eq $HeroCams[0].Value)
    $Spawned = @($Hero | Select-String -Pattern "Spawn: 1 Hero of slot (\d) around" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Picked = [regex]::Match("$(& $Value $Hero "(Select hero: selected .*)$")", "#(\d+) at \((-?\d+), (-?\d+)\)")
    $HeroGap = if ($Picked.Success) { [Math]::Round([Math]::Sqrt([Math]::Pow([int]$Picked.Groups[2].Value - $BuildHero[0], 2) + [Math]::Pow([int]$Picked.Groups[3].Value - $BuildHero[1], 2))) } else { -1 }
    $Checks["his hero and slot 1's: F4 selects his alone, by his centre ($HeroGap cm from where he was made), the camera kept ($($HeroSel | Select-Object -Skip 1 -First 1))"] =
        (($Spawned -join ",") -eq "0,1") -and $Picked.Success -and ($HeroGap -ge 0) -and ($HeroGap -le 500) -and
        ($HeroSel.Count -ge 2) -and ($HeroSel[1] -eq "Hero #$($Picked.Groups[1].Value) of slot 0 (own)") -and
        [bool]($Hero | Select-String -SimpleMatch -Pattern "Selection: Hero #$($Picked.Groups[1].Value) of slot 0 (own)" -Quiet) -and ($HeroCams.Count -ge 3) -and ($HeroCams[2].Value -eq $HeroCams[0].Value)
    $Centred = [regex]::Match("$(& $Value $Hero "(Select hero: camera on .*)$")", "#(\d+) at \((-?\d+), (-?\d+)\)")
    $Checks["F4 again: the camera on him, the selection kept ($(if ($HeroCams.Count -ge 4) { $HeroCams[3].Value }); $($HeroSel | Select-Object -Skip 2 -First 1))"] =
        $Picked.Success -and $Centred.Success -and ($Centred.Groups[1].Value -eq $Picked.Groups[1].Value) -and ($HeroCams.Count -ge 4) -and
        ([Math]::Abs([int]$HeroCams[3].Groups[1].Value - [int]$Centred.Groups[2].Value) -le 2) -and ([Math]::Abs([int]$HeroCams[3].Groups[2].Value - [int]$Centred.Groups[3].Value) -le 2) -and
        ($HeroSel.Count -ge 3) -and ($HeroSel[2] -eq "Hero #$($Picked.Groups[1].Value) of slot 0 (own)")
    # J8a.5 loop 2: the hero's block, written by the game screen in script (Script/UI/HOLGameScreenScript.as).
    $HeroBlock = @($Hero | Select-String -Pattern "HUD hero: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
    $Checks["the hero's block in script: hidden without a hero, then alive level 1 hp 450/450 ($($HeroBlock -join '; '))"] =
        ($HeroBlock.Count -ge 2) -and ($HeroBlock[0] -eq "hidden") -and ($HeroBlock[1] -eq "alive level 1 hp 450/450")
    $Checks["capture of the hero's block (build_hero_block)"] = Test-Path (Join-Path $Shots "build_hero_block.png")
    $AllOk = (Report "build hero" $Checks) -and $AllOk
    # J8a.5 loop 4: the block during the death and the return (times at 20 %), the return's button and the portrait.
    $HeroRet = @(Get-Section "build hero return" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $RetBlock = @($HeroRet | Select-String -Pattern "HUD hero: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
    # The delay as the rule says it at level 1 and 20 % (the net check's formula): at most ceil(152 / 20) = 8 s.
    $RetDelaySteps = [int][Math]::Floor((30 + 8 * 1) * 20 * 20 / 100)
    $RetDelayMax = [int][Math]::Ceiling($RetDelaySteps / 20)
    $Dead = @($RetBlock | Where-Object { $_ -match "^dead ready in (\d+) s$" } | ForEach-Object { [int]($_ -replace '^dead ready in (\d+) s$', '$1') })
    $Checks["dead: ready in S s, 0 < S <= $RetDelayMax, then less a second later ($(($RetBlock | Select-Object -First 2) -join '; '))"] =
        [bool]($HeroRet | Select-String -SimpleMatch "hol.Hero.Kill: slot 0, 1 hero(es)" -Quiet) -and ($RetBlock.Count -ge 2) -and ($Dead.Count -ge 2) -and
        ($RetBlock[0] -eq "dead ready in $($Dead[0]) s") -and ($RetBlock[1] -eq "dead ready in $($Dead[1]) s") -and
        ($Dead[0] -gt 0) -and ($Dead[0] -le $RetDelayMax) -and ($Dead[1] -gt 0) -and ($Dead[1] -lt $Dead[0])
    $Prices = @(for ($i = 2; $i -lt [Math]::Min($RetBlock.Count, 5); $i++) { if ($RetBlock[$i] -match "^ready price (\d+) (buyable|no gold|no place)$") { [pscustomobject]@{ Price = [int]$Matches[1]; Why = $Matches[2] } } })
    $Checks["ready (Hero return: slot 0 ready): price P buyable, 0 < P <= 150 ($(if ($RetBlock.Count -ge 3) { $RetBlock[2] }))"] =
        [bool]($HeroRet | Select-String -SimpleMatch "Hero return: slot 0 ready at step" -Quiet) -and ($Prices.Count -ge 1) -and ($RetBlock[2] -match "^ready price") -and
        ($Prices[0].Why -eq "buyable") -and ($Prices[0].Price -gt 0) -and ($Prices[0].Price -le 150)
    $Checks["the gold emptied: no gold; given back: buyable, the price not going up ($(($RetBlock | Select-Object -Skip 3 -First 2) -join '; '))"] =
        ($Prices.Count -eq 3) -and ($Prices[1].Why -eq "no gold") -and ($Prices[2].Why -eq "buyable") -and
        ($Prices[1].Price -le $Prices[0].Price) -and ($Prices[2].Price -le $Prices[1].Price)
    # The price shown just before the click against the one the server took ("bought for X gold"), the gold read by
    # hol.Hero.Report before the click and after the return.
    $Shown = if ($Prices.Count -eq 3) { $Prices[2].Price } else { -1 }
    $Taken = @($HeroRet | Select-String -Pattern "Hero return: slot 0 bought for (\d+) gold" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $Golds = @($HeroRet | Select-String -Pattern "Hero gold slot 0 local: (-?\d+)" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $ButtonClick = & $Value $HeroRet "HUD click HeroReturnButton: (.*)$"
    $Checks["the return's button clicked ($ButtonClick): bought for $($Taken -join ', ') gold, shown $Shown just before; gold $($Golds -join ' then '); back $(if ($RetBlock.Count -ge 6) { $RetBlock[5] })"] =
        ($ButtonClick -eq "done") -and ($Taken.Count -eq 1) -and ($Taken[0] -eq $Shown) -and ($Golds.Count -ge 2) -and ($Golds[0] -eq 500) -and
        ($Golds[1] -eq $Golds[0] - $Taken[0]) -and ($RetBlock.Count -ge 6) -and ($RetBlock[5] -eq "alive level 1 hp 450/450")
    # The portrait: as F4, a first click selects him (the camera kept), a second centres the camera on him.
    $PortraitClicks = @($HeroRet | Select-String -Pattern "HUD click HeroPortrait: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $PortraitPick = [regex]::Match("$(& $Value $HeroRet "(Select hero: selected .*)$")", "#(\d+) at \((-?\d+), (-?\d+)\)")
    $PortraitCam = [regex]::Match("$(& $Value $HeroRet "(Select hero: camera on .*)$")", "#(\d+) at \((-?\d+), (-?\d+)\)")
    $RetSel = @($HeroRet | Select-String -Pattern "Selection report: selected (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $RetCams = @($HeroRet | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), " | ForEach-Object { $_.Matches[0] })
    $Checks["the portrait clicked ($($PortraitClicks -join ', ')): selects him ($($RetSel | Select-Object -First 1)), again the camera on him ($(if ($RetCams.Count -ge 2) { $RetCams[1].Value }))"] =
        ($PortraitClicks.Count -eq 2) -and ($PortraitClicks[0] -eq "done") -and ($PortraitClicks[1] -eq "done") -and $PortraitPick.Success -and $PortraitCam.Success -and
        ($PortraitCam.Groups[1].Value -eq $PortraitPick.Groups[1].Value) -and ($RetSel.Count -ge 2) -and ($RetSel[0] -eq "Hero #$($PortraitPick.Groups[1].Value) of slot 0 (own)") -and
        ($RetCams.Count -ge 2) -and ([Math]::Abs([int]$RetCams[1].Groups[1].Value - [int]$PortraitCam.Groups[2].Value) -le 2) -and
        ([Math]::Abs([int]$RetCams[1].Groups[2].Value - [int]$PortraitCam.Groups[3].Value) -le 2)
    $Missing = @("dead", "ready" | Where-Object { -not (Test-Path (Join-Path $Shots "build_hero_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $Checks["both heroes removed after"] = (@($HeroRet | Select-String -Pattern "Remove: 1 units of slot [01] within").Count -eq 2)
    $AllOk = (Report "build hero return" $Checks) -and $AllOk
    # J8a.5 loop 7b: the block's texts in English and Japanese, as its widgets show them ("HUD hero texts: level | HP |
    # delay | button | tooltip").
    $HeroTx = @(Get-Section "build hero texts" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $Tx = @($HeroTx | Select-String -Pattern "HUD hero texts: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
    $TxWanted = @(
        @("alive, en: Level 1, HP 450 / 450", '^Level 1 \| HP 450 / 450 \|$'),
        @("alive, ja: レベル 1, HP 450 / 450", '^レベル 1 \| HP 450 / 450 \|$'),
        @("dead, en: Ready in 0:0S", '^Level 1 \| HP 0 / 450 \| Ready in 0:0\d$'),
        @("dead, ja: 帰還まで 0:0S", '^レベル 1 \| HP 0 / 450 \| 帰還まで 0:0\d$'),
        @("ready, en: Bring the hero back — P gold", '^Level 1 \| HP 0 / 450 \|  \| Bring the hero back — \d+ gold \|$'),
        @("no gold, en: Not enough gold", '^Level 1 \| HP 0 / 450 \|  \| Bring the hero back — \d+ gold \| Not enough gold$'),
        @("no gold, ja: 金が足りません", '^レベル 1 \| HP 0 / 450 \|  \| 英雄を呼び戻す — \d+ 金 \| 金が足りません$'),
        @("given back, ja: 英雄を呼び戻す — P 金", '^レベル 1 \| HP 0 / 450 \|  \| 英雄を呼び戻す — \d+ 金 \|$'))
    for ($i = 0; $i -lt $TxWanted.Count; $i++) {
        $Seen = if ($Tx.Count -gt $i) { $Tx[$i] } else { "none" }
        $Checks["$($TxWanted[$i][0]) ($Seen)"] = ($Tx.Count -gt $i) -and ($Tx[$i] -match $TxWanted[$i][1])
    }
    $Checks["back in French, the return bought, the hero removed"] =
        [bool]($HeroTx | Select-String -SimpleMatch "Hero return: slot 0 by its main forum" -Quiet) -and [bool]($HeroTx | Select-String -Pattern "Remove: 1 units of slot 0 within" -Quiet)
    $Missing = @("block_ja", "ready_en" | Where-Object { -not (Test-Path (Join-Path $Shots "build_hero_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build hero texts" $Checks) -and $AllOk
    # J8b.2 loop 3: the hero's XP in a match, as the simulation writes it ("Hero XP: slot 0 +X (source), xp T", "Hero
    # level: ...", hol.Hero.Report's "Hero true slot 0: state S level L hp H xp T ...").
    $HeroXP = @(Get-Section "build hero xp" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $Gains = @($HeroXP | Select-String -Pattern "Hero XP: slot 0 \+(\d+) \((\w+)\), xp (\d+)" | ForEach-Object { "+$($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value) = $($_.Matches[0].Groups[3].Value)" })
    $Levels = @($HeroXP | Select-String -Pattern "Hero level: slot 0 (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
    $XPSheets = @($HeroXP | Select-String -Pattern "Hero true slot 0: (state \w+ level \d+ hp \d+ xp \d+)" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $LevelUp = [regex]::Match("$($Levels | Select-Object -First 1)", "^level 1 -> 2, hp (\d+) / 486$")
    $Checks["his Peasant kill: combat +50 = 200, level 2, the 36 HP gained added ($(($Gains | Select-Object -First 1)); $($Levels | Select-Object -First 1); $($XPSheets | Select-Object -First 1))"] =
        ($Gains.Count -ge 1) -and ($Gains[0] -eq "+50 combat = 200") -and $LevelUp.Success -and ([int]$LevelUp.Groups[1].Value -ge 36) -and
        ($XPSheets.Count -ge 1) -and ($XPSheets[0] -match "^state Alive level 2 hp \d+ xp 200$")
    $Checks["dead, a House finished: development +25 = 225, the sheet still level 2 ($(($Gains | Select-Object -Skip 1 -First 1)); $($XPSheets | Select-Object -Skip 1 -First 1))"] =
        [bool]($HeroXP | Select-String -SimpleMatch "hol.Hero.Kill: slot 0, 1 hero(es)" -Quiet) -and ($Gains.Count -eq 2) -and ($Gains[1] -eq "+25 development = 225") -and
        ($XPSheets.Count -ge 2) -and ($XPSheets[1] -match "^state (Dead|ReadyToReturn) level 2 hp 0 xp 225$")
    $Checks["back by the forum: level 2 at full health ($($Levels | Select-Object -Skip 1 -First 1); $($XPSheets | Select-Object -Skip 2 -First 1))"] =
        [bool]($HeroXP | Select-String -SimpleMatch "Hero return: slot 0 by its main forum" -Quiet) -and ($Levels.Count -ge 2) -and ($Levels[1] -eq "level 1 -> 2, hp 486 / 486") -and
        ($XPSheets.Count -ge 3) -and ($XPSheets[2] -eq "state Alive level 2 hp 486 xp 225")
    # J8b.3: the return at level 2 dearer (100 + 50 x 2 = 200 at the ready step, bought just after), his block at level 2.
    $XPPrice = @($HeroXP | Select-String -Pattern "Hero return: slot 0 bought for (\d+) gold" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $XPBlock = "$($HeroXP | Select-String -Pattern "HUD hero: (.*)$" | Select-Object -Skip 1 -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })"
    $Checks["the return at level 2 bought for $($XPPrice -join ', ') gold (150 < P <= 200), his block back: $XPBlock"] =
        ($XPPrice.Count -eq 1) -and ($XPPrice[0] -gt 150) -and ($XPPrice[0] -le 200) -and ($XPBlock -eq "alive level 2 hp 486/486")
    # J8b.4 (J8b-Q4): the XP's bar (after his kill, back, at 600), the alerts (his level, tier 1 at level 3; one sound each
    # time), his golden flash at each level up of his sheet (none at his return: the same level), the captures.
    $XPBars = @($HeroXP | Select-String -Pattern "HUD hero xp: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
    $Checks["his XP's bar: xp 200/600 bar 0%, xp 225/600 bar 6%, xp 600/1300 bar 0%, xp 1300/2200 bar 0%, xp 3300 max bar 100% ($($XPBars -join '; '))"] =
        (($XPBars -join ";") -eq "xp 200/600 bar 0%;xp 225/600 bar 6%;xp 600/1300 bar 0%;xp 1300/2200 bar 0%;xp 3300 max bar 100%")
    $XPAlerts = @($HeroXP | Select-String -Pattern "Hero message: (.*) \(slot 0\)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() } | Where-Object { $_ -notmatch "prêt" })
    $XPSounds = @($HeroXP | Select-String -Pattern "Hero level alert: slot 0 level (\d -> \d) \(sound\)" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["the alerts: niveau 2, niveau 3 and Palier 1 atteint, in English level 4, in Japanese level 6 and tier 2, a sound each time ($($XPAlerts -join '; '); sounds $($XPSounds -join ', '))"] =
        (($XPAlerts -join ";") -eq "Paladin-Commandant : niveau 2;Paladin-Commandant : niveau 3;Palier 1 atteint;Paladin-Commander: level 4;聖騎士団長：レベル 6;段階2に到達") -and
        (($XPSounds -join ",") -eq "1 -> 2,2 -> 3,3 -> 4,4 -> 6")
    $XPFlashes = @($HeroXP | Select-String -Pattern "Hero flash: slot (\d) level (\d -> \d)" | ForEach-Object { "slot $($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" })
    $Checks["his golden flash at each level up, none at his return ($($XPFlashes -join '; '))"] = (($XPFlashes -join ";") -eq "slot 0 1 -> 2;slot 0 2 -> 3;slot 0 3 -> 4;slot 0 4 -> 6")
    $Missing = @("flash", "tier", "tier_ja" | Where-Object { -not (Test-Path (Join-Path $Shots "build_hero_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build hero xp" $Checks) -and $AllOk
    # J8c.2 loop 2: the aura, the cry and the banner in a match, read from the hits (hol.Combat.LogHits: "... ManAtArms hits
    # #N Spearman for D", "arrow of #N Archer lands on #N Spearman for D"), in three phases: before the cry, from the cry to
    # level 4, after it (the banner).
    $Aura = @(Get-Section "build hero aura" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $CryAt = -1; $BannerAt = -1
    for ($i = 0; $i -lt $Aura.Count; $i++) {
        if ($CryAt -lt 0 -and $Aura[$i] -match "slot 0 cries Serrez les rangs, ") { $CryAt = $i }
        if ($BannerAt -lt 0 -and $Aura[$i] -match "Hero level: slot 0 level 1 -> 4") { $BannerAt = $i }
    }
    $AuraPhases = @(
        , @(if ($CryAt -gt 0) { $Aura[0..($CryAt - 1)] })
        , @(if ($CryAt -ge 0 -and $BannerAt -gt $CryAt) { $Aura[$CryAt..($BannerAt - 1)] })
        , @(if ($BannerAt -ge 0) { $Aura[$BannerAt..($Aura.Count - 1)] }))
    $AuraHits = @($AuraPhases | ForEach-Object { , @($_ | Select-String -Pattern "ManAtArms hits #\d+ (Spearman|Hero) for ([\d.]+)," | ForEach-Object { "$($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" }) })
    $AuraArrows = @($AuraPhases | ForEach-Object { , @($_ | Select-String -Pattern "Archer lands on #\d+ Spearman for ([\d.]+)," | ForEach-Object { $_.Matches[0].Groups[1].Value }) })
    $Checks["in his aura: the Man-at-arms' hits Spearman 10.2 or Hero 3.06, the arrows 8.5 (hits $($AuraHits[0] -join ', '); arrows $($AuraArrows[0] -join ', '))"] =
        ($AuraHits[0].Count -ge 1) -and (@($AuraHits[0] | Where-Object { $_ -notin @("Spearman 10.2", "Hero 3.06") }).Count -eq 0) -and
        ($AuraArrows[0].Count -ge 1) -and (@($AuraArrows[0] | Where-Object { $_ -ne "8.5" }).Count -eq 0)
    $Cried = if ($CryAt -ge 0 -and $Aura[$CryAt] -match "cries Serrez les rangs, (\d+) unit") { [int]$Matches[1] } else { -1 }
    $Checks["the cry: him and the Spearman under it ($Cried units), refused again during its cool-down"] =
        ($Cried -ge 2) -and (@($Aura | Select-String -SimpleMatch "Hero ability: slot 0 cries Serrez les rangs").Count -eq 1) -and
        [bool]($Aura | Select-String -SimpleMatch "hol.Hero.CloseRanks: slot 0 refused" -Quiet)
    $Checks["under aura and cry: hits Spearman 7.2 or Hero 2.16, arrows 6 (hits $($AuraHits[1] -join ', '); arrows $($AuraArrows[1] -join ', '))"] =
        ($AuraHits[1].Count -ge 1) -and (@($AuraHits[1] | Where-Object { $_ -notin @("Spearman 7.2", "Hero 2.16") }).Count -eq 0) -and
        ($AuraArrows[1].Count -ge 1) -and (@($AuraArrows[1] | Where-Object { $_ -ne "6" }).Count -eq 0)
    $Checks["level 4: the banner refused 17 m away, planted 16 m away"] =
        [bool]($Aura | Select-String -SimpleMatch "hol.Hero.Banner: slot 0 at ($($BuildHero[0] + 1700), $($BuildHero[1])) refused" -Quiet) -and
        [bool]($Aura | Select-String -SimpleMatch "hol.Hero.Banner: slot 0 at ($($BuildHero[0] + 1600), $($BuildHero[1])) planted" -Quiet)
    $Checks["22 m from him, 6 m from his banner: hits 10.2 ($($AuraHits[2] -join ', ')); the abilities reset at the end"] =
        ($AuraHits[2].Count -ge 1) -and (@($AuraHits[2] | Where-Object { $_ -ne "Spearman 10.2" }).Count -eq 0) -and
        [bool]($Aura | Select-String -SimpleMatch "hol.Hero.ResetAbilities: slot 0 reset" -Quiet)
    $AllOk = (Report "build hero aura" $Checks) -and $AllOk
    # J8c.4 loop 1: the abilities from the grid's keys (the grid refuses what is not ready, the server casts the rest).
    $GridCase = @(Get-Section "build hero grid" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $Checks["level 1: Z refused by the grid (the banner opens at 4)"] = [bool]($GridCase | Select-String -SimpleMatch "Grid: hero banner not ready (0 s)" -Quiet)
    $Checks["A: the cry sent and cast"] = [bool]($GridCase | Select-String -SimpleMatch "Grid: hero cry" -Quiet) -and
        (@($GridCase | Select-String -SimpleMatch "Hero ability: slot 0 cries Serrez les rangs").Count -eq 1)
    $CrySeconds = if (($GridCase -join "`n") -match "Grid: hero cry not ready \((\d+) s\)") { [int]$Matches[1] } else { -1 }
    $Checks["A again: refused by the grid, $CrySeconds s left (59 or 60)"] = $CrySeconds -ge 59 -and $CrySeconds -le 60
    $Planted = if (($GridCase -join "`n") -match "Hero ability: slot 0 plants his banner at \((-?\d+), (-?\d+)\)") { @([int]$Matches[1], [int]$Matches[2]) } else { $null }
    $Checks["level 4: Z then a left click 10 m from him plants the banner there (at $($Planted -join ', '), 30 cm at most off)"] =
        [bool]($GridCase | Select-String -SimpleMatch "Grid: hero banner, click on the ground" -Quiet) -and ($null -ne $Planted) -and
        ([Math]::Abs($Planted[0] - ($BuildHero[0] + 1000)) -le 30) -and ([Math]::Abs($Planted[1] - $BuildHero[1]) -le 30)
    # J8c.4 loop 2: the HUD at the four reports (selected, after the cry, level 4, banner planted).
    $GridReports = @($GridCase | Select-String -SimpleMatch "HUD report:" | ForEach-Object { $_.Line })
    $GridCells = @($GridCase | Select-String -SimpleMatch "HUD cells:" | ForEach-Object { $_.Line })
    $GridTips = @($GridCase | Select-String -SimpleMatch "HUD tooltips:" | ForEach-Object { $_.Line })
    $GridBoxes = @($GridCase | Select-String -SimpleMatch "HUD hero abilities:" | ForEach-Object { $_.Line -replace '^.*HUD hero abilities: ', '' })
    $Checks["4 reports ($($GridReports.Count), block lines $($GridBoxes.Count))"] = $GridReports.Count -eq 4 -and $GridBoxes.Count -eq 4
    if ($GridReports.Count -eq 4 -and $GridBoxes.Count -eq 4) {
        $Checks["selected, level 1: grid A=CloseRanks enabled Z=Banner greyed, the banner's tooltip 'Niveau 4 requis'; block '$($GridBoxes[0])'"] =
            $GridReports[0].Contains("A=CloseRanks enabled Z=Banner greyed") -and $GridTips[0].Contains("Bannière de l'Aube") -and
            $GridTips[0].Contains("Niveau 4 requis") -and $GridBoxes[0].StartsWith("aura on, cry ready, banner locked 4 | Serrez les rangs (A) / ")
        $Checks["after the cry: A greyed, 'Rangs 59/60 s' on it; block '$($GridBoxes[1])'"] =
            $GridReports[1].Contains("A=CloseRanks greyed") -and ($GridCells[1] -match "A: Rangs (59|60) s") -and
            ($GridBoxes[1] -match "^aura on, cry (59|60) s, banner locked 4 \| Serrez les rangs \(A\) / .+ / Prête dans (59|60) s$")
        $Checks["level 4: Z=Banner enabled; block '$($GridBoxes[2])'"] =
            $GridReports[2].Contains("Z=Banner enabled") -and ($GridBoxes[2] -match "^aura on, cry \d+ s, banner ready")
        $Checks["banner planted: Z greyed; block '$($GridBoxes[3])'"] =
            $GridReports[3].Contains("Z=Banner greyed") -and ($GridBoxes[3] -match "^aura on, cry \d+ s, banner (119|120) s")
    }
    # J8c.4 loop 3: the aura circles, the player's alone (hol.Select.Report's "Aura rings:" at the 4 points).
    $AuraRings = @($GridCase | Select-String -SimpleMatch "Aura rings:" | ForEach-Object { $_.Line -replace '^.*Aura rings: ', '' })
    $Checks["4 aura ring lines ($($AuraRings.Count))"] = $AuraRings.Count -eq 4
    if ($AuraRings.Count -eq 4) {
        $Checks["selected: his hero's circle of 12 m ('$($AuraRings[0])')"] = $AuraRings[0] -match "^hero \(-?\d+, -?\d+\) 1200$"
        $Checks["aimed 10 m away: the reach of 16 m, the aura there in reach ('$($AuraRings[1])')"] =
            $AuraRings[1] -match "^hero \(-?\d+, -?\d+\) 1200, reach 1600, aim \(-?\d+, -?\d+\) ok$"
        $Checks["aimed 18 m away: out of reach ('$($AuraRings[2])')"] =
            $AuraRings[2] -match "^hero \(-?\d+, -?\d+\) 1200, reach 1600, aim \(-?\d+, -?\d+\) out of reach$"
        $Checks["planted: the hero's and the banner's circles, the aim gone ('$($AuraRings[3])')"] =
            $AuraRings[3] -match "^hero \(-?\d+, -?\d+\) 1200, banner \(-?\d+, -?\d+\) 1200$"
    }
    # J8c.4 loop 5: the cry's white flash, on the units of his player within 12 m of him (at least his hero), once.
    $CryFlashes = @($GridCase | Select-String -Pattern "Cry flash: slot 0, (\d+) units" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $Checks["the cry flashed once on at least his hero ($($CryFlashes -join ', ') units)"] = ($CryFlashes.Count -eq 1) -and ($CryFlashes[0] -ge 1)
    # J8c.4 loop 4: his planted banner drawn (hol.Hero.BannerDisplay), one standard of slot 0 10 m from his hero.
    $BannerShown = "$($GridCase | Select-String -Pattern "Banner display: (1 shown, slot 0 \(-?\d+, -?\d+, -?\d+\))" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })"
    $Checks["planted: one banner drawn at the place ('$BannerShown')"] =
        ($BannerShown -match "^1 shown, slot 0 \((-?\d+), (-?\d+), -?\d+\)$") -and ([math]::Abs([int]$Matches[1] - ($AuraHero[0] + 1000)) -le 20) -and ([math]::Abs([int]$Matches[2] - $AuraHero[1]) -le 20)
    $AllOk = (Report "build hero grid" $Checks) -and $AllOk
    # L.1 (D485): a right click on a finished deposit. The units of hol.Units.Where (before the order, 10 s and 13 s
    # after): the four selected are the four newest (three gatherers, then the empty one); the unloads after the order.
    $DropOrder = @(Get-Section "build dropoff order" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $ReadWhere = { param($Line) $Units = @{}
        foreach ($Part in ($Line -replace '^.*Units where: \d+ units: ', '') -split '; ') {
            if ($Part -match '^#(\d+) slot 0 at \((-?\d+), (-?\d+)\) (\w+)(?:, carries (\w+) ([^,]+))?') {
                $Units[[int]$Matches[1]] = [pscustomobject]@{ X = [int]$Matches[2]; Y = [int]$Matches[3]; Activity = $Matches[4]; Load = $Matches[5]; Amount = $Matches[6] } } }
        $Units }
    $Wheres = @($DropOrder | Where-Object { $_ -match "Units where: " } | ForEach-Object { , (& $ReadWhere $_) })
    $DropUnits = if ($Wheres.Count -ge 1) { @($Wheres[0].Keys | Sort-Object | Select-Object -Last 4) } else { @() }
    $DropCarriers = @($DropUnits | Select-Object -First 3)
    $DropEmptyId = @($DropUnits | Select-Object -Last 1)
    $DropDist = { param($U, $At) if ($U) { [Math]::Round([Math]::Sqrt([Math]::Pow($U.X - $At[0], 2) + [Math]::Pow($U.Y - $At[1], 2))) } else { -1 } }
    $Before = @($DropCarriers | ForEach-Object { $U = $Wheres[0][$_]; "#$_ $($U.Activity) $($U.Load) $($U.Amount), forum $(& $DropDist $U @(-15000, 0)) cm, D $(& $DropDist $U $DropD) cm" })
    $Checks["set-up: the three newest carry wood (partly), the forum nearer them than D; the fourth carries nothing ($($Before -join '; '))"] =
        ($DropUnits.Count -eq 4) -and (@($DropCarriers | Where-Object { $Wheres[0][$_].Load -eq "Wood" -and [int]$Wheres[0][$_].Amount -gt 0 -and [int]$Wheres[0][$_].Amount -lt 10 }).Count -eq 3) -and
        (@($DropCarriers | Where-Object { (& $DropDist $Wheres[0][$_] @(-15000, 0)) -lt (& $DropDist $Wheres[0][$_] $DropD) }).Count -ge 1) -and
        (-not $Wheres[0][$DropEmptyId[0]].Load)
    $AtD = "LumberCamp ($($DropD -join ', '))"
    $DropUnloads = @($DropOrder | Select-String -Pattern "Gather: slot 0 unloaded (\d+) (\w+) \(stock [^)]*\) at (\w+) \((-?\d+), (-?\d+)\), unit #(\d+)" | ForEach-Object {
        $G = $_.Matches[0].Groups; [pscustomobject]@{ Amount = [int]$G[1].Value; Resource = $G[2].Value; At = "$($G[3].Value) ($($G[4].Value), $($G[5].Value))"; Unit = [int]$G[6].Value } } |
        Where-Object { $DropUnits -contains $_.Unit })
    $AtDUnits = @($DropUnloads | Where-Object { $_.At -eq $AtD -and $_.Resource -eq "Wood" -and $DropCarriers -contains $_.Unit } | ForEach-Object { $_.Unit } | Sort-Object -Unique)
    $Checks["right click on D: each of the three unloads its wood at D, nowhere else, the empty one nothing ($($DropUnloads.Count) unloads: $(($DropUnloads | ForEach-Object { "#$($_.Unit) $($_.Amount) $($_.Resource) at $($_.At)" }) -join '; '))"] =
        ($AtDUnits.Count -eq 3) -and (@($DropUnloads | Where-Object { $_.At -ne $AtD -or $DropEmptyId -contains $_.Unit }).Count -eq 0)
    $After = @(1, 2 | Where-Object { $_ -lt $Wheres.Count } | ForEach-Object { $W = $Wheres[$_]; @($DropUnits | ForEach-Object { $U = $W[$_]; "#$_ $(if ($U) { "$($U.Activity) $($U.Load) $($U.Amount), D $(& $DropDist $U $DropD) cm" } else { 'gone' })" }) -join ', ' })
    $Checks["then the four idle in front of D (within 12 m), carrying nothing, 12 s and 15 s after the order ($($After -join ' | '))"] =
        ($Wheres.Count -ge 3) -and ($DropUnits.Count -eq 4) -and (@(1, 2 | ForEach-Object { $W = $Wheres[$_]; $DropUnits | Where-Object { -not $W[$_] -or $W[$_].Activity -ne "Idle" -or $W[$_].Load -or (& $DropDist $W[$_] $DropD) -gt 1200 } }).Count -eq 0)
    $AllOk = (Report "build dropoff" $Checks) -and $AllOk

    # L.3 (D485): Shift + right click on D, after house S. hol.Units.Where before the order, at S's first work, when S is
    # finished, 10 s and 13 s after: the two selected are the two newest (the carrier, then the empty one).
    $ShiftOrder = @(Get-Section "build dropoffshift order" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $ShiftWheres = @($ShiftOrder | Where-Object { $_ -match "Units where: " } | ForEach-Object { , (& $ReadWhere $_) })
    $ShiftUnits = if ($ShiftWheres.Count -ge 1) { @($ShiftWheres[0].Keys | Sort-Object | Select-Object -Last 2) } else { @() }
    $ShiftCarrier = if ($ShiftUnits.Count -eq 2) { $ShiftUnits[0] } else { -1 }
    $ShiftEmpty = if ($ShiftUnits.Count -eq 2) { $ShiftUnits[1] } else { -1 }
    $ShiftSay = { param($W, $Id) $U = $W[$Id]; if ($U) { "#$Id $($U.Activity) $($U.Load) $($U.Amount), D $(& $DropDist $U $DropD) cm, S $(& $DropDist $U $DropShiftHouse) cm" } else { "#$Id gone" } }
    $Checks["set-up: the carrier holds some wood, the other nothing ($(if ($ShiftWheres.Count -ge 1) { (& $ShiftSay $ShiftWheres[0] $ShiftCarrier), (& $ShiftSay $ShiftWheres[0] $ShiftEmpty) -join '; ' }))"] =
        ($ShiftUnits.Count -eq 2) -and $ShiftWheres[0][$ShiftCarrier].Load -eq "Wood" -and [int]$ShiftWheres[0][$ShiftCarrier].Amount -gt 0 -and
        [int]$ShiftWheres[0][$ShiftCarrier].Amount -lt 10 -and -not $ShiftWheres[0][$ShiftEmpty].Load
    $ShiftAt = [regex]::Escape("House at ($($DropShiftHouse -join ', '))")
    $ShiftEvents = @($ShiftOrder | ForEach-Object {
        if ($_ -match "Construction: slot 0 finished $ShiftAt") { [pscustomobject]@{ Kind = "finished"; Unit = -1; At = "" } }
        elseif ($_ -match "Gather: slot 0 unloaded (\d+) (\w+) \(stock [^)]*\) at (\w+) \((-?\d+), (-?\d+)\), unit #(\d+)" -and $ShiftUnits -contains [int]$Matches[6]) {
            [pscustomobject]@{ Kind = "unloaded $($Matches[1]) $($Matches[2])"; Unit = [int]$Matches[6]; At = "$($Matches[3]) ($($Matches[4]), $($Matches[5]))" } } })
    $ShiftTokens = @($ShiftEvents | ForEach-Object { if ($_.Unit -ge 0) { "#$($_.Unit) $($_.Kind) at $($_.At)" } else { "S finished" } })
    $FinishedAt = [array]::IndexOf(@($ShiftEvents | ForEach-Object { $_.Kind }), "finished")
    $EarlyUnloads = @(for ($i = 0; $i -lt $ShiftEvents.Count; $i++) { if ($ShiftEvents[$i].Unit -eq $ShiftCarrier -and ($FinishedAt -lt 0 -or $i -lt $FinishedAt)) { $ShiftEvents[$i] } })
    $AtWork = if ($ShiftWheres.Count -ge 2) { $ShiftWheres[1] } else { @{} }
    $Checks["(1) the carrier does not unload before S is finished, and builds S ($($ShiftTokens -join '; ') | at S's first work: $(if ($ShiftWheres.Count -ge 2) { & $ShiftSay $AtWork $ShiftCarrier }))"] =
        ($FinishedAt -ge 0) -and ($EarlyUnloads.Count -eq 0) -and ($ShiftCarrier -ge 0) -and $AtWork[$ShiftCarrier] -and $AtWork[$ShiftCarrier].Activity -eq "Building"
    $LateUnloads = @(for ($i = $FinishedAt + 1; $FinishedAt -ge 0 -and $i -lt $ShiftEvents.Count; $i++) { if ($ShiftEvents[$i].Unit -eq $ShiftCarrier) { $ShiftEvents[$i] } })
    $Checks["(2) after S, the carrier unloads its wood at D, nowhere else"] =
        ($LateUnloads.Count -ge 1) -and (@($LateUnloads | Where-Object { $_.At -ne $AtD -or $_.Kind -notmatch "Wood$" }).Count -eq 0) -and
        (@($ShiftEvents | Where-Object { $_.Unit -eq $ShiftCarrier -and $_.At -ne $AtD }).Count -eq 0)
    $ShiftAfter = @(3, 4 | Where-Object { $_ -lt $ShiftWheres.Count } | ForEach-Object { "$(& $ShiftSay $ShiftWheres[$_] $ShiftCarrier), $(& $ShiftSay $ShiftWheres[$_] $ShiftEmpty)" })
    $Checks["(3) then the carrier idle in front of D (within 12 m), carrying nothing, 10 s and 13 s after S ($($ShiftAfter -join ' | '))"] =
        ($ShiftWheres.Count -ge 5) -and (@(3, 4 | Where-Object { $U = $ShiftWheres[$_][$ShiftCarrier]; -not $U -or $U.Activity -ne "Idle" -or $U.Load -or (& $DropDist $U $DropD) -gt 1200 }).Count -eq 0)
    # It may still walk to S at the carrier's first work (it starts farther): building at that work or at S's end.
    $AtEnd = if ($ShiftWheres.Count -ge 3) { $ShiftWheres[2] } else { @{} }
    $Checks["(4) the empty one builds S, unloads nothing, then idle carrying nothing (at S's first work: $(if ($ShiftWheres.Count -ge 2) { & $ShiftSay $AtWork $ShiftEmpty }); at S's end: $(if ($ShiftWheres.Count -ge 3) { & $ShiftSay $AtEnd $ShiftEmpty }))"] =
        ($ShiftWheres.Count -ge 5) -and (@($AtWork, $AtEnd | Where-Object { $_[$ShiftEmpty] -and $_[$ShiftEmpty].Activity -eq "Building" }).Count -ge 1) -and (@($ShiftEvents | Where-Object { $_.Unit -eq $ShiftEmpty }).Count -eq 0) -and
        (@(3, 4 | Where-Object { $U = $ShiftWheres[$_][$ShiftEmpty]; -not $U -or $U.Activity -ne "Idle" -or $U.Load }).Count -eq 0)
    $AllOk = (Report "build dropoffshift" $Checks) -and $AllOk

    # L.5 (D485): Shift moves round a Shift + right click on D. The section's events in order (hol.Units.Where, the unloads
    # of the Peasant: the newest unit of the first Where), split at "Group 5: selected" into A (P then D) and B (D then B).
    $MoveOrder = @(Get-Section "build dropoffmove order" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $MoveEvents = New-Object System.Collections.Generic.List[object]
    $MovePeasant = -1
    $MovePart = "A"
    foreach ($Line in $MoveOrder) {
        if ($Line -match "Group 5: selected") { $MovePart = if ($MovePart -eq "A") { "B" } else { "C" } }
        elseif ($Line -match "Units where: ") {
            $W = & $ReadWhere $Line
            if ($MovePeasant -lt 0 -and $W.Count -gt 0) { $MovePeasant = @($W.Keys | Sort-Object | Select-Object -Last 1)[0] }
            $MoveEvents.Add([pscustomobject]@{ Part = $MovePart; Kind = "where"; U = $W[$MovePeasant]; At = "" })
        }
        elseif ($Line -match "Gather: slot 0 unloaded (\d+) (\w+) \(stock [^)]*\) at (\w+) \((-?\d+), (-?\d+)\), unit #(\d+)" -and [int]$Matches[6] -eq $MovePeasant) {
            $MoveEvents.Add([pscustomobject]@{ Part = $MovePart; Kind = "unload $($Matches[1]) $($Matches[2])"; U = $null; At = "$($Matches[3]) ($($Matches[4]), $($Matches[5]))" })
        }
    }
    $MoveSay = { param($U) if ($U) { "$($U.Activity) $($U.Load) $($U.Amount) at ($($U.X), $($U.Y))" } else { "gone" } }
    foreach ($Part in "A", "B", "C") {
        $To = if ($Part -eq "A") { $DropMoveP } else { $DropMoveB }
        $PE = @($MoveEvents | Where-Object { $_.Part -eq $Part })
        $PW = @($PE | Where-Object { $_.Kind -eq "where" })
        $FirstUnload = -1
        for ($i = 0; $i -lt $PE.Count; $i++) { if ($PE[$i].Kind -ne "where") { $FirstUnload = $i; break } }
        $Unloads = @($PE | Where-Object { $_.Kind -ne "where" })
        $Near = @(for ($i = 0; $i -lt $PE.Count; $i++) { if ($PE[$i].Kind -eq "where" -and $PE[$i].U -and (& $DropDist $PE[$i].U $To) -le 300) { $i } })
        $Closest = @($PW | Where-Object { $_.U } | ForEach-Object { & $DropDist $_.U $To } | Sort-Object | Select-Object -First 1)
        $Story = @($PE | ForEach-Object { if ($_.Kind -eq "where") { if ($_.U) { "$($_.U.Activity) $($_.U.Load) $($_.U.Amount) to-$Part $(& $DropDist $_.U $To) D $(& $DropDist $_.U $DropD)" } else { "gone" } } else { "$($_.Kind) at $($_.At)" } })
        Write-Host "  dropoffmove $Part (#$MovePeasant): $($Story -join ' | ')"
        $Checks["$Part set-up: the Peasant carries some wood before the orders ($(if ($PW.Count -ge 1) { & $MoveSay $PW[0].U }))"] =
            ($PW.Count -ge 1) -and $PW[0].U -and $PW[0].U.Load -eq "Wood" -and [int]$PW[0].U.Amount -gt 0 -and [int]$PW[0].U.Amount -lt 10
        $Checks["$Part it unloads its wood at D, nowhere else ($(($Unloads | ForEach-Object { "$($_.Kind) at $($_.At)" }) -join '; '))"] =
            ($Unloads.Count -ge 1) -and (@($Unloads | Where-Object { $_.At -ne $AtD -or $_.Kind -notmatch "Wood$" }).Count -eq 0)
        if ($Part -eq "A") {
            $Checks["A Shift P then Shift D: it passes within 3 m of P before it unloads (closest $Closest cm; first near at event $(@($Near)[0]), first unload at $FirstUnload)"] =
                ($Near.Count -ge 1) -and ($FirstUnload -ge 0) -and ($Near[0] -lt $FirstUnload)
            $Checks["A then idle in front of D (within 12 m), carrying nothing ($(if ($PW.Count -ge 1) { & $MoveSay $PW[-1].U }), D $(if ($PW.Count -ge 1) { & $DropDist $PW[-1].U $DropD }) cm)"] =
                ($PW.Count -ge 3) -and $PW[-1].U -and $PW[-1].U.Activity -eq "Idle" -and -not $PW[-1].U.Load -and (& $DropDist $PW[-1].U $DropD) -le 1200
        } elseif ($Part -eq "C") {
            $NearP = @(for ($i = 0; $i -lt $PE.Count; $i++) { if ($PE[$i].Kind -eq "where" -and $PE[$i].U -and (& $DropDist $PE[$i].U $DropMoveP) -le 300) { $i } })
            $ClosestP = @($PW | Where-Object { $_.U } | ForEach-Object { & $DropDist $_.U $DropMoveP } | Sort-Object | Select-Object -First 1)
            $Checks["C Shift P, Shift D, Shift B: it passes within 3 m of P before it unloads (closest $ClosestP cm; first near at event $(@($NearP)[0]), first unload at $FirstUnload)"] =
                ($NearP.Count -ge 1) -and ($FirstUnload -ge 0) -and ($NearP[0] -lt $FirstUnload)
            $Checks["C then within 3 m of B after it unloads (closest $Closest cm; first near at event $(@($Near)[0]), first unload at $FirstUnload)"] =
                ($Near.Count -ge 1) -and ($FirstUnload -ge 0) -and ($Near[0] -gt $FirstUnload)
            $Checks["C then idle within 3 m of B, carrying nothing ($(if ($PW.Count -ge 1) { & $MoveSay $PW[-1].U }), B $(if ($PW.Count -ge 1) { & $DropDist $PW[-1].U $DropMoveB }) cm)"] =
                ($PW.Count -ge 3) -and $PW[-1].U -and $PW[-1].U.Activity -eq "Idle" -and -not $PW[-1].U.Load -and (& $DropDist $PW[-1].U $DropMoveB) -le 300
        } else {
            $Checks["B Shift D then Shift B: it comes within 3 m of B only after it unloads (closest $Closest cm; first near at event $(@($Near)[0]), first unload at $FirstUnload)"] =
                ($Near.Count -ge 1) -and ($FirstUnload -ge 0) -and ($Near[0] -gt $FirstUnload)
            $Checks["B then idle within 3 m of B, carrying nothing ($(if ($PW.Count -ge 1) { & $MoveSay $PW[-1].U }), B $(if ($PW.Count -ge 1) { & $DropDist $PW[-1].U $DropMoveB }) cm)"] =
                ($PW.Count -ge 3) -and $PW[-1].U -and $PW[-1].U.Activity -eq "Idle" -and -not $PW[-1].U.Load -and (& $DropDist $PW[-1].U $DropMoveB) -le 300
        }
    }
    $AllOk = (Report "build dropoffmove" $Checks) -and $AllOk

    # L.6a: Shift + ground then Shift + work. Each section's Peasant: the newest unit of its first hol.Units.Where.
    $Checks = [ordered]@{}
    $SwWheres = @(Get-Section "build shiftwork order" | Where-Object { $_ -match "Units where: " } | ForEach-Object { , (& $ReadWhere $_) })
    $SwA = if ($SwWheres.Count -ge 1 -and $SwWheres[0].Count -gt 0) { @($SwWheres[0].Keys | Sort-Object | Select-Object -Last 1)[0] } else { -1 }
    $SwAU = @($SwWheres | ForEach-Object { $_[$SwA] })
    $SwStory = @($SwAU | ForEach-Object { if ($_) { "$($_.Activity) $($_.Load) $($_.Amount) Pa $(& $DropDist $_ $ShiftWorkPa)" } else { "gone" } })
    Write-Host "  shiftwork a (#$SwA): $($SwStory -join ' | ')"
    $SwNear = @(for ($i = 0; $i -lt $SwAU.Count; $i++) { if ($SwAU[$i] -and (& $DropDist $SwAU[$i] $ShiftWorkPa) -le 300) { $i } })
    $SwChop = -1
    for ($i = 0; $i -lt $SwAU.Count; $i++) { if ($SwAU[$i] -and $SwAU[$i].Activity -eq "Gathering" -and $SwAU[$i].Load -eq "Wood" -and [int]$SwAU[$i].Amount -gt 0) { $SwChop = $i; break } }
    $SwClosest = @($SwAU | Where-Object { $_ } | ForEach-Object { & $DropDist $_ $ShiftWorkPa } | Sort-Object | Select-Object -First 1)
    $Checks["(a) set-up: the Peasant carries nothing before the orders ($(if ($SwAU.Count -ge 1) { & $MoveSay $SwAU[0] }))"] = ($SwAU.Count -ge 1) -and $SwAU[0] -and -not $SwAU[0].Load
    $Checks["(a) Shift Pa then Shift tree: it passes within 3 m of Pa before it starts gathering (closest $SwClosest cm; first near at sample $(@($SwNear)[0]), first wood at sample $SwChop)"] =
        ($SwNear.Count -ge 1) -and ($SwChop -ge 0) -and ($SwNear[0] -lt $SwChop)
    $Checks["(a) then it gathers wood (first wood at sample $SwChop)"] = $SwChop -ge 0
    $SwSite = @(Get-Section "build shiftwork site" | ForEach-Object { $_ })
    $SwHAt = [regex]::Escape("House at ($($ShiftWorkH -join ', '))")
    $SwBWheres = @($SwSite | Where-Object { $_ -match "Units where: " } | ForEach-Object { , (& $ReadWhere $_) })
    $SwB = if ($SwBWheres.Count -ge 1 -and $SwBWheres[0].Count -gt 0) { @($SwBWheres[0].Keys | Sort-Object | Select-Object -Last 1)[0] } else { -1 }
    $SwBU = @($SwBWheres | ForEach-Object { $_[$SwB] })
    $SwBStory = @($SwBU | ForEach-Object { if ($_) { "$($_.Activity) Pb $(& $DropDist $_ $ShiftWorkPb) H $(& $DropDist $_ $ShiftWorkH)" } else { "gone" } })
    Write-Host "  shiftwork b (#$SwB): $($SwBStory -join ' | ')"
    Write-Host "  shiftwork b log: $(@($SwSite | Select-String -Pattern "foundation at|first work on $SwHAt|finished $SwHAt|Queued|queued" | ForEach-Object { ($_.Line -replace '^.*?(Construction|Queued|Move|Build|Gather|Drop)', '$1') }) -join ' / ')"
    $SwBNear = @(for ($i = 0; $i -lt $SwBU.Count; $i++) { if ($SwBU[$i] -and (& $DropDist $SwBU[$i] $ShiftWorkPb) -le 300) { $i } })
    $SwBBuild = -1
    for ($i = 0; $i -lt $SwBU.Count; $i++) { if ($SwBU[$i] -and $SwBU[$i].Activity -eq "Building") { $SwBBuild = $i; break } }
    $SwBClosest = @($SwBU | Where-Object { $_ } | ForEach-Object { & $DropDist $_ $ShiftWorkPb } | Sort-Object | Select-Object -First 1)
    $Checks["(b) H placed, unbuilt before the Peasant's orders"] = [bool]($SwSite | Select-String -Pattern "placed House foundation at \($($ShiftWorkH -join ', ')\)" -Quiet)
    $Checks["(b) Shift Pb then Shift H: it passes within 3 m of Pb before it builds (closest $SwBClosest cm; first near at sample $(@($SwBNear)[0]), first Building at sample $SwBBuild)"] =
        ($SwBNear.Count -ge 1) -and ($SwBBuild -ge 0) -and ($SwBNear[0] -lt $SwBBuild)
    $Checks["(b) then it builds H (first Building at sample $SwBBuild)"] = $SwBBuild -ge 0
    $SwAfter = @(Get-Section "build shiftwork after" | ForEach-Object { $_ })
    $SwCWheres = @($SwAfter | Where-Object { $_ -match "Units where: " } | ForEach-Object { , (& $ReadWhere $_) })
    $SwC = if ($SwCWheres.Count -ge 1 -and $SwCWheres[0].Count -gt 0) { @($SwCWheres[0].Keys | Sort-Object | Select-Object -Last 1)[0] } else { -1 }
    $SwCStory = @($SwCWheres | ForEach-Object { $U = $_[$SwC]; if ($U) { "$($U.Activity) H $(& $DropDist $U $ShiftWorkH) Q $(& $DropDist $U $ShiftWorkQ)" } else { "gone" } })
    Write-Host "  shiftwork c, seen only (#$SwC, every 1 s): $($SwCStory -join ' | ')"
    Write-Host "  shiftwork c log: $(@($SwAfter | Select-String -Pattern "finished $SwHAt|Queued|queued" | ForEach-Object { ($_.Line -replace '^.*?(Construction|Queued|Move|Build|Gather|Drop)', '$1') }) -join ' / ')"
    $AllOk = (Report "build shiftwork" $Checks) -and $AllOk

    # M.1: Shift chains with a placement or a site. A section's events in order: its Peasant's hol.Units.Where samples (the
    # newest unit of the first one), a site's first work and its end.
    $ReadChain = { param($Lines, $Site)
        $SiteAt = [regex]::Escape("House at ($($Site -join ', '))")
        $Id = -1
        $Events = New-Object System.Collections.Generic.List[object]
        foreach ($Line in $Lines) {
            if ($Line -match "Units where: ") {
                $W = & $ReadWhere $Line
                if ($Id -lt 0 -and $W.Count -gt 0) { $Id = @($W.Keys | Sort-Object | Select-Object -Last 1)[0] }
                $Events.Add([pscustomobject]@{ Kind = "where"; U = $W[$Id] })
            }
            elseif ($Line -match "Construction: slot 0 first work on $SiteAt") { $Events.Add([pscustomobject]@{ Kind = "first work"; U = $null }) }
            elseif ($Line -match "Construction: slot 0 finished $SiteAt") { $Events.Add([pscustomobject]@{ Kind = "finished"; U = $null }) }
        }
        @{ Id = $Id; Events = $Events.ToArray() } }
    $ChainIndex = { param($Events, $Kind) for ($i = 0; $i -lt $Events.Count; $i++) { if ($Events[$i].Kind -eq $Kind) { return $i } }; -1 }
    $ChainNear = { param($Events, $At, $From) @(for ($i = [Math]::Max($From, 0); $i -lt $Events.Count; $i++) { if ($Events[$i].Kind -eq "where" -and $Events[$i].U -and (& $DropDist $Events[$i].U $At) -le 300) { $i } }) }
    $ChainClosest = { param($Events, $At) @($Events | Where-Object { $_.Kind -eq "where" -and $_.U } | ForEach-Object { & $DropDist $_.U $At } | Sort-Object | Select-Object -First 1) }
    $ChainStory = { param($Events, $Points) @($Events | ForEach-Object { $E = $_; if ($E.Kind -ne "where") { $E.Kind.ToUpper() } elseif ($E.U) {
        ("$($E.U.Activity) $($E.U.Load) " + (@($Points.Keys | ForEach-Object { "$_ $(& $DropDist $E.U $Points[$_])" }) -join ' ')) -replace '  ', ' ' } else { "gone" } }) -join ' | ' }

    $Checks = [ordered]@{}
    $SpLines = @(Get-Section "build shiftplace order" | ForEach-Object { $_ })
    $Sp = & $ReadChain $SpLines $ShiftPlaceH
    $SpE = $Sp.Events
    Write-Host "  shiftplace (#$($Sp.Id)): $(& $ChainStory $SpE ([ordered]@{ P = $ShiftPlaceP; H = $ShiftPlaceH; Q = $ShiftPlaceQ }))"
    Write-Host "  shiftplace log: $(@($SpLines | Select-String -Pattern "foundation at|Placement|Queued|queued|applied|Build order|Build: " | ForEach-Object { ($_.Line -replace '^.*?(Construction|Queued|Move|Build|Gather|Drop|Placement)', '$1') }) -join ' / ')"
    $SpWork = & $ChainIndex $SpE "first work"
    $SpDone = & $ChainIndex $SpE "finished"
    $SpNearP = & $ChainNear $SpE $ShiftPlaceP 0
    $SpNearQ = & $ChainNear $SpE $ShiftPlaceQ 0
    $SpWheres = @($SpE | Where-Object { $_.Kind -eq "where" })
    $Checks["set-up: the Peasant carries nothing, H placed with Shift ($(if ($SpWheres.Count -ge 1) { & $MoveSay $SpWheres[0].U }))"] =
        ($SpWheres.Count -ge 1) -and $SpWheres[0].U -and -not $SpWheres[0].U.Load -and [bool]($SpLines | Select-String -Pattern "placed House foundation at \($($ShiftPlaceH -join ', ')\)" -Quiet)
    $Checks["Shift P then Shift + place H: it passes within 3 m of P before H's first work (closest $(& $ChainClosest $SpE $ShiftPlaceP) cm; first near at event $(@($SpNearP)[0]), first work at event $SpWork)"] =
        ($SpNearP.Count -ge 1) -and ($SpWork -ge 0) -and ($SpNearP[0] -lt $SpWork)
    $Checks["then it builds H, finished (event $SpDone)"] = ($SpWork -ge 0) -and ($SpDone -ge $SpWork)
    $Checks["then Shift Q: within 3 m of Q only after H is finished (closest $(& $ChainClosest $SpE $ShiftPlaceQ) cm; first near at event $(@($SpNearQ)[0]))"] =
        ($SpNearQ.Count -ge 1) -and ($SpDone -ge 0) -and ($SpNearQ[0] -gt $SpDone)
    $Checks["then idle within 3 m of Q ($(if ($SpWheres.Count -ge 1) { & $MoveSay $SpWheres[-1].U }), Q $(if ($SpWheres.Count -ge 1) { & $DropDist $SpWheres[-1].U $ShiftPlaceQ }) cm)"] =
        ($SpWheres.Count -ge 3) -and $SpWheres[-1].U -and $SpWheres[-1].U.Activity -eq "Idle" -and (& $DropDist $SpWheres[-1].U $ShiftPlaceQ) -le 300
    $AllOk = (Report "build shiftplace" $Checks) -and $AllOk

    $Checks = [ordered]@{}
    $SsSetup = @(Get-Section "build shiftsite" | ForEach-Object { $_ })
    $SsLines = @(Get-Section "build shiftsite order" | ForEach-Object { $_ })
    $Ss = & $ReadChain $SsLines $ShiftSiteS
    $SsE = $Ss.Events
    Write-Host "  shiftsite a (#$($Ss.Id)): $(& $ChainStory $SsE ([ordered]@{ S = $ShiftSiteS; Q2 = $ShiftSiteQ2; Berries = $ShiftSiteBerries }))"
    Write-Host "  shiftsite a log: $(@($SsLines | Select-String -Pattern "Queued|queued|applied|Build order|Build: |Gather order" | ForEach-Object { ($_.Line -replace '^.*?(Construction|Queued|Move|Build|Gather|Drop)', '$1') }) -join ' / ')"
    $SsWork = & $ChainIndex $SsE "first work"
    $SsDone = & $ChainIndex $SsE "finished"
    $SsNearQ = & $ChainNear $SsE $ShiftSiteQ2 0
    $SsFood = -1
    for ($i = [Math]::Max(@($SsNearQ)[0], 0); $SsNearQ.Count -ge 1 -and $i -lt $SsE.Count; $i++) {
        if ($SsE[$i].Kind -eq "where" -and $SsE[$i].U -and $SsE[$i].U.Activity -eq "Gathering" -and $SsE[$i].U.Load -eq "Food") { $SsFood = $i; break } }
    $SsAnyFood = @(for ($i = 0; $i -lt $SsE.Count; $i++) { if ($SsE[$i].Kind -eq "where" -and $SsE[$i].U -and $SsE[$i].U.Load -eq "Food") { $i } })
    $Checks["(a) set-up: S and S2 placed, unbuilt before the orders"] =
        [bool]($SsSetup | Select-String -Pattern "placed House foundation at \($($ShiftSiteS -join ', ')\)" -Quiet) -and
        [bool]($SsSetup | Select-String -Pattern "placed House foundation at \($($ShiftSiteS2 -join ', ')\)" -Quiet) -and
        -not ($SsSetup | Select-String -Pattern "first work on House at \(($($ShiftSiteS -join ', ')|$($ShiftSiteS2 -join ', '))\)" -Quiet)
    $Checks["(a) Shift S, Shift Q2, Shift berries: it builds S, finished (first work at event $SsWork, finished at $SsDone)"] = ($SsWork -ge 0) -and ($SsDone -ge $SsWork)
    $Checks["(a) then within 3 m of Q2 only after S is finished (closest $(& $ChainClosest $SsE $ShiftSiteQ2) cm; first near at event $(@($SsNearQ)[0]))"] =
        ($SsNearQ.Count -ge 1) -and ($SsDone -ge 0) -and ($SsNearQ[0] -gt $SsDone)
    $Checks["(a) then it gathers food after Q2 (first at event $SsFood; any food at $(@($SsAnyFood)[0]))"] = ($SsFood -ge 0) -and (@($SsAnyFood)[0] -ge $SsFood)
    $SlLines = @(Get-Section "build shiftsite later" | ForEach-Object { $_ })
    $Sl = & $ReadChain $SlLines $ShiftSiteS2
    $SlE = $Sl.Events
    Write-Host "  shiftsite b (#$($Sl.Id)): $(& $ChainStory $SlE ([ordered]@{ S2 = $ShiftSiteS2; Q3 = $ShiftSiteQ3 }))"
    Write-Host "  shiftsite b log: $(@($SlLines | Select-String -Pattern "Queued|queued|applied|Build order|Build: " | ForEach-Object { ($_.Line -replace '^.*?(Construction|Queued|Move|Build|Gather|Drop)', '$1') }) -join ' / ')"
    $SlWork = & $ChainIndex $SlE "first work"
    $SlDone = & $ChainIndex $SlE "finished"
    $SlNearQ = & $ChainNear $SlE $ShiftSiteQ3 0
    $SlWheres = @($SlE | Where-Object { $_.Kind -eq "where" })
    $SlOrderLine = [array]::FindIndex([string[]]$SlLines, [Predicate[string]] { param($L) $L -match "Order: \d+ units to \(-?\d+, -?\d+\), queued" })
    $SlWorkLine = [array]::FindIndex([string[]]$SlLines, [Predicate[string]] { param($L) $L -match "first work on House at \($($ShiftSiteS2 -join ', ')\)" })
    $Checks["(b) Shift S2, 0.5 s later Shift Q3 on its way (log line $SlOrderLine, before S2's first work at line $SlWorkLine; $(if ($SlWheres.Count -ge 2) { & $MoveSay $SlWheres[1].U })): it builds S2, finished (first work at event $SlWork, finished at $SlDone)"] =
        ($SlWork -ge 0) -and ($SlDone -ge $SlWork) -and ($SlOrderLine -ge 0) -and ($SlWorkLine -gt $SlOrderLine)
    $Checks["(b) then within 3 m of Q3 only after S2 is finished (closest $(& $ChainClosest $SlE $ShiftSiteQ3) cm; first near at event $(@($SlNearQ)[0]))"] =
        ($SlNearQ.Count -ge 1) -and ($SlDone -ge 0) -and ($SlNearQ[0] -gt $SlDone)
    $Checks["(b) then idle within 3 m of Q3 ($(if ($SlWheres.Count -ge 1) { & $MoveSay $SlWheres[-1].U }), Q3 $(if ($SlWheres.Count -ge 1) { & $DropDist $SlWheres[-1].U $ShiftSiteQ3 }) cm)"] =
        ($SlWheres.Count -ge 3) -and $SlWheres[-1].U -and $SlWheres[-1].U.Activity -eq "Idle" -and (& $DropDist $SlWheres[-1].U $ShiftSiteQ3) -le 300
    $AllOk = (Report "build shiftsite" $Checks) -and $AllOk

    # N.1: a rally point in several steps. The markers' report: the flag's foot, the order lines and points drawn.
    $RsMarkers = { param($Lines) $M = @($Lines | Select-String -Pattern "Selection markers: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }); if ($M.Count) { $M[-1] } else { "" } }
    $RsFlagAt = { param($Say, $At) ($Say -match "flag 1 at \((-?\d+), (-?\d+)\)") -and ([Math]::Abs([int]$Matches[1] - $At[0]) -le 60) -and ([Math]::Abs([int]$Matches[2] - $At[1]) -le 60) }
    $RsLines = { param($Say) if ($Say -match "order lines (\d+), order points (\d+)") { @([int]$Matches[1], [int]$Matches[2]) } else { @(-1, -1) } }
    $Checks = [ordered]@{}
    $RsA = @(Get-Section "build rallysteps" | ForEach-Object { $_ })
    $RsAE = (& $ReadChain $RsA $RallyStepsR2).Events
    $RsAW = @($RsAE | Where-Object { $_.Kind -eq "where" })
    Write-Host "  rallysteps a: $(& $ChainStory $RsAE ([ordered]@{ R1 = $RallyStepsR1; R2 = $RallyStepsR2 }))"
    Write-Host "  rallysteps a log: $(@($RsA | Select-String -Pattern "Rally: |Queued|queued|Order chain" | ForEach-Object { ($_.Line -replace '^.*?(Rally|Queued|Move|Order chain)', '$1') }) -join ' / ')"
    $RsASay = & $RsMarkers $RsA
    $RsALines = & $RsLines $RsASay
    $Checks["(a) right click R1, Shift + right click R2: the flag at R2, two segments and one step point ($RsASay)"] =
        (& $RsFlagAt $RsASay $RallyStepsR2) -and ($RsALines[0] -eq 2) -and ($RsALines[1] -eq 1)
    $RsANear1 = & $ChainNear $RsAE $RallyStepsR1 0
    $RsANear2 = & $ChainNear $RsAE $RallyStepsR2 0
    $Checks["(a) the Peasant produced passes within 3 m of R1 (closest $(& $ChainClosest $RsAE $RallyStepsR1) cm), before R2 (first near R1 at $(@($RsANear1)[0]), R2 at $(@($RsANear2)[0]))"] =
        ($RsANear1.Count -ge 1) -and ($RsANear2.Count -ge 1) -and ($RsANear1[0] -lt $RsANear2[0])
    $Checks["(a) then idle within 3 m of R2 ($(if ($RsAW.Count) { & $MoveSay $RsAW[-1].U }), R2 $(if ($RsAW.Count) { & $DropDist $RsAW[-1].U $RallyStepsR2 }) cm)"] =
        ($RsAW.Count -ge 3) -and $RsAW[-1].U -and $RsAW[-1].U.Activity -eq "Idle" -and (& $DropDist $RsAW[-1].U $RallyStepsR2) -le 300
    $RsAChains = Get-ChainReports $RsA
    $RsAChain = if ($RsAChains.Count -ge 1) { @($RsAChains[0].Chains | ForEach-Object { $_.Steps }) -join " | " } else { "none" }
    $RsAChainOk = $false
    if ($RsAChains.Count -ge 1) { foreach ($C in $RsAChains[0].Chains) {
        if ($C.Steps -match "^Rally at \((-?\d+), (-?\d+)\) -> Rally at \((-?\d+), (-?\d+)\)$" -and [Math]::Abs([int]$Matches[1] - $RallyStepsR1[0]) -le 300 -and [Math]::Abs([int]$Matches[2] - $RallyStepsR1[1]) -le 300 -and
            [Math]::Abs([int]$Matches[3] - $RallyStepsR2[0]) -le 300 -and [Math]::Abs([int]$Matches[4] - $RallyStepsR2[1]) -le 300) { $RsAChainOk = $true } } }
    $Checks["(a) its chain: two white Rally steps, R1 then R2 ($RsAChain)"] = $RsAChainOk

    $RsB = @(Get-Section "build rallysteps source" | ForEach-Object { $_ })
    $RsBE = (& $ReadChain $RsB $RallyStepsR2).Events
    Write-Host "  rallysteps b: $(& $ChainStory $RsBE ([ordered]@{ R1 = $RallyStepsR1; Gold = $RallyStepsGold }))"
    Write-Host "  rallysteps b log: $(@($RsB | Select-String -Pattern "Rally: |Queued|queued|Gather order" | ForEach-Object { ($_.Line -replace '^.*?(Rally|Queued|Move|Gather)', '$1') }) -join ' / ')"
    $RsBSay = & $RsMarkers $RsB
    $RsBLines = & $RsLines $RsBSay
    $Checks["(b) right click R1, Shift + right click on the gold: the flag on the vein, two segments ($RsBSay)"] =
        (& $RsFlagAt $RsBSay $RallyStepsGold) -and ($RsBLines[0] -eq 2) -and ($RsBLines[1] -eq 1)
    $RsBNear1 = & $ChainNear $RsBE $RallyStepsR1 0
    $RsBGold = -1
    for ($i = 0; $i -lt $RsBE.Count; $i++) { if ($RsBE[$i].Kind -eq "where" -and $RsBE[$i].U -and $RsBE[$i].U.Load -eq "Gold") { $RsBGold = $i; break } }
    $Checks["(b) the Peasant produced passes within 3 m of R1 (closest $(& $ChainClosest $RsBE $RallyStepsR1) cm; first near at $(@($RsBNear1)[0])), then mines gold (first carried at $RsBGold)"] =
        ($RsBNear1.Count -ge 1) -and ($RsBGold -gt $RsBNear1[0])

    $RsC = @(Get-Section "build rallysteps replace" | ForEach-Object { $_ })
    $RsCSay = & $RsMarkers $RsC
    $RsCLines = & $RsLines $RsCSay
    $Checks["(c) a plain right click at R3 after the steps: R3 alone, the flag there, one segment ($RsCSay)"] =
        (& $RsFlagAt $RsCSay $RallyStepsR3) -and ($RsCLines[0] -eq 1) -and ($RsCLines[1] -eq 0)

    $RsD = @(Get-Section "build rallysteps nine" | ForEach-Object { $_ })
    $RsDSay = & $RsMarkers $RsD
    $RsDLines = & $RsLines $RsDSay
    $RsDRefused = @($RsD | Select-String -Pattern "Rally: slot 0 cannot add a step to the rally point of MainCentre \(8 steps at most\)").Count
    $Checks["(d) a right click then 8 Shift + right clicks: the 9th refused (refusals $RsDRefused), the flag at the 8th, 8 segments ($RsDSay)"] =
        ($RsDRefused -eq 1) -and (& $RsFlagAt $RsDSay $RallyStepsRow[7]) -and ($RsDLines[0] -eq 8) -and ($RsDLines[1] -eq 7)
    $AllOk = (Report "build rallysteps" $Checks) -and $AllOk
    # EtapeDefini C.1, the corner: X's centre from its "finished" line, the four Men-at-arms from the last "Units where" of
    # the section (slot 0's units within 8 m of X). In X's frame, each body's signed distance to the drawn slab (half 370 =
    # 400 - VisualInset 30, square corners) less its radius (60): its margin. A: every body out of the slab (margin >= -10;
    # the still-unit rule measured from the body with a round corner leaves ~30 cm of the body in the slab's corner).
    # B: the case stands: four units, all idle, each within 250 cm of a different corner of the slab.
    $Corner = @(Get-Section "build corner" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $CornerAt = [regex]::Match("$(& $Value $Corner "(Construction: slot 0 finished House at \(-?\d+, -?\d+\))")", "\((-?\d+), (-?\d+)\)")
    $CornerX = if ($CornerAt.Success) { [double]$CornerAt.Groups[1].Value } else { $CornerHouse[0] }
    $CornerY = if ($CornerAt.Success) { [double]$CornerAt.Groups[2].Value } else { $CornerHouse[1] }
    $CornerWhere = "$(& $Value $Corner "Units where: (.*)$")"
    $CornerUnits = @(foreach ($Part in (($CornerWhere -replace '^\d+ units: ', '') -split '; ')) {
        if ($Part -match '^#(\d+) slot 0 at \((-?\d+), (-?\d+)\) (\w+)') {
            $Ux = [double]$Matches[2] - $CornerX; $Uy = [double]$Matches[3] - $CornerY
            if ([Math]::Sqrt($Ux * $Ux + $Uy * $Uy) -le 800) {
                $Dx = [Math]::Abs($Ux) - 370; $Dy = [Math]::Abs($Uy) - 370
                $Signed = if ($Dx -gt 0 -or $Dy -gt 0) { [Math]::Sqrt([Math]::Pow([Math]::Max($Dx, 0), 2) + [Math]::Pow([Math]::Max($Dy, 0), 2)) } else { [Math]::Max($Dx, $Dy) }
                [pscustomobject]@{ Id = [int]$Matches[1]; Activity = $Matches[4]; Corner = "$(if ($Ux -lt 0) { '-x' } else { '+x' })$(if ($Uy -lt 0) { '-y' } else { '+y' })"
                    Margin = [Math]::Round($Signed - 60); ToCorner = [Math]::Round([Math]::Sqrt($Dx * $Dx + $Dy * $Dy)) }
            }
        }
    })
    $CornerSay = @($CornerUnits | ForEach-Object { "$($_.Corner) $($_.Margin)" }) -join ', '
    $Checks["every body out of the visible wall's corner (margin >= -10 cm; corner margin: $CornerSay)"] =
        $CornerAt.Success -and ($CornerUnits.Count -eq 4) -and (@($CornerUnits | Where-Object { $_.Margin -lt -10 }).Count -eq 0)
    $Checks["four Men-at-arms idle, each within 250 cm of a different corner of the slab (house at ($CornerX, $CornerY); $(@($CornerUnits | ForEach-Object { "$($_.Corner) $($_.Activity) $($_.ToCorner) cm" }) -join ', '))"] =
        $CornerAt.Success -and ($CornerUnits.Count -eq 4) -and (@($CornerUnits | Where-Object { $_.Activity -eq 'Idle' }).Count -eq 4) -and
        (@($CornerUnits | Select-Object -ExpandProperty Corner -Unique).Count -eq 4) -and (@($CornerUnits | Where-Object { $_.ToCorner -gt 250 }).Count -eq 0)
    $Checks["the four removed, the house destroyed after"] = [bool]($Corner | Select-String -Pattern "Remove: 4 units of slot 0 within 1500" -Quiet) -and
        [bool]($Corner | Select-String -SimpleMatch "Building destroyed: House of slot 0 at ($CornerX, $CornerY)" -Quiet)
    $AllOk = (Report "build corner" $Checks) -and $AllOk
    # EtapeDefini T, the bottom bar's width: one "HUD layout" line per window size (plus the one back at 1600x900), the bar
    # (its right edge) left of the minimap's place at each (gap >= 0); the bar at its full size (scale 1) from 1920x1080
    # and at 1600x900 (the look validated at G.1 kept).
    $HudWidth = @(Get-Section "build barwidth" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $HudLayouts = @(foreach ($Line in $HudWidth) {
        if ($Line -match "HUD layout: viewport (\d+x\d+), dpi ([\d.]+), width (\d+), bar (\d+) shown (\d+) \(scale ([\d.]+)\) right (\d+), minimap (\d+) left (\d+), gap (-?\d+)") {
            [pscustomobject]@{ Size = $Matches[1]; Width = [int]$Matches[3]; Bar = [int]$Matches[4]; Shown = [int]$Matches[5]; Scale = [double]$Matches[6]; Gap = [int]$Matches[10] }
        }
    })
    $HudSay = @($HudLayouts | ForEach-Object { "$($_.Size): width $($_.Width) bar $($_.Bar) shown $($_.Shown) gap $($_.Gap)" }) -join ', '
    $Checks["a layout line at each of the $($HudSizes.Count) sizes and back at 1600x900 ($HudSay)"] =
        ($HudLayouts.Count -eq $HudSizes.Count + 1) -and (@(Compare-Object ($HudSizes + "1600x900") @($HudLayouts | ForEach-Object { $_.Size }) -SyncWindow 0).Count -eq 0)
    $Checks["the bar left of the minimap's place at every size (gap >= 0)"] = ($HudLayouts.Count -gt 0) -and (@($HudLayouts | Where-Object { $_.Gap -lt 0 }).Count -eq 0)
    $Checks["the bar at its full size at 1920x1080 and 1600x900 (scale 1)"] =
        (@($HudLayouts | Where-Object { $_.Size -in @("1920x1080", "1600x900") }).Count -eq 2) -and
        (@($HudLayouts | Where-Object { $_.Size -in @("1920x1080", "1600x900") -and ($_.Scale -lt 0.995 -or $_.Shown -ne $_.Bar) }).Count -eq 0)
    $AllOk = (Report "build barwidth" $Checks) -and $AllOk
}

if (($Parts -contains "anims") -and ($Parts -contains "camera")) {
    # What each state calls for (J5c.2, J6a.4; HOLUnitAnimation::Choose and the Peasant's row of the unit types): the
    # sequence and the prop (1 axe, 2 pickaxe, 3 basket of berries, 4 logs, 5 hotte of stone, 6 hotte of gold, 7 hammer).
    $GatherShown = @{ Food = "A_Peasant_GatherBerries prop 0"; Wood = "A_Peasant_ChopWood prop 1"; Stone = "A_Peasant_MinePick prop 2"; Gold = "A_Peasant_MinePick prop 2" }
    $CarryWalkShown = @{ Food = "A_Peasant_CarryFood prop 3"; Wood = "A_Peasant_CarryWood prop 4"; Stone = "A_Peasant_CarryStone prop 5"; Gold = "A_Peasant_CarryStone prop 6" }
    $CarryIdleShown = @{ Food = "A_Peasant_IdleCarryFood prop 3"; Wood = "A_Peasant_IdleCarryWood prop 4"; Stone = "A_Peasant_IdleCarryStone prop 5"; Gold = "A_Peasant_IdleCarryStone prop 6" }
    $IdleShown = "A_SW_Idle prop 0"
    $WalkShown = "A_SW_Walk_F prop 0"
    $BuildShown = "A_Peasant_Build prop 7"
    # Gathering or building: its gesture and tool at any speed; else, with a load, the carrying walk or the loaded idle of
    # the load (by its speed: a unit starting, stopping or pushed may show either), the load shown; else the walk or the idle.
    $Allowed = { param([string] $State)
        if ($State -match '^Peasant Gathering (\w+)') { return @($GatherShown[$Matches[1]]) }
        if ($State -match '^Peasant Building') { return @($BuildShown) }
        if ($State -match '^(Spearman|ManAtArms|Archer) ') { return @("A_$($Matches[1])_Idle prop 0", "A_$($Matches[1])_Walk prop 0") }
        if ($State -match ' carrying (\w+)$') { return @($CarryWalkShown[$Matches[1]], $CarryIdleShown[$Matches[1]]) }
        return @($IdleShown, $WalkShown) }
    # The last report of a section: Peasants drawn, and state -> (shown -> count).
    $ReadAnims = { param($Lines)
        $Head = $Lines | Select-String -Pattern "Anim report: slot 0, (\d+) drawn, (\d+) not drawn" | Select-Object -Last 1
        $Line = $Lines | Select-String -Pattern "Anim report by state: (.*)$" | Select-Object -Last 1
        $States = [ordered]@{}
        if ($Line) {
            foreach ($Part in ($Line.Matches[0].Groups[1].Value -split '; ')) {
                if ($Part -match '^(.+?): (.+)$') {
                    $Name = $Matches[1]; $Shown = @{}
                    foreach ($Outcome in ($Matches[2] -split ', ')) { if ($Outcome -match '^(.+) x(\d+)$') { $Shown[$Matches[1]] = [int]$Matches[2] } }
                    $States[$Name] = $Shown
                }
            }
        }
        @{ Drawn = $(if ($Head) { [int]$Head.Matches[0].Groups[1].Value } else { 0 }); NotDrawn = $(if ($Head) { [int]$Head.Matches[0].Groups[2].Value } else { -1 });
            States = $States; Text = $(if ($Line) { $Line.Matches[0].Groups[1].Value } else { "no report" }) } }
    # How many Peasants in the states matching a pattern show one thing.
    $CountShown = { param($Report, [string] $StatePattern, [string] $Shown)
        $Count = 0; foreach ($State in $Report.States.Keys) { if ($State -match $StatePattern) { $Count += [int]$Report.States[$State][$Shown] } }; $Count }
    $Checks = [ordered]@{}
    $Reports = [ordered]@{ working = (& $ReadAnims (Get-Section "anims working")); walking = (& $ReadAnims (Get-Section "anims walking")); standing = (& $ReadAnims (Get-Section "anims standing")) }
    foreach ($Moment in $Reports.Keys) {
        $Report = $Reports[$Moment]
        $Wrong = @(foreach ($State in $Report.States.Keys) { $Ok = & $Allowed $State
            foreach ($Shown in $Report.States[$State].Keys) { if ($Shown -notin $Ok) { "$State shows $Shown x$($Report.States[$State][$Shown])" } } })
        $Checks["$Moment`: every Peasant drawn shows what its state calls for ($($Report.Drawn) drawn, $($Report.NotDrawn) not; $($Wrong -join '; '))"] =
            ($Report.Drawn -ge 17) -and ($Wrong.Count -eq 0)
    }
    $Working = $Reports["working"]
    foreach ($Resource in "Wood", "Stone", "Gold", "Food") {
        $Count = & $CountShown $Working "^Peasant Gathering $Resource" $GatherShown[$Resource]
        $Checks["gathering $Resource`: $($GatherShown[$Resource]) ($Count of 3)"] = $Count -ge 2
    }
    $Count = & $CountShown $Working "^Peasant Building" $BuildShown
    $Checks["building: $BuildShown ($Count of 2)"] = $Count -ge 2
    $Count = & $CountShown $Working "^Peasant Idle$" $IdleShown
    $Checks["idle, nothing carried: $IdleShown ($Count, 3 at least)"] = $Count -ge 3
    foreach ($Resource in "Wood", "Stone", "Gold", "Food") {
        $Count = & $CountShown $Reports["walking"] "^Peasant Moving carrying $Resource$" $CarryWalkShown[$Resource]
        $Checks["walking with $Resource`: $($CarryWalkShown[$Resource]) ($Count of 3)"] = $Count -ge 2
        $Count = & $CountShown $Reports["standing"] "^Peasant Idle carrying $Resource$" $CarryIdleShown[$Resource]
        $Checks["standing with $Resource`: $($CarryIdleShown[$Resource]) ($Count of 3)"] = $Count -ge 2
    }
    # The soldiers (J7a.3): their armed idle at rest, their armed walk on the way, at rest again once there.
    foreach ($Soldier in "Spearman", "ManAtArms") {
        foreach ($Moment in @(@("working", "Idle", "Idle"), @("walking", "Moving", "Walk"), @("standing", "Idle", "Idle"))) {
            $Shown = "A_$Soldier`_$($Moment[2]) prop 0"
            $Count = & $CountShown $Reports[$Moment[0]] "^$Soldier $($Moment[1])$" $Shown
            $Checks["$($Moment[0]) $Soldier`: $Shown ($Count of 3)"] = $Count -ge 2
        }
    }
    $Missing = @("work_wood", "work_stone", "work_gold", "work_berries", "work_build", "work_idle", "soldiers_idle", "soldiers_walk", "walk_berries", "stand_wood", "stand_stone", "stand_gold", "stand_berries" |
        Where-Object { -not (Test-Path (Join-Path $Shots "anims_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "anims" $Checks) -and $AllOk
    foreach ($Moment in $Reports.Keys) { Write-Host "  $Moment`: $($Reports[$Moment].Text)" }
}

if (($Parts -contains "formation") -and ($Parts -contains "camera")) {
    # Formed up in place inside a mixed army (the user's trial of select_type_q, "elles peinent"): within 6 s the 6 of
    # the type picked set down (phase None) at most 60 cm from their places; each one's time, and how far the others moved.
    $Checks = [ordered]@{}
    $ParseFormation = {
        param($Line)
        $Units = @{}
        foreach ($M in [regex]::Matches("$Line", "#(\d+) (\w+) at \((-?\d+), (-?\d+)\) .*? place \((-?\d+), (-?\d+)\) off (\d+) phase (\w+)")) {
            $Units[[int]$M.Groups[1].Value] = @{ Type = $M.Groups[2].Value; X = [double]$M.Groups[3].Value; Y = [double]$M.Groups[4].Value; Off = [int]$M.Groups[7].Value; Phase = $M.Groups[8].Value }
        }
        $Units
    }
    foreach ($Regroup in $RegroupCases) {
        $RegroupLines = @(@(Get-Section "formation $($Regroup.Name)") + @(Get-Section "formation $($Regroup.Name) end") | Where-Object { $_ -match "Formation report: " })
        $Reports = @($RegroupLines | ForEach-Object { & $ParseFormation $_ })
        $Ids = if ($Reports.Count) { @($Reports[0].Keys | Where-Object { [Math]::Abs($Reports[0][$_].X - $RegroupAt[0]) -lt 1500 -and [Math]::Abs($Reports[0][$_].Y - $RegroupAt[1]) -lt 1500 } | Sort-Object) } else { @() }
        $Last = if ($Reports.Count) { $Reports[-1] } else { @{} }
        $Sent = @($Ids | Where-Object { $Reports[0][$_].Type -eq $Regroup.Type })
        $Others = @($Ids | Where-Object { $Sent -notcontains $_ })
        # Each one's time: the first report it is set down within 60 cm (-1: never).
        $Times = @($Sent | ForEach-Object { $Id = $_; $T = -1
            for ($r = 1; $r -lt $Reports.Count; $r++) { if ($Reports[$r][$Id] -and $Reports[$r][$Id].Phase -eq "None" -and $Reports[$r][$Id].Off -le 60) { $T = $r * 0.5; break } }
            $T })
        $MaxTime = if (@($Times | Where-Object { $_ -lt 0 }).Count -or -not $Times.Count) { -1 } else { ($Times | Measure-Object -Maximum).Maximum }
        $At6 = if ($Reports.Count -gt 12) { $Reports[12] } else { @{} }
        $Walking = @($Sent | Where-Object { -not $At6[$_] -or $At6[$_].Phase -ne "None" -or $At6[$_].Off -gt 60 })
        $SentText = (@(0..($Sent.Count - 1)) | Where-Object { $Sent.Count } | ForEach-Object { "#$($Sent[$_]) $($Times[$_]) s ($($Last[$Sent[$_]].Phase) $($Last[$Sent[$_]].Off))" }) -join ", "
        $Moved = @($Others | ForEach-Object { if ($Last[$_]) { [Math]::Round([Math]::Sqrt([Math]::Pow($Last[$_].X - $Reports[0][$_].X, 2) + [Math]::Pow($Last[$_].Y - $Reports[0][$_].Y, 2))) } })
        $MovedMax = if ($Moved.Count) { ($Moved | Measure-Object -Maximum).Maximum } else { 0 }
        $Text = "$($Regroup.Name) ($($Regroup.Type)): $($Ids.Count) units ($($Sent.Count) sent), all set within 60 cm at $(if ($MaxTime -ge 0) { "$MaxTime s" } else { 'never (8 s)' }), $($Walking.Count) still off at 6 s ($SentText); the others moved up to $MovedMax cm ($($Moved -join ' '))"
        Write-Host "  $Text"
        # Set down there and kept: none off its place at 6 s nor at the end (not set down then pushed off, nor set short).
        $EndOff = @($Sent | Where-Object { -not $Last[$_] -or $Last[$_].Phase -ne "None" -or $Last[$_].Off -gt 60 })
        $Checks[$Text] = $Ids.Count -eq 16 -and $Sent.Count -eq 6 -and $MaxTime -ge 0 -and $MaxTime -le 6 -and $Walking.Count -eq 0 -and $EndOff.Count -eq 0
    }
    $AllOk = (Report "formation regroup" $Checks) -and $AllOk
}

if (($Parts -contains "formation") -and ($Parts -contains "camera")) {
    # Reinforcements onto friends at rest (MV.4.6): within 12 s every one sent set down (phase None) at most 60 cm from its
    # place, none left on its final leg more than 1 m off (before: places on the friends, stuck ~3 m off).
    $Checks = [ordered]@{}
    $OrdersLine = @(Get-Section "reinforce") | Where-Object { $_ -match "Unit orders: " } | Select-Object -Last 1
    $Sent = @()
    if ($OrdersLine) {
        $Sent = @([regex]::Matches($OrdersLine, "#(\d+) EHOLMovePhase::(\w+) slot \(-?\d+, -?\d+\) at (\d+) cm") | ForEach-Object { @{ Id = [int]$_.Groups[1].Value; Phase = $_.Groups[2].Value; Off = [int]$_.Groups[3].Value } } |
            Sort-Object { $_.Id } | Select-Object -Last 6)
    }
    $Set = @($Sent | Where-Object { $_.Phase -eq "None" -and $_.Off -le 60 })
    $Stuck = @($Sent | Where-Object { $_.Phase -eq "FinalLeg" -and $_.Off -gt 100 })
    $Checks["reinforcements: $($Set.Count)/6 set down within 60 cm of their places at 12 s, $($Stuck.Count) on their final leg > 1 m off ($(($Sent | ForEach-Object { "$($_.Phase) $($_.Off)" }) -join ', '))"] = $Sent.Count -eq 6 -and $Set.Count -eq 6 -and $Stuck.Count -eq 0
    $AllOk = (Report "formation reinforce" $Checks) -and $AllOk
}

# The formation's other reports (groups, stragglers, blocks meet, backsteps) are not cases: not read when cases are asked.
if (($Parts -contains "formation") -and ($Parts -contains "camera") -and -not $SubCases.ContainsKey("formation")) {
    # The formation (J7a.4), from hol.Units.Formation: id -> type, location, yaw, speed (velocity), own speed, march
    # speed, front yaw, facing the front, holding its place, distance to it; and the place returns so far.
    $ReadFormation = { param($Lines)
        $Line = $Lines | Select-String -Pattern "Formation report: returns (\d+); (.*)$" | Select-Object -Last 1
        $Units = @{}; $Returns = -1
        if ($Line) {
            $Returns = [int]$Line.Matches[0].Groups[1].Value
            foreach ($M in [regex]::Matches($Line.Matches[0].Groups[2].Value, "#(\d+) (\w+) at \((-?\d+), (-?\d+)\) yaw (-?[\d.]+) speed (-?\d+) own (\d+) march (\d+) front (-?[\d.]+) faces (\d) held (\d) returning (\d) place \((-?\d+), (-?\d+)\) off (\d+) phase (\w+)")) {
                $G = $M.Groups
                $Units[[int]$G[1].Value] = @{ Type = $G[2].Value; X = [double]$G[3].Value; Y = [double]$G[4].Value; Yaw = [double]$G[5].Value; Speed = [double]$G[6].Value;
                    Own = [double]$G[7].Value; March = [double]$G[8].Value; Front = [double]$G[9].Value; Faces = $G[10].Value -eq "1"; Held = $G[11].Value -eq "1";
                    Off = [double]$G[15].Value; Phase = $G[16].Value }
            }
        }
        @{ Returns = $Returns; Units = $Units } }
    $AllReports = { param($Lines) @($Lines | Select-String -Pattern "Formation report: " | ForEach-Object { & $ReadFormation @($_.Line) }) }
    $YawGap = { param([double] $A, [double] $B) [Math]::Abs(((($A - $B) % 360) + 540) % 360 - 180) }
    $Checks = [ordered]@{}
    $Start = @(& $AllReports (Get-Section "formation"))
    $Ids = @()
    if ($Start.Count -ge 2) { $Ids = @($Start[1].Units.Keys | Where-Object { -not $Start[0].Units.ContainsKey($_) } | Sort-Object) }
    $Checks["six units made ($($Ids.Count))"] = $Ids.Count -eq 6
    if ($Ids.Count -eq 6) {
        $Begin = $Start[1].Units
        $Slowest = ($Ids | ForEach-Object { $Begin[$_].Own } | Measure-Object -Minimum).Minimum
        # 1. One pace: each order's march speed is the slowest's; no walker more than 10 % above it, the Peasants
        # (600 cm/s alone) at it (median of their walking samples within 15 %).
        $March = @(& $AllReports (Get-Section "formation march"))
        $Marches = @(foreach ($R in $March) { foreach ($Id in $Ids) { if ($R.Units.ContainsKey($Id) -and $R.Units[$Id].March -gt 0) { $R.Units[$Id].March } } })
        $WrongMarch = @($Marches | Where-Object { [Math]::Abs($_ - $Slowest) -gt 1 })
        $Checks["march speed of the six orders = the slowest's ($Slowest cm/s; $($Marches.Count) samples, $($WrongMarch.Count) other)"] = ($Marches.Count -ge 12) -and ($WrongMarch.Count -eq 0)
        $Walking = @(foreach ($R in $March) { foreach ($Id in $Ids) { if ($R.Units.ContainsKey($Id) -and $R.Units[$Id].Speed -gt 100 -and $R.Units[$Id].March -gt 0 -and $R.Units[$Id].Phase -ne "FormUp") { , @($R.Units[$Id].Type, $R.Units[$Id].Speed) } } })
        $Fastest = ($Walking | ForEach-Object { $_[1] } | Measure-Object -Maximum).Maximum
        $PeasantSpeeds = @($Walking | Where-Object { $_[0] -eq "Peasant" } | ForEach-Object { $_[1] } | Sort-Object)
        $Median = if ($PeasantSpeeds.Count -gt 0) { $PeasantSpeeds[[int][Math]::Floor($PeasantSpeeds.Count / 2)] } else { 0 }
        $Checks["no walker in the march faster than the slowest + 10 % (fastest $Fastest cm/s of $($Walking.Count) samples)"] = ($Walking.Count -gt 0) -and ($Fastest -le $Slowest * 1.1)
        $Checks["the Peasants at the slowest's pace (median $Median cm/s of $($PeasantSpeeds.Count) samples)"] = ($PeasantSpeeds.Count -ge 6) -and ([Math]::Abs($Median - $Slowest) -le $Slowest * 0.15)
        # 2. At rest: on their places (60 cm), no overlap, rows across the march, the soldiers in front, facing the march.
        $End = (& $ReadFormation (Get-Section "formation arrived")).Units
        $Cx = ($Ids | ForEach-Object { $Begin[$_].X } | Measure-Object -Average).Average; $Cy = ($Ids | ForEach-Object { $Begin[$_].Y } | Measure-Object -Average).Average
        $Fx = $FormTo[0] - $Cx; $Fy = $FormTo[1] - $Cy; $Fl = [Math]::Sqrt($Fx * $Fx + $Fy * $Fy); $Fx /= $Fl; $Fy /= $Fl
        $Ahead = @{}; foreach ($Id in $Ids) { if ($End.ContainsKey($Id)) { $Ahead[$Id] = ($End[$Id].X - $FormTo[0]) * $Fx + ($End[$Id].Y - $FormTo[1]) * $Fy } }
        $Offs = @($Ids | ForEach-Object { if ($End.ContainsKey($_)) { $End[$_].Off } else { 1e9 } })
        $Checks["arrived on their places ($($Offs -join ', ') cm, 60 at most)"] = @($Offs | Where-Object { $_ -le 60 }).Count -eq 6
        $Closest = 1e9
        foreach ($A in $Ids) { foreach ($B in $Ids) { if ($A -lt $B -and $End.ContainsKey($A) -and $End.ContainsKey($B)) {
            $Closest = [Math]::Min($Closest, [Math]::Sqrt([Math]::Pow($End[$A].X - $End[$B].X, 2) + [Math]::Pow($End[$A].Y - $End[$B].Y, 2))) } } }
        $Checks["no overlap ($([Math]::Round($Closest)) cm closest, bodies 120)"] = $Closest -ge 110
        $Soldiers = @($Ids | Where-Object { $Begin[$_].Type -ne "Peasant" }); $Peasants = @($Ids | Where-Object { $Begin[$_].Type -eq "Peasant" })
        if ($Ahead.Count -eq 6) {
            $SoldierBack = ($Soldiers | ForEach-Object { $Ahead[$_] } | Measure-Object -Minimum).Minimum
            $PeasantFront = ($Peasants | ForEach-Object { $Ahead[$_] } | Measure-Object -Maximum).Maximum
            $Checks["the soldiers a row in front of the Peasants (last soldier $([Math]::Round($SoldierBack)), first Peasant $([Math]::Round($PeasantFront)) cm along the march)"] =
                ($SoldierBack - $PeasantFront -ge 120)
            $Spreads = @(foreach ($Kind in @(, $Soldiers) + @(, $Peasants)) { $V = @($Kind | ForEach-Object { $Ahead[$_] }); [Math]::Round(($V | Measure-Object -Maximum).Maximum - ($V | Measure-Object -Minimum).Minimum) })
            $Checks["in rows across the march (spread along it: soldiers $($Spreads[0]), Peasants $($Spreads[1]) cm, 60 at most)"] = @($Spreads | Where-Object { $_ -le 60 }).Count -eq 2
        } else { $Checks["the six reported at rest ($($Ahead.Count))"] = $false }
        $MarchYaw = [Math]::Atan2($Fy, $Fx) * 180 / [Math]::PI
        $Yaws = @($Ids | ForEach-Object { if ($End.ContainsKey($_)) { [Math]::Round((& $YawGap $End[$_].Yaw $MarchYaw)) } else { 999 } })
        $Checks["facing the march at rest (off by $($Yaws -join ', ') degrees, 10 at most)"] = @($Yaws | Where-Object { $_ -le 10 }).Count -eq 6
        # 3. The drag, an arrow (the user's 2nd trial): its ground base and end (the order's log); a front of
        # floor(length / 150) + 1 columns centred on its base, facing along it; the others a row (180 cm) behind.
        $FrontLines = Get-Section "formation front"
        $LineOrder = $FrontLines | Select-String -Pattern "Order: 6 units in line from \((-?\d+), (-?\d+)\) to \((-?\d+), (-?\d+)\)" | Select-Object -First 1
        $Checks["the drag ordered the six in line ($(if ($LineOrder) { $LineOrder.Matches[0].Value } else { 'no order' }))"] = [bool]$LineOrder
        if ($LineOrder -and $End.Count -ge 6) {
            $G = $LineOrder.Matches[0].Groups
            $Ax = [double]$G[1].Value; $Ay = [double]$G[2].Value; $Bx = [double]$G[3].Value; $By = [double]$G[4].Value
            $Dx = $Bx - $Ax; $Dy = $By - $Ay; $Length = [Math]::Sqrt($Dx * $Dx + $Dy * $Dy); $Dx /= $Length; $Dy /= $Length
            $Width = [Math]::Min(6, [Math]::Max(1, [Math]::Floor($Length / 150) + 1))
            $Mx = $Ax; $My = $Ay
            $Nx = $Dx; $Ny = $Dy; $Dx = $Ny; $Dy = -$Nx
            $FrontYaw = [Math]::Atan2($Ny, $Nx) * 180 / [Math]::PI
            $Front = (& $ReadFormation $FrontLines).Units
            $Behind = @{}; $Along = @{}
            foreach ($Id in $Ids) { if ($Front.ContainsKey($Id)) { $Behind[$Id] = ($Mx - $Front[$Id].X) * $Nx + ($My - $Front[$Id].Y) * $Ny; $Along[$Id] = ($Front[$Id].X - $Mx) * $Dx + ($Front[$Id].Y - $My) * $Dy } }
            $FirstRow = @($Behind.Keys | Where-Object { [Math]::Abs($Behind[$_]) -le 60 })
            $Checks["front row at the arrow's base: $($FirstRow.Count) units, $Width columns wanted (length $([Math]::Round($Length)) cm)"] = $FirstRow.Count -eq $Width
            $Spans = @($FirstRow | ForEach-Object { [Math]::Round($Along[$_]) } | Sort-Object)
            $Checks["front row across the arrow, centred on its base, within its length / 2 + 75 cm (at $($Spans -join ', ') cm of the base)"] = ($FirstRow.Count -gt 0) -and (@($Spans | Where-Object { [Math]::Abs($_) -le $Length / 2 + 75 }).Count -eq $FirstRow.Count)
            $Checks["the front row is the soldiers ($(@($FirstRow | ForEach-Object { $Begin[$_].Type }) -join ', '))"] = ($FirstRow.Count -gt 0) -and (@($FirstRow | Where-Object { $Begin[$_].Type -eq "Peasant" }).Count -eq 0)
            $Rest = @($Behind.Keys | Where-Object { [Math]::Abs($Behind[$_]) -gt 60 } | ForEach-Object { [Math]::Round($Behind[$_]) })
            $Checks["the others in rows behind it ($($Rest -join ', ') cm, every 180 +- 60)"] = ($Rest.Count -eq 6 - $Width) -and
                (@($Rest | Where-Object { ([Math]::Round($_ / 180) -ge 1) -and ([Math]::Abs($_ - 180 * [Math]::Round($_ / 180)) -le 60) }).Count -eq $Rest.Count)
            $Fronts = @($Ids | ForEach-Object { if ($Front.ContainsKey($_)) { [Math]::Round((& $YawGap $Front[$_].Front $FrontYaw)) } else { 999 } })
            $Checks["front yaw of the orders along the arrow ($([Math]::Round($FrontYaw)) degrees; off by $($Fronts -join ', '), 2 at most)"] = @($Fronts | Where-Object { $_ -le 2 }).Count -eq 6
            $Yaws = @($Ids | ForEach-Object { if ($Front.ContainsKey($_) -and $Front[$_].Speed -lt 20) { [Math]::Round((& $YawGap $Front[$_].Yaw $FrontYaw)) } else { 999 } })
            $Checks["facing the front at rest (off by $($Yaws -join ', ') degrees, 10 at most)"] = @($Yaws | Where-Object { $_ -le 10 }).Count -eq 6
            # 4. Back to their places once the walker has gone through.
            $Pushed = @(& $AllReports (Get-Section "formation push")); $Before = $Pushed[0]
            $PushedOff = ($Pushed | ForEach-Object { $R = $_; $Ids | ForEach-Object { if ($R.Units.ContainsKey($_)) { $R.Units[$_].Off } } } | Measure-Object -Maximum).Maximum
            $After = & $ReadFormation (Get-Section "formation back")
            # The walker pushes them less than a radius (they hold their places); the return is seen at the march's end
            # instead: the soldiers arriving push a Peasant off its place, it walks back (its own pace, not the march's).
            $Returns = (& $ReadFormation (Get-Section "formation arrived")).Returns
            # (The block march, J7a.4: the soldiers no longer push a Peasant on arrival; walking back is the test Formation.Return's.)
            Write-Host "  formation: place returns on arrival $($Start[1].Returns) -> $Returns (no longer checked: the block arrives without pushing)"
            $Offs = @($Ids | ForEach-Object { if ($After.Units.ContainsKey($_)) { $After.Units[$_].Off } else { 1e9 } })
            $Held = @($Ids | Where-Object { $After.Units.ContainsKey($_) -and $After.Units[$_].Held }).Count
            $Checks["all six on their places after a walker went through (pushed $PushedOff cm at most, returns $($Before.Returns) -> $($After.Returns); $($Offs -join ', ') cm, 60 at most; $Held holding)"] = (@($Offs | Where-Object { $_ -le 60 }).Count -eq 6) -and ($Held -eq 6)
            $Yaws = @($Ids | ForEach-Object { if ($After.Units.ContainsKey($_)) { [Math]::Round((& $YawGap $After.Units[$_].Yaw $FrontYaw)) } else { 999 } })
            $Checks["facing the front again (off by $($Yaws -join ', ') degrees, 10 at most)"] = @($Yaws | Where-Object { $_ -le 10 }).Count -eq 6
        }
    }
    # 5. The plain drag keeps the default width (no "its width" in its order), the Shift move waits for its end, then
    # the six go there in formation.
    $PlainLines = @(Get-Section "formation plain")
    $PlainOrder = $PlainLines | Select-String -Pattern "Order: (\d+) units in line from .*$" | Select-Object -First 1
    $Checks["plain drag: orientation only ($(if ($PlainOrder) { $PlainOrder.Matches[0].Value } else { 'no order' }))"] = [bool]$PlainOrder -and ($PlainOrder.Matches[0].Value -notmatch "its width")
    $Queued = $PlainLines | Select-String -Pattern "Move order of slot 0 queued \(Shift\) for (\d+) units" | Select-Object -First 1
    $Started = $PlainLines | Select-String -Pattern "Queued move started for (\d+) units" | Select-Object -First 1
    $Checks["Shift move queued behind the drag's ($(if ($Queued) { $Queued.Matches[0].Value } else { 'not queued' }))"] = [bool]$Queued
    $Checks["queued move started once the drag's walk was over ($(if ($Started) { $Started.Matches[0].Value } else { 'never' }))"] = [bool]$Started
    if ($PlainOrder -and $Start.Count -ge 2) {
        $QueuedEnd = (& $ReadFormation (Get-Section "formation queued")).Units
        $Near = @($Ids | Where-Object { $QueuedEnd.ContainsKey($_) -and ([Math]::Sqrt([Math]::Pow($QueuedEnd[$_].X - $FormQueuedTo[0], 2) + [Math]::Pow($QueuedEnd[$_].Y - $FormQueuedTo[1], 2)) -le 600) -and $QueuedEnd[$_].Held })
        $Checks["the six at the queued point in formation ($($Near.Count) within 6 m, holding their places)"] = $Near.Count -eq 6
    }
    # The drag's and the queued move's block marches: the six were already in rows, so no stop (the user's 4th trial: formed
    # up on the move, 2 steps at most), nobody left the block, none faster than the slowest + 10 %.
    $PlainBlocks = @($PlainLines + @(Get-Section "formation queued") | Select-String -Pattern "Block march: arrived after (\d+) steps of form-up and (\d+) of march \((\d+) held\), file gap (\d+) cm at most, speed (\d+) cm/s at most, (\d+) units left it")
    $Checks["plain drag and queued move: two block marches arrived ($($PlainBlocks.Count))"] = $PlainBlocks.Count -eq 2
    $PlainStarts = @($PlainLines + @(Get-Section "formation queued") | Select-String -Pattern "Block march: \d+ units, path of \d+ points, (\d+) cm, pace (\d+) cm/s")
    for ($i = 0; $i -lt $PlainBlocks.Count; $i++) {
        $G = $PlainBlocks[$i].Matches[0].Groups
        # No rigid turn at the end (the user's 3rd trial): the march takes no longer than its path at the pace (20 steps a
        # second), + 20 % (the user's 4th trial: the block slows down while its units take their slots on the move and
        # while it re-forms in its last metres, instead of stopping before and after).
        $Expected = if ($i -lt $PlainStarts.Count) { [Math]::Ceiling([double]$PlainStarts[$i].Matches[0].Groups[1].Value / [double]$PlainStarts[$i].Matches[0].Groups[2].Value * 20 * 1.2) } else { 0 }
        # The block walks at its slowest unit's pace (the Man-at-arms, read from the table: 697 cm/s since D345).
        $Pace = if ($i -lt $PlainStarts.Count) { [double]$PlainStarts[$i].Matches[0].Groups[2].Value } else { 697.0 }
        $Checks["plain/queued block: form-up $($G[1].Value) steps (2 at most), march $($G[2].Value) steps ($Expected at most), $($G[6].Value) left, speed $($G[5].Value) cm/s ($Pace + 10 % at most), held $($G[3].Value), gap $($G[4].Value) cm"] =
            ([int]$G[1].Value -le 2) -and ([int]$G[2].Value -le $Expected) -and ([int]$G[6].Value -eq 0) -and ([int]$G[5].Value -le $Pace * 1.1)
    }
    # The arrivals (the user's 2nd and 3rd trials): no rigid turn, each unit from the last 1.5 m of the path straight to a
    # place of its kind: no two straight ways of one kind crossing, (almost) nobody pushed, everyone standing within 1 s.
    $Settled = @(@(Get-Section "formation front") + $PlainLines + @(Get-Section "formation queued") + @(Get-Section "formation block") + @(Get-Section "formation block end") |
        Select-String -Pattern "Block march: settled after (-?\d+) steps, (\d+) crossings, (\d+) pushes, (\d+) cm from its place at most at the end")
    $Checks["arrivals watched: four block marches settled ($($Settled.Count))"] = $Settled.Count -eq 4
    foreach ($S in $Settled) {
        $G = $S.Matches[0].Groups
        $Checks["arrival: settled in $($G[1].Value) steps (8 at most), $($G[2].Value) crossings (0), $($G[3].Value) pushes (2 at most), $($G[4].Value) cm off at the end"] =
            ([int]$G[1].Value -ge 0) -and ([int]$G[1].Value -le 8) -and ([int]$G[2].Value -eq 0) -and ([int]$G[3].Value -le 2)
    }
    # 6. The block march: form-up in place (3 s at most), then on the group's path every unit within 80 cm of its slot and
    # none faster than the slowest + 10 %; it arrives, nobody left behind, then each on its place.
    $BlockLines = @(Get-Section "formation block")
    $BlockReports = @(& $AllReports $BlockLines)
    $BlockIds = @()
    if ($BlockReports.Count -ge 2) { $BlockIds = @($BlockReports[1].Units.Keys | Where-Object { -not $BlockReports[0].Units.ContainsKey($_) } | Sort-Object) }
    $Checks["block: nine units made ($($BlockIds.Count))"] = $BlockIds.Count -eq 9
    if ($BlockIds.Count -eq 9) {
        $BlockSlowest = ($BlockIds | ForEach-Object { $BlockReports[1].Units[$_].Own } | Measure-Object -Minimum).Minimum
        $InBlock = @(foreach ($Line in ($BlockLines | Select-String -Pattern "Formation report: ")) {
            foreach ($M in [regex]::Matches($Line.Line, "#(\d+) (\w+) at \(-?\d+, -?\d+\) yaw -?[\d.]+ speed (-?\d+) own \d+ march \d+ .*? off (\d+) phase (\w+)")) {
                if (($BlockIds -contains [int]$M.Groups[1].Value) -and $M.Groups[5].Value -eq "Block") { , @([double]$M.Groups[3].Value, [double]$M.Groups[4].Value) } } })
        $BlockFastest = ($InBlock | ForEach-Object { $_[0] } | Measure-Object -Maximum).Maximum
        $BlockGap = ($InBlock | ForEach-Object { $_[1] } | Measure-Object -Maximum).Maximum
        $Checks["block: marching samples ($($InBlock.Count))"] = $InBlock.Count -ge 60
        $Checks["block: none faster than the slowest + 10 % ($BlockFastest cm/s at most, slowest $BlockSlowest)"] = ($InBlock.Count -gt 0) -and ($BlockFastest -le $BlockSlowest * 1.1)
        $Checks["block: each within 80 cm of its slot on the path ($BlockGap cm at most)"] = ($InBlock.Count -gt 0) -and ($BlockGap -le 80)
        $Report = $BlockLines + @(Get-Section "formation block end") | Select-String -Pattern "Block report: units (\d+) formup (-?\d+) march (\d+) held (\d+) left (\d+) pace (\d+) gap (\d+) speed (\d+) arrived (\d)" | Select-Object -Last 1
        $Checks["block: formed up, marched, arrived, nobody left ($(if ($Report) { $Report.Matches[0].Value } else { 'no report' }))"] = [bool]$Report -and
            ([int]$Report.Matches[0].Groups[1].Value -eq 9) -and ([int]$Report.Matches[0].Groups[2].Value -ge 0) -and ([int]$Report.Matches[0].Groups[2].Value -le 60) -and
            ([int]$Report.Matches[0].Groups[5].Value -eq 0) -and ([int]$Report.Matches[0].Groups[9].Value -eq 1)
        $BlockEnd = (& $ReadFormation (Get-Section "formation block end")).Units
        $BlockOffs = @($BlockIds | ForEach-Object { if ($BlockEnd.ContainsKey($_)) { $BlockEnd[$_].Off } else { 1e9 } })
        $Checks["block: on their places at the end ($($BlockOffs -join ', ') cm, 60 at most)"] = @($BlockOffs | Where-Object { $_ -le 60 }).Count -eq 9
    }
    $Missing = @("mixed", "front", "back", "queued", "block", "block_end" | Where-Object { -not (Test-Path (Join-Path $Shots "formation_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "formation" $Checks) -and $AllOk

    # The control groups and Ctrl + A (J7a.5): the counts each step logged.
    $G = @(Get-Section "groups")
    $Find = { param([string] $Pattern) $M = $G | Select-String -Pattern $Pattern | Select-Object -First 1; if ($M) { $M.Matches[0] } else { $null } }
    $Report0 = & $ReadFormation @($G | Select-String -Pattern "Formation report: " | Select-Object -First 1 | ForEach-Object { $_.Line })
    $Soldiers = @($Report0.Units.Values | Where-Object { $_.Type -ne "Peasant" }).Count
    $Checks = [ordered]@{}
    $Created = & $Find "Group 1: created, (\d+) units"
    $Checks["Shift + & creates group 1 with the 9 selected ($(if ($Created) { $Created.Groups[1].Value }))"] = $Created -and [int]$Created.Groups[1].Value -eq 9
    $Army = & $Find "Select army: (\d+) soldiers"
    $Checks["Ctrl + A selects the $Soldiers soldiers ($(if ($Army) { $Army.Groups[1].Value }))"] = $Army -and $Soldiers -gt 0 -and [int]$Army.Groups[1].Value -eq $Soldiers
    $Created2 = & $Find "Group 2: created, (\d+) units"
    $Checks["Shift + 2 creates group 2 with the army ($(if ($Created2) { $Created2.Groups[1].Value }))"] = $Created2 -and [int]$Created2.Groups[1].Value -eq $Soldiers
    # Ctrl + & sent between Ctrl + A and Shift + 2: no line of a group there, the army still selected.
    $From = [Array]::FindIndex([string[]]$G, [Predicate[string]]{ param($L) $L -match "Select army: " })
    $To = [Array]::FindIndex([string[]]$G, [Predicate[string]]{ param($L) $L -match "Group 2: created" })
    $Ctrl = if ($From -ge 0 -and $To -gt $From) { @($G[$From..$To] | Select-Object -Skip 1 | Select-Object -SkipLast 1) } else { @() }
    $CtrlSel = $Ctrl | Select-String -Pattern "Selection report: selected (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
    $Checks["Ctrl + & alone does nothing: no group change, the army still selected ($CtrlSel)"] = ($Ctrl.Count -gt 0) -and ($CtrlSel -match "^$Soldiers units") -and
        -not ($Ctrl | Select-String -Pattern "Group \d: ") -and -not ($G | Select-String -Pattern "Group \d: assigned")
    $Checks["Ctrl + A starts no placement nor production"] = -not ($G | Select-String -Pattern "Placement: ")
    $Removed = & $Find "Group 1: removed, (\d+) units"
    $Checks["remove the army from group 1: its 3 Peasants left ($(if ($Removed) { $Removed.Groups[1].Value }))"] = $Removed -and [int]$Removed.Groups[1].Value -eq 3
    $Added = & $Find "Group 1: added, (\d+) units"
    $Checks["Shift + 1 adds the army to group 1 ($(if ($Added) { $Added.Groups[1].Value }))"] = $Added -and [int]$Added.Groups[1].Value -eq 3 + $Soldiers
    $Selected = @($G | Select-String -Pattern "Group 1: selected, (\d+) units")
    $Checks["& selects group 1 ($($Selected.Count) times, $(if ($Selected) { $Selected[0].Matches[0].Groups[1].Value }) units)"] = $Selected.Count -eq 2 -and [int]$Selected[0].Matches[0].Groups[1].Value -eq 3 + $Soldiers
    $Centred = & $Find "Group 1: selected, \d+ units, 0 buildings, camera to \((-?\d+), (-?\d+)\)"
    $Focus = & $Find "Camera: focus \((-?\d+), (-?\d+),"
    $Checks["& twice centres the camera on group 1"] = $Centred -and $Focus -and [Math]::Abs([double]$Focus.Groups[1].Value - [double]$Centred.Groups[1].Value) -lt 50 -and [Math]::Abs([double]$Focus.Groups[2].Value - [double]$Centred.Groups[2].Value) -lt 50
    $Removed2 = @($G | Select-String -Pattern "Group 1: removed, (\d+) units")
    $Checks["Ctrl + Shift + & removes the army again ($(if ($Removed2.Count -ge 2) { $Removed2[1].Matches[0].Groups[1].Value }))"] = $Removed2.Count -ge 2 -and [int]$Removed2[1].Matches[0].Groups[1].Value -eq 3
    $Checks["group 0 empty"] = [bool]($G | Select-String -Pattern "Group 0: empty")
    # The groups' bar (J7a.5): what it showed after each change, the slot clicked.
    $Bars = @($G | Select-String -Pattern "HUD groups: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
    $Checks["bar after Shift + &: 1=9 ($($Bars -join ' | '))"] = $Bars -contains "1=9"
    $Checks["bar after Shift + 2 / remove 1: 1=3 2=$Soldiers"] = $Bars -contains "1=3 2=$Soldiers"
    $Checks["bar after Shift + 1: 1=$(3 + $Soldiers) 2=$Soldiers"] = $Bars -contains "1=$(3 + $Soldiers) 2=$Soldiers"
    $Checks["bar slot 2 clicked selects group 2"] = [bool]($G | Select-String -Pattern "HUD: group 2 clicked")
    $Checks["capture groups_bar"] = Test-Path (Join-Path $Shots "groups_bar.png")
    $Checks["group 2 twice: the camera to it"] = [bool]($G | Select-String -Pattern "Group 2: selected, $Soldiers units, 0 buildings, camera to")
    $AllOk = (Report "groups" $Checks) -and $AllOk

    # The straggler (the user's 1st trial of J7b): the five leave at once (rows formed on the move, formup 0), one straggler
    # left behind, the block hardly held, the six on their places at the end; the Archer (faster) is only reported: does it
    # join the block on the way?
    $Checks = [ordered]@{}
    foreach ($Mark in "straggler", "straggler archer") {
        $Lines = @(Get-Section $Mark)
        $EndLines = @(Get-Section "$Mark end")
        $Reports = @(& $AllReports $Lines)
        $StragIds = @()
        if ($Reports.Count -ge 2) { $StragIds = @($Reports[1].Units.Keys | Where-Object { -not $Reports[0].Units.ContainsKey($_) } | Sort-Object) }
        $Checks["${Mark}: six units made ($($StragIds.Count))"] = $StragIds.Count -eq 6
        $Checks["${Mark}: files on the move at once"] = [bool]($Lines | Select-String -Pattern "Block march: files on the move, start")
        $Left = $Lines | Select-String -Pattern "Block march: (\d+) stragglers left behind" | Select-Object -First 1
        $Checks["${Mark}: one straggler left behind ($(if ($Left) { $Left.Matches[0].Groups[1].Value } else { 'no line' }))"] = $Left -and [int]$Left.Matches[0].Groups[1].Value -eq 1
        $Report = $Lines + $EndLines | Select-String -Pattern "Block report: units (\d+) formup (-?\d+) march (\d+) held (\d+) left (\d+) pace (\d+) gap (\d+) speed (\d+) arrived (\d) settle (-?\d+) crossings (\d+) pushes (\d+) arrivaloff (\d+) stragglers (\d+) joined (\d+)" | Select-Object -Last 1
        $Text = if ($Report) { $Report.Matches[0].Value } else { 'no report' }
        Write-Host "  ${Mark}: $Text"
        foreach ($J in @($Lines + $EndLines | Select-String -Pattern "Block march: (a straggler joined.*|\d+ stragglers, \d+ joined.*)$")) { Write-Host "  ${Mark}: $($J.Matches[0].Value)" }
        if ($Report) {
            $G = $Report.Matches[0].Groups
            # Held: the steps the block waited (formup 0, it never waits for the straggler); a few for the start allowed. Settle
            # (steps of 50 ms) runs until the straggler too is on its place: 10 m behind at the same pace, it comes ~2 s after
            # the block (34 steps measured; the Archer 15): 50 at most, against 8 for a block without one.
            $Checks["${Mark}: left at once, not held, arrived, settled, one straggler ($Text)"] = ([int]$G[2].Value -eq 0) -and ([int]$G[4].Value -le 4) -and
                ([int]$G[9].Value -eq 1) -and ([int]$G[10].Value -ge 0) -and ([int]$G[10].Value -le 50) -and ([int]$G[14].Value -eq 1)
            if ($Mark -eq "straggler") { $Checks["${Mark}: the Man-at-arms behind does not join (joined $($G[15].Value))"] = [int]$G[15].Value -eq 0 }
        } else { $Checks["${Mark}: block report"] = $false }
        $End = (& $ReadFormation $EndLines).Units
        $Offs = @($StragIds | ForEach-Object { if ($End.ContainsKey($_)) { $End[$_].Off } else { 1e9 } })
        $Checks["${Mark}: the six on their places at the end ($($Offs -join ', ') cm, 60 at most)"] = $StragIds.Count -eq 6 -and @($Offs | Where-Object { $_ -le 60 }).Count -eq 6
    }
    $AllOk = (Report "straggler" $Checks) -and $AllOk

    # The scattered group (the user's trial of playtest note 1 of J7c): no straggler, the block forms its rows on the move at once (playtest
    # note 1 of J7b: no gathering first); the units keep clear of each other meanwhile (the closest two members, measured by the server until the
    # arrival's blend: 2 radii of 60 cm, a little less allowed for the separation's slack), it arrives, the six on their places.
    # Small blocks (the user's 2nd trial): the two packets of three 10 m apart march as two blocks of three (one formation for the
    # order, each block its own path and slots), no gathering (formup 0), never held, never a step back, both arrive and settle,
    # the six on their places (60 cm at most). The body's numbers from its Block report, the 2nd block's from its tagged lines.
    $Checks = [ordered]@{}
    $Lines = @(Get-Section "scattered")
    $EndLines = @(Get-Section "scattered end")
    $Reports = @(& $AllReports $Lines)
    $ScatterIds = @()
    if ($Reports.Count -ge 2) { $ScatterIds = @($Reports[1].Units.Keys | Where-Object { -not $Reports[0].Units.ContainsKey($_) } | Sort-Object) }
    $Checks["scattered: six units made ($($ScatterIds.Count))"] = $ScatterIds.Count -eq 6
    $Checks["scattered: files on the move at once"] = [bool]($Lines | Select-String -Pattern "Block march: files on the move, start")
    $Left = $Lines | Select-String -Pattern "Block march: (\d+) stragglers left behind" | Select-Object -First 1
    $Checks["scattered: no straggler left behind ($(if ($Left) { $Left.Matches[0].Groups[1].Value } else { 'no line' }))"] = $Left -and [int]$Left.Matches[0].Groups[1].Value -eq 0
    $Split = $Lines | Select-String -Pattern "Block march: (\d+) blocks \(sizes ([\d, ]+)\), (\d+) stragglers" | Select-Object -First 1
    $Checks["scattered: two blocks of three ($(if ($Split) { $Split.Matches[0].Value } else { 'no line' }))"] = $Split -and $Split.Matches[0].Groups[1].Value -eq "2" -and
        $Split.Matches[0].Groups[2].Value -eq "3, 3" -and $Split.Matches[0].Groups[3].Value -eq "0"
    $Small = @($Lines + $EndLines | Select-String -Pattern "Block march: block 2 of 2: .*$" | ForEach-Object { $_.Matches[0].Value })
    foreach ($S in $Small) { Write-Host "  scattered: $S" }
    $SArr = $Small | Select-String -Pattern "arrived after (\d+) steps of form-up and (\d+) of march \((\d+) held\).*?, (\d+) units left it" | Select-Object -First 1
    $SBack = $Small | Select-String -Pattern ": (\d+) backsteps on the march" | Select-Object -First 1
    $SSettle = $Small | Select-String -Pattern "settled after (-?\d+) steps" | Select-Object -First 1
    $SSpacing = $Small | Select-String -Pattern "closest two units (-?\d+) cm apart" | Select-Object -First 1
    $Checks["scattered: block 2 arrived, no gathering, not held, nobody left ($(if ($SArr) { $SArr.Matches[0].Value } else { 'no line' }))"] = $SArr -and
        [int]$SArr.Matches[0].Groups[1].Value -eq 0 -and [int]$SArr.Matches[0].Groups[3].Value -eq 0 -and [int]$SArr.Matches[0].Groups[4].Value -eq 0
    $Checks["scattered: block 2 no step back ($(if ($SBack) { $SBack.Matches[0].Groups[1].Value } else { 'no line' }))"] = $SBack -and [int]$SBack.Matches[0].Groups[1].Value -eq 0
    $Checks["scattered: block 2 settled in 50 steps at most ($(if ($SSettle) { $SSettle.Matches[0].Groups[1].Value } else { 'no line' }))"] = $SSettle -and
        [int]$SSettle.Matches[0].Groups[1].Value -ge 0 -and [int]$SSettle.Matches[0].Groups[1].Value -le 50
    $Checks["scattered: block 2 no two units closer than 100 cm on the way ($(if ($SSpacing) { $SSpacing.Matches[0].Groups[1].Value } else { 'no line' }) cm)"] = $SSpacing -and
        [int]$SSpacing.Matches[0].Groups[1].Value -ge 100
    $Report = $Lines + $EndLines | Select-String -Pattern "Block report: units (\d+) formup (-?\d+) march (\d+) held (\d+) left (\d+) pace (\d+) gap (\d+) speed (\d+) arrived (\d) settle (-?\d+) crossings (\d+) pushes (\d+) arrivaloff (\d+) stragglers (\d+) joined (\d+) queued (\d+) spacing (-?\d+)" | Select-Object -Last 1
    $Text = if ($Report) { $Report.Matches[0].Value } else { 'no report' }
    Write-Host "  scattered: $Text"
    foreach ($J in @($Lines + $EndLines | Select-String -Pattern "Block march: (a unit fell behind.*|files on the move.*)$")) { Write-Host "  scattered: $($J.Matches[0].Value)" }
    if ($Report) {
        $G = $Report.Matches[0].Groups
        $Checks["scattered: block 1 of three, no gathering, not held, nobody left, arrived, settled in $($G[10].Value) steps (50 at most) ($Text)"] =
            ([int]$G[1].Value -eq 3) -and ([int]$G[2].Value -eq 0) -and ([int]$G[4].Value -eq 0) -and ([int]$G[5].Value -eq 0) -and ([int]$G[14].Value -eq 0) -and
            ([int]$G[9].Value -eq 1) -and ([int]$G[10].Value -ge 0) -and ([int]$G[10].Value -le 50)
        $Checks["scattered: no two members closer than 100 cm on the way ($($G[17].Value) cm)"] = [int]$G[17].Value -ge 100
    } else { $Checks["scattered: block report"] = $false }
    $End = (& $ReadFormation $EndLines).Units
    $Offs = @($ScatterIds | ForEach-Object { if ($End.ContainsKey($_)) { $End[$_].Off } else { 1e9 } })
    $Checks["scattered: the six on their places at the end ($($Offs -join ', ') cm, 60 at most)"] = $ScatterIds.Count -eq 6 -and @($Offs | Where-Object { $_ -le 60 }).Count -eq 6
    $AllOk = (Report "scattered" $Checks) -and $AllOk

    # The close straggler (the user's trial of playtest note 1 of J7b): the 6th within the body's 4.5 m is one of the body
    # (no straggler), the block forms its rows on the move at once (formup 0) and is hardly held (a few steps for the start), nobody falls
    # behind, it arrives, the six on their places.
    $Checks = [ordered]@{}
    $Lines = @(Get-Section "straggler near")
    $EndLines = @(Get-Section "straggler near end")
    $Reports = @(& $AllReports $Lines)
    $NearIds = @()
    if ($Reports.Count -ge 2) { $NearIds = @($Reports[1].Units.Keys | Where-Object { -not $Reports[0].Units.ContainsKey($_) } | Sort-Object) }
    $Checks["straggler near: six units made ($($NearIds.Count))"] = $NearIds.Count -eq 6
    $Checks["straggler near: files on the move at once"] = [bool]($Lines | Select-String -Pattern "Block march: files on the move, start")
    $Left = $Lines | Select-String -Pattern "Block march: (\d+) stragglers left behind" | Select-Object -First 1
    $Checks["straggler near: no straggler, the 6th one of the body ($(if ($Left) { $Left.Matches[0].Groups[1].Value } else { 'no line' }))"] = $Left -and [int]$Left.Matches[0].Groups[1].Value -eq 0
    $Report = $Lines + $EndLines | Select-String -Pattern "Block report: units (\d+) formup (-?\d+) march (\d+) held (\d+) left (\d+) pace (\d+) gap (\d+) speed (\d+) arrived (\d) settle (-?\d+) crossings (\d+) pushes (\d+) arrivaloff (\d+) stragglers (\d+) joined (\d+) queued (\d+) spacing (-?\d+)" | Select-Object -Last 1
    $Text = if ($Report) { $Report.Matches[0].Value } else { 'no report' }
    Write-Host "  straggler near: $Text"
    foreach ($J in @($Lines + $EndLines | Select-String -Pattern "Block march: (a unit fell behind.*|files on the move.*)$")) { Write-Host "  straggler near: $($J.Matches[0].Value)" }
    $Checks["straggler near: nobody fell behind"] = -not ($Lines + $EndLines | Select-String -Pattern "Block march: a unit fell behind")
    if ($Report) {
        $G = $Report.Matches[0].Groups
        $Checks["straggler near: left at once, not held, nobody left, arrived ($Text)"] = ([int]$G[1].Value -eq 6) -and ([int]$G[2].Value -eq 0) -and
            ([int]$G[4].Value -le 4) -and ([int]$G[5].Value -eq 0) -and ([int]$G[9].Value -eq 1)
    } else { $Checks["straggler near: block report"] = $false }
    $End = (& $ReadFormation $EndLines).Units
    $Offs = @($NearIds | ForEach-Object { if ($End.ContainsKey($_)) { $End[$_].Off } else { 1e9 } })
    $Checks["straggler near: the six on their places at the end ($($Offs -join ', ') cm, 60 at most)"] = $NearIds.Count -eq 6 -and @($Offs | Where-Object { $_ -le 60 }).Count -eq 6
    $AllOk = (Report "straggler near" $Checks) -and $AllOk

    # Two blocks meeting (the user's rule for playtest note 1 of J7b: each separate packet marches in its own formation, the
    # blocks merge when they meet, else at the arrival): the two files 5 m apart march as two blocks of three, their paths
    # converge, the smaller merges into the main block on the way (its three units counted by the main block's stragglers line),
    # never held, never a step back (the merged units take their slots on the move), the six on their places (60 cm at most).
    $Checks = [ordered]@{}
    $Lines = @(Get-Section "blocks meet")
    $EndLines = @(Get-Section "blocks meet end")
    $Reports = @(& $AllReports $Lines)
    $MeetIds = @()
    if ($Reports.Count -ge 2) { $MeetIds = @($Reports[1].Units.Keys | Where-Object { -not $Reports[0].Units.ContainsKey($_) } | Sort-Object) }
    $Checks["blocks meet: six units made ($($MeetIds.Count))"] = $MeetIds.Count -eq 6
    $Split = $Lines | Select-String -Pattern "Block march: (\d+) blocks \(sizes ([\d, ]+)\), (\d+) stragglers" | Select-Object -First 1
    $Checks["blocks meet: two blocks of three ($(if ($Split) { $Split.Matches[0].Value } else { 'no line' }))"] = $Split -and $Split.Matches[0].Groups[1].Value -eq "2" -and
        $Split.Matches[0].Groups[2].Value -eq "3, 3" -and $Split.Matches[0].Groups[3].Value -eq "0"
    foreach ($J in @($Lines + $EndLines | Select-String -Pattern "Block march: (block \d+ of \d+.*|a unit fell behind.*|files on the move.*|\d+ stragglers, .*)$")) { Write-Host "  blocks meet: $($J.Matches[0].Value)" }
    $Merge = $Lines + $EndLines | Select-String -Pattern "Block march: block 2 of 2 merged into the main block after (\d+) steps \((\d+) cm apart\)" | Select-Object -First 1
    $Checks["blocks meet: block 2 merged into the main block on the way ($(if ($Merge) { $Merge.Matches[0].Value } else { 'no line' }))"] = [bool]$Merge
    $Merged = $Lines + $EndLines | Select-String -Pattern "Block march: \d+ stragglers, \d+ joined it on the way, \d+ queued behind it, (\d+) units of other blocks merged into it" | Select-Object -Last 1
    $Checks["blocks meet: the main block counts the 3 merged units ($(if ($Merged) { $Merged.Matches[0].Groups[1].Value } else { 'no line' }))"] = $Merged -and [int]$Merged.Matches[0].Groups[1].Value -eq 3
    $Backs = @($Lines + $EndLines | Select-String -Pattern "Block march: (block \d+ of \d+: )?(\d+) backsteps on the march" | ForEach-Object { [int]$_.Matches[0].Groups[2].Value })
    $Checks["blocks meet: no step back ($($Backs -join ', '))"] = $Backs.Count -ge 1 -and @($Backs | Where-Object { $_ -ne 0 }).Count -eq 0
    $Report = $Lines + $EndLines | Select-String -Pattern "Block report: units (\d+) formup (-?\d+) march (\d+) held (\d+) left (\d+) pace (\d+) gap (\d+) speed (\d+) arrived (\d) settle (-?\d+) crossings (\d+) pushes (\d+) arrivaloff (\d+) stragglers (\d+) joined (\d+) queued (\d+) spacing (-?\d+)" | Select-Object -Last 1
    $Text = if ($Report) { $Report.Matches[0].Value } else { 'no report' }
    Write-Host "  blocks meet: $Text"
    if ($Report) {
        $G = $Report.Matches[0].Groups
        $Checks["blocks meet: the main block of six, not held, arrived ($Text)"] = ([int]$G[1].Value -eq 6) -and ([int]$G[4].Value -eq 0) -and ([int]$G[9].Value -eq 1)
    } else { $Checks["blocks meet: block report"] = $false }
    $End = (& $ReadFormation $EndLines).Units
    $Offs = @($MeetIds | ForEach-Object { if ($End.ContainsKey($_)) { $End[$_].Off } else { 1e9 } })
    $Checks["blocks meet: the six on their places at the end ($($Offs -join ', ') cm, 60 at most)"] = $MeetIds.Count -eq 6 -and @($Offs | Where-Object { $_ -le 60 }).Count -eq 6
    $AllOk = (Report "blocks meet" $Checks) -and $AllOk

    # Never a half turn on the march (the user's 4th trial of playtest note 1 of J7b: a unit a little ahead of its slot turned
    # back into its row): in every block march of the part (formation, straggler, scattered, blocks meet), no step on which a unit of the
    # block walked against the march (its velocity along it below -1 cm/s; the arrival's re-forming left out).
    $Checks = [ordered]@{}
    $BackLines = @(); $SlowLines = @(); $FileLines = @(); $ArrBackLines = @(); $InPart = $false
    foreach ($Line in $Log) {
        if ($Line -match "Test mark: (.*)$") { $InPart = $Matches[1] -match "^(formation|straggler|scattered|blocks meet)" }
        elseif ($InPart -and $Line -match "Block march: (?:block \d+ of \d+: )?(\d+) backsteps on the march") { $BackLines += [int]$Matches[1] }
        elseif ($InPart -and $Line -match "Block march: (?:block \d+ of \d+: )?(\d+) slowed steps on the march") { $SlowLines += [int]$Matches[1] }
        elseif ($InPart -and $Line -match "Block march: (?:block \d+ of \d+: )?arrived after \d+ steps of form-up and (\d+) of march \((\d+) held\), file gap (\d+) cm at most.*?files formed after (-?\d+) steps") {
            $FileLines += , @([int]$Matches[1], [int]$Matches[2], [int]$Matches[3], [int]$Matches[4]) }
        elseif ($InPart -and $Line -match "Block march: (?:block \d+ of \d+: )?(\d+) arrival backsteps, (\d+) cm walked back") { $ArrBackLines += , @([int]$Matches[1], [int]$Matches[2]) }
    }
    # Files on the move, rows at the arrival (the user's 5th trial of playtest note 1 of J7b: nobody slowed down on the march).
    # Slowed: steps on which a member walked along the march below 90 % of the pace (its first half second left out: the
    # start). The slowest kind sliding to its file keeps cos 20 degrees = 94 % of it, so a slide is never one; 2 steps (0.1 s)
    # allowed for a blip of the avoidance between two of one kind in one file, anything slowed on purpose lasts far longer.
    # Held: the arrival's re-forming waiting for a unit far off its slot, none (every unit already in its file by then).
    # File gap: a unit is a member within 40 cm across of its file, then pushed a little by its neighbours: 60 cm at most.
    # Files formed: a unit slides across at a third of the pace at least (Pace tan 20 degrees, ~1.8 m/s), so 4 s (80 steps)
    # cover the 7 m of the widest gathering of the part; -1 (never) fails. Arrival backsteps (the head walking back while
    # the rows close up, all its units summed): a shuffle of a few cm while the rows close up, 50 cm at most (no head turning
    # back; 27 measured for the mixed block of nine, 0-9 elsewhere).
    Write-Host "  slowed steps per block march: $($SlowLines -join ', ')"
    Write-Host "  march, held, file gap, files formed after (steps) per block march: $(($FileLines | ForEach-Object { $_ -join '/' }) -join ', ')"
    Write-Host "  arrival backsteps (steps/cm) per block march: $(($ArrBackLines | ForEach-Object { $_ -join '/' }) -join ', ')"
    $Checks["slowed: block marches measured ($($SlowLines.Count))"] = $SlowLines.Count -eq $BackLines.Count
    $Checks["slowed: 2 steps at most on the march ($($SlowLines -join ', '))"] = $SlowLines.Count -gt 0 -and @($SlowLines | Where-Object { $_ -gt 2 }).Count -eq 0
    $Checks["files: block marches measured ($($FileLines.Count))"] = $FileLines.Count -eq $BackLines.Count
    $Checks["files: never held ($(($FileLines | ForEach-Object { $_[1] }) -join ', '))"] = $FileLines.Count -gt 0 -and @($FileLines | Where-Object { $_[1] -ne 0 }).Count -eq 0
    $Checks["files: file gap 60 cm at most ($(($FileLines | ForEach-Object { $_[2] }) -join ', '))"] = $FileLines.Count -gt 0 -and @($FileLines | Where-Object { $_[2] -gt 60 }).Count -eq 0
    $Checks["files: formed after 80 steps at most ($(($FileLines | ForEach-Object { $_[3] }) -join ', '))"] = $FileLines.Count -gt 0 -and @($FileLines | Where-Object { $_[3] -lt 0 -or $_[3] -gt 80 }).Count -eq 0
    $Checks["arrival backsteps: block marches measured ($($ArrBackLines.Count))"] = $ArrBackLines.Count -eq $BackLines.Count
    $Checks["arrival backsteps: 50 cm walked back at most ($(($ArrBackLines | ForEach-Object { $_[1] }) -join ', '))"] = $ArrBackLines.Count -gt 0 -and @($ArrBackLines | Where-Object { $_[1] -gt 50 }).Count -eq 0
    $Checks["backsteps: block marches measured ($($BackLines.Count))"] = $BackLines.Count -ge 9
    $Checks["backsteps: none on the march ($($BackLines -join ', '))"] = $BackLines.Count -gt 0 -and @($BackLines | Where-Object { $_ -ne 0 }).Count -eq 0
    $AllOk = (Report "backsteps" $Checks) -and $AllOk
}

if (($Parts -contains "combat") -and ($Parts -contains "camera")) {
    # The melee (J7b.2), from hol.Combat.Report: "Combat report: step S units N fighting F hits H kills K population 0=a 1=b",
    # then "#id Type slot s hp h/max target t attacking a hits n damage d kills k first f attackers c" per unit.
    $Lines = @(Get-Section "combat") + @(Get-Section "combat away") + @(Get-Section "combat end")
    $AwayFrom = @(Get-Section "combat").Count
    $Reports = @()
    $Current = $null
    for ($LineIndex = 0; $LineIndex -lt $Lines.Count; $LineIndex++) {
        $Line = $Lines[$LineIndex]
        if ($Line -match "Combat report: step (\d+) units (\d+) fighting (\d+) hits (\d+) kills (\d+) population (.*)$") {
            $Current = @{ Away = $LineIndex -ge $AwayFrom; Step = [int64]$Matches[1]; Fighting = [int]$Matches[3]; Hits = [int64]$Matches[4]; Kills = [int64]$Matches[5]; Population = @{}; Units = @{} }
            foreach ($Pair in ($Matches[6].Trim() -split " ")) { if ($Pair -match "^(\d+)=(\d+)$") { $Current.Population[[int]$Matches[1]] = [int]$Matches[2] } }
            $Reports += $Current
        } elseif ($Current -and $Line -match "#(\d+) (\w+) slot (\d+) hp (-?[\d.]+)/(\d+) target (\d+) attacking (\d) hits (\d+) damage ([\d.]+) kills (\d+) first (-?\d+) attackers (\d+)(?: at (-?\d+) (-?\d+))?") {
            $Current.Units[[int]$Matches[1]] = @{ Type = $Matches[2]; Slot = [int]$Matches[3]; Hp = [double]$Matches[4]; Target = [int]$Matches[6]; Hits = [int]$Matches[8]; Kills = [int]$Matches[10]; First = [int64]$Matches[11]; X = $(if ($Matches[13]) { [double]$Matches[13] } else { 0.0 }) }
        }
    }
    $Checks = [ordered]@{}
    $Checks["combat reports ($($Reports.Count))"] = $Reports.Count -ge 20
    if ($Reports.Count -ge 3) {
        $Before = $Reports[0]; $Made = $Reports[1]; $Last = $Reports[-1]
        $Ids = @($Made.Units.Keys | Where-Object { -not $Before.Units.ContainsKey($_) })
        $MenAtArms = @($Ids | Where-Object { $Made.Units[$_].Type -eq "ManAtArms" -and $Made.Units[$_].Slot -eq 0 })
        $Spearmen = @($Ids | Where-Object { $Made.Units[$_].Type -eq "Spearman" -and $Made.Units[$_].Slot -eq 1 })
        $Checks["3 Men-at-arms and 3 Spearmen made ($($MenAtArms.Count), $($Spearmen.Count))"] = $MenAtArms.Count -eq 3 -and $Spearmen.Count -eq 3
        $Alive = @($Spearmen | Where-Object { $Last.Units.ContainsKey($_) })
        $Survivors = @($MenAtArms | Where-Object { $Last.Units.ContainsKey($_) })
        $Checks["the Spearmen killed ($($Alive.Count) left)"] = $Alive.Count -eq 0
        $Checks["the Men-at-arms survive ($($Survivors.Count))"] = $Survivors.Count -eq 3
        $MaaHits = ($Survivors | ForEach-Object { $Last.Units[$_].Hits } | Measure-Object -Sum).Sum
        $MaaKills = ($Survivors | ForEach-Object { $Last.Units[$_].Kills } | Measure-Object -Sum).Sum
        $Checks["7 hits of 12 per Spearman, D345 ($MaaHits hits, $MaaKills kills)"] = $MaaHits -eq 21 -and $MaaKills -eq 3
        $Hurt = @($Survivors | Where-Object { $Last.Units[$_].Hp -lt 155 })
        $Checks["the Spearmen struck back ($($Hurt.Count) Men-at-arms hurt)"] = $Hurt.Count -ge 1
        $Checks["the survivors stop (no target)"] = @($Survivors | Where-Object { $Last.Units[$_].Target -ne 0 }).Count -eq 0
        # The order away (J7b.2): the report sent with it (the order applied at the next step), then one a second.
        $AwayIndex = [Array]::FindIndex([object[]]$Reports, [Predicate[object]]{ param($R) $R.Away })
        if ($AwayIndex -ge 1 -and $AwayIndex + 4 -lt $Reports.Count) {
            $AtOrder = $Reports[$AwayIndex]; $Walking = $Reports[$AwayIndex + 1]; $Later = $Reports[$AwayIndex + 4]
            $Struck = ($MenAtArms | Where-Object { $AtOrder.Units.ContainsKey($_) } | ForEach-Object { $AtOrder.Units[$_].Hits } | Measure-Object -Sum).Sum
            $Checks["the Men-at-arms struck before the order ($Struck hits)"] = $Struck -ge 3
            $Targets = @($MenAtArms | Where-Object { $Walking.Units.ContainsKey($_) -and $Walking.Units[$_].Target -ne 0 }).Count
            $Checks["the order clears their targets (1 s later: $Targets with a target)"] = $Targets -eq 0
            $Frozen = ($MenAtArms | Where-Object { $Later.Units.ContainsKey($_) } | ForEach-Object { $Later.Units[$_].Hits } | Measure-Object -Sum).Sum
            $Checks["no hit while walking away (4 s: $Struck -> $Frozen)"] = $Frozen -eq $Struck
            $X0 = ($MenAtArms | Where-Object { $AtOrder.Units.ContainsKey($_) } | ForEach-Object { $AtOrder.Units[$_].X } | Measure-Object -Average).Average
            $X4 = ($MenAtArms | Where-Object { $Later.Units.ContainsKey($_) } | ForEach-Object { $Later.Units[$_].X } | Measure-Object -Average).Average
            $Checks["they walk away (4 s: x $([int]$X0) -> $([int]$X4))"] = $X4 -le $X0 - 800
            $XEnd = ($Survivors | ForEach-Object { $Last.Units[$_].X } | Measure-Object -Average).Average
            $Checks["they fight on near their destination only (end x $([int]$XEnd))"] = $XEnd -le $CombatMenAtArms[0] - 1200
            Write-Host "  combat away: hits $Struck at the order, $Frozen 4 s later; x $([int]$X0) -> $([int]$X4) -> $([int]$XEnd)"
        } else {
            $Checks["the order away's reports ($AwayIndex of $($Reports.Count))"] = $false
        }
        $Pop0 = if ($Made.Population.ContainsKey(1)) { $Made.Population[1] } else { 0 }
        $Pop1 = if ($Last.Population.ContainsKey(1)) { $Last.Population[1] } else { 0 }
        $Checks["population of slot 1 down by 3 ($Pop0 -> $Pop1)"] = $Pop1 -eq $Pop0 - 3
        $Firsts = @($MenAtArms | ForEach-Object { $Last.Units[$_].First } | Where-Object { $_ -ge 0 })
        $FirstHit = if ($Firsts.Count -gt 0) { (($Firsts | Measure-Object -Minimum).Minimum - $Made.Step) * 0.05 } else { -1 }
        $KillSteps = @($Lines | Select-String -Pattern "Combat: step (\d+) #\d+ Spearman of slot 1 killed" | ForEach-Object { [int64]$_.Matches[0].Groups[1].Value })
        $LastKill = if ($KillSteps.Count -gt 0) { (($KillSteps | Measure-Object -Maximum).Maximum - $Made.Step) * 0.05 } else { -1 }
        $Checks["first hit within 3 s ($FirstHit s)"] = $FirstHit -ge 0 -and $FirstHit -le 3
        Write-Host "  combat: first hit $FirstHit s after the spawn, kills at $(($KillSteps | ForEach-Object { ($_ - $Made.Step) * 0.05 }) -join ', ') s, MaA HP $(($Survivors | ForEach-Object { $Last.Units[$_].Hp }) -join ', '), total hits $($Last.Hits)"
    }
    # The health bars (J7b.5): each bar's unit wounded in the combat report just before (health never rises), none selected
    # before the box, then the 3 Men-at-arms among them, selected; unhurt unselected units without one.
    $Bars = @()
    $Wounded = @{}
    $HpReports = @()
    foreach ($Line in @(Get-Section "combat")) {
        if ($Line -match "Combat report: step") { $Wounded = @{}; $HpReports += ,@{} }
        elseif ($Line -match "#(\d+) \w+ slot \d+ hp (-?[\d.]+)/(\d+) ") { if ($HpReports.Count) { $HpReports[-1][$Matches[1]] = @([Math]::Max([int][Math]::Ceiling([double]$Matches[2]), 0), [int]$Matches[3]) }; if ([double]$Matches[2] -lt [int]$Matches[3]) { $Wounded[$Matches[1]] = $true } }
        elseif ($Line -match "Health bars: bars (\d+) \(([^)]*)\), unhurt unselected (\d+)") {
            $Items = @(if ($Matches[2]) { $Matches[2] -split ", " })
            $Bars += @{ Count = [int]$Matches[1]; Unhurt = [int]$Matches[3]; Wounded = $Wounded.Clone(); Report = $HpReports.Count - 1;
                Health = @($Items | ForEach-Object { if ($_ -match "^#(\d+) (\d+)/(\d+)") { @{ Id = $Matches[1]; Hp = [int]$Matches[2]; Max = [int]$Matches[3] } } });
                Ids = @($Items | ForEach-Object { if ($_ -match "^#(\d+)") { $Matches[1] } }); Selected = @($Items | Where-Object { $_ -match " selected$" } | ForEach-Object { if ($_ -match "^#(\d+)") { $Matches[1] } }) }
        }
    }
    if ($Bars.Count -ge 2) {
        $B0 = $Bars[0]; $B1 = $Bars[1]
        $Unwounded0 = @($B0.Ids | Where-Object { -not $B0.Wounded.ContainsKey($_) })
        $Checks["bars, nothing selected: the wounded alone ($($B0.Count) bars, $($Unwounded0.Count) unhurt among them, $($B0.Unhurt) unhurt without)"] = ($B0.Count -ge 1) -and ($Unwounded0.Count -eq 0) -and ($B0.Selected.Count -eq 0) -and ($B0.Unhurt -ge 1)
        $Unwounded1 = @($B1.Ids | Where-Object { -not $B1.Wounded.ContainsKey($_) -and $B1.Selected -notcontains $_ })
        $Checks["bars, 3 Men-at-arms selected: theirs too ($($B1.Selected.Count) selected, $($B1.Count) bars)"] = ($B1.Selected.Count -eq 3) -and ($Unwounded1.Count -eq 0) -and ($B1.Count -ge 3)
        # J7-Q5 revised: each bar shows its unit's exact health: of its type's maximum, between its health in the combat
        # reports just before and just after when the section has one (health never rises; the bars are those of the last
        # frame drawn).
        $Mismatch = @(foreach ($B in @($B0, $B1)) { foreach ($H in $B.Health) {
            $Prev = if ($B.Report -ge 0) { $HpReports[$B.Report][$H.Id] }; $Next = if ($B.Report + 1 -lt $HpReports.Count) { $HpReports[$B.Report + 1][$H.Id] }
            if (-not $Prev -or $H.Max -ne $Prev[1] -or $H.Hp -gt $Prev[0] -or ($Next -and $H.Hp -lt $Next[0])) { "#$($H.Id) $($H.Hp)/$($H.Max)" } } })
        $Checks["bars, exact health between the combat reports' around ($($Mismatch.Count) off: $($Mismatch -join ', '))"] = ($B0.Health.Count -eq $B0.Count) -and ($B1.Health.Count -eq $B1.Count) -and ($Mismatch.Count -eq 0)
        Write-Host "  combat bars: $($B0.Count) then $($B1.Count) bars ($($B1.Selected.Count) selected), unhurt unselected $($B0.Unhurt) / $($B1.Unhurt)"
    } else {
        $Checks["health bars reports ($($Bars.Count))"] = $false
    }
    $Checks["capture"] = (Test-Path (Join-Path $Shots "combat_end.png")) -and (Test-Path (Join-Path $Shots "combat_bars.png"))
    $AllOk = (Report "combat" $Checks) -and $AllOk
}

if (($Parts -contains "combat") -and ($Parts -contains "camera")) {
    # The strikes and the fall (J7b.5), from hol.Units.AnimReport: in the melee the Men-at-arms and the Spearmen striking play
    # their strike; the Peasants ordered on a Spearman strike with the knife (prop 8); the Peasant killed plays its fall,
    # the knife in hand (J7c); no
    # unit striking or dying shows anything but its type's gesture.
    $Checks = [ordered]@{}
    $Shown = @{}
    $Wrong = @()
    foreach ($Line in @(Get-Section "combat") + @(Get-Section "orders attack") + @(Get-Section "combat fall")) {
        if ($Line -notmatch "Anim report by state: (.*)$") { continue }
        foreach ($Part in ($Matches[1] -split '; ')) {
            if ($Part -notmatch '^(\w+) (Attacking|Dying): (.+)$') { continue }
            $Type = $Matches[1]; $State = $Matches[2]; $Outcomes = $Matches[3]
            # J7c: a unit strikes and falls with its attack's prop (the Peasant's knife, 8; the soldiers' weapons are in
            # their mesh).
            $Prop = if ($Type -eq "Peasant") { 8 } else { 0 }
            $Expected = if ($State -eq "Attacking") { "A_$Type`_Attack prop $Prop" } else { "A_$Type`_Fall prop $Prop" }
            $Key = "$Type $State"
            if (-not $Shown.ContainsKey($Key)) { $Shown[$Key] = 0 }
            foreach ($Outcome in ($Outcomes -split ', ')) {
                if ($Outcome -notmatch '^(.+) x(\d+)$') { continue }
                if ($Matches[1] -eq $Expected) { $Shown[$Key] = [Math]::Max($Shown[$Key], [int]$Matches[2]) } else { $Wrong += "$Key shows $Outcome" }
            }
        }
    }
    $Count = [int]$Shown["ManAtArms Attacking"]
    $Checks["melee: Men-at-arms striking play A_ManAtArms_Attack ($Count of 3)"] = $Count -ge 2
    $Count = [int]$Shown["Spearman Attacking"]
    $Checks["melee: Spearmen striking play A_Spearman_Attack ($Count of 3)"] = $Count -ge 2
    $Count = [int]$Shown["Peasant Attacking"]
    $Checks["Peasants striking play A_Peasant_Attack with the knife (prop 8) ($Count)"] = $Count -ge 1
    $Count = [int]$Shown["Peasant Dying"]
    $Checks["the Peasant killed plays A_Peasant_Fall ($Count)"] = $Count -ge 1
    $Checks["striking or dying: never another sequence ($($Wrong -join '; '))"] = $Wrong.Count -eq 0
    # Falling (J7b code review, point 4): a unit keeps for its whole fall the yaw it was last drawn at alive (no half-turn
    # back to its place's front); the Peasant killed is reported falling, turned by at most a few degrees.
    $Falls = @(Get-Section "combat fall" | Where-Object { $_ -match "Anim report falls: slot 1, (\d+) falling, largest turn ([\d.]+) deg" } | ForEach-Object {
        [pscustomobject]@{ Falling = [int]$Matches[1]; Turn = [double]$Matches[2] } })
    $Fall = $Falls | Select-Object -First 1
    $Checks["falling: keeps the yaw it died with ($(if ($Fall) { "$($Fall.Falling) falling, largest turn $($Fall.Turn) deg" } else { 'no report' }))"] = $Fall -and $Fall.Falling -ge 1 -and $Fall.Turn -lt 3
    $Missing = @("melee", "peasant_strike", "fall" | Where-Object { -not (Test-Path (Join-Path $Shots "combat_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    Write-Host "  combat anims: $(($Shown.Keys | Sort-Object | ForEach-Object { "$_ x$($Shown[$_])" }) -join ', ')"
    $AllOk = (Report "combat anims" $Checks) -and $AllOk

    # The bodies (J7c, bodies kept 30 s), from hol.Units.CorpseReport: 5 s after the fall, the Peasant's body still on the
    # ground, its fall ended, laid ~4 s before (once its 1 s of Dying is over: HOLCombat::DyingSteps; no older body to count
    # on: the Peasants the orders' Spearman killed later are now helped by the idle Men-at-arms, MV.4 coming to aid);
    # ~40 s later, the bodies gone at 31 s (held 30 s, sunk in 1 s), none older left.
    $Checks = [ordered]@{}
    $CorpsePattern = "Corpse report: (\d+) corpses \((.*)\), (\d+) sinking, (\d+) fall ended, oldest ([\d.]+) s; (\d+) removed, longest kept ([\d.]+) s"
    $ReadCorpseReport = {
        param($Lines)
        $Lines | Where-Object { $_ -match $CorpsePattern } | Select-Object -First 1 | ForEach-Object {
            [pscustomobject]@{ Corpses = [int]$Matches[1]; Types = $Matches[2]; Sinking = [int]$Matches[3]; FallEnded = [int]$Matches[4]; Oldest = [double]$Matches[5]
                Removed = [int]$Matches[6]; LongestKept = [double]$Matches[7] } }
    }
    $Kept = & $ReadCorpseReport @(Get-Section "combat fall")
    $Checks["bodies kept: 5 s after the fall ($(if ($Kept) { "$($Kept.Corpses) ($($Kept.Types)), $($Kept.FallEnded) fall ended, oldest $($Kept.Oldest) s" } else { 'no report' }))"] =
        $Kept -and $Kept.Corpses -ge 1 -and $Kept.Oldest -ge 3.9 -and $Kept.FallEnded -ge 1
    $Gone = & $ReadCorpseReport @(Get-Section "combat corpse expiry")
    $Checks["bodies gone at 31 s ($(if ($Gone) { "$($Gone.Removed) removed, longest kept $($Gone.LongestKept) s, oldest left $($Gone.Oldest) s" } else { 'no report' }))"] =
        $Gone -and $Gone.Removed -ge 1 -and $Gone.LongestKept -ge 31.0 -and $Gone.LongestKept -le 31.5 -and $Gone.Oldest -lt 31.1
    $Checks["capture combat_corpse"] = Test-Path (Join-Path $Shots "combat_corpse.png")
    $AllOk = (Report "combat bodies" $Checks) -and $AllOk
}

if (($Parts -contains "combat") -and ($Parts -contains "camera")) {
    # The shot (J7c.2a): every arrow released lands after its flight (distance at release / arrow speed, in whole steps,
    # 1 at least) or is lost (target dead in flight); a unit killed by an arrow takes the dying path (its fall).
    $Checks = [ordered]@{}
    $Shot = @(Get-Section "combat arrows") + @(Get-Section "combat arrows fight")
    $StepMs = 50; $ArrowSpeed = 3750.0  # HOLCombat::DefaultArrowSpeed (37.5 m/s since the D345 trial)
    $Releases = @{}; $BadFlight = 0
    foreach ($Line in $Shot) {
        if ($Line -match "Combat: step (\d+) #(\d+) Archer releases an arrow at #(\d+) \w+: [\d.]+ damage, (\d+) cm, (\d+) ms, lands step (\d+)") {
            $Release = [int64]$Matches[1]; $Cm = [double]$Matches[4]; $Ms = [int]$Matches[5]; $Lands = [int64]$Matches[6]
            $Expected = [Math]::Max(1, [Math]::Ceiling($Ms / $StepMs))
            if ([Math]::Abs($Ms - $Cm * 1000.0 / $ArrowSpeed) -gt 1.0 -or ($Lands - $Release) -ne $Expected) { $BadFlight++ }
            $Releases["$($Matches[2]) $Release"] = $Lands
        }
    }
    $Landed = 0; $Lost = 0; $BadLanding = 0; $Delays = @()
    foreach ($Line in $Shot) {
        if ($Line -match "Combat: step (\d+) arrow of #(\d+) Archer lands on #\d+ \w+ for [\d.]+, -?[\d.]+ HP left \(released step (\d+), (\d+) steps\)") {
            $Landed++; $Delays += [int]$Matches[4]
            if (-not $Releases.ContainsKey("$($Matches[2]) $($Matches[3])") -or $Releases["$($Matches[2]) $($Matches[3])"] -ne [int64]$Matches[1]) { $BadLanding++ }
        }
        elseif ($Line -match "Combat: step \d+ arrow of #\d+ Archer at #\d+ lost \(target dead\)") { $Lost++ }
    }
    $Kills = @($Shot | Select-String -Pattern "Combat: step \d+ #\d+ \w+ of slot 1 killed by #\d+ Archer of slot 0 \(\d+ hits struck, by arrow\)")
    $Falls = @(Get-Section "combat arrows" | Where-Object { $_ -match "Anim report falls: slot 1, (\d+) falling" } | ForEach-Object { [int]$Matches[1] })
    $Delay = if ($Delays.Count) { "$(($Delays | Measure-Object -Minimum).Minimum)-$(($Delays | Measure-Object -Maximum).Maximum) steps" } else { "none" }
    $Checks["arrows: $($Releases.Count) released, $Landed landed, $Lost lost, flights $Delay"] = $Releases.Count -ge 5 -and $Landed -ge 4 -and ($Landed + $Lost) -ge ($Releases.Count - 4) -and ($Landed + $Lost) -le $Releases.Count
    $Checks["each flight = distance at release / 37.5 m/s in whole steps ($BadFlight off)"] = $Releases.Count -gt 0 -and $BadFlight -eq 0
    $Checks["each arrow lands at its step ($BadLanding off)"] = $Landed -gt 0 -and $BadLanding -eq 0
    $Checks["kills by arrow: $($Kills.Count), then falling: $(if ($Falls.Count) { $Falls[0] } else { 'no report' })"] = $Kills.Count -ge 1 -and $Falls.Count -ge 1 -and $Falls[0] -ge 1
    # The arrows drawn (J7c.2b): as many launched on the display as released by the simulation (1 per shooter off at
    # most), none left in flight once the shot's units are gone; the close-ups taken.
    $ArrowReports = @($Shot | Where-Object { $_ -match "Arrow report: in flight (\d+) launched (\d+) landed (\d+)" } | ForEach-Object { , @([int]$Matches[1], [int]$Matches[2], [int]$Matches[3]) })
    if ($ArrowReports.Count -ge 2) {
        $Drawn = $ArrowReports[-1][1] - $ArrowReports[0][1]
        $Checks["arrows drawn: $Drawn launched for $($Releases.Count) released, $($ArrowReports[-1][0]) in flight at the end"] = [Math]::Abs($Drawn - $Releases.Count) -le 4 -and $ArrowReports[-1][0] -eq 0 -and $Drawn -ge 5
    }
    else { $Checks["arrow reports of the display: $($ArrowReports.Count) (2 expected)"] = $false }
    $Checks["close-ups of the arrows in the air (combat_arrows_1, _2)"] = (Test-Path (Join-Path $Shots "combat_arrows_1.png")) -and (Test-Path (Join-Path $Shots "combat_arrows_2.png"))
    $AllOk = (Report "combat arrows" $Checks) -and $AllOk

    # The target that flees (J7c.2b): the first arrow released at 20 m (D345) lands on the Spearman running away at ~8 m/s
    # (damage at the landing), out of reach by then; none left in flight on the display at the end.
    $Checks = [ordered]@{}
    $Flee = @(Get-Section "combat arrows flee")
    $FirstRelease = $Flee | Where-Object { $_ -match "Combat: step (\d+) #(\d+) Archer releases an arrow at #(\d+) Spearman: [\d.]+ damage, (\d+) cm, (\d+) ms, lands step (\d+)" } | Select-Object -First 1
    if ($FirstRelease -and $FirstRelease -match "Combat: step (\d+) #(\d+) Archer releases an arrow at #(\d+) Spearman: [\d.]+ damage, (\d+) cm, (\d+) ms, lands step (\d+)") {
        $FleeArcher = $Matches[2]; $FleeTarget = $Matches[3]; $FleeCm = [int]$Matches[4]; $FleeMs = [int]$Matches[5]; $FleeStep = $Matches[1]; $FleeLands = $Matches[6]
        $FleeLanding = $Flee | Where-Object { $_ -match "Combat: step $FleeLands arrow of #$FleeArcher Archer lands on #$FleeTarget Spearman for ([\d.]+), -?[\d.]+ HP left \(released step $FleeStep," } | Select-Object -First 1
        # The Spearman's distance to its shooter in each pair of reports after the order (Units where: slot 1, then slot 0).
        $Wheres = @($Flee | Where-Object { $_ -match "Units where: " })
        $Distances = @()
        for ($i = 0; $i -lt $Wheres.Count; $i++) {
            if ($Wheres[$i] -match "#$FleeTarget slot 1 at \((-?\d+), (-?\d+)\)") {
                $Tx = [double]$Matches[1]; $Ty = [double]$Matches[2]
                $Archers = @([regex]::Matches(($Wheres | Select-Object -Skip $i -First 2) -join " ", "#$FleeArcher slot 0 at \((-?\d+), (-?\d+)\)") | ForEach-Object { [Math]::Sqrt([Math]::Pow([double]$_.Groups[1].Value - $Tx, 2) + [Math]::Pow([double]$_.Groups[2].Value - $Ty, 2)) })
                if ($Archers.Count) { $Distances += [int](($Archers | Measure-Object -Minimum).Minimum) }
            }
        }
        $Checks["first arrow released at $FleeCm cm, $FleeMs ms ($([int]($FleeCm * 10 / [Math]::Max($FleeMs, 1))) m/s apparent), then the Spearman runs: $($Distances -join ', ') cm off"] = $FleeCm -ge 1800 -and $FleeCm -le 2120
        $Checks["it lands on the fleeing Spearman at its step (damage $(if ($FleeLanding -and $FleeLanding -match 'for ([\d.]+),') { $Matches[1] } else { 'none' }))"] = [bool]$FleeLanding
        $Checks["the Spearman out of reach meanwhile (farthest $(if ($Distances.Count) { ($Distances | Measure-Object -Maximum).Maximum } else { '?' }) cm > 2000)"] = $Distances.Count -ge 1 -and ($Distances | Measure-Object -Maximum).Maximum -gt 2000
    }
    else { $Checks["an arrow released at the Spearman"] = $false }
    $FleeReports = @($Flee | Where-Object { $_ -match "Arrow report: in flight (\d+) launched" } | ForEach-Object { [int]$Matches[1] })
    $Checks["none in flight on the display at the end ($(if ($FleeReports.Count) { $FleeReports[-1] } else { 'no report' }))"] = $FleeReports.Count -ge 1 -and $FleeReports[-1] -eq 0
    $AllOk = (Report "combat arrows flee" $Checks) -and $AllOk

    # The last attacker forgotten (EtapeDefini S): the hit's step from the Archer's landing line, then the Man-at-arms in each
    # combat report (its target, its place) and its activity in the "Units where" that follows each report (the first landing: chased
    # across the map, it is shot again once the Archer has arrived). Within 4 s of the hit
    # (80 steps) it takes the Archer on (the riposte kept); from 5.5 s on (110 steps: the 5 s memory and a search) it has no
    # target, stands still (less than 30 cm walked over the reports left) and is idle.
    $Checks = [ordered]@{}
    $Forget = @(Get-Section "combat forget")
    $Landing = $Forget | Where-Object { $_ -match "Combat: step (\d+) arrow of #(\d+) Archer lands on #(\d+) ManAtArms for" } | Select-Object -First 1
    if ($Landing -and $Landing -match "Combat: step (\d+) arrow of #(\d+) Archer lands on #(\d+) ManAtArms for") {
        $HitStep = [int64]$Matches[1]; $ForgetArcherId = [int]$Matches[2]; $ForgetManId = $Matches[3]
        $Checks["the Archer's arrow lands on the Man-at-arms (step $HitStep), then the Archer is sent away"] = [bool]($Forget | Select-String -Pattern "Move order: \d+ units to \(")
        $Rows = @(); $ReportStep = -1
        foreach ($Line in $Forget) {
            if ($Line -match "Combat report: step (\d+) units") { $ReportStep = [int64]$Matches[1] }
            elseif ($ReportStep -ge 0 -and $Line -match "#$ForgetManId ManAtArms slot 0 hp [\d.]+/\d+ target (\d+) attacking \d+ .* at (-?\d+) (-?\d+)") {
                $Rows += [pscustomobject]@{ Step = $ReportStep; Target = [int]$Matches[1]; X = [double]$Matches[2]; Y = [double]$Matches[3]; Activity = "?" }
            }
            elseif ($Rows.Count -gt 0 -and $Line -match "Units where: .*#$ForgetManId slot 0 at \(-?\d+, -?\d+\) (\w+)") { $Rows[-1].Activity = $Matches[1] }
        }
        $Within = @($Rows | Where-Object { $_.Step -gt $HitStep -and $_.Step -le $HitStep + 80 })
        $After = @($Rows | Where-Object { $_.Step -ge $HitStep + 110 })
        $Riposting = @($Within | Where-Object { $_.Target -eq $ForgetArcherId }).Count
        $Checks["within 4 s of the hit it takes the Archer on (riposte: target #$ForgetArcherId in $Riposting/$($Within.Count) reports)"] = $Within.Count -ge 1 -and $Riposting -ge 1
        $Checks["from 5.5 s after the hit it has no target ($(($After | ForEach-Object { "+$($_.Step - $HitStep) target $($_.Target)" }) -join ', '))"] = $After.Count -ge 4 -and @($After | Where-Object { $_.Target -ne 0 }).Count -eq 0
        $Walked = if ($After.Count -ge 2) { [int][Math]::Sqrt([Math]::Pow($After[-1].X - $After[0].X, 2) + [Math]::Pow($After[-1].Y - $After[0].Y, 2)) } else { -1 }
        $Checks["and stands still (walked $Walked cm from 5.5 s to the end, at ($(if ($After.Count) { "$($After[-1].X), $($After[-1].Y)" } else { '?' })))"] = $After.Count -ge 2 -and $Walked -le 30
        $Checks["and is idle ($(($After | ForEach-Object { $_.Activity }) -join ', '))"] = $After.Count -ge 1 -and @($After | Where-Object { $_.Activity -ne "Idle" }).Count -eq 0
    }
    else { $Checks["an arrow of the Archer lands on the Man-at-arms"] = $false }
    $AllOk = (Report "combat forget" $Checks) -and $AllOk

    # Reinforcements behind their friends in the fight (MV.4): the 6 sent by a plain move on the ground.
    # Coming to aid (MV.4, choice C): from 12 s after the order (walked there), no idle melee of slot 0 stays within 20 m of a
    # fighting friend (hol.Combat.AidReport each second, with the ids). One that just killed its target is idle until its next
    # search (every 4 steps, 0.2 s) and may be caught by one sample: the same id in two samples running stayed idle 1 s.
    $Checks = [ordered]@{}
    $Checks["a plain move on the ground (no attack order)"] = [bool](Get-Section "combat reinforce sent" | Select-String -SimpleMatch "Order: 6 units to (")
    $AidRows = @(Get-Section "combat reinforce sent" | Where-Object { $_ -match "Aid report: slot 0 idle melee \d+ near a fighting friend (\d+) .*closest enemy -?\d+ cm(.*)$" } |
        ForEach-Object { [pscustomobject]@{ Near = [int]$Matches[1]; Ids = @($Matches[2] -split ' ' | Where-Object { $_ }) } })
    $AidStayed = @(for ($i = 12; $i -lt $AidRows.Count - 1; $i++) { $Next = $AidRows[$i + 1].Ids; $AidRows[$i].Ids | Where-Object { $Next -contains $_ } })
    $Checks["idle melee near a fighting friend, each second: $(($AidRows | ForEach-Object { $_.Near }) -join ',') (from 12 s on, none in two samples running: $(if ($AidStayed.Count) { $AidStayed -join ' ' } else { 'none' }))"] = $AidRows.Count -ge 20 -and $AidStayed.Count -eq 0
    $AllOk = (Report "combat reinforce" $Checks) -and $AllOk

    # Coming to aid, control (MV.4, choice C): aid off, idle melee near their fighting friends; aid on, none left within 3 s.
    $Checks = [ordered]@{}
    $AidOff = @(Get-Section "combat aid" | Where-Object { $_ -match "Aid report: slot 0 idle melee (\d+) near a fighting friend (\d+)" } | ForEach-Object { "$($Matches[1])/$($Matches[2])" })
    $AidOn = @(Get-Section "combat aid on" | Where-Object { $_ -match "Aid report: slot 0 idle melee (\d+) near a fighting friend (\d+)" } | ForEach-Object { "$($Matches[1])/$($Matches[2])" })
    $Checks["aid off: idle/near the fight each second $($AidOff -join ', ') (some near, the case is meaningful)"] = $AidOff.Count -ge 3 -and @($AidOff | Where-Object { [int]($_ -split '/')[1] -ge 1 }).Count -eq $AidOff.Count
    $Checks["aid on: idle/near the fight each second $($AidOn -join ', ') (none idle from 3 s on)"] = $AidOn.Count -ge 4 -and @($AidOn | Select-Object -Skip 2 | Where-Object { [int]($_ -split '/')[0] -ne 0 }).Count -eq 0
    $AllOk = (Report "combat aid" $Checks) -and $AllOk
}

if (($Parts -contains "combat") -and ($Parts -contains "camera") -and -not $SubCases.ContainsKey("combat")) {
    # The orders (J7b.3): Peasants ordered on a Spearman strike it; the stop clears their targets and their hits stay.
    $Checks = [ordered]@{}
    $Attack = @(Get-Section "orders attack")
    $Stop = @(Get-Section "orders stop")
    $Checks["right click on the enemy: attack order"] = [bool]($Attack | Select-String -SimpleMatch "Order: 4 units attack #")
    $Checks["applied by the server"] = [bool]($Attack | Select-String -Pattern "Attack order of slot 0 for 4 units on #")
    $PeasantHits = {
        param($Lines)
        $Last = @{}
        foreach ($Line in $Lines) { if ($Line -match "#(\d+) Peasant slot 0 hp -?\d+/\d+ target (\d+) attacking \d hits (\d+)") { $Last[$Matches[1]] = @{ Target = [int]$Matches[2]; Hits = [int]$Matches[3] } } }
        $Last
    }
    $AtStop = & $PeasantHits $Attack
    $Hits = ($AtStop.Values | ForEach-Object { $_.Hits } | Measure-Object -Sum).Sum
    $Checks["ordered Peasants strike the Spearman ($Hits hits)"] = $Hits -ge 1
    $Block = [regex]::Match((@($Attack | Select-String -Pattern "HUD report: " | Select-Object -Last 1) -join ''), 'selection Peasant x4 \(own\), [^,]*').Value
    $Checks["their selection block: fighting, none idle ($Block)"] = $Block -match 'fighting [1-9]\d* moving \d+ idle 0$'
    $StopReports = @($Stop | Select-String -Pattern "Combat report:").Count
    $After = & $PeasantHits $Stop
    $Targets = @($After.Values | Where-Object { $_.Target -ne 0 }).Count
    $Checks["stop applied"] = [bool]($Stop | Select-String -Pattern "Stop order of slot 0 for \d+ units")
    $Checks["stop clears their targets ($Targets with a target)"] = $StopReports -ge 2 -and $After.Count -ge 1 -and $Targets -eq 0
    Write-Host "  orders: Peasant hits $Hits before the stop, $(($After.Values | ForEach-Object { $_.Hits } | Measure-Object -Sum).Sum) 2 s after"

    # The reports of a section: one hashtable per report, unit id -> type, slot, target, hits, position.
    function Get-CombatReports($Lines) {
        $Result = @()
        $Units = $null
        foreach ($Line in $Lines) {
            if ($Line -match "Combat report: step (\d+)") {
                $Units = @{}
                $Result += , $Units
            } elseif ($null -ne $Units -and $Line -match "#(\d+) (\w+) slot (\d+) hp (-?[\d.]+)/\d+ target (\d+) attacking \d hits (\d+) .* at (-?\d+) (-?\d+)") {
                $Units[[int]$Matches[1]] = @{ Type = $Matches[2]; Slot = [int]$Matches[3]; Hp = [double]$Matches[4]; Target = [int]$Matches[5]; Hits = [int]$Matches[6]; X = [double]$Matches[7]; Y = [double]$Matches[8] }
            }
        }
        return , $Result
    }

    # The attack-move (J7b.3): engaged on the way, the Spearman killed, then on to the destination.
    $Move = @(Get-Section "orders attackmove")
    $Checks["attack-move ordered"] = [bool]($Move | Select-String -SimpleMatch "Order: 3 units attack-move to")
    $MoveReports = Get-CombatReports $Move
    if ($MoveReports.Count -ge 10) {
        $Made = $MoveReports[0]; $Last = $MoveReports[-1]
        $MoveMen = @($Made.Keys | Where-Object { $Made[$_].Type -eq "ManAtArms" -and $Made[$_].Slot -eq 0 -and [Math]::Abs($Made[$_].X - $OrdersMoveFrom[0]) -le 500 -and [Math]::Abs($Made[$_].Y - $OrdersMoveFrom[1]) -le 500 })
        $MoveSpear = @($Made.Keys | Where-Object { $Made[$_].Type -eq "Spearman" -and $Made[$_].Slot -eq 1 -and [Math]::Abs($Made[$_].X - $OrdersMoveSpearman[0]) -le 300 -and [Math]::Abs($Made[$_].Y - $OrdersMoveSpearman[1]) -le 300 })
        $Checks["3 Men-at-arms and the Spearman made ($($MoveMen.Count), $($MoveSpear.Count))"] = $MoveMen.Count -eq 3 -and $MoveSpear.Count -eq 1
        if ($MoveMen.Count -eq 3 -and $MoveSpear.Count -eq 1) {
            $SpearId = $MoveSpear[0]
            $Engaged = @($MoveReports | Where-Object { $R = $_; @($MoveMen | Where-Object { $R.ContainsKey($_) -and $R[$_].Target -eq $SpearId }).Count -gt 0 }).Count
            $Checks["they take on the Spearman on the way ($Engaged reports)"] = $Engaged -ge 1
            $Checks["the Spearman killed"] = -not $Last.ContainsKey($SpearId)
            $MoveHits = ($MoveMen | Where-Object { $Last.ContainsKey($_) } | ForEach-Object { $Last[$_].Hits } | Measure-Object -Sum).Sum
            $Checks["7 hits of 12 to kill it, D345 ($MoveHits)"] = $MoveHits -eq 7
            $Arrived = @($MoveMen | Where-Object { $Last.ContainsKey($_) -and [Math]::Sqrt([Math]::Pow($Last[$_].X - $OrdersMoveTo[0], 2) + [Math]::Pow($Last[$_].Y - $OrdersMoveTo[1], 2)) -le 400 })
            $Checks["then on to the destination ($($Arrived.Count) of 3 within 4 m)"] = $Arrived.Count -eq 3
            # Where they stood when the Spearman fell: off their destination (the fight on the way, not at the end).
            $DeathIndex = [Array]::FindIndex([object[]]$MoveReports, [Predicate[object]]{ param($R) -not $R.ContainsKey($SpearId) })
            if ($DeathIndex -ge 0) {
                $XAtDeath = ($MoveMen | Where-Object { $MoveReports[$DeathIndex].ContainsKey($_) } | ForEach-Object { $MoveReports[$DeathIndex][$_].X } | Measure-Object -Average).Average
                $Checks["the fight on the way (x $([int]$XAtDeath) at the kill)"] = $XAtDeath -le $OrdersMoveTo[0] - 800
            }
            $XEnd = ($MoveMen | Where-Object { $Last.ContainsKey($_) } | ForEach-Object { "$([int]$Last[$_].X),$([int]$Last[$_].Y)" }) -join " "
            Write-Host "  attack-move: engaged in $Engaged reports, Spearman dead at report $DeathIndex of $($MoveReports.Count), $MoveHits hits, end $XEnd"
        }
    } else {
        $Checks["attack-move reports ($($MoveReports.Count))"] = $false
    }

    # Hold position (J7b.3): no chase of the enemy Peasant out of reach; the Spearman struck once in reach.
    $Hold = @(Get-Section "orders hold")
    $Reach = @(Get-Section "orders hold reach")
    $Checks["hold applied"] = [bool]($Hold | Select-String -Pattern "Hold order of slot 0 for 1 units")
    $HoldReports = Get-CombatReports $Hold
    $ReachReports = Get-CombatReports $Reach
    if ($HoldReports.Count -ge 3 -and $ReachReports.Count -ge 5) {
        $First = $HoldReports[0]
        $Holder = @($First.Keys | Where-Object { $First[$_].Type -eq "ManAtArms" -and $First[$_].Slot -eq 0 -and [Math]::Abs($First[$_].X - $OrdersHold[0]) -le 150 -and [Math]::Abs($First[$_].Y - $OrdersHold[1]) -le 150 })
        $Checks["the holder made ($($Holder.Count))"] = $Holder.Count -eq 1
        if ($Holder.Count -eq 1) {
            $Id = $Holder[0]
            $Moved = (@($HoldReports | Where-Object { $_.ContainsKey($Id) } | ForEach-Object { [Math]::Sqrt([Math]::Pow($_[$Id].X - $First[$Id].X, 2) + [Math]::Pow($_[$Id].Y - $First[$Id].Y, 2)) }) | Measure-Object -Maximum).Maximum
            $HoldTargets = @($HoldReports | Where-Object { $_.ContainsKey($Id) -and $_[$Id].Target -ne 0 }).Count
            $Checks["no chase of the Peasant out of reach (moved $([int]$Moved) cm, $HoldTargets reports with a target)"] = $Moved -le 30 -and $HoldTargets -eq 0
            # The Peasant out of reach (edge to edge over the Man-at-arms' 70 cm) but within the aggression's 16 m: a unit
            # not holding would go.
            $PeasantIds = @($First.Keys | Where-Object { $First[$_].Type -eq "Peasant" -and $First[$_].Slot -eq 1 -and [Math]::Abs($First[$_].X - $OrdersHoldPeasant[0]) -le 300 -and [Math]::Abs($First[$_].Y - $OrdersHoldPeasant[1]) -le 300 })
            $Gap = if ($PeasantIds.Count -eq 1) { [Math]::Sqrt([Math]::Pow($First[$PeasantIds[0]].X - $First[$Id].X, 2) + [Math]::Pow($First[$PeasantIds[0]].Y - $First[$Id].Y, 2)) - 120 } else { -1 }
            $Checks["the Peasant out of reach, within 16 m ($([int]$Gap) cm edge to edge)"] = $Gap -gt 70 -and $Gap -lt 1600
            $End = $ReachReports[-1]
            $HoldHits = if ($End.ContainsKey($Id)) { $End[$Id].Hits } else { -1 }
            $Checks["the attacker struck once in reach ($HoldHits hits)"] = $HoldHits -ge 2
            # Two enemies never push each other back (the user's trial of J7b): the Spearman walking into it does not move it.
            $Start = $ReachReports[0]
            $MovedReach = if ($Start.ContainsKey($Id)) { (@($ReachReports | Where-Object { $_.ContainsKey($Id) } | ForEach-Object { [Math]::Sqrt([Math]::Pow($_[$Id].X - $Start[$Id].X, 2) + [Math]::Pow($_[$Id].Y - $Start[$Id].Y, 2)) }) | Measure-Object -Maximum).Maximum } else { 999 }
            $Checks["still holding while it fights, not pushed (moved $([int]$MovedReach) cm)"] = $MovedReach -le 20
            Write-Host "  hold: moved $([int]$Moved) cm before the Spearman, $([int]$MovedReach) cm after, $HoldHits hits"
        }
    } else {
        $Checks["hold reports ($($HoldReports.Count), $($ReachReports.Count))"] = $false
    }

    # A queue after an ordered target (the review of J7b): A killed, B attacked from the queue and killed, then C taken on.
    $Queue = @(Get-Section "orders queue")
    $Seek = @(Get-Section "orders queue seek")
    $Checks["queue: A ordered, B queued"] = [bool]($Queue | Select-String -Pattern "Order: 3 units attack #\d+ Peasant of slot 1$") -and [bool]($Queue | Select-String -Pattern "Order: 3 units attack #\d+ Peasant of slot 1, queued")
    $QueueReports = Get-CombatReports $Queue
    $SeekReports = Get-CombatReports $Seek
    $Near = { param($R, $Point) @($R.Keys | Where-Object { $R[$_].Type -eq "Peasant" -and $R[$_].Slot -eq 1 -and [Math]::Abs($R[$_].X - $Point[0]) -le 300 -and [Math]::Abs($R[$_].Y - $Point[1]) -le 300 }) }
    if ($QueueReports.Count -ge 5 -and $SeekReports.Count -ge 3) {
        $IdA = @(& $Near $QueueReports[0] $QueueA); $IdB = @(& $Near $QueueReports[0] $QueueB); $IdC = @(); foreach ($R in $SeekReports) { $IdC = @(& $Near $R $QueueC); if ($IdC.Count -gt 0) { break } }
        $Checks["A, B and C made ($($IdA.Count), $($IdB.Count), $($IdC.Count))"] = $IdA.Count -eq 1 -and $IdB.Count -eq 1 -and $IdC.Count -eq 1
        if ($IdA.Count -eq 1 -and $IdB.Count -eq 1 -and $IdC.Count -eq 1) {
            $Checks["A killed"] = -not $QueueReports[-1].ContainsKey($IdA[0])
            $OnB = @($QueueReports | Where-Object { $R = $_; @($R.Values | Where-Object { $_.Type -eq "ManAtArms" -and $_.Slot -eq 0 -and $_.Target -eq $IdB[0] }).Count -gt 0 }).Count
            $Checks["B attacked from the queue once A is dead ($OnB reports)"] = $OnB -ge 1
            $Checks["B killed"] = -not $QueueReports[-1].ContainsKey($IdB[0])
            $Checks["C taken on by their own aggression afterwards"] = -not $SeekReports[-1].ContainsKey($IdC[0])
            Write-Host "  queue: A #$($IdA[0]), B #$($IdB[0]) targeted in $OnB reports, C #$($IdC[0])"
        }
    } else {
        $Checks["queue reports ($($QueueReports.Count), $($SeekReports.Count))"] = $false
    }

    # Two enemies never push each other back (the user's trial of J7b): the Spearman the 4 Peasants were ordered on, once
    # struck, stays where it is however they crowd it (it strikes back, standing).
    $AttackReports = Get-CombatReports $Attack
    # The Spearman: the one the ordered Peasants target (it may already have walked a few metres from its spawn towards
    # them: each step of the sequence is a frame of game time on the fixed time step, Docs/Archive/TestSpeedInProgress.md).
    $Spear = @(if ($AttackReports.Count -ge 1) {
        $Targets = @($AttackReports | ForEach-Object { $_.Values } | Where-Object { $_.Type -eq "Peasant" -and $_.Slot -eq 0 -and $_.Target -gt 0 } | ForEach-Object { "$($_.Target)" })
        $AttackReports[0].Keys | Where-Object { $AttackReports[0][$_].Type -eq "Spearman" -and $AttackReports[0][$_].Slot -eq 1 -and $Targets -contains "$_" } })
    $StruckIndex = [Array]::FindIndex([object[]]$AttackReports, [Predicate[object]]{ param($R) @($R.Values | Where-Object { $_.Type -eq "Peasant" -and $_.Slot -eq 0 -and $_.Hits -ge 1 }).Count -gt 0 })
    if ($Spear.Count -eq 1 -and $StruckIndex -ge 0) {
        $SpearId = $Spear[0]
        $From = $AttackReports[$StruckIndex][$SpearId]
        $Pushed = (@($AttackReports[$StruckIndex..($AttackReports.Count - 1)] | Where-Object { $_.ContainsKey($SpearId) } | ForEach-Object { [Math]::Sqrt([Math]::Pow($_[$SpearId].X - $From.X, 2) + [Math]::Pow($_[$SpearId].Y - $From.Y, 2)) }) | Measure-Object -Maximum).Maximum
        $Checks["the Spearman crowded by 4 Peasants not pushed (moved $([int]$Pushed) cm)"] = $Pushed -le 20
        Write-Host "  orders: the Spearman moved $([int]$Pushed) cm once struck (reports $StruckIndex to $($AttackReports.Count - 1))"
    } else {
        $Checks["the Spearman struck by the Peasants ($($Spear.Count) found, first hit at report $StruckIndex)"] = $false
    }
    # Two enemies never push each other back (the user's trial of J7b): the 12 standing Peasants the Men-at-arms walk
    # through, from the order on, do not move (the separation's correction all on the walkers); the target struck all the same.
    $PushReports = Get-CombatReports @(Get-Section "orders push")
    if ($PushReports.Count -ge 5) {
        $At = $PushReports[0]
        $Wall = @($At.Keys | Where-Object { $At[$_].Type -eq "Peasant" -and $At[$_].Slot -eq 1 -and [Math]::Abs($At[$_].X - $PushWall[0]) -le 450 -and [Math]::Abs($At[$_].Y - $PushWall[1]) -le 450 })
        $Target = @($At.Keys | Where-Object { $At[$_].Type -eq "Peasant" -and $At[$_].Slot -eq 1 -and [Math]::Abs($At[$_].X - $PushTarget[0]) -le 100 -and [Math]::Abs($At[$_].Y - $PushTarget[1]) -le 100 })
        $Men = @($At.Keys | Where-Object { $At[$_].Type -eq "ManAtArms" -and $At[$_].Slot -eq 0 -and [Math]::Abs($At[$_].X - $PushMenAtArms[0]) -le 450 -and [Math]::Abs($At[$_].Y - $PushMenAtArms[1]) -le 450 })
        $Checks["12 Peasants, their target and 3 Men-at-arms made ($($Wall.Count), $($Target.Count), $($Men.Count))"] = $Wall.Count -eq 12 -and $Target.Count -eq 1 -and $Men.Count -eq 3
        $WallMoved = (@(foreach ($R in $PushReports) { foreach ($P in $Wall) { if ($R.ContainsKey($P)) { [Math]::Sqrt([Math]::Pow($R[$P].X - $At[$P].X, 2) + [Math]::Pow($R[$P].Y - $At[$P].Y, 2)) } } }) | Measure-Object -Maximum).Maximum
        $Checks["the standing enemies not pushed by the walkers (at most $([int]$WallMoved) cm)"] = $WallMoved -le 20
        $End = $PushReports[-1]
        $PushHits = ($Men | Where-Object { $End.ContainsKey($_) } | ForEach-Object { $End[$_].Hits } | Measure-Object -Sum).Sum
        $Struck = if ($Target.Count -eq 1) { -not $End.ContainsKey($Target[0]) -or $End[$Target[0]].Hp -lt 50 } else { $false }
        $Checks["the target struck through them ($PushHits hits)"] = $Struck
        Write-Host "  push: the 12 standing Peasants moved $([int]$WallMoved) cm at most, Men-at-arms hits $PushHits"
    } else {
        $Checks["push reports ($($PushReports.Count))"] = $false
    }
    $AllOk = (Report "orders" $Checks) -and $AllOk
}

if ($Parts -contains "net") {
    # The lines of a client's section, from its mark to the next one ("net", then "gather").
    function Get-ClientSection($Lines, [string] $Mark) {
        $Inside = $false
        foreach ($Line in $Lines) {
            if ($Line -match "Test mark: (.*)$") { $Inside = $Matches[1] -eq $Mark; continue }
            if ($Inside) { $Line }
        }
    }
    $Checks = [ordered]@{}
    $Checks["server listening"] = $NetListening
    $HeldSlots = @()
    $NetHeld = @{}
    $NetSelectedTrees = @{}
    $NetPopups = @{}
    foreach ($NetClient in $NetClients) {
        # Up to 300 s from the client's own launch (2026-10-27): 120 s from here killed a client launched late on a busy
        # machine (Turnkey slow) while it quit, after a good match.
        $ClientLeft = [Math]::Max(0, [int]($NetClient.Process.StartTime.AddSeconds(300) - (Get-Date)).TotalMilliseconds)
        $ClientTimedOut = -not $NetClient.Process.WaitForExit($ClientLeft)
        if ($ClientTimedOut) { $NetClient.Process.Kill() }
        $N = $NetClient.Client
        $ClientLines = if (Test-Path $NetClient.Log) { Get-Content $NetClient.Log } else { @() }
        $Checks["client $N : sequence done"] = (-not $ClientTimedOut) -and [bool]($ClientLines | Select-String -Pattern "Test mark: end" -Quiet)
        $Held = $ClientLines | Select-String -Pattern "Camera above the start of slot (\d+) \(" | Select-Object -First 1 | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }
        $Checks["client $N : holds a slot ($Held)"] = $null -ne $Held
        if ($null -eq $Held) { continue }
        $HeldSlots += $Held
        # R3.2: the slot of its name (client 1 Bob in slot 1, client 2 Alice in slot 0), its name sent to the server.
        $NetName = @("Bob", "Alice")[$N - 1]
        $Checks["client $N : holds the slot of $NetName ($Held)"] = $Held -eq (2 - $N)
        $Checks["client $N : logged in as $NetName"] = (Test-Path $NetServerLog) -and [bool](Select-String -Path $NetServerLog -Pattern "Login request: .*[?&]Name=$NetName(\?|&|\s|$)" -Quiet) -and
            [bool](Select-String -Path $NetServerLog -Pattern "$NetName holds slot $(2 - $N) " -Quiet)
        $NetHeld[$N] = $Held
        $Camera =$ClientLines | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), (-?\d+)\)" | Select-Object -First 1
        $Start = Get-SlotStart $Held
        $Checks["client $N : camera above slot $Held"] = $Camera -and ([Math]::Abs([int]$Camera.Matches[0].Groups[1].Value - $Start[0]) -le 2) -and
            ([Math]::Abs([int]$Camera.Matches[0].Groups[2].Value - $Start[1]) -le 2)
        $Buildings = @($ClientLines | Select-String -Pattern "Building report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $Checks["client $N : both centres, each in its colour ($($Buildings -join '; '))"] = ($Buildings.Count -eq 3) -and
            ($Buildings -contains "MainCentre of slot 0, colour 0 shown") -and ($Buildings -contains "MainCentre of slot 1, colour 1 shown") -and
            ($Buildings -contains "2 buildings")
        # The first three (the army's comes later, J7a.6).
        $Selection = @($ClientLines | Select-String -Pattern "Selection report: (.*)$" | Select-Object -First 3 | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $Wanted = @(0, 1 | ForEach-Object { "selected MainCentre of slot $_ ($(if ($_ -eq $Held) { 'own' } else { 'other' }))" }) + "selected none"
        $Checks["client $N : clicks on the two centres, then the void ($($Selection -join '; '))"] = ($Selection -join '|') -eq ($Wanted -join '|')
        $NetLines = Get-ClientSection $ClientLines "net"
        $Production = $NetLines | Select-String -Pattern "Production report: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
        $Checks["client $N : own Peasant paid and counted ($Production)"] = [bool]($Production -match "^F 150 W 200 S 200 G 200, population 1/10, .*queue 0 ")
        $Checks["client $N : other player's economy not received"] = [bool]($Production -match "; others \[slot $(1 - $Held) F 0 W 0 S 0 G 0, population 0/0\]$")
        $Shown = $NetLines | Select-String -Pattern "Units report: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
        $Checks["client $N : both Peasants shown ($Shown)"] = $Shown -eq "client, 2 displayed: Peasant of slot 0 x1, Peasant of slot 1 x1"
        # EtapeDefini N.1: the rally point's two steps replicated to their owner (the flag at P2, two segments, a point at P1).
        $NetRallyStart = Get-SlotStart $Held
        $NetRallySign = if ($NetRallyStart[0] -lt 0) { 1 } else { -1 }
        $NetRallyP2 = @(($NetRallyStart[0] + $NetRallySign * 1600), ($NetRallyStart[1] + 4000))
        $NetRallySay = @($ClientLines | Select-String -Pattern "Selection markers: (.*flag 1 at .*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value } |
            Where-Object { $_ -match "flag 1 at \((-?\d+), (-?\d+)\)" -and [Math]::Abs([int]$Matches[1] - $NetRallyP2[0]) -le 60 -and [Math]::Abs([int]$Matches[2] - $NetRallyP2[1]) -le 60 })
        $Checks["client $N : its centre's rally point in two steps received (Shift + right click): the flag at P2, two segments, a point at P1 ($(@($NetRallySay)[0]))"] =
            ($NetRallySay.Count -ge 1) -and [bool]($NetRallySay[0] -match "order lines 2, order points 1")
        $Missing = @("start", "slot0", "slot1", "peasants" | Where-Object { -not (Test-Path (Join-Path $NetClient.Shots "qcnet_${N}_$_.png")) })
        $Checks["client $N : captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
        if ($Parts -contains "sources") {
            # 5 wood left by the server in the tree nearest each start: 31900 - 2 x 95.
            $Sources = $NetLines | Select-String -Pattern "Sources \(client\): (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
            $Checks["client $N : 376 sources received, the used-up tree gone ($Sources)"] = $Sources -eq "375 standing of 376 (Tree 319: 31710 left, BerryBush 48: 9600 left, StoneVein 4: 10000 left, GoldVein 4: 10000 left); shown 375, navmesh 0"
            $Checks["client $N : saw the tree used up"] = [bool]($ClientLines | Select-String -Pattern "Sources: source 0 used up \(client\)" -Quiet)

            # Gathering (J5a.5, D299, D303).
            $Gather = @(Get-ClientSection $ClientLines "gather")
            $Other = 1 - $Held
            $Clicked = $Gather | Select-String -Pattern "Right click on source (\d+)" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
            $Checks["client $N : its Peasant alone, right click on a tree (source $Clicked)"] = [bool]($Gather | Select-String -Pattern "Selection: Peasant #\d+ of slot $Held \(own\)" -Quiet) -and ($null -ne $Clicked)
            $Loads = @($Gather | Select-String -Pattern "HUD report: .*, EHOLUnitActivity::(\w+) EHOLResource::Wood (\d+)/10" | ForEach-Object { "$($_.Matches[0].Groups[1].Value -replace 'Gathering', 'G' -replace 'Moving', 'M' -replace 'Idle', 'I') $($_.Matches[0].Groups[2].Value)" })
            $Checks["client $N : HUD shows its exact load in real time, D303 ($($Loads -join ', '))"] =
                @($Loads | Where-Object { $_ -match "^G [1-9]" }).Count -ge 2
            $OwnKnown = [bool]($Gather | Select-String -Pattern "slot $Held at \(-?\d+, -?\d+\) \w+, carries Wood \d+" -Quiet)
            $Checks["client $N : its own Peasant's load known here"] = $OwnKnown
            $Checks["client $N : the other's Peasant seen gathering and carrying"] = [bool]($Gather | Select-String -Pattern "slot $Other at \(-?\d+, -?\d+\) Gathering" -Quiet) -and
                [bool]($Gather | Select-String -Pattern "slot $Other at \(-?\d+, -?\d+\) \w+, carries Wood \?" -Quiet)
            # J5c.1: what it gathers comes with its activity (the axe on this client), carrying or not yet.
            $Checks["client $N : the other's Peasant seen gathering wood (resource in the activity's bits)"] =
                [bool]($Gather | Select-String -Pattern "slot $Other at \(-?\d+, -?\d+\) Gathering, gathers Wood" -Quiet) -and
                [bool]($Gather | Select-String -Pattern "slot $Other at \(-?\d+, -?\d+\) Gathering, carries Wood \?, gathers Wood" -Quiet) -and
                -not ($Gather | Select-String -Pattern "slot $Other at \(-?\d+, -?\d+\) Gathering(, carries \w+ [\d?]+)?, gathers (Food|Stone|Gold)" -Quiet)
            $Checks["client $N : the other's load never received"] = -not ($Gather | Select-String -Pattern "slot $Other at \(-?\d+, -?\d+\) \w+, carries \w+ \d" -Quiet)
            $UsedUp = @($Gather | Select-String -Pattern "Sources: source (\d+) used up \(client\)" | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique)
            $Final = $Gather | Select-String -Pattern "Sources \(client\): (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
            $Checks["client $N : both gathered trees used up, gone here ($($UsedUp -join ', '); $Final)"] = ($UsedUp.Count -eq 2) -and
                [bool]($Final -match "^373 standing of 376 .*; shown 373, navmesh 0$")
            $Stock = $Gather | Select-String -Pattern "Production report: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
            $Checks["client $N : its wood rose at the deposit, the other's stock never received ($Stock)"] =
                [bool]($Stock -match "^F 150 W 2[1-9]0 S 200 G 200, ") -and [bool]($Stock -match "; others \[slot $Other F 0 W 0 S 0 G 0, population 0/0\]$")
            $Checks["client $N : gather capture"] = Test-Path (Join-Path $NetClient.Shots "qcnet_${N}_gather.png")

            # J5b.3 (D307): the HUD counts this player's Peasants alone (the other's, idle then gathering, never counted).
            $Huds = @($Gather | Select-String -Pattern "HUD report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
            $Checks["client $N : HUD before the order: idle 1, no gatherer ($(if ($Huds.Count) { [regex]::Match($Huds[0], 'gatherers .*$').Value }))"] =
                ($Huds.Count -ge 1) -and [bool]($Huds[0] -match ", gatherers F 0 W 0 S 0 G 0, idle 1$")
            $Counts = @($Huds | Select-Object -Skip 1 | ForEach-Object { [regex]::Match($_, 'gatherers .*$').Value } | Sort-Object -Unique)
            $Checks["client $N : HUD after: its one wood gatherer, none idle ($($Counts -join '; '))"] = ($Huds.Count -eq 17) -and
                (($Counts -join '|') -eq "gatherers F 0 W 1 S 0 G 0, idle 0")
            # The tree nearest slot 0's start selected on both clients (one gathers it, the other sees it): what is left goes down.
            $Remaining = @($Gather | Select-String -Pattern "HUD report: .*, selection Tree source (\d+), (\d+) left," | ForEach-Object { , @([int]$_.Matches[0].Groups[1].Value, [int]$_.Matches[0].Groups[2].Value) })
            $Amounts = @($Remaining | ForEach-Object { $_[1] })
            $NonIncreasing = $true; for ($i = 1; $i -lt $Amounts.Count; $i++) { if ($Amounts[$i] -gt $Amounts[$i - 1]) { $NonIncreasing = $false } }
            $Picked = @($Remaining | ForEach-Object { $_[0] } | Sort-Object -Unique)
            $NetSelectedTrees[$N] = $Picked -join ','
            $Checks["client $N : slot 0's tree selected, 'Reste' going down (source $($Picked -join ', '): $($Amounts -join ', '))"] =
                ($Picked.Count -eq 1) -and ($Amounts.Count -ge 2) -and $NonIncreasing -and ($Amounts[-1] -lt $Amounts[0])
            # J5c.3 (D298): its own unloads' +10 at its centre, never the other's (sent to the owner alone).
            $Popups = @($Gather | Select-String -Pattern "Deposit popup: \+(\d+) (\w+) at \((-?\d+), (-?\d+)\)" | ForEach-Object { $_.Matches[0] })
            $OwnStart = Get-SlotStart $Held; $OtherStart = Get-SlotStart $Other
            $NearStart = { param($M, $At) [Math]::Sqrt([Math]::Pow([double]$M.Groups[3].Value - $At[0], 2) + [Math]::Pow([double]$M.Groups[4].Value - $At[1], 2)) -le 1500 }
            $NearOwn = @($Popups | Where-Object { & $NearStart $_ $OwnStart })
            $NearOther = @($Popups | Where-Object { & $NearStart $_ $OtherStart })
            $NetPopups[$N] = @{ Slot = $Held; Count = $Popups.Count }
            $Checks["client $N : +10 of its own unloads at its centre, none of the other's ($(($Popups | ForEach-Object { "+$($_.Groups[1].Value) $($_.Groups[2].Value)" }) -join ', '); $($NearOther.Count) at the other's)"] =
                ($Popups.Count -ge 1) -and ($NearOwn.Count -eq $Popups.Count) -and ($NearOther.Count -eq 0) -and
                (@($Popups | Where-Object { $_.Groups[1].Value -ne "10" -or $_.Groups[2].Value -ne "Wood" }).Count -eq 0)
            $Checks["client $N : the other's Peasant seen on its round trip (GatherTrip)"] = [bool]($Gather | Select-String -Pattern "slot $Other at \(-?\d+, -?\d+\) GatherTrip" -Quiet)
        }
        if ($Parts -contains "build") {
            # Placement (J6a.3): both foundations seen here, its own wood 50 less, the other's stock never received; the
            # house near the enemy centre refused (nothing taken).
            $BuildLines = @(Get-ClientSection $ClientLines "build")
            $Other = 1 - $Held
            $Behind = { param($Slot, $Distance) $At = Get-SlotStart $Slot; @(($At[0] + $(if ($At[0] -lt 0) { -$Distance } else { $Distance })), $At[1]) }
            $OwnHouse = Get-NearSpot $Held $NetHouseNear[0] $NetHouseNear[1] 2
            $OtherHouse = Get-NearSpot $Other $NetHouseNear[0] $NetHouseNear[1] 2
            $Reports = @($BuildLines | Select-String -Pattern "Build report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
            $Stocks = @($BuildLines | Select-String -Pattern "Production report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
            $Woods = @($Stocks | ForEach-Object { if ($_ -match "^F \d+ W (\d+) ") { [int]$Matches[1] } else { -1 } })
            # J6a.4: the other's foundation is a ghost for its owner alone, received here at its first work only.
            $Checks["client $N : its own foundation seen, not the other's ($(if ($Reports.Count) { $Reports[0] }))"] = ($Reports.Count -eq 3) -and
                [bool]($Reports[0] -match "3 buildings") -and [bool]($Reports[0] -match "House of slot $Held \(own\) (foundation|under construction \d+%) at \($($OwnHouse -join ', ')\)") -and
                -not ($Reports[0] -match "House of slot $Other")
            $Arrived = @($ClientLines | Select-String -Pattern "Building arrived: House of slot $Other, (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
            $Checks["client $N : the other's site received at its first work ($($Arrived -join ', '); $(if ($Reports.Count -ge 2) { $Reports[1] }))"] = ($Arrived -join ',') -eq "under construction" -and
                ($Reports.Count -ge 2) -and [bool]($Reports[1] -match "4 buildings") -and [bool]($Reports[1] -match "House of slot $Other \(other\) (under construction \d+%|finished) at \($($OtherHouse -join ', ')\)")
            $Checks["client $N : its wood 50 less, then unchanged by the refusal ($($Woods -join ' -> '))"] = ($Woods.Count -eq 3) -and ($Woods[0] -ge 50) -and
                ($Woods[1] -eq $Woods[0] - 50) -and ($Woods[2] -eq $Woods[1])
            $Checks["client $N : the other's stock never received"] = ($Stocks.Count -eq 3) -and
                (@($Stocks | Where-Object { $_ -notmatch "; others \[slot $Other F 0 W 0 S 0 G 0, population 0/0\]$" }).Count -eq 0)
            $Checks["client $N : no foundation near the enemy centre ($(if ($Reports.Count -ge 3) { $Reports[2] }))"] = ($Reports.Count -eq 3) -and [bool]($Reports[2] -match "4 buildings")
            $Missing = @("build0", "build1", "site0", "site1" | Where-Object { -not (Test-Path (Join-Path $NetClient.Shots "qcnet_${N}_$_.png")) })
            $Checks["client $N : build captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
            # The camps (J6b.3): its camp paid (100 wood), its +10 at its camp; the other's camp received at its first work,
            # then both finished here; the camp on a tree refused (nothing taken); the other's stock never received.
            $CampLines = @(Get-ClientSection $ClientLines "camp")
            $CampAt = { param($Slot) Get-NearSpot $Slot $NetCampNear[0] $NetCampNear[1] 2 }
            $OwnCamp = & $CampAt $Held; $OtherCamp = & $CampAt $Other
            $CampReports = @($CampLines | Select-String -Pattern "Build report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
            $CampStocks = @($CampLines | Select-String -Pattern "Production report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
            $CampWoods = @($CampStocks | ForEach-Object { if ($_ -match "^F \d+ W (\d+) ") { [int]$Matches[1] } else { -1 } })
            $CampPopups = @($CampLines | Select-String -Pattern "Deposit popup: \+\d+ Wood at \((-?\d+), (-?\d+)\)" | ForEach-Object { [Math]::Sqrt([Math]::Pow([double]$_.Matches[0].Groups[1].Value - $OwnCamp[0], 2) + [Math]::Pow([double]$_.Matches[0].Groups[2].Value - $OwnCamp[1], 2)) })
            $Checks["client $N : its camp paid 100 wood, then the camp on a tree takes nothing ($($CampWoods -join ' -> '))"] = ($Woods.Count -eq 3) -and ($CampWoods.Count -eq 3) -and
                ($CampWoods[0] -eq $Woods[2] - 100) -and ($CampWoods[2] -ge $CampWoods[1])
            $Checks["client $N : its +10 at its camp ($(($CampPopups | ForEach-Object { [Math]::Round($_) }) -join ', ') cm off)"] = ($CampPopups.Count -ge 1) -and
                (@($CampPopups | Where-Object { $_ -gt 1000 }).Count -eq 0)
            $CampArrived = @($CampLines | Select-String -Pattern "Building arrived: LumberCamp of slot $Other, (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
            $Checks["client $N : the other's camp received at its first work ($($CampArrived -join ', '))"] = ($CampArrived -join ',') -eq "under construction"
            $Checks["client $N : both camps finished here ($(if ($CampReports.Count) { $CampReports[0] }))"] = ($CampReports.Count -eq 2) -and
                [bool]($CampReports[0] -match "LumberCamp of slot $Held \(own\) finished at \($($OwnCamp -join ', ')\)") -and
                [bool]($CampReports[0] -match "LumberCamp of slot $Other \(other\) finished at \($($OtherCamp -join ', ')\)")
            $Checks["client $N : no camp on the tree ($(if ($CampReports.Count -ge 2) { $CampReports[1] }))"] = ($CampReports.Count -eq 2) -and
                ([regex]::Matches($CampReports[1], "LumberCamp of slot").Count -eq 2)
            $Checks["client $N : the other's stock never received, camps included"] = ($CampStocks.Count -eq 3) -and
                (@($CampStocks | Where-Object { $_ -notmatch "; others \[slot $Other F 0 W 0 S 0 G 0, population 0/0\]$" }).Count -eq 0)
        }
        # The army (J7a.6): both barracks' soldiers shown with their types, the other's drawn with its mesh and colour;
        # control group 1 made of its 5 units and selected by its digit.
        $ArmyLines = @(Get-ClientSection $ClientLines "army")
        $Other = 1 - $Held
        # The order lines (D328): client 1's chain received by it alone; client 2 none before its own order (client 1's
        # under way), then its own alone. Each chain: one Move of its 5 units.
        $NetChains = Get-ChainReports $ArmyLines -Received
        $OwnChain = { param($Report) ($null -ne $Report) -and ($Report.Slot -eq $Held) -and ((Format-ChainReport $Report) -eq "Move (5 units)") }
        if ($N -eq 1) {
            $Checks["client 1 : its march's chain received, its own alone ($(Format-ChainReport $NetChains[0]))"] = ($NetChains.Count -eq 1) -and (& $OwnChain $NetChains[0])
        } else {
            $Checks["client 2 : no chain received while client 1's is under way (slot $(if ($NetChains.Count) { $NetChains[0].Slot }): $(if ($NetChains.Count) { $NetChains[0].Count }))"] =
                ($NetChains.Count -eq 2) -and ($NetChains[0].Slot -eq $Held) -and ($NetChains[0].Count -eq 0)
            $Checks["client 2 : then its own chain alone ($(Format-ChainReport $NetChains[-1]))"] = ($NetChains.Count -eq 2) -and (& $OwnChain $NetChains[1])
        }
        $ArmyShown = $ArmyLines | Select-String -Pattern "Units report: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
        $Checks["client $N : both armies shown with their types ($ArmyShown)"] = $ArmyShown -eq "client, 10 displayed: ManAtArms of slot 0 x2, ManAtArms of slot 1 x2, Peasant of slot 0 x1, Peasant of slot 1 x1, Spearman of slot 0 x2, Spearman of slot 1 x2"
        $Looks = $ArmyLines | Select-String -Pattern "Units looks: client, (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
        $OtherLooks = @(foreach ($Type in "Spearman", "ManAtArms") { [regex]::Match("$Looks", "$Type of slot $Other skinned (\w+) colour (\d+) x2") })
        $Checks["client $N : the other's soldiers drawn, their meshes, the other's colour ($Looks)"] = (@($OtherLooks | Where-Object { $_.Success -and $_.Groups[2].Value -eq "$Other" }).Count -eq 2) -and
            ($OtherLooks[0].Groups[1].Value -ne $OtherLooks[1].Groups[1].Value)
        $Checks["client $N : control group 1 made of its 5 units, its digit selects them"] = [bool]($ArmyLines | Select-String -Pattern "Group 1: created" -Quiet) -and
            [bool]($ArmyLines | Select-String -Pattern "Group 1: selected, 5 units" -Quiet) -and
            [bool]($ArmyLines | Select-String -Pattern "Selection report: selected 5 units \(own\): ManAtArms x2, Peasant x1, Spearman x2" -Quiet)
        $Missing = @("qcnet_${N}_army_other", "net_army_c$N" | Where-Object { -not (Test-Path (Join-Path $NetClient.Shots "$_.png")) })
        $Checks["client $N : army captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
        $ClientErrors = @($ClientLines | Select-String -Pattern "LogHOLMatch: (Error|Warning)|LogHOLNet: (Error|Warning)|LogHOLTest: Error")
        $Checks["client $N : no match, network or sequence error ($($ClientErrors.Count))"] = $ClientErrors.Count -eq 0
    }
    if ($Parts -contains "move") {
        # Each client sees both Peasants beside their owners' starts (D288): its own order obeyed, the other client's too.
        foreach ($NetClient in $NetClients) {
            $N = $NetClient.Client
            $Where = (Get-ClientSection (Get-Content $NetClient.Log -ErrorAction SilentlyContinue) "net") | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
            foreach ($Slot in 0, 1) {
                $Start = Get-SlotStart $Slot
                $Seen = [regex]::Match("$Where", "slot $Slot at \((-?\d+), (-?\d+)\)")
                $Gap = if ($Seen.Success) { [Math]::Sqrt([Math]::Pow([int]$Seen.Groups[1].Value - ($Start[0] - 1000), 2) + [Math]::Pow([int]$Seen.Groups[2].Value - ($Start[1] + 2500), 2)) } else { -1 }
                $Checks["client $N : Peasant of slot $Slot where its owner sent it ($([Math]::Round($Gap)) cm off)"] = ($Gap -ge 0) -and ($Gap -le 150)
            }
        }
        $Refused = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Move order of slot (\d): 1 units ignored, not its own" | ForEach-Object { $_.Matches[0].Groups[1].Value } })
        $Checks["server: each player's order left the other's Peasant alone ($($Refused -join ', '))"] = (($Refused | Sort-Object) -join ',') -eq "0,1"
    }
    # The army's march (J7a.6): each client's 10 units where the server has them (the server's last report with them
    # all), each slot's 5 around its right click (frame (0, 30 m) of its centre), no two of a slot overlapping.
    $ServerWheres = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Units where: \d+ units: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value } })
    $ParseWhere = { param($Text) $Map = @{}; foreach ($M in [regex]::Matches([string]$Text, "#(\d+) slot (\d) at \((-?\d+), (-?\d+)\)")) { $Map[$M.Groups[1].Value] = @([int]$M.Groups[2].Value, [double]$M.Groups[3].Value, [double]$M.Groups[4].Value) }; $Map }
    foreach ($NetClient in $NetClients) {
        $N = $NetClient.Client
        $ArmyLines = @(Get-ClientSection (Get-Content $NetClient.Log -ErrorAction SilentlyContinue) "army")
        $Seen = & $ParseWhere ($ArmyLines | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $Server = $null
        foreach ($Report in $ServerWheres) { $Map = & $ParseWhere $Report; if ($Seen.Count -ge 10 -and @($Seen.Keys | Where-Object { -not $Map.ContainsKey($_) }).Count -eq 0) { $Server = $Map } }
        $Gaps = @(if ($Server) { foreach ($Id in $Seen.Keys) { [Math]::Sqrt([Math]::Pow($Seen[$Id][1] - $Server[$Id][1], 2) + [Math]::Pow($Seen[$Id][2] - $Server[$Id][2], 2)) } })
        $MaxGap = if ($Gaps.Count) { [Math]::Round(($Gaps | Measure-Object -Maximum).Maximum) } else { -1 }
        $Checks["client $N : the 10 units where the server has them ($($Seen.Count) seen, at most $MaxGap cm off)"] = ($Seen.Count -eq 10) -and ($Gaps.Count -eq 10) -and ($MaxGap -ge 0) -and ($MaxGap -le 100)
        foreach ($Slot in 0, 1) {
            $Ids = @($Seen.Keys | Where-Object { $Seen[$_][0] -eq $Slot })
            $Closest = [double]::MaxValue
            for ($i = 0; $i -lt $Ids.Count; $i++) { for ($j = $i + 1; $j -lt $Ids.Count; $j++) {
                $Closest = [Math]::Min($Closest, [Math]::Sqrt([Math]::Pow($Seen[$Ids[$i]][1] - $Seen[$Ids[$j]][1], 2) + [Math]::Pow($Seen[$Ids[$i]][2] - $Seen[$Ids[$j]][2], 2))) } }
            $Target = @(if ($Slot -eq 0) { -15000, 3000 } else { 15000, -3000 })
            $Far = if ($Ids.Count) { @($Ids | ForEach-Object { [Math]::Sqrt([Math]::Pow($Seen[$_][1] - $Target[0], 2) + [Math]::Pow($Seen[$_][2] - $Target[1], 2)) } | Measure-Object -Maximum).Maximum } else { -1 }
            $Checks["client $N : slot $Slot's 5 units in rows at the click ($([Math]::Round($Far)) cm at most from it, closest pair $([Math]::Round($Closest)) cm)"] =
                ($Ids.Count -eq 5) -and ($Far -le 600) -and ($Closest -ge 100)
        }
    }
    # The fight seen by client 1 (J7b.4): the 4 units the server made, their health falling (never rising), each value one
    # the server's hits left (J7-Q5 revised: exact health; 0 for a unit dead, whose last hit left 0 or less) and the
    # Men-at-arms' last one exactly the server's last; a unit striking one of the other slot (Attacking, target #id), the
    # Spearmen Dying then gone, the Men-at-arms left where the server has them; the HUD shows the attack of its selected unit.
    $FightClient = $NetClients | Where-Object { $_.Client -eq 1 } | Select-Object -First 1
    if ($FightClient) {
        $ClientLines = @(Get-Content $FightClient.Log -ErrorAction SilentlyContinue)
        $Before = & $ParseWhere (@(Get-ClientSection $ClientLines "army") | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $FightLines = @(Get-ClientSection $ClientLines "combat")
        $Wheres = @($FightLines | Select-String -Pattern "Units where: \d+ units: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        # The server's hits (hol.Combat.LogHits): id -> the health each hit left it, in order.
        $ServerHp = @{}
        if (Test-Path $NetServerLog) {
            foreach ($M in (Select-String -Path $NetServerLog -Pattern "Combat: step \d+ #\d+ \w+ hits #(\d+) \w+ for [\d.]+, (-?[\d.]+) HP left")) {
                $Hit = $M.Matches[0].Groups
                if (-not $ServerHp.ContainsKey($Hit[1].Value)) { $ServerHp[$Hit[1].Value] = @() }
                $ServerHp[$Hit[1].Value] += [int][Math]::Ceiling([double]$Hit[2].Value)
            }
        }
        $Unhurt = 1000000
        $IsServerHp = { param($Id, $Hp) $Hp -eq $Unhurt -or ($ServerHp.ContainsKey($Id) -and (($Hp -gt 0 -and $ServerHp[$Id] -contains $Hp) -or ($Hp -eq 0 -and @($ServerHp[$Id] | Where-Object { $_ -le 0 }).Count -gt 0))) }
        $Fighters = @{}   # id -> @{ Slot; Levels (health shown, $Unhurt unhurt); Dying; Attacks (targets of the other slot) }
        $LastSeen = @{}
        foreach ($Report in $Wheres) {
            $LastSeen = @{}
            foreach ($Entry in ($Report -split "; ")) {
                if ($Entry -notmatch "^#(\d+) slot (\d) at \((-?\d+), (-?\d+)\) (\w+)") { continue }
                $Id = $Matches[1]; $Slot = [int]$Matches[2]; $X = [double]$Matches[3]; $Y = [double]$Matches[4]; $Activity = $Matches[5]
                $LastSeen[$Id] = @($Slot, $X, $Y)
                if ($Before.ContainsKey($Id)) { continue }
                if (-not $Fighters.ContainsKey($Id)) { $Fighters[$Id] = @{ Slot = $Slot; Levels = @(); Dying = $false; Targets = @() } }
                $Level = if ($Entry -match ", hp (\d+)/\d+") { [int]$Matches[1] } else { $Unhurt }
                $Fighters[$Id].Levels += $Level
                if ($Activity -eq "Dying") { $Fighters[$Id].Dying = $true }
                if ($Activity -eq "Attacking" -and $Entry -match ", target #(\d+)") { $Fighters[$Id].Targets += $Matches[1] }
            }
        }
        $Ids = @($Fighters.Keys)
        $Checks["client 1 : the fight's 4 units shown ($($Ids.Count), $($Wheres.Count) reports)"] = $Ids.Count -eq 4
        $Rising = @($Ids | Where-Object { $L = $Fighters[$_].Levels; @(for ($k = 1; $k -lt $L.Count; $k++) { if ($L[$k] -gt $L[$k - 1]) { 1 } }).Count -gt 0 })
        $Hurt = @($Ids | Where-Object { ($Fighters[$_].Levels | Measure-Object -Minimum).Minimum -lt $Unhurt })
        $LevelText = ($Ids | Sort-Object { [int]$_ } | ForEach-Object { "#$_ slot $($Fighters[$_].Slot): $(($Fighters[$_].Levels | Select-Object -Unique | ForEach-Object { if ($_ -eq $Unhurt) { 'full' } else { $_ } }) -join '>') (server: $((@('full') + @(if ($ServerHp.ContainsKey($_)) { $ServerHp[$_] | ForEach-Object { [Math]::Max($_, 0) } })) -join '>'))" }) -join "; "
        $Checks["client 1 : health falls, never rises ($($Hurt.Count) hurt, $($Rising.Count) rising)"] = ($Hurt.Count -ge 3) -and ($Rising.Count -eq 0)
        $Foreign = @(foreach ($Id in $Ids) { foreach ($Hp in ($Fighters[$Id].Levels | Select-Object -Unique)) { if (-not (& $IsServerHp $Id $Hp)) { "#$Id $Hp" } } })
        $ShownCount = @(foreach ($Id in $Ids) { $Fighters[$Id].Levels | Where-Object { $_ -ne $Unhurt } | Select-Object -Unique }).Count
        $Checks["client 1 : each health shown one the server's hits left ($ShownCount values, $($ServerHp.Count) units hit on the server, not the server's: $($Foreign -join ', '))"] = ($ServerHp.Count -ge 3) -and ($ShownCount -ge 3) -and ($Foreign.Count -eq 0)
        $Aimed = @($Ids | Where-Object { $F = $Fighters[$_]; @($F.Targets | Where-Object { $Fighters.ContainsKey($_) -and $Fighters[$_].Slot -ne $F.Slot }).Count -gt 0 })
        $Checks["client 1 : units striking one of the other slot ($($Aimed.Count) seen Attacking)"] = $Aimed.Count -ge 2
        $Spears = @($Ids | Where-Object { $Fighters[$_].Slot -eq 1 })
        $Mens = @($Ids | Where-Object { $Fighters[$_].Slot -eq 0 })
        $DyingSeen = @($Spears | Where-Object { $Fighters[$_].Dying })
        $Checks["client 1 : the Spearmen seen Dying ($($DyingSeen.Count) of $($Spears.Count))"] = ($Spears.Count -eq 2) -and ($DyingSeen.Count -ge 1)
        $Gone = @($Spears | Where-Object { -not $LastSeen.ContainsKey($_) })
        $Left = @($Mens | Where-Object { $LastSeen.ContainsKey($_) })
        $Checks["client 1 : then removed, the Men-at-arms left ($($Gone.Count) gone, $($Left.Count) left)"] = ($Gone.Count -eq 2) -and ($Left.Count -eq 2)
        $Finals = @(foreach ($Id in $Left) { $LastShown = $Fighters[$Id].Levels[-1]; $LastSent = if ($ServerHp.ContainsKey($Id)) { $ServerHp[$Id][-1] } else { $Unhurt }
            @{ Ok = $LastShown -eq $LastSent; Text = "#$Id $(if ($LastShown -eq $Unhurt) { 'full' } else { $LastShown }) (server $(if ($LastSent -eq $Unhurt) { 'full' } else { $LastSent }))" } })
        $Checks["client 1 : the Men-at-arms' last health the server's exactly ($(($Finals | ForEach-Object { $_.Text }) -join ', '))"] = ($Finals.Count -eq 2) -and (@($Finals | Where-Object { -not $_.Ok }).Count -eq 0)
        $Server = $null
        foreach ($Report in $ServerWheres) { $Map = & $ParseWhere $Report; if ($Left.Count -and @($Left | Where-Object { -not $Map.ContainsKey($_) }).Count -eq 0 -and @($Spears | Where-Object { $Map.ContainsKey($_) }).Count -eq 0) { $Server = $Map } }
        $Gaps = @(if ($Server) { foreach ($Id in $Left) { [Math]::Sqrt([Math]::Pow($LastSeen[$Id][1] - $Server[$Id][1], 2) + [Math]::Pow($LastSeen[$Id][2] - $Server[$Id][2], 2)) } })
        $MaxGap = if ($Gaps.Count) { [Math]::Round(($Gaps | Measure-Object -Maximum).Maximum) } else { -1 }
        $Checks["client 1 : the Men-at-arms where the server has them (at most $MaxGap cm off)"] = ($Gaps.Count -eq 2) -and ($MaxGap -ge 0) -and ($MaxGap -le 100)
        $HudAttack = [bool]($FightLines | Select-String -SimpleMatch "EHOLUnitActivity::Attacking" -Quiet)
        $Checks["client 1 : the HUD shows its selected unit's attack"] = $HudAttack
        # The health bars on client 1 (J7b.5): its selected unit's bar, and others only for units wounded in the Units where
        # just before (health never rises); each bar's health one the server's hits left, or the maximum (J7-Q5 revised).
        # A unit hit between the Units where and the bars (the hit arrived in between, a few ms) is wounded in the next Units
        # where: an unselected bar is wrong only for a unit unhurt in both (seen 2026-10-28: #13 hit 10 ms after the report).
        $BarRuns = @(); $WoundedNow = @{}; $Pending = @()
        foreach ($Line in $FightLines) {
            if ($Line -match "Units where: \d+ units: (.*)$") {
                $WoundedNow = @{}
                foreach ($Entry in ($Matches[1] -split "; ")) { if ($Entry -match "^#(\d+) .*, hp \d+/\d+") { $WoundedNow[$Matches[1]] = $true } }
                foreach ($Wait in $Pending) { $Wait.Run.Wrong += @($Wait.Ids | Where-Object { -not $WoundedNow.ContainsKey($_) }).Count }
                $Pending = @()
            } elseif ($Line -match "Health bars: bars (\d+) \(([^)]*)\)") {
                $Items = @(if ($Matches[2]) { $Matches[2] -split ", " })
                $Unsure = @($Items | Where-Object { $_ -notmatch " selected$" -and $_ -match "^#(\d+)" -and -not $WoundedNow.ContainsKey($Matches[1]) } | ForEach-Object { if ($_ -match "^#(\d+)") { $Matches[1] } })
                $NotSent = @($Items | Where-Object { $_ -match "^#(\d+) (\d+)/(\d+)" -and -not ($Matches[2] -eq $Matches[3] -or (& $IsServerHp $Matches[1] ([int]$Matches[2]))) })
                $Run = @{ Selected = @($Items | Where-Object { $_ -match " selected$" }).Count; Others = $Items.Count - @($Items | Where-Object { $_ -match " selected$" }).Count; Wrong = 0; NotSent = $NotSent.Count }
                $BarRuns += $Run
                if ($Unsure.Count) { $Pending += @{ Run = $Run; Ids = $Unsure } }
            }
        }
        # No Units where after them: still unhurt, wrong.
        foreach ($Wait in $Pending) { $Wait.Run.Wrong += $Wait.Ids.Count }
        $BarsSel = @($BarRuns | Where-Object { $_.Selected -ge 1 }).Count
        $BarsOther = @($BarRuns | Where-Object { $_.Others -ge 1 }).Count
        $BarsWrong = ($BarRuns | ForEach-Object { $_.Wrong } | Measure-Object -Sum).Sum
        $BarsNotSent = ($BarRuns | ForEach-Object { $_.NotSent } | Measure-Object -Sum).Sum
        $Checks["client 1 : each bar's health the server's ($BarsNotSent not)"] = ($BarRuns.Count -ge 20) -and ($BarsNotSent -eq 0)
        $Checks["client 1 : health bars, selected one ($BarsSel of $($BarRuns.Count)), wounded others ($BarsOther), none for an unhurt unselected ($BarsWrong)"] = ($BarRuns.Count -ge 20) -and ($BarsSel -ge 1) -and ($BarsOther -ge 1) -and ($BarsWrong -eq 0)
        $Checks["client 1 : fight capture"] = Test-Path (Join-Path $FightClient.Shots "net_combat_c1.png")
        $FirstDying = [Array]::FindIndex([string[]]$Wheres, [Predicate[string]]{ param($R) $R -match "\) Dying" })
        $FirstGone = [Array]::FindIndex([string[]]$Wheres, [Predicate[string]]{ param($R) $R2 = & $ParseWhere $R; $Spears.Count -and @($Spears | Where-Object { -not $R2.ContainsKey($_) }).Count -eq $Spears.Count })
        Write-Host "  net combat (client 1): health $LevelText; first Dying at report $FirstDying, both Spearmen gone at $FirstGone (0.5 s apart); MaA gap $MaxGap cm"
    }
    $Checks["two clients in slots 0 and 1 ($($HeldSlots -join ', '))"] = (($HeldSlots | Sort-Object) -join ',') -eq "0,1"
    if ($Parts -contains "sources") {
        $Checks["the same tree selected on both clients ($(($NetSelectedTrees.Values | Sort-Object) -join ' / '))"] = ($NetSelectedTrees.Count -eq 2) -and
            (@($NetSelectedTrees.Values | Sort-Object -Unique).Count -eq 1) -and ($NetSelectedTrees.Values -notcontains "")
    }
    # The lobby manager's calls (R3.6, R3.7): "empty" once both clients are gone (10 s at most), then the listener stops.
    $ManagerDeadline = (Get-Date).AddSeconds(10)
    while ((Get-Date) -lt $ManagerDeadline -and -not ((Test-Path $ManagerCallsFile) -and (Select-String -Path $ManagerCallsFile -SimpleMatch "/matches/QC1/empty" -Quiet))) {
        Start-Sleep -Milliseconds 250
    }
    # R3.8: the server launched by the manager quits by itself once empty (10 s at most), killed only as a last resort.
    $NetQuitDeadline = (Get-Date).AddSeconds(10)
    while ($NetServerProcess -and -not $NetServerProcess.HasExited -and (Get-Date) -lt $NetQuitDeadline) { Start-Sleep -Milliseconds 250 }
    $NetServerQuitAlone = [bool]($NetServerProcess -and $NetServerProcess.HasExited)
    if ($NetServerProcess -and -not $NetServerProcess.HasExited) { Stop-Process -Id $NetServerProcess.Id -Force }
    $ManagerListener.Close()
    $NetQuitLine = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -SimpleMatch "Server quits: empty after the match" }).Count
    $Checks["server quits alone once empty ($NetQuitLine log line, exited: $NetServerQuitAlone)"] = $NetServerQuitAlone -and ($NetQuitLine -eq 1)
    $ManagerCalls = @(if (Test-Path $ManagerCallsFile) { Get-Content $ManagerCallsFile })
    $ManagerEvents = @($ManagerCalls | ForEach-Object { if ($_ -match "^POST /matches/QC1/(\w+) ") { $Matches[1] } })
    Write-Host "  manager calls: $($ManagerCalls -join ' | ')"
    $Checks["manager: listening with port $NetPort"] = $ManagerCalls -contains "POST /matches/QC1/listening {`"port`":$NetPort}"
    $Checks["manager: started once, after listening ($($ManagerEvents -join ', '))"] = (@($ManagerCalls | Where-Object { $_ -eq "POST /matches/QC1/started {}" }).Count -eq 1) -and
        ([array]::IndexOf($ManagerEvents, "started") -gt [array]::IndexOf($ManagerEvents, "listening")) -and ([array]::IndexOf($ManagerEvents, "listening") -ge 0)
    $Checks["manager: empty once, last"] = (@($ManagerCalls | Where-Object { $_ -eq "POST /matches/QC1/empty {}" }).Count -eq 1) -and ($ManagerEvents[-1] -eq "empty")
    # The melee on (J7b.2) on the dedicated server: the two armies stand at their own centres, far apart, no one killed;
    # the fight made for client 1 (J7b.4) kills its 2 Spearmen alone.
    # Before the hero sheets' phase (J8a.3: its own fight, Bob's hero and the Men-at-arms against him).
    $HeroMarkLine = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -SimpleMatch "Test mark: net hero" }) | Select-Object -First 1
    $NetKills = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Combat: step \d+ #\d+ (\w+ of slot \d) killed by" | Where-Object { $_.Line -notmatch "by arrow" -and (-not $HeroMarkLine -or $_.LineNumber -lt $HeroMarkLine.LineNumber) } | ForEach-Object { $_.Matches[0].Groups[1].Value } })
    # The arrows on the clients (J7c.2b2, Q4: deduced, 0 bit more): 3 Archers of slot 0 shoot 2 Spearmen for 12 s, after
    # client 1's fight window, then all removed; each client launched about as many arrows as the server released (1 more or less per shooter), none
    # left in flight at the end; the gap between a drawn arrow's landing and the health drop received, reported.
    $NetReleased = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "releases an arrow" }).Count
    foreach ($NetClient in $NetClients) {
        $ArrowLine = @(if (Test-Path $NetClient.Log) { Select-String -Path $NetClient.Log -Pattern "Arrow report: in flight (\d+) launched (\d+) landed (\d+) gaps (\d+) mean (-?\d+) min (-?\d+) max (-?\d+) ms" }) | Select-Object -Last 1
        $InFlight = if ($ArrowLine) { [int]$ArrowLine.Matches[0].Groups[1].Value } else { -1 }
        $Launched = if ($ArrowLine) { [int]$ArrowLine.Matches[0].Groups[2].Value } else { -1 }
        $GapText = if ($ArrowLine) { "gaps $($ArrowLine.Matches[0].Groups[4].Value) mean $($ArrowLine.Matches[0].Groups[5].Value) min $($ArrowLine.Matches[0].Groups[6].Value) max $($ArrowLine.Matches[0].Groups[7].Value) ms" } else { "no report" }
        Write-Host "  net arrows (client $($NetClient.Client)): server released $NetReleased, launched $Launched, in flight $InFlight; arrow landing -> health drop: $GapText"
        $Checks["client $($NetClient.Client) : arrows deduced ($Launched launched for $NetReleased released, $InFlight in flight at the end)"] = $NetReleased -gt 0 -and [math]::Abs($Launched - $NetReleased) -le 3 -and $InFlight -eq 0
    }
    $Checks["server: combat on, the fight's 2 Spearmen alone killed ($($NetKills -join ', '))"] = ($NetKills -join ',') -eq "Spearman of slot 1,Spearman of slot 1"
    # The bodies (J7c, bodies kept 30 s): client 1 keeps the fight's (hol.Units.CorpseReport at its end); the dedicated
    # server lays none (its report says 0).
    $CorpseCount = {
        param($Lines)
        $Lines | Where-Object { $_ -match "Corpse report: (\d+) corpses" } | Select-Object -Last 1 | ForEach-Object { [int]$Matches[1] }
    }
    $Client1Log = ($NetClients | Where-Object { $_.Client -eq 1 } | Select-Object -First 1).Log
    $ClientCorpses = & $CorpseCount @(if ($Client1Log -and (Test-Path $Client1Log)) { Get-Content $Client1Log })
    $ServerCorpses = & $CorpseCount @(if (Test-Path $NetServerLog) { Get-Content $NetServerLog })
    $Checks["bodies: client 1 keeps the fight's ($ClientCorpses), the server none ($ServerCorpses)"] = ($null -ne $ClientCorpses) -and $ClientCorpses -ge 1 -and ($null -ne $ServerCorpses) -and $ServerCorpses -eq 0
    # R3.4: client 1's 500 wood refused by the server (its log), the two army cheats of resources alone granted (before the
    # hero's return: Bob's two gold cheats there are its own, J8a.4 loop 5).
    $CheatRefused = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Cheat refused: resources from Bob" }).Count
    $ReturnMarkLine = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -SimpleMatch "Test mark: net hero return" | Select-Object -First 1 | ForEach-Object { $_.LineNumber } })
    $CheatGranted = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Cheat: slot \d resources" |
        Where-Object { -not $ReturnMarkLine.Count -or $_.LineNumber -lt $ReturnMarkLine[0] } }).Count
    $Checks["server: client 1's cheat refused ($CheatRefused), the army's granted ($CheatGranted of 2)"] = $CheatRefused -eq 1 -and $CheatGranted -eq 2
    # D361: client 1's console "pause" refused by the server (its log), the server never paused, its simulation stepping on.
    $PauseRefused = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Pause refused from Bob \(slot 1\)" }).Count
    $PauseReports = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Pause report: paused (\d), simulation step (-?\d+)" | ForEach-Object { $_.Matches[0] } })
    $PauseSays = ($PauseReports | ForEach-Object { "paused $($_.Groups[1].Value) step $($_.Groups[2].Value)" }) -join "; "
    $Checks["server: client 1's pause refused ($PauseRefused), the match not paused and stepping on ($PauseSays)"] = $PauseRefused -eq 1 -and
        $PauseReports.Count -ge 2 -and @($PauseReports | Where-Object { $_.Groups[1].Value -ne "0" }).Count -eq 0 -and
        [int64]$PauseReports[1].Groups[2].Value -gt [int64]$PauseReports[0].Groups[2].Value
    # D513 (P5): the limited pause, decided by the server: Bob's pause, Alice's early resume and Bob's second pause refused,
    # the match frozen while paused (the step unchanged 1 s apart), Bob's resume, the match stepping on.
    $MatchPauses = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Pause report: .*, simulation step (-?\d+), match paused (\d) by slot (-?\d+), (-?\d+) left" | ForEach-Object { $_.Matches[0] } })
    $MatchPauseSays = ($MatchPauses | ForEach-Object { "step $($_.Groups[1].Value) paused $($_.Groups[2].Value) by $($_.Groups[3].Value) left $($_.Groups[4].Value)" }) -join "; "
    $PauseLines = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Pause: " | ForEach-Object { $_.Line -replace '^.*Pause: ', '' } })
    $PauseLinesSay = $PauseLines -join " | "
    $Checks["server: Bob's pause, Alice's early resume and Bob's second pause refused, Bob's resume ($PauseLinesSay)"] =
        @($PauseLines | Where-Object { $_ -eq "slot 1 (Bob) paused, 2 left" }).Count -eq 1 -and
        @($PauseLines | Where-Object { $_ -eq "refused to Alice (slot 0): timer running" }).Count -eq 1 -and
        @($PauseLines | Where-Object { $_ -eq "refused to Bob (slot 1): already paused" }).Count -eq 1 -and
        @($PauseLines | Where-Object { $_ -eq "resumed by Bob (slot 1), 2 left" }).Count -eq 1
    $Checks["server: the match frozen while paused, stepping on once resumed ($MatchPauseSays)"] = $MatchPauses.Count -eq 6 -and
        $MatchPauses[2].Groups[2].Value -eq "1" -and $MatchPauses[2].Groups[3].Value -eq "1" -and $MatchPauses[2].Groups[4].Value -eq "2" -and
        $MatchPauses[3].Groups[2].Value -eq "1" -and $MatchPauses[3].Groups[1].Value -eq $MatchPauses[2].Groups[1].Value -and
        $MatchPauses[4].Groups[2].Value -eq "0" -and $MatchPauses[5].Groups[2].Value -eq "0" -and
        [int64]$MatchPauses[5].Groups[1].Value -gt [int64]$MatchPauses[4].Groups[1].Value
    # The pause screen (P5.3) on each client: Bob's (« Reprendre » his, at once), then hidden once resumed; Alice's
    # (« Reprendre » greyed while the timer runs, 1 to 60 s).
    foreach ($N in 1, 2) {
        $ClientLog = Join-Path $TestSaved "Logs\quick_net_client$N.log"
        $Screens = @(if (Test-Path $ClientLog) { Select-String -Path $ClientLog -Pattern "HUD pause: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() } })
        $Texts = @(if (Test-Path $ClientLog) { Select-String -Path $ClientLog -Pattern "HUD pause texts: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() } })
        $Says = ($Screens -join "; ") + " / " + ($Texts -join "; ")
        $Resume = if ($N -eq 1) { "yes" } else { "no" }
        $Wanted = "^paused by Bob, (\d+) s, 2 left, resume $Resume$"
        $Shown = @($Screens | Where-Object { $_ -match $Wanted -and [int]$Matches[1] -ge 1 -and [int]$Matches[1] -le 60 })
        $Hidden = if ($N -eq 1) { $Screens.Count -ge 2 -and $Screens[-1] -eq "hidden" } else { $true }
        # Its own world paused while the match is (the units' animations, falls, arrows and bodies stand still), then running.
        $WorldLines = @(if (Test-Path $ClientLog) { Select-String -Path $ClientLog -Pattern "Pause: this machine's world (paused|running)" | ForEach-Object { $_.Matches[0].Groups[1].Value } })
        $Checks["client $N : its world paused then running ($($WorldLines -join ', '))"] = ($WorldLines -join ",") -eq "paused,running"
        $Checks["client $N : the pause screen, resume $Resume, then hidden once resumed ($Says)"] = $Shown.Count -ge 1 -and $Hidden -and
            @($Texts | Where-Object { $_ -match "^Partie en pause \| Mise en pause par Bob \| Tout le monde pourra reprendre dans \d:\d\d \| Pauses restantes : 2 \| Reprendre$" }).Count -ge 1
    }
    $ServerErrors = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "LogHOLMatch: (Error|Warning)|LogHOLNet: (Error|Warning)" })
    $Checks["server: no match or network error ($($ServerErrors.Count))"] = $ServerErrors.Count -eq 0
    # The scripts on the dedicated server (J8a5-Q3c, D484): every .as compiles there too (the editor's -server loads
    # no ClientOnly module: a script naming a class of HeroesOfLibertyUI failed there, "'UHOLGameScreen' is not
    # declared", and the server quit at once on -as-exit-on-error; the interface module is Runtime since).
    $ServerScriptErrors = @(Get-ScriptErrors $NetServerLog)
    $Checks["server: scripts compiled ($(if ($ServerScriptErrors.Count) { $ServerScriptErrors -join ' ' } else { 'no error' }))"] =
        (Test-Path $NetServerLog) -and $ServerScriptErrors.Count -eq 0
    $Born = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Production: Peasant of slot (\d) done" | ForEach-Object { $_.Matches[0].Groups[1].Value } })
    $Checks["server: one Peasant born per slot ($($Born -join ', '))"] = (($Born | Sort-Object) -join ',') -eq "0,1"
    # R3.9-R3.10: the server waits for the named players (one of them alone missing at some point), then starts all
    # here at simulation step 0: no step and no production line before the start.
    $WaitOne = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Match waits for: (Alice|Bob)$" })
    $StartLines = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Match starts: all here \(simulation step (\d+)\)" })
    $StartStep = if ($StartLines.Count -eq 1) { [int]$StartLines[0].Matches[0].Groups[1].Value } else { -1 }
    $ProdBefore = @(if ($StartLines.Count -eq 1) { Select-String -Path $NetServerLog -Pattern "Production: " | Where-Object { $_.LineNumber -lt $StartLines[0].LineNumber } })
    $Checks["server: waits for the absent ($(($WaitOne | ForEach-Object { $_.Matches[0].Groups[1].Value }) -join ', ')), then starts all here ($($StartLines.Count)) at step $StartStep, $($ProdBefore.Count) production lines before"] =
        $WaitOne.Count -ge 1 -and $StartLines.Count -eq 1 -and $StartStep -eq 0 -and $ProdBefore.Count -eq 0
    # R3.10: client 2 (Alice), launched first, shows « En attente de Bob (M:SS) » before its capture during the wait,
    # then hides it once the match starts. The time left is under the server's wait of this test (hol.Match.StartWaitSeconds
    # 120: 1:SS, or 0:SS once her start took more than a minute).
    $Checks["client 2 (Alice) has captured her wait banner before client 1's launch"] = $NetAliceReady
    $AliceClient = @($NetClients | Where-Object { $_.Client -eq 2 })
    $WaitShown = ""; $WaitCaptured = $false; $WaitHidden = $false
    if ($AliceClient.Count -eq 1) {
        $AliceLines = @(if (Test-Path $AliceClient[0].Log) { Get-Content $AliceClient[0].Log })
        $ShownAt = -1; $CaptureAt = -1; $HiddenAt = -1
        for ($i = 0; $i -lt $AliceLines.Count; $i++) {
            if ($ShownAt -lt 0 -and $AliceLines[$i] -match "Wait banner: shown, (.*)$") { $ShownAt = $i; $WaitShown = $Matches[1] }
            if ($CaptureAt -lt 0 -and $AliceLines[$i] -match "Test capture: qcnet_2_wait ") { $CaptureAt = $i }
            if ($HiddenAt -lt 0 -and $AliceLines[$i] -match "Wait banner: hidden") { $HiddenAt = $i }
        }
        $WaitCaptured = ($ShownAt -ge 0) -and ($CaptureAt -gt $ShownAt) -and ($HiddenAt -gt $CaptureAt) -and
            (Test-Path (Join-Path $AliceClient[0].Shots "qcnet_2_wait.png"))
        $WaitHidden = $HiddenAt -ge 0
    }
    $Checks["client 2's wait banner ($WaitShown), captured while shown, hidden at the start ($WaitHidden)"] = $WaitCaptured -and
        ($WaitShown -match "^En attente de Bob \(([01]:[0-5]\d|2:00)\)$")
    if ($Parts -contains "sources") {
        $Unloaded = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Gather: slot (\d) unloaded 10 Wood" | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique })
        $Checks["server: each player's Peasant unloaded 10 wood at its centre ($($Unloaded -join ', '))"] = ($Unloaded -join ',') -eq "0,1"
        # Each client's +10 at most its own player's unloads (one per unload); the server shows none (no player there).
        $TooMany = @($NetPopups.Values | Where-Object { $_.Count -gt @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Gather: slot $($_.Slot) unloaded [1-9]" }).Count })
        $ServerPopups = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Deposit popup:" }).Count
        $Checks["each client's +10 no more than its own unloads ($($TooMany.Count) over), none on the server ($ServerPopups)"] = ($NetPopups.Count -eq 2) -and ($TooMany.Count -eq 0) -and ($ServerPopups -eq 0)
        # J6a.1 (D310): over the round trips, no Peasant's body in a footprint (the server's last report).
        $Watch = if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Footprint watch: (\d+) steps, \d+ samples, closest body gap (-?\d+) cm \((.*)\); bodies in a footprint: (\d+) unit-steps" | Select-Object -Last 1 }
        $W = if ($Watch) { $Watch.Matches[0].Groups } else { $null }
        $Checks["server: no Peasant's body in a footprint ($(if ($W) { "$($W[1].Value) steps, closest gap $($W[2].Value) cm, $($W[4].Value) unit-steps inside" }))"] =
            ($null -ne $W) -and ([int]$W[1].Value -ge 200) -and ([int]$W[2].Value -ge 0) -and ([int]$W[4].Value -eq 0)
    }
    if ($Parts -contains "build") {
        $ServerLines = @(if (Test-Path $NetServerLog) { Get-Content $NetServerLog })
        $Placed = @($ServerLines | Select-String -Pattern "Construction: slot (\d) placed House foundation at \((-?\d+), (-?\d+)\), paid F 0 W 50 S 0 G 0" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object)
        $HousesWanted = "0: ($((Get-NearSpot 0 $NetHouseNear[0] $NetHouseNear[1] 2) -join ', '))|1: ($((Get-NearSpot 1 $NetHouseNear[0] $NetHouseNear[1] 2) -join ', '))"
        $Checks["server: one house paid and placed per slot, behind its centre ($($Placed -join '; '))"] = ($Placed -join '|') -eq $HousesWanted
        $NearEnemy = @($ServerLines | Select-String -Pattern "Construction: slot (\d) cannot place House at \((-?\d+), (-?\d+)\): too near an enemy main centre" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object)
        $Checks["server: each house near the enemy centre refused ($($NearEnemy -join '; '))"] = ($NearEnemy -join '|') -eq "0: ($((Get-NearSpot 1 $NetEnemyNear[0] $NetEnemyNear[1] 2) -join ', '))|1: ($((Get-NearSpot 0 $NetEnemyNear[0] $NetEnemyNear[1] 2) -join ', '))"
        $Started = @($ServerLines | Select-String -Pattern "Construction: slot (\d) first work on House at \((-?\d+), (-?\d+)\), 1 builders" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object)
        $Checks["server: each house's first work by its Peasant ($($Started -join '; '))"] = ($Started -join '|') -eq $HousesWanted
        # The camps (J6b.3): one lumber camp paid per slot, finished, its builder sent to the wood, unloading at the camp;
        # each camp asked on a tree refused.
        $CampsWanted = "0: ($((Get-NearSpot 0 $NetCampNear[0] $NetCampNear[1] 2) -join ', '))|1: ($((Get-NearSpot 1 $NetCampNear[0] $NetCampNear[1] 2) -join ', '))"
        $CampPlaced = @($ServerLines | Select-String -Pattern "Construction: slot (\d) placed LumberCamp foundation at \((-?\d+), (-?\d+)\), paid F 0 W 100 S 0 G 0" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object)
        $Checks["server: one lumber camp paid and placed per slot by its wood ($($CampPlaced -join '; '))"] = ($CampPlaced -join '|') -eq $CampsWanted
        $CampSent = @($ServerLines | Select-String -Pattern "Construction: slot (\d) camp LumberCamp at \((-?\d+), (-?\d+)\) finished: 1 builders sent to gather Wood" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object)
        $Checks["server: each camp finished, its builder sent to the wood ($($CampSent -join '; '))"] = ($CampSent -join '|') -eq $CampsWanted
        $CampUnloads = @($ServerLines | Select-String -Pattern "Gather: slot (\d) unloaded \d+ Wood \(stock [^)]*\) at LumberCamp \((-?\d+), (-?\d+)\)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object -Unique)
        $Checks["server: each builder unloaded at its camp ($($CampUnloads -join '; '))"] = ($CampUnloads -join '|') -eq $CampsWanted
        $OnTree = @($ServerLines | Select-String -Pattern "Construction: slot (\d) cannot place LumberCamp at \(-?\d+, -?\d+\): (.*)$" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): $($_.Matches[0].Groups[2].Value)" } | Sort-Object)
        $Checks["server: each camp on a tree refused ($($OnTree -join '; '))"] = (@($OnTree | ForEach-Object { $_.Substring(0, 1) }) -join ',') -eq "0,1"
    }
    # The end of the match (V1.4c, D356-D358), the net part's last: client 1's King died (V5.5a; the surrender over the
    # network is checked by the "lobby" part); the server defeats its slot,
    # collapses it and ends the match, client 2's slot the winner; the banners Defeat on client 1, Victory on client 2
    # (the game state's replication and OnRep); client 2 sees client 1's units go (none left 5 s after its banner).
    if ($Parts -contains "victory") {
        $Loser = $NetHeld[1]; $Winner = $NetHeld[2]
        $ServerLines = @(if (Test-Path $NetServerLog) { Get-Content $NetServerLog })
        # V5.5a (Regicide): client 1's defeat by its King's death (the server's KillKing 1) in place of its surrender.
        $Checks["victory: server, a King born for each slot"] = (@(0, 1) | Where-Object { $ServerLines | Select-String -Pattern "Regicide: king of slot $_ spawned at \(-?\d+, -?\d+\)" -Quiet }).Count -eq 2
        $Checks["victory: server, the King of slot $Loser (client 1) died"] = ($null -ne $Loser) -and [bool]($ServerLines | Select-String -SimpleMatch "Regicide: king of slot $Loser died" -Quiet) -and
            -not ($ServerLines | Select-String -SimpleMatch "Regicide: king of slot $Winner died" -Quiet)
        $Checks["victory: server, slot $Loser (client 1) defeated by its King's death"] = ($null -ne $Loser) -and [bool]($ServerLines | Select-String -SimpleMatch "Victory: slot $Loser defeated (King)" -Quiet) -and
            -not ($ServerLines | Select-String -SimpleMatch "Victory: slot $Loser defeated (Surrender)" -Quiet)
        $Collapsed = $ServerLines | Select-String -Pattern "Victory: slot $Loser collapsed \((\d+) units, (\d+) buildings\)" | Select-Object -First 1
        $Checks["victory: server, slot $Loser collapsed ($(if ($Collapsed) { "$($Collapsed.Matches[0].Groups[1].Value) units, $($Collapsed.Matches[0].Groups[2].Value) buildings" }))"] =
            ($null -ne $Collapsed) -and ([int]$Collapsed.Matches[0].Groups[1].Value -ge 1) -and ([int]$Collapsed.Matches[0].Groups[2].Value -ge 1)
        $Over = @($ServerLines | Select-String -Pattern "Victory: match over, (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $Checks["victory: server, match over, winners $Winner ($($Over -join '; '))"] = ($null -ne $Winner) -and (($Over -join '|') -eq "winners $Winner")
        $Banner = @{}
        foreach ($NetClient in $NetClients) {
            $Banner[$NetClient.Client] = @(Get-ClientSection (Get-Content $NetClient.Log -ErrorAction SilentlyContinue) "victory" | Select-String -CaseSensitive -Pattern "End banner: (Victory|Defeat|MatchOver)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        }
        $Checks["manager: finished, winners [$Winner]"] = ($null -ne $Winner) -and (@($ManagerCalls | Where-Object { $_ -eq "POST /matches/QC1/finished {`"winners`":[$Winner]}" }).Count -eq 1)
        $Checks["victory: client 1, End banner: Defeat ($($Banner[1] -join ', '))"] = ($Banner[1] -join ',') -eq "Defeat"
        $Checks["victory: client 2, End banner: Victory ($($Banner[2] -join ', '))"] = ($Banner[2] -join ',') -eq "Victory"
        $Client2Log = ($NetClients | Where-Object { $_.Client -eq 2 } | Select-Object -First 1).Log
        $Shown = @(Get-ClientSection @(if ($Client2Log) { Get-Content $Client2Log -ErrorAction SilentlyContinue }) "victory" | Select-String -Pattern "Units report: (.*)$" | ForEach-Object {
            $Text = $_.Matches[0].Groups[1].Value
            ([regex]::Matches($Text, "of slot $Loser x(\d+)") | ForEach-Object { [int]$_.Groups[1].Value } | Measure-Object -Sum).Sum })
        # V5.5a: 7 reports: before the forum's fall, the Kings shown, then each second after the banner.
        $Checks["victory: client 2 sees client 1's units go ($($Shown -join ' -> '))"] = ($Shown.Count -eq 7) -and ($Shown[0] -ge 1) -and ($Shown[-1] -eq 0)
        # V5.5a: each client shows the other's King (the unit King replicated), and its own.
        foreach ($Pair in @(@(1, $Client1Log, $Winner), @(2, $Client2Log, $Loser))) {
            $KingsShown = @(Get-ClientSection @(if ($Pair[1]) { Get-Content $Pair[1] -ErrorAction SilentlyContinue }) "victory" | Select-String -Pattern "Units report: (.*)$" |
                ForEach-Object { $_.Matches[0].Groups[1].Value } | Where-Object { $_ -match "King of slot" }) | Select-Object -First 1
            $Checks["victory: client $($Pair[0]) shows the King of the other, slot $($Pair[2]), and its own ($([regex]::Matches("$KingsShown", 'King of slot \d x\d+') -join ', '))"] =
                ($null -ne $Loser) -and "$KingsShown" -match "King of slot $($Pair[2]) x1" -and "$KingsShown" -match "King of slot $(1 - [int]$Pair[2]) x1"
        }
        # V2.5 (D353): client 1's forum fell on the server (lives 1); client 2's players' table shows it (the lives public).
        $Checks["victory: server, the forum of slot $Loser (client 1) fell, lives 1"] = ($null -ne $Loser) -and [bool]($ServerLines | Select-String -SimpleMatch "Victory: forum of slot $Loser fell, lives 1 " -Quiet)
        $Table2 = @(Get-ClientSection @(if ($Client2Log) { Get-Content $Client2Log -ErrorAction SilentlyContinue }) "victory" | Select-String -Pattern "HUD forum: .*, players \[([^\]]*)\]" |
            ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -Last 1
        $LoserRow = @("$Table2" -split " \| " | Where-Object { $_ -match "^slot $Loser " }) | Select-Object -First 1
        $Checks["victory: client 2's table shows slot $Loser at 1 life, in ruin, held by slot $Winner ($LoserRow)"] = ($null -ne $Loser) -and "$LoserRow" -match "^slot $Loser( ally| enemy)? ('[^']*' )?colour \d+ lives 1 ruin hold \d+ s holding by slot $Winner$"
        # V4.5 (D353, D202): the fall's message to each (the server's multicast): client 1 its own with the alert, client 2
        # the named one alone (another camp); the hold the same; client 1's defeat by its surrender, no alert (V4.5b: its own loss).
        $Client1Log = ($NetClients | Where-Object { $_.Client -eq 1 } | Select-Object -First 1).Log
        $Messages = @{}
        foreach ($Pair in @(@(1, $Client1Log), @(2, $Client2Log))) {
            $Messages[$Pair[0]] = @(Get-ClientSection @(if ($Pair[1]) { Get-Content $Pair[1] -ErrorAction SilentlyContinue }) "victory" | Select-String -Pattern "Message: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        }
        # The names the lobby gave the slots (M6-Q12), as the players' table: Alice, Bob, not Joueur N.
        $LoserName = if ($null -ne $Loser) { $NetNames[[int]$Loser] } else { "" }; $WinnerName = if ($null -ne $Winner) { $NetNames[[int]$Winner] } else { "" }
        # V5.5a: its King's death before its defeat, no alert for either (its own loss; another camp's for client 2).
        $Wanted1 = "Votre forum est tombé — 1 vie (alert)|$WinnerName tient votre ruine (alert)|Votre Roi est mort|Vous êtes vaincu"
        $Wanted2 = "Le forum de $LoserName est tombé — 1 vie|Vous tenez la ruine de $LoserName|Le Roi de $LoserName est mort|$LoserName est vaincu"
        $Checks["victory: client 1's messages, with the alert ($($Messages[1] -join '; '))"] = ($null -ne $Loser) -and ($Messages[1] -join '|') -eq $Wanted1
        $Checks["victory: client 2's messages, without alert ($($Messages[2] -join '; '))"] = ($null -ne $Loser) -and ($Messages[2] -join '|') -eq $Wanted2
        $Alert1 = @(Get-ClientSection @(if ($Client1Log) { Get-Content $Client1Log -ErrorAction SilentlyContinue }) "victory" | Select-String -Pattern "HUD alert: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -First 1
        $Alert2 = @(Get-ClientSection @(if ($Client2Log) { Get-Content $Client2Log -ErrorAction SilentlyContinue }) "victory" | Select-String -Pattern "HUD alert: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value }) | Select-Object -First 1
        $Checks["victory: client 1's HUD alert on its forum ($Alert1)"] = "$Alert1" -match "^.+ \(slot $Loser\)$"
        $Checks["victory: client 2's HUD alert: none ($Alert2)"] = "$Alert2" -eq "none"
        $DefeatAlerts = @(Get-ClientSection @(if ($Client1Log) { Get-Content $Client1Log -ErrorAction SilentlyContinue }) "victory" | Select-String -CaseSensitive -SimpleMatch "Alert: Vous êtes vaincu")
        $Checks["victory: client 1, no alert of its defeat ($($DefeatAlerts.Count))"] = $DefeatAlerts.Count -eq 0
    }
    # "hero" alone (J8a.6): the join and the server's checks only (no production, army, fight nor arrows played).
    if ($NetHeroOnly) {
        $Kept = [ordered]@{}
        foreach ($Key in @($Checks.Keys)) {
            if ($Key -match "^(server listening|two clients in slots|server quits alone|manager: |server: (no match or network error|scripts compiled|waits for the absent)|client 2 \(Alice\) above|client 2's wait banner|client \d : (sequence done|holds a slot|holds the slot of|logged in as|camera above|both centres))") {
                $Kept[$Key] = $Checks[$Key]
            }
        }
        $Checks = $Kept
    }
    $AllOk = (Report "net 2 clients" $Checks) -and $AllOk
    # The hero sheets over the network (J8a.3, J8-Q5), the server's heroes of the "net hero" phase (Alice slot 0 xp 5, Bob
    # slot 1 xp 7, Bob's wounded then killed): each hol.Hero.Report read into its public sheets, its local private sheet,
    # the other player states' private values as received, and (server) the true sheets.
    $ReadHeroReports = {
        param($Lines)
        $Reports = New-Object System.Collections.ArrayList
        $Current = $null
        foreach ($Line in $Lines) {
            if ($Line -match "Hero report: net mode \d+") { $Current = @{ Sheet = @{}; Local = $null; Other = @{}; Truth = @{} }; [void]$Reports.Add($Current); continue }
            if (-not $Current) { continue }
            if ($Line -match "Hero sheet slot (\d): state (\w+) level \d+ hp (\d+)") { $Current.Sheet[[int]$Matches[1]] = @{ State = $Matches[2]; Hp = [int]$Matches[3] } }
            elseif ($Line -match "Hero gold slot (\d) local: (-?\d+)") { $Current.Gold = [int]$Matches[2] }
            elseif ($Line -match "Hero private slot (\d) local: xp (\d+) death (-?\d+)") { $Current.Local = @{ Slot = [int]$Matches[1]; Xp = [int]$Matches[2]; Death = [int]$Matches[3] } }
            elseif ($Line -match "Hero private slot (\d) other: xp (\d+) death (-?\d+)") { $Current.Other[[int]$Matches[1]] = @{ Xp = [int]$Matches[2]; Death = [int]$Matches[3] } }
            elseif ($Line -match "Hero true slot (\d): state (\w+) level \d+ hp (\d+) xp (\d+) death (-?\d+)") {
                $Current.Truth[[int]$Matches[1]] = @{ State = $Matches[2]; Hp = [int]$Matches[3]; Xp = [int]$Matches[4]; Death = [int]$Matches[5] } }
        }
        , $Reports
    }
    $HeroChecks = [ordered]@{}
    $AliceSlot = 0; $BobSlot = 1; $HeroMax = 450
    $HeroXp = @{ $AliceSlot = 5; $BobSlot = 7 }
    $HeroChecks["Bob (client 1) in slot $BobSlot, Alice (client 2) in slot $AliceSlot (held $($NetHeld[1]), $($NetHeld[2]))"] = "$($NetHeld[1])" -eq "$BobSlot" -and "$($NetHeld[2])" -eq "$AliceSlot"
    $HeroServer = & $ReadHeroReports @(Get-ClientSection @(if (Test-Path $NetServerLog) { Get-Content $NetServerLog }) "net hero")
    $Wounded = if ($HeroServer.Count -ge 1) { $HeroServer[0].Truth[$BobSlot] } else { $null }
    $Fallen = if ($HeroServer.Count -ge 2) { $HeroServer[1].Truth[$BobSlot] } else { $null }
    $TrueHp = if ($Wounded) { $Wounded.Hp } else { -1 }
    # J8b.2: Bob's hero earns 120 (combat) for each Man-at-arms he kills meanwhile: his XP read back from the server's.
    $BobXp = if ($Wounded) { $Wounded.Xp } else { -1 }
    if ($BobXp -ge $HeroXp[$BobSlot] -and ($BobXp - $HeroXp[$BobSlot]) % 120 -eq 0) { $HeroXp[$BobSlot] = $BobXp }
    $HeroChecks["server: 2 reports ($($HeroServer.Count)), Bob's hero wounded and alive (hp $TrueHp of $HeroMax), xp 5 and 7 + 120 a Man-at-arms he killed ($(if ($HeroServer.Count) { "$($HeroServer[0].Truth[$AliceSlot].Xp), $($HeroServer[0].Truth[$BobSlot].Xp)" }))"] =
        $HeroServer.Count -eq 2 -and $Wounded -and $Wounded.State -eq "Alive" -and $TrueHp -gt 0 -and $TrueHp -lt $HeroMax -and
        $HeroServer[0].Truth[$AliceSlot].Xp -eq $HeroXp[$AliceSlot] -and $HeroServer[0].Truth[$BobSlot].Xp -eq $HeroXp[$BobSlot] -and $HeroXp[$BobSlot] -lt 200 -and
        $HeroServer[0].Truth[$AliceSlot].State -eq "Alive"
    $HeroChecks["server: Bob's hero dead after hol.Hero.Kill, death step known ($(if ($Fallen) { "$($Fallen.State), death $($Fallen.Death)" }))"] = $Fallen -and $Fallen.State -eq "Dead" -and $Fallen.Death -ge 0
    # The share sent for the wounded hero (HOLUnitReplication::GetWireHealthShare): ceil(hp * 255 / 450); with his public
    # sheet received (loop 4b), the client shows his exact health from it, the share filling his bar: hol.Units.Where logs
    # both, "hp <sheet>/450 share <received>/255".
    $Share = if ($TrueHp -gt 0) { [int][Math]::Ceiling($TrueHp * 255 / $HeroMax) } else { -1 }
    foreach ($NetClient in $NetClients) {
        $N = $NetClient.Client
        $Own = if ($N -eq 1) { $BobSlot } else { $AliceSlot }
        $Foe = 1 - $Own
        $HeroLines = @(Get-ClientSection @(Get-Content $NetClient.Log -ErrorAction SilentlyContinue) "hero")
        $Seen = & $ReadHeroReports $HeroLines
        $First = if ($Seen.Count -ge 1) { $Seen[0] } else { $null }
        $Second = if ($Seen.Count -ge 2) { $Seen[1] } else { $null }
        # 1. The exact health in the public sheet, the server's own (rounded up), while wounded.
        $SheetHp = if ($First -and $First.Sheet[$BobSlot]) { $First.Sheet[$BobSlot].Hp } else { -1 }
        $HeroChecks["client $N : 2 reports ($($Seen.Count)), Bob's hero's sheet hp $SheetHp = the server's $TrueHp, Alive ($(if ($First) { $First.Sheet[$BobSlot].State }))"] =
            $Seen.Count -eq 2 -and $TrueHp -gt 0 -and $SheetHp -eq $TrueHp -and $First.Sheet[$BobSlot].State -eq "Alive"
        # 2. The hero unit's health (Bob's wounded hero, the only slot 1 unit of 450 at most shown wounded): exact from his
        # sheet, and the share received.
        $Where = $HeroLines | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
        $Received = [regex]::Match("$Where", "slot $BobSlot at \(-?\d+, -?\d+\) [^;]*?hp (\d+)/$HeroMax(?: share (\d+)/255)?")
        $ReceivedHp = if ($Received.Success) { [int]$Received.Groups[1].Value } else { -1 }
        $ReceivedShare = if ($Received.Success -and $Received.Groups[2].Success) { [int]$Received.Groups[2].Value } else { -1 }
        $HeroChecks["client $N : the hero unit's hp $ReceivedHp = the server's $TrueHp (from his sheet), share $ReceivedShare/255 received (expected $Share)"] =
            $TrueHp -gt 0 -and $ReceivedHp -eq $TrueHp -and $Share -gt 0 -and $ReceivedShare -eq $Share
        # 3. The XP to its owner only: its own as local, the other's default (COND_OwnerOnly).
        $LocalText = if ($First -and $First.Local) { "slot $($First.Local.Slot) xp $($First.Local.Xp)" } else { "none" }
        $OtherXp = if ($First -and $First.Other[$Foe]) { $First.Other[$Foe].Xp } else { -1 }
        $HeroChecks["client $N : its own XP $($HeroXp[$Own]) seen ($LocalText), the other's hidden (slot $Foe xp $OtherXp)"] =
            $First -and $First.Local -and $First.Local.Slot -eq $Own -and $First.Local.Xp -eq $HeroXp[$Own] -and $OtherXp -eq 0
        # 4. Dead in the public sheet after the kill, for both.
        $HeroChecks["client $N : Bob's hero Dead in the public sheet after the kill ($(if ($Second -and $Second.Sheet[$BobSlot]) { "$($Second.Sheet[$BobSlot].State) hp $($Second.Sheet[$BobSlot].Hp)" }))"] =
            $Second -and $Second.Sheet[$BobSlot] -and $Second.Sheet[$BobSlot].State -eq "Dead" -and $Second.Sheet[$AliceSlot].State -eq "Alive"
        # 5. The death step to its owner only: Bob's own the server's, Alice's copy of Bob's -1.
        if ($Own -eq $BobSlot) {
            $Death = if ($Second -and $Second.Local) { $Second.Local.Death } else { -2 }
            $HeroChecks["client $N : its hero's death step known ($Death, the server's $(if ($Fallen) { $Fallen.Death }))"] = $Fallen -and $Death -ge 0 -and $Death -eq $Fallen.Death
        }
        else {
            $Death = if ($Second -and $Second.Other[$BobSlot]) { $Second.Other[$BobSlot].Death } else { -2 }
            $HeroChecks["client $N : the other's death step hidden (slot $BobSlot death $Death)"] = $Death -eq -1
        }
    }
    $AllOk = (Report "net hero sheets" $HeroChecks) -and $AllOk
    # The hero's return over the network (J8a.4 loop 5), the times at 20 % (hol.Hero.ReturnTimePercent 20 on every machine):
    # the server's lines (orders taken at the step, the script's refusals, ready, bought, born), the clients' reports
    # (ready seen, Bob's gold, both back), nothing decided on a client.
    $ReturnChecks = [ordered]@{}
    $ServerLines = @(if (Test-Path $NetServerLog) { Get-Content $NetServerLog })
    $BobLevel = @($ServerLines | Select-String -Pattern "Hero true slot $BobSlot\: state Dead level (\d+)" | Select-Object -First 1 | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $BobLevel = if ($BobLevel.Count) { $BobLevel[0] } else { -1 }
    $DeathStep = if ($Fallen) { $Fallen.Death } else { -1 }
    $ReturnDelay = [int][Math]::Floor((30 + 8 * $BobLevel) * 20 * 20 / 100)
    $ReturnDrop = [int][Math]::Floor(60 * 20 * 20 / 100)
    $ReadyAt = $DeathStep + $ReturnDelay
    $BobOrders = @($ServerLines | Select-String -Pattern "Hero return order: slot $BobSlot at step (\d+) (\w+)" | ForEach-Object { @{ Step = [int]$_.Matches[0].Groups[1].Value; Result = $_.Matches[0].Groups[2].Value } })
    $Early = @($ServerLines | Select-String -Pattern "Hero return refused: slot $BobSlot at step (\d+): not ready \(ready at step (\d+)\)" | ForEach-Object { @{ Step = [int]$_.Matches[0].Groups[1].Value; Ready = [int]$_.Matches[0].Groups[2].Value } }) | Select-Object -First 1
    $ReturnChecks["server: Bob's first order refused, too early ($(if ($BobOrders.Count) { "step $($BobOrders[0].Step) $($BobOrders[0].Result)" }); $(if ($Early) { "not ready at $($Early.Step), ready at $($Early.Ready)" } else { 'no refusal' })), ready = death $DeathStep + $ReturnDelay (level $BobLevel at 20 %)"] =
        $BobOrders.Count -ge 1 -and $BobOrders[0].Result -eq "refused" -and $Early -and $Early.Step -eq $BobOrders[0].Step -and $Early.Step -lt $Early.Ready -and $DeathStep -ge 0 -and $BobLevel -ge 1 -and $Early.Ready -eq $ReadyAt
    $ReadyLine = @($ServerLines | Select-String -Pattern "Hero return: slot $BobSlot ready at step (\d+)" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $ReturnChecks["server: Bob's hero ready once, at step $ReadyAt ($($ReadyLine -join ', '))"] = $ReadyLine.Count -eq 1 -and $ReadyLine[0] -eq $ReadyAt
    $CannotPay = @($ServerLines | Select-String -SimpleMatch "Hero return: slot $BobSlot cannot pay")
    $ReturnChecks["server: Bob's order without gold refused, cannot pay ($(if ($CannotPay.Count) { $CannotPay[0].Line -replace '^.*Hero return: ', '' } else { 'none' }); orders $(($BobOrders | ForEach-Object { $_.Result }) -join ', '))"] =
        $CannotPay.Count -eq 1 -and $CannotPay[0].Line -match "\(0 gold\)" -and $BobOrders.Count -eq 3 -and $BobOrders[1].Result -eq "refused" -and $BobOrders[2].Result -eq "bought"
    $Bought = @($ServerLines | Select-String -Pattern "Hero return: slot $BobSlot bought for (\d+) gold" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
    $BoughtStep = if ($BobOrders.Count -eq 3) { $BobOrders[2].Step } else { -1 }
    $SinceReady = $BoughtStep - $ReadyAt
    $ExpectedPrice = if ($SinceReady -lt 0) { -1 } elseif ($SinceReady -ge $ReturnDrop) { 0 } else { [int][Math]::Floor((150 * ($ReturnDrop - $SinceReady) + $ReturnDrop - 1) / $ReturnDrop) }
    $ReturnChecks["server: Bob's return bought at step $BoughtStep for $($Bought -join ', ') gold = PriceAt ($ExpectedPrice, $SinceReady steps after ready of $ReturnDrop), 0 < price < 150"] =
        $Bought.Count -eq 1 -and $Bought[0] -eq $ExpectedPrice -and $ExpectedPrice -gt 0 -and $ExpectedPrice -lt 150
    $AliceLines = @($ServerLines | Select-String -Pattern "Hero return( order)?: slot $AliceSlot (.*)$" | ForEach-Object { $_.Matches[0].Groups[2].Value })
    $ReturnChecks["server: Alice's hero back by itself at price 0, no order nor gold ($($AliceLines -join '; '))"] =
        @($AliceLines | Where-Object { $_ -like "ready at step*" }).Count -eq 1 -and @($AliceLines | Where-Object { $_ -eq "by its main forum" }).Count -eq 1 -and
        @($ServerLines | Select-String -Pattern "Hero return order: slot $AliceSlot|Hero return: slot $AliceSlot (bought|cannot)").Count -eq 0
    $HeroBack = & $ReadHeroReports @(Get-ClientSection $ServerLines "net hero return")
    $BackTruth = if ($HeroBack.Count -ge 1) { $HeroBack[-1].Truth } else { @{} }
    $ReturnChecks["server: both heroes Alive at full health ($(foreach ($Slot in $AliceSlot, $BobSlot) { if ($BackTruth[$Slot]) { "slot $Slot $($BackTruth[$Slot].State) $($BackTruth[$Slot].Hp)" } }))"] =
        $BackTruth[$AliceSlot] -and $BackTruth[$BobSlot] -and $BackTruth[$AliceSlot].State -eq "Alive" -and $BackTruth[$BobSlot].State -eq "Alive" -and
        $BackTruth[$AliceSlot].Hp -eq $HeroMax -and $BackTruth[$BobSlot].Hp -eq $HeroMax
    foreach ($NetClient in $NetClients) {
        $N = $NetClient.Client
        $ClientLines = @(Get-Content $NetClient.Log -ErrorAction SilentlyContinue)
        $Seen = & $ReadHeroReports @(Get-ClientSection $ClientLines "hero return")
        $First = if ($Seen.Count -ge 1) { $Seen[0] } else { $null }
        $Last = if ($Seen.Count -ge 1) { $Seen[-1] } else { $null }
        $ReturnChecks["client $N : Bob's hero ReadyToReturn in the public sheet once ready ($(if ($First -and $First.Sheet[$BobSlot]) { $First.Sheet[$BobSlot].State }))"] =
            $First -and $First.Sheet[$BobSlot] -and $First.Sheet[$BobSlot].State -eq "ReadyToReturn"
        $ReturnChecks["client $N : both heroes Alive at full health at the end ($(if ($Last) { foreach ($Slot in $AliceSlot, $BobSlot) { if ($Last.Sheet[$Slot]) { "slot $Slot $($Last.Sheet[$Slot].State) $($Last.Sheet[$Slot].Hp) " } } }))"] =
            $Last -and $Seen.Count -ge 2 -and $Last.Sheet[$AliceSlot].State -eq "Alive" -and $Last.Sheet[$BobSlot].State -eq "Alive" -and $Last.Sheet[$AliceSlot].Hp -eq $HeroMax -and $Last.Sheet[$BobSlot].Hp -eq $HeroMax
        if ($N -eq 1) {
            # Bob: his report with 1 000 gold (ready), then after his bought order (Alive, the price taken).
            $Before = if ($Seen.Count -ge 4) { $Seen[1] } else { $null }
            $After = if ($Seen.Count -ge 4) { $Seen[2] } else { $null }
            $Price = if ($Bought.Count -eq 1) { $Bought[0] } else { -1 }
            $ReturnChecks["client $N : Bob's gold $(if ($Before) { $Before.Gold }) then $(if ($After) { $After.Gold }) (the price $Price taken), his hero $(if ($After -and $After.Sheet[$BobSlot]) { "$($After.Sheet[$BobSlot].State) $($After.Sheet[$BobSlot].Hp)" }) after his order"] =
                $Before -and $After -and $Before.Gold -eq 1000 -and $Price -gt 0 -and $After.Gold -eq 1000 - $Price -and $After.Sheet[$BobSlot].State -eq "Alive" -and $After.Sheet[$BobSlot].Hp -eq $HeroMax
        }
        else {
            $Golds = @($Seen | ForEach-Object { $_.Gold })
            $ReturnChecks["client $N : Alice's gold not lowered by her hero's return ($($Golds -join ', '))"] = $Golds.Count -ge 2 -and $null -ne $Golds[0] -and $Golds[-1] -ge $Golds[0]
        }
        # J8a.4 loop 6 (D202): the minor alert, its owner alone, once (Bob's hero ready first, Alice's later).
        $OwnSlot = if ($N -eq 1) { $BobSlot } else { $AliceSlot }
        # J8b.4: the level alerts (also "Hero message: ...") checked in "net hero xp".
        $ReadyLines = @($ClientLines | Select-String -Pattern "Hero message: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value } | Where-Object { $_ -notmatch "^(Paladin|Palier)" })
        $ReturnChecks["client $N : alert seen once for its own hero only ($($ReadyLines -join '; '))"] =
            $ReadyLines.Count -eq 1 -and $ReadyLines[0] -ceq "Votre héros est prêt à revenir (slot $OwnSlot)"
        $Probe = @($ClientLines | Select-String -Pattern "Hero return probe: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $Acted = @($ClientLines | Select-String -Pattern "Hero return order:|Hero return: slot|Hero return refused:")
        $ReturnChecks["client $N : nothing decided here: probe '$($Probe -join '; ')', $($Acted.Count) return line(s)"] =
            $Probe.Count -eq 1 -and ($Probe[0] -match "^net mode 3, buy refused, decide refused, decisions 0$" -or $Probe[0] -match "^net mode 3, no decision$") -and $Acted.Count -eq 0
    }
    $AllOk = (Report "net hero return" $ReturnChecks) -and $AllOk
    # The hero's block on the clients (J8a.5 loop 6): each hol.HUD.Report's "HUD hero:" line in order, in the "hero" section
    # (born, wounded, Bob's dead, 1 s later) and the "hero return" one (ready; Bob: no gold, 1 000 gold before his click,
    # after it; back). Its own hero only; never "unknown" once the server's clock is received (the match long started).
    $BlockChecks = [ordered]@{}
    $BlockLevel = 1
    $DelayBound = [int][Math]::Ceiling($ReturnDelay / 20)
    foreach ($NetClient in $NetClients) {
        $N = $NetClient.Client
        $ClientLines = @(Get-Content $NetClient.Log -ErrorAction SilentlyContinue)
        $ReadBlock = { param($Mark) @(Get-ClientSection $ClientLines $Mark | Select-String -Pattern "HUD hero: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() }) }
        $InHero = & $ReadBlock "hero"
        $InReturn = & $ReadBlock "hero return"
        $FullBlock = "alive level $BlockLevel hp $HeroMax/$HeroMax"
        $BlockChecks["client $N : 4 blocks in 'hero' ($($InHero.Count)), its own hero born: $FullBlock ($($InHero[0]))"] = $InHero.Count -eq 4 -and $InHero[0] -eq $FullBlock
        $Unknown = @($ClientLines | Select-String -SimpleMatch "HUD hero: unknown")
        $BlockChecks["client $N : never 'unknown' ($($Unknown.Count))"] = $Unknown.Count -eq 0
        if ($N -eq 1) {
            # Bob: wounded (the server's health), dead not ready (1 <= S <= the delay's bound, falling), ready (buyable iff
            # his gold covers the price), no gold (his click greyed), buyable before the click, alive after it, back.
            $BlockChecks["client $N : wounded, alive level $BlockLevel hp $TrueHp/$HeroMax ($($InHero[1]))"] = $TrueHp -gt 0 -and $InHero[1] -eq "alive level $BlockLevel hp $TrueHp/$HeroMax"
            $Dead = @($InHero[2..3] | ForEach-Object { if ("$_" -match "^dead ready in (\d+) s$") { [int]$Matches[1] } })
            $BlockChecks["client $N : dead, ready in S s, 1 <= S <= $DelayBound and falling ($($InHero[2]); $($InHero[3]))"] =
                $Dead.Count -eq 2 -and $Dead[0] -ge 1 -and $Dead[0] -le $DelayBound -and $Dead[1] -ge 1 -and $Dead[1] -lt $Dead[0]
            $Seen = & $ReadHeroReports @(Get-ClientSection $ClientLines "hero return")
            $ReadyGold = if ($Seen.Count -ge 1) { $Seen[0].Gold } else { $null }
            $ReadyPrice = if ("$($InReturn[0])" -match "^ready price (\d+) (buyable|no gold)$") { [int]$Matches[1] } else { -1 }
            $ReadyWord = if ($ReadyPrice -ge 0 -and $null -ne $ReadyGold -and $ReadyGold -ge $ReadyPrice) { "buyable" } else { "no gold" }
            $BlockChecks["client $N : 5 blocks in 'hero return' ($($InReturn.Count)), ready: $($InReturn[0]) (gold $($ReadyGold): $ReadyWord)"] =
                $InReturn.Count -eq 5 -and $ReadyPrice -gt 0 -and $ReadyPrice -le 150 -and $InReturn[0] -eq "ready price $ReadyPrice $ReadyWord"
            $NoGold = if ("$($InReturn[1])" -match "^ready price (\d+) no gold$") { [int]$Matches[1] } else { -1 }
            $Clicks = @(Get-ClientSection $ClientLines "hero return" | Select-String -Pattern "HUD click HeroReturnButton: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })
            # The console's order sent in the same frame: the server's price of its refusal (its gap to the shown one, as below:
            # the order taken a step after the client's estimate floored, the price falling, never above the shown one).
            $Refused = @($ServerLines | Select-String -Pattern "Hero return: slot $BobSlot cannot pay (\d+) gold" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
            $BlockChecks["client $N : gold 0, $($InReturn[1]) (the server's refusal at $($Refused -join ', ')), its click greyed ($($Clicks[0]))"] =
                $NoGold -gt 0 -and $NoGold -le $ReadyPrice -and $Refused.Count -eq 1 -and [Math]::Abs($NoGold - $Refused[0]) -le 2 -and $Clicks.Count -eq 2 -and $Clicks[0] -eq "hidden, greyed or unknown"
            $Shown = if ("$($InReturn[2])" -match "^ready price (\d+) buyable$") { [int]$Matches[1] } else { -1 }
            $Taken = if ($Bought.Count -eq 1) { $Bought[0] } else { -1 }
            $BlockChecks["client $N : 1 000 gold, $($InReturn[2]), its click sent ($($Clicks[1])), bought on the server for $Taken (shown $Shown, gap $($Shown - $Taken))"] =
                $Shown -gt 0 -and $Shown -le $NoGold -and $Clicks.Count -eq 2 -and $Clicks[1] -eq "done" -and $Taken -gt 0 -and [Math]::Abs($Shown - $Taken) -le 2
            $BlockChecks["client $N : after the click, $FullBlock ($($InReturn[3])), back ($($InReturn[4]))"] = $InReturn[3] -eq $FullBlock -and $InReturn[4] -eq $FullBlock
        }
        else {
            # Alice: her own block, never Bob's: alive while Bob's is dead, hers dead (just killed) once Bob's is ready, back.
            $BlockChecks["client $N : wounded and Bob's dead, her own alive ($($InHero[1..3] -join '; '))"] = @($InHero[1..3] | Where-Object { $_ -eq $FullBlock }).Count -eq 3
            $Own = if ("$($InReturn[0])" -match "^dead ready in (\d+) s$") { [int]$Matches[1] } else { -1 }
            $BlockChecks["client $N : 2 blocks in 'hero return' ($($InReturn.Count)), her own just killed: $($InReturn[0]) (1 <= S <= $DelayBound), back: $($InReturn[1])"] =
                $InReturn.Count -eq 2 -and $Own -ge 1 -and $Own -le $DelayBound -and $InReturn[1] -eq $FullBlock
        }
    }
    $AllOk = (Report "net hero block" $BlockChecks) -and $AllOk
    # J8b.3: Bob's hero at level 2 (xp 220) over the network: the level public to both clients, the XP to Bob alone, his
    # body's maximum at his level (486) on both, his block at level 2 on Bob's.
    $XPChecks = [ordered]@{}
    $XPServerLines = @(Get-ClientSection @(if (Test-Path $NetServerLog) { Get-Content $NetServerLog }) "net hero xp")
    $XPServer = & $ReadHeroReports $XPServerLines
    $XPTruth = if ($XPServer.Count -ge 1) { $XPServer[0].Truth[$BobSlot] } else { $null }
    $XPLevelUp = "$($XPServerLines | Select-String -Pattern "Hero level: slot $BobSlot (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })"
    $XPHp = if ($XPTruth) { $XPTruth.Hp } else { -1 }
    $XPChecks["server: Bob's hero level 2 at 486 / 486, then wounded alive, xp 220 ($XPLevelUp; $(if ($XPTruth) { "$($XPTruth.State) hp $XPHp xp $($XPTruth.Xp)" }))"] =
        ($XPLevelUp -eq "level 1 -> 2, hp 486 / 486") -and $XPTruth -and $XPTruth.State -eq "Alive" -and $XPHp -gt 400 -and $XPHp -lt 486 -and $XPTruth.Xp -eq 220
    foreach ($NetClient in $NetClients) {
        $N = $NetClient.Client
        $XPLines = @(Get-ClientSection @(Get-Content $NetClient.Log -ErrorAction SilentlyContinue) "hero xp")
        $Levels = @($XPLines | Select-String -Pattern "Hero sheet slot ${BobSlot}: state (\w+) level (\d+) hp (\d+)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) level $($_.Matches[0].Groups[2].Value) hp $($_.Matches[0].Groups[3].Value)" })
        $XPChecks["client $N : Bob's public sheet Alive level 2 hp $XPHp, the server's ($($Levels -join '; '))"] = ($Levels.Count -eq 1) -and ($Levels[0] -eq "Alive level 2 hp $XPHp")
        $Seen = & $ReadHeroReports $XPLines
        $Report = if ($Seen.Count -ge 1) { $Seen[0] } else { $null }
        if ($N -eq 1) {
            $XPChecks["client $N (Bob): his XP 220 seen ($(if ($Report -and $Report.Local) { "slot $($Report.Local.Slot) xp $($Report.Local.Xp)" }))"] =
                $Report -and $Report.Local -and $Report.Local.Slot -eq $BobSlot -and $Report.Local.Xp -eq 220
            $Block = "$($XPLines | Select-String -Pattern "HUD hero: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })"
            $XPChecks["client $N (Bob): his block at level 2 ($Block)"] = $Block -eq "alive level 2 hp $XPHp/486"
        }
        else {
            $OtherXp = if ($Report -and $Report.Other[$BobSlot]) { $Report.Other[$BobSlot].Xp } else { -1 }
            $XPChecks["client $N (Alice): Bob's XP hidden (xp $OtherXp)"] = $OtherXp -eq 0
        }
        $Where = $XPLines | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
        $BobHero = @([regex]::Matches("$Where", "slot $BobSlot at \(-?\d+, -?\d+\) [^;]*?hp (\d+)/(\d+)") | Where-Object { [int]$_.Groups[2].Value -gt 400 })
        $XPChecks["client $N : Bob's wounded hero shown at $XPHp / 486 ($(@($BobHero | ForEach-Object { "$($_.Groups[1].Value)/$($_.Groups[2].Value)" }) -join ', '))"] =
            ($BobHero.Count -eq 1) -and ($BobHero[0].Groups[1].Value -eq "$XPHp") -and ($BobHero[0].Groups[2].Value -eq "486")
        # J8b.4: his golden flash on both clients, the alerts and his XP's bar on Bob's alone.
        $Flashes = @($XPLines | Select-String -Pattern "Hero flash: slot (\d) level (\d -> \d)" | ForEach-Object { "slot $($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" })
        $XPChecks["client $N : Bob's hero's golden flash ($($Flashes -join '; '))"] = (($Flashes -join ";") -eq "slot $BobSlot 1 -> 2")
        $Alerts = @($XPLines | Select-String -Pattern "Hero message: (Paladin.*|Palier.*) \(slot (\d)\)$" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) (slot $($_.Matches[0].Groups[2].Value))" })
        $Sounds = @($XPLines | Select-String -Pattern "Hero level alert: slot (\d) level (\d -> \d) \(sound\)" | ForEach-Object { "slot $($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" })
        if ($N -eq 1) {
            $Bar = "$($XPLines | Select-String -Pattern "HUD hero xp: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value.Trim() })"
            $XPChecks["client $N (Bob): his alert and its sound ($($Alerts -join '; '); $($Sounds -join '; ')), his XP's bar ($Bar)"] =
                (($Alerts -join ";") -eq "Paladin-Commandant : niveau 2 (slot $BobSlot)") -and (($Sounds -join ";") -eq "slot $BobSlot 1 -> 2") -and ($Bar -eq "xp 220/600 bar 5%")
        }
        else {
            $XPChecks["client $N (Alice): no alert for Bob's hero ($($Alerts -join '; ')$($Sounds -join '; '))"] = ($Alerts.Count -eq 0) -and ($Sounds.Count -eq 0)
        }
    }
    $AllOk = (Report "net hero xp" $XPChecks) -and $AllOk
    # J8c.3: Bob's abilities over the network (J8-Q5): the server casts his cry once (the second refused) and plants his
    # banner; his ready steps reach Bob alone, the cries and the banner both clients, as the server has them.
    $AbChecks = [ordered]@{}
    $AbServerLines = @(Get-ClientSection @(if (Test-Path $NetServerLog) { Get-Content $NetServerLog }) "net hero abilities")
    $AbOrders = @($AbServerLines | Select-String -Pattern "Hero ability order: slot $BobSlot (cry|banner) at step \d+ (cast|refused)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" })
    $AbTrueReady = "$($AbServerLines | Select-String -Pattern "Hero true slot ${BobSlot}: .* ready (\S+)" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })"
    $AbTrueSheet = "$($AbServerLines | Select-String -Pattern "Hero sheet slot ${BobSlot}: .* (cries \d+ banner \S+ \S+ until -?\d+)" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })"
    $AbReadySteps = @("$AbTrueReady" -split "," | ForEach-Object { [int]$_ })
    $AbChecks["server: the cry cast, refused again, the banner cast ($($AbOrders -join ', ')); ready $AbTrueReady; $AbTrueSheet"] =
        (($AbOrders -join ",") -eq "cry cast,cry refused,banner cast") -and ($AbReadySteps.Count -eq 3) -and ($AbReadySteps[0] -eq 0) -and
        ($AbReadySteps[1] -gt 1200) -and ($AbReadySteps[2] -gt 2400) -and ($AbTrueSheet -match "^cries 1 banner 14665 299 until (\d+)$") -and
        ([int]$Matches[1] -eq $AbReadySteps[2] - 1500)
    foreach ($NetClient in $NetClients) {
        $N = $NetClient.Client
        $AbLines = @(Get-ClientSection @(Get-Content $NetClient.Log -ErrorAction SilentlyContinue) "hero abilities")
        $AbSheet = "$($AbLines | Select-String -Pattern "Hero sheet slot ${BobSlot}: .* (cries \d+ banner \S+ \S+ until -?\d+)" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })"
        $AbChecks["client $N : Bob's cries and banner as the server's ($AbSheet)"] = ($AbSheet -ne "") -and ($AbSheet -eq $AbTrueSheet)
        $AbShown = "$($AbLines | Select-String -Pattern "Banner display: (1 shown, slot ${BobSlot} \(14665, 299, -?\d+\))" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })"
        $AbChecks["client $N : Bob's banner drawn where it stands ($AbShown)"] = $AbShown -ne ""
        $AbCryFlashes = @($AbLines | Select-String -Pattern "Cry flash: slot ${BobSlot}, (\d+) units" | ForEach-Object { [int]$_.Matches[0].Groups[1].Value })
        $AbChecks["client $N : Bob's cry flashed once on his units ($($AbCryFlashes -join ', '))"] = $AbCryFlashes.Count -eq 1
        if ($N -eq 1) {
            $AbLocal = "$($AbLines | Select-String -Pattern "Hero private slot $BobSlot local: .* ready (\S+)" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })"
            $AbChecks["client $N (Bob): his ready steps as the server's ($AbLocal)"] = ($AbLocal -ne "") -and ($AbLocal -eq $AbTrueReady)
        }
        else {
            $AbOther = "$($AbLines | Select-String -Pattern "Hero private slot $BobSlot other: .* cooldowns (\d+)" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })"
            $AbChecks["client $N (Alice): Bob's cool-downs hidden (cooldowns $AbOther)"] = $AbOther -eq "0"
        }
    }
    $AllOk = (Report "net hero abilities" $AbChecks) -and $AllOk
}

if ($TestsProcess) {
    $TestsTimedOut = -not $TestsProcess.WaitForExit(300000)
    if ($TestsTimedOut) { $TestsProcess.Kill() }
    $Results = @(if (Test-Path $TestsLog) { Select-String -Path $TestsLog -Pattern "Test Completed\. Result=\{(\w+)\}.*Path=\{([^}]+)\}" | ForEach-Object { $_.Matches[0] } })
    $Failures = @($Results | Where-Object { $_.Groups[1].Value -ne "Success" } | ForEach-Object { $_.Groups[2].Value })
    # Every test started has completed (the forced exit of -TestExit cut the last lines of the log; SoftQuit flushes it).
    $Started = @(if (Test-Path $TestsLog) { Select-String -Path $TestsLog -Pattern "Test Started\. Name=" }).Count
    $Checks = [ordered]@{}
    $Checks["tests ran ($($Results.Count))"] = (-not $TestsTimedOut) -and $Results.Count -gt 0
    $Checks["every test started completed ($($Results.Count) of $Started)"] = $Results.Count -eq $Started
    $Checks["all green (failed: $($Failures -join ', '))"] = $Failures.Count -eq 0
    # The scripts (D484): a script that does not compile would drop its tests without failing one (the editor quits at
    # once on it, -as-exit-on-error, instead of waiting in its "fix the errors" window).
    $ScriptErrors = @(Get-ScriptErrors $TestsLog)
    $Checks["scripts compiled ($(if ($ScriptErrors.Count) { $ScriptErrors -join ' ' } else { 'no error' }))"] = $ScriptErrors.Count -eq 0
    $ScriptTests = @($Results | Where-Object { $_.Groups[2].Value -like "Angelscript.UnitTests.*" }).Count
    $Checks["script tests ran ($ScriptTests)"] = $ScriptTests -gt 0
    $AllOk = (Report "tests $($Results.Count - $Failures.Count)/$($Results.Count)" $Checks) -and $AllOk
}

# The "lobby" part (M6 R5.4): lobby_match.ps1 on its own, once the rest is done (its three games and the test manager
# would share the machine with the others): lobby, Ready, match, a spectator, give-up, Alice staying past 60 s in the finished match, back to the lobby (~150 s). It needs
# the cooked server of net_match.ps1 -Stage and the test manager of build_manager.ps1.
if ($Parts -contains "lobby") {
    $LobbyServer = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLibertyServer.exe"
    $Checks = [ordered]@{}
    if (-not (Test-Path $LobbyServer)) {
        $Checks["cooked server $LobbyServer present (run Tools\Tests\net_match.ps1 -Stage)"] = $false
    }
    else {
        $LobbyOutput = @(& powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "lobby_match.ps1") -EngineDir $EngineDir 2>&1 | ForEach-Object { "$_" })
        $LobbyLine = @($LobbyOutput | Where-Object { $_ -match "^lobby_match : " }) | Select-Object -Last 1
        if ($LASTEXITCODE -ne 0) { $LobbyOutput | Select-Object -Last 40 | ForEach-Object { Write-Host "  | $_" } }
        $Checks["lobby_match: $LobbyLine"] = ($LASTEXITCODE -eq 0) -and ("$LobbyLine" -match "^lobby_match : PASS")
        # A cooked server older than the code may not replicate as the clients do (Iris "protocol mismatch": Carol never
        # in the match, 2026-10-27): said only when it fails, as a re-stage takes minutes.
        $GameModule = Get-ChildItem (Join-Path $Project "Binaries\Win64") -Filter "UnrealEditor-HeroesOfLiberty*.dll" -ErrorAction SilentlyContinue |
            Sort-Object LastWriteTime | Select-Object -Last 1
        if (-not $Checks["lobby_match: $LobbyLine"] -and $GameModule -and (Get-Item $LobbyServer).LastWriteTime -lt $GameModule.LastWriteTime) {
            $Checks["cooked server newer than the code (it is older than $($GameModule.Name): run Tools\Tests\net_match.ps1 -StageOnly)"] = $false
        }
    }
    $AllOk = (Report "lobby" $Checks) -and $AllOk
}

# The "aiserver" part (IA0.5 B1, IA0.5-Q2 B): the AI's brain on a dedicated server, no client. The editor in server mode
# (as "net", D275) on Plain, slot 0 Human without a name (the match does not wait for it: R3.9 waits only for named
# players), slot 1 AI, 5 starting Peasants for everyone ("startingPeasants": 5 in the config, so $NoStartingPeasants is
# not passed). Once the rest is done (alone on the machine, ~30 s). The brain's lines, the births and the first order
# come before the sequence starts (-ExecCmds runs after the map's BeginPlay, and waitlog reads only what follows it):
# read from the server's log once it has quit. The sequence waits 5 s of play, asks slot 1's gathering, then quits.
# A free UDP port in this copy's range of 10 (as "net"), for "aiserver" and "netai" (each one alone on the machine by then).
function Get-AITestPort {
    $Index = @{ "MyCrusader" = 0; "HeroesOfLiberty-Elrond" = 1; "HeroesOfLiberty-Arwen" = 2; "HeroesOfLiberty-Glorfindel" = 3;
        "HeroesOfLiberty-Haldir" = 4; "HeroesOfLiberty-Galadriel" = 5; "HeroesOfLiberty-Celeborn" = 6 }[(Split-Path -Leaf $Project)]
    if ($null -eq $Index) { $Index = 7 }
    $Port = 7800 + 10 * $Index
    while ($Port -lt 7810 + 10 * $Index) {
        try { $Probe = New-Object Net.Sockets.UdpClient($Port); $Probe.Close(); break } catch { $Port++ }
    }
    return $Port
}
# The 8 slots of a version 2 match config on Plain: the states given, slots 0 and 1 named when names are given.
function Write-AIMatchConfig([string] $Path, [string[]] $States, [string[]] $Names = @()) {
    $Slots = for ($i = 0; $i -lt 8; $i++) {
        $Name = if ($i -lt $Names.Count) { ", `"playerName`": `"$($Names[$i])`"" } else { "" }
        "{ `"state`": `"$($States[$i])`", `"colorIndex`": $i$Name }"
    }
    "{ `"version`": 2, `"map`": `"/Game/Maps/Plain.Plain`", `"startingPeasants`": 5, `"slots`": [ $($Slots -join ', ') ] }" |
        Set-Content -Encoding ascii $Path
}
if ($Parts -contains "aiserver") {
    $AIServerLog = Join-Path $TestSaved "Logs\quick_aiserver.log"
    $AIServerConfig = Join-Path $TestSaved "quick_aiserver.json"
    $AIServerSequence = Join-Path $TestSaved "quick_aiserver.seq"
    Remove-Item $AIServerLog -ErrorAction SilentlyContinue
    Write-AIMatchConfig $AIServerConfig @("Human", "AI", "Closed", "Closed", "Closed", "Closed", "Closed", "Closed")
    @("wait 5", "hol.Gather.Report 1", "waitlog Gather: slot 1 to source", "hol.Units.Where 1", "waitlog Units where:", "quit") |
        Set-Content -Encoding utf8 $AIServerSequence
    # A free UDP port in this copy's range of 10 (as "net"; nobody joins, but two copies must not share one).
    $AIPort = Get-AITestPort
    $AIServerArguments = "`"$UProject`" /Game/Maps/Plain -server -Port=$AIPort -useperfthreads -unattended -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -nosound -saveddirsuffix=Tests_NetSrv -HOLCheats -MatchConfig=`"$AIServerConfig`" -abslog=`"$AIServerLog`" -ExecCmds=`"hol.Test.Run $AIServerSequence`""
    $AIServerProcess = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $AIServerArguments -PassThru -WindowStyle Hidden
    $AIServerTimedOut = -not $AIServerProcess.WaitForExit(180000)
    if ($AIServerTimedOut) { Stop-Process -Id $AIServerProcess.Id -Force -ErrorAction SilentlyContinue }
    $AILog = if (Test-Path $AIServerLog) { Get-Content $AIServerLog } else { @() }
    function Find-AILine([string] $Pattern) { @($AILog | Select-String -Pattern $Pattern | ForEach-Object { $_.Line }) | Select-Object -First 1 }
    $Checks = [ordered]@{}
    $Checks["server quit by itself in 180 s"] = -not $AIServerTimedOut
    $ScriptErrors = @(Get-ScriptErrors $AIServerLog)
    $Checks["scripts compiled ($(if ($ScriptErrors.Count) { $ScriptErrors -join ' ' } else { 'no error' }))"] = $ScriptErrors.Count -eq 0
    $Timeout = Find-AILine "Test sequence: timeout at step .*"
    $Checks["sequence ran to its end ($(if ($Timeout) { $Timeout.Trim() } else { 'no timeout' }))"] = -not $Timeout
    $Checks["5 starting Peasants for slot 1"] = [bool](Find-AILine "Starting Peasants of slot 1: 5 in front of its main forum")
    $Awake = Find-AILine "AI brain \S+ awake, net mode \w+, thinking slots \[[^\]]*\]"
    $Checks["brain awake on the dedicated server, slot 1 thinking ($(if ($Awake) { ($Awake -replace '^.*(AI brain )', '$1').Trim() } else { 'no awake line' }))"] =
        [bool]$Awake -and $Awake -match "net mode DedicatedServer, thinking slots \[1\]"
    $Checks["AI slot 1 thinks (AI slot 1 step)"] = [bool](Find-AILine "AI slot 1 step \d+:")
    $Checks["5 idle -> gather"] = [bool](Find-AILine "AI slot 1 step \d+: 5 idle -> gather \w+ #\d+")
    # IA1.5: the 5 starting Peasants spread, 3 on the food and 2 on the wood (two orders).
    # IA1.7: the first Peasant goes to build the lumber camp, the four others spread 2 and 2.
    $Checks["Gather order of slot 1: 2 units to source (food)"] = [bool](Find-AILine "Gather order of slot 1: 2 units to source \d+ \(Food\)")
    $Checks["Gather order of slot 1: 2 units to source (wood)"] = [bool](Find-AILine "Gather order of slot 1: 2 units to source \d+ \(Wood\)")
    # The report after 5 s: the Peasants sent are on their way, gathering or unloading (a starting hero, if any, stays
    # among the "not gathering").
    $GatherLine = Find-AILine "Gather: slot 1 to source \d+, gathering \d+, to deposit \d+, not gathering \d+"
    $Busy = 0
    if ($GatherLine -match "Gather: slot 1 to source (\d+), gathering (\d+), to deposit (\d+)") { $Busy = [int]$Matches[1] + [int]$Matches[2] + [int]$Matches[3] }
    $Checks["slot 1's Peasants at work ($(if ($GatherLine) { ($GatherLine -replace '^.*(Gather: slot 1)', '$1').Trim() } else { 'no report' }))"] = $Busy -ge 1
    $AllOk = (Report "aiserver" $Checks) -and $AllOk
}

# The "aiduel" part (IA0.7, IA-Q3): two AI slots against each other, no client, no window (the editor in server mode
# as "aiserver", on a fixed time step: the game's seconds go by faster than the clock). 2 min of the game's time, a
# gather report every 2 s; run twice, with hol.Simulation.StepHashes 1. Checks: no Peasant of an AI slot idle (phase
# None, in the report) for more than 10 s; the two runs' step hashes the same (the match is reproducible: places fixed
# on Plain, the brain reads its summary in a fixed order); the wall time of the 2 min of game (the speed measured).
function Invoke-AIDuelRun([string] $Tag, [int] $GameSeconds, [int] $Fps) {
    $Log = Join-Path $TestSaved "Logs\quick_aiduel_$Tag.log"
    $Config = Join-Path $TestSaved "quick_aiduel.json"
    $Sequence = Join-Path $TestSaved "quick_aiduel_$Tag.seq"
    Remove-Item $Log -ErrorAction SilentlyContinue
    Write-AIMatchConfig $Config @("AI", "AI", "Closed", "Closed", "Closed", "Closed", "Closed", "Closed")
    $Lines = @("hol.Simulation.StepHashes 1")
    for ($Second = 0; $Second -lt $GameSeconds; $Second += 2) { $Lines += @("hol.Gather.Report", "wait 2") }
    $Lines += @("hol.Gather.Report", "quit")
    $Lines | Set-Content -Encoding utf8 $Sequence
    $Port = Get-AITestPort
    $Arguments = "`"$UProject`" /Game/Maps/Plain -server -Port=$Port -useperfthreads -unattended -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -nosound -nullrhi -UseFixedTimeStep -FPS=$Fps -saveddirsuffix=Tests_NetSrv -HOLCheats -MatchConfig=`"$Config`" -abslog=`"$Log`" -ExecCmds=`"hol.Test.Run $Sequence`""
    $Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
    $TimedOut = -not $Process.WaitForExit(420000)
    if ($TimedOut) { Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue }
    return @{ Log = $Log; TimedOut = $TimedOut; Sequence = $Sequence }
}
if ($Parts -contains "aiduel") {
    $DuelFps = 20
    $DuelRuns = @()
    foreach ($Tag in "a", "b") {
        $Watch = [Diagnostics.Stopwatch]::StartNew()
        $Run = Invoke-AIDuelRun $Tag $DuelSeconds $DuelFps
        $Run.WallSeconds = $Watch.Elapsed.TotalSeconds
        $DuelRuns += $Run
    }
    $Checks = [ordered]@{}
    $Hashes = @()
    foreach ($Run in $DuelRuns) {
        $Text = if (Test-Path $Run.Log) { Get-Content $Run.Log } else { @() }
        $Checks["run $($Run.Log | Split-Path -Leaf): the server quit by itself"] = -not $Run.TimedOut
        $ScriptErrors = @(Get-ScriptErrors $Run.Log)
        $Checks["run $($Run.Log | Split-Path -Leaf): scripts compiled"] = $ScriptErrors.Count -eq 0
        $Timeout = $Text | Select-String -Pattern "Test sequence: timeout at step" | Select-Object -First 1
        $Checks["run $($Run.Log | Split-Path -Leaf): sequence ran to its end"] = -not $Timeout
        $Checks["run $($Run.Log | Split-Path -Leaf): thinking slots [0, 1]"] = [bool]($Text | Select-String -Pattern "thinking slots \[0, ?1\]" -Quiet)
        $Run.Hashes = @($Text | Select-String -Pattern "Step hash: step (\d+) ([0-9a-f]{16})" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) $($_.Matches[0].Groups[2].Value)" })
        # The idle time: the reports are 2 s of game apart; per slot, the longest run of reports with a Peasant idle.
        $Reports = @{}
        foreach ($Match in ($Text | Select-String -Pattern "Gather: slot (\d+) to source \d+, gathering \d+, to deposit \d+, not gathering (\d+) .*building (\d+)")) {
            $Slot = [int]$Match.Matches[0].Groups[1].Value
            if (-not $Reports.ContainsKey($Slot)) { $Reports[$Slot] = @() }
            # IA1.5: a Peasant on a building site is not idle (the opening's house, camp and mill).
            $Reports[$Slot] += [int]$Match.Matches[0].Groups[2].Value - [int]$Match.Matches[0].Groups[3].Value
        }
        $Run.MaxIdle = 0.0
        $Run.NumReports = 0
        foreach ($Slot in $Reports.Keys) {
            $Run.NumReports = [Math]::Max($Run.NumReports, $Reports[$Slot].Count)
            $Streak = 0
            # The starting hero is among the "not gathering": the Peasants idle are the ones above it.
            $Heroes = @($Text | Select-String -Pattern "Starting hero of slot $Slot spawned").Count
            foreach ($NotGathering in $Reports[$Slot]) {
                $Streak = if ($NotGathering -gt $Heroes) { $Streak + 1 } else { 0 }
                $Run.MaxIdle = [Math]::Max($Run.MaxIdle, 2.0 * $Streak)
            }
        }
    }
    $First = $DuelRuns[0]
    $Checks["gather reports taken ($($First.NumReports), one per 2 s)"] = $First.NumReports -ge ($DuelSeconds / 2)
    $Checks["no AI Peasant idle more than 10 s (longest $($First.MaxIdle) s and $($DuelRuns[1].MaxIdle) s)"] = $First.MaxIdle -le 10 -and $DuelRuns[1].MaxIdle -le 10
    # IA1.7: the status line of the brain (every 5 s per slot): the population rises past 20 (the houses come before the cap),
    # no queue stays blocked at the cap more than 10 s. (No cap on a stock: the user abolished it, 2026-10-29: the AI has
    # nothing to buy before IA2.)
    $FirstText = if (Test-Path $First.Log) { Get-Content $First.Log } else { @() }
    $Status = @{}
    foreach ($Match in ($FirstText | Select-String -Pattern "AI slot (\d+) step (\d+): status pop (\d+)/(\d+) queued (\d+) stock (\d+) (\d+) (\d+) (\d+) peasants (\d+) houses (\d+) camps (\d+) mills (\d+) farms (\d+)")) {
        $G = $Match.Matches[0].Groups
        $Slot = [int]$G[1].Value
        if (-not $Status.ContainsKey($Slot)) { $Status[$Slot] = @() }
        $Status[$Slot] += [pscustomobject]@{ Used = [int]$G[3].Value; Cap = [int]$G[4].Value; Queued = [int]$G[5].Value; Food = [int]$G[6].Value; Wood = [int]$G[7].Value; Stone = [int]$G[8].Value; Gold = [int]$G[9].Value; Peasants = [int]$G[10].Value; Houses = [int]$G[11].Value; Mills = [int]$G[13].Value; Farms = [int]$G[14].Value }
    }
    $MinLines = if ($Status.Count) { ($Status.Values | ForEach-Object { $_.Count } | Measure-Object -Minimum).Minimum } else { 0 }
    $Checks["AI status lines read (at least $MinLines per slot, $($Status.Count) slots)"] = $Status.Count -ge 2 -and $MinLines -ge ($DuelSeconds / 5 - 3)
    foreach ($Slot in ($Status.Keys | Sort-Object)) {
        $Rows = $Status[$Slot]
        $MaxUsed = ($Rows | Measure-Object -Property Used -Maximum).Maximum
        $Streak = 0; $MaxBlocked = 0
        foreach ($Row in $Rows) {
            $Streak = if ($Row.Used -ge $Row.Cap -and $Row.Queued -gt 0) { $Streak + 1 } else { 0 }
            $MaxBlocked = [Math]::Max($MaxBlocked, 5 * $Streak)
        }
        $Last = $Rows[-1]
        if ($DuelSeconds -ge 300) { $Checks["slot $Slot : population past 20 (max $MaxUsed of cap $($Last.Cap), $($Last.Houses) houses, $($Last.Peasants) Peasants)"] = $MaxUsed -gt 20 }
        $Checks["slot $Slot : no queue blocked at the cap more than 10 s (longest $MaxBlocked s)"] = $MaxBlocked -le 10
        Write-Host "  slot $Slot at the end: pop $($Last.Used)/$($Last.Cap), $($Last.Peasants) Peasants, stock F$($Last.Food) W$($Last.Wood) S$($Last.Stone) G$($Last.Gold), $($Last.Houses) houses, $($Last.Mills) mills, $($Last.Farms) farms"
    }
    $LastStep = if ($First.Hashes.Count) { ($First.Hashes[-1] -split ' ')[0] } else { "none" }
    $Checks["step hashes: $($First.Hashes.Count) steps (last $LastStep), 2 min of game = 2400 steps at least"] = $First.Hashes.Count -ge ($DuelSeconds * 20)
    $Same = ($First.Hashes.Count -gt 0) -and ($First.Hashes.Count -eq $DuelRuns[1].Hashes.Count) -and -not (Compare-Object $First.Hashes $DuelRuns[1].Hashes)
    $Checks["the two runs' step hashes are the same"] = $Same
    $Speed = if ($First.WallSeconds -gt 0) { [Math]::Round($DuelSeconds / $First.WallSeconds, 2) } else { 0 }
    $Checks["speed: $DuelSeconds s of game in $([Math]::Round($First.WallSeconds, 1)) s of wall (x$Speed, with the engine's start) and $([Math]::Round($DuelRuns[1].WallSeconds, 1)) s"] = $true
    $AllOk = (Report "aiduel" $Checks) -and $AllOk
}

# The "netai" part (IA0.5 B2, IA0.5-Q1 B, Q2 B): the AI's brain on a dedicated server with two clients. The server of
# "aiserver" (the editor in server mode on Plain, 5 starting Peasants for everyone) with Alice (slot 0) and Bob (slot 1)
# named in the config (the match waits for them, R3.9), and the AI in a third slot (slot 2: Plain has 8 places, 270 deg).
# Lighter than "net": no lobby manager, no capture; once the rest is done (alone on the machine, ~60 s). Each client,
# once both are in, waits 8 s and says where it has slot 2's units (the AI's Peasants on their way or gathering, from
# what the server replicated), then signals and quits; the server says the same, waits for both signals and quits. The
# server's lines (the brain, its first order, no order for the humans) read from its log once it has quit.
if ($Parts -contains "netai") {
    $NetAIServerLog = Join-Path $TestSaved "Logs\quick_netai_server.log"
    $NetAIConfig = Join-Path $TestSaved "quick_netai.json"
    $NetAIServerSequence = Join-Path $TestSaved "quick_netai_server.seq"
    Remove-Item $NetAIServerLog, (Join-Path $TestSaved "Logs\quick_netai_client*.log"), (Join-Path $TestSaved "quick_netai_*.flag") -ErrorAction SilentlyContinue
    # The slots: Alice and Bob, the AI in slot 2.
    $NetAIStates = @("Human", "Human", "AI", "Closed", "Closed", "Closed", "Closed", "Closed")
    Write-AIMatchConfig $NetAIConfig $NetAIStates @("Alice", "Bob")
    $NetAIDone = @(1, 2 | ForEach-Object { Join-Path $TestSaved "quick_netai_done$_.flag" })
    @("waitplayers 2", "mark netai", "wait 8", "hol.Gather.Report 2", "wait 1", "hol.Units.Where 2", "waitlog Units where:",
        "waitfile $($NetAIDone[0]) 240", "waitfile $($NetAIDone[1]) 240", "wait 1", "quit") | Set-Content -Encoding utf8 $NetAIServerSequence
    $NetAIPort = Get-AITestPort
    $NetAIServerArguments = "`"$UProject`" /Game/Maps/Plain -server -Port=$NetAIPort -useperfthreads -unattended -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -nosound -saveddirsuffix=Tests_NetSrv -HOLCheats -MatchConfig=`"$NetAIConfig`" -abslog=`"$NetAIServerLog`" -ExecCmds=`"hol.Test.Run $NetAIServerSequence`""
    $NetAIServer = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $NetAIServerArguments -PassThru -WindowStyle Hidden
    $NetAIListening = $false
    $Deadline = (Get-Date).AddSeconds(90)
    while (-not $NetAIListening -and (Get-Date) -lt $Deadline -and -not $NetAIServer.HasExited) {
        Start-Sleep -Milliseconds 250
        $NetAIListening = (Test-Path $NetAIServerLog) -and (Select-String -Path $NetAIServerLog -Pattern "listening on port" -Quiet)
    }
    # The clients as in "net" (their saved folders, display, names: Bob by -PlayerName=, Alice in the URL).
    $NetAIClients = @()
    if ($NetAIListening) {
        foreach ($Client in 1, 2) {
            $ClientSettings = Join-Path $Project "Saved_Tests_Net$Client\Config\WindowsEditor\GameUserSettings.ini"
            if (-not (Test-Path $ClientSettings)) {
                New-Item -ItemType Directory -Force (Split-Path $ClientSettings) | Out-Null
                (Get-Content (Join-Path $PSScriptRoot "GameUserSettings.ini")) `
                    -replace '^(DisplayIndex|LastUserConfirmedDisplayIndex)=.*', '$1=-1' | Set-Content -Encoding ascii $ClientSettings
            }
            $ClientSequence = Join-Path $TestSaved "quick_netai_client$Client.seq"
            @("waitmap Plain", "waitlog Camera above the start", "waitplayers 2", "mark netai", "wait 8", "hol.Units.Where 2", "wait 1", "hol.Units.Where 2",
                "signal $($NetAIDone[$Client - 1])", "mark end", "quit") | Set-Content -Encoding utf8 $ClientSequence
            $ClientLog = Join-Path $TestSaved "Logs\quick_netai_client$Client.log"
            $ClientDisplay = if ($Visible) { "-windowed -ResX=940 -ResY=530 -WinX=$(3940 + ($Client - 1) * 960) -WinY=1020" } else { "-RenderOffScreen -ResX=960 -ResY=540 -nosound" }
            $ClientUrl = if ($Client -eq 1) { "127.0.0.1:${NetAIPort} -PlayerName=Bob" } else { "127.0.0.1:${NetAIPort}?Name=Alice" }
            $ClientArguments = "`"$UProject`" $ClientUrl -game $ClientDisplay -saveddirsuffix=Tests_Net$Client -nosplash -LiveCoding=0 -asdebugport=0 -as-exit-on-error -culture=fr -abslog=`"$ClientLog`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Test.Run $ClientSequence`""
            $NetAIClients += @{ Client = $Client; Log = $ClientLog; Process = (Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $ClientArguments -PassThru) }
        }
    }
    $Checks = [ordered]@{}
    $Checks["server listening"] = $NetAIListening
    foreach ($NetClient in $NetAIClients) {
        $ClientTimedOut = -not $NetClient.Process.WaitForExit(240000)
        if ($ClientTimedOut) { Stop-Process -Id $NetClient.Process.Id -Force -ErrorAction SilentlyContinue }
        $N = $NetClient.Client
        $ClientLines = if (Test-Path $NetClient.Log) { Get-Content $NetClient.Log } else { @() }
        $Checks["client $N : sequence done"] = (-not $ClientTimedOut) -and [bool]($ClientLines | Select-String -Pattern "Test mark: end" -Quiet)
        $Held = $ClientLines | Select-String -Pattern "Camera above the start of slot (\d+) \(" | Select-Object -First 1 | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }
        $Checks["client $N : holds the slot of $(@('Bob', 'Alice')[$N - 1]) ($Held)"] = ($null -ne $Held) -and $Held -eq (2 - $N)
        # The last "Units where" of slot 2: its 5 Peasants on their gathering's way or at their source (its starting hero
        # stays idle, the brain only sends Peasants).
        $Where = $ClientLines | Select-String -Pattern "Units where: (.*)$" | Select-Object -Last 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
        $AIUnits = @([regex]::Matches("$Where", "#\d+ slot 2 at \([^)]*\) (\w+)") | ForEach-Object { $_.Groups[1].Value })
        # IA1.5: one Peasant is on the opening's house site (Building) while the 4 others gather.
        $AIBusy = @($AIUnits | Where-Object { $_ -in "GatherTrip", "Gathering", "Building" }).Count
        $Checks["client $N : the AI's 5 Peasants seen, at work ($AIBusy of $($AIUnits.Count): $(if ($Where) { $Where } else { 'no report' }))"] = $AIBusy -eq 5
        $ScriptErrors = @(Get-ScriptErrors $NetClient.Log)
        $Checks["client $N : scripts compiled ($(if ($ScriptErrors.Count) { $ScriptErrors -join ' ' } else { 'no error' }))"] = $ScriptErrors.Count -eq 0
    }
    # The server quits once both clients have signalled (killed only as a last resort).
    $NetAIServerTimedOut = -not $NetAIServer.WaitForExit($(if ($NetAIClients.Count) { 30000 } else { 1000 }))
    if ($NetAIServerTimedOut) { Stop-Process -Id $NetAIServer.Id -Force -ErrorAction SilentlyContinue }
    $Checks["server quit by itself"] = -not $NetAIServerTimedOut
    $NetAILog = if (Test-Path $NetAIServerLog) { Get-Content $NetAIServerLog } else { @() }
    function Find-NetAILine([string] $Pattern) { @($NetAILog | Select-String -Pattern $Pattern | ForEach-Object { $_.Line }) | Select-Object -First 1 }
    $ScriptErrors = @(Get-ScriptErrors $NetAIServerLog)
    $Checks["server : scripts compiled ($(if ($ScriptErrors.Count) { $ScriptErrors -join ' ' } else { 'no error' }))"] = $ScriptErrors.Count -eq 0
    $Timeout = Find-NetAILine "Test sequence: timeout at step .*"
    $Checks["server : sequence ran to its end ($(if ($Timeout) { $Timeout.Trim() } else { 'no timeout' }))"] = -not $Timeout
    $Checks["server : Alice holds slot 0, Bob slot 1"] = [bool](Find-NetAILine "Alice holds slot 0 ") -and [bool](Find-NetAILine "Bob holds slot 1 ")
    $Checks["server : 5 starting Peasants for slot 2"] = [bool](Find-NetAILine "Starting Peasants of slot 2: 5 in front of its main forum")
    $Awake = Find-NetAILine "AI brain \S+ awake, net mode \w+, thinking slots \[[^\]]*\]"
    $Checks["server : brain awake on the dedicated server, slot 2 thinking ($(if ($Awake) { ($Awake -replace '^.*(AI brain )', '$1').Trim() } else { 'no awake line' }))"] =
        [bool]$Awake -and $Awake -match "net mode DedicatedServer, thinking slots \[2\]"
    $Checks["server : AI slot 2 thinks (AI slot 2 step)"] = [bool](Find-NetAILine "AI slot 2 step \d+:")
    $Checks["server : 5 idle -> gather"] = [bool](Find-NetAILine "AI slot 2 step \d+: 5 idle -> gather \w+ #\d+")
    $Checks["server : Gather order of slot 2: 2 units to source (food)"] = [bool](Find-NetAILine "Gather order of slot 2: 2 units to source \d+ \(Food\)")
    $Checks["server : Gather order of slot 2: 2 units to source (wood)"] = [bool](Find-NetAILine "Gather order of slot 2: 2 units to source \d+ \(Wood\)")
    # The humans' Peasants are theirs: the AI orders none of them.
    $HumanOrder = Find-NetAILine "Gather order of slot [01]:"
    $Checks["server : no gather order for the humans' slots ($(if ($HumanOrder) { ($HumanOrder -replace '^.*(Gather order)', '$1').Trim() } else { 'none' }))"] = -not $HumanOrder
    $AllOk = (Report "netai" $Checks) -and $AllOk
}

Write-Host "quick_check : $(if ($AllOk) { 'PASS' } else { 'FAIL' }) in $([Math]::Round($Clock.Elapsed.TotalSeconds)) s (game launch done at $GameSeconds s)"
if (-not $AllOk) { exit 1 }
