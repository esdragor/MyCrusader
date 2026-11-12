# Test of the build queue of Tools\build.ps1 (one build at a time, one waiting build per copy, orphans, -Priority merge, CSV).
# Uses a private queue directory + CSV in %TEMP% and a fake build (-FakeBuild, a sleep) in place of UnrealBuildTool: it
# never touches the real queue and never starts UBT, so it can run while other copies compile.
# Run: powershell -ExecutionPolicy Bypass -File <copy>\Tools\Tests\build_queue_test.ps1 [-Only b|c|d]
# Prints PASS/FAIL per check, a final "build_queue_test : PASS|FAIL"; exit code 0 / 1.
param([string] $Only = "", [string] $BuildScript = "")
$ErrorActionPreference = "Stop"
$Repo = (Resolve-Path "$PSScriptRoot\..\..").Path
$Build = if ($BuildScript) { $BuildScript } else { Join-Path $Repo "Tools\build.ps1" }
$Tmp = Join-Path $env:TEMP ("hol_bq_test_" + [guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force $Tmp | Out-Null
$env:HOL_BUILD_POLL_SECONDS = "1"
$Failures = 0
function Check($Condition, $Message) {
    if ($Condition) { Write-Host "  PASS $Message" } else { Write-Host "  FAIL $Message"; $script:Failures++ }
}
# fake copies: a folder with the real .uproject
foreach ($N in 'A', 'B', 'C', 'D', 'E', 'G', 'W', 'X', 'Y', 'Z') {
    New-Item -ItemType Directory -Force (Join-Path $Tmp "wt_$N") | Out-Null
    Copy-Item (Join-Path $Repo "HeroesOfLiberty.uproject") (Join-Path $Tmp "wt_$N")
}
# HOL_FAKE_GATE (a file path): the fake build also waits until that file exists, so a scenario holds the slot as long as it
# needs instead of racing a fixed sleep against the slow start of each build.ps1 (Get-CimInstance ~2 s per start).
Set-Content (Join-Path $Tmp "fake.ps1") 'Start-Sleep -Seconds ([int]$env:HOL_FAKE_SECONDS); if ($env:HOL_FAKE_GATE) { $G = $env:HOL_FAKE_GATE; $End = (Get-Date).AddSeconds(120); while (-not (Test-Path $G) -and (Get-Date) -lt $End) { Start-Sleep -Milliseconds 300 } }; "fake build done"; exit 0'
$Fake = Join-Path $Tmp "fake.ps1"

function New-Scenario($Name) {
    $S = @{ Dir = Join-Path $Tmp "state_$Name"; Csv = Join-Path $Tmp "q_$Name.csv"; Procs = @{} }
    $S
}
function Arguments($S, $Name, $Extra) {
    @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$Build`"", '-Root', "`"$(Join-Path $Tmp "wt_$Name")`"",
      '-QueueDir', "`"$($S.Dir)`"", '-CsvPath', "`"$($S.Csv)`"", '-FakeBuild', "`"$Fake`"") + $Extra
}
function Launch($S, $Name, $Seconds, $Extra = @(), $Key = $Name) {
    $env:HOL_FAKE_SECONDS = "$Seconds"
    $Out = Join-Path $Tmp "out_$($S.Dir | Split-Path -Leaf)_$Key.txt"
    $P = Start-Process powershell -ArgumentList (Arguments $S $Name $Extra) -PassThru -WindowStyle Hidden -RedirectStandardOutput $Out
    $P.Handle | Out-Null   # keeps the exit code readable
    $S.Procs[$Key] = @{ Proc = $P; Out = $Out }
}
function Tickets($S) {
    @(Get-ChildItem $S.Dir -Filter "*.json" -ErrorAction SilentlyContinue | ForEach-Object { Get-Content $_.FullName -Raw | ConvertFrom-Json })
}
function Wait-Until($Condition, $Seconds = 40) {
    $End = (Get-Date).AddSeconds($Seconds)
    while ((Get-Date) -lt $End) { if (& $Condition) { return $true }; Start-Sleep -Milliseconds 400 }
    $false
}
function Wait-Exit($S, $Key, $Seconds = 90) {
    $P = $S.Procs[$Key].Proc
    if (-not $P.WaitForExit($Seconds * 1000)) { return $null }
    $P.ExitCode
}
function Rows($S) { if (Test-Path $S.Csv) { @(Import-Csv $S.Csv) } else { @() } }
function Out-Text($S, $Key) { if (Test-Path $S.Procs[$Key].Out) { Get-Content $S.Procs[$Key].Out -Raw } else { "" } }

if (-not $Only -or $Only -eq 'b') {
    Write-Host "[b] one waiting build per copy"
    $S = New-Scenario b
    Launch $S 'A' 16
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.worktree -eq 'wt_A' -and $_.state -eq 'compiling' }).Count -eq 1 }) "A holds the slot (compiling)"
    Launch $S 'B' 3 @() 'B1'
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.worktree -eq 'wt_B' }).Count -eq 1 }) "B1 waits in the queue"
    Launch $S 'B' 3 @() 'B2'
    Launch $S 'C' 3
    Check ((Wait-Exit $S 'B1' 30) -eq 3) "B1 (replaced) exits with code 3"
    Check ((Out-Text $S 'B1') -match 'superseded by a newer build of the same copy') "B1 says superseded by a newer build of the same copy"
    Check (@(Tickets $S | Where-Object { $_.worktree -eq 'wt_B' }).Count -eq 1) "only one wt_B ticket left in the queue"
    $Codes = @('A', 'B2', 'C' | ForEach-Object { Wait-Exit $S $_ })
    Check (($Codes -join ',') -eq '0,0,0') "A, B2, C finish PASS (exit 0), got: $($Codes -join ',')"
    $R = Rows $S
    Check (@($R | Where-Object { $_.worktree -eq 'wt_B' -and $_.result -eq 'SUPERSEDED' }).Count -eq 1) "CSV: one SUPERSEDED row for wt_B"
    $Pass = @($R | Where-Object { $_.result -eq 'PASS' } | Sort-Object compile_start)
    Check (($Pass.worktree -join ',') -eq 'wt_A,wt_B,wt_C') "CSV: PASS order A, B (kept the place of B1), C, got: $($Pass.worktree -join ',')"
    Check (@($R | Where-Object { $_.result -eq 'PASS' -and $_.worktree -eq 'wt_C' -and $_.waited_behind -match 'wt_A' }).Count -eq 1) "CSV: C records it waited behind wt_A"
    Check (@(Tickets $S).Count -eq 0) "queue empty at the end"
}

if (-not $Only -or $Only -eq 'c') {
    Write-Host "[c] orphans"
    $S = New-Scenario c
    New-Item -ItemType Directory -Force $S.Dir | Out-Null
    $Gate = Join-Path $Tmp "gate_c"
    $env:HOL_FAKE_GATE = $Gate   # A holds the slot until the orphan checks are done
    Launch $S 'A' 1
    Remove-Item Env:HOL_FAKE_GATE
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.state -eq 'compiling' }).Count -eq 1 }) "A holds the slot"
    # a ticket whose owner process died (pid that does not exist)
    $Dead = @{ id = "deadbeef"; root = "x:\dead"; worktree = "wt_G"; pid = 999999; pidStart = 1; parentPid = 0; parentStart = 0
               priority = "normal"; requested = (Get-Date).AddMinutes(-1).ToString("o"); state = "waiting"; started = "" }
    $DeadFile = Join-Path $S.Dir "dead_deadbeef.json"
    [IO.File]::WriteAllText($DeadFile, ($Dead | ConvertTo-Json), (New-Object Text.UTF8Encoding($false)))
    # a build whose launcher exits right after starting it (parent gone while it waits)
    $Wrapper = Join-Path $Tmp "wrap.ps1"
    $WrapperText = @'
param($Build, $Root, $Q, $Csv, $Fake)
Start-Process powershell -WindowStyle Hidden -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$Build`"", '-Root', "`"$Root`"",
    '-QueueDir', "`"$Q`"", '-CsvPath', "`"$Csv`"", '-FakeBuild', "`"$Fake`"")
# D gets queued with a live parent (wait for ITS ticket, not a fixed delay: build.ps1 starts slowly), then this launcher exits
$End = (Get-Date).AddSeconds(60)
while ((Get-Date) -lt $End -and -not (Get-ChildItem $Q -Filter '*.json' -ErrorAction SilentlyContinue | Where-Object { (Get-Content $_.FullName -Raw) -match 'wt_D' })) { Start-Sleep -Milliseconds 300 }
'@
    Set-Content $Wrapper $WrapperText
    $env:HOL_FAKE_SECONDS = "3"
    Start-Process powershell -Wait -WindowStyle Hidden -ArgumentList @('-NoProfile', '-File', "`"$Wrapper`"", "`"$Build`"", "`"$(Join-Path $Tmp 'wt_D')`"", "`"$($S.Dir)`"", "`"$($S.Csv)`"", "`"$Fake`"")
    Launch $S 'E' 3   # a normal build with a live parent: must NOT be removed
    Check (Wait-Until { @(Rows $S | Where-Object { $_.worktree -eq 'wt_D' -and $_.result -eq 'ORPHAN' }).Count -eq 1 } 40) "CSV: orphan build wt_D left the queue (ORPHAN row)"
    Check (-not (Test-Path $DeadFile)) "ticket of the dead owner (wt_G) removed from the queue"
    Check (@(Rows $S | Where-Object { $_.worktree -eq 'wt_G' -and $_.result -eq 'ORPHAN' }).Count -eq 1) "CSV: wt_G logged as ORPHAN"
    Set-Content $Gate "go"
    $Codes = @('A', 'E' | ForEach-Object { Wait-Exit $S $_ })
    Check (($Codes -join ',') -eq '0,0') "A and E (live parent) still finish PASS, got: $($Codes -join ',')"
    Check (@(Rows $S | Where-Object { $_.worktree -eq 'wt_D' -and $_.result -eq 'PASS' }).Count -eq 0) "wt_D never compiled"
    Check (@(Tickets $S).Count -eq 0) "queue empty at the end"
}

if (-not $Only -or $Only -eq 'd') {
    Write-Host "[d] -Priority merge, then starved copies, then FIFO"
    $S = New-Scenario d
    # history: X and Y compiled 5 min ago (recent), W never (starved)
    $Stamp = (Get-Date).AddMinutes(-5).ToString("yyyy-MM-dd HH:mm:ss")
    Set-Content $S.Csv ("worktree,requested,compile_start,end,wait_s,duration_s,result,waited_behind,priority`r`nwt_X,$Stamp,$Stamp,$Stamp,0,1,PASS,`"`",normal`r`nwt_Y,$Stamp,$Stamp,$Stamp,0,1,PASS,`"`",normal") -Encoding ascii
    $Gate = Join-Path $Tmp "gate_d"
    $env:HOL_FAKE_GATE = $Gate   # A holds the slot until every other build is queued (no race against a fixed sleep)
    Launch $S 'A' 1
    Remove-Item Env:HOL_FAKE_GATE
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.state -eq 'compiling' }).Count -eq 1 }) "A holds the slot"
    foreach ($N in 'X', 'Y', 'W') {
        Launch $S $N 2
        Check (Wait-Until { @(Tickets $S | Where-Object { $_.worktree -eq "wt_$N" }).Count -eq 1 }) "wt_$N queued"
    }
    Launch $S 'Z' 2 @('-Priority', 'merge')
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.worktree -eq "wt_Z" }).Count -eq 1 }) "wt_Z (merge) queued last"
    Set-Content $Gate "go"
    foreach ($K in 'A', 'X', 'Y', 'W', 'Z') { Wait-Exit $S $K | Out-Null }
    $Order = @(Rows $S | Where-Object { $_.duration_s -ne '1' -or $_.worktree -eq 'wt_A' } | Sort-Object compile_start | ForEach-Object { $_.worktree }) -join ','
    Check ($Order -eq 'wt_A,wt_Z,wt_W,wt_X,wt_Y') "start order A (compiling), Z (merge), W (starved), X, Y (FIFO); got: $Order"
    $Z = Rows $S | Where-Object { $_.worktree -eq 'wt_Z' }
    Check ($Z.priority -eq 'merge') "CSV records priority merge for wt_Z"
    # environment variable form
    Remove-Item Env:HOL_BUILD_PRIORITY -ErrorAction SilentlyContinue
    $env:HOL_BUILD_PRIORITY = "merge"
    $S2 = New-Scenario d2
    Launch $S2 'E' 1
    Wait-Exit $S2 'E' | Out-Null
    Remove-Item Env:HOL_BUILD_PRIORITY
    Check (@(Rows $S2 | Where-Object { $_.priority -eq 'merge' }).Count -eq 1) "HOL_BUILD_PRIORITY=merge gives priority merge"
}

if (-not $Only -or $Only -eq 'e') {
    Write-Host "[e] -Wait : a waiting build asked with -Wait is never replaced (no exit 3), the newer build queues behind it"
    $S = New-Scenario e
    Launch $S 'A' 12
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.worktree -eq 'wt_A' -and $_.state -eq 'compiling' }).Count -eq 1 }) "A holds the slot (compiling)"
    Launch $S 'B' 2 @('-Wait') 'B1'
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.worktree -eq 'wt_B' }).Count -eq 1 }) "B1 (-Wait) waits in the queue"
    Launch $S 'B' 2 @() 'B2'
    Check (Wait-Until { @(Tickets $S | Where-Object { $_.worktree -eq 'wt_B' }).Count -eq 2 }) "B2 (newer, no -Wait) queues behind B1 instead of replacing it: 2 wt_B tickets"
    $Codes = @('A', 'B1', 'B2' | ForEach-Object { Wait-Exit $S $_ 60 })
    Check (($Codes -join ',') -eq '0,0,0') "A, B1, B2 all finish PASS (exit 0, none superseded), got: $($Codes -join ',')"
    $R = Rows $S
    Check (@($R | Where-Object { $_.worktree -eq 'wt_B' -and $_.result -eq 'PASS' }).Count -eq 2) "CSV: two PASS rows for wt_B"
    Check (@($R | Where-Object { $_.result -eq 'SUPERSEDED' }).Count -eq 0) "CSV: no SUPERSEDED row"
    $Pass = @($R | Where-Object { $_.result -eq 'PASS' } | Sort-Object compile_start)
    Check (($Pass.worktree -join ',') -eq 'wt_A,wt_B,wt_B') "CSV: PASS order A, B1, B2, got: $($Pass.worktree -join ',')"
    Check (@(Tickets $S).Count -eq 0) "queue empty at the end"
}

Get-Process powershell -ErrorAction SilentlyContinue | Out-Null
Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
if ($Failures) { Write-Host "build_queue_test : FAIL ($Failures check(s))"; exit 1 }
Write-Host "build_queue_test : PASS"
exit 0
