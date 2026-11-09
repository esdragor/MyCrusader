# Test of Tools\check_placeholders.ps1 (PlaceholdersInProgress P2, PQ-4 = A, 2026-11-09). No engine, a few seconds:
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\check_placeholders_test.ps1
# Each case copies the real tables, the real map (Tools\Data\placeholders_map.csv) and the real Placeholders.txt into a
# temporary folder, breaks ONE thing, and asserts the EFFECT: the checker exits 1 and names the culprit (a provisional type
# with no line in Placeholders.txt must fail; a clean copy passes). It also proves quick_check calls the checker.
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$Tool = Join-Path $Root "Tools\check_placeholders.ps1"
$Data = Join-Path $Root "Tools\Data"
$Work = Join-Path ([IO.Path]::GetTempPath()) ("check_placeholders_test_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $Work | Out-Null
$Checks = [ordered]@{}

function New-Case([string] $Name) {
    $Dir = Join-Path $Work $Name
    New-Item -ItemType Directory -Path $Dir | Out-Null
    foreach ($T in "UnitTypes", "BuildingTypes", "SourceTypes") { Copy-Item (Join-Path $Data "$T.csv") $Dir }
    Copy-Item (Join-Path $Data "placeholders_map.csv") $Dir
    Copy-Item (Join-Path $Root "Placeholders.txt") $Dir
    return $Dir
}
function Invoke-Tool([string] $Dir) {
    $Out = @(powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -DataDir $Dir -MapFile (Join-Path $Dir "placeholders_map.csv") -PlaceholdersFile (Join-Path $Dir "Placeholders.txt") 2>&1)
    return [pscustomobject]@{ Code = $LASTEXITCODE; Text = ($Out -join "`n") }
}
# Appends a copy of the table's row `From` under the new name `Fake`.
function Add-Row([string] $Dir, [string] $Table, [string] $From, [string] $Fake) {
    $File = Join-Path $Dir "$Table.csv"
    $Lines = @(Get-Content $File -Encoding UTF8)
    $Row = $Lines | Where-Object { $_ -like "$From,*" } | Select-Object -First 1
    $Lines += ($Row -replace ("^" + [regex]::Escape($From) + ","), "$Fake,")
    Set-Content -Path $File -Value $Lines -Encoding UTF8
}

if (-not (Test-Path $Tool)) {
    Write-Host "KO  Tools\check_placeholders.ps1 exists"
    Write-Host "check_placeholders_test : FAIL (the tool is missing)"
    Remove-Item $Work -Recurse -Force
    exit 1
}

# 1. The real data passes.
$D = New-Case "clean"; $R = Invoke-Tool $D
$Checks["real tables + real list: PASS (exit 0)"] = ($R.Code -eq 0)

# 2. A new unit with the mannequin and no line anywhere: FAIL, named.
$D = New-Case "unit_unmapped"; Add-Row $D "UnitTypes" "Spearman" "FakeUnit"; $R = Invoke-Tool $D
$Checks["new mannequin unit with no map entry: FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "FakeUnit")

# 3. A new building with an empty Mesh and no entry: FAIL, named.
$D = New-Case "building_unmapped"; Add-Row $D "BuildingTypes" "House" "FakeHouse"; $R = Invoke-Tool $D
$Checks["new building with empty Mesh, no map entry: FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "FakeHouse")

# 4. A new source with an empty Mesh and no entry: FAIL, named.
$D = New-Case "source_unmapped"; Add-Row $D "SourceTypes" "Tree" "FakeTree"; $R = Invoke-Tool $D
$Checks["new source with empty Mesh, no map entry: FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "FakeTree")

# 5. The type is in the map but its line is missing from Placeholders.txt: FAIL, named.
$D = New-Case "line_missing"
Add-Row $D "UnitTypes" "Spearman" "FakeUnit"
Add-Content -Path (Join-Path $D "placeholders_map.csv") -Value "FakeUnit,UnitTypes,Fake Unit Line" -Encoding UTF8
$R = Invoke-Tool $D
$Checks["map entry whose line is not in Placeholders.txt: FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "FakeUnit")

# 5b. ...and with the line added, the same case passes.
Add-Content -Path (Join-Path $D "Placeholders.txt") -Value "- Fake Unit Line [mannequin]" -Encoding UTF8
$R = Invoke-Tool $D
$Checks["same type once its line is added: PASS"] = ($R.Code -eq 0)

# 6. The real line of a real type is deleted from Placeholders.txt: FAIL, named.
$D = New-Case "real_line_deleted"
$P = Join-Path $D "Placeholders.txt"
Set-Content -Path $P -Value @(Get-Content $P -Encoding UTF8 | Where-Object { $_ -notmatch "^- Lancier Aube" }) -Encoding UTF8
$R = Invoke-Tool $D
$Checks["Spearman's line deleted from Placeholders.txt: FAIL naming Spearman"] = ($R.Code -ne 0 -and $R.Text -match "Spearman")

# 7. A map entry whose type no longer exists in the tables: FAIL, named (the converse).
$D = New-Case "type_gone"
Add-Content -Path (Join-Path $D "placeholders_map.csv") -Value "GhostType,UnitTypes,Paysan Aube" -Encoding UTF8
$R = Invoke-Tool $D
$Checks["map entry for a type gone from the tables: FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "GhostType")

# 8. A new building with a REAL mesh (not the mannequin, not empty) needs no line: PASS.
$D = New-Case "real_mesh"; Add-Row $D "BuildingTypes" "MainCentre" "FakeForum"; $R = Invoke-Tool $D
$Checks["new building with a real model, no entry: PASS (not provisional)"] = ($R.Code -eq 0)

# 9. quick_check calls the checker (static) so it runs at every commit.
$Quick = Get-Content (Join-Path $Root "Tools\Tests\quick_check.ps1") -Raw
$Checks["quick_check.ps1 calls check_placeholders.ps1"] = ($Quick -match "check_placeholders\.ps1")

Remove-Item $Work -Recurse -Force
$Failed = @($Checks.Keys | Where-Object { -not $Checks[$_] })
foreach ($Name in $Checks.Keys) { Write-Host "$(if ($Checks[$Name]) { 'ok ' } else { 'KO ' }) $Name" }
Write-Host "check_placeholders_test : $(if ($Failed.Count -eq 0) { 'PASS' } else { 'FAIL' }) ($($Checks.Count - $Failed.Count)/$($Checks.Count))"
if ($Failed.Count -gt 0) { exit 1 }
exit 0
