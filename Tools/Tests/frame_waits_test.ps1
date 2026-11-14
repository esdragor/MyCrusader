# Test de L14-7 et L14-8 (2026-11-14), sans editeur ni compilation :
#  - L14-8 : Tools\Scenarios\FrameWaits.ps1 (Get-FrameWaitProblem) signale un `frames N` > 30 qui attend un evenement du jeu (hors prise
#    de camera : en fenetre cachee ~150 images/s, `frames 600` dure 4 s, a633b97) et un `waitlog` juste apres un `wait` de plus de 10 s ;
#    le vrai check.ps1 -FrameWaits Fail refuse un tel scenario AVANT de lancer un editeur.
#  - L14-7 : le vrai Tools\build.ps1 imprime Get-StaleBinaryProblem au debut (faux build -FakeBuild, file et CSV temporaires).
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\frame_waits_test.ps1 ; code de sortie = nombre de FAIL.
$ErrorActionPreference = 'Continue'
$Tools = Split-Path $PSScriptRoot
$Tmp = Join-Path $env:TEMP ('frame_waits_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Sc = Join-Path $Tmp 'Tools\Scenarios'
New-Item -ItemType Directory -Force $Sc | Out-Null
$Fails = 0
function Check([bool]$Ok, [string]$What, [string]$Detail = '') {
    if ($Ok) { Write-Output "PASS $What" } else { $script:Fails++; Write-Output "FAIL $What  $Detail" }
}
try {
    Set-Content (Join-Path $Sc 'bad.txt') @('# a scenario that waits for the game by frames', 'hol.Map.Load x', 'frames 600', 'waitlog Unit arrived', 'wait 12', 'waitlog Garrison entered', 'hol.Camera.FocusNear 0 0', 'frames 90', 'hol.Test.Shot a', 'frames 20')
    Set-Content (Join-Path $Sc 'good.txt') @('hol.Map.Load x', 'wait 3', 'waitlog Unit arrived', 'hol.Camera.FocusNear 0 0', 'frames 60', 'hol.Test.Shot a')

    Write-Output '--- 1. Get-FrameWaitProblem'
    $fw = Join-Path $Tools 'Scenarios\FrameWaits.ps1'
    if (Test-Path $fw) { . $fw }
    $p = @(if (Get-Command Get-FrameWaitProblem -ErrorAction SilentlyContinue) { Get-FrameWaitProblem (Join-Path $Sc 'bad.txt') })
    $p | ForEach-Object { Write-Host "  > $($_.Level) $($_.Text)" }
    Check (@($p | Where-Object { $_.Level -eq 'FAIL' -and $_.Text -match 'line 3' -and $_.Text -match 'frames 600' }).Count -eq 1) 'frames 600 before a waitlog is a FAIL (line 3)'
    Check (@($p | Where-Object { $_.Level -eq 'WARN' -and $_.Text -match 'line 6' -and $_.Text -match 'wait 12' }).Count -eq 1) 'a waitlog right after wait 12 is a WARN (line 6)'
    Check (@($p | Where-Object { $_.Text -match 'line (8|10)\b' }).Count -eq 0) 'frames 90 around a camera shot and frames 20 are not reported'
    $g = @(if (Get-Command Get-FrameWaitProblem -ErrorAction SilentlyContinue) { Get-FrameWaitProblem (Join-Path $Sc 'good.txt') } else { 'missing' })
    Check ($g.Count -eq 0) 'a scenario waiting by wait/waitlog and settling a shot by frames has no problem' ($g -join ' ')

    Write-Output '--- 2. check.ps1 -FrameWaits Fail refuses bad.txt before launching any editor'
    $o = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tools 'Scenarios\check.ps1') bad -Root $Tmp -FrameWaits Fail 2>&1 | Out-String
    Write-Host $o.TrimEnd()
    Check ($LASTEXITCODE -ne 0 -and $o -match 'check bad : FAIL' -and $o -match 'frames 600') 'check.ps1 refuses with the line named' $o

    Write-Output '--- 3. build.ps1 prints Get-StaleBinaryProblem first (L14-7)'
    $Root = Join-Path $Tmp 'wt'
    New-Item -ItemType Directory -Force (Join-Path $Root 'Binaries\Win64'), (Join-Path $Root 'Source\HeroesOfLiberty'), (Join-Path $Tmp 'queue') | Out-Null
    Copy-Item (Join-Path (Split-Path $Tools) 'HeroesOfLiberty.uproject') $Root
    $dll = Join-Path $Root 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
    Set-Content $dll 'old'; (Get-Item $dll).LastWriteTime = (Get-Date).AddHours(-1)
    Set-Content (Join-Path $Root 'Source\HeroesOfLiberty\a.cpp') 'newer'
    $fake = Join-Path $Tmp 'fake.ps1'; Set-Content $fake ("Set-Content '$dll' 'new'; 'fake compile ok'; exit 0")
    $o = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tools 'build.ps1') -Root $Root -QueueDir (Join-Path $Tmp 'queue') -CsvPath (Join-Path $Tmp 'q.csv') -FakeBuild $fake 2>&1 | Out-String
    Write-Host $o.TrimEnd()
    Check ($o -match 'LaunchCheck .*older than its C\+\+ source') 'build.ps1 prints the stale binary problem (dll older than a.cpp) before building' $o
    $o = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tools 'build.ps1') -Root $Root -QueueDir (Join-Path $Tmp 'queue') -CsvPath (Join-Path $Tmp 'q.csv') -FakeBuild $fake 2>&1 | Out-String
    Check ($o -match 'LaunchCheck .*vide') 'after that build the dll is newer: LaunchCheck says vide' $o
} catch {
    $Fails++; Write-Output ('FAIL exception : ' + $_)
} finally {
    Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
}
Write-Output ("frame_waits_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $Fails
