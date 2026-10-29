# Check of the network playtest setup (Tools\Package\add_playtest.ps1, run by package.ps1 -Playtest) without packaging:
# a fake package (Windows\ and WindowsServer\ with only the files the setup reads, a Version.txt "fake-<time>") is made
# in -WorkDir, the setup runs on it, then:
# - the tree and the exact bytes of Manager\Gestionnaire.bat, Manager\appsettings.json, Windows\servers.json and
#   Windows\Joueur_<Name>.bat (Alice, Bob, Carol, Dan, Eve) are compared with the expected ones;
# - the manager published there starts on a free port >= 7790 (--urls, never 7700) and its /status must give the fake
#   version as gameVersion; it is then stopped by its pid.
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\playtest_setup.ps1 [-Root <copy>] [-WorkDir <dir>] [-NoBuild]
# -NoBuild: the setup reuses <copy>\Saved\Manager\HOLManager.exe instead of publishing it again.
# Prints one line PASS / FAIL with the faults; exit code 0 / 1.
param(
    [string] $Root = "",
    [string] $WorkDir = "",
    [switch] $NoBuild
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
if (-not $WorkDir) { $WorkDir = Join-Path $env:TEMP "HOL_playtest_setup" }
$Package = Join-Path $WorkDir "fake-package"
$Faults = New-Object Collections.Generic.List[string]
function Fault([string] $Text) { $Faults.Add($Text) }

# 1. The fake package.
if (Test-Path $Package) { Remove-Item -Recurse -Force $Package }
$Version = "fake-$(Get-Date -Format 'HHmmss')"
foreach ($Dir in "Windows\HeroesOfLiberty", "WindowsServer\HeroesOfLiberty") { New-Item -ItemType Directory -Force (Join-Path $Package $Dir) | Out-Null }
Set-Content (Join-Path $Package "Windows\HeroesOfLiberty.exe") "fake" -Encoding ascii
Set-Content (Join-Path $Package "WindowsServer\HeroesOfLibertyServer.exe") "fake" -Encoding ascii
foreach ($Dir in "Windows", "WindowsServer") {
    [IO.File]::WriteAllText((Join-Path $Package "$Dir\HeroesOfLiberty\Version.txt"), $Version, (New-Object Text.UTF8Encoding $false))
}

# 2. The setup.
$Setup = Join-Path $Project "Tools\Package\add_playtest.ps1"
if (-not (Test-Path $Setup)) { Write-Host "playtest_setup : FAIL (no $Setup)"; exit 1 }
$SetupArgs = @("-ExecutionPolicy", "Bypass", "-File", $Setup, "-Root", $Project, "-PackageDir", $Package)
if ($NoBuild) { $SetupArgs += "-NoBuild" }
$Output = & powershell @SetupArgs
$Output | ForEach-Object { Write-Host "  $_" }
if ($LASTEXITCODE -ne 0) { Write-Host "playtest_setup : FAIL (add_playtest exit $LASTEXITCODE)"; exit 1 }

# 3. The files, byte for byte (the .bat in CRLF; the .json as the hand-made package of 2026-10-25).
$Crlf = "`r`n"
$Expected = [ordered]@{
    "Manager\Gestionnaire.bat" = "@echo off${Crlf}cd /d `"%~dp0`"${Crlf}HOLManager.exe${Crlf}pause${Crlf}"
    "Manager\appsettings.json" = (@(
        '{'
        '  "Logging": { "LogLevel": { "Default": "Information", "Microsoft.AspNetCore": "Warning" } },'
        '  "AllowedHosts": "*",'
        '  "urls": "http://0.0.0.0:7700",'
        '  "ManagerUrl": "http://127.0.0.1:7700",'
        '  "ServerExe": "..\\WindowsServer\\HeroesOfLibertyServer.exe",'
        '  "GameVersion": "",'
        '  "MaxServers": 2,'
        '  "FirstServerPort": 7777,'
        '  "MatchesDir": "Saved\\Matches",'
        '  "LogDir": "Logs"'
        '}') -join "`n")
    "Windows\servers.json" = '{ "servers": [ { "name": "PC local", "url": "http://127.0.0.1:7700" } ] }'
}
foreach ($Name in "Alice", "Bob", "Carol", "Dan", "Eve") {
    $Expected["Windows\Joueur_$Name.bat"] = "@echo off${Crlf}start `"`" `"%~dp0HeroesOfLiberty.exe`" -windowed -ResX=1280 -ResY=720 -saveddirsuffix=_$Name -PlayerName=$Name${Crlf}"
}
foreach ($File in $Expected.Keys) {
    $Path = Join-Path $Package $File
    if (-not (Test-Path $Path)) { Fault "$File missing"; continue }
    $Bytes = [IO.File]::ReadAllBytes($Path)
    $Want = [Text.Encoding]::UTF8.GetBytes($Expected[$File])
    if ([Convert]::ToBase64String($Bytes) -ne [Convert]::ToBase64String($Want)) { Fault "$File differs" }
}
foreach ($File in "Manager\HOLManager.exe", "Manager\HOLManager.staticwebassets.endpoints.json") {
    if (-not (Test-Path (Join-Path $Package $File))) { Fault "$File missing" }
}
# Nothing else in Manager\ (no Logs\ or Saved\ of a manager run elsewhere).
$Extra = @(Get-ChildItem (Join-Path $Package "Manager") -ErrorAction SilentlyContinue | Where-Object {
    $_.Name -notin "Gestionnaire.bat", "appsettings.json", "HOLManager.exe", "HOLManager.staticwebassets.endpoints.json" })
if ($Extra.Count) { Fault "Manager\ has more: $(($Extra | ForEach-Object Name) -join ', ')" }

# 4. The manager starts and reads the version of the package.
$Exe = Join-Path $Package "Manager\HOLManager.exe"
if (Test-Path $Exe) {
    # A free TCP port of this copy's own range (copy_ports.ps1: another copy's test manager in its own range).
    . (Join-Path $PSScriptRoot "copy_ports.ps1")
    $Port = Get-FreeTcpPort (7790 + 10 * (Get-CopyIndex (Resolve-Path "$PSScriptRoot\..\..").Path))
    $Manager = Start-Process -FilePath $Exe -ArgumentList "--urls=http://127.0.0.1:$Port" -WorkingDirectory (Split-Path $Exe) `
        -WindowStyle Hidden -PassThru
    try {
        $Status = $null
        for ($i = 0; $i -lt 40 -and -not $Status; $i++) {
            Start-Sleep -Milliseconds 500
            try { $Status = Invoke-RestMethod "http://127.0.0.1:$Port/status" -TimeoutSec 2 } catch {}
        }
        if (-not $Status) { Fault "manager: no /status on port $Port" }
        elseif ($Status.gameVersion -ne $Version) { Fault "manager: gameVersion '$($Status.gameVersion)', expected '$Version'" }
    }
    finally { Stop-Process -Id $Manager.Id -Force -ErrorAction SilentlyContinue }
}

if ($Faults.Count) { Write-Host "playtest_setup : FAIL ($($Faults -join '; '))"; exit 1 }
Write-Host "playtest_setup : PASS ($Package, version $Version, manager /status on port $Port)"
exit 0
