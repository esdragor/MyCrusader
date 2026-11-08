# Screenshots of a map's ground from the game's camera (chantier Cartes, sol CC0), off screen, no window. Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\ground_shots.ps1 -Tag before|after [-Map Steppe] [-Size Small] [-Root <copy>]
# One solo match on <Map>_<Size> (2 slots, no places drawn), then captures at the world's centre (nothing there: bare ground): the game's
# camera height, then 8 and 14 wheel steps closer (a low view), and one above the human's base. They are copied to
# <copy>\Saved\GroundShots\<Tag>_<Map>_<name>.png. Prints one line PASS / FAIL; exit code 0 / 1.
param(
    [Parameter(Mandatory = $true)] [string] $Tag,
    [string] $Map = "Steppe",
    [string] $Size = "Small",
    [string] $Root = "",
    [string] $EngineDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "ground_shots : FAIL (engine '$Association' not found in the registry; pass -EngineDir)"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Editor = Join-Path $EngineDir "UnrealEditor.exe"
$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Work = Join-Path $Project "Saved_Tests"
New-Item -ItemType Directory -Force (Join-Path $Work "Logs") | Out-Null
$Suffix = "Tests_Ground"
$Dir = Join-Path $Project "Saved_$Suffix"
if (Test-Path $Dir) { Remove-Item -Recurse -Force $Dir -ErrorAction SilentlyContinue }
$Log = Join-Path $Work "Logs\ground_shots.log"
$Sequence = Join-Path $Work "ground_shots.seq"
$Config = Join-Path $Work "ground_shots.json"
Remove-Item $Log -ErrorAction SilentlyContinue
"{ `"version`": 3, `"seed`": 11, `"map`": `"/Game/Maps/$Map.$Map`", `"mapSize`": `"$Size`", `"bDrawPlaces`": false, `"bTeamsTogether`": true, `"slots`": [ { `"state`": `"Human`", `"colorIndex`": 0 }, { `"state`": `"AI`", `"colorIndex`": 1 }, { `"state`": `"Closed`", `"colorIndex`": 2 }, { `"state`": `"Closed`", `"colorIndex`": 3 }, { `"state`": `"Closed`", `"colorIndex`": 4 }, { `"state`": `"Closed`", `"colorIndex`": 5 }, { `"state`": `"Closed`", `"colorIndex`": 6 }, { `"state`": `"Closed`", `"colorIndex`": 7 } ] }" | Set-Content -Encoding ascii $Config
@("waitmap MainMenu", "waitui", "hol.Match.Open $Config", "waitmap ${Map}_$Size", "waitlog Camera above the start of slot 0", "frames 40",
    "capture ground_base", "hol.Camera.Focus 0 0", "frames 40", "capture ground_centre", "hol.Camera.Zoom 8", "frames 40", "capture ground_low",
    "hol.Camera.Zoom 6", "frames 40", "capture ground_lower", "quit") | Set-Content -Encoding ascii $Sequence
$Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -nosplash -culture=fr -LiveCoding=0 -as-exit-on-error -asdebugport=0 -saveddirsuffix=$Suffix -abslog=`"$Log`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Test.Run $Sequence`""
$Process = Start-Process -FilePath $Editor -ArgumentList $Arguments -PassThru -WindowStyle Hidden
if (-not $Process.WaitForExit(600000)) { $Process.Kill() }
$Shots = Join-Path $Dir "Screenshots\WindowsEditor\HOL"
$Out = Join-Path $Project "Saved\GroundShots"
New-Item -ItemType Directory -Force $Out | Out-Null
$Made = @()
foreach ($Name in "ground_base", "ground_centre", "ground_low", "ground_lower") {
    $File = Join-Path $Shots "$Name.png"
    if ((Test-Path $File) -and (Get-Item $File).Length -gt 0) { Copy-Item $File (Join-Path $Out "${Tag}_${Map}_$($Name -replace '^ground_','').png") -Force; $Made += $Name }
}
Get-Content $Log -ErrorAction SilentlyContinue | Select-String -Pattern "Test capture|Error:|Camera above" | Select-Object -First 12 | ForEach-Object { $_.Line -replace "^.*?\]", "" }
Write-Host "ground_shots : $(if ($Made.Count -eq 4) { 'PASS' } else { 'FAIL' }) ($Out)"
exit $(if ($Made.Count -eq 4) { 0 } else { 1 })
