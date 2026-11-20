# main_code_filter_test.ps1 (L20-3) : cas du filtre Test-MainCodeCommit (MainCodeFilter.ps1), sans depot ni gate.
# Un « codage » de la session principale = commit de fichiers de code (Source/, Script/, Tools/) ; ne comptent pas : titre `Merge ...`,
# titre `Docs:`, commit d'assets seuls (.uasset / .umap). Code 0 = tout vert.
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'MainCodeFilter.ps1')
$fails = 0
function Check([bool]$ok, [string]$name) { if ($ok) { Write-Output "PASS $name" } else { Write-Output "FAIL $name"; $script:fails++ } }

Check (Test-MainCodeCommit 'Fix pop' @('Script/Pop/Pop.as')) 'commit de Script/ compte'
Check (Test-MainCodeCommit 'Fix mass' @('Source/HeroesOfLiberty/Mass.cpp')) 'commit de Source/ compte'
Check (Test-MainCodeCommit 'Fix test' @('Tools/Tests/a.ps1')) 'commit de Tools/Tests/ compte (comme avant)'
Check (Test-MainCodeCommit 'Fix gate' @('Tools/AutoJourney/session_gate.ps1')) 'commit de Tools/ hors Tests compte'
Check (Test-MainCodeCommit 'Regen + fix' @('Content/Maps/A.uasset', 'Script/Pop/Pop.as')) '.uasset + code : compte (le code y est)'
Check (-not (Test-MainCodeCommit 'Merge branch ''elrond'' into main' @('Script/Pop/Pop.as'))) 'titre Merge ... ne compte pas'
Check (-not (Test-MainCodeCommit 'Merge agents faramir, haldir' @('Source/HeroesOfLiberty/Mass.cpp', 'Tools/x.ps1'))) 'fusion groupee ne compte pas'
Check (-not (Test-MainCodeCommit 'Docs: session docs 2026-11-20' @('Script/Pop/Pop.as'))) 'titre Docs: ne compte pas'
Check (-not (Test-MainCodeCommit 'Regenerate maps' @('Content/Maps/A.uasset', 'Content/Maps/B.umap'))) 'assets seuls (.uasset/.umap) ne comptent pas'
Check (-not (Test-MainCodeCommit 'Edit docs' @('Docs/Rapports/X.md', 'EtapeDefini.md'))) 'commit sans fichier de code ne compte pas'
Check (-not (Test-MainCodeCommit 'Regen' @('Script/Maps/A.uasset'))) '.uasset sous un dossier de code ne compte pas'
Check (-not (Test-MainCodeCommit 'x' @())) 'commit sans fichier ne compte pas'

if ($fails -eq 0) { Write-Output 'main_code_filter_test : VERT'; exit 0 } else { Write-Output "main_code_filter_test : ROUGE ($fails)"; exit 1 }
