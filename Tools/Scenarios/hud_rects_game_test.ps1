# Test of the game's "HUD rects:" line (L1-12b), on the REAL chain: the scenarios run in the game (check.ps1), hol.HUD.Report writes the line,
# HudRects.ps1 reads it. It asserts the EFFECT in the game's own log:
#   - ai_evolving: a "HUD rects:" line exists and holds the cheats' panel and the AI line (AILevelPanel) with real boxes (positive width and height);
#   - duel_challenge: a "HUD rects:" line holds the duel window (DuelPanel) with a real box;
#   - the verdict of preflight.ps1 on a real overlap: the real ai_evolving line plus one copy of the AI line's box shifted by 5 units is found by
#     Get-HudRectOverlaps (the function preflight.ps1 FAILs with); the real line itself is printed (INFO: overlaps that really exist, not judged here).
# Run (the game must be built: Tools\build.ps1):
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\hud_rects_game_test.ps1 [-Root <worktree>]
param([string] $Root = "")
if (-not $Root) { $Root = (Resolve-Path "$PSScriptRoot\..\..").Path }
$Root = (Resolve-Path $Root).Path
. (Join-Path $PSScriptRoot "HudRects.ps1")
$Fails = 0
function Case([string] $Name, [bool] $Ok, [string] $Detail = "") {
    if ($Ok) { Write-Host "  PASS $Name" } else { $script:Fails++; Write-Host "  FAIL $Name $Detail" }
}
function Get-Box([string] $Line, [string] $Widget) {
    $Pattern = [regex]::Escape($Widget) + '=(-?[\d\.]+),(-?[\d\.]+),(-?[\d\.]+),(-?[\d\.]+)'
    if ($Line -notmatch $Pattern) { return $null }
    $Inv = [Globalization.CultureInfo]::InvariantCulture
    return [pscustomobject]@{ X0 = [double]::Parse($Matches[1], $Inv); Y0 = [double]::Parse($Matches[2], $Inv); X1 = [double]::Parse($Matches[3], $Inv); Y1 = [double]::Parse($Matches[4], $Inv) }
}
function Test-Real($Box) { return ($null -ne $Box) -and ($Box.X1 - $Box.X0 -gt 1) -and ($Box.Y1 - $Box.Y0 -gt 1) }
function Invoke-Scenario([string] $Name) {
    & powershell -ExecutionPolicy Bypass -File (Join-Path $Root "Tools\Scenarios\check.ps1") $Name -Root $Root -TimeoutSeconds 240 | Out-Null
    $Log = Join-Path $Root "Saved_Tests\Logs\scenario_$Name.log"
    if (Test-Path $Log) { return @(Get-Content $Log) }
    return @()
}

Write-Host "hud_rects_game_test: ai_evolving"
$Ai = Invoke-Scenario "ai_evolving"
$AiLines = @($Ai | Where-Object { $_ -match "HUD rects:" })
Case "ai_evolving: the game logs a 'HUD rects:' line" ($AiLines.Count -gt 0) "(no such line in Saved_Tests\Logs\scenario_ai_evolving.log)"
$AiLast = if ($AiLines.Count -gt 0) { $AiLines[-1] } else { "" }
Case "ai_evolving: the cheats' panel is in the line with a real box" (Test-Real (Get-Box $AiLast "cheats")) $AiLast
Case "ai_evolving: the AI line (AILevelPanel) is in the line with a real box" (Test-Real (Get-Box $AiLast "AILevelPanel")) $AiLast

Write-Host "hud_rects_game_test: duel_challenge"
$Duel = Invoke-Scenario "duel_challenge"
$DuelLines = @($Duel | Where-Object { $_ -match "HUD rects:" -and $_ -match "DuelPanel=" })
Case "duel_challenge: a 'HUD rects:' line holds the duel window (DuelPanel) with a real box" ($DuelLines.Count -gt 0 -and (Test-Real (Get-Box $DuelLines[-1] "DuelPanel"))) "(lines with rects: $(@($Duel | Where-Object { $_ -match 'HUD rects:' }).Count))"

Write-Host "hud_rects_game_test: a real overlap fails preflight"
$AiBox = Get-Box $AiLast "AILevelPanel"
if (Test-Real $AiBox) {
    $Inv = [Globalization.CultureInfo]::InvariantCulture
    $Ghost = "; ghost=" + (($AiBox.X0 + 5).ToString($Inv)) + "," + (($AiBox.Y0 + 5).ToString($Inv)) + "," + (($AiBox.X1 + 5).ToString($Inv)) + "," + (($AiBox.Y1 + 5).ToString($Inv))
    $Doctored = @($AiLast + $Ghost)
    $Found = @(Get-HudRectOverlaps $Doctored)
    Case "the real line with a box shifted by 5 units over the AI line: preflight's verdict is an overlap" ($Found.Count -eq 1 -and $Found[0] -match "AILevelPanel x ghost") ($Found -join " / ")
}
else { Case "a real AI line box to shift" $false }
$RealAi = @(Get-HudRectOverlaps $AiLines)
$RealDuel = @(Get-HudRectOverlaps $Duel)
Write-Host ("  INFO ai_evolving real overlaps: " + $(if ($RealAi.Count -eq 0) { "none" } else { $RealAi -join " | " }))
Write-Host ("  INFO duel_challenge real overlaps: " + $(if ($RealDuel.Count -eq 0) { "none" } else { $RealDuel -join " | " }))

Write-Host "hud_rects_game_test : $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
if ($Fails -eq 0) { exit 0 } else { exit 1 }
