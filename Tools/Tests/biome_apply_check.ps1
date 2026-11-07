# B1 visuel (BiomeInProgress.md), test de l'EFFET sur le monde charge, sur la vraie chaine : deux parties de la Plaine (biome snow, puis
# fresh_grass dans le fichier de configuration), chacune lancee par Tools\Scenarios\check.ps1 ; le journal porte
#   "Biome expect: ..." (ce que le script UHOLBiomeApplier dit avoir regle, calcule par HOLBiomeApply::LookOf) et
#   "Biome world: ..." (hol.Biome.World, C++ : ce que le MONDE tient, lu sur la lumiere, le ciel, la brume et le sol).
# PASS seulement si, pour chaque biome, le monde = l'attendu, ET si les deux mondes different (le biome change vraiment la lumiere).
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\biome_apply_check.ps1 [-Root <worktree>]
param([string] $Root = "")
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Fails = 0
function Fields([string]$Line) {
    $Map = @{}
    foreach ($M in [regex]::Matches($Line, '(\w+)=([-0-9.,eE]+)')) { $Map[$M.Groups[1].Value] = @($M.Groups[2].Value -split ',' | ForEach-Object { [double]::Parse($_, [Globalization.CultureInfo]::InvariantCulture) }) }
    return $Map
}
$Worlds = @{}
foreach ($Name in @("biome_apply_snow", "biome_apply_grass")) {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $Root "Tools\Scenarios\check.ps1") $Name -Root $Root | Out-Null
    $Log = Join-Path $Root "Saved_Tests\Logs\scenario_$Name.log"
    $Text = if (Test-Path $Log) { Get-Content $Log } else { @() }
    $Expect = $Text | Where-Object { $_ -match 'Biome expect:' } | Select-Object -Last 1
    $World = $Text | Where-Object { $_ -match 'Biome world:' } | Select-Object -Last 1
    if (-not $World) { Write-Host "FAIL $Name : no 'Biome world:' line (the scenario did not reach hol.Biome.World)"; $Fails++; continue }
    if (-not $Expect) { Write-Host "FAIL $Name : the applier never said it set a biome ('Biome expect:' absent); world read: $World"; $Fails++; continue }
    $E = Fields $Expect; $W = Fields $World; $Worlds[$Name] = $W
    foreach ($Key in @("sun", "sunPitch", "sky", "fog", "groundA")) {
        if (-not $E.ContainsKey($Key) -or -not $W.ContainsKey($Key)) { Write-Host "FAIL $Name : '$Key' missing (expect: $($E.ContainsKey($Key)), world: $($W.ContainsKey($Key)))"; $Fails++; continue }
        $Tolerance = if ($Key -eq "fog") { 0.0005 } elseif ($Key -eq "groundA") { 0.002 } else { 0.01 }
        for ($I = 0; $I -lt $E[$Key].Count; $I++) {
            if ([math]::Abs($E[$Key][$I] - $W[$Key][$I]) -gt $Tolerance) { Write-Host "FAIL $Name : $Key[$I] expected $($E[$Key][$I]) but the world holds $($W[$Key][$I])"; $Fails++ }
        }
    }
    if ($Fails -eq 0) { Write-Host "OK   $Name : $World" }
}
if ($Worlds.Count -eq 2) {
    $A = $Worlds["biome_apply_snow"]; $B = $Worlds["biome_apply_grass"]
    $Differ = ($A["sun"][0] -ne $B["sun"][0]) -or ($A["sky"][0] -ne $B["sky"][0]) -or ($A["fog"][0] -ne $B["fog"][0])
    if (-not $Differ) { Write-Host "FAIL the two biomes leave the same light, sky and haze in the world"; $Fails++ } else { Write-Host "OK   the two biomes leave different lights in the world" }
}
Write-Host "biome_apply_check: $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
exit $(if ($Fails -eq 0) { 0 } else { 1 })
