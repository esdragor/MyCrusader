# End-to-end check of the RTS camera (J1, D257): a match launched from the New game screen, at 2 then 8 players, on
# the match map; the camera must start above the player's start slot, then move with the arrows, the mouse wheel and
# the screen edge (a cursor simulated inside the game, never the desktop's). Run from Windows PowerShell (never Git
# Bash: it rewrites /Game/... paths), with no editor of the project open:
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\check_camera.ps1
# Results: a PASS / FAIL line per case, captures Saved_Tests\Screenshots\WindowsEditor\HOL\cam_<case>_*.png.
param(
    [string] $Engine = "C:\UE\UE_5.8_AS_1\Engine\Binaries\Win64\UnrealEditor.exe"
)

$Project = Resolve-Path "$PSScriptRoot\..\.."
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
$TestSaved = Join-Path $Project "Saved_Tests"
$Shots = Join-Path $TestSaved "Screenshots\WindowsEditor\HOL"
$Log = Join-Path $TestSaved "Logs\HeroesOfLiberty.log"

$TestSettings = Join-Path $TestSaved "Config\WindowsEditor\GameUserSettings.ini"
if (-not (Test-Path $TestSettings)) {
    New-Item -ItemType Directory -Force (Split-Path $TestSettings) | Out-Null
    Copy-Item (Join-Path $PSScriptRoot "GameUserSettings.ini") $TestSettings
}

# Start slots of /Game/Maps/Plain (Tools/Editor/create_match_map.py): 150 m from the centre, at these angles.
$SlotAngles = @(180, 0, 270, 90, 225, 45, 315, 135)
function Get-SlotStart([int] $Slot) {
    $Radians = $SlotAngles[$Slot] * [Math]::PI / 180
    return @([Math]::Round(15000 * [Math]::Cos($Radians)), [Math]::Round(15000 * [Math]::Sin($Radians)))
}

# The local player takes the first human slot (D243): 2 players -> slot 0; 8 players, seven AI first -> slot 7.
$Cases = @(
    @{ Name = "2p"; Slot = 0; States = @("Human", "Human", "Closed", "Closed", "Closed", "Closed", "Closed", "Closed") },
    @{ Name = "8p"; Slot = 7; States = @("AI", "AI", "AI", "AI", "AI", "AI", "AI", "Human") }
)

$AllOk = $true
foreach ($Case in $Cases) {
    $Slots = for ($i = 0; $i -lt 8; $i++) { "{ `"state`": `"$($Case.States[$i])`", `"colorIndex`": $i }" }
    $Config = Join-Path $TestSaved "camera_$($Case.Name).json"
    "{ `"map`": `"/Game/Maps/Plain.Plain`", `"slots`": [ $($Slots -join ', ') ] }" | Set-Content -Encoding ascii $Config

    $Name = "cam_$($Case.Name)"
    # Arrow up held 1 s, one wheel step out, then the cursor 1 s on the right edge; a report after each.
    $Commands = @(
        "hol.Camera.TestCursor 0.5 0.5",
        "hol.UI.NewGame.Launch $Config 3",
        "hol.UI.Capture ${Name}_start 9", "hol.Camera.Report 9.2",
        "hol.UI.Key Up 9.5 1.0", "hol.Camera.Report 11",
        "hol.UI.Key MouseScrollDown 11.5", "hol.Camera.Report 12",
        "hol.Delay 12.5 hol.Camera.TestCursor 0.999 0.5", "hol.Delay 13.5 hol.Camera.TestCursor 0.5 0.5", "hol.Camera.Report 14",
        "hol.UI.Capture ${Name}_end 14.5 quit"
    ) -join ","
    $Arguments = "`"$UProject`" -game -windowed -ResX=1600 -ResY=900 -WinX=3940 -WinY=100 -saveddirsuffix=Tests -nosplash -culture=fr -ExecCmds=`"$Commands`""
    Remove-Item (Join-Path $Shots "$Name*.png") -ErrorAction SilentlyContinue
    $Process = Start-Process -FilePath $Engine -ArgumentList $Arguments -PassThru
    if (-not $Process.WaitForExit(180000)) { $Process.Kill(); Write-Host "$($Case.Name) : FAIL (timeout)"; $AllOk = $false; continue }

    # Reports: start, after the arrow, after the wheel, after the edge.
    $Reports = @(Select-String -Path $Log -Pattern "Camera: focus \((-?\d+), (-?\d+), (-?\d+)\), height (\d+)" | ForEach-Object {
        $g = $_.Matches[0].Groups; @{ X = [int]$g[1].Value; Y = [int]$g[2].Value; Height = [int]$g[4].Value } })
    $Expected = Get-SlotStart $Case.Slot
    $Checks = [ordered]@{}
    $Checks["match map, no pawn"] = Select-String -Path $Log -Pattern "Match map: RTS camera, no pawn" -Quiet
    $Checks["holds slot $($Case.Slot)"] = Select-String -Path $Log -Pattern "holds slot $($Case.Slot) " -Quiet
    $Checks["four reports"] = $Reports.Count -eq 4
    if ($Reports.Count -eq 4) {
        $Checks["starts above slot $($Case.Slot) ($($Expected -join ', '))"] = ([Math]::Abs($Reports[0].X - $Expected[0]) -le 2) -and ([Math]::Abs($Reports[0].Y - $Expected[1]) -le 2)
        $Up = $Reports[1].X - $Reports[0].X
        $Checks["arrow up 1 s: +X ~40 m (got $([Math]::Round($Up / 100)) m)"] = ($Up -ge 3000) -and ($Up -le 5000) -and ($Reports[1].Y -eq $Reports[0].Y)
        $Checks["wheel down: 35 -> 40 m (got $($Reports[2].Height / 100) m)"] = $Reports[2].Height -eq 4000
        $Edge = $Reports[3].Y - $Reports[2].Y
        $Checks["right edge 1 s: +Y (got $([Math]::Round($Edge / 100)) m)"] = ($Edge -ge 3000) -and ($Edge -le 6500)
    }
    $Errors = @(Select-String -Path $Log -Pattern "LogHOLUI: (Error|Warning)|LogHOLMatch: (Error|Warning)" | ForEach-Object { $_.Line })
    $Checks["no interface or match error"] = $Errors.Count -eq 0
    $Checks["captures"] = (Test-Path (Join-Path $Shots "${Name}_start.png")) -and (Test-Path (Join-Path $Shots "${Name}_end.png"))

    $Failed = @($Checks.Keys | Where-Object { -not $Checks[$_] })
    $AllOk = $AllOk -and ($Failed.Count -eq 0)
    Write-Host "$($Case.Name) : $(if ($Failed.Count -eq 0) { 'PASS' } else { 'FAIL' })"
    $Checks.Keys | ForEach-Object { Write-Host "  [$(if ($Checks[$_]) { 'ok' } else { 'KO' })] $_" }
    $Errors | Select-Object -First 5 | ForEach-Object { Write-Host "  $_" }
}
if (-not $AllOk) { exit 1 }
