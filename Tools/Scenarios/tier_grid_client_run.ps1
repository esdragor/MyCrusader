# MIG-CPP-UnitTier proof on a NETWORK CLIENT (reprise 12, 2026-11-18): since the C++ rule HOLUnitTier::Check is gone, the production grid of
# the HUD asks the script board (UHOLUnitTierSubsystem, HOLUnitTierBoardScript.as) whether a unit's tier is open. If that board did not exist in
# a client's world, every unit cell would be greyed for a joined player. Dedicated server + 2 seated clients (Alice slot 0, Bob slot 1); Alice
# selects her own forum and logs her HUD (hol.HUD.Report): the Peasant cell must be "enabled" and the client log must not have the line
# "UnitTier: no script board". Red witness: -Expect greyed after stubbing CheckUnitTier in Script\Units\HOLUnitTierBoardScript.as with
# "return HOLUnitTier::TierTooLow;" (the same client then shows the Peasant greyed: the grid really reads the script board on the client).
# Usage: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\tier_grid_client_run.ps1 -Root <worktree> [-Expect enabled|greyed]
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [string]$Out = (Join-Path $env:TEMP "tier_grid_client"), [ValidateSet('enabled', 'greyed')][string]$Expect = 'enabled')
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7994
$cfg = Join-Path $Out "tier_grid.json"
$cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = 5; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
Set-Content -Encoding ascii $cfg $cfgJson
$seq = Join-Path $Out "tier_grid_alice.seq"
@("waitplayers 2", "frames 30", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)", "frames 5", "hol.HUD.Report", "frames 5", "mark tier grid end") | Set-Content -Encoding utf8 $seq
$sl = "$Out\tier_grid_server.log"; $c1 = "$Out\tier_grid_bob.log"; $c2 = "$Out\tier_grid_alice.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
"start $(Get-Date -Format HH:mm:ss) (expect the Peasant cell $Expect)"
$sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=TierSrv -abslog=`"$sl`"" -PassThru -WindowStyle Hidden
"server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 180) $(Get-Date -Format HH:mm:ss)"
$a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=TierCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
"Bob joined: $(WaitFor $c1 'Welcomed by server' 120) $(Get-Date -Format HH:mm:ss)"
$b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Alice -game -RenderOffScreen -ResX=960 -ResY=540 -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -culture=fr -saveddirsuffix=TierCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Test.Run $seq`"" -PassThru -WindowStyle Hidden
"Alice reported her HUD: $(WaitFor $c2 'Test mark: tier grid end|mark tier grid end|HUD report: .*selection MainCentre' 150) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 3
foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
$report = if (Test-Path $c2) { Select-String -Path $c2 -Pattern 'HUD report: .*selection MainCentre' | Select-Object -Last 1 } else { $null }
$cell = if ($report) { [regex]::Match($report.Line, '\b\w+=Peasant (enabled|greyed|disabled)').Value } else { '' }
$noBoard = if (Test-Path $c2) { (Select-String -Path $c2 -Pattern 'UnitTier: no script board' | Measure-Object).Count } else { -1 }
"Alice's grid: $(if ($report) { [regex]::Match($report.Line, 'grid [^;]*?(?=, orders|, queue|$)').Value } else { 'NO HUD report with her forum selected' })"
"Peasant cell on the client: '$cell' ; 'no script board' lines in her log: $noBoard"
$ok = $cell -match "Peasant $Expect$" -and ($Expect -eq 'greyed' -or $noBoard -eq 0)
"tier_grid_client : $(if ($ok) { 'PASS' } else { 'FAIL' }) (expected $Expect)"
if (-not $ok -and (Test-Path $c2)) { "=====ALICE (last lines of the sequence)"; Select-String -Path $c2 -Pattern 'Test |Selection:|HUD report|UnitTier|NetworkFailure|Angelscript: Error' | Select-Object -Last 12 | % { $_.Line.Substring(0, [Math]::Min(260, $_.Line.Length)) } }
exit $(if ($ok) { 0 } else { 1 })
