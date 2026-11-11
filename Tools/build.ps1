# Builds one copy of the project (the main copy or a worktree) without waiting on the other sessions by hand.
# Every copy shares one engine (UE 5.8 AS 3), so:
# - one UnrealBuildTool at a time, whatever the copy: builds queue in OUR queue (see below), and UnrealBuildTool still gets
#   -WaitMutex as a backstop against any build started without this script;
# - the Live Coding lock is named after the engine's UnrealEditor.exe, shared by every copy: an editor or game opened
#   elsewhere with Live Coding on would make UnrealBuildTool refuse. That is a false alarm for another copy (its own
#   Binaries and Intermediate), so the guard is skipped then (-NoHotReloadFromIDE).
# Games, tests and editors of OTHER copies never block this build. One of THIS copy does (its DLLs are locked): the
# script stops and names it (-Force to try anyway).
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\build.ps1 [-Root <copy>] [-Target HeroesOfLibertyEditor] [-Config Development] [-Force] [-Priority merge]
# Full log: <copy>\Saved\Logs\build.log. Prints one line PASS / FAIL with the first errors.
# Exit codes: 0 PASS, 1 FAIL, 2 this copy is busy, 3 superseded by a newer build of the same copy (never compiled),
# 4 orphan (the process that launched this build is gone, so it left the queue without compiling).
#
# -Priority merge (or the environment variable HOL_BUILD_PRIORITY=merge): for the grouped-merge build. It goes before every
#   normal build in the queue, never before the build that is already compiling. Queue order: 1. merge; 2. copies that have
#   compiled nothing for 30 min (or ever, per BuildQueue.csv); 3. the rest, first come first served.
#
# ONE WAITING BUILD PER COPY: a new build of the same -Root replaces the one still waiting (it takes its place in the line
#   and the better of the two priorities); the replaced one ends with "superseded by a newer build of the same copy", exit 3.
#   A build that is already compiling is never touched. A waiting build whose launching process (parent) no longer exists
#   (agent finished, shell closed) leaves the queue, exit 4. Detached launches that must outlive their parent:
#   set HOL_BUILD_NO_ORPHAN_CHECK=1.
#
# THE QUEUE (design, most robust choice): a DIRECTORY of ticket files, one JSON per build, NOT a single shared state file
#   and NOT deduced from processes. Each ticket is written/removed only under an exclusive lock file (queue.lock, opened
#   with FileShare.None: the OS releases it if the holder dies, so a crash can never leave a stale lock). Liveness is
#   judged from the ticket owner's pid + start time (no pid-reuse confusion). A dead owner's ticket is dropped by whoever
#   looks next. The head of the line claims the slot by turning its ticket "compiling" inside the lock, only when no
#   other ticket is compiling and no foreign UnrealBuildTool (started without this script) is running.
#   State: C:\Users\baudo\Documents\MyCrusader\Docs\BuildQueue.state\   Statistics: ...\Docs\BuildQueue.csv
#   (one row per build: worktree, requested, compile_start, end, wait_s, duration_s, result, waited_behind, priority;
#   summary with Tools\build_queue_report.ps1).
# -Wait (L11-2): ONE blocking call that waits its turn in the queue and then builds; nothing to poll. A waiting build asked with
#   -Wait is never replaced by a newer build of the same copy (no exit 3, no lost place): the newer build queues behind it.
#   The orphan rule still applies (launching process gone: exit 4). Run it with -Wait whenever a sibling build may be queued.
# Tests only (Tools\Tests\build_queue_test.ps1): -QueueDir, -CsvPath, -FakeBuild <script> replaces Build.bat; env
#   HOL_BUILD_POLL_SECONDS (default 3) is the queue polling period.
param(
    [string] $Root = "",
    [string] $Target = "HeroesOfLibertyEditor",
    [string] $Config = "Development",
    [switch] $Force,
    [switch] $Wait,
    [ValidateSet("normal", "merge")] [string] $Priority = $(if ($env:HOL_BUILD_PRIORITY -eq "merge") { "merge" } else { "normal" }),
    [string] $QueueDir = "C:\Users\baudo\Documents\MyCrusader\Docs\BuildQueue.state",
    [string] $CsvPath = "C:\Users\baudo\Documents\MyCrusader\Docs\BuildQueue.csv",
    [string] $FakeBuild = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not (Test-Path $UProject)) { Write-Host "build : FAIL (no HeroesOfLiberty.uproject in $Project)"; exit 1 }
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
if (-not $EngineRoot) { Write-Host "build : FAIL (engine '$Association' of the .uproject not found in the registry)"; exit 1 }
$EngineRoot = $EngineRoot.Replace('/', '\').TrimEnd('\')
$Log = Join-Path $Project "Saved\Logs\build.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
$Worktree = Split-Path -Leaf $Project
$Poll = [int]($(if ($env:HOL_BUILD_POLL_SECONDS) { $env:HOL_BUILD_POLL_SECONDS } else { 3 }))
$StarvedMinutes = 30
$Requested = Get-Date

# The processes of this engine (paths compared with backslashes, case ignored).
function Get-EngineProcesses {
    @(Get-CimInstance Win32_Process | Where-Object {
        $_.CommandLine -and $_.CommandLine.Replace('/', '\') -like "*$EngineRoot\*" })
}
function Copy-Of($Process) {
    if ($Process.CommandLine.Replace('/', '\') -match '([A-Za-z]:\\[^"=]*?)\\HeroesOfLiberty\.uproject') { Split-Path -Leaf $Matches[1] } else { "another project" }
}
function Is-ThisCopy($Process) { $Process.CommandLine.Replace('/', '\') -like "*$UProject*" }
function Get-ForeignBuilds {
    @(Get-EngineProcesses | Where-Object { $_.CommandLine -match 'UnrealBuildTool' -and $_.Name -match '^(dotnet|UnrealBuildTool)' })
}

# ---------------------------------------------------------------- queue plumbing
$Utf8 = New-Object Text.UTF8Encoding($false)
function Use-Lock([scriptblock] $Body) {
    New-Item -ItemType Directory -Force $QueueDir | Out-Null
    $Stream = $null
    $Deadline = (Get-Date).AddSeconds(30)
    while (-not $Stream) {
        try { $Stream = [IO.File]::Open((Join-Path $QueueDir "queue.lock"), 'OpenOrCreate', 'ReadWrite', 'None') }
        catch [IO.IOException] {
            if ((Get-Date) -gt $Deadline) { break }   # cannot happen with a live holder for 30 s; go on rather than hang
            Start-Sleep -Milliseconds (80 + (Get-Random -Maximum 160))
        }
    }
    try { & $Body } finally { if ($Stream) { $Stream.Dispose() } }
}
function Test-Alive($ProcessId, $StartTicks) {
    if (-not $ProcessId) { return $false }
    try {
        $P = Get-Process -Id $ProcessId -ErrorAction Stop
        if ($StartTicks) { return [math]::Abs($P.StartTime.Ticks - [int64] $StartTicks) -lt 20000000 }
        return $true
    } catch { return $false }
}
function Add-CsvRow($Row) {
    # Append under an exclusive open: two writers just retry. Header written when the file is new/empty.
    $Header = "worktree,requested,compile_start,end,wait_s,duration_s,result,waited_behind,priority"
    $Deadline = (Get-Date).AddSeconds(15)
    while ($true) {
        try {
            New-Item -ItemType Directory -Force (Split-Path $CsvPath) | Out-Null
            $Stream = [IO.File]::Open($CsvPath, 'OpenOrCreate', 'ReadWrite', 'None')
            try {
                $Stream.Seek(0, 'End') | Out-Null
                $Text = ""
                if ($Stream.Length -eq 0) { $Text = $Header + "`r`n" }
                $Text += $Row + "`r`n"
                $Bytes = $Utf8.GetBytes($Text)
                $Stream.Write($Bytes, 0, $Bytes.Length)
            } finally { $Stream.Dispose() }
            return
        } catch [IO.IOException] {
            if ((Get-Date) -gt $Deadline) { Write-Host "build : could not write $CsvPath (locked), row skipped"; return }
            Start-Sleep -Milliseconds (80 + (Get-Random -Maximum 160))
        }
    }
}
function Format-Time($Time) { if ($Time) { ([datetime] $Time).ToString("yyyy-MM-dd HH:mm:ss") } else { "" } }
function Log-Build($Name, $RequestedAt, $StartedAt, $EndedAt, $Result, $Behind, $Prio) {
    $Wait = if ($StartedAt) { [math]::Round(([datetime] $StartedAt - [datetime] $RequestedAt).TotalSeconds) } else { [math]::Round(([datetime] $EndedAt - [datetime] $RequestedAt).TotalSeconds) }
    $Duration = if ($StartedAt) { [math]::Round(([datetime] $EndedAt - [datetime] $StartedAt).TotalSeconds) } else { 0 }
    Add-CsvRow ((@($Name, (Format-Time $RequestedAt), (Format-Time $StartedAt), (Format-Time $EndedAt), $Wait, $Duration, $Result, ('"' + $Behind + '"'), $Prio)) -join ",")
}
function Read-Tickets {
    # Call inside the lock. Returns the readable tickets, newest schema; unreadable files are ignored.
    $List = @()
    foreach ($File in Get-ChildItem $QueueDir -Filter "*.json" -ErrorAction SilentlyContinue) {
        try { $T = Get-Content $File.FullName -Raw | ConvertFrom-Json; $T | Add-Member NoteProperty File $File.FullName -Force; $List += $T } catch { }
    }
    $List
}
function Remove-Ticket($Ticket, $Reason) {
    if ($Reason) { [IO.File]::WriteAllText("$($Ticket.File).gone", $Reason, $Utf8) }
    Remove-Item $Ticket.File -Force -ErrorAction SilentlyContinue
}
function Get-LastEnds {
    $Ends = @{}
    if (Test-Path $CsvPath) {
        try {
            foreach ($R in Import-Csv $CsvPath) {
                if ($R.result -in 'PASS', 'FAIL' -and $R.end) { $Ends[$R.worktree] = [datetime] $R.end }
            }
        } catch { }
    }
    $Ends
}
function Get-Order($Waiting) {
    # merge first; then copies with nothing compiled for 30 min; then first come first served.
    $Ends = Get-LastEnds
    $Now = Get-Date
    $Waiting | Sort-Object @{ Expression = {
            if ($_.priority -eq 'merge') { 0 }
            elseif (-not $Ends.ContainsKey($_.worktree) -or ($Now - $Ends[$_.worktree]).TotalMinutes -ge $StarvedMinutes) { 1 }
            else { 2 } } }, @{ Expression = { [datetime]::Parse($_.requested, [Globalization.CultureInfo]::InvariantCulture, 'RoundtripKind') } }, @{ Expression = { $_.id } }
}
function Clean-Dead($Tickets) {
    # Inside the lock. Drops tickets whose owner is gone (logging them) and returns the survivors.
    $Alive = @()
    foreach ($T in $Tickets) {
        if (Test-Alive $T.pid $T.pidStart) { $Alive += $T; continue }
        Remove-Ticket $T $null
        Log-Build $T.worktree $T.requested $(if ($T.state -eq 'compiling') { $T.started } else { $null }) (Get-Date) $(if ($T.state -eq 'compiling') { "FAIL" } else { "ORPHAN" }) "owner process died" $T.priority
    }
    $Alive
}

# ---------------------------------------------------------------- 1. this copy busy
$EngineProcesses = Get-EngineProcesses
$Unreal = @($EngineProcesses | Where-Object { $_.Name -match '^UnrealEditor' })
$Mine = @($Unreal | Where-Object { Is-ThisCopy $_ })
if ($Mine.Count -and -not $Force) {
    Write-Host "build : BUSY (Unreal runs in this copy, its DLLs are locked; wait for it or close it, or -Force):"
    $Mine | ForEach-Object { Write-Host "  pid $($_.ProcessId) $($_.Name) $(($_.CommandLine -replace '^.*?\.uproject"?\s*', '') -replace '^(.{0,120}).*', '$1')" }
    exit 2
}

# 2. Live Coding open in other copies only: skip UnrealBuildTool's guard (false alarm, see above).
$LiveCoding = @($Unreal | Where-Object { $_.CommandLine -notmatch '-LiveCoding=0' -and $_.CommandLine -notmatch '-run=' })
$Extra = @()
if ($LiveCoding.Count -and -not @($LiveCoding | Where-Object { Is-ThisCopy $_ }).Count) {
    $Extra += "-NoHotReloadFromIDE"
    Write-Host "Live Coding open elsewhere ($(($LiveCoding | ForEach-Object { Copy-Of $_ } | Select-Object -Unique) -join ', ')): its guard skipped"
}

# ---------------------------------------------------------------- 3. the queue
$Self = Get-Process -Id $PID
$ParentId = 0; $ParentStart = 0
if (-not $env:HOL_BUILD_NO_ORPHAN_CHECK) {
    try {
        $Parent = Get-Process -Id (Get-CimInstance Win32_Process -Filter "ProcessId=$PID").ParentProcessId -ErrorAction Stop
        if ($Parent.StartTime -le $Self.StartTime) { $ParentId = $Parent.Id; $ParentStart = $Parent.StartTime.Ticks }   # else: pid reused, unknown
    } catch { }   # parent already gone at launch: nothing to watch
}
$Ticket = [pscustomobject]@{
    id = [guid]::NewGuid().ToString("N"); root = $Project.ToLowerInvariant(); worktree = $Worktree
    pid = $PID; pidStart = $Self.StartTime.Ticks; parentPid = $ParentId; parentStart = $ParentStart
    priority = $Priority; requested = $Requested.ToString("o"); state = "waiting"; started = ""; wait = [bool]$Wait
}
$TicketFile = $null
$Behind = New-Object Collections.Generic.List[string]
$Outcome = $null   # PASS / FAIL / SUPERSEDED / ORPHAN, for the CSV
$Result = 0
$StartedAt = $null
$LastShown = ""

try {
    Use-Lock {
        $Others = Clean-Dead (Read-Tickets)
        # -Wait tickets are never replaced: this build queues behind them (it compiles after, quick when nothing changed)
        foreach ($Old in @($Others | Where-Object { $_.root -eq $Ticket.root -and $_.state -eq 'waiting' -and -not $_.wait })) {
            # one waiting build per copy: the newest replaces it, keeps its place in the line and the better priority
            if ($Old.priority -eq 'merge') { $Ticket.priority = 'merge' }
            if ([datetime]::Parse($Old.requested, [Globalization.CultureInfo]::InvariantCulture, 'RoundtripKind') -lt $Requested) {
                $Ticket.requested = $Old.requested
            }
            Remove-Ticket $Old "superseded"
        }
        $script:TicketFile = Join-Path $QueueDir "$($Ticket.requested -replace '[^0-9]', '')_$($Ticket.id).json"
        $Ticket | Add-Member NoteProperty File $TicketFile -Force
        [IO.File]::WriteAllText($TicketFile, ($Ticket | Select-Object * -ExcludeProperty File | ConvertTo-Json), $Utf8)
    }
    $RequestedAt = [datetime]::Parse($Ticket.requested, [Globalization.CultureInfo]::InvariantCulture, 'RoundtripKind')

    $Go = $false
    while (-not $Go) {
        $Foreign = if ($FakeBuild) { @() } else { Get-ForeignBuilds }
        $Decision = Use-Lock {
            $All = Clean-Dead (Read-Tickets)
            $Mine = @($All | Where-Object { $_.id -eq $Ticket.id })
            if (-not $Mine.Count) {
                $Gone = "$TicketFile.gone"
                $Why = if (Test-Path $Gone) { (Get-Content $Gone -Raw).Trim() } else { "orphan" }
                Remove-Item $Gone -Force -ErrorAction SilentlyContinue
                return @{ Kind = $Why }
            }
            if ($Ticket.parentPid -and -not (Test-Alive $Ticket.parentPid $Ticket.parentStart)) {
                Remove-Item $TicketFile -Force -ErrorAction SilentlyContinue
                return @{ Kind = "orphan" }
            }
            # Waiting builds whose parent died are dropped for everybody (their own loop then reports orphan).
            foreach ($W in @($All | Where-Object { $_.state -eq 'waiting' -and $_.id -ne $Ticket.id -and $_.parentPid })) {
                if (-not (Test-Alive $W.parentPid $W.parentStart)) { Remove-Ticket $W "orphan"; $All = @($All | Where-Object { $_.id -ne $W.id }) }
            }
            $Compiling = @($All | Where-Object { $_.state -eq 'compiling' })
            $Waiting = @(Get-Order @($All | Where-Object { $_.state -eq 'waiting' }))
            $Ahead = @($Compiling) + @($Waiting | Where-Object { $_.id -ne $Ticket.id } | Select-Object -First ([array]::IndexOf(@($Waiting.id), $Ticket.id)))
            if (-not $Compiling.Count -and $Waiting.Count -and $Waiting[0].id -eq $Ticket.id -and -not $Foreign.Count) {
                $Ticket.state = "compiling"; $Ticket.started = (Get-Date).ToString("o")
                [IO.File]::WriteAllText($TicketFile, ($Ticket | Select-Object * -ExcludeProperty File | ConvertTo-Json), $Utf8)
                return @{ Kind = "go"; Ahead = @() }
            }
            return @{ Kind = "wait"; Ahead = @($Ahead | ForEach-Object { $_.worktree }); Position = ([array]::IndexOf(@($Waiting.id), $Ticket.id) + 1); Of = $Waiting.Count }
        }
        switch ($Decision.Kind) {
            "go" { $Go = $true }
            "wait" {
                $Names = @($Decision.Ahead) + @($Foreign | ForEach-Object { Copy-Of $_ }) | Select-Object -Unique
                foreach ($N in $Names) { if ($N -and -not $Behind.Contains($N)) { $Behind.Add($N) } }
                $Shown = "$($Decision.Position)/$($Decision.Of) $($Names -join ',')"
                if ($Shown -ne $LastShown) {
                    Write-Host "queued behind the build of: $($Names -join ', ') (position $($Decision.Position) of $($Decision.Of) waiting; one build at a time on this engine; $Priority)"
                    $LastShown = $Shown
                }
                Start-Sleep -Seconds $Poll
            }
            "superseded" {
                Write-Host "build : superseded by a newer build of the same copy ($Worktree); this one never compiled"
                $Outcome = "SUPERSEDED"; $Result = 3; $Go = $null; break
            }
            default {
                Write-Host "build : orphan (the process that launched this build is gone); left the queue without compiling ($Worktree)"
                $Outcome = "ORPHAN"; $Result = 4; $Go = $null; break
            }
        }
        if ($null -eq $Go) { break }
    }

    if ($Outcome) {
        Log-Build $Worktree $RequestedAt $null (Get-Date) $Outcome ($Behind -join "+") $Ticket.priority
    } else {
        $StartedAt = Get-Date
        $Clock = [Diagnostics.Stopwatch]::StartNew()
        if ($FakeBuild) { & $FakeBuild *> $Log }
        else { & (Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat") $Target Win64 $Config "-Project=$UProject" -WaitMutex @Extra *> $Log }
        $Code = $LASTEXITCODE
        $Seconds = [math]::Round($Clock.Elapsed.TotalSeconds)
        $Text = Get-Content $Log
        $Outcome = if ($Code -eq 0) { "PASS" } else { "FAIL" }
        Log-Build $Worktree $RequestedAt $StartedAt (Get-Date) $Outcome ($Behind -join "+") $Ticket.priority
        if ($Code -eq 0) {
            Write-Host "build : PASS ($Target $Config, $Worktree, $Seconds s)"
            $Result = 0
        } else {
            Write-Host "build : FAIL ($Target $Config, $Worktree, $Seconds s, exit $Code; log $Log)"
            $Text | Select-String -Pattern 'error|Live Coding|conflicting instance' | Select-Object -First 15 | ForEach-Object { Write-Host "  $($_.Line.Trim())" }
            $Result = 1
        }
    }
} finally {
    if ($TicketFile -and (Test-Path $TicketFile)) { Use-Lock { Remove-Item $TicketFile -Force -ErrorAction SilentlyContinue } }
}
exit $Result
