# edit_line.ps1 (L14-1, 2026-11-14) : edite UNE ligne d'un fichier de suivi (EtapeDefini.md, Auto_Journey_File.md, Retours\E-*.md,
# *InProgress.md) sans Read prealable ni Edit refuse ("fichier non lu", "fins de ligne melangees").
# Lecon du 2026-11-14 : un Edit multi-lignes a perdu des CRLF et colle des lignes dans Auto_Journey_File.md.
# Usage :
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\edit_line.ps1 -File <chemin absolu> -Match "<texte>" -NewLine "<ligne neuve>" [-Regex] [-InsertAfter] [-DryRun]
#   -Match : texte LITTERAL (crochets, parentheses, accents sans echappement) contenu dans la ligne ; -Regex : -Match est une regex .NET.
#   Remplace la ligne trouvee par -NewLine ; -InsertAfter : la garde et insere -NewLine juste apres.
# Regles : EXACTEMENT une ligne doit correspondre (0 ou plusieurs : refus, numeros cites, fichier inchange octet pour octet) ;
#   -NewLine ne contient aucune fin de ligne (une ligne a la fois) ; chaque ligne garde sa propre fin (CRLF, LF, aucune pour la
#   derniere) ; une ligne inseree prend la fin de la ligne trouvee (sinon CRLF si le fichier en a) ; le fichier est relu juste
#   avant l'ecriture (pas de copie perimee), en UTF-8, sans BOM ajoute (un BOM deja present est garde : aucun diff parasite).
# Sortie : "edit_line : <fichier> ligne N" + avant / apres ; code 0 ; 1 refus (fichier absent, 0 ou plusieurs lignes, fin de ligne
#   dans -NewLine). Test : Tools\Tests\edit_line_test.ps1.
param(
    [Parameter(Mandatory = $true)] [string] $File,
    [Parameter(Mandatory = $true)] [string] $Match,
    [Parameter(Mandatory = $true)] [AllowEmptyString()] [string] $NewLine,
    [switch] $Regex,
    [switch] $InsertAfter,
    [switch] $DryRun
)
$ErrorActionPreference = 'Stop'
function Refuse([string]$Why) { Write-Output "edit_line : REFUS ($Why) ; fichier inchange : $File"; exit 1 }

if (-not (Test-Path -LiteralPath $File -PathType Leaf)) { Refuse 'fichier absent' }
if ($NewLine -match "[`r`n]") { Refuse 'le texte neuf contient une fin de ligne : une ligne a la fois' }
if (-not $Match) { Refuse '-Match vide' }

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
$Hits = @(for ($i = 0; $i -lt $Lines.Count; $i++) {
    $b = $Lines[$i].Body
    if ($(if ($Regex) { [regex]::IsMatch($b, $Match) } else { $b.Contains($Match) })) { $i }
})
if ($Hits.Count -eq 0) { Refuse "aucune ligne ne contient '$Match'" }
if ($Hits.Count -gt 1) { Refuse ("{0} lignes correspondent (lignes {1}) : preciser -Match" -f $Hits.Count, (($Hits | ForEach-Object { $_ + 1 }) -join ', ')) }

$i = $Hits[0]
$Old = $Lines[$i].Body
$DefaultEol = if ($Text.Contains("`r`n")) { "`r`n" } elseif ($Text.Contains("`n")) { "`n" } else { "`r`n" }
if ($InsertAfter) {
    $Eol = $Lines[$i].Eol
    if (-not $Eol) { $Lines[$i].Eol = $DefaultEol; $Lines.Insert($i + 1, [pscustomobject]@{ Body = $NewLine; Eol = '' }) }
    else { $Lines.Insert($i + 1, [pscustomobject]@{ Body = $NewLine; Eol = $Eol }) }
} else {
    $Lines[$i].Body = $NewLine
}
$Out = New-Object System.Text.StringBuilder
foreach ($l in $Lines) { [void]$Out.Append($l.Body); [void]$Out.Append($l.Eol) }
$NewBytes = $Utf8.GetBytes($Out.ToString())
if ($HasBom) { $NewBytes = [byte[]](0xEF, 0xBB, 0xBF) + $NewBytes }

Write-Output ("edit_line : {0} ligne {1}{2}" -f $Path, ($i + 1), $(if ($InsertAfter) { ' (insertion apres)' } else { '' }))
Write-Output ("  avant : " + $Old)
Write-Output ("  apres : " + $NewLine)
if ($DryRun) { Write-Output '  DryRun : rien ecrit'; exit 0 }
# Relu juste avant d'ecrire : une autre session a pu ecrire entre-temps (plusieurs sessions ecrivent EtapeDefini.md en parallele).
$Again = [IO.File]::ReadAllBytes($Path)
if ($Again.Length -ne $Bytes.Length -or [Convert]::ToBase64String($Again) -ne [Convert]::ToBase64String($Bytes)) { Refuse 'le fichier a change pendant l edition : relancer' }
[IO.File]::WriteAllBytes($Path, $NewBytes)
exit 0
