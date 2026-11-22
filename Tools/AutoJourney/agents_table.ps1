# agents_table.ps1 (L15-4, 2026-11-22) : ajoute / met a jour UNE ligne du tableau "## Agents du <date du jour>" d'Auto_Journey_Jour.md
# sans Edit ni reecriture de lignes longues (L15-10 : 3 lignes en double et des id "-" jamais remplis le 2026-11-15).
# Usage (8 colonnes : Agent, id, Worktree, Tache, C++, Lance, Fini, Resume) :
#   ajouter : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\agents_table.ps1 -Add <agent> -Id <id> -Worktree <worktree> -Task "<tache>" -Cpp oui|non [-Jour <fichier>]
#   mettre a jour : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\agents_table.ps1 -Update <agent> -Summary "<texte>" [-Jour <fichier>]
# Regles :
#   - L'HEURE vient toujours de Get-Date (HH:mm) : Lance a l'ajout (Fini = "en cours", resume vide), Fini a la mise a jour ; aucun parametre d'heure.
#   - -Jour : fichier a editer, defaut = copie principale C:\Users\baudo\Documents\MyCrusader\Auto_Journey_Jour.md (le test passe toujours -Jour).
#   - La section est "## Agents du <yyyy-MM-dd du jour>" (jusqu'au "## " suivant) ; un agent d'un autre jour n'y compte pas.
#   - -Add refuse (code 1) si l'agent existe deja dans la section du jour (utiliser -Update) ; -Update refuse (code 1) si l'agent est absent
#     ou present sur plusieurs lignes ; section ou tableau introuvable = refus ; un texte avec fin de ligne = refus ; fichier inchange a l'octet.
#   - Les | du texte sont echappes en \| ; le reste du fichier est conserve a l'octet pres (CRLF, UTF-8 sans BOM, BOM deja present garde,
#     derniere ligne sans fin de ligne : rien de colle). La ligne neuve prend la fin de ligne de la ligne qui la precede.
# Sortie : "agents_table : <fichier> ligne N" puis la ligne RELUE dans le fichier apres l'ecriture.
# Codes : 0 ecrit et relu ; 1 refus (fichier intact) ; 2 erreur inattendue ; 3 relecture differente (original restaure).
# Test : Tools\AutoJourney\agents_table_test.ps1 (copie temporaire synthetique, jamais le vrai Jour).
[CmdletBinding()]
param(
    [string] $Add,
    [string] $Id,
    [string] $Worktree,
    [string] $Task,
    [ValidateSet('oui', 'non')] [string] $Cpp,
    [string] $Update,
    [AllowEmptyString()] [string] $Summary,
    [string] $Jour = 'C:\Users\baudo\Documents\MyCrusader\Auto_Journey_Jour.md'
)
$ErrorActionPreference = 'Stop'
trap { try { Write-Output ("agents_table : ECHEC (erreur inattendue : " + $_.Exception.Message + ")") } catch { }; exit 2 }
function Refuse([string]$Why) { Write-Output "agents_table : REFUS ($Why) ; fichier inchange : $Jour"; exit 1 }
$enc = New-Object System.Text.UTF8Encoding $false
$PipeRe = '(?<!\\)\|'

$IsAdd = $PSBoundParameters.ContainsKey('Add')
$IsUpdate = $PSBoundParameters.ContainsKey('Update')
if ($IsAdd -eq $IsUpdate) { Refuse 'exactement un de -Add ou -Update' }
$Agent = if ($IsAdd) { $Add } else { $Update }
if ([string]::IsNullOrWhiteSpace($Agent)) { Refuse 'nom d''agent vide' }
$Agent = $Agent.Trim()
if ($Agent -match '[|\r\n]') { Refuse 'nom d''agent avec | ou fin de ligne' }

function Cell([string]$Text, [string]$Name) {
    if ($Text -match '[\r\n]') { Refuse "$Name contient une fin de ligne (une ligne a la fois)" }
    return ([regex]::Replace($Text.Trim(), $PipeRe, '\|'))
}
$Now = (Get-Date).ToString('HH:mm')
if ($IsAdd) {
    foreach ($p in @(@('Id', $Id), @('Worktree', $Worktree), @('Task', $Task), @('Cpp', $Cpp))) {
        if ([string]::IsNullOrWhiteSpace($p[1])) { Refuse ("-" + $p[0] + " obligatoire avec -Add") }
    }
    $NewRow = '| ' + $Agent + ' | ' + (Cell $Id 'Id') + ' | ' + (Cell $Worktree 'Worktree') + ' | ' + (Cell $Task 'Task') + ' | ' + $Cpp + ' | ' + $Now + ' | en cours | |'
} else {
    if (-not $PSBoundParameters.ContainsKey('Summary')) { Refuse '-Summary obligatoire avec -Update' }
    $SummaryCell = Cell $Summary 'Summary'
}

if (-not (Test-Path -LiteralPath $Jour -PathType Leaf)) { Refuse 'fichier absent' }
$Jour = (Resolve-Path -LiteralPath $Jour).ProviderPath
$origBytes = [IO.File]::ReadAllBytes($Jour)
$text = $enc.GetString($origBytes)

# lignes + fins de ligne separees (lines[i] suivie de seps[i] ; la derniere ligne peut ne pas avoir de fin)
$parts = [regex]::Split($text, '(\r\n|\n|\r)')
$lines = New-Object 'System.Collections.Generic.List[string]'
$seps = New-Object 'System.Collections.Generic.List[string]'
for ($i = 0; $i -lt $parts.Count; $i++) { if ($i % 2 -eq 0) { $lines.Add($parts[$i]) } else { $seps.Add($parts[$i]) } }

$today = Get-Date -Format 'yyyy-MM-dd'
$secHeader = "## Agents du $today"
$start = -1
for ($i = 0; $i -lt $lines.Count; $i++) { if ($lines[$i].TrimEnd() -eq $secHeader) { $start = $i; break } }
if ($start -lt 0) { Refuse "section '$secHeader' introuvable" }
$end = $lines.Count
for ($i = $start + 1; $i -lt $lines.Count; $i++) { if ($lines[$i].StartsWith('## ')) { $end = $i; break } }

$lastRow = -1
$hits = New-Object 'System.Collections.Generic.List[int]'
for ($i = $start + 1; $i -lt $end; $i++) {
    if (-not $lines[$i].StartsWith('|')) { continue }
    $lastRow = $i
    $tok = [regex]::Split($lines[$i], $PipeRe)
    if ($tok.Count -ge 2 -and $tok[1].Trim() -eq $Agent) { $hits.Add($i) }
}
if ($lastRow -lt 0) { Refuse "aucun tableau dans la section '$secHeader'" }

if ($IsAdd) {
    if ($hits.Count -gt 0) { Refuse ("$Agent existe deja dans la section du jour (ligne " + ($hits[0] + 1) + ") : utiliser -Update") }
    $eol = if ($lastRow -lt $seps.Count) { $seps[$lastRow] } elseif ($lastRow -gt 0) { $seps[$lastRow - 1] } else { "`r`n" }
    if ($lastRow -lt $seps.Count) { $seps.Insert($lastRow, $eol) } else { $seps.Add($eol) }
    $lines.Insert($lastRow + 1, $NewRow)
    $target = $lastRow + 1
    $expected = $NewRow
} else {
    if ($hits.Count -eq 0) { Refuse "$Agent absent de la section du jour (utiliser -Add)" }
    if ($hits.Count -gt 1) { Refuse ("$Agent present sur " + $hits.Count + " lignes (" + (($hits | ForEach-Object { $_ + 1 }) -join ', ') + ") : ambigu") }
    $target = $hits[0]
    $tok = [regex]::Split($lines[$target], $PipeRe)
    if ($tok.Count -lt 10) { Refuse ("ligne " + ($target + 1) + " : moins de 8 colonnes") }
    $tok[7] = ' ' + $Now + ' '
    $tok[8] = ' ' + $SummaryCell + ' '
    $lines[$target] = ($tok -join '|')
    $expected = $lines[$target]
}

$sb = New-Object System.Text.StringBuilder
for ($i = 0; $i -lt $lines.Count; $i++) { [void]$sb.Append($lines[$i]); if ($i -lt $seps.Count) { [void]$sb.Append($seps[$i]) } }
[IO.File]::WriteAllBytes($Jour, $enc.GetBytes($sb.ToString()))

# relecture : la ligne doit etre dans le fichier, sinon l'original est restaure
$back = [regex]::Split($enc.GetString([IO.File]::ReadAllBytes($Jour)), '\r\n|\n|\r')
if ($back.Count -le $target -or $back[$target] -ne $expected) {
    [IO.File]::WriteAllBytes($Jour, $origBytes)
    Write-Output "agents_table : relecture differente, original restaure : $Jour"
    exit 3
}
Write-Output ("agents_table : $Jour ligne " + ($target + 1))
Write-Output $back[$target]
exit 0
