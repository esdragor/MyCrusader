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

    Write-Output '--- 6. L15-2 : -Match must hit EXACTLY one line (refusal proposes -Append) ; -Append adds a line without overwriting any'
    $f = Put 'Etape2.md' ("# Etapes`r`n- HALDIR-1114g : D799 historique`r`n- HALDIR-1115a : D799 en cours`r`n- ARWEN : fini`r`n")
    $before = Bytes $f
    $o = Run @('-File', $f, '-Match', 'D799', '-NewLine', '- HALDIR-1115a : fait')
    Check ($Code -ne 0 -and $o -match '2 lignes' -and $o -match '-Append' -and (Same $before (Bytes $f))) 'L15-2 two lines hold -Match: refused, -Append proposed, file unchanged' $o
    $o = Run @('-File', $f, '-Match', 'D800', '-NewLine', 'x')
    Check ($Code -ne 0 -and $o -match 'aucune ligne' -and $o -match '-Append' -and (Same $before (Bytes $f))) 'L15-2 no line holds -Match: refused, -Append proposed, file unchanged' $o
    $o = Run @('-File', $f, '-Append', "- FARAMIR-1115b : L15-2 fait$e", '-After', 'HALDIR')
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- HALDIR-1114g : D799 historique`r`n- HALDIR-1115a : D799 en cours`r`n- FARAMIR-1115b : L15-2 fait$e`r`n- ARWEN : fini`r`n") 'L15-2 -Append -After: added after the LAST line holding -After, CRLF kept, no line overwritten' ((Text $f) -replace "`r", '\r' -replace "`n", '\n')
    $o = Run @('-File', $f, '-Append', 'fin')
    Check ($Code -eq 0 -and (Text $f).EndsWith("- ARWEN : fini`r`nfin`r`n")) 'L15-2 -Append without -After: added at the end of the file with CRLF' ((Text $f) -replace "`r", '\r' -replace "`n", '\n')
    $f2 = Put 'noeol.md' "a`r`nb"
    $o = Run @('-File', $f2, '-Append', 'c')
    Check ($Code -eq 0 -and (Text $f2) -eq "a`r`nb`r`nc") 'L15-2 -Append to a file without final newline: the old last line gets a CRLF, nothing glued' ((Text $f2) -replace "`r", '\r' -replace "`n", '\n')
    $before = Bytes $f
    $o = Run @('-File', $f, '-Append', 'x', '-After', 'introuvable')
    Check ($Code -ne 0 -and (Same $before (Bytes $f))) 'L15-2 -Append -After that no line holds: refused, file unchanged' $o
    $o = Run @('-File', $f, '-Append', "a`r`nb")
    Check ($Code -ne 0 -and (Same $before (Bytes $f))) 'L15-2 -Append text holding a line break: refused, file unchanged' $o

    Write-Output '--- 7. AX-3 : exit code 0 on success, loud failure, written line read back, output cut short does not lose the edit'
    $f = Put 'Ax3.md' ("# Etapes`r`n- D858 : ligne un`r`n- CELEBORN-1120 : D858 voisin`r`n- ARWEN : fini`r`n")
    $o = Run @('-File', $f, '-Append', "- GLORFINDEL-1120d : AX-3 (sans C++, outil)", '-After', 'CELEBORN')
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- D858 : ligne un`r`n- CELEBORN-1120 : D858 voisin`r`n- GLORFINDEL-1120d : AX-3 (sans C++, outil)`r`n- ARWEN : fini`r`n") 'AX-3a -Append: exit code 0 and the line is written after the last -After line' ($o + ' / code ' + $Code)
    Check ($o -match 'relu : - GLORFINDEL-1120d : AX-3') 'AX-3a the tool prints the line READ BACK from the file after writing ("relu : ...")' $o
    $f = Put 'Ax3b.md' ("# Etapes`r`n- D858 : ligne un`r`n- CELEBORN-1120 : D858 voisin`r`n- ARWEN : fini`r`n")
    $before = Bytes $f
    $o = Run @('-File', $f, '-Match', 'D858', '-NewLine', '- D858 : neuf')
    Check ($Code -ne 0 -and $o -match '2 lignes' -and (Same $before (Bytes $f))) 'AX-3b -Match hitting TWO lines (D858 is also in the neighbour): refused with a non-zero code, nothing written' $o
    $o = Run @('-File', $f, '-Match', 'CELEBORN-1120 : D858', '-NewLine', '- CELEBORN-1120 : D858 fait')
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- D858 : ligne un`r`n- CELEBORN-1120 : D858 fait`r`n- ARWEN : fini`r`n") 'AX-3c -Match text found in the NEIGHBOUR line only: exactly that line changes, the D858 line and the others intact' ((Text $f) -replace "`r", '\r' -replace "`n", '\n')
    Check ($o -match 'relu : - CELEBORN-1120 : D858 fait') 'AX-3c the line read back after writing is shown' $o
    $o = Run @('-File', $f, '-Match', '-- tiret', '-NewLine', 'zz')
    Check ($Code -ne 0 -and $o -match 'aucune ligne') 'AX-3d a -Match starting with "--": handled as a value, refused loudly when absent (no silent success)' $o
    # output consumer that stops reading after the first line (| Select-Object -First 1) must not lose the edit
    $f = Put 'Ax3c.md' ("# Etapes`r`n- A`r`n- B`r`n")
    $null = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -File $f -Append '- coupe' | Select-Object -First 1
    Check ((Text $f) -eq "# Etapes`r`n- A`r`n- B`r`n- coupe`r`n") 'AX-3e output cut by Select-Object -First 1: the edit is still written (print comes AFTER the write)' ((Text $f) -replace "`r", '\r' -replace "`n", '\n')
    # a write that fails (read-only file): non-zero code and a message that starts with "edit_line : ECHEC", file unchanged
    $f = Put 'Ax3d.md' ("a`r`nb`r`n")
    $before = Bytes $f
    Set-ItemProperty -LiteralPath $f -Name IsReadOnly -Value $true
    $o = Run @('-File', $f, '-Append', 'c')
    $failCode = $Code
    Set-ItemProperty -LiteralPath $f -Name IsReadOnly -Value $false
    Check ($failCode -ne 0 -and $o -match 'edit_line : ECHEC' -and (Same $before (Bytes $f))) 'AX-3f write failure (read-only file): non-zero code, message "edit_line : ECHEC", file unchanged' ($o + ' / code ' + $failCode)
    Write-Output '--- 8. HALDIR-1120pd : empty -NewLine never erases a line (only an explicit -Delete), multi-line -Match is named, -After tolerates accents (NFC/NFD) and odd spaces'
    $f = Put 'Pd1.md' ("# Etapes`r`n- SAURON-1120pb : file$e`r`n- ARWEN : fini`r`n")
    $before = Bytes $f
    $o = Run @('-File', $f, '-Match', 'SAURON-1120pb', '-NewLine', '""')
    Check ($Code -ne 0 -and $o -match '-Delete' -and (Same $before (Bytes $f))) 'PD-1 an EMPTY -NewLine is refused (message names -Delete), non-zero code, file unchanged' ($o + ' / code ' + $Code)
    $o = Run @('-File', $f, '-Match', 'SAURON-1120pb', '-NewLine', '" "')
    Check ($Code -ne 0 -and (Same $before (Bytes $f))) 'PD-1b a blank (spaces only) -NewLine is refused too, file unchanged' ($o + ' / code ' + $Code)
    $o = Run @('-File', $f, '-Match', 'SAURON-1120pb', '-Delete')
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- ARWEN : fini`r`n") 'PD-2 explicit -Delete removes exactly that line, the other lines and their CRLF kept' $o
    $f = Put 'Pd2.md' ("# Etapes`r`n- A : un`r`n- B : deux`r`n")
    $before = Bytes $f
    $o = Run @('-File', $f, '-Match', "A : un`nB : deux", '-NewLine', 'zz')
    Check ($Code -ne 0 -and $o -match 'multi' -and (Same $before (Bytes $f))) 'PD-3 a multi-line -Match is refused with a message that says it is multi-line (one line at a time), file unchanged' ($o + ' / code ' + $Code)
    # -After with an accent: the file holds the DECOMPOSED form (e + U+0301), the argument the composed one (and the reverse)
    $nfd = 'e' + [string][char]0x0301
    $f = Put 'Pd3.md' ("# Etapes`r`n- fin d${nfd}j${a} vu`r`n- ARWEN : fini`r`n")
    $o = Run @('-File', $f, '-Append', '- apres accent', '-After', "d${e}j")
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- fin d${nfd}j${a} vu`r`n- apres accent`r`n- ARWEN : fini`r`n") 'PD-4 -After "d(e acute)j" finds a line holding the decomposed accent (Unicode NFC/NFD tolerated)' $o
    $f = Put 'Pd4.md' ("# Etapes`r`n- fin d${e}j${a} vu`r`n- ARWEN : fini`r`n")
    $o = Run @('-File', $f, '-Append', '- apres accent', '-After', "d${nfd}j")
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- fin d${e}j${a} vu`r`n- apres accent`r`n- ARWEN : fini`r`n") 'PD-4b the reverse: composed accent in the file, decomposed in -After' $o
    # -After ASCII exact text, file line holds a no-break space and a double space where the argument has one plain space
    $nb = [string][char]0x00A0
    $f = Put 'Pd5.md' ("# Etapes`r`n- E-specialization_cheat${nb}=${nb}OK (abc)`r`n- ARWEN : fini`r`n")
    $o = Run @('-File', $f, '-Append', '- apres cheat', '-After', 'E-specialization_cheat = OK')
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- E-specialization_cheat${nb}=${nb}OK (abc)`r`n- apres cheat`r`n- ARWEN : fini`r`n") 'PD-5 an ASCII -After "E-specialization_cheat = OK" finds the line even when it holds no-break spaces' $o
    $f = Put 'Pd6.md' ("# Etapes`r`n- E-specialization_cheat  =  OK (abc)`r`n- ARWEN : fini`r`n")
    $o = Run @('-File', $f, '-Append', '- apres cheat', '-After', 'E-specialization_cheat = OK')
    Check ($Code -eq 0 -and (Text $f) -eq "# Etapes`r`n- E-specialization_cheat  =  OK (abc)`r`n- apres cheat`r`n- ARWEN : fini`r`n") 'PD-5b an ASCII -After finds a line holding doubled spaces' $o
    # a refusal for an absent -After says what was tried (exact, then normalised) so the agent does not guess
    $o = Run @('-File', $f, '-Append', 'x', '-After', 'texte absent du fichier')
    Check ($Code -ne 0 -and $o -match 'normalis') 'PD-6 an absent -After is refused and the message says the comparison was also tried after Unicode / space normalisation' $o
} catch {
    $Fails++; Write-Output ('FAIL exception : ' + $_)
} finally {
    Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
}
Write-Output ("edit_line_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $Fails
