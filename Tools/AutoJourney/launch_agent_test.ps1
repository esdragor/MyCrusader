# launch_agent_test.ps1 : test minimal de launch_agent.ps1 (L27-10). Aucun C++, aucun editeur, -NoGate (session_gate a son propre test).
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
$d = Get-Date -Format 'yyyy-MM-dd'
$seed = "## Agents du $d`r`n`r`n### S$([char]0xE9)ance du soir (19:00 -> 23:00)`r`ntexte`r`n| Agent | Worktree | Branche | Commits | T$([char]0xE2)che | C++ |`r`n| OLD-1 | HeroesOfLiberty-Old | b | - | x | sans C++ |`r`n`r`n## Autre section`r`n| NOT-MINE | w | b | - | y | z |`r`n"
[IO.File]::WriteAllText($Jour, $seed)
$out = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Agent 'TEST-1124x' -Root $Here -Task 'tache de test' -JourFile $Jour -Now '21:30' -NoGate | Out-String
$code = $LASTEXITCODE
Check ($code -eq 0) 'code 0' "code=$code out=$out"
Check ($out -match 'ARR.T DUR . 21:45') 'enonce : ARRET DUR a +15 min (21:45)' $out
Check ($out -match 'brief_commun\.md' -and $out -match 'agent_rules\.md') 'enonce : cite brief_commun.md et agent_rules.md' $out
Check ($out -match 'Tu es TEST-1124x' -and $out -match 'tache de test' -and $out -match 'sans C\+\+') 'enonce : agent, tache, sans C++' $out
$n = @($out -split "`r?`n" | Where-Object { $_ -match '^(Tu es|ARR|Worktree|Lis la fin|T.CHE|Fichiers|Commits|Rapport)' }).Count
Check ($n -eq 8) 'enonce : 8 lignes' "n=$n"
$j = [IO.File]::ReadAllText($Jour)
$i1 = $j.IndexOf('| OLD-1 |'); $i2 = $j.IndexOf('| TEST-1124x |'); $i3 = $j.IndexOf('## Autre section')
Check ($i2 -gt $i1 -and $i2 -lt $i3) 'tableau : la ligne est apres OLD-1 et avant la section suivante' $j
Check ($j.Contains('| NOT-MINE |') -and $j -match "\r\n") 'tableau : le reste du fichier et les CRLF sont gardes'
$out2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Agent 'TEST-2' -Root $Here -Task 't2' -Cpp -JourFile $Jour -Now '23:55' -NoGate | Out-String
Check ($out2 -match 'ARR.T DUR . 00:10' -and $out2 -match 'C\+\+ : raison') '-Cpp : mention C++ et arret apres minuit' $out2
Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output ("launch_agent_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $(if ($Fails -eq 0) { 0 } else { 1 })
