# The Forum's variants side by side, IN THE GAME (VisualInProgress.md): for each letter, compose SM_Forum from that variant
# (make_forum_mesh.py --compose --variant=<letter>), run the scenario forum_variant without a window (check.ps1: it takes the
# capture fv_mid) and keep the picture as Saved\forum_variants\<letter>.png; then Tools\Editor\forum_variants_sheet.py makes
# Saved\forum_variants_sheet.png. About 80 s a variant. Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\forum_variants.ps1 [-Letters A,B,C] [-Root <worktree>]
param(
    [string[]] $Letters = @("A", "B", "C", "D", "E", "F", "G", "H", "I", "J"),
    [string] $Root = ""
)

$Letters = @($Letters | ForEach-Object { $_ -split "," } | Where-Object { $_ })   # -File passes K,L,M as one string
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
$Cmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$Script = Join-Path $Root "Tools\Editor\make_forum_mesh.py"
$Out = Join-Path $Root "Saved\forum_variants"
New-Item -ItemType Directory -Force $Out | Out-Null
$Shot = Join-Path $Root "Saved_Tests\Screenshots\WindowsEditor\HOL\fv_mid.png"

foreach ($Letter in $Letters) {
    $Start = Get-Date
    & $Cmd $UProject -run=pythonscript "-script=$Script --compose --variant=$Letter" -unattended -nullrhi -nosplash -nosound -LiveCoding=0 | Out-Null
    $global:LASTEXITCODE = 0
    Remove-Item $Shot -ErrorAction SilentlyContinue
    $Check = powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Root "Tools\Scenarios\check.ps1") forum_variant -Root $Root 2>&1 | Select-Object -Last 1
    if (Test-Path $Shot) {
        Copy-Item $Shot (Join-Path $Out "$Letter.png") -Force
        Write-Host ("variant {0}: ok ({1:N0} s) {2}" -f $Letter, ((Get-Date) - $Start).TotalSeconds, $Check)
    }
    else {
        Write-Host ("variant {0}: NO CAPTURE ({1:N0} s) {2}" -f $Letter, ((Get-Date) - $Start).TotalSeconds, $Check)
    }
}
python (Join-Path $Root "Tools\Editor\forum_variants_sheet.py") $Out (Join-Path $Root "Saved\forum_variants_sheet.png")
