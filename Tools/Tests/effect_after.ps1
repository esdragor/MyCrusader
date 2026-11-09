# Celeborn-22 : waits for the melee_stuck lot of effect_measure.ps1, stops the rest of that chain (the melee_stuck_old lot), then runs quick_check on the touched parts.
$Root = 'C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn'
$Out = "$Root\Saved\c22"
while (-not (Select-String -Path "$Out\progress.txt" -Pattern 'done melee_stuck ' -Quiet -ErrorAction SilentlyContinue)) { Start-Sleep -Seconds 5 }
Start-Sleep -Seconds 5
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like '*effect_measure.ps1*' -or $_.CommandLine -like '*repeat_scenario.ps1*melee_stuck_old*' } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Get-Process UnrealEditor-Cmd, UnrealEditor -ErrorAction SilentlyContinue | Where-Object { $_.Path -like '*hol_bincopy_HeroesOfLiberty-Celeborn*' } | Stop-Process -Force -ErrorAction SilentlyContinue
"quick_check start $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath "$Out\progress.txt" -Append -Encoding utf8
& "$Root\Tools\Tests\quick_check.ps1" -Parts tests, combat, move, neutrals *> "$Out\qc.txt"
"quick_check done $(Get-Date -Format HH:mm:ss)" | Out-File -FilePath "$Out\progress.txt" -Append -Encoding utf8
