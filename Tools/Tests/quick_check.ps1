# Quick check of the project (D267), on two levels (D327):
#   - by default (no argument, under a minute): the automation tests in their own process WHILE a single short game goes
#     through the "smoke" part: the title and New game screens in French, a 2-player match launched from there, the
#     camera moved by the arrow key, its own centre selected then nothing, 3 Men-at-arms boxed and sent beside an enemy
#     Peasant, which they kill;
#   - -Full (~530 s): every part below, the default before D327 (at a closure, or when a part's ground changed);
#   - -Parts a,b,c: those parts alone (tests, smoke, menu, camera, production, move, sources, gather, build, anims,
#     formation, combat, net). The solo parts (production to combat) run in the camera part's 2-player match, so the
#     camera part comes with them; each one counts what the parts before it left (Peasants, sources used up). "net" runs
#     alone (~95 s: its camera, centres, production, army and fight); with "move", "sources" or "build" its clients also
#     go through their orders, gathering, houses and camps (the server's commands for those come with them).
# The full run: a single game launch goes through the menu in French, English and Japanese (language switched live), a
# drop-down list, then a match at 2 and at 8 players with the camera checks of D257 and the selection checks of D264 (own
# centre, another's, void, Escape).
# Every step starts as soon as the one before is done (hol.Test.Run), no fixed delays. Replaced capture_screens.ps1,
# check_camera.ps1 and the automation command for everyday checks; the short stress tests S1 and S2 stay apart (only
# when units, the simulation or rendering change, and at a closure).
# Alongside, the "net" part (D273, M2.5.6): a two-client match on a dedicated server, the editor in server mode (D275),
# each client checking its camera, the two centres in their colours and a click on each.
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
# on D (a right click on the site), D cancelled after its first work (half refunded), its builders idle. With "net": each
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
# a camp 30 m from any wood: its builders stand idle.
# J6b.2 (D314), solo: six Peasants on round trips between the vein north-east of slot 0 and its centre, 50 m (their kept
# paths, D305); house K placed across their way and started: the kept paths across it dropped, those walking across it
# given a new path once its tiles are rebuilt (one log line), no body in a footprint after; six Peasants sent 90 m
# across open ground, house N started across their way: they go round it and arrive.
# J6b.2b (D324), solo: houses P and Q whose tiles touch, their bodies inset 115 cm in them (2.3 m apart): the navmesh's
# paths go through the way between them; one Peasant sent across passes through it, on its middle line; two sent together
# pass one after the other; none put back by a wall, none given a new path, all arrive.
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths), with no editor of the project open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1 [-Full | -Parts net,sources,move] [-EngineDir <dir>] [-Visible]
# (-File passes a list as one string: the script splits it at the commas.)
# The game renders off screen, with no window (the user works on the machine meanwhile); -Visible shows it on the
# second screen instead, windowed 1600 x 900, and the two clients side by side under it.
# The engine is the project's (EngineAssociation of the .uproject, found in the registry), unless -EngineDir is given.
# Results: a PASS / FAIL line per part, captures in Saved_Tests\Screenshots\WindowsEditor\HOL (qc_*, cam_*, sel_*),
# contact sheets qc_<culture>.png (Python with Pillow), logs Saved_Tests\Logs\quick_game.log and quick_tests.log.
param(
    [string[]] $Parts = @("tests", "smoke"),
    [switch] $Full,
    [string] $EngineDir = "",
    [switch] $Visible
)
# -Full (D327): every part, the default before D327 (~530 s).
if ($Full) { $Parts = @("tests", "menu", "camera", "production", "move", "sources", "gather", "build", "anims", "formation", "combat", "net") }
# A list passed by -File arrives as one string ("net,sources"): split it.
$Parts = @($Parts | ForEach-Object { $_ -split "," } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
# The solo parts run in the camera part's 2-player match: asked alone, the camera part comes with them.
if (@($Parts | Where-Object { $_ -in "production", "move", "sources", "gather", "build", "anims", "formation", "combat" }).Count -and -not ($Parts -contains "camera")) {
    $Parts += "camera"
}
Write-Host "parts: $($Parts -join ', ')"

$Clock = [Diagnostics.Stopwatch]::StartNew()
$Project = Resolve-Path "$PSScriptRoot\..\.."
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
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
$TestsProcess = $null
if ($Parts -contains "tests") {
    $TestsArguments = "`"$UProject`" -ExecCmds=`"Automation RunTests HeroesOfLiberty;SoftQuit`" -saveddirsuffix=Tests -unattended -nullrhi -nosplash -LiveCoding=0 -nosound -abslog=`"$TestsLog`""
    $TestsProcess = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor-Cmd.exe") -ArgumentList $TestsArguments -PassThru -WindowStyle Hidden
}

# 3. (started now, the clients join further down) Two-client match on a dedicated server (D273): the editor in dedicated
# server mode on Plain (D275: no cooking, always the code of the moment; the cooked server of net_match.ps1 stays for
# S5 and the user's trials), two humans in slots 0 and 1 at the colours of their slots.
$NetServerProcess = $null
$NetServerLog = Join-Path $TestSaved "Logs\quick_net_server.log"
if ($Parts -contains "net") {
    $NetConfig = Join-Path $TestSaved "quick_net.json"
    $NetSlots = for ($i = 0; $i -lt 8; $i++) { "{ `"state`": `"$(if ($i -lt 2) { 'Human' } else { 'Closed' })`", `"colorIndex`": $i }" }
    "{ `"map`": `"/Game/Maps/Plain.Plain`", `"slots`": [ $($NetSlots -join ', ') ] }" | Set-Content -Encoding ascii $NetConfig
    Remove-Item $NetServerLog, (Join-Path $TestSaved "Logs\quick_net_client*.log"), (Join-Path $TestSaved "quick_net_*.flag") -ErrorAction SilentlyContinue
    # Sources (J5a.2): 5 s after both clients joined, the server uses up the first tree of the wood beside slot 0.
    # Gathering (J5a.5): a second later, it leaves 5 wood in the tree nearest each start (the one each client's Peasant
    # will gather), so that the gathering uses it up within seconds; every unload logged.
    $NetSourceCommand = if ($Parts -contains "sources") {
        ",hol.Net.WhenClients 2 hol.Delay 5 hol.Sources.Take 0" +
        ",hol.Net.WhenClients 2 hol.Delay 6 hol.Sources.TakeNear -15000 0 95,hol.Net.WhenClients 2 hol.Delay 6 hol.Sources.TakeNear 15000 0 95" +
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
    $NetServerSequence = Join-Path $TestSaved "quick_net_server.seq"
    @("waitfile $NetFightFlag 900", "mark net fight", "hol.Combat.LogHits 1", "hol.Units.Spawn 2 -6300 -5500 0 ManAtArms 150",
        "hol.Units.Spawn 2 -5700 -5500 1 Spearman 150") + @(for ($i = 0; $i -lt 30; $i++) { "wait 1"; "hol.Units.Where" }) +
        @("waitfile $NetArrowsFlag 300", "mark net arrows", "hol.Units.Spawn 3 -2000 -5000 0 Archer 150", "hol.Units.Spawn 2 -2000 -3600 1 Spearman 150", "wait 12",
        "hol.Units.Remove -2000 -4300 1500 1", "hol.Units.Remove -2000 -4300 1500 0", "mark net arrows end", "signal $NetArrowsEndFlag", "hol.Units.CorpseReport") |
        Set-Content -Encoding utf8 $NetServerSequence
    $NetSourceCommand += ",hol.Test.Run $NetServerSequence"
    # -useperfthreads (J7c.4): a dedicated server, the editor's -server too, runs every ParallelFor on one thread by the
    # engine's default (FApp::ShouldUseThreadingForPerformance); the parallel processors of D302 run as in the game.
    $NetServerArguments = "`"$UProject`" /Game/Maps/Plain -server -useperfthreads -unattended -nosplash -LiveCoding=0 -nosound -saveddirsuffix=Tests_NetSrv -MatchConfig=`"$NetConfig`" -abslog=`"$NetServerLog`" -ExecCmds=`"hol.Prod.BuildTime 2$NetSourceCommand`""
    $NetServerProcess = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $NetServerArguments -PassThru -WindowStyle Hidden
}

# 2. One game launch for the menu and the camera.
$Steps = New-Object System.Collections.Generic.List[string]
function Step([string[]] $Lines) { $Lines | ForEach-Object { $Steps.Add($_) } }
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
        }
        Back
        for ($Tab = 0; $Tab -lt 4; $Tab++) {
            Open "Settings:$Tab"; Step "capture qc_${Culture}_$($MenuScreens[3 + $Tab])"
        }
        Back
    }
    Step "hol.UI.Language fr", "waitui"
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
$CampFar = @(-19200, -800)
$CampFarSpawn = @(-19200, -2000)
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
# Then the block march (the user's 1st trial of J7a.4): 3 Peasants, 3 Spearmen and 3 Men-at-arms (600, 583, 523 cm/s) on
# the six's first ground (gone by then), sent to the same point 35 m east.
$FormBlockPeasants = @(-6000, -5000)
$FormBlockSpearmen = @(-6000, -5500)
$FormBlockMenAtArms = @(-6000, -6000)
# Then the straggler (the user's 1st trial of J7b), after the groups: 5 Men-at-arms close together on the six's first
# ground (gone by then) and a 6th 10 m behind them (west), sent 37 m east-north-east (between the block's end and the
# six's queued point, more than 12 m from every enemy of the combat parts): the five leave at once, the 6th walks on its
# own path to its place. Then the same with an Archer (584 cm/s against 523) as the 6th, sent 35 m north-east.
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
# The melee (J7b.2), solo: 3 Men-at-arms of slot 0 and 3 Spearmen of slot 1, 6 m apart on the formation's first ground.
$CombatMenAtArms = @(-6300, -5500)
$CombatSpearmen = @(-5700, -5500)
# The attack-move and hold position (J7b.3), on ground clear of the parts before (more than 12 m from their soldiers).
$OrdersMoveFrom = @(-6500, -1500)
$OrdersMoveTo = @(-3500, -1500)
$OrdersMoveSpearman = @(-5000, -1100)
$OrdersHold = @(-4500, 2500)
# (Spawn places a unit up to ~1.5 m off its point: the Peasant 5.5 m away, out of reach whatever the draw, within 12 m.)
$OrdersHoldPeasant = @(-3950, 2500)
$OrdersHoldSpearman = @(-4500, 3300)
# Two enemies never push each other back (the user's trial of J7b), on the attack-move's start (left by then): 3 Men-at-arms
# of slot 0 ordered on a Peasant of slot 1 standing behind 12 others of slot 1 (no aggression: they stand, never strike).
$PushMenAtArms = @(-6500, -1500)
$PushWall = @(-5700, -1500)
$PushTarget = @(-5000, -1500)
# A queue after an ordered target (the review of J7b): 3 Men-at-arms made for it on clear ground (more than 12 m from
# every unit left by the parts before: the push's wall is still being fought, the hold's Peasant still stands) ordered on
# a Peasant of slot 1 15 m north (A), then with Shift on another 15 m further north (B, out of their 12 m once A is dead):
# B is attacked once A is killed; then a Peasant made 6 m beyond B (C) is taken on by their own aggression.
$QueueMenAtArms = @(-2000, 0)
$QueueA = @(-2000, 1500)
$QueueB = @(-2000, 3000)
$QueueC = @(-2000, 3600)
# The shot (J7c.2a): 4 Archers of slot 0 on clear ground south (more than 16 m from the units left), 3 Spearmen of slot 1
# 14 m north of them and 2 Peasants 10 m north-east.
$ArrowArchers = @(-2000, -5000)
$ArrowSpearmen = @(-2000, -3600)
$ArrowPeasants = @(-1300, -4300)
# The target that flees (J7c.2b): on the shot's ground once cleared, the Spearman 15 m north of the 3 Archers, sent 55 m north.
$FleeArchers = @(-2000, -5000)
$FleeSpearman = @(-2000, -3500)
$FleeTo = @(-2000, 2000)
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
# from a Peasant of slot 1 (out of their 12 m aggression), boxed and sent 4 m short of it: once there they take it on and
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
        "hol.Combat.Enabled 0", "mark smoke dying", "wait 1.5", "hol.Units.Report 200", "mark smoke end"
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
            Step "waitidle $($GatherBefore + $GatherBerries + 1)", "wait 1", "frames 2", "hol.Units.Where 0", "hol.Prod.Report", "hol.HUD.Report"
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
            # Nothing selected: the placements of J6a.3 send no builder (J6a.4). Escape logs nothing when nothing is selected
            # (the build part asked alone): the report says it, after a mark (the selection part counts its reports).
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
            # walkers crossing it get a new path (the log line); watched 12 s (every body put back by a footprint logged).
            Key Escape
            Step "mark build cut", "waitlog Selection: none", "hol.Units.SpawnSeed 109", "hol.Gather.SpeedScale 5", "hol.Units.Spawn 6 $($CutSpawn -join ' ') 0 Peasant 100", "wait 0.5",
                "hol.Camera.Focus $($CutSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CutSpawn[0] - 350) $($CutSpawn[1] - 300) $($CutSpawn[0] + 350) $($CutSpawn[1] + 300)", "waitlog Selection: 6 units (own): Peasant x6",
                "hol.Select.OrderSource $($CutVein -join ' ')", "waitlog Gather order of slot 0: 6 units to source", "wait 25",
                "hol.Units.Spawn 1 $($CutKBuilder -join ' ') 0 Peasant 10", "wait 0.5", "hol.Camera.Focus $($CutK -join ' ')", "frames 3", "hol.Select.ClickUnit -1",
                "waitlog Selection: Peasant #", "hol.Build.Place House", "waitlog Placement ghost: House at ($($CutK -join ', ')) valid", "hol.Build.Click",
                "waitlog Build order of slot 0: 1 units to House at ($($CutK -join ', '))", "waitlog Construction: slot 0 first work on House at ($($CutK -join ', '))",
                "waitlog Buildings (D310): footprint House at ($($CutK -join ', '))", "hol.Nav.Footprints on", "hol.Units.KeepOut 2", "wait 1.5",
                "hol.Camera.Focus $($CutK[0]) $($CutK[1] + 200)", "frames 3", "capture build_cut_gather", "wait 10", "hol.Nav.Footprints off", "hol.Units.KeepOut 1",
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
            # 12 m aggression drew it off the Men-at-arms' way), the gap's three 15 m from it. Waited for without their counts
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
            # (captured); its rally point by a right click; A, Z then E (2 s each): all three born, they walk to the rally point
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
            Key E
            Step "waitlog Production: Archer of slot 0 done", "hol.Prod.BuildTime 0", "wait 9", "hol.Units.Where 0", "hol.Prod.Report", "frames 3", "capture build_barracks_soldiers"
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
            Step "waitlog Selection: none"
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
            # Each group selected by a box around it, a right click on its source.
            foreach ($Group in $AnimGroups) {
                Step "hol.Camera.Focus $($Group.Spawn -join ' ')", "frames 3",
                    "hol.Select.BoxGround $($Group.Spawn[0] - 400) $($Group.Spawn[1] - 400) $($Group.Spawn[0] + 400) $($Group.Spawn[1] + 400)",
                    "waitlog Selection: 3 units (own): Peasant x3", "hol.Select.OrderSource $($Group.Source -join ' ')", "waitlog Gather order of slot 0: 3 units to source"
            }
            # At their sources within ~1.5 s (spawned 5 to 8 m away): all at work, the report, close-ups.
            Step "wait 3", "mark anims working", "hol.Units.AnimReport", "hol.Units.Where 0", "hol.Camera.Zoom 14"
            foreach ($Group in $AnimGroups) { Step "hol.Camera.Focus $($Group.Look -join ' ')", "frames 3", "capture anims_work_$($Group.Name)" }
            Step "hol.Camera.Focus $($AnimHouse[0] - 350) $($AnimHouse[1])", "frames 3", "capture anims_work_build",
                "hol.Camera.Focus $($AnimIdle -join ' ')", "frames 3", "capture anims_work_idle",
                "hol.Camera.Focus $($AnimSpearmen[0]) $(($AnimSpearmen[1] + $AnimMenAtArms[1]) / 2)", "frames 3", "capture anims_soldiers_idle",
                "hol.Camera.Focus $(($AnimOthers[0] + $AnimSpearmen[0]) / 2) $($AnimOthers[1])", "frames 3", "capture anims_colors", "hol.Camera.Zoom -14", "wait 3"
            # Each group (a few wood, stone, gold or food each by now: 0.7 a second) selected around its source, sent ~20 m
            # away: it keeps its load (D296). Reported on the way, then once they stand there.
            foreach ($Group in $AnimGroups) {
                Step "hol.Camera.Focus $($Group.Source -join ' ')", "frames 3",
                    "hol.Select.BoxGround $($Group.Source[0] - 700) $($Group.Source[1] - 700) $($Group.Source[0] + 700) $($Group.Source[1] + 700)",
                    "waitlog Selection: ", "hol.Select.OrderGround $($Group.Target -join ' ')", "waitlog Order: "
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
            Step "hol.Camera.Zoom -14", "hol.Build.Report", "hol.Select.Click 0.03 0.05", "frames 3"
        }
        if (($Parts -contains "formation") -and $Case.Name -eq "2p") {
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
        }
        if (($Parts -contains "combat") -and $Case.Name -eq "2p") {
            # The melee (J7b.2), solo: 3 Men-at-arms of slot 0 and 3 Spearmen of slot 1 made 6 m apart on the formation's
            # first ground (left by then): they take each other on by themselves (aggression 12 m), walk into reach and strike.
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
            # their way (within the attack-move's 12 m): they take it on, kill it, then walk on to their destination.
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
            # 16 m aggression): they shoot by themselves; each arrow lands after its flight (distance at release / 25 m/s),
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
            # and a Spearman of slot 1 15 m off; as soon as the first arrow is released the Spearman runs north: the arrow
            # still lands on it (damage at the landing), the drawn arrow follows it and is gone on it.
            Step "mark combat arrows flee", "hol.Units.Remove $($FleeArchers[0]) $($FleeArchers[1]) 12000 1", "wait 0.2", "hol.Cheat.UnitSpeed 1.5", "hol.Units.SpawnSeed 132", "hol.Camera.Focus $($FleeArchers[0]) $($FleeArchers[1] + 900)",
                "hol.Units.Spawn 3 $($FleeArchers -join ' ') 0 Archer 150", "hol.Units.Spawn 1 $($FleeSpearman -join ' ') 1 Spearman 150", "hol.Camera.Zoom 12",
                "waitlog releases an arrow", "hol.Units.Move $($FleeTo -join ' ') 1", "hol.Units.Where 1", "hol.Units.Where 0", "wait 0.1", "capture combat_flee_1",
                "hol.Units.Where 1", "capture combat_flee_2", "hol.Units.Where 1", "hol.Units.Where 0", "wait 0.25", "hol.Units.Where 1", "hol.Units.Where 0", "hol.Camera.Zoom -12", "wait 2", "hol.Units.Where 1", "hol.Units.Where 0",
                "hol.Combat.ArrowReport", "hol.Units.Remove $($FleeArchers[0]) $($FleeArchers[1]) 6000 1", "hol.Units.Remove $($FleeArchers[0]) $($FleeArchers[1]) 6000 0",
                "hol.Cheat.UnitSpeed 1", "wait 1", "hol.Combat.ArrowReport", "hol.Combat.LogHits 0", "mark combat arrows flee end"
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
$GameArguments = "`"$UProject`" -game $Display -saveddirsuffix=Tests -nosplash -LiveCoding=0 -culture=fr -abslog=`"$GameLog`" -ExecCmds=`"hol.Combat.Enabled 0,hol.Test.Run $SequenceFile`""
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
    for ($Client = 1; $Client -le 2; $Client++) {
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
        $NetSteps = @("waitmap Plain", "waitlog Camera above the start", "waitplayers 2", "mark net", "frames 30",
            "hol.Camera.Report", "hol.Buildings.Report", "capture qcnet_${Client}_start")
        foreach ($Slot in 0, 1) {
            $NetSteps += "hol.Camera.FocusSlot $Slot", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot $Slot",
                "frames 2", "hol.Select.Report", "capture qcnet_${Client}_slot$Slot"
        }
        $NetSteps += "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "hol.Select.Report"
        # Production (D284): each client has its centre make a Peasant (2 s, the server's hol.Prod.BuildTime), then both
        # Peasants must be shown, and the other player's resources must not have reached this client (D282).
        $NetSteps += "hol.Prod.Queue", "waitunits 2", "wait 1", "hol.Units.Report", "hol.Prod.Report", "capture qcnet_${Client}_peasants"
        # Orders (J4, D288, D289): each client sends every unit it knows, the other player's Peasant too, 25 m beside its
        # start; the server moves its own alone; then each client must see both Peasants where their owners sent them.
        if ($Parts -contains "move") {
            $NetSteps += "hol.Units.OrderNear -1000 2500 all", "wait 12", "hol.Units.Where", "capture qcnet_${Client}_moved"
        }
        # Sources (J5a.2): the 376 sources received, the tree the server used up gone (5 s after both joined, long done).
        if ($Parts -contains "sources") {
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
        $Other = 3 - $Client
        if ($Parts -contains "build") {
            $NetSteps += "mark build", "hol.Units.OrderNear 1500 2500", "wait 3", "hol.Prod.Report", "hol.Select.Own", "waitlog Select own: 1 units", "hol.Build.PlaceNear House $($NetHouseNear -join ' ')",
                "signal $(Join-Path $TestSaved "quick_net_placed$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_placed$Other.flag")", "wait 2",
                "hol.Build.Report", "hol.Prod.Report", "hol.Camera.FocusSlot 0", "frames 5", "capture qcnet_${Client}_build0", "hol.Camera.FocusSlot 1", "frames 5",
                "capture qcnet_${Client}_build1", "waitlog Building arrived: House of slot", "wait 1", "hol.Build.Report", "hol.Camera.FocusSlot 0", "frames 5",
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
        $NetSteps += "signal $(Join-Path $TestSaved "quick_net_done$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_done$Other.flag")", "hol.Combat.ArrowReport", "mark end", "quit"
        $ClientSequence = Join-Path $TestSaved "quick_net_client$Client.seq"
        $NetSteps | Set-Content -Encoding utf8 $ClientSequence
        $ClientLog = Join-Path $TestSaved "Logs\quick_net_client$Client.log"
        $ClientDisplay = if ($Visible) { "-windowed -ResX=940 -ResY=530 -WinX=$(3940 + ($Client - 1) * 960) -WinY=1020" } else { "-RenderOffScreen -ResX=960 -ResY=540 -nosound" }
        $ClientArguments = "`"$UProject`" 127.0.0.1 -game $ClientDisplay -saveddirsuffix=Tests_Net$Client -nosplash -LiveCoding=0 -culture=fr -abslog=`"$ClientLog`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Prod.BuildTime 2,hol.Gather.LogPopups 1,hol.Test.Run $ClientSequence`""
        $NetClients += @{ Client = $Client; Log = $ClientLog; Shots = $ClientShots;
            Process = (Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $ClientArguments -PassThru) }
    }
}

# 780 s with the camera's parts (-Full): the sequence takes ~285 s since J6b.2b (the build gap part), ~30 s more with the
# barracks (J7a.2), the start ~20 s; past 480 s with the combat and orders parts (J7b.3: attack-move and hold, ~45 s more);
# ~660 s with the straggler's groups (J7b trial feedback 1, ~105 s more), when 660 s cut the last parts off.
# 120 s otherwise: the default's smoke part is done ~40 s after the launch (D327), the menu part ~15 s.
$GameLimitSeconds = if ($Parts -contains "camera") { 780 } else { 120 }
$GameTimedOut = -not $GameProcess.WaitForExit($GameLimitSeconds * 1000)
if ($GameTimedOut) { $GameProcess.Kill() }
$GameSeconds = [Math]::Round($Clock.Elapsed.TotalSeconds)

$AllOk = $true
function Report([string] $Part, [System.Collections.Specialized.OrderedDictionary] $Checks) {
    $Failed = @($Checks.Keys | Where-Object { -not $Checks[$_] })
    Write-Host "$Part : $(if ($Failed.Count -eq 0) { 'PASS' } else { 'FAIL' })"
    $Checks.Keys | Where-Object { -not $Checks[$_] } | ForEach-Object { Write-Host "  [KO] $_" }
    return $Failed.Count -eq 0
}

$Log = if (Test-Path $GameLog) { Get-Content $GameLog } else { @() }
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
# Each capture shows the screen it is named after (the screens on top of the layers when it was taken; in a match,
# the game HUD of J3).
$Expected = @{ title = "menu=HOLTitleScreen modal=none"; quit = "menu=HOLTitleScreen modal=HOLConfirmDialog";
    newgame = "menu=HOLNewGameScreen modal=none"; dropdown = "menu=HOLNewGameScreen modal=none"; video = "menu=HOLSettingsScreen modal=none";
    sound = "menu=HOLSettingsScreen modal=none"; controls = "menu=HOLSettingsScreen modal=none"; language = "menu=HOLSettingsScreen modal=none";
    start = "hud=HOLGameScreen"; end = "hud=HOLGameScreen"; own = "hud=HOLGameScreen"; other = "hud=HOLGameScreen";
    queue = "hud=HOLGameScreen"; done = "hud=HOLGameScreen"; berries = "hud=HOLGameScreen"; wood = "hud=HOLGameScreen"; idle = "hud=HOLGameScreen";
    popup = "hud=HOLGameScreen"; green = "hud=HOLGameScreen"; red = "hud=HOLGameScreen"; foundations = "hud=HOLGameScreen";
    site = "hud=HOLGameScreen"; three = "hud=HOLGameScreen"; finished = "hud=HOLGameScreen"; rally = "hud=HOLGameScreen";
    greyed = "hud=HOLGameScreen"; fr = "hud=HOLGameScreen"; en = "hud=HOLGameScreen"; ja = "hud=HOLGameScreen"; ghost = "hud=HOLGameScreen";
    block = "hud=HOLGameScreen"; popfull = "hud=HOLGameScreen"; f1 = "hud=HOLGameScreen"; f2 = "hud=HOLGameScreen"; next = "hud=HOLGameScreen";
    stone = "hud=HOLGameScreen"; play = "hud=HOLGameScreen"; melee = "hud=HOLGameScreen" }
$WrongScreens = @($Log | Select-String -Pattern "Test capture: (\S+) \((.*)\)" | ForEach-Object {
    $Capture = $_.Matches[0].Groups[1].Value; $Shown = $_.Matches[0].Groups[2].Value
    $Want = $Expected[($Capture -split "_")[-1]]
    if ($Want -and $Shown -ne $Want) { "$Capture shows $Shown" } })
$SequenceChecks["captures show the right screen ($($WrongScreens -join '; '))"] = $WrongScreens.Count -eq 0
$SequenceChecks["sequence done (no step timed out)"] = [bool]($Log | Select-String -Pattern "Test mark: end" -Quiet)
$Errors = @($Log | Select-String -Pattern "LogHOLUI: (Error|Warning)|LogHOLMatch: (Error|Warning)|LogHOLTest: Error" | ForEach-Object { $_.Line })
$SequenceChecks["no interface, match or sequence error ($($Errors.Count))"] = $Errors.Count -eq 0
$AllOk = (Report "sequence" $SequenceChecks) -and $AllOk
$Errors | Select-Object -First 5 | ForEach-Object { Write-Host "  $_" }

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
        }
        $AllOk = (Report "menu $Culture" $Checks) -and $AllOk
        if (Get-Command python -ErrorAction SilentlyContinue) {
            python (Join-Path $PSScriptRoot "make_sheet.py") $Shots "qc_$Culture" $MenuScreens | Out-Null
        }
    }
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
    $AllOk = (Report "move" $Checks) -and $AllOk
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
    $Checks["HUD while gathering: F $($GatherBerries + 1), W $GatherTrees, idle $GatherBefore ($([regex]::Match([string]$HudWorking, 'gatherers .*$').Value))"] =
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
    $Checks["HUD: idle $WantIdle, F 0, W $GatherTrees ($([regex]::Match([string]$HudIdle, 'gatherers .*$').Value))"] =
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
    $Checks["A rises with its work ($([regex]::Match([string]$SiteReport, 'House of slot 0 \(own\) under construction \d+%').Value))"] = [bool]($SiteReport -match "House of slot 0 \(own\) under construction (\d+)% at \($($BuildA -join ', ')\)") -and
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
    $Checks["D cancelled after its first work: half refunded ($CancelledD)"] = [bool]($CancelledD -match "at \($($BuildD -join ', ')\), refund F 0 W 25 S 0 G 0 \(F \d+ W 95 ")
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
    $Checks["the Peasant on its gathering's round trip ($([regex]::Match([string]$QueueHud, 'EHOLUnitActivity::\w+( EHOLResource::\w+ \d+/\d+)?').Value))"] =
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
    $Checks["Peasants without wood: the age panels, I open (house, lumber camp, mining camp on A, Z, E, barracks on Q, greyed), II to IV locked, none chosen; orders S, V ($Grid)"] = (@($Hud | Select-String -Pattern "HUD report: ").Count -ge 1) -and
        [bool](($Hud | Select-String -Pattern "HUD report: " | Select-Object -First 1).Line -match "grid ages I=open \[A=House greyed Z=LumberCamp greyed E=MiningCamp greyed Q=Barracks greyed\] II=locked \[\] III=locked \[\] IV=locked \[\] \(chosen none\), orders S=Stop V=Hold, queue ")
    # The user's trial of J6a.5: a cell writes the name alone, its cost is the tooltip's.
    $Cells = ($Hud | Select-String -Pattern "HUD cells: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["the cells write the name alone, no cost ($Cells)"] = [bool]($Cells -match "^S: Arr.+t; V: Tenir; I A: Maison; I Z: Camp de b.+cherons; I E: Camp de mineurs; I Q: Caserne$") -and -not ($Cells -match "\d")
    $Tooltips = ($Hud | Select-String -Pattern "HUD tooltips: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["tooltips: name, cost missing, effect ($Tooltips)"] = [bool]($Tooltips -match "^S: Arr.+t \| Annule les ordres et le combat; V: Tenir la position \| N'attaque .+; I A: Palier I \S+ Fondation; I A: Maison \| W 50 missing \| \+10 population; I Z: Camp de b.+cherons \| W 100 missing \| D.+p.+t : bois; I E: Camp de mineurs \| W 100 missing \| D.+p.+t : pierre, or; I Q: Caserne \| W 150 missing \| Produit : Lancier, Homme d'armes, Archer; II Z: Palier II \S+ Essor \| Palier non atteint; III E: Palier III \S+ Puissance \| Palier non atteint; IV R: Palier IV \S+ L.+gende \| Palier non atteint$")
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
    $Checks["the site's block: progress, builders, Annuler on V ($([regex]::Match([string]$Block, 'selection House[^,]*, under construction \d+%, builders \d+').Value); $([regex]::Match([string]$Block, 'V=Cancel \w+').Value))"] =
        [bool]($Block -match "selection House \(own\), under construction (\d+)%, builders (\d+), grid V=Cancel enabled") -and ([int]$Matches[1] -ge 1) -and ([int]$Matches[1] -le 60) -and ([int]$Matches[2] -ge 1)
    $CancelledJ = & $Value $HudSite "(Construction: slot 0 cancelled House .*)$"
    $Checks["Annuler clicked: half refunded ($CancelledJ)"] = [bool]($CancelledJ -match "at \($($BuildJ -join ', ')\), refund F 0 W 25 S 0 G 0 ")
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
    $SentFar = & $Value $Far "(camp LumberCamp at \($($CampFar -join ', ')\) finished: .*)$"
    $IdleFar = @($Far | Select-String -Pattern "Build: a builder of slot 0 stops \(no site left\), idle").Count
    $Checks["a camp 30 m from any wood: no source within 25 m, its two builders idle ($SentFar; $IdleFar idle)"] =
        [bool]($SentFar -match "finished: no accepted source within 25 m, 2 builders go on as after a house$") -and ($IdleFar -eq 2) -and
        -not ($Far | Select-String -Pattern "goes on to the nearby site" -Quiet)
    $Missing = @("wood", "stone", "idle" | Where-Object { -not (Test-Path (Join-Path $Shots "build_camp_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build camp" $Checks) -and $AllOk

    # J6b.2 (D314): paths cut by a footprint. The footprint's line; the watch of the bodies over the seconds after it.
    $Cut = @(Get-Section "build cut" | ForEach-Object { $_ })
    $CutMove = @(Get-Section "build cut move" | ForEach-Object { $_ })
    $Checks = [ordered]@{}
    $ReadCut = { param($Lines, $At) $Line = $Lines | Select-String -Pattern "Buildings \(D310\): footprint House at \($($At -join ', ')\): (\d+) walkers given a new path, (\d+) reused gather paths dropped(.*)\((.*)\)$" | Select-Object -First 1
        if ($Line) { $G = $Line.Matches[0].Groups; [pscustomobject]@{ Walkers = [int]$G[1].Value; Dropped = [int]$G[2].Value; Text = ($Line.Matches[0].Value -replace '^Buildings \(D310\): ', ''); Rebuilt = ($G[4].Value -match '^tiles rebuilt') } } }
    $ReadWatch = { param($Lines) $W = $Lines | Select-String -Pattern "Footprint watch: (\d+) steps, .*; bodies in a footprint: (\d+) unit-steps, .*; put back out: (\d+), new paths: (\d+)$" | Select-Object -Last 1
        if ($W) { [pscustomobject]@{ Steps = [int]$W.Matches[0].Groups[1].Value; Inside = [int]$W.Matches[0].Groups[2].Value; Out = [int]$W.Matches[0].Groups[3].Value; Repaths = [int]$W.Matches[0].Groups[4].Value } } }
    $K = & $ReadCut $Cut $CutK
    $Checks["house K across the round trips: its kept paths dropped, its walkers given a new path once its tiles were rebuilt ($(if ($K) { $K.Text }))"] =
        ($null -ne $K) -and ($K.Walkers -ge 1) -and ($K.Dropped -ge 1) -and $K.Rebuilt
    $WatchK = & $ReadWatch $Cut
    $PushedK = @($Cut | Select-String -Pattern "Keep-out: .*$" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 2)
    # Same rule as house N below (J6b.3): a walker brushing K's wall is put back by the guard (a few cm a step, D310), 10 in
    # a row give it one new path; none in the footprint, a few put back, a new path at most.
    $Checks["none in it over 12 s after it, a few put back, a new path at most ($(if ($WatchK) { "$($WatchK.Steps) steps, $($WatchK.Inside) unit-steps in it, $($WatchK.Out) put back, $($WatchK.Repaths) new paths for it" }) $($PushedK -join '; '))"] =
        ($null -ne $WatchK) -and ($WatchK.Steps -ge 200) -and ($WatchK.Inside -eq 0) -and ($WatchK.Out -le 20) -and ($WatchK.Repaths -le 1)
    $KLine = -1; for ($i = 0; $i -lt $Cut.Count; $i++) { if ($Cut[$i] -match "Buildings \(D310\): footprint House at \($($CutK -join ', ')\)") { $KLine = $i; break } }
    $UnloadsK = if ($KLine -ge 0) { @($Cut[$KLine..($Cut.Count - 1)] | Select-String -Pattern "Gather: slot 0 unloaded \d+ \w+ \(stock [^)]*\) at MainCentre ").Count } else { 0 }
    $Checks["the round trips go on around it ($UnloadsK unloads at the centre after it)"] = $UnloadsK -ge 3
    $N = & $ReadCut $CutMove $CutN
    $Checks["house N across a move order: its walkers given a new path ($(if ($N) { $N.Text }))"] = ($null -ne $N) -and ($N.Walkers -ge 6) -and $N.Rebuilt
    $WatchN = & $ReadWatch $CutMove
    $PushedN = @($CutMove | Select-String -Pattern "Keep-out: .*$" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 2)
    # D324: round N's body (5.7 m; its 8 m tiles before D324), the group of six squeezes its inner Peasant against the west wall, which the
    # footprints' guard keeps out (a few cm a step, D310): a slide along the wall, not a walk into the building. A slide of 10
    # steps in a row (half a second) gives that walker one new path from where it stands (RepathBuildingPushes, D310), as
    # #48 at the full quick_check of J6b.3: one at most, and no body in the footprint after the guard.
    # D344: with 4 m tiles the wall it slides along is 5.7 m (3.7 m before): 25 to 40 put back measured, limit 50 (20 before).
    $Checks["no walk into it: none in it, a few slides along its wall, a new path at most ($(if ($WatchN) { "$($WatchN.Steps) steps, $($WatchN.Inside) unit-steps in it, $($WatchN.Out) put back, $($WatchN.Repaths) new paths for it" }) $($PushedN -join '; '))"] =
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
        $Pushed = @($GapCase.Lines | Select-String -Pattern "Keep-out: .*$" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 2)
        $Checks["$($GapCase.Name): $($R.Walkers.Count) walker(s) seen in the way ($($Between.Count) samples: $(($Between | Select-Object -First 4) -join ', '))"] = ($R.Walkers.Count -eq $GapCase.Count) -and ($Between.Count -ge $GapCase.Count)
        $Checks["$($GapCase.Name): on the way's middle line, within 60 cm (off: $($Off -join ', '))"] = $Off.Count -eq 0
        if ($GapCase.Count -ge 2) { $Checks["two: one after the other ($Abreast samples with both in the way less than 1 m apart along it)"] = $Abreast -eq 0 }
        $Checks["$($GapCase.Name): none put back by a wall ($(if ($Watch) { "$($Watch.Steps) steps, $($Watch.Out) put back, $($Watch.Repaths) new paths" }) $($Pushed -join '; '))"] =
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
    $Checks["the Peasants' grid: Q = Caserne ($PeasantCells)"] = [bool]($PeasantCells -match "; I Q: Caserne$") -and
        [bool](($Barracks | Select-String -Pattern "HUD report: " | Select-Object -First 1).Line -match "Q=Barracks enabled")
    $At = [regex]::Escape("Barracks at ($($BarracksAt -join ', '))")
    $Checks["the keys A, Q: the ghost, placed, built by the three, finished"] = [bool]($Barracks | Select-String -Pattern "Placement: Barracks \(cost F 0 W 150 S 0 G 0\)" -Quiet) -and
        [bool]($Barracks | Select-String -Pattern "Build order of slot 0: 3 units to $At$" -Quiet) -and [bool]($Barracks | Select-String -Pattern "Construction: slot 0 finished $At" -Quiet)
    $BarracksCells = @($Barracks | Select-String -Pattern "HUD cells: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["the barracks selected: A Lancier, Z Homme d'armes, E Archer ($(@($BarracksCells | Select-Object -Last 1)))"] = ($BarracksCells.Count -ge 2) -and
        [bool]($BarracksCells[-1] -match "^A: Lancier; Z: Homme d'armes; E: Archer$")
    # The ground point under the cursor, a few cm off the camera's focus (as the group's rally point of J6a.5).
    $RallyAt = if ([string](& $Value $Barracks "(Rally: slot 0 set the rally point of Barracks at .*)$") -match "at \((-?\d+), (-?\d+)\)") { @([int]$Matches[1], [int]$Matches[2]) } else { @() }
    $Checks["its rally point by a right click ($($RallyAt -join ', '))"] = ($RallyAt.Count -eq 2) -and ([Math]::Abs($RallyAt[0] - $BarracksRally[0]) -le 50) -and
        ([Math]::Abs($RallyAt[1] - $BarracksRally[1]) -le 50)
    $Born = @($Barracks | Select-String -Pattern "Production: (Spearman|ManAtArms|Archer) of slot 0 done" | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["a Lancier, a Homme d'armes and an Archer born ($($Born -join ', '))"] = ($Born -join ',') -eq "Spearman,ManAtArms,Archer"
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
        $Checks["plain/queued block: form-up $($G[1].Value) steps (2 at most), march $($G[2].Value) steps ($Expected at most), $($G[6].Value) left, speed $($G[5].Value) cm/s (523 + 10 % at most), held $($G[3].Value), gap $($G[4].Value) cm"] =
            ([int]$G[1].Value -le 2) -and ([int]$G[2].Value -le $Expected) -and ([int]$G[6].Value -eq 0) -and ([int]$G[5].Value -le 523 * 1.1)
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
        } elseif ($Current -and $Line -match "#(\d+) (\w+) slot (\d+) hp (-?\d+)/(\d+) target (\d+) attacking (\d) hits (\d+) damage (\d+) kills (\d+) first (-?\d+) attackers (\d+)(?: at (-?\d+) (-?\d+))?") {
            $Current.Units[[int]$Matches[1]] = @{ Type = $Matches[2]; Slot = [int]$Matches[3]; Hp = [int]$Matches[4]; Target = [int]$Matches[6]; Hits = [int]$Matches[8]; Kills = [int]$Matches[10]; First = [int64]$Matches[11]; X = $(if ($Matches[13]) { [double]$Matches[13] } else { 0.0 }) }
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
        $Checks["10 hits of 8 per Spearman ($MaaHits hits, $MaaKills kills)"] = $MaaHits -eq 30 -and $MaaKills -eq 3
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
        elseif ($Line -match "#(\d+) \w+ slot \d+ hp (-?\d+)/(\d+) ") { if ($HpReports.Count) { $HpReports[-1][$Matches[1]] = @([Math]::Max([int]$Matches[2], 0), [int]$Matches[3]) }; if ([int]$Matches[2] -lt [int]$Matches[3]) { $Wounded[$Matches[1]] = $true } }
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
    # ground, its fall ended; ~40 s later, the bodies gone at 31 s (held 30 s, sunk in 1 s), none older left.
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
        $Kept -and $Kept.Corpses -ge 1 -and $Kept.Oldest -ge 4.5 -and $Kept.FallEnded -ge 1
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
    $StepMs = 50; $ArrowSpeed = 2500.0
    $Releases = @{}; $BadFlight = 0
    foreach ($Line in $Shot) {
        if ($Line -match "Combat: step (\d+) #(\d+) Archer releases an arrow at #(\d+) \w+: \d+ damage, (\d+) cm, (\d+) ms, lands step (\d+)") {
            $Release = [int64]$Matches[1]; $Cm = [double]$Matches[4]; $Ms = [int]$Matches[5]; $Lands = [int64]$Matches[6]
            $Expected = [Math]::Max(1, [Math]::Ceiling($Ms / $StepMs))
            if ([Math]::Abs($Ms - $Cm * 1000.0 / $ArrowSpeed) -gt 1.0 -or ($Lands - $Release) -ne $Expected) { $BadFlight++ }
            $Releases["$($Matches[2]) $Release"] = $Lands
        }
    }
    $Landed = 0; $Lost = 0; $BadLanding = 0; $Delays = @()
    foreach ($Line in $Shot) {
        if ($Line -match "Combat: step (\d+) arrow of #(\d+) Archer lands on #\d+ \w+ for \d+, -?\d+ HP left \(released step (\d+), (\d+) steps\)") {
            $Landed++; $Delays += [int]$Matches[4]
            if (-not $Releases.ContainsKey("$($Matches[2]) $($Matches[3])") -or $Releases["$($Matches[2]) $($Matches[3])"] -ne [int64]$Matches[1]) { $BadLanding++ }
        }
        elseif ($Line -match "Combat: step \d+ arrow of #\d+ Archer at #\d+ lost \(target dead\)") { $Lost++ }
    }
    $Kills = @($Shot | Select-String -Pattern "Combat: step \d+ #\d+ \w+ of slot 1 killed by #\d+ Archer of slot 0 \(\d+ hits struck, by arrow\)")
    $Falls = @(Get-Section "combat arrows" | Where-Object { $_ -match "Anim report falls: slot 1, (\d+) falling" } | ForEach-Object { [int]$Matches[1] })
    $Delay = if ($Delays.Count) { "$(($Delays | Measure-Object -Minimum).Minimum)-$(($Delays | Measure-Object -Maximum).Maximum) steps" } else { "none" }
    $Checks["arrows: $($Releases.Count) released, $Landed landed, $Lost lost, flights $Delay"] = $Releases.Count -ge 5 -and $Landed -ge 4 -and ($Landed + $Lost) -ge ($Releases.Count - 4) -and ($Landed + $Lost) -le $Releases.Count
    $Checks["each flight = distance at release / 25 m/s in whole steps ($BadFlight off)"] = $Releases.Count -gt 0 -and $BadFlight -eq 0
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

    # The target that flees (J7c.2b): the first arrow released at 15 m lands on the Spearman running away at ~8 m/s
    # (damage at the landing), out of reach by then; none left in flight on the display at the end.
    $Checks = [ordered]@{}
    $Flee = @(Get-Section "combat arrows flee")
    $FirstRelease = $Flee | Where-Object { $_ -match "Combat: step (\d+) #(\d+) Archer releases an arrow at #(\d+) Spearman: \d+ damage, (\d+) cm, (\d+) ms, lands step (\d+)" } | Select-Object -First 1
    if ($FirstRelease -and $FirstRelease -match "Combat: step (\d+) #(\d+) Archer releases an arrow at #(\d+) Spearman: \d+ damage, (\d+) cm, (\d+) ms, lands step (\d+)") {
        $FleeArcher = $Matches[2]; $FleeTarget = $Matches[3]; $FleeCm = [int]$Matches[4]; $FleeMs = [int]$Matches[5]; $FleeStep = $Matches[1]; $FleeLands = $Matches[6]
        $FleeLanding = $Flee | Where-Object { $_ -match "Combat: step $FleeLands arrow of #$FleeArcher Archer lands on #$FleeTarget Spearman for (\d+), -?\d+ HP left \(released step $FleeStep," } | Select-Object -First 1
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
        $Checks["first arrow released at $FleeCm cm, $FleeMs ms ($([int]($FleeCm * 10 / [Math]::Max($FleeMs, 1))) m/s apparent), then the Spearman runs: $($Distances -join ', ') cm off"] = $FleeCm -ge 1400 -and $FleeCm -le 1600
        $Checks["it lands on the fleeing Spearman at its step (damage $(if ($FleeLanding -and $FleeLanding -match 'for (\d+),') { $Matches[1] } else { 'none' }))"] = [bool]$FleeLanding
        $Checks["the Spearman out of reach meanwhile (farthest $(if ($Distances.Count) { ($Distances | Measure-Object -Maximum).Maximum } else { '?' }) cm > 1500)"] = $Distances.Count -ge 1 -and ($Distances | Measure-Object -Maximum).Maximum -gt 1500
    }
    else { $Checks["an arrow released at the Spearman"] = $false }
    $FleeReports = @($Flee | Where-Object { $_ -match "Arrow report: in flight (\d+) launched" } | ForEach-Object { [int]$Matches[1] })
    $Checks["none in flight on the display at the end ($(if ($FleeReports.Count) { $FleeReports[-1] } else { 'no report' }))"] = $FleeReports.Count -ge 1 -and $FleeReports[-1] -eq 0
    $AllOk = (Report "combat arrows flee" $Checks) -and $AllOk
}

if (($Parts -contains "combat") -and ($Parts -contains "camera")) {
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
            } elseif ($null -ne $Units -and $Line -match "#(\d+) (\w+) slot (\d+) hp (-?\d+)/\d+ target (\d+) attacking \d hits (\d+) .* at (-?\d+) (-?\d+)") {
                $Units[[int]$Matches[1]] = @{ Type = $Matches[2]; Slot = [int]$Matches[3]; Hp = [int]$Matches[4]; Target = [int]$Matches[5]; Hits = [int]$Matches[6]; X = [double]$Matches[7]; Y = [double]$Matches[8] }
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
            $Checks["10 hits of 8 to kill it ($MoveHits)"] = $MoveHits -eq 10
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
            # The Peasant out of reach (edge to edge over 50 cm) but within the aggression's 12 m: a unit not holding would go.
            $PeasantIds = @($First.Keys | Where-Object { $First[$_].Type -eq "Peasant" -and $First[$_].Slot -eq 1 -and [Math]::Abs($First[$_].X - $OrdersHoldPeasant[0]) -le 300 -and [Math]::Abs($First[$_].Y - $OrdersHoldPeasant[1]) -le 300 })
            $Gap = if ($PeasantIds.Count -eq 1) { [Math]::Sqrt([Math]::Pow($First[$PeasantIds[0]].X - $First[$Id].X, 2) + [Math]::Pow($First[$PeasantIds[0]].Y - $First[$Id].Y, 2)) - 120 } else { -1 }
            $Checks["the Peasant out of reach, within 12 m ($([int]$Gap) cm edge to edge)"] = $Gap -gt 50 -and $Gap -lt 1200
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
    $Spear = @(if ($AttackReports.Count -ge 1) { $AttackReports[0].Keys | Where-Object { $AttackReports[0][$_].Type -eq "Spearman" -and $AttackReports[0][$_].Slot -eq 1 -and [Math]::Abs($AttackReports[0][$_].X - $CombatSpearmen[0]) -le 300 -and [Math]::Abs($AttackReports[0][$_].Y - $CombatSpearmen[1]) -le 300 } })
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
    $NetSelectedTrees = @{}
    $NetPopups = @{}
    foreach ($NetClient in $NetClients) {
        $ClientTimedOut = -not $NetClient.Process.WaitForExit(120000)
        if ($ClientTimedOut) { $NetClient.Process.Kill() }
        $N = $NetClient.Client
        $ClientLines = if (Test-Path $NetClient.Log) { Get-Content $NetClient.Log } else { @() }
        $Checks["client $N : sequence done"] = (-not $ClientTimedOut) -and [bool]($ClientLines | Select-String -Pattern "Test mark: end" -Quiet)
        $Held = $ClientLines | Select-String -Pattern "Camera above the start of slot (\d+) \(" | Select-Object -First 1 | ForEach-Object { [int]$_.Matches[0].Groups[1].Value }
        $Checks["client $N : holds a slot ($Held)"] = $null -ne $Held
        if ($null -eq $Held) { continue }
        $HeldSlots += $Held
        $Camera = $ClientLines | Select-String -Pattern "Camera: focus \((-?\d+), (-?\d+), (-?\d+)\)" | Select-Object -First 1
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
        $OtherLooks = @(foreach ($Type in "Spearman", "ManAtArms") { [regex]::Match([string]$Looks, "$Type of slot $Other skinned (\w+) colour (\d+) x2") })
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
                $Seen = [regex]::Match([string]$Where, "slot $Slot at \((-?\d+), (-?\d+)\)")
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
            foreach ($M in (Select-String -Path $NetServerLog -Pattern "Combat: step \d+ #\d+ \w+ hits #(\d+) \w+ for \d+, (-?\d+) HP left")) {
                $Hit = $M.Matches[0].Groups
                if (-not $ServerHp.ContainsKey($Hit[1].Value)) { $ServerHp[$Hit[1].Value] = @() }
                $ServerHp[$Hit[1].Value] += [int]$Hit[2].Value
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
        $BarRuns = @(); $WoundedNow = @{}
        foreach ($Line in $FightLines) {
            if ($Line -match "Units where: \d+ units: (.*)$") {
                $WoundedNow = @{}
                foreach ($Entry in ($Matches[1] -split "; ")) { if ($Entry -match "^#(\d+) .*, hp \d+/\d+") { $WoundedNow[$Matches[1]] = $true } }
            } elseif ($Line -match "Health bars: bars (\d+) \(([^)]*)\)") {
                $Items = @(if ($Matches[2]) { $Matches[2] -split ", " })
                $Wrong = @($Items | Where-Object { $_ -notmatch " selected$" -and $_ -match "^#(\d+)" -and -not $WoundedNow.ContainsKey($Matches[1]) })
                $NotSent = @($Items | Where-Object { $_ -match "^#(\d+) (\d+)/(\d+)" -and -not ($Matches[2] -eq $Matches[3] -or (& $IsServerHp $Matches[1] ([int]$Matches[2]))) })
                $BarRuns += @{ Selected = @($Items | Where-Object { $_ -match " selected$" }).Count; Others = $Items.Count - @($Items | Where-Object { $_ -match " selected$" }).Count; Wrong = $Wrong.Count; NotSent = $NotSent.Count }
            }
        }
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
    if ($NetServerProcess -and -not $NetServerProcess.HasExited) { Stop-Process -Id $NetServerProcess.Id -Force }
    # The melee on (J7b.2) on the dedicated server: the two armies stand at their own centres, far apart, no one killed;
    # the fight made for client 1 (J7b.4) kills its 2 Spearmen alone.
    $NetKills = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Combat: step \d+ #\d+ (\w+ of slot \d) killed by" | Where-Object { $_.Line -notmatch "by arrow" } | ForEach-Object { $_.Matches[0].Groups[1].Value } })
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
    $ServerErrors = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "LogHOLMatch: (Error|Warning)|LogHOLNet: (Error|Warning)" })
    $Checks["server: no match or network error ($($ServerErrors.Count))"] = $ServerErrors.Count -eq 0
    $Born = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "Production: Peasant of slot (\d) done" | ForEach-Object { $_.Matches[0].Groups[1].Value } })
    $Checks["server: one Peasant born per slot ($($Born -join ', '))"] = (($Born | Sort-Object) -join ',') -eq "0,1"
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
    $AllOk = (Report "net 2 clients" $Checks) -and $AllOk
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
    $AllOk = (Report "tests $($Results.Count - $Failures.Count)/$($Results.Count)" $Checks) -and $AllOk
}

Write-Host "quick_check : $(if ($AllOk) { 'PASS' } else { 'FAIL' }) in $([Math]::Round($Clock.Elapsed.TotalSeconds)) s (game launch done at $GameSeconds s)"
if (-not $AllOk) { exit 1 }
