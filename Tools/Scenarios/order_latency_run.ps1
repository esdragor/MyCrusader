# L2 M1 (2026-11-18): LATENCY of the 16 player orders, RPC C++ (hol.Rpc.OrderScript 0) versus RPC script (hol.Rpc.OrderScript 1). Criterion A of D846 (local):
# median AND p95 of (script - C++) below 1 simulation step (-StepMs, 50 ms), 100 % received, 0 inversion, no negative delta (same clock on both processes).
# Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\order_latency_run.ps1 -Root <worktree> [-Units 1|50|200]   (several sizes: -Units 1,50,200)
# Needs the C++ lot (OrderSend / OrderRecv lines behind hol.Rpc.OrderTrace) AND the script pilot (hol.Rpc.OrderLatency) built / loaded. Nothing here is visible: nullrhi, hidden windows.
#
# ===== CONTRACT WITH C1 (C++ controller), journal lines, written ONLY when hol.Rpc.OrderTrace is 1 (default 0) =====
# Clock = FPlatformTime::Seconds() (same origin on both processes of the machine), printed with 6 decimals. Canonical lines (LogHOLMatch or any category, Display):
#   client, first line of each Request<X> after the guards and BEFORE the cvar branch (same in both modes):
#     OrderSend: <Order> key <K> units <n> mode <0|1> t <seconds>
#   server, first line of each Do<X> (common point of both modes):
#     OrderRecv: <Order> key <K> units <n> t <seconds>
#   <Order> is one word: Move AttackUnit HealUnit AttackBuilding AttackMove Stop ToggleDeploy MoveInLine Gather PlaceBuilding BuildUnits DropOff Garrison UngarrisonAll CancelBuilding SetRallyPoint.
#   <K> = X of the single target point as an integer (Move: Target.X, MoveInLine: Start.X, SetRally and PlaceBuilding: Point.X); an order with NO point
#   (Stop, ToggleDeploy, BuildUnits, Garrison, UngarrisonAll, CancelBuilding, ...) prints key 0 (the order scenario then pairs them by occurrence).
#   The parser also accepts the card form: OrderSend: kind=Move key=1005 units=50 mode=1 t=123.456 (the "kind=", "units", "mode" parts are optional; without "mode" the
#   mode is the last "OrderLatency: mode <m>" marker of the client log).
# ===== CONTRACT WITH A2 (script pilot, client only, green client) =====
#   Command line given by this scenario on the green client: -ExecCmds="hol.Rpc.OrderTrace 1,hol.Rpc.OrderLatency <N>"   (N = units of the player, 1 | 50 | 200)
#   The pilot waits until the local player has N units visible and the connection is up, then (state machine in a Tick, ONE order per tick, no burst), 4 blocks of 100 Move
#   (A/B/A/B: blocks 0 and 2 with hol.Rpc.OrderScript 0, blocks 1 and 3 with hol.Rpc.OrderScript 1, set by System::ExecuteConsoleCommand), keys 1000 .. 1399 (one Move
#   per key, target X = key, via hol.Units.Order <key> <Y> which sends ALL the N units), 2 s pause between blocks. It logs, in the client log (Angelscript Log):
#     OrderLatency: start units <n> orders 400
#     OrderLatency: mode <0|1> block <b>        (after the cvar is set, before the block's first order)
#     OrderLatency: done sent <k>
#   The scenario waits for "OrderLatency: done" (-TimeoutSec), then for 5 s of server flush.
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [int[]]$Units = @(50), [string]$Out = (Join-Path $env:TEMP "order_latency"), [double]$StepMs = 50, [int]$TimeoutSec = 240, [int]$Port = 7993)
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$inv = [Globalization.CultureInfo]::InvariantCulture
$rxSend = [regex]'OrderSend:? (?:kind=)?(\w+) key[ =](-?\d+)(?: units[ =](\d+))?(?: mode[ =](\d))? t[ =]([0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)'
$rxRecv = [regex]'OrderRecv:? (?:kind=)?(\w+) key[ =](-?\d+)(?: units[ =](\d+))?(?: mode[ =](\d))? t[ =]([0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)'
$rxMode = [regex]'OrderLatency: mode (\d)'

function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
function Get-Median($v) { $s = @($v | Sort-Object); $n = $s.Count; if ($n -eq 0) { return [double]::NaN }; if ($n % 2 -eq 1) { return [double]$s[($n - 1) / 2] }; return ([double]$s[$n / 2 - 1] + [double]$s[$n / 2]) / 2 }
function Get-Pct($v, [double]$p) { $s = @($v | Sort-Object); $n = $s.Count; if ($n -eq 0) { return [double]::NaN }; $i = [int][Math]::Ceiling($p * $n) - 1; if ($i -lt 0) { $i = 0 }; return [double]$s[$i] }
function Get-Max($v) { $m = [double]::NaN; foreach ($x in $v) { if ([double]::IsNaN($m) -or $x -gt $m) { $m = [double]$x } }; return $m }
function Fmt($x) { if ([double]::IsNaN($x)) { return 'n/a' }; return $x.ToString('0.000', $inv) }
function Count-Inversions($seq) { $c = 0; $a = @($seq); for ($i = 0; $i -lt $a.Count; $i++) { for ($j = $i + 1; $j -lt $a.Count; $j++) { if ($a[$i] -gt $a[$j]) { $c++ } } }; return $c }

# Reads the client log (sends, in file order) and the server log (receives, in file order); identity = Order|Key, or Order|#occurrence for key 0.
function Read-Sends($file) {
    $list = New-Object System.Collections.ArrayList; $cur = -1; $occ = @{}
    foreach ($line in [IO.File]::ReadLines($file)) {
        $mm = $rxMode.Match($line); if ($mm.Success) { $cur = [int]$mm.Groups[1].Value; continue }
        $m = $rxSend.Match($line); if (-not $m.Success) { continue }
        $kind = $m.Groups[1].Value; $key = [int]$m.Groups[2].Value
        if ($key -eq 0) { if (-not $occ.ContainsKey($kind)) { $occ[$kind] = 0 }; $occ[$kind]++; $id = "$kind|#$($occ[$kind])" } else { $id = "$kind|$key" }
        $mode = $cur; if ($m.Groups[4].Success) { $mode = [int]$m.Groups[4].Value }
        [void]$list.Add([pscustomobject]@{ Id = $id; Kind = $kind; Mode = $mode; T = [double]::Parse($m.Groups[5].Value, $inv); Idx = $list.Count })
    }
    return , $list
}
function Read-Recvs($file) {
    $list = New-Object System.Collections.ArrayList; $occ = @{}
    foreach ($line in [IO.File]::ReadLines($file)) {
        $m = $rxRecv.Match($line); if (-not $m.Success) { continue }
        $kind = $m.Groups[1].Value; $key = [int]$m.Groups[2].Value
        if ($key -eq 0) { if (-not $occ.ContainsKey($kind)) { $occ[$kind] = 0 }; $occ[$kind]++; $id = "$kind|#$($occ[$kind])" } else { $id = "$kind|$key" }
        [void]$list.Add([pscustomobject]@{ Id = $id; T = [double]::Parse($m.Groups[5].Value, $inv) })
    }
    return , $list
}

function Run-Size([int]$n) {
    $tag = "n$n"
    $cfg = Join-Path $Out "latency_$tag.json"
    # startingPeasants = N: no cap seen in the match config (only Max(x, -1)); the 200-unit cap of an order is HOLPlayerOrders::IsAccepted. Both seats get N.
    $cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = $n; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
    Set-Content -Encoding ascii $cfg $cfgJson
    $sl = "$Out\latency_${tag}_server.log"; $c1 = "$Out\latency_${tag}_client_red.log"; $c2 = "$Out\latency_${tag}_client_green.log"
    Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
    $procs = @()
    try {
        "[$tag] start $(Get-Date -Format HH:mm:ss)"
        $sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$Port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=LatSrv -abslog=`"$sl`" -ExecCmds=`"hol.Rpc.OrderTrace 1`"" -PassThru -WindowStyle Hidden
        $procs += $sv
        "[$tag] server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 120) $(Get-Date -Format HH:mm:ss)"
        $a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${Port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=LatCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
        $procs += $a
        "[$tag] red client connected (no order): $(WaitFor $c1 'Welcomed by server' 90)"
        Start-Sleep 5
        $b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${Port}?Name=Alice -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=LatCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Rpc.OrderTrace 1,hol.Rpc.OrderLatency $n`"" -PassThru -WindowStyle Hidden
        $procs += $b
        $started = WaitFor $c2 'OrderLatency: start' 120
        "[$tag] green pilot started: $started"
        $done = $started -and (WaitFor $c2 'OrderLatency: done' $TimeoutSec)
        "[$tag] green pilot done: $done $(Get-Date -Format HH:mm:ss)"
        Start-Sleep 5
    } finally {
        foreach ($p in $procs) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
    }
    if (-not (Test-Path $c2) -or -not (Test-Path $sl)) { "[$tag] FAIL: no log"; $script:lastOk = $false; return }
    $seenUnits = ''; $um = Select-String -Path $c2 -Pattern 'OrderLatency: start units (\d+)' | Select-Object -First 1
    if ($um) { $seenUnits = $um.Matches[0].Groups[1].Value }
    "[$tag] pilot saw units: '$seenUnits' (expected $n)  server Order lines: $((Select-String -Path $c2 -Pattern 'Order: \d+ units' | Measure-Object).Count)"
    $sends = Read-Sends $c2; $recvs = Read-Recvs $sl
    "[$tag] OrderSend lines: $($sends.Count)  OrderRecv lines: $($recvs.Count)"
    $sendById = @{}; foreach ($s in $sends) { $sendById[$s.Id] = $s }
    $seen = @{}; $dup = 0; $orphan = 0; $neg = 0; $seqRecv = New-Object System.Collections.ArrayList
    $delta = @{ 0 = New-Object System.Collections.ArrayList; 1 = New-Object System.Collections.ArrayList }
    $recvCount = @{ 0 = 0; 1 = 0 }; $sentCount = @{ 0 = 0; 1 = 0 }
    foreach ($s in $sends) { if ($sentCount.ContainsKey([int]$s.Mode)) { $sentCount[[int]$s.Mode]++ } }
    foreach ($r in $recvs) {
        if (-not $sendById.ContainsKey($r.Id)) { $orphan++; continue }
        if ($seen.ContainsKey($r.Id)) { $dup++; continue }
        $seen[$r.Id] = 1; $s = $sendById[$r.Id]
        $d = ($r.T - $s.T) * 1000.0
        if ($d -lt 0) { $neg++ }
        [void]$seqRecv.Add([int]$s.Idx)
        if ($delta.ContainsKey([int]$s.Mode)) { [void]$delta[[int]$s.Mode].Add($d); $recvCount[[int]$s.Mode]++ }
    }
    $inversions = Count-Inversions $seqRecv
    $ok = $true; $why = @()
    $med = @{}; $p95 = @{}
    foreach ($m in 0, 1) {
        $v = @($delta[$m]); $med[$m] = Get-Median $v; $p95[$m] = Get-Pct $v 0.95
        $pc = 0; if ($sentCount[$m] -gt 0) { $pc = 100.0 * $recvCount[$m] / $sentCount[$m] }
        "[$tag] mode $m ($(@('RPC C++', 'RPC script')[$m])): sent $($sentCount[$m]) received $($recvCount[$m]) ($($pc.ToString('0.0', $inv)) %)  median $(Fmt $med[$m]) ms  p95 $(Fmt $p95[$m]) ms  max $(Fmt (Get-Max $v)) ms"
        if ($sentCount[$m] -eq 0) { $ok = $false; $why += "mode $m sent 0" } elseif ($recvCount[$m] -ne $sentCount[$m]) { $ok = $false; $why += "mode $m not 100 % received" }
    }
    $dMed = $med[1] - $med[0]; $dP95 = $p95[1] - $p95[0]
    "[$tag] script - C++ : median $(Fmt $dMed) ms, p95 $(Fmt $dP95) ms (limit $StepMs ms = 1 simulation step)"
    "[$tag] inversions (send order vs receive order): $inversions  duplicates: $dup  receives without send: $orphan  negative deltas (clock): $neg"
    if ([double]::IsNaN($dMed) -or [double]::IsNaN($dP95)) { $ok = $false; $why += 'no measure' } else {
        if ($dMed -ge $StepMs) { $ok = $false; $why += 'median gap >= 1 step' }
        if ($dP95 -ge $StepMs) { $ok = $false; $why += 'p95 gap >= 1 step' }
    }
    if ($inversions -ne 0) { $ok = $false; $why += 'inversions' }
    if ($neg -ne 0) { $ok = $false; $why += 'negative delta: clock not shared, fall back' }
    if ($dup -ne 0 -or $orphan -ne 0) { $ok = $false; $why += 'duplicate or orphan receive' }
    if ("$seenUnits" -ne "$n") { $ok = $false; $why += "pilot units '$seenUnits' != $n" }
    if ($ok) { "[$tag] VERDICT PASS" } else { "[$tag] VERDICT FAIL: $($why -join '; ')" }
    $script:lastOk = $ok
}

$all = $true
foreach ($n in $Units) { $script:lastOk = $false; Run-Size $n; if ($script:lastOk -ne $true) { $all = $false } }
if ($all) { "ALL SIZES PASS (criterion A, D846)"; exit 0 } else { "AT LEAST ONE SIZE FAILS"; exit 1 }
