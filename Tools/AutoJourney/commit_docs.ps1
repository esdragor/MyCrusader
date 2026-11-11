# commit_docs.ps1 (L9-5) : commite les SEULS docs de la copie principale (Auto_Journey*.md, EtapeDefini.md, Retours\*, Placeholders*.txt/md, *InProgress.md suivis, Docs\SessionEnd.txt), jamais du code.
# A lancer par la session avant CHAQUE Launch et Merge : des docs non commites de la copie principale ont bloque l'avance rapide deux fois.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier> [-Root C:\Users\baudo\Documents\MyCrusader] [-Message "Docs: ..."] [-DryRun]
# Sortie : la liste des fichiers, puis "commit <sha> : n fichier(s)" ou "rien a committer" ; code 0 ; code 2 si -Root n'est pas un depot git.
# Ne touche ni au code modifie ni aux autres fichiers deja indexes (git commit -- <chemins> ne prend que ces chemins).
param(
  [string]$Root = 'C:\Users\baudo\Documents\MyCrusader',
  [string]$Message = '',
  [switch]$DryRun
)
$ErrorActionPreference = 'Continue'
$inside = (& git.exe -C $Root rev-parse --is-inside-work-tree 2>$null | Out-String).Trim()
if ($inside -ne 'true') { Write-Output "ERREUR : $Root n'est pas un depot git"; exit 2 }
# L10-tool (2026-11-11) : + Placeholders*.txt/md, *InProgress.md SUIVIS (un *InProgress.md neuf non suivi reste hors du commit), Docs/SessionEnd.txt (meme liste que $mdocOk de session_gate -Phase Merge)
$Specs = @(':(glob)Auto_Journey*.md', 'EtapeDefini.md', 'Retours', ':(glob)Placeholders*.txt', ':(glob)Placeholders*.md', ':(glob)*InProgress.md', 'Docs/SessionEnd.txt')
$UntrackedSkip = '(^|/)[^/]*InProgress\.md$'
$lines = @(& git.exe -C $Root -c core.quotepath=false status --porcelain -uall -- @Specs 2>$null | Where-Object { $_ } | Where-Object { -not ($_ -match '^\?\? ' -and $_.Substring(3).Trim('"') -match $UntrackedSkip) })
if ($lines.Count -eq 0) { Write-Output 'rien a committer (aucun doc modifie)'; exit 0 }
foreach ($l in $lines) { Write-Output ("  " + $l) }
if ($DryRun) { Write-Output ("DryRun : {0} fichier(s) seraient commites" -f $lines.Count); exit 0 }
# Chemins EXPLICITES (un dossier comme pathspec de "git commit --" omet les fichiers SUPPRIMES : bug L9-5 trouve par commit_docs_test)
$tracked = @(& git.exe -C $Root -c core.quotepath=false diff --name-only --no-renames HEAD -- @Specs 2>$null | Where-Object { $_ })
$new = @(& git.exe -C $Root -c core.quotepath=false ls-files --others --exclude-standard -- @Specs 2>$null | Where-Object { $_ -and $_ -notmatch $UntrackedSkip })
$paths = @($tracked + $new | Sort-Object -Unique)
& git.exe -C $Root add -A -- @paths 2>&1 | Out-Null
if (-not $Message) { $Message = 'Docs: session docs ' + (Get-Date -Format 'yyyy-MM-dd HH:mm') }
& git.exe -C $Root commit -q -m $Message -- @paths 2>&1 | Out-Null
$sha = (& git.exe -C $Root rev-parse --short HEAD 2>$null | Out-String).Trim()
Write-Output ("commit {0} : {1} fichier(s)" -f $sha, $lines.Count)
exit 0
