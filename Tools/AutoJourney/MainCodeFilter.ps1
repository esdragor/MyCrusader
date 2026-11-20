# MainCodeFilter.ps1 (L20-3) : filtre du controle L19-1 de `session_gate.ps1 -Phase Close`.
# Source dot-sourcee par session_gate.ps1 et main_code_filter_test.ps1 (fonction isolee, testable sans depot).
# Un « codage » de la session principale = commit de FICHIERS DE CODE (Source/, Script/, Tools/) ; ne comptent pas (permis par la procedure) :
# les fusions (titre `Merge ...`, fusions groupees), les commits `Docs:`, les commits d'assets seuls (.uasset / .umap).
# Les avances rapides de main et les cherry-pick n'ont pas d'entree `commit:` dans le reflog : session_gate ne les voit deja pas.
function Test-MainCodeCommit {
  param([string]$Title, [string[]]$Files)
  if ($Title -match '^\s*Merge\b' -or $Title -match '^\s*Docs:') { return $false }
  return (@($Files | Where-Object { $_ -match '^(Script|Source|Tools)/' -and $_ -notmatch '\.(uasset|umap)$' }).Count -gt 0)
}
