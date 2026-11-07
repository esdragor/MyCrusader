# B3 (BiomeInProgress.md), test de l EFFET sur le monde charge, sur la vraie chaine : trois parties de la Plaine (biome snow, fresh_grass,
# aucun biome), chacune lancee par Tools\Scenarios\check.ps1 ; le journal porte
#   "Biome decor expect: pieces=N kinds=K" (le plan que le script dit avoir pose) et
#   "Biome decor world: layers= pieces= kinds= sites= minAvoid= collide= nav= shadow=" (hol.Biome.DecorWorld : ce que le MONDE tient, lu sur les
#   meshes instancies eux-memes).
# PASS seulement si : snow et fresh_grass posent des pieces (pieces > 0, = le plan), sur au moins un site mesure, aucune a moins de 800 cm
# d un site, aucun composant avec collision, navigation ou ombre, une seule couche ; les deux biomes posent un decor different ; sans biome,
# rien n est pose.
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\biome_decor_check.ps1 [-Root <worktree>]
param([string] $Root = "")
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Fails = 0
function Fields([string]$Line) {
    $Map = @{}
    foreach ($M in [regex]::Matches($Line, '(\w+)=(-?[0-9.]+)')) { $Map[$M.Groups[1].Value] = [double]::Parse($M.Groups[2].Value, [Globalization.CultureInfo]::InvariantCulture) }
    return $Map
}
$Worlds = @{}
foreach ($Name in @("biome_decor_snow", "biome_decor_grass", "biome_decor_default")) {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $Root "Tools\Scenarios\check.ps1") $Name -Root $Root | Out-Null
    $Log = Join-Path $Root "Saved_Tests\Logs\scenario_$Name.log"
    $Text = if (Test-Path $Log) { Get-Content $Log } else { @() }
    $WorldLine = $Text | Where-Object { $_ -match 'Biome decor world:' } | Select-Object -Last 1
    $ExpectLine = $Text | Where-Object { $_ -match 'Biome decor expect:' } | Select-Object -Last 1
    if (-not $WorldLine) { Write-Host "FAIL $Name : no 'Biome decor world:' line (the scenario did not reach hol.Biome.DecorWorld)"; $Fails++; continue }
    $W = Fields $WorldLine; $Worlds[$Name] = $W
    $Before = $Fails
    if ($Name -eq "biome_decor_default") {
        if ($W["pieces"] -ne 0 -or $W["layers"] -ne 0) { Write-Host "FAIL $Name : the map's own biome must lay nothing, the world holds $WorldLine"; $Fails++ }
    } else {
        if (-not $ExpectLine) { Write-Host "FAIL $Name : the applier never said it laid a decor ('Biome decor expect:' absent); world read: $WorldLine"; $Fails++; continue }
        $E = Fields $ExpectLine
        if ($W["pieces"] -le 0) { Write-Host "FAIL $Name : no piece of decor in the world ($WorldLine)"; $Fails++ }
        if ($W["pieces"] -ne $E["pieces"]) { Write-Host "FAIL $Name : the world holds $($W['pieces']) pieces, the plan said $($E['pieces'])"; $Fails++ }
        if ($W["kinds"] -ne $E["kinds"]) { Write-Host "FAIL $Name : the world holds $($W['kinds']) kinds, the plan said $($E['kinds'])"; $Fails++ }
        if ($W["layers"] -ne 1) { Write-Host "FAIL $Name : expected one decor actor, the world holds $($W['layers'])"; $Fails++ }
        if ($W["sites"] -le 0) { Write-Host "FAIL $Name : no site found on the map, so the distance proves nothing ($WorldLine)"; $Fails++ }
        elseif ($W["minAvoid"] -lt 800) { Write-Host "FAIL $Name : a piece lies $($W['minAvoid']) cm from a site (less than 800)"; $Fails++ }
        if ($W["collide"] -ne 0 -or $W["nav"] -ne 0 -or $W["shadow"] -ne 0) { Write-Host "FAIL $Name : a decor component collides ($($W['collide'])), affects the navigation ($($W['nav'])) or casts a shadow ($($W['shadow']))"; $Fails++ }
    }
    if ($Fails -eq $Before) { Write-Host "OK   $Name : $WorldLine" }
}
if ($Worlds.ContainsKey("biome_decor_snow") -and $Worlds.ContainsKey("biome_decor_grass")) {
    $A = $Worlds["biome_decor_snow"]; $B = $Worlds["biome_decor_grass"]
    if ($A["pieces"] -eq $B["pieces"] -and $A["kinds"] -eq $B["kinds"]) { Write-Host "FAIL the two biomes lay the same number of pieces of the same number of kinds"; $Fails++ } else { Write-Host "OK   the two biomes lay a different decor" }
}
Write-Host "biome_decor_check: $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
exit $(if ($Fails -eq 0) { 0 } else { 1 })
