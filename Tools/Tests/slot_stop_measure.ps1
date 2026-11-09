# Celeborn-20 : SlotBlockedStopDistance 120 / 150 / 250 (defaut) x8 sur trois scenarios, une suite de lots, sortie dans Saved\c20\m_<nom>.txt
$Root = 'C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn'
$Out = "$Root\Saved\c20"
$Order = @('melee_stuck_s120', 'melee_stuck_s150', 'melee_stuck', 'melee_stuck_old_s120', 'melee_stuck_old_s150', 'melee_stuck_old', 'diplomacy_qground_watch_s120', 'diplomacy_qground_watch_s150', 'diplomacy_qground_watch')
foreach ($Name in $Order) {
    $Start = Get-Date
    New-Item -ItemType Directory -Force "$Out\stale" | Out-Null
    Get-ChildItem "$Root\Saved_Tests\Logs\scenario_${Name}_run*.log" -ErrorAction SilentlyContinue | Move-Item -Destination "$Out\stale" -Force -ErrorAction SilentlyContinue
    & powershell -ExecutionPolicy Bypass -File "$Root\Tools\Tests\repeat_scenario.ps1" $Name -Runs 8 -Root $Root *> "$Out\m_$Name.txt"
    $Gaps = ''
    "done $Name $(Get-Date -Format HH:mm:ss) (start $($Start.ToString('HH:mm:ss')))" | Out-File -FilePath "$Out\progress.txt" -Append -Encoding utf8
    $Logs = Get-ChildItem "$Root\Saved_Tests\Logs\scenario_${Name}_run*.log" -ErrorAction SilentlyContinue | ForEach-Object { $_.FullName }
    if ($Logs) {
        & python "$Root\Tools\Tests\sanity_stall_check.py" $Logs *> "$Out\stall_$Name.txt"
        & python "$Root\Tools\Tests\melee_stuck_gaps.py" --walk $Logs *> "$Out\gaps_$Name.txt"
        & python "$Root\Tools\Tests\melee_stuck_gaps.py" --strike $Logs *> "$Out\strike_$Name.txt"
    }
}
"ALL DONE $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath "$Out\progress.txt" -Append -Encoding utf8
