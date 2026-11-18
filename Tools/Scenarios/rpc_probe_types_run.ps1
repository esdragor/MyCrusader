# ELROND-1118f L2 S0 (D846): proof of the 4 script-RPC parameter-type combinations of the 16 L2 orders. Dedicated server + a red client (hol.Rpc.ProbeTypes 0: no "RpcProbeTypes" line) then a green client (1).
# Prints a table "combination -> received with the expected values yes/no". Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\rpc_probe_types_run.ps1 -Root <worktree>
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [string]$Out = (Join-Path $env:TEMP "rpc_probe_types"))
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$s = $Out
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7989
$sl = "$s\types_server.log"; $c1 = "$s\types_client_red.log"; $c2 = "$s\types_client_green.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
# A number as the journal may print it: 1001 or 1001.0 or 1001.000000.
function N($v) { return "$v(\.0*)?" }
function P($x, $y, $z) { return "\(" + (N $x) + ", " + (N $y) + ", " + (N $z) + "\)" }
"start $(Get-Date -Format HH:mm:ss)"
$sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -saveddirsuffix=RpcTypesSrv -abslog=`"$sl`"" -PassThru -WindowStyle Hidden
"server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 90) $(Get-Date -Format HH:mm:ss)"
$a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:$port -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=RpcTypesCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
"server spawned: $(WaitFor $sl 'RpcProbe: the C\+\+ bridge spawned' 90) $(Get-Date -Format HH:mm:ss)"
"red client sees probe: $(WaitFor $c1 'RpcProbe: the client sees' 40) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 6
$redLines = (Select-String -Path $sl -Pattern 'RpcProbeTypes' -ErrorAction SilentlyContinue | Measure-Object).Count
"RED CHECK $(Get-Date -Format HH:mm:ss): server 'RpcProbeTypes' lines = $redLines (0 expected)"
$b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:$port -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=RpcTypesCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Rpc.ProbeTypes 1`"" -PassThru -WindowStyle Hidden
"green server received DropOff (last call): $(WaitFor $sl 'RpcProbeTypes: DropOff received' 90) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 3
foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
# Expected values: the 10-parameter-by-parameter lines of Script\Net\HOLScriptRpcProbe.as (FireTypesProbe). Buildings are nullptr or an actor name: any non-empty token proves the AHOLBuilding type.
$ids = "11,22,33"
$checks = @(
    @{ Name = "(a) 2 x FVector_NetQuantize (MoveInLine)";            Pat = "RpcProbeTypes: MoveInLine received: Start=" + (P 1001 2 3) + " End=" + (P 1004 5 6) },
    @{ Name = "(b) FName + point + TArray<uint32> (PlaceBuilding)";  Pat = "RpcProbeTypes: PlaceBuilding received: Name=ProbeHouse Point=" + (P 2001 7 -8) + " Ids=\[$ids\] Count=3" },
    @{ Name = "(c) 2 x AHOLBuilding + point + int + bool (SetRally)"; Pat = "RpcProbeTypes: SetRallyPoint received: BuildingA=\S+ BuildingB=\S+ Point=" + (P 3001 9 10) + " Int=42 Bool=true" },
    @{ Name = "(d) TArray<uint32> + AHOLBuilding + point (DropOff)";  Pat = "RpcProbeTypes: DropOff received: Ids=\[$ids\] Count=3 Building=\S+ Point=" + (P 4001 11 12) }
)
"=====TABLE (combination -> received with the expected values)"
$allOk = $true
foreach ($c in $checks) {
    $hit = $null
    if (Test-Path $sl) { $hit = Select-String -Path $sl -Pattern $c.Pat | Select-Object -First 1 }
    $ok = [bool]$hit
    if (-not $ok) { $allOk = $false }
    "{0,-62} -> {1}" -f $c.Name, $(if ($ok) { "oui" } else { "NON" })
}
"RED (no cvar, no RpcProbeTypes line on the server): $(if ($redLines -eq 0) { 'oui' } else { 'NON' })"
"VERDICT: $(if ($allOk -and $redLines -eq 0) { 'PASS' } else { 'FAIL' })"
"=====SERVER"; if (Test-Path $sl) { Select-String -Path $sl -Pattern 'RpcProbeTypes|Angelscript: Error|HOLScriptRpcProbe' | % { $_.Line.Substring(0, [Math]::Min(260, $_.Line.Length)) } | Select-Object -First 15 }
"=====GREEN CLIENT"; if (Test-Path $c2) { Select-String -Path $c2 -Pattern 'RpcProbeTypes|NetworkFailure|Angelscript: Error' | % { $_.Line.Substring(0, [Math]::Min(260, $_.Line.Length)) } | Select-Object -First 10 }
