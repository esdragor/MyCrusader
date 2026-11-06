# session_gate.ps1 : les controles mecaniques d'une seance Auto_Journey (lecture seule, ne modifie rien).
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\session_gate.ps1 -Phase Launch -End 13:30
#   -Phase Launch : avant CHAQUE lancement d'agent (heure, temps restant, agents C++, tableau des agents, docs non suivis)
#   -Phase Merge  : avant l'agent de fusion (pre-vol de chaque branche : -Branches ai-ia1,fog,...)
#   -Phase Close  : avant le bilan (worktrees propres et a jour, reponses non traitees, bilan des lecons)
# Sortie : une ligne PASS / WARN / FAIL par controle, code de sortie 1 s'il y a un FAIL.
#   -Phase Essay  : avant de donner une commande play.ps1 a l'utilisateur (-Root <worktree> : contient main, propre, build plus recent que le dernier commit de code)
#   -Phase Report : a la RECEPTION du rapport de chaque agent (-Root <worktree de l'agent>) : la copie principale n'a que des docs voulus modifies, le worktree contient main, est propre, et son binaire n'est pas plus vieux que le dernier code (sinon build.ps1 AVANT tout quick_check : binaire perime = faux rouges)
#   Launch verifie aussi la file : au moins 3 lignes '[ ]' etiquetees 'sans C++' dans Auto_Journey_File.md (de quoi relancer sans toucher au plafond de 3 agents C++)
param(
  [Parameter(Mandatory=$true)][ValidateSet('Launch','Merge','Close','Essay','Report')][string]$Phase,
  [string]$End = '13:30',
  [string[]]$Branches = @(),
  [string]$Root = '',
  [switch]$Scenarios,                      # Launch : la tache de l'agent joue des scenarios (donc exige un binaire du worktree a jour, -Root)
  [string[]]$Exclude = @(),                # Close : worktrees hors perimetre (nom court 'Haldir' ou complet 'HeroesOfLiberty-Haldir')
  [string]$RetoursDir = '',                # Close : dossier des Retours\ (defaut : la copie principale)
  [int]$MinMinutes = 5   # 2026-11-05 : l'utilisateur prefere finir 15 min APRES l'heure que 15 min avant (sauf consigne contraire dans le prompt de lancement) ; l'ancien seuil de 20 min laissait la seance vide
)
$ErrorActionPreference = 'Continue'
$Branches = @($Branches | ForEach-Object { $_ -split ',' } | Where-Object { $_ })   # -File passe "a,b" comme une seule chaine
$Main = 'C:\Users\baudo\Documents\MyCrusader'
$Exclude = @($Exclude | ForEach-Object { $_ -split ',' } | Where-Object { $_ } | ForEach-Object { $_ -replace '^HeroesOfLiberty-', '' })
if (-not $RetoursDir) { $RetoursDir = "$Main\Retours" }
$Fails = 0
function Say([string]$Level, [string]$Msg) { if ($Level -eq 'FAIL') { $script:Fails++ }; Write-Output ("{0,-4} {1}" -f $Level, $Msg) }

$now = Get-Date
$endAt = [datetime]::ParseExact($now.ToString('yyyy-MM-dd') + ' ' + $End, 'yyyy-MM-dd HH:mm', $null)
$left = [int]($endAt - $now).TotalMinutes
Write-Output ("Heure PowerShell : {0}  fin dure {1}  reste {2} min" -f $now.ToString('HH:mm:ss'), $End, $left)

$wts = Get-ChildItem 'C:\Users\baudo\Documents' -Directory -Filter 'HeroesOfLiberty-*' | Select-Object -ExpandProperty FullName

if ($Phase -eq 'Launch') {
  if ($left -lt $MinMinutes) { Say 'FAIL' ("il reste {0} min (< {1}) : AUCUN lancement d'agent" -f $left, $MinMinutes) }
  else { Say 'PASS' ("temps restant {0} min ; arret dur des agents = {1} (jusqu'a +15 min toleres, jamais 15 min trop tot)" -f $left, $endAt.ToString('HH:mm')) }

  # L8-9 : au DEBUT de chaque Auto_Journey tous les worktrees sont remis a jour (sync_worktrees.ps1) ; WARN par worktree en retard (les agents deja lances en cours de seance retardent apres chaque fusion : normal)
  foreach ($w in $wts) {
    $n9 = Split-Path $w -Leaf
    if ($Exclude -contains ($n9 -replace '^HeroesOfLiberty-', '')) { continue }
    $b9 = [int](git -C $w rev-list --count 'HEAD..main' 2>$null)
    if ($b9 -gt 0) { Say 'WARN' "$n9 : retard de $b9 commit(s) sur main : au debut de la seance lancer Tools\AutoJourney\sync_worktrees.ps1 AVANT le premier lancement" }
  }

  $q = @(Get-ChildItem "$Main\Docs\BuildQueue.state" -Filter '*.json' -ErrorAction SilentlyContinue).Count
  if ($q -ge 3) { Say 'FAIL' "$q builds dans la file : plafond de 3 agents C++ atteint, le nouvel agent demarre SANS C++" } else { Say 'PASS' "$q build(s) dans la file (< 3)" }

  $jour = Get-Content "$Main\Auto_Journey_Jour.md" -Raw -ErrorAction SilentlyContinue
  $marker = '## Agents du ' + $now.ToString('yyyy-MM-dd')
  if ($jour -and $jour.Contains($marker)) { Say 'PASS' "tableau agent -> id -> worktree present ($marker)" }
  else { Say 'FAIL' "pas de tableau '$marker' dans Auto_Journey_Jour.md : l'ecrire AVANT de lancer ou d'envoyer un message (colonnes : nom, id, worktree, branche, tache, heure de lancement)" }

  $st = git -C $Main status --short 2>$null
  $untracked = @($st | Where-Object { $_ -match '^\?\? .*InProgress\.md' })
  if ($untracked.Count -gt 0) { Say 'FAIL' ("dossiers non suivis dans la copie principale (bloquent la fusion) : " + (($untracked | ForEach-Object { $_.Substring(3) }) -join ', ') + ' -> demander l''accord de les commiter') } else { Say 'PASS' 'aucun *InProgress.md non suivi' }
  $dirty = @($st | Where-Object { $_ -match '^ ?M ' })
  if ($dirty.Count -gt 0) { Say 'WARN' ("fichiers suivis modifies dans la copie principale : " + (($dirty | ForEach-Object { $_.Substring(3) }) -join ', ')) } else { Say 'PASS' 'copie principale propre' }
  $file = Get-Content "$Main\Auto_Journey_File.md" -ErrorAction SilentlyContinue
  $noCpp = @($file | Where-Object { $_ -match '^\s*- \[ \]' -and $_ -match 'sans C\+\+' }).Count
  if ($noCpp -ge 3) { Say 'PASS' "$noCpp ligne(s) '[ ]' sans C++ en file (relance possible sans toucher au plafond)" }
  else { Say 'WARN' "seulement $noCpp ligne(s) '[ ]' etiquetee(s) 'sans C++' dans Auto_Journey_File.md (< 3) : en ecrire avant de lancer, sinon les agents finis ne pourront pas etre relances (2026-11-05 : 30 min vides)" }

  # L7-2 : la DLL du worktree de l'agent doit etre plus recente que le dernier commit de code de main (sinon faux rouges, appels perdus)
  if (-not $Root -or -not (Test-Path $Root)) {
    if ($Scenarios) { Say 'FAIL' "tache avec scenarios : donner -Root <worktree de l'agent> pour verifier sa DLL (-Root absent ou introuvable)" }
  } else {
    $lastMain = git -C $Main log -1 --format=%ct main -- Source Script Content Tools/Data 2>$null
    $bin = Get-ChildItem (Join-Path $Root 'Binaries\Win64') -Filter '*.dll' -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $bin -or -not $lastMain) { Say $(if ($Scenarios) { 'FAIL' } else { 'WARN' }) "DLL du worktree ou commit de code de main introuvable ($Root) : build.ps1 avant de lancer" }
    else {
      $codeTime = [DateTimeOffset]::FromUnixTimeSeconds([int64]$lastMain).LocalDateTime
      if ($bin.LastWriteTime -ge $codeTime) { Say 'PASS' ("DLL du worktree du {0} >= dernier commit de code de main du {1}" -f $bin.LastWriteTime.ToString('MM-dd HH:mm'), $codeTime.ToString('MM-dd HH:mm')) }
      else { Say $(if ($Scenarios) { 'FAIL' } else { 'WARN' }) ("DLL du worktree du {0} plus ANCIENNE que le dernier commit de code de main du {1} : build.ps1 (sync) AVANT de lancer un agent qui joue des scenarios" -f $bin.LastWriteTime.ToString('MM-dd HH:mm'), $codeTime.ToString('MM-dd HH:mm')) }
    }
  }
}

if ($Phase -eq 'Report') {
  $docOk = '^(Auto_Journey.*\.md|EtapeDefini\.md|GDD\.md|[A-Za-z0-9_]+InProgress\.md|Retours/.*)$'
  $bad = @(git -C $Main status --short 2>$null | Where-Object { $_ -match '^ ?[MADR]' } | ForEach-Object { $_.Substring(3).Trim('"') } | Where-Object { $_ -notmatch $docOk })
  if ($bad.Count -gt 0) { Say 'FAIL' ("la copie principale porte des fichiers modifies hors docs voulus (ecriture au mauvais endroit ?) : " + ($bad -join ', ')) } else { Say 'PASS' 'copie principale : seulement des docs voulus' }
  if (-not $Root -or -not (Test-Path $Root)) { Say 'FAIL' 'donner -Root <worktree de l agent>' }
  else {
    $dirty = @(git -C $Root status --short 2>$null | Where-Object { $_ -notmatch 'IN_USE\.txt' -and $_ -match '^ ?[MADR]' })
    if ($dirty.Count -gt 0) { Say 'FAIL' "$Root : $($dirty.Count) fichier(s) suivi(s) non commite(s)" } else { Say 'PASS' "$Root : propre" }
    $behind = [int](git -C $Root rev-list --count 'HEAD..main' 2>$null)
    if ($behind -gt 0) { Say 'WARN' "$Root : retard de $behind commit(s) sur main (git merge main avant le rapport)" } else { Say 'PASS' 'contient main' }
    # L7-4 : aucun processus detache du worktree (chain/loop/UnrealEditor) ; un build en cours est liste mais ne compte pas
    $stray = & powershell -NoProfile -ExecutionPolicy Bypass -File "$PSScriptRoot\stray_processes.ps1" -Root $Root 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0) { Say 'WARN' ("processus detaches restants pour $Root (stray_processes.ps1 -Root ... -Stop) : " + (($stray -split "`n" | Where-Object { $_ -match 'PID' } | Select-Object -First 3) -join ' | ')) } else { Say 'PASS' 'aucun processus detache' }
    $lastCode = git -C $Root log -1 --format=%ct -- Source Script Content Tools/Data 2>$null
    $bin = Get-ChildItem (Join-Path $Root 'Binaries\Win64') -Filter '*.dll' -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($bin -and $lastCode) {
      $codeTime = [DateTimeOffset]::FromUnixTimeSeconds([int64]$lastCode).LocalDateTime
      if ($bin.LastWriteTime -ge $codeTime) { Say 'PASS' ("binaire du {0} >= dernier code du {1}" -f $bin.LastWriteTime.ToString('HH:mm'), $codeTime.ToString('HH:mm')) }
      else { Say 'WARN' ("binaire du {0} plus ANCIEN que le dernier code du {1} : build.ps1 AVANT tout quick_check (sinon faux rouges CommandGrid/Technologies/Map.Table)" -f $bin.LastWriteTime.ToString('HH:mm'), $codeTime.ToString('HH:mm')) }
    } else { Say 'WARN' 'binaire ou commit de code introuvable' }
  }
}

if ($Phase -eq 'Merge') {
  if ($Branches.Count -eq 0) { Say 'FAIL' 'donner -Branches <b1,b2,...>' }
  foreach ($b in $Branches) {
    $out = & powershell -ExecutionPolicy Bypass -File "$Main\Tools\AutoJourney\preflight_merge.ps1" -Branch $b 2>&1 | Out-String
    if ($out -match 'REFUSE|BLOQUE') { Say 'FAIL' ("$b : " + (($out -split "`n" | Where-Object { $_ -match 'REFUSE|BLOQUE' } | Select-Object -First 1).Trim())) } else { Say 'PASS' "$b : pre-vol de fusion OK" }
  }
  Say 'WARN' "rappel : rien n'entre dans main sans build.ps1 PASS ET quick_check -Parts tests tout vert SUR LE RESULTAT DE LA FUSION ; un .uasset en conflit se REGENERE, jamais 'theirs'"
}

if ($Phase -eq 'Essay') {
  if (-not $Root -or -not (Test-Path $Root)) { Say 'FAIL' 'donner -Root <worktree qui porte le build>' }
  else {
    $mainHead = (git -C $Main rev-parse HEAD 2>$null)
    $codeDiff = @(git -C $Root diff --name-only $mainHead HEAD -- Source Script Content Tools/Data Config 2>$null)
    if ($codeDiff.Count -eq 0) { Say 'PASS' "$Root : aucun ecart de code avec main ($($mainHead.Substring(0,7)))" } else { Say 'FAIL' "$Root : $($codeDiff.Count) fichier(s) de code different de main (ex. $($codeDiff[0])) : l'essai ne compte pas comme integre (git merge main + build)" }
    $dirty = @(git -C $Root status --short 2>$null | Where-Object { $_ -notmatch 'IN_USE\.txt' -and $_ -match '^ ?[MADR]' })
    if ($dirty.Count -gt 0) { Say 'FAIL' "$Root : $($dirty.Count) fichier(s) non commite(s) : le build ne correspond pas a un commit" } else { Say 'PASS' 'worktree propre' }
    $lastCode = git -C $Root log -1 --format=%ct -- Source Config 2>$null  # the DLL only goes stale with C++ (Source) or Config changes; a Script/ (Angelscript) or Content commit never needs a DLL rebuild (false FAIL seen 2026-11-06 with the audio .as commit)
    $bin = Get-ChildItem (Join-Path $Root 'Binaries\Win64') -Filter '*.dll' -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($bin -and $lastCode) {
      $codeTime = [DateTimeOffset]::FromUnixTimeSeconds([int64]$lastCode).LocalDateTime
      if ($bin.LastWriteTime -ge $codeTime) { Say 'PASS' ("binaire du {0} >= dernier commit de code du {1}" -f $bin.LastWriteTime.ToString('HH:mm'), $codeTime.ToString('HH:mm')) }
      else { Say 'FAIL' ("binaire du {0} PLUS ANCIEN que le dernier commit de code du {1} : rebuild (build.ps1) avant l'essai" -f $bin.LastWriteTime.ToString('HH:mm'), $codeTime.ToString('HH:mm')) }
    } else { Say 'WARN' 'binaire ou commit de code introuvable : verifier le build a la main' }
    Say 'WARN' "rappel : l'essai d'un REGLAGE/BUG est rejoue par l'integrateur (geste reel + PNG avant/apres) AVANT cette commande"
  }
}

if ($Phase -eq 'Close') {
  foreach ($w in $wts) {
    $n = Split-Path $w -Leaf
    if ($Exclude -contains ($n -replace '^HeroesOfLiberty-', '')) { Say 'WARN' "$n : exclu du perimetre (-Exclude), non controle" ; continue }
    $dirty = @(git -C $w status --short 2>$null | Where-Object { $_ -notmatch 'IN_USE\.txt' -and $_ -match '^ ?[MADR]' })
    if ($dirty.Count -gt 0) { Say 'FAIL' "$n : $($dirty.Count) fichier(s) suivi(s) NON commite(s) (commiter sur la branche, jamais laisser en l'air)" } else { Say 'PASS' "$n : propre" }
    $behind = [int](git -C $w rev-list --count 'HEAD..main' 2>$null)
    if ($behind -gt 0) { Say 'FAIL' "$n : retard de $behind commit(s) sur main : L8-9, lancer Tools\AutoJourney\sync_worktrees.ps1 -NoBuild (ou avec build) avant le bilan, aucun agent ne tourne" } else { Say 'PASS' "$n : a jour avec main" }
  }
  $dm = @(git -C $Main status --short 2>$null | Where-Object { $_ -match '^ ?[MADR]' })
  if ($dm.Count -gt 0) { Say 'FAIL' ("copie principale : fichiers suivis non commites : " + (($dm | ForEach-Object { $_.Substring(3) }) -join ', ')) } else { Say 'PASS' 'copie principale propre' }
  # L7-6 : chaque E-*.md ouvert ([... ESSAYER]) porte GESTE: reel|interne et SON: entendu|non entendu
  foreach ($e in @(Get-ChildItem $RetoursDir -Filter 'E-*.md' -ErrorAction SilentlyContinue)) {
    $txt = Get-Content $e.FullName -Raw -Encoding UTF8
    if ($txt -notmatch '(?m)^#.*ESSAYER\]') { continue }
    if ($txt -match '(?m)^\s*\**R[^\s:]*ponse[^:\n]*:\**\s*\S') { continue }   # deja repondu par l'utilisateur : plus a corriger
    $miss = @()
    if ($txt -notmatch '(?m)^\s*\**GESTE\s*:\**\s*(r.el|interne)') { $miss += 'GESTE: reel|interne' }
    if ($txt -notmatch '(?m)^\s*\**SON\s*:\**\s*(entendu|non entendu|sans objet)') { $miss += 'SON: entendu|non entendu|sans objet' }
    if ($miss.Count -gt 0) { Say 'WARN' ("Retours\{0} : ligne(s) de statut manquante(s) : {1} (gabarit Tools\AutoJourney\E_template.md)" -f $e.Name, ($miss -join ' ; ')) }
  }
  $ret = @(Get-ChildItem "$Main\Retours" -Filter '*.md' -ErrorAction SilentlyContinue | Where-Object { (Get-Content $_.FullName -Raw) -match '(?m)^\s*\**R[ée]ponse[^:\n]*:\**\s*\S' })
  if ($ret.Count -gt 0) { Say 'WARN' ("reponses de l'utilisateur encore dans Retours\ (a reporter dans le chantier puis supprimer) : " + (($ret | ForEach-Object { $_.Name }) -join ', ')) } else { Say 'PASS' 'aucune reponse non traitee dans Retours\' }
  $jour = Get-Content "$Main\Auto_Journey_Jour.md" -Raw -ErrorAction SilentlyContinue
  $m2 = '### Lecons appliquees ' + $now.ToString('yyyy-MM-dd')
  if ($jour -and $jour.Contains($m2)) { Say 'PASS' "bilan des lecons present ($m2)" } else { Say 'FAIL' "pas de '$m2' dans Auto_Journey_Jour.md : une ligne par regle de Auto_Journey_Checklist.md, 'appliquee' ou 'non appliquee' + la preuve" }
}

if ($Fails -gt 0) { Write-Output "=> $Fails FAIL"; exit 1 } else { Write-Output '=> aucun FAIL'; exit 0 }
