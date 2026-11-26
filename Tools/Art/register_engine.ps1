# register_engine.ps1 (D927, AR7-d) : enregistre le moteur decompresse sous le nom attendu par le projet (EngineAssociation
# de HeroesOfLiberty.uproject, "UE 5.8 HOL"), comme le fait le lanceur Epic pour une version installee. Une fois par poste.
# Usage : powershell -ExecutionPolicy Bypass -File Tools\Art\register_engine.ps1 -EnginePath <dossier qui contient Engine\>
# Codes : 0 enregistre ; 1 dossier sans Engine\Binaries\Win64\UnrealEditor.exe. -RegistryPath : reserve au test.
param(
  [Parameter(Mandatory = $true)] [string] $EnginePath,
  [string] $Name = '',
  [string] $RegistryPath = 'HKCU:\Software\Epic Games\Unreal Engine\Builds'
)
$ErrorActionPreference = 'Continue'
if (-not $Name) {
  $Project = Get-ChildItem (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path -Filter '*.uproject' -File | Select-Object -First 1
  $Name = "$((Get-Content -Raw $Project.FullName | ConvertFrom-Json).EngineAssociation)"
}
$Exe = Join-Path $EnginePath 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path $Exe)) { Write-Output "register_engine : FAIL pas de $Exe (donnez le dossier qui contient Engine\)"; exit 1 }
$Full = (Resolve-Path $EnginePath).Path.TrimEnd('\') -replace '\\', '/'
if (-not (Test-Path $RegistryPath)) { New-Item -Path $RegistryPath -Force | Out-Null }
New-ItemProperty -Path $RegistryPath -Name $Name -Value $Full -PropertyType String -Force | Out-Null
Write-Output "register_engine : PASS '$Name' -> $Full"
$Redist = Join-Path $EnginePath 'Engine\Extras\Redist\en-us\vc_redist.x64.exe'
if (Test-Path $Redist) { Write-Output "register_engine : si l'editeur ne demarre pas, installez une fois $Redist" }
exit 0
