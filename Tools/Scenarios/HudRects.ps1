# The "a panel hides another" rule on the boxes of every visible widget (L1-12b), dot-sourced by preflight.ps1 and hud_rects_test.ps1.
# The game's log line (to be written by hol.HUD.Report, see FogInProgress.md "besoin C++ L1-12b"):
#   HUD rects: <name>=<x0>,<y0>,<x1>,<y1>; <name>=...        (Slate units of the HUD, as HOLHUDLayout::Overlaps, visible widgets only)
# Two boxes cover each other when they overlap by more than the tolerance on BOTH axes (touching edges and a hairline do not count: the same
# rule as HOLHUDLayout::Overlaps in Source\HeroesOfLibertyUI\HUD\HOLHUDLayoutRules.h).

# "A x B" for each pair of boxes of one "HUD rects:" line that cover each other (empty: none, or no such line).
function Find-HudOverlaps([string] $Line, [double] $Tolerance = 1.0) {
    $Marker = $Line.IndexOf("HUD rects:")
    if ($Marker -lt 0) { return @() }
    $Boxes = @()
    foreach ($Entry in ($Line.Substring($Marker + "HUD rects:".Length) -split ";")) {
        if ($Entry -notmatch '^\s*(.+?)\s*=\s*(-?[\d\.]+),(-?[\d\.]+),(-?[\d\.]+),(-?[\d\.]+)\s*$') { continue }
        $Boxes += [pscustomobject]@{
            Name = $Matches[1]
            X0 = [double]::Parse($Matches[2], [Globalization.CultureInfo]::InvariantCulture); Y0 = [double]::Parse($Matches[3], [Globalization.CultureInfo]::InvariantCulture)
            X1 = [double]::Parse($Matches[4], [Globalization.CultureInfo]::InvariantCulture); Y1 = [double]::Parse($Matches[5], [Globalization.CultureInfo]::InvariantCulture)
        }
    }
    $Found = @()
    for ($A = 0; $A -lt $Boxes.Count; $A++) {
        for ($B = $A + 1; $B -lt $Boxes.Count; $B++) {
            $Across = [Math]::Min($Boxes[$A].X1, $Boxes[$B].X1) - [Math]::Max($Boxes[$A].X0, $Boxes[$B].X0)
            $Down = [Math]::Min($Boxes[$A].Y1, $Boxes[$B].Y1) - [Math]::Max($Boxes[$A].Y0, $Boxes[$B].Y0)
            if ($Across -gt $Tolerance -and $Down -gt $Tolerance) { $Found += "$($Boxes[$A].Name) x $($Boxes[$B].Name)" }
        }
    }
    return $Found
}

# One text per "HUD rects:" line of a log that holds overlaps: "<line number>: A x B; C x D". Empty when no line or no overlap.
function Get-HudRectOverlaps([string[]] $LogLines) {
    $Out = @()
    for ($i = 0; $i -lt $LogLines.Count; $i++) {
        if ($LogLines[$i] -notmatch "HUD rects:") { continue }
        $Found = @(Find-HudOverlaps $LogLines[$i])
        if ($Found.Count -gt 0) { $Out += "line $($i + 1): " + ($Found -join "; ") }
    }
    return $Out
}
