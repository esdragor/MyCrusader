# Test without the editor of Tools\Art\apply_art_send.ps1 (AR8-a, D929): the art send of a fixed model (folder or zip with
# manifest.json format 1) is refused before any write when its manifest is wrong, -DryRun prints the plan and writes nothing,
# and an application (-NoEditor: the text files only, the editor import is the end-to-end check) leaves check_placeholders green:
#   - refusals: a unit type (D929: units through the packaged editor), an unknown type, a manifest with seven faults listed together;
#     the tree is unchanged after each (hashes of the text files, no ArtSource folder);
#   - -DryRun on the folder and on its zip: exit 0, the plan names SM_House, DA_Art_House and the overrides, nothing written;
#   - final (folder): the House line leaves Placeholders.txt and its map entry, ArtVisuals.csv says Final with the fields,
#     assets_art.csv gains the licence row (used_in House), the source lands under ArtSource, check_placeholders PASS;
#   - provisional (zip, a faction variant): Placeholders.txt unchanged, a new ArtVisuals.csv row, check_placeholders PASS.
# Runs in a sandbox copy of the data under %TEMP%; the worktree is never written. Exit code 0 = green, 1 = red.
param([string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path)
$ErrorActionPreference = 'Stop'
$Tool = Join-Path $Root 'Tools\Art\apply_art_send.ps1'
$fails = 0
function Check([string]$Name, [bool]$Ok, [string]$Detail = '') {
    if ($Ok) { Write-Output "ok $Name" } else { Write-Output "FAIL $Name $Detail"; $script:fails++ }
}

# Sandbox: the data the tool reads and writes, nothing else.
$Box = Join-Path $env:TEMP ('apply_art_send_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
foreach ($d in 'Tools\Data', 'Docs', 'Content\Art\Buildings\House', 'Sends') { New-Item -ItemType Directory -Force (Join-Path $Box $d) | Out-Null }
foreach ($f in 'BuildingTypes.csv', 'SourceTypes.csv', 'UnitTypes.csv', 'Factions.csv', 'ArtVisuals.csv', 'placeholders_map.csv') {
    Copy-Item (Join-Path $Root "Tools\Data\$f") (Join-Path $Box "Tools\Data\$f")
}
Copy-Item (Join-Path $Root 'Placeholders.txt') (Join-Path $Box 'Placeholders.txt')
Copy-Item (Join-Path $Root 'Docs\assets_art.csv') (Join-Path $Box 'Docs\assets_art.csv')
Copy-Item (Join-Path $Root 'Content\Art\Buildings\House\DA_Art_House.uasset') (Join-Path $Box 'Content\Art\Buildings\House\DA_Art_House.uasset')
$Watched = 'Placeholders.txt', 'Docs\assets_art.csv', 'Tools\Data\ArtVisuals.csv', 'Tools\Data\placeholders_map.csv'
function Get-State { ($Watched | ForEach-Object { (Get-FileHash (Join-Path $Box $_)).Hash }) -join ',' }
function Invoke-CheckPlaceholders {
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Root 'Tools\check_placeholders.ps1') -DataDir (Join-Path $Box 'Tools\Data') `
        -MapFile (Join-Path $Box 'Tools\Data\placeholders_map.csv') -PlaceholdersFile (Join-Path $Box 'Placeholders.txt') -ManifestFile (Join-Path $Box 'Tools\Data\ArtVisuals.csv')
    return @{ Code = $LASTEXITCODE; Text = ($out -join "`n") }
}
function New-Send([string]$Name, [hashtable]$Manifest, [string]$ModelName = 'model.glb') {
    $dir = Join-Path $Box "Sends\$Name"
    New-Item -ItemType Directory -Force $dir | Out-Null
    [IO.File]::WriteAllBytes((Join-Path $dir $ModelName), [Text.Encoding]::ASCII.GetBytes('glTF fake model for apply_art_send_test'))
    [IO.File]::WriteAllText((Join-Path $dir 'manifest.json'), ($Manifest | ConvertTo-Json -Depth 4), (New-Object Text.UTF8Encoding $false))
    return $dir
}
function Get-Manifest([string]$Type = 'House', [string]$Status = 'provisional', [string]$Faction = '') {
    return @{ format = 1; type = $Type; faction = $Faction; family = 'Buildings'; model = 'model.glb'; yaw = 90; scale = 0.5; status = $Status
        author = 'Test'; notes = ''; made_with = 'test'
        licence = @{ file_or_pack = 'house_test.glb'; source = 'Test artist'; licence = 'CC0'; url = ''; downloaded_on = '2026-11-27'; licence_proof = 'test' } }
}
function Invoke-Tool([string[]]$ToolArgs) {
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool @ToolArgs -Root $Box
    return @{ Code = $LASTEXITCODE; Text = ($out -join "`n") }
}

$base = Invoke-CheckPlaceholders
Check 'baseline check_placeholders PASS in the sandbox' ($base.Code -eq 0) $base.Text
$state = Get-State

# 1. Refusals before any write.
$unit = New-Send 'Archer_unit' (Get-Manifest 'Archer')
$r = Invoke-Tool @('-From', $unit)
Check 'unit type refused (exit 1)' ($r.Code -eq 1) "code $($r.Code): $($r.Text)"
Check 'unit refusal names D929' ($r.Text -match 'D929') $r.Text
$r = Invoke-Tool @('-From', (New-Send 'Castle' (Get-Manifest 'Castle')))
Check 'unknown type refused (exit 1)' ($r.Code -eq 1 -and $r.Text -match 'Castle') "code $($r.Code): $($r.Text)"
$bad = Get-Manifest 'House' 'maybe' 'Elves'
$bad.format = 2; $bad.family = 'Sources'; $bad.model = 'model.fbx'; $bad.scale = 0; $bad.licence.source = ''
$r = Invoke-Tool @('-From', (New-Send 'House_bad' $bad 'model.fbx'))
$refusals = @($r.Text -split "`n" | Where-Object { $_ -match '^\s*REFUS ' })
Check 'seven faults refused together (exit 1)' ($r.Code -eq 1 -and $refusals.Count -eq 7) "code $($r.Code), $($refusals.Count) REFUS lines: $($r.Text)"
foreach ($word in 'format', 'family', 'faction', 'model', 'licence', 'scale', 'status') {
    Check "refusal lists $word" ([bool]($refusals | Where-Object { $_ -match $word })) $r.Text
}
Check 'refusals wrote nothing' ((Get-State) -eq $state -and -not (Test-Path (Join-Path $Box 'ArtSource'))) 'a text file changed or ArtSource exists'

# 2. -DryRun on the folder and on its zip.
$good = New-Send 'House_20261127-0900' (Get-Manifest 'House' 'final')
$zip = Join-Path $Box 'Sends\House_20261127-0900.zip'
Compress-Archive -Path $good -DestinationPath $zip -Force
foreach ($case in @(@{ Name = 'folder'; From = $good }, @{ Name = 'zip'; From = $zip })) {
    $r = Invoke-Tool @('-From', $case.From, '-DryRun')
    Check "DryRun $($case.Name) exit 0" ($r.Code -eq 0) "code $($r.Code): $($r.Text)"
    Check "DryRun $($case.Name) plan names SM_House" ($r.Text -match '/Game/Art/Buildings/House/SM_House\b') $r.Text
    Check "DryRun $($case.Name) plan names DA_Art_House and the overrides" ($r.Text -match 'DA_Art_House' -and $r.Text -match 'MeshYawOffset = 90' -and $r.Text -match 'MeshScale = 0.5') $r.Text
    Check "DryRun $($case.Name) plan removes the Maison line (final)" ($r.Text -match 'Placeholders\.txt.*- Maison') $r.Text
    Check "DryRun $($case.Name) wrote nothing" ((Get-State) -eq $state -and -not (Test-Path (Join-Path $Box 'ArtSource'))) 'a text file changed or ArtSource exists'
}

# 3. Final, folder, text files only.
$r = Invoke-Tool @('-From', $good, '-NoEditor')
Check 'final apply exit 0' ($r.Code -eq 0) "code $($r.Code): $($r.Text)"
$lines = Get-Content (Join-Path $Box 'Placeholders.txt') -Encoding UTF8
Check 'final: Maison line removed from Placeholders.txt' (-not ($lines | Where-Object { $_ -match '^- Maison' })) ''
Check 'final: other lines kept' ((@($lines).Count) -gt 10) "$(@($lines).Count) lines left"
Check 'final: House map entry removed' (-not (Import-Csv (Join-Path $Box 'Tools\Data\placeholders_map.csv') | Where-Object { $_.Type -eq 'House' })) ''
$row = Import-Csv (Join-Path $Box 'Tools\Data\ArtVisuals.csv') | Where-Object { $_.Type -eq 'House' -and $_.Faction -eq '' }
Check 'final: ArtVisuals House Final with Mesh;MeshYawOffset;MeshScale' ($row.Status -eq 'Final' -and $row.Fields -eq 'Mesh;MeshYawOffset;MeshScale') "$($row.Status) '$($row.Fields)'"
$licence = Import-Csv (Join-Path $Box 'Docs\assets_art.csv') -Encoding UTF8 | Where-Object { $_.file_or_pack -eq 'house_test.glb' }
Check 'final: assets_art.csv row used_in House' ($licence.used_in -eq 'House' -and $licence.licence -eq 'CC0') "$($licence | Out-String)"
Check 'final: source copied under ArtSource\Buildings\House' (Test-Path (Join-Path $Box 'ArtSource\Buildings\House\House_20261127-0900\model.glb')) ''
$c = Invoke-CheckPlaceholders
Check 'final: check_placeholders PASS' ($c.Code -eq 0) $c.Text

# 4. Provisional, zip, faction variant: Placeholders.txt unchanged.
$before = (Get-FileHash (Join-Path $Box 'Placeholders.txt')).Hash
$dawn = New-Send 'Barracks_Dawn_20261127-0910' (Get-Manifest 'Barracks' 'provisional' 'Dawn')
$zip2 = Join-Path $Box 'Sends\Barracks_Dawn_20261127-0910.zip'
Compress-Archive -Path (Join-Path $dawn '*') -DestinationPath $zip2 -Force
$r = Invoke-Tool @('-From', $zip2, '-NoEditor')
Check 'provisional zip apply exit 0' ($r.Code -eq 0) "code $($r.Code): $($r.Text)"
Check 'provisional: Placeholders.txt unchanged' ((Get-FileHash (Join-Path $Box 'Placeholders.txt')).Hash -eq $before) ''
$row = Import-Csv (Join-Path $Box 'Tools\Data\ArtVisuals.csv') | Where-Object { $_.Type -eq 'Barracks' -and $_.Faction -eq 'Dawn' }
Check 'provisional: ArtVisuals Barracks,Dawn row Provisoire with DA_Art_Barracks_Dawn' ($row.Status -eq 'Provisoire' -and $row.Asset -eq '/Game/Art/Buildings/Barracks/DA_Art_Barracks_Dawn') "$($row | Out-String)"
$c = Invoke-CheckPlaceholders
Check 'provisional: check_placeholders PASS' ($c.Code -eq 0) $c.Text

Remove-Item -Recurse -Force $Box -ErrorAction SilentlyContinue
if ($fails -gt 0) { Write-Output "apply_art_send_test: RED ($fails failures)"; exit 1 }
Write-Output 'apply_art_send_test: GREEN'
exit 0
