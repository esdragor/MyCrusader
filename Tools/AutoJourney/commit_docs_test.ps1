# commit_docs_test.ps1 : test d'effet de commit_docs.ps1 (L9-5). Aucun C++, aucun editeur ; travaille sur un VRAI depot git jetable dans %TEMP%.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (une ligne PASS/FAIL par cas, code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'commit_docs.ps1'
$Gate = Join-Path $Here 'session_gate.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
function Git { param([string]$Repo) & git.exe -C $Repo @args 2>&1 }   # git.exe : a bare "git" would call this function again (names are case-insensitive)
function Put([string]$Repo, [string]$Rel, [string]$Text) {
  $p = Join-Path $Repo $Rel
  $d = Split-Path $p -Parent
  if (-not (Test-Path $d)) { New-Item -ItemType Directory -Path $d -Force | Out-Null }
  [IO.File]::WriteAllText($p, $Text)
}
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('commitdocs-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Repo = Join-Path $Tmp 'main'
New-Item -ItemType Directory -Path $Repo -Force | Out-Null
Git $Repo init -q | Out-Null
Git $Repo config user.name 'Test' | Out-Null
Git $Repo config user.email 'test@example.com' | Out-Null
Git $Repo config core.autocrlf false | Out-Null
Put $Repo 'Auto_Journey_Jour.md' 'v1'
Put $Repo 'Auto_Journey_File.md' 'v1'
Put $Repo 'EtapeDefini.md' 'v1'
Put $Repo 'Retours/E-old.md' 'v1'
Put $Repo 'Retours/E-keep.md' 'v1'
Put $Repo 'Source/x.cpp' 'int x;'
Put $Repo 'other.md' 'v1'
Put $Repo 'Placeholders.txt' 'v1'
Put $Repo 'PlaceholdersInProgress.md' 'v1'
Put $Repo 'FooInProgress.md' 'v1'
Put $Repo 'Docs/SessionEnd.txt' 'v1'
Put $Repo 'Docs/Archive/OldInProgress.md' 'v1'
Put $Repo 'Docs/Priorite_X.md' 'v1'
Put $Repo 'Docs/Autre.md' 'v1'
Git $Repo add -A | Out-Null
Git $Repo commit -q -m 'init' | Out-Null
$Head0 = (Git $Repo rev-parse HEAD | Out-String).Trim()

Check (Test-Path $Tool) 'commit_docs.ps1 existe'
if (-not (Test-Path $Tool)) { Write-Output "commit_docs_test : $Fails FAIL"; exit 1 }

# Etat de depart : docs modifies / supprimes / nouveaux + du CODE modifie + un fichier hors docs deja indexe
Put $Repo 'Auto_Journey_Jour.md' 'v2'
Put $Repo 'EtapeDefini.md' 'v2'
Remove-Item (Join-Path $Repo 'Retours/E-old.md')
Put $Repo 'Retours/E-new.md' 'v1'
Put $Repo 'Source/x.cpp' 'int x; int y;'
Put $Repo 'other.md' 'v2'
Git $Repo add other.md | Out-Null
# L10-tool (2026-11-11) : Placeholders*, *InProgress.md SUIVIS, Docs/SessionEnd.txt sont aussi des docs ; un *InProgress.md NON suivi ou Docs/Archive/ ne le sont pas
Put $Repo 'Placeholders.txt' 'v2'
Put $Repo 'PlaceholdersInProgress.md' 'v2'
Put $Repo 'FooInProgress.md' 'v2'
Put $Repo 'Docs/SessionEnd.txt' 'v2'
Put $Repo 'Docs/Archive/OldInProgress.md' 'v2'
Put $Repo 'NewInProgress.md' 'v1'

# --- DryRun : liste, ne committe pas
$dry = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -DryRun | Out-String
$codeDry = $LASTEXITCODE
Check ($codeDry -eq 0 -and $dry -match 'Auto_Journey_Jour\.md' -and $dry -match 'E-new\.md' -and $dry -match 'E-old\.md' -and $dry -match 'Placeholders\.txt' -and $dry -match 'PlaceholdersInProgress\.md' -and $dry -match 'FooInProgress\.md' -and $dry -match 'SessionEnd\.txt') 'DryRun: liste les docs (modifie, nouveau, supprime, Placeholders, *InProgress.md suivi, SessionEnd.txt)' "code=$codeDry out=$dry"
Check ($dry -notmatch 'x\.cpp' -and $dry -notmatch 'other\.md' -and $dry -notmatch 'NewInProgress' -and $dry -notmatch 'OldInProgress') 'DryRun: ne liste ni le code ni les fichiers hors docs' $dry
Check ((Git $Repo rev-parse HEAD | Out-String).Trim() -eq $Head0) 'DryRun: aucun commit cree'

# --- Vrai passage
$out = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo | Out-String
$code = $LASTEXITCODE
$Head1 = (Git $Repo rev-parse HEAD | Out-String).Trim()
Check ($code -eq 0 -and $Head1 -ne $Head0) 'commit cree (code 0, HEAD avance)' "code=$code out=$out"
Check ((Git $Repo rev-list --count "$Head0..HEAD" | Out-String).Trim() -eq '1') 'exactement 1 commit'
$files = @(Git $Repo show --name-only --no-renames --format= HEAD | ForEach-Object { "$_".Trim() } | Where-Object { $_ })
$expect = @('Auto_Journey_Jour.md', 'EtapeDefini.md', 'Retours/E-new.md', 'Retours/E-old.md', 'Placeholders.txt', 'PlaceholdersInProgress.md', 'FooInProgress.md', 'Docs/SessionEnd.txt') | Sort-Object
Check ((($files | Sort-Object) -join '|') -eq ($expect -join '|')) 'le commit contient EXACTEMENT les docs (y compris la suppression et le nouveau Retours)' ($files -join ', ')
$st = @(Git $Repo status --short | ForEach-Object { "$_" })
Check (($st | Where-Object { $_ -match 'Source/x\.cpp' }).Count -eq 1 -and ($st | Where-Object { $_ -match '^ M Source/x\.cpp' }).Count -eq 1) 'le code modifie reste NON commite' ($st -join ' ; ')
Check (($st | Where-Object { $_ -match '^M  other\.md' }).Count -eq 1) 'un fichier hors docs deja indexe reste indexe, hors du commit' ($st -join ' ; ')
$msg = (Git $Repo log -1 --format=%B | Out-String)
Check ($msg -match '^Docs' -and $msg -notmatch 'Co-Authored-By') 'message en anglais commencant par Docs, sans Co-Authored-By' $msg
Check (-not (Git $Repo status --short -- Auto_Journey_Jour.md EtapeDefini.md Retours Placeholders.txt PlaceholdersInProgress.md FooInProgress.md Docs/SessionEnd.txt | Out-String).Trim()) 'plus aucun doc en attente'
Check ((@($st | Where-Object { $_ -match '^\?\? NewInProgress\.md' }).Count -eq 1) -and (@($st | Where-Object { $_ -match '^ M Docs/Archive/OldInProgress\.md' }).Count -eq 1)) 'un *InProgress.md NON suivi et Docs/Archive/ restent hors du commit' ($st -join ' ; ')

# --- Rien a committer : pas de commit, code 0
$out2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo | Out-String
$code2 = $LASTEXITCODE
Check ($code2 -eq 0 -and (Git $Repo rev-parse HEAD | Out-String).Trim() -eq $Head1 -and $out2 -match 'rien') 'rien a committer: code 0, pas de commit, message "rien"' "code=$code2 out=$out2"

# --- -Message explicite
Put $Repo 'Auto_Journey_File.md' 'v2'
$out3 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Message 'Docs: custom text' | Out-String
Check ((Git $Repo log -1 --format=%s | Out-String).Trim() -eq 'Docs: custom text') '-Message est utilise tel quel' $out3

# --- L14-12 (2026-11-14) : -Only <fichiers> commite CES docs seuls (une archive de l'agent de docs avait embarque Auto_Journey_Jour.md et des fiches E
# en cours d'edition par la recette). Avec -File, "a,b" arrive en une seule chaine : la liste est separee par des virgules ; \ ou / acceptes.
Put $Repo 'EtapeDefini.md' 'v3'
Put $Repo 'Auto_Journey_Jour.md' 'v3'
Put $Repo 'Retours/E-new.md' 'v2'
Remove-Item (Join-Path $Repo 'Retours/E-keep.md')
$HeadO = (Git $Repo rev-parse HEAD | Out-String).Trim()
$o1 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Only 'EtapeDefini.md' -DryRun | Out-String
Check ($LASTEXITCODE -eq 0 -and $o1 -match 'EtapeDefini\.md' -and $o1 -notmatch 'Auto_Journey_Jour' -and $o1 -notmatch 'E-new' -and (Git $Repo rev-parse HEAD | Out-String).Trim() -eq $HeadO) '-Only -DryRun: liste le seul fichier nomme, aucun commit' $o1
$o2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Only 'EtapeDefini.md,Retours\E-new.md,Retours/E-keep.md' -Message 'Docs: only' | Out-String
$c2 = $LASTEXITCODE
$f2 = @(Git $Repo show --name-only --no-renames --format= HEAD | ForEach-Object { "$_".Trim() } | Where-Object { $_ } | Sort-Object)
Check ($c2 -eq 0 -and (Git $Repo rev-list --count "$HeadO..HEAD" | Out-String).Trim() -eq '1' -and ($f2 -join '|') -eq 'EtapeDefini.md|Retours/E-keep.md|Retours/E-new.md') '-Only: le commit contient EXACTEMENT les fichiers nommes (modifie, \ ou /, suppression)' "code=$c2 files=$($f2 -join ', ') out=$o2"
$st2 = (Git $Repo status --short | Out-String)
Check ($st2 -match '(?m)^ M Auto_Journey_Jour\.md' -and $st2 -match '(?m)^ M Source/x\.cpp') '-Only: un doc modifie NON nomme (autre session) reste hors du commit' $st2
$HeadO2 = (Git $Repo rev-parse HEAD | Out-String).Trim()
$o3 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Only 'Source/x.cpp' | Out-String
$c3 = $LASTEXITCODE
Check ($c3 -ne 0 -and (Git $Repo rev-parse HEAD | Out-String).Trim() -eq $HeadO2 -and $o3 -match 'x\.cpp') '-Only d''un fichier hors docs (code): refuse, code non nul, aucun commit' "code=$c3 out=$o3"
$o4 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Only 'EtapeDefini.md' | Out-String
Check ($LASTEXITCODE -eq 0 -and $o4 -match 'rien' -and (Git $Repo rev-parse HEAD | Out-String).Trim() -eq $HeadO2) '-Only d''un doc inchange: rien a committer, code 0' $o4
Git $Repo checkout -q -- Auto_Journey_Jour.md | Out-Null

# --- 2026-11-24 (SAURON-1124si) : un survey Docs\Priorite_*.md est un doc de suivi ; -Only doit le commiter, mais pas un autre Docs\*.md
Put $Repo 'Docs/Priorite_X.md' 'v2'
Put $Repo 'Docs/Autre.md' 'v2'
$HeadP = (Git $Repo rev-parse HEAD | Out-String).Trim()
$o5 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Only 'Docs\Priorite_X.md' -Message 'Docs: priorite' | Out-String
$c5 = $LASTEXITCODE
$f5 = @(Git $Repo show --name-only --no-renames --format= HEAD | ForEach-Object { "$_".Trim() } | Where-Object { $_ })
Check ($c5 -eq 0 -and (Git $Repo rev-list --count "$HeadP..HEAD" | Out-String).Trim() -eq '1' -and ($f5 -join '|') -eq 'Docs/Priorite_X.md') '-Only Docs\Priorite_X.md: commite exactement ce survey' "code=$c5 files=$($f5 -join ', ') out=$o5"
$HeadP2 = (Git $Repo rev-parse HEAD | Out-String).Trim()
$o6 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Only 'Docs/Autre.md' | Out-String
Check ($LASTEXITCODE -eq 3 -and (Git $Repo rev-parse HEAD | Out-String).Trim() -eq $HeadP2) '-Only Docs/Autre.md (hors Priorite_*): refuse code 3, aucun commit' $o6
Git $Repo checkout -q -- Docs/Autre.md Docs/Priorite_X.md | Out-Null   # l'etat du depot jetable sert encore aux cas session_gate ci-dessous

# --- Pas un depot git
$NoGit = Join-Path $Tmp 'plain'
New-Item -ItemType Directory -Path $NoGit | Out-Null
$out4 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $NoGit | Out-String
Check ($LASTEXITCODE -ne 0) 'Root hors depot git: code non nul' $out4

# --- session_gate -Phase Merge : main n'a que des docs (WARN) ou du code modifie (FAIL). RESTE de L9-5 : tant que session_gate n'a pas -MainRoot, ces 3 cas sont sautes.
if (-not (Select-String -Path $Gate -Pattern 'MainRoot' -Quiet)) {
  Write-Output 'SKIP session_gate -Phase Merge (pas encore de -MainRoot : reste de L9-5)'
  Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
  Write-Output ("commit_docs_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
  exit $(if ($Fails -eq 0) { 0 } else { 1 })
}
$MainDocs = Join-Path $Tmp 'maindocs'
Copy-Item $Repo $MainDocs -Recurse
Git $MainDocs checkout -q -- Source/x.cpp | Out-Null
Git $MainDocs reset -q | Out-Null
Git $MainDocs checkout -q -- other.md | Out-Null
Git $MainDocs checkout -q -- Docs/Archive/OldInProgress.md | Out-Null
Put $MainDocs 'EtapeDefini.md' 'v9'
$g1 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Merge -End 23:59 -MainRoot $MainDocs | Out-String
Check ($g1 -match '(?m)^WARN .*commit_docs' -and $g1 -notmatch '(?m)^FAIL .*copie principale') 'Merge: main avec seulement des docs modifies -> WARN (lancer commit_docs.ps1), pas de FAIL' $g1
$g2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Merge -End 23:59 -MainRoot $Repo | Out-String
Check ($g2 -match '(?m)^FAIL .*copie principale.*x\.cpp') 'Merge: main avec du CODE modifie -> FAIL qui nomme le fichier' $g2
Git $MainDocs checkout -q -- EtapeDefini.md | Out-Null
$g3 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Merge -End 23:59 -MainRoot $MainDocs | Out-String
Check ($g3 -match '(?m)^PASS .*copie principale' -and $g3 -notmatch '(?m)^(FAIL|WARN) .*copie principale') 'Merge: main propre -> PASS' $g3

Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output ("commit_docs_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $(if ($Fails -eq 0) { 0 } else { 1 })
