# B1 visuel, essai de casse sur le monde charge : un biome pose en cours de partie puis retour au look de la carte (hol.Biome.Index -1).
# Le scenario biome_apply_restore lit le monde 5 fois (hol.Biome.World) : [0] la carte seule, [1] snow, [2] snow encore, [3] steppe, [4] retour a la carte.
# PASS seulement si [1] differe de [0] (le biome agit), [2] = [1] (poser deux fois ne change rien), [3] differe de [1], et [4] = [0] (le retour a la carte rend le look de la carte).
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\biome_restore_check.ps1 [-Root <worktree>]
param([string] $Root = "")
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
& powershell -ExecutionPolicy Bypass -File (Join-Path $Root "Tools\Scenarios\check.ps1") biome_apply_restore -Root $Root | Out-Null
$Log = Join-Path $Root "Saved_Tests\Logs\scenario_biome_apply_restore.log"
$Lines = @(if (Test-Path $Log) { Get-Content $Log | Where-Object { $_ -match 'Biome world:' } | ForEach-Object { ($_ -replace '^.*Biome world:', '').Trim() } } else { @() })
if ($Lines.Count -lt 5) { Write-Host "FAIL: only $($Lines.Count) 'Biome world:' lines, expected 5"; exit 1 }
$Fails = 0
function Check([bool]$Ok, [string]$Text) { if ($Ok) { Write-Host "OK   $Text" } else { Write-Host "FAIL $Text"; $script:Fails++ } }
for ($I = 0; $I -lt 5; $I++) { Write-Host "  [$I] $($Lines[$I])" }
Check ($Lines[1] -ne $Lines[0]) "snow changes the world the map gave"
Check ($Lines[2] -eq $Lines[1]) "the same biome twice leaves the world as it is"
Check ($Lines[3] -ne $Lines[1]) "another biome changes the world again"
Check ($Lines[4] -eq $Lines[0]) "back to the map's own look: the world holds the look the map file carries"
Write-Host "biome_restore_check: $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
exit $(if ($Fails -eq 0) { 0 } else { 1 })
