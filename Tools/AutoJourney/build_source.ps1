# build_source.ps1 : empreinte du C++ compile (HALDIR-1127ar1). ASCII seulement. A dot-sourcer, une seule copie :
#   Tools\build.ps1 l'ECRIT apres un build PASS de la cible d'editeur (<Root>\Binaries\Win64\HOL_BuildSource.txt ; un FAIL l'efface) ;
#   Tools\AutoJourney\integrate.ps1 la COMPARE a celle de main avant de sauter un build faute de C++ dans les rouges / verts.
# Pourquoi (integ-98-art, 2026-11-26) : la DLL du worktree venait d'une autre branche (integ-100) et la preuve d'integration a porte sur
# elle ; le controle par dates (Get-StaleBinaryProblem, "each dll newer than its sources") ne voit pas un fichier SUPPRIME cote main.
# Empreinte = "source=<hash d'arbre de <rev>:Source> uproject=<hash du blob *.uproject>" ; "dirty" pour l'arbre de travail quand
# `git status --porcelain -- Source *.uproject` n'est pas vide (un build d'arbre sale ne correspond a aucun commit) ou hors depot git.
# Binaries/ est ignore par git (.gitignore) : le fichier ne salit aucun arbre.

function Get-BuildSourcePath([string] $Root) { return (Join-Path $Root 'Binaries\Win64\HOL_BuildSource.txt') }

function Get-SourceFingerprint([string] $Root, [string] $Rev = '') {
  # $Rev vide = l'arbre de travail de $Root (HEAD s'il est propre, sinon "dirty") ; sinon le commit $Rev ('' si git echoue).
  $Commit = $Rev
  if (-not $Rev) {
    $Dirty = @(& git.exe -C $Root status --porcelain -- Source '*.uproject' 2>$null)
    if ($LASTEXITCODE -ne 0 -or $Dirty.Count) { return 'dirty' }
    $Commit = 'HEAD'
  }
  $Lines = @(& git.exe -C $Root ls-tree $Commit 2>$null)
  if ($LASTEXITCODE -ne 0) { if ($Rev) { return '' } else { return 'dirty' } }
  $SourceHash = 'none'
  $Projects = @()
  foreach ($Line in $Lines) {
    if ("$Line" -notmatch '^\d+ (\w+) ([0-9a-f]+)\t(.+)$') { continue }
    if ($Matches[1] -eq 'tree' -and $Matches[3] -eq 'Source') { $SourceHash = $Matches[2] }
    elseif ($Matches[1] -eq 'blob' -and $Matches[3] -like '*.uproject') { $Projects += $Matches[2] }
  }
  $ProjectHash = $(if ($Projects.Count) { ($Projects | Sort-Object) -join ',' } else { 'none' })
  return "source=$SourceHash uproject=$ProjectHash"
}

function Get-DllSourceProblem([string] $Root, [string] $Rev = 'main') {
  # '' = la DLL du worktree a ete compilee depuis le C++ de $Rev ; sinon la raison lisible (plan / resume d'integrate.ps1).
  $Want = Get-SourceFingerprint $Root $Rev
  $File = Get-BuildSourcePath $Root
  if (-not (Test-Path $File)) { return "DLL du worktree ne correspond pas au C++ de $Rev (empreinte absente : $File ; attendue $Want)" }
  $Have = ([IO.File]::ReadAllText($File)).Trim()
  if (-not $Want -or $Have -ne $Want) { return "DLL du worktree ne correspond pas au C++ de $Rev (empreinte $Have != $Want)" }
  return ''
}
