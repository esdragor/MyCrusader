# extend_session.ps1 (L14-13, 2026-11-14) : prolonge (ou avance) la seance : reecrit Docs\SessionEnd.txt de la copie principale avec la nouvelle fin
# DATEE ('yyyy-MM-dd HH:mm' ; une heure deja passee aujourd'hui = le LENDEMAIN, seance prolongee apres minuit), puis rappelle ce qui reste a la main :
# l'heure d'ARRET DUR a mettre dans l'enonce (brief) de chaque agent lance ou relance, et les alarmes a recreer (celles de l'ancienne fin).
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier> <HH:MM | 'yyyy-MM-dd HH:mm'> [-Root C:\Users\baudo\Documents\MyCrusader]
# Code 0 ; code 2 si l'heure est illisible (fichier inchange). Test : extend_session_test.ps1. session_gate.ps1 accepte une fin du lendemain.
param(
  [Parameter(Position = 0)][string]$Time = '',
  [string]$Root = 'C:\Users\baudo\Documents\MyCrusader'
)
$ErrorActionPreference = 'Continue'
$Inv = [Globalization.CultureInfo]::InvariantCulture
$now = Get-Date
$t = ($Time.Trim() -replace '\s+', ' ') -replace '(^| )(\d):', '${1}0$2:'   # '1:00' -> '01:00'
$endAt = [datetime]::MinValue
if ($t -match '^\d{4}-\d{2}-\d{2} \d{2}:\d{2}$') {
  if (-not [datetime]::TryParseExact($t, 'yyyy-MM-dd HH:mm', $Inv, [Globalization.DateTimeStyles]::None, [ref]$endAt)) { $endAt = [datetime]::MinValue }
}
elseif ($t -match '^\d{2}:\d{2}$') {
  $d = [datetime]::MinValue
  if ([datetime]::TryParseExact($now.ToString('yyyy-MM-dd') + ' ' + $t, 'yyyy-MM-dd HH:mm', $Inv, [Globalization.DateTimeStyles]::None, [ref]$d)) {
    $endAt = if ($d -lt $now) { $d.AddDays(1) } else { $d }
  }
}
if ($endAt -eq [datetime]::MinValue) {
  Write-Output "ERREUR : heure illisible '$Time' (attendu HH:MM ou 'yyyy-MM-dd HH:mm') ; Docs\SessionEnd.txt inchange"
  exit 2
}
$File = Join-Path $Root 'Docs\SessionEnd.txt'
$old = if (Test-Path $File) { ((Get-Content $File -ErrorAction SilentlyContinue | Select-Object -First 1) -as [string]).Trim() } else { '' }
$new = $endAt.ToString('yyyy-MM-dd HH:mm')
$dir = Split-Path $File -Parent
if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
[IO.File]::WriteAllText($File, $new + "`r`n")
$shift = ''
$oldAt = [datetime]::MinValue
if ($old -and [datetime]::TryParseExact($old, 'yyyy-MM-dd HH:mm', $Inv, [Globalization.DateTimeStyles]::None, [ref]$oldAt)) {
  $shift = ' ({0:+0;-0} min)' -f [int]($endAt - $oldAt).TotalMinutes
}
Write-Output ("SessionEnd : '{0}' -> '{1}'{2} ecrit dans {3}" -f $(if ($old) { $old } else { '(absent)' }), $new, $shift, $File)
Write-Output ("Fin dure de la seance : {0} ; il reste {1} min (Get-Date {2})" -f $new, [int]($endAt - $now).TotalMinutes, $now.ToString('HH:mm'))
Write-Output ("A FAIRE A LA MAIN (1) brief des agents : l'enonce de chaque agent lance ou relance porte 'ARRET DUR a {0} (Get-Date PowerShell)' ; les agents deja lances gardent l'ancienne heure ({1}) jusqu'a un message SendMessage" -f $endAt.ToString('HH:mm'), $(if ($old) { $old } else { 'inconnue' }))
Write-Output ("A FAIRE A LA MAIN (2) alarmes : supprimer celles posees pour l'ancienne fin ({0}) et recreer chaque alarme a la meme distance de la nouvelle fin {1}{2}" -f $(if ($old) { $old } else { 'inconnue' }), $new, $shift)
Write-Output "Puis : commit_docs.ps1 -Only Docs\SessionEnd.txt (commit de ce seul fichier) ; session_gate.ps1 lit la nouvelle fin."
exit 0
