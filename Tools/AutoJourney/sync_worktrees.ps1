# Auto_Journey : fin de seance, chaque worktree recupere main dans sa branche (git merge main).
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\sync_worktrees.ps1 [-DryRun]
# Regles : jamais de push ; un worktree avec des fichiers SUIVIS modifies (travail non commite) est saute et signale ;
# un conflit annule la fusion (merge --abort) et est signale ; un worktree deja a jour est ignore ; main lui-meme est ignore.
# Sortie : une ligne par worktree : nom | branche | resultat. Code de sortie 0 meme avec des sauts (lire la sortie).
param([string]$Docs = 'C:\Users\baudo\Documents', [switch]$DryRun)

$main = Join-Path $Docs 'MyCrusader'
$dirs = @(Get-ChildItem $Docs -Directory -Filter 'HeroesOfLiberty-*' | ForEach-Object { $_.FullName })
Write-Output ("Heure PowerShell : " + (Get-Date -Format 'HH:mm') + "  main = " + (git -C $main rev-parse --short HEAD))
foreach ($d in $dirs) {
    $name = Split-Path $d -Leaf
    $branch = (git -C $d rev-parse --abbrev-ref HEAD 2>$null)
    if (-not $branch) { Write-Output "$name | ? | pas un worktree git"; continue }
    $dirty = @(git -C $d status --porcelain --untracked-files=no 2>$null | Where-Object { $_ -notmatch 'IN_USE\.txt' -and $_ -notmatch 'HeroesOfLiberty\.uproject' })
    if ($dirty.Count -gt 0) { Write-Output ("{0} | {1} | SAUTE : {2} fichier(s) suivi(s) modifie(s) non commite(s)" -f $name, $branch, $dirty.Count); continue }
    $behind = [int]((git -C $d rev-list --count 'HEAD..main' 2>$null))
    if ($behind -eq 0) { Write-Output "$name | $branch | deja a jour"; continue }
    if ($DryRun) { Write-Output "$name | $branch | retard de $behind commit(s) : a fusionner"; continue }
    $uproj = @(git -C $d status --porcelain --untracked-files=no 2>$null | Where-Object { $_ -match 'HeroesOfLiberty\.uproject' })
    if ($uproj.Count -gt 0) { git -C $d checkout -- HeroesOfLiberty.uproject 2>$null }
    $null = git -C $d merge main -m "Merge branch 'main' into $branch" 2>&1
    $code = $LASTEXITCODE
    $unmerged = @(git -C $d diff --name-only --diff-filter=U 2>$null)
    if ($unmerged.Count -gt 0) {
        git -C $d merge --abort 2>$null
        Write-Output ("{0} | {1} | CONFLIT annule ({2} fichier(s) : {3})" -f $name, $branch, $unmerged.Count, ($unmerged | Select-Object -First 3) -join ', ')
        continue
    }
    $now = [int]((git -C $d rev-list --count 'HEAD..main' 2>$null))
    if ($now -eq 0) { Write-Output "$name | $branch | mis a jour (+$behind de main) = $(git -C $d rev-parse --short HEAD)" }
    else { Write-Output "$name | $branch | ECHEC fusion (code $code), retard restant $now" }
}
