# Packages the game for the players (M6 R1): the game client and the dedicated server, built, cooked, staged in .pak
# files and archived side by side in <copy>\Saved\Packages\<version>\ (Windows\ the game, WindowsServer\ the server).
# The maps cooked are those of DefaultGame.ini ([/Script/UnrealEd.ProjectPackagingSettings] +MapsToCook).
# The engine is shared by every copy (as Tools\build.ps1):
# - Unreal running in THIS copy locks its DLLs (the cooker loads them): the script stops and names it (-Force to try anyway);
# - one UnrealBuildTool at a time on this engine: a build of another copy running, the script says whose and waits for
#   it to end before starting (the build step of UAT would otherwise refuse or collide);
# - Live Coding open in other copies only: UnrealBuildTool's guard skipped (false alarm, see build.ps1).
# Run from Windows PowerShell (never Git Bash: it rewrites /Game/... paths):
#   powershell -ExecutionPolicy Bypass -File Tools\Package\package.ps1 [-Root <copy>] [-Config Development] [-Version <name>] [-Force] [-Playtest]
# -Playtest: the package also gets the network playtest on one PC (Tools\Package\add_playtest.ps1, after Version.txt):
# Manager\ (the published lobby manager, Gestionnaire.bat, appsettings.json on port 7700 starting ..\WindowsServer\),
# Windows\servers.json (http://127.0.0.1:7700) and Windows\Joueur_Alice/Bob/Carol/Dan/Eve.bat (one window and one
# Saved folder per player). The files come from Tools\Package\Playtest\; checked by Tools\Tests\playtest_setup.ps1.
# Full log: <copy>\Saved_Tests\Logs\package.log (UAT's output as it comes, then the result line). Prints one line
# PASS / FAIL with the first errors; exit code 0 / 1 (2: this copy is busy). 20 to 40 min the first time.
param(
    [string] $Root = "",
    [string] $Config = "Development",
    # Version of the package (R1.7), <yyyy-mm-dd>-<short commit> by default: the archive's folder name, and the
    # content of <archive>\<platform>\HeroesOfLiberty\Version.txt, which the game and the server read (HOLVersion.h).
    [string] $Version = "",
    [switch] $Force,
    [switch] $Playtest
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not (Test-Path $UProject)) { Write-Host "package : FAIL (no HeroesOfLiberty.uproject in $Project)"; exit 1 }
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
if (-not $EngineRoot) { Write-Host "package : FAIL (engine '$Association' of the .uproject not found in the registry)"; exit 1 }
$EngineRoot = $EngineRoot.Replace('/', '\').TrimEnd('\')
$RunUAT = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"

if (-not $Version) {
    $Commit = (& git -C $Project rev-parse --short HEAD 2>$null)
    if (-not $Commit) { $Commit = "nogit" }
    $Version = "$(Get-Date -Format 'yyyy-MM-dd')-$Commit"
}
$Archive = Join-Path $Project "Saved\Packages\$Version"
$Logs = Join-Path $Project "Saved_Tests\Logs"
New-Item -ItemType Directory -Force $Logs | Out-Null
$Log = Join-Path $Logs "package.log"
$ErrorLog = Join-Path $Logs "package.stderr.log"
# Lines of this script: to the screen and the log; UAT then rewrites the log, so they are added again at its end.
$Notes = New-Object Collections.Generic.List[string]
function Say([string] $Line) { Write-Host $Line; Add-Content -Path $Log -Value $Line -Encoding utf8; $Notes.Add($Line) }
Set-Content -Path $Log -Value "package : $Version, $(Split-Path -Leaf $Project), started $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -Encoding utf8

# The processes of this engine (paths compared with backslashes, case ignored), as build.ps1.
function Get-EngineProcesses {
    @(Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -and $_.CommandLine.Replace('/', '\') -like "*$EngineRoot\*" })
}
function Copy-Of($Process) {
    if ($Process.CommandLine.Replace('/', '\') -match '([A-Za-z]:\\[^"=]*?)\\HeroesOfLiberty\.uproject') { Split-Path -Leaf $Matches[1] } else { "another project" }
}
function Is-ThisCopy($Process) { $Process.CommandLine.Replace('/', '\') -like "*$UProject*" }
$EngineProcesses = Get-EngineProcesses
$Unreal = @($EngineProcesses | Where-Object { $_.Name -match '^UnrealEditor' })

# 1. This copy busy: stop, unless -Force.
$Mine = @($Unreal | Where-Object { Is-ThisCopy $_ })
if ($Mine.Count -and -not $Force) {
    Say "package : BUSY (Unreal runs in this copy, its DLLs are locked; wait for it or close it, or -Force):"
    $Mine | ForEach-Object { Say "  pid $($_.ProcessId) $($_.Name) $(($_.CommandLine -replace '^.*?\.uproject"?\s*', '') -replace '^(.{0,120}).*', '$1')" }
    exit 2
}

# 2. Live Coding open in other copies only: skip UnrealBuildTool's guard.
$LiveCoding = @($Unreal | Where-Object { $_.CommandLine -notmatch '-LiveCoding=0' -and $_.CommandLine -notmatch '-run=' })
$Extra = @()
if ($LiveCoding.Count -and -not @($LiveCoding | Where-Object { Is-ThisCopy $_ }).Count) {
    $Extra += "-ubtargs=-NoHotReloadFromIDE"
    Say "Live Coding open elsewhere ($(($LiveCoding | ForEach-Object { Copy-Of $_ } | Select-Object -Unique) -join ', ')): its guard skipped"
}

# 3. Another build running on this engine: say whose, wait for it to end (at most 1 h).
$Clock = [Diagnostics.Stopwatch]::StartNew()
$Said = ""
while ($true) {
    $Builds = @(Get-EngineProcesses | Where-Object { $_.CommandLine -match 'UnrealBuildTool' -and $_.Name -match '^(dotnet|UnrealBuildTool)' })
    if (-not $Builds.Count) { break }
    $Whose = ($Builds | ForEach-Object { Copy-Of $_ } | Select-Object -Unique) -join ', '
    if ($Whose -ne $Said) { Say "queued behind the build of: $Whose (one build at a time on this engine)"; $Said = $Whose }
    if ($Clock.Elapsed.TotalMinutes -gt 60) { Say "package : FAIL (still waiting for the build of $Whose after 60 min)"; exit 1 }
    Start-Sleep -Seconds 5
}
$Waited = [math]::Round($Clock.Elapsed.TotalSeconds)

# The same version packaged again (same day, same commit): UAT copies over the old archive without removing what it
# no longer stages, so the old archive goes first (a packaged game still running from it keeps it: stop).
if (Test-Path $Archive) {
    try { Remove-Item -Recurse -Force $Archive -ErrorAction Stop; Say "previous archive of $Version removed" }
    catch { Say "package : FAIL (previous archive $Archive not removed: $($_.Exception.Message))"; exit 1 }
}

# 4. UAT: client (the Game target) and server, Development, cooked, in .pak files, archived. Its output goes straight
# to the log (readable while it runs, from its first line); its errors to package.stderr.log, appended at the end.
$Arguments = @("BuildCookRun", "-project=`"$UProject`"", "-noP4", "-utf8output", "-unattended",
    "-platform=Win64", "-clientconfig=$Config", "-server", "-serverplatform=Win64", "-serverconfig=$Config",
    "-build", "-cook", "-stage", "-pak", "-prereqs", "-archive", "-archivedirectory=`"$Archive`"") + $Extra
Say "RunUAT $($Arguments -join ' ')"
$Clock.Restart()
# Not -Wait: it waits for every descendant, and the cook may start daemons that stay up (zenserver, UnrealTraceServer),
# which hung the script after a successful UAT. Wait for UAT itself (its handle read first so ExitCode is kept).
$Uat = Start-Process -FilePath $RunUAT -ArgumentList ($Arguments -join ' ') -NoNewWindow -PassThru `
    -RedirectStandardOutput $Log -RedirectStandardError $ErrorLog
$null = $Uat.Handle
$Uat.WaitForExit()
$Code = $Uat.ExitCode
$Minutes = [math]::Round($Clock.Elapsed.TotalMinutes, 1)
Add-Content -Path $Log -Value (@("--- package.ps1 ---") + $Notes) -Encoding utf8
$Errors = @(Get-Content $ErrorLog -Encoding utf8 -ErrorAction SilentlyContinue)
if ($Errors.Count) { Add-Content -Path $Log -Value (@("--- stderr ---") + $Errors) -Encoding utf8 }

$Waiting = if ($Waited -gt 5) { ", waited $Waited s for another build" } else { "" }
# 5. The version, written beside the cooked game of each platform (R1.7): after the build, so no tracked file changes
# and a new version rebuilds nothing; the game reads it as a loose file (HOLVersion::Get).
if ($Code -eq 0) {
    $Games = @(Get-ChildItem $Archive -Directory -ErrorAction SilentlyContinue | ForEach-Object { Join-Path $_.FullName "HeroesOfLiberty" } | Where-Object { Test-Path $_ })
    if (-not $Games.Count) { Say "package : FAIL (${Version}: no <platform>\HeroesOfLiberty folder in $Archive for Version.txt)"; exit 1 }
    foreach ($Game in $Games) { [IO.File]::WriteAllText((Join-Path $Game "Version.txt"), $Version, (New-Object Text.UTF8Encoding $false)) }
}
# 6. -Playtest: the manager and the players' launchers beside the game (add_playtest.ps1).
if ($Code -eq 0 -and $Playtest) {
    $Setup = @(& powershell -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot "add_playtest.ps1") -Root $Project -PackageDir $Archive)
    $SetupCode = $LASTEXITCODE
    $Setup | ForEach-Object { Say "  $_" }
    if ($SetupCode -ne 0) { Say "package : FAIL (${Version}: playtest setup, add_playtest exit $SetupCode)"; exit 1 }
}
if ($Code -eq 0) {
    $Sizes = @(Get-ChildItem $Archive -Directory -ErrorAction SilentlyContinue | ForEach-Object {
        $Bytes = (Get-ChildItem $_.FullName -Recurse -File | Measure-Object Length -Sum).Sum
        "$($_.Name) $([math]::Round($Bytes / 1MB)) MB" })
    Say "package : PASS ($Version, $(Split-Path -Leaf $Project), $Minutes min$Waiting; $Archive : $($Sizes -join ', '))"
    exit 0
}
Say "package : FAIL ($Version, $(Split-Path -Leaf $Project), $Minutes min$Waiting, exit $Code; log $Log)"
Get-Content $Log -Encoding utf8 | Select-String -Pattern 'error|Error:|ERROR' | Select-Object -First 15 | ForEach-Object { Say "  $($_.Line.Trim())" }
exit 1
