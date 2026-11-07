# Where a capture of a scenario run lands (used by preflight.ps1, tested by capture_resolve_selftest.ps1).
# The engine writes captures under Saved_Tests\Screenshots\<platform>\HOL (WindowsEditor in the -game runs of check.ps1), not directly under
# Saved_Tests\Screenshots\HOL. An old file left in the second place by an earlier run must neither hide the new one (the first version took
# Screenshots\HOL whenever it existed, so the pre-flight called a good capture "old") nor be the path printed for a missing capture
# (the en_/ja_ captures of legions_roster_gesture printed Screenshots\HOL\ while the files were in WindowsEditor\HOL\).
# Returns the path of the NEWEST existing candidate, or the WindowsEditor path (where the files really go) when none exists.
function Resolve-CapturePath([string]$Root, [string]$Capture) {
    $Candidates = @(
        (Join-Path $Root ("Saved_Tests\Screenshots\WindowsEditor\HOL\" + $Capture + ".png")),
        (Join-Path $Root ("Saved_Tests\Screenshots\HOL\" + $Capture + ".png"))
    )
    $Existing = @($Candidates | Where-Object { Test-Path $_ } | Sort-Object { (Get-Item $_).LastWriteTime } -Descending)
    if ($Existing.Count -gt 0) { return $Existing[0] }
    return $Candidates[0]
}
