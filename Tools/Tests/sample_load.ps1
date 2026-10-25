# R5.5: samples every 2 s, until -Seconds, the memory of this worktree's dedicated servers (working set / private
# bytes, per pid), the editor clients' total, the PC's total CPU and free memory, into a CSV.
param([string] $Out, [int] $Seconds = 240)
$Root = (Resolve-Path "$PSScriptRoot\..\..").Path
$Cpu = New-Object System.Diagnostics.PerformanceCounter("Processor", "% Processor Time", "_Total")
$null = $Cpu.NextValue()
"time,kind,pid,ws_mb,private_mb,cpu_total,free_mb" | Set-Content $Out
$End = (Get-Date).AddSeconds($Seconds)
while ((Get-Date) -lt $End) {
    Start-Sleep -Seconds 2
    $C = [Math]::Round($Cpu.NextValue(), 1)
    $Free = [Math]::Round((Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory / 1KB)
    $T = (Get-Date).ToString("HH:mm:ss")
    $Lines = @()
    Get-Process HeroesOfLibertyServer -ErrorAction SilentlyContinue | Where-Object { $_.Path -like "$Root*" } | ForEach-Object {
        $Lines += "$T,server,$($_.Id),$([Math]::Round($_.WorkingSet64/1MB)),$([Math]::Round($_.PrivateMemorySize64/1MB)),$C,$Free" }
    $Ed = Get-CimInstance Win32_Process -Filter "Name='UnrealEditor.exe'" | Where-Object { $_.CommandLine -like "*Glorfindel*" }
    if ($Ed) { $Lines += "$T,clients,$(@($Ed).Count),$([Math]::Round(($Ed | Measure-Object WorkingSetSize -Sum).Sum/1MB)),$([Math]::Round(($Ed | Measure-Object PrivatePageCount -Sum).Sum/1MB)),$C,$Free" }
    if (-not $Lines) { $Lines = "$T,none,0,0,0,$C,$Free" }
    $Lines | Add-Content $Out
}
