# Test without the editor of Tools\Editor\import_art_icons.py (AR5-e): every icon booth PNG has its destination texture computed.
# - a unit icon lands where the garrison frame reads it (the fallback path of Script\UI\HOLGarrisonFrameRules.as, read from the file);
# - buildings and sources land under /Game/UI/Icons/Buildings and /Game/UI/Icons/Sources;
# - a name outside the three families is refused (NONE);
# - each PNG of Saved\ArtIcons (when the art_icon_all scenario made them) has a destination in its family folder.
# Exit code 0 = green, 1 = red.
param([string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path)
$ErrorActionPreference = 'Stop'
$script = Join-Path $Root 'Tools\Editor\import_art_icons.py'
$python = (Get-Command python -ErrorAction SilentlyContinue)
if (-not $python) { $python = (Get-Command py -ErrorAction SilentlyContinue) }
if (-not $python) { Write-Output 'FAIL no python'; exit 1 }
$fails = 0
function Get-Plan([string[]]$Names) {
    $map = @{}
    foreach ($line in (& $python.Source $script --plan @Names)) {
        if ($line -match '^(\S+) -> (\S+)$') { $map[$Matches[1]] = $Matches[2] }
    }
    return $map
}
function Assert-Equal([string]$Name, [string]$Got, [string]$Want) {
    if ($Got -ne $Want) { Write-Output "FAIL $Name : got '$Got', want '$Want'"; $script:fails++ } else { Write-Output "ok $Name -> $Got" }
}

# The path the garrison frame reads for a unit (the f-string of UnitIconPath), the object name removed.
$rules = Get-Content -Raw (Join-Path $Root 'Script\UI\HOLGarrisonFrameRules.as')
if ($rules -notmatch 'return f"(/Game/UI/Icons/Units/T_Unit_\{TypeKey\})\.T_Unit_\{TypeKey\}"') { Write-Output 'FAIL fallback path not found in HOLGarrisonFrameRules.as'; exit 1 }
$unitPattern = $Matches[1]

$plan = Get-Plan @('T_Unit_Archer.png', 'T_Unit_Spearman.png', 'T_Building_Barracks.png', 'T_Source_Tree.png', 'T_Hat_Archer.png', 'readme.txt')
Assert-Equal 'T_Unit_Archer.png' $plan['T_Unit_Archer.png'] ($unitPattern -replace '\{TypeKey\}', 'Archer')
Assert-Equal 'T_Unit_Spearman.png' $plan['T_Unit_Spearman.png'] ($unitPattern -replace '\{TypeKey\}', 'Spearman')
Assert-Equal 'T_Building_Barracks.png' $plan['T_Building_Barracks.png'] '/Game/UI/Icons/Buildings/T_Building_Barracks'
Assert-Equal 'T_Source_Tree.png' $plan['T_Source_Tree.png'] '/Game/UI/Icons/Sources/T_Source_Tree'
Assert-Equal 'T_Hat_Archer.png' $plan['T_Hat_Archer.png'] 'NONE'
Assert-Equal 'readme.txt' $plan['readme.txt'] 'NONE'

$folder = Join-Path $Root 'Saved\ArtIcons'
$pngs = @(Get-ChildItem -Path $folder -Filter *.png -ErrorAction SilentlyContinue | ForEach-Object Name)
if ($pngs.Count -gt 0) {
    $all = Get-Plan $pngs
    $families = @{ 'Unit' = 'Units'; 'Building' = 'Buildings'; 'Source' = 'Sources' }
    $bad = 0
    foreach ($png in $pngs) {
        $stem = [IO.Path]::GetFileNameWithoutExtension($png)
        $family = ($stem -split '_')[1]
        $want = "/Game/UI/Icons/$($families[$family])/$stem"
        if ($all[$png] -ne $want) { Write-Output "FAIL $png : got '$($all[$png])', want '$want'"; $bad++ }
    }
    if ($bad -gt 0) { $fails += $bad } else { Write-Output "ok $($pngs.Count) PNG of Saved\ArtIcons have their destination" }
} else {
    Write-Output 'note: no PNG in Saved\ArtIcons (run the art_icon_all scenario), only the fixed names are checked'
}
if ($fails -gt 0) { Write-Output "import_art_icons_test: RED ($fails failures)"; exit 1 }
Write-Output 'import_art_icons_test: GREEN'
exit 0
