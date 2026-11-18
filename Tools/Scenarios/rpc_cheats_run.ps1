# P2 / P1 proof of the C++ -> script RPC routes for the 13 cheats and the Hero / Duel / Diplomacy / Economy routes not yet proved (ELROND-1118b, PLAN-1118a sections 2 and 3). NOT RUN when written (machine saturated).
# Model: rpc_routes_seats_run.ps1. Dedicated server with -HOLCheats, Plain, 2 Human seats (Alice slot 0 = GREEN client, Bob slot 1 = RED client).
# SERVER and GREEN client: the 13 cheat cvars hol.Rpc.ScriptCheat<Name> at 1 (plus ScriptHero / ScriptDiplomacy at 1 for the P1 routes; the Economy, Duel and Specialization cvars are retired, D845).
# D845 (lot C): the Resources, Signal and Chat cheats/RPCs have no C++ RPC and no cvar left either (the Resources cvar is deleted). ScriptSignal / ScriptChat stay (the C++ Client relays read them), they are not set here.
# RED client (D845, witness "client with no call"): it joins and executes NO ProofRoute / ProofCheats (no cvar, no command), so the server must write NO "RpcCheat: / RpcHero: / RpcDuel: / RpcDiplomacy: / RpcEconomy: / RpcMatch: / RpcSpec: " line before the green client joins (the red check).
# The green client fires, 4 s after its first owner tick, the hook of Script\Net\HOLRpcPlayer.as: hol.Rpc.ProofCheats (13 bits, the table below) and hol.Rpc.ProofRoute (P1 bits 32 talent, 64 duel answer, 128 stance, 256 cancel production).
# The server must write "RpcCheat: <name> received" for each cheat (log lines added in the script bodies by ELROND-1118b). The table prints name -> received yes/no.
# Known limits (said in the report): Defeat / FallForum / SkipForum / HoldPresence target slot 7 (a Closed slot, so the match does not end); if the rules refuse a closed slot the table says "refused", which still proves the route.
# DamageBuilding (hol.Cheat.DamageNear) and SpawnBuilding (type Barracks, NOT checked against Buildings.csv) need the client to see its own main centre (replicated): on Plain with a seat they should exist.
# WatchUnit has NO console command and no script hook (it fires on a selection change of an own gatherer, C++ only): not proved here, "requires C++" (or a selection command).
# Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\rpc_cheats_run.ps1 -Root <worktree> [-Cheats 8191] [-Routes 0]     (P1: -Cheats 0 -Routes 480)
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [string]$Out = (Join-Path $env:TEMP "rpc_cheats"), [int]$Cheats = 8191, [int]$Routes = 0)
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7993
$cfg = Join-Path $Out "cheats_seats.json"
$cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = 5; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
Set-Content -Encoding ascii $cfg $cfgJson
$sl = "$Out\cheats_server.log"; $c1 = "$Out\cheats_client_red.log"; $c2 = "$Out\cheats_client_green.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
# name, bit of hol.Rpc.ProofCheats, cvar suffix, expected server line.
$cheatTable = @(
  @{ Name = 'FastBuild'; Bit = 1; Pat = 'RpcCheat: fast build received' },
  @{ Name = 'Population'; Bit = 2; Pat = 'RpcCheat: population received' },
  @{ Name = 'Technology'; Bit = 4; Pat = 'RpcCheat: technology received' },
  @{ Name = 'Tier'; Bit = 8; Pat = 'RpcCheat: tier received' },
  @{ Name = 'UnitSpeed'; Bit = 16; Pat = 'RpcCheat: unit speed received' },
  @{ Name = 'GatherSpeed'; Bit = 32; Pat = 'RpcCheat: gather speed received' },
  @{ Name = 'Wound'; Bit = 64; Pat = 'RpcCheat: wound received' },
  @{ Name = 'DamageBuilding'; Bit = 128; Pat = 'RpcCheat: damage building received' },
  @{ Name = 'SpawnBuilding'; Bit = 256; Pat = 'RpcCheat: spawn building received' },
  @{ Name = 'Defeat'; Bit = 512; Pat = 'RpcCheat: defeat received' },
  @{ Name = 'FallForum'; Bit = 1024; Pat = 'RpcCheat: fall forum received' },
  @{ Name = 'SkipForum'; Bit = 2048; Pat = 'RpcCheat: skip forum received' },
  @{ Name = 'HoldPresence'; Bit = 4096; Pat = 'RpcCheat: hold presence received' }
)
$routeTable = @(
  @{ Name = 'HeroTalent'; Bit = 32; Pat = 'RpcHero: talent received' },
  @{ Name = 'DuelAnswer'; Bit = 64; Pat = 'RpcDuel: received by the server: slot \d+ (accept|refuse)' },
  @{ Name = 'DiplomacyStance'; Bit = 128; Pat = 'RpcDiplomacy: received by the server: sender slot 0' },
  @{ Name = 'CancelProduction'; Bit = 256; Pat = 'RpcEconomy: cancel received' }
)
$cheatCvars = @($cheatTable | ForEach-Object { "hol.Rpc.ScriptCheat$($_.Name)" }) + @('hol.Rpc.ScriptHero', 'hol.Rpc.ScriptDiplomacy')
$on = ($cheatCvars | ForEach-Object { "$_ 1" }) -join ','
$proof = "hol.Rpc.ProofCheats $Cheats,hol.Rpc.ProofRoute $Routes"
"start $(Get-Date -Format HH:mm:ss) cheats=$Cheats routes=$Routes"
$sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=CheatSrv -abslog=`"$sl`" -ExecCmds=`"$on`"" -PassThru -WindowStyle Hidden
"server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 90) $(Get-Date -Format HH:mm:ss)"
# RED client (D845): joins, runs no ProofRoute / ProofCheats and no cvar command. No script line expected on the server.
$a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=CheatCli1 -abslog=`"$c1`" -PassThru -WindowStyle Hidden
"red client joined, made no call: $(WaitFor $c1 'Welcomed by server' 90) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 12
$pat = 'RpcCheat: |RpcHero: |RpcDuel: |RpcDiplomacy: |RpcEconomy: |RpcMatch: |RpcSpec: '
$redCount = (Select-String -Path $sl -Pattern $pat | Measure-Object).Count
"RED CHECK $(Get-Date -Format HH:mm:ss): script lines on the server = $redCount (expected 0)"
# GREEN client: cvars 1 on the client too, same hooks.
$b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Alice -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=CheatCli2 -abslog=`"$c2`" -ExecCmds=`"$on,$proof`"" -PassThru -WindowStyle Hidden
"green client ran its commands: $(WaitFor $c2 'RpcRoute: the client runs' 90) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 15
foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
"=====TABLE (server received a script line for the green client)"
foreach ($row in $cheatTable) { if (($Cheats -band $row.Bit) -ne 0) { $got = (Select-String -Path $sl -Pattern $row.Pat | Measure-Object).Count -gt 0; $ref = (Select-String -Path $sl -Pattern ('RpcCheat: ' + ($row.Pat -replace '^RpcCheat: ', '' -replace ' received$', '') + ' refused') | Measure-Object).Count -gt 0; "{0,-18} -> received {1}{2}" -f $row.Name, $(if ($got) { 'oui' } else { 'non' }), $(if ($ref) { ' (refused line seen)' } else { '' }) } }
foreach ($row in $routeTable) { if (($Routes -band $row.Bit) -ne 0) { $got = (Select-String -Path $sl -Pattern $row.Pat | Measure-Object).Count -gt 0; "{0,-18} -> received {1}" -f $row.Name, $(if ($got) { 'oui' } else { 'non' }) } }
"WatchUnit          -> not proved (no console command, no script hook: requires C++ or a selection command)"
"RED CHECK again: script lines before the green client = $redCount (expected 0)"
"=====SERVER (script lines)"; Select-String -Path $sl -Pattern "$pat|RpcPlayer|Angelscript: Error" | ForEach-Object { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 50
"=====RED CLIENT"; if (Test-Path $c1) { Select-String -Path $c1 -Pattern 'Welcomed by server|RpcRoute|NetworkFailure|Angelscript: Error' | ForEach-Object { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) } | Select-Object -First 20 }
"=====GREEN CLIENT"; if (Test-Path $c2) { Select-String -Path $c2 -Pattern 'RpcRoute|NetworkFailure|Angelscript: Error' | ForEach-Object { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) } | Select-Object -First 20 }
