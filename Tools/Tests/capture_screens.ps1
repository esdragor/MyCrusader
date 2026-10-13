# End-to-end check of the front end (D246): every menu screen captured in French, English and Japanese, and a match
# launched from the New game screen by the same path as its button. Run from Windows PowerShell (never Git Bash: it
# rewrites /Game/... paths), with no editor of the project open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\capture_screens.ps1
# Results: Saved_Tests\Screenshots\WindowsEditor\HOL\e2e_<culture>_<screen>.png, one contact sheet per culture
# (e2e_<culture>.png, needs Python with Pillow) and a PASS / FAIL line per culture.
# Test launches use their own Saved_Tests folder (-saveddirsuffix=Tests) whose settings put the window on the user's
# second screen (Tools\Tests\GameUserSettings.ini; -WinX/-WinY are logical coordinates).
param(
    [string[]] $Cultures = @("fr", "en", "ja"),
    [string] $Engine = "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor.exe"
)

$Project = Resolve-Path "$PSScriptRoot\..\.."
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
$TestSaved = Join-Path $Project "Saved_Tests"
$Shots = Join-Path $TestSaved "Screenshots\WindowsEditor\HOL"
$Log = Join-Path $TestSaved "Logs\HeroesOfLiberty.log"

# The test settings, once: windowed 1600 x 900 on the second screen.
$TestSettings = Join-Path $TestSaved "Config\WindowsEditor\GameUserSettings.ini"
if (-not (Test-Path $TestSettings)) {
    New-Item -ItemType Directory -Force (Split-Path $TestSettings) | Out-Null
    Copy-Item (Join-Path $PSScriptRoot "GameUserSettings.ini") $TestSettings
}

# Screens in order, each opened by the same path as its button, then captured; the match is launched last.
$Screens = @("title", "quit", "newgame", "video", "sound", "controls", "language")
$Allok = $true
foreach ($Culture in $Cultures) {
    $Name = { param($Screen) "e2e_${Culture}_$Screen" }
    $Commands = @(
        "hol.UI.Capture $(& $Name title) 4",
        "hol.UI.Open Quit 4.5", "hol.UI.Capture $(& $Name quit) 5.5", "hol.UI.Key Escape 6",
        "hol.UI.Open NewGame 6.5", "hol.UI.Capture $(& $Name newgame) 7.5", "hol.UI.Key Escape 8",
        "hol.UI.Open Settings:0 8.5", "hol.UI.Capture $(& $Name video) 9.5",
        "hol.UI.Open Settings:1 10", "hol.UI.Capture $(& $Name sound) 10.8",
        "hol.UI.Open Settings:2 11.3", "hol.UI.Capture $(& $Name controls) 12.1",
        "hol.UI.Open Settings:3 12.6", "hol.UI.Capture $(& $Name language) 13.4", "hol.UI.Key Escape 14",
        "hol.UI.NewGame.Launch - 15", "hol.UI.Capture $(& $Name battle) 24 quit"
    ) -join ","
    $Arguments = "`"$UProject`" -game -windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100 -saveddirsuffix=Tests -nosplash -culture=$Culture -ExecCmds=`"$Commands`""
    # Captures of an earlier run would hide a missing one.
    Remove-Item (Join-Path $Shots "e2e_${Culture}*.png") -ErrorAction SilentlyContinue
    $Process = Start-Process -FilePath $Engine -ArgumentList $Arguments -PassThru
    if (-not $Process.WaitForExit(180000)) { $Process.Kill(); Write-Host "$Culture : FAIL (timeout)"; $Allok = $false; continue }

    $Missing = @($Screens + "battle" | Where-Object { -not (Test-Path (Join-Path $Shots "$(& $Name $_).png")) })
    $Launched = Select-String -Path $Log -Pattern "Match on BattleMap: given configuration" -Quiet
    $Errors = @(Select-String -Path $Log -Pattern "LogHOLUI: (Error|Warning)|LogHOLMatch: (Error|Warning)" | ForEach-Object { $_.Line })
    $Ok = ($Missing.Count -eq 0) -and $Launched -and ($Errors.Count -eq 0)
    $Allok = $Allok -and $Ok
    $Status = if ($Ok) { "PASS" } else { "FAIL" }
    Write-Host "$Culture : $Status (missing captures: $($Missing -join ', '); match launched from the screen: $Launched; interface errors: $($Errors.Count))"
    $Errors | Select-Object -First 5 | ForEach-Object { Write-Host "  $_" }

    if (Get-Command python -ErrorAction SilentlyContinue) {
        python (Join-Path $PSScriptRoot "make_sheet.py") $Shots "e2e_$Culture" ($Screens + "battle")
    }
}
if ($Allok) { exit 0 } else { exit 1 }
