# Shared editor slots (Auto_Journey_Checklist L7-3): 2 to 8 editors ran at once on the machine and the pre-flights of one session did not
# finish. Like Tools\build.ps1 for builds, a ticket per editor in a shared folder; at most -Max editors (default 2) run at the same time,
# the others wait in the order they asked. Dot-source it (. EditorSlot.ps1), then:
#   $Got = Enter-EditorSlot -Name <scenario> -Root <worktree> [-Max 2] [-WaitSeconds 1200]   # $true when a slot is held (waits otherwise)
#   ... start the editor, wait for it, stop it ...
#   Exit-EditorSlot                                                                           # frees the slot
# check.ps1 (hence preflight.ps1) takes one around its game. A ticket belongs to a process (pid + start time): when that process dies
# without freeing it (killed, crashed), the next one to look drops it, so a slot is never lost. Tickets: one JSON per editor under
# C:\Users\baudo\Documents\MyCrusader\Docs\BuildQueue.state\editors (a sub-folder: build.ps1 reads the *.json of the folder above) or
# $env:HOL_EDITOR_SLOT_DIR (tests). All reads and writes happen under an exclusive lock file (queue.lock), as build.ps1 does.
# Tested end to end by Tools\Scenarios\editor_slot_test.ps1. Not covered: quick_check.ps1 and a hand launch of the editor.

$script:EditorSlotTicket = $null

function Get-EditorSlotDir {
    if ($env:HOL_EDITOR_SLOT_DIR) { $Dir = $env:HOL_EDITOR_SLOT_DIR }
    else { $Dir = "C:\Users\baudo\Documents\MyCrusader\Docs\BuildQueue.state\editors" }
    New-Item -ItemType Directory -Force $Dir | Out-Null
    return $Dir
}

function Use-EditorSlotLock([scriptblock]$Body) {
    $LockPath = Join-Path (Get-EditorSlotDir) "queue.lock"
    $Stream = $null
    for ($Try = 0; $Try -lt 200 -and -not $Stream; $Try++) {
        try { $Stream = [IO.File]::Open($LockPath, [IO.FileMode]::OpenOrCreate, [IO.FileAccess]::ReadWrite, [IO.FileShare]::None) }
        catch { Start-Sleep -Milliseconds 100 }
    }
    if (-not $Stream) { throw "editor slots: the lock $LockPath is held by another process for 20 s" }
    try { return (& $Body) } finally { $Stream.Dispose() }
}

function Test-EditorSlotOwner($Ticket) {
    $Process = Get-Process -Id ([int]$Ticket.ownerPid) -ErrorAction SilentlyContinue
    if (-not $Process) { return $false }
    try { return ($Process.StartTime.ToUniversalTime().Ticks -eq [int64]$Ticket.ownerStart) } catch { return $false }
}

# The tickets whose owner is alive, oldest request first (inside the lock); the others are removed.
function Read-EditorSlotTickets {
    $Alive = @()
    foreach ($File in Get-ChildItem (Get-EditorSlotDir) -Filter *.json -ErrorAction SilentlyContinue) {
        $Ticket = $null
        try { $Ticket = Get-Content $File.FullName -Raw | ConvertFrom-Json } catch { }
        if (-not $Ticket) { continue }
        if (Test-EditorSlotOwner $Ticket) { $Ticket | Add-Member NoteProperty File $File.FullName -Force; $Alive += $Ticket }
        else { Remove-Item $File.FullName -Force -ErrorAction SilentlyContinue }
    }
    return @($Alive | Sort-Object { [datetime]::Parse($_.requested, [Globalization.CultureInfo]::InvariantCulture, 'RoundtripKind') })
}

function Enter-EditorSlot([string]$Name, [string]$Root, [int]$Max = 2, [int]$WaitSeconds = 1200) {
    $Self = Get-Process -Id $PID
    $Id = [guid]::NewGuid().ToString("N").Substring(0, 8)
    $Requested = (Get-Date).ToString("o")
    $File = Join-Path (Get-EditorSlotDir) (("{0}_{1}.json" -f (Get-Date).ToUniversalTime().ToString("yyyyMMddHHmmssfff"), $Id))
    $Ticket = [ordered]@{ id = $Id; name = $Name; root = $Root; state = "waiting"; requested = $Requested; ownerPid = $PID; ownerStart = $Self.StartTime.ToUniversalTime().Ticks }
    Use-EditorSlotLock { [IO.File]::WriteAllText($File, ($Ticket | ConvertTo-Json), (New-Object Text.UTF8Encoding $false)) } | Out-Null
    $script:EditorSlotTicket = $File
    $Start = Get-Date
    $Told = $false
    while ($true) {
        $Entered = Use-EditorSlotLock {
            $All = Read-EditorSlotTickets
            $Running = @($All | Where-Object { $_.state -eq "running" }).Count
            $Waiting = @($All | Where-Object { $_.state -eq "waiting" })
            $Place = 0
            for ($I = 0; $I -lt $Waiting.Count; $I++) { if ($Waiting[$I].id -eq $Id) { $Place = $I } }
            if ($Running + $Place -lt $Max) {
                $Ticket.state = "running"
                [IO.File]::WriteAllText($File, ($Ticket | ConvertTo-Json), (New-Object Text.UTF8Encoding $false))
                return $true
            }
            return $false
        }
        if ($Entered) { return $true }
        if (((Get-Date) - $Start).TotalSeconds -ge $WaitSeconds) {
            Exit-EditorSlot
            Write-Host "editor slot: no slot after $WaitSeconds s ($Max editors already run)"
            return $false
        }
        if (-not $Told) { Write-Host "editor slot: $Max editors already run on this machine, waiting for a slot ($Name)"; $Told = $true }
        Start-Sleep -Seconds 2
    }
}

function Exit-EditorSlot {
    if ($script:EditorSlotTicket) {
        Remove-Item $script:EditorSlotTicket -Force -ErrorAction SilentlyContinue
        $script:EditorSlotTicket = $null
    }
}
