# art_visual_io_test.ps1 : les scripts d'editeur ecrivent le DA_Art, plus les colonnes visuelles des CSV de types
# (AR0-d2 etape 3b, D925, GLORFINDEL-1125ar2). Aucun editeur, aucun C++ : lecture des sources de Tools\Editor.
#  1. Tools\Editor\art_visual_io.py (module partage) charge ou cree le DA_Art_<Type> (HOLArtVisual), pose le champ,
#     l'enregistre (save_loaded_asset) et le journalise.
#  2. Chaque script qui fabrique un modele dessine par un type l'ecrit par art_visual_io.set_art_field : make_arrow_mesh.py
#     (Projectile, MuzzleSmoke) et make_peasant_mesh.py (SkinnedMesh, AnimationProvider du paysan et des soldats).
#  3. Aucun script de Tools\Editor (hors make_*_types.py et make_art_visuals.py, lecteurs du CSV, etape 3) ne renvoie a une
#     colonne visuelle d'UnitTypes.csv / BuildingTypes.csv / SourceTypes.csv.
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\art_visual_io_test.ps1   (code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Editor = Join-Path (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) 'Editor'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
    if ($Ok) { Write-Output "PASS $Name" } else { Write-Output "FAIL $Name $Detail"; $script:Fails++ }
}

# 1. Le module partage ecrit vraiment l'asset.
$IoPath = Join-Path $Editor 'art_visual_io.py'
$Io = if (Test-Path $IoPath) { [IO.File]::ReadAllText($IoPath) } else { '' }
Check ($Io -match 'def set_art_field\(family, type_name, field, value\)') 'io.signature' 'set_art_field(family, type_name, field, value) absent'
Check ($Io -match 'HOLArtVisual') 'io.class' 'ne vise pas la classe HOLArtVisual'
Check ($Io -match 'DA_Art_') 'io.asset_name' 'ne nomme pas DA_Art_<Type>'
Check ($Io -match 'save_loaded_asset') 'io.saves' "n'enregistre pas l'asset"
Check ($Io -match 'unreal\.log\(') 'io.logs' 'aucune ligne de journal (preuve dans l editeur)'

# 2. Les fabricants de modeles appellent le module.
foreach ($Name in @('make_arrow_mesh.py', 'make_peasant_mesh.py')) {
    $Text = [IO.File]::ReadAllText((Join-Path $Editor $Name))
    Check (($Text -match '(?m)^import art_visual_io') -and ($Text -match 'art_visual_io\.set_art_field\(')) "writer.$Name" "n'ecrit pas le DA_Art par art_visual_io.set_art_field"
}

# 3. Plus aucune colonne visuelle d'un CSV de types dans les scripts d'editeur.
$Columns = 'Mesh|MeshYawOffset|MeshScale|SkinnedMesh|AnimationProvider|IdleSequence|WalkSequence|WalkPlayRate|GatherSequences|GatherPlayRate|CarryWalkSequences|CarryWalkPlayRate|CarryIdleSequences|GatherProps|CarryProps|BuildSequence|BuildPlayRate|BuildProp|AttackSequence|AttackProp|FallSequence|Projectile|MuzzleSmoke'
$Readers = @('make_unit_types.py', 'make_building_types.py', 'make_source_types.py', 'make_art_visuals.py')
$Hits = @()
foreach ($File in Get-ChildItem -Path $Editor -Filter '*.py' -File) {
    if ($Readers -contains $File.Name) { continue }
    $Lines = [IO.File]::ReadAllLines($File.FullName)
    for ($i = 0; $i -lt $Lines.Count; $i++) {
        $Line = $Lines[$i]
        if ($Line -cmatch '(Unit|Building|Source)Types\.csv' -and $Line -cmatch "\b($Columns)\b") { $Hits += "$($File.Name):$($i + 1)" }
    }
}
Check ($Hits.Count -eq 0) 'no_csv_visual_column' ($Hits -join ', ')

if ($Fails -gt 0) { Write-Output "art_visual_io_test: $Fails FAIL"; exit 1 }
Write-Output 'art_visual_io_test: PASS'
exit 0
