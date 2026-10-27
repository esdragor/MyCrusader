# Opens the game on a test scenario (Tools/Scenarios/<Name>.txt, README.md), with the user's own settings (window,
# screen, saved games): the user's trial, not a scripted check. Run from Windows PowerShell (never Git Bash: it rewrites
# /Game/... paths):
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play.ps1 [-Name] <name> [-Root <worktree>] [-EngineDir <dir>]
# Without a name: lists the scenarios, each with the first comment line of its file. -LiveCoding=0: an open game
# would otherwise block every build of the project (UnrealBuildTool refuses while Live Coding runs), any worktree.
param(
    [Parameter(Position = 0)] [string] $Name = "",
    [string] $Root = "",
    [string] $EngineDir = ""
)

if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$Scenarios = Join-Path $Root "Tools\Scenarios"

if (-not $Name) {
    Write-Host "Scenarios ($Scenarios):"
    foreach ($File in Get-ChildItem (Join-Path $Scenarios "*.txt") | Sort-Object Name) {
        $Description = Get-Content -Encoding UTF8 $File.FullName | Where-Object { $_ -match "^\s*#" } | Select-Object -First 1
        $Description = $(if ($Description) { ($Description -replace "^\s*#\s*", "") } else { "" })
        Write-Host ("  {0,-20} {1}" -f $File.BaseName, $Description)
    }
    Write-Host "Usage: play.ps1 <name> [-Root <worktree>]"
    exit 0
}

if (-not (Test-Path (Join-Path $Scenarios "$Name.txt"))) {
    Write-Host "play: no scenario '$Name' in $Scenarios (play.ps1 alone lists them)"
    exit 1
}

$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
if (-not $EngineDir) {
    # An installed build registered under its name (HKCU\Software\Epic Games\Unreal Engine\Builds), as quick_check.ps1.
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "play: engine '$Association' of the .uproject not found in the registry; pass -EngineDir"; exit 1 }
    $EngineDir = Join-Path $EngineRoot "Engine\Binaries\Win64"
}
$Editor = Join-Path $EngineDir "UnrealEditor.exe"
Write-Host "play: scenario $Name ($UProject)"
# The game starts without waiting behind another copy's build (UBT's SDK check, up to minutes) and with its Angelscript
# debug server on a free port, where no VS Code connects (on the default one the game froze, 2026-10-27).
$env:UE_SKIP_UBT_SDK_SETUP = "1"
Start-Process -FilePath $Editor -ArgumentList @("`"$UProject`"", "-game", "-nosplash", "-culture=fr", "-LiveCoding=0", "-asdebugport=0", "-ExecCmds=`"hol.Scenario $Name`"")
