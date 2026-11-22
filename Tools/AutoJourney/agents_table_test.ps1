# agents_table_test.ps1 (L15-4) : teste agents_table.ps1 sur une COPIE TEMPORAIRE synthetique, jamais sur le vrai Auto_Journey_Jour.md
# (le test passe toujours -Jour ; le vrai fichier, edite en parallele par d'autres sessions, ne se compare pas).
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\agents_table_test.ps1   (code 0 = vert, 1 = rouge)
# Fichier ASCII seul (pas de BOM necessaire) ; les accents sont fabriques avec [char]0xE9.
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$tool = Join-Path $here 'agents_table.ps1'
$tmp = Join-Path $env:TEMP ('agents_table_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $tmp | Out-Null
$realJour = 'C:\Users\baudo\Documents\MyCrusader\Auto_Journey_Jour.md'
$realHash = if (Test-Path -LiteralPath $realJour) { (Get-FileHash -LiteralPath $realJour -Algorithm SHA256).Hash } else { '' }
$enc = New-Object System.Text.UTF8Encoding $false
$eacute = [string][char]0xE9
$today = Get-Date -Format 'yyyy-MM-dd'
$fail = 0; $ok = 0
function Check([string]$Name, [bool]$Cond, [string]$Detail = '') {
    if ($Cond) { $script:ok++; Write-Output "  ok   $Name" } else { $script:fail++; Write-Output "  ROUGE $Name $Detail" }
}
function New-Fixture([string]$Name, [string]$Body) {
    $p = Join-Path $tmp $Name
    [IO.File]::WriteAllBytes($p, $enc.GetBytes($Body))
    return $p
}
function Get-Bytes([string]$P) { return , [IO.File]::ReadAllBytes($P) }
function Same-Bytes($A, $B) { return ([Convert]::ToBase64String($A) -eq [Convert]::ToBase64String($B)) }
function Run-Tool([string[]]$ToolArgs) {
    $ErrorActionPreference = 'Continue'
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $tool @ToolArgs 2>&1 | Out-String
    return @{ Code = $LASTEXITCODE; Out = $out }
}
function Cells([string]$Line) { $p = [regex]::Split($Line, '(?<!\\)\|'); return , @($p[1..($p.Count - 2)] | ForEach-Object { $_.Trim() }) }
function Now-Candidates { return @((Get-Date).ToString('HH:mm'), (Get-Date).AddMinutes(-1).ToString('HH:mm'), (Get-Date).AddMinutes(1).ToString('HH:mm')) }

$hdr = '| Agent | id | Worktree | T' + $eacute + 'che | C++ | Lanc' + $eacute + ' | Fini | R' + $eacute + 'sum' + $eacute + ' |'
$sep = '|---|---|---|---|---|---|---|---|'
$rowA = '| AAA-1 | v1 | wt-a | tache a | non | 09:00 | en cours | |'
$rowB = '| BBB-1 | v2 | wt-b | tache b | oui | 09:10 | 09:20 | fait ' + $eacute + ' |'
$bodyHead = "# Jour`r`n`r`nintro avec | un pipe`r`n`r`n## Agents du 2000-01-01`r`n`r`n$hdr`r`n$sep`r`n| X-1 | a | w | t | non | 10:00 | 10:05 | vieux |`r`n`r`n"
$bodyToday = "## Agents du $today`r`n`r`ntexte libre`r`n`r`n$hdr`r`n$sep`r`n$rowA`r`n$rowB`r`n"
$bodyTail = "`r`n## Autre section`r`n`r`n- une ligne`r`n- derniere`r`n"
$base = $bodyHead + $bodyToday + $bodyTail

Write-Output "agents_table_test : copie temporaire $tmp"

# 1. -Add : ligne neuve juste apres la derniere ligne du tableau du jour, reste a l'octet pres
$f1 = New-Fixture 'add.md' $base
$before = Get-Bytes $f1
$r = Run-Tool @('-Jour', $f1, '-Add', 'CCC-1', '-Id', 'v3', '-Worktree', 'wt-c', '-Task', 'tache c', '-Cpp', 'oui')
Check 'add : code 0' ($r.Code -eq 0) "code=$($r.Code) out=$($r.Out)"
$after = $enc.GetString((Get-Bytes $f1))
$lines = [regex]::Split($after, "`r`n")
$idx = [array]::FindIndex($lines, [Predicate[string]] { param($l) $l.StartsWith('| CCC-1 ') })
Check 'add : ligne presente' ($idx -ge 0)
if ($idx -ge 0) {
    Check 'add : juste apres la derniere ligne du tableau du jour' ($lines[$idx - 1] -eq $rowB)
    $c = Cells $lines[$idx]
    Check 'add : 8 cellules' ($c.Count -eq 8) "n=$($c.Count)"
    Check 'add : agent/id/worktree/tache/C++' (($c[0] -eq 'CCC-1') -and ($c[1] -eq 'v3') -and ($c[2] -eq 'wt-c') -and ($c[3] -eq 'tache c') -and ($c[4] -eq 'oui')) ($lines[$idx])
    Check 'add : Lance = Get-Date (HH:mm)' ((Now-Candidates) -contains $c[5]) "lance=$($c[5])"
    Check 'add : Fini = en cours, resume vide' (($c[6] -eq 'en cours') -and ($c[7] -eq ''))
    $without = ($lines | Where-Object { -not $_.StartsWith('| CCC-1 ') }) -join "`r`n"
    Check 'add : reste du fichier identique a l''octet (CRLF, accents)' (Same-Bytes $before $enc.GetBytes($without))
    Check 'add : la sortie contient la ligne relue' ($r.Out.Contains($lines[$idx]))
}
Check 'add : pas de BOM' (-not ((Get-Bytes $f1)[0] -eq 0xEF))
Check 'add : aucun LF seul' (([regex]::Matches($after, "(?<!`r)`n")).Count -eq 0)

# 2. -Add : | du texte echappes
$f2 = New-Fixture 'pipe.md' $base
$r = Run-Tool @('-Jour', $f2, '-Add', 'DDD-1', '-Id', 'v4', '-Worktree', 'wt-d', '-Task', 'a|b et c|d', '-Cpp', 'non')
Check 'pipe : code 0' ($r.Code -eq 0) $r.Out
$l2 = [regex]::Split($enc.GetString((Get-Bytes $f2)), "`r`n") | Where-Object { $_.StartsWith('| DDD-1 ') }
Check 'pipe : | echappe en \|' (($l2 -ne $null) -and $l2.Contains('a\|b et c\|d'))
Check 'pipe : toujours 8 cellules' ((Cells $l2).Count -eq 8)

# 3. -Add : agent deja present dans la section du jour = refus code 1, fichier intact ; present un autre jour seulement = ajoute
$f3 = New-Fixture 'dup.md' $base
$before = Get-Bytes $f3
$r = Run-Tool @('-Jour', $f3, '-Add', 'AAA-1', '-Id', 'v9', '-Worktree', 'wt', '-Task', 't', '-Cpp', 'non')
Check 'doublon : refus code 1' ($r.Code -eq 1) "code=$($r.Code) out=$($r.Out)"
Check 'doublon : fichier intact' (Same-Bytes $before (Get-Bytes $f3))
$r = Run-Tool @('-Jour', $f3, '-Add', 'X-1', '-Id', 'v9', '-Worktree', 'wt', '-Task', 't', '-Cpp', 'non')
Check 'agent d''un autre jour : ajoute (code 0)' ($r.Code -eq 0) "code=$($r.Code) out=$($r.Out)"

# 4. -Update : Fini = Get-Date, resume echappe, une seule ligne change
$f4 = New-Fixture 'upd.md' $base
$before = $enc.GetString((Get-Bytes $f4))
$r = Run-Tool @('-Jour', $f4, '-Update', 'AAA-1', '-Summary', "fait | ok $eacute")
Check 'update : code 0' ($r.Code -eq 0) "code=$($r.Code) out=$($r.Out)"
$after = $enc.GetString((Get-Bytes $f4))
$bl = [regex]::Split($before, "`r`n"); $al = [regex]::Split($after, "`r`n")
Check 'update : meme nombre de lignes' ($bl.Count -eq $al.Count)
$diff = @(); for ($i = 0; $i -lt [Math]::Min($bl.Count, $al.Count); $i++) { if ($bl[$i] -ne $al[$i]) { $diff += $i } }
Check 'update : une seule ligne change' ($diff.Count -eq 1) "diff=$($diff -join ',')"
if ($diff.Count -eq 1) {
    $c = Cells $al[$diff[0]]
    Check 'update : 8 cellules' ($c.Count -eq 8)
    Check 'update : agent/id/worktree/tache/C++/Lance inchanges' (($c[0] -eq 'AAA-1') -and ($c[1] -eq 'v1') -and ($c[2] -eq 'wt-a') -and ($c[3] -eq 'tache a') -and ($c[4] -eq 'non') -and ($c[5] -eq '09:00'))
    Check 'update : Fini = Get-Date (HH:mm)' ((Now-Candidates) -contains $c[6]) "fini=$($c[6])"
    Check 'update : resume echappe' ($c[7] -eq "fait \| ok $eacute") "resume=$($c[7])"
    Check 'update : sortie = ligne relue' ($r.Out.Contains($al[$diff[0]]))
}
Check 'update : CRLF et pas de BOM conserves' ((-not ((Get-Bytes $f4)[0] -eq 0xEF)) -and (([regex]::Matches($after, "(?<!`r)`n")).Count -eq 0))

# 5. -Update : absent ou ambigu = refus code 1, fichier intact
$f5 = New-Fixture 'upd_absent.md' $base
$before = Get-Bytes $f5
$r = Run-Tool @('-Jour', $f5, '-Update', 'ZZZ-9', '-Summary', 'x')
Check 'update absent : refus code 1' ($r.Code -eq 1) "code=$($r.Code) out=$($r.Out)"
Check 'update absent : fichier intact' (Same-Bytes $before (Get-Bytes $f5))
$f5b = New-Fixture 'upd_ambigu.md' ($bodyHead + $bodyToday + $rowA + "`r`n" + $bodyTail)
$before = Get-Bytes $f5b
$r = Run-Tool @('-Jour', $f5b, '-Update', 'AAA-1', '-Summary', 'x')
Check 'update ambigu : refus code 1' ($r.Code -eq 1) "code=$($r.Code) out=$($r.Out)"
Check 'update ambigu : fichier intact' (Same-Bytes $before (Get-Bytes $f5b))
$r = Run-Tool @('-Jour', $f5b, '-Update', 'X-1', '-Summary', 'x')
Check 'update : agent d''un autre jour = absent de la section du jour (refus)' ($r.Code -eq 1) "code=$($r.Code)"

# 6. l'heure ne vient jamais d'un parametre
$f6 = New-Fixture 'heure.md' $base
$before = Get-Bytes $f6
$r = Run-Tool @('-Jour', $f6, '-Add', 'EEE-1', '-Id', 'v', '-Worktree', 'w', '-Task', 't', '-Cpp', 'non', '-Lance', '01:00')
Check 'heure en parametre : refuse (code non nul)' ($r.Code -ne 0) "code=$($r.Code)"
Check 'heure en parametre : fichier intact' (Same-Bytes $before (Get-Bytes $f6))

# 7. section du jour absente = refus code 1 ; tableau en derniere ligne sans fin de ligne = rien de colle
$f7 = New-Fixture 'sans_section.md' ($bodyHead + $bodyTail)
$before = Get-Bytes $f7
$r = Run-Tool @('-Jour', $f7, '-Add', 'FFF-1', '-Id', 'v', '-Worktree', 'w', '-Task', 't', '-Cpp', 'non')
Check 'section absente : refus code 1' ($r.Code -eq 1) "code=$($r.Code) out=$($r.Out)"
Check 'section absente : fichier intact' (Same-Bytes $before (Get-Bytes $f7))
$f8 = New-Fixture 'fin_sans_crlf.md' ($bodyHead + "## Agents du $today`r`n`r`n$hdr`r`n$sep`r`n$rowA")
$r = Run-Tool @('-Jour', $f8, '-Add', 'GGG-1', '-Id', 'v', '-Worktree', 'w', '-Task', 't', '-Cpp', 'non')
Check 'derniere ligne sans CRLF : code 0' ($r.Code -eq 0) "code=$($r.Code) out=$($r.Out)"
$t8 = $enc.GetString((Get-Bytes $f8))
$l8 = [regex]::Split($t8, "`r`n")
Check 'derniere ligne sans CRLF : AAA-1 puis GGG-1 sur deux lignes' (($l8[-2] -eq $rowA) -and $l8[-1].StartsWith('| GGG-1 |'))

Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output "agents_table_test : $ok ok, $fail rouge(s)"
if ($fail -eq 0) { Write-Output 'agents_table_test : VERT'; exit 0 } else { Write-Output 'agents_table_test : ROUGE'; exit 1 }
