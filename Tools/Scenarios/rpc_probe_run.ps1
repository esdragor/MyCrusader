# MIG-P1d replay: dedicated server + 2 clients; red client (hol.Dev.RpcProbeSend 0) then green client (1). Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\rpc_probe_run.ps1 -Root <worktree>
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [string]$Out = (Join-Path $env:TEMP "rpc_probe"))
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$s = $Out
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7988
$sl = "$s\rpc2_server.log"; $c1 = "$s\rpc2_client_red.log"; $c2 = "$s\rpc2_client_green.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
"start $(Get-Date -Format HH:mm:ss)"
$sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -saveddirsuffix=RpcSrv -abslog=`"$sl`"" -PassThru -WindowStyle Hidden
"server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 90) $(Get-Date -Format HH:mm:ss)"
$a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:$port -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=RpcCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
"server spawned: $(WaitFor $sl 'RpcProbe: the C\+\+ bridge spawned' 90) $(Get-Date -Format HH:mm:ss)"
"red client sees probe: $(WaitFor $c1 'RpcProbe: the client sees' 40) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 6
"RED CHECK $(Get-Date -Format HH:mm:ss): server line 'received by the server' present = $(Select-String -Path $sl -Pattern 'received by the server' -Quiet)"
$b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:$port -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=RpcCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Dev.RpcProbeSend 1`"" -PassThru -WindowStyle Hidden
"green server received: $(WaitFor $sl 'received by the server' 90) $(Get-Date -Format HH:mm:ss)"
"green client pong: $(WaitFor $c2 'RpcProbe: received by the client' 30) $(Get-Date -Format HH:mm:ss)"
foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
"=====SERVER"; Select-String -Path $sl -Pattern 'RpcProbe|HOLScriptRpcProbe|Angelscript: Error|Join succeeded' | % { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 15
"=====RED CLIENT"; if (Test-Path $c1) { Select-String -Path $c1 -Pattern 'RpcProbe|NetworkFailure' | % { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 8 }
"=====GREEN CLIENT"; if (Test-Path $c2) { Select-String -Path $c2 -Pattern 'RpcProbe|NetworkFailure' | % { $_.Line.Substring(0, [Math]::Min(220, $_.Line.Length)) } | Select-Object -First 8 }
