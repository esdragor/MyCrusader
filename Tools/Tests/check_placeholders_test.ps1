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
    Set-Content -Path (Join-Path $Dir "ArtVisuals.csv") -Value $ManifestHeader -Encoding UTF8
    return $Dir
}
# Runs the checker on a case folder; -Manifest overrides the manifest path (a missing file = "no manifest").
function Invoke-Tool([string] $Dir, [string] $Manifest = "") {
    if ($Manifest -eq "") { $Manifest = Join-Path $Dir "ArtVisuals.csv" }
    $Out = @(powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -DataDir $Dir -MapFile (Join-Path $Dir "placeholders_map.csv") -PlaceholdersFile (Join-Path $Dir "Placeholders.txt") -ManifestFile $Manifest 2>&1)
    return [pscustomobject]@{ Code = $LASTEXITCODE; Text = ($Out -join "`n") }
}
# Art manifest (ArtistesInProgress AR0-b, D924): one row per (Type, Faction).
$ManifestHeader = "Type,Faction,Family,Asset,Fields,Status"
function Add-Art([string] $Dir, [string] $Type, [string] $Faction, [string] $Family, [string] $Status) {
    $Line = "$Type,$Faction,$Family,/Game/Art/$Family/$Type/DA_Art_$Type,Mesh;Scale,$Status"
    Add-Content -Path (Join-Path $Dir "ArtVisuals.csv") -Value $Line -Encoding UTF8
}
# Removes a type from the map and its line(s) from Placeholders.txt (what a finished art pass does).
function Remove-Listing([string] $Dir, [string] $Type) {
    $M = Join-Path $Dir "placeholders_map.csv"
    $LinePattern = (@(Import-Csv $M -Encoding UTF8 | Where-Object { $_.Type -eq $Type }) | Select-Object -First 1).Pattern
    Set-Content -Path $M -Value @(Get-Content $M -Encoding UTF8 | Where-Object { $_ -notlike "$Type,*" }) -Encoding UTF8
    $P = Join-Path $Dir "Placeholders.txt"
    Set-Content -Path $P -Value @(Get-Content $P -Encoding UTF8 | Where-Object { $_ -notmatch $LinePattern }) -Encoding UTF8
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
Set-Content -Path $P -Value @(Get-Content $P -Encoding UTF8 | Where-Object { $_ -notmatch "^- Lancier L.gion" }) -Encoding UTF8
$R = Invoke-Tool $D
$Checks["Spearman's line deleted from Placeholders.txt: FAIL naming Spearman"] = ($R.Code -ne 0 -and $R.Text -match "Spearman")

# 7. A map entry whose type no longer exists in the tables: FAIL, named (the converse).
$D = New-Case "type_gone"
Add-Content -Path (Join-Path $D "placeholders_map.csv") -Value "GhostType,UnitTypes,Paysan Aube" -Encoding UTF8
$R = Invoke-Tool $D
$Checks["map entry for a type gone from the tables: FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "GhostType")

# 8. AR0-d2: the CSV Mesh no longer decides. A new building copied from a row with a real CSV Mesh, but no Final DA_Art
# in the manifest, is provisional: no entry = FAIL naming it.
$D = New-Case "real_mesh"; Add-Row $D "BuildingTypes" "MainCentre" "FakeForum"; $R = Invoke-Tool $D
$Checks["new building with a CSV Mesh but no Final DA_Art, no entry: FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "FakeForum")
# 8b. ...and once its DA_Art is Final with fields set, it needs no line: PASS.
Add-Art $D "FakeForum" "" "Buildings" "Final"; $R = Invoke-Tool $D
$Checks["same building once its DA_Art is Final with fields: PASS"] = ($R.Code -eq 0)

# 9. quick_check calls the checker (static) so it runs at every commit.
$Quick = Get-Content (Join-Path $Root "Tools\Tests\quick_check.ps1") -Raw
$Checks["quick_check.ps1 calls check_placeholders.ps1"] = ($Quick -match "check_placeholders\.ps1")

# --- Art manifest Tools\Data\ArtVisuals.csv (AR0-b, D924) ---
# 10. The real manifest exists with exactly the agreed header (no asset exists yet: header only is allowed).
$RealManifest = Join-Path $Data "ArtVisuals.csv"
$Checks["Tools\Data\ArtVisuals.csv exists, header '$ManifestHeader'"] = ((Test-Path $RealManifest) -and ((@(Get-Content $RealManifest -Encoding UTF8))[0].Trim() -eq $ManifestHeader))

# 11. A type marked Final that still has its line in Placeholders.txt: FAIL naming it.
$D = New-Case "final_still_listed"; Add-Art $D "Spearman" "Aube" "Units" "Final"; $R = Invoke-Tool $D
$Checks["Final type still listed in Placeholders.txt: FAIL naming Spearman"] = ($R.Code -ne 0 -and $R.Text -match "\[KO\].*Spearman.*Final")

# 12. A Final type is no longer required as provisional, although its CSV Mesh is still the mannequin: PASS.
$D = New-Case "final_unlisted"; Add-Art $D "Spearman" "Aube" "Units" "Final"; Remove-Listing $D "Spearman"
$R = Invoke-Tool $D
$Checks["Final type with mannequin Mesh, no map entry, no line: PASS"] = ($R.Code -eq 0)

# 12b. Same with an empty Mesh (a building).
$D = New-Case "final_empty_mesh"; Add-Row $D "BuildingTypes" "House" "FakeHouse"; Add-Art $D "FakeHouse" "Aube" "Buildings" "Final"
$R = Invoke-Tool $D
$Checks["Final building with empty Mesh, no map entry: PASS"] = ($R.Code -eq 0)

# 13. Provisoire = current behaviour: the line is still required.
$D = New-Case "provisional_unlisted"; Add-Art $D "Spearman" "Aube" "Units" "Provisoire"; Remove-Listing $D "Spearman"
$R = Invoke-Tool $D
$Checks["Provisoire type with no line: FAIL naming Spearman"] = ($R.Code -ne 0 -and $R.Text -match "Spearman")

# 14. A type Final for one faction but Provisoire for another stays provisional (its line is still required).
$D = New-Case "final_one_faction"; Add-Art $D "Spearman" "Aube" "Units" "Final"; Add-Art $D "Spearman" "Croc" "Units" "Provisoire"
Remove-Listing $D "Spearman"; $R = Invoke-Tool $D
$Checks["Final for one faction only, no line: FAIL naming Spearman"] = ($R.Code -ne 0 -and $R.Text -match "Spearman")

# 15. Manifest absent or header only = current behaviour (real data passes, a new mannequin unit still fails).
$D = New-Case "manifest_absent"; $R = Invoke-Tool $D (Join-Path $D "none.csv")
$Checks["manifest absent: real data PASS"] = ($R.Code -eq 0)
Add-Row $D "UnitTypes" "Spearman" "FakeUnit"; $R = Invoke-Tool $D (Join-Path $D "none.csv")
$Checks["manifest absent: new mannequin unit FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "FakeUnit")
$R = Invoke-Tool $D
$Checks["manifest header only: new mannequin unit FAIL naming it"] = ($R.Code -ne 0 -and $R.Text -match "FakeUnit")

# 16. AR0-d2: a DA_Art marked Final that sets no field (empty asset = neutral fallback) is still provisional.
$D = New-Case "final_no_field"; Remove-Listing $D "Spearman"
Add-Content -Path (Join-Path $D "ArtVisuals.csv") -Value "Spearman,,Units,/Game/Art/Units/Spearman/DA_Art_Spearman,,Final" -Encoding UTF8
$R = Invoke-Tool $D
$Checks["Final DA_Art with no field set, no line: FAIL naming Spearman"] = ($R.Code -ne 0 -and $R.Text -match "Spearman")

Remove-Item $Work -Recurse -Force
$Failed = @($Checks.Keys | Where-Object { -not $Checks[$_] })
foreach ($Name in $Checks.Keys) { Write-Host "$(if ($Checks[$Name]) { 'ok ' } else { 'KO ' }) $Name" }
Write-Host "check_placeholders_test : $(if ($Failed.Count -eq 0) { 'PASS' } else { 'FAIL' }) ($($Checks.Count - $Failed.Count)/$($Checks.Count))"
if ($Failed.Count -gt 0) { exit 1 }
exit 0
