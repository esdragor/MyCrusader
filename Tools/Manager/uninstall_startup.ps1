# Removes the shortcut "Serveur HeroesOfLiberty" from the user's Startup folder (M6 R2.12); the running manager, if
# any, is left alone.
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Manager\uninstall_startup.ps1 [-StartupDir <dir>]
# -StartupDir: default [Environment]::GetFolderPath('Startup'); another folder for a trial.
# Prints one line PASS / FAIL; exit code 0 / 1 (no shortcut is a PASS).
param(
    [string] $StartupDir = ""
)

if (-not $StartupDir) { $StartupDir = [Environment]::GetFolderPath('Startup') }
$Link = Join-Path $StartupDir "Serveur HeroesOfLiberty.lnk"
if (-not (Test-Path $Link)) { Write-Host "uninstall_startup : PASS (no shortcut in $StartupDir)"; exit 0 }
try {
    Remove-Item -LiteralPath $Link -Force -ErrorAction Stop
} catch {
    Write-Host "uninstall_startup : FAIL ($($_.Exception.Message))"; exit 1
}
Write-Host "uninstall_startup : PASS (removed $Link)"
exit 0
