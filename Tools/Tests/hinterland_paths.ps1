# The paths of L'Arriere-pays on the game's own navmesh (chantier Cartes, C2; modelled on the ridge's crossings.ps1): a solo match on
# <Map>_<Size> (Hinterland by default), then hol.Nav.Path between the places the recipe says (Tools/Tests/hinterland_dump.py: the
# slots, a gold vein of each pocket, the openings of the pockets, the passages of the belt), each judged by its length against the
# straight line (the rocks are NavArea_Null volumes: the navmesh knows them, the trees do not):
#   base -> its own pocket                 goes round by a passage and the corridor: at least MinDetour times the straight line
#   base -> the neighbour's pocket         the same (the pocket is not reached through the middle)
#   centre -> a pocket                     the same
#   base -> the neighbour's base           no detour (the middle is open): at most MaxOpen times the straight line
#   through a passage of the belt          a path exists and is short (the gap is wide enough): at most MaxGap
#   through an opening of a pocket         the same
# Off screen, no window. Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\hinterland_paths.ps1 [-Size Small] [-Map Hinterland] [-Root <copy>]
# -Map Steppe plays the same questions on a map without pockets: it must come out FAIL (the red of this test, no detour anywhere).
# Prints each path, then one line PASS / FAIL; exit code 0 / 1. Log: <copy>\Saved_Tests\Logs\hinterland_paths.log
param(
    [string] $Size = "Small",
    [string] $Map = "Hinterland",
    [double] $MinDetour = 1.25,
    [double] $MaxOpen = 1.15,
    [double] $MaxGap = 1.35,
    [double] $CentreX = -1500,
    [double] $CentreY = 2500,
    [string] $Root = "",
    [string] $EngineDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "hinterland_paths : FAIL (engine '$Association' of the .uproject not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Dump = (& python (Join-Path $PSScriptRoot "hinterland_dump.py") $Size) -join "" | ConvertFrom-Json
if (-not $Dump) { Write-Host "hinterland_paths : FAIL (hinterland_dump.py gave nothing for $Size)"; exit 1 }
$Saved = Join-Path $Project "Saved_Tests"
$Log = Join-Path $Saved "Logs\hinterland_paths.log"
$Sequence = Join-Path $Saved "hinterland_paths.seq"
$Config = Join-Path $Saved "hinterland_paths.json"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
Remove-Item $Log -ErrorAction SilentlyContinue
$Slots = for ($i = 0; $i -lt 8; $i++) { "{ `"state`": `"$(if ($i -eq 0) { 'Human' } else { 'Closed' })`", `"colorIndex`": $i }" }
"{ `"version`": 3, `"seed`": 1, `"map`": `"/Game/Maps/$Map.$Map`", `"mapSize`": `"$Size`", `"bDrawPlaces`": false, `"bTeamsTogether`": true, `"slots`": [ $($Slots -join ', ') ] }" | Set-Content -Encoding ascii $Config

function Point($P) { [string]::Format([Globalization.CultureInfo]::InvariantCulture, "{0:F0} {1:F0}", $P.x, $P.y) }
function Away($P, [double] $Distance) { $H = [Math]::Sqrt($P.x * $P.x + $P.y * $P.y); [pscustomobject]@{ x = $P.x + $P.x / $H * $Distance; y = $P.y + $P.y / $H * $Distance } }
# 22 m in front of each slot, towards the middle: the main centre made at the slot at the start has a hole in the navmesh.
$Slot0 = Away $Dump.slots[0] -2200; $Slot1 = Away $Dump.slots[1] -2200
# 7 m outward of a vein, towards the back wall: a source is a hole of the navmesh once the match has started.
$Gold0 = Away ($Dump.veins.'0' | Where-Object { $_.type -like "GoldVein*" } | Select-Object -First 1) 700
$Gold1 = Away ($Dump.veins.'1' | Where-Object { $_.type -like "GoldVein*" } | Select-Object -First 1) 700
$Passage = $Dump.passages[0]
$Opening = $Dump.openings | Where-Object { $_.slot -eq 0 } | Select-Object -First 1
# Near the middle of the map (not exactly on it: the point (0, 0) is a corner of the navmesh's tiles, off it for the projection).
$Zero = [pscustomobject]@{ x = $CentreX; y = $CentreY }
# name, from, to, kind: detour = at least MinDetour, open = at most MaxOpen, gap = at most MaxGap (a path that exists)
$Queries = @(
    @{ Name = "base -> own pocket"; From = $Slot0; To = $Gold0; Kind = "detour" },
    @{ Name = "base -> neighbour's pocket"; From = $Slot0; To = $Gold1; Kind = "detour" },
    @{ Name = "own pocket -> centre"; From = $Gold0; To = $Zero; Kind = "detour" },
    @{ Name = "base -> neighbour's base"; From = $Slot0; To = $Slot1; Kind = "open" },
    @{ Name = "through a passage of the belt"; From = (Away $Passage -1600); To = (Away $Passage 1600); Kind = "gap" },
    @{ Name = "through an opening of a pocket"; From = (Away $Opening 1600); To = (Away $Opening -900); Kind = "gap" }
)
$Lines = @("waitmap MainMenu", "waitui", "hol.Match.Open $($Config.Replace([string][char]92, '/'))", "waitmap ${Map}_$Size", "waitlog Camera above the start of slot 0", "frames 5", "wait 5")
foreach ($Query in $Queries) { $Lines += "hol.Nav.Path $(Point $Query.From) $(Point $Query.To)" }
$Lines += "frames 2", "quit"
$Lines | Set-Content -Encoding ascii $Sequence

$Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -nosplash -unattended -Multiprocess -culture=fr -LiveCoding=0 -asdebugport=0 -as-exit-on-error -saveddirsuffix=Tests -abslog=`"$Log`" -ExecCmds=`"hol.Test.Run $Sequence`""
$Process = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru
if (-not $Process.WaitForExit(240000)) { $Process.Kill() }

$Text = if (Test-Path $Log) { Get-Content $Log } else { @() }
$Results = @($Text | Select-String -Pattern "Nav path: (.*)" | ForEach-Object { $_.Matches[0].Groups[1].Value })
if ($Results.Count -ne $Queries.Count) { Write-Host "hinterland_paths : FAIL ($($Results.Count) path lines for $($Queries.Count) questions; log $Log)"; exit 1 }
$Failed = 0
for ($i = 0; $i -lt $Queries.Count; $i++) {
    $Query = $Queries[$i]; $Result = $Results[$i]
    $Match = [regex]::Match($Result, "length ([\d.]+) m, straight ([\d.]+) m, ratio ([\d.]+)")
    $Ok = $false; $Ratio = "-"
    if ($Match.Success -and $Result -notmatch "partial") {
        $Ratio = [double]::Parse($Match.Groups[3].Value, [Globalization.CultureInfo]::InvariantCulture)
        $Ok = switch ($Query.Kind) { "detour" { $Ratio -ge $MinDetour } "open" { $Ratio -le $MaxOpen } "gap" { $Ratio -le $MaxGap } }
    }
    if (-not $Ok) { $Failed++ }
    Write-Host ("  {0,-34} {1,-4} length {2} m, straight {3} m, ratio {4}   [{5}]" -f $Query.Name, $(if ($Ok) { "ok" } else { "FAIL" }), $Match.Groups[1].Value, $Match.Groups[2].Value, $Ratio, $Query.Kind)
    if (-not $Ok) { Write-Host "      $Result" }
}
Write-Host "hinterland_paths : $(if ($Failed -eq 0) { 'PASS' } else { 'FAIL' }) ($Map $Size, $Failed of $($Queries.Count) questions failed)"
exit $(if ($Failed -eq 0) { 0 } else { 1 })
