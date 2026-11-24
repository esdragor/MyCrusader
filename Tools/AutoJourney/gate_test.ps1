# gate_test.ps1 : test d'effet des outils de processus (L7-1, L7-4, L7-6, L7-8, L7-2). Aucun C++, aucun editeur.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (sortie : PASS/FAIL par cas, code 1 s'il y a un FAIL)
# Les cas travaillent sur de FAUX worktrees dans %TEMP% (processus factices, DLL factice, Retours factice).
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Gate = Join-Path $Here 'session_gate.ps1'
$Stray = Join-Path $Here 'stray_processes.ps1'
$Rules = Join-Path $Here 'agent_rules.md'
$Fails = 0
$Passes = 0
$T0 = Get-Date
$env:HOL_GATE_SKIP_NW = '1'   # le test complet lance session_gate ~100 fois ; no_window_test (7 s) a son propre test
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { $script:Passes++; Write-Output ("PASS $Name  (t={0}s)" -f [int]((Get-Date) - $T0).TotalSeconds) } else { $script:Fails++; Write-Output ("FAIL $Name  (t={0}s) $Detail" -f [int]((Get-Date) - $T0).TotalSeconds) }
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
# 2026-11-23 09:13 : '-Scenarios faction_click' (nom de scenario apres le switch) etait lu comme -End (1er parametre positionnel libre) -> 'il reste -1065517033 min'.
# Sans -End nomme : l'heure de fin doit venir de SessionEnd.txt, le nom de scenario ne doit jamais etre pris pour l'heure.
$ScnEnd = Join-Path $Tmp 'SessionEnd_scn.txt'
[IO.File]::WriteAllText($ScnEnd, (Get-Date).AddHours(3).ToString('yyyy-MM-dd HH:mm'), (New-Object Text.UTF8Encoding $false))
$scnPos = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -NoCpp -SessionEndFile $ScnEnd -Root $Wt -Scenarios faction_click | Out-String
Check ($scnPos -match '(?m)^PASS temps restant 1[0-9]{2} min' -and -not ($scnPos -match 'il reste -')) 'launch -Scenarios <nom> : le nom de scenario n est pas pris pour l heure de fin (temps restant lu de SessionEnd.txt)' $scnPos

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

# --- L14-6 (BILAN-1123pm) : rouges C++ jamais vus. -Phase Report -ReportFile <rapport> refuse un TEST_ROUGE « suppose » sans justification.
$RepSup = Join-Path $Tmp 'rapport-suppose.md'
Set-Content -Path $RepSup -Encoding UTF8 -Value "TEST_ROUGE: suppose rouge (commit abc1234) + assertit l'effet : oui   TEST_ROUGE_SEPARE: oui"
$rsup = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo -ReportFile $RepSup | Out-String
Check ($rsup -match '(?m)^FAIL .*L14-6.*suppos') 'L14-6 report: TEST_ROUGE suppose sans justification -> FAIL' $rsup
$RepJus = Join-Path $Tmp 'rapport-suppose-justifie.md'
Set-Content -Path $RepJus -Encoding UTF8 -Value "TEST_ROUGE: suppose rouge (commit abc1234) ; justification : lot C++ non compile, le rouge se verra au build de la session principale"
$rjus = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo -ReportFile $RepJus | Out-String
Check ($rjus -match '(?m)^WARN .*L14-6.*suppos' -and -not ($rjus -match '(?m)^FAIL .*L14-6')) 'L14-6 report: TEST_ROUGE suppose AVEC justification -> WARN, pas FAIL' $rjus
$RepVu = Join-Path $Tmp 'rapport-vu.md'
Set-Content -Path $RepVu -Encoding UTF8 -Value "TEST_ROUGE: vu a 19:41 : 1410/1419 (commit abc1234) + assertit l'effet : oui"
$rvu = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo -ReportFile $RepVu | Out-String
Check ($rvu -match '(?m)^PASS .*L14-6' -and -not ($rvu -match '(?m)^(FAIL|WARN) .*L14-6')) 'L14-6 report: TEST_ROUGE vu a l execution -> PASS' $rvu
$RepNone = Join-Path $Tmp 'rapport-introuvable.md'
$rnone = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo -ReportFile $RepNone | Out-String
Check ($rnone -match '(?m)^FAIL .*L14-6.*introuvable') 'L14-6 report: rapport introuvable -> FAIL' $rnone
# --- SAURON-1124sd anomalies : (1) Report refusait Placeholders.txt / Docs\SessionEnd.txt modifies dans la copie principale (Merge les accepte) ; (2) « vu a 10:00 (pas suppose) » donnait un faux FAIL.
$FakeMain = Join-Path $Tmp 'FakeMain'
New-Item -ItemType Directory -Path (Join-Path $FakeMain 'Docs') -Force | Out-Null
git -C $FakeMain init -q 2>$null | Out-Null
Set-Content -Path (Join-Path $FakeMain 'Placeholders.txt') -Value 'a'
Set-Content -Path (Join-Path $FakeMain 'Docs\SessionEnd.txt') -Value 'a'
git -C $FakeMain add . 2>$null | Out-Null
git -C $FakeMain -c user.name=t -c user.email=t@t commit -q -m docs 2>$null | Out-Null
Set-Content -Path (Join-Path $FakeMain 'Placeholders.txt') -Value 'b'
Set-Content -Path (Join-Path $FakeMain 'Docs\SessionEnd.txt') -Value 'b'
$rdoc = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo -MainDir $FakeMain | Out-String
Check ($rdoc -match '(?m)^PASS .*copie principale : seulement des docs voulus' -and -not ($rdoc -match '(?m)^FAIL .*copie principale')) 'report: Placeholders.txt + Docs\SessionEnd.txt modifies dans la copie principale -> PASS (comme Merge)' $rdoc
$RepNeg = Join-Path $Tmp 'rapport-vu-pas-suppose.md'
Set-Content -Path $RepNeg -Encoding UTF8 -Value "TEST_ROUGE: vu a 10:00 (pas suppose) : 3 FAIL sur 3 (commit abc1234) + assertit l'effet : oui"
$rneg = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Repo -ReportFile $RepNeg | Out-String
Check ($rneg -match '(?m)^PASS .*L14-6' -and -not ($rneg -match '(?m)^(FAIL|WARN) .*L14-6')) 'L14-6 report: « vu a 10:00 (pas suppose) » -> PASS, pas de faux FAIL' $rneg

# --- L7-8 + L7-6 : Close -Exclude et ligne GESTE / SON des E-*.md
$Ret = Join-Path $Tmp 'Retours'
New-Item -ItemType Directory -Path $Ret | Out-Null
Set-Content -Path (Join-Path $Ret 'E-bad.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-bad : un essai sans statut`nSTATUT : VU $A L'$($E)CRAN"
Set-Content -Path (Join-Path $Ret 'E-good.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-good : un essai complet`nSTATUT : VU $A L'$($E)CRAN`nGESTE: r$($E)el`nSON: non entendu"
Set-Content -Path (Join-Path $Ret 'E-done.md') -Encoding UTF8 -Value "# [OK] E-done : essai deja valide`nSTATUT : VU"
Set-Content -Path (Join-Path $Ret 'E-interne.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-interne : essai au geste interne`nSTATUT : LOGIQUE SEULEMENT`nGESTE: interne`nSON: non entendu"
$close = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -Exclude Faramir -RetoursDir $Ret | Out-String
Check (-not ($close -match '(?m)^(PASS|FAIL)\s+HeroesOfLiberty-Faramir ')) 'close -Exclude Faramir: aucun controle PASS/FAIL pour ce worktree' $close
Check ($close -match '(?m)^WARN\s+HeroesOfLiberty-Faramir : exclu') 'close -Exclude: une ligne dit que Faramir est exclu' $close
Check ($close -match '(?m)^WARN .*E-bad\.md.*GESTE' -and $close -match 'SON') 'close: E-bad.md signale (GESTE et SON manquants)' $close
Check (-not ($close -match 'E-good\.md.*(GESTE|SON)') -and -not ($close -match 'E-done\.md.*(GESTE|SON)')) 'close: E-good.md complet et E-done.md clos ne sont pas signales' $close
# SAURON-1124sc, L14-5 : un essai [A ESSAYER] au GESTE interne est liste (WARN) ; un essai au geste reel, un essai clos et un essai sans ligne GESTE n'y figurent pas
Check ($close -match '(?m)^WARN .*L14-5.*geste interne.*E-interne\.md') 'L14-5 close: E-interne.md (GESTE: interne) liste dans un WARN geste interne' $close
Check (-not ($close -match '(?m)^WARN .*L14-5[^\r\n]*(E-good|E-done|E-bad)\.md')) 'L14-5 close: E-good (reel), E-done (clos), E-bad (sans GESTE) ne sont pas listes' $close
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
  "- AgentZ (featz-inconnue, sans C++) : **rien, pas dans main.**",
  "- AgentW (featx, sans C++) : **docs seulement, pas dans main.**")
$EWts = Join-Path $Tmp 'emptywts'; New-Item -ItemType Directory -Path $EWts -Force | Out-Null
$etClose = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -End 23:59 -WorktreesDir $EWts -RetoursDir $Ret -EtapeFile (Join-Path $ERepo 'EtapeDefini.md') | Out-String
Check ($etClose -match '(?m)^WARN .*commits .*AgentX.*pas dans main') 'L90 close: EtapeDefini dit pas dans main pour les commits de featx deja dans main -> WARN' $etClose
Check (-not ($etClose -match '(?m)^WARN .*(featy|featz).*pas dans main')) 'L90 close: featy (hors main) et branche inconnue -> pas de WARN' $etClose
Check ($etClose -match '(?m)^WARN .*branche featx.*AgentW') 'L90 close: ligne sans SHA, branche featx (1er mot entre parentheses) deja dans main -> WARN' $etClose

# --- L15-1 (2026-11-15, demande de l'utilisateur) : Close FAIL pour tout worktree de '## Agents du <date>' (-JourFile) qui n'est pas a main (+0/-0),
# qui a des commits Source plus recents que sa DLL, ou dont Get-StaleBinaryProblem (Tools\Scenarios\LaunchCheck.ps1) n'est pas vide ; la copie
# principale aussi si elle figure au tableau (-MainRoot). Vrais depots jetables : HeroesOfLiberty-T1 (branche work, 1 commit Source d'avance sur
# main, DLL de 2000), HeroesOfLiberty-Other (meme retard mais hors du tableau du jour : non controle), fausse copie principale a main, DLL fraiche.
$JWts = Join-Path $Tmp 'jourwts'
$JT1 = Join-Path $JWts 'HeroesOfLiberty-T1'; $JOther = Join-Path $JWts 'HeroesOfLiberty-Other'; $JMain = Join-Path $Tmp 'FakeMainL15'
function GitJ([string]$Dir) { git -C $Dir -c user.name=t -c user.email=t@t @args 2>&1 | Out-Null }
foreach ($jr in $JT1, $JOther, $JMain) {
  New-Item -ItemType Directory -Path (Join-Path $jr 'Source\M'), (Join-Path $jr 'Binaries\Win64') -Force | Out-Null
  GitJ $jr init -q -b main
  Set-Content (Join-Path $jr 'Source\M\a.cpp') 'int a;'; GitJ $jr add Source; GitJ $jr commit -q -m base
  $jd = Join-Path $jr 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'; Set-Content $jd 'x'; (Get-Item $jd).LastWriteTime = (Get-Date).AddMinutes(2)
}
foreach ($jr in $JT1, $JOther) {
  GitJ $jr checkout -q -b work
  Set-Content (Join-Path $jr 'Source\M\a.cpp') 'int a2;'; GitJ $jr add Source; GitJ $jr commit -q -m 'src work'
  (Get-Item (Join-Path $jr 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll')).LastWriteTime = [datetime]'2000-01-01'
}
$JFile = Join-Path $Tmp 'Jour.md'
$jToday = (Get-Date).ToString('yyyy-MM-dd')
Set-Content -Path $JFile -Encoding UTF8 -Value @('## Agents du 2000-01-01', '| Agent | id | worktree | tache |', '|---|---|---|---|', '| OLD | x | HeroesOfLiberty-Other | ancien |', '',
  "## Agents du $jToday", '', '| Agent | id | worktree | tache | C++ | lance |', '|---|---|---|---|---|---|',
  '| T1-AGENT | a1 | HeroesOfLiberty-T1 | outil | non | 09:00 |', '| NETTOYAGE | a2 | copie principale | docs (HeroesOfLiberty-Other cite en tache) | non | 09:01 |', '| LECTEUR | a3 | lecture seule | plan | non | 09:02 |', '',
  '## Autre section', '| X | y | HeroesOfLiberty-Other | z |')
function Run-J { & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -End 23:59 -WorktreesDir $JWts -RetoursDir $Ret -JourFile $JFile -MainRoot $JMain -EtapeFile (Join-Path $ERepo 'EtapeDefini.md') | Out-String }
$jAhead = [int](git -C $JT1 rev-list --count 'main..HEAD'); $jSrc = [int](git -C $JT1 rev-list --count HEAD -- Source)   # calcules par git, pas recopies
$jRed = Run-J
Check ($jRed -match "(?m)^FAIL HeroesOfLiberty-T1 : L15-1 pas a main \(\+$jAhead/-0\)") "L15-1 close: worktree du tableau en avance de $jAhead commit sur main -> FAIL +$jAhead/-0" $jRed
Check ($jRed -match "(?m)^FAIL HeroesOfLiberty-T1 : L15-1 $jSrc commit\(s\) Source plus recents que sa DLL") "L15-1 close: $jSrc commits Source apres la DLL -> FAIL qui les compte" $jRed
Check ($jRed -match '(?m)^FAIL HeroesOfLiberty-T1 : L15-1 Get-StaleBinaryProblem.*a\.cpp') 'L15-1 close: Get-StaleBinaryProblem non vide (a.cpp plus recent que la DLL) -> FAIL' $jRed
Check ($jRed -match '(?m)^PASS copie principale : L15-1 a main') 'L15-1 close: copie principale citee au tableau, a main, DLL fraiche -> PASS' $jRed
Check (-not ($jRed -match '(?m)^(PASS|FAIL|WARN) HeroesOfLiberty-Other : L15-1')) 'L15-1 close: worktree hors du tableau du jour (ancienne section, autre section, colonne tache) -> non controle' $jRed
GitJ $JT1 branch -f main work
(Get-Item (Join-Path $JT1 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll')).LastWriteTime = (Get-Date).AddMinutes(2)
$jGreen = Run-J
Check ($jGreen -match '(?m)^PASS HeroesOfLiberty-T1 : L15-1 a main' -and -not ($jGreen -match '(?m)^FAIL .*L15-1')) 'L15-1 close: T1 a main (branch -f) et DLL rebuildee -> PASS, aucun FAIL L15-1' $jGreen
# D808 (2026-11-15, reponse a Q-close-gate-l15-1 = B + B) : Q1 B, des commits Source plus recents que la DLL ne font FAIL que si Get-StaleBinaryProblem
# est aussi non vide (un fichier plus recent que la DLL prouve un build manquant), sinon WARN (commit apres le build, fichier deja compile).
# Q2 B : la copie principale (docs seulement) n'est controlee que sur 'a main' (+0/-0), jamais sur sa DLL.
(Get-Item (Join-Path $JT1 'Source\M\a.cpp')).LastWriteTime = [datetime]'2000-01-01'
(Get-Item (Join-Path $JT1 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll')).LastWriteTime = [datetime]'2001-01-01'   # build apres le fichier, avant les commits
(Get-Item (Join-Path $JMain 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll')).LastWriteTime = [datetime]'2000-01-01'  # copie principale : DLL ancienne
$jLagW = [int](git -C $JT1 rev-list --count HEAD -- Source)   # tous les commits Source sont posterieurs a 2001 : calcule par git
$jStaleT1 = & { param($s, $r) . $s; Get-StaleBinaryProblem $r } (Join-Path (Split-Path -Parent $Here) 'Scenarios\LaunchCheck.ps1') $JT1
Check (-not $jStaleT1) 'D808 pre-condition: T1 sans fichier Source plus recent que la DLL (Get-StaleBinaryProblem vide)' $jStaleT1
$jWarn = Run-J
Check ($jWarn -match "(?m)^WARN HeroesOfLiberty-T1 : L15-1 $jLagW commit\(s\) Source plus recents que sa DLL" -and -not ($jWarn -match '(?m)^FAIL HeroesOfLiberty-T1 : L15-1')) "D808 Q1 close: $jLagW commits Source apres la DLL, Get-StaleBinaryProblem vide -> WARN, pas FAIL" $jWarn
Check ($jWarn -match '(?m)^PASS copie principale : L15-1 a main' -and -not ($jWarn -match '(?m)^(FAIL|WARN) copie principale : L15-1')) 'D808 Q2 close: copie principale a main, DLL ancienne -> PASS a main, DLL non controlee' $jWarn
$jNoSec = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -End 23:59 -WorktreesDir $JWts -RetoursDir $Ret -JourFile (Join-Path $Tmp 'absentJour.md') -EtapeFile (Join-Path $ERepo 'EtapeDefini.md') | Out-String
Check ($jNoSec -match "(?m)^WARN L15-1 : pas de section '## Agents du") 'L15-1 close: pas de tableau du jour -> WARN (non controle), jamais PASS muet' $jNoSec

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
# FARAMIR-1121a : un mot de 'branches:' qui se resout en commit (sha) est accepte et controle comme ancetre du -Root ; le texte entre parentheses est ignore
$shaA = (git -C $G rev-parse --short fixa).Trim()
$shaB = (git -C $G rev-parse --short fixb).Trim()
Set-Content -Path (Join-Path $RetE 'E-shaok.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-shaok : essai`nGESTE: r$($E)el`nbranches: work $shaA (integ-37, contient integ-25 $shaB)"
Set-Content -Path (Join-Path $RetE 'E-shabad.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-shabad : essai`nGESTE: r$($E)el`nbranches: work $shaB (integ-37)"
Set-Content -Path (Join-Path $RetE 'E-typo.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-typo : essai`nGESTE: r$($E)el`nbranches: nexistepas"
$eShaOk = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-shaok | Out-String
Check ($eShaOk -match "(?m)^PASS .*$shaA" -and -not ($eShaOk -match "(?m)^FAIL ")) 'essay: un sha dans branches: (texte entre parentheses ignore) -> PASS, aucun FAIL' $eShaOk
$eShaBad = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-shabad | Out-String
Check ($eShaBad -match "(?m)^FAIL .*$shaB.*ABSENTE" -and -not ($eShaBad -match "(?m)^FAIL .*inconnue")) 'essay: un sha non ancetre du Root -> FAIL ABSENTE (pas inconnue)' $eShaBad
$eTypo = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay E-typo | Out-String
Check ($eTypo -match "(?m)^FAIL .*nexistepas.*inconnue") 'essay: un mot qui n est ni branche ni commit -> FAIL inconnue' $eTypo

# --- AX-9 (2026-11-24, HALDIR-1124j ; regle 13 de l'utilisateur 2026-11-14) : une fiche E-* dont la ligne de commande est ABREGEE ("meme commande avec ...",
# "idem", "comme ci-dessus") sans `play.ps1 <nom>` complet sur cette ligne fait FAIL "commande abregee" en phase Essay ; une commande complete, ou une phrase
# qui CITE l'abreviation pour l'interdire ("jamais ..."), ne fait pas FAIL.
$Ecirc = [string][char]0xEA   # e circonflexe
Set-Content -Path (Join-Path $RetE 'E-abbrev.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-abbrev : essai`nGESTE: r$($E)el`nbranches: fixa`nCommande : m$($Ecirc)me commande avec -TimeoutSec 60"
Set-Content -Path (Join-Path $RetE 'E-abbrev2.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-abbrev2 : essai`nGESTE: r$($E)el`nbranches: fixa`nCommande : meme commande avec -Hidden"
Set-Content -Path (Join-Path $RetE 'E-abbrev3.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-abbrev3 : essai`nGESTE: r$($E)el`nbranches: fixa`nCommande : idem"
Set-Content -Path (Join-Path $RetE 'E-abbrev4.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-abbrev4 : essai`nGESTE: r$($E)el`nbranches: fixa`nCommande : comme ci-dessus, avec -TimeoutSec 90"
Set-Content -Path (Join-Path $RetE 'E-fullcmd.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-fullcmd : essai`nGESTE: r$($E)el`nbranches: fixa`nCommande : powershell -ExecutionPolicy Bypass -File X\Tools\Scenarios\play.ps1 scen_full -Root X"
Set-Content -Path (Join-Path $RetE 'E-forbid.md') -Encoding UTF8 -Value "# [$A ESSAYER] E-forbid : essai`nGESTE: r$($E)el`nbranches: fixa`nCommande compl$($E)te : jamais m$($Ecirc)me commande avec ... ni idem`nCommande : powershell -File play.ps1 scen_forbid -Root X"
foreach ($ab in 'E-abbrev', 'E-abbrev2', 'E-abbrev3', 'E-abbrev4') {
  $eAb = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay $ab | Out-String
  Check ($eAb -match "(?m)^FAIL .*$ab\.md.*abreg") "AX-9 essay: fiche $ab avec une commande abregee -> FAIL commande abregee" $eAb
}
foreach ($okc in 'E-fullcmd', 'E-forbid') {
  $eFull = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Essay -Root $G -RetoursDir $RetE -Essay $okc | Out-String
  Check (-not ($eFull -match '(?m)^FAIL .*abreg') -and $eFull -match '(?m)^PASS .*fixa') "AX-9 essay: fiche $okc (commande complete, ou abreviation citee pour l'interdire) -> pas de FAIL abregee, PASS fixa" $eFull
}

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

# --- L19-1 (2026-11-20) : (1) Launch exige 'check-list lue (L<N>)' dans la section du jour d'Auto_Journey_Jour.md ; (2) Close compte les commits de code de la session principale
$NoBom = New-Object Text.UTF8Encoding($false)
$LDate = (Get-Date).ToString('yyyy-MM-dd')
$LDir = Join-Path $Tmp 'l191'; New-Item -ItemType Directory -Path $LDir | Out-Null
$LJour = Join-Path $LDir 'Auto_Journey_Jour.md'
[IO.File]::WriteAllText((Join-Path $LDir 'Auto_Journey_Checklist.md'), "| L3-1 | a |`r`n| L19-2 | b |`r`n| L7-9 | c |`r`n", $NoBom)
function Set-LJour([string]$Body) { [IO.File]::WriteAllText($LJour, "# Jour`r`n`r`n## Agents du $LDate`r`n$Body`r`n`r`n## Autre section`r`n", $NoBom) }
function Run-L([string[]]$More = @()) { & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Launch -End 23:59 -JourFile $LJour @More | Out-String }
Set-LJour 'aucune lecture'
$l1 = Run-L
Check ($l1 -match '(?m)^FAIL .*check-list.*L19\b') 'L19-1 launch: pas de ligne check-list lue -> FAIL (derniere lecon = plus grand N numerique, L19)' $l1
Set-LJour 'check-list lue (L3)'
$l2 = Run-L
Check ($l2 -match '(?m)^FAIL .*check-list') 'L19-1 launch: ligne check-list lue (L3) alors que la derniere lecon est L19 -> FAIL' $l2
Set-LJour 'check-list lue (L19) a 08:50'
$l3 = Run-L
Check ($l3 -match '(?m)^PASS .*check-list' -and -not ($l3 -match '(?m)^FAIL .*check-list')) 'L19-1 launch: check-list lue (L19) -> PASS' $l3
Set-LJour 'aucune lecture'
$l4 = Run-L @('-AgentName', 'NETTOYAGE-1120a')
Check ($l4 -match '(?m)^WARN .*check-list' -and -not ($l4 -match '(?m)^FAIL .*check-list')) 'L19-1 launch: agent NETTOYAGE-* sans lecture -> WARN seulement' $l4
$l5 = Run-L @('-AgentName', 'ETAPE0-1120b')
Check ($l5 -match '(?m)^WARN .*check-list' -and -not ($l5 -match '(?m)^FAIL .*check-list')) 'L19-1 launch: agent ETAPE0-* sans lecture -> WARN seulement' $l5
$l6 = Run-L @('-AgentName', 'GLORFINDEL-1120j')
Check ($l6 -match '(?m)^FAIL .*check-list') 'L19-1 launch: agent de travail sans lecture -> FAIL' $l6

# (2) Close : commits de code de la session principale
$CR = Join-Path $Tmp 'closerepo'; $CW = Join-Path $Tmp 'closewts'
New-Item -ItemType Directory -Path $CR, $CW | Out-Null
function Git-C([string[]]$GitArgs) { & git -C $CR @GitArgs 2>&1 | Out-Null }
function Add-CodeCommit([string]$Dir, [string]$Rel, [string]$Msg) {
  $fp = Join-Path $Dir $Rel; New-Item -ItemType Directory -Path (Split-Path $fp) -Force | Out-Null
  [IO.File]::WriteAllText($fp, [guid]::NewGuid().ToString(), $NoBom)
  & git -C $Dir add -- $Rel 2>&1 | Out-Null; & git -C $Dir commit -q -m $Msg 2>&1 | Out-Null
}
Git-C @('init', '-q', '-b', 'main'); Git-C @('config', 'user.email', 'a@b.c'); Git-C @('config', 'user.name', 'T'); Git-C @('config', 'core.autocrlf', 'false')
Add-CodeCommit $CR 'Docs\base.md' 'base'
$cSha0 = (& git -C $CR rev-parse --short HEAD).Trim()
[IO.File]::WriteAllText((Join-Path $CR 'EtapeDefini.md'), "x`r`n", $NoBom)
$CJour = Join-Path $CR 'Auto_Journey_Jour.md'
function Set-CJour([bool]$WithSha) {
  $sl = if ($WithSha) { "Etat au lancement : main = $cSha0 (local)`r`n" } else { '' }
  [IO.File]::WriteAllText($CJour, "# Jour`r`n`r`n## Agents du $LDate`r`n$sl`r`n## Autre`r`n", $NoBom)
}
Set-CJour $true
# travail d'agent : branche d'un worktree HeroesOfLiberty-*, fusionnee dans main (ne compte pas)
Git-C @('worktree', 'add', '-q', '-b', 'agent-a', (Join-Path $CW 'HeroesOfLiberty-A'), 'main')
Add-CodeCommit (Join-Path $CW 'HeroesOfLiberty-A') 'Script\agent_a.as' 'agent a work'
Git-C @('merge', '-q', '--no-ff', 'agent-a', '-m', 'Merge agent-a')
# travail de l'integrateur (branche integ-*, worktree Arwen) : compte comme code de la session principale
Git-C @('worktree', 'add', '-q', '-b', 'integ-1', (Join-Path $CW 'HeroesOfLiberty-Arwen'), 'main')
function Run-C([string[]]$More = @()) { & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Close -End 23:59 -WorktreesDir $CW -RetoursDir (Join-Path $Tmp 'closeretours') -JourFile $CJour -EtapeFile (Join-Path $CR 'EtapeDefini.md') @More | Out-String }
$c0 = Run-C
Check ($c0 -match '(?m)^PASS .*L19-1' -and -not ($c0 -match '(?m)^(WARN|FAIL) .*L19-1')) 'L19-1 close: seul un commit d agent fusionne dans main -> PASS (0)' $c0
Add-CodeCommit (Join-Path $CW 'HeroesOfLiberty-Arwen') 'Script\integ1.as' 'Integration: integ one'
# (a) integ-y extraite dans un AUTRE worktree qu'Arwen = branche de travail d'un agent : non comptee ; (b) cherry-pick sur integ-1 : non compte
Git-C @('worktree', 'add', '-q', '-b', 'integ-y', (Join-Path $CW 'HeroesOfLiberty-B'), 'main')
Add-CodeCommit (Join-Path $CW 'HeroesOfLiberty-B') 'Script\integy.as' 'Integration: agent owned integ y'
Add-CodeCommit (Join-Path $CW 'HeroesOfLiberty-A') 'Script\cpsrc.as' 'cherry source from agent'
$cpSha = (& git -C (Join-Path $CW 'HeroesOfLiberty-A') rev-parse HEAD).Trim()
& git -C (Join-Path $CW 'HeroesOfLiberty-Arwen') cherry-pick $cpSha 2>&1 | Out-Null
# L22-6 (2026-11-23, reponse B) : seul un commit portant la ligne de fin 'Agent: <nom>' (trailer) est exempte ; un commit de code sur integ-1 SANS trailer compte
# ('Integration: integ one' ci-dessus), AVEC trailer non (le commit ci-dessous ne doit ni compter ni etre liste)
Add-CodeCommit (Join-Path $CW 'HeroesOfLiberty-Arwen') 'Script\integ_trailer.as' "Integration: trailer carrier`n`nAgent: GALADRIEL-1123d"
$ci = Run-C
Check ($ci -match '(?m)^WARN .*L19-1.*\b1\b' -and -not ($ci -match '(?m)^FAIL .*L19-1') -and $ci -match 'Integration: integ one' -and -not ($ci -match 'Merge agent-a|agent a work|agent owned integ y|cherry source from agent')) 'L19-1 close (reflog): 1 commit direct sur integ-1 compte ; fusion, cherry-pick et commit sur integ-y (worktree d agent) non' $ci
Check (-not ($ci -match 'trailer carrier') -and $ci -match '(?m)^WARN .*L19-1.* 1 fois') 'L22-6 close: commit sur integ-1 AVEC trailer Agent: exempte (compte reste 1), SANS trailer compte' $ci
Add-CodeCommit $CR 'Script\main1.as' 'Integration: one'
Add-CodeCommit $CR 'Source\main2.cpp' 'quick_check follows D858 two'
Add-CodeCommit $CR 'Docs\onlydoc.md' 'doc only commit'
$c2 = Run-C
Check ($c2 -match '(?m)^WARN .*L19-1.*\b3\b' -and -not ($c2 -match '(?m)^FAIL .*L19-1')) 'L19-1 close: 2 commits de code directs sur main (+1 doc) + 1 sur integ-1 -> WARN avec le compte 3' $c2
Check ($c2 -match 'Integration: one' -and $c2 -match 'quick_check follows D858 two') 'L19-1 close: la liste donne sha + titre des commits' $c2
Add-CodeCommit $CR 'Tools\Tests\main3.ps1' 'Integration: three'
Add-CodeCommit $CR 'Script\main4.as' 'Integration: four'
$c4 = Run-C
Check ($c4 -match '(?m)^FAIL .*L19-1.*\b5\b') 'L19-1 close: 5 commits de code (> 3) -> FAIL avec le compte 5' $c4
Check ($c4 -match 'agents') 'L19-1 close: le FAIL dit de confier le code aux agents' $c4
$c4b = Run-C @('-MaxMainCodeCommits', '6')
Check ($c4b -match '(?m)^WARN .*L19-1' -and -not ($c4b -match '(?m)^FAIL .*L19-1')) 'L19-1 close: -MaxMainCodeCommits 6 : 5 commits -> WARN' $c4b
Add-CodeCommit $CR 'Script\docs_titled.as' 'Docs: session docs touching a script (L20-3)'
Add-CodeCommit $CR 'Script\Maps\regen.uasset' 'Regenerate maps assets only (L20-3)'
$c6 = Run-C
Check ($c6 -match '(?m)^FAIL .*L19-1.*\b5\b' -and -not ($c6 -match 'Docs: session docs touching|Regenerate maps assets only')) 'L19-1 close (L20-3): un commit Docs: et un commit .uasset seul ne sont pas comptes (toujours 5)' $c6
# --- L27-12 (2026-11-24) : Close WARN quand le bilan (Auto_Journey_Jour.md) n'a pas de ligne 'version launcher' (relevee dans Saved\Manager\Versions\latest.txt)
[IO.File]::WriteAllText($CJour, "# Jour`r`n`r`n## Agents du $LDate`r`n`r`n## Autre`r`n", $NoBom)
$vl0 = Run-C
Check ($vl0 -match '(?m)^WARN .*L27-12.*version launcher') 'L27-12 close: bilan SANS ligne version launcher -> WARN' $vl0
[IO.File]::WriteAllText($CJour, "# Jour`r`n`r`n## Bilan du $LDate`r`n`r`n- Version launcher : 0.4.3 (publie, Launcher 7)`r`n`r`n## Autre`r`n", $NoBom)
$vl1 = Run-C
Check ($vl1 -match '(?m)^PASS .*L27-12.*version launcher' -and -not ($vl1 -match '(?m)^WARN .*L27-12')) 'L27-12 close: bilan AVEC la ligne version launcher -> PASS, aucun WARN L27-12' $vl1
# L27-12 (SAURON-1124sb) : une vieille ligne 'version launcher' dans le bilan d'un AUTRE jour ne compte pas -> WARN (seule la section '## Bilan du <date du jour>' est lue)
[IO.File]::WriteAllText($CJour, "# Jour`r`n`r`n## Bilan du 2020-01-01`r`n`r`n- Version launcher : 0.1.0 (vieux jour)`r`n`r`n## Bilan du $LDate`r`n`r`n- rien ici`r`n`r`n## Autre`r`n", $NoBom)
$vl2 = Run-C
Check ($vl2 -match '(?m)^WARN .*L27-12.*version launcher' -and -not ($vl2 -match '(?m)^PASS .*L27-12')) 'L27-12 close: vieille ligne version launcher d un autre jour (bilan du jour sans la ligne) -> WARN' $vl2
foreach ($wn in 'A', 'B', 'Arwen') { & git -C $CR worktree remove --force (Join-Path $CW "HeroesOfLiberty-$wn") 2>&1 | Out-Null }

Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
Write-Output ("=> {0} PASS, {1} FAIL ({2} s)" -f $Passes, $Fails, [int]((Get-Date) - $T0).TotalSeconds)
if ($Fails -gt 0) { exit 1 } else { exit 0 }
