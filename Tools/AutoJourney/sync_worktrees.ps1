# Auto_Journey : fin de seance, chaque worktree recupere main dans sa branche (git merge main), puis son binaire est remis a jour.
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\AutoJourney\sync_worktrees.ps1 [-DryRun] [-NoBuild] [-WaitBuilds]
# Regles : jamais de push ; un worktree avec des fichiers SUIVIS modifies (travail non commite) est saute et signale (ni fusion ni build) ;
# un conflit annule la fusion (merge --abort) et est signale ; main lui-meme est ignore.
# BUILD (2026-11-05, Auto_Journey_Checklist L6-5) : un binaire perime donne 3 a 5 faux rouges (CommandGrid, Technologies, Map.Table).
# Apres la fusion, chaque worktree dont le CODE a change (Source, Script, Content, Tools/Data entre l'ancien et le nouveau HEAD),
# et chaque worktree deja a jour dont le binaire est plus ancien que le dernier commit de code (meme test que
# session_gate.ps1 -Phase Report), recoit un Tools\build.ps1 -Root <worktree> :
#  - PRIORITE BASSE = priorite de file "normal" (jamais "merge", donc derriere un build de fusion) + processus BelowNormal ;
#    la file de build.ps1 compile un worktree a la fois, donc rien n'est lance en double ;
#  - detache par defaut (HOL_BUILD_NO_ORPHAN_CHECK=1 : le build survit a ce script) ; sortie dans <worktree>\Saved\Logs\sync_build.out ;
#  - -WaitBuilds : attend la fin de tous les builds et imprime leur ligne PASS / FAIL (code 0 PASS, 1 FAIL, 2 occupe, 3 remplace, 4 orphelin) ;
#  - -NoBuild : ne lance aucun build ; -DryRun : ne fusionne ni ne compile, dit seulement ce qui serait fait.
# Sortie : une ligne par worktree : nom | branche | resultat. Code de sortie 0 meme avec des sauts (lire la sortie).
# Parametres de test (Tools\Tests\sync_worktrees_test.ps1) : -BuildScript, -QueueDir, -CsvPath, -FakeBuild sont passes a build.ps1.
param(
    [string]$Docs = 'C:\Users\baudo\Documents',
    [switch]$DryRun,
    [switch]$NoBuild,
    [switch]$WaitBuilds,
    [string]$BuildScript = 'C:\Users\baudo\Documents\MyCrusader\Tools\build.ps1',
    [string]$QueueDir = '',
    [string]$CsvPath = '',
    [string]$FakeBuild = ''
)

$main = Join-Path $Docs 'MyCrusader'
$dirs = @(Get-ChildItem $Docs -Directory -Filter 'HeroesOfLiberty-*' | ForEach-Object { $_.FullName })
$CodePaths = @('Source', 'Script', 'Content', 'Tools/Data')   # les memes que session_gate.ps1 -Phase Report
$Builds = New-Object System.Collections.Generic.List[object]

# Vrai si le binaire du worktree est plus ancien que son dernier commit de code ($null si pas de binaire ou pas de commit de code).
function Test-StaleBinary([string]$Dir) {
    $lastCode = git -C $Dir log -1 --format=%ct -- @CodePaths 2>$null
    $bin = Get-ChildItem (Join-Path $Dir 'Binaries\Win64') -Filter '*.dll' -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $bin -or -not $lastCode) { return $null }
    return ($bin.LastWriteTime -lt [DateTimeOffset]::FromUnixTimeSeconds([int64]$lastCode).LocalDateTime)
}

# Lance build.ps1 en priorite basse pour ce worktree ; renvoie une phrase pour la ligne de sortie.
function Start-LowPriorityBuild([string]$Dir, [string]$Name) {
    $logDir = Join-Path $Dir 'Saved\Logs'
    New-Item -ItemType Directory -Force $logDir | Out-Null
    $out = Join-Path $logDir 'sync_build.out'
    $psArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $BuildScript, '-Root', $Dir, '-Priority', 'normal')
    if ($QueueDir) { $psArgs += @('-QueueDir', $QueueDir) }
    if ($CsvPath) { $psArgs += @('-CsvPath', $CsvPath) }
    if ($FakeBuild) { $psArgs += @('-FakeBuild', $FakeBuild) }
    $quoted = ($psArgs | ForEach-Object { if ($_ -match '\s') { '"' + $_ + '"' } else { $_ } }) -join ' '
    $old = $env:HOL_BUILD_NO_ORPHAN_CHECK
    $env:HOL_BUILD_NO_ORPHAN_CHECK = '1'
    try {
        $p = Start-Process powershell -ArgumentList $quoted -WindowStyle Hidden -PassThru -RedirectStandardOutput $out -RedirectStandardError "$out.err"
        $null = $p.Handle   # garde le handle : sans cela ExitCode reste vide une fois le processus fini
        try { $p.PriorityClass = 'BelowNormal' } catch { }
    } finally { $env:HOL_BUILD_NO_ORPHAN_CHECK = $old }
    $Builds.Add([pscustomobject]@{ Name = $Name; Process = $p; Out = $out })
    return "build lance (priorite basse, pid $($p.Id), sortie $out)"
}

Write-Output ("Heure PowerShell : " + (Get-Date -Format 'HH:mm') + "  main = " + (git -C $main rev-parse --short HEAD))
foreach ($d in $dirs) {
    $name = Split-Path $d -Leaf
    $branch = (git -C $d rev-parse --abbrev-ref HEAD 2>$null)
    if (-not $branch) { Write-Output "$name | ? | pas un worktree git"; continue }
    $dirty = @(git -C $d status --porcelain --untracked-files=no 2>$null | Where-Object { $_ -notmatch 'IN_USE\.txt' -and $_ -notmatch 'HeroesOfLiberty\.uproject' })
    if ($dirty.Count -gt 0) { Write-Output ("{0} | {1} | SAUTE : {2} fichier(s) suivi(s) modifie(s) non commite(s)" -f $name, $branch, $dirty.Count); continue }
    $behind = [int]((git -C $d rev-list --count 'HEAD..main' 2>$null))
    $wantBuild = $false; $why = ''
    if ($behind -eq 0) {
        $stale = Test-StaleBinary $d
        if ($stale -eq $true) { $wantBuild = $true; $why = 'binaire plus ancien que le dernier code' }
        else { Write-Output ("$name | $branch | deja a jour" + $(if ($null -eq $stale) { ' (pas de binaire a comparer)' } else { ', binaire a jour' })); continue }
        if ($NoBuild) { Write-Output "$name | $branch | deja a jour, MAIS $why (-NoBuild : pas de build)"; continue }
        if ($DryRun) { Write-Output "$name | $branch | deja a jour, build a lancer ($why)"; continue }
        Write-Output "$name | $branch | deja a jour, $why : $(Start-LowPriorityBuild $d $name)"
        continue
    }
    $codeChanged = @(git -C $d diff --name-only 'HEAD...main' -- @CodePaths 2>$null).Count -gt 0
    if ($DryRun) { Write-Output ("$name | $branch | retard de $behind commit(s) : a fusionner" + $(if ($codeChanged -and -not $NoBuild) { ', puis build (code change)' } else { '' })); continue }
    $uproj = @(git -C $d status --porcelain --untracked-files=no 2>$null | Where-Object { $_ -match 'HeroesOfLiberty\.uproject' })
    if ($uproj.Count -gt 0) { git -C $d checkout -- HeroesOfLiberty.uproject 2>$null }
    $before = git -C $d rev-parse HEAD 2>$null
    $null = git -C $d merge main -m "Merge branch 'main' into $branch" 2>&1
    $code = $LASTEXITCODE
    $unmerged = @(git -C $d diff --name-only --diff-filter=U 2>$null)
    if ($unmerged.Count -gt 0) {
        git -C $d merge --abort 2>$null
        Write-Output ("{0} | {1} | CONFLIT annule ({2} fichier(s) : {3})" -f $name, $branch, $unmerged.Count, ($unmerged | Select-Object -First 3) -join ', ')
        continue
    }
    $now = [int]((git -C $d rev-list --count 'HEAD..main' 2>$null))
    if ($now -ne 0) { Write-Output "$name | $branch | ECHEC fusion (code $code), retard restant $now"; continue }
    $line = "$name | $branch | mis a jour (+$behind de main) = $(git -C $d rev-parse --short HEAD)"
    $codeNow = @(git -C $d diff --name-only $before HEAD -- @CodePaths 2>$null).Count -gt 0
    if ($codeNow -and -not $NoBuild) { $line += " ; code change : $(Start-LowPriorityBuild $d $name)" }
    elseif ($codeNow) { $line += ' ; code change, MAIS -NoBuild : binaire perime, lancer Tools\build.ps1' }
    Write-Output $line
}

if ($WaitBuilds -and $Builds.Count -gt 0) {
    foreach ($b in $Builds) {
        $b.Process.WaitForExit()
        $res = @(if (Test-Path $b.Out) { Get-Content $b.Out | Where-Object { $_ -match '^build :' } | Select-Object -Last 1 })
        Write-Output ("build {0} | code {1} | {2}" -f $b.Name, $b.Process.ExitCode, $(if ($res.Count) { $res[0] } else { '(pas de ligne build :)' }))
    }
}
