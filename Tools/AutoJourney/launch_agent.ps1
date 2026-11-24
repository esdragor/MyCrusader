# launch_agent.ps1 (L27-10, SAURON-1124si) : amorce d'un agent Auto_Journey en UN appel.
# 1) session_gate -Phase Launch (NoCpp sauf -Cpp) ; FAIL = code 1, rien n'est ecrit ni imprime
# 2) heure d'arret = maintenant + 15 min (Get-Date PowerShell) ; 3) ligne du tableau de la section "### Seance du soir" (ou "## Agents du <date>")
#    de Auto_Journey_Jour.md ; 4) imprime l'enonce de 8 lignes du modele d'Auto_Journey.md.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier> -Agent <ID> -Root <worktree> -Task <texte> [-Cpp]
# Options de test : -JourFile <chemin> (defaut <copie principale>\Auto_Journey_Jour.md), -Now 'HH:mm' (heure du lancement), -NoGate (saute session_gate), -Minutes N.
param(
  [Parameter(Mandatory = $true)][string]$Agent,
  [Parameter(Mandatory = $true)][string]$Root,
  [Parameter(Mandatory = $true)][string]$Task,
  [switch]$Cpp,
  [string]$MainRoot = 'C:\Users\baudo\Documents\MyCrusader',
  [string]$JourFile = '',
  [string]$Now = '',
  [int]$Minutes = 15,
  [switch]$NoGate
)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $JourFile) { $JourFile = Join-Path $MainRoot 'Auto_Journey_Jour.md' }
if (-not (Test-Path $Root)) { Write-Output "ERREUR : worktree introuvable : $Root"; exit 2 }

# 1) gate de lancement
if (-not $NoGate) {
  $gateArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $Here 'session_gate.ps1'), '-Phase', 'Launch', '-Root', $Root)
  if (-not $Cpp) { $gateArgs += '-NoCpp' }
  $gateOut = & powershell.exe @gateArgs 2>&1 | Out-String
  $gateCode = $LASTEXITCODE
  Write-Output $gateOut.TrimEnd()
  if ($gateCode -ne 0) { Write-Output "ARRET : session_gate -Phase Launch en echec (code $gateCode), agent non lance"; exit 1 }
}

# 2) heure d'arret
$start = if ($Now) { [datetime]::ParseExact($Now, 'HH:mm', $null) } else { Get-Date }
$stop = $start.AddMinutes($Minutes).ToString('HH:mm')
$date = (Get-Date).ToString('yyyy-MM-dd')
$short = (Split-Path $Root -Leaf) -replace '^HeroesOfLiberty-', ''
$branch = (& git.exe -C $Root branch --show-current 2>$null | Out-String).Trim()
if (-not $branch) { $branch = '?' }
$cppText = if ($Cpp) { 'C++ : raison dite dans la tache' } else { 'sans C++' }

# 3) ligne du tableau
$bytes = [IO.File]::ReadAllBytes($JourFile)
$bom = ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF)
$text = [IO.File]::ReadAllText($JourFile)
$eol = if ($text.Contains("`r`n")) { "`r`n" } else { "`n" }
$lines = New-Object System.Collections.Generic.List[string]
foreach ($l in ($text -split "`r?`n")) { $lines.Add($l) }
$row = '| {0} | HeroesOfLiberty-{1} | {2} | - | {3} | {4} |' -f $Agent, $short, $branch, $Task.Replace('|', '/'), $cppText
$start0 = -1
for ($i = 0; $i -lt $lines.Count; $i++) { if ($lines[$i] -match '^### S.ance du soir') { $start0 = $i; break } }
if ($start0 -lt 0) { for ($i = 0; $i -lt $lines.Count; $i++) { if ($lines[$i] -match ('^## Agents du ' + $date)) { $start0 = $i; break } } }
if ($start0 -lt 0) { Write-Output "ERREUR : ni '### Seance du soir' ni '## Agents du $date' dans $JourFile"; exit 2 }
$end0 = $lines.Count
for ($i = $start0 + 1; $i -lt $lines.Count; $i++) { if ($lines[$i] -match '^##+ ' -and $lines[$i] -notmatch '^### S.ance du soir') { if ($lines[$i] -match '^## ' ) { $end0 = $i; break } } }
$lastRow = -1
for ($i = $start0 + 1; $i -lt $end0; $i++) { if ($lines[$i] -match '^\|') { $lastRow = $i } }
if ($lastRow -ge 0) { $lines.Insert($lastRow + 1, $row) }
else {
  $ins = $end0; while ($ins -gt $start0 + 1 -and -not $lines[$ins - 1].Trim()) { $ins-- }
  $lines.Insert($ins, $row)
  $lines.Insert($ins, '| Agent | Worktree | Branche | Commits | Tache | C++ |')
  $lines.Insert($ins, '')
}
[IO.File]::WriteAllText($JourFile, ($lines -join $eol), (New-Object Text.UTF8Encoding $bom))
Write-Output "ligne ecrite dans $JourFile : $row"

# 4) enonce
$ar = 'ARR' + [char]0xCA + 'T DUR'
$tc = 'T' + [char]0xC2 + 'CHE'
$sc = if ($Cpp) { 'C++ : raison dite dans la tache' } else { 'sans C++' }
Write-Output ''
Write-Output ('Tu es {0} ({1}). Lis {2}\Tools\AutoJourney\brief_commun.md puis agent_rules.md et applique-les.' -f $Agent, $Task, $MainRoot)
$a2 = [string][char]0xE0
Write-Output ("$ar $a2 $stop (Get-Date PowerShell lu juste avant ; $Minutes min ou 25 appels vus par toi, le premier atteint).")
Write-Output ('Worktree {0} (branche {1}) ; {2}.' -f $Root, $branch, $sc)
Write-Output ('Lis la fin de ton *InProgress.md dans {0} et ta ligne de {1}\Auto_Journey_File.md.' -f $Root, $MainRoot)
Write-Output ('{0} : {1} ; premier Edit avant le 6e appel ; rouge VU puis correctif en deux commits.' -f $tc, $Task)
Write-Output ('Fichiers d{0}j{1} pris par un autre agent de la vague : voir Auto_Journey_Jour.md (L21-2).' -f [char]0xE9, [char]0xE0)
Write-Output ('Commits en anglais, derniere ligne "Agent: {0}", jamais de push ni de fusion dans main.' -f $Agent)
Write-Output ('Rapport : {0}\Docs\Rapports\{1}.md (13 champs d''agent_rules.md), reponse finale de 6 lignes.' -f $MainRoot, $Agent)
exit 0
