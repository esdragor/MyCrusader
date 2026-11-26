# publish_editor_binaries.ps1 (D927, AR7-d) : copie les DLL d'editeur du projet d'un arbre compile vers ArtistBinaries\Win64,
# pour qu'un artiste sans Visual Studio ouvre l'editeur (Tools\Art\open_editor.ps1 les recopie dans Binaries\Win64).
# A lancer par la session principale apres chaque integration dans main qui touche le C++, depuis le worktree d'integration
# compile, puis commiter ArtistBinaries (fichiers en LFS, .gitattributes).
# Usage : powershell -ExecutionPolicy Bypass -File Tools\Art\publish_editor_binaries.ps1 -From <arbre compile> [-Root <depot qui recoit>] [-DryRun]
# Copie : UnrealEditor.modules et les DLL qu'il nomme (pas de .pdb) ; ecrit source_commit.txt (commit de -From, date).
# Refuse (FAIL, code 1) : C++ non commite dans -From (Source, *.uproject) ; DLL plus vieilles que le dernier commit C++ ;
# fichier nomme absent. Code 0 = copie faite (ou plan imprime avec -DryRun).
param(
  [string] $From = '',
  [string] $Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path,
  [switch] $DryRun
)
$ErrorActionPreference = 'Continue'
if (-not $From) { $From = $Root }
function Stop-Fail([string] $Text) { Write-Output "publish : FAIL $Text"; exit 1 }
$Bin = Join-Path $From 'Binaries\Win64'
$Modules = Join-Path $Bin 'UnrealEditor.modules'
if (-not (Test-Path $Modules)) { Stop-Fail "pas de $Modules (arbre non compile ?)" }
try { $Json = Get-Content -Raw $Modules | ConvertFrom-Json } catch { Stop-Fail "UnrealEditor.modules illisible : $_" }
$Files = @('UnrealEditor.modules') + @($Json.Modules.PSObject.Properties | ForEach-Object { "$($_.Value)" })
foreach ($Name in $Files) { if (-not (Test-Path (Join-Path $Bin $Name))) { Stop-Fail "$Name absent de $Bin" } }

$Dirty = @(& git.exe -C $From status --porcelain -- Source '*.uproject' 2>&1 | ForEach-Object { "$_" } | Where-Object { $_ })
if ($Dirty.Count) { Stop-Fail "C++ non commite dans $From (premier : $($Dirty[0])) : les DLL ne correspondraient a aucun commit" }
$Sha = (& git.exe -C $From rev-parse HEAD 2>&1 | Select-Object -First 1).ToString().Trim()
# Par module : UBT ne relie que les modules changes, une DLL est comparee au dernier commit de Source\<Module> (ou de Source entier
# si le dossier du module n'existe pas).
foreach ($Module in $Json.Modules.PSObject.Properties) {
  $ModuleDir = "Source/$($Module.Name)"
  $Scope = if (Test-Path (Join-Path $From $ModuleDir)) { $ModuleDir } else { 'Source' }
  $CppTime = (& git.exe -C $From log -1 --format=%ct -- $Scope '*.uproject' 2>&1 | Select-Object -First 1)
  if ($CppTime -notmatch '^\d+$') { continue }
  $CppDate = [DateTimeOffset]::FromUnixTimeSeconds([long] $CppTime).LocalDateTime
  $Dll = Get-Item (Join-Path $Bin "$($Module.Value)")
  if ($Dll.LastWriteTime -lt $CppDate) { Stop-Fail "$($Module.Value) ($($Dll.LastWriteTime)) plus vieille que le dernier commit de $Scope ($CppDate) : recompiler $From" }
}

$Kit = Join-Path $Root 'ArtistBinaries\Win64'
Write-Output "publish : source $Sha ($From) -> $Kit : $($Files -join ', ')"
if ($DryRun) { Write-Output 'publish : DryRun, rien copie'; exit 0 }
New-Item -ItemType Directory -Force $Kit | Out-Null
Get-ChildItem $Kit -File | Where-Object { ($Files -notcontains $_.Name) -and ($_.Name -ne 'source_commit.txt') } | Remove-Item -Force
foreach ($Name in $Files) { Copy-Item -Force (Join-Path $Bin $Name) (Join-Path $Kit $Name) }
Set-Content -Encoding ascii (Join-Path $Kit 'source_commit.txt') @($Sha, (Get-Date -Format 'yyyy-MM-dd HH:mm'))
Write-Output "publish : PASS $($Files.Count) fichier(s) ; commiter : git -C $Root add ArtistBinaries ; git -C $Root commit -m `"Art: editor binaries for artists ($($Sha.Substring(0, 7)))`""
exit 0
