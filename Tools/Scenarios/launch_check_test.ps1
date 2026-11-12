# Test of LaunchCheck.ps1 (preflight.ps1, section "the user's launch"): the trials that passed the agent's preflight and never started for the
# user on 2026-11-11 evening (Retours\E-disconnect_1v1.md, E-audio_compare_buildings_stone/_metal.md, E-legions_roster_gesture.md).
# Every case uses the real chain: the real play.ps1 -DryRun command line, the real scenario files, a real excerpt of the user's game log.
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\launch_check_test.ps1     (exit 0 = green)
$Root = (Resolve-Path "$PSScriptRoot\..\..").Path
. (Join-Path $PSScriptRoot "LaunchCheck.ps1")
$Fails = 0
function Expect([bool]$Ok, [string]$Text) { if ($Ok) { Write-Host "PASS $Text" } else { Write-Host "FAIL $Text"; $script:Fails++ } }

# 1. disconnect_1v1: named Human seats (Alice, Bob). The user ran play.ps1 without -PlayerName (log: "joined as a spectator (name '')",
#    "timeout at step 5: waitlog Camera above the start of slot"). The command play.ps1 builds without -PlayerName must name the local player.
$P = @(Get-LaunchProblems -Root $Root -Name "disconnect_1v1")
Expect (@($P | Where-Object { $_ -match 'spectator' }).Count -eq 0) "disconnect_1v1: play.ps1 without -PlayerName names the local player ($($P -join ' / '))"
$Dry = (& powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "play.ps1") disconnect_1v1 -Root $Root -DryRun) -join " "
Expect ($Dry -match '-PlayerName=Alice') "disconnect_1v1: play.ps1 -DryRun passes -PlayerName=Alice by itself ($Dry)"
# A scenario without named seats is not touched.
$Dry2 = (& powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "play.ps1") legions_roster_gesture -Root $Root -DryRun) -join " "
Expect ($Dry2 -notmatch '-PlayerName=') "legions_roster_gesture: no -PlayerName added (no named seat)"

# 2. audio_compare_buildings_*: the candidate sounds lived only in the Arwen worktree, never committed. Played from another copy, the log said
#    "candidate missing" at Display level, which preflight did not read. Real lines of the user's log (Haldir, 2026-11-11 18:17).
$UserLog = @(
    "[2026.11.11-18.17.00:115][994]LogUObjectGlobals: Warning: Objet 'Object /Game/Audio/Candidates/soft/SW_s_stone2_mine_deep.SW_s_stone2_mine_deep' introuvable",
    "[2026.11.11-18.17.00:115][994]LogHOLMatch: candidate missing Building.Stone A asset /Game/Audio/Candidates/soft/SW_s_stone2_mine_deep.SW_s_stone2_mine_deep")
$L = @(Get-LogLaunchProblems $UserLog)
Expect ($L.Count -ge 1) "audio: a missing candidate in the log is a problem ($($L -join ' / '))"
Expect (@(Get-LogLaunchProblems @("[0]LogHOLMatch: candidate Building.Stone A played s_stone2_mine_deep")).Count -eq 0) "audio: a played candidate is not a problem"
# The files a trial needs and that git does not hold (untracked or modified): another copy will not have them.
$U = @(Get-UncommittedDependencies @("?? Content/Audio/Candidates/soft/SW_s_stone2_mine_deep.uasset", " M Script/Audio/HOLAudioPlan.as", "?? Saved/Logs/x.log") "audio_compare_buildings_stone")
Expect ($U.Count -eq 2) "audio: untracked Content and modified Script are uncommitted dependencies, Saved is not ($($U -join ' / '))"
Expect (@(Get-UncommittedDependencies @("?? Tools/Scenarios/audio_compare_buildings_stone.txt") "audio_compare_buildings_stone").Count -eq 1) "audio: an uncommitted scenario file is a dependency"
Expect (@(Get-UncommittedDependencies @("?? Tools/Scenarios/other.txt") "audio_compare_buildings_stone").Count -eq 0) "audio: another scenario's file is not"

# 3. A step that never came for the user: the "real launch" log of play.ps1 -Hidden (Saved\Logs) says timeout / failed.
$T = @(Get-LogLaunchProblems @("[723]LogHOLTest: Error: Test sequence: timeout at step 41: waitlog HUD selection name: Garde noire$", "[723]LogHOLUI: Display: Scenario panel: scenario legions_roster_gesture failed, 0 notes"))
Expect ($T.Count -ge 1) "a timed-out step in the user's launch is a problem ($($T -join ' / '))"

if ($Fails -eq 0) { Write-Host "launch_check_test: PASS"; exit 0 } else { Write-Host "launch_check_test: FAIL ($Fails)"; exit 1 }
