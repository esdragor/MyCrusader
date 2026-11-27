# apply_art_send.ps1 (AR8-a, D929): applies an artist's "send" of a FIXED model (building or resource, glTF / glb) to a tree.
# A send = a folder (or its zip) with manifest.json (format 1, ArtistesInProgress.md "Conception AR8"), the model (.glb, or .gltf
# and its files), preview.png optional. Usage (Docs\ARTISTES.md, "Envoi d'un modele fixe"):
#   powershell -ExecutionPolicy Bypass -File Tools\Art\apply_art_send.ps1 -From <zip|folder> [-Root <tree>] [-DryRun] [-NoEditor] [-Engine <dir>]
# 1. Refusals, ALL listed together BEFORE any write (FAIL, exit 1): unknown format; type absent from BuildingTypes.csv /
#    SourceTypes.csv; unit type (units go through the packaged editor, D929); family not the table of the type; unknown faction;
#    model absent or not .glb / .gltf; incomplete licence (file_or_pack, source, licence); yaw not a number; scale <= 0;
#    status neither final nor provisional.
# 2. -DryRun prints the plan (files and fields that would change) and writes nothing.
# 3. Application in -Root (default: the tree of this script; the integration tree, never the main copy by default):
#    - the send folder is copied to ArtSource\<Family>\<Type>\<send name>\ (Git LFS, D926, .gitattributes);
#    - the hidden editor (UnrealEditor-Cmd -nullrhi, Tools\Editor\apply_art_send.py) imports the model as
#      /Game/Art/<Family>/<Type>/SM_<Type>[_<Faction>] and writes DA_Art_<Type>[_<Faction>] (created when missing, TypeName /
#      Faction): Mesh, bOverrideMeshYawOffset + MeshYawOffset = yaw, bOverrideMeshScale + MeshScale = scale, Status
#      (final = Final, provisional = Placeholder); -NoEditor skips this step (tests: text files only);
#    - Tools\Data\ArtVisuals.csv (art manifest read by check_placeholders.ps1): the row of the asset gets the fields and the status;
#    - Docs\assets_art.csv: the licence row (used_in = the type);
#    - final: once every art row of the type is Final, its line leaves Placeholders.txt and its entry leaves
#      Tools\Data\placeholders_map.csv (check_placeholders.ps1 stays green); provisional: neither file changes.
# Exit: 0 = applied (or plan printed), 1 = refused (nothing written), 2 = the editor step failed (text files untouched).
# Test: Tools\Art\apply_art_send_test.ps1.
param(
    [Parameter(Mandatory = $true)][string]$From,
    # Default: the tree of this script (computed below: $PSScriptRoot is empty in a param default under powershell -File, 5.1).
    [string]$Root = '',
    [switch]$DryRun,
    [switch]$NoEditor,
    [string]$Engine = 'C:\UE\UE_5.8_HOL'
)
$ErrorActionPreference = 'Stop'
$Inv = [Globalization.CultureInfo]::InvariantCulture
if (-not $Root) { $Root = Join-Path (Split-Path -Parent $MyInvocation.MyCommand.Path) '..\..' }
$Root = (Resolve-Path -LiteralPath $Root).Path.TrimEnd('\')
$Data = Join-Path $Root 'Tools\Data'
$Refusals = New-Object System.Collections.Generic.List[string]
function Refuse([string]$Text) { $Refusals.Add($Text) }
function Fail-Refused {
    $Refusals | ForEach-Object { Write-Output "  REFUS $_" }
    Write-Output "apply_art_send : FAIL ($($Refusals.Count) refus, rien n'est ecrit) : $From"
    exit 1
}
function Format-Number($Value) { return ([double]$Value).ToString('0.######', $Inv) }

# --- The send: folder, or zip extracted to %TEMP% (outside the tree) ---------------------------------------------------------
$Work = $null
if (-not (Test-Path -LiteralPath $From)) { Refuse "envoi introuvable : $From"; Fail-Refused }
$From = (Resolve-Path -LiteralPath $From).Path
$SendName = [IO.Path]::GetFileNameWithoutExtension($From.TrimEnd('\'))
if ((Get-Item -LiteralPath $From).PSIsContainer) {
    $Search = $From
} elseif ([IO.Path]::GetExtension($From) -ieq '.zip') {
    $Work = Join-Path $env:TEMP ('hol_art_send_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
    Expand-Archive -LiteralPath $From -DestinationPath $Work -Force
    $Search = $Work
} else { Refuse "envoi ni dossier ni .zip : $From"; Fail-Refused }
$Manifests = @(Get-ChildItem -LiteralPath $Search -Recurse -Filter 'manifest.json' -File)
if ($Manifests.Count -ne 1) { Refuse "manifest.json : $($Manifests.Count) trouve(s) dans l'envoi (il en faut un)"; Fail-Refused }
$SendDir = $Manifests[0].DirectoryName
if ($SendDir.TrimEnd('\') -ne $Search.TrimEnd('\')) { $SendName = Split-Path $SendDir -Leaf }
try { $M = [IO.File]::ReadAllText($Manifests[0].FullName) | ConvertFrom-Json } catch { Refuse "manifest.json illisible (JSON) : $($_.Exception.Message)"; Fail-Refused }
function Field([string]$Name) { $v = $M.$Name; if ($null -eq $v) { return '' }; return "$v".Trim() }

# --- Checks (every refusal collected, nothing written) ------------------------------------------------------------------------
if ((Field 'format') -ne '1') { Refuse "format inconnu : '$(Field 'format')' (format 1 attendu)" }
$Type = Field 'type'
$Tables = @{ Buildings = 'BuildingTypes'; Sources = 'SourceTypes' }
$Names = @{}
foreach ($t in 'BuildingTypes', 'SourceTypes', 'UnitTypes') { $Names[$t] = @(Import-Csv (Join-Path $Data "$t.csv") -Encoding UTF8 | ForEach-Object { $_.Name }) }
$Family = $null
if (-not $Type) { Refuse "type absent du manifeste" }
elseif ($Names['UnitTypes'] -ccontains $Type) { Refuse "type $Type : type d'unite, refuse ici (unites par l'editeur emballe, D929)" }
else {
    foreach ($f in $Tables.Keys) { if ($Names[$Tables[$f]] -ccontains $Type) { $Family = $f } }
    if (-not $Family) { Refuse "type $Type absent de BuildingTypes.csv et de SourceTypes.csv" }
    elseif ((Field 'family') -cne $Family) { Refuse "family '$(Field 'family')' fausse : $Type est dans $($Tables[$Family]).csv (family $Family)" }
}
$Faction = Field 'faction'
if ($Faction) {
    $Known = @(Import-Csv (Join-Path $Data 'Factions.csv') -Encoding UTF8 | ForEach-Object { $_.Faction } | Sort-Object -Unique)
    if ($Known -cnotcontains $Faction) { Refuse "faction inconnue : '$Faction' (connues : $($Known -join ', '))" }
}
$Model = Field 'model'
$ModelPath = $null
if (-not $Model) { Refuse "model absent du manifeste" }
elseif (@('.glb', '.gltf') -notcontains [IO.Path]::GetExtension($Model).ToLowerInvariant()) { Refuse "model '$Model' n'est pas un .glb / .gltf (FBX et autres : editeur emballe, D929)" }
else {
    $ModelPath = [IO.Path]::GetFullPath((Join-Path $SendDir $Model))
    if (-not $ModelPath.StartsWith($SendDir.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) { Refuse "model '$Model' sort du dossier de l'envoi" }
    elseif (-not (Test-Path -LiteralPath $ModelPath -PathType Leaf)) { Refuse "model '$Model' absent de l'envoi" }
}
$LicenceColumns = 'file_or_pack', 'source', 'licence', 'url', 'downloaded_on', 'licence_proof'
$Licence = $M.licence
$Missing = @('file_or_pack', 'source', 'licence' | Where-Object { $null -eq $Licence -or -not "$($Licence.$_)".Trim() })
if ($Missing.Count) { Refuse "licence incomplete : $($Missing -join ', ') vide(s) (colonnes de Docs\assets_art.csv)" }
$Yaw = 0.0; $Scale = 0.0
if (-not [double]::TryParse((Field 'yaw'), [Globalization.NumberStyles]::Float, $Inv, [ref]$Yaw)) { Refuse "yaw '$(Field 'yaw')' n'est pas un nombre" }
if (-not [double]::TryParse((Field 'scale'), [Globalization.NumberStyles]::Float, $Inv, [ref]$Scale) -or $Scale -le 0) { Refuse "scale '$(Field 'scale')' doit etre un nombre > 0" }
$Status = Field 'status'
if (@('final', 'provisional') -cnotcontains $Status) { Refuse "status '$Status' : final ou provisional attendu" }
if ($Refusals.Count) { if ($Work) { Remove-Item -Recurse -Force $Work -ErrorAction SilentlyContinue }; Fail-Refused }

# --- Plan -------------------------------------------------------------------------------------------------------------------------
$Suffix = if ($Faction) { "_$Faction" } else { '' }
$Folder = "/Game/Art/$Family/$Type"
$MeshPath = "$Folder/SM_$Type$Suffix"
$ArtPath = "$Folder/DA_Art_$Type$Suffix"
$ArtExists = Test-Path -LiteralPath (Join-Path $Root "Content\Art\$Family\$Type\DA_Art_$Type$Suffix.uasset")
$SourceRel = "ArtSource\$Family\$Type\$SendName"
$SourceDir = Join-Path $Root $SourceRel
$ArtStatus = if ($Status -eq 'final') { 'Final' } else { 'Placeholder' }
$RowStatus = if ($Status -eq 'final') { 'Final' } else { 'Provisoire' }
$NewFields = 'Mesh', 'MeshYawOffset', 'MeshScale'

$ArtVisualsFile = Join-Path $Data 'ArtVisuals.csv'
$MapFile = Join-Path $Data 'placeholders_map.csv'
$PlaceholdersFile = Join-Path $Root 'Placeholders.txt'
$AssetsFile = Join-Path $Root 'Docs\assets_art.csv'
function Read-Text([string]$Path) {
    $bytes = [IO.File]::ReadAllBytes($Path)
    $bom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
    $text = (New-Object Text.UTF8Encoding $false).GetString($bytes, $(if ($bom) { 3 } else { 0 }), $bytes.Length - $(if ($bom) { 3 } else { 0 }))
    $nl = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
    $lines = New-Object System.Collections.Generic.List[string]
    $text -split "`r?`n" | ForEach-Object { $lines.Add($_) }
    if ($lines.Count -and $lines[$lines.Count - 1] -eq '') { $lines.RemoveAt($lines.Count - 1) }
    return @{ Path = $Path; Bom = $bom; Nl = $nl; Lines = $lines }
}
function Write-Text($File) { [IO.File]::WriteAllText($File.Path, (($File.Lines -join $File.Nl) + $File.Nl), (New-Object Text.UTF8Encoding $File.Bom)) }
function ConvertTo-CsvField([string]$Value) { if ($Value -match '[",\r\n]') { return '"' + $Value.Replace('"', '""') + '"' }; return $Value }

# ArtVisuals.csv: the row (Type, Faction, Family) gets the fields and the status.
$Art = Read-Text $ArtVisualsFile
$Prefix = "$Type,$Faction,$Family,"
$Index = -1
for ($i = 1; $i -lt $Art.Lines.Count; $i++) { if ($Art.Lines[$i].StartsWith($Prefix, [StringComparison]::Ordinal)) { $Index = $i; break } }
$Fields = New-Object System.Collections.Generic.List[string]
if ($Index -ge 0) { ($Art.Lines[$Index].Split(',')[4]).Split(';') | Where-Object { $_ } | ForEach-Object { $Fields.Add($_) } }
foreach ($f in $NewFields) { if (-not $Fields.Contains($f)) { $Fields.Add($f) } }
$ArtRow = "$Type,$Faction,$Family,$ArtPath,$($Fields -join ';'),$RowStatus"
$ArtRowBefore = if ($Index -ge 0) { $Art.Lines[$Index] } else { '(absente)' }
if ($Index -ge 0) { $Art.Lines[$Index] = $ArtRow } else { $Art.Lines.Add($ArtRow) }

# Final type (check_placeholders.ps1's rule): every art row of the type Final with at least one field.
$TypeRows = 0; $TypeProvisional = 0
for ($i = 1; $i -lt $Art.Lines.Count; $i++) {
    $Cells = $Art.Lines[$i].Split(',')
    if ($Cells.Count -lt 6 -or $Cells[0] -cne $Type -or $Cells[2] -cne $Family) { continue }
    $TypeRows++
    if ($Cells[5].Trim() -ne 'Final' -or $Cells[4].Trim(' ;') -eq '') { $TypeProvisional++ }
}
$TypeFinal = $TypeRows -gt 0 -and $TypeProvisional -eq 0
$Table = $Tables[$Family]
$Map = Read-Text $MapFile
$MapEntries = @(Import-Csv $MapFile -Encoding UTF8)
$Entry = @($MapEntries | Where-Object { $_.Type -ceq $Type -and $_.Table -ceq $Table }) | Select-Object -First 1
$Places = Read-Text $PlaceholdersFile
$RemovedLines = @()
$RemoveEntry = $false
if ($Status -eq 'final' -and $TypeFinal -and $Entry) {
    $RemoveEntry = $true
    # A line shared with another provisional type of the map (same pattern) stays.
    $Shared = @($MapEntries | Where-Object { $_.Pattern -ceq $Entry.Pattern -and -not ($_.Type -ceq $Type -and $_.Table -ceq $Table) })
    if (-not $Shared.Count) { $RemovedLines = @($Places.Lines | Where-Object { $_ -match '^- ' -and $_ -match $Entry.Pattern }) }
}

$LicenceRow = (@($LicenceColumns | ForEach-Object { ConvertTo-CsvField "$($Licence.$_)".Trim() }) + (ConvertTo-CsvField $Type)) -join ','
$Assets = Read-Text $AssetsFile
$LicenceNew = -not $Assets.Lines.Contains($LicenceRow)

Write-Output "apply_art_send : envoi $SendName ($Family/$Type$(if ($Faction) { ", faction $Faction" }), status $Status) -> $Root"
Write-Output "  import      : $Model -> $MeshPath"
Write-Output "  DA_Art      : $ArtPath ($(if ($ArtExists) { 'existe' } else { 'cree, TypeName ' + $Type + $(if ($Faction) { ', Faction ' + $Faction }) })) : Mesh = $MeshPath ; bOverrideMeshYawOffset, MeshYawOffset = $(Format-Number $Yaw) ; bOverrideMeshScale, MeshScale = $(Format-Number $Scale) ; Status = $ArtStatus"
Write-Output "  source      : $SourceRel\ ($(@(Get-ChildItem -LiteralPath $SendDir -Recurse -File).Count) fichier(s), Git LFS D926)"
Write-Output "  Tools\Data\ArtVisuals.csv : $ArtRowBefore -> $ArtRow"
Write-Output "  Docs\assets_art.csv : $(if ($LicenceNew) { '+ ' + $LicenceRow } else { 'ligne deja presente' })"
if ($RemovedLines.Count) { $RemovedLines | ForEach-Object { Write-Output "  Placeholders.txt : retire $_" } }
elseif ($Status -eq 'final' -and $RemoveEntry) { Write-Output "  Placeholders.txt : inchange (ligne /$($Entry.Pattern)/ partagee avec un autre type provisoire)" }
elseif ($Status -eq 'final' -and -not $TypeFinal) { Write-Output "  Placeholders.txt : inchange (d'autres fiches de $Type restent provisoires)" }
else { Write-Output "  Placeholders.txt : inchange ($Status)" }
Write-Output "  Tools\Data\placeholders_map.csv : $(if ($RemoveEntry) { "retire $Type,$Table,$($Entry.Pattern)" } else { 'inchange' })"
if ($DryRun) {
    if ($Work) { Remove-Item -Recurse -Force $Work -ErrorAction SilentlyContinue }
    Write-Output 'apply_art_send : DRYRUN (rien n''est ecrit)'
    exit 0
}

# --- Application ---------------------------------------------------------------------------------------------------------------
$SourceExisted = Test-Path -LiteralPath $SourceDir
New-Item -ItemType Directory -Force $SourceDir | Out-Null
Copy-Item -Path (Join-Path $SendDir '*') -Destination $SourceDir -Recurse -Force
if (-not $NoEditor) {
    $UProject = Get-ChildItem -LiteralPath $Root -Filter '*.uproject' -File | Select-Object -First 1
    $Editor = Join-Path $Engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
    $Script = Join-Path $Root 'Tools\Editor\apply_art_send.py'
    $Log = Join-Path $Root 'Saved\Logs\apply_art_send.log'
    $PlanFile = Join-Path $env:TEMP ('hol_art_send_plan_' + [guid]::NewGuid().ToString('N').Substring(0, 8) + '.json')
    $Plan = [ordered]@{ model = (Join-Path $SourceDir $Model); family = $Family; type = $Type; faction = $Faction; folder = $Folder
        mesh = $MeshPath; art = $ArtPath; yaw = $Yaw; scale = $Scale; status = $ArtStatus }
    [IO.File]::WriteAllText($PlanFile, ($Plan | ConvertTo-Json), (New-Object Text.UTF8Encoding $false))
    New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
    # Content/Art/** is LFS "lockable" (D926): git checks those files out read-only; the editor could not save them.
    $ContentDir = Join-Path $Root "Content\Art\$Family\$Type"
    $Writable = @("DA_Art_$Type$Suffix.uasset", "SM_$Type$Suffix.uasset" | ForEach-Object { Join-Path $ContentDir $_ } | Where-Object { Test-Path -LiteralPath $_ })
    if (Test-Path -LiteralPath (Join-Path $ContentDir "SM_$Type$($Suffix)_Parts")) { $Writable += @(Get-ChildItem -LiteralPath (Join-Path $ContentDir "SM_$Type$($Suffix)_Parts") -Recurse -File | ForEach-Object FullName) }
    foreach ($f in $Writable) { $item = Get-Item -LiteralPath $f; if ($item.IsReadOnly) { $item.IsReadOnly = $false; Write-Output "  lecture seule levee (LFS lockable) : $($f.Substring($Root.Length + 1))" } }
    $env:HOL_ART_SEND_PLAN = $PlanFile
    $EditorArgs = @($UProject.FullName, '-run=pythonscript', "-script=$Script", '-unattended', '-nosplash', '-nosound', '-Multiprocess', '-LiveCoding=0', "-abslog=$Log")
    Write-Output "apply_art_send : editeur cache (journal $Log)"
    & $Editor @EditorArgs -nullrhi | Out-Null
    Remove-Item $PlanFile -ErrorAction SilentlyContinue
    $Done = @(Select-String -LiteralPath $Log -Pattern 'apply_art_send: DONE' -SimpleMatch | ForEach-Object { $_.Line })
    @(Select-String -LiteralPath $Log -Pattern 'apply_art_send: ' -SimpleMatch | ForEach-Object { '  ' + ($_.Line -replace '^.*?apply_art_send: ', 'editeur : ') }) | Write-Output
    if (-not $Done.Count) {
        if (-not $SourceExisted) { Remove-Item -Recurse -Force $SourceDir -ErrorAction SilentlyContinue }
        if ($Work) { Remove-Item -Recurse -Force $Work -ErrorAction SilentlyContinue }
        Write-Output "apply_art_send : FAIL (etape editeur, code $LASTEXITCODE, pas de ligne DONE ; fichiers texte non touches) : $Log"
        exit 2
    }
}
Write-Text $Art
if ($LicenceNew) { $Assets.Lines.Add($LicenceRow); Write-Text $Assets }
if ($RemovedLines.Count) { foreach ($l in $RemovedLines) { [void]$Places.Lines.Remove($l) }; Write-Text $Places }
if ($RemoveEntry) {
    $MapLine = @($Map.Lines | Where-Object { $_.StartsWith("$Type,$Table,", [StringComparison]::Ordinal) }) | Select-Object -First 1
    if ($MapLine) { [void]$Map.Lines.Remove($MapLine); Write-Text $Map }
}
if ($Work) { Remove-Item -Recurse -Force $Work -ErrorAction SilentlyContinue }
Write-Output "apply_art_send : PASS ($Family/$Type$Suffix, $Status$(if ($NoEditor) { ', -NoEditor : sans import' }))"
exit 0
