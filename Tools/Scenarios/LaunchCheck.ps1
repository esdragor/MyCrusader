# Checks of the USER'S launch of a trial, dot-sourced by preflight.ps1, play.ps1 and check.ps1 (test: launch_check_test.ps1).
# 2026-11-11 evening, four trials that passed the agent's preflight never started for the user: what the agent's run (check.ps1, its own
# worktree, -unattended, Saved_Tests) did not see and the user's launch (play.ps1 from another copy, Saved) hit:
#   - disconnect_1v1: named Human seats (playerName in the .json) and play.ps1 without -PlayerName = the user is a spectator, step 5 never comes;
#   - audio_compare_buildings_stone/_metal: candidate sounds only in the agent's worktree, never committed: "candidate missing" in the log, a
#     Display line the preflight did not read;
#   - legions_roster_gesture: step 41 timed out in the user's game (the binary of the copy was older than its scripts).

# The name of the local player of a scenario whose Human seats are NAMED: the first named Human seat of <Name>.json, "" otherwise.
function Get-ScenarioPlayerName([string]$Root, [string]$Name) {
    $Json = Join-Path $Root "Tools\Scenarios\$Name.json"
    if (-not (Test-Path $Json)) { return "" }
    try { $Config = Get-Content -Raw -Encoding UTF8 $Json | ConvertFrom-Json } catch { return "" }
    foreach ($Slot in @($Config.slots)) {
        if ($Slot.state -eq "Human" -and $Slot.playerName) { return [string]$Slot.playerName }
    }
    return ""
}

# The lines of `git status --porcelain` that a trial depends on and that git does not hold: another copy (the user's) will not have them.
function Get-UncommittedDependencies([string[]]$Porcelain, [string]$Name) {
    $Out = @()
    foreach ($Line in @($Porcelain)) {
        if (-not $Line -or $Line.Length -lt 4) { continue }
        $Path = $Line.Substring(3).Trim('"') -replace '\\', '/'
        if ($Path -match '^(Content|Script|Source|Config|Plugins)/' -or $Path -match "^Tools/Scenarios/$([regex]::Escape($Name))\.") { $Out += $Line.Trim() }
    }
    return $Out
}

# Static problems of the user's launch: the command play.ps1 builds (as the trial sheet gives it, -PlayerName only if passed) and the files
# the trial needs that are not committed in -Root.
function Get-LaunchProblems([string]$Root, [string]$Name, [string]$PlayerName = "") {
    $Problems = @()
    $Seat = Get-ScenarioPlayerName $Root $Name
    if ($Seat) {
        $PlayArgs = @("-ExecutionPolicy", "Bypass", "-File", (Join-Path $Root "Tools\Scenarios\play.ps1"), $Name, "-Root", $Root, "-DryRun")
        if ($PlayerName) { $PlayArgs += @("-PlayerName", $PlayerName) }
        $Dry = (& powershell @PlayArgs) -join " "
        if ($Dry -notmatch '-PlayerName=\S') { $Problems += "named Human seats ($Seat ...) but play.ps1 gives no -PlayerName: the user would be a spectator" }
    }
    $Porcelain = @(git -C $Root status --porcelain --untracked-files=all 2>$null)
    $Deps = @(Get-UncommittedDependencies $Porcelain $Name)
    if ($Deps.Count -gt 0) { $Problems += "$($Deps.Count) uncommitted file(s) the trial may need, a copy without them will not play it, first: $($Deps[0])" }
    return $Problems
}

# A binary of -Root older than its code: "" if fresh (or no dll), the problem otherwise. Compared like UBT does: the newest C++ source FILE
# of the copy (a merge or fast-forward writes its files at that time) against the dll. Not the commit date (42e5e41): a dll built before
# the commit of the same code is fresh, and a no-op rebuild would never make it newer than that commit (launch_check_test.ps1, case 4).
function Get-StaleBinaryProblem([string]$Root) {
    $Dll = Join-Path $Root "Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll"
    if (-not (Test-Path $Dll)) { return "" }
    $Dirs = @("Source", "Plugins") | ForEach-Object { Join-Path $Root $_ } | Where-Object { Test-Path $_ }
    if (-not $Dirs) { return "" }
    $Newest = Get-ChildItem $Dirs -Recurse -File -Include *.h, *.cpp, *.inl, *.cs -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch '\\(Intermediate|Binaries)\\' } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    $DllTime = (Get-Item $Dll).LastWriteTime
    if ($Newest -and $Newest.LastWriteTime -gt $DllTime) {
        return "the binary of $Root ($($DllTime.ToString('s'))) is older than its C++ source $($Newest.FullName.Substring($Root.Length).TrimStart('\')) ($($Newest.LastWriteTime.ToString('s'))): Tools\build.ps1 in that copy first"
    }
    return ""
}

# Problems in a game log of the scenario (check.ps1's or the user's launch): an asset the scenario asked for and did not find, a step that
# timed out, a failed scenario, a local player that became a spectator.
function Get-LogLaunchProblems([string[]]$Lines) {
    $Patterns = @(
        'candidate missing',
        "LogUObjectGlobals: Warning: .*/Game/Audio/.*(introuvable|Failed to find)",
        'Test sequence: timeout at step',
        'Scenario panel: scenario \S+ failed',
        "joined as a spectator \(name ''")
    $Out = @()
    foreach ($Line in @($Lines)) {
        foreach ($P in $Patterns) { if ($Line -match $P) { $Out += $Line.Trim(); break } }
    }
    return $Out
}
