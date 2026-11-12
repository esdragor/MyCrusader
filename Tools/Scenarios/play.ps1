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

# A scenario with NAMED Human seats needs the local player's name, or the user is a spectator and the scenario never starts (disconnect_1v1,
# 2026-11-11: the user's command had no -PlayerName). Without -PlayerName, the first named Human seat of the .json (LaunchCheck.ps1).
. (Join-Path $PSScriptRoot "LaunchCheck.ps1")
if (-not $PlayerName) { $PlayerName = Get-ScenarioPlayerName $Root $Name }

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
# The process table comes from a Toolhelp32 snapshot (0.13 s) and not from Get-CimInstance (1.7-3 s a pass, longer while the game
# loads): with CIM the wait overshot -TimeoutSec by 15-18 s (2026-11-11).
Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; public static class HolPlayTh { [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)] public struct PE { public uint dwSize, cntUsage, th32ProcessID; public IntPtr th32DefaultHeapID; public uint th32ModuleID, cntThreads, th32ParentProcessID; public int pcPriClassBase; public uint dwFlags; [MarshalAs(UnmanagedType.ByValTStr, SizeConst=260)] public string szExeFile; } [DllImport("kernel32.dll")] public static extern IntPtr CreateToolhelp32Snapshot(uint f, uint p); [DllImport("kernel32.dll", CharSet=CharSet.Unicode)] public static extern bool Process32First(IntPtr h, ref PE e); [DllImport("kernel32.dll", CharSet=CharSet.Unicode)] public static extern bool Process32Next(IntPtr h, ref PE e); [DllImport("kernel32.dll")] public static extern bool CloseHandle(IntPtr h); public static System.Collections.Generic.List<uint[]> Snap() { var l = new System.Collections.Generic.List<uint[]>(); IntPtr h = CreateToolhelp32Snapshot(2, 0); PE e = new PE(); e.dwSize = (uint)Marshal.SizeOf(typeof(PE)); if (Process32First(h, ref e)) { do { l.Add(new uint[]{ e.th32ProcessID, e.th32ParentProcessID }); } while (Process32Next(h, ref e)); } CloseHandle(h); return l; } }'
function Update-Seen {
    $Snap = @([HolPlayTh]::Snap())
    $Kids = @{}
    foreach ($P in $Snap) { $K = [int]$P[1]; if (-not $Kids.ContainsKey($K)) { $Kids[$K] = @() }; $Kids[$K] += [int]$P[0] }
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
$Clean = $false
$TimedOut = $false
try {
    while ($true) {
        if ((Get-Date) -ge $Deadline) { $TimedOut = $true; break }
        [void](Update-Seen)
        if ((Get-Alive).Count -eq 0) { $Clean = $true; break }
        Start-Sleep -Milliseconds 300
    }
} finally {
    if (-not $Clean) { [void](Update-Seen) }   # timeout or interrupted: find the children before closing them
    $Left = @(Get-Alive | Where-Object { -not $_.HasExited })
    if ($Left.Count -gt 0) {
        Write-Host ("play: closing {0} process(es) of the hidden game ({1})" -f $Left.Count, (($Left | ForEach-Object { $_.ProcessName + ":" + $_.Id }) -join ", "))
        # Stop-Process, not taskkill.exe: taskkill took 2-3 s per call here
        foreach ($P in ($Left | Sort-Object Id -Descending)) { Stop-Process -Id $P.Id -Force -ErrorAction SilentlyContinue }
    }
}
Write-Host ("play: hidden game ended after {0:N1} s (-TimeoutSec {1})" -f ((Get-Date) - $Deadline.AddSeconds(-$TimeoutSec)).TotalSeconds, $TimeoutSec)
exit 0
