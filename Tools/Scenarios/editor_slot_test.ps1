# End-to-end test of the shared editor slots (Tools\Scenarios\EditorSlot.ps1, Auto_Journey_Checklist L7-3): three real processes ask for a
# slot at once with a cap of 2; at most 2 may hold one at the same time (and 2 do, so the cap is not 1), the third enters only after
# one left; a ticket whose owner is dead does not count. Uses a temporary folder (HOL_EDITOR_SLOT_DIR), no editor is started.
# Run: powershell -ExecutionPolicy Bypass -File Tools\Scenarios\editor_slot_test.ps1   (about 20 s). Exit 0 = PASS.
$ErrorActionPreference = "Stop"
$Lib = Join-Path $PSScriptRoot "EditorSlot.ps1"
$Fails = 0
function Check([bool]$Ok, [string]$Text) { if ($Ok) { Write-Host "  OK   $Text" } else { Write-Host "  FAIL $Text"; $script:Fails++ } }
if (-not (Test-Path $Lib)) { Write-Host "  FAIL EditorSlot.ps1 does not exist"; Write-Host "editor_slot_test: FAIL (1)"; exit 1 }

$Dir = Join-Path $env:TEMP ("hol_editor_slots_" + [guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force $Dir | Out-Null
$env:HOL_EDITOR_SLOT_DIR = $Dir
$Events = Join-Path $Dir "events.log"
$Child = Join-Path $Dir "child.ps1"
@'
param([string]$Lib, [string]$Name, [string]$Events, [int]$Hold)
. $Lib
# Two processes may write at the same instant: retry (a lost line made the test flaky).
function Log([string]$Line) { for ($I = 0; $I -lt 100; $I++) { try { Add-Content $Events $Line -ErrorAction Stop; return } catch { Start-Sleep -Milliseconds 50 } } }
$Got = Enter-EditorSlot -Name $Name -Root "test" -Max 2 -WaitSeconds 60
Log ("enter {0} {1} {2}" -f $Name, [DateTime]::UtcNow.Ticks, $Got)
Start-Sleep -Seconds $Hold
Log ("exit {0} {1}" -f $Name, [DateTime]::UtcNow.Ticks)
Exit-EditorSlot
'@ | Set-Content $Child -Encoding UTF8

$Procs = foreach ($Name in "a", "b", "c") {
    Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$Child`" -Lib `"$Lib`" -Name $Name -Events `"$Events`" -Hold 6" -PassThru -WindowStyle Hidden -RedirectStandardOutput (Join-Path $Dir "out_$Name.txt")
    Start-Sleep -Milliseconds 700
}
$Procs | Wait-Process -Timeout 90
# The waiting message says that the editor is busy and how many are ahead in the queue (L7-3, 2026-11-07); the two that got a slot at once say nothing about waiting.
$OutC = [string]$(if (Test-Path (Join-Path $Dir "out_c.txt")) { Get-Content (Join-Path $Dir "out_c.txt") -Raw } else { "" })
Check ($OutC -match 'editeur occupe, \d+ en file') "the third prints 'editeur occupe, N en file' while it waits (output: $(($OutC -replace '\s+', ' ').Trim()))"
Check ($OutC -match 'en file \(c\)')"the message names the scenario (c)"
$OutA = [string]$(if (Test-Path (Join-Path $Dir "out_a.txt")) { Get-Content (Join-Path $Dir "out_a.txt") -Raw } else { "" })
Check ($OutA -notmatch 'occupe') "the first one does not wait and says nothing about a busy editor"
$Rows = @(Get-Content $Events | ForEach-Object { $F = $_ -split ' '; [pscustomobject]@{ Kind = $F[0]; Name = $F[1]; At = [int64]$F[2]; Got = $F[3] } })
$Enters = @($Rows | Where-Object Kind -eq "enter" | Sort-Object At)
$Exits = @($Rows | Where-Object Kind -eq "exit" | Sort-Object At)
Check ($Enters.Count -eq 3 -and $Exits.Count -eq 3) "the three processes entered and left (enters $($Enters.Count), exits $($Exits.Count))"
Check (@($Enters | Where-Object { $_.Got -notmatch 'True' }).Count -eq 0) "every wait ended with a slot (no timeout)"
$Max = 0
foreach ($E in $Enters) {
    $Open = @($Enters | Where-Object { $_.At -le $E.At }).Count - @($Exits | Where-Object { $_.At -le $E.At }).Count
    if ($Open -gt $Max) { $Max = $Open }
}
Check ($Max -eq 2) "at most 2 editors at once, and 2 did run together (peak $Max)"
if ($Enters.Count -eq 3 -and $Exits.Count -eq 3) {
    Check ($Enters[2].At -ge $Exits[0].At) "the third waited for a slot to be released (entered $([math]::Round(($Enters[2].At - $Exits[0].At) / 1e7, 1)) s after the first exit)"
}
Check (@(Get-ChildItem $Dir -Filter *.json -ErrorAction SilentlyContinue).Count -eq 0) "every ticket was removed at the end"

# A ticket whose owner is dead is dropped: write two for a process that is gone, then one entry must still succeed at once.
. $Lib
$Dead = Start-Process powershell -ArgumentList "-NoProfile -Command exit" -PassThru -WindowStyle Hidden
$Dead | Wait-Process
foreach ($I in 1, 2) {
    @{ id = "dead$I"; name = "dead$I"; root = "x"; state = "running"; requested = (Get-Date).ToString("o"); ownerPid = $Dead.Id; ownerStart = $Dead.StartTime.ToUniversalTime().Ticks } | ConvertTo-Json | Set-Content (Join-Path $Dir "20000101_dead$I.json") -Encoding UTF8
}
$T = Get-Date
$Got = Enter-EditorSlot -Name "after-dead" -Root "test" -Max 2 -WaitSeconds 20
Check ($Got -and ((Get-Date) - $T).TotalSeconds -lt 10) "two tickets of a dead owner do not hold the slots"
Exit-EditorSlot
Remove-Item $Dir -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item Env:\HOL_EDITOR_SLOT_DIR

# Every tool that starts a real editor takes a slot around it (a slot taken before Start-Process, freed after the wait/kill), not only check.ps1.
$Tests = Join-Path (Split-Path $PSScriptRoot -Parent) "Tests"
foreach ($Script in "crossings", "hinterland_paths", "cols_paths", "crown_paths", "fords_paths") {
    $Text = Get-Content (Join-Path $Tests "$Script.ps1") -Raw
    $Enter = $Text.IndexOf("Enter-EditorSlot"); $Start = $Text.IndexOf("Start-Process"); $Exit = $Text.LastIndexOf("Exit-EditorSlot"); $Wait = $Text.IndexOf("WaitForExit")
    Check ($Text -match 'EditorSlot\.ps1' -and $Enter -ge 0 -and $Enter -lt $Start -and $Exit -gt $Wait) "$Script.ps1 takes an editor slot before its editor and frees it after the wait"
}
if ($Fails -eq 0) { Write-Host "editor_slot_test: PASS"; exit 0 }
Write-Host "editor_slot_test: FAIL ($Fails)"
exit 1
