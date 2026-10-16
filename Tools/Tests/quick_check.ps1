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
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths), with no editor of the project open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1 [-EngineDir <dir>] [-Visible]
#   powershell -ExecutionPolicy Bypass -Command "& .\Tools\Tests\quick_check.ps1 -Parts net,sources,move"   (-File passes a list as one string)
# The game renders off screen, with no window (the user works on the machine meanwhile); -Visible shows it on the
# second screen instead, windowed 1600 x 900, and the two clients side by side under it.
# The engine is the project's (EngineAssociation of the .uproject, found in the registry), unless -EngineDir is given.
# Results: a PASS / FAIL line per part, captures in Saved_Tests\Screenshots\WindowsEditor\HOL (qc_*, cam_*, sel_*),
# contact sheets qc_<culture>.png (Python with Pillow), logs Saved_Tests\Logs\quick_game.log and quick_tests.log.
param(
    [string[]] $Parts = @("tests", "menu", "camera", "production", "move", "sources", "gather", "net"),
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

# Start slots of /Game/Maps/Plain (Tools/Editor/create_match_map.py): 150 m from the centre, at these angles.
$SlotAngles = @(180, 0, 270, 90, 225, 45, 315, 135)
function Get-SlotStart([int] $Slot) {
    $Radians = $SlotAngles[$Slot] * [Math]::PI / 180
    return @([Math]::Round(15000 * [Math]::Cos($Radians)), [Math]::Round(15000 * [Math]::Sin($Radians)))
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
        $NetSteps += "mark end", "quit"
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
    popup = "hud=HOLGameScreen" }
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
    # the mark on, every unload logged by the simulation and every +10 by the controller, in the same step.
    $GatherMark = 0; for ($i = 0; $i -lt $Log.Count; $i++) { if ($Log[$i] -match "Test mark: gather$") { $GatherMark = $i; break } }
    $FromGather = $Log[$GatherMark..($Log.Count - 1)]
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
