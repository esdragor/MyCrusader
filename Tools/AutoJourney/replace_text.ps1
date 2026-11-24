# replace_text.ps1 (ARWEN-1124f, F7 ; L15-3) : remplace UN texte exact et UNIQUE dans un fichier, sans Edit refuse ("not read"),
# en gardant le BOM et les fins de ligne (remplace la commande PowerShell inline du brief commun, item 5).
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\replace_text.ps1 -File <chemin ABSOLU> -Old "<texte>" -New "<texte>"
# Regles : texte LITTERAL (aucune regex, '$' non interprete) ; EXACTEMENT une occurrence de -Old, sinon refus et fichier inchange ;
#   le BOM UTF-8 present (EF BB BF) est garde, jamais ajoute ; le reste du fichier est conserve a l'octet pres ; le fichier est relu
#   apres l'ecriture (sinon l'original est restaure).
# Sortie / codes : 0 = remplace et relu ; 1 = refus (fichier absent, -Old vide, 0 ou plusieurs occurrences) ; 2 = chemin relatif
#   (ou erreur inattendue) ; 3 = relecture differente (original restaure).
# Pour un .ps1 : verifie ensuite l'analyse sans le lancer : [System.Management.Automation.Language.Parser]::ParseFile($p, [ref]$null, [ref]$errs); $errs
param(
    [Parameter(Mandatory = $true)] [string] $File,
    [Parameter(Mandatory = $true)] [string] $Old,
    [AllowEmptyString()] [string] $New = ''
)
$ErrorActionPreference = 'Stop'
trap { try { Write-Output ("replace_text : ECHEC (erreur inattendue : " + $_.Exception.Message + ")") } catch { }; exit 2 }

if (-not [IO.Path]::IsPathRooted($File)) {
    Write-Output "replace_text : refus, chemin RELATIF ($File) : donne un chemin absolu (un chemin relatif part dans le repertoire du processus)."
    exit 2
}
if (-not (Test-Path -LiteralPath $File -PathType Leaf)) { Write-Output "replace_text : refus, fichier absent ($File)."; exit 1 }
if ($Old.Length -eq 0) { Write-Output 'replace_text : refus, -Old est vide.'; exit 1 }

$bytes = [IO.File]::ReadAllBytes($File)
$hasBom = ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF)
$enc = New-Object System.Text.UTF8Encoding $false
$text = $enc.GetString($bytes, $(if ($hasBom) { 3 } else { 0 }), $bytes.Length - $(if ($hasBom) { 3 } else { 0 }))

$count = 0; $pos = 0; $firstAt = -1
while (($i = $text.IndexOf($Old, $pos, [StringComparison]::Ordinal)) -ge 0) {
    if ($count -eq 0) { $firstAt = $i }
    $count++; $pos = $i + $Old.Length
}
if ($count -ne 1) {
    Write-Output "replace_text : refus, $count occurrence(s) de -Old dans $File (il en faut exactement 1) ; fichier inchange."
    exit 1
}
$line = 1 + ([regex]::Matches($text.Substring(0, $firstAt), "`n")).Count

$newText = $text.Substring(0, $firstAt) + $New + $text.Substring($firstAt + $Old.Length)
$outBytes = $enc.GetBytes($newText)
if ($hasBom) { $outBytes = [byte[]](0xEF, 0xBB, 0xBF) + $outBytes }
[IO.File]::WriteAllBytes($File, $outBytes)

$back = [IO.File]::ReadAllBytes($File)
if ([Convert]::ToBase64String($back) -ne [Convert]::ToBase64String($outBytes)) {
    [IO.File]::WriteAllBytes($File, $bytes)
    Write-Output "replace_text : relecture differente, original restaure ($File)."
    exit 3
}
Write-Output "replace_text : $File ligne $line, 1 remplacement$(if ($hasBom) { ' (BOM garde)' })."
exit 0
