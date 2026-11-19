# known_reds_tests_part_test.ps1 : rouges connus de la partie "tests" de quick_check (L8-5, SAURON-1116n, 2026-11-16).
# Avant : Find-KnownRed comparait known_reds.txt au NOM DE LA PARTIE ("tests 1326/1328"), jamais aux tests en echec ; les
# 2 rouges connus D831 (Match.StartingHero.NomadFirstForum / NomadReplicated, listes dans known_reds.txt) sortaient en REGRESSION.
# Voulu : un echec de la partie tests dont TOUS les tests KO sont dans known_reds.txt (et aucune autre verification KO) = rouge
# connu ; un seul test KO hors liste, ou une autre verification KO (scripts non compiles...) = REGRESSION.
# Aucun C++, aucun editeur : la VRAIE fonction Report de quick_check.ps1 (analyse syntaxique) et le vrai KnownReds.ps1, sur une
# liste factice (les 2 noms Nomad), plus l'appel reel "Report tests ..." de quick_check.ps1 qui doit lui passer $Failures.
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\known_reds_tests_part_test.ps1   (code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
$Quick = Join-Path $Here 'quick_check.ps1'
. (Join-Path $Here 'KnownReds.ps1')
$First = 'HeroesOfLiberty.Match.StartingHero.NomadFirstForum'
$Second = 'HeroesOfLiberty.Match.StartingHero.NomadReplicated'
$FixtureList = Join-Path ([IO.Path]::GetTempPath()) ('known_reds_tests_part-' + [guid]::NewGuid().ToString('N').Substring(0, 8) + '.txt')
Set-Content -Path $FixtureList -Encoding ASCII -Value @('# test | depuis | cause', "$First | 2026-11-16 | D831 forum en case E", "$Second | 2026-11-16 | D831 ForumCell en dur")
$Fixture = @(Get-KnownReds $FixtureList)
Remove-Item $FixtureList -ErrorAction SilentlyContinue
Check ($Fixture.Count -eq 2) 'liste factice : les 2 tests Nomad lus'

# L'appel reel de la partie tests dans quick_check.ps1 passe la liste des tests en echec a Report.
$QuickText = [IO.File]::ReadAllText($Quick)
Check ($QuickText -match 'Report "tests \$\(\$Results\.Count - \$Failures\.Count\)/\$\(\$Results\.Count\)" \$Checks \$Failures\)') 'quick_check.ps1 : Report "tests N/M" $Checks $Failures'

$ast = [System.Management.Automation.Language.Parser]::ParseFile($Quick, [ref]$null, [ref]$null)
$fn = $ast.FindAll({ param($a) $a -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $a.Name -eq 'Report' }, $true) | Select-Object -First 1
Check ($null -ne $fn) 'quick_check.ps1 definit Report'
if ($fn) {
  . ([scriptblock]::Create($fn.Extent.Text))
  function TestsChecks([bool]$Green, [bool]$Compiled = $true) {
    $c = [ordered]@{}
    $c['tests ran (1328)'] = $true
    $c['all green (failed: ...)'] = $Green
    $c['scripts compiled (no error)'] = $Compiled
    $c
  }
  # 1. les 2 Nomad seuls en echec -> rouge connu, aucune regression
  $SubCases = @{}; $KnownReds = @($Fixture); $FailedParts = @()
  $out = & { $null = Report 'tests 1326/1328' (TestsChecks $false) @($First, $Second) } 6>&1 | Out-String
  Check ($out -match '(?m)^tests 1326/1328 : FAIL \(rouge connu depuis 2026-11-16') 'les 2 Nomad seuls KO -> "rouge connu depuis 2026-11-16"' $out
  $sum = Get-RedSummary $FailedParts $KnownReds
  Check ($sum -match 'aucune regression') 'les 2 Nomad seuls KO -> red summary "aucune regression"' $sum
  # 2. un test hors liste en plus -> REGRESSION
  $SubCases = @{}; $KnownReds = @($Fixture); $FailedParts = @()
  $out = & { $null = Report 'tests 1325/1328' (TestsChecks $false) @($First, 'HeroesOfLiberty.Fake.Other', $Second) } 6>&1 | Out-String
  Check ($out -match '(?m)^tests 1325/1328 : FAIL \(REGRESSION') 'un test KO hors liste -> REGRESSION' $out
  $sum = Get-RedSummary $FailedParts $KnownReds
  Check ($sum -match '1 regression') 'un test KO hors liste -> red summary "1 regression"' $sum
  # 3. les 2 Nomad, mais les scripts ne compilent pas -> REGRESSION
  $SubCases = @{}; $KnownReds = @($Fixture); $FailedParts = @()
  $out = & { $null = Report 'tests 1326/1328' (TestsChecks $false $false) @($First, $Second) } 6>&1 | Out-String
  Check ($out -match '(?m)^tests 1326/1328 : FAIL \(REGRESSION') 'les 2 Nomad + scripts non compiles -> REGRESSION' $out
  # 4. tout vert -> PASS sans etiquette
  $SubCases = @{}; $KnownReds = @($Fixture); $FailedParts = @()
  $out = & { $null = Report 'tests 1328/1328' (TestsChecks $true) @() } 6>&1 | Out-String
  Check ($out -match '(?m)^tests 1328/1328 : PASS\s*$') 'tout vert -> PASS sans etiquette' $out
  # 5. une seule des deux lignes dans la liste -> l'autre est une REGRESSION
  $SubCases = @{}; $KnownReds = @($Fixture | Select-Object -First 1); $FailedParts = @()
  $out = & { $null = Report 'tests 1326/1328' (TestsChecks $false) @($First, $Second) } 6>&1 | Out-String
  Check ($out -match '(?m)^tests 1326/1328 : FAIL \(REGRESSION') 'NomadReplicated retire de la liste -> REGRESSION' $out
  # --- SAURON-1119cn : un test SCRIPT (Angelscript) liste, en echec par ASSERTION. Le journal ecrit sa ligne
  # "Angelscript: Error: Assertion failed (...)" ; Get-ScriptErrors (vraie fonction de quick_check.ps1) la prenait pour une erreur de
  # compilation -> 2 KO ("all green" + "scripts compiled") -> REGRESSION alors que le test etait dans known_reds.txt.
  $GetErr = $ast.FindAll({ param($a) $a -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $a.Name -eq 'Get-ScriptErrors' }, $true) | Select-Object -First 1
  Check ($null -ne $GetErr) 'quick_check.ps1 definit Get-ScriptErrors'
  if ($GetErr) {
    . ([scriptblock]::Create($GetErr.Extent.Text))
    $ScriptTest = 'Angelscript.UnitTests.Tests.HOLCommandGridSweeps_Test.Test_CGSWResearchRowFits'
    $ScriptList = [pscustomobject]@{ Test = $ScriptTest; Since = '2026-11-19'; Cause = 'D856 Forge' }
    $AssertLog = Join-Path ([IO.Path]::GetTempPath()) ('known_reds_assert-' + [guid]::NewGuid().ToString('N').Substring(0, 8) + '.log')
    $CompileLog = Join-Path ([IO.Path]::GetTempPath()) ('known_reds_compile-' + [guid]::NewGuid().ToString('N').Substring(0, 8) + '.log')
    Set-Content -Path $AssertLog -Encoding ASCII -Value @(
      'LogAutomationController: Test Started. Name={Test_CGSWResearchRowFits} Path={' + $ScriptTest + '}',
      '[2026.11.19-18.20.01:123][  0]LogScript: Angelscript: Error: Assertion failed (Expected true, got false)',
      '[2026.11.19-18.20.01:124][  0]LogAutomationController: Test Completed. Result={Fail} Name={Test_CGSWResearchRowFits} Path={' + $ScriptTest + '}')
    Set-Content -Path $CompileLog -Encoding ASCII -Value @(
      '[2026.11.19-18.20.01:100][  0]LogScript: Angelscript: Error: Script/Command/HOLX.as:',
      '[2026.11.19-18.20.01:100][  0]LogScript: Angelscript: Error:  (12:5): Function Y is not BlueprintCallable or ScriptCallable and cannot be called from here',
      '[2026.11.19-18.20.01:101][  0]LogScript: Angelscript: Error: Hot reload failed, script compile errors')
    $AssertErrors = @(Get-ScriptErrors $AssertLog)
    $CompileErrors = @(Get-ScriptErrors $CompileLog)
    Remove-Item $AssertLog, $CompileLog -ErrorAction SilentlyContinue
    Check ($AssertErrors.Count -eq 0) 'Get-ScriptErrors : "Angelscript: Error: Assertion failed (...)" n est PAS une erreur de compilation' ($AssertErrors -join ' | ')
    Check ($CompileErrors.Count -ge 1 -and ($CompileErrors -join ' ') -match 'not BlueprintCallable or ScriptCallable') 'Get-ScriptErrors : une vraie erreur de compilation (not BlueprintCallable or ScriptCallable) reste detectee' ($CompileErrors -join ' | ')
    # 6. test script liliste seul KO par assertion (les 4 lignes reelles du 2026-11-19 18:20) -> rouge connu, aucune regression
    $SubCases = @{}; $KnownReds = @($ScriptList); $FailedParts = @()
    $Compiled = @($AssertErrors).Count -eq 0
    $c = [ordered]@{}; $c["all green (failed: $ScriptTest)"] = $false; $c["scripts compiled ($(if ($AssertErrors.Count) { $AssertErrors -join ' ' } else { 'no error' }))"] = $Compiled
    $out = & { $null = Report 'tests 1786/1787' $c @($ScriptTest) } 6>&1 | Out-String
    Check ($out -match '(?m)^tests 1786/1787 : FAIL \(rouge connu depuis 2026-11-19') 'test script liste KO par assertion -> "rouge connu"' $out
    $sum = Get-RedSummary $FailedParts $KnownReds
    Check ($sum -match 'aucune regression') 'test script liste KO par assertion -> red summary "aucune regression"' $sum
    # 7. test script KO par assertion NON liste -> REGRESSION
    $SubCases = @{}; $KnownReds = @(); $FailedParts = @()
    $out = & { $null = Report 'tests 1786/1787' $c @($ScriptTest) } 6>&1 | Out-String
    Check ($out -match '(?m)^tests 1786/1787 : FAIL \(REGRESSION') 'test script KO par assertion non liste -> REGRESSION' $out
    # 8. un test script liste + un test non liste -> REGRESSION
    $SubCases = @{}; $KnownReds = @($ScriptList); $FailedParts = @()
    $out = & { $null = Report 'tests 1785/1787' $c @($ScriptTest, 'Angelscript.UnitTests.Tests.Other_Test.Test_X') } 6>&1 | Out-String
    Check ($out -match '(?m)^tests 1785/1787 : FAIL \(REGRESSION') 'test script liste + test non liste -> REGRESSION' $out
    # 9. test script liste, mais une vraie erreur de compilation dans le journal (tests 0/0 ou chute) -> REGRESSION
    $SubCases = @{}; $KnownReds = @($ScriptList); $FailedParts = @()
    $c2 = [ordered]@{}; $c2["all green (failed: $ScriptTest)"] = $false; $c2["scripts compiled ($($CompileErrors -join ' '))"] = @($CompileErrors).Count -eq 0
    $out = & { $null = Report 'tests 1786/1787' $c2 @($ScriptTest) } 6>&1 | Out-String
    Check ($out -match '(?m)^tests 1786/1787 : FAIL \(REGRESSION') 'test script liste + vraie erreur de compilation -> REGRESSION' $out
  }
}
if ($Fails -gt 0) { Write-Output "=> $Fails FAIL"; exit 1 } else { Write-Output '=> tout PASS'; exit 0 }
