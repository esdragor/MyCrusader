# Quick check of the project (D267): the automation tests run in their own process WHILE a single game launch goes
# through the menu in French, English and Japanese (language switched live), a drop-down list, then a match at 2 and
# at 8 players with the camera checks of D257 and the selection checks of D264 (own centre, another's, void, Escape).
# Every step starts as soon as the one before is done (hol.Test.Run), no fixed delays. Replaced capture_screens.ps1,
# check_camera.ps1 and the automation command for everyday checks; the short stress tests S1 and S2 stay apart (only
# when units, the simulation or rendering change, and at a closure).
# Alongside, the "net" part (D273, M2.5.6): a two-client match on a dedicated server, the editor in server mode (D275),
# each client checking its camera, the two centres in their colours and a click on each.
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths), with no editor of the project open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\quick_check.ps1 [-Parts tests,menu,camera,production,net] [-EngineDir <dir>] [-Visible]
# The game renders off screen, with no window (the user works on the machine meanwhile); -Visible shows it on the
# second screen instead, windowed 1600 x 900, and the two clients side by side under it.
# The engine is the project's (EngineAssociation of the .uproject, found in the registry), unless -EngineDir is given.
# Results: a PASS / FAIL line per part, captures in Saved_Tests\Screenshots\WindowsEditor\HOL (qc_*, cam_*, sel_*),
# contact sheets qc_<culture>.png (Python with Pillow), logs Saved_Tests\Logs\quick_game.log and quick_tests.log.
param(
    [string[]] $Parts = @("tests", "menu", "camera", "production", "net"),
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
    $TestsArguments = "`"$UProject`" -ExecCmds=`"Automation RunTests HeroesOfLiberty`" -TestExit=`"Automation Test Queue Empty`" -saveddirsuffix=Tests -unattended -nullrhi -nosplash -nosound -abslog=`"$TestsLog`""
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
    $NetServerArguments = "`"$UProject`" /Game/Maps/Plain -server -unattended -nosplash -nosound -saveddirsuffix=Tests_NetSrv -MatchConfig=`"$NetConfig`" -abslog=`"$NetServerLog`""
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
            Step "hol.HUD.Click GridCell00", "waitlog queued Peasant (1 in"
            Key A
            Step "waitlog queued Peasant (2 in"
            Key A
            Step "waitlog queued Peasant (3 in", "frames 5", "hol.HUD.Report", "capture prod_queue"
            Step "hol.HUD.Click QueueItem2", "waitlog cancelled Peasant", "waitlog (2 born here)", "frames 10", "hol.HUD.Report", "hol.Prod.Report", "capture prod_done",
                "hol.Prod.BuildTime 0"
        }
    }
}
Step "mark end", "quit"
$SequenceFile = Join-Path $TestSaved "quick_check.seq"
$Steps | Set-Content -Encoding utf8 $SequenceFile

Get-ChildItem $Shots -Filter "qc_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "cam_*.png" -ErrorAction SilentlyContinue | Remove-Item
Get-ChildItem $Shots -Filter "sel_*.png" -ErrorAction SilentlyContinue | Remove-Item
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
        $NetSteps += "hol.Select.Click 0.03 0.05", "waitlog Selection: none", "hol.Select.Report", "mark end", "quit"
        $ClientSequence = Join-Path $TestSaved "quick_net_client$Client.seq"
        $NetSteps | Set-Content -Encoding utf8 $ClientSequence
        $ClientLog = Join-Path $TestSaved "Logs\quick_net_client$Client.log"
        $ClientDisplay = if ($Visible) { "-windowed -ResX=940 -ResY=530 -WinX=$(3940 + ($Client - 1) * 960) -WinY=1020" } else { "-RenderOffScreen -ResX=960 -ResY=540 -nosound" }
        $ClientArguments = "`"$UProject`" 127.0.0.1 -game $ClientDisplay -saveddirsuffix=Tests_Net$Client -nosplash -culture=fr -abslog=`"$ClientLog`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Test.Run $ClientSequence`""
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
    queue = "hud=HOLGameScreen"; done = "hud=HOLGameScreen" }
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

if ($Parts -contains "net") {
    $Checks = [ordered]@{}
    $Checks["server listening"] = $NetListening
    $HeldSlots = @()
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
        $Missing = @("start", "slot0", "slot1" | Where-Object { -not (Test-Path (Join-Path $NetClient.Shots "qcnet_${N}_$_.png")) })
        $Checks["client $N : captures (missing: $($Missing -join ', '))"] = $Missing.Count -eq 0
        $ClientErrors = @($ClientLines | Select-String -Pattern "LogHOLMatch: (Error|Warning)|LogHOLNet: (Error|Warning)|LogHOLTest: Error")
        $Checks["client $N : no match, network or sequence error ($($ClientErrors.Count))"] = $ClientErrors.Count -eq 0
    }
    $Checks["two clients in slots 0 and 1 ($($HeldSlots -join ', '))"] = (($HeldSlots | Sort-Object) -join ',') -eq "0,1"
    if ($NetServerProcess -and -not $NetServerProcess.HasExited) { Stop-Process -Id $NetServerProcess.Id -Force }
    $ServerErrors = @(if (Test-Path $NetServerLog) { Select-String -Path $NetServerLog -Pattern "LogHOLMatch: (Error|Warning)|LogHOLNet: (Error|Warning)" })
    $Checks["server: no match or network error ($($ServerErrors.Count))"] = $ServerErrors.Count -eq 0
    $AllOk = (Report "net 2 clients" $Checks) -and $AllOk
}

if ($TestsProcess) {
    $TestsTimedOut = -not $TestsProcess.WaitForExit(300000)
    if ($TestsTimedOut) { $TestsProcess.Kill() }
    $Results = @(if (Test-Path $TestsLog) { Select-String -Path $TestsLog -Pattern "Test Completed\. Result=\{(\w+)\}.*Path=\{([^}]+)\}" | ForEach-Object { $_.Matches[0] } })
    $Failures = @($Results | Where-Object { $_.Groups[1].Value -ne "Success" } | ForEach-Object { $_.Groups[2].Value })
    $Checks = [ordered]@{}
    $Checks["tests ran ($($Results.Count))"] = (-not $TestsTimedOut) -and $Results.Count -gt 0
    $Checks["all green (failed: $($Failures -join ', '))"] = $Failures.Count -eq 0
    $AllOk = (Report "tests $($Results.Count - $Failures.Count)/$($Results.Count)" $Checks) -and $AllOk
}

Write-Host "quick_check : $(if ($AllOk) { 'PASS' } else { 'FAIL' }) in $([Math]::Round($Clock.Elapsed.TotalSeconds)) s (game launch done at $GameSeconds s)"
if (-not $AllOk) { exit 1 }
