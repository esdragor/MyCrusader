# Pan exposure of a scenario (2026-11-08, E-fog_trees: "le scenario reste bloque en preparation").
# A scenario that puts the camera (hol.Camera.Focus*) and, many frames later, clicks on a ground point
# (hol.Select.LeftClickGround, OrderGround, ...) depends on the camera NOT moving in between. In the user's game (play.ps1: window, the user's
# settings) the cursor at the screen's edge (bEdgeScroll, ~40 m/s) or a key held pans the camera during those frames; the point then leaves the
# screen ("hol.Select.LeftClickGround: the point is not on the screen"), the click never comes, the next `waitlog Scripted left click at` waits
# for ever and the panel stays on "en preparation...". check.ps1 and preflight.ps1 run with the edge scrolling OFF and no cursor, so they never saw it.
# Rule: at most $Max frames (a `wait <s>` counts 60 frames per second) between the last camera placement and a ground click; put the camera right
# before the click, never before a long wait.
function Get-PanExposure([string[]] $Lines, [int] $Max = 10) {
    $Frames = -1   # -1: no camera placed yet by the scenario (the default camera, not a drift concern)
    $Offenders = @()
    $Number = 0
    foreach ($Raw in $Lines) {
        $Number++
        $Line = $Raw.Trim()
        if (-not $Line -or $Line.StartsWith("#")) { continue }
        $Line = $Line -replace '^quiet\s+', ''
        if ($Line -match '^hol\.Camera\.Focus\w*\b') { $Frames = 0; continue }
        if ($Line -match '^frames\s+(\d+)') { if ($Frames -ge 0) { $Frames += [int]$Matches[1] }; continue }
        if ($Line -match '^wait\s+([\d\.]+)') { if ($Frames -ge 0) { $Frames += [int]([double]$Matches[1] * 60) }; continue }
        if ($Line -match '^hol\.Select\.(LeftClickGround|OrderGround|ClickGround|AttackMove|BoxGround|Banner|ClickUnit)\b' -and $Frames -gt $Max) {
            $What = if ($Matches[1] -eq 'ClickUnit') { "the unit leaves the screen (hol.Select.ClickUnit finds nothing, the selection stays empty)" } else { "the point leaves the screen" }
            $Offenders += "line ${Number}: '$Line' comes $Frames frames after the camera was placed (max $Max): a cursor at the screen's edge or a held key pans the camera meanwhile and $What"
        }
    }
    return $Offenders
}
