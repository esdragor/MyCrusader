# Max servers measure (M6, D502 remeasure): how many match servers this PC can host at once, i.e. the manager's
# MaxServers. R5.5 (2026-10-25) measured two battles with the clients' full games on the same PC (4 editor windows,
# 9.4 GB, most of the load): the real worst case is the user's own game plus X servers, the friends' games being on
# their PCs. So here each count X of -Counts runs X dedicated servers at once, each in the S3 battle (1000 against 1000
# in melee, the heaviest simulation and the worst case for the health sent per hit, D342 / J7c.4), each with -Clients
# light clients (net_match.ps1 -Headless: no renderer, no sound, no window) that join and receive the match like a
# player's game at a fraction of its cost. Meanwhile the PC's processor, its free memory and each process (working set,
# processor seconds) are sampled every 2 s. Per count, the report gives per server the simulation step (mean, max),
# the replication step, the bytes sent per client (D450: <= 34 KB/s) and the server's processor share, and for the PC
# the processor mean and peak over the measure window (the last -Seconds + 10 s of the servers' lives: the battles).
# Verdict per count: "fits" when every server's S5 report passes, its mean simulation step is within -StepBudget ms
# (25 by default: half of the 50 ms step, the other half left to replication and jitter) and the PC's processor over
# the window, the light clients' processor seconds taken off (they stand for the friends' games, run elsewhere), stays
# within -CpuBudget % (70 by default: what remains, ~5 of 16 logical processors, is the user's own game). The
# recommendation is the largest count that fits; MaxServers is the user's decision (D502).
# Checks first that the machine is calm (no Unreal / build / server, processor calm), re-stages the cooked server when
# it is older than the game DLLs, then writes Saved_Tests\Logs\max_servers_<date>.txt (+ .csv of the samples).
# ~3 min per count (+ ~2 min staging). Run from Windows PowerShell (never Git Bash):
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\max_servers_measure.ps1                 (1, 2, 3, 4 servers)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\max_servers_measure.ps1 -Counts 2,3      (chosen counts)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\max_servers_measure.ps1 -DryRun          (checks and plan only)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\max_servers_measure.ps1 -Quick -Force    (1 server, 200 units, short: proves the chain)
param(
    [int[]] $Counts = @(1, 2, 3, 4),
    [switch] $DryRun,
    [switch] $Quick,
    # Runs even on a busy machine (the numbers are then worth less).
    [switch] $Force,
    [double] $MaxCpu = 15,
    # Units of each battle (both armies together: 2000 = 1000 against 1000), light clients per server, measure length.
    [int] $Units = 2000,
    [int] $Clients = 2,
    [int] $Seconds = 20,
    # Budgets of the verdict: mean simulation step of each server (ms), PC processor over the measure window (%).
    [double] $StepBudget = 25,
    [double] $CpuBudget = 70
)
$Project = (Resolve-Path "$PSScriptRoot\..\..").Path
$NetMatch = Join-Path $PSScriptRoot "net_match.ps1"
. (Join-Path $PSScriptRoot "copy_ports.ps1")
$Logs = Join-Path $Project "Saved_Tests\Logs"
New-Item -ItemType Directory -Force $Logs | Out-Null
$Stamp = Get-Date -Format "MMdd_HHmm"
$ReportFile = Join-Path $Logs "max_servers_$Stamp$(if ($Quick) { '_quick' }).txt"
$SamplesFile = Join-Path $Logs "max_servers_$Stamp$(if ($Quick) { '_quick' }).csv"
# Unreal started from here skips the SDK setup of UBT (the other sessions build at the same time).
$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Clock = [Diagnostics.Stopwatch]::StartNew()
$Lines = @()
function Say([string] $Text) { Write-Host $Text; $script:Lines += $Text }
function N([double] $Value, [int] $Digits = 1) { ([math]::Round($Value, $Digits)).ToString([Globalization.CultureInfo]::InvariantCulture) }

if ($Quick) { $Counts = @(1); $Units = 200; $Seconds = 10 }
# The servers' UDP ports: this copy's lobby-test range (copy_ports.ps1), one per server of the largest count.
$CopyIndex = Get-CopyIndex $Project
$FirstPort = 7900 + 10 * $CopyIndex

# 1. A calm machine: no Unreal, build or server running, the processor calm.
$Busy = Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(UnrealEditor(-Cmd)?|UnrealBuildTool|UnrealInsights|ShaderCompileWorker|HeroesOfLiberty\w*|AutomationTool|cl|link|MSBuild)\.exe$' }
$Cpu = (Get-Counter '\Processor(_Total)\% Processor Time' -SampleInterval 2 -MaxSamples 5).CounterSamples | ForEach-Object { $_.CookedValue }
$CpuMean = [math]::Round(($Cpu | Measure-Object -Average).Average, 1)
$Heavy = Get-Process | Where-Object { $_.Name -match '^(Discord|opera|chrome|msedge|firefox|GitKraken|Battle\.net|steam|RelicCardinal|AoE\w*|ShivaGet|IMF|DriverBooster)$' } |
    Select-Object -ExpandProperty Name -Unique
$Cores = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
Say "max_servers_measure : $(Get-Date -Format 'yyyy-MM-dd HH:mm'), $Project, copy $CopyIndex (servers on UDP $FirstPort+), $Cores logical processors"
Say "Processor: $CpuMean % on average over 10 s (limit $MaxCpu %)"
if ($Busy) { Say "Busy: $(($Busy | ForEach-Object { "$($_.Name) ($($_.ProcessId))" }) -join ', ')" }
if ($Heavy) { Say "Heavy apps open: $($Heavy -join ', ') (not representative of a calm PC)" }
$Calm = -not $Busy -and $CpuMean -le $MaxCpu
if (-not $Calm -and -not $Force) { Say "max_servers_measure : the machine is not calm; close what runs, or -Force"; $Lines | Set-Content -Encoding utf8 $ReportFile; exit 1 }
if (-not $Calm) { Say "(forced on a busy machine: the numbers are worth less)" }

# 2. The staged server: at least as new as the game's DLLs (the server's code), else re-staged.
$Server = Join-Path $Project "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Binaries\Win64\HeroesOfLibertyServer.exe"
$Dlls = Get-ChildItem (Join-Path $Project "Binaries\Win64") -Filter "UnrealEditor-HeroesOfLiberty*.dll" -ErrorAction SilentlyContinue
$Newest = ($Dlls | Measure-Object LastWriteTime -Maximum).Maximum
$NeedStage = -not (Test-Path $Server) -or ($Newest -and (Get-Item $Server).LastWriteTime -lt $Newest)
Say "Staged server: $(if (Test-Path $Server) { (Get-Item $Server).LastWriteTime.ToString('yyyy-MM-dd HH:mm') } else { 'none' }); game DLLs: $(if ($Newest) { $Newest.ToString('yyyy-MM-dd HH:mm') } else { '?' }) -> $(if ($NeedStage) { 're-stage' } else { 'up to date' })"
Say "Plan: counts $($Counts -join ', ') ; per server: S3 with $Units units, $Clients light clients, measure $Seconds s ; budgets: step <= $StepBudget ms, PC <= $CpuBudget %"
if ($DryRun) { Say "max_servers_measure : dry run, nothing started"; $Lines | Set-Content -Encoding utf8 $ReportFile; exit 0 }
if ($NeedStage) {
    & powershell -ExecutionPolicy Bypass -File $NetMatch -StageOnly | ForEach-Object { Write-Host "  | $_" }
    if ($LASTEXITCODE -ne 0) { Say "max_servers_measure : staging FAILED"; $Lines | Set-Content -Encoding utf8 $ReportFile; exit 1 }
}

# 3. Each count: X net_match runs at once (one server + its light clients each), sampled every 2 s until they end.
"time,count,kind,pid,ws_mb,cpu_seconds,cpu_total,free_mb" | Set-Content $SamplesFile
$CpuCounter = New-Object System.Diagnostics.PerformanceCounter("Processor", "% Processor Time", "_Total")
$null = $CpuCounter.NextValue()
$Results = @()
$Failed = $false
$Window = $Seconds + 10
foreach ($Count in $Counts) {
    $CountClock = [Diagnostics.Stopwatch]::StartNew()
    Say ""
    Say "== $Count server$(if ($Count -gt 1) { 's' }) : starting"
    $Runs = @()
    for ($i = 1; $i -le $Count; $i++) {
        $Tag = "Mx$($Count)_$i"
        $Port = $FirstPort + $i - 1
        $Out = Join-Path $Logs "max_servers_$Tag.out"
        $Arguments = "-ExecutionPolicy Bypass -File `"$NetMatch`" -Headless -Clients $Clients -Units $Units -Test S3 -Seconds $Seconds -Port $Port -Tag $Tag"
        $Process = Start-Process powershell -ArgumentList $Arguments -PassThru -WindowStyle Hidden -RedirectStandardOutput $Out
        $Runs += @{ Tag = $Tag; Port = $Port; Process = $Process; Out = $Out; ServerLog = (Join-Path $Logs "net_server$Tag.log") }
        Write-Host "  run $i : net_match pid $($Process.Id), port $Port, tag $Tag"
    }
    # Samples: PC processor and free memory; this project's servers and the headless clients of these tags.
    $Samples = @()
    $Deadline = (Get-Date).AddSeconds(60 + 40 + $Seconds + 120 + 60)
    while ($true) {
        Start-Sleep -Seconds 2
        $Now = Get-Date
        $C = [math]::Round($CpuCounter.NextValue(), 1)
        $Free = [math]::Round((Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory / 1KB)
        $Servers = @(Get-Process HeroesOfLibertyServer -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "$Project*" })
        $Editors = @(Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" | Where-Object { $_.CommandLine -like "*Tests_NetMx$($Count)_*" })
        $Sample = @{ Time = $Now; Cpu = $C; Free = $Free; Servers = @{}; ClientsWs = 0; ClientsCpu = 0.0; ClientCount = $Editors.Count }
        foreach ($S in $Servers) {
            $Sample.Servers[$S.Id] = @{ Ws = [math]::Round($S.WorkingSet64 / 1MB); CpuSeconds = $S.TotalProcessorTime.TotalSeconds; Start = $S.StartTime }
            "$($Now.ToString('HH:mm:ss')),$Count,server,$($S.Id),$($Sample.Servers[$S.Id].Ws),$(N $S.TotalProcessorTime.TotalSeconds 2),$C,$Free" | Add-Content $SamplesFile
        }
        foreach ($E in $Editors) {
            $Sample.ClientsWs += [math]::Round($E.WorkingSetSize / 1MB)
            $Sample.ClientsCpu += ($E.KernelModeTime + $E.UserModeTime) / 1e7
        }
        if ($Editors.Count) { "$($Now.ToString('HH:mm:ss')),$Count,clients,$($Editors.Count),$($Sample.ClientsWs),$(N $Sample.ClientsCpu 2),$C,$Free" | Add-Content $SamplesFile }
        if (-not $Servers.Count -and -not $Editors.Count) { "$($Now.ToString('HH:mm:ss')),$Count,none,0,0,0,$C,$Free" | Add-Content $SamplesFile }
        $Samples += $Sample
        $Alive = @($Runs | Where-Object { -not $_.Process.HasExited })
        if (-not $Alive.Count) { break }
        if ($Now -gt $Deadline) {
            Say "  timeout: $($Alive.Count) net_match run(s) still alive, killed"
            foreach ($Run in $Alive) { Stop-Process -Id $Run.Process.Id -Force -ErrorAction SilentlyContinue }
            Get-Process HeroesOfLibertyServer -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "$Project*" } | Stop-Process -Force -ErrorAction SilentlyContinue
            Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" | Where-Object { $_.CommandLine -like "*Tests_NetMx$($Count)_*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
            break
        }
    }
    # Per server: s5_report's lines (written by net_match into the .out) and its processor share over its last $Window s.
    $CountOk = $true
    $Steps = @()
    $Shares = @()
    $ServerRows = @()
    $FirstExit = $null
    foreach ($Run in $Runs) {
        $Output = if (Test-Path $Run.Out) { Get-Content $Run.Out } else { @() }
        $S5 = @($Output | Where-Object { $_ -match '^S5 : ' }) | Select-Object -Last 1
        $Pass = $S5 -match 'PASS'
        $ServerLine = @($Output | Where-Object { $_ -match 'Server: replication' }) | Select-Object -Last 1
        $OutLine = @($Output | Where-Object { $_ -match 'Server: out per client' }) | Select-Object -Last 1
        $Sim = [regex]::Match("$ServerLine", 'simulation ([\d.]+) ms per step \(max ([\d.]+)')
        $Rep = [regex]::Match("$ServerLine", 'replication ([\d.]+) ms per step \(max ([\d.]+)')
        $SimMean = if ($Sim.Success) { [double]$Sim.Groups[1].Value } else { [double]::NaN }
        $SimMax = if ($Sim.Success) { [double]$Sim.Groups[2].Value } else { [double]::NaN }
        $PerClient = [regex]::Match("$OutLine", 'out per client ([\d. /]+) B/s')
        $Bytes = if ($PerClient.Success) { @($PerClient.Groups[1].Value -split ' / ' | ForEach-Object { [double]$_ }) } else { @() }
        $BytesMax = if ($Bytes.Count) { ($Bytes | Measure-Object -Maximum).Maximum } else { [double]::NaN }
        # The server's pid from net_match's first line; its processor share over the last $Window s of its samples.
        $ServerPid = [regex]::Match(($Output -join "`n"), 'server: pid (\d+)').Groups[1].Value
        $Share = [double]::NaN; $Ws = 0; $Exit = $null
        if ($ServerPid) {
            $Mine = @($Samples | Where-Object { $_.Servers.ContainsKey([int]$ServerPid) })
            if ($Mine.Count -ge 2) {
                $Last = $Mine[-1]; $Exit = $Last.Time
                $From = @($Mine | Where-Object { $_.Time -le $Last.Time.AddSeconds(-$Window) }) | Select-Object -Last 1
                if (-not $From) { $From = $Mine[0] }
                $Span = ($Last.Time - $From.Time).TotalSeconds
                if ($Span -gt 0) { $Share = 100 * ($Last.Servers[[int]$ServerPid].CpuSeconds - $From.Servers[[int]$ServerPid].CpuSeconds) / $Span }
                $Ws = ($Mine | ForEach-Object { $_.Servers[[int]$ServerPid].Ws } | Measure-Object -Maximum).Maximum
            }
        }
        if ($Exit -and (-not $FirstExit -or $Exit -lt $FirstExit)) { $FirstExit = $Exit }
        $Ok = $Pass -and -not [double]::IsNaN($SimMean) -and $SimMean -le $StepBudget
        $CountOk = $CountOk -and $Ok
        if (-not [double]::IsNaN($SimMean)) { $Steps += $SimMean }
        $Shares += $Share
        $ServerRows += "  server $($Run.Tag) (port $($Run.Port)): simulation $(N $SimMean 2) ms per step (max $(N $SimMax 1)), replication $(if ($Rep.Success) { "$($Rep.Groups[1].Value) ms (max $($Rep.Groups[2].Value))" } else { '?' }), out per client $(if ($Bytes.Count) { ($Bytes | ForEach-Object { N ($_ / 1000) 1 }) -join ' / ' } else { '?' }) KB/s, CPU $(N $Share 0) % of one core over its last $Window s, working set $Ws MB -> $(if ($Ok) { 'ok' } else { 'NOT ok' }) ($(if ($S5) { $S5 } else { 'no S5 report' }))"
    }
    # The PC over the measure window: the last $Window s before the first server's exit (the battles, all at once).
    $WindowSamples = @()
    if ($FirstExit) { $WindowSamples = @($Samples | Where-Object { $_.Time -le $FirstExit -and $_.Time -ge $FirstExit.AddSeconds(-$Window) }) }
    $PcMean = if ($WindowSamples.Count) { ($WindowSamples | ForEach-Object { $_.Cpu } | Measure-Object -Average).Average } else { [double]::NaN }
    $PcPeak = if ($WindowSamples.Count) { ($WindowSamples | ForEach-Object { $_.Cpu } | Measure-Object -Maximum).Maximum } else { [double]::NaN }
    $AllPeak = if ($Samples.Count) { ($Samples | ForEach-Object { $_.Cpu } | Measure-Object -Maximum).Maximum } else { [double]::NaN }
    $FreeMin = if ($Samples.Count) { ($Samples | ForEach-Object { $_.Free } | Measure-Object -Minimum).Minimum } else { 0 }
    $ClientsWs = if ($WindowSamples.Count) { ($WindowSamples | ForEach-Object { $_.ClientsWs } | Measure-Object -Maximum).Maximum } else { 0 }
    $ClientsShare = [double]::NaN
    if ($WindowSamples.Count -ge 2) {
        $Span = ($WindowSamples[-1].Time - $WindowSamples[0].Time).TotalSeconds
        if ($Span -gt 0) { $ClientsShare = 100 * ($WindowSamples[-1].ClientsCpu - $WindowSamples[0].ClientsCpu) / $Span }
    }
    # The PC without the light clients (they stand for the friends' games, which run elsewhere): their processor
    # seconds taken off the PC's mean; the servers' own total is given beside it as a cross-check.
    $PcNoClients = if ([double]::IsNaN($ClientsShare)) { $PcMean } else { $PcMean - $ClientsShare / $Cores }
    $ServersShare = ($Shares | Where-Object { -not [double]::IsNaN($_) } | Measure-Object -Sum).Sum
    $Fits = $CountOk -and -not [double]::IsNaN($PcNoClients) -and $PcNoClients -le $CpuBudget
    $Failed = $Failed -or -not $CountOk
    Say "== $Count server$(if ($Count -gt 1) { 's' }) : $(if ($Fits) { 'FITS' } else { 'does not fit' }) in $(N $CountClock.Elapsed.TotalMinutes 1) min"
    $ServerRows | ForEach-Object { Say $_ }
    Say "  PC over the window ($Window s before the first server's end, $($WindowSamples.Count) samples): processor mean $(N $PcMean 0) % (peak $(N $PcPeak 0) %; whole run peak $(N $AllPeak 0) %, loading included), free memory lowest $([math]::Round($FreeMin / 1024, 1)) GB"
    Say "  light clients ($($Clients * $Count), all on this PC): $ClientsWs MB together, $(N $ClientsShare 0) % of one core together over the window (not a player's game: no renderer)"
    Say "  PC without the light clients: $(N $PcNoClients 0) % (budget $CpuBudget %) ; the servers alone: $(N $ServersShare 0) % of one core = $(N ($ServersShare / $Cores) 1) % of the PC"
    $Results += @{ Count = $Count; Fits = $Fits; StepMean = $(if ($Steps.Count) { ($Steps | Measure-Object -Average).Average } else { [double]::NaN }); PcMean = $PcNoClients }
}

# 4. Recommendation: the largest count that fits, the counts above it not fitting.
Say ""
$Fitting = @($Results | Where-Object { $_.Fits } | ForEach-Object { $_.Count })
$Best = if ($Fitting.Count) { ($Fitting | Measure-Object -Maximum).Maximum } else { 0 }
Say "counts measured: $(($Results | ForEach-Object { "$($_.Count) -> $(if ($_.Fits) { 'fits' } else { 'no' }) (step $(N $_.StepMean 1) ms, PC $(N $_.PcMean 0) %)" }) -join ' ; ')"
Say "MaxServers recommended: $Best (largest count whose servers all keep a mean simulation step <= $StepBudget ms with S5 PASS and the PC without the light clients <= $CpuBudget % ; the user's own game is the margin left)"
Say "max_servers_measure : $(if ($Failed) { 'FAIL' } else { 'PASS' }) in $(N $Clock.Elapsed.TotalMinutes 1) min - report $ReportFile, samples $SamplesFile"
$Lines | Set-Content -Encoding utf8 $ReportFile
exit $(if ($Failed) { 1 } else { 0 })
