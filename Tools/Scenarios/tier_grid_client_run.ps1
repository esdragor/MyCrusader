# Grid proofs on a NETWORK CLIENT and on the DEDICATED SERVER.
# (1) MIG-CPP-UnitTier proof (reprise 12, 2026-11-18): since the C++ rule HOLUnitTier::Check is gone, the production grid of
# the HUD asks the script board (UHOLUnitTierSubsystem, HOLUnitTierBoardScript.as) whether a unit's tier is open. If that board did not exist in
# a client's world, every unit cell would be greyed for a joined player. Dedicated server + 2 seated clients (Alice slot 0, Bob slot 1); Alice
# selects her own forum and logs her HUD (hol.HUD.Report): the Peasant cell must be "enabled" and the client log must not have the line
# "UnitTier: no script board".
# (2) Lot L4 proof (2026-11-19, after L3a): L3a made "no script grid board = empty grid" the rule of 19 functions of HOLCommandGrid.cpp
# (HOLCommandGrid.cpp:51, one Error line per function and world: "CommandGrid: <Function> refused, no script grid board"). Proof that no game world
# is without its board: the journals of the dedicated server, of Bob and of Alice must hold NONE of those lines, and each must hold
# "Board: CommandGrid script <class>" (HOLBoardLog.cpp:51, logged by every game world: server and client alike) and NO "Board: CommandGrid ABSENT".
#
# COMMAND (green pass, ~3 to 5 minutes, one call, launches 3 editor processes, kills them at the end; exit code 0 = PASS, 1 = FAIL):
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\tier_grid_client_run.ps1 -Root <worktree> -Expect enabled
# COMMAND (red pass, same call with -Expect empty, AFTER the main session put the stub "return FHOLGridCommandView();" at the top of
# GetCommandAnswer, Script\Player\HOLCommandGridBoard.as:13; the stub is removed by the main session, then the green pass must run again):
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\tier_grid_client_run.ps1 -Root <worktree> -Expect empty
# (-Expect greyed = the older red witness of the unit tiers: stub CheckUnitTier in Script\Units\HOLUnitTierBoardScript.as with
#  "return HOLUnitTier::TierTooLow;", the Peasant cell then shows "greyed".)
#
# VERDICTS (the line "tier_grid_client : PASS|FAIL (expected <mode>)", then one line "CHECK <name> : ok|FAIL <why>" per check):
#   -Expect enabled, PASS = Alice's HUD shows "<key>=Peasant enabled" (the grid read an answer of the script board on a client) AND UnitTier has no
#     "no script board" line in Alice's log AND, in the 3 journals (server, Bob, Alice): zero "CommandGrid: ... refused, no script grid board" line,
#     at least one "Board: CommandGrid script ..." line, zero "Board: CommandGrid ABSENT". Meaning: no game world of this match lacked its board.
#   -Expect empty, PASS = RED WITNESS: with the stubbed answer the HUD of Alice has NO Peasant cell (grid "empty" or other cells only), while the board IS
#     there (the same 3 journal checks stay green: an empty cell caused by a missing board would fail them). Meaning: the green pass really reads the
#     script board's answer on the client, it cannot come from a cell that is always filled. If the Peasant cell is still there with the stub, the stub
#     was not posed or the script not reloaded.
#   FAIL names in the CHECK lines: hud-report (no "HUD report ... selection MainCentre" line: Alice never selected her forum, nothing to conclude),
#     peasant-cell (the cell is not in the expected state), unittier-no-board, grid-refused-<server|bob|alice> (a refused line, cited: the board was
#     missing in that process), board-line-<server|bob|alice> (no "Board: CommandGrid script" line, or an ABSENT line, cited), log-missing-<server|bob|alice>.
param([string]$Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent), [string]$Out = (Join-Path $env:TEMP "tier_grid_client"), [ValidateSet('enabled', 'greyed', 'empty')][string]$Expect = 'enabled')
$w = $Root; New-Item -ItemType Directory -Force $Out | Out-Null
$up = (Get-ChildItem $w -Filter *.uproject).FullName
$ed = 'C:\UE\UE_5.8_HOL\Engine\Binaries\Win64\UnrealEditor.exe'
$port = 7994
$cfg = Join-Path $Out "tier_grid.json"
$cfgJson = @{ version = 2; mode = "Diplomacy"; map = "/Game/Maps/Plain.Plain"; startingPeasants = 5; slots = @(@{ state = "Human"; colorIndex = 0; playerName = "Alice" }, @{ state = "Human"; colorIndex = 1; playerName = "Bob" }, @{ state = "Closed"; colorIndex = 2 }, @{ state = "Closed"; colorIndex = 3 }, @{ state = "Closed"; colorIndex = 4 }, @{ state = "Closed"; colorIndex = 5 }, @{ state = "Closed"; colorIndex = 6 }, @{ state = "Closed"; colorIndex = 7 }) } | ConvertTo-Json -Depth 5
Set-Content -Encoding ascii $cfg $cfgJson
$seq = Join-Path $Out "tier_grid_alice.seq"
@("waitplayers 2", "frames 30", "hol.Camera.FocusSlot 0", "frames 3", "hol.Select.Click 0.5 0.5", "waitlog Selection: MainCentre of slot 0 (own)", "frames 5", "hol.HUD.Report", "frames 5", "mark tier grid end") | Set-Content -Encoding utf8 $seq
$sl = "$Out\tier_grid_server.log"; $c1 = "$Out\tier_grid_bob.log"; $c2 = "$Out\tier_grid_alice.log"
Remove-Item $sl, $c1, $c2 -ErrorAction SilentlyContinue
function WaitFor($f, $pat, $sec) { $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt $sec) { if ((Test-Path $f) -and (Select-String -Path $f -Pattern $pat -Quiet)) { return $true }; Start-Sleep 2 }; return $false }
# Lines of a journal that a dying editor may still hold open: shared read; empty array when the file is missing.
function Read-LogLines([string]$LogPath) {
    if (-not (Test-Path $LogPath)) { return @() }
    $fs = $null
    $sr = $null
    try {
        $fs = New-Object System.IO.FileStream($LogPath, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read, [System.IO.FileShare]::ReadWrite)
        $sr = New-Object System.IO.StreamReader($fs)
        $text = $sr.ReadToEnd()
        return @($text -split "`r?`n")
    }
    finally {
        if ($sr) { $sr.Dispose() } elseif ($fs) { $fs.Dispose() }
    }
}
# Cut a cited journal line to 260 characters.
function Format-Cited([string]$Line) { $t = $Line.Trim(); return $t.Substring(0, [Math]::Min(260, $t.Length)) }
# The L4 grid checks on ONE journal. NEGATIVE: no "CommandGrid: <Function> refused, no script grid board" line (text of HOLCommandGrid.cpp:51).
# POSITIVE: at least one "Board: CommandGrid script <class>" line (HOLBoardLog.cpp:51) and no "Board: CommandGrid ABSENT" line (HOLBoardLog.cpp:55).
# Returns the failures as "<check name> : FAIL <what> | <cited line>" (nothing when the journal is clean) and keeps the first presence line in $script:BoardLines.
function Test-GridJournal([string]$Who, [string]$LogPath) {
    $fails = New-Object System.Collections.Generic.List[string]
    if (-not (Test-Path $LogPath)) { $fails.Add("log-missing-$Who : FAIL no journal at $LogPath"); return $fails.ToArray() }
    $logLines = Read-LogLines $LogPath
    $refused = @($logLines | Where-Object { $_ -match 'CommandGrid: .* refused, no script grid board' })
    $present = @($logLines | Where-Object { $_ -match 'Board: CommandGrid script \S+' })
    $absent = @($logLines | Where-Object { $_ -match 'Board: CommandGrid ABSENT' })
    foreach ($ln in $refused) { $fails.Add("grid-refused-$Who : FAIL refused line | $(Format-Cited $ln)") }
    foreach ($ln in $absent) { $fails.Add("board-line-$Who : FAIL board ABSENT | $(Format-Cited $ln)") }
    if ($present.Count -eq 0 -and $absent.Count -eq 0) { $fails.Add("board-line-$Who : FAIL no 'Board: CommandGrid script' line in the journal (the board is not proven present)") }
    $script:BoardLines[$Who] = if ($present.Count -gt 0) { Format-Cited $present[0] } else { 'no presence line' }
    return $fails.ToArray()
}
"start $(Get-Date -Format HH:mm:ss) (expect the Peasant cell $Expect)"
$sv = Start-Process $ed -ArgumentList "`"$up`" /Game/Maps/Plain -server -nullrhi -nosound -Port=$port -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -as-exit-on-error -HOLCheats -MatchConfig=`"$cfg`" -saveddirsuffix=TierSrv -abslog=`"$sl`"" -PassThru -WindowStyle Hidden
"server up: $(WaitFor $sl 'Bringing World .* up for play|Angelscript: Error' 180) $(Get-Date -Format HH:mm:ss)"
$a = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Bob -game -nullrhi -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -saveddirsuffix=TierCli1 -abslog=`"$c1`"" -PassThru -WindowStyle Hidden
"Bob joined: $(WaitFor $c1 'Welcomed by server' 120) $(Get-Date -Format HH:mm:ss)"
$b = Start-Process $ed -ArgumentList "`"$up`" 127.0.0.1:${port}?Name=Alice -game -RenderOffScreen -ResX=960 -ResY=540 -nosound -unattended -nosplash -Multiprocess -LiveCoding=0 -asdebugport=0 -culture=fr -saveddirsuffix=TierCli2 -abslog=`"$c2`" -ExecCmds=`"hol.Camera.TestCursor 0.5 0.5,hol.Test.Run $seq`"" -PassThru -WindowStyle Hidden
"Alice reported her HUD: $(WaitFor $c2 'Test mark: tier grid end|mark tier grid end|HUD report: .*selection MainCentre' 150) $(Get-Date -Format HH:mm:ss)"
Start-Sleep 3
foreach ($p in $a, $b, $sv) { if ($p -and -not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
Start-Sleep 2
$report = if (Test-Path $c2) { Select-String -Path $c2 -Pattern 'HUD report: .*selection MainCentre' | Select-Object -Last 1 } else { $null }
$cell = if ($report) { [regex]::Match($report.Line, '\b\w+=Peasant (enabled|greyed|disabled)').Value } else { '' }
$gridText = if ($report) { [regex]::Match($report.Line, 'grid [^;]*?(?=, orders|, queue|$)').Value } else { 'NO HUD report with her forum selected' }
$noBoard = if (Test-Path $c2) { (Select-String -Path $c2 -Pattern 'UnitTier: no script board' | Measure-Object).Count } else { -1 }
"Alice's grid: $gridText"
"Peasant cell on the client: '$cell' ; 'no script board' lines in her log: $noBoard"

# Checks, one named line each: "<name> : ok ..." or "<name> : FAIL <why>".
$script:BoardLines = @{}
$checks = New-Object System.Collections.Generic.List[string]
$cellOk = switch ($Expect) {
    'enabled' { $cell -match 'Peasant enabled$' }
    'greyed' { $cell -match 'Peasant greyed$' }
    'empty' { $cell -eq '' }   # the stub gives no command: no Peasant cell at all (the HUD report skips a cell without a type)
}
if ($report) { $checks.Add("hud-report : ok") } else { $checks.Add("hud-report : FAIL no 'HUD report: ... selection MainCentre' line in Alice's journal (nothing to conclude on the cell)") }
if ($report -and $cellOk) { $checks.Add("peasant-cell : ok (expected $Expect, got '$cell')") }
elseif ($report) { $checks.Add("peasant-cell : FAIL expected $Expect, got '$cell' ; $gridText") }
if ($Expect -ne 'greyed') {
    if ($noBoard -eq 0) { $checks.Add("unittier-no-board : ok") } else { $checks.Add("unittier-no-board : FAIL $noBoard line(s) 'UnitTier: no script board' in Alice's log (-1 = no log)") }
}
foreach ($pair in @(@('server', $sl), @('bob', $c1), @('alice', $c2))) {
    $found = @(Test-GridJournal $pair[0] $pair[1])
    if ($found.Count -eq 0) { $checks.Add("grid-journal-$($pair[0]) : ok (no refused line ; $($script:BoardLines[$pair[0]]))") }
    else { foreach ($item in $found) { $checks.Add($item) } }
}
$bad = @($checks | Where-Object { $_ -match ' : FAIL ' })
$ok = ($bad.Count -eq 0)
"tier_grid_client : $(if ($ok) { 'PASS' } else { 'FAIL' }) (expected $Expect)"
foreach ($ck in $checks) { "CHECK $ck" }
if (-not $ok -and (Test-Path $c2)) { "=====ALICE (last lines of the sequence)"; Select-String -Path $c2 -Pattern 'Test |Selection:|HUD report|UnitTier|CommandGrid|NetworkFailure|Angelscript: Error' | Select-Object -Last 12 | % { $_.Line.Substring(0, [Math]::Min(260, $_.Line.Length)) } }
exit $(if ($ok) { 0 } else { 1 })
