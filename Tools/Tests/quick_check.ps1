# Quick check of the project (D267): the automation tests run in their own process WHILE a single game launch goes
# through the menu in French, English and Japanese (language switched live), a drop-down list, then a match at 2 and
# at 8 players with the camera checks of D257 and the selection checks of D264 (own centre, another's, void, Escape).
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
# another one touching it with Shift (still in the mode), the ghost red on them, Escape; refused (nothing taken): on a
# house, on a tree, near the enemy centre, without enough wood; the second foundation selected by a click and cancelled
# (refunded). With "net": each client places a house behind its centre, sees the other's, pays alone, and a house near
# the enemy centre is refused. The two clients wait for each other before quitting (each one's last report done).
# J6a.4 (D312), solo: a Peasant sent by a right click on house A (a ghost until then): its first work blocks the navmesh,
# it rises, finished alone in the table's time (hol.Build.Speed 2), the population cap 10 -> 20, the builder idle; three
# Peasants place house C, then D with Shift: C built by the three (about twice as fast), then D; the first Peasant helps
# on D (a right click on the site), D cancelled after its first work (half refunded), its builders idle. With "net": each
# client's Peasant builds its house; the other's foundation never reaches a client before its first work, then it does.
# The user's trial of J6a.4, solo: the centre's rally point on a ghost house E (the flag on it), every site finished at
# its first blow (hol.Cheat.FastBuild 1): the Peasant produced builds E, goes on to F 9 m away (hol.Build.NextSiteRange,
# 20 m), not to G 39 m away, then stands idle; Shift + place H, Shift + place I, Shift + right click on a tree: H built,
# then I, then it gathers wood.
# J6a.5 (D313), solo: every Peasant selected without wood, the grid shows house, lumber camp and mining camp on A, Z, E,
# greyed, their tooltips; a click on the lumber camp: no ghost, the message "Pas assez de bois" (captured in French,
# English and Japanese); with wood, the key Z starts the lumber camp's ghost; two Peasants place house J by the key A,
# the site selected shows its progress, its builders and "Annuler" (V), clicked: refunded; the population full, the key
# A at the centre: the message "Population maximale". The user's trial of J6a.5: the cells write the name alone (the cost
# in the tooltip only); F1 and F3: no building, the message "Aucun bâtiment militaire", nothing selected; F2: every
# economic building of the player selected (centre, houses, sites; not its ghosts), the camera where it was; F2 three
# times again: the centre, then the houses, the camera on each, the selection kept.
# The "anims" part (J5c.4), in solo: what the presentation shows, read from hol.Units.AnimReport (sequence and prop of
# each Peasant drawn, per state). Two Peasants build a house; three each chop wood, mine stone, mine gold, pick berries;
# three stand idle: each gesture with its tool (axe, pickaxe, nothing for the berries), the hammer, the plain idle. Then
# the four groups of gatherers walk away with their loads (the carrying walk of the load, the load shown), and stand
# there (the loaded idle, the load still shown). Every Peasant drawn, whatever it does, must show what its state calls for.
# J6b.1 (D312), solo: a lumber camp placed by the wood and built by two Peasants: once finished, they gather the nearest
# wood and unload at the camp (a gatherer nearby unloads at the centre until then); the centre's rally point on the camp:
# the Peasant born once it is finished gathers there too; a mining camp by the stone vein: its builders gather the stone;
# a camp 34 m from any wood: its builders stand idle.
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths), with no editor of the project open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1 [-EngineDir <dir>] [-Visible]
#   powershell -ExecutionPolicy Bypass -Command "& .\Tools\Tests\quick_check.ps1 -Parts net,sources,move"   (-File passes a list as one string)
# The game renders off screen, with no window (the user works on the machine meanwhile); -Visible shows it on the
# second screen instead, windowed 1600 x 900, and the two clients side by side under it.
# The engine is the project's (EngineAssociation of the .uproject, found in the registry), unless -EngineDir is given.
# Results: a PASS / FAIL line per part, captures in Saved_Tests\Screenshots\WindowsEditor\HOL (qc_*, cam_*, sel_*),
# contact sheets qc_<culture>.png (Python with Pillow), logs Saved_Tests\Logs\quick_game.log and quick_tests.log.
param(
    [string[]] $Parts = @("tests", "menu", "camera", "production", "move", "sources", "gather", "build", "anims", "net"),
    [string] $EngineDir = "",
    [switch] $Visible
)

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
    $TestsArguments = "`"$UProject`" -ExecCmds=`"Automation RunTests HeroesOfLiberty;SoftQuit`" -saveddirsuffix=Tests -unattended -nullrhi -nosplash -nosound -abslog=`"$TestsLog`""
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
    Remove-Item $NetServerLog, (Join-Path $TestSaved "Logs\quick_net_client*.log") -ErrorAction SilentlyContinue
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
    $NetServerArguments = "`"$UProject`" /Game/Maps/Plain -server -unattended -nosplash -nosound -saveddirsuffix=Tests_NetSrv -MatchConfig=`"$NetConfig`" -abslog=`"$NetServerLog`" -ExecCmds=`"hol.Prod.BuildTime 2$NetSourceCommand`""
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
# centre (4 x 4 tiles) stands on the 3 m tile grid (D311), its middle on the grid point nearest its slot (diagonal slots:
# 1.07 m off on each axis), where the camera starts and the scripted checks aim.
$SlotAngles = @(180, 0, 270, 90, 225, 45, 315, 135)
$Tile = 300
function Get-SlotStart([int] $Slot) {
    $Radians = $SlotAngles[$Slot] * [Math]::PI / 180
    return @(([Math]::Round(15000 * [Math]::Cos($Radians) / $Tile) * $Tile), ([Math]::Round(15000 * [Math]::Sin($Radians) / $Tile) * $Tile))
}
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
# Placement in solo (J6a.3): house A 12 m behind slot 0's centre (its middle on a grid point, 2 x 2 tiles), house B
# touching it, a spot across both.
$BuildA = @(-16200, 0)
$BuildB = @(-16200, 600)
$BuildOver = @(-16200, 300)
# Building in solo (J6a.4): the Peasant sent to A, then three Peasants for house C and house D touching it (Shift), all
# behind slot 0's centre; the navmesh probed across A (14 m, through its middle).
$BuildPeasant = @(-18000, -300)
$BuildThree = @(-19000, 600)
$BuildC = @(-17400, 600)
$BuildD = @(-17400, 1200)
$BuildAcross = @(-16200, -700, -16200, 700)
# The user's trial of J6a.4 (solo): house E behind slot 0's centre, on the wood's side, its rally point on it; F 9 m from
# E, G 39 m from it on the other side; H and I behind F; a tree of the wood's back rows.
$BuildE = @(-16800, -1500)
$BuildF = @(-16800, -3000)
$BuildG = @(-16800, 3000)
$BuildH = @(-17700, -3000)
$BuildI = @(-18300, -3000)
$BuildTree = @(-15770, -2400)
# The construction HUD (J6a.5): house J behind slot 0's centre, away from the others, two Peasants made beside it.
$BuildJ = @(-18600, 1800)
$BuildJSpawn = @(-19500, 1800)
# The rally flags of a group (the user's 3rd trial of J6a.5): a right click on the ground behind slot 0's centre.
$BuildRally = @(-18000, -1500)
# The animations (J5c.4), solo: house K behind slot 0's centre and its wood, its builders made beside it; each group of
# gatherers made a few metres from its source (the wood's edge, the stone and gold veins on either side of slot 0, slot 4's
# berries: slot 0's are used up by the gather part), then sent ~20 m away with its load; three idle Peasants on their own.
# Look: where its close-up at work is taken from (the wood's edge, not under the crowns).
$AnimHouse = @(-18600, -5100)
$AnimBuilders = @(-19400, -5100)
$AnimIdle = @(-9000, 2000)
$AnimGroups = @(
    @{ Name = "wood"; Spawn = @(-13700, -1500); Source = @(-14221, -1900); Target = @(-12500, -3000); Look = @(-14200, -1450) },
    @{ Name = "stone"; Spawn = @(-11087, -5400); Source = @(-11087, -4592); Target = @(-11087, -6600); Look = @(-11087, -4592) },
    @{ Name = "gold"; Spawn = @(-11087, 5400); Source = @(-11087, 4592); Target = @(-11087, 6600); Look = @(-11087, 4592) },
    @{ Name = "berries"; Spawn = @(-11668, -9546); Source = @(-12021, -9193); Target = @(-10200, -8000); Look = @(-12021, -9193) }
)
# The camps (J6b.1): every Peasant of slot 0 sent behind its centre; lumber camp L 4 m off the edge of slot 0's wood (on
# the map's side), a Peasant made beside the wood sent to its tree nearest L, two builders made beyond L; mining camp M
# 4 m off the stone vein between slots 0 and 4 (at (-11087, -4592)), its two builders made beside it; a lumber camp 34 m
# from every wood, behind the centre.
$CampCrowd = @(-18600, 1500)
$CampL = @(-13500, -2100)
$CampLSpawn = @(-12600, -2100)
$CampEarly = @(-13700, -1300)
$CampTree = @(-14230, -1800)
$CampM = @(-12000, -3900)
$CampMSpawn = @(-12000, -3000)
$CampFar = @(-19500, -900)
$CampFarSpawn = @(-19500, -1800)
if ($Parts -contains "camera") {
    $First = $true
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
                    "hol.Prod.Queue", "hol.Prod.Queue", "hol.Prod.Queue", "waitlog (5 born here)", "hol.Prod.BuildTime 0", "wait 7", "hol.Units.Where 0", "hol.Units.Orders 0"
                Step "hol.Camera.Focus -12000 0", "frames 5", "hol.Select.BoxGround -14000 -500 -13000 500", "waitlog Selection: 3 units (own): Peasant x3",
                    "frames 3", "hol.Select.Report", "capture move_box"
                Step "hol.Select.ClickUnit -1 add", "waitlog Selection: 2 units (own): Peasant x2", "hol.Select.ClickUnit -1 add",
                    "waitlog Selection: 3 units (own): Peasant x3", "hol.Select.OrderGround -10500 0", "mark move walk"
                for ($i = 0; $i -lt 44; $i++) { Step "hol.Units.Where 0", "wait 0.25" }
                Step "mark move arrived", "hol.Units.Where 0", "hol.Nav.Units", "hol.Select.Report", "capture move_arrived"
                Key Escape
                Step "waitlog Selection: none", "hol.Select.Report"
            }
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
            Step "mark gather", "hol.Nav.Footprints navmesh", "hol.Nav.Footprints on", "hol.Gather.LogDeposits 1", "hol.Gather.LogPopups 1", "hol.Cheat.Food 1000", "hol.Cheat.Population 165", "hol.Prod.BuildTime 2", "hol.Units.Where 0",
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
            Step "waitidle $($GatherBefore + $GatherBerries + 1)", "frames 2", "hol.Units.Where 0", "hol.Prod.Report", "hol.HUD.Report"
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
            # Nothing selected: the placements of J6a.3 send no builder (J6a.4).
            Key Escape
            Step "waitlog Selection: none"
            # Placement (J6a.3, D311), 120 wood exactly (two houses of 50): the cursor stays in the middle of the screen
            # (hol.Camera.TestCursor), the camera brings the ghost where wanted. House A behind slot 0's centre, B touching
            # it (Shift: still in the mode), then the ghost on both: red; Escape leaves the mode.
            Step "mark build", "hol.Cheat.Wood -100000", "hol.Cheat.Wood 120", "hol.Build.Report",
                "hol.Camera.Focus $($BuildA -join ' ')", "frames 3", "hol.Build.Place House", "waitlog Placement ghost: House at ($($BuildA -join ', ')) valid",
                "frames 3", "hol.Select.Report", "capture build_ghost_green", "hol.Build.Click shift",
                "waitlog Construction: slot 0 placed House foundation at ($($BuildA -join ', '))"
            Step "hol.Camera.Focus $($BuildB -join ' ')", "frames 3", "hol.Build.Click shift", "waitlog Construction: slot 0 placed House foundation at ($($BuildB -join ', '))",
                "hol.Camera.Focus $($BuildOver -join ' ')", "waitlog Placement ghost: House at ($($BuildOver -join ', ')) refused", "frames 3", "hol.Select.Report",
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
            # Building (J6a.4, D312), twice as fast (hol.Build.Speed 2: house 10 s for one builder). The population cap of the
            # centre alone (the gather part's cheat taken back), 100 wood more (170). The navmesh across house A, a ghost:
            # straight. A Peasant made beside it, selected, a right click on A: it walks there, works, A blocks the navmesh
            # (a detour now), rises (captured), is finished alone in 200 steps, the cap 10 -> 20, the Peasant idle.
            $Uncheat = if ($Parts -contains "gather") { "hol.Cheat.Population -165" } else { "hol.Build.Report" }
            Step "mark build site", "hol.Build.Speed 2", $Uncheat, "hol.Cheat.Wood 100", "wait 0.3", "hol.Prod.Report", "hol.HUD.Report",
                "hol.Nav.Path $($BuildAcross -join ' ')", "hol.Units.Spawn 1 $($BuildPeasant -join ' ') 0 Peasant 10", "wait 0.5",
                "hol.Camera.Focus -17000 0", "frames 3", "hol.Select.ClickUnit -1", "waitlog Selection: Peasant #", "hol.Select.OrderGround $($BuildA -join ' ')",
                "waitlog Build order of slot 0: 1 units to House at ($($BuildA -join ', '))", "waitlog Construction: slot 0 first work on House at ($($BuildA -join ', '))",
                "wait 2", "hol.Nav.Path $($BuildAcross -join ' ')", "wait 2", "hol.Build.Report", "frames 2", "capture build_site",
                "waitlog Construction: slot 0 finished House at ($($BuildA -join ', '))", "waitlog Build: a builder of slot 0 stops (no site left), idle", "wait 0.3",
                "hol.Prod.Report", "hol.HUD.Report", "hol.Build.Report"
            # Three Peasants made behind, selected by a box; house C placed by Shift + click, D touching it by another one
            # (queued in that order), Escape. The three build C (about twice as fast as one alone), then D.
            Step "mark build three", "hol.Units.Spawn 3 $($BuildThree -join ' ') 0 Peasant 150", "wait 0.5", "hol.Camera.Focus $($BuildThree[0] + 600) $($BuildThree[1])", "frames 3",
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
            # E, goes on to F (9 m), not to G (39 m), then stands idle.
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
                "hol.HUD.Report", "capture build_hud_greyed", "hol.HUD.Click GridCell01", "waitlog HUD message: NotEnoughWood", "frames 3", "hol.HUD.Report",
                "capture build_hud_fr"
            foreach ($Culture in "en", "ja") {
                Step "hol.UI.Language $Culture", "frames 5", "hol.HUD.Click GridCell01", "waitlog HUD message: NotEnoughWood", "frames 3", "capture build_hud_$Culture"
            }
            Step "hol.UI.Language fr", "frames 5"
            # With wood, the key Z: the lumber camp's ghost; Escape.
            Step "mark build hud ghost", "hol.Cheat.Wood 500", "wait 0.3", "hol.HUD.Report"
            Key Z
            Step "waitlog Placement: LumberCamp (cost", "frames 3", "hol.HUD.Report", "capture build_hud_ghost"
            Key Escape
            Step "waitlog Placement: ended"
            # Two Peasants made behind, selected by a box; the key A, a click: house J placed, they build it; the site selected:
            # its block (progress, builders, "Annuler" on V), the click on it refunds half.
            Step "mark build hud site", "hol.Units.Spawn 2 $($BuildJSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($BuildJSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($BuildJSpawn[0] - 400) $($BuildJSpawn[1] - 400) $($BuildJSpawn[0] + 400) $($BuildJSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($BuildJ -join ' ')", "frames 3"
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
            Step "mark build camp", "waitlog Selection: none", "hol.Units.Move $($CampCrowd -join ' ') 0", "hol.Cheat.Wood 1000", "hol.Cheat.Food 500",
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
            Step "mark build camp mining", "waitlog Selection: none", "hol.Units.Spawn 2 $($CampMSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($CampMSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CampMSpawn[0] - 400) $($CampMSpawn[1] - 400) $($CampMSpawn[0] + 400) $($CampMSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($CampM -join ' ')", "frames 3", "hol.Build.Place MiningCamp", "waitlog Placement ghost: MiningCamp at ($($CampM -join ', ')) valid",
                "hol.Build.Click", "waitlog Build order of slot 0: 2 units to MiningCamp at ($($CampM -join ', '))", "waitlog Construction: slot 0 finished MiningCamp at ($($CampM -join ', '))",
                "waitlog Construction: slot 0 camp MiningCamp at ($($CampM -join ', ')) finished: ", "wait 9", "hol.Units.Where 0",
                "hol.Camera.Focus $($CampM[0] + 400) $($CampM[1] - 300)", "frames 5", "capture build_camp_stone"
            # A lumber camp far from every wood (34 m): no accepted source within 25 m, its two builders stand idle.
            Key Escape
            Step "mark build camp far", "waitlog Selection: none", "hol.Units.Spawn 2 $($CampFarSpawn -join ' ') 0 Peasant 100", "wait 0.5", "hol.Camera.Focus $($CampFarSpawn -join ' ')", "frames 3",
                "hol.Select.BoxGround $($CampFarSpawn[0] - 400) $($CampFarSpawn[1] - 400) $($CampFarSpawn[0] + 400) $($CampFarSpawn[1] + 400)", "waitlog Selection: 2 units (own): Peasant x2",
                "hol.Camera.Focus $($CampFar -join ' ')", "frames 3", "hol.Build.Place LumberCamp", "waitlog Placement ghost: LumberCamp at ($($CampFar -join ', ')) valid",
                "hol.Build.Click", "waitlog Build order of slot 0: 2 units to LumberCamp at ($($CampFar -join ', '))", "waitlog Construction: slot 0 finished LumberCamp at ($($CampFar -join ', '))",
                "waitlog Construction: slot 0 camp LumberCamp at ($($CampFar -join ', ')) finished: no accepted source", "wait 1", "hol.Units.Where 0", "hol.HUD.Report",
                "hol.Camera.Focus $($CampFar[0]) $($CampFar[1] - 300)", "frames 5", "capture build_camp_idle", "hol.Build.Speed 1", "hol.Gather.SpeedScale 1", "hol.Build.Report"
        }
        if (($Parts -contains "anims") -and $Case.Name -eq "2p") {
            # The animations (J5c.4), at the table's rates of construction (two builders: ~10 s a house) and gathering.
            # Two Peasants made beside K, selected by a box, K placed by a click: they walk there and hammer.
            Step "mark anims", "hol.Cheat.FastBuild 0", "hol.Build.Speed 1", "hol.Cheat.GatherSpeed 1", "hol.Cheat.Wood 100",
                "hol.Units.Spawn 2 $($AnimBuilders -join ' ') 0 Peasant 100", "hol.Units.Spawn 3 $($AnimIdle -join ' ') 0 Peasant 150"
            foreach ($Group in $AnimGroups) { Step "hol.Units.Spawn 3 $($Group.Spawn -join ' ') 0 Peasant 150" }
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
                "hol.Camera.Focus $($AnimIdle -join ' ')", "frames 3", "capture anims_work_idle", "hol.Camera.Zoom -14", "wait 3"
            # Each group (a few wood, stone, gold or food each by now: 0.7 a second) selected around its source, sent ~20 m
            # away: it keeps its load (D296). Reported on the way, then once they stand there.
            foreach ($Group in $AnimGroups) {
                Step "hol.Camera.Focus $($Group.Source -join ' ')", "frames 3",
                    "hol.Select.BoxGround $($Group.Source[0] - 700) $($Group.Source[1] - 700) $($Group.Source[0] + 700) $($Group.Source[1] + 700)",
                    "waitlog Selection: ", "hol.Select.OrderGround $($Group.Target -join ' ')", "waitlog Order: "
            }
            Step "wait 0.5", "mark anims walking", "hol.Units.AnimReport", "hol.Units.Where 0", "hol.Camera.Zoom 14"
            $Last = $AnimGroups[-1]
            # The last group sent, a few metres on its way.
            Step "hol.Camera.Focus $($Last.Source[0] + ($Last.Target[0] - $Last.Source[0]) / 5) $($Last.Source[1] + ($Last.Target[1] - $Last.Source[1]) / 5)", "frames 2", "capture anims_walk_$($Last.Name)",
                "hol.Camera.Zoom -14", "wait 4", "mark anims standing", "hol.Units.AnimReport", "hol.Units.Where 0", "hol.Camera.Zoom 14"
            foreach ($Group in $AnimGroups) { Step "hol.Camera.Focus $($Group.Target -join ' ')", "frames 3", "capture anims_stand_$($Group.Name)" }
            Step "hol.Camera.Zoom -14", "hol.Build.Report"
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
$Display = if ($Visible) { "-windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100" } else { "-RenderOffScreen -ResX=1600 -ResY=900 -nosound" }
$GameArguments = "`"$UProject`" -game $Display -saveddirsuffix=Tests -nosplash -culture=fr -abslog=`"$GameLog`" -ExecCmds=`"hol.Test.Run $SequenceFile`""
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
        # Placement (J6a.3, D311): its Peasant sent aside first (no unload meanwhile), then selected, a house 18 m behind its
        # centre, which its Peasant builds (J6a.4); once both placed (each waits for the other), its own foundation seen here
        # and not the other's (a ghost for its owner alone until its first work), its own wood 50 less, the other's stock
        # never received; then the other's site arrives at its first work (captured); a house 24 m behind the enemy centre,
        # refused by the server (anti-rush radius, D22).
        $Other = 3 - $Client
        if ($Parts -contains "build") {
            $NetSteps += "mark build", "hol.Units.OrderNear -1000 2500", "wait 1", "hol.Prod.Report", "hol.Select.Own", "waitlog Select own: 1 units", "hol.Build.PlaceNear House -1800 0",
                "signal $(Join-Path $TestSaved "quick_net_placed$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_placed$Other.flag")", "wait 2",
                "hol.Build.Report", "hol.Prod.Report", "hol.Camera.FocusSlot 0", "frames 5", "capture qcnet_${Client}_build0", "hol.Camera.FocusSlot 1", "frames 5",
                "capture qcnet_${Client}_build1", "waitlog Building arrived: House of slot", "wait 1", "hol.Build.Report", "hol.Camera.FocusSlot 0", "frames 5",
                "capture qcnet_${Client}_site0", "hol.Camera.FocusSlot 1", "frames 5", "capture qcnet_${Client}_site1", "hol.Build.PlaceNear House -2400 0 enemy", "wait 2", "hol.Build.Report", "hol.Prod.Report"
        }
        # Each client waits for the other one's last report before quitting: a player gone leaves the other's reports (D282's
        # "others []", the flake of 2026-10-16).
        $NetSteps += "signal $(Join-Path $TestSaved "quick_net_done$Client.flag")", "waitfile $(Join-Path $TestSaved "quick_net_done$Other.flag")", "mark end", "quit"
        $ClientSequence = Join-Path $TestSaved "quick_net_client$Client.seq"
        $NetSteps | Set-Content -Encoding utf8 $ClientSequence
        $ClientLog = Join-Path $TestSaved "Logs\quick_net_client$Client.log"
        $ClientDisplay = if ($Visible) { "-windowed -ResX=940 -ResY=530 -WinX=$(3940 + ($Client - 1) * 960) -WinY=1020" } else { "-RenderOffScreen -ResX=960 -ResY=540 -nosound" }
        $ClientArguments = "`"$UProject`" 127.0.0.1 -game $ClientDisplay -saveddirsuffix=Tests_Net$Client -nosplash -culture=fr -abslog=`"$ClientLog`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Prod.BuildTime 2,hol.Gather.LogPopups 1,hol.Test.Run $ClientSequence`""
        $NetClients += @{ Client = $Client; Log = $ClientLog; Shots = $ClientShots;
            Process = (Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $ClientArguments -PassThru) }
    }
}

$GameTimedOut = -not $GameProcess.WaitForExit(300000)
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
    stone = "hud=HOLGameScreen" }
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
        $Checks["main centres on the 3 m grid, along its lines ($($Centres.Count) of $OpenSlots; $(($Centres | ForEach-Object { "$($_.Groups[1].Value): ($($_.Groups[2].Value), $($_.Groups[3].Value)) yaw $($_.Groups[4].Value)" }) -join '; '))"] =
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
        $Checks["around the centre made at run time ($($Paths[0]))"] = (& $Ratio $Paths[0]) -gt 1.15
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
        $Selected = @($Log | Select-String -Pattern "Selection report: selected (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
        $Wanted = "3 units (own): Peasant x3"
        $Checks["box around them selects the three ($($Solo | Select-String -Pattern 'Selection report: selected (.*)$' | ForEach-Object { $_.Matches[0].Groups[1].Value }))"] =
            [bool]($Solo | Select-String -Pattern "Selection report: selected $([regex]::Escape($Wanted))" -Quiet) -and
            [bool]($Solo | Select-String -Pattern "Selection markers: rings 3" -Quiet)
        $Checks["Shift + click takes one out, then back in"] = [bool]($Solo | Select-String -Pattern "Selection: 2 units \(own\): Peasant x2" -Quiet)
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
        $Checks["Escape clears the selection"] = ($Selected.Count -ge 1) -and ($Selected[-1] -eq "none")
        $Checks["captures"] = (Test-Path (Join-Path $Shots "move_box.png")) -and (Test-Path (Join-Path $Shots "move_arrived.png"))
    }
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
    $Checks["ghost red on the two houses, its click places nothing, shown"] = [bool]($Build | Select-String -Pattern "Placement ghost: House at \($($BuildOver -join ', ')\) refused \(on a building\)" -Quiet) -and
        [bool]($Build | Select-String -Pattern "Placement: not here \(on a building\)" -Quiet) -and
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
    $Missing = @("ghost_green", "ghost_red", "foundations" | Where-Object { -not (Test-Path (Join-Path $Shots "build_$_.png")) })
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
        ((& $Ratio $Paths[0]) -ge 0) -and ((& $Ratio $Paths[0]) -le 1.02) -and ((& $Ratio $Paths[1]) -ge 1.15)
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
    $Checks["E finished, its builder goes on to F 9 m away ($($Nearby -join '; '))"] = ($Nearby.Count -eq 1) -and
        [bool]($Nearby[0] -match "^House at \($($BuildF -join ', ')\), 900 cm from the one finished$")
    $Sites = @($Rally | Select-String -Pattern "Construction: slot 0 (finished|first work on) House at \((-?\d+), (-?\d+)\)" | ForEach-Object { "$($_.Matches[0].Groups[1].Value) ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" })
    $Checks["E then F, each at its first blow, G (39 m) never started ($($Sites -join '; '))"] = ($Sites -join '|') -eq
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
    $Checks = [ordered]@{}
    $Grid = & $Value $Hud "HUD report: .*, grid (.*), queue "
    $Checks["Peasants without wood: house, lumber camp, mining camp on A, Z, E, greyed ($Grid)"] = (@($Hud | Select-String -Pattern "HUD report: ").Count -ge 1) -and
        [bool](($Hud | Select-String -Pattern "HUD report: " | Select-Object -First 1).Line -match "grid A=House greyed Z=LumberCamp greyed E=MiningCamp greyed, queue ")
    # The user's trial of J6a.5: a cell writes the name alone, its cost is the tooltip's.
    $Cells = ($Hud | Select-String -Pattern "HUD cells: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["the cells write the name alone, no cost ($Cells)"] = [bool]($Cells -match "^A: Maison; Z: Camp de b.+cherons; E: Camp de mineurs$") -and -not ($Cells -match "\d")
    $Tooltips = ($Hud | Select-String -Pattern "HUD tooltips: (.*)$" | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value })
    $Checks["tooltips: name, cost missing, effect ($Tooltips)"] = [bool]($Tooltips -match "^A: Maison \| W 50 missing \| \+10 population; Z: Camp de b.+cherons \| W 100 missing \| D.+p.+t : bois; E: Camp de mineurs \| W 100 missing \| D.+p.+t : pierre, or$")
    $Checks["a click on a greyed cell: no ghost, the message (not enough wood)"] = [bool]($Hud | Select-String -Pattern "Command feedback: not enough Wood" -Quiet) -and
        [bool]($Hud | Select-String -Pattern 'HUD message: NotEnoughWood "Pas assez de bois"' -Quiet) -and -not ($Hud | Select-String -Pattern "Placement: LumberCamp" -Quiet) -and
        [bool](($Hud | Select-String -Pattern "HUD report: " | Select-Object -Last 1).Line -match ", message NotEnoughWood")
    $Checks["the message in the three languages (HUD message lines: $(@($Hud | Select-String -Pattern 'HUD message: NotEnoughWood').Count))"] = @($Hud | Select-String -Pattern "HUD message: NotEnoughWood").Count -eq 3
    $Checks["with wood, the key Z: the lumber camp's ghost, the cells ready"] = [bool]($HudGhost | Select-String -Pattern "Placement: LumberCamp \(cost F 0 W 100 S 0 G 0\)" -Quiet) -and
        [bool]($HudGhost | Select-String -Pattern "grid A=House enabled Z=LumberCamp enabled E=MiningCamp enabled" -Quiet) -and [bool]($HudGhost | Select-String -Pattern "Placement: ended" -Quiet)
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
        [bool](($HudF1 | Select-String -Pattern "HUD report: " | Select-Object -Last 1).Line -match "selection none, grid hidden, queue hidden, message NoMilitaryBuilding")
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
    $Checks["a camp 34 m from any wood: no source within 25 m, its two builders idle ($SentFar; $IdleFar idle)"] =
        [bool]($SentFar -match "finished: no accepted source within 25 m, 2 builders go on as after a house$") -and ($IdleFar -eq 2) -and
        -not ($Far | Select-String -Pattern "goes on to the nearby site" -Quiet)
    $Missing = @("wood", "stone", "idle" | Where-Object { -not (Test-Path (Join-Path $Shots "build_camp_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "build camp" $Checks) -and $AllOk
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
    $Missing = @("work_wood", "work_stone", "work_gold", "work_berries", "work_build", "work_idle", "walk_berries", "stand_wood", "stand_stone", "stand_gold", "stand_berries" |
        Where-Object { -not (Test-Path (Join-Path $Shots "anims_$_.png")) })
    $Checks["captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
    $AllOk = (Report "anims" $Checks) -and $AllOk
    foreach ($Moment in $Reports.Keys) { Write-Host "  $Moment`: $($Reports[$Moment].Text)" }
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
        $Selection = @($ClientLines | Select-String -Pattern "Selection report: (.*)$" | ForEach-Object { $_.Matches[0].Groups[1].Value })
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
            $OwnHouse = & $Behind $Held 1800
            $OtherHouse = & $Behind $Other 1800
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
        }
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
    $Checks["two clients in slots 0 and 1 ($($HeldSlots -join ', '))"] = (($HeldSlots | Sort-Object) -join ',') -eq "0,1"
    if ($Parts -contains "sources") {
        $Checks["the same tree selected on both clients ($(($NetSelectedTrees.Values | Sort-Object) -join ' / '))"] = ($NetSelectedTrees.Count -eq 2) -and
            (@($NetSelectedTrees.Values | Sort-Object -Unique).Count -eq 1) -and ($NetSelectedTrees.Values -notcontains "")
    }
    if ($NetServerProcess -and -not $NetServerProcess.HasExited) { Stop-Process -Id $NetServerProcess.Id -Force }
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
        $Checks["server: one house paid and placed per slot, behind its centre ($($Placed -join '; '))"] = ($Placed -join '|') -eq "0: (-16800, 0)|1: (16800, 0)"
        $NearEnemy = @($ServerLines | Select-String -Pattern "Construction: slot (\d) cannot place House at \((-?\d+), (-?\d+)\): too near an enemy main centre" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object)
        $Checks["server: each house near the enemy centre refused ($($NearEnemy -join '; '))"] = ($NearEnemy -join '|') -eq "0: (17400, 0)|1: (-17400, 0)"
        $Started = @($ServerLines | Select-String -Pattern "Construction: slot (\d) first work on House at \((-?\d+), (-?\d+)\), 1 builders" | ForEach-Object { "$($_.Matches[0].Groups[1].Value): ($($_.Matches[0].Groups[2].Value), $($_.Matches[0].Groups[3].Value))" } | Sort-Object)
        $Checks["server: each house's first work by its Peasant ($($Started -join '; '))"] = ($Started -join '|') -eq "0: (-16800, 0)|1: (16800, 0)"
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
