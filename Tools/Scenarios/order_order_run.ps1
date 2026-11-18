# L2 M1 (2026-11-18): ORDER (inversions) of the player orders that depend on their arrival order, in ONE mode at a time: hol.Rpc.OrderScript 0 (RPC C++) then 1 (RPC script).
# 3 kinds of pairs, -Pairs of each (100): Move then Move(queue) ; Stop then Move ; PlaceBuilding then BuildUnits. Inversion = the SECOND order of a pair received
# before the FIRST one (order of the OrderRecv lines of the server log); also counted: global inversions (receive order vs send order over all orders), orders lost.
# Verdict PASS = 100 % received and 0 inversion in BOTH modes (D846 Q2 = A: no mixed mode, the 16 orders share one cvar).
# Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\order_order_run.ps1 -Root <worktree> [-Pairs 100] [-Units 5]
# Journal lines: SAME contract as Tools\Scenarios\order_latency_run.ps1 (header: OrderSend / OrderRecv behind hol.Rpc.OrderTrace 1, "key 0" for an order without point,
# which is then identified by its kind and its occurrence number, so Stop and BuildUnits are paired like the others).
# ===== CONTRACT WITH A2 (script pilot, client only, green client; a second cvar next to hol.Rpc.OrderLatency) =====
#   Command line given by this scenario on the green client: -ExecCmds="hol.Rpc.OrderTrace 1,hol.Rpc.OrderOrder <pairs> <mode>"   (mode = 0 or 1)
#   The pilot waits until the player has its units and a main centre, sets hol.Rpc.OrderScript <mode> ITSELF (System::ExecuteConsoleCommand, never mixed), then, ONE order per
#   tick in this exact order, sends <pairs> pairs of each kind: (1) Move then Move with bQueue, (2) Stop then Move, (3) PlaceBuilding then BuildUnits (on the building just placed
#   or on the main centre; the resources come from -HOLCheats on the server). Each pair has increasing keys (Move key = target X, 2000 + 2*i and 2000 + 2*i + 1 ...). Client log lines:
#     OrderOrder: start pairs <p> mode <0|1>
#     OrderOrder: done sent <k>          (k = 6 * pairs)
#   The pairs are read in send order (OrderSend lines of the client log, 2 per pair): the pair kind is the Order of its first line (Move, Stop, PlaceBuilding).
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [int]$Pairs = 100, [int]$Units = 5, [string]$Out = (Join-Path $env:TEMP "order_order"), [int]$TimeoutSec = 240, [int]$Port = 7994)
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$inv = [Globalization.CultureInfo]::InvariantCulture
$rxSend = [regex]'OrderSend:? (?:kind=)?(\w+) key[ =](-?\d+)(?: units[ =](\d+))?(?: mode[ =](\d))? t[ =]([0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)'
$rxRecv = [regex]'OrderRecv:? (?:kind=)?(\w+) key[ =](-?\d+)(?: units[ =](\d+))?(?: mode[ =](\d))? t[ =]([0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?)'

function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
function Count-Inversions($seq) { $c = 0; $a = @($seq); for ($i = 0; $i -lt $a.Count; $i++) { for ($j = $i + 1; $j -lt $a.Count; $j++) { if ($a[$i] -gt $a[$j]) { $c++ } } }; return $c }
# A log still open by the editor cannot be read by ReadLines (sharing violation): open it with FileShare.ReadWrite.
function Read-LinesShared($file) { $fs = [IO.File]::Open($file, "Open", "Read", "ReadWrite"); $sr = New-Object IO.StreamReader($fs); try { $out = New-Object System.Collections.Generic.List[string]; while ($null -ne ($l = $sr.ReadLine())) { $out.Add($l) }; return , $out } finally { $sr.Dispose(); $fs.Dispose() } }
function Read-Orders($file, $rx) {
    $list = New-Object System.Collections.ArrayList; $occ = @{}
    foreach ($line in (Read-LinesShared $file)) {
        $m = $rx.Match($line); if (-not $m.Success) { continue }
        $kind = $m.Groups[1].Value; $key = [int]$m.Groups[2].Value
        if ($key -eq 0) { if (-not $occ.ContainsKey($kind)) { $occ[$kind] = 0 }; $occ[$kind]++; $id = "$kind|#$($occ[$kind])" } else { $id = "$kind|$key" }
        [void]$list.Add([pscustomobject]@{ Id = $id; Kind = $kind; Idx = $list.Count })
    }
    return , $list
}

function Run-Mode([int]$mode) {
    $tag = "mode$mode"
    $cfg = Join-Path $Out "order_$tag.json"
    $cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = $Units; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
    Set-Content -Encoding ascii $cfg $cfgJson
    $sl = "$Out\order_${tag}_server.log"; $c1 = "$Out\order_${tag}_client_red.log"; $c2 = "$Out\order_${tag}_client_green.log"
    Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
    $procs = @()
    try {
        "[$tag] start $(Get-Date -Format HH:mm:ss)"
        $sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$Port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=OrdSrv -abslog=`"$sl`" -ExecCmds=`"hol.Rpc.OrderTrace 1`"" -PassThru -WindowStyle Hidden
        $procs += $sv
        "[$tag] server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 120) $(Get-Date -Format HH:mm:ss)"
        $a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${Port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=OrdCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
        $procs += $a
        "[$tag] red client connected (no order): $(WaitFor $c1 'Welcomed by server' 90)"
        Start-Sleep 5
        $b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${Port}?Name=Alice -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=OrdCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Rpc.OrderTrace 1,hol.Rpc.OrderOrder $Pairs $mode`"" -PassThru -WindowStyle Hidden
        $procs += $b
        $started = WaitFor $c2 'OrderOrder: start' 120
        "[$tag] green pilot started: $started"
        $done = $started -and (WaitFor $c2 'OrderOrder: done' $TimeoutSec)
        "[$tag] green pilot done: $done $(Get-Date -Format HH:mm:ss)"
        Start-Sleep 5
    } finally {
        foreach ($p in $procs) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
    }
    $script:lastOk = $false
    if (-not (Test-Path $c2) -or -not (Test-Path $sl)) { "[$tag] FAIL: no log"; return }
    $sends = Read-Orders $c2 $rxSend; $recvs = Read-Orders $sl $rxRecv
    "[$tag] OrderSend lines: $($sends.Count)  OrderRecv lines: $($recvs.Count)  (expected $(6 * $Pairs) sent)"
    $sendIdx = @{}; foreach ($s in $sends) { $sendIdx[$s.Id] = $s.Idx }
    $recvPos = @{}; $dup = 0; $orphan = 0; $seq = New-Object System.Collections.ArrayList
    foreach ($r in $recvs) {
        if (-not $sendIdx.ContainsKey($r.Id)) { $orphan++; continue }
        if ($recvPos.ContainsKey($r.Id)) { $dup++; continue }
        $recvPos[$r.Id] = $r.Idx; [void]$seq.Add([int]$sendIdx[$r.Id])
    }
    $lost = $sends.Count - $recvPos.Count
    $kinds = [ordered]@{ 'Move' = 'Move then Move(queue)'; 'Stop' = 'Stop then Move'; 'PlaceBuilding' = 'Place then BuildUnits'; 'other' = 'other first order' }
    $stat = @{}; foreach ($k in $kinds.Keys) { $stat[$k] = @{ pairs = 0; inv = 0; lost = 0 } }
    for ($i = 0; $i + 1 -lt $sends.Count; $i += 2) {
        $f = $sends[$i]; $s2 = $sends[$i + 1]; $k = $f.Kind; if (-not $kinds.Contains($k)) { $k = 'other' }
        $stat[$k].pairs++
        if (-not $recvPos.ContainsKey($f.Id) -or -not $recvPos.ContainsKey($s2.Id)) { $stat[$k].lost++; continue }
        if ($recvPos[$s2.Id] -lt $recvPos[$f.Id]) { $stat[$k].inv++ }
    }
    $pairInv = 0; $ok = $true; $why = @()
    foreach ($k in $kinds.Keys) {
        if ($stat[$k].pairs -eq 0 -and $k -eq 'other') { continue }
        "[$tag] $($kinds[$k]): pairs $($stat[$k].pairs) (expected $Pairs unless other), inverted $($stat[$k].inv), with a lost order $($stat[$k].lost)"
        $pairInv += $stat[$k].inv
        if ($k -ne 'other' -and $stat[$k].pairs -ne $Pairs) { $ok = $false; $why += "$k pairs $($stat[$k].pairs) != $Pairs" }
    }
    $global = Count-Inversions $seq
    "[$tag] inverted pairs: $pairInv  global inversions: $global  lost: $lost  duplicates: $dup  receives without send: $orphan"
    if ($pairInv -ne 0) { $ok = $false; $why += 'inverted pair' }
    if ($global -ne 0) { $ok = $false; $why += 'global inversion' }
    if ($lost -ne 0 -or $sends.Count -eq 0) { $ok = $false; $why += 'not 100 % received' }
    if ($dup -ne 0 -or $orphan -ne 0) { $ok = $false; $why += 'duplicate or orphan receive' }
    if ($ok) { "[$tag] VERDICT PASS" } else { "[$tag] VERDICT FAIL: $($why -join '; ')" }
    $script:lastOk = $ok
}

$all = $true
foreach ($m in 0, 1) { Run-Mode $m; if ($script:lastOk -ne $true) { $all = $false } }
if ($all) { "BOTH MODES PASS (0 inversion, 100 % received)"; exit 0 } else { "AT LEAST ONE MODE FAILS"; exit 1 }
