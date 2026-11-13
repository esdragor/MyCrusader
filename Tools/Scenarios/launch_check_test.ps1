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

# 4. legions_roster_gesture: the copy fast-forwarded on main (C++ of 17:01-17:17 arrived at 18:01) without a rebuild (dll 16:02). The check
#    runs on a real throwaway git copy: a real dll file, real source files, real commits. UBT relinks only when a source file is newer than the
#    dll, so "stale" = a source file newer than the dll; a dll built BEFORE the commit of the same code (build, test, then commit) is fresh,
#    and a no-op rebuild would never refresh it.
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ("launch_check_" + [guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force "$Tmp\Source\HeroesOfLiberty", "$Tmp\Binaries\Win64", "$Tmp\Tools\Scenarios" | Out-Null
git -C $Tmp init -q 2>$null; git -C $Tmp config user.email t@t; git -C $Tmp config user.name t
Set-Content "$Tmp\.gitignore" "Binaries/"
Set-Content "$Tmp\Source\HeroesOfLiberty\A.cpp" "int A = 1;"
$Dll = "$Tmp\Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll"; Set-Content $Dll "dll"
(Get-Item "$Tmp\Source\HeroesOfLiberty\A.cpp").LastWriteTime = (Get-Date).AddMinutes(-30)
(Get-Item $Dll).LastWriteTime = (Get-Date).AddMinutes(-20)
git -C $Tmp add -A 2>$null; git -C $Tmp commit -q -m "built then committed" 2>$null
Expect (-not (Get-StaleBinaryProblem $Tmp)) "binary: built before the commit of the same code is fresh ($(Get-StaleBinaryProblem $Tmp))"
Set-Content "$Tmp\Source\HeroesOfLiberty\A.cpp" "int A = 2;"   # a merge / fast-forward writes the file now
git -C $Tmp commit -q -am "C++ after the build" 2>$null
Expect ([bool](Get-StaleBinaryProblem $Tmp)) "binary: a source file newer than the dll is a stale binary ($(Get-StaleBinaryProblem $Tmp))"
(Get-Item $Dll).LastWriteTime = Get-Date
Expect (-not (Get-StaleBinaryProblem $Tmp)) "binary: rebuilt after the change is fresh"
# 6. One dll per module (2026-11-12 recette): Source\HeroesOfLibertyUI has its own UnrealEditor-HeroesOfLibertyUI.dll. Each module's dll is
#    compared to the sources of ITS folder: a UI source newer than the UI dll is stale even when the main dll is newer still, and the reverse.
New-Item -ItemType Directory -Force "$Tmp\Source\HeroesOfLibertyUI" | Out-Null
$UiDll = "$Tmp\Binaries\Win64\UnrealEditor-HeroesOfLibertyUI.dll"; Set-Content $UiDll "ui dll"
(Get-Item $UiDll).LastWriteTime = (Get-Date).AddMinutes(-10)
Set-Content "$Tmp\Source\HeroesOfLibertyUI\Hud.cpp" "int Hud = 1;"
(Get-Item "$Tmp\Source\HeroesOfLibertyUI\Hud.cpp").LastWriteTime = (Get-Date).AddMinutes(-5)
(Get-Item $Dll).LastWriteTime = Get-Date
git -C $Tmp add -A 2>$null; git -C $Tmp commit -q -m "UI module" 2>$null
$P6 = Get-StaleBinaryProblem $Tmp
Expect ([bool]$P6 -and $P6 -match 'HeroesOfLibertyUI') "binary: a UI source newer than the UI dll is stale though the main dll is newer ($P6)"
(Get-Item $UiDll).LastWriteTime = Get-Date
(Get-Item "$Tmp\Source\HeroesOfLiberty\A.cpp").LastWriteTime = (Get-Date).AddSeconds(30)
$P6b = Get-StaleBinaryProblem $Tmp
Expect ([bool]$P6b -and $P6b -match 'A\.cpp') "binary: a main-module source newer than the main dll is stale though the UI dll is newer ($P6b)"
(Get-Item $Dll).LastWriteTime = (Get-Date).AddMinutes(1); (Get-Item $UiDll).LastWriteTime = (Get-Date).AddMinutes(1)
Expect (-not (Get-StaleBinaryProblem $Tmp)) "binary: both module dlls newer than their sources is fresh"
# 5. The uncommitted files on a real git status (not a hand-written porcelain): a scenario file only in this copy, then committed.
Set-Content "$Tmp\Tools\Scenarios\only_here.txt" "wait 1"
Expect (@(Get-LaunchProblems -Root $Tmp -Name "only_here" | Where-Object { $_ -match 'uncommitted' }).Count -eq 1) "uncommitted: an untracked scenario file of the trial is a problem"
git -C $Tmp add -A 2>$null; git -C $Tmp commit -q -m "scenario" 2>$null
Expect (@(Get-LaunchProblems -Root $Tmp -Name "only_here").Count -eq 0) "uncommitted: once committed it is not"
Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue

if ($Fails -eq 0) { Write-Host "launch_check_test: PASS"; exit 0 } else { Write-Host "launch_check_test: FAIL ($Fails)"; exit 1 }
