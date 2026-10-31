# Measures the flicker ("grésillement") of the order lines under the fog (F7.3, user's trial 2026-10-30): plays the scenario
# fog_orders_flicker off screen (8 captures one frame apart, camera fixed, units almost still) and compares the frames
# (fog_flicker.py). Prints one line: the mean change between consecutive frames, in the pixels of the lines, and PASS / FAIL
# against -Max. Console commands to run before the scenario go in -Exec (comma separated), e.g. "hol.Orders.LineWidth 12.5".
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\fog_flicker.ps1 [-Exec "..."] [-Max 12] [-Root <copy>] [-Tag name]
param(
    [string] $Root = "",
    [string] $Exec = "",
    [double] $Max = 0.5,
    [string] $Tag = "flick",
    [string] $Scenario = "fog_orders_flicker",
    [int] $TimeoutSeconds = 240
)
$Root = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds").$Association
$Editor = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
$Saved = Join-Path $Root "Saved_Tests_Fogb"
$Shots = Join-Path $Saved "Screenshots\WindowsEditor\HOL"
$Log = Join-Path $Saved "flicker.log"
$Attempt = 0
while ($true) {
$Attempt++
# A saved folder kept from a previous run renders the world black off screen (seen 2026-10-31: the first run fine, every later one
# black until the folder was removed): a fresh folder each time.
if (Test-Path $Saved) { Remove-Item $Saved -Recurse -Force -ErrorAction SilentlyContinue }
New-Item -ItemType Directory -Force $Saved | Out-Null
Remove-Item $Log -ErrorAction SilentlyContinue
Get-ChildItem $Shots -Filter "flick_*.png" -ErrorAction SilentlyContinue | Remove-Item
$Commands = (@($Exec -split "," | Where-Object { $_.Trim() }) + @("hol.Scenario $Scenario")) -join ","
$env:UE_SKIP_UBT_SDK_SETUP = "1"
# Two runs: the first compiles the shaders of the fog material (captures made meanwhile are dark): the sequence waits for the
# render report, then the captures are made; a second launch would only be needed after a material change.
$Arguments = "`"$UProject`" -game -RenderOffScreen -ResX=1600 -ResY=900 -nosound -saveddirsuffix=Tests_Fogb -nosplash -LiveCoding=0 -asdebugport=0 -culture=fr -abslog=`"$Log`" -ExecCmds=`"$Commands`""
$Process = Start-Process -FilePath $Editor -ArgumentList $Arguments -PassThru
$Deadline = (Get-Date).AddSeconds($TimeoutSeconds)
$Done = $false
while ((Get-Date) -lt $Deadline -and -not $Process.HasExited) {
    Start-Sleep -Seconds 2
    if ((Test-Path $Log) -and (Select-String -Path $Log -Pattern "Test mark: flicker done" -Quiet)) { $Done = $true; break }
}
if (-not $Process.HasExited) { Stop-Process -Id $Process.Id -Force }
if (-not $Done) { Write-Host "fog_flicker : FAIL (the scenario did not reach its end; log $Log)"; exit 1 }
python (Join-Path $PSScriptRoot "fog_flicker.py") $Shots $Max $Tag
if ($LASTEXITCODE -eq 3 -and $Attempt -lt 6) { Write-Host "fog_flicker : black frames (attempt $Attempt), again"; continue }
exit $LASTEXITCODE
}
