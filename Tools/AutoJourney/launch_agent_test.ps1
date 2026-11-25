# launch_agent_test.ps1 : test de launch_agent.ps1 (L27-10, TOOLS-1125a). Aucun C++, aucun editeur, -NoGate (session_gate a son propre test) sauf le cas du faux gate.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (PASS/FAIL par cas, code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'launch_agent.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('launchagent-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $Tmp -Force | Out-Null
$Jour = Join-Path $Tmp 'Auto_Journey_Jour.md'
$Ren = Join-Path $Tmp 'Rapports'
$d = Get-Date -Format 'yyyy-MM-dd'
$seed = "## Agents du $d`r`n`r`n### S$([char]0xE9)ance du soir (19:00 -> 23:00)`r`ntexte`r`n| Agent | Worktree | Branche | Commits | T$([char]0xE2)che | C++ |`r`n| OLD-1 | HeroesOfLiberty-Old | b | - | x | sans C++ |`r`n`r`n## Autre section`r`n| NOT-MINE | w | b | - | y | z |`r`n"
[IO.File]::WriteAllText($Jour, $seed)
$tache = "Ligne un de la tache`r`nLigne deux"
$outLines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Agent 'TEST-1124x' -Root $Here -Task 'tache de test' -Tache $tache -JourFile $Jour -EnonceDir $Ren -Now '21:30' -NoGate)
$code = $LASTEXITCODE
$out = $outLines -join "`n"
$file = Join-Path $Ren 'ENONCE-TEST-1124x.txt'
Check ($code -eq 0) 'code 0' "code=$code out=$out"
Check ($outLines.Count -eq 1) 'sortie : UNE seule ligne' "n=$($outLines.Count) out=$out"
Check ($out -eq "PASS TEST-1124x arret 22:00 enonce $file") 'sortie : PASS <AGENT> arret HH:mm enonce <chemin> (22:00 = +30 min par defaut)' $out
Check (Test-Path -LiteralPath $file) 'enonce : fichier ENONCE-<AGENT>.txt ecrit' $file
$e = if (Test-Path -LiteralPath $file) { [IO.File]::ReadAllText($file) } else { '' }
Check ($e -match 'ARR.T DUR . 22:00') 'enonce : ARRET DUR a +30 min (22:00)' $e
Check ($e -match '30 min ou 45 appels' -and $e -match 'deux t.ches de ta file . encha.ner') 'enonce : 30 min ou 45 appels, deux taches a enchainer' $e
Check ($e -match 'brief_commun\.md' -and $e -match 'agent_rules\.md') 'enonce : cite brief_commun.md et agent_rules.md' $e
Check ($e -match 'Tu es TEST-1124x' -and $e -match 'tache de test' -and $e -match 'sans C\+\+') 'enonce : agent, tache, sans C++' $e
$n = @($e -split "`r?`n" | Where-Object { $_ -match '^(Tu es|ARR|Worktree|Lis la fin|T.CHE|Fichiers|Commits|Rapport)' }).Count
Check ($n -eq 8) 'enonce : 8 lignes' "n=$n"
Check ($e -match "(?m)^Ligne un de la tache\r?\nLigne deux\r?\n") 'enonce : -Tache multi-ligne ajoute a la suite' $e
Check (-not $e.Contains([string][char]0xFEFF)) 'enonce : sans BOM'
$j = [IO.File]::ReadAllText($Jour)
$i1 = $j.IndexOf('| OLD-1 |'); $i2 = $j.IndexOf('| TEST-1124x |'); $i3 = $j.IndexOf('## Autre section')
Check ($i2 -gt $i1 -and $i2 -lt $i3) 'tableau : la ligne est apres OLD-1 et avant la section suivante' $j
Check ($j.Contains('| NOT-MINE |') -and $j -match "\r\n") 'tableau : le reste du fichier et les CRLF sont gardes'
$out2 = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Agent 'TEST-2' -Root $Here -Task 't2' -Cpp -JourFile $Jour -EnonceDir $Ren -Now '23:55' -NoGate) -join "`n"
$e2 = [IO.File]::ReadAllText((Join-Path $Ren 'ENONCE-TEST-2.txt'))
Check ($out2 -match '^PASS TEST-2 arret 00:25 enonce ' -and $e2 -match 'C\+\+ : raison') '-Cpp : mention C++ et arret apres minuit (00:25)' "$out2 / $e2"
Check ($e2 -notmatch 'Ligne un') 'sans -Tache : rien en plus'
$out3 = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Agent 'TEST-3' -Root $Here -Task 't3' -JourFile $Jour -EnonceDir $Ren -Now '10:00' -Minutes 15 -NoGate) -join "`n"
Check ($out3 -match '^PASS TEST-3 arret 10:15 ' -and ([IO.File]::ReadAllText((Join-Path $Ren 'ENONCE-TEST-3.txt')) -match '15 min ou 45 appels')) '-Minutes 15 reste possible' $out3
# gate en echec : UNE ligne FAIL <premiere ligne FAIL du gate>, rien d'ecrit, code 1
$Gate = Join-Path $Tmp 'fake_gate.ps1'
[IO.File]::WriteAllText($Gate, "Write-Output 'PASS ok'`r`nWrite-Output 'FAIL launch : IN_USE.txt de Elrond occupe'`r`nWrite-Output 'FAIL autre'`r`nexit 1`r`n")
$jourBefore = [IO.File]::ReadAllText($Jour)
$out4 = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Agent 'TEST-4' -Root $Here -Task 't4' -JourFile $Jour -EnonceDir $Ren -Now '10:00' -GateScript $Gate)
$code4 = $LASTEXITCODE
Check ($code4 -eq 1 -and $out4.Count -eq 1 -and $out4[0] -eq 'FAIL launch : IN_USE.txt de Elrond occupe') 'gate FAIL : code 1 et UNE ligne = premiere ligne FAIL du gate' "code=$code4 out=$($out4 -join '|')"
Check (-not (Test-Path (Join-Path $Ren 'ENONCE-TEST-4.txt')) -and [IO.File]::ReadAllText($Jour) -eq $jourBefore) 'gate FAIL : ni enonce ni ligne de tableau ecrits'
Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output ("launch_agent_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $(if ($Fails -eq 0) { 0 } else { 1 })
