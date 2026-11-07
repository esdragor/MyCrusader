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

# --- L7-1, L7-4, L7-6 : agent_rules.md
$r = Get-Content $Rules -Raw -Encoding UTF8
Check ($r -match '60 appels') 'agent_rules: arret a 60 appels declares'
Check ($r -match 'tool_uses') 'agent_rules: tool_uses releve par la session principale'
Check ($r -match 'PROCESSUS_DETACHES') 'agent_rules: champ PROCESSUS_DETACHES'
Check ($r -match 'GESTE' -and $r -match 'SON') 'agent_rules: statuts GESTE et SON'
Check ($r -match 'stray_processes') 'agent_rules: stray_processes.ps1 cite'

Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
if ($Fails -gt 0) { Write-Output "=> $Fails FAIL"; exit 1 } else { Write-Output '=> tout PASS'; exit 0 }
