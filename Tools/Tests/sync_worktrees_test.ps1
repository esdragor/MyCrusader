# Test de bout en bout de Tools\AutoJourney\sync_worktrees.ps1 (fusion de main + build en priorite basse) SANS compiler de C++ :
# un depot temporaire (main + un worktree), la vraie file de build.ps1 (-QueueDir et -CsvPath temporaires), un faux build (-FakeBuild)
# qui ecrit une DLL, et le vrai session_gate.ps1 -Phase Report pour juger le binaire.
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\sync_worktrees_test.ps1
# Code de sortie : nombre d'assertions rouges (0 = tout vert). N'ecrit rien hors du dossier temporaire.
$ErrorActionPreference = 'Continue'
$Tools = Split-Path $PSScriptRoot
$Sync = Join-Path $Tools 'AutoJourney\sync_worktrees.ps1'
$Gate = Join-Path $Tools 'AutoJourney\session_gate.ps1'
$Build = Join-Path $Tools 'build.ps1'
$RealUProject = Join-Path (Split-Path $Tools) 'HeroesOfLiberty.uproject'
$Root = Join-Path $env:TEMP ('sync_wt_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Docs = Join-Path $Root 'Docs'
$Main = Join-Path $Docs 'MyCrusader'
$Wt = Join-Path $Docs 'HeroesOfLiberty-T'
$Queue = Join-Path $Root 'queue'
$Csv = Join-Path $Root 'BuildQueue.csv'
$Fake = Join-Path $Root 'fake_build.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$What, [string]$Detail = '') {
    if ($Ok) { Write-Output "PASS $What" } else { $script:Fails++; Write-Output "FAIL $What  $Detail" }
}
function RunGit([string]$Dir, [string[]]$GitArgs) { $o = & git -C $Dir @GitArgs 2>&1; if ($LASTEXITCODE -ne 0) { throw "git $GitArgs : $o" }; $o }
function Sync([string[]]$Extra) {
    $o = & powershell -NoProfile -ExecutionPolicy Bypass -File $Sync -Docs $Docs -BuildScript $Build -QueueDir $Queue -CsvPath $Csv -FakeBuild $Fake @Extra 2>&1 | Out-String
    Write-Host $o.TrimEnd()
    $o
}
function GateBinary { (& powershell -NoProfile -ExecutionPolicy Bypass -File $Gate -Phase Report -Root $Wt -End 23:59 2>&1 | Out-String) -split "`r?`n" | Where-Object { $_ -match 'binaire' } }
function Commit-OnMain([string]$File, [string]$Text, [string]$Msg) {
    $p = Join-Path $Main $File
    New-Item -ItemType Directory -Force (Split-Path $p) | Out-Null
    Set-Content $p $Text
    RunGit $Main @('add', $File) | Out-Null
    RunGit $Main @('commit', '-m', $Msg) | Out-Null
}
function DllTime { (Get-Item (Join-Path $Wt 'Binaries\Win64\x.dll')).LastWriteTime }

try {
    New-Item -ItemType Directory -Force $Main, $Queue | Out-Null
    git -C $Main init -b main 2>&1 | Out-Null
    RunGit $Main @('config', 'user.email', 't@t'); RunGit $Main @('config', 'user.name', 'sync test'); RunGit $Main @('config', 'core.autocrlf', 'false')
    Copy-Item $RealUProject (Join-Path $Main 'HeroesOfLiberty.uproject')
    RunGit $Main @('add', 'HeroesOfLiberty.uproject') | Out-Null
    Commit-OnMain 'Source\a.cpp' 'v1' 'init'
    RunGit $Main @('worktree', 'add', $Wt, '-b', 't') | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $Wt 'Binaries\Win64') | Out-Null
    Set-Content (Join-Path $Wt 'Binaries\Win64\x.dll') 'old'
    Set-Content $Fake ("Set-Content '" + (Join-Path $Wt 'Binaries\Win64\x.dll') + "' (Get-Date -Format o); 'fake compile ok'; exit 0")

    Write-Output '--- 1. docs-only change on main: merged, NO build'
    Commit-OnMain 'Docs\note.md' 'doc' 'docs only'
    $o = Sync @('-DryRun'); Check ($o -match 'retard de 1 commit' -and $o -notmatch 'puis build') 'dry run: lagging, no build announced for a docs change'
    $o = Sync @(); Check ($o -match 'mis a jour' -and $o -notmatch 'build lance') 'docs-only merge launches no build'

    Write-Output '--- 2. code change on main, -NoBuild: merged, stale binary is flagged and the gate WARNs'
    (Get-Item (Join-Path $Wt 'Binaries\Win64\x.dll')).LastWriteTime = (Get-Date).AddHours(-1)
    Commit-OnMain 'Source\a.cpp' 'v2' 'code change 2'
    $o = Sync @('-NoBuild'); Check ($o -match 'mis a jour' -and $o -match 'MAIS -NoBuild' -and $o -notmatch 'build lance') '-NoBuild: no build, stale binary reported'
    $g = GateBinary; Check (($g -join ' ') -match 'WARN.*plus ANCIEN') 'session_gate Report WARNs about the stale binary' ($g -join ' ')

    Write-Output '--- 3. already up to date but stale binary: build chained (low priority), waited'
    $before = DllTime
    $o = Sync @('-WaitBuilds')
    Check ($o -match 'deja a jour, binaire plus ancien que le dernier code : build lance \(priorite basse') 'up-to-date worktree with stale binary gets a build'
    Check ($o -match 'build HeroesOfLiberty-T \| code 0 \| build : PASS') 'build.ps1 (real queue, fake compile) ends PASS'
    Check ((DllTime) -gt $before) 'the binary is now newer' "before $before after $(DllTime)"
    $g = GateBinary; Check (($g -join ' ') -match 'PASS.*binaire') 'session_gate Report now PASSes on the binary' ($g -join ' ')
    $csvText = if (Test-Path $Csv) { Get-Content $Csv -Raw } else { '' }
    Check ($csvText -match 'HeroesOfLiberty-T.*PASS.*normal') 'the build went through the queue with priority normal (not merge)' $csvText

    Write-Output '--- 4. lagging worktree with a code change: merge and build in one run'
    Commit-OnMain 'Source\a.cpp' 'v3' 'code change 3'
    $before = DllTime; Start-Sleep -Seconds 2
    $o = Sync @('-WaitBuilds')
    Check ($o -match 'mis a jour .*code change : build lance') 'merge with code change chains the build'
    Check ($o -match 'build HeroesOfLiberty-T \| code 0 \| build : PASS') 'that build ends PASS'
    Check ((DllTime) -gt $before) 'binary refreshed after the merge'

    Write-Output '--- 5. nothing to do'
    $o = Sync @(); Check ($o -match 'deja a jour, binaire a jour' -and $o -notmatch 'build lance') 'up to date and fresh binary: nothing launched'

    Write-Output '--- 6. dirty worktree is skipped (no merge, no build)'
    Commit-OnMain 'Source\a.cpp' 'v4' 'code change 4'
    Add-Content (Join-Path $Wt 'Source\a.cpp') 'local edit'
    $o = Sync @(); Check ($o -match 'SAUTE' -and $o -notmatch 'build lance') 'dirty worktree skipped'

    # L14-3 (2026-11-13) : une AUTRE session de l'utilisateur commitait dans Arwen (d751-root, "Faction choice", 19:11-19:17) ;
    # sync_worktrees y a fusionne main et la recette a conclu a tort. Un commit de moins de 2 h hors de main, d'un worktree absent
    # du tableau '## Agents du <date>' d'Auto_Journey_Jour.md, fait sauter ET signaler le worktree (sync et status).
    Write-Output '--- 7. recent commit of another session (worktree not in the Agents table): skipped and reported'
    RunGit $Wt @('checkout', '--', 'Source\a.cpp') | Out-Null
    Set-Content (Join-Path $Wt 'Source\b.cpp') 'faction choice'
    RunGit $Wt @('add', 'Source\b.cpp') | Out-Null
    RunGit $Wt @('commit', '-m', 'Faction choice (other session)') | Out-Null
    $foreign = (RunGit $Wt @('rev-parse', '--short', 'HEAD') | Out-String).Trim()
    $o = Sync @('-NoBuild')
    Check ($o -match 'SAUTE' -and $o -match 'autre session' -and $o -match $foreign) 'sync: worktree with a recent commit of another session is skipped and named' $o
    Check ([int](RunGit $Wt @('rev-list', '--count', 'HEAD..main')) -gt 0) 'sync: main was NOT merged into it'
    $st = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tools 'AutoJourney\status.ps1') -Docs $Docs 2>&1 | Out-String
    Write-Host $st.TrimEnd()
    Check ($st -match 'HeroesOfLiberty-T .*AUTRE SESSION') 'status: the worktree is flagged AUTRE SESSION' $st
    $st = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tools 'AutoJourney\status.ps1') -Docs $Docs -InMain $foreign 2>&1 | Out-String
    Check ($st -match "$foreign.*ABSENT de main") 'status -InMain: a branch-only commit is ABSENT de main (merge-base --is-ancestor)' $st
    $mainSha = (RunGit $Main @('rev-parse', '--short', 'main') | Out-String).Trim()
    $st = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Tools 'AutoJourney\status.ps1') -Docs $Docs -InMain $mainSha 2>&1 | Out-String
    Check ($st -match "$mainSha.*DANS main") 'status -InMain: a main commit is DANS main' $st

    Write-Output '--- 8. the same worktree listed in today''s Agents table: merged as usual'
    [IO.File]::WriteAllText((Join-Path $Main 'Auto_Journey_Jour.md'), ("## Agents du " + (Get-Date -Format 'yyyy-MM-dd') + "`r`n`r`n| Agent | id | worktree |`r`n|---|---|---|`r`n| TESTEUR-1114a | a1 | HeroesOfLiberty-T |`r`n"))
    $o = Sync @('-NoBuild')
    Check ($o -match 'mis a jour' -and $o -notmatch 'autre session') 'sync: a listed worktree is merged' $o

    # L29-6 (2026-11-26) : la sync de fin a fusionne main dans tests-bindings-1125s (Faramir) alors qu'elle l'avait sautee au debut.
    # Un worktree dont IN_USE.txt dit OCCUPE, ou dit « libre » mais annonce une branche PRETE (elle attend integrate.ps1), est « pris » :
    # ni fusion ni build. Un « libre : ... » sans mention est fusionne comme avant. L'EFFET est juge sur git rev-parse HEAD.
    Write-Output '--- 9. worktrees taken through IN_USE.txt (OCCUPE / branche PRETE) are skipped, a plain libre one is merged'
    $taken = @(
        @{ N = 'U1'; Line = 'OCCUPE par TEST-1 (2026-11-26 11:00) : une tache'; Skip = $true },
        @{ N = 'U2'; Line = 'libre : X fini 2026-11-26 (DEUX BRANCHES PRETES pour integrate.ps1)'; Skip = $true },
        @{ N = 'U3'; Line = 'libre : Y fini 2026-11-26 10:00 (rien a signaler)'; Skip = $false }
    )
    foreach ($t in $taken) {
        $t.Dir = Join-Path $Docs ('HeroesOfLiberty-' + $t.N)
        RunGit $Main @('worktree', 'add', $t.Dir, '-b', ('b-' + $t.N)) | Out-Null
    }
    Commit-OnMain 'Docs\after_taken.md' 'later' 'main moves after the worktrees were cut'
    foreach ($t in $taken) {
        [IO.File]::WriteAllText((Join-Path $t.Dir 'IN_USE.txt'), ($t.Line + "`r`nCPP: non`r`n"))
        $t.Before = (RunGit $t.Dir @('rev-parse', 'HEAD') | Out-String).Trim()
    }
    $o = Sync @('-NoBuild')
    foreach ($t in $taken) {
        $after = (RunGit $t.Dir @('rev-parse', 'HEAD') | Out-String).Trim()
        $line = @($o -split "`r?`n" | Where-Object { $_ -match ('HeroesOfLiberty-' + $t.N + ' \|') }) -join ' '
        if ($t.Skip) {
            Check ($after -eq $t.Before) ("IN_USE '" + $t.Line.Substring(0, 12) + "': HEAD unchanged (main NOT merged)") "before $($t.Before) after $after"
            Check ($line -match 'SAUTE' -and $line -match 'pris') ("IN_USE '" + $t.Line.Substring(0, 12) + "': line says SAUTE ... pris") $line
        } else {
            Check ($after -ne $t.Before) 'IN_USE libre without mention: main merged (HEAD moved)' "before $($t.Before) after $after"
            Check ($line -match 'mis a jour' -and $line -notmatch 'pris') 'IN_USE libre without mention: reported as mis a jour' $line
        }
    }
} catch {
    $Fails++; Write-Output ('FAIL exception : ' + $_)
} finally {
    try { git -C $Main worktree remove --force $Wt 2>&1 | Out-Null } catch { }
    Remove-Item -Recurse -Force $Root -ErrorAction SilentlyContinue
}
Write-Output ("sync_worktrees_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $Fails
