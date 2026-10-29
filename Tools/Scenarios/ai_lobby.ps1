# IA au salon (IA0.6 L1-L5) : ton essai d'un salon multijoueur contre une IA, sur ce PC.
# Démarre le gestionnaire de salons de cette copie (Saved\Manager\HOLManager.exe, Tools\Manager\build_manager.ps1) avec
# l'éditeur de cette copie comme serveur de partie (comme lobby_match.ps1 -AI : rien à cuire), puis ouvre le jeu en
# fenêtre sous le nom « Chef », branché sur ce gestionnaire. Quand tu fermes le jeu, le gestionnaire et les serveurs de
# cette copie sont arrêtés (par PID et par chemin : jamais ceux d'une autre copie).
# À faire :
#  1. Multijoueur > Créer un salon ; Taille : Petite (2 places) ou plus ;
#  2. sur la ligne 2, liste d'état : « IA » : la ligne devient « IA Normal 2 », Niveau grisé sur Normal ; change sa
#     couleur et son équipe ; repasse-la en Fermé puis remets-la (le Prêt se remet à zéro à chaque changement) ;
#  3. Prêt, puis Lancer (chef seul + une IA suffit) : la partie s'ouvre, le tableau montre « IA Normal 2 », ses 5
#     Paysans partent récolter (console ² : hol.Camera.FocusSlot 1 pour les voir).
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\ai_lobby.ps1 [-Root <copie>]
param([string] $Root = "")

if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
if (-not $EngineRoot) { Write-Host "ai_lobby: engine '$Association' of the .uproject not found in the registry"; exit 1 }
$Editor = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
$Exe = Join-Path $Root "Saved\Manager\HOLManager.exe"
if (-not (Test-Path $Exe)) { Write-Host "ai_lobby: no $Exe (run Tools\Manager\build_manager.ps1)"; exit 1 }

$Work = Join-Path $Root "Saved_Trial\AILobby"
New-Item -ItemType Directory -Force $Work | Out-Null
# The match server: this copy's editor in server mode, as lobby_match.ps1 -AI (the manager runs it with its arguments).
$ServerExe = Join-Path $Work "editor_server.cmd"
"@`"$Editor`" `"$UProject`" %* -server -nullrhi -nosound -nosplash -LiveCoding=0 -asdebugport=0 -saveddirsuffix=Trial_LobbySrv" |
    Set-Content -Encoding ascii $ServerExe

# This copy's own trial ports (Tools\Tests\copy_ports.ps1), away from the test scripts' ranges (managers TCP 7790-7889,
# match servers UDP 7900-7999, both per copy): neither another copy's test nor one of this copy lands here.
. (Join-Path $Root "Tools\Tests\copy_ports.ps1")
$CopyIndex = Get-CopyIndex $Root
$Port = Get-FreeTcpPort (7590 + 10 * $CopyIndex)
$FirstServerPort = 8000 + 10 * $CopyIndex
$Url = "http://127.0.0.1:${Port}"
$Servers = Join-Path $Work "servers.json"
"{ `"servers`": [ { `"name`": `"Essai IA`", `"url`": `"$Url`" } ] }" | Set-Content -Encoding ascii $Servers
$ManagerLog = Join-Path $Work "manager.log"
$Manager = Start-Process -FilePath $Exe -ArgumentList "--urls $Url --GameVersion=dev --ServerExe=`"$ServerExe`" --ManagerUrl=$Url --FirstServerPort=$FirstServerPort" `
    -WorkingDirectory $Work -PassThru -WindowStyle Hidden -RedirectStandardOutput $ManagerLog
Write-Host "ai_lobby: manager on $Url (pid $($Manager.Id), log $ManagerLog)"

$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Game = Start-Process -FilePath $Editor -PassThru -ArgumentList @("`"$UProject`"", "-game", "-nosplash", "-culture=fr", "-LiveCoding=0",
    "-asdebugport=0", "-PlayerName=Chef", "-ServersFile=`"$Servers`"")
Write-Host "ai_lobby: game open (pid $($Game.Id)); close it to stop the manager and the match servers of this copy"
$Game.WaitForExit()

Stop-Process -Id $Manager.Id -Force -ErrorAction SilentlyContinue
Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue |
    Where-Object { $_.CommandLine -and $_.CommandLine.Contains($UProject) -and $_.CommandLine.Contains(" -server ") -and $_.CommandLine.Contains("Trial_LobbySrv") } |
    ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Write-Host "ai_lobby: done (manager and match servers stopped)"
