# Test of Tools\AutoJourney\ab_matrix.ps1 (L9-7, 2026-11-08) : configs x scenarios x N -> one "PASS k/N" line per config/scenario in a file.
# No game and no engine : a FAKE repeat_scenario (written here, passed by -RepeatTool) prints canned "repeat <name> : PASS k/N" lines, so the
# real parsing, the table, the N >= 8 rule and the file are tested end to end. The REAL repeat_scenario summary format is covered by
# repeat_scenario_validity_test.ps1.
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\ab_matrix_test.ps1
# Prints one PASS / FAIL line per case, then "ab_matrix_test : PASS" or "FAIL (n)"; exit = number of red cases.
$ErrorActionPreference = "Stop"
$Tool = Join-Path $PSScriptRoot "..\AutoJourney\ab_matrix.ps1"
$Work = Join-Path ([IO.Path]::GetTempPath()) ("hol_ab_matrix_" + [Guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force $Work | Out-Null
$Fails = 0
function Case([string] $Name, [bool] $Ok, [string] $Detail = "") {
    Write-Host "  $(if ($Ok) { 'PASS' } else { 'FAIL' }) : $Name$(if (-not $Ok -and $Detail) { " ($Detail)" })"
    if (-not $Ok) { $script:Fails++ }
}

# The fake repeat tool : the number of passed runs depends on the scenario name (suffix _s120 -> 3, _s150 -> all, else 5) ; "nores" prints nothing.
$Fake = Join-Path $Work "fake_repeat.ps1"
Set-Content -Encoding ascii $Fake @'
param([Parameter(Position = 0)] [string] $Name = "", [int] $Runs = 12, [string] $Root = "")
if ($Name -match '^nores') { exit 3 }
$K = if ($Name -match '_s120$') { 3 } elseif ($Name -match '_s150$') { $Runs } else { 5 }
Write-Host "  pass 1/${Runs} : check ${Name} : PASS (3 s)"
Write-Host ("repeat $Name : PASS $K/$Runs (20 / 21 / 25 s), verdict " + $(if ($K -eq $Runs) { 'OK' } else { 'RED' }))
exit ($Runs - $K)
'@
$Out = Join-Path $Work "matrix.txt"
function Run-Tool([string[]] $ToolArgs) {
    $ErrorActionPreference = "Continue"
    $Lines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool @ToolArgs -RepeatTool $Fake -Root $Work 2>&1 | ForEach-Object { "$_" })
    return [pscustomobject]@{ Exit = $LASTEXITCODE; Lines = $Lines; Text = ($Lines -join "`n") }
}

try {
    # ---- (1) 3 configs x 2 scenarios + one scenario without result, N = 8 : one line per config/scenario in the file.
    $R = Run-Tool @("-Configs", "s120=_s120,s150=_s150,default=", "-Scenarios", "melee_stuck,diplo", "-N", "8", "-Out", $Out)
    $T = if (Test-Path $Out) { Get-Content $Out -Raw } else { "" }
    Case "the table file was written (exit $($R.Exit))" ($T.Length -gt 0) ($R.Text)
    Case "s120 / melee_stuck : 'PASS 3/8', verdict RED" ($T -match "(?m)^s120 \| melee_stuck \| PASS 3/8 \| verdict RED")
    Case "s150 / melee_stuck : 'PASS 8/8', verdict OK" ($T -match "(?m)^s150 \| melee_stuck \| PASS 8/8 \| verdict OK")
    Case "default / diplo : 'PASS 5/8', verdict RED" ($T -match "(?m)^default \| diplo \| PASS 5/8 \| verdict RED")
    Case "the spread of the cell is kept" ($T -match "(?m)^default \| diplo \| .*20 / 21 / 25 s")
    $Data = @($T -split "`r?`n" | Where-Object { $_ -match '^(s120|s150|default) \| ' })
    Case "exactly one line per config/scenario (6, got $($Data.Count))" ($Data.Count -eq 6)
    Case "the header names N = 8" ($T -match "N = 8")
    Case "the tool prints the table too" ($R.Text -match "s120 \| melee_stuck \| PASS 3/8")

    # ---- (2) a scenario that gives no result line : its own line, never a PASS k/N.
    Remove-Item $Out -Force -ErrorAction SilentlyContinue
    $R = Run-Tool @("-Configs", "default=", "-Scenarios", "nores,diplo", "-N", "8", "-Out", $Out)
    $T = Get-Content $Out -Raw
    Case "no result line : 'NO RESULT', not a PASS k/N" ($T -match "(?m)^default \| nores \| NO RESULT" -and -not [bool]($T -match "nores \| PASS"))
    Case "no result line : the next cell still ran" ($T -match "(?m)^default \| diplo \| PASS 5/8")

    # ---- (3) N below 8 is refused (no conclusion from 4), unless -Exploratory, and then the file says so.
    Remove-Item $Out -Force -ErrorAction SilentlyContinue
    $R = Run-Tool @("-Configs", "default=", "-Scenarios", "diplo", "-N", "4", "-Out", $Out)
    Case "N = 4 without -Exploratory : refused (exit 2, no file)" ($R.Exit -eq 2 -and -not (Test-Path $Out)) "exit $($R.Exit)"
    $R = Run-Tool @("-Configs", "default=", "-Scenarios", "diplo", "-N", "2", "-Exploratory", "-Out", $Out)
    $T = if (Test-Path $Out) { Get-Content $Out -Raw } else { "" }
    Case "N = 2 -Exploratory : runs and the file says 'NOT A CONCLUSION'" ($T -match "NOT A CONCLUSION" -and $T -match "(?m)^default \| diplo \| PASS 5/2")
} finally {
    Remove-Item $Work -Recurse -Force -ErrorAction SilentlyContinue
}
Write-Host ("ab_matrix_test : " + $(if ($Fails -eq 0) { "PASS" } else { "FAIL ($Fails)" }))
exit $Fails
