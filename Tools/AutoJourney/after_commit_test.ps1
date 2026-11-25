# after_commit_test.ps1 (TOOLS-1125a) : test de after_commit.ps1. Aucun build, aucun editeur : la fonction de resume est pure (Get-AfterSummary),
# et le parcours complet se joue avec un faux build.ps1 et un faux quick_check.ps1 dans un dossier temporaire.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (PASS/FAIL par cas, code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'after_commit.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
. $Tool

# --- fonction de resume pure ---
$okTests = @('Automation : running', 'tests 120/120 : PASS', 'quick_check : PASS in 90 s')
$r = Get-AfterSummary -BuildExit 0 -BuildSeconds 42 -BuildLines @('build : PASS') -TestExit 0 -TestLines $okTests
Check ($r.Ok -and $r.Line -eq 'PASS build 42 s, tests 120/120') 'resume : build et tests verts' $r.Line

$buildErr = @('build : LaunchCheck debut', 'Compiling Foo.cpp', 'C:\x\Foo.cpp(12): error C2065: ''Bar'': undeclared identifier', 'Foo.cpp(30): error C2143: syntax', 'build : FAIL (compile)')
$r = Get-AfterSummary -BuildExit 1 -BuildSeconds 10 -BuildLines $buildErr -TestExit $null -TestLines @()
Check ((-not $r.Ok) -and $r.Line -eq 'FAIL build : C:\x\Foo.cpp(12): error C2065: ''Bar'': undeclared identifier') 'resume : build rouge = premiere erreur (pas la derniere)' $r.Line

$r = Get-AfterSummary -BuildExit 2 -BuildSeconds 1 -BuildLines @('build : FAIL (this copy is busy: an editor holds its DLLs)') -TestExit $null -TestLines @()
Check ((-not $r.Ok) -and $r.Line -match '^FAIL build : .*busy') 'resume : build sans ligne error = ligne FAIL de build.ps1' $r.Line

$r = Get-AfterSummary -BuildExit 1 -BuildSeconds 1 -BuildLines @() -TestExit $null -TestLines @()
Check ((-not $r.Ok) -and $r.Line -eq 'FAIL build : no output from build.ps1 (exit 1)') 'resume : build muet' $r.Line

$redTests = @('tests 118/120 : FAIL (REGRESSION : pas dans known_reds.txt)', '  [KO] every test started completed (118 of 120)', '  [KO] all green (failed: Angelscript.UnitTests.Combat.Hit, Angelscript.UnitTests.Gather.Load)', 'quick_check : FAIL in 80 s')
$r = Get-AfterSummary -BuildExit 0 -BuildSeconds 40 -BuildLines @() -TestExit 1 -TestLines $redTests
Check ((-not $r.Ok) -and $r.Line -eq 'FAIL tests 118/120 : Angelscript.UnitTests.Combat.Hit') 'resume : test rouge = premier test en echec' $r.Line

$r = Get-AfterSummary -BuildExit 0 -BuildSeconds 40 -BuildLines @() -TestExit 1 -TestLines @('tests 0/0 : FAIL', '  [KO] tests ran (0)', '  [KO] scripts compiled (Foo.as(3) : ERR : unexpected)')
Check ((-not $r.Ok) -and $r.Line -eq 'FAIL tests 0/0 : tests ran (0)') 'resume : sans test lance = premier [KO]' $r.Line

$r = Get-AfterSummary -BuildExit 0 -BuildSeconds 40 -BuildLines @() -TestExit 1 -TestLines @('quick_check : FAIL (check_placeholders: a provisional type has no line in Placeholders.txt, see above)')
Check ((-not $r.Ok) -and $r.Line -match '^FAIL tests 0/0 : quick_check : FAIL \(check_placeholders') 'resume : quick_check sorti avant les tests = sa ligne FAIL' $r.Line

$r = Get-AfterSummary -BuildExit 0 -BuildSeconds 40 -BuildLines @() -TestExit 1 -TestLines @('tests 120/120 : PASS', 'quick_check : FAIL')
Check ((-not $r.Ok) -and $r.Line -match '^FAIL tests 120/120 : ') 'resume : code de sortie non nul malgre tests verts = FAIL' $r.Line

$r = Get-AfterSummary -BuildExit 0 -BuildSeconds 40 -BuildLines @() -TestExit 0 -TestLines @('tests 118/120 : PASS')
Check ((-not $r.Ok) -and $r.Line -match '^FAIL tests 118/120 : ') 'resume : N different de M = FAIL meme si la ligne dit PASS' $r.Line

# --- parcours complet avec faux build et faux quick_check ---
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('aftercommit-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Wt = Join-Path $Tmp 'HeroesOfLiberty-Fake'
$Out = Join-Path $Tmp 'Rapports'
New-Item -ItemType Directory -Path (Join-Path $Wt 'Tools\Tests') -Force | Out-Null
$FakeBuild = Join-Path $Tmp 'fake_build.ps1'
$FakeQc = Join-Path $Tmp 'fake_qc.ps1'
$Marker = Join-Path $Tmp 'qc_ran.txt'
[IO.File]::WriteAllText($FakeBuild, "Write-Output ('build args: ' + (`$args -join ' '))`r`nWrite-Output 'build : PASS'`r`nexit 0`r`n")
[IO.File]::WriteAllText($FakeQc, "Set-Content -Path '$Marker' -Value (`$args -join ' ')`r`nWrite-Output 'tests 5/5 : PASS'`r`nWrite-Output 'quick_check : PASS in 3 s'`r`nexit 0`r`n")
$lines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool $Wt -OutDir $Out -BuildScript $FakeBuild -QuickCheck $FakeQc)
$code = $LASTEXITCODE
Check ($code -eq 0 -and $lines.Count -eq 1 -and $lines[0] -match '^PASS build \d+ s, tests 5/5$') 'parcours : PASS, une ligne, code 0' "code=$code out=$($lines -join '|')"
$files = @(Get-ChildItem -LiteralPath $Out -Filter 'AFTER-Fake-*.txt' -ErrorAction SilentlyContinue)
Check ($files.Count -eq 1 -and $files[0].Name -match '^AFTER-Fake-\d{4}\.txt$') 'parcours : sortie complete dans AFTER-<worktree>-<HHmm>.txt' ($files.Name -join ',')
$content = if ($files.Count) { [IO.File]::ReadAllText($files[0].FullName) } else { '' }
Check ($content -match 'build : PASS' -and $content -match 'quick_check : PASS in 3 s' -and $content -match 'PASS build') 'parcours : le fichier a la sortie du build, de quick_check et le resume' $content
Check ($content -match '-LiveCoding=0' -and $content -match '-Wait') 'parcours : build lance avec -LiveCoding=0 -Wait' $content
Check ((Test-Path $Marker) -and ([IO.File]::ReadAllText($Marker) -match 'Parts tests')) 'parcours : quick_check lance avec -Parts tests' ''

# build rouge : quick_check ne doit pas etre lance
Remove-Item $Marker -Force -ErrorAction SilentlyContinue
[IO.File]::WriteAllText($FakeBuild, "Write-Output 'x.cpp(1): error C1: boom'`r`nWrite-Output 'build : FAIL (compile)'`r`nexit 1`r`n")
$lines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool $Wt -OutDir $Out -BuildScript $FakeBuild -QuickCheck $FakeQc)
$code = $LASTEXITCODE
Check ($code -eq 1 -and $lines.Count -eq 1 -and $lines[0] -eq 'FAIL build : x.cpp(1): error C1: boom') 'parcours : build rouge = FAIL build, code 1' "code=$code out=$($lines -join '|')"
Check (-not (Test-Path $Marker)) 'parcours : build rouge = quick_check pas lance'

# tests rouges
[IO.File]::WriteAllText($FakeBuild, "Write-Output 'build : PASS'`r`nexit 0`r`n")
[IO.File]::WriteAllText($FakeQc, "Write-Output 'tests 4/5 : FAIL'`r`nWrite-Output '  [KO] all green (failed: A.B.C)'`r`nexit 1`r`n")
$lines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool $Wt -OutDir $Out -BuildScript $FakeBuild -QuickCheck $FakeQc)
$code = $LASTEXITCODE
Check ($code -eq 1 -and $lines.Count -eq 1 -and $lines[0] -eq 'FAIL tests 4/5 : A.B.C') 'parcours : tests rouges = FAIL tests N/M, code 1' "code=$code out=$($lines -join '|')"

# worktree absent
$lines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool (Join-Path $Tmp 'absent') -OutDir $Out -BuildScript $FakeBuild -QuickCheck $FakeQc)
Check ($LASTEXITCODE -eq 1 -and $lines.Count -eq 1 -and $lines[0] -match '^FAIL build : worktree introuvable') 'worktree absent : FAIL build, code 1' ($lines -join '|')

Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output ("after_commit_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $(if ($Fails -eq 0) { 0 } else { 1 })
