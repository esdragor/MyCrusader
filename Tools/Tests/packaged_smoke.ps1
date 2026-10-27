# Smoke check of the packaged game (M6 R1.2): the client of Tools\Package\package.ps1, off screen (-RenderOffScreen,
# no window, no sound), plays a short sequence (hol.Test.Run, as quick_check.ps1): the title screen, the New game screen
# (the path of "Jouer"), a match on Plain for one human, a capture, a Peasant produced by the main centre. Then its log
# is read: it fails on content the cook left out (Failed to load, Couldn't find file, LoadObject failed...), a fatal
# error or a crash, an interface / match / sequence error, a step of the sequence missed, or the line
# "Version: <name of the package folder>" missing (R1.7).
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\packaged_smoke.ps1 [-Root <copy>] [-Package <dir>] [-Culture fr]
# Without -Package: the newest folder of <copy>\Saved\Packages. Log: <copy>\Saved_Tests\Logs\packaged_smoke.log; the
# captures smoke_*.png are copied to <copy>\Saved_Tests\Screenshots\Smoke. Prints one line PASS / FAIL with the first
# faults; exit code 0 / 1.
param(
    [string] $Root = "",
    [string] $Package = "",
    [string] $Culture = "fr",
    [int] $TimeoutSeconds = 300
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
if (-not $Package) {
    $Newest = Get-ChildItem (Join-Path $Project "Saved\Packages") -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime | Select-Object -Last 1
    if (-not $Newest) { Write-Host "packaged_smoke : FAIL (no package in $Project\Saved\Packages; run Tools\Package\package.ps1)"; exit 1 }
    $Package = $Newest.FullName
}
$Package = (Resolve-Path $Package).Path
# The client: the game itself (not the bootstrap exe at the top, which starts it), in Windows\ or WindowsClient\.
$Client = Get-ChildItem $Package -Recurse -File -Include "HeroesOfLiberty.exe", "HeroesOfLibertyClient.exe" -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match '\\Binaries\\Win64\\' -and $_.FullName -notmatch '\\WindowsServer\\' } | Select-Object -First 1
if (-not $Client) { Write-Host "packaged_smoke : FAIL (no game client exe in $Package)"; exit 1 }

$TestSaved = Join-Path $Project "Saved_Tests"
$Logs = Join-Path $TestSaved "Logs"
$Shots = Join-Path $TestSaved "Screenshots\Smoke"
New-Item -ItemType Directory -Force $Logs, $Shots | Out-Null
$GameLog = Join-Path $Logs "packaged_smoke.log"
Remove-Item -ErrorAction SilentlyContinue $GameLog
Get-ChildItem $Shots -Filter "smoke_*.png" -ErrorAction SilentlyContinue | Remove-Item

# One human on Plain, nobody else (the format of -MatchConfig=, Tools\Scenarios\README.md).
$Config = Join-Path $TestSaved "packaged_smoke.json"
'{ "map": "/Game/Maps/Plain.Plain", "startingPeasants": 0, "slots": [ { "state": "Human", "colorIndex": 0 } ] }' | Set-Content -Encoding ascii $Config
# The steps (HOLTestSequence.cpp). A Peasant in 2 s, not 20 (hol.Prod.BuildTime, as quick_check's net part).
$Steps = @(
    "waitmap MainMenu", "waitui", "mark smoke title", "capture smoke_title",
    "hol.UI.Open NewGame", "waitui", "capture smoke_newgame",
    "hol.UI.NewGame.Launch $Config 0", "waitmap Plain", "waitlog Camera above the start", "frames 30", "mark smoke match",
    "capture smoke_start",
    "hol.Prod.BuildTime 2", "hol.Prod.Queue", "waitlog Production: Peasant of slot 0 done", "waitunits 1", "frames 10",
    "hol.Units.Report", "capture smoke_peasant",
    "mark end", "quit")
$SequenceFile = Join-Path $TestSaved "packaged_smoke.seq"
$Steps | Set-Content -Encoding utf8 $SequenceFile

$Arguments = "-RenderOffScreen -ResX=1600 -ResY=900 -nosound -unattended -as-exit-on-error -nosplash -culture=$Culture -abslog=`"$GameLog`" -ExecCmds=`"hol.Test.Run $SequenceFile`""
$Start = Get-Date
$Process = Start-Process -FilePath $Client.FullName -ArgumentList $Arguments -PassThru -WindowStyle Hidden
Write-Host "packaged_smoke : $($Client.FullName) pid $($Process.Id), log $GameLog"
$TimedOut = -not $Process.WaitForExit($TimeoutSeconds * 1000)
if ($TimedOut) { Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue }
$Seconds = [math]::Round(((Get-Date) - $Start).TotalSeconds)

$Log = @(Get-Content $GameLog -Encoding utf8 -ErrorAction SilentlyContinue)
$Faults = New-Object Collections.Generic.List[string]
if (-not $Log.Count) { $Faults.Add("no log written") }
if ($TimedOut) { $Faults.Add("the game did not quit within $TimeoutSeconds s (killed)") }

# Content the cook left out, crashes and errors of the game's own categories.
# Not content: the engine probes the DLLs of profilers absent from a player's machine (aqProf, VTune, PIX) at start.
$ProfilerProbe = "LogWindows: Failed to load '(aqProf|VtuneApi|VtuneApi32e|WinPixGpuCapturer)\.dll'"
$Missing = @($Log | Select-String -Pattern "Failed to load|Couldn't find file|LoadObject.*fail|Failed to find object|LoadPackage.*fail|LoadPackage: SkipPackage|LogStreaming: Error|Can't find file for asset" |
    Where-Object { $_.Line -notmatch $ProfilerProbe } | ForEach-Object { $_.Line.Trim() })
$Fatal = @($Log | Select-String -Pattern "Fatal error|Unhandled Exception|Assertion failed|appError|=== Critical error ===" | ForEach-Object { $_.Line.Trim() })
$Errors = @($Log | Select-String -Pattern "LogHOLUI: (Error|Warning)|LogHOLMatch: (Error|Warning)|LogHOLTest: Error" | ForEach-Object { $_.Line.Trim() })
if ($Missing.Count) { $Faults.Add("$($Missing.Count) missing content lines") }
if ($Fatal.Count) { $Faults.Add("$($Fatal.Count) fatal lines") }
if ($Errors.Count) { $Faults.Add("$($Errors.Count) interface, match or sequence errors") }

# Each step of the sequence reached, each capture on the screen it is named after (quick_check's $Expected).
$Wanted = [ordered]@{
    "title screen"            = "Test capture: smoke_title (menu=HOLTitleScreen modal=none)"
    "New game screen (Jouer)" = "Test capture: smoke_newgame (menu=HOLNewGameScreen modal=none)"
    "match on Plain"          = "Test capture: smoke_start (hud=HOLGameScreen)"
    "Peasant produced"        = "Production: Peasant of slot 0 done"
    "Peasant captured"        = "Test capture: smoke_peasant (hud=HOLGameScreen)"
    "sequence done"           = "Test mark: end"
}
foreach ($Step in $Wanted.Keys) {
    if (-not ($Log | Select-String -SimpleMatch $Wanted[$Step] -Quiet)) { $Faults.Add("step missed: $Step") }
}
# The version of the package (R1.7): the name of its folder, written by the game at start (LogHOLNet; the engine's own
# "LogInit: Version: 5.8..." is not it).
$Version = Split-Path -Leaf $Package
if (-not ($Log | Select-String -SimpleMatch "Version: $Version" -Quiet)) {
    $Said = @($Log | Select-String -Pattern "LogHOL\w+: (Display: )?Version: \S+" | ForEach-Object { $_.Matches[0].Value } | Select-Object -First 1)
    $Faults.Add("version line 'Version: $Version' missing$(if ($Said.Count) { " (the game says '$($Said[0])')" })")
}

# The captures, from the packaged game's own Saved folder (beside it, or in the user's local app data).
$Found = @(foreach ($Folder in $Package, (Join-Path $env:LOCALAPPDATA "HeroesOfLiberty")) {
    Get-ChildItem $Folder -Recurse -File -Filter "smoke_*.png" -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $Start } })
$Found | ForEach-Object { Copy-Item $_.FullName $Shots -Force }

if (-not $Faults.Count) {
    Write-Host "packaged_smoke : PASS ($Seconds s, $($Found.Count) captures in $Shots)"
    exit 0
}
Write-Host "packaged_smoke : FAIL ($Seconds s, $($Faults -join '; '); log $GameLog)"
@($Missing | Select-Object -First 10) + @($Fatal | Select-Object -First 5) + @($Errors | Select-Object -First 5) | ForEach-Object { Write-Host "  $_" }
exit 1
