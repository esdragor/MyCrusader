# A GROUP on the water of Les Gues (chantier Cartes, C7): runs the scenario fords_group (10 Men-at-arms ordered by the player's own group
# march order on Fords_Medium: phase "bridge" from in front of the base to the middle through the bridge, phase "round" from the middle to a
# point across the water far from every gap) and reads from the log, at every reading, how many soldiers stand IN the water (inside the band
# and outside the gaps' corridors). Exit 0 only if none ever does and the group reaches both goals.
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\fords_group.ps1 [-Root <wt>] [-NoRun]
param([string] $Root = "", [switch] $NoRun)
if (-not $Root) { $Root = (Resolve-Path "$PSScriptRoot\..\..").Path }
$Log = Join-Path $Root "Saved_Tests\Logs\scenario_fords_group.log"
if (-not $NoRun) {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $Root "Tools\Scenarios\check.ps1") fords_group -Root $Root -TimeoutSeconds 300 | Out-Null
}
if (-not (Test-Path $Log)) { Write-Host "fords_group: no log"; exit 1 }
# Fords_Medium: 4 places, ring radius 0.35 x 23100 cm, a bridge (16 m) on each place's axis, a ford (5 m) 0.3 x 90 degrees counterclockwise of it.
$Ring = 0.35 * 23100.0; $HalfBand = 900.0
$Gaps = @()
foreach ($A in 180, 270, 0, 90) { $Gaps += @{ Angle = $A; Half = 800.0 }; $Gaps += @{ Angle = ($A + 27) % 360; Half = 250.0 } }
function InWater($X, $Y) {
    $R = [Math]::Sqrt($X * $X + $Y * $Y)
    if ([Math]::Abs($R - $Ring) -gt $HalfBand) { return $false }
    $Angle = [Math]::Atan2($Y, $X) * 180.0 / [Math]::PI
    foreach ($G in $Gaps) {
        $D = [Math]::Abs((($Angle - $G.Angle + 540.0) % 360.0) - 180.0) * [Math]::PI / 180.0 * $Ring  # arc length from the gap's axis
        if ($D -le $G.Half + 60.0) { return $false }
    }
    return $true
}
$Phase = ""; $Drowned = 0; $Readings = 0; $Mean = @{}
foreach ($Line in Get-Content $Log) {
    if ($Line -match "Scenario note: PHASE (\w+)") { $Phase = $Matches[1]; continue }
    if ($Phase -and $Line -match "Units where: \d+ units: (.*)$") {
        $Points = [regex]::Matches($Matches[1], "slot 0 at \((-?\d+), (-?\d+)\)")
        if ($Points.Count -eq 0) { continue }
        $Readings++; $Sx = 0; $Sy = 0
        foreach ($P in $Points) {
            $X = [double]$P.Groups[1].Value; $Y = [double]$P.Groups[2].Value; $Sx += $X; $Sy += $Y
            if (InWater $X $Y) { $Drowned++; Write-Host "  in the water: ($X, $Y) phase $Phase" }
        }
        $Mean[$Phase] = @(($Sx / $Points.Count), ($Sy / $Points.Count))
    }
}
Write-Host "readings $Readings, soldier readings in the water $Drowned"
$Ok = $true
function Check($Name, $Cond) { if ($Cond) { Write-Host "  ok   $Name" } else { Write-Host "  RED  $Name"; $script:Ok = $false } }
Check "enough readings (20)" ($Readings -ge 20)
Check "nobody in the water" ($Drowned -eq 0)
if ($Mean.ContainsKey("bridge")) { Check "phase bridge: the group's mean ends within 25 m of the middle" ([Math]::Sqrt($Mean["bridge"][0] * $Mean["bridge"][0] + $Mean["bridge"][1] * $Mean["bridge"][1]) -le 2500) } else { Check "phase bridge read" $false }
if ($Mean.ContainsKey("round")) { Check "phase round: the group's mean ends within 30 m of the goal across the water" ([Math]::Sqrt([Math]::Pow($Mean["round"][0] + 5792, 2) + [Math]::Pow($Mean["round"][1] + 9451, 2)) -le 3000) } else { Check "phase round read" $false }
if ($Ok) { Write-Host "fords_group PASS"; exit 0 } else { Write-Host "fords_group FAIL"; exit 1 }
