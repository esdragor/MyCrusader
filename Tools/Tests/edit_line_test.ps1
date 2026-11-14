# Test de bout en bout de Tools\AutoJourney\edit_line.ps1 (L14-1) : edition sure d'UNE ligne d'un fichier de suivi, sans Read
# prealable, fins de ligne de chaque ligne preservees (CRLF, LF, derniere ligne sans fin), jamais de BOM ajoute, accents intacts,
# refus (fichier inchange octet pour octet) si 0 ou plusieurs lignes correspondent, ou si le texte neuf contient une fin de ligne.
# Lecon du 2026-11-14 : un Edit multi-lignes a perdu des CRLF et colle des lignes dans Auto_Journey_File.md.
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\edit_line_test.ps1
# Code de sortie : nombre d'assertions rouges (0 = tout vert). N'ecrit rien hors du dossier temporaire.
$ErrorActionPreference = 'Continue'
$Tool = Join-Path (Split-Path $PSScriptRoot) 'AutoJourney\edit_line.ps1'
$Tmp = Join-Path $env:TEMP ('edit_line_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force $Tmp | Out-Null
$Fails = 0
$Utf8 = New-Object System.Text.UTF8Encoding($false)
$e = [string][char]0xE9; $a = [string][char]0xE0   # accents ecrits par code : ce .ps1 reste ASCII (PowerShell 5.1 lit un .ps1 sans BOM en ANSI)
function Check([bool]$Ok, [string]$What, [string]$Detail = '') {
    if ($Ok) { Write-Output "PASS $What" } else { $script:Fails++; Write-Output "FAIL $What  $Detail" }
}
function Put([string]$Name, [string]$Text) { $p = Join-Path $Tmp $Name; [IO.File]::WriteAllBytes($p, $Utf8.GetBytes($Text)); $p }
function Bytes([string]$Path) { [IO.File]::ReadAllBytes($Path) }
function Text([string]$Path) { $Utf8.GetString((Bytes $Path)) }
function Same([byte[]]$A, [byte[]]$B) { if ($A.Length -ne $B.Length) { return $false }; for ($i = 0; $i -lt $A.Length; $i++) { if ($A[$i] -ne $B[$i]) { return $false } }; $true }
function Run([string[]]$ToolArgs) {
    $o = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool @ToolArgs 2>&1 | Out-String
    $script:Code = $LASTEXITCODE
    Write-Host ("  > code $script:Code : " + $o.Trim())
    $o
}

try {
    Write-Output '--- 1. CRLF file, literal match with markdown brackets and parentheses: only that line changes'
    $f = Put 'EtapeDefini.md' ("# Etapes`r`n- [ ] (sans C++, outil) Faramir : en cours$e`r`n- [ ] Arwen : fini $a 10:00`r`n")
    $o = Run @('-File', $f, '-Match', '- [ ] (sans C++, outil) Faramir', '-NewLine', "- [x] (sans C++, outil) Faramir : fait$e (abc123)")
    $t = Text $f
    Check ($Code -eq 0) 'exit code 0 on a unique match' $o
    Check ($t -eq "# Etapes`r`n- [x] (sans C++, outil) Faramir : fait$e (abc123)`r`n- [ ] Arwen : fini $a 10:00`r`n") 'the line is replaced, the other lines and every CRLF kept, accents intact' ($t -replace "`r", '\r' -replace "`n", '\n')
    $b = Bytes $f
    Check (-not ($b.Length -ge 3 -and $b[0] -eq 0xEF -and $b[1] -eq 0xBB -and $b[2] -eq 0xBF)) 'no BOM added'
    Check (([regex]::Matches($t, "(?<!`r)`n")).Count -eq 0) 'no bare LF introduced'

    Write-Output '--- 2. mixed EOL (CRLF then LF) and no final newline: each line keeps its own terminator'
    $f = Put 'Auto_Journey_File.md' ("a1`r`nb2`nc3")
    $o = Run @('-File', $f, '-Match', 'c3', '-NewLine', 'c3 fait')
    Check ((Text $f) -eq "a1`r`nb2`nc3 fait") 'last line without newline replaced, LF line untouched' ((Text $f) -replace "`r", '\r' -replace "`n", '\n')
    $o = Run @('-File', $f, '-Match', 'a1', '-NewLine', 'a1 bis')
    Check ((Text $f) -eq "a1 bis`r`nb2`nc3 fait") 'first CRLF line replaced, CRLF kept' ((Text $f) -replace "`r", '\r' -replace "`n", '\n')

    Write-Output '--- 3. refusals leave the file byte for byte'
    $f = Put 'E-x.md' ("statut : [A ESSAYER]`r`nligne x`r`nligne x deux`r`n")
    $before = Bytes $f
    $o = Run @('-File', $f, '-Match', 'ligne x', '-NewLine', 'zz')
    Check ($Code -ne 0 -and $o -match '2 lignes' -and (Same $before (Bytes $f))) 'two matching lines: refused, lines named, file unchanged' $o
    $o = Run @('-File', $f, '-Match', 'absente', '-NewLine', 'zz')
    Check ($Code -ne 0 -and $o -match 'aucune ligne' -and (Same $before (Bytes $f))) 'no matching line: refused, file unchanged' $o
    $o = Run @('-File', $f, '-Match', 'statut', '-NewLine', "deux`r`nlignes")
    Check ($Code -ne 0 -and (Same $before (Bytes $f))) 'a new text holding a line break: refused, file unchanged' $o
    $o = Run @('-File', (Join-Path $Tmp 'absent.md'), '-Match', 'x', '-NewLine', 'y')
    Check ($Code -ne 0) 'a missing file: refused' $o

    Write-Output '--- 4. -InsertAfter and -Regex'
    $o = Run @('-File', $f, '-Match', '^ligne x$', '-Regex', '-NewLine', "ins$e", '-InsertAfter')
    Check ($Code -eq 0 -and (Text $f) -eq "statut : [A ESSAYER]`r`nligne x`r`nins$e`r`nligne x deux`r`n") 'regex anchors one line, the new line is inserted after it with the file CRLF' ((Text $f) -replace "`r", '\r' -replace "`n", '\n')

    Write-Output '--- 5. a BOM file keeps its BOM (no diff), a no-BOM file never gets one'
    $p = Join-Path $Tmp 'bom.md'; [IO.File]::WriteAllBytes($p, [byte[]](0xEF, 0xBB, 0xBF) + $Utf8.GetBytes("x`r`ny`r`n"))
    $o = Run @('-File', $p, '-Match', 'y', '-NewLine', 'y2')
    $b = Bytes $p
    Check ($b[0] -eq 0xEF -and $Utf8.GetString($b, 3, $b.Length - 3) -eq "x`r`ny2`r`n") 'BOM kept, line replaced' $o
} catch {
    $Fails++; Write-Output ('FAIL exception : ' + $_)
} finally {
    Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
}
Write-Output ("edit_line_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $Fails
