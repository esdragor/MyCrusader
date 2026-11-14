# Sol de biome (BiomeInProgress.md, 2026-11-14), test de l'EFFET sur le monde charge, sur la vraie chaine : la partie du scenario
# rocks_compare2 (son .json, recopie a l'identique dans biome_ground_probe.json : ce test refuse s'ils different) est lancee par
# Tools\Scenarios\check.ps1, le biome 0 (Herbe fraiche) est pose par hol.Biome.Index, puis hol.Biome.World (C++) lit le MONDE :
# "groundA=" n'y figure que si un mesh de sol M_GroundVaried (MI_Ground_* de la carte) existe, et doit valoir le groundA annonce
# par HOLBiomeApplier ("Biome expect:"). Rouge si la carte chargee est la carte de test Plain.umap (dalles grises M_MatteGround,
# aucun MI_Ground_*, que l'applicateur ne reprend pas), vert si c'est une carte generee (Plain_Small...).
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\biome_ground_check.ps1 [-Root <worktree>]
param([string] $Root = "")
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Fails = 0
$Scen = Join-Path $Root "Tools\Scenarios"
$Real = (Get-Content (Join-Path $Scen "rocks_compare2.json") -Raw).Trim()
$Probe = (Get-Content (Join-Path $Scen "biome_ground_probe.json") -Raw).Trim()
if ($Real -ne $Probe) { Write-Host "FAIL biome_ground_probe.json differs from rocks_compare2.json (copy it again)"; $Fails++ }
& powershell -ExecutionPolicy Bypass -File (Join-Path $Scen "check.ps1") biome_ground_probe -Root $Root | Out-Null
$Log = Join-Path $Root "Saved_Tests\Logs\scenario_biome_ground_probe.log"
$Text = if (Test-Path $Log) { Get-Content $Log } else { @() }
$Map = $Text | Where-Object { $_ -match 'LoadMap|Browse.*Maps/|Maps/Plain' } | Select-Object -Last 1
$Expect = $Text | Where-Object { $_ -match 'Biome expect:' } | Select-Object -Last 1
$World = $Text | Where-Object { $_ -match 'Biome world:' } | Select-Object -Last 1
if ($Map) { Write-Host "map  $($Map.Trim())" }
if (-not $World) { Write-Host "FAIL no 'Biome world:' line (the scenario did not reach hol.Biome.World)"; $Fails++ }
elseif ($World -notmatch 'groundA=([-0-9.,eE]+)') { Write-Host "FAIL the loaded world has no biome ground (no M_GroundVaried / MI_Ground_* mesh): $($World.Trim())"; $Fails++ }
else {
    $W = $Matches[1]
    if (-not $Expect -or $Expect -notmatch 'groundA=([-0-9.,eE]+)') { Write-Host "FAIL the applier never announced a ground ('Biome expect:' groundA absent)"; $Fails++ }
    else {
        $E = $Matches[1] -split ',' | ForEach-Object { [double]::Parse($_, [Globalization.CultureInfo]::InvariantCulture) }
        $G = $W -split ',' | ForEach-Object { [double]::Parse($_, [Globalization.CultureInfo]::InvariantCulture) }
        for ($I = 0; $I -lt $E.Count; $I++) { if ([math]::Abs($E[$I] - $G[$I]) -gt 0.002) { Write-Host "FAIL groundA[$I] expected $($E[$I]) but the world holds $($G[$I])"; $Fails++ } }
        if ($Fails -eq 0) { Write-Host "OK   the biome ground is on the world: groundA=$W (expected $($Matches[1]))" }
    }
}
Write-Host "biome_ground_check: $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
exit $(if ($Fails -eq 0) { 0 } else { 1 })
