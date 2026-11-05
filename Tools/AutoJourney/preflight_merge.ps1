# Auto_Journey : pre-vol d'une fusion dans main, a lancer AVANT l'agent de fusion. Lecture seule, ne modifie rien.
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\preflight_merge.ps1 -Branch tiers
# Verifie : (1) la branche contient main (sinon refusionner main dans la branche d'abord) ;
# (2) aucun fichier SUIVI modifie non commite de la copie principale ne recoupe les fichiers que la branche change
#     (ce sont eux qui bloquent `git merge --ff-only`) ; liste aussi tous les fichiers sales avec leur heure de modification
#     pour retrouver l'autre session ; (3) le dernier commit de main (une autre session a-t-elle fusionne depuis ?).
# Code de sortie : 0 = l'avance rapide est possible, 1 = bloquee (la sortie dit pourquoi). Jamais de push.
param([Parameter(Mandatory = $true)][string]$Branch, [string]$Main = 'C:\Users\baudo\Documents\MyCrusader')

$ErrorActionPreference = 'Continue'
$ok = $true
Write-Output ("Heure PowerShell : " + (Get-Date -Format 'HH:mm:ss'))
Write-Output ("main   = " + (git -C $Main log -1 '--format=%h %cd %s' '--date=format:%H:%M'))
Write-Output ("{0} = {1}" -f $Branch, (git -C $Main log -1 '--format=%h %cd %s' '--date=format:%H:%M' $Branch))

$behind = [int](git -C $Main rev-list --count "$Branch..main")
if ($behind -gt 0) { Write-Output "(1) REFUSE : $Branch n'a pas les $behind dernier(s) commit(s) de main (git merge main dans la branche, rejouer tests)"; $ok = $false }
else { Write-Output "(1) OK : $Branch contient main" }

$changed = @(git -C $Main diff --name-only "main...$Branch")
$dirty = @(git -C $Main status --porcelain --untracked-files=no | ForEach-Object { $_.Substring(3).Trim('"') })
$blockers = @($dirty | Where-Object { $changed -contains $_ })
if ($blockers.Count -gt 0) {
    Write-Output "(2) REFUSE : fichiers non commites de la copie principale que la branche change (bloquent le fast-forward) :"
    $blockers | ForEach-Object { Write-Output ("    {0}  (modifie {1})" -f $_, (Get-Item (Join-Path $Main $_)).LastWriteTime.ToString('HH:mm')) }
    $ok = $false
} else { Write-Output "(2) OK : aucun fichier sale ne recoupe la branche" }
if ($dirty.Count -gt 0) {
    Write-Output ("    fichiers suivis sales de la copie principale ({0}) :" -f $dirty.Count)
    $dirty | ForEach-Object { Write-Output ("      {0}  {1}" -f (Get-Item (Join-Path $Main $_)).LastWriteTime.ToString('HH:mm'), $_) }
}
$ahead = [int](git -C $Main rev-list --count "main..$Branch")
Write-Output "(3) $Branch a $ahead commit(s) de plus que main"
if ($ok) { Write-Output "RESULTAT : avance rapide possible (git -C $Main merge --ff-only $Branch)"; exit 0 }
Write-Output "RESULTAT : BLOQUE"; exit 1

