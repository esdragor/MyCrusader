# The paths of Les Gues on the game's own navmesh (chantier Cartes, C7; modelled on cols_paths.ps1): a solo match on <Map>_<Size>
# (Fords by default), then hol.Nav.Path between the places the recipe says (Tools/Tests/fords_dump.py: the slots, the points on each side of
# every ford and bridge), each judged by its length against the straight line (the water is NavArea_Null volumes the navmesh knows):
#   across the water, far from every gap   the water is in the way: the way round (a ford or a bridge) is at least MinWall times the straight line
#   base -> neighbour's base               a path exists and is not far longer than the straight line: at most MaxNeighbour
#   base -> the middle                     reachable: at most MaxOpen (through the bridge in front of the base, or the river's bridge)
#   across each ford and bridge            a path exists and is short (the gap is wide enough): at most MaxGap
# Off screen, no window. Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\fords_paths.ps1 [-Size Medium] [-Map Fords] [-Root <copy>]
# -Map Steppe plays the same questions on a map without water: it must come out FAIL (the water does not exist: ratio about 1; the red of this test).
# Prints each path, then one line PASS / FAIL; exit code 0 / 1. Log: <copy>\Saved_Tests\Logs\fords_paths.log
param(
    [string] $Size = "Medium",
    [string] $Map = "Fords",
    [double] $MinWall = 1.3,
    [double] $MaxNeighbour = 1.6,
    [double] $MaxOpen = 1.3,
    [double] $MaxGap = 1.35,
    [string] $Root = "",
    [string] $EngineDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "fords_paths : FAIL (engine '$Association' of the .uproject not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Dump = (& python (Join-Path $PSScriptRoot "fords_dump.py") $Size) -join "" | ConvertFrom-Json
if (-not $Dump) { Write-Host "fords_paths : FAIL (fords_dump.py gave nothing for $Size)"; exit 1 }
$Saved = Join-Path $Project "Saved_Tests"
$Log = Join-Path $Saved "Logs\fords_paths.log"
$Sequence = Join-Path $Saved "fords_paths.seq"
$Config = Join-Path $Saved "fords_paths.json"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
Remove-Item $Log -ErrorAction SilentlyContinue
$Slots = for ($i = 0; $i -lt $Dump.places; $i++) { "{ `"state`": `"$(if ($i -eq 0) { 'Human' } else { 'Closed' })`", `"colorIndex`": $i }" }
"{ `"version`": 3, `"seed`": 1, `"map`": `"/Game/Maps/$Map.$Map`", `"mapSize`": `"$Size`", `"bDrawPlaces`": false, `"bTeamsTogether`": true, `"slots`": [ $($Slots -join ', ') ] }" | Set-Content -Encoding ascii $Config

function Point($P) { [string]::Format([Globalization.CultureInfo]::InvariantCulture, "{0:F0} {1:F0}", $P.x, $P.y) }
function Away($P, [double] $Distance) { $H = [Math]::Sqrt($P.x * $P.x + $P.y * $P.y); [pscustomobject]@{ x = $P.x + $P.x / $H * $Distance; y = $P.y + $P.y / $H * $Distance } }
# 22 m in front of each slot, towards the middle: the main centre made at the slot at the start has a hole in the navmesh.
$Slot0 = Away $Dump.slots[0] -2200
$Middle = [pscustomobject]@{ x = 0; y = 0 }
$Queries = @()
foreach ($Index in @($Dump.neighbours)) {
    $Queries += @{ Name = "base -> neighbour's base ($Index)"; From = $Slot0; To = (Away $Dump.slots[$Index] -2200); Kind = "reach" }
}
foreach ($Wall in @($Dump.walls)) {
    $Queries += @{ Name = "across the water ($($Wall.label))"; From = [pscustomobject]@{ x = $Wall.ax; y = $Wall.ay }; To = [pscustomobject]@{ x = $Wall.bx; y = $Wall.by }; Kind = "wall" }
}
$Queries += @{ Name = "base -> the middle"; From = $Slot0; To = $Middle; Kind = "open" }
foreach ($Pass in @($Dump.passes)) {
    $Queries += @{ Name = "across $($Pass.label)"; From = [pscustomobject]@{ x = $Pass.ax; y = $Pass.ay }; To = [pscustomobject]@{ x = $Pass.bx; y = $Pass.by }; Kind = "gap" }
}
$Lines = @("waitmap MainMenu", "waitui", "hol.Match.Open $($Config.Replace([string][char]92, '/'))", "waitmap ${Map}_$Size", "waitlog Camera above the start of slot 0", "frames 5", "wait 5")
foreach ($Query in $Queries) { $Lines += "hol.Nav.Path $(Point $Query.From) $(Point $Query.To)" }
$Lines += "frames 2", "quit"
$Lines | Set-Content -Encoding ascii $Sequence

. (Join-Path $PSScriptRoot "..\Scenarios\EditorSlot.ps1")
# One of the 2 editor slots of the machine (EditorSlot.ps1, L7-3), freed once the editor is done.
$Slot = Enter-EditorSlot -Name "fords_paths" -Root $Project -Max 2 -WaitSeconds 1500
if (-not $Slot) { Write-Host "fords_paths : FAIL (no editor slot after 25 min: 2 editors already run on this machine)"; exit 1 }
$Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -nosplash -unattended -Multiprocess -culture=fr -LiveCoding=0 -asdebugport=0 -as-exit-on-error -saveddirsuffix=Tests -abslog=`"$Log`" -ExecCmds=`"hol.Test.Run $Sequence`""
$Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru -WindowStyle Hidden
if (-not $Process.WaitForExit(240000)) { $Process.Kill() }
Exit-EditorSlot

$Text = if (Test-Path $Log) { Get-Content $Log } else { @() }
$Results = @($Text | Select-String -Pattern "Nav path: (.*)" | ForEach-Object { $_.Matches[0].Groups[1].Value })
if ($Results.Count -ne $Queries.Count) { Write-Host "fords_paths : FAIL ($($Results.Count) path lines for $($Queries.Count) questions; log $Log)"; exit 1 }
$Failed = 0; $Skipped = 0
for ($i = 0; $i -lt $Queries.Count; $i++) {
    $Query = $Queries[$i]; $Result = $Results[$i]
    if (($Query.Kind -eq "wall" -or $Query.Kind -eq "gap") -and $Result -match "off the navmesh") { $Skipped++; Write-Host ("  {0,-34} skip (a point of the pair is off the navmesh: a source's hole)" -f $Query.Name); continue }
    $Match = [regex]::Match($Result, "length ([\d.]+) m, straight ([\d.]+) m, ratio ([\d.]+)")
    $Ok = $false; $Ratio = "-"
    if ($Match.Success -and $Result -notmatch "partial") {
        $Ratio = [double]::Parse($Match.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
        $Ok = switch ($Query.Kind) { "wall" { $Ratio -ge $MinWall } "reach" { $Ratio -le $MaxNeighbour } "open" { $Ratio -le $MaxOpen } "gap" { $Ratio -le $MaxGap } }
    }
    if (-not $Ok) { $Failed++ }
    Write-Host ("  {0,-34} {1,-4} length {2} m, straight {3} m, ratio {4}   [{5}]" -f $Query.Name, $(if ($Ok) { "ok" } else { "FAIL" }), $Match.Groups[1].Value, $Match.Groups[2].Value, $Ratio, $Query.Kind)
    if (-not $Ok) { Write-Host "      $Result" }
}
if (($Queries | Where-Object { $_.Kind -eq "wall" }).Count - $Skipped -lt 1) { Write-Host "  no water could be tried"; $Failed++ }
Write-Host "fords_paths : $(if ($Failed -eq 0) { 'PASS' } else { 'FAIL' }) ($Map $Size, $Failed of $($Queries.Count) questions failed, $Skipped skipped)"
exit $(if ($Failed -eq 0) { 0 } else { 1 })
