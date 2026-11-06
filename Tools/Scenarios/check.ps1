# Runs a scenario (Tools/Scenarios/<name>.txt) without a window and says whether it reaches "Scenario <name>: ready" with no
# error: the check that a scenario is testable before it is listed for the user (Auto_Journey_Retours.md, section 1), and the
# test of a chain written as a scenario (every waitlog must come). Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\check.ps1 <name> [-Root <worktree>] [-TimeoutSeconds 240] [-Keys 3]
# Prints PASS / FAIL, the scenario's own lines and its errors; the full log is Saved_Tests\Logs\scenario_<name>.log.
# A step "waitkey" is passed by itself every -Keys seconds is NOT done here: a scenario with waitkey steps stops there (FAIL
# "waiting for a key"), such a one is checked up to its first key.
param(
    [Parameter(Position = 0, Mandatory = $true)] [string] $Name,
    [string] $Root = "",
    [string] $EngineDir = "",
    [int] $TimeoutSeconds = 240
)

if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
if (-not (Test-Path (Join-Path $Root "Tools\Scenarios\$Name.txt"))) { Write-Host "check: no scenario '$Name'"; exit 1 }
$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "check: engine '$Association' not found in the registry; pass -EngineDir"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
. (Join-Path $PSScriptRoot "..\Tests\TestSettings.ps1")
. (Join-Path $PSScriptRoot "EditorSlot.ps1")
# Edge scrolling off in the test settings (bEdgeScroll): a cursor at the screen's edge would pan the camera during the check.
Set-TestEdgeScrollOff (Join-Path $Root "Saved_Tests")
$Log = Join-Path $Root "Saved_Tests\Logs\scenario_$Name.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
# The log of THIS pass only (2026-11-03: a log still held by the previous pass's process could not be removed, the loop below
# read its old "ready" line: three stale PASS). A log that cannot be removed is a FAIL, never read; preflight.ps1 reads this
# same path, so no other name is taken.
$Removed = $false
for ($Try = 0; $Try -lt 10 -and -not $Removed; $Try++) {
    if (-not (Test-Path $Log)) { $Removed = $true; break }
    try { Remove-Item $Log -Force -ErrorAction Stop; $Removed = $true } catch { Start-Sleep -Milliseconds 500 }
}
if (-not $Removed) {
    Write-Host "check $Name : FAIL (the log $Log of an earlier pass is held by another process and cannot be removed: a game of this worktree still running? Stop it, then run again; nothing was read from it)"
    exit 1
}
# One of the 2 editor slots of the machine (EditorSlot.ps1, L7-3): wait for our turn instead of piling one more editor on the others.
if (-not (Enter-EditorSlot -Name $Name -Root $Root -Max 2 -WaitSeconds 1500)) { Write-Host "check $Name : FAIL (no editor slot after 25 min: 2 editors already run on this machine)"; exit 1 }
$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Arguments = "`"$UProject`" -game -RenderOffScreen -nosound -unattended -nosplash -culture=fr -LiveCoding=0 -asdebugport=0 -saveddirsuffix=Tests -abslog=`"$Log`" -ExecCmds=`"hol.Scenario $Name`""
$Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
$Start = Get-Date
$Result = "FAIL (timeout after $TimeoutSeconds s)"
while (((Get-Date) - $Start).TotalSeconds -lt $TimeoutSeconds) {
    Start-Sleep -Seconds 3
    # Never a log older than this pass's start (the engine creates it after the removal above).
    if ((Test-Path $Log) -and (Get-Item $Log).LastWriteTime -ge $Start.AddSeconds(-2)) {
        $Text = Get-Content $Log -Raw -ErrorAction SilentlyContinue
        if ($Text -match "Scenario ${Name}: ready") { $Result = "PASS"; break }
        if ($Text -match "Test sequence: timeout at step") { $Result = "FAIL (a step timed out)"; break }
        if ($Text -match "LogHOLTest: Error") { $Result = "FAIL (error)"; break }
        # A script that does not compile (a stale build, a broken .as) shows at the start of the log and the scenario never goes on: stop at the FIRST such line, do not wait for the timeout (2026-11-05).
        if ($Text -match "Angelscript: Error") { $Result = "FAIL (Angelscript: Error in the log, the build of this worktree is stale or a script does not compile: first line below)"; break }
        if ($Text -match "waitkey") { if ($Text -match "Test sequence: waiting for a key") { $Result = "FAIL (waiting for a key: a scenario for the user, checked up to its first key)"; break } }
    }
    if ($Process.HasExited) { $Result = "FAIL (the game exited)"; break }
}
Get-Process -Id $Process.Id -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Exit-EditorSlot
if (Test-Path $Log) {
    Get-Content $Log | Select-String -Pattern "Scenario |Test sequence|Production:|Technology:|LogHOLTest: Error|Error:" | Select-Object -Last 40 | ForEach-Object { $_.Line }
}
Write-Host "check $Name : $Result ($([int]((Get-Date) - $Start).TotalSeconds) s)"
if ($Result -eq "PASS") { exit 0 } else { exit 1 }

