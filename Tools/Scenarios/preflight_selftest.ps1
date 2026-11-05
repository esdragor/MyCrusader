# Self-test of preflight.ps1 / check.ps1: a script with a compile error must make the pre-flight FAIL at the FIRST "Angelscript: Error"
# line of the log, not after the whole scenario (a stale build used to cost the full timeout). Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\preflight_selftest.ps1 [-Root <worktree>]
# It writes a broken script (Script\Tests\ZZPreflightBroken.as) in THIS worktree, runs preflight.ps1 on the fixture scenario
# zz_preflight_selftest (a 90 s quiet scenario), and ALWAYS deletes the broken script. PASS = the pre-flight FAILs in under 60 s
# and its output holds the "Angelscript: Error" line. FAIL (exit 1) otherwise.
param([string] $Root = "")

if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Broken = Join-Path $Root "Script\Tests\ZZPreflightBroken.as"
$Out = Join-Path ([IO.Path]::GetTempPath()) "preflight_selftest_out.txt"
New-Item -ItemType Directory -Force (Split-Path $Broken) | Out-Null
Set-Content -Path $Broken -Encoding UTF8 -Value "void ZZPreflightBrokenFunction() { this is not valid angelscript }"
try {
    $Start = Get-Date
    & powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "preflight.ps1") zz_preflight_selftest -Root $Root *> $Out
    $PreflightExit = $LASTEXITCODE
    $Wall = [int]((Get-Date) - $Start).TotalSeconds
} finally {
    Remove-Item $Broken -Force -ErrorAction SilentlyContinue
}
$Text = Get-Content $Out -Raw
$HasLine = $Text -match "Angelscript: Error"
Write-Host "preflight exit $PreflightExit after $Wall s, 'Angelscript: Error' in its output: $HasLine"
if ($PreflightExit -ne 0 -and $HasLine -and $Wall -lt 60) { Write-Host "preflight_selftest : PASS"; exit 0 }
Write-Host "preflight_selftest : FAIL (expected: preflight FAIL, the error line shown, in under 60 s)"
exit 1
