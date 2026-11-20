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
Git $Repo add -A | Out-Null
Git $Repo commit -q -m init | Out-Null
foreach ($B in 'red-as', 'red-cpp', 'g1', 'g2', 'gcpp') { Git $Repo branch $B main | Out-Null }
$RedAs = CommitOn $Repo 'red-as' 'Script/Tests/t_test.as' 't'
$RedCpp = CommitOn $Repo 'red-cpp' 'Source/t.cpp' 't'
CommitOn $Repo 'g1' 'Script/g1.as' 'g1' | Out-Null
CommitOn $Repo 'g2' 'Script/g2.as' 'g2' | Out-Null
CommitOn $Repo 'gcpp' 'Source/g.h' 'g' | Out-Null
Git $Repo checkout -q main | Out-Null
$Base = @('-Root', $Repo, '-Branch', 'integ-x', '-DryRun')

# 1. Script only, one red, two greens given as ONE comma list (the -File trap): every step in order, no build.
$P = Run-Plan ($Base + @('-Reds', $RedAs, '-Greens', 'g1,g2'))
$K = Kinds $P
Check ($P.Code -eq 0) 'script only: exit 0' "code=$($P.Code) out=$($P.Out -join ' | ')"
Check (($K -join ',') -eq 'branch,red,build,tests,reset,green,green,build,tests,parts,summary') 'script only: order of the steps (reset to main between the red tests and the greens)' ($K -join ',')
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

Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
Write-Output "integrate_test : $(if ($Fails) { "$Fails FAIL" } else { 'PASS' })"
if ($Fails) { exit 1 } else { exit 0 }
