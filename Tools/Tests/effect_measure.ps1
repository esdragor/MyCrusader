# Celeborn-22 : N=8 on the CURRENT binary, three scenarios at the default hol.Orders.SlotBlockedStopDistance, each lot followed by the EFFECT check
# (melee_stuck_gaps.py --effect : the real Q + click gesture must make the targets lose HP), the strike check and the walk check.
# Output : Saved\c22\m_<name>.txt (PASS k/N, k = PASS count), effect_<name>.txt, strike_<name>.txt, walk_<name>.txt, progress.txt, logs copied to Saved\c22\<name>\.
param([string[]]$Order = @('diplomacy_qground_watch', 'melee_stuck', 'melee_stuck_old'), [int]$Runs = 8)
$Root = 'C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn'
$Out = "$Root\Saved\c22"
New-Item -ItemType Directory -Force -Path $Out | Out-Null
foreach ($Name in $Order) {
    $Start = Get-Date
    & powershell -ExecutionPolicy Bypass -File "$Root\Tools\Tests\repeat_scenario.ps1" $Name -Runs $Runs -Root $Root *> "$Out\m_$Name.txt"
    New-Item -ItemType Directory -Force -Path "$Out\$Name" | Out-Null
    $Logs = Get-ChildItem "$Root\Saved_Tests\Logs\scenario_${Name}_run*.log" -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -gt $Start }
    $Paths = @($Logs | ForEach-Object { $_.FullName })
    $Logs | ForEach-Object { Copy-Item $_.FullName "$Out\$Name\" -Force }
    if ($Paths.Count -gt 0) {
        & python "$Root\Tools\Tests\melee_stuck_gaps.py" --effect @Paths *> "$Out\effect_$Name.txt"
        & python "$Root\Tools\Tests\melee_stuck_gaps.py" --strike @Paths *> "$Out\strike_$Name.txt"
        & python "$Root\Tools\Tests\melee_stuck_gaps.py" --walk @Paths *> "$Out\walk_$Name.txt"
    }
    "done $Name $(Get-Date -Format HH:mm:ss) (start $($Start.ToString('HH:mm:ss'))), $($Paths.Count) logs" | Out-File -FilePath "$Out\progress.txt" -Append -Encoding utf8
}
"ALL DONE $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath "$Out\progress.txt" -Append -Encoding utf8
