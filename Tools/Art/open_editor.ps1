# open_editor.ps1 (D927, AR7-d) : ouvre l'editeur sur un poste d'artiste sans Visual Studio. Lance par OpenEditor.cmd (double-clic).
# 1. trouve le moteur nomme par EngineAssociation du .uproject (registre HKCU, ecrit par Tools\Art\register_engine.ps1) ;
# 2. refuse des DLL faites pour un autre moteur (BuildId de UnrealEditor.modules du kit et du moteur) ;
# 3. recopie le kit ArtistBinaries\Win64 dans Binaries\Win64, sauf sur un poste de developpement (Intermediate\Build present) sans -Force ;
# 4. avertit (WARN) si du C++ a ete commite apres le commit dont sortent les DLL du kit ;
# 5. lance UnrealEditor.exe sur le .uproject (avec -DryRun : imprime la ligne LAUNCH sans lancer).
# Codes : 0 lance (ou DryRun) ; 1 refus (FAIL, message pour l'artiste).
param(
  [string] $Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
  [string] $EngineRoot = '',
  [string] $RegistryPath = 'HKCU:\Software\Epic Games\Unreal Engine\Builds',
  [switch] $Force,
  [switch] $DryRun
)
$ErrorActionPreference = 'Continue'
function Stop-Fail([string] $Text) { Write-Output "open_editor : FAIL $Text"; exit 1 }
function Get-BuildId([string] $Path) {
  if (-not (Test-Path $Path)) { return '' }
  try { return "$((Get-Content -Raw $Path | ConvertFrom-Json).BuildId)" } catch { return '' }
}

$Project = Get-ChildItem $Root -Filter '*.uproject' -File | Select-Object -First 1
if (-not $Project) { Stop-Fail "aucun .uproject dans $Root" }
$Association = "$((Get-Content -Raw $Project.FullName | ConvertFrom-Json).EngineAssociation)"
$Register = 'Tools\Art\register_engine.ps1 -EnginePath <dossier du moteur>'
if (-not $EngineRoot) {
  $EngineRoot = "$((Get-ItemProperty -Path $RegistryPath -Name $Association -ErrorAction SilentlyContinue).$Association)"
  if (-not $EngineRoot) { Stop-Fail "moteur '$Association' introuvable sur ce poste : decompressez le zip du moteur puis lancez $Register" }
}
$Exe = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path $Exe)) { Stop-Fail "pas de UnrealEditor.exe sous $EngineRoot : decompressez le zip du moteur puis lancez $Register" }

$Kit = Join-Path $Root 'ArtistBinaries\Win64'
$Bin = Join-Path $Root 'Binaries\Win64'
$KitId = Get-BuildId (Join-Path $Kit 'UnrealEditor.modules')
$EngineId = Get-BuildId (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.modules')
if (-not $KitId) { Stop-Fail "pas de kit $Kit\UnrealEditor.modules : faites git pull, ou demandez la publication des DLL" }
if ($KitId -ne $EngineId) { Stop-Fail "les DLL du kit sont faites pour un autre moteur (BuildId $KitId, moteur $EngineId) : demandez le zip du moteur a jour" }

$DevMachine = Test-Path (Join-Path $Root 'Intermediate\Build')
if ($DevMachine -and -not $Force) {
  Write-Output 'open_editor : poste de developpement (Intermediate\Build present) : DLL locales gardees (-Force pour prendre le kit)'
} else {
  New-Item -ItemType Directory -Force $Bin | Out-Null
  $Copied = 0
  foreach ($File in Get-ChildItem $Kit -File) {
    $Target = Join-Path $Bin ($(if ($File.Name -eq 'source_commit.txt') { 'artist_source_commit.txt' } else { $File.Name }))
    if ((Test-Path $Target) -and ((Get-FileHash $Target).Hash -eq (Get-FileHash $File.FullName).Hash)) { continue }
    Copy-Item -Force $File.FullName $Target
    $Copied++
  }
  Write-Output "open_editor : kit -> Binaries\Win64 : $Copied fichier(s) copie(s)"
}

$KitSha = @(Get-Content (Join-Path $Kit 'source_commit.txt') -ErrorAction SilentlyContinue)[0]
$CppSha = "$(& git.exe -C $Root log -1 --format=%H -- Source '*.uproject' 2>$null)".Trim()
if ($KitSha -and $CppSha) {
  & git.exe -C $Root merge-base --is-ancestor $CppSha $KitSha 2>$null
  if ($LASTEXITCODE -ne 0) {
    Write-Output "open_editor : WARN le code C++ a change apres ces DLL (DLL de $($KitSha.Substring(0, 7)), C++ de $($CppSha.Substring(0, 7))) : l'editeur peut refuser certains fichiers ; demandez la publication des DLL"
  }
}

Write-Output "open_editor : LAUNCH $Exe `"$($Project.FullName)`""
if (-not $DryRun) { Start-Process -FilePath $Exe -ArgumentList "`"$($Project.FullName)`"" }
exit 0
