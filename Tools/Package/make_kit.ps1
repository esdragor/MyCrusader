# Makes the friends' kit (L9): the launcher alone, which installs the game itself on its first run.
# HOLLauncher is published (Release, win-x64, one self-contained exe) into <copy>\Saved\HOLLauncher\, laid in the
# versions folder served by the lobby manager (launcher\<LauncherInfo.Version>\HOLLauncher.exe, then
# launcher\latest.txt: what L7 serves to the launchers already out there), and zipped with hpatchz.exe
# (Tools\ThirdParty\HDiffPatch\), a servers.json in the game's format aimed at -ServerUrl, and LISEZMOI.txt:
#   <copy>\Saved\Kit\HOLLauncher-kit.zip (or -Out <file>).
# The access key (L11, Manager\PROTOCOL.md "Clé d'accès"): <ManagerDir>\access_key.txt, created if missing or empty
# (32 random bytes, base64url without "="), kept otherwise, and written in the kit's servers.json ("key"): the manager
# then serves only this kit's launchers and games. The key itself is never printed nor logged.
# Unpacked in an empty folder, "Installer le jeu" downloads the whole game from that manager (GameUpdater).
# Separate from publish.ps1 on purpose: the kit does not depend on the game (no package, no archive), and is made
# again only when the launcher or the address changes.
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Package\make_kit.ps1 -ServerUrl http://<public address of the box>:7700
#       [-ServerName <name>] [-Root <copy>] [-VersionsDir <folder>] [-ManagerDir <folder>] [-Out <zip>] [-Force]
# - -VersionsDir: <copy>\Saved\Manager\Versions by default, like publish.ps1;
# - -ManagerDir: the manager's folder, where access_key.txt lies (AccessKeyFile); the parent of -VersionsDir by default;
# - LauncherInfo.Version (Manager\HOLLauncher\LauncherInfo.cs) names launcher\<lv>\: raise it by hand before making a
#   kit with a changed launcher, or the launchers already installed will not see it;
# - -Force: replace launcher\<lv>\HOLLauncher.exe even when it holds a different exe (L13a); without it, that is a FAIL
#   (the launchers already installed would keep the old exe under the same version).
# Full log: <copy>\Saved\Logs\make_kit.log. Prints one line PASS / FAIL; exit code 0 / 1.
param(
    [Parameter(Mandatory = $true)] [string] $ServerUrl,
    [string] $ServerName = "Heroes of Liberty",
    [string] $Root = "",
    [string] $VersionsDir = "",
    [string] $ManagerDir = "",
    [string] $Out = "",
    [switch] $Force
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$Log = Join-Path $Project "Saved\Logs\make_kit.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
Set-Content -Path $Log -Value "make_kit : $(Split-Path -Leaf $Project), started $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -Encoding utf8
function Say([string] $Line) { Write-Host $Line; Add-Content -Path $Log -Value $Line -Encoding utf8 }
function Fail([string] $Why) { Say "make_kit : FAIL ($Why; log $Log)"; exit 1 }

$ServerUrl = $ServerUrl.TrimEnd('/')
if ($ServerUrl -notmatch '^https?://[^/\s]+$') { Fail "-ServerUrl wants http://<address>:<port>, not '$ServerUrl'" }
if (-not $VersionsDir) { $VersionsDir = Join-Path $Project "Saved\Manager\Versions" }
$VersionsDir = [IO.Path]::GetFullPath($VersionsDir)
if (-not $ManagerDir) { $ManagerDir = Split-Path $VersionsDir }
$ManagerDir = [IO.Path]::GetFullPath($ManagerDir)
if (-not $Out) { $Out = Join-Path $Project "Saved\Kit\HOLLauncher-kit.zip" }
$Out = [IO.Path]::GetFullPath($Out)
if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) { Fail "dotnet not found" }
$Hpatchz = Join-Path $Project "Tools\ThirdParty\HDiffPatch\hpatchz.exe"
if (-not (Test-Path $Hpatchz)) { Fail "no $Hpatchz" }

# 1. The launcher's version, read where the launcher reads it.
$InfoFile = Join-Path $Project "Manager\HOLLauncher\LauncherInfo.cs"
if (-not (Test-Path $InfoFile)) { Fail "no $InfoFile" }
$Match = Select-String -Path $InfoFile -Pattern 'const string Version = "([^"]+)"' | Select-Object -First 1
if (-not $Match) { Fail "no Version constant in $InfoFile" }
$LauncherVersion = $Match.Matches[0].Groups[1].Value

# 2. dotnet publish: one self-contained exe (HOLLauncher.csproj).
$PublishDir = Join-Path $Project "Saved\HOLLauncher"
$Csproj = Join-Path $Project "Manager\HOLLauncher\HOLLauncher.csproj"
& dotnet publish $Csproj --nologo -c Release -r win-x64 -o $PublishDir 2>&1 | Out-File -Append -Encoding utf8 $Log
$Exe = Join-Path $PublishDir "HOLLauncher.exe"
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $Exe)) {
    Select-String -Path $Log -Pattern ": error " | Select-Object -First 3 | ForEach-Object { Write-Host "  $($_.Line.Trim())" }
    Fail "dotnet publish of HOLLauncher"
}

# 3. The versions folder: launcher\<lv>\HOLLauncher.exe (through a .tmp, never half an exe), latest.txt last.
#    A different exe already there under the same version: FAIL unless -Force (L13a), before anything is written.
$LauncherDir = Join-Path $VersionsDir "launcher\$LauncherVersion"
$Served = Join-Path $LauncherDir "HOLLauncher.exe"
$Replaced = (Test-Path $Served) -and ((Get-FileHash $Served).Hash -ne (Get-FileHash $Exe).Hash)
if ($Replaced -and -not $Force) {
    Fail "launcher\$LauncherVersion\HOLLauncher.exe already holds a different exe: raise LauncherInfo.Version in Manager\HOLLauncher\LauncherInfo.cs, or pass -Force"
}
New-Item -ItemType Directory -Force $LauncherDir | Out-Null
Copy-Item $Exe "$Served.tmp" -Force
Move-Item "$Served.tmp" $Served -Force
[IO.File]::WriteAllText((Join-Path $VersionsDir "launcher\latest.txt"), $LauncherVersion)
if ($Replaced) { Say "  -Force: launcher\$LauncherVersion\HOLLauncher.exe replaced by a different exe (the launchers already installed keep the old one)" }

# 4. The access key next to the manager: kept if there, else created (no BOM, no line break; the manager trims).
$KeyFile = Join-Path $ManagerDir "access_key.txt"
$AccessKey = if (Test-Path $KeyFile) { [IO.File]::ReadAllText($KeyFile).Trim().TrimStart([char]0xFEFF).Trim() } else { "" }
if ($AccessKey) { $KeyState = "reprise" } else {
    $Bytes = New-Object byte[] 32
    $Rng = [Security.Cryptography.RandomNumberGenerator]::Create()
    $Rng.GetBytes($Bytes)
    $Rng.Dispose()
    $AccessKey = [Convert]::ToBase64String($Bytes).TrimEnd('=').Replace('+', '-').Replace('/', '_')
    New-Item -ItemType Directory -Force $ManagerDir | Out-Null
    [IO.File]::WriteAllText($KeyFile, $AccessKey, (New-Object Text.UTF8Encoding $false))
    $KeyState = "créée"
}

# 5. The kit: exe, hpatchz.exe, servers.json (the game's format, GameServers), LISEZMOI.txt (UTF-8 with BOM, CRLF: Notepad).
$Stage = Join-Path $Project "Saved\Kit\stage"
if (Test-Path $Stage) { Remove-Item -Recurse -Force $Stage }
New-Item -ItemType Directory -Force $Stage | Out-Null
Copy-Item $Exe, $Hpatchz $Stage
$Servers = [ordered]@{ servers = @([ordered]@{ name = $ServerName; url = $ServerUrl; key = $AccessKey }) } | ConvertTo-Json -Depth 3
[IO.File]::WriteAllText((Join-Path $Stage "servers.json"), $Servers, (New-Object Text.UTF8Encoding $false))
$Readme = @(
    "Heroes of Liberty : installation"
    ""
    "1. Dézippez ces fichiers dans un dossier vide (par exemple Documents\HeroesOfLiberty)."
    "2. Lancez HOLLauncher.exe puis cliquez « Installer le jeu » (environ 350 Mo à télécharger)."
    "3. Ensuite, à chaque fois : lancez HOLLauncher.exe et cliquez « Jouer » (« Mettre à jour le jeu » d'abord s'il le propose)."
    ""
    "Si Windows affiche « Windows a protégé votre ordinateur » : « Informations complémentaires » puis « Exécuter quand même » (le lanceur n'est pas signé)."
) -join "`r`n"
[IO.File]::WriteAllText((Join-Path $Stage "LISEZMOI.txt"), $Readme + "`r`n", (New-Object Text.UTF8Encoding $true))
New-Item -ItemType Directory -Force (Split-Path $Out) | Out-Null
if (Test-Path $Out) { Remove-Item -Force $Out }
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($Stage, $Out, [IO.Compression.CompressionLevel]::Optimal, $false)
Remove-Item -Recurse -Force $Stage

$Size = [math]::Round((Get-Item $Out).Length / 1MB, 1)
Say "make_kit : PASS ($Out, $Size MB ; launcher $LauncherVersion -> $LauncherDir ; servers.json -> $ServerUrl ; clé : $KeyState, $KeyFile)"
exit 0
