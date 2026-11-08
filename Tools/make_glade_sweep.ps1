# Makes /Game/Maps/Crown (Content\Maps\Crown.umap), the test map of the glade sweep (Tools\Editor\maps\glade_sweep.py): the same forest eight
# times side by side with other glades each time. Not a match map. Same engine launch as make_maps.ps1 (editor alone, -Multiprocess, never while this copy builds).
#   powershell -ExecutionPolicy Bypass -File <copy>\Tools\make_glade_sweep.ps1 [-Root <copy>]
# Log: <copy>\Saved\Logs\make_glade_sweep.log. Prints one line PASS / FAIL; exit code 0 / 1.
param([string] $Root = "")

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not (Test-Path $UProject)) { Write-Host "make_glade_sweep : FAIL (no HeroesOfLiberty.uproject in $Project)"; exit 1 }
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
if (-not $EngineRoot) { Write-Host "make_glade_sweep : FAIL (engine '$Association' not found in the registry)"; exit 1 }
$Editor = Join-Path ($EngineRoot.Replace('/', '\').TrimEnd('\')) "Engine\Binaries\Win64\UnrealEditor.exe"

$File = Join-Path $Project "Content\Maps\Crown.umap"
if (Test-Path $File) { Remove-Item $File -Force }
$Log = Join-Path $Project "Saved\Logs\make_glade_sweep.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
Remove-Item $Log -ErrorAction SilentlyContinue
$Script = Join-Path $Project "Tools\Editor\make_glade_sweep.py"
if ($Script -match ' ') { Write-Host "make_glade_sweep : FAIL (a space in $Script)"; exit 1 }
$Clock = [Diagnostics.Stopwatch]::StartNew()
$EditorArguments = "`"$UProject`" -nosplash -unattended -Multiprocess -LiveCoding=0 -nosound -abslog=`"$Log`" -ExecCmds=`"py $Script`""
$Process = Start-Process -FilePath $Editor -ArgumentList $EditorArguments -PassThru -WindowStyle Hidden
$TimedOut = -not $Process.WaitForExit(1800000)
if ($TimedOut) { Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue }
$Seconds = [math]::Round($Clock.Elapsed.TotalSeconds)

$Text = if (Test-Path $Log) { Get-Content $Log } else { @() }
$Created = @($Text | Select-String -Pattern "Match map created at /Game/Maps/Crown:")
$Errors = @($Text | Select-String -Pattern "Match map: |LogPython: Error|No navmesh was built|Could not save|Already there" | ForEach-Object { $_.Line.Trim() })
if (-not $TimedOut -and -not $Errors.Count -and $Created.Count -and (Test-Path $File)) {
    Write-Host "make_glade_sweep : PASS ($Seconds s)"
    $Created | ForEach-Object { Write-Host "  $($_.Line -replace '^.*LogPython: ', '')" }
    exit 0
}
Write-Host "make_glade_sweep : FAIL ($Seconds s$(if ($TimedOut) { ', timed out' }) ; log $Log)"
$Errors | Select-Object -First 10 | ForEach-Object { Write-Host "  $_" }
exit 1
