# build_wait.ps1 (L15-6) : ONE call that waits its turn in the queue of Tools\build.ps1 and hands back a SHORT result.
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\build_wait.ps1 <worktree> [-ResultFile <file>]
#         [-BuildScript <build.ps1>] [extra build.ps1 arguments, e.g. -Priority merge]
# It runs `build.ps1 -Root <worktree> -Wait` (the blocking form: queue, compile, nothing to poll), then writes and prints
# 3 lines : PASS | FAIL | BUSY | SUPERSEDED | ORPHAN | TIMEOUT, the duration (queue wait + compile), the last error line.
# Result file (default <worktree>\Saved\Logs\build_wait_result.txt, the previous one is deleted first, so a stale PASS is never read).
# Exit code = the exit code of build.ps1 (0 PASS, 1 FAIL, 2 BUSY, 3 SUPERSEDED, 4 ORPHAN, 5 TIMEOUT), 1 if the worktree is missing.
# Never use Wait-Process / Start-Sleep loops around a build : this call IS the wait. Full log : <worktree>\Saved\Logs\build.log.
# Test : Tools\AutoJourney\build_wait_test.ps1 (no real build).
param([Parameter(Position = 0)] [string] $Worktree = "", [string] $ResultFile = "", [string] $BuildScript = "",
      [Parameter(ValueFromRemainingArguments = $true)] [string[]] $BuildArgs)
$ErrorActionPreference = "Continue"
$Names = @{ 0 = "PASS"; 1 = "FAIL"; 2 = "BUSY"; 3 = "SUPERSEDED"; 4 = "ORPHAN"; 5 = "TIMEOUT" }

if (-not $Worktree -or -not (Test-Path -LiteralPath $Worktree -PathType Container)) {
    Write-Output "FAIL"
    Write-Output "worktree: '$Worktree' not found, nothing built"
    exit 1
}
$Root = (Resolve-Path -LiteralPath $Worktree).Path
if (-not $ResultFile) { $ResultFile = Join-Path $Root "Saved\Logs\build_wait_result.txt" }
New-Item -ItemType Directory -Force (Split-Path -Parent $ResultFile) | Out-Null
Remove-Item -LiteralPath $ResultFile -Force -ErrorAction SilentlyContinue
if (-not $BuildScript) { $BuildScript = Join-Path (Split-Path -Parent $PSScriptRoot) "build.ps1" }

$Clock = [Diagnostics.Stopwatch]::StartNew()
$Lines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $BuildScript -Root $Root -Wait @BuildArgs 2>&1 | ForEach-Object { "$_" })
$Code = $LASTEXITCODE
$Seconds = [int][math]::Round($Clock.Elapsed.TotalSeconds)

$Name = if ($Names.ContainsKey([int]$Code)) { $Names[[int]$Code] } else { "FAIL" }
if ($Code -eq 0) { $Last = "none" }
else {
    $Errors = @($Lines | Where-Object { $_ -match 'error|Live Coding|conflicting instance' -and $_ -notmatch '^\s*build : (FAIL|PASS)\b' })
    $Pick = if ($Errors.Count) { $Errors[-1] } else { @($Lines | Where-Object { $_.Trim() })[-1] }
    $Last = if ($Pick) { $Pick.Trim() } else { "no output from build.ps1 (exit $Code)" }
    if ($Last.Length -gt 300) { $Last = $Last.Substring(0, 300) }
}
$Text = "$Name`r`nworktree: $(Split-Path -Leaf $Root), duration: $Seconds s (queue + build), build.ps1 exit $Code`r`nlast error: $Last`r`n"
[IO.File]::WriteAllText($ResultFile, $Text, (New-Object Text.UTF8Encoding $false))
Write-Output $Text.TrimEnd()
Write-Output "result file: $ResultFile"
exit $Code
