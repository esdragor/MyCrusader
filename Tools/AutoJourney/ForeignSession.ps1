# ForeignSession.ps1 (L14-3, 2026-11-14) : a dot-sourcer (sync_worktrees.ps1, status.ps1).
# 2026-11-13 : une AUTRE session de l'utilisateur commitait dans le worktree Arwen (d751-root, "Faction choice", 19:11-19:17) ;
# sync_worktrees y a fusionne main et la recette a conclu a tort. Un worktree est "d'une autre session" quand il a un commit
# (hors fusion) de moins de $Hours heures absent de main, ET que son nom court (HeroesOfLiberty-<Nom> -> <Nom>) ou sa branche
# n'apparait dans aucune section '## Agents du <aujourd'hui ou hier>' d'Auto_Journey_Jour.md (hier : seance passee minuit).

# Texte des sections '## Agents du <date>' d'aujourd'hui et d'hier (vide si le fichier manque).
function Get-SessionAgentsText([string]$JourFile) {
    if (-not $JourFile -or -not (Test-Path -LiteralPath $JourFile)) { return '' }
    $dates = @((Get-Date).ToString('yyyy-MM-dd'), (Get-Date).AddDays(-1).ToString('yyyy-MM-dd'))
    $out = New-Object System.Text.StringBuilder
    $in = $false
    foreach ($l in (Get-Content -LiteralPath $JourFile -Encoding UTF8)) {
        if ($l -match '^## ') { $in = $false; foreach ($d in $dates) { if ($l -match ('^## Agents du ' + [regex]::Escape($d))) { $in = $true } } }
        if ($in) { [void]$out.AppendLine($l) }
    }
    $out.ToString()
}

# Renvoie $null si le worktree est de la seance (ou sans commit recent hors de main), sinon une phrase qui nomme le commit.
function Get-ForeignSessionNote([string]$Dir, [string]$JourFile, [double]$Hours = 2) {
    $since = [int64]([DateTimeOffset]::Now.AddHours(-$Hours).ToUnixTimeSeconds())
    $recent = @(git -C $Dir log --no-merges --format='%ct|%h|%s' 'main..HEAD' 2>$null | Where-Object { $_ -and [int64](($_ -split '\|')[0]) -ge $since })
    if ($recent.Count -eq 0) { return $null }
    $short = (Split-Path $Dir -Leaf) -replace '^HeroesOfLiberty-', ''
    $branch = (git -C $Dir rev-parse --abbrev-ref HEAD 2>$null)
    $agents = Get-SessionAgentsText $JourFile
    foreach ($n in @($short, $branch)) { if ($n -and $agents -match ('(?i)(^|[^A-Za-z0-9_])' + [regex]::Escape($n) + '($|[^A-Za-z0-9_])')) { return $null } }
    $p = $recent[0] -split '\|', 3
    $at = [DateTimeOffset]::FromUnixTimeSeconds([int64]$p[0]).LocalDateTime.ToString('HH:mm')
    return ("autre session ? {0} commit(s) de moins de {1} h hors de main (dernier {2} a {3} : {4}) et worktree absent des sections '## Agents du' d'aujourd'hui/hier ({5})" -f $recent.Count, $Hours, $p[1], $at, $p[2], $(if ($JourFile) { Split-Path $JourFile -Leaf } else { 'pas de fichier' }))
}
