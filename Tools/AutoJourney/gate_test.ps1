# gate_test.ps1 : test d'effet des outils de processus (L7-1, L7-4, L7-6, L7-8, L7-2). Aucun C++, aucun editeur.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (sortie : PASS/FAIL par cas, code 1 s'il y a un FAIL)
# Les cas travaillent sur de FAUX worktrees dans %TEMP% (processus factices, DLL factice, Retours factice).
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Gate = Join-Path $Here 'session_gate.ps1'
$Stray = Join-Path $Here 'stray_processes.ps1'
$Rules = Join-Path $Here 'agent_rules.md'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('gatetest-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $Tmp | Out-Null
$A = [string][char]0xC0   # A grave
$E = [string][char]0xE9   # e acute

# --- L7-4 : stray_processes.ps1
Check (Test-Path $Stray) 'stray_processes.ps1 existe'
if (Test-Path $Stray) {
  $FakeRoot = Join-Path $Tmp 'HeroesOfLiberty-Fake'
  New-Item -ItemType Directory -Path $FakeRoot | Out-Null
  $none = & powershell -NoProfile -ExecutionPolicy Bypass -File $Stray -Root $FakeRoot | Out-String
  $codeNone = $LASTEXITCODE
  Check ($codeNone -eq 0 -and $none -match 'aucun') 'stray: aucun processus -> "aucun", code 0' "code=$codeNone out=$none"
  $p = Start-Process powershell -ArgumentList @('-NoProfile', '-Command', "Start-Sleep 120 # $FakeRoot\chain.ps1") -PassThru -WindowStyle Hidden
  Start-Sleep -Milliseconds 1500
  $list = & powershell -NoProfile -ExecutionPolicy Bypass -File $Stray -Root $FakeRoot | Out-String
  $codeList = $LASTEXITCODE
  Check ($codeList -eq 1 -and $list -match [string]$p.Id) 'stray: processus detache liste (PID) et code 1' "code=$codeList out=$list"
  $stop = & powershell -NoProfile -ExecutionPolicy Bypass -File $Stray -Root $FakeRoot -Stop | Out-String
  Start-Sleep -Milliseconds 800
  Check (-not (Get-Process -Id $p.Id -ErrorAction SilentlyContinue)) 'stray: -Stop arrete le processus' $stop
  if (Get-Process -Id $p.Id -ErrorAction SilentlyContinue) { Stop-Process -Id $p.Id -Force }
}

# --- L7-2 : session_gate Launch -Scenarios -Root : DLL du worktree plus ancienne que le dernier code de main
$Wt = Join-Path $Tmp 'HeroesOfLiberty-FakeDll'
New-Item -ItemType Directory -Path (Join-Path $Wt 'Binaries\Win64') -Force | Out-Null
$Dll = Join-Path $Wt 'Binaries\Win64\Fake.dll'
Set-Content -Path $Dll -Value 'x'
(Get-Item $Dll).LastWriteTime = [datetime]'2000-01-01'
$old = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Scenarios -Root $Wt | Out-String
Check ($old -match '(?m)^FAIL .*DLL') 'launch -Scenarios: DLL ancienne -> FAIL' $old
$oldNoScn = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Root $Wt | Out-String
Check (-not ($oldNoScn -match '(?m)^FAIL .*DLL') -and $oldNoScn -match '(?m)^WARN .*DLL') 'launch sans -Scenarios: DLL ancienne -> WARN seulement' $oldNoScn
(Get-Item $Dll).LastWriteTime = Get-Date
$fresh = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Scenarios -Root $Wt | Out-String
Check ($fresh -match '(?m)^PASS .*DLL' -and -not ($fresh -match '(?m)^FAIL .*DLL')) 'launch -Scenarios: DLL recente -> PASS' $fresh
$noRoot = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Scenarios | Out-String
Check ($noRoot -match '(?m)^FAIL .*-Root') 'launch -Scenarios sans -Root -> FAIL' $noRoot

# --- Elrond-8 (2026-11-07) : binaire PERIME par une fusion. Le commit d'un fichier fusionne garde sa date d'auteur (ancienne), donc
# "binaire >= date du dernier commit de code" laisse passer un binaire compile AVANT la fusion ; le fichier, lui, porte l'heure de la fusion.
# Cas : vrai depot jetable, commit de code date de 2000, DLL de 2001, mais Source\a.cpp ecrit APRES la DLL (= fusion) -> Report doit signaler (WARN),
# Essay doit dire FAIL ; DLL plus recente que le fichier -> PASS.
$Repo = Join-Path $Tmp 'HeroesOfLiberty-FakeMerge'
New-Item -ItemType Directory -Path (Join-Path $Repo 'Source\M'), (Join-Path $Repo 'Binaries\Win64') -Force | Out-Null
git -C $Repo init -q 2>$null | Out-Null
Set-Content -Path (Join-Path $Repo 'Source\M\a.cpp') -Value 'int a;'
git -C $Repo add Source 2>$null | Out-Null
$env:GIT_AUTHOR_DATE = '2000-01-01T00:00:00'; $env:GIT_COMMITTER_DATE = '2000-01-01T00:00:00'
git -C $Repo -c user.name=t -c user.email=t@t commit -q -m code 2>$null | Out-Null
Remove-Item Env:GIT_AUTHOR_DATE, Env:GIT_COMMITTER_DATE
$MDll = Join-Path $Repo 'Binaries\Win64\Fake.dll'
Set-Content -Path $MDll -Value 'x'
(Get-Item $MDll).LastWriteTime = [datetime]'2001-01-01'
(Get-Item (Join-Path $Repo 'Source\M\a.cpp')).LastWriteTime = [datetime]'2002-01-01'   # le fichier arrive par la fusion APRES la compilation
$rep = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo | Out-String
Check ($rep -match '(?m)^WARN .*(binaire|DLL).*(ANCIEN|perim).*Source') 'report: source ecrite apres le binaire (fusion) -> WARN binaire perime' $rep
$ess = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $Repo | Out-String
Check ($ess -match '(?m)^FAIL .*(binaire|DLL).*(ANCIEN|perim).*Source') 'essay: source ecrite apres le binaire (fusion) -> FAIL binaire perime' $ess
(Get-Item $MDll).LastWriteTime = Get-Date
$rep2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo | Out-String
Check (-not ($rep2 -match '(?m)^WARN .*(binaire|DLL).*(ANCIEN|perim)') -and $rep2 -match '(?m)^PASS .*binaire') 'report: binaire plus recent que le fichier -> PASS' $rep2

# --- L7-8 + L7-6 : Close -Exclude et ligne GESTE / SON des E-*.md
$Ret = Join-Path $Tmp 'Retours'
New-Item -ItemType Directory -Path $Ret | Out-Null
Set-Content -Path (Join-Path $Ret 'E-bad.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-bad : un essai sans statut`nSTATUT : VU $A L'$($E)CRAN"
Set-Content -Path (Join-Path $Ret 'E-good.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-good : un essai complet`nSTATUT : VU $A L'$($E)CRAN`nGESTE: r$($E)el`nSON: non entendu"
Set-Content -Path (Join-Path $Ret 'E-done.md') -Encoding UTF8 -Value "# [OK] E-done : essai deja valide`nSTATUT : VU"
$close = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -Exclude Faramir -RetoursDir $Ret | Out-String
Check (-not ($close -match '(?m)^(PASS|FAIL)\s+HeroesOfLiberty-Faramir ')) 'close -Exclude Faramir: aucun controle PASS/FAIL pour ce worktree' $close
Check ($close -match '(?m)^WARN\s+HeroesOfLiberty-Faramir : exclu') 'close -Exclude: une ligne dit que Faramir est exclu' $close
Check ($close -match '(?m)^WARN .*E-bad\.md.*GESTE' -and $close -match 'SON') 'close: E-bad.md signale (GESTE et SON manquants)' $close
Check (-not ($close -match 'E-good\.md.*(GESTE|SON)') -and -not ($close -match 'E-done\.md.*(GESTE|SON)')) 'close: E-good.md complet et E-done.md clos ne sont pas signales' $close
$closeAll = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -RetoursDir $Ret | Out-String
Check ($closeAll -match '(?m)^(PASS|FAIL)\s+HeroesOfLiberty-Faramir ' -and -not ($closeAll -match 'Faramir : exclu')) 'close sans -Exclude: Faramir est controle' $closeAll

# --- L8-3 : l'heure de fin vient de Docs\SessionEnd.txt (une ligne '2026-11-07 12:00'), -End reste prioritaire
$EndFile = Join-Path $Tmp 'SessionEnd.txt'
$Today = (Get-Date).ToString('yyyy-MM-dd')
Set-Content -Path $EndFile -Value "$Today 23:41" -Encoding ASCII
foreach ($ph in 'Launch', 'Merge', 'Essay', 'Report', 'Close') {
  $o = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase $ph -SessionEndFile $EndFile -RetoursDir $Ret | Out-String
  Check ($o -match 'fin dure 23:41') "L8-3 $ph : fin dure lue dans SessionEnd.txt (sans -End)" $o
}
$o = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 22:11 -SessionEndFile $EndFile | Out-String
Check ($o -match 'fin dure 22:11' -and -not ($o -match 'fin dure 23:41')) 'L8-3 : -End reste prioritaire sur SessionEnd.txt' $o
$o = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -SessionEndFile (Join-Path $Tmp 'absent.txt') | Out-String
Check ($o -match '(?m)^WARN .*SessionEnd') 'L8-3 : ni -End ni fichier -> WARN SessionEnd (defaut 13:30)' $o
$real = Get-Content 'C:\Users\baudo\Documents\MyCrusader\Docs\SessionEnd.txt' -ErrorAction SilentlyContinue | Select-Object -First 1
Check ($real -match '^\d{4}-\d{2}-\d{2} \d{2}:\d{2}$') 'L8-3 : le vrai Docs\SessionEnd.txt a le format 2026-11-07 12:00' "lu : $real"

# --- L8-7 : Launch FAIL sous 3 lignes '[ ]' sans C++ dans la file
$Q2 = Join-Path $Tmp 'queue2.md'
Set-Content -Path $Q2 -Encoding ASCII -Value @('- [ ] (a, sans C++, outil) x', '- [ ] (b, sans C++) y', '- [x] (c, sans C++) fait', '- [ ] (d, C++) z')
$o = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -QueueFile $Q2 | Out-String
Check ($o -match "(?m)^FAIL seulement 2 ligne\(s\) '\[ \]'") 'L8-7 : 2 lignes sans C++ -> FAIL' $o
$Q3 = Join-Path $Tmp 'queue3.md'
Set-Content -Path $Q3 -Encoding ASCII -Value @('- [ ] (a, sans C++) x', '- [ ] (b, sans C++) y', '- [ ] (c, sans C++) z')
$o = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -QueueFile $Q3 | Out-String
Check ($o -match "(?m)^PASS 3 ligne\(s\) '\[ \]' sans C\+\+" -and -not ($o -match "(?m)^FAIL seulement")) 'L8-7 : 3 lignes sans C++ -> PASS' $o

# --- L8-6 : fins de ligne. .gitattributes existe (CRLF dans l'arbre, LF dans l'index) ; eol_check.ps1 liste (et -Fix corrige) les fichiers MELES de l'arbre de travail
$RepoRoot = Split-Path -Parent (Split-Path -Parent $Here)
$Attr = Join-Path $RepoRoot '.gitattributes'
Check (Test-Path $Attr) 'L8-6 : .gitattributes present a la racine'
$ck = git -C $RepoRoot check-attr eol text -- 'Tools/AutoJourney/x.ps1' 'Foo.md' 'Foo.uasset' | Out-String
Check ($ck -match 'x\.ps1: eol: crlf' -and $ck -match 'Foo\.md: eol: crlf' -and $ck -match 'Foo\.uasset: text: unset') 'L8-6 : .ps1 et .md en CRLF, .uasset binaire' $ck
$Eol = Join-Path $Here 'eol_check.ps1'
Check (Test-Path $Eol) 'L8-6 : eol_check.ps1 existe'
if (Test-Path $Eol) {
  $Repo = Join-Path $Tmp 'eolrepo'
  New-Item -ItemType Directory -Path $Repo | Out-Null
  git -C $Repo init -q 2>&1 | Out-Null
  git -C $Repo config user.email t@t.t; git -C $Repo config user.name t
  Copy-Item $Attr (Join-Path $Repo '.gitattributes')
  $bom = [byte[]](0xEF, 0xBB, 0xBF)
  $mixed = $bom + [Text.Encoding]::UTF8.GetBytes("a`r`nb`nc`r`nd`n")
  [IO.File]::WriteAllBytes((Join-Path $Repo 'mixed.md'), $mixed)
  [IO.File]::WriteAllBytes((Join-Path $Repo 'clean.md'), [Text.Encoding]::UTF8.GetBytes("a`r`nb`r`n"))
  git -C $Repo add .gitattributes mixed.md clean.md 2>&1 | Out-Null
  git -C $Repo commit -q -m t 2>&1 | Out-Null
  $chk = & powershell -NoProfile -ExecutionPolicy Bypass -File $Eol -Root $Repo | Out-String
  $chkCode = $LASTEXITCODE
  Check ($chkCode -eq 1 -and $chk -match 'mixed\.md' -and -not ($chk -match 'clean\.md')) 'L8-6 : eol_check liste mixed.md seul, code 1' "code=$chkCode out=$chk"
  $fix = & powershell -NoProfile -ExecutionPolicy Bypass -File $Eol -Root $Repo -Fix | Out-String
  $bytes = [IO.File]::ReadAllBytes((Join-Path $Repo 'mixed.md'))
  $text = [Text.Encoding]::UTF8.GetString($bytes)
  Check (($bytes[0] -eq 0xEF) -and ($text -match "^\uFEFFa`r`nb`r`nc`r`nd`r`n$")) 'L8-6 : -Fix met le fichier en CRLF partout, BOM gardee' $fix
  $after = & powershell -NoProfile -ExecutionPolicy Bypass -File $Eol -Root $Repo | Out-String
  Check ($LASTEXITCODE -eq 0) 'L8-6 : apres -Fix, eol_check code 0' $after
  $st = git -C $Repo status --short | Out-String
  Check (-not ($st -match 'mixed\.md')) 'L8-6 : -Fix ne laisse aucun diff git (index LF)' $st
}

# --- L7-1, L7-4, L7-6 : agent_rules.md
$r = Get-Content $Rules -Raw -Encoding UTF8
Check ($r -match '60 appels') 'agent_rules: arret a 60 appels declares'
Check ($r -match 'tool_uses') 'agent_rules: tool_uses releve par la session principale'
Check ($r -match 'PROCESSUS_DETACHES') 'agent_rules: champ PROCESSUS_DETACHES'
Check ($r -match 'GESTE' -and $r -match 'SON') 'agent_rules: statuts GESTE et SON'
Check ($r -match 'stray_processes') 'agent_rules: stray_processes.ps1 cite'

# --- L8-1 : session_gate Essay : le worktree doit contenir le tip de CHAQUE branche citee dans le E-*.md de l'essai
# (2026-11-06 : qground joue depuis Elrond sans les correctifs de melee de Celeborn). Vrai depot git factice : work contient fixa, pas fixb.
$G = Join-Path $Tmp 'HeroesOfLiberty-FakeGit'
New-Item -ItemType Directory -Path $G | Out-Null
function GitIn { git -C $G -c user.name=t -c user.email=t@t @args 2>&1 | Out-Null }
GitIn init
GitIn checkout -b main
GitIn commit --allow-empty -m base
GitIn checkout -b fixa
GitIn commit --allow-empty -m a
GitIn checkout main
GitIn checkout -b fixb
GitIn commit --allow-empty -m b
GitIn checkout main
GitIn checkout -b work
GitIn merge fixa --no-edit
$RetE = Join-Path $Tmp 'RetoursEssay'
New-Item -ItemType Directory -Path $RetE | Out-Null
Set-Content -Path (Join-Path $RetE 'E-miss.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-miss : essai`nGESTE: r$($E)el`nbranches: fixa, fixb"
Set-Content -Path (Join-Path $RetE 'E-ok.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-ok : essai`nGESTE: r$($E)el`nbranches: fixa"
Set-Content -Path (Join-Path $RetE 'E-prose.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-prose : essai`nGESTE: r$($E)el`nLe correctif est sur la branche fixb, pas sur fixa."
Set-Content -Path (Join-Path $RetE 'E-none.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-none : essai`nGESTE: r$($E)el`nrien a citer"
$eMiss = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-miss | Out-String
Check ($eMiss -match "(?m)^FAIL .*fixb" -and -not ($eMiss -match "(?m)^FAIL .*fixa")) 'essay: branche citee absente du worktree -> FAIL qui nomme fixb (pas fixa)' $eMiss
$eOk = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-ok | Out-String
Check ($eOk -match "(?m)^PASS .*fixa" -and -not ($eOk -match "(?m)^FAIL .*branche")) 'essay: branche citee contenue -> PASS' $eOk
$eProse = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-prose | Out-String
Check ($eProse -match "(?m)^FAIL .*fixb") 'essay: sans ligne branches:, nom de branche trouve dans le texte -> FAIL fixb' $eProse
$eNone = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-none | Out-String
Check ($eNone -match "(?m)^WARN .*branches:") 'essay: aucune branche citee -> WARN (ajouter la ligne branches:)' $eNone

# --- L8-1 : la ligne branches: est dans le gabarit et dans les regles
$Tpl = Get-Content (Join-Path $Here 'E_template.md') -Raw -Encoding UTF8
Check ($Tpl -match '(?m)^branches\s*:') 'E_template: ligne branches:'
Check ($r -match 'branches\s*:') 'agent_rules: ligne branches: des essais'

Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
if ($Fails -gt 0) { Write-Output "=> $Fails FAIL"; exit 1 } else { Write-Output '=> tout PASS'; exit 0 }
