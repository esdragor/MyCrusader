# Opens the game on a test scenario (Tools/Scenarios/<Name>.txt, README.md), with the user's own settings (window,
# screen, saved games): the user's trial, not a scripted check. Run from Windows PowerShell (never Git Bash: it rewrites
# /Game/... paths):
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play.ps1 [-Name] <name> [-Root <worktree>] [-EngineDir <dir>] [-PlayerName <name>]
# Without a name: lists the scenarios, each with the first comment line of its file. -LiveCoding=0: an open game
# would otherwise block every build of the project (UnrealBuildTool refuses while Live Coding runs), any worktree.
# -Hidden (agents, L11-6 2026-11-11): no window, the virtual cursor is centred (hol.Camera.TestCursor 0.5 0.5, as the desktop mouse of a
# windowless game sits on the screen edge and the camera drifts), and play.ps1 WAITS for the game to end (at most -TimeoutSec, default 300)
# then closes the editor and every child it started, so nothing stays alive after the script. -NoWait gives the old behaviour (returns at once,
# the editor lives on: stray_processes.ps1 -Stop at the end). -DryRun prints the command line and starts nothing.
param(
    [Parameter(Position = 0)] [string] $Name = "",
    [string] $Root = "",
    [string] $EngineDir = "",
    [switch] $Hidden,   # for an AGENT replaying the user's gesture: no window (-RenderOffScreen -nosound -unattended, -WindowStyle Hidden); by default the window is visible, for the user only (agent_rules.md, "aucun lancement fenetre")
    [string] $PlayerName = "",   # -PlayerName=<name> on the game's command line: the local player of a scenario whose Human seats are NAMED (playerName in the .json) needs a name, otherwise it is a spectator
    [int] $TimeoutSec = 300,     # -Hidden only: the longest wait for the game to end, then it is closed
    [switch] $NoWait,            # -Hidden only: do not wait, do not close (old behaviour)
    [switch] $DryRun             # print the command line, start nothing
)

if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Scenarios = Join-Path $Root "Tools\Scenarios"

if (-not $Name) {
    Write-Host "Scenarios ($Scenarios):"
    foreach ($File in Get-ChildItem (Join-Path $Scenarios "*.txt") | Sort-Object Name) {
        $Description = Get-Content -Encoding UTF8 $File.FullName | Where-Object { $_ -match "^\s*#" } | Select-Object -First 1
        $Description = $(if ($Description) { ($Description -replace "^\s*#\s*", "") } else { "" })
        Write-Host ("  {0,-20} {1}" -f $File.BaseName, $Description)
    }
    Write-Host "Usage: play.ps1 <name> [-Root <worktree>]"
    exit 0
}

if (-not (Test-Path (Join-Path $Scenarios "$Name.txt"))) {
    Write-Host "play: no scenario '$Name' in $Scenarios (play.ps1 alone lists them)"
    exit 1
}

$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    # An installed build registered under its name (HKCU\Software\Epic Games\Unreal Engine\Builds), as quick_check.ps1.
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "play: engine '$Association' of the .uproject not found in the registry; pass -EngineDir"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Editor = Join-Path $EngineDir "UnrealEditor.exe"
Write-Host "play: scenario $Name ($UProject)"
# The game starts without waiting behind another copy's build (UBT's SDK check, up to minutes) and with its Angelscript
# debug server on a free port, where no VS Code connects (on the default one the game froze, 2026-10-27).
$env:UE_SKIP_UBT_SDK_SETUP = "1"
$GameArgs = @("`"$UProject`"", "-game", "-nosplash", "-culture=fr", "-LiveCoding=0", "-asdebugport=0")
if ($PlayerName) { $GameArgs += "-PlayerName=$PlayerName" }
# Hidden: the virtual cursor first (a console variable, it survives the scenario's `open`), then the scenario.
$ExecCmds = "hol.Scenario $Name"
if ($Hidden) { $ExecCmds = "hol.Camera.TestCursor 0.5 0.5,$ExecCmds" }
$GameArgs += "-ExecCmds=`"$ExecCmds`""
if ($Hidden) { $GameArgs += @("-RenderOffScreen", "-nosound", "-unattended") }
# Test seam (play_hidden_test.ps1): arguments put BEFORE the game's, so a fake engine can be a script host.
if ($env:HOL_PLAY_PREARGS) { $GameArgs = @($env:HOL_PLAY_PREARGS) + $GameArgs }
if ($DryRun) { Write-Host ("play: " + $Editor + " " + ($GameArgs -join " ")); exit 0 }

if (-not $Hidden) {
    Start-Process -FilePath $Editor -ArgumentList $GameArgs
    exit 0
}
$Proc = Start-Process -FilePath $Editor -ArgumentList $GameArgs -WindowStyle Hidden -PassThru
if ($NoWait) { exit 0 }

# Wait for the game and every process it started (even one whose parent already left: the set of seen PIDs only grows),
# at most -TimeoutSec, then close them all, deepest first. Never touches a process this launch did not start.
$Seen = New-Object 'System.Collections.Generic.HashSet[int]'
[void]$Seen.Add([int]$Proc.Id)
function Update-Seen {
    $Snap = @(Get-CimInstance Win32_Process -Property ProcessId,ParentProcessId)
    $Kids = @{}
    foreach ($P in $Snap) { $K = [int]$P.ParentProcessId; if (-not $Kids.ContainsKey($K)) { $Kids[$K] = @() }; $Kids[$K] += [int]$P.ProcessId }
    $Queue = New-Object 'System.Collections.Generic.Queue[int]'
    foreach ($Id in @($Seen)) { $Queue.Enqueue($Id) }
    while ($Queue.Count -gt 0) {
        $Cur = $Queue.Dequeue()
        if ($Kids.ContainsKey($Cur)) { foreach ($C in $Kids[$Cur]) { if ($Seen.Add($C)) { $Queue.Enqueue($C) } } }
    }
    return $Snap
}
function Get-Alive { $Live = @(Get-Process -Id @($Seen) -ErrorAction SilentlyContinue); return $Live }
$Deadline = (Get-Date).AddSeconds($TimeoutSec)
try {
    while ((Get-Date) -lt $Deadline) {
        [void](Update-Seen)
        if ((Get-Alive).Count -eq 0) { break }
        Start-Sleep -Milliseconds 300
    }
} finally {
    [void](Update-Seen)
    $Left = @(Get-Alive)
    if ($Left.Count -gt 0) {
        Write-Host ("play: closing {0} process(es) of the hidden game ({1})" -f $Left.Count, (($Left | ForEach-Object { $_.ProcessName + ":" + $_.Id }) -join ", "))
        foreach ($P in ($Left | Sort-Object Id -Descending)) { & taskkill /PID $P.Id /F 2>&1 | Out-Null }
    }
}
exit 0
