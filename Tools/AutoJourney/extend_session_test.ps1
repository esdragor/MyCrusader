# extend_session_test.ps1 (L14-13, 2026-11-14) : test d'effet de extend_session.ps1 <HH:MM> et de la lecture d'un Docs\SessionEnd.txt date
# (y compris du lendemain) par session_gate.ps1. Aucun editeur ; travaille sur une copie principale jetable dans %TEMP%.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (une ligne PASS/FAIL par cas, code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'extend_session.ps1'
$Gate = Join-Path $Here 'session_gate.ps1'
$Root = (Resolve-Path (Join-Path $Here '..\..')).Path
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('extsess-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path (Join-Path $Tmp 'Docs') -Force | Out-Null
$EndFile = Join-Path $Tmp 'Docs\SessionEnd.txt'
$Now = Get-Date
$Old = $Now.AddMinutes(20).ToString('yyyy-MM-dd HH:mm')
[IO.File]::WriteAllText($EndFile, $Old + "`r`n")

Check (Test-Path $Tool) 'extend_session.ps1 existe'

# --- Prolonger plus tard dans la journee : la date du jour, l'ancienne et la nouvelle heure imprimees, rappel du brief et des alarmes
$T1 = $Now.AddMinutes(90)
if ($T1.Date -ne $Now.Date) { $T1 = $Now.Date.AddHours(23).AddMinutes(59) }   # pres de minuit : rester aujourd'hui
$o1 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool $T1.ToString('HH:mm') -Root $Tmp | Out-String
$c1 = $LASTEXITCODE
$f1 = ((Get-Content $EndFile -ErrorAction SilentlyContinue | Select-Object -First 1) -as [string])
Check ($c1 -eq 0 -and $f1 -eq $T1.ToString('yyyy-MM-dd HH:mm')) 'HH:MM plus tard aujourd''hui: SessionEnd.txt = date du jour + heure' "code=$c1 file='$f1' out=$o1"
Check ($o1 -match [regex]::Escape($Old) -and $o1 -match [regex]::Escape($T1.ToString('HH:mm'))) 'la sortie cite l''ancienne fin et la nouvelle' $o1
Check ($o1 -match '(?i)ARRET DUR' -and $o1 -match '(?i)alarme') 'la sortie rappelle l''heure d''arret des agents (brief) et les alarmes a recreer' $o1

# --- Une heure deja passee aujourd'hui = le lendemain (seance prolongee apres minuit)
$T2 = $Now.AddMinutes(-2)
if ($T2.Date -ne $Now.Date) { $T2 = $Now }   # juste apres minuit : cas sans objet, l'egalite ci-dessous reste vraie le lendemain
$o2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool $T2.ToString('HH:mm') -Root $Tmp | Out-String
$f2 = ((Get-Content $EndFile -ErrorAction SilentlyContinue | Select-Object -First 1) -as [string])
Check ($LASTEXITCODE -eq 0 -and $f2 -eq $Now.Date.AddDays(1).ToString('yyyy-MM-dd') + ' ' + $T2.ToString('HH:mm')) 'HH:MM deja passee: SessionEnd.txt = LENDEMAIN + heure' "file='$f2' out=$o2"

# --- session_gate lit une fin datee du lendemain sans WARN "pas d'aujourd'hui", et garde le WARN pour une date perimee
$g1 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Root -SessionEndFile $EndFile | Out-String
Check ($g1 -notmatch "(?m)^WARN SessionEnd" -and $g1 -match ('fin dure ' + [regex]::Escape($T2.ToString('HH:mm')))) 'session_gate: fin du lendemain acceptee (pas de WARN SessionEnd)' $g1
$m = [int]((Get-Date).Date.AddDays(1).AddHours(1) - (Get-Date)).TotalMinutes
Check ($g1 -match ('reste (\d+) min') -and [int]$Matches[1] -gt 0) 'session_gate: temps restant positif pour une fin du lendemain' $g1
[IO.File]::WriteAllText($EndFile, $Now.AddDays(-1).ToString('yyyy-MM-dd') + " 14:15`r`n")
$g2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Root -SessionEndFile $EndFile | Out-String
Check ($g2 -match "(?m)^WARN SessionEnd") 'session_gate: fin d''hier = WARN (fichier perime)' $g2
[IO.File]::WriteAllText($EndFile, $Now.AddDays(3).ToString('yyyy-MM-dd') + " 14:15`r`n")
$g3 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Root -SessionEndFile $EndFile | Out-String
Check ($g3 -match "(?m)^WARN SessionEnd") 'session_gate: fin dans 3 jours = WARN (plus de 24 h)' $g3

# --- Heure illisible : code non nul, fichier inchange
$before = (Get-Content $EndFile -Raw)
$o5 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool 'demain' -Root $Tmp | Out-String
Check ($LASTEXITCODE -ne 0 -and (Get-Content $EndFile -Raw) -eq $before) 'heure illisible: code non nul, SessionEnd.txt inchange' $o5

Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output ("extend_session_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $(if ($Fails -eq 0) { 0 } else { 1 })
