# Summary of the build queue statistics written by Tools\build.ps1 (C:\Users\baudo\Documents\MyCrusader\Docs\BuildQueue.csv):
# per worktree the number of builds, median / max wait in the queue and median / max compile duration (PASS and FAIL),
# the number of superseded / orphan entries, and the number of builds per hour.
#   powershell -ExecutionPolicy Bypass -File Tools\build_queue_report.ps1 [-Csv <file>] [-Since "2026-11-03 08:00"]
param(
    [string] $Csv = "C:\Users\baudo\Documents\MyCrusader\Docs\BuildQueue.csv",
    [datetime] $Since = [datetime]::MinValue
)
if (-not (Test-Path $Csv)) { Write-Host "build_queue_report : no $Csv yet"; exit 0 }
$Rows = @(Import-Csv $Csv | Where-Object { [datetime] $_.requested -ge $Since })
if (-not $Rows.Count) { Write-Host "build_queue_report : no rows"; exit 0 }
function Median($Values) {
    $V = @($Values | Sort-Object)
    if (-not $V.Count) { return 0 }
    $M = [int][math]::Floor($V.Count / 2)
    if ($V.Count % 2) { $V[$M] } else { ($V[$M - 1] + $V[$M]) / 2 }
}
$Built = @($Rows | Where-Object { $_.result -in 'PASS', 'FAIL' })
Write-Host ("builds: {0} ({1} PASS, {2} FAIL), superseded {3}, orphan {4}; from {5} to {6}" -f $Rows.Count,
    @($Rows | Where-Object result -eq 'PASS').Count, @($Rows | Where-Object result -eq 'FAIL').Count,
    @($Rows | Where-Object result -eq 'SUPERSEDED').Count, @($Rows | Where-Object result -eq 'ORPHAN').Count,
    ($Rows | Sort-Object requested | Select-Object -First 1).requested, ($Rows | Sort-Object requested | Select-Object -Last 1).requested)
Write-Host ""
Write-Host "Per worktree (compiled builds)"
$Built | Group-Object worktree | Sort-Object Name | ForEach-Object {
    $Name = $_.Name
    $Waits = @($_.Group | ForEach-Object { [double] $_.wait_s }); $Durs = @($_.Group | ForEach-Object { [double] $_.duration_s })
    [pscustomobject]@{
        worktree = $_.Name; builds = $_.Count
        wait_med_s = Median $Waits; wait_max_s = ($Waits | Measure-Object -Maximum).Maximum
        dur_med_s = Median $Durs; dur_max_s = ($Durs | Measure-Object -Maximum).Maximum
        merge = @($_.Group | Where-Object priority -eq 'merge').Count
        dropped = @($Rows | Where-Object { $_.worktree -eq $Name -and $_.result -in 'SUPERSEDED', 'ORPHAN' }).Count
    }
} | Format-Table -AutoSize | Out-String -Width 200 | Write-Host
$AllWaits = @($Built | ForEach-Object { [double] $_.wait_s })
Write-Host ("All: wait median {0} s, max {1} s; duration median {2} s, max {3} s" -f (Median $AllWaits), ($AllWaits | Measure-Object -Maximum).Maximum,
    (Median @($Built | ForEach-Object { [double] $_.duration_s })), (($Built | ForEach-Object { [double] $_.duration_s } | Measure-Object -Maximum).Maximum))
Write-Host ""
Write-Host "Builds requested per hour"
$Rows | Group-Object { ([datetime] $_.requested).ToString("yyyy-MM-dd HH") + ":00" } | Sort-Object Name |
    ForEach-Object { [pscustomobject]@{ hour = $_.Name; requested = $_.Count; compiled = @($_.Group | Where-Object { $_.result -in 'PASS', 'FAIL' }).Count
        wait_med_s = Median @($_.Group | Where-Object { $_.result -in 'PASS', 'FAIL' } | ForEach-Object { [double] $_.wait_s }) } } |
    Format-Table -AutoSize | Out-String -Width 200 | Write-Host
