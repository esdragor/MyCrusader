# regen_tables.ps1 (L20-9, 2026-11-20) : regenere les .uasset UnitTypes / BuildingTypes depuis les CSV de Tools\Data.
# Les deux scripts Python tournent dans UnrealEditor-Cmd.exe (sans fenetre : -nullrhi -nosound -unattended), l'un apres l'autre :
#   make_unit_types.py puis make_building_types.py (commande qui a marche le 2026-11-20 a 16:29), puis make_source_types.py
#   (SourceTypes, ajoute par HALDIR-1125ar7 : colonne Mesh retiree, AR0-d2 point 3).
# Usage : powershell -ExecutionPolicy Bypass -File regen_tables.ps1 -Root <worktree> [-DryRun] [-Engine <dossier du moteur>]
#   -DryRun : imprime les deux commandes, ne lance rien. Sans -DryRun : lance l'editeur (Regle 13 d'agent_rules : seul, file de build vide).
# Code de sortie : 0 = les deux scripts ont reussi (ou DryRun), 1 = -Root / uproject / editeur introuvable, 2 = un script a echoue.
param(
  [Parameter(Mandatory = $true)][string]$Root,
  [string]$Engine = 'C:\UE\UE_5.8_HOL',
  [switch]$DryRun
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Root)) { Write-Output "regen_tables : -Root introuvable : $Root"; exit 1 }
$Root = (Resolve-Path -LiteralPath $Root).Path.TrimEnd('\')
$UProject = Get-ChildItem -LiteralPath $Root -Filter '*.uproject' -File | Select-Object -First 1
if (-not $UProject) { Write-Output "regen_tables : aucun .uproject sous $Root"; exit 1 }
$Editor = Join-Path $Engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not $DryRun -and -not (Test-Path -LiteralPath $Editor)) { Write-Output "regen_tables : editeur introuvable : $Editor"; exit 1 }
$LogDir = Join-Path $Root 'Saved\Logs'

$Steps = @(
  @{ Name = 'unit';     Script = (Join-Path $Root 'Tools\Editor\make_unit_types.py') },
  @{ Name = 'building'; Script = (Join-Path $Root 'Tools\Editor\make_building_types.py') },
  @{ Name = 'source';   Script = (Join-Path $Root 'Tools\Editor\make_source_types.py') }
)
$Code = 0
foreach ($s in $Steps) {
  $Log = Join-Path $LogDir ("regen_tables_" + $s.Name + ".log")
  $EditorArgs = @($UProject.FullName, '-run=pythonscript', ('-script=' + $s.Script), '-unattended', '-nosplash', '-nosound', '-Multiprocess', '-LiveCoding=0', ('-abslog=' + $Log))
  $Line = '"' + $Editor + '" ' + ((($EditorArgs + '-nullrhi') | ForEach-Object { if ($_ -match '\s') { '"' + $_ + '"' } else { $_ } }) -join ' ')
  if ($DryRun) { Write-Output $Line; continue }
  if (-not (Test-Path -LiteralPath $s.Script)) { Write-Output "regen_tables : script absent : $($s.Script)"; $Code = 2; break }
  New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
  Write-Output "regen_tables : $($s.Name) : $Line"
  & $Editor @EditorArgs -nullrhi
  if ($LASTEXITCODE -ne 0) { Write-Output "regen_tables : $($s.Name) a echoue (code $LASTEXITCODE), journal : $Log"; $Code = 2; break }
}
exit $Code
