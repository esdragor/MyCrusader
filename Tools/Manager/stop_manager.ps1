# Stops the lobby manager started by "Gestionnaire HOL.bat" (<copy>\Saved\Manager\HOLManager.exe) and the game servers
# it started (its child processes), then the cmd.exe that runs that .bat, so that its tab or window closes.
# Never by window title: under Windows Terminal the title belongs to WindowsTerminal.exe, and killing it closes every
# tab (2026-11-24: a title filter took the whole terminal down). Only processes whose executable path is -Exe, their
# children, and their parent cmd.exe when its command line names Gestionnaire.
# Asks before stopping when the manager says lobbies or matches are running (GET /status), unless -Force.
#   powershell -ExecutionPolicy Bypass -File Tools\Manager\stop_manager.ps1 [-Exe <HOLManager.exe>] [-Port 7700] [-Force]
# Prints one line; exit code 0 (stopped or not running) / 1 (still running or refused).
param(
    [string] $Exe = "",
    [int] $Port = 7700,
    [switch] $Force
)

if (-not $Exe) { $Exe = Join-Path (Resolve-Path "$PSScriptRoot\..\..").Path "Saved\Manager\HOLManager.exe" }
$Exe = [IO.Path]::GetFullPath($Exe)

function Get-Managers {
    @(Get-CimInstance Win32_Process -Filter "Name = 'HOLManager.exe'" |
        Where-Object { $_.ExecutablePath -and [string]::Equals($_.ExecutablePath, $Exe, 'OrdinalIgnoreCase') })
}

$Managers = Get-Managers
if ($Managers.Count -eq 0) { Write-Host "stop_manager : not running ($Exe)"; exit 0 }

if (-not $Force) {
    try {
        $Status = Invoke-RestMethod "http://127.0.0.1:$Port/status" -TimeoutSec 3
        if ($Status.lobbies -gt 0 -or $Status.servers -gt 0) {
            Write-Host "Salons ouverts : $($Status.lobbies), parties en cours : $($Status.servers)."
            $Answer = Read-Host "Des joueurs utilisent le gestionnaire. L'arreter quand meme ? (o/n)"
            if ($Answer -notmatch '^[oOyY]') { Write-Host "stop_manager : not stopped (refused)"; exit 1 }
        }
    } catch {
        # No answer: nothing to protect, stop it anyway.
    }
}

foreach ($Manager in $Managers) {
    $Parent = Get-CimInstance Win32_Process -Filter "ProcessId = $($Manager.ParentProcessId)" -ErrorAction SilentlyContinue
    # The manager and its game servers (children).
    & taskkill.exe /PID $Manager.ProcessId /T /F 2>&1 | Out-Null
    # The cmd.exe of "Gestionnaire HOL.bat" only (never a terminal): its tab or window then closes.
    if ($Parent -and $Parent.Name -eq 'cmd.exe' -and $Parent.CommandLine -like '*Gestionnaire*') {
        & taskkill.exe /PID $Parent.ProcessId /F 2>&1 | Out-Null
    }
}

Start-Sleep -Milliseconds 500
if ((Get-Managers).Count -gt 0) { Write-Host "stop_manager : FAIL (still running)"; exit 1 }
Write-Host "stop_manager : stopped ($Exe)"
exit 0
