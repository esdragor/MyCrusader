# MainCodeFilter.ps1 (L20-3) : filtre du controle L19-1 de `session_gate.ps1 -Phase Close`.
# Source dot-sourcee par session_gate.ps1 et main_code_filter_test.ps1 (fonction isolee, testable sans depot).
# STUB (rouge) : comportement d'avant L20-3 (Script/, Source/, Tools/Tests/ ; aucun filtre sur le titre ni sur les .uasset).
function Test-MainCodeCommit {
  param([string]$Title, [string[]]$Files)
  return (@($Files | Where-Object { $_ -match '^(Script|Source|Tools/Tests)/' }).Count -gt 0)
}
