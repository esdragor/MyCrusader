# Checks of the tool quick_check.ps1 (2026-11-15, Auto_Journey_File, Faramir section): "-Parts values" asked ALONE left before the
# known-red / REGRESSION tagging (L8-5) of every other part: a red values printed "values : FAIL" with neither "(REGRESSION ...)"
# nor a "red summary" line (the red of 3224cbd / d2c3ab6 sat in main from 2026-11-12, told nowhere).
# End to end, no engine: a throwaway repository (Tools\Tests, Tools\Data, Tools\check_placeholders.ps1, Placeholders.txt copied
# from this worktree) whose value registry Tools\Data\ValueSheet.csv is made WRONG on purpose (its first row points to a file
# that does not exist), then the real quick_check.ps1 -Parts values run in it, twice:
#   1. known_reds.txt without "values": "values : FAIL (REGRESSION ...)" and "red summary : 1 regression(s) : values", exit 1;
#   2. known_reds.txt with a "values" line: "values : FAIL (rouge connu depuis ...)" and "red summary : aucune regression", exit 1.
# Both require the real registry check (make_value_sheet.py --check) to be the one that went red ([KO] line).
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\quick_check_values_red_test.ps1 [-Path <quick_check.ps1 to judge>] [-Scratch <dir>]
# The throwaway repository goes under -Scratch (default: the temp folder), never in a real worktree, and is removed at the end.
param(
    [string] $Path = (Join-Path $PSScriptRoot "quick_check.ps1"),
    [string] $Scratch = [IO.Path]::GetTempPath()
)
$ErrorActionPreference = "Stop"
$Worktree = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$Repo = Join-Path $Scratch ("qc_values_red-" + [guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force (Join-Path $Repo "Tools\Tests"), (Join-Path $Repo "Tools\Data") | Out-Null
Copy-Item $Path (Join-Path $Repo "Tools\Tests\quick_check.ps1")
Copy-Item (Join-Path $PSScriptRoot "KnownReds.ps1") (Join-Path $Repo "Tools\Tests\KnownReds.ps1")
Copy-Item (Join-Path $Worktree "Tools\check_placeholders.ps1") (Join-Path $Repo "Tools\check_placeholders.ps1")
Copy-Item (Join-Path $Worktree "Placeholders.txt") (Join-Path $Repo "Placeholders.txt")
Get-ChildItem (Join-Path $Worktree "Tools\Data") -File | Copy-Item -Destination (Join-Path $Repo "Tools\Data")
# The wrong registry: its first row's value lives in a file that is not there.
$Registry = Join-Path $Repo "Tools\Data\ValueSheet.csv"
$Rows = @(Import-Csv $Registry -Encoding UTF8)
$Rows[0].File = "Nowhere/Missing_$($Rows[0].Id).csv"
$Rows | Export-Csv $Registry -NoTypeInformation -Encoding UTF8
$ErrorActionPreference = "Continue"

function Invoke-Values([string[]] $KnownLines) {
    Set-Content -Path (Join-Path $Repo "Tools\Tests\known_reds.txt") -Encoding ASCII -Value (@("# test | depuis | cause") + $KnownLines)
    $Out = @(powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Repo "Tools\Tests\quick_check.ps1") -Parts values 2>&1 | ForEach-Object { "$_" })
    [pscustomobject]@{ Text = ($Out -join "`n"); Code = $LASTEXITCODE }
}
$Checks = [ordered]@{}
$New = Invoke-Values @()
$Checks["regression: the wrong registry is caught by make_value_sheet.py --check ([KO] line)"] = $New.Text -match "\[KO\] make_value_sheet\.py --check"
$Checks["regression: 'values : FAIL (REGRESSION : pas dans known_reds.txt)'"] = $New.Text -match "(?m)^values : FAIL \(REGRESSION : pas dans known_reds\.txt\)"
$Checks["regression: 'red summary : 1 regression(s) : values'"] = $New.Text -match "(?m)^red summary : 1 regression\(s\) : values"
$Checks["regression: exit code 1 (got $($New.Code))"] = $New.Code -eq 1
$Known = Invoke-Values @("values | 2026-11-15 | registre faux du test")
$Checks["known red: the wrong registry is caught by make_value_sheet.py --check ([KO] line)"] = $Known.Text -match "\[KO\] make_value_sheet\.py --check"
$Checks["known red: 'values : FAIL (rouge connu depuis 2026-11-15 : registre faux du test)'"] = $Known.Text -match "(?m)^values : FAIL \(rouge connu depuis 2026-11-15 : registre faux du test\)"
$Checks["known red: 'red summary : aucune regression'"] = $Known.Text -match "(?m)^red summary : aucune regression"
$Checks["known red: exit code 1 (got $($Known.Code))"] = $Known.Code -eq 1
Remove-Item -Recurse -Force $Repo -ErrorAction SilentlyContinue

$Failed = @($Checks.Keys | Where-Object { -not $Checks[$_] })
foreach ($Name in $Checks.Keys) { Write-Host "$(if ($Checks[$Name]) { 'ok ' } else { 'KO ' }) $Name" }
if ($Failed.Count) { Write-Host "--- output (regression case):"; Write-Host $New.Text }
Write-Host "quick_check_values_red_test : $(if ($Failed.Count -eq 0) { 'PASS' } else { 'FAIL' }) ($Path)"
if ($Failed.Count -gt 0) { exit 1 }
exit 0
