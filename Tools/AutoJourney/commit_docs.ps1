# commit_docs.ps1 (L9-5) : commite les SEULS docs de la copie principale (Auto_Journey*.md, EtapeDefini.md, Retours\*, Placeholders*.txt/md, *InProgress.md suivis, Docs\SessionEnd.txt), jamais du code.
# A lancer par la session avant CHAQUE Launch et Merge : des docs non commites de la copie principale ont bloque l'avance rapide deux fois.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier> [-Root C:\Users\baudo\Documents\MyCrusader] [-Message "Docs: ..."] [-DryRun]
# Sortie : la liste des fichiers, puis "commit <sha> : n fichier(s)" ou "rien a committer" ; code 0 ; code 2 si -Root n'est pas un depot git.
# Ne touche ni au code modifie ni aux autres fichiers deja indexes (git commit -- <chemins> ne prend que ces chemins).
# L14-12 (2026-11-14) : -Only "a,b,..." (chemins relatifs a -Root, \ ou /, separes par des virgules : -File passe "a,b" en une seule chaine)
# commite CES docs seuls (une archive de l'agent de docs avait embarque Auto_Journey_Jour.md et des fiches E d'autres sessions) ; un chemin hors
# de la liste des docs = refus (code 3, aucun commit) ; un *InProgress.md neuf NOMME est accepte. Test : commit_docs_test.ps1.
# L29-8 (2026-11-26, SAURON-1126m) : -Lines "<texte>||<texte>" (textes litteraux separes par ||, valable avec -Only d'UN SEUL fichier deja suivi) ne
# commite que les BLOCS modifies (git diff -U0 HEAD) dont une ligne ajoutee ou retiree contient un de ces textes : les lignes des autres sessions restent
# modifiees dans l'arbre, hors du commit (index temporaire : read-tree HEAD + apply --cached du patch filtre + commit-tree + update-ref, puis reset du
# chemin). Aucun texte trouve = code 5, plusieurs fichiers / fichier non suivi = code 4, aucun commit. Sans -Lines : comportement inchange.
# Un commit qui emporte EtapeDefini.md lance d'abord eol_check.ps1 -Root <Root> -Fix (arbre en CRLF, jamais de fins de ligne melees ni de LF).
param(
  [string]$Root = 'C:\Users\baudo\Documents\MyCrusader',
  [string]$Message = '',
  [string[]]$Only = @(),
  [Alias('Lines')][string[]]$LineTexts = @(),
  [switch]$DryRun
)
$ErrorActionPreference = 'Continue'
$inside = (& git.exe -C $Root rev-parse --is-inside-work-tree 2>$null | Out-String).Trim()
if ($inside -ne 'true') { Write-Output "ERREUR : $Root n'est pas un depot git"; exit 2 }
# L10-tool (2026-11-11) : + Placeholders*.txt/md, *InProgress.md SUIVIS (un *InProgress.md neuf non suivi reste hors du commit), Docs/SessionEnd.txt (meme liste que $mdocOk de session_gate -Phase Merge)
$Specs = @(':(glob)Auto_Journey*.md', 'EtapeDefini.md', 'Retours', ':(glob)Placeholders*.txt', ':(glob)Placeholders*.md', ':(glob)*InProgress.md', 'Docs/SessionEnd.txt')
$UntrackedSkip = '(^|/)[^/]*InProgress\.md$'
$Only = @($Only | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim().Replace('\', '/') } | Where-Object { $_ })
$LineTexts = @($LineTexts | ForEach-Object { $_ -split '\|\|' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
if ($LineTexts.Count -gt 0 -and $Only.Count -ne 1) { Write-Output 'ERREUR : -Lines demande -Only avec UN SEUL fichier'; exit 4 }
if ($Only.Count -gt 0) {
  $DocRe = '^(Auto_Journey[^/]*\.md|EtapeDefini\.md|Retours/.+|Placeholders[^/]*\.(txt|md)|[^/]*InProgress\.md|Docs/SessionEnd\.txt|Docs/Archive/[^/]+\.(md|txt)|Docs/Priorite_[^/]+\.md)$'   # = $Specs + Docs/Priorite_*.md (survey des priorites, nomme seulement ; SAURON-1124si)
  $bad = @($Only | Where-Object { $_ -notmatch $DocRe })
  if ($bad.Count -gt 0) { Write-Output ("ERREUR : -Only hors des docs (jamais du code ni un autre fichier) : " + ($bad -join ', ')); exit 3 }
  $Specs = @($Only | ForEach-Object { ':(literal)' + $_ })
  $UntrackedSkip = '^$never'
}
$lines = @(& git.exe -C $Root -c core.quotepath=false status --porcelain -uall -- @Specs 2>$null | Where-Object { $_ } | Where-Object { -not ($_ -match '^\?\? ' -and $_.Substring(3).Trim('"') -match $UntrackedSkip) })
if ($lines.Count -eq 0) { Write-Output 'rien a committer (aucun doc modifie)'; exit 0 }
foreach ($l in $lines) { Write-Output ("  " + $l) }
if ($DryRun) { Write-Output ("DryRun : {0} fichier(s) seraient commites" -f $lines.Count); exit 0 }
# L29-8 : EtapeDefini.md passe par eol_check -Fix avant tout add (un LF seul y est reparti dans un commit de docs)
if (@($lines | Where-Object { $_ -match '(^|[ /])EtapeDefini\.md$' }).Count -gt 0) { & (Join-Path $PSScriptRoot 'eol_check.ps1') -Root $Root -Fix | Out-Null }
if ($LineTexts.Count -gt 0) {
  $f = $Only[0]
  & git.exe -C $Root cat-file -e ("HEAD:" + $f) 2>$null
  if ($LASTEXITCODE -ne 0) { Write-Output ("ERREUR : -Lines demande un fichier deja suivi (absent de HEAD) : " + $f); exit 4 }
  $latin = [Text.Encoding]::GetEncoding(28591)   # octets 1 pour 1 : le patch garde ses octets UTF-8 intacts
  $needles = @($LineTexts | ForEach-Object { $latin.GetString([Text.Encoding]::UTF8.GetBytes($_)) })
  $stamp = [guid]::NewGuid().ToString('N').Substring(0, 8)
  $patchFile = Join-Path ([IO.Path]::GetTempPath()) ("commit_docs_$stamp.patch")
  $idxFile = Join-Path ([IO.Path]::GetTempPath()) ("commit_docs_$stamp.index")
  & git.exe -C $Root -c core.quotepath=false diff -U0 --no-color --no-renames "--output=$patchFile" HEAD -- $f 2>$null
  $raw = if (Test-Path $patchFile) { $latin.GetString([IO.File]::ReadAllBytes($patchFile)) } else { '' }
  $head = New-Object System.Collections.Generic.List[string]; $hunks = New-Object System.Collections.Generic.List[object]; $cur = $null
  foreach ($l in ($raw -split "`n")) {
    if ($l -like '@@ *') { $cur = New-Object System.Collections.Generic.List[string]; $hunks.Add($cur) }
    if ($null -ne $cur) { $cur.Add($l) } else { $head.Add($l) }
  }
  $keep = @($hunks | Where-Object {
    $hit = $false
    foreach ($hl in ($_ | Select-Object -Skip 1)) { if ($hl -match '^[+-]') { $txt = $hl.Substring(1); foreach ($nd in $needles) { if ($txt.Contains($nd)) { $hit = $true } } } }
    $hit
  })
  if ($keep.Count -eq 0) { Remove-Item $patchFile -ErrorAction SilentlyContinue; Write-Output ('ERREUR : aucun bloc modifie de ' + $f + ' ne contient un texte de -Lines : rien commite'); exit 5 }
  $body = (@($head | Where-Object { $_ -ne '' }) + @($keep | ForEach-Object { $_ }) | Where-Object { $_ -ne '' }) -join "`n"
  [IO.File]::WriteAllBytes($patchFile, $latin.GetBytes($body + "`n"))
  $oldIdx = $env:GIT_INDEX_FILE
  try {
    $env:GIT_INDEX_FILE = $idxFile
    & git.exe -C $Root read-tree HEAD 2>&1 | Out-Null
    $ap = & git.exe -C $Root apply --cached --unidiff-zero $patchFile 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0) { Write-Output ("ERREUR : git apply du patch filtre a echoue : " + $ap.Trim()); exit 5 }
    $tree = (& git.exe -C $Root write-tree 2>$null | Out-String).Trim()
  } finally { if ($null -eq $oldIdx) { Remove-Item Env:GIT_INDEX_FILE -ErrorAction SilentlyContinue } else { $env:GIT_INDEX_FILE = $oldIdx }; Remove-Item $idxFile -ErrorAction SilentlyContinue }
  $parent = (& git.exe -C $Root rev-parse HEAD 2>$null | Out-String).Trim()
  if (-not $Message) { $Message = 'Docs: session docs ' + (Get-Date -Format 'yyyy-MM-dd HH:mm') }
  $newSha = (& git.exe -C $Root commit-tree $tree -p $parent -m $Message 2>$null | Out-String).Trim()
  & git.exe -C $Root update-ref HEAD $newSha $parent 2>&1 | Out-Null
  & git.exe -C $Root reset -q HEAD -- $f 2>&1 | Out-Null   # l'index du chemin suit le nouveau HEAD ; l'arbre garde les lignes des autres sessions
  Remove-Item $patchFile -ErrorAction SilentlyContinue
  Write-Output ('commit {0} : 1 fichier(s), {1} bloc(s) sur {2} (-Lines)' -f (& git.exe -C $Root rev-parse --short HEAD 2>$null | Out-String).Trim(), $keep.Count, $hunks.Count)
  exit 0
}
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
