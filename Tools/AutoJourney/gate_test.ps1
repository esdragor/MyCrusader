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

# --- L11-1 (2026-11-10) : session_gate Launch signale en WARN les python/pythonw orphelins du worktree (7 glade_budget.py detaches, 30 min perdues)
$PyRoot = Join-Path $Tmp 'HeroesOfLiberty-FakePy'
New-Item -ItemType Directory -Path $PyRoot | Out-Null
$FakePyExe = Join-Path $Tmp 'pythonw.exe'
Copy-Item (Join-Path $env:SystemRoot 'System32\cmd.exe') $FakePyExe
$gpClean = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Root $PyRoot | Out-String
Check ($gpClean -match '(?m)^PASS .*python' -and -not ($gpClean -match '(?m)^WARN .*python')) 'launch: aucun python orphelin -> PASS python' $gpClean
$pyOrphan = Start-Process $FakePyExe -ArgumentList "/c `"set MARK=$PyRoot\Tools\Tests\glade_budget.py & ping -n 120 127.0.0.1 >nul`"" -PassThru -WindowStyle Hidden
Start-Sleep -Milliseconds 1500
$gpWarn = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Root $PyRoot | Out-String
Check ($gpWarn -match "(?m)^WARN .*python.*$($pyOrphan.Id)") 'launch: pythonw orphelin du Root -> WARN avec son PID' $gpWarn
Check (-not (Get-Process -Id $pyOrphan.Id -ErrorAction SilentlyContinue) -eq $false) 'launch: le WARN ne tue pas le processus (signale seulement)'
& powershell -NoProfile -ExecutionPolicy Bypass -File $Stray -Root $PyRoot -PythonOnly -Stop | Out-Null
if (Get-Process -Id $pyOrphan.Id -ErrorAction SilentlyContinue) { Stop-Process -Id $pyOrphan.Id -Force }

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

# --- L11-7 + L11-1 (2026-11-11) : plafond de 3 agents C++ compte depuis la file de build ET IN_USE.txt ; WARN (pas FAIL) si la tache est sans C++ (-NoCpp)
$CapQ = Join-Path $Tmp 'queue'; $CapW = Join-Path $Tmp 'wts'
New-Item -ItemType Directory -Path $CapQ, $CapW | Out-Null
$selfStart = (Get-Process -Id $PID).StartTime.Ticks
function New-Ticket([string]$Name, [string]$Id, [int]$OwnerPid = $PID, [long]$OwnerStart = $selfStart) {
  $t = [ordered]@{ id = $Id; root = "x:\$Name"; worktree = "HeroesOfLiberty-$Name"; pid = $OwnerPid; pidStart = $OwnerStart; parentPid = 0; parentStart = 0; priority = 'normal'; requested = (Get-Date).ToString('o'); state = 'waiting'; started = '' }
  [IO.File]::WriteAllText((Join-Path $CapQ "$Id.json"), ($t | ConvertTo-Json), (New-Object Text.UTF8Encoding($false)))
}
function New-InUse([string]$Name, [string]$Text) {
  $d = Join-Path $CapW "HeroesOfLiberty-$Name"; New-Item -ItemType Directory -Path $d -Force | Out-Null
  Set-Content -Path (Join-Path $d 'IN_USE.txt') -Value $Text -Encoding ASCII
}
function Clear-Cap { Get-ChildItem $CapQ -File | Remove-Item -Force; Get-ChildItem $CapW -Directory | Remove-Item -Recurse -Force }
function Run-Cap([string[]]$More = @()) { & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -QueueDir $CapQ -WorktreesDir $CapW @More | Out-String }
New-Ticket 'T1' 'aa01'; New-Ticket 'T2' 'aa02'; New-Ticket 'T3' 'aa03'
$cap3 = Run-Cap
Check ($cap3 -match '(?m)^FAIL .*plafond') 'launch: 3 agents C++ dans la file -> FAIL plafond' $cap3
$cap3n = Run-Cap @('-NoCpp')
Check ($cap3n -match '(?m)^WARN .*plafond' -and -not ($cap3n -match '(?m)^FAIL .*plafond')) 'launch -NoCpp: plafond atteint -> WARN seulement (tache sans C++)' $cap3n
Clear-Cap
New-Ticket 'T1' 'bb01'; New-Ticket 'T2' 'bb02'
New-InUse 'U3' "OCCUPE par U3 (2026-11-11 14:00) : agent Auto_Journey, arret dur 15:12`nCPP: oui"
$capMix = Run-Cap
Check ($capMix -match '(?m)^FAIL .*plafond') 'launch: 2 tickets + 1 IN_USE.txt OCCUPE + CPP: oui = 3 agents -> FAIL plafond' $capMix
Clear-Cap
# L13-2 (2026-11-13, faux FAIL vu 3 fois : Haldir et Sauron comptes alors qu'ils etaient sans C++) : le TEXTE d'un IN_USE.txt ne compte plus ;
# seul un IN_USE.txt OCCUPE (1re ligne) portant la ligne 'CPP: oui' compte. Libre + 'CPP: oui', OCCUPE + 'CPP: non', 'Pris ... C++' sans marqueur : rien.
New-Ticket 'T1' 'ee01'; New-Ticket 'T2' 'ee02'
New-InUse 'U3' "Pris (2026-11-11 14:00) : agent Auto_Journey C++, arret dur 15:12"
New-InUse 'U4' "libre : agent fini (2026-11-13 16:39)`nCPP: oui"
New-InUse 'U5' "OCCUPE par U5 (2026-11-13 16:40) : outils`nCPP: non"
New-InUse 'U6' "Haldir agent L73/B4/L65 from 16:40 (lit les fichiers C++ sans compiler)"
$capText = Run-Cap
Check ($capText -match '(?m)^PASS 2 agent\(s\) C\+\+' -and -not ($capText -match '(?m)^(FAIL|WARN) .*plafond')) 'L13-2 launch: texte citant C++ / libre + CPP: oui / OCCUPE + CPP: non ne comptent pas -> 2 agents, PASS plafond' $capText
Clear-Cap
New-Ticket 'T1' 'cc01'; New-Ticket 'T2' 'cc02'
New-InUse 'U3' "Pris (2026-11-11 14:00) : agent Auto_Journey sans C++ (outils)"
New-InUse 'U4' "Libre (2026-11-11 13:00) : ancien agent C++ fini"
New-Ticket 'T5' 'cc05' 999999 1
$capOk = Run-Cap
Check ($capOk -match '(?m)^PASS .*plafond' -and -not ($capOk -match '(?m)^(FAIL|WARN) .*plafond')) 'launch: sans C++ / Libre / ticket au proprietaire mort ne comptent pas -> PASS plafond' $capOk
Clear-Cap
New-Ticket 'T1' 'dd01'; New-Ticket 'T1' 'dd02'; New-Ticket 'T2' 'dd03'
New-InUse 'T2' "OCCUPE par T2`nCPP: oui"
$capDup = Run-Cap
Check ($capDup -match '(?m)^PASS .*plafond') 'launch: 2 tickets du meme worktree + son IN_USE.txt = 1 agent, 2 en tout -> PASS plafond' $capDup

# --- L11-1 : binaire perime PAR LES SOURCES en Launch : FAIL si l'agent va tester (-Scenarios), WARN sinon
$LRepo = Join-Path $Tmp 'HeroesOfLiberty-FakeLaunch'
New-Item -ItemType Directory -Path (Join-Path $LRepo 'Source\M'), (Join-Path $LRepo 'Binaries\Win64') -Force | Out-Null
$LDll = Join-Path $LRepo 'Binaries\Win64\Fake.dll'
Set-Content -Path $LDll -Value 'x'
Set-Content -Path (Join-Path $LRepo 'Source\M\a.cpp') -Value 'int a;'
(Get-Item $LDll).LastWriteTime = (Get-Date).AddMinutes(-10)    # plus recent que le dernier commit de code de main, mais Source ecrit APRES
$lsStale = Run-Cap @('-Scenarios', '-Root', $LRepo)
Check ($lsStale -match '(?m)^FAIL .*(binaire|DLL).*(ANCIEN|perim).*Source') 'launch -Scenarios: Source ecrit apres le binaire -> FAIL' $lsStale
$lnStale = Run-Cap @('-Root', $LRepo)
Check ($lnStale -match '(?m)^WARN .*(binaire|DLL).*(ANCIEN|perim).*Source' -and -not ($lnStale -match '(?m)^FAIL .*(binaire|DLL).*Source')) 'launch sans -Scenarios: Source ecrit apres le binaire -> WARN seulement' $lnStale
(Get-Item $LDll).LastWriteTime = (Get-Date).AddMinutes(1)
$lsFresh = Run-Cap @('-Scenarios', '-Root', $LRepo)
Check (-not ($lsFresh -match '(?m)^(FAIL|WARN) .*(binaire|DLL).*Source')) 'launch -Scenarios: binaire plus recent que les Source -> pas de signal' $lsFresh

# --- L87 (L12-2, 2026-11-12) : Launch FAIL quand le worktree a PLUS de 5 commits touchant Source posterieurs a sa DLL (meme sans -Scenarios).
# Vrai depot jetable : DLL d'il y a 1 h, 5 commits Source + 1 commit Script apres elle -> pas de FAIL ; 6e commit Source -> FAIL.
$SRepo = Join-Path $Tmp 'HeroesOfLiberty-FakeSrcLag'
New-Item -ItemType Directory -Path (Join-Path $SRepo 'Source\M'), (Join-Path $SRepo 'Script'), (Join-Path $SRepo 'Binaries\Win64') -Force | Out-Null
function GitS { git -C $SRepo -c user.name=t -c user.email=t@t @args 2>&1 | Out-Null }
GitS init -q
$SDll = Join-Path $SRepo 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
Set-Content -Path $SDll -Value 'x'
(Get-Item $SDll).LastWriteTime = (Get-Date).AddHours(-1)
foreach ($i in 1..5) { Set-Content -Path (Join-Path $SRepo 'Source\M\a.cpp') -Value "int a$i;"; GitS add Source; GitS commit -q -m "src $i" }
Set-Content -Path (Join-Path $SRepo 'Script\s.as') -Value 'x'; GitS add Script; GitS commit -q -m 'script'
$lag5 = Run-Cap @('-Root', $SRepo)
Check (-not ($lag5 -match '(?m)^FAIL .*commit\(s\) Source')) 'L87 launch: 5 commits Source apres la DLL (+1 Script) -> pas de FAIL de retard' $lag5
Set-Content -Path (Join-Path $SRepo 'Source\M\a.cpp') -Value 'int a6;'; GitS add Source; GitS commit -q -m 'src 6'
$lag6 = Run-Cap @('-Root', $SRepo)
Check ($lag6 -match '(?m)^FAIL .*6 commit\(s\) Source') 'L87 launch: 6 commits Source apres la DLL -> FAIL (sync_worktrees.ps1 sans -NoBuild)' $lag6

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

# --- L9-4 (2026-11-12) : session_gate -Phase Close lance eol_check.ps1 sur chaque worktree (annonce par L9-4, constate NON BRANCHE le 2026-11-12) :
# un fichier suivi aux fins de ligne MELEES (Edit multi-lignes refuse, conflits totaux a la fusion) = WARN qui nomme le worktree et le fichier ;
# un worktree propre = PASS. Deux vrais depots jetables dans un faux dossier de worktrees (-WorktreesDir).
$EolWts = Join-Path $Tmp 'eolwts'
$EolMix = Join-Path $EolWts 'HeroesOfLiberty-EolMixed'
$EolClean = Join-Path $EolWts 'HeroesOfLiberty-EolClean'
foreach ($er in $EolMix, $EolClean) {
  New-Item -ItemType Directory -Path $er -Force | Out-Null
  git -C $er init -q 2>&1 | Out-Null
  Copy-Item $Attr (Join-Path $er '.gitattributes')
  [IO.File]::WriteAllBytes((Join-Path $er 'clean.md'), [Text.Encoding]::UTF8.GetBytes("a`r`nb`r`n"))
}
[IO.File]::WriteAllBytes((Join-Path $EolMix 'mixed.md'), [Text.Encoding]::UTF8.GetBytes("a`r`nb`nc`r`n"))
foreach ($er in $EolMix, $EolClean) {
  git -C $er add -A 2>&1 | Out-Null
  git -C $er -c user.name=t -c user.email=t@t commit -q -m t 2>&1 | Out-Null
}
# Ligne 99 (recette 2026-11-12) : si `git ls-files` echoue (ici un .git qui pointe dans le vide), la liste vide valait PASS a tort.
$EolBroken = Join-Path $EolWts 'HeroesOfLiberty-EolBroken'
New-Item -ItemType Directory -Path $EolBroken -Force | Out-Null
Set-Content (Join-Path $EolBroken '.git') 'gitdir: C:/nonexistent_hol_gitdir' -Encoding ascii
$eolBr = & powershell -NoProfile -ExecutionPolicy Bypass -File $Eol -Root $EolBroken | Out-String
$eolBrCode = $LASTEXITCODE
Check ($eolBrCode -eq 2 -and $eolBr -match 'git ls-files' -and -not ($eolBr -match 'aucun fichier mixte')) 'ligne 99 : eol_check sur un depot illisible -> code 2 et ERREUR git ls-files, jamais "aucun fichier mixte"' "code=$eolBrCode out=$eolBr"
$eolPre =& powershell -NoProfile -ExecutionPolicy Bypass -File $Eol -Root $EolMix | Out-String
Check ($eolPre -match 'MIXTE mixed\.md') 'L9-4 : le faux worktree EolMixed a bien un fichier mixte (pre-condition)' $eolPre
$eolClose = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -End 23:59 -WorktreesDir $EolWts -RetoursDir $Ret | Out-String
Check ($eolClose -match '(?m)^(WARN|FAIL) .*HeroesOfLiberty-EolMixed.*fins de ligne.*mixed\.md') 'L9-4 close: worktree aux fins de ligne melees -> WARN/FAIL qui nomme le fichier' $eolClose
Check ($eolClose -match '(?m)^PASS .*HeroesOfLiberty-EolClean.*fins de ligne' -and -not ($eolClose -match '(?m)^(WARN|FAIL) .*HeroesOfLiberty-EolClean.*fins de ligne')) 'L9-4 close: worktree aux fins de ligne propres -> PASS fins de ligne' $eolClose
Check ($eolClose -match '(?m)^(WARN|FAIL) .*HeroesOfLiberty-EolBroken.*fins de ligne non controlees' -and -not ($eolClose -match '(?m)^PASS .*HeroesOfLiberty-EolBroken.*fins de ligne')) 'ligne 99 close: git ls-files en echec -> fins de ligne non controlees, jamais PASS' $eolClose

# --- L7-1, L7-4, L7-6 : agent_rules.md
$r = Get-Content $Rules -Raw -Encoding UTF8
Check ($r -match '45 appels') 'agent_rules: arret a 45 appels vus par l agent (L9-2)'
Check ($r -match 'tool_uses') 'agent_rules: tool_uses releve par la session principale'
Check ($r -match 'PROCESSUS_DETACHES') 'agent_rules: champ PROCESSUS_DETACHES'
Check ($r -match 'GESTE' -and $r -match 'SON') 'agent_rules: statuts GESTE et SON'
Check ($r -match 'stray_processes') 'agent_rules: stray_processes.ps1 cite'
# L13-2 / L88 / L89 : marqueur CPP du IN_USE.txt, LAUNCHCHECK_DEBUT, TESTS_TAUTOLOGIQUES dans agent_rules.md ET dans le modele d'enonce d'Auto_Journey.md
$ajText = Get-Content (Join-Path (Split-Path -Parent (Split-Path -Parent $Here)) 'Auto_Journey.md') -Raw -Encoding UTF8
$ajModel = if ($ajText -match '(?s)## Mod.le d..nonc. d.un agent \(.*?```(.*?)```') { $Matches[1] } else { '' }
foreach ($k in 'CPP: oui', 'LAUNCHCHECK_DEBUT', 'TESTS_TAUTOLOGIQUES') {
  Check ($r.Contains($k)) "agent_rules: cite '$k'"
  Check ($ajModel.Contains($k)) "Auto_Journey.md modele d'enonce: cite '$k'"
}
Check ($ajText -match 'L12-6') 'Auto_Journey.md: fusion = index de Retours, LISEZ-MOI et EtapeDefini dans le meme commit (L12-6)'

# --- L90 (L12-6) : Close WARN quand EtapeDefini.md dit 'pas dans main' pour une branche (ou des commits cites) deja dans main
$ERepo = Join-Path $Tmp 'FakeMainCopy'
New-Item -ItemType Directory -Path $ERepo -Force | Out-Null
function GitE { git -C $ERepo -c user.name=t -c user.email=t@t @args 2>&1 | Out-Null }
GitE init -q -b main; Set-Content (Join-Path $ERepo 'r.txt') 'r'; GitE add r.txt; GitE commit -q -m root
GitE checkout -q -b featx; Set-Content (Join-Path $ERepo 'x.txt') 'x'; GitE add x.txt; GitE commit -q -m x
$shaX = (git -C $ERepo rev-parse --short HEAD).Trim()
GitE checkout -q main; GitE merge -q --no-ff featx -m 'merge featx'
GitE checkout -q -b featy; Set-Content (Join-Path $ERepo 'y.txt') 'y'; GitE add y.txt; GitE commit -q -m y
$shaY = (git -C $ERepo rev-parse --short HEAD).Trim()
GitE checkout -q main
Set-Content -Path (Join-Path $ERepo 'EtapeDefini.md') -Encoding UTF8 -Value @(
  "- AgentX (featx, sans C++, 2026-11-13) : **outil fait ($shaX), pas dans main.**",
  "- AgentY (featy, sans C++, 2026-11-13) : **outil fait ($shaY), pas dans main.**",
  "- AgentZ (featz-inconnue, sans C++) : **rien, pas dans main.**")
$EWts = Join-Path $Tmp 'emptywts'; New-Item -ItemType Directory -Path $EWts -Force | Out-Null
$etClose = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -End 23:59 -WorktreesDir $EWts -RetoursDir $Ret -EtapeFile (Join-Path $ERepo 'EtapeDefini.md') | Out-String
Check ($etClose -match '(?m)^WARN .*featx.*pas dans main') 'L90 close: EtapeDefini dit pas dans main pour featx deja dans main -> WARN' $etClose
Check (-not ($etClose -match '(?m)^WARN .*(featy|featz).*pas dans main')) 'L90 close: featy (hors main) et branche inconnue -> pas de WARN' $etClose

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
# 2026-11-09 : seule la ligne 'branches:' compte ; un nom de branche cite en PROSE (fiche sans ligne branches:) ne fait plus FAIL, il fait WARN.
Check ($eProse -match "(?m)^WARN .*branches:" -and -not ($eProse -match "(?m)^FAIL .*fixb")) 'essay: sans ligne branches:, un nom de branche en prose ne fait pas FAIL (WARN)' $eProse
# la prose contient le mot 'branches' et un nom de branche absent du worktree, la vraie ligne est plus bas : seule la ligne compte
Set-Content -Path (Join-Path $RetE 'E-prose2.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-prose2 : essai`nGESTE: r$($E)el`nLes branches du correctif : fixb (a fusionner plus tard), voir fixb.`nbranches: fixa"
$eProse2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-prose2 | Out-String
Check ($eProse2 -match "(?m)^PASS .*fixa" -and -not ($eProse2 -match "(?m)^FAIL .*(fixb|branche)")) 'essay: prose contenant "branches" + fixb, ligne branches: fixa -> PASS fixa, aucun FAIL' $eProse2
# sans -Essay (tous les E-*.md ouverts) : la fiche en prose pure ne fait pas FAIL non plus
$eAll = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE | Out-String
Check (-not ($eAll -match "(?m)^FAIL .*(E-prose|E-none)")) 'essay sans -Essay : les fiches en prose (sans ligne branches:) ne font pas FAIL' $eAll
# -Essay accepte le NOM DU SCENARIO : la fiche est retrouvee par sa commande play.ps1 <scenario>
Set-Content -Path (Join-Path $RetE 'E-fiche_autre_nom.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-fiche_autre_nom : essai`nGESTE: r$($E)el`nCommande : powershell -File play.ps1 scen_alias -Root X`nbranches: fixa, fixb"
$eScn = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay scen_alias | Out-String
Check ($eScn -match "(?m)^FAIL .*fixb" -and -not ($eScn -match "(?m)^FAIL .*introuvable")) 'essay: -Essay <nom du scenario> retrouve la fiche par play.ps1 <scenario> (FAIL fixb)' $eScn
$eNone = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-none | Out-String
Check ($eNone -match "(?m)^WARN .*branches:") 'essay: aucune branche citee -> WARN (ajouter la ligne branches:)' $eNone

# --- L12-1c (2026-11-12) : session_gate Launch : WARN si la memoire virtuelle libre < seuil (8 Go) ou si un cl.exe d'un AUTRE moteur compile
$VmRoot = Join-Path $Tmp 'HeroesOfLiberty-FakeVm'
New-Item -ItemType Directory -Path $VmRoot | Out-Null
$vmLow = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Root $VmRoot -MinFreeVirtualGB 100000 | Out-String
Check ($vmLow -match '(?m)^WARN .*memoire virtuelle libre') 'launch: memoire virtuelle libre < seuil -> WARN (mesure reelle, seuil 100000 Go)' $vmLow
$vmOk = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Root $VmRoot -MinFreeVirtualGB 0 | Out-String
Check ($vmOk -match '(?m)^PASS .*memoire virtuelle libre' -and -not ($vmOk -match '(?m)^WARN .*memoire virtuelle libre')) 'launch: memoire virtuelle libre >= seuil -> PASS' $vmOk
$FakeCl = Join-Path $Tmp 'cl.exe'
Copy-Item (Join-Path $env:SystemRoot 'System32\cmd.exe') $FakeCl
$FakeEngine = 'C:\UE\FakeHolEngine'
$clOther = Start-Process $FakeCl -ArgumentList "/c `"set MARK=C:\UE\FakeOtherEngine\Engine & ping -n 120 127.0.0.1 >nul`"" -PassThru -WindowStyle Hidden
$clOurs = Start-Process $FakeCl -ArgumentList "/c `"set MARK=$FakeEngine\Engine\Binaries\DotNET & ping -n 120 127.0.0.1 >nul`"" -PassThru -WindowStyle Hidden
Start-Sleep -Milliseconds 1500
$clOut = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -Root $VmRoot -EngineRoot $FakeEngine | Out-String
Check ($clOut -match "(?m)^WARN .*cl\.exe d'un autre moteur.*\b$($clOther.Id)\b") "launch: cl.exe d'un autre moteur -> WARN avec son PID ($($clOther.Id))" $clOut
Check (-not ($clOut -match "(?m)^WARN .*cl\.exe.*\b$($clOurs.Id)\b")) "launch: cl.exe du moteur du projet ($($clOurs.Id)) -> pas de WARN" $clOut
foreach ($cp in $clOther, $clOurs) { Stop-Process -Id $cp.Id -Force -ErrorAction SilentlyContinue }
Get-CimInstance Win32_Process -Filter "Name='PING.EXE'" -ErrorAction SilentlyContinue | Where-Object { $_.ParentProcessId -in @($clOther.Id, $clOurs.Id) } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }

# --- L8-1 : la ligne branches: est dans le gabarit et dans les regles
$Tpl = Get-Content (Join-Path $Here 'E_template.md') -Raw -Encoding UTF8
Check ($Tpl -match '(?m)^branches\s*:') 'E_template: ligne branches:'
Check ($r -match 'branches\s*:') 'agent_rules: ligne branches: des essais'

Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
if ($Fails -gt 0) { Write-Output "=> $Fails FAIL"; exit 1 } else { Write-Output '=> tout PASS'; exit 0 }
