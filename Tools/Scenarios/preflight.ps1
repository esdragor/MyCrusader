# Pre-flight of a trial before it is listed for the user (Auto_Journey_Retours.md, section 1). check.ps1 only says that the scenario
# reaches "ready"; this one says whether the TRIAL is worth the user's time. Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\preflight.ps1 <name> [-Root <worktree>] [-MaxSeconds 180] [-PlayerName <name>]
# Checks (each prints OK / FAIL / WARN, exit 1 if any FAIL):
#   1. check.ps1 passes (the scenario reaches "ready", no error), run -unattended, so the waitkey steps go on by themselves (check.ps1 stops at the first "Angelscript: Error" line of the log instead of waiting for its timeout; tested by preflight_selftest.ps1);
#   2. the user's time: waitkey steps and `wait` seconds in the .txt, and the wall time of the run, stay under -MaxSeconds (default 180 s,
#      the user's limit: a trial lasts 3 minutes at most, the scenario puts the user in the situation, no dead wait: a waiting event is
#      triggered by the scenario or released by a waitkey);
#   3. the .txt holds at least one `note` (what the user must try) and no log line "LogHOL... Warning/Error" appeared during the run;
#   4. every point "to look at" is declared in Tools/Scenarios/<name>.look, one per line: `<capture name> | <what the user must see>`;
#      the scenario must take it (`capture <capture name>`), the PNG must exist and be new: the script prints its path, THE AGENT
#      MUST OPEN EACH PNG (Read) and see the point before writing the trial. No .look file = FAIL (a trial with nothing to see is not listed);
#   4b. HUD panels covering each other (lesson 12b): the log line "HUD overlap: ..." of hol.HUD.Report must say none;
#   5. the sanity audit of the game itself (units of a neutral camp with a player's colour, a unit that spins without reaching its goal,
#      units that shake): a scenario may call `hol.Test.Sanity`; any log line "Sanity: FAIL ..." is a FAIL. Without that call: WARN
#      (the audit is queued in Auto_Journey_File.md, section « Processus »);
#   6. the USER'S launch (LaunchCheck.ps1, 2026-11-12): play.ps1 names the player (first named Human seat of the .json), no
#      uncommitted file the trial needs (-AllowUncommitted = WARN, then it plays only with -Root), each module dll is not older
#      than the newest C++ file of its own Source\<module> folder, and the log has no « candidate missing », step timeout or spectator.
# A trial that does not pass is NOT "ready": it is written « LOGIQUE SEULEMENT » in the retours file, or not listed.
param(
    [Parameter(Position = 0, Mandatory = $true)] [string] $Name,
    [string] $Root = "",
    [int] $MaxSeconds = 180,
    [string] $PlayerName = "",  # passed to check.ps1 then to the game (-PlayerName=)
    [switch] $AllowUncommitted  # the trial needs files git does not hold (audio candidates, decision B): WARN instead of FAIL, it plays ONLY with -Root <this worktree>
)

if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Txt = Join-Path $Root "Tools\Scenarios\$Name.txt"
if (-not (Test-Path $Txt)) { Write-Host "preflight: no scenario '$Name'"; exit 1 }
$Look = Join-Path $Root "Tools\Scenarios\$Name.look"
$Fails = 0
. (Join-Path $PSScriptRoot "HudRects.ps1")

function Report([string]$Level, [string]$Text) {
    if ($Level -eq "FAIL") { $script:Fails++ }
    Write-Host ("{0,-5} {1}" -f $Level, $Text)
}

# 2 (static part) and 3 (static part): what the .txt asks of the user's time
$Lines = Get-Content $Txt | Where-Object { $_.Trim() -and -not $_.Trim().StartsWith("#") }
$Waits = ($Lines | Where-Object { $_ -match '^wait\s+([\d\.]+)' } | ForEach-Object { [double]$Matches[1] } | Measure-Object -Sum).Sum
$Keys = @($Lines | Where-Object { $_ -match '^waitkey' }).Count
$Notes = @($Lines | Where-Object { $_ -match '^note(reset)?\s' }).Count
$Captures = @($Lines | Where-Object { $_ -match '^capture\s+(\S+)' } | ForEach-Object { ($_ -split '\s+')[1] })
# A capture sequence (`captureseq <Name> <Count> <Step> [Command]`, rule 12a) takes <Name>_0 ... <Name>_<Count-1>; it spans (Count-1) * Step s.
foreach ($Seq in @($Lines | Where-Object { $_ -match '^captureseq\s+(\S+)\s+(\d+)\s+([\d\.]+)' })) {
    $null = $Seq -match '^captureseq\s+(\S+)\s+(\d+)\s+([\d\.]+)'
    foreach ($i in 0..([int]$Matches[2] - 1)) { $Captures += "$($Matches[1])_$i" }
    $Waits += ([int]$Matches[2] - 1) * [double]$Matches[3]
}
if ($Waits -gt $MaxSeconds) { Report "FAIL" "static: the scenario waits $Waits s of game time (> $MaxSeconds s)" }
else { Report "OK" "static: $Waits s of explicit waits, $Keys waitkey step(s) (Enter = next stage, the user decides when)" }
if ($Notes -lt 1) { Report "FAIL" "no note/notereset line: the user is not told what to try" } else { Report "OK" "$Notes note(s) for the panel" }

# 2b (static, E-fog_trees 2026-11-08): a ground click far (> 10 frames) from the camera placement. check.ps1 runs with the edge scrolling off and no cursor,
# so it never sees what the user's game (play.ps1: edge scrolling on, a cursor, keys) does: the camera drifts, the point leaves the screen, the click never comes,
# the panel stays on "en preparation...". See PanExposure.ps1 and pan_exposure_test.ps1.
. (Join-Path $PSScriptRoot "PanExposure.ps1")
$PanOffenders = @(Get-PanExposure (Get-Content -Encoding UTF8 $Txt))
if ($PanOffenders.Count -gt 0) { Report "FAIL" "pan exposure: $($PanOffenders[0]) ($($PanOffenders.Count) in all; set the camera right before the click, 2 frames, not before a long wait)" }
else { Report "OK" "pan exposure: every ground click is within 10 frames of the camera placement" }

# 6 (static, 2026-11-12): the USER'S launch, not the agent's. Four trials passed this preflight and never started for the user (2026-11-11):
# play.ps1 without -PlayerName on named seats (spectator), files only in the agent's worktree, a stale binary in the copy (LaunchCheck.ps1,
# launch_check_test.ps1). Run the preflight with the -Root of the command given to the user.
. (Join-Path $PSScriptRoot "LaunchCheck.ps1")
$LaunchProblems = @(Get-LaunchProblems -Root $Root -Name $Name -PlayerName $PlayerName)
foreach ($Problem in $LaunchProblems) {
    if ($AllowUncommitted -and $Problem -match 'uncommitted') { Report "WARN" "user's launch: $Problem (-AllowUncommitted: give the trial ONLY with -Root $Root)" }
    else { Report "FAIL" "user's launch: $Problem" }
}
if ($LaunchProblems.Count -eq 0) { Report "OK" "user's launch: play.ps1 names the local player if needed, every file the trial needs is committed" }
# A binary older than the C++ sources of this copy: the user's game runs old code (legions_roster_gesture, step 41). LaunchCheck.ps1.
$Stale = Get-StaleBinaryProblem $Root
if (-not (Test-Path (Join-Path $Root "Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll"))) { Report "FAIL" "user's launch: no binary in $Root (Tools\build.ps1 in that copy first)" }
elseif ($Stale) { Report "FAIL" "user's launch: $Stale" }
else { Report "OK" "user's launch: binary not older than any C++ source of $Root" }

# 4 (static part): the .look file
$Points = @()
if (-not (Test-Path $Look)) { Report "FAIL" "no $Name.look: declare each point to look at as '<capture name> | <what the user must see>'" }
else {
    $Points = @(Get-Content $Look | Where-Object { $_.Trim() -and -not $_.Trim().StartsWith("#") } | ForEach-Object {
        $Parts = $_ -split '\|', 2
        [pscustomobject]@{ Capture = $Parts[0].Trim(); Text = $Parts[1].Trim() }
    })
    if ($Points.Count -eq 0) { Report "FAIL" "$Name.look is empty" }
    foreach ($P in $Points) {
        if ($Captures -notcontains $P.Capture) { Report "FAIL" "point '$($P.Capture)' has no `capture $($P.Capture)` step in $Name.txt" }
    }
}

# 1, 2 (wall time), 3, 5: the run
$Start = Get-Date
$CheckArgs = @("-ExecutionPolicy", "Bypass", "-File", (Join-Path $Root "Tools\Scenarios\check.ps1"), $Name, "-Root", $Root, "-TimeoutSeconds", ($MaxSeconds + 60))
if ($PlayerName) { $CheckArgs += @("-PlayerName", $PlayerName) }   # an empty value would reach check.ps1 as "-PlayerName" with no argument
& powershell @CheckArgs | Out-Host
$CheckExit = $LASTEXITCODE
$Wall = [int]((Get-Date) - $Start).TotalSeconds
if ($CheckExit -eq 0) { Report "OK" "check.ps1: the scenario reaches ready" } else { Report "FAIL" "check.ps1 failed (see above)" }
if ($Wall -gt $MaxSeconds) { Report "WARN" "wall time of the run $Wall s (> $MaxSeconds s; the off-screen run is not the user's time, but a scenario this long has to be cut)" }

$Log = Join-Path $Root "Saved_Tests\Logs\scenario_$Name.log"
if (Test-Path $Log) {
    $Text = Get-Content $Log
    # Any error line of the game or of the script engine (a stale build shows as "Angelscript: Error"), and HOL warnings.
    # LogAutomationTest errors are written by the engine at frame 0 of every run (also on a clean one): not a verdict on the scenario.
    $Bad = @($Text | Select-String -Pattern 'LogHOL\w*: (Warning|Error)|Angelscript: Error|\]\w+: Error:' | Where-Object { $_.Line -notmatch 'LogHOLTest: Warning: .*waitkey|LogAutomationTest: Error' })
    if ($Bad.Count -gt 0) { Report "FAIL" "$($Bad.Count) warning/error line(s) in the log, first: $($Bad[0].Line.Trim()) (a script error right at start usually means the build of this worktree is stale: Tools\build.ps1 first)" } else { Report "OK" "no warning or error in the log" }
    # 6 (log): what the scenario asked for and did not get, even at Display level ("candidate missing": audio_compare_buildings_*, 2026-11-11).
    $Missing = @(Get-LogLaunchProblems $Text)
    if ($Missing.Count -gt 0) { Report "FAIL" "$($Missing.Count) missing asset / failed step line(s) in the log, first: $($Missing[0])" } else { Report "OK" "no missing asset, timed-out step or spectator in the log" }
    # 12b: a panel that hides another. hol.HUD.Report writes "HUD overlap: none" or "HUD overlap: A x B; ..." (the script's panels and the cheats');
    # any line but "none" is a FAIL. Without a hol.HUD.Report in the scenario: WARN (the check is blind).
    $Overlaps = @($Text | Select-String -Pattern 'HUD overlap: ')
    if ($Overlaps.Count -eq 0) { Report "WARN" "no 'HUD overlap:' line: the scenario does not call hol.HUD.Report at the moment of the screenshots (a panel hiding another is not checked)" }
    elseif (@($Overlaps | Where-Object { $_.Line -notmatch 'HUD overlap: none' }).Count -gt 0) { Report "FAIL" ("HUD panels cover each other: " + (($Overlaps | Where-Object { $_.Line -notmatch 'HUD overlap: none' } | Select-Object -First 2 | ForEach-Object { $_.Line.Trim() }) -join " / ")) }
    else { Report "OK" "no HUD panel covers another ($($Overlaps.Count) report(s))" }
    # 12b, every visible widget: "HUD rects: name=x0,y0,x1,y1; ..." (HudRects.ps1; written by hol.HUD.Report once the game logs the boxes of ALL
    # visible widgets, not only the script's panels, the cheats' panel, PlayersPanel and MenuPanel: FogInProgress.md, "besoin C++ L1-12b").
    # Without that line nothing is said here (the line above is the check that exists today); with it, two boxes that cover each other are a FAIL.
    $RectLines = @($Text | Where-Object { $_ -match 'HUD rects:' })
    if ($RectLines.Count -gt 0) {
        $RectOverlaps = @(Get-HudRectOverlaps $Text)
        if ($RectOverlaps.Count -gt 0) { Report "FAIL" ("a widget hides another (HUD rects): " + (($RectOverlaps | Select-Object -First 2) -join " / ")) }
        else { Report "OK" "no visible widget covers another ($($RectLines.Count) HUD rects line(s))" }
    }
    $Sanity = @($Text | Select-String -Pattern 'Sanity: ')
    if ($Sanity.Count -eq 0) { Report "WARN" "no 'Sanity:' audit line: the scenario does not call hol.Test.Sanity (neutral colours, spinning, shaking not checked)" }
    elseif (@($Sanity | Where-Object { $_.Line -match 'Sanity: FAIL' }).Count -gt 0) { Report "FAIL" ("sanity audit: " + (($Sanity | Where-Object { $_.Line -match 'Sanity: FAIL' } | Select-Object -First 3 | ForEach-Object { $_.Line.Trim() }) -join " / ")) }
    else { Report "OK" "sanity audit passed" }
}

# 4 (dynamic part): the captures taken during this run
. (Join-Path $PSScriptRoot "CaptureResolve.ps1")
foreach ($P in $Points) {
    # The engine writes captures under Screenshots\<platform>\HOL (WindowsEditor in the -game runs of check.ps1): CaptureResolve.ps1 takes the newest.
    $File = Resolve-CapturePath $Root $P.Capture
    if ((Test-Path $File) -and ((Get-Item $File).LastWriteTime -ge $Start)) { Report "OK" "point '$($P.Capture)': $File  <- OPEN IT and see: $($P.Text)" }
    else { Report "FAIL" "point '$($P.Capture)': no new capture at $File" }
}

Write-Host "preflight $Name : $(if ($Fails -eq 0) { 'PASS (list the trial only once every PNG above has been opened and seen)' } else { "FAIL ($Fails)" })"
if ($Fails -eq 0) { exit 0 } else { exit 1 }
