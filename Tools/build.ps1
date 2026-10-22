# Builds one copy of the project (the main copy or a worktree) without waiting on the other sessions by hand.
# Every copy shares one engine (UE 5.8 AS 3), so:
# - one UnrealBuildTool at a time, whatever the copy: a build started meanwhile queues behind it (-WaitMutex);
# - the Live Coding lock is named after the engine's UnrealEditor.exe, shared by every copy: an editor or game opened
#   elsewhere with Live Coding on would make UnrealBuildTool refuse. That is a false alarm for another copy (its own
#   Binaries and Intermediate), so the guard is skipped then (-NoHotReloadFromIDE).
# Games, tests and editors of OTHER copies never block this build. One of THIS copy does (its DLLs are locked): the
# script stops and names it (-Force to try anyway).
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\build.ps1 [-Root <copy>] [-Target HeroesOfLibertyEditor] [-Config Development] [-Force]
# Full log: <copy>\Saved\Logs\build.log. Prints one line PASS / FAIL with the first errors; exit code 0 / 1 (2: this
# copy is busy).
param(
    [string] $Root = "",
    [string] $Target = "HeroesOfLibertyEditor",
    [string] $Config = "Development",
    [switch] $Force
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not (Test-Path $UProject)) { Write-Host "build : FAIL (no HeroesOfLiberty.uproject in $Project)"; exit 1 }
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
if (-not $EngineRoot) { Write-Host "build : FAIL (engine '$Association' of the .uproject not found in the registry)"; exit 1 }
$EngineRoot = $EngineRoot.Replace('/', '\').TrimEnd('\')
$Log = Join-Path $Project "Saved\Logs\build.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null

# The processes of this engine (paths compared with backslashes, case ignored).
$EngineProcesses = @(Get-CimInstance Win32_Process | Where-Object {
    $_.CommandLine -and $_.CommandLine.Replace('/', '\') -like "*$EngineRoot\*" })
function Copy-Of($Process) {
    if ($Process.CommandLine.Replace('/', '\') -match '([A-Za-z]:\\[^"=]*?)\\HeroesOfLiberty\.uproject') { Split-Path -Leaf $Matches[1] } else { "another project" }
}
function Is-ThisCopy($Process) { $Process.CommandLine.Replace('/', '\') -like "*$UProject*" }
$Unreal = @($EngineProcesses | Where-Object { $_.Name -match '^UnrealEditor' })

# 1. This copy busy (game, test, editor or commandlet loading its DLLs): stop, unless -Force.
$Mine = @($Unreal | Where-Object { Is-ThisCopy $_ })
if ($Mine.Count -and -not $Force) {
    Write-Host "build : BUSY (Unreal runs in this copy, its DLLs are locked; wait for it or close it, or -Force):"
    $Mine | ForEach-Object { Write-Host "  pid $($_.ProcessId) $($_.Name) $(($_.CommandLine -replace '^.*?\.uproject"?\s*', '') -replace '^(.{0,120}).*', '$1')" }
    exit 2
}

# 2. Live Coding open in other copies only: skip UnrealBuildTool's guard (false alarm, see above).
$LiveCoding = @($Unreal | Where-Object { $_.CommandLine -notmatch '-LiveCoding=0' -and $_.CommandLine -notmatch '-run=' })
$Extra = @()
if ($LiveCoding.Count -and -not @($LiveCoding | Where-Object { Is-ThisCopy $_ }).Count) {
    $Extra += "-NoHotReloadFromIDE"
    Write-Host "Live Coding open elsewhere ($(($LiveCoding | ForEach-Object { Copy-Of $_ } | Select-Object -Unique) -join ', ')): its guard skipped"
}

# 3. Another build running: say whose, then queue behind it.
$Builds = @($EngineProcesses | Where-Object { $_.CommandLine -match 'UnrealBuildTool' -and $_.Name -match '^(dotnet|UnrealBuildTool)' })
if ($Builds.Count) {
    Write-Host "queued behind the build of: $(($Builds | ForEach-Object { Copy-Of $_ } | Select-Object -Unique) -join ', ') (one build at a time on this engine)"
}

$Clock = [Diagnostics.Stopwatch]::StartNew()
& (Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat") $Target Win64 $Config "-Project=$UProject" -WaitMutex @Extra *> $Log
$Code = $LASTEXITCODE
$Seconds = [math]::Round($Clock.Elapsed.TotalSeconds)
$Text = Get-Content $Log
if ($Code -eq 0) {
    Write-Host "build : PASS ($Target $Config, $(Split-Path -Leaf $Project), $Seconds s)"
    exit 0
}
Write-Host "build : FAIL ($Target $Config, $(Split-Path -Leaf $Project), $Seconds s, exit $Code; log $Log)"
$Text | Select-String -Pattern 'error|Live Coding|conflicting instance' | Select-Object -First 15 | ForEach-Object { Write-Host "  $($_.Line.Trim())" }
exit 1
