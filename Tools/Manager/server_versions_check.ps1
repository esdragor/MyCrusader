# D481 end-to-end check of "one server per game version" on the real chain, without packaging: two fake archives
# (b3-v1, b3-v2: a dummy game exe + Version.txt, and the manager tests' FakeServer as WindowsServer\HeroesOfLibertyServer.exe
# with a dummy .pdb) published by the real publish.ps1 into a scratch folder, read by the real HOLManager.exe (built by
# build_manager.ps1, copied there) on a free local port, driven through its real HTTP routes (PROTOCOL.md).
# Checks: Servers\<v>\ has the exe and no .pdb; a lobby takes the latest version and its match runs Servers\<v>\'s exe
# (never the manager's ServerExe, a decoy here); after v2 is published while v1's match runs, /status and new lobbies say
# v2, a v1 client is refused (version_mismatch), v2's match runs Servers\v2\; Servers\v1\ stays while v1 is in use, then
# is deleted about RetireDelay (1 min) after its lobby is gone, Servers\v2\ kept; publishing v2 again is refused.
# Takes about 4 min. Stops every process it started (manager, fake servers: by pid, never by name).
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Manager\server_versions_check.ps1 [-Root <copy>] [-WorkDir <folder>]
#       [-Port 7791] [-NoBuild]
# Prints its steps, then one line PASS / FAIL; exit code 0 / 1.
param(
    [string] $Root = "",
    [string] $WorkDir = "",
    [int] $Port = 7791,
    [switch] $NoBuild
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
if (-not $WorkDir) { $WorkDir = Join-Path $env:TEMP "hol_server_versions_check" }
$WorkDir = [IO.Path]::GetFullPath($WorkDir)
$Base = "http://127.0.0.1:$Port"
$Started = New-Object System.Collections.ArrayList
$ManagerProcess = $null

function Say([string] $Line) { Write-Host "  $Line" }
function StopAll {
    foreach ($ProcessId in $Started) { Stop-Process -Id $ProcessId -Force -ErrorAction SilentlyContinue }
    # The fake servers started by the manager, by their pid files (never by name: other sessions run real servers).
    Get-ChildItem (Join-Path $WorkDir "Manager\Saved\Matches") -Recurse -Filter fake.pid -ErrorAction SilentlyContinue | ForEach-Object {
        $Process = Get-Process -Id ([int](Get-Content $_.FullName -Raw).Trim()) -ErrorAction SilentlyContinue
        if ($Process -and $Process.Path -like "$WorkDir\*") { Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue }
    }
}
function Fail([string] $Why) { StopAll; Write-Host "server_versions_check : FAIL ($Why; work folder $WorkDir)"; exit 1 }

function Call([string] $Method, [string] $Path, $Body = $null, [string] $Token = "") {
    $Headers = @{}
    if ($Token) { $Headers["X-Player-Token"] = $Token }
    $Request = @{ Uri = "$Base$Path"; Method = $Method; Headers = $Headers; UseBasicParsing = $true; TimeoutSec = 10 }
    if ($null -ne $Body) { $Request.Body = ($Body | ConvertTo-Json -Compress); $Request.ContentType = "application/json" }
    try {
        $Response = Invoke-WebRequest @Request
        $Text = $Response.Content
        $Code = [int] $Response.StatusCode
    }
    catch [System.Net.WebException] {
        if (-not $_.Exception.Response) { return @{ Code = 0; Json = $null; Text = $_.Exception.Message } }
        $Code = [int] $_.Exception.Response.StatusCode
        $Reader = New-Object IO.StreamReader $_.Exception.Response.GetResponseStream()
        $Text = $Reader.ReadToEnd()
    }
    $Json = $null
    if ($Text) { try { $Json = $Text | ConvertFrom-Json } catch { } }
    return @{ Code = $Code; Json = $Json; Text = $Text }
}

function Publish([string] $Version) {
    $Lines = & powershell -ExecutionPolicy Bypass -File (Join-Path $Project "Tools\Package\publish.ps1") -Root $Project `
        -Archive (Join-Path $WorkDir "Archives\$Version") -VersionsDir $VersionsDir -ServersDir $ServersDir 2>&1
    return @{ Code = $LASTEXITCODE; Last = ($Lines | Select-Object -Last 1) }
}

# A lobby created by $Leader and joined by $Other, both ready, started: its match id and match folder.
function StartMatch([string] $Version, [string] $Leader, [string] $Other) {
    $Created = Call POST "/lobbies" @{ name = "Salon $Leader"; playerName = $Leader; gameVersion = $Version }
    if ($Created.Code -ne 201) { Fail "lobby of $Leader not created: $($Created.Code) $($Created.Text)" }
    $Id = $Created.Json.lobby.id
    $Joined = Call POST "/lobbies/$Id/join" @{ playerName = $Other; gameVersion = $Version }
    if ($Joined.Code -ne 200) { Fail "$Other not in $Id`: $($Joined.Code) $($Joined.Text)" }
    foreach ($Token in $Created.Json.token, $Joined.Json.token) {
        $Ready = Call PUT "/lobbies/$Id/me" @{ ready = $true } $Token
        if ($Ready.Code -ne 200) { Fail "ready in $Id`: $($Ready.Code) $($Ready.Text)" }
    }
    $Start = Call POST "/lobbies/$Id/start" @{} $Created.Json.token
    if ($Start.Code -ne 202) { Fail "start of $Id`: $($Start.Code) $($Start.Text)" }
    $MatchDir = Join-Path $WorkDir "Manager\Saved\Matches\$($Start.Json.matchId)"
    $Deadline = (Get-Date).AddSeconds(20)
    while (-not (Test-Path (Join-Path $MatchDir "commandline.txt"))) {
        if ((Get-Date) -gt $Deadline) { Fail "no commandline.txt from the server of $($Start.Json.matchId)" }
        Start-Sleep -Milliseconds 200
    }
    return @{ Lobby = $Id; Match = $Start.Json.matchId; Dir = $MatchDir; Tokens = @($Created.Json.token, $Joined.Json.token) }
}

function CheckLaunched($Match, [string] $Version) {
    $Expected = Join-Path $ServersDir "$Version\HeroesOfLibertyServer.exe"
    $CommandLine = Get-Content (Join-Path $Match.Dir "commandline.txt") -Raw
    if ($CommandLine -notlike "*$Expected*") { Fail "$($Match.Match) launched by $CommandLine, not $Expected" }
    $LogLine = Get-Content (Join-Path $WorkDir "Manager\Logs\manager-*.log") | Where-Object { $_ -like "*Starting the server of $($Match.Match) (version $Version)*" }
    if (-not $LogLine -or $LogLine -notlike "*$Expected*") { Fail "no log line 'Starting the server of $($Match.Match) (version $Version): $Expected'" }
    Say "$($Match.Match) (lobby $($Match.Lobby)) launched from $Expected"
}

# 0. Fresh work folder, free port, the manager and the fake server built.
if (Test-Path $WorkDir) { Remove-Item -Recurse -Force $WorkDir -ErrorAction SilentlyContinue }
if (Test-Path $WorkDir) { Fail "work folder not removable (a process left from a previous run?)" }
if (Get-NetTCPConnection -LocalPort $Port -State Listen -ErrorAction SilentlyContinue) { Fail "port $Port already used" }
$ManagerExe = Join-Path $Project "Saved\Manager\HOLManager.exe"
if (-not $NoBuild) {
    $Build = & powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "build_manager.ps1") -Root $Project
    if ($LASTEXITCODE -ne 0) { Fail "build_manager.ps1: $($Build | Select-Object -Last 1)" }
}
$FakeProject = Join-Path $Project "Manager\FakeServer\FakeServer.csproj"
$FakeDir = Join-Path $WorkDir "FakeBuild"
& dotnet build $FakeProject --nologo -v q -o $FakeDir 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0 -or -not (Test-Path (Join-Path $FakeDir "FakeServer.exe"))) { Fail "dotnet build of FakeServer" }
if (-not (Test-Path $ManagerExe)) { Fail "no $ManagerExe" }

# 1. The fake archives: the apphost renamed keeps loading FakeServer.dll beside it.
foreach ($Version in "b3-v1", "b3-v2") {
    $Archive = Join-Path $WorkDir "Archives\$Version"
    New-Item -ItemType Directory -Force (Join-Path $Archive "Windows\HeroesOfLiberty"), (Join-Path $Archive "WindowsServer") | Out-Null
    Set-Content -Path (Join-Path $Archive "Windows\HeroesOfLiberty.exe") -Value "fake game $Version" -Encoding ascii
    Set-Content -Path (Join-Path $Archive "Windows\HeroesOfLiberty\Version.txt") -Value $Version -Encoding ascii
    Copy-Item (Join-Path $FakeDir "FakeServer.exe") (Join-Path $Archive "WindowsServer\HeroesOfLibertyServer.exe")
    Copy-Item (Join-Path $FakeDir "FakeServer.dll"), (Join-Path $FakeDir "FakeServer.runtimeconfig.json") (Join-Path $Archive "WindowsServer")
    Set-Content -Path (Join-Path $Archive "WindowsServer\HeroesOfLibertyServer.pdb") -Value "symbols" -Encoding ascii
}
$ManagerDir = Join-Path $WorkDir "Manager"
$VersionsDir = Join-Path $ManagerDir "Versions"
$ServersDir = Join-Path $ManagerDir "Servers"
New-Item -ItemType Directory -Force $ManagerDir, (Join-Path $WorkDir "Decoy") | Out-Null
Copy-Item $ManagerExe $ManagerDir
# The manager's own ServerExe: a decoy in Decoy\, a fallback to it would show in the command line and the log.
Copy-Item (Join-Path $FakeDir "*") (Join-Path $WorkDir "Decoy")
Rename-Item (Join-Path $WorkDir "Decoy\FakeServer.exe") "HeroesOfLibertyServer.exe"

# 2. v1 published, then the manager started.
$P1 = Publish "b3-v1"
if ($P1.Code -ne 0) { Fail "publish b3-v1: $($P1.Last)" }
$V1Dir = Join-Path $ServersDir "b3-v1"
if (-not (Test-Path (Join-Path $V1Dir "HeroesOfLibertyServer.exe"))) { Fail "no $V1Dir\HeroesOfLibertyServer.exe" }
if (Get-ChildItem $V1Dir -Recurse -Filter *.pdb) { Fail ".pdb copied into $V1Dir" }
Say "published b3-v1: $V1Dir\HeroesOfLibertyServer.exe, no .pdb"

$ManagerArgs = @("--urls=$Base", "--ManagerUrl=$Base", "--GameVersion=b3-v0", "--ServerExe=$(Join-Path $WorkDir 'Decoy\HeroesOfLibertyServer.exe')",
    "--ServersDir=$ServersDir", "--VersionsDir=$VersionsDir", "--FirstServerPort=7891", "--AccessKeyFile=no_key.txt")
$ManagerProcess = Start-Process -FilePath (Join-Path $ManagerDir "HOLManager.exe") -ArgumentList $ManagerArgs -WorkingDirectory $ManagerDir `
    -WindowStyle Hidden -PassThru
[void] $Started.Add($ManagerProcess.Id)
$Deadline = (Get-Date).AddSeconds(20)
do {
    Start-Sleep -Milliseconds 300
    $Status = Call GET "/status"
    if ((Get-Date) -gt $Deadline) { Fail "manager not answering on $Base" }
} while ($Status.Code -ne 200)
if ($Status.Json.gameVersion -ne "b3-v1") { Fail "/status gameVersion $($Status.Json.gameVersion), not b3-v1" }
Say "manager pid $($ManagerProcess.Id) on $Base, /status gameVersion b3-v1"

# 3. A v1 match.
$M1 = StartMatch "b3-v1" "Bob" "Alice"
$Listed = (Call GET "/lobbies").Json.lobbies | Where-Object { $_.id -eq $M1.Lobby }
if ($Listed.gameVersion -ne "b3-v1") { Fail "/lobbies gives $($M1.Lobby) version $($Listed.gameVersion), not b3-v1" }
CheckLaunched $M1 "b3-v1"

# 4. v2 published while v1's match runs.
$P2 = Publish "b3-v2"
if ($P2.Code -ne 0) { Fail "publish b3-v2: $($P2.Last)" }
$PublishedAt = Get-Date
$V2Dir = Join-Path $ServersDir "b3-v2"
if (-not (Test-Path (Join-Path $V2Dir "HeroesOfLibertyServer.exe")) -or (Get-ChildItem $V2Dir -Recurse -Filter *.pdb)) { Fail "bad $V2Dir" }
$Status = Call GET "/status"
if ($Status.Json.gameVersion -ne "b3-v2") { Fail "/status gameVersion $($Status.Json.gameVersion) after publishing b3-v2" }
Say "published b3-v2 while $($M1.Match) runs: /status gameVersion b3-v2"
$Old = Call POST "/lobbies" @{ name = "Old"; playerName = "Olga"; gameVersion = "b3-v1" }
if ($Old.Code -lt 400 -or $Old.Json.error -ne "version_mismatch") { Fail "v1 lobby creation after v2: $($Old.Code) $($Old.Text)" }
$M2 = StartMatch "b3-v2" "Carl" "Dana"
$Listed = (Call GET "/lobbies").Json.lobbies | Where-Object { $_.id -eq $M2.Lobby }
if ($Listed.gameVersion -ne "b3-v2") { Fail "/lobbies gives $($M2.Lobby) version $($Listed.gameVersion), not b3-v2" }
# M2 is in "starting": not joinable; a v1 client is refused on version before that (PROTOCOL.md order).
$Late = Call POST "/lobbies/$($M2.Lobby)/join" @{ playerName = "Eve"; gameVersion = "b3-v1" }
if ($Late.Json.error -ne "version_mismatch") { Fail "v1 client into $($M2.Lobby): $($Late.Code) $($Late.Text)" }
Say "v1 client refused: create $($Old.Code) $($Old.Json.error), join $($M2.Lobby) $($Late.Code) $($Late.Json.error)"
CheckLaunched $M2 "b3-v2"

# 5. Publishing v2 again refused, its server untouched.
$P3 = Publish "b3-v2"
if ($P3.Code -eq 0) { Fail "b3-v2 published twice" }
if (-not (Test-Path (Join-Path $V2Dir "HeroesOfLibertyServer.exe"))) { Fail "$V2Dir damaged by the second publication" }
Say "second publication of b3-v2 refused: $($P3.Last)"

# 6. v1 kept while in use (its match, up to 60 s, then its lobby back from the match) past RetireDelay after v2.
while ((Get-Date) -lt $PublishedAt.AddSeconds(75)) {
    if (-not (Test-Path $V1Dir)) { Fail "$V1Dir deleted while v1 was in use" }
    Start-Sleep -Seconds 1
}
$Lobby1 = (Call GET "/lobbies").Json.lobbies | Where-Object { $_.id -eq $M1.Lobby }
Say "75 s after publishing b3-v2: $V1Dir still there (lobby $($M1.Lobby) state $($Lobby1.state))"
# Stop v1's server if still running, then both players leave: v1 no longer used.
$Pid1 = [int](Get-Content (Join-Path $M1.Dir "fake.pid") -Raw).Trim()
Stop-Process -Id $Pid1 -Force -ErrorAction SilentlyContinue
$Deadline = (Get-Date).AddSeconds(15)
while ((Call GET "/lobbies").Json.lobbies | Where-Object { $_.id -eq $M1.Lobby -and $_.state -ne "lobby" }) {
    if ((Get-Date) -gt $Deadline) { Fail "lobby $($M1.Lobby) not back from its match" }
    Start-Sleep -Milliseconds 500
}
foreach ($Token in $M1.Tokens) {
    $Left = Call POST "/lobbies/$($M1.Lobby)/leave" $null $Token
    if ($Left.Code -ne 204) { Fail "leave $($M1.Lobby): $($Left.Code) $($Left.Text)" }
}
$FreedAt = Get-Date
$Deadline = $FreedAt.AddSeconds(100)
while (Test-Path $V1Dir) {
    if ((Get-Date) -gt $Deadline) { Fail "$V1Dir still there 100 s after v1 was freed" }
    Start-Sleep -Seconds 1
}
$After = [math]::Round(((Get-Date) - $FreedAt).TotalSeconds)
if ($After -lt 55) { Fail "$V1Dir deleted after $After s, before RetireDelay" }
if (-not (Test-Path (Join-Path $V2Dir "HeroesOfLibertyServer.exe"))) { Fail "$V2Dir deleted" }
$Deleted = Get-Content (Join-Path $ManagerDir "Logs\manager-*.log") | Where-Object { $_ -like "*Server of version b3-v1 deleted*" }
if (-not $Deleted) { Fail "no log line 'Server of version b3-v1 deleted'" }
Say "$V1Dir deleted $After s after v1 was freed, $V2Dir kept"

StopAll
Write-Host "server_versions_check : PASS (v1 then v2 published, matches on Servers\<v>\, v1 refused, Servers\b3-v1 deleted after $After s; work folder $WorkDir)"
exit 0
