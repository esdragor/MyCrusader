# integrate_test.ps1 : test d'effet de integrate.ps1 (L19-9), sur le plan imprime par -DryRun. Aucun build, aucun editeur, aucun quick_check ;
# travaille sur un VRAI depot git jetable dans %TEMP% (main + une branche rouge + des branches vertes, avec ou sans C++).
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (une ligne PASS/FAIL par cas, code 1 s'il y a un FAIL)
# Piege du matin (2026-11-20) : `powershell -File` passe une liste en UN SEUL argument ("a,b") ; l'outil doit la couper aux virgules.
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'integrate.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
function Git { param([string]$Repo) & git.exe -C $Repo @args 2>&1 }   # git.exe : a bare "git" would call this function again
function Put([string]$Repo, [string]$Rel, [string]$Text) {
  $p = Join-Path $Repo $Rel
  $d = Split-Path $p -Parent
  if (-not (Test-Path $d)) { New-Item -ItemType Directory -Path $d -Force | Out-Null }
  [IO.File]::WriteAllText($p, $Text)
}
function CommitOn([string]$Repo, [string]$Branch, [string]$Rel, [string]$Text) {
  Git $Repo checkout -q $Branch | Out-Null
  Put $Repo $Rel $Text
  Git $Repo add $Rel | Out-Null
  Git $Repo commit -q -m "change $Rel" | Out-Null
  return (Git $Repo rev-parse HEAD | Out-String).Trim()
}
# The plan lines: "PLAN <n> <kind> ..." ; returns @{ Code; Lines }.
function Run-Plan([string[]]$ToolArgs) {
  $Out = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool @ToolArgs 2>&1 | ForEach-Object { "$_" })
  return @{ Code = $LASTEXITCODE; Out = $Out; Lines = @($Out | Where-Object { $_ -match '^PLAN \d+ ' }) }
}
function Kinds($Plan) { return @($Plan.Lines | ForEach-Object { if ($_ -match '^PLAN \d+ (\S+)') { $Matches[1] } }) }

Check (Test-Path $Tool) 'integrate.ps1 exists'
if (-not (Test-Path $Tool)) { Write-Output "integrate_test : $Fails FAIL"; exit 1 }

$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('integrate-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Repo = Join-Path $Tmp 'wt'
New-Item -ItemType Directory -Path $Repo -Force | Out-Null
Git $Repo init -q -b main | Out-Null
Git $Repo config user.name 'Test' | Out-Null
Git $Repo config user.email 'test@example.com' | Out-Null
Git $Repo config core.autocrlf false | Out-Null
Put $Repo 'Source/a.cpp' 'int a;'
Put $Repo 'Script/a.as' 'a'
Put $Repo 'Tools/Data/UnitTypes.csv' 'id,hp'
Put $Repo 'Content/Units/UnitTypes.uasset' 'unit-v1'
Put $Repo 'Tools/Data/BuildingTypes.csv' 'id,hp'
Put $Repo 'Content/Buildings/BuildingTypes.uasset' 'bld-v1'
Git $Repo add -A | Out-Null
Git $Repo commit -q -m init | Out-Null
foreach ($B in 'red-as', 'red-cpp', 'g1', 'g2', 'gcpp') { Git $Repo branch $B main | Out-Null }
$RedAs = CommitOn $Repo 'red-as' 'Script/Tests/t_test.as' 't'
$RedCpp = CommitOn $Repo 'red-cpp' 'Source/t.cpp' 't'
CommitOn $Repo 'g1' 'Script/g1.as' 'g1' | Out-Null
CommitOn $Repo 'g2' 'Script/g2.as' 'g2' | Out-Null
CommitOn $Repo 'gcpp' 'Source/g.h' 'g' | Out-Null
foreach ($B in 'gcsv', 'gboth') { Git $Repo branch $B main | Out-Null }
CommitOn $Repo 'gcsv' 'Tools/Data/BuildingTypes.csv' "id,hp`nnew,1" | Out-Null
CommitOn $Repo 'gboth' 'Tools/Data/UnitTypes.csv' "id,hp`nnew,1" | Out-Null
CommitOn $Repo 'gboth' 'Content/Units/UnitTypes.uasset' 'unit-v2' | Out-Null
Git $Repo checkout -q main | Out-Null
$Base = @('-Root', $Repo, '-Branch', 'integ-x', '-DryRun')

# 1. Script only, one red, two greens given as ONE comma list (the -File trap): every step in order, no build.
$P = Run-Plan ($Base + @('-Reds', $RedAs, '-Greens', 'g1,g2'))
$K = Kinds $P
Check ($P.Code -eq 0) 'script only: exit 0' "code=$($P.Code) out=$($P.Out -join ' | ')"
Check (($K -join ',') -eq 'branch,red,build,tests,reset,green,green,regen,build,tests,parts,summary') 'script only: order of the steps (reset to main between the red tests and the greens)' ($K -join ',')
Check (@($P.Lines | Where-Object { $_ -match '^PLAN \d+ reset .*reset --hard [0-9a-f]{40}' }).Count -eq 1) 'plan shows the reset to the sha of main' ($P.Lines -join ' | ')
$GreenLines = @($P.Lines | Where-Object { $_ -match '^PLAN \d+ green ' })
Check ($GreenLines.Count -eq 2) 'list split at the commas: 2 green steps, not one name "g1,g2"' "count=$($GreenLines.Count)"
Check (($GreenLines[0] -match 'g1') -and ($GreenLines[1] -match 'g2') -and -not ($GreenLines -match 'g1,g2')) 'greens merged one by one in the order given' ($GreenLines -join ' | ')
$BuildLines = @($P.Lines | Where-Object { $_ -match '^PLAN \d+ build ' })
Check (($BuildLines.Count -eq 2) -and (@($BuildLines | Where-Object { $_ -match 'SKIP' }).Count -eq 2)) 'script only: both builds SKIP (no .cpp / .h changed)' ($BuildLines -join ' | ')
Check (@($P.Lines | Where-Object { $_ -match 'quick_check\.ps1 .*-Parts tests' }).Count -eq 2) 'tests run twice (after the reds, after the greens) with -Parts tests'
Check (@($P.Lines | Where-Object { $_ -match '^PLAN \d+ parts .*-Parts net,build' }).Count -eq 1) 'default final parts = net,build'
Check (@($P.Lines | Where-Object { $_ -match '^PLAN \d+ branch .*switch -c integ-x main' }).Count -eq 1) 'branch created from main'
Check (@($P.Lines | Where-Object { $_ -match "^PLAN \d+ red .*cherry-pick $RedAs" }).Count -eq 1) 'red = cherry-pick of its sha'
Check (@($P.Lines | Where-Object { $_ -match '^PLAN \d+ summary .*Docs\\Integrate_integ-x\.txt' }).Count -eq 1) 'summary goes to Docs\Integrate_<Branch>.txt'
Git $Repo rev-parse --verify -q 'integ-x' | Out-Null
Check ($LASTEXITCODE -ne 0) '-DryRun creates no branch'
Check (-not (Test-Path (Join-Path $Repo 'Docs\Integrate_integ-x.txt'))) '-DryRun writes no summary'

# 2. A green with C++ (.h): the second build is real, with the merge priority; the first (script red) stays skipped.
$P = Run-Plan ($Base + @('-Reds', $RedAs, '-Greens', 'g1,gcpp', '-Parts', 'tests,net'))
$BuildLines = @($P.Lines | Where-Object { $_ -match '^PLAN \d+ build ' })
Check (($BuildLines.Count -eq 2) -and ($BuildLines[0] -match 'SKIP') -and ($BuildLines[1] -notmatch 'SKIP') -and ($BuildLines[1] -match 'build\.ps1 .*-LiveCoding=0 .*-Wait .*-Priority merge')) 'C++ in a green: only the second build runs (-LiveCoding=0 -Wait -Priority merge)' ($BuildLines -join ' | ')
Check (@($P.Lines | Where-Object { $_ -match '^PLAN \d+ parts .*-Parts tests,net' }).Count -eq 1) '-Parts given as one comma string is kept whole for quick_check'

# 3. A red with C++ (.cpp): the first build is real.
$P = Run-Plan ($Base + @('-Reds', "$RedCpp,$RedAs", '-Greens', 'g1'))
$BuildLines = @($P.Lines | Where-Object { $_ -match '^PLAN \d+ build ' })
$RedLines = @($P.Lines | Where-Object { $_ -match '^PLAN \d+ red ' })
Check ($RedLines.Count -eq 2) 'two reds in one comma list: 2 red steps' "count=$($RedLines.Count)"
Check (($BuildLines.Count -eq 2) -and ($BuildLines[0] -notmatch 'SKIP') -and ($BuildLines[1] -match 'SKIP')) 'C++ in a red: first build runs, second skipped' ($BuildLines -join ' | ')

# 4. No reds: the red stage is skipped (no cherry-pick, no tests of the reds), the greens still merge.
$P = Run-Plan ($Base + @('-Greens', 'g1'))
Check ($P.Code -eq 0) 'no reds: exit 0' "out=$($P.Out -join ' | ')"
Check (@($P.Lines | Where-Object { $_ -match '^PLAN \d+ red ' -and $_ -notmatch 'SKIP' }).Count -eq 0) 'no reds: no cherry-pick step'
Check (@($P.Lines | Where-Object { $_ -match '^PLAN \d+ green ' -and $_ -match 'g1' }).Count -eq 1) 'no reds: the green is still merged'

# 5. An unknown ref stops the plan (exit 3) instead of printing a plan that cannot run.
$P = Run-Plan ($Base + @('-Reds', $RedAs, '-Greens', 'g1,nope'))
Check ($P.Code -eq 3) 'unknown green branch: exit 3' "code=$($P.Code)"
Check (($P.Out -join ' ') -match 'nope') 'unknown green branch: named in the message'

# 6. REAL run (git only, -SkipBuildTests) : a red holds a STUB that a later commit of its own branch replaces (case integ-32, 2026-11-20).
#    The reds only serve to see the tests fail; the greens must merge from main, without the cherry-picked reds -> PASS, no conflict.
Git $Repo checkout -q main | Out-Null
Git $Repo branch stubbr main | Out-Null
$Stub1 = CommitOn $Repo 'stubbr' 'Script/stub.as' "line1`nSTUB`nline3`n"
CommitOn $Repo 'stubbr' 'Script/stub.as' "line1`nREAL`nline3`n" | Out-Null
Git $Repo checkout -q main | Out-Null
$MainSha = (Git $Repo rev-parse main | Out-String).Trim()
$Real = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Branch integ-real -Reds $Stub1 -Greens stubbr -SkipBuildTests 2>&1 | ForEach-Object { "$_" })
$RealCode = $LASTEXITCODE
Check ($RealCode -eq 0) 'stub red then its own branch: no conflict, exit 0 (was 2)' "code=$RealCode out=$($Real -join ' | ')"
Check (($Real -join ' ') -match 'INTEGRATE integ-real : PASS') 'stub red then its own branch: summary says PASS' ($Real -join ' | ')
Check (($Real -join ' ') -match 'verts depuis main, rouges retires') 'summary says "verts depuis main, rouges retires"' ($Real -join ' | ')
$Content = (Git $Repo show 'integ-real:Script/stub.as' | Out-String)
Check (($Content -match 'REAL') -and ($Content -notmatch 'STUB')) 'integration branch carries the real code, not the stub' $Content
$Base0 = (Git $Repo merge-base main integ-real | Out-String).Trim()
Check ($Base0 -eq $MainSha) 'integration branch still descends from the main it started from'

# 7. A count of 0/0 (or no count at all) from quick_check -Parts tests is a FAIL, never a PASS (integ-50, 2026-11-22: a script did not compile,
#    the tests of the reds gave 0/0, the tool wrote "FAIL attendu 0/0" + "rouges vus : aucun" and concluded PASS).
#    quick_check is SIMULATED by an untracked fake Tools\Tests\quick_check.ps1 in the temp repo (the run is real, git included, no -SkipBuildTests):
#    each "-Parts tests" call n prints $QcDir\tests<n>.txt (line 1 = exit code, the rest = output); any other -Parts prints "parts ok" and exits 0.
$QcDir = Join-Path $Tmp 'qc'
New-Item -ItemType Directory -Path $QcDir -Force | Out-Null
Put $Repo 'Tools/Tests/quick_check.ps1' ("param([string]`$Parts = '')`r`n`$d = `$env:HOL_FAKE_QC_DIR`r`nif (`$Parts -ne 'tests') { 'parts ok'; exit 0 }`r`n`$nf = Join-Path `$d 'n.txt'`r`n`$n = 1 + [int](@(Get-Content `$nf -ErrorAction SilentlyContinue)[0])`r`nSet-Content -Path `$nf -Value `$n`r`n`$f = Join-Path `$d ('tests' + `$n + '.txt')`r`n`$lines = @(Get-Content `$f)`r`n`$lines | Select-Object -Skip 1`r`nexit ([int]`$lines[0])`r`n")
$KoLine1 = "  [KO] scripts compiled (C:/x/Script/Fake/Broken_Test.as: (12:5): No matching signatures to 'Foo()')"
$KoLine2 = 'red summary : 1 regression(s) : tests 0/0 ; 0 rouge(s) connu(s)'
$script:CaseNo = 0
function Run-Fake([string]$Reds, [string]$Greens, [string[]]$TestsFiles) {
  $script:CaseNo++
  Remove-Item (Join-Path $QcDir '*') -Force -ErrorAction SilentlyContinue
  for ($i = 0; $i -lt $TestsFiles.Count; $i++) { [IO.File]::WriteAllText((Join-Path $QcDir ('tests' + ($i + 1) + '.txt')), $TestsFiles[$i]) }
  Git $Repo checkout -q -f main | Out-Null
  $env:HOL_FAKE_QC_DIR = $QcDir
  $ToolArgs = @('-Root', $Repo, '-Branch', "integ-fake$($script:CaseNo)")
  if ($Reds) { $ToolArgs += @('-Reds', $Reds) }
  if ($Greens) { $ToolArgs += @('-Greens', $Greens) }
  $Out = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool @ToolArgs 2>&1 | ForEach-Object { "$_" })
  return @{ Code = $LASTEXITCODE; Text = ($Out -join "`n") }
}
$RedOk = "1`ntests 1/2 : FAIL`nall green (failed: Fake.Case)`n"
$GreenOk = "0`ntests 2/2 : PASS`n"

# 7a. Reds give 0/0 and a script KO: FAIL, exact lines, "rouges vus : aucun (0/0)", the greens are never run.
$R = Run-Fake $RedAs 'g1' @("1`ntests 0/0 : FAIL`n$KoLine1`n$KoLine2`n", $GreenOk)
Check ($R.Code -eq 1) '0/0 in the red tests: exit 1 (was a PASS)' "code=$($R.Code) out=$($R.Text)"
Check ($R.Text -match 'INTEGRATE integ-fake1 : FAIL') '0/0 in the red tests: summary says FAIL' $R.Text
Check ($R.Text -match 'rouges vus : aucun \(0/0\)') '0/0 in the red tests: "rouges vus : aucun (0/0)"' $R.Text
Check (($R.Text -match 'scripts compiled') -and ($R.Text -match 'Broken_Test\.as: \(12:5\)')) '0/0 in the red tests: the "scripts compiled" KO and the .as error line are in the summary' $R.Text
Check ($R.Text -match 'tests verts : NON LANCE') '0/0 in the red tests: the greens are not run' $R.Text

# 7b. Reds give no count at all (the tool crashed / printed nothing usable): same FAIL.
$R = Run-Fake $RedAs 'g1' @("1`nsomething went wrong`n", $GreenOk)
Check ($R.Code -eq 1) 'no count in the red tests: exit 1' "code=$($R.Code) out=$($R.Text)"
Check ($R.Text -match 'rouges vus : aucun \(0/0\)') 'no count in the red tests: "rouges vus : aucun (0/0)"' $R.Text

# 7c. Reds seen red (1/2, a failed name), greens 2/2: still a PASS, the red is named.
$R = Run-Fake $RedAs 'g1' @($RedOk, $GreenOk)
Check ($R.Code -eq 0) 'reds 1/2 then greens 2/2: exit 0 (rule untouched)' "code=$($R.Code) out=$($R.Text)"
Check ($R.Text -match 'rouges vus : Fake\.Case') 'reds 1/2: the red seen is named' $R.Text

# 7d. Greens give 0/0 (exit 0 or not): FAIL.
$R = Run-Fake $RedAs 'g1' @($RedOk, "0`ntests 0/0 : PASS`n$KoLine1`n$KoLine2`n")
Check ($R.Code -eq 1) '0/0 in the green tests: exit 1' "code=$($R.Code) out=$($R.Text)"
Check (($R.Text -match 'INTEGRATE integ-fake4 : FAIL') -and ($R.Text -match 'tests verts : FAIL') -and ($R.Text -match 'Broken_Test\.as: \(12:5\)')) '0/0 in the green tests: summary FAIL with the .as error line' $R.Text

# 7e. No reds (-Reds empty): the red stage is untouched (skipped), greens 2/2 -> PASS ; greens 0/0 -> FAIL.
$R = Run-Fake '' 'g1' @($GreenOk)
Check ($R.Code -eq 0) 'no reds, greens 2/2: exit 0' "code=$($R.Code) out=$($R.Text)"
Check (($R.Text -match 'tests rouges : SKIP \(pas de rouge\)') -and ($R.Text -notmatch 'rouges vus : aucun \(0/0\)')) 'no reds: tests rouges SKIP, no 0/0 verdict on the red stage' $R.Text
$R = Run-Fake '' 'g1' @("0`ntests 0/0 : PASS`n")
Check ($R.Code -eq 1) 'no reds, greens 0/0: exit 1' "code=$($R.Code) out=$($R.Text)"
Git $Repo checkout -q -f main | Out-Null

# 8. Regen of the tables (GALADRIEL-1123pa ; DIAG-1123a: integ-55 went red because BuildingTypes.uasset was not regenerated after a new line of BuildingTypes.csv).
#    Rule: after the merge of the greens, if a table CSV (Tools/Data/UnitTypes.csv, BuildingTypes.csv) changed vs main and its .uasset did not, integrate.ps1 runs
#    regen_tables.ps1 (here a FAKE script given by -RegenScript, never the editor), checks its exit code, commits the regenerated .uasset ("Regen tables after merge")
#    and the summary says "regen: lance" ; "regen: inutile" when nothing is needed ; "regen: FAIL" when the script fails.
$FakeRegen = Join-Path $Tmp 'fake_regen.ps1'
[IO.File]::WriteAllText($FakeRegen, "param([string]`$Root, [switch]`$DryRun)`r`nif (`$env:HOL_FAKE_REGEN_CODE -and `$env:HOL_FAKE_REGEN_CODE -ne '0') { 'fake regen fails'; exit ([int]`$env:HOL_FAKE_REGEN_CODE) }`r`n[IO.File]::WriteAllText((Join-Path `$Root 'Content\Buildings\BuildingTypes.uasset'), 'bld-regen')`r`nexit 0`r`n")
$RegenArgs = @('-RegenScript', $FakeRegen)
function Regen-Line($Plan) { return @($Plan.Lines | Where-Object { $_ -match '^PLAN \d+ regen ' }) }
$P = Run-Plan ($Base + @('-Greens', 'g1,gcsv') + $RegenArgs)
$L = @(Regen-Line $P)
Check (($L.Count -eq 1) -and ($L[0] -notmatch 'SKIP') -and ($L[0] -match 'fake_regen\.ps1')) 'a green changes BuildingTypes.csv without its .uasset: the plan has a real regen step' ($P.Lines -join ' | ')
$P = Run-Plan ($Base + @('-Greens', 'g1,g2') + $RegenArgs)
$L = @(Regen-Line $P)
Check (($L.Count -eq 1) -and ($L[0] -match 'SKIP')) 'no table CSV changed: the regen step is SKIP' ($P.Lines -join ' | ')
$P = Run-Plan ($Base + @('-Greens', 'gboth') + $RegenArgs)
$L = @(Regen-Line $P)
Check (($L.Count -eq 1) -and ($L[0] -match 'SKIP')) 'UnitTypes.csv AND UnitTypes.uasset changed in the greens: the regen step is SKIP (already regenerated)' ($P.Lines -join ' | ')
function Run-Regen([string]$Greens, [string]$Code) {
  $script:CaseNo++
  Git $Repo checkout -q -f main | Out-Null
  $env:HOL_FAKE_REGEN_CODE = $Code
  $Br = "integ-regen$($script:CaseNo)"
  $Out = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Branch $Br -Greens $Greens -SkipBuildTests @RegenArgs 2>&1 | ForEach-Object { "$_" })
  $Rc = $LASTEXITCODE
  $env:HOL_FAKE_REGEN_CODE = ''
  return @{ Code = $Rc; Text = ($Out -join "`n"); Branch = $Br }
}
$R = Run-Regen 'g1,gcsv' '0'
$Subject = (Git $Repo log -1 --format=%s $R.Branch | Out-String).Trim()
$Asset = (Git $Repo show "$($R.Branch):Content/Buildings/BuildingTypes.uasset" | Out-String).Trim()
Check ($R.Code -eq 0) 'real run, BuildingTypes.csv changed: exit 0' "code=$($R.Code) out=$($R.Text)"
Check ($R.Text -match 'regen: lance') 'real run: the summary says "regen: lance"' $R.Text
Check ($Subject -eq 'Regen tables after merge') 'real run: the last commit of the integration branch is "Regen tables after merge"' "subject=$Subject"
Check ($Asset -eq 'bld-regen') 'real run: the regenerated .uasset is committed on the integration branch' "asset=$Asset"
$R = Run-Regen 'g1' '0'
$Subject = (Git $Repo log -1 --format=%s $R.Branch | Out-String).Trim()
Check (($R.Code -eq 0) -and ($R.Text -match 'regen: inutile') -and ($Subject -ne 'Regen tables after merge')) 'real run, no CSV changed: "regen: inutile", no regen commit' "code=$($R.Code) subject=$Subject out=$($R.Text)"
$R = Run-Regen 'gboth' '0'
Check (($R.Code -eq 0) -and ($R.Text -match 'regen: inutile')) 'real run, CSV and .uasset both changed in the green: "regen: inutile"' "code=$($R.Code) out=$($R.Text)"
$R = Run-Regen 'gcsv' '2'
$Subject = (Git $Repo log -1 --format=%s $R.Branch | Out-String).Trim()
Check (($R.Code -eq 1) -and ($R.Text -match 'regen: FAIL') -and ($R.Text -match 'INTEGRATE integ-regen\d+ : FAIL') -and ($Subject -ne 'Regen tables after merge')) 'regen script exits 2: exit 1, "regen: FAIL", nothing committed' "code=$($R.Code) subject=$Subject out=$($R.Text)"
Git $Repo checkout -q -f main | Out-Null
Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
Write-Output "integrate_test : $(if ($Fails) { "$Fails FAIL" } else { 'PASS' })"
if ($Fails) { exit 1 } else { exit 0 }
