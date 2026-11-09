# check_placeholders.ps1 (PlaceholdersInProgress P2, PQ-4 = A, 2026-11-09): the list of provisional things is kept by a
# check, not by a rule in prose. No engine, under a second. Called by Tools\Tests\quick_check.ps1 (every run).
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\check_placeholders.ps1 [-DataDir <dir>] [-MapFile <csv>] [-PlaceholdersFile <txt>]
# A type of UnitTypes / BuildingTypes / SourceTypes is PROVISIONAL when its Mesh is empty or is the engine mannequin
# (SM_Mannequin). Placeholders.txt names things in French, the tables by key, so Tools\Data\placeholders_map.csv links them
# (Type, Table, Pattern = a case-insensitive regex that must match one "- ..." line of Placeholders.txt).
# Fails (exit 1) when:
#   - a provisional type has no entry in the map                      (a new provisional nobody listed),
#   - a map entry's pattern matches no line of Placeholders.txt        (its line is missing),
#   - a map entry names a type no longer in its table                  (a type gone: clean the map and the list).
# A type with a real model (neither empty nor the mannequin) needs no line; its map entry, if any, is still checked.
param(
    [string] $DataDir = (Join-Path $PSScriptRoot "Data"),
    [string] $MapFile = (Join-Path $PSScriptRoot "Data\placeholders_map.csv"),
    [string] $PlaceholdersFile = (Join-Path $PSScriptRoot "..\Placeholders.txt")
)
$ErrorActionPreference = "Stop"
$Problems = New-Object System.Collections.Generic.List[string]

$Lines = @(Get-Content $PlaceholdersFile -Encoding UTF8 | Where-Object { $_ -match "^- " })
$Map = @(Import-Csv $MapFile -Encoding UTF8)
$Tables = "UnitTypes", "BuildingTypes", "SourceTypes"
$Provisional = 0

foreach ($Table in $Tables) {
    $Rows = @(Import-Csv (Join-Path $DataDir "$Table.csv") -Encoding UTF8)
    foreach ($Row in $Rows) {
        $Mesh = "$($Row.Mesh)".Trim()
        if ($Mesh -ne "" -and $Mesh -notmatch "SM_Mannequin") { continue }
        $Provisional++
        $Entry = @($Map | Where-Object { $_.Type -eq $Row.Name -and $_.Table -eq $Table }) | Select-Object -First 1
        if (-not $Entry) { $Problems.Add("$Table/$($Row.Name): provisional (Mesh '$Mesh') but no entry in $(Split-Path $MapFile -Leaf) and no line in Placeholders.txt"); continue }
    }
    foreach ($Entry in @($Map | Where-Object { $_.Table -eq $Table })) {
        if (-not ($Rows | Where-Object { $_.Name -eq $Entry.Type })) { $Problems.Add("$Table/$($Entry.Type): in $(Split-Path $MapFile -Leaf) but no longer in $Table.csv (type gone: remove its map entry and its line in Placeholders.txt)"); continue }
        if (-not ($Lines | Where-Object { $_ -match $Entry.Pattern })) { $Problems.Add("$Table/$($Entry.Type): no line of Placeholders.txt matches /$($Entry.Pattern)/ (add the line, or remove the type from the map if it got its final model)") }
    }
}
foreach ($Entry in @($Map | Where-Object { $Tables -notcontains $_.Table })) { $Problems.Add("$($Entry.Type): unknown table '$($Entry.Table)' in the map") }

if ($Problems.Count -eq 0) {
    Write-Host "check_placeholders : PASS ($Provisional provisional types, $($Map.Count) map entries, $($Lines.Count) lines)"
    exit 0
}
$Problems | ForEach-Object { Write-Host "  [KO] $_" }
Write-Host "check_placeholders : FAIL ($($Problems.Count) problem(s))"
exit 1
