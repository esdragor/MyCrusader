# ELROND-1124sh: reads the game log of the scenario monk_seal_slow (or monk_seal_slow_control) and checks the Seal of Light's slow (D859, x0.60).
# The enemy ManAtArms is the LAST slot-1 unit of each "Units where" line; the lines are one game second apart (wait 1).
# Step = x distance between two consecutive lines. A step is IN when both its ends are inside the Seal's zone (x -12200..-11000 on y=300),
# OUT when both are outside. Ratio = mean IN step / mean OUT step; PASS when it is <= 0.75 (the rule gives 0.60), FAIL otherwise (no Seal: 1.0).
# Usage: powershell -ExecutionPolicy Bypass -File monk_seal_slow_check.ps1 -Log <HeroesOfLiberty_N.log>
param([Parameter(Mandatory = $true)] [string] $Log)
$ZoneMinX = -12200; $ZoneMaxX = -11000
$Xs = @()
foreach ($Line in (Select-String -Path $Log -Pattern 'Units where' | ForEach-Object { $_.Line })) {
	$M = [regex]::Matches($Line, '#(\d+) slot 1 at \((-?\d+), (-?\d+)\) (\w+)')
	if ($M.Count -gt 0) { $Xs += [int]$M[$M.Count - 1].Groups[2].Value }
}
$In = @(); $Out = @()
for ($i = 1; $i -lt $Xs.Count; ++$i) {
	$A = $Xs[$i - 1]; $B = $Xs[$i]; $Step = $B - $A
	if ($Step -le 0) { continue }
	$AIn = ($A -ge $ZoneMinX -and $A -le $ZoneMaxX); $BIn = ($B -ge $ZoneMinX -and $B -le $ZoneMaxX)
	if ($AIn -and $BIn) { $In += $Step } elseif (-not $AIn -and -not $BIn -and ($A -lt $ZoneMinX -and $B -lt $ZoneMinX -or $A -gt $ZoneMaxX -and $B -gt $ZoneMaxX)) { $Out += $Step }
}
if ($In.Count -eq 0 -or $Out.Count -eq 0) { "FAIL: no full IN step ($($In.Count)) or OUT step ($($Out.Count)) in $($Xs.Count) readings"; exit 2 }
$MeanIn = ($In | Measure-Object -Average).Average; $MeanOut = ($Out | Measure-Object -Average).Average
$Ratio = [math]::Round($MeanIn / $MeanOut, 2)
"IN steps: $($In -join ', ') (mean $([int]$MeanIn)) ; OUT steps: $($Out -join ', ') (mean $([int]$MeanOut)) ; ratio $Ratio"
if ($Ratio -le 0.75) { "PASS: the enemy walks at $([int]($Ratio * 100)) % in the zone"; exit 0 } else { "FAIL: the enemy walks at $([int]($Ratio * 100)) % in the zone (expected ~60 %)"; exit 1 }
