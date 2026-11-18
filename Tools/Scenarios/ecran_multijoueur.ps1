# Ecran Multijoueur avec un gestionnaire de salons (D840, lot ECR-3) : la liste, Creer, Rejoindre, Observer, Retour.
# Demarre le gestionnaire de cette copie (Saved\Manager\HOLManager.exe) puis le jeu en fenetre sous le nom Chef, branche dessus (modele ai_lobby.ps1).
# A faire (3 min) : Multijoueur ; la liste montre 0 salon ; Creer un salon ouvre Nouvelle partie en mode salon ; Retour revient a la liste ;
# la liste ressemble a celle d'avant (nom, carte, joueurs, etat, Rejoindre / Observer).
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\ecran_multijoueur.ps1 [-Root <copie>]
param([string] $Root = "")

if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
if (-not $EngineRoot) { Write-Host "ecran_multijoueur: engine '$Association' of the .uproject not found in the registry"; exit 1 }
$Editor = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor.exe"
$Exe = Join-Path $Root "Saved\Manager\HOLManager.exe"
if (-not (Test-Path $Exe)) { Write-Host "ecran_multijoueur: no $Exe (run Tools\Manager\build_manager.ps1)"; exit 1 }

$Work = Join-Path $Root "Saved_Trial\EcranMultijoueur"
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
"{ `"servers`": [ { `"name`": `"Essai ecran multijoueur`", `"url`": `"$Url`" } ] }" | Set-Content -Encoding ascii $Servers
$ManagerLog = Join-Path $Work "manager.log"
$Manager = Start-Process -FilePath $Exe -ArgumentList "--urls $Url --GameVersion=dev --VersionsDir=`"$(Join-Path $Work "Versions")`" --AccessKeyFile=no_key.txt --ServerExe=`"$ServerExe`" --ManagerUrl=$Url --FirstServerPort=$FirstServerPort" `
    -WorkingDirectory $Work -PassThru -WindowStyle Hidden -RedirectStandardOutput $ManagerLog
Write-Host "ecran_multijoueur: manager on $Url (pid $($Manager.Id), log $ManagerLog)"

$env:UE_SKIP_UBT_SDK_SETUP = "1"
$Game = Start-Process -FilePath $Editor -PassThru -ArgumentList @("`"$UProject`"", "-game", "-nosplash", "-culture=fr", "-LiveCoding=0",
    "-asdebugport=0", "-PlayerName=Chef", "-ServersFile=`"$Servers`"")
Write-Host "ecran_multijoueur: game open (pid $($Game.Id)); close it to stop the manager and the match servers of this copy"
$Game.WaitForExit()

Stop-Process -Id $Manager.Id -Force -ErrorAction SilentlyContinue
Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue |
    Where-Object { $_.CommandLine -and $_.CommandLine.Contains($UProject) -and $_.CommandLine.Contains(" -server ") -and $_.CommandLine.Contains("Trial_LobbySrv") } |
    ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Write-Host "ecran_multijoueur: done (manager and match servers stopped)"
