# session_gate.ps1 : les controles mecaniques d'une seance Auto_Journey (lecture seule, ne modifie rien).
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\session_gate.ps1 -Phase Launch -End 13:30
#   -Phase Launch : avant CHAQUE lancement d'agent (heure, temps restant, agents C++, tableau des agents, docs non suivis)
#   -Phase Merge  : avant l'agent de fusion (pre-vol de chaque branche : -Branches ai-ia1,fog,...)
#   -Phase Close  : avant le bilan (worktrees propres et a jour, reponses non traitees, bilan des lecons)
# Sortie : une ligne PASS / WARN / FAIL par controle, code de sortie 1 s'il y a un FAIL.
#   -Phase Essay  : avant de donner une commande play.ps1 a l'utilisateur (-Root <worktree> : contient main, propre, build plus recent que le dernier commit de code ; -Essay <nom> : contient aussi le tip de chaque branche citee dans Retours\E-<nom>.md, ligne "branches: a, b" (L8-1))
#   -Phase Report : a la RECEPTION du rapport de chaque agent (-Root <worktree de l'agent>) : la copie principale n'a que des docs voulus modifies, le worktree contient main, est propre, et son binaire n'est pas plus vieux que le dernier code (sinon build.ps1 AVANT tout quick_check : binaire perime = faux rouges)
#   Launch verifie aussi la file : au moins 3 lignes '[ ]' etiquetees 'sans C++' dans Auto_Journey_File.md (de quoi relancer sans toucher au plafond de 3 agents C++)
param(
  [Parameter(Mandatory=$true)][ValidateSet('Launch','Merge','Close','Essay','Report')][string]$Phase,
  [string]$End = '',                       # L8-3 : prioritaire ; absent = la ligne de Docs\SessionEnd.txt ('2026-11-07 12:00', creee une fois par la session principale), sinon 13:30 avec un WARN
  [string]$SessionEndFile = '',            # L8-3 : defaut <copie principale>\Docs\SessionEnd.txt
  [string]$QueueFile = '',                 # L8-7 : defaut <copie principale>\Auto_Journey_File.md (la file des lignes '[ ]')
  [string[]]$Branches = @(),
  [string]$Root = '',
  [string]$MainRoot = '',                  # Merge (L9-5) : la copie principale a controler (defaut : C:\Users\baudo\Documents\MyCrusader) ; docs seuls modifies = WARN (commit_docs.ps1), CODE modifie = FAIL
  [switch]$Scenarios,                     # Launch : la tache de l'agent joue des scenarios (donc exige un binaire du worktree a jour, -Root) ; sans -Scenarios un binaire perime n'est qu'un WARN
  [switch]$NoCpp,                         # Launch (L11-7) : la tache est declaree 'sans C++' : le plafond de 3 agents C++ atteint n'est plus qu'un WARN (l'agent ne compile rien)
  [string]$QueueDir = '',                 # Launch : dossier de tickets de build.ps1 (defaut <copie principale>\Docs\BuildQueue.state) ; sert aux tests
  [string]$WorktreesDir = 'C:\Users\baudo\Documents',   # dossier qui contient les HeroesOfLiberty-* (IN_USE.txt) ; sert aux tests
  [string[]]$Exclude = @(),                # Close : worktrees hors perimetre (nom court 'Haldir' ou complet 'HeroesOfLiberty-Haldir')
  [string]$RetoursDir = '',                # Close, Essay : dossier des Retours\ (defaut : la copie principale)
  [string]$Essay = '',                     # Essay : nom de l'essai (E-<nom>.md de Retours\) dont les branches citees doivent etre dans le worktree ; defaut : tous les E-*.md ouverts
  [double]$MinFreeVirtualGB = 8,           # Launch (L12-1c) : memoire virtuelle libre sous ce seuil = WARN (2026-11-12 : builds et jeux qui echouent faute de memoire)
  [string]$EngineRoot = 'C:\UE\UE_5.8_HOL', # Launch (L12-1c) : un cl.exe dont aucun ancetre ne cite ce moteur = compilation d'un AUTRE moteur (ProjectShiva sur AS_3...) = WARN
  [int]$MinMinutes = 5  # 2026-11-05 : l'utilisateur prefere finir 15 min APRES l'heure que 15 min avant (sauf consigne contraire dans le prompt de lancement) ; l'ancien seuil de 20 min laissait la seance vide
)
$ErrorActionPreference = 'Continue'
$Branches = @($Branches | ForEach-Object { $_ -split ',' } | Where-Object { $_ })   # -File passe "a,b" comme une seule chaine
$Main = 'C:\Users\baudo\Documents\MyCrusader'
$Exclude = @($Exclude | ForEach-Object { $_ -split ',' } | Where-Object { $_ } | ForEach-Object { $_ -replace '^HeroesOfLiberty-', '' })
if (-not $RetoursDir) { $RetoursDir = "$Main\Retours" }
$Fails = 0
function Say([string]$Level, [string]$Msg) { if ($Level -eq 'FAIL') { $script:Fails++ }; Write-Output ("{0,-4} {1}" -f $Level, $Msg) }
# Elrond-8 (2026-11-07) : le plus recent des fichiers C++/config d'un worktree (heure d'ECRITURE). Une fusion ecrit les fichiers qu'elle apporte a l'heure
# de la fusion alors que leur commit garde sa date d'auteur : comparer le binaire a la DATE DE COMMIT laisse passer un binaire compile AVANT la fusion
# (faux rouges comme 'camera 2p/8p right edge got 0 m', 2026-11-06). On compare donc aussi a l'heure d'ecriture des fichiers.
function Get-NewestCode([string]$Root) {
  $f = @(); foreach ($d in 'Source', 'Config') { $p = Join-Path $Root $d; if (Test-Path $p) { $f += Get-ChildItem $p -Recurse -File -Include '*.cpp', '*.h', '*.cs', '*.ini' -ErrorAction SilentlyContinue } }
  $f | Sort-Object LastWriteTime -Descending | Select-Object -First 1
}
function Say-StaleBySource([string]$Root, $Bin, [string]$Level) {
  $n = Get-NewestCode $Root
  if ($n -and $Bin -and $n.LastWriteTime -gt $Bin.LastWriteTime.AddSeconds(5)) { Say $Level ("binaire du {0} plus ANCIEN que {1} ecrit le {2} (fusion ou checkout apres la compilation) : Source modifiee apres le binaire, build.ps1 AVANT tout quick_check ou essai" -f $Bin.LastWriteTime.ToString('MM-dd HH:mm'), $n.Name, $n.LastWriteTime.ToString('MM-dd HH:mm')) }
}

$now = Get-Date
if (-not $SessionEndFile) { $SessionEndFile = "$Main\Docs\SessionEnd.txt" }
if (-not $QueueFile) { $QueueFile = "$Main\Auto_Journey_File.md" }
# L8-3 : l'heure de fin vient de -End, sinon de la 1re ligne de SessionEnd.txt ('2026-11-07 12:00' ou 'HH:mm'), sinon 13:30 + WARN
$endNote = $null
$endText = $End
if (-not $endText -and (Test-Path $SessionEndFile)) { $endText = ((Get-Content $SessionEndFile -ErrorAction SilentlyContinue | Select-Object -First 1) -as [string]).Trim() }
if (-not $endText) { $endText = '13:30'; $endNote = "ni -End ni $SessionEndFile : fin dure 13:30 par defaut (ecrire 'AAAA-MM-JJ HH:mm' dans Docs\SessionEnd.txt, une fois par seance)" }
if ($endText -notmatch '^\d{4}-\d{2}-\d{2} ') { $endText = $now.ToString('yyyy-MM-dd') + ' ' + $endText }   # 'HH:mm' seul = aujourd'hui
$endText = $endText -replace ' (\d):', ' 0$1:'
$endAt = [datetime]::ParseExact($endText, 'yyyy-MM-dd HH:mm', [Globalization.CultureInfo]::InvariantCulture)
if ($endAt.Date -ne $now.Date) { $endNote = "l'heure de fin lue ($endText) n'est pas d'aujourd'hui : mettre Docs\SessionEnd.txt a jour" }
$left = [int]($endAt - $now).TotalMinutes
Write-Output ("Heure PowerShell : {0}  fin dure {1}  reste {2} min" -f $now.ToString('HH:mm:ss'), $endAt.ToString('HH:mm'), $left)
if ($endNote) { Say 'WARN' "SessionEnd : $endNote" }

if (-not $QueueDir) { $QueueDir = "$Main\Docs\BuildQueue.state" }
$wts = Get-ChildItem $WorktreesDir -Directory -Filter 'HeroesOfLiberty-*' | Select-Object -ExpandProperty FullName

if ($Phase -eq 'Launch') {
  if ($left -lt $MinMinutes) { Say 'FAIL' ("il reste {0} min (< {1}) : AUCUN lancement d'agent" -f $left, $MinMinutes) }
  else { Say 'PASS' ("temps restant {0} min ; arret dur des agents = {1} (jusqu'a +15 min toleres, jamais 15 min trop tot)" -f $left, $endAt.ToString('HH:mm')) }

  # 2026-11-08 : aucun script d'outil ne lance un UnrealEditor visible (pop sur l'ecran de l'utilisateur, vole le focus) : no_window_test.ps1, WARN
  $nwTest = Join-Path $PSScriptRoot 'no_window_test.ps1'
  if (Test-Path $nwTest) {
    $nwOut = (& powershell -NoProfile -ExecutionPolicy Bypass -File $nwTest 2>&1 | Out-String)
    if ($LASTEXITCODE -eq 0) { Say 'PASS' 'no_window_test : aucun script d outil ne lance d editeur fenetre' }
    else { Say 'WARN' ("no_window_test ROUGE (un outil ouvre un editeur visible) : " + (($nwOut -split "`n" | Where-Object { $_ -match '^FAIL|\.ps1:\d+' } | Select-Object -First 3 | ForEach-Object { $_.Trim() }) -join ' ; ')) }
  }

  # L11-1 (2026-11-10) : python/pythonw detaches d'un worktree (7 glade_budget.py orphelins = machine ralentie 30 min) : WARN avec le PID, jamais d'arret ici
  # (stray_processes.ps1 -Root <worktree> -PythonOnly -Stop). Avec -Root : ce worktree seul ; sans : la copie principale et tous les worktrees.
  $pyAlive = @(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue | Where-Object { $_.Name -match '^(py|pythonw?[\d.]*)(\.exe)?$' }).Count
  $pyRoots = @(); if ($Root) { $pyRoots = @($Root) } else { $pyRoots = @($Main) + @($wts) }
  $pyWarn = 0
  if ($pyAlive -gt 0) {
    foreach ($pr in $pyRoots) {
      if (-not (Test-Path $pr)) { continue }
      if ($Exclude -contains ((Split-Path $pr -Leaf) -replace '^HeroesOfLiberty-', '')) { continue }
      $pyOut = & powershell -NoProfile -ExecutionPolicy Bypass -File "$PSScriptRoot\stray_processes.ps1" -Root $pr -PythonOnly 2>&1 | Out-String
      if ($LASTEXITCODE -ne 0) {
        $pyWarn++
        Say 'WARN' ("python/pythonw orphelin(s) pour $pr (stray_processes.ps1 -Root $pr -PythonOnly -Stop) : " + (($pyOut -split "`n" | Where-Object { $_ -match 'PID' } | Select-Object -First 3 | ForEach-Object { ($_ -replace '\s+', ' ').Trim() }) -join ' | '))
      }
    }
  }
  if ($pyWarn -eq 0) { Say 'PASS' 'aucun python/pythonw orphelin' }

  # L12-1c (2026-11-12) : memoire virtuelle libre (mesure reelle) sous le seuil = WARN ; build.ps1 attend lui-meme tant qu'elle est sous 8 Go
  $os = Get-CimInstance Win32_OperatingSystem -ErrorAction SilentlyContinue
  $vmFree = if ($os) { [math]::Round($os.FreeVirtualMemory / 1MB, 1) } else { -1 }
  if ($vmFree -ge 0 -and $vmFree -lt $MinFreeVirtualGB) { Say 'WARN' ("memoire virtuelle libre {0} Go < {1} Go : attendre (Start-Sleep 120) ou finir des tests avant tout lancement de jeu ou de build" -f $vmFree, $MinFreeVirtualGB) }
  elseif ($vmFree -ge 0) { Say 'PASS' ("memoire virtuelle libre {0} Go >= {1} Go" -f $vmFree, $MinFreeVirtualGB) }
  # L12-1c : cl.exe d'un AUTRE moteur (aucun ancetre, lui compris, ne cite $EngineRoot) : il prend la machine sans passer par la file de build.ps1
  $allProcs = @(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue)
  $byPid = @{}; foreach ($pp in $allProcs) { $byPid[[int]$pp.ProcessId] = $pp }
  $engNorm = $EngineRoot.Replace('/', '\').TrimEnd('\')
  $foreignCl = @()
  foreach ($c in @($allProcs | Where-Object { $_.Name -ieq 'cl.exe' })) {
    $cur = $c; $ours = $false; $seen = 0
    while ($cur -and $seen -lt 6) {
      if ($cur.CommandLine -and $cur.CommandLine.Replace('/', '\') -like "*$engNorm\*") { $ours = $true; break }
      $seen++; $par = [int]$cur.ParentProcessId; $cur = if ($par -and $par -ne [int]$cur.ProcessId) { $byPid[$par] } else { $null }
    }
    if (-not $ours) { $foreignCl += $c.ProcessId }
  }
  if ($foreignCl.Count) { Say 'WARN' ("cl.exe d'un autre moteur que {0} en cours (PID {1}) : la machine compile hors de la file de build.ps1, builds et jeux plus lents" -f $engNorm, ($foreignCl -join ', ')) }
  else { Say 'PASS' "aucun cl.exe d'un autre moteur" }

  # L8-9 : au DEBUT de chaque Auto_Journey tous les worktrees sont remis a jour (sync_worktrees.ps1) ; WARN par worktree en retard (les agents deja lances en cours de seance retardent apres chaque fusion : normal)
  foreach ($w in $wts) {
    $n9 = Split-Path $w -Leaf
    if ($Exclude -contains ($n9 -replace '^HeroesOfLiberty-', '')) { continue }
    $b9 = [int](git -C $w rev-list --count 'HEAD..main' 2>$null)
    if ($b9 -gt 0) { Say 'WARN' "$n9 : retard de $b9 commit(s) sur main : au debut de la seance lancer Tools\AutoJourney\sync_worktrees.ps1 AVANT le premier lancement" }
  }

  # Plafond de 3 agents C++ (regle 10) : worktrees DISTINCTS qui ont un ticket vivant dans la file de build.ps1 (proprietaire en vie ; un -Wait peut en poser deux)
  # + worktrees dont IN_USE.txt (texte libre) ne commence pas par 'libre' et cite 'C++' sans 'sans C++'. FAIL pour un agent C++, WARN avec -NoCpp (L11-7).
  $cppAgents = @{}
  foreach ($tf in @(Get-ChildItem $QueueDir -Filter '*.json' -ErrorAction SilentlyContinue)) {
    try {
      $tk = Get-Content $tf.FullName -Raw | ConvertFrom-Json
      $alive = $false
      try { $pp = Get-Process -Id ([int]$tk.pid) -ErrorAction Stop; $alive = (-not $tk.pidStart) -or ([math]::Abs($pp.StartTime.Ticks - [int64]$tk.pidStart) -lt 20000000) } catch { }
      if ($alive -and $tk.worktree) { $cppAgents[[string]$tk.worktree] = 'file de build' }
    } catch { }
  }
  foreach ($w in $wts) {
    $iu = Join-Path $w 'IN_USE.txt'
    if (-not (Test-Path $iu)) { continue }
    if ((Get-Item $iu).LastWriteTime -lt (Get-Date).AddHours(-12)) { continue }   # une fiche vieille de plus de 12 h n'est plus un agent en cours
    $iuText = (Get-Content $iu -Raw -ErrorAction SilentlyContinue)
    if (-not $iuText) { continue }
    $iuFirst = ($iuText -split "`n" | Where-Object { $_.Trim() } | Select-Object -First 1)
    if ($iuFirst -match '^\s*libre' -or $iuText -notmatch 'C\+\+' -or $iuFirst -match 'sans C\+\+') { continue }
    $nm = Split-Path $w -Leaf
    if (-not $cppAgents.ContainsKey($nm)) { $cppAgents[$nm] = 'IN_USE.txt' }
  }
  $ceilLevel = if ($NoCpp) { 'WARN' } else { 'FAIL' }
  $cppList = (($cppAgents.GetEnumerator() | Sort-Object Name | ForEach-Object { "$($_.Name) ($($_.Value))" }) -join ', ')
  if ($cppAgents.Count -ge 3) { Say $ceilLevel ("{0} agents C++ (plafond de 3 atteint : {1}) : le nouvel agent demarre SANS C++{2}" -f $cppAgents.Count, $cppList, $(if ($NoCpp) { ' (tache declaree sans C++ : WARN seulement)' } else { '' })) }
  else { Say 'PASS' ("{0} agent(s) C++ (file de build + IN_USE.txt), plafond de 3 respecte{1}" -f $cppAgents.Count, $(if ($cppList) { " : $cppList" } else { '' })) }

  $jour = Get-Content "$Main\Auto_Journey_Jour.md" -Raw -ErrorAction SilentlyContinue
  $marker = '## Agents du ' + $now.ToString('yyyy-MM-dd')
  if ($jour -and $jour.Contains($marker)) { Say 'PASS' "tableau agent -> id -> worktree present ($marker)" }
  else { Say 'FAIL' "pas de tableau '$marker' dans Auto_Journey_Jour.md : l'ecrire AVANT de lancer ou d'envoyer un message (colonnes : nom, id, worktree, branche, tache, heure de lancement)" }

  $st = git -C $Main status --short 2>$null
  $untracked = @($st | Where-Object { $_ -match '^\?\? .*InProgress\.md' })
  if ($untracked.Count -gt 0) { Say 'FAIL' ("dossiers non suivis dans la copie principale (bloquent la fusion) : " + (($untracked | ForEach-Object { $_.Substring(3) }) -join ', ') + ' -> demander l''accord de les commiter') } else { Say 'PASS' 'aucun *InProgress.md non suivi' }
  $dirty = @($st | Where-Object { $_ -match '^ ?M ' })
  if ($dirty.Count -gt 0) { Say 'WARN' ("fichiers suivis modifies dans la copie principale : " + (($dirty | ForEach-Object { $_.Substring(3) }) -join ', ')) } else { Say 'PASS' 'copie principale propre' }
  $file = Get-Content $QueueFile -ErrorAction SilentlyContinue
  $noCppLines = @($file | Where-Object { $_ -match '^\s*- \[ \]' -and $_ -match 'sans C\+\+' }).Count
  if ($noCppLines -ge 3) { Say 'PASS' "$noCppLines ligne(s) '[ ]' sans C++ en file (relance possible sans toucher au plafond)" }
  else { Say 'FAIL' "seulement $noCppLines ligne(s) '[ ]' etiquetee(s) 'sans C++' dans $(Split-Path $QueueFile -Leaf) (< 3) : en ecrire AVANT de lancer, sinon les agents finis ne pourront pas etre relances (2026-11-05 : 30 min vides ; L8-7 : FAIL et non plus WARN)" }

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
    # L11-1 : binaire plus vieux que les SOURCES du worktree (fusion ou checkout apres la compilation) : FAIL seulement si l'agent va y tester (-Scenarios), sinon WARN
    if ($bin) { Say-StaleBySource $Root $bin $(if ($Scenarios) { 'FAIL' } else { 'WARN' }) }
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
      Say-StaleBySource $Root $bin 'WARN'
    } else { Say 'WARN' 'binaire ou commit de code introuvable' }
  }
}

if ($Phase -eq 'Merge') {
  # L9-5 : des docs non commites de la copie principale ont bloque l'avance rapide deux fois ; du CODE modifie dans la copie principale ne doit jamais s'y trouver
  $mr = if ($MainRoot) { $MainRoot } else { $Main }
  $mdocOk = '^(Auto_Journey.*\.md|EtapeDefini\.md|Retours/.*|Placeholders[^/]*\.(txt|md)|[A-Za-z0-9_]+InProgress\.md|Docs/SessionEnd\.txt)$'   # = Specs de commit_docs.ps1
  $mst = @(git -C $mr -c core.quotepath=false status --short 2>$null | Where-Object { $_ -match '^ ?[MADR]' } | ForEach-Object { $_.Substring(3).Trim('"') })
  $mcode = @($mst | Where-Object { $_ -notmatch $mdocOk })
  $mdocs = @($mst | Where-Object { $_ -match $mdocOk })
  if ($mcode.Count -gt 0) { Say 'FAIL' ("copie principale $mr : du CODE ou des fichiers hors docs sont modifies (lecon L9-5, ne pas fusionner par-dessus) : " + ($mcode -join ', ')) }
  elseif ($mdocs.Count -gt 0) { Say 'WARN' ("copie principale $mr : $($mdocs.Count) doc(s) modifie(s) non commite(s) : lancer Tools\AutoJourney\commit_docs.ps1 AVANT la fusion (sinon l'avance rapide est bloquee)") }
  else { Say 'PASS' "copie principale $mr : propre" }
  if ($Branches.Count -eq 0 -and -not $MainRoot) { Say 'FAIL' 'donner -Branches <b1,b2,...>' }
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
      Say-StaleBySource $Root $bin 'FAIL'
    } else { Say 'WARN' 'binaire ou commit de code introuvable : verifier le build a la main' }
    # L8-1 : le worktree doit contenir le tip de CHAQUE branche citee dans le E-*.md de l'essai (ligne "branches: a, b", a defaut les noms
    # de branches locales trouves dans le texte). 2026-11-06 : qground joue depuis Elrond sans les correctifs de melee de Celeborn.
    $eFiles = @()
    if ($Essay) {
      $en = [IO.Path]::GetFileNameWithoutExtension($Essay) -replace '^E-', ''
      $eFiles = @(Get-ChildItem $RetoursDir -Filter 'E-*.md' -ErrorAction SilentlyContinue | Where-Object { $_.BaseName -eq "E-$en" })
      if ($eFiles.Count -eq 0) {
        # 2026-11-09 : -Essay accepte aussi le NOM DU SCENARIO (celui de la commande play.ps1) : on retrouve la ou les fiches qui le citent.
        $scn = [regex]::Escape($en)
        $eFiles = @(Get-ChildItem $RetoursDir -Filter 'E-*.md' -ErrorAction SilentlyContinue | Where-Object {
          (Get-Content $_.FullName -Raw -Encoding UTF8) -match "(?i)play\.ps1\s+['""]?$scn(?![A-Za-z0-9_])" })
        if ($eFiles.Count -gt 0) { Say 'PASS' ("scenario '{0}' : fiche(s) retrouvee(s) par sa commande play.ps1 : {1}" -f $en, (($eFiles | ForEach-Object { $_.Name }) -join ', ')) }
      }
      if ($eFiles.Count -eq 0) { Say 'FAIL' "Retours\E-$en.md introuvable dans $RetoursDir, et aucune fiche E-*.md ne cite 'play.ps1 $en' : les branches de l'essai ne sont pas verifiees" }
    } else {
      $eFiles = @(Get-ChildItem $RetoursDir -Filter 'E-*.md' -ErrorAction SilentlyContinue | Where-Object {
        $t0 = Get-Content $_.FullName -Raw -Encoding UTF8
        $t0 -match '(?m)^#.*ESSAYER\]' -and $t0 -notmatch '(?m)^\s*\**R[^\s:]*ponse[^:\n]*:\**\s*\S' })
      if ($eFiles.Count -eq 0) { Say 'WARN' "aucun E-*.md ouvert dans $RetoursDir (ou donner -Essay <nom>) : branches de l'essai non verifiees" }
    }
    $localBranches = @(git -C $Root branch --format='%(refname:short)' 2>$null)
    foreach ($ef in $eFiles) {
      $etxt = Get-Content $ef.FullName -Raw -Encoding UTF8
      $named = $true
      $cited = @()
      $bm = [regex]::Match($etxt, '(?mi)^\s*\**branches\s*:\**\s*(.+)$')
      if ($bm.Success) {
        # 2026-11-08 : seule la LISTE compte, pas le commentaire qui suit (fiches "branches: heal-sleep   (le scenario vient de ...)" : 17 faux FAIL) :
        # on coupe a la premiere parenthese et a deux espaces ou plus, puis on ne garde que des noms de branche plausibles.
        $listText = ($bm.Groups[1].Value -replace '\s*[\(\[].*$', '') -replace '\s{2,}.*$', ''
        $cited = @($listText -split '[,;\s`]+' | Where-Object { $_ -and $_ -ne '-' -and $_ -ne 'aucune' -and $_ -match '^[A-Za-z0-9][A-Za-z0-9._/-]*$' })
      }
      else {
        # 2026-11-09 : seule la ligne 'branches:' compte. Plus de lecture de la PROSE (le mot « branches », « maps », « tiers »... dans le texte
        # d'une fiche n'est pas une liste de branches) : sans la ligne, c'est un WARN plus bas, pas un FAIL.
        $named = $false
      }
      if ($cited.Count -eq 0 -and $bm.Success -and $listText.Trim() -eq 'aucune') { Say 'PASS' ("{0} : branches: aucune (seul main compte)" -f $ef.Name); continue }
      if ($cited.Count -eq 0) { Say 'WARN' ("{0} : aucune branche citee : ajouter la ligne 'branches: a, b' (les branches dont le correctif est joue ; gabarit E_template.md)" -f $ef.Name); continue }
      foreach ($cb in $cited) {
        if ($localBranches -notcontains $cb) { Say 'FAIL' ("{0} : branche '{1}' citee mais inconnue de git (faute de frappe ?)" -f $ef.Name, $cb); continue }
        git -C $Root merge-base --is-ancestor $cb HEAD 2>$null
        if ($LASTEXITCODE -eq 0) { Say 'PASS' ("{0} : le worktree contient le tip de la branche {1}" -f $ef.Name, $cb) }
        else { Say 'FAIL' ("{0} : branche '{1}' ABSENTE du worktree (son tip n'est pas ancetre de HEAD){2} : la fusionner ici (git merge {1}) puis build.ps1, ou jouer l'essai dans un worktree qui la contient" -f $ef.Name, $cb, $(if ($named) { '' } else { ' [deduite du texte, pas d une ligne branches:]' })) }
      }
    }
    Say 'WARN' "rappel : l'essai d'un REGLAGE/BUG est rejoue par l'integrateur (geste reel + PNG avant/apres) AVANT cette commande"
  }
}

if ($Phase -eq 'Close') {
  # L9-4 (branche le 2026-11-12) : fichiers suivis aux fins de ligne MELEES dans l'arbre de travail (eol_check.ps1, L8-6) : Edit multi-lignes refuse,
  # conflit total a la fusion (Auto_Journey_File.md, 3 fois). WARN qui nomme les fichiers et la commande -Fix (aucun diff git), PASS sinon.
  $EolCheck = Join-Path $PSScriptRoot 'eol_check.ps1'
  function Say-Eol([string]$Dir, [string]$Label) {
    if (-not (Test-Path $EolCheck)) { Say 'WARN' "$Label : fins de ligne non controlees ($EolCheck introuvable)"; return }
    $eolOut = @(& $EolCheck -Root $Dir 2>$null); $eolCode = $LASTEXITCODE
    $eolErr = @($eolOut | Where-Object { $_ -match '^ERREUR ' })
    # ligne 99 (recette 2026-11-12) : git ls-files en echec = rien releve, jamais PASS
    if ($eolCode -eq 2 -or $eolErr.Count) { Say 'WARN' ("$Label : fins de ligne non controlees (" + (($eolErr | Select-Object -First 1) -replace '^ERREUR ', '') + ")"); return }
    $mx = @($eolOut | Where-Object { $_ -match '^MIXTE ' } | ForEach-Object { $_.Substring(6) })
    if ($mx.Count -gt 0) { Say 'WARN' ("{0} : fins de ligne melees dans {1} fichier(s) suivi(s) : {2} -> powershell -ExecutionPolicy Bypass -File {3} -Root {4} -Fix (aucun diff git)" -f $Label, $mx.Count, (($mx | Select-Object -First 5) -join ', '), $EolCheck, $Dir) }
    else { Say 'PASS' "$Label : fins de ligne propres (eol_check)" }
  }
  foreach ($w in $wts) {
    $n = Split-Path $w -Leaf
    if ($Exclude -contains ($n -replace '^HeroesOfLiberty-', '')) { Say 'WARN' "$n : exclu du perimetre (-Exclude), non controle" ; continue }
    $dirty = @(git -C $w status --short 2>$null | Where-Object { $_ -notmatch 'IN_USE\.txt' -and $_ -match '^ ?[MADR]' })
    if ($dirty.Count -gt 0) { Say 'FAIL' "$n : $($dirty.Count) fichier(s) suivi(s) NON commite(s) (commiter sur la branche, jamais laisser en l'air)" } else { Say 'PASS' "$n : propre" }
    $behind = [int](git -C $w rev-list --count 'HEAD..main' 2>$null)
    if ($behind -gt 0) { Say 'FAIL' "$n : retard de $behind commit(s) sur main : L8-9, lancer Tools\AutoJourney\sync_worktrees.ps1 -NoBuild (ou avec build) avant le bilan, aucun agent ne tourne" } else { Say 'PASS' "$n : a jour avec main" }
    Say-Eol $w $n
  }
  Say-Eol $Main 'copie principale'
  $dm = @(git -C $Main status --short 2>$null | Where-Object { $_ -match '^ ?[MADR]' })
  if ($dm.Count -gt 0) { Say 'FAIL' ("copie principale : fichiers suivis non commites : " + (($dm | ForEach-Object { $_.Substring(3) }) -join ', ')) } else { Say 'PASS' 'copie principale propre' }
  # ligne 100 (recette 2026-11-12) : les fiches Retours\ jamais ajoutees a git (statut ??) echappaient au controle ci-dessus
  $ru = @(git -C $Main status --short --untracked-files=all -- Retours 2>$null | Where-Object { $_ -match '^\?\? ' } | ForEach-Object { $_.Substring(3) })
  if ($ru.Count -gt 0) { Say 'WARN' ("copie principale : {0} fichier(s) de Retours\ non suivi(s) par git (a commiter ou supprimer) : {1}" -f $ru.Count, (($ru | Select-Object -First 8) -join ', ')) } else { Say 'PASS' 'copie principale : aucun fichier non suivi dans Retours\' }
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
