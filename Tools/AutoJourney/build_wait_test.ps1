# build_wait_test.ps1 : test of build_wait.ps1 (L15-6). No real build: Tools\build.ps1 is replaced by fake scripts
# (-BuildScript), plus one case that runs the REAL build.ps1 with a private queue and -FakeBuild.
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\build_wait_test.ps1
# Exit code 0 = green, 1 = red. Prints PASS / FAIL per check.
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$tool = Join-Path $here 'build_wait.ps1'
$realBuild = Join-Path (Split-Path -Parent $here) 'build.ps1'
$tmp = Join-Path $env:TEMP ('build_wait_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $tmp | Out-Null
$failures = 0
function Check($ok, $msg) { if ($ok) { Write-Output "  PASS $msg" } else { Write-Output "  FAIL $msg"; $script:failures++ } }

# A fake worktree (a folder is enough for the fake builds) and a fake build script per scenario.
function New-Wt($name) { $d = Join-Path $tmp $name; New-Item -ItemType Directory -Force (Join-Path $d 'Saved\Logs') | Out-Null; $d }
function New-FakeBuild($name, $body) { $f = Join-Path $tmp "$name.ps1"; Set-Content -Path $f -Value $body -Encoding ASCII; $f }
function Invoke-Tool([string[]]$ToolArgs) {
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $tool @ToolArgs 2>&1 | ForEach-Object { "$_" }
    [pscustomobject]@{ Code = $LASTEXITCODE; Out = ($out -join "`n") }
}
function Result-Path($wt) { Join-Path $wt 'Saved\Logs\build_wait_result.txt' }
function Result-Text($wt) { $p = Result-Path $wt; if (Test-Path $p) { [IO.File]::ReadAllText($p) } else { '' } }

# 1. PASS, and the build is asked with -Wait and -Root <worktree> (ONE blocking call, no polling by the agent)
Write-Output '[1] PASS : -Wait and -Root passed on, short result file'
$wt = New-Wt 'wt1'
$argsLog = Join-Path $tmp 'args1.txt'
$fb = New-FakeBuild 'fb_pass' ("Set-Content '$argsLog' (`$args -join ' '); Write-Host 'build : PASS (HeroesOfLibertyEditor Development, wt1, 7 s)'; exit 0")
$r = Invoke-Tool @($wt, '-BuildScript', $fb)
$txt = Result-Text $wt
Check ($r.Code -eq 0) "exit code 0 (got $($r.Code))"
Check ($txt -match '^PASS\b') "result file starts with PASS (got: $(($txt -split "`n")[0]))"
Check ($txt -match 'duration: \d+ s') 'result file gives the duration'
Check (($txt -split "`r?`n" | Where-Object { $_.Trim() }).Count -le 4) 'result file is short (4 lines at most)'
$logged = if (Test-Path $argsLog) { Get-Content $argsLog -Raw } else { '' }
Check ($logged -match '-Wait') 'build.ps1 got -Wait'
Check ($logged -match [regex]::Escape($wt)) 'build.ps1 got the worktree as -Root'
Check ($r.Out -match 'PASS') 'the result is also printed (one call, nothing else to read)'

# 2. FAIL : the LAST error line of the build is reported, exit 1, a stale PASS result is replaced
Write-Output '[2] FAIL : last error, exit 1, stale result replaced'
$wt = New-Wt 'wt2'
Set-Content (Result-Path $wt) 'PASS stale' -Encoding ASCII
$fb = New-FakeBuild 'fb_fail' "Write-Host 'build : FAIL (HeroesOfLibertyEditor Development, wt2, 31 s, exit 6; log x)'; Write-Host '  Foo.cpp(12): error C2065: first problem'; Write-Host '  Bar.cpp(40): error C1189: second problem'; exit 1"
$r = Invoke-Tool @($wt, '-BuildScript', $fb)
$txt = Result-Text $wt
Check ($r.Code -eq 1) "exit code 1 (got $($r.Code))"
Check ($txt -match '^FAIL\b') "result file starts with FAIL, stale PASS gone (got: $(($txt -split "`n")[0]))"
Check ($txt -match 'last error: .*C1189: second problem') 'last error is the LAST error line'
Check ($txt -notmatch 'C2065') 'earlier errors are left out (short result)'

# 3. Other exit codes of build.ps1 get a name
Write-Output '[3] exit codes 2..5 and unknown'
$map = @{ 2 = 'BUSY'; 3 = 'SUPERSEDED'; 4 = 'ORPHAN'; 5 = 'TIMEOUT'; 9 = 'FAIL' }
foreach ($c in 2, 3, 4, 5, 9) {
    $wt = New-Wt "wt3_$c"
    $fb = New-FakeBuild "fb_$c" "Write-Host 'build : something happened in the queue ($c)'; exit $c"
    $r = Invoke-Tool @($wt, '-BuildScript', $fb)
    $txt = Result-Text $wt
    Check ($r.Code -eq $c) "build exit $c is passed on (got $($r.Code))"
    Check ($txt -match ('^' + $map[$c] + '\b')) "build exit $c is named $($map[$c])"
}

# 4. The duration covers the whole call (queue wait included)
Write-Output '[4] duration measured'
$wt = New-Wt 'wt4'
$fb = New-FakeBuild 'fb_slow' "Start-Sleep -Seconds 3; Write-Host 'build : PASS (x, wt4, 3 s)'; exit 0"
$r = Invoke-Tool @($wt, '-BuildScript', $fb)
$txt = Result-Text $wt
$d = if ($txt -match 'duration: (\d+) s') { [int]$Matches[1] } else { -1 }
Check ($d -ge 3 -and $d -lt 30) "duration at least 3 s (got $d)"

# 5. -ResultFile : another place for the result
Write-Output '[5] -ResultFile'
$wt = New-Wt 'wt5'
$rf = Join-Path $tmp 'custom_result.txt'
$fb = New-FakeBuild 'fb_pass5' "Write-Host 'build : PASS (x, wt5, 1 s)'; exit 0"
$r = Invoke-Tool @($wt, '-BuildScript', $fb, '-ResultFile', $rf)
Check ((Test-Path $rf) -and ([IO.File]::ReadAllText($rf) -match '^PASS\b')) 'result written to -ResultFile'

# 6. A worktree that does not exist: FAIL, exit 1, nothing built
Write-Output '[6] missing worktree'
$r = Invoke-Tool @((Join-Path $tmp 'nope'), '-BuildScript', $fb)
Check ($r.Code -eq 1) "exit code 1 (got $($r.Code))"
Check ($r.Out -match 'FAIL' -and $r.Out -match 'nope') 'FAIL printed with the missing path'

# 7. The REAL build.ps1 (queue, ticket, CSV) with a private queue and a fake compiler : PASS, then FAIL
Write-Output '[7] real build.ps1 with a fake compiler'
$wt = New-Wt 'wt7'
Copy-Item (Join-Path (Split-Path -Parent (Split-Path -Parent $here)) 'HeroesOfLiberty.uproject') $wt
$q = Join-Path $tmp 'state7'; $csv = Join-Path $tmp 'q7.csv'
$okc = New-FakeBuild 'compiler_ok' "'[1/1] Compile Fake.cpp'; exit 0"
$badc = New-FakeBuild 'compiler_bad' "'[1/1] Compile Fake.cpp'; 'Fake.cpp(3): error C2143: syntax error from the fake compiler'; `$global:LASTEXITCODE = 6; exit 6"
$env:HOL_BUILD_POLL_SECONDS = '1'; $env:HOL_BUILD_MIN_FREE_VM_GB = '0'
$r = Invoke-Tool @($wt, '-QueueDir', $q, '-CsvPath', $csv, '-FakeBuild', $okc)
Check ($r.Code -eq 0 -and (Result-Text $wt) -match '^PASS\b') "real build.ps1 PASS through the tool (exit $($r.Code))"
$r = Invoke-Tool @($wt, '-QueueDir', $q, '-CsvPath', $csv, '-FakeBuild', $badc)
Check ($r.Code -eq 1 -and (Result-Text $wt) -match '^FAIL\b') "real build.ps1 FAIL through the tool (exit $($r.Code))"
Check ((Result-Text $wt) -match 'last error: .*error C2143') 'last error of the real FAIL is reported'

# 8. HALDIR-1127ar1: the REAL build.ps1 writes <Root>\Binaries\Win64\HOL_BuildSource.txt after a PASS of the editor target = fingerprint of the
#    C++ it compiled ("source=<tree of HEAD:Source> uproject=<blob of HEAD:*.uproject>", computed HERE with raw git), "dirty" when Source or the
#    .uproject differs from HEAD; a FAIL erases it (integrate.ps1 compares it with main before skipping a build).
Write-Output '[8] real build.ps1 writes HOL_BuildSource.txt (fingerprint of the compiled C++)'
$ErrorActionPreference = 'Continue'
$wt = New-Wt 'wt8'
Copy-Item (Join-Path (Split-Path -Parent (Split-Path -Parent $here)) 'HeroesOfLiberty.uproject') $wt
New-Item -ItemType Directory -Force (Join-Path $wt 'Source') | Out-Null
[IO.File]::WriteAllText((Join-Path $wt 'Source\a.cpp'), 'int a;')
[IO.File]::WriteAllText((Join-Path $wt '.gitignore'), "Binaries/`nSaved/`n")
foreach ($g in @(@('init', '-q', '-b', 'main'), @('config', 'user.name', 'Test'), @('config', 'user.email', 'test@example.com'), @('config', 'core.autocrlf', 'false'), @('add', '-A'), @('commit', '-q', '-m', 'init'))) { & git.exe -C $wt @g 2>&1 | Out-Null }
$print = "source=$((& git.exe -C $wt rev-parse 'HEAD:Source' | Out-String).Trim()) uproject=$((& git.exe -C $wt rev-parse 'HEAD:HeroesOfLiberty.uproject' | Out-String).Trim())"
$srcFile = Join-Path $wt 'Binaries\Win64\HOL_BuildSource.txt'
function Src-Text { if (Test-Path $srcFile) { ([IO.File]::ReadAllText($srcFile)).Trim() } else { '<absent>' } }
$q = Join-Path $tmp 'state8'; $csv = Join-Path $tmp 'q8.csv'
function Invoke-Build($fake) { $o = & powershell -NoProfile -ExecutionPolicy Bypass -File $realBuild -Root $wt -QueueDir $q -CsvPath $csv -FakeBuild $fake 2>&1 | ForEach-Object { "$_" }; [pscustomobject]@{ Code = $LASTEXITCODE; Out = ($o -join "`n") } }
$r = Invoke-Build $okc
Check ($r.Code -eq 0 -and (Src-Text) -eq $print) "PASS on a clean tree: HOL_BuildSource.txt = '$print' (got '$(Src-Text)', exit $($r.Code))"
[IO.File]::WriteAllText((Join-Path $wt 'Source\a.cpp'), 'int a2;')
$r = Invoke-Build $okc
Check ($r.Code -eq 0 -and (Src-Text) -eq 'dirty') "PASS on a dirty Source: HOL_BuildSource.txt = 'dirty' (got '$(Src-Text)')"
& git.exe -C $wt checkout -q -- Source 2>&1 | Out-Null
$r = Invoke-Build $okc
Check ((Src-Text) -eq $print) "PASS again on the clean tree: fingerprint back (got '$(Src-Text)')"
$r = Invoke-Build $badc
Check ($r.Code -eq 1 -and (Src-Text) -eq '<absent>') "FAIL: HOL_BuildSource.txt erased (got '$(Src-Text)', exit $($r.Code))"
$r = Invoke-Build $okc
$o = & powershell -NoProfile -ExecutionPolicy Bypass -File $realBuild -Root $wt -Target HeroesOfLibertyServer -QueueDir $q -CsvPath $csv -FakeBuild $badc 2>&1
Check ((Src-Text) -eq $print) "FAIL of another target (Server): the editor fingerprint is kept (got '$(Src-Text)')"
$ErrorActionPreference = 'Stop'

Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
if ($failures -eq 0) { Write-Output 'build_wait_test : PASS'; exit 0 } else { Write-Output "build_wait_test : FAIL ($failures)"; exit 1 }
