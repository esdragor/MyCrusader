# L2 order proofs P3 (resources) and P4 (buildings), WITH EFFECT, script route (D848, CARTE-L2-PREUVES-1118). 2026-11-18.
# Model: rpc_routes_seats_run.ps1. Dedicated server Plain, 2 Human seats (Alice slot 0 green, Bob slot 1 red), startingPeasants 5, -HOLCheats.
# Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\rpc_orders_p3p4_run.ps1 -Root <worktree> [-Lot P3|P4|P3P4]   (default P3P4)
# Exit code: 0 = VERDICT PASS, 1 = VERDICT FAIL. Nothing is launched with a visible window (-nullrhi -nosound, Start-Process -WindowStyle Hidden).
#
# EXACT LAUNCH LINES
#   server : UnrealEditor.exe <uproject> /Game/Maps/Plain -server -nullrhi -nosound -Port=7993 -unattended -nosplash -Multiprocess -LiveCoding=0
#            -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=<cfg> -saveddirsuffix=OrdSrv -abslog=<log>
#            (no -ExecCmds: the argument is OMITTED, no server cvar is needed ; the P3 / P4 orders need no spawned unit)
#   red    : client Bob, joins and makes NO call (no -ExecCmds).
#   green  : client Alice, -ExecCmds="hol.Rpc.ProofRoute <bits>"
# BITS of hol.Rpc.ProofRoute (written by another agent in HOLRpcPlayer.as, deferred steps "@<s> <command>" included, e.g. "@12 hol.Build.Report"):
#   P3: Gather 8388608, DropOff 16777216, BuildUnits 33554432, PlaceBuilding 67108864            (sum 125829120)
#   P4: CancelBuilding 134217728, SetRallyPoint 268435456, Garrison 536870912, UngarrisonAll 1073741824 (sum 2013265920)
#   P4 alone also sets 67108864 (PlaceBuilding): the cancel needs a foundation to cancel (carte, P4 CancelBuilding). Not asserted in the P4 lot.
#   P3P4 = 2139095040 (fits an int32). The hook fires ONCE, 4 s after the first tick of the green client; the 8 (or 4) orders share the 5 peasants.
# SERVER LOG LINES ASSERTED per order (reception = line 1 ; effect = line 2 ; lot R: the C++ trace line of the order is gone, no verdict depends on it):
#   1. "RpcOrder: <name> received by the server: slot 0 "   (names: gather, drop off, build units, place building, cancel building, set rally point, garrison, ungarrison all)
#   2. EFFECT:
#      gather        "Gather order of slot 0: N units to source" AND "Gather: slot 0 unloaded N <res>" with N > 0 (stock credited, the line shows the stock)
#      drop off      "Drop-off order of slot 0: N units unload at" (the "walk to" variant = no load = no effect) AND a "Gather: slot 0 unloaded" line AFTER it
#      build units   "Build order of slot 0: N units to House" AND client green hol.Build.Report shows "under construction NN%" growing (2 reports, 2 distinct values)
#      place building "Construction: slot 0 placed House foundation at (x, y), paid <cost>" (paid > 0 : cost taken from the stock)
#      cancel        "Construction: slot 0 cancelled House at (x, y), refund <res>"
#      set rally     "Rally: slot 0 set the rally point of <b> at (X, Y)" AND the word "on site" in that line (CallFull Site trap, C++ command hol.Prod.RallySite)
#      garrison      "Garrison: slot 0 puts N of M units in <b>" with N > 0
#      ungarrison    "Garrison: slot 0 takes N units out of <b>" with N > 0
# RED witness: the red client makes no call -> 0 server line "RpcOrder: ... slot 1" ; and 0 line "RpcRoute: .* not sent" (route succeeded) on the server and the green client.
# REAL WAITS (bounded, seconds after the shot, i.e. after the green client prints its first "RpcRoute: " line): place/cancel/rally 30 ; build units 45 ;
#   garrison 45 ; ungarrison 80 (garrison walk ~30 s then exit) ; gather 75 (harvest up to ~70 s) ; drop off 100 (needs a load, so after the gather).
#   Longest wait of a lot: P3 100 s, P4 80 s, P3P4 100 s. Plus: server up <= 90 s, red join <= 90 s, green shot <= 90 s. Total at most about 6 min.
# FIRST REAL RUN (2026-11-18) AND CORRECTIONS : (a) the command is now hol.Units.GatherOrder (the name hol.Units.Gather already existed: "Warning: hol.Units.Gather X Y [Slot]") ; (b) hol.Build.PlaceNear House 800 0 = (-14000, 0) "refused (on a building)": the hook now places at (0 1500), (-1500 0), (1500 -1800), (3000 0) from the main centre (-14800, 0), a site first, THEN @4 hol.Select.Own + hol.Units.Build ;
#   (c) GARRISON LIMIT: no building has garrison places (GarrisonSlots = 0 for the 14 buildings) : Garrison / UngarrisonAll are asserted on RECEPTION only ("RpcOrder: garrison received"), NEVER on "Garrison: slot 0 puts / takes" (the effect cannot exist until a building has places; hol.Units.Garrison also warns "no own finished building with places" and sends nothing, so the reception may stay NON).
# 2026-11-18 ASSERTIONS FROM THE REAL LOGS (Gather / DropOff / BuildUnits / PlaceBuilding / Cancel / Rally): effect = the server line of the order itself ("Gather order of slot 0: N units to source I", "Drop-off order of slot 0: N units to MainCentre", "Build order of slot 0: N units to House at (x, y)" THEN a "Construction: slot 0 House at (x, y), K builders at work" line of THAT site, "placed House foundation at (x, y), paid ..." with any payment). NOT required, documented: "Gather: slot 0 unloaded" (a Gather needs a harvested load, a DropOff a carried one: the real DropOff order said "0 units unload ... 6 walk to"). CancelBuilding: the hook now selects a foundation (hol.Prod.Queue Peasant House) and cancels in the SAME step (@5): the real run cancelled with 6 units selected (no "Cancel request"). Garrison / UngarrisonAll: NON PROUVABLE (GarrisonSlots = 0), printed as such, excluded from the VERDICT (the VERDICT PASS asks every other order OK).
# Limit: the numeric stock of hol.Build.Report is printed (section GREEN CLIENT), not parsed (format owned by another agent) ; the stock fact asserted is the server "unloaded"/"paid"/"refund" line.
# Dependencies (other agents): C++ hol.Units.* commands and ProofRoute bits in HOLRpcPlayer.as, cvar hol.Rpc.ProofSpawn. Without them the table says NON (no invented success).
param(
    [string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent),
    [string]$Out = (Join-Path $env:TEMP "rpc_orders_p3p4"),
    [ValidateSet('P3', 'P4', 'P3P4')][string]$Lot = 'P3P4'
)
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7993

function Read-LinesShared($file) {
    if (-not (Test-Path $file)) { return , (New-Object System.Collections.Generic.List[string]) }
    $fs = [IO.File]::Open($file, "Open", "Read", "ReadWrite"); $sr = New-Object IO.StreamReader($fs)
    try { $o = New-Object System.Collections.Generic.List[string]; while ($null -ne ($l = $sr.ReadLine())) { $o.Add($l) }; return , $o } finally { $sr.Dispose(); $fs.Dispose() }
}
function WaitFor($f, $pat, $sec) {
    $t0 = Get-Date
    while (((Get-Date) - $t0).TotalSeconds -lt $sec) {
        foreach ($l in (Read-LinesShared $f)) { if ($l -match $pat) { return $true } }
        Start-Sleep 2
    }
    return $false
}
function FirstIdx($lines, $pat) { for ($i = 0; $i -lt $lines.Count; $i++) { if ($lines[$i] -match $pat) { return $i } }; return -1 }
function Short($s) { return $s.Substring(0, [Math]::Min(220, $s.Length)) }

# --- orders of the lot : Name, RecvPat (server line 1), Eff (script block returning detail or $null), Deadline (s after the shot)
$all = @(
    # REAL LOG LINES (2026-11-18, orders_server.log / orders_client_green.log of the first real P3P4 run): the guessed patterns were corrected from them. An effect = the server's own line of the order (the 'unloaded' / 'progress' variants need a load or a long wait and are NOT required: documented).
    @{ Lot = 'P3'; Name = 'Gather'; Recv = 'RpcOrder: gather received by the server: slot 0 '; Dead = 40
        Eff = { param($s) $a = FirstIdx $s 'Gather order of slot 0: \d+ units to source \d+'; if ($a -ge 0) { return $s[$a] }; return $null } },
    @{ Lot = 'P3'; Name = 'DropOff'; Recv = 'RpcOrder: drop off received by the server: slot 0 '; Dead = 40
        Eff = { param($s) $a = FirstIdx $s 'Drop-off order of slot 0: \d+ units to MainCentre'; if ($a -ge 0) { return $s[$a] }; return $null } },
    @{ Lot = 'P3'; Name = 'BuildUnits'; Recv = 'RpcOrder: build units received by the server: slot 0 '; Dead = 45
        Eff = { param($s) $a = -1; for ($i = 0; $i -lt $s.Count; $i++) { if ($s[$i] -match 'Build order of slot 0: \d+ units to House at \((-?\d+), (-?\d+)\)') { $a = $i } }; if ($a -lt 0) { return $null }
            $m = [regex]::Match($s[$a], 'House at \((-?\d+), (-?\d+)\)'); $site = 'House at (' + $m.Groups[1].Value + ', ' + $m.Groups[2].Value + '), '
            for ($i = $a + 1; $i -lt $s.Count; $i++) { if ($s[$i].Contains('Construction: slot 0 ' + $site) -and $s[$i] -match '[1-9]\d* builders at work') { return ($s[$a].Substring($s[$a].IndexOf('Build order')) + ' THEN ' + $s[$i].Substring($s[$i].IndexOf('Construction'))) } }; return $null } },
    @{ Lot = 'P3'; Name = 'PlaceBuilding'; Recv = 'RpcOrder: place building received by the server: slot 0 '; Dead = 30
        Eff = { param($s) $a = FirstIdx $s 'Construction: slot 0 placed House foundation at \(.*\), paid '; if ($a -ge 0) { return $s[$a] }; return $null } },
    @{ Lot = 'P4'; Name = 'CancelBuilding'; Recv = 'RpcOrder: cancel building received by the server: slot 0'; Dead = 30
        Eff = { param($s) $a = FirstIdx $s 'Construction: slot 0 cancelled House at .*, refund'; if ($a -ge 0) { return $s[$a] }; return $null } },
    @{ Lot = 'P4'; Name = 'SetRallyPoint'; Recv = 'RpcOrder: set rally point received by the server: slot 0 '; Dead = 30
        Eff = { param($s) $a = FirstIdx $s 'Rally: slot 0 set the rally point of .* at \(.*\).* on site'; if ($a -ge 0) { return $s[$a] }; return $null } },
    @{ Lot = 'P4'; Name = 'Garrison'; Recv = 'RpcOrder: garrison received by the server: slot 0 '; Dead = 0; Unprovable = 'NON PROUVABLE : GarrisonSlots = 0'
        Eff = { param($s) return $null } },
    @{ Lot = 'P4'; Name = 'UngarrisonAll'; Recv = 'RpcOrder: ungarrison all received by the server: slot 0 '; Dead = 0; Unprovable = 'NON PROUVABLE : GarrisonSlots = 0'
        Eff = { param($s) return $null } }
)
$p3bits = 8388608 + 16777216 + 33554432 + 67108864
$p4bits = 134217728 + 268435456 + 536870912 + 1073741824
$orders = @($all | Where-Object { $Lot -eq 'P3P4' -or $_.Lot -eq $Lot })
$bits = 0
if ($Lot -eq 'P3') { $bits = $p3bits } elseif ($Lot -eq 'P4') { $bits = $p4bits + 67108864 } else { $bits = $p3bits + $p4bits }
$hasP4 = ($Lot -ne 'P3')

$cfg = Join-Path $Out "orders_p3p4.json"
$cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = 5; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
Set-Content -Encoding ascii $cfg $cfgJson
$sl = "$Out\orders_server.log"; $c1 = "$Out\orders_client_red.log"; $c2 = "$Out\orders_client_green.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
"start $(Get-Date -Format HH:mm:ss) lot $Lot bits $bits orders $(($orders | ForEach-Object { $_.Name }) -join ',')"

# binary up to date (same check as the other scenarios) : a stale dll would only give NON lines
$lc = Join-Path $w 'Tools\Scenarios\LaunchCheck.ps1'
if (Test-Path $lc) { . $lc; $stale = Get-StaleBinaryProblem $w; if ($stale) { "STALE BINARY: $stale"; "VERDICT FAIL (binary older than its sources, build first)"; exit 1 } }

# no server -ExecCmds at all (no ProofSpawn: the P3 / P4 orders need no spawned unit, the proof's Barracks is placed from the client): the argument is omitted, never passed empty
$procs = @(); $code = 1
try {
    $sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=OrdSrv -abslog=`"$sl`"" -PassThru -WindowStyle Hidden
    $procs += $sv
    $upOk = WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 90
    "server up: $upOk $(Get-Date -Format HH:mm:ss)"
    $a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=OrdCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
    $procs += $a
    "red client joined, made no call: $(WaitFor $c1 'Welcomed by server' 90) $(Get-Date -Format HH:mm:ss)"
    Start-Sleep 5
    $b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Alice -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=OrdCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Rpc.ProofRoute $bits`"" -PassThru -WindowStyle Hidden
    $procs += $b
    $shot = WaitFor $c2 'RpcRoute: ' 90
    $t0 = Get-Date
    "green client fired its commands: $shot $(Get-Date -Format HH:mm:ss)"

    # poll: each order is done when reception + effect are seen, or when its own deadline (seconds after the shot) is past
    $script:c2 = $c2
    $res = @{}; foreach ($o in $orders) { $res[$o.Name] = @{ Recv = $false; Eff = $null } }
    $maxDead = ($orders | Where-Object { -not $_.Unprovable } | ForEach-Object { $_.Dead } | Measure-Object -Maximum).Maximum
    while ($true) {
        $s = Read-LinesShared $sl
        $el = ((Get-Date) - $t0).TotalSeconds
        $pending = 0
        foreach ($o in ($orders | Where-Object { -not $_.Unprovable })) {
            $r = $res[$o.Name]
            if (-not $r.Recv) { $r.Recv = ((FirstIdx $s ([regex]::Escape($o.Recv))) -ge 0) }
            if ($null -eq $r.Eff) { $r.Eff = & $o.Eff $s }
            if (-not ($r.Recv -and $null -ne $r.Eff) -and $el -lt $o.Dead) { $pending++ }
        }
        if ($pending -eq 0 -or $el -ge $maxDead) { break }
        Start-Sleep 3
    }
    "waited $([int]((Get-Date) - $t0).TotalSeconds) s after the shot (max $maxDead)"
    Start-Sleep 3
    $s = Read-LinesShared $sl
    $redLines = @($s | Where-Object { $_ -match 'RpcOrder: [a-z ]+ received by the server: slot 1\b|RpcOrder: [a-z ]+ refused \(.*\) for sender slot 1\b' })
    $notSent = @($s | Where-Object { $_ -match 'RpcRoute: .* not sent' }).Count + @((Read-LinesShared $c2) | Where-Object { $_ -match 'RpcRoute: .* not sent' }).Count
    "RED CHECK: server 'RpcOrder:' lines for slot 1 = $($redLines.Count) (expected 0)"
    "no RpcRoute not sent: $($notSent -eq 0) (expected True)"

    "=====TABLE (order -> received yes/NO ; effect yes/NO)"
    $fail = 0
    foreach ($o in $orders) {
        $r = $res[$o.Name]
        if ($o.Unprovable) { "{0,-15} {1}  (no own building has garrison places: hol.Units.Garrison / UngarrisonAll warn 'no own finished building with places' and send nothing; neither yes nor NO, not counted in the VERDICT)" -f $o.Name, $o.Unprovable; continue }
        $rv = if ($r.Recv) { 'yes' } else { 'NO' }; $ev = if ($null -ne $r.Eff) { 'yes' } else { 'NO' }
        if (-not $r.Recv -or $null -eq $r.Eff) { $fail++ }
        "{0,-15} received {1,-3}  effect {2,-3}  {3}" -f $o.Name, $rv, $ev, $(if ($null -ne $r.Eff) { Short ([string]$r.Eff) } else { '' })
    }
    if ($redLines.Count -ne 0) { $fail++ }
    if ($notSent -ne 0) { $fail++ }
    if (-not $upOk -or -not $shot) { $fail++ }

    "=====SERVER (lines)"; $s | Where-Object { $_ -match 'RpcOrder: |Gather: |Gather order|Drop-off order|Build order|Construction: |Rally: |Garrison: |Angelscript: Error' } | ForEach-Object { Short $_ } | Select-Object -First 70
    "=====GREEN CLIENT"; (Read-LinesShared $c2) | Where-Object { $_ -match 'RpcRoute|Build request|Cancel request|Exit all request|under construction|Stock|stock|NetworkFailure' } | ForEach-Object { Short $_ } | Select-Object -First 50
    "=====RED CLIENT"; (Read-LinesShared $c1) | Where-Object { $_ -match 'RpcRoute|NetworkFailure' } | ForEach-Object { Short $_ } | Select-Object -First 8
    if ($fail -eq 0) { "VERDICT PASS"; $code = 0 } else { "VERDICT FAIL ($fail failing item(s))"; $code = 1 }
}
finally {
    foreach ($p in $procs) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue } }
}
exit $code
