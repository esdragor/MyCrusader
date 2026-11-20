# edit_line.ps1 (L14-1, 2026-11-14 ; -Append L15-2, 2026-11-15) : edite UNE ligne d'un fichier de suivi (EtapeDefini.md, Auto_Journey_File.md,
# Retours\E-*.md, *InProgress.md) sans Read prealable ni Edit refuse ("fichier non lu", "fins de ligne melangees"), ou en AJOUTE une.
# Lecon du 2026-11-14 : un Edit multi-lignes a perdu des CRLF et colle des lignes dans Auto_Journey_File.md.
# Lecon L15-2 (2026-11-15) : -Match "D799" a remplace la ligne ENTIERE d'un autre agent (c'etait la seule qui contenait le texte) : pour
#   ajouter sa ligne, -Append ; -Match ne doit apparaitre que sur TA ligne.
# Usage :
#   remplacer : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\edit_line.ps1 -File <chemin absolu> -Match "<texte>" -NewLine "<ligne neuve>" [-Regex] [-InsertAfter] [-DryRun]
#   ajouter   : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\edit_line.ps1 -File <chemin absolu> -Append "<ligne neuve>" [-After "<texte>"] [-Regex] [-DryRun]
#   -Match : texte LITTERAL (crochets, parentheses, accents sans echappement) contenu dans la ligne ; -Regex : -Match / -After sont des regex .NET.
#   Remplace la ligne trouvee par -NewLine ; -InsertAfter : la garde et insere -NewLine juste apres.
#   -Append : ajoute la ligne sans en ecraser aucune, juste apres la DERNIERE ligne qui contient -After (aucune : refus), ou en fin de fichier sans -After.
# Regles : avec -Match, EXACTEMENT une ligne doit correspondre (0 ou plusieurs : refus qui propose -Append, numeros cites, fichier inchange
#   octet pour octet) ; le texte neuf ne contient aucune fin de ligne (une ligne a la fois) ; chaque ligne garde sa propre fin (CRLF, LF,
#   aucune pour la derniere) ; une ligne inseree prend la fin de la ligne qui la precede (sinon CRLF si le fichier en a, et la derniere ligne
#   sans fin en recoit une : rien de colle) ; le fichier est relu juste avant l'ecriture (pas de copie perimee), en UTF-8, sans BOM ajoute
#   (un BOM deja present est garde : aucun diff parasite).
# Sortie : "edit_line : <fichier> ligne N" + avant / apres / relu (la ligne RELUE dans le fichier apres l'ecriture) ; code 0 = ecrit et
#   relu ; 1 refus (fichier absent, 0 ou plusieurs lignes, fin de ligne dans le texte neuf, -After introuvable, fichier change) ;
#   2 "edit_line : ECHEC (...)" (ecriture impossible, erreur inattendue) ; 3 relecture differente (original restaure). Jamais 255.
#   AX-3 (2026-11-20) : l'ecriture precede l'impression et une sortie coupee ("| Select-Object -First 1") ne perd plus l'edition ;
#   le code de sortie ne depend que du resultat de l'ecriture. Test : Tools\Tests\edit_line_test.ps1.
param(
    [Parameter(Mandatory = $true)] [string] $File,
    [string] $Match = '',
    [AllowEmptyString()] [string] $NewLine,
    [AllowEmptyString()] [string] $Append,
    [string] $After = '',
    [switch] $Regex,
    [switch] $InsertAfter,
    [switch] $DryRun
)
$ErrorActionPreference = 'Stop'
# Erreur inattendue (avant l'ecriture) : message clair et code 2, jamais un code muet.
trap { try { Write-Output ("edit_line : ECHEC (erreur inattendue : " + $_.Exception.Message + ")") } catch { }; exit 2 }
function Refuse([string]$Why) { Write-Output "edit_line : REFUS ($Why) ; fichier inchange : $File"; exit 1 }
$Hint = ' ; pour AJOUTER une ligne sans en ecraser une : -Append "<ligne>" [-After "<texte de la ligne qui precede>"]'
$IsAppend = $PSBoundParameters.ContainsKey('Append')

if (-not (Test-Path -LiteralPath $File -PathType Leaf)) { Refuse 'fichier absent' }
if ($IsAppend) {
    if ($Match -or $PSBoundParameters.ContainsKey('NewLine') -or $InsertAfter) { Refuse '-Append ne se combine pas avec -Match, -NewLine ou -InsertAfter' }
    $NewText = $Append
} else {
    if (-not $Match) { Refuse ('-Match vide' + $Hint) }
    if (-not $PSBoundParameters.ContainsKey('NewLine')) { Refuse '-NewLine manquant' }
    $NewText = $NewLine
}
if ($NewText -match "[`r`n]") { Refuse 'le texte neuf contient une fin de ligne : une ligne a la fois' }

$Path = (Resolve-Path -LiteralPath $File).Path
$Bytes = [IO.File]::ReadAllBytes($Path)
$HasBom = ($Bytes.Length -ge 3 -and $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF)
$Utf8 = New-Object System.Text.UTF8Encoding($false)
$Text = if ($HasBom) { $Utf8.GetString($Bytes, 3, $Bytes.Length - 3) } else { $Utf8.GetString($Bytes) }

# Lignes avec leur propre fin : (texte, fin) ; un fichier vide n'a aucune ligne.
$Lines = New-Object System.Collections.Generic.List[object]
foreach ($m in [regex]::Matches($Text, "([^`r`n]*)(`r`n|`n|`r|$)")) {
    if ($m.Length -eq 0) { continue }
    $Lines.Add([pscustomobject]@{ Body = $m.Groups[1].Value; Eol = $m.Groups[2].Value })
}
function Find-Lines([string]$Needle) {
    @(for ($k = 0; $k -lt $Lines.Count; $k++) {
        $b = $Lines[$k].Body
        if ($(if ($Regex) { [regex]::IsMatch($b, $Needle) } else { $b.Contains($Needle) })) { $k }
    })
}
$DefaultEol = if ($Text.Contains("`r`n")) { "`r`n" } elseif ($Text.Contains("`n")) { "`n" } else { "`r`n" }
function Insert-After([int]$At, [string]$Body) {
    if ($At -lt 0) { $Lines.Insert(0, [pscustomobject]@{ Body = $Body; Eol = $(if ($Lines.Count) { $DefaultEol } else { '' }) }); return 0 }
    $Eol = $Lines[$At].Eol
    if (-not $Eol) { $Lines[$At].Eol = $DefaultEol; $Lines.Insert($At + 1, [pscustomobject]@{ Body = $Body; Eol = '' }) }
    else { $Lines.Insert($At + 1, [pscustomobject]@{ Body = $Body; Eol = $Eol }) }
    return ($At + 1)
}

if ($IsAppend) {
    if ($After) {
        $AHits = Find-Lines $After
        if ($AHits.Count -eq 0) { Refuse "aucune ligne ne contient -After '$After'" }
        $At = $AHits[$AHits.Count - 1]
    } else { $At = $Lines.Count - 1 }
    $i = Insert-After $At $NewText
    $Old = ''
    $Mode = ' (ajout)'
} else {
    $Hits = Find-Lines $Match
    if ($Hits.Count -eq 0) { Refuse ("aucune ligne ne contient '$Match'" + $Hint) }
    if ($Hits.Count -gt 1) { Refuse (("{0} lignes correspondent (lignes {1}) : preciser -Match" -f $Hits.Count, (($Hits | ForEach-Object { $_ + 1 }) -join ', ')) + $Hint) }
    $i = $Hits[0]
    $Old = $Lines[$i].Body
    $Mode = ''
    if ($InsertAfter) { $i = Insert-After $i $NewText; $Mode = ' (insertion apres)' }
    else { $Lines[$i].Body = $NewText }
}
$Out = New-Object System.Text.StringBuilder
foreach ($l in $Lines) { [void]$Out.Append($l.Body); [void]$Out.Append($l.Eol) }
$NewBytes = $Utf8.GetBytes($Out.ToString())
if ($HasBom) { $NewBytes = [byte[]](0xEF, 0xBB, 0xBF) + $NewBytes }

# AX-3 (2026-11-20) : toute impression passe par Say (jamais fatale : une sortie coupee, p. ex. "| Select-Object -First 1", ne doit ni
#   perdre l'edition ni changer le code de sortie) ; l'ecriture vient AVANT toute impression de resultat ; la ligne ecrite est RELUE
#   dans le fichier et affichee ("relu : ...") ; un echec d'ecriture ou de relecture = "edit_line : ECHEC (...)" et code 2 (fichier
#   restaure octet pour octet), jamais un code muet 255 ; succes = code 0.
function Say([string]$Msg) { try { Write-Output $Msg } catch { } }
function Fail([string]$Why, [int]$Code = 2) { Say "edit_line : ECHEC ($Why) ; fichier : $Path"; exit $Code }
if ($DryRun) {
    Say ("edit_line : {0} ligne {1}{2}" -f $Path, ($i + 1), $Mode)
    if (-not $IsAppend) { Say ("  avant : " + $Old) }
    Say ("  apres : " + $NewText)
    Say '  DryRun : rien ecrit'
    exit 0
}
# Relu juste avant d'ecrire : une autre session a pu ecrire entre-temps (plusieurs sessions ecrivent EtapeDefini.md en parallele).
try {
    $Again = [IO.File]::ReadAllBytes($Path)
    if ($Again.Length -ne $Bytes.Length -or [Convert]::ToBase64String($Again) -ne [Convert]::ToBase64String($Bytes)) { Refuse 'le fichier a change pendant l edition : relancer' }
    [IO.File]::WriteAllBytes($Path, $NewBytes)
} catch {
    Fail ('ecriture impossible : ' + $_.Exception.Message)
}
# Relecture : les octets sont ceux attendus ET la ligne i du fichier relu est la ligne neuve ; sinon on restaure l'original.
$Check = $null
try { $Check = [IO.File]::ReadAllBytes($Path) } catch { }
$Good = ($Check -ne $null -and $Check.Length -eq $NewBytes.Length -and [Convert]::ToBase64String($Check) -eq [Convert]::ToBase64String($NewBytes))
$Read = ''
if ($Good) {
    $ct = if ($HasBom) { $Utf8.GetString($Check, 3, $Check.Length - 3) } else { $Utf8.GetString($Check) }
    $rl = @([regex]::Matches($ct, "([^`r`n]*)(`r`n|`n|`r|$)") | Where-Object { $_.Length -gt 0 } | ForEach-Object { $_.Groups[1].Value })
    if ($i -lt $rl.Count) { $Read = $rl[$i] }
    if ($Read -cne $NewText) { $Good = $false }
}
if (-not $Good) {
    $restored = $false
    try { [IO.File]::WriteAllBytes($Path, $Bytes); $restored = $true } catch { }
    Fail ('la relecture ne montre pas la ligne attendue ; original ' + $(if ($restored) { 'restaure' } else { 'NON restaure' })) 3
}
Say ("edit_line : {0} ligne {1}{2}" -f $Path, ($i + 1), $Mode)
if (-not $IsAppend) { Say ("  avant : " + $Old) }
Say ("  apres : " + $NewText)
Say ("  relu : " + $Read)
exit 0
