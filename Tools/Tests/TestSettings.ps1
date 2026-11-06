# Shared by the test tools (dot-sourced by Scenarios\check.ps1 and Tests\quick_check.ps1): the settings file of a test launch
# (-saveddirsuffix=Tests: <worktree>\Saved_Tests\Config\WindowsEditor\GameUserSettings.ini).
#
# Set-TestEdgeScrollOff: writes bEdgeScroll=False (HOLGameUserSettings, the "edge scrolling" option of the controls screen) in
# that file, created from the template next to this script when missing. Why: with the option on, a cursor at the screen's edge
# pans the camera, so a scenario or a check moved the camera by itself (2026-11-04, Auto_Journey_File.md "Outils de scénario").
# Idempotent; every other line of the file is left as it is. Not used by play.ps1: that one is the user's own trial, with the
# user's own settings.
function Set-TestEdgeScrollOff([string] $SavedDir) {
    $Settings = Join-Path $SavedDir "Config\WindowsEditor\GameUserSettings.ini"
    $Section = "[/Script/HeroesOfLiberty.HOLGameUserSettings]"
    if (-not (Test-Path $Settings)) {
        New-Item -ItemType Directory -Force (Split-Path $Settings) | Out-Null
        Copy-Item (Join-Path $PSScriptRoot "GameUserSettings.ini") $Settings
    }
    $Lines = @(Get-Content $Settings)
    $Out = New-Object System.Collections.Generic.List[string]
    $Done = $false
    foreach ($Line in $Lines) {
        if ($Line -match '^bEdgeScroll=') {
            if (-not $Done) { $Out.Add("bEdgeScroll=False"); $Done = $true }   # a second copy would be ignored: dropped
            continue
        }
        $Out.Add($Line)
        if (-not $Done -and $Line.Trim() -eq $Section) { $Out.Add("bEdgeScroll=False"); $Done = $true }
    }
    if (-not $Done) { $Out.Add(""); $Out.Add($Section); $Out.Add("bEdgeScroll=False") }
    Set-Content -Encoding ascii $Settings $Out
}
