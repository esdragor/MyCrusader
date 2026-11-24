# replace_text_test.ps1 (ARWEN-1124f, F7) : teste replace_text.ps1 (remplacement exact et unique, BOM garde) sur des fichiers TEMPORAIRES.
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\replace_text_test.ps1   (code 0 = vert, 1 = rouge)
# Fichier ASCII seul (pas de BOM necessaire) ; les accents sont fabriques avec [char]0xE9.
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$tool = Join-Path $here 'replace_text.ps1'
$tmp = Join-Path $env:TEMP ('replace_text_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $tmp | Out-Null
$fail = 0; $ok = 0
$eacute = [string][char]0xE9
function Check([string]$Name, [bool]$Cond, [string]$Detail = '') {
    if ($Cond) { $script:ok++; Write-Output "  ok   $Name" } else { $script:fail++; Write-Output "  ROUGE $Name $Detail" }
}
function New-Fixture([string]$Name, [byte[]]$Bytes) {
    $p = Join-Path $tmp $Name
    [IO.File]::WriteAllBytes($p, $Bytes)
    return $p
}
function Run-Tool([string[]]$ToolArgs) {
    $ErrorActionPreference = 'Continue'
    if (-not (Test-Path -LiteralPath $tool)) { return @{ Code = 99; Out = "outil absent : $tool" } }
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $tool @ToolArgs 2>&1 | Out-String
    return @{ Code = $LASTEXITCODE; Out = $out }
}
function Same-Bytes($A, $B) { return ([Convert]::ToBase64String($A) -eq [Convert]::ToBase64String($B)) }
$utf8 = New-Object System.Text.UTF8Encoding $false
$bomBytes = [byte[]](0xEF, 0xBB, 0xBF)

Write-Output "replace_text_test : dossier temporaire $tmp"

# 1. Remplacement unique : le texte change, le reste est conserve a l'octet pres (CRLF compris), code 0.
$orig1 = $utf8.GetBytes("ligne un`r`nvaleur = ALPHA ($eacute)`r`nligne trois`r`n")
$f1 = New-Fixture 'un.txt' $orig1
$r1 = Run-Tool @('-File', $f1, '-Old', 'ALPHA', '-New', 'BETA')
$exp1 = $utf8.GetBytes("ligne un`r`nvaleur = BETA ($eacute)`r`nligne trois`r`n")
Check '1 unique : code 0' ($r1.Code -eq 0) "code=$($r1.Code) $($r1.Out)"
Check '1 unique : fichier = attendu a l''octet pres (CRLF et accent gardes)' (Same-Bytes ([IO.File]::ReadAllBytes($f1)) $exp1)

# 2. Zero occurrence : code 1, fichier intact.
$f2 = New-Fixture 'zero.txt' $orig1
$r2 = Run-Tool @('-File', $f2, '-Old', 'ABSENT', '-New', 'X')
Check '2 zero : code 1' ($r2.Code -eq 1) "code=$($r2.Code) $($r2.Out)"
Check '2 zero : fichier intact' (Same-Bytes ([IO.File]::ReadAllBytes($f2)) $orig1)

# 3. Deux occurrences : code 1, fichier intact.
$orig3 = $utf8.GetBytes("a ALPHA b`r`nc ALPHA d`r`n")
$f3 = New-Fixture 'deux.txt' $orig3
$r3 = Run-Tool @('-File', $f3, '-Old', 'ALPHA', '-New', 'BETA')
Check '3 deux : code 1' ($r3.Code -eq 1) "code=$($r3.Code) $($r3.Out)"
Check '3 deux : fichier intact' (Same-Bytes ([IO.File]::ReadAllBytes($f3)) $orig3)

# 4. Fichier a BOM : les 3 octets EF BB BF restent en tete apres l'ecriture.
$orig4 = $bomBytes + $utf8.GetBytes("# titre $eacute`r`nvaleur = ALPHA`r`n")
$f4 = New-Fixture 'bom.ps1' $orig4
$r4 = Run-Tool @('-File', $f4, '-Old', 'ALPHA', '-New', 'BETA')
$got4 = [IO.File]::ReadAllBytes($f4)
$exp4 = $bomBytes + $utf8.GetBytes("# titre $eacute`r`nvaleur = BETA`r`n")
Check '4 BOM : code 0' ($r4.Code -eq 0) "code=$($r4.Code) $($r4.Out)"
Check '4 BOM : 3 octets EF BB BF gardes' ($got4.Length -ge 3 -and $got4[0] -eq 0xEF -and $got4[1] -eq 0xBB -and $got4[2] -eq 0xBF)
Check '4 BOM : contenu = attendu, un seul BOM' (Same-Bytes $got4 $exp4)

# 5. Chemin relatif : code 2, fichier intact (rien ecrit nulle part).
$orig5 = $utf8.GetBytes("valeur = ALPHA`r`n")
$f5 = New-Fixture 'rel.txt' $orig5
Push-Location $tmp
try { $r5 = Run-Tool @('-File', 'rel.txt', '-Old', 'ALPHA', '-New', 'BETA') } finally { Pop-Location }
Check '5 relatif : code 2' ($r5.Code -eq 2) "code=$($r5.Code) $($r5.Out)"
Check '5 relatif : fichier intact' (Same-Bytes ([IO.File]::ReadAllBytes($f5)) $orig5)

# 6. Texte litteral : -Old avec des caracteres de regex et un '$' dans -New ne sont pas interpretes.
$f6 = New-Fixture 'litteral.txt' ($utf8.GetBytes("x = (a.b)[0] + 1`r`n"))
$r6 = Run-Tool @('-File', $f6, '-Old', '(a.b)[0]', '-New', '$1 & $0')
Check '6 litteral : code 0' ($r6.Code -eq 0) "code=$($r6.Code) $($r6.Out)"
Check '6 litteral : $1 et $0 ecrits tels quels' (Same-Bytes ([IO.File]::ReadAllBytes($f6)) ($utf8.GetBytes("x = " + '$1 & $0' + " + 1`r`n")))

Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output "replace_text_test : $ok ok, $fail rouge"
if ($fail -gt 0) { exit 1 } else { exit 0 }
