# Auto_Journey : etat de tous les worktrees en une commande (etape 0 et veilleur).
# Lecture seule. Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\status.ps1
# Sortie : une ligne par worktree : nom | branche | libre/pris | fichiers modifies | avance/retard sur main | dernier commit.
# "libre" = la premiere ligne d'IN_USE.txt commence par Libre ; sinon "pris" (a reprendre sans ecraser, voir Auto_Journey.md).
# L14-3 (2026-11-14) : "AUTRE SESSION ?" en fin de ligne = commit de moins de 2 h hors de main, worktree absent de '## Agents du <date>'
#   d'Auto_Journey_Jour.md (ForeignSession.ps1) : ne pas y fusionner ni y conclure. -InMain <sha> : dit si ce commit est DANS main ou
#   ABSENT de main par `git merge-base --is-ancestor` (a lancer AVANT d'ecrire "absent de main" dans une recette).
param([string]$Docs = 'C:\Users\baudo\Documents', [string]$JourFile = '', [string]$InMain = '')

$main = Join-Path $Docs 'MyCrusader'
if (-not $JourFile) { $JourFile = Join-Path $main 'Auto_Journey_Jour.md' }
. (Join-Path $PSScriptRoot 'ForeignSession.ps1')
if ($InMain) {
    $null = git -C $main rev-parse --verify --quiet "$InMain^{commit}" 2>$null
    if ($LASTEXITCODE -ne 0) { Write-Output "$InMain : INCONNU (pas un commit de ce depot)"; exit 1 }
    git -C $main merge-base --is-ancestor $InMain main 2>$null
    if ($LASTEXITCODE -eq 0) { Write-Output "$InMain : DANS main (git merge-base --is-ancestor $InMain main)" } else { Write-Output "$InMain : ABSENT de main (git merge-base --is-ancestor $InMain main : faux)" }
    exit 0
}
$dirs = @($main) + @(Get-ChildItem $Docs -Directory -Filter 'HeroesOfLiberty-*' | ForEach-Object { $_.FullName })
Write-Output ("Heure PowerShell : " + (Get-Date -Format 'HH:mm'))
foreach ($d in $dirs) {
    $name = Split-Path $d -Leaf
    if ($d -eq $main) { $name = 'MyCrusader(main)' }
    $branch = (git -C $d rev-parse --abbrev-ref HEAD 2>$null)
    $dirty = @(git -C $d status --short 2>$null | Where-Object { $_ -notmatch 'IN_USE\.txt' }).Count
    $lr = (git -C $d rev-list --left-right --count 'main...HEAD' 2>$null)
    $behind = ''; $ahead = ''
    if ($lr) { $parts = $lr -split '\s+'; $behind = $parts[0]; $ahead = $parts[1] }
    $last = (git -C $d log -1 --format='%h %ar %s' 2>$null)
    if ($last.Length -gt 80) { $last = $last.Substring(0, 80) }
    $use = ''
    $f = Join-Path $d 'IN_USE.txt'
    if (Test-Path $f) {
        $use = (Get-Content $f -TotalCount 1 -Encoding UTF8)
        $state = if ($use -match '^\s*Libre') { 'libre' } else { 'pris' }
        $age = [int]((Get-Date) - (Get-Item $f).LastWriteTime).TotalMinutes
        $state = "$state(IN_USE il y a ${age} min)"
    } else { $state = 'sans IN_USE' }
    $foreign = if ($d -ne $main) { Get-ForeignSessionNote $d $JourFile } else { $null }
    Write-Output (("{0} | {1} | {2} | modifies {3} | +{4}/-{5} vs main | {6}" -f $name, $branch, $state, $dirty, $ahead, $behind, $last) + $(if ($foreign) { " | AUTRE SESSION ? ($foreign)" } else { '' }))
}
# L91 : builds marques BUILD LENT par Tools\build.ps1 (colonne load de Docs\BuildQueue.csv) dans les 2 dernieres heures.
$csv = Join-Path $main 'Docs\BuildQueue.csv'
if (Test-Path $csv) {
    $since = (Get-Date).AddHours(-2)
    $slow = @(Import-Csv $csv | Where-Object { $_.load -match 'BUILD LENT' -and $_.end -and ([datetime] $_.end) -ge $since })
    foreach ($r in $slow) { Write-Output ("BUILD LENT : {0} fini a {1} : {2} (diagnostic de charge, prevenir l'utilisateur)" -f $r.worktree, $r.end, $r.load) }
}
