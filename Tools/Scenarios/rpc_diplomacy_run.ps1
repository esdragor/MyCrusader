# MIG2-S3 pilot proof (ELROND-1117o): ServerDiplomacyStance written in script (Script/Net/HOLRpcPlayer.as), dedicated server + 2 clients. Red client: hol.Rpc.DiplomacyTarget -1 (the
# server's line 'RpcDiplomacy: received by the server' must be ABSENT); green client: hol.Rpc.DiplomacyTarget 1, stance 2 (the line present, then 'Diplomacy order: slot N ... queued' if the
# client has a slot, or 'RpcDiplomacy: refused (1)' if it has none on this map). Needs the C++ lot built. Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\rpc_diplomacy_run.ps1 -Root <worktree>
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [string]$Out = (Join-Path $env:TEMP "rpc_diplomacy"))
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$s = $Out
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7989
$sl = "$s\dip_server.log"; $c1 = "$s\dip_client_red.log"; $c2 = "$s\dip_client_green.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
"start $(Get-Date -Format HH:mm:ss)"
$sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -saveddirsuffix=DipSrv -abslog=`"$sl`"" -PassThru -WindowStyle Hidden
"server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 90) $(Get-Date -Format HH:mm:ss)"
$a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:$port -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=DipCli1 -abslog=`"$c1`" -ExecCmds=`"hol.Rpc.DiplomacyTarget -1`"" -PassThru -WindowStyle Hidden
"server spawned the actor: $(WaitFor $sl 'RpcPlayer: the C\+\+ bridge spawned' 90) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 10
"RED CHECK $(Get-Date -Format HH:mm:ss): server line 'RpcDiplomacy: received by the server' present = $(Select-String -Path $sl -Pattern 'RpcDiplomacy: received by the server' -Quiet) (expected False)"
$b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:$port -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=DipCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Rpc.DiplomacyTarget 1,hol.Rpc.DiplomacyStance 2`"" -PassThru -WindowStyle Hidden
"green client called: $(WaitFor $c2 'RpcDiplomacy: the client calls ServerDiplomacyStance' 90) $(Get-Date -Format HH:mm:ss)"
"green server received: $(WaitFor $sl 'RpcDiplomacy: received by the server' 30) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 3
foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
"=====SERVER"; Select-String -Path $sl -Pattern 'RpcPlayer|RpcDiplomacy|Diplomacy order|Angelscript: Error|Join succeeded' | % { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 20
"=====RED CLIENT"; if (Test-Path $c1) { Select-String -Path $c1 -Pattern 'RpcDiplomacy|NetworkFailure' | % { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 8 }
"=====GREEN CLIENT"; if (Test-Path $c2) { Select-String -Path $c2 -Pattern 'RpcDiplomacy|NetworkFailure' | % { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 8 }
