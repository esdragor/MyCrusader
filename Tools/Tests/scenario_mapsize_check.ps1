# Sol de biome des scenarios de revue visuelle (BiomeInProgress.md, enquete SAURON-1114d du 2026-11-14) : un .json de scenario SANS mapSize charge la
# CARTE DE TEST /Game/Maps/Plain (EHOLMapSize::None : dalles grises M_MatteGround, aucun MI_Ground_*), que HOLBiomeApplier ne teinte pas ; avec mapSize,
# FHOLMapSizeRules::GetMapFile charge la carte generee (Plain_Small...) et son sol de biome. Ce test lit chaque scenario de Tools\Scenarios : un scenario
# qui MONTRE le decor ou le biome (son .txt appelle hol.Rocks.Review, hol.Resource.Review ou hol.Biome.) doit porter mapSize, sauf ceux listes dans
# $Exceptions (carte de test voulue). Les autres scenarios sans mapSize (jeu, carte de test) sont comptes, pas refuses (Retours\Q-scenarios-mapsize.md).
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\scenario_mapsize_check.ps1 [-Root <worktree>] [-List]
param([string] $Root = "", [switch] $List)
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Scen = Join-Path (Resolve-Path $Root).Path "Tools\Scenarios"
# biome_apply_noground : essai de casse, une carte SANS sol de biome est voulue. glade_sweep : carte d'essai GladeSweep (make_glade_sweep.ps1).
$Exceptions = @("biome_apply_noground", "glade_sweep")
$Fails = 0
$Others = @()
foreach ($Json in Get-ChildItem (Join-Path $Scen "*.json")) {
    $Text = Get-Content $Json.FullName -Raw
    if ($Text -match '"mapSize"') { continue }
    $Name = $Json.BaseName
    $Txt = Join-Path $Scen "$Name.txt"
    $Shows = (Test-Path $Txt) -and ((Get-Content $Txt -Raw) -match 'hol\.Rocks\.Review|hol\.Resource\.Review|hol\.Biome\.')
    if ($Shows -and $Exceptions -notcontains $Name) { Write-Host "FAIL $Name shows the decor or the biome but has no mapSize (test map, grey ground)"; $Fails++ }
    elseif (-not $Shows) { $Others += $Name }
}
Write-Host "INFO $($Others.Count) other scenarios without mapSize (gameplay on the test map, not refused)$(if ($List) { ': ' + ($Others -join ' ') })"
Write-Host "scenario_mapsize_check: $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
exit $(if ($Fails -eq 0) { 0 } else { 1 })
