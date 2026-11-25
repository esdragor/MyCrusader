# launch_agent.ps1 (L27-10, SAURON-1124si; TOOLS-1125a : un seul retour d'une ligne, duree 30 min) : amorce d'un agent Auto_Journey en UN appel.
# 1) session_gate -Phase Launch (NoCpp sauf -Cpp) ; FAIL = code 1, rien n'est ecrit
# 2) heure d'arret = maintenant + 30 min (Get-Date PowerShell) ; 3) ligne du tableau de la section "### Seance du soir" (ou "## Agents du <date>")
#    de Auto_Journey_Jour.md ; 4) ECRIT l'enonce de 8 lignes (+ -Tache) dans <copie principale>\Docs\Rapports\ENONCE-<AGENT>.txt au lieu de l'imprimer.
# Sortie : UNE ligne. `PASS <AGENT> arret HH:mm enonce <chemin>` (code 0) ou `FAIL <premiere ligne FAIL du gate / de l'erreur>` (code 1 gate, 2 erreur).
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier> -Agent <ID> -Root <worktree> -Task <texte> [-Tache <texte multi-ligne>] [-Cpp]
# Options de test : -JourFile <chemin> (defaut <copie principale>\Auto_Journey_Jour.md), -EnonceDir <dossier> (defaut <copie principale>\Docs\Rapports),
#                   -Now 'HH:mm' (heure du lancement), -NoGate (saute session_gate), -GateScript <ps1> (remplace session_gate), -Minutes N (defaut 30).
param(
  [Parameter(Mandatory = $true)][string]$Agent,
  [Parameter(Mandatory = $true)][string]$Root,
  [Parameter(Mandatory = $true)][string]$Task,
  [string]$Tache = '',
  [switch]$Cpp,
  [string]$MainRoot = 'C:\Users\baudo\Documents\MyCrusader',
  [string]$JourFile = '',
  [string]$EnonceDir = '',
  [string]$GateScript = '',
  [string]$Now = '',
  [int]$Minutes = 30,
  [switch]$NoGate
)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $JourFile) { $JourFile = Join-Path $MainRoot 'Auto_Journey_Jour.md' }
if (-not $EnonceDir) { $EnonceDir = Join-Path $MainRoot 'Docs\Rapports' }
if (-not $GateScript) { $GateScript = Join-Path $Here 'session_gate.ps1' }
function Stop-Launch([string]$Message, [int]$Code) { Write-Output ('FAIL ' + $Message); exit $Code }
if (-not (Test-Path $Root)) { Stop-Launch "ERREUR : worktree introuvable : $Root" 2 }

# 1) gate de lancement
if (-not $NoGate) {
  $gateArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $GateScript, '-Phase', 'Launch', '-Root', $Root)
  if (-not $Cpp) { $gateArgs += '-NoCpp' }
  $gateOut = & powershell.exe @gateArgs 2>&1 | Out-String
  $gateCode = $LASTEXITCODE
  if ($gateCode -ne 0) {
    $firstFail = @($gateOut -split "`r?`n" | Where-Object { $_ -match '^\s*FAIL' } | Select-Object -First 1)
    $msg = if ($firstFail.Count) { ([string]$firstFail[0]).Trim() -replace '^FAIL\s*:?\s*', '' } else { "session_gate -Phase Launch en echec (code $gateCode)" }
    Stop-Launch $msg 1
  }
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
if ($start0 -lt 0) { Stop-Launch "ERREUR : ni '### Seance du soir' ni '## Agents du $date' dans $JourFile" 2 }
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

# 4) enonce, ecrit dans <EnonceDir>\ENONCE-<AGENT>.txt (UTF-8 sans BOM, chemin absolu)
$ar = 'ARR' + [char]0xCA + 'T DUR'
$tc = 'T' + [char]0xC2 + 'CHE'
$sc = if ($Cpp) { 'C++ : raison dite dans la tache' } else { 'sans C++' }
$a2 = [string][char]0xE0
$e = New-Object System.Collections.Generic.List[string]
$e.Add(('Tu es {0} ({1}). Lis {2}\Tools\AutoJourney\brief_commun.md puis agent_rules.md et applique-les.' -f $Agent, $Task, $MainRoot))
$e.Add("$ar $a2 $stop (Get-Date PowerShell lu juste avant ; $Minutes min ou 45 appels, deux t" + [char]0xE2 + "ches de ta file $a2 encha" + [char]0xEE + "ner ; le premier seuil atteint).")
$e.Add(('Worktree {0} (branche {1}) ; {2}.' -f $Root, $branch, $sc))
$e.Add(('Lis la fin de ton *InProgress.md dans {0} et ta ligne de {1}\Auto_Journey_File.md.' -f $Root, $MainRoot))
$e.Add(('{0} : {1} ; premier Edit avant le 6e appel ; rouge VU puis correctif en deux commits.' -f $tc, $Task))
$e.Add(('Fichiers d{0}j{1} pris par un autre agent de la vague : voir Auto_Journey_Jour.md (L21-2).' -f [char]0xE9, [char]0xE0))
$e.Add(('Commits en anglais, derniere ligne "Agent: {0}", jamais de push ni de fusion dans main.' -f $Agent))
$e.Add(('Rapport : {0}\Docs\Rapports\{1}.md (13 champs d''agent_rules.md), reponse finale de 6 lignes.' -f $MainRoot, $Agent))
if ($Tache.Trim()) { foreach ($t in ($Tache.TrimEnd() -split "`r?`n")) { $e.Add($t) } }
try {
  if (-not (Test-Path -LiteralPath $EnonceDir)) { New-Item -ItemType Directory -Path $EnonceDir -Force | Out-Null }
  $enonce = Join-Path (Resolve-Path -LiteralPath $EnonceDir).Path ('ENONCE-' + $Agent + '.txt')
  [IO.File]::WriteAllText($enonce, (($e -join "`r`n") + "`r`n"), (New-Object Text.UTF8Encoding $false))
}
catch { Stop-Launch "ERREUR : enonce non ecrit : $($_.Exception.Message)" 2 }
Write-Output ('PASS {0} arret {1} enonce {2}' -f $Agent, $stop, $enonce)
exit 0
