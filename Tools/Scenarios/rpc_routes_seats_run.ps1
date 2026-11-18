# MIG2-S3 routes proof WITH SEATS (Alice slot 0 green, Bob slot 1 red; match config with 2 Human seats) (session principale, 2026-11-17): the C++ interface calls (hol.Duel.Challenge, hol.Match.Pause, hol.Spec.Cancel, hol.Hero.Return) go through the script RPC actor (no C++ RPC and no hol.Rpc.Script* cvar left since lots B-E; lot E 2026-11-18)
# (the actor is their only entry). Dedicated server + 2 clients. Client 1 (RED): joins and makes no call -> the server must show NO script line (RpcDuel / RpcMatch / RpcSpec / RpcHero).
# Client 2 (GREEN): runs hol.Rpc.ProofRoute -> the server shows the script lines (received or refused: the client has no seat on this map, a refusal is a received call).
# Lot E: the server starts WITHOUT -ExecCmds (no cvar to set), the green client runs hol.Rpc.ProofRoute <n> alone. Needs the C++ lot built. Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\rpc_routes_run.ps1 -Root <worktree>
# D845 (lot B): the Duel, Specialization, Hero return, Match pause routes have NO C++ RPC and no cvar left (their script RPC is the only entry), so the old red witness "cvar at 0 -> the C++ RPC answers, no script line" does not exist any more.
# The RED client is now the witness "client with no call": it joins and executes NO route command (no hol.Rpc.ProofRoute, so no Duel / Spec / Hero / Match bit), the server must show 0 script lines for it; the GREEN client executes the commands.
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [string]$Out = (Join-Path $env:TEMP "rpc_routes"))
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7992
$cfg = Join-Path $Out "routes_seats.json"
$cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = 5; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
Set-Content -Encoding ascii $cfg $cfgJson
$sl = "$Out\routes_server.log"; $c1 = "$Out\routes_client_red.log"; $c2 = "$Out\routes_client_green.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
"start $(Get-Date -Format HH:mm:ss)"
$sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=RouteSrv -abslog=`"$sl`"" -PassThru -WindowStyle Hidden
"server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 90) $(Get-Date -Format HH:mm:ss)"
$a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=RouteCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
"red client joined, made no call: $(WaitFor $c1 'Welcomed by server' 90) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 5
# ProofRoute 31775 = 31 + 1024 (vision) + 2048 (hero ability) + 4096 (rage) + 8192 (watch) + 16384 (surrender, last): the server must also show the lines of the lot E map below.
$pat = 'RpcDuel: |RpcMatch: |RpcSpec: |RpcHero: |RpcCheat: '
$patE = 'Diplomacy order: |Hero ability order: |Rage order: '
"RED CHECK $(Get-Date -Format HH:mm:ss): script lines on the server = $((Select-String -Path $sl -Pattern $pat | Measure-Object).Count) (expected 0)"
$b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Alice -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=RouteCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Rpc.ProofRoute 31775`"" -PassThru -WindowStyle Hidden
"green client ran its commands: $(WaitFor $c2 'RpcRoute: the client runs .hol.Cheat.Resources' 90) $(Get-Date -Format HH:mm:ss)"
"green server received a script line: $(WaitFor $sl 'RpcMatch: ' 30) $(Get-Date -Format HH:mm:ss)"
foreach ($pt in 'Diplomacy order: slot 0 vision 1 toward slot 1 queued', 'RpcHero: ability received by the server: slot 0', 'RpcHero: rage received by the server: slot 0', 'RpcHero: watch received by the server: slot 0', 'RpcMatch: received by the server: slot 0 surrenders', 'Hero ability order: slot 0', 'Rage order: slot 0 queued') { "server has '$pt': $(WaitFor $sl ([regex]::Escape($pt)) 30)" }
"no RpcRoute not sent on the server: $(-not (Select-String -Path $sl -Pattern 'RpcRoute: .* not sent' -Quiet)) (expected True)"
Start-Sleep 5
foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
"=====SERVER (script lines)"; Select-String -Path $sl -Pattern "$pat|$patE|RpcPlayer|Angelscript: Error" | % { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 45
"=====RED CLIENT"; if (Test-Path $c1) { Select-String -Path $c1 -Pattern 'RpcRoute|NetworkFailure' | % { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) } | Select-Object -First 8 }
"=====GREEN CLIENT"; if (Test-Path $c2) { Select-String -Path $c2 -Pattern 'RpcRoute|NetworkFailure' | % { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) } | Select-Object -First 8 }
