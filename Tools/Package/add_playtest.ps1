# Turns a package of package.ps1 into a network playtest on one PC (M6), as the hand-made package of 2026-10-25:
# - <package>\Manager\ : the lobby manager published by Tools\Manager\build_manager.ps1 (HOLManager.exe and
#   HOLManager.staticwebassets.endpoints.json), Gestionnaire.bat (starts it, window kept open) and appsettings.json
#   (listens on http://0.0.0.0:7700, starts ..\WindowsServer\HeroesOfLibertyServer.exe, at most 2 servers from UDP port
#   7777, GameVersion empty: read from WindowsServer\HeroesOfLiberty\Version.txt);
# - <package>\Windows\servers.json : the manager http://127.0.0.1:7700;
# - <package>\Windows\Joueur_<Name>.bat for each player: the game in a 1280x720 window, its own Saved folder
#   (-saveddirsuffix=_<Name>) and -PlayerName=<Name>.
# The files come from Tools\Package\Playtest\ ({Name} replaced in Joueur.bat); the .bat are written in CRLF, the .json
# in LF without a last line break, whatever git did to them. Run by package.ps1 -Playtest, or alone on a package:
#   powershell -ExecutionPolicy Bypass -File Tools\Package\add_playtest.ps1 -PackageDir <package> [-Root <copy>]
#       [-Players Alice,Bob,Carol,Dan,Eve] [-NoBuild]
# -NoBuild: <copy>\Saved\Manager\HOLManager.exe as it is, not published again. Prints one line PASS / FAIL; exit 0 / 1.
param(
    [Parameter(Mandatory = $true)] [string] $PackageDir,
    [string] $Root = "",
    [string[]] $Players = @("Alice", "Bob", "Carol", "Dan", "Eve"),
    [switch] $NoBuild
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
if (-not (Test-Path $PackageDir)) { Write-Host "add_playtest : FAIL (no $PackageDir)"; exit 1 }
$PackageDir = (Resolve-Path $PackageDir).Path
$Templates = Join-Path $PSScriptRoot "Playtest"
$Client = Join-Path $PackageDir "Windows"
$Server = Join-Path $PackageDir "WindowsServer\HeroesOfLibertyServer.exe"
$ManagerDir = Join-Path $PackageDir "Manager"
if (-not (Test-Path (Join-Path $Client "HeroesOfLiberty.exe"))) { Write-Host "add_playtest : FAIL (no Windows\HeroesOfLiberty.exe in $PackageDir)"; exit 1 }
if (-not (Test-Path $Server)) { Write-Host "add_playtest : FAIL (no WindowsServer\HeroesOfLibertyServer.exe in $PackageDir)"; exit 1 }

# A manager running from this package locks its .exe: name it.
$Running = @(Get-Process HOLManager -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq (Join-Path $ManagerDir "HOLManager.exe") })
if ($Running.Count) { Write-Host "add_playtest : FAIL (HOLManager.exe runs from $ManagerDir, pid $($Running.Id -join ', '))"; exit 1 }

# 1. The manager, published (or as it is with -NoBuild).
$Published = Join-Path $Project "Saved\Manager"
if (-not $NoBuild) {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $Project "Tools\Manager\build_manager.ps1") -Root $Project | ForEach-Object { Write-Host "  $_" }
    if ($LASTEXITCODE -ne 0) { Write-Host "add_playtest : FAIL (build_manager)"; exit 1 }
}
New-Item -ItemType Directory -Force $ManagerDir | Out-Null
foreach ($File in "HOLManager.exe", "HOLManager.staticwebassets.endpoints.json") {
    $From = Join-Path $Published $File
    if (-not (Test-Path $From)) { Write-Host "add_playtest : FAIL (no $From; run Tools\Manager\build_manager.ps1)"; exit 1 }
    Copy-Item $From $ManagerDir -Force
}

# 2. The text files: line breaks set here, UTF-8 without BOM.
function Write-Text([string] $Template, [string] $To, [hashtable] $Replace = @{}) {
    $Text = (Get-Content (Join-Path $Templates $Template) -Raw -Encoding utf8) -replace "`r`n", "`n"
    foreach ($Key in $Replace.Keys) { $Text = $Text.Replace($Key, $Replace[$Key]) }
    $Text = if ($To -like "*.bat") { $Text.TrimEnd("`n") + "`n" -replace "`n", "`r`n" } else { $Text.TrimEnd("`n") }
    [IO.File]::WriteAllText($To, $Text, (New-Object Text.UTF8Encoding $false))
}
Write-Text "Gestionnaire.bat" (Join-Path $ManagerDir "Gestionnaire.bat")
Write-Text "appsettings.json" (Join-Path $ManagerDir "appsettings.json")
Write-Text "servers.json" (Join-Path $Client "servers.json")
foreach ($Name in $Players) { Write-Text "Joueur.bat" (Join-Path $Client "Joueur_$Name.bat") @{ "{Name}" = $Name } }

Write-Host "add_playtest : PASS ($PackageDir : Manager\Gestionnaire.bat, Windows\servers.json, Joueur_$($Players -join '/').bat)"
exit 0
