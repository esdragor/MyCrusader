# UserPromptSubmit hook (2026-11-20, lesson L19-1): when the user's prompt starts an Auto_Journey session,
# inject the conductor rule and the checklist reading into Claude's context. Prints nothing otherwise.
$ErrorActionPreference = 'SilentlyContinue'
$raw = [Console]::In.ReadToEnd()
try { $in = $raw | ConvertFrom-Json } catch { exit 0 }
$prompt = [string]$in.prompt
# Only the launch phrases ("commence ton Auto_Journey", "commence un auto journee", "continue / reprends ... auto journ..."),
# not any message that merely names Auto_Journey_File.md (agent reports arrive as prompts too).
if ($prompt -notmatch '(?i)\b(commence|continue|reprends|lance|relance)\b[^\r\n]{0,40}auto[ _\-]?journ') { exit 0 }

$checklist = 'C:\Users\baudo\Documents\MyCrusader\Auto_Journey_Checklist.md'
$last = 'L?'
if (Test-Path $checklist) {
    $nums = [regex]::Matches((Get-Content $checklist -Raw), '\bL(\d+)-') | ForEach-Object { [int]$_.Groups[1].Value }
    if ($nums) { $last = 'L' + ($nums | Measure-Object -Maximum).Maximum }
}

$text = @"
RAPPEL AUTO_JOURNEY (hook du projet, lecon L19-1 du 2026-11-20) :
1. Lire $checklist EN ENTIER apres le nettoyage 0a et AVANT le premier lancement d'agent de travail ; ecrire dans Auto_Journey_Jour.md la ligne 'check-list lue ($last)'.
2. La session principale est le CHEF D'ORCHESTRE : elle lance les agents, lit leurs rapports de 10 lignes, tient le tableau des agents, lance build.ps1 / quick_check en UNE commande d'arriere-plan et avance main.
3. Tout le reste va a un SOUS-AGENT : correction d'une erreur de compilation vue a l'integration, diagnostic d'un rouge, retouche d'un outil ou d'un scenario, pre-vols et lecture des PNG, fiches E et Q, en-tete des Retours, enregistrement des reponses.
4. Chemins ABSOLUS pour toute ecriture [IO.File] (le repertoire du processus est la copie principale).
"@

$out = @{ hookSpecificOutput = @{ hookEventName = 'UserPromptSubmit'; additionalContext = $text } }
$out | ConvertTo-Json -Depth 4 -Compress
exit 0
