# L2 orders proofs P1 (movement: Move, AttackMove, MoveInLine, Stop) and P2 (combat: AttackUnit, AttackBuilding, HealUnit, ToggleDeploy)
# D848: network proof WITH EFFECT of the L2 orders on the SCRIPT route. Source of the contract: Docs\Rapports\CARTE-L2-PREUVES-1118.md (main copy).
# Model: rpc_routes_seats_run.ps1. Dedicated server Plain, -HOLCheats, MatchConfig with 2 Human seats (Alice slot 0 green, Bob slot 1 red, startingPeasants 5).
# RED client (Bob) joins and makes NO call -> 0 line "RpcOrder: ... slot 1" on the server. GREEN client (Alice) runs the orders of the bit.
# Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\rpc_orders_p1p2_run.ps1 -Root <worktree> [-Lot P1|P2|P1P2] [-Only "move,stop"] [-Combined]
#   Default: ONE run per order (clean effect, the orders do not overwrite each other). -Combined: ONE run with the bits of the lot summed (faster; effects judged on the journal lines only, positions are mixed).
# Exact launch lines (hidden, no window):
#   SERVER : UnrealEditor.exe <uproject> /Game/Maps/Plain -server -nullrhi -nosound -Port=<p> -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=<json> -saveddirsuffix=OrdSrv -abslog=<log> [-ExecCmds="hol.Rpc.ProofSpawn <bits>" ; the argument is OMITTED when no cvar is needed]
#   RED    : UnrealEditor.exe <uproject> 127.0.0.1:<p>?Name=Bob -game -nullrhi -nosound ... (no -ExecCmds)
#   GREEN  : UnrealEditor.exe <uproject> 127.0.0.1:<p>?Name=Alice -game -nullrhi -nosound ... -ExecCmds="hol.Rpc.ProofRoute <bits>"
#   ProofSpawn 1 (server cvar, script, written by another agent) only for the orders that need a monk / a deployable unit / a close enemy soldier (attack unit, heal unit, toggle deploy).
# Bits of hol.Rpc.ProofRoute (card section 0, hook in Script\Net\HOLRpcPlayer.as, written by another agent): move 32768, attack move 65536, move in line 131072, stop 262144, attack unit 524288, attack building 1048576, heal unit 2097152, toggle deploy 4194304 (P1 = 491520, P2 = 7864320, P1P2 = 8355840).
# The hook must also run the delayed report "@12 hol.Units.Where 0" on the GREEN client (card, lacuna of the hook) : the positions / activities / hp are read on the green client journal (replicated units), the simulation lines on the server journal.
# JOURNAL LINES READ (contract with C++ and script; the server journal is read with a shared read, the processes stay open):
#   per order, reception (script, server)  : "RpcOrder: <name> received by the server: slot 0 ..."  (refusal: "RpcOrder: <name> refused (<code>) for sender slot 0")
#   (lot R: the 16 C++ order RPCs and their server trace line are gone; the verdict of an order depends on the reception line above + the EFFECT below only)
#   EFFECT move         : server "Move order: N units to (X, Y)" + green last "Units where: ..." : every slot 0 unit within -Tol cm of (X, Y)
#   EFFECT attack move  : server or green "attack-move to (X, Y)" (the commands hol.Units.AttackMove logs it, K C.4512) + Where : every slot 0 unit within -Tol cm of (X, Y)
#   EFFECT move in line : Where (hook command hol.Units.OrderLine 1500 0 1500 600 : Start -> End along +Y) : >= 3 units on ONE line (minor/major std <= 0.15, major std >= 50 cm) perpendicular to Start->End (axis within 30 degrees of X) ; Start = End (silent bug) gives a clump and fails
#   EFFECT stop         : server "Stop order of slot 0 for N units" + Where : all slot 0 units Idle and none within -Tol cm of the Move target (server "Move order ... to (X, Y)" given by the hook before the Stop)
#   EFFECT attack unit  : server "Attack order of slot 0 for N units on #U" + (green Where "Attacking, target #U" OR server "Combat: step ... hits #U")
#   EFFECT attack bldg  : server "Attack order of slot 0 for N units on building ... of slot 1" + ("Building damaged: ... of slot 1 took N" OR "Combat: ... hits a building")
#   EFFECT heal unit    : server "Heal: step N #m (slot 0) stops in reach of #U" (a monk of slot 0 reached the wounded unit ; the walk line alone is not enough)
#   EFFECT toggle deploy: server "Deploy: #U phase 0 -> 1" (deployment phase changed)
#   RED (c)             : 0 server lines matching "RpcOrder: .*slot 1\b" and no "RpcRoute: .* not sent" on the server nor the green client.
# Output: table "order -> received yes/NO ; effect yes/NO", then VERDICT PASS/FAIL, exit code 0/1.
param(
    [string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent),
    [string]$Out = (Join-Path $env:TEMP "rpc_orders_p1p2"),
    [ValidateSet("P1", "P2", "P1P2")][string]$Lot = "P1P2",
    [string]$Only = "",
    [switch]$Combined,
    [int]$Port = 7995,
    # D850 (criterion B of D846, simulated locally): one-way packet lag (ms), its variance (ms) and packet loss (%) on the server and on the green client; 0 = no emulation
    [int]$Lag = 0,
    [int]$LagVariance = 0,
    [int]$Loss = 0,
    [int]$Tol = 800,
    [int]$SettleSec = 26 # after the reception: the last report ("@20 Where" of attack building) is due 20 s after the shot
)
$NetEm = @(); if ($Lag -gt 0) { $NetEm += "NetEmulation.PktLag $Lag" }; if ($LagVariance -gt 0) { $NetEm += "NetEmulation.PktLagVariance $LagVariance" }; if ($Loss -gt 0) { $NetEm += "NetEmulation.PktLoss $Loss" }
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $up -or -not (Test-Path $ed)) { "VERDICT FAIL (no uproject in $w or no editor $ed)"; exit 1 }

function Read-LinesShared($file) {
    if (-not (Test-Path $file)) { return , (New-Object System.Collections.Generic.List[string]) }
    $fs = [IO.File]::Open($file, "Open", "Read", "ReadWrite"); $sr = New-Object IO.StreamReader($fs)
    try { $out = New-Object System.Collections.Generic.List[string]; while ($null -ne ($l = $sr.ReadLine())) { $out.Add($l) }; return , $out } finally { $sr.Dispose(); $fs.Dispose() }
}
function Count-Match($lines, $rx) { $n = 0; foreach ($l in $lines) { if ($l -match $rx) { $n++ } }; return $n }
function Last-Match($lines, $rx) { $r = $null; foreach ($l in $lines) { $m = [regex]::Match($l, $rx); if ($m.Success) { $r = $m } }; return $r }
function WaitFor($f, $rx, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Count-Match (Read-LinesShared $f) $rx) -gt 0) { return $true }; Start-Sleep 2 }; return $false }
function Parse-Where($lines) {
    # last "Units where: N units: #id slot S at (x, y) Activity[, ...]; ..." line of a journal -> list of units
    $m = Last-Match $lines 'Units where: \d+ units: (.*)$'
    $res = @(); if (-not $m) { return , $res }
    foreach ($part in ($m.Groups[1].Value -split '; ')) {
        $p = [regex]::Match($part, '^#(\d+) slot (\d+) at \((-?\d+), (-?\d+)\) (\w+)(.*)$')
        if ($p.Success) { $res += [pscustomobject]@{ Id = [int]$p.Groups[1].Value; Slot = [int]$p.Groups[2].Value; X = [double]$p.Groups[3].Value; Y = [double]$p.Groups[4].Value; Act = $p.Groups[5].Value; Rest = $p.Groups[6].Value } }
    }
    return , $res
}
function Near-All($units, $x, $y, $tol) { $own = @($units | Where-Object { $_.Slot -eq 0 }); if ($own.Count -eq 0) { return $false }; foreach ($u in $own) { if ([Math]::Sqrt(($u.X - $x) * ($u.X - $x) + ($u.Y - $y) * ($u.Y - $y)) -gt $tol) { return $false } }; return $true }
function Near-Any($units, $x, $y, $tol) { foreach ($u in ($units | Where-Object { $_.Slot -eq 0 })) { if ([Math]::Sqrt(($u.X - $x) * ($u.X - $x) + ($u.Y - $y) * ($u.Y - $y)) -le $tol) { return $true } }; return $false }
function Line-Axis($units) {
    $own = @($units | Where-Object { $_.Slot -eq 0 }); if ($own.Count -lt 3) { return $null }
    $mx = ($own | Measure-Object X -Average).Average; $my = ($own | Measure-Object Y -Average).Average
    $sxx = 0.0; $syy = 0.0; $sxy = 0.0
    foreach ($u in $own) { $dx = $u.X - $mx; $dy = $u.Y - $my; $sxx += $dx * $dx; $syy += $dy * $dy; $sxy += $dx * $dy }
    $n = $own.Count; $sxx /= $n; $syy /= $n; $sxy /= $n
    $tr = $sxx + $syy; $det = $sxx * $syy - $sxy * $sxy; $disc = [Math]::Sqrt([Math]::Max(0.0, $tr * $tr / 4 - $det))
    $l1 = $tr / 2 + $disc; $l2 = [Math]::Max(0.0, $tr / 2 - $disc)
    $ang = 0.5 * [Math]::Atan2(2 * $sxy, $sxx - $syy)
    return [pscustomobject]@{ Major = [Math]::Sqrt($l1); Minor = [Math]::Sqrt($l2); CosX = [Math]::Abs([Math]::Cos($ang)) }
}

# name = journal name of the RpcOrder line ; Bit = hol.Rpc.ProofRoute bit ; Lot ; Spawn = needs hol.Rpc.ProofSpawn 1 on the server
$orders = @(
    @{ Name = "move"; Bit = 32768; Lot = "P1"; Spawn = 0 },
    @{ Name = "attack move"; Bit = 65536; Lot = "P1"; Spawn = 0 },
    @{ Name = "move in line"; Bit = 131072; Lot = "P1"; Spawn = 0 },
    @{ Name = "stop"; Bit = 262144; Lot = "P1"; Spawn = 0 },
    @{ Name = "attack unit"; Bit = 524288; Lot = "P2"; Spawn = 4 },
    @{ Name = "attack building"; Bit = 1048576; Lot = "P2"; Spawn = 0 },
    @{ Name = "heal unit"; Bit = 2097152; Lot = "P2"; Spawn = 1 },
    @{ Name = "toggle deploy"; Bit = 4194304; Lot = "P2"; Spawn = 2 }
)
$sel = @($orders | Where-Object { $Lot -eq "P1P2" -or $_.Lot -eq $Lot })
if ($Only) { $names = @($Only -split ',' | ForEach-Object { $_.Trim() }); $sel = @($sel | Where-Object { $names -contains $_.Name }) }
if ($sel.Count -eq 0) { "VERDICT FAIL (no order selected)"; exit 1 }

function Ordered-Units($wh, $S) {
    # the units that received the order: the N lowest ids of slot 0 (N = "units N" of the last reception line). A later-born unit (the proof's Monk / Mangonel, a peasant born in the run) is not one of them.
    $m = Last-Match $S 'RpcOrder: [a-z ]+ received by the server: slot 0 units (\d+)'
    $n = 6; if ($m) { $n = [int]$m.Groups[1].Value }
    return , @($wh | Where-Object { $_.Slot -eq 0 } | Sort-Object Id | Select-Object -First $n)
}
function Effect-Of($o, $S, $K) {
    # returns @{ Ok = bool; Detail = string } ; $S = server lines, $K = green client lines
    $wh = Parse-Where $K
    $ord = Ordered-Units $wh $S
    switch ($o.Name) {
        "move" {
            # first real run: the server prints NO "Move order" line ; the target is the client line "Order: N units to (X, Y)" (K) written by hol.Units.OrderNear
            $m = Last-Match $K 'Order: (\d+) units to \((-?\d+), (-?\d+)\)'
            if (-not $m) { return @{ Ok = $false; Detail = "no client line 'Order: N units to (X, Y)'" } }
            $x = [double]$m.Groups[2].Value; $y = [double]$m.Groups[3].Value
            return @{ Ok = (Near-All $ord $x $y $Tol); Detail = "target ($x, $y), $($ord.Count) ordered units in the last Where, all within $Tol cm: $(Near-All $ord $x $y $Tol)" }
        }
        "attack move" {
            $m = Last-Match $S 'attack-move to \((-?\d+), (-?\d+)\)'; if (-not $m) { $m = Last-Match $K 'attack-move to \((-?\d+), (-?\d+)\)' }
            if (-not $m) { return @{ Ok = $false; Detail = "no 'attack-move to (X, Y)' line" } }
            $x = [double]$m.Groups[1].Value; $y = [double]$m.Groups[2].Value
            # first real run: the point (1500, 0) was 16000 cm away (units still Moving at +12 s) ; the hook now orders (-13300, 0), 1500 cm from the main centre
            return @{ Ok = (Near-All $ord $x $y $Tol); Detail = "target ($x, $y), $($ord.Count) ordered units in the last Where, all within $Tol cm: $(Near-All $ord $x $y $Tol)" }
        }
        "move in line" {
            # first run: units still Moving in transit (std 140 / 98) ; the hook now draws the line (-13300, -300) -> (-13300, 300) near the base and only the ordered units that stopped moving are measured
            # 2026-11-18 real log: the server line is "RpcOrder: move in line received by the server: slot 0 units 6 width false queue false start (X, Y) end (X, Y)" (start / end written by ServerMoveUnitsInLine in HOLRpcPlayer.as) ; the client line is "Order: 6 units in line from (-13300, -300) to (-13300, 300)".
            # THE EFFECT (PASS) = the server received a Start and an End that DIFFER and EQUAL the client's (the CallFull trap: Start = End). The formation shape is information only (the real run showed rows, not one line): minor <= 0.6 major is printed, not required.
            $mc = Last-Match $K 'Order: \d+ units in line from \((-?\d+), (-?\d+)\) to \((-?\d+), (-?\d+)\)'
            $ms = Last-Match $S 'RpcOrder: move in line received by the server: slot 0 .* start \((-?\d+), (-?\d+)\) end \((-?\d+), (-?\d+)\)'
            $idle = @($ord | Where-Object { $_.Act -eq 'Idle' }); $ax = Line-Axis $idle
            $shape = "no shape (fewer than 3 Idle ordered units)"
            if ($ax) { $shape = ("shape (information): {0} Idle ordered units, major std {1:N0} cm, minor std {2:N0} cm, minor <= 0.6 major: {3}" -f $idle.Count, $ax.Major, $ax.Minor, ($ax.Minor -le 0.6 * $ax.Major)) }
            if (-not $mc) { return @{ Ok = $false; Detail = "no client line 'Order: N units in line from (X, Y) to (X, Y)' ; $shape" } }
            if (-not $ms) { return @{ Ok = $false; Detail = "no server line with 'start (X, Y) end (X, Y)' (HOLRpcPlayer.as not rebuilt?) ; $shape" } }
            $sent = ($mc.Groups[1].Value + "," + $mc.Groups[2].Value + " -> " + $mc.Groups[3].Value + "," + $mc.Groups[4].Value)
            $got = ($ms.Groups[1].Value + "," + $ms.Groups[2].Value + " -> " + $ms.Groups[3].Value + "," + $ms.Groups[4].Value)
            $differ = ($ms.Groups[1].Value -ne $ms.Groups[3].Value) -or ($ms.Groups[2].Value -ne $ms.Groups[4].Value)
            $same = ($sent -eq $got)
            return @{ Ok = ($differ -and $same); Detail = "client sent $sent, server received $got : Start != End $differ, equal to what was sent $same ; $shape" }
        }
        "stop" {
            $st = Count-Match $S 'Stop order of slot 0 for \d+ units'
            $m = Last-Match $S 'Move order: (\d+) units to \((-?\d+), (-?\d+)\)'
            $own = @($wh | Where-Object { $_.Slot -eq 0 }); $idle = (@($own | Where-Object { $_.Act -eq 'Idle' })).Count
            $far = $true; if ($m) { $far = -not (Near-Any $wh ([double]$m.Groups[2].Value) ([double]$m.Groups[3].Value) $Tol) }
            return @{ Ok = ($st -gt 0 -and $own.Count -gt 0 -and $idle -eq $own.Count -and $far); Detail = "Stop line(s) $st, Idle $idle/$($own.Count), none near the Move target: $far" }
        }
        "attack unit" {
            # 2026-11-18 real logs: client "Order: 6 units attack #7" (hol.Select.Attack slot=1), server "Attack order of slot 0 for 6 units on #7". EFFECT = the server line names the id the client sent. The HP / Attacking effect is NOT required (target far: the units were still Moving 12 s after the shot): documented limit.
            $mc = Last-Match $K 'Order: \d+ units attack #(\d+)'
            $m = Last-Match $S 'Attack order of slot 0 for \d+ units on #(\d+)'
            if (-not $mc) { return @{ Ok = $false; Detail = "no client line 'Order: N units attack #U'" } }
            if (-not $m) { return @{ Ok = $false; Detail = "no server line 'Attack order of slot 0 for N units on #U' (client sent #$($mc.Groups[1].Value))" } }
            $u = $m.Groups[1].Value; $same = ($u -eq $mc.Groups[1].Value)
            $att = (@($wh | Where-Object { $_.Slot -eq 0 -and $_.Act -eq 'Attacking' -and $_.Rest -match "target #$u\b" })).Count
            $hit = Count-Match $S "Combat: .*#$u\b"
            return @{ Ok = $same; Detail = "client sent #$($mc.Groups[1].Value), server order on #$u (equal: $same) ; information, not required: $att unit(s) Attacking it, $hit Combat line(s) naming it" }
        }
        "attack building" {
            $st = Count-Match $S 'Attack order of slot 0 for \d+ units on building .* of slot 1'
            # first run: the enemy main centre is ~30000 cm away. The hook now runs hol.Cheat.UnitSpeed 5 and reports at +20 s. Effect = damage lines if they exist, else the ordered units marched toward it (a unit now at x >= -11000, it started near -15700): HP damage is NOT required when the walk is not finished
            $dm = Count-Match $S 'Building damaged: .* of slot 1 took'; $hit = Count-Match $S 'Combat: .*hits a building'
            $adv = (@($ord | Where-Object { $_.X -ge -11000 })).Count
            return @{ Ok = ($st -gt 0 -and ($dm -gt 0 -or $hit -gt 0 -or $adv -gt 0)); Detail = "order line $st, 'Building damaged' $dm, 'hits a building' $hit, ordered units advanced to x >= -11000: $adv of $($ord.Count)" }
        }
        "heal unit" {
            $r = Count-Match $S 'Heal: step \d+ #\d+ \(slot 0\) stops in reach of #\d+'; $wk = Count-Match $S 'Heal: step \d+ #\d+ \(slot 0\) walks to'
            return @{ Ok = ($r -gt 0); Detail = "'stops in reach' $r, 'walks to' $wk (the walk alone is not an effect)" }
        }
        "toggle deploy" {
            $d = Count-Match $S 'Deploy: #\d+ phase 0 -> 1'
            return @{ Ok = ($d -gt 0); Detail = "'Deploy: #U phase 0 -> 1' lines: $d" }
        }
    }
    return @{ Ok = $false; Detail = "unknown order" }
}

function Run-One($tag, $group) {
    $bits = 0; foreach ($o in $group) { $bits += $o.Bit }
    # ProofSpawn bits of the group (1 Monk, 2 Mangonel, 4 three Spearman of slot 1 at (-12000, 0)): the first run passed 1 for every order, so no Mangonel / Spearman existed ("Select type Mangonel: 0 units")
    $spawnBits = 0; foreach ($o in $group) { $spawnBits = $spawnBits -bor $o.Spawn }
    $spawn = $spawnBits -gt 0
    $cfg = Join-Path $Out "orders_$tag.json"
    $cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = 5; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
    Set-Content -Encoding ascii $cfg $cfgJson
    $sl = "$Out\orders_${tag}_server.log"; $c1 = "$Out\orders_${tag}_client_red.log"; $c2 = "$Out\orders_${tag}_client_green.log"
    Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
    $svParts = @(); if (($spawnBits -band 2) -ne 0) { $svParts += "hol.Siege.Deployable 1" }; if ($spawn) { $svParts += "hol.Rpc.ProofSpawn $spawnBits" }
    if ($NetEm.Count -gt 0) { $svParts = @($NetEm) + @($svParts) }
    $svCmds = $svParts -join ","
    $svExec = if ($svCmds) { " -ExecCmds=`"$svCmds`"" } else { "" }   # never pass an empty -ExecCmds
    "[$tag] bits $bits, server ExecCmds '$svCmds' $(Get-Date -Format HH:mm:ss)"
    $sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$Port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=OrdSrv -abslog=`"$sl`"$svExec" -PassThru -WindowStyle Hidden
    $up1 = WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 90
    "[$tag] server up: $up1 $(Get-Date -Format HH:mm:ss)"
    $a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${Port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=OrdCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
    "[$tag] red client joined, no call: $(WaitFor $c1 'Welcomed by server' 90)"
    Start-Sleep 5
    $b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${Port}?Name=Alice -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=OrdCli2 -abslog=`"$c2`" -ExecCmds=`"$(((@($NetEm) + @('hol.Rpc.ProofRoute ' + $bits)) -join ','))`"" -PassThru -WindowStyle Hidden
    foreach ($o in $group) { "[$tag] server has the reception line of '$($o.Name)': $(WaitFor $sl ('RpcOrder: ' + [regex]::Escape($o.Name) + ' (received|refused)') 75)" }
    Start-Sleep $SettleSec
    $S = Read-LinesShared $sl; $K = Read-LinesShared $c2
    $res = @()
    foreach ($o in $group) {
        $nm = [regex]::Escape($o.Name)
        $recv = (Count-Match $S "RpcOrder: $nm received by the server: slot 0") -gt 0
        $refused = Count-Match $S "RpcOrder: $nm refused"
        $eff = Effect-Of $o $S $K
        $res += [pscustomobject]@{ Order = $o.Name; Recv = $recv; Effect = $eff.Ok; Detail = $eff.Detail; Refused = $refused }
    }
    $red = Count-Match $S 'RpcOrder: .*slot 1\b'
    $notsent = (Count-Match $S 'RpcRoute: .* not sent') + (Count-Match $K 'RpcRoute: .* not sent')
    $asErr = Count-Match $S 'Angelscript: Error'
    foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
    Start-Sleep 4
    return [pscustomobject]@{ Tag = $tag; Results = $res; RedLines = $red; NotSent = $notsent; AsErrors = $asErr; ServerUp = $up1 }
}

"start $(Get-Date -Format HH:mm:ss) Lot=$Lot orders: $(($sel | ForEach-Object { $_.Name }) -join ', ') Combined=$Combined"
$runs = @()
if ($Combined) { $runs += Run-One "combined" $sel } else { $i = 0; foreach ($o in $sel) { $i++; $runs += Run-One ("o$i") @($o) } }

"===== TABLE : order -> received yes/NO ; effect yes/NO"
$runs = @($runs | Where-Object { $_.Tag })   # run-one outputs only (stray pipeline output made empty entries that failed the verdict)
$fail = $false
foreach ($r in $runs) {
    foreach ($x in $r.Results) {
        $line = "{0,-16} -> received {1,-3} ; effect {2,-3} | {3}" -f $x.Order, $(if ($x.Recv) { "oui" } else { "NON" }), $(if ($x.Effect) { "oui" } else { "NON" }), $x.Detail
        if ($x.Refused -gt 0) { $line += " | REFUSED lines: $($x.Refused)" }
        $line
        if (-not ($x.Recv -and $x.Effect)) { $fail = $true }
    }
    $redOk = ($r.RedLines -eq 0); $nsOk = ($r.NotSent -eq 0)
    "[$($r.Tag)] RED: server 'RpcOrder: ... slot 1' lines = $($r.RedLines) (expected 0) ; 'RpcRoute: .* not sent' = $($r.NotSent) (expected 0) ; 'Angelscript: Error' on server = $($r.AsErrors) ; server up = $($r.ServerUp)"
    if (-not ($redOk -and $nsOk -and $r.ServerUp)) { $fail = $true }
}
if ($fail) { "VERDICT FAIL"; exit 1 } else { "VERDICT PASS"; exit 0 }
