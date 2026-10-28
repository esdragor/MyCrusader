# Upload measure (M6 R8c, plan line R8.6): what the home box must upload for a dedicated server, i.e. the server's
# outgoing bandwidth, per client and in total, in four cases run in turn through net_match.ps1:
#   (a) ordinary match, 4 clients   (b) S5 (S2 walk, 2000 units, D273), 4 clients
#   (c) ordinary match, 8 clients   (d) S5, 8 clients
# Bandwidth comes from the server's own replication reports (hol.Net.ReportInterval, every 2 s): the bytes each
# client connection really sent (UNetConnection::OutTotalBytes, all actors / RPCs / units, not the units alone) and
# its packets; the "wire" figure adds 28 B per packet (IPv4 + UDP headers), an upper bound used for the box line.
# Windows gives no per-process network counter and loopback never reaches the network card's counters, so the
# connection totals are the honest source; each client's received B/s (Net (display)) is printed as a cross-check.
# The clients all run on this PC (second screen, as net_match): their frame rates say nothing of the friends' PCs.
# Checks first that the machine is calm (no Unreal / build / server, processor calm), re-stages the cooked server if
# it is older than the game DLLs, then writes Saved_Tests\Logs\upload_measure_<date>.txt. ~13 min (+ ~10 min staging).
# Run from Windows PowerShell (never Git Bash):
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\upload_measure.ps1            (the four cases)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\upload_measure.ps1 -DryRun    (checks and plan only)
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\upload_measure.ps1 -Quick -Force   (2 clients, 200 units, short: proves the chain)
param(
    [switch] $DryRun,
    [switch] $Quick,
    # Runs even on a busy machine (the numbers are then worth less).
    [switch] $Force,
    [double] $MaxCpu = 15,
    # Steady part of an ordinary match, after its warm-up, and of S5 (the stress test's measure).
    [int] $Seconds = 60,
    [int] $Warmup = 20,
    [int] $Units = 2000
)
$Project = (Resolve-Path "$PSScriptRoot\..\..").Path
$NetMatch = Join-Path $PSScriptRoot "net_match.ps1"
$Logs = Join-Path $Project "Saved_Tests\Logs"
New-Item -ItemType Directory -Force $Logs | Out-Null
$Stamp = Get-Date -Format "MMdd_HHmm"
$ReportFile = Join-Path $Logs "upload_measure_$Stamp$(if ($Quick) { '_quick' }).txt"
$Interval = 2
# Unreal started from here skips the SDK setup of UBT (the other sessions build at the same time).
$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Tag = "Up"
$Clock = [Diagnostics.Stopwatch]::StartNew()
$Lines = @()
function Say([string] $Text) { Write-Host $Text; $script:Lines += $Text }

if ($Quick) {
    $Cases = @(@{ Name = "ordinary"; Clients = 2; Units = 0; Map = "/Game/Maps/Plain_Large" },
        @{ Name = "S5"; Clients = 2; Units = 200; Map = "" })
    $Seconds = 10; $Warmup = 6
}
else {
    $Cases = @(@{ Name = "ordinary"; Clients = 4; Units = 0; Map = "/Game/Maps/Plain_Large" },
        @{ Name = "S5"; Clients = 4; Units = $Units; Map = "" },
        @{ Name = "ordinary"; Clients = 8; Units = 0; Map = "/Game/Maps/Plain_Huge" },
        @{ Name = "S5"; Clients = 8; Units = $Units; Map = "" })
}

# 1. A calm machine: no Unreal, build or server running, the processor calm.
$Busy = Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(UnrealEditor|UnrealEditor-Cmd|UnrealBuildTool|UnrealInsights|ShaderCompileWorker|HeroesOfLiberty|AutomationTool)' }
$Cpu = (Get-Counter '\Processor(_Total)\% Processor Time' -SampleInterval 2 -MaxSamples 5).CounterSamples | ForEach-Object { $_.CookedValue }
$CpuMean = [math]::Round(($Cpu | Measure-Object -Average).Average, 1)
$Heavy = Get-Process | Where-Object { $_.Name -match '^(Discord|opera|chrome|msedge|firefox|GitKraken|Battle\.net|steam|RelicCardinal|AoE\w*|ShivaGet|IMF|DriverBooster)$' } |
    Select-Object -ExpandProperty Name -Unique
Write-Host "Processor: $CpuMean % on average over 10 s (limit $MaxCpu %)"
if ($Heavy) { Write-Host "Open and worth closing: $($Heavy -join ', ')" }
if ($Busy) { Write-Host "Running: $(($Busy | ForEach-Object { "$($_.ProcessId) $($_.Name)" }) -join ', ')" }
if (($Busy -or $CpuMean -gt $MaxCpu) -and -not $Force) {
    Write-Host "upload_measure : STOP - the machine is not calm (close Unreal, builds and the applications above, or run with -Force)"
    exit 2
}

# 2. The staged server current: re-staged when older than the game's code (as quick_check's lobby part says).
$Staged = @("Saved\StagedBuilds\WindowsServer\HeroesOfLibertyServer.exe",
    "Saved\StagedBuilds\WindowsServer\HeroesOfLiberty\Binaries\Win64\HeroesOfLibertyServer.exe") |
    ForEach-Object { Join-Path $Project $_ } | Where-Object { Test-Path $_ } | ForEach-Object { Get-Item $_ }
$GameModule = Get-ChildItem (Join-Path $Project "Binaries\Win64") -Filter "UnrealEditor-HeroesOfLiberty*.dll" -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTime | Select-Object -Last 1
$StagedTime = ($Staged | Sort-Object LastWriteTime | Select-Object -Last 1).LastWriteTime
$NeedStage = (-not $StagedTime) -or ($GameModule -and $StagedTime -lt $GameModule.LastWriteTime)

$Head = "$(git -C $Project log -1 --format='%h %s' 2>$null)"
if ($Head.Length -gt 80) { $Head = $Head.Substring(0, 77) + "..." }
Say "Upload measure $Stamp$(if ($Quick) { ' (QUICK: proves the chain, numbers not representative)' }) - $Project at $Head"
Say "  machine: processor $CpuMean % before the start$(if ($Busy) { " ; RUNNING: $(($Busy | Select-Object -ExpandProperty Name -Unique) -join ', ')" })$(if ($Heavy) { " ; open: $($Heavy -join ', ')" })"
Say "  staged server: $(if ($StagedTime) { $StagedTime.ToString('yyyy-MM-dd HH:mm') } else { 'none' }), code $(if ($GameModule) { $GameModule.LastWriteTime.ToString('yyyy-MM-dd HH:mm') } else { '?' }) -> $(if ($NeedStage) { 'RE-STAGE (net_match -StageOnly, ~10 min)' } else { 'current' })"
Say "  plan: $(($Cases | ForEach-Object { "$($_.Name) x$($_.Clients)$(if ($_.Units) { " ($($_.Units) units)" })" }) -join ', '); steady part $Seconds s, reports every $Interval s"
if ($DryRun) { Write-Host "Dry run: nothing staged or measured."; exit 0 }

if ($NeedStage) {
    Write-Host "Staging the server..."
    & powershell -ExecutionPolicy Bypass -File $NetMatch -StageOnly | ForEach-Object { Write-Host "  | $_" }
    if ($LASTEXITCODE -ne 0) { Say "upload_measure : FAIL - staging failed (Saved_Tests\Logs\net_stage.log)"; $Lines | Set-Content -Encoding utf8 $ReportFile; exit 1 }
}

function Stat([double[]] $Values) {
    if (-not $Values) { return @{ Mean = [double]::NaN; Peak = [double]::NaN } }
    return @{ Mean = ($Values | Measure-Object -Average).Average; Peak = ($Values | Measure-Object -Maximum).Maximum }
}
function Num([string] $Text) { return @($Text.Trim() -split '\s+' | Where-Object { $_ -and $_ -ne 'none' } | ForEach-Object { [double]::Parse($_, [Globalization.CultureInfo]::InvariantCulture) }) }
function Kb([double] $Bytes) { if ([double]::IsNaN($Bytes)) { return "?" }; return ([math]::Round($Bytes / 1000, 1)).ToString([Globalization.CultureInfo]::InvariantCulture) }

# Reads one case's logs: the server's reports with every client connected and no birth (after the start), the first
# $Skip of them dropped (warm-up), and the clients' display reports.
function Read-Case([int] $Clients, [int] $Skip) {
    $ServerLog = Join-Path $Logs "net_server$Tag.log"
    $Reports = @()
    $Simulation = $null
    if (Test-Path $ServerLog) {
        foreach ($Line in Get-Content $ServerLog) {
            if ($Line -match 'Simulation: ([\d.]+) ms average per step \(max ([\d.]+) ms' -and -not $Simulation) { $Simulation = "$($Matches[1]) ms (max $($Matches[2]))" }
            if ($Line -notmatch 'Net \(server\): (\d+) units, \d+ steps in [\d.]+ s, [\d.]+ sends per unit and per second \(birth (\d+)') { continue }
            $NumUnits = [int]$Matches[1]; $Births = [int]$Matches[2]
            if ($Births -gt 0 -or $Line -notmatch '; ([\d.]+) ms per step \(max ([\d.]+)\)') { continue }
            $StepMs = [double]::Parse($Matches[1], [Globalization.CultureInfo]::InvariantCulture); $StepMax = [double]::Parse($Matches[2], [Globalization.CultureInfo]::InvariantCulture)
            if ($Line -notmatch 'out B/s per client:([\d. a-z]+); packets/s:([\d. a-z]+)') { continue }
            $Out = Num $Matches[1]; $Packets = Num $Matches[2]
            if ($Out.Count -ne $Clients) { continue }
            $Reports += @{ Units = $NumUnits; Out = $Out; Packets = $Packets; StepMs = $StepMs; StepMax = $StepMax }
        }
    }
    $Reports = @($Reports | Select-Object -Skip $Skip)
    $Fps = @(); $In = @(); $ClientsSeen = 0
    for ($Client = 1; $Client -le $Clients; $Client++) {
        $ClientLog = Join-Path $Logs "net_client$Tag$Client.log"
        if (-not (Test-Path $ClientLog)) { continue }
        $Seen = $false
        foreach ($Line in Select-String -Path $ClientLog -Pattern 'Net \(display\):.* in ([\d.]+) B/s; ([\d.]+) fps') {
            $In += [double]::Parse($Line.Matches[0].Groups[1].Value, [Globalization.CultureInfo]::InvariantCulture)
            $Fps += [double]::Parse($Line.Matches[0].Groups[2].Value, [Globalization.CultureInfo]::InvariantCulture)
            $Seen = $true
        }
        if ($Seen) { $ClientsSeen++ }
    }
    return @{ Reports = $Reports; Simulation = $Simulation; Fps = $Fps; In = $In; ClientsSeen = $ClientsSeen }
}

# 3. The cases.
$Failed = $false
$Needed = @{}
foreach ($Case in $Cases) {
    $N = $Case.Clients
    $Label = "$($Case.Name) x$N$(if ($Case.Units) { " ($($Case.Units) units)" })"
    Write-Host "== $Label ..."
    $CaseClock = [Diagnostics.Stopwatch]::StartNew()
    $Verdict = ""
    $CpuText = ""
    if ($Case.Units -gt 0) {
        # S5: net_match waits for the stress test, closes everything and runs s5_report.py.
        $Output = @(& powershell -ExecutionPolicy Bypass -File $NetMatch -Clients $N -Units $Case.Units -Seconds $Seconds -Report -ReportInterval $Interval -Tag $Tag 2>&1 | ForEach-Object { "$_" })
        # A staged server without StressTest (staged by another tool, e.g. a release package: 2026-10-27) quits at once:
        # staged again by net_match (its maps include StressTest), then the case runs once more.
        if (Select-String -Path (Join-Path $Logs "net_server$Tag.log") -Pattern "Failed to enter /Game/Maps/StressTest" -Quiet -ErrorAction SilentlyContinue) {
            Say "  (staged server without StressTest: re-staged by net_match -StageOnly, ~10 min)"
            & powershell -ExecutionPolicy Bypass -File $NetMatch -StageOnly | ForEach-Object { Write-Host "  | $_" }
            $Output = @(& powershell -ExecutionPolicy Bypass -File $NetMatch -Clients $N -Units $Case.Units -Seconds $Seconds -Report -ReportInterval $Interval -Tag $Tag 2>&1 | ForEach-Object { "$_" })
        }
        $S5Line = @($Output | Where-Object { $_ -match '^S5 : ' }) | Select-Object -Last 1
        $Verdict = "s5_report: $(if ($S5Line) { $S5Line } else { 'none' })"
        $Pids = @($Output | Where-Object { $_ -match 'pid (\d+)' } | ForEach-Object { [int]([regex]::Match($_, 'pid (\d+)').Groups[1].Value) })
        $Skip = 1
    }
    else {
        # Ordinary: net_match leaves the match running; reports are waited for, then everything is closed.
        $Output = @(& powershell -ExecutionPolicy Bypass -File $NetMatch -Clients $N -Map $Case.Map -Report -ReportInterval $Interval -Tag $Tag 2>&1 | ForEach-Object { "$_" })
        $Pids = @($Output | Where-Object { $_ -match 'pid (\d+)' } | ForEach-Object { [int]([regex]::Match($_, 'pid (\d+)').Groups[1].Value) })
        $Skip = [math]::Ceiling($Warmup / $Interval)
        $Want = $Skip + [math]::Ceiling($Seconds / $Interval)
        $Deadline = (Get-Date).AddSeconds(180 + $Warmup + $Seconds)
        while ((Get-Date) -lt $Deadline) {
            if ((Read-Case $N 0).Reports.Count -ge $Want) { break }
            if ($Pids -and -not (Get-Process -Id $Pids[0] -ErrorAction SilentlyContinue)) { break }
            Start-Sleep -Seconds 2
        }
        $ServerProcess = if ($Pids) { Get-Process -Id $Pids[0] -ErrorAction SilentlyContinue }
        if ($ServerProcess) { $CpuText = "; server CPU $([math]::Round(100 * $ServerProcess.TotalProcessorTime.TotalSeconds / ((Get-Date) - $ServerProcess.StartTime).TotalSeconds)) % of one core" }
    }
    # Whatever net_match started is closed, whatever happened.
    foreach ($ProcessId in $Pids) { Stop-Process -Id $ProcessId -Force -ErrorAction SilentlyContinue }
    Start-Sleep -Seconds 2
    $Data = Read-Case $N $Skip
    $Totals = @($Data.Reports | ForEach-Object { ($_.Out | Measure-Object -Sum).Sum })
    $Wire = @($Data.Reports | ForEach-Object { ($_.Out | Measure-Object -Sum).Sum + 28 * ($_.Packets | Measure-Object -Sum).Sum })
    $PerClient = @($Data.Reports | ForEach-Object { $_.Out } )
    $Total = Stat $Totals; $WireStat = Stat $Wire; $Client = Stat $PerClient
    $Step = Stat @($Data.Reports | ForEach-Object { $_.StepMs }); $StepMax = (Stat @($Data.Reports | ForEach-Object { $_.StepMax })).Peak
    $Fps = Stat $Data.Fps; $In = Stat $Data.In
    $MatchOk = ($Data.Reports.Count -ge 2) -and ($Data.ClientsSeen -eq $N) -and (($Case.Units -eq 0) -or ($Verdict -match 'S5 : (PASS|FAIL)'))
    $Failed = $Failed -or -not $MatchOk
    # The most units seen: the stress test removes its units before its last report (0 units, 2026-10-28).
    $UnitsSeen = if ($Data.Reports) { ($Data.Reports | ForEach-Object { $_.Units } | Measure-Object -Maximum).Maximum } else { "?" }
    Say ""
    Say "== $Label : match $(if ($MatchOk) { 'PASS' } else { 'FAIL' }) in $([math]::Round($CaseClock.Elapsed.TotalMinutes, 1)) min, pids $($Pids -join ' ') closed"
    Say "  clients $N, units replicated $UnitsSeen, $($Data.Reports.Count) server reports of $Interval s in the steady part"
    Say "  server out total: mean $(Kb $Total.Mean) KB/s, peak $(Kb $Total.Peak) KB/s ; with IP/UDP headers (28 B/packet): mean $(Kb $WireStat.Mean), peak $(Kb $WireStat.Peak) KB/s"
    Say "  server out per client: mean $(Kb $Client.Mean) KB/s, peak $(Kb $Client.Peak) KB/s ; clients received $(Kb $In.Mean) KB/s on average (cross-check)"
    Say "  server: replication $([math]::Round($Step.Mean, 3)) ms per step (max $([math]::Round($StepMax, 3)))$(if ($Data.Simulation) { ", simulation $($Data.Simulation) per step" })$CpuText"
    Say "  clients (all on this PC, not representative): $([math]::Round($Fps.Mean, 1)) fps on average, lowest report $([math]::Round((($Data.Fps | Measure-Object -Minimum).Minimum), 1)) fps ; $($Data.ClientsSeen)/$N clients reported"
    if ($Verdict) { Say "  $Verdict" }
    $Key = "$N"
    if (-not $Needed.ContainsKey($Key) -or $WireStat.Peak -gt $Needed[$Key]) { $Needed[$Key] = $WireStat.Peak }
}

# 4. What the box must upload: the worst peak (with headers) of the cases of each number of players.
Say ""
$Parts = @($Needed.Keys | Sort-Object { [int]$_ } | ForEach-Object { "$(Kb $Needed[$_]) KB/s for $_ players ($(([math]::Round($Needed[$_] * 8 / 1e6, 2)).ToString([Globalization.CultureInfo]::InvariantCulture)) Mbit/s)" })
Say "box upload needed: $($Parts -join ', ') - worst 2 s peak with IP/UDP headers, one match; KB = 1000 B"
Say "upload_measure : $(if ($Failed) { 'FAIL' } else { 'PASS' }) in $([math]::Round($Clock.Elapsed.TotalMinutes, 1)) min - report $ReportFile"
$Lines | Set-Content -Encoding utf8 $ReportFile
exit $(if ($Failed) { 1 } else { 0 })
