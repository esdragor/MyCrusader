# Test of Tools\Scenarios\HudRects.ps1 (L1-12b): the rule preflight.ps1 applies to the "HUD rects:" line of a scenario's log, i.e. the boxes of
# every VISIBLE widget as drawn (name=x0,y0,x1,y1 in the HUD's Slate units, ';' between widgets): two boxes that cover each other by more than
# the tolerance on both axes are a panel hiding another (the HUD line of ai_evolving under "Triches", the duel window under the scenario's panel).
# It runs the rule on the same text preflight.ps1 reads (the log line), not on values written into the test.
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\hud_rects_test.ps1
$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "HudRects.ps1")
$Fails = 0
function Case([string] $Name, [bool] $Ok, [string] $Detail = "") {
    Write-Host "  $(if ($Ok) { 'PASS' } else { 'FAIL' }) : $Name$(if (-not $Ok -and $Detail) { " ($Detail)" })"
    if (-not $Ok) { $script:Fails++ }
}

$Two = "[2026.11.07-09.50.00:000][  1]LogHOLUI: Display: HUD rects: cheats=0,0,220,300; ai_line=10,250,200,280; menu=1700,0,1920,60"
$Found = @(Find-HudOverlaps $Two)
Case "two boxes that cover each other are found (cheats x ai_line)" ($Found.Count -eq 1 -and $Found[0] -eq "cheats x ai_line") ($Found -join " / ")

$Touch = "HUD rects: a=0,0,100,100; b=100,0,200,100; c=0,100,100,200"
Case "touching edges do not count" (@(Find-HudOverlaps $Touch).Count -eq 0) (@(Find-HudOverlaps $Touch) -join " / ")

$Hair = "HUD rects: a=0,0,100,100; b=99.5,0,200,100"
Case "a hairline (under the 1-unit tolerance) does not count" (@(Find-HudOverlaps $Hair).Count -eq 0)

$Nested = "HUD rects: window=100,100,900,700; duel window=200,200,400,300"
$FoundNested = @(Find-HudOverlaps $Nested)
Case "a box inside another is found, names with a space kept" ($FoundNested.Count -eq 1 -and $FoundNested[0] -eq "window x duel window") ($FoundNested -join " / ")

$Apart = "HUD rects: a=0,0,10,10; b=20,20,30,30; c=0,40,10,50"
Case "separate boxes: none" (@(Find-HudOverlaps $Apart).Count -eq 0)

$Several = "HUD rects: a=0,0,100,100; b=50,50,150,150; c=120,120,200,200"
$FoundSeveral = @(Find-HudOverlaps $Several)
Case "a chain a-b-c gives a x b and b x c, not a x c" ($FoundSeveral.Count -eq 2 -and $FoundSeveral -contains "a x b" -and $FoundSeveral -contains "b x c") ($FoundSeveral -join " / ")

Case "a line without rects, or a malformed entry, finds nothing and does not throw" (@(Find-HudOverlaps "HUD overlap: none").Count -eq 0 -and @(Find-HudOverlaps "HUD rects: a=1,2,3; b=x").Count -eq 0)

# The same rule on a log file, as preflight.ps1 calls it: one verdict per "HUD rects:" line.
$Log = Join-Path ([IO.Path]::GetTempPath()) ("hud_rects_test_" + [Guid]::NewGuid().ToString("N").Substring(0, 8) + ".log")
Set-Content -Encoding ascii $Log @("Scenario x: step 1", $Apart, $Two, "Scenario x: ready")
try {
    $FromLog = @(Get-HudRectOverlaps (Get-Content $Log))
    Case "from a log: only the line that overlaps is reported, with its widgets" ($FromLog.Count -eq 1 -and $FromLog[0] -match "cheats x ai_line") ($FromLog -join " / ")
} finally { Remove-Item $Log -Force -ErrorAction SilentlyContinue }

Write-Host "hud_rects_test : $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
if ($Fails -eq 0) { exit 0 } else { exit 1 }
