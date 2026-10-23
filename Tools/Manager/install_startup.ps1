# Starts the lobby manager with Windows (M6 R2.12): puts the shortcut "Serveur HeroesOfLiberty" to HOLManager.exe in
# the user's Startup folder. The manager then runs minimized (its log: <ManagerDir>\Logs\manager-<date>.log).
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Manager\install_startup.ps1 [-ManagerDir <dir>] [-StartupDir <dir>]
# -ManagerDir: folder of the published HOLManager.exe (default: <copy>\Saved\Manager, from build_manager.ps1).
# -StartupDir: default [Environment]::GetFolderPath('Startup'); another folder for a trial.
# Prints one line PASS / FAIL; exit code 0 / 1.
param(
    [string] $ManagerDir = "",
    [string] $StartupDir = ""
)

$ShortcutName = "Serveur HeroesOfLiberty.lnk"
if (-not $ManagerDir) { $ManagerDir = Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "Saved\Manager" }
if (-not $StartupDir) { $StartupDir = [Environment]::GetFolderPath('Startup') }
$Exe = Join-Path $ManagerDir "HOLManager.exe"
if (-not (Test-Path $Exe)) { Write-Host "install_startup : FAIL (no $Exe; run build_manager.ps1 or give -ManagerDir)"; exit 1 }
if (-not (Test-Path $StartupDir)) { Write-Host "install_startup : FAIL (no folder $StartupDir)"; exit 1 }
$Exe = (Resolve-Path $Exe).Path
$Link = Join-Path (Resolve-Path $StartupDir).Path $ShortcutName

try {
    $Shell = New-Object -ComObject WScript.Shell
    $Shortcut = $Shell.CreateShortcut($Link)
    $Shortcut.TargetPath = $Exe
    # appsettings.json is read from the working directory.
    $Shortcut.WorkingDirectory = Split-Path $Exe
    $Shortcut.WindowStyle = 7
    $Shortcut.Description = "HeroesOfLiberty lobby manager (port 7700)"
    $Shortcut.Save()
    $Target = $Shell.CreateShortcut($Link).TargetPath
} catch {
    Write-Host "install_startup : FAIL ($($_.Exception.Message))"; exit 1
}
if ($Target -ne $Exe) { Write-Host "install_startup : FAIL (shortcut points to '$Target', not $Exe)"; exit 1 }
Write-Host "install_startup : PASS ($Link -> $Exe)"
exit 0
