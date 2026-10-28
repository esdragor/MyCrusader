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
# Then the server pass (IA0.5 B3, IA0.5-Q3 A, ~20 s): the package's dedicated server (WindowsServer\...\
# HeroesOfLibertyServer.exe) alone, no client, on Plain with slot 0 Human without a name (the match does not wait for it)
# and slot 1 AI, 5 starting Peasants (quick_check's "aiserver" part). Its log (<copy>\Saved_Tests\Logs\
# packaged_smoke_server.log) must show it listening, the precompiled scripts, the AI's brain awake on the dedicated
# server thinking for slot 1, its first step (5 idle -> gather) and the gather order of its 5 Peasants, no script error
# (Get-ScriptFileErrors: the engine's own bind errors aside) and no fatal line; then the server is stopped.
# -SkipServer: no server pass (said on the result line), for a package without a server; -ServerOnly: the server pass
# alone (no client run, said on the result line).
param(
    [string] $Root = "",
    [string] $Package = "",
    [string] $Culture = "fr",
    [int] $TimeoutSeconds = 300,
    [switch] $SkipServer,
    [switch] $ServerOnly,
    [int] $ServerTimeoutSeconds = 30
)
if ($SkipServer -and $ServerOnly) { Write-Host "packaged_smoke : FAIL (-SkipServer and -ServerOnly together: nothing to check)"; exit 1 }

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
if (-not $Package) {
    $Newest = Get-ChildItem (Join-Path $Project "Saved\Packages") -Directory -ErrorAction SilentlyContinue | Sort-Object LastWriteTime | Select-Object -Last 1
    if (-not $Newest) { Write-Host "packaged_smoke : FAIL (no package in $Project\Saved\Packages; run Tools\Package\package.ps1)"; exit 1 }
    $Package = $Newest.FullName
}
$Package = (Resolve-Path $Package).Path
$TestSaved = Join-Path $Project "Saved_Tests"
$Logs = Join-Path $TestSaved "Logs"
$Shots = Join-Path $TestSaved "Screenshots\Smoke"
New-Item -ItemType Directory -Force $Logs, $Shots | Out-Null
. (Join-Path $PSScriptRoot "..\Package\script_errors.ps1")
$Faults = New-Object Collections.Generic.List[string]
$Missing = @(); $Fatal = @(); $Errors = @(); $Found = @(); $Seconds = 0; $ServerSeconds = 0

if (-not $ServerOnly) {
# The client: the game itself (not the bootstrap exe at the top, which starts it), in Windows\ or WindowsClient\.
$Client = Get-ChildItem $Package -Recurse -File -Include "HeroesOfLiberty.exe", "HeroesOfLibertyClient.exe" -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match '\\Binaries\\Win64\\' -and $_.FullName -notmatch '\\WindowsServer\\' } | Select-Object -First 1
if (-not $Client) { Write-Host "packaged_smoke : FAIL (no game client exe in $Package; -ServerOnly checks its server alone)"; exit 1 }

$GameLog = Join-Path $Logs "packaged_smoke.log"
Remove-Item -ErrorAction SilentlyContinue $GameLog
Get-ChildItem $Shots -Filter "smoke_*.png" -ErrorAction SilentlyContinue | Remove-Item

# One human on Plain, nobody else (the format of -MatchConfig=, Tools\Scenarios\README.md).
$Config = Join-Path $TestSaved "packaged_smoke.json"
'{ "map": "/Game/Maps/Plain.Plain", "startingPeasants": 0, "startingHero": 0, "slots": [ { "state": "Human", "colorIndex": 0 } ] }' | Set-Content -Encoding ascii $Config
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

# The scripts (D484): the game starts on the package's precompiled cache (Tools\Package\precompile_scripts.ps1, run by
# package.ps1), without compiling its .as files, and none fails (the engine's own bind errors aside, script_errors.ps1).
if (-not ($Log | Select-String -SimpleMatch "Using fully precompiled scripts" -Quiet)) {
    $Faults.Add("scripts not precompiled (no 'Using fully precompiled scripts'; Tools\Package\precompile_scripts.ps1)")
}
$ScriptErrors = @(Get-ScriptFileErrors $GameLog)
if ($ScriptErrors.Count) { $Faults.Add("script errors: $($ScriptErrors -join ' ')") }

# The captures, from the packaged game's own Saved folder (beside it, or in the user's local app data).
$Found = @(foreach ($Folder in $Package, (Join-Path $env:LOCALAPPDATA "HeroesOfLiberty")) {
    Get-ChildItem $Folder -Recurse -File -Filter "smoke_*.png" -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $Start } })
$Found | ForEach-Object { Copy-Item $_.FullName $Shots -Force }
}

# The server pass (IA0.5 B3): the AI's brain in the packaged dedicated server, on its precompiled scripts.
$ServerSaid = "server pass SKIPPED (-SkipServer)"
if (-not $SkipServer) {
    $Server = Get-ChildItem $Package -Recurse -File -Filter "HeroesOfLibertyServer.exe" -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match '\\Binaries\\Win64\\' } | Select-Object -First 1
    if (-not $Server) {
        $Faults.Add("server: no HeroesOfLibertyServer.exe in $Package (-SkipServer for a package without a server)")
        $ServerSaid = "server pass FAIL (no server exe)"
    } else {
        $ServerLog = Join-Path $Logs "packaged_smoke_server.log"
        Remove-Item -ErrorAction SilentlyContinue $ServerLog
        # Slot 0 Human without a name, slot 1 AI, the rest closed, 5 starting Peasants (quick_check's Write-AIMatchConfig).
        $ServerConfig = Join-Path $TestSaved "packaged_smoke_server.json"
        $Slots = for ($i = 0; $i -lt 8; $i++) {
            $State = if ($i -eq 0) { "Human" } elseif ($i -eq 1) { "AI" } else { "Closed" }
            "{ `"state`": `"$State`", `"colorIndex`": $i }"
        }
        "{ `"version`": 2, `"map`": `"/Game/Maps/Plain.Plain`", `"startingPeasants`": 5, `"slots`": [ $($Slots -join ', ') ] }" |
            Set-Content -Encoding ascii $ServerConfig
        # A free UDP port in this copy's range of 10 (quick_check's Get-AITestPort; nobody joins).
        $Index = @{ "MyCrusader" = 0; "HeroesOfLiberty-Elrond" = 1; "HeroesOfLiberty-Arwen" = 2; "HeroesOfLiberty-Glorfindel" = 3;
            "HeroesOfLiberty-Haldir" = 4; "HeroesOfLiberty-Galadriel" = 5; "HeroesOfLiberty-Celeborn" = 6 }[(Split-Path -Leaf $Project)]
        if ($null -eq $Index) { $Index = 7 }
        $Port = 7800 + 10 * $Index
        while ($Port -lt 7810 + 10 * $Index) { try { $Probe = New-Object Net.Sockets.UdpClient($Port); $Probe.Close(); break } catch { $Port++ } }
        $ServerArguments = "/Game/Maps/Plain -log -unattended -nosound -Port=$Port -MatchConfig=`"$ServerConfig`" -abslog=`"$ServerLog`""
        $ServerStart = Get-Date
        $ServerProcess = Start-Process -FilePath $Server.FullName -ArgumentList $ServerArguments -PassThru -WindowStyle Hidden
        Write-Host "packaged_smoke : $($Server.FullName) pid $($ServerProcess.Id), port $Port, log $ServerLog"
        # The lines come in the first seconds of the match: read the log until the gather order (no test sequence).
        $Deadline = $ServerStart.AddSeconds($ServerTimeoutSeconds)
        while ((Get-Date) -lt $Deadline -and -not $ServerProcess.HasExited) {
            if ((Test-Path $ServerLog) -and (Select-String -Path $ServerLog -SimpleMatch "Gather order of slot 1: 5 units to source" -Quiet)) { break }
            Start-Sleep -Milliseconds 500
        }
        $ServerExited = $ServerProcess.HasExited
        $ServerSeconds = [math]::Round(((Get-Date) - $ServerStart).TotalSeconds)
        # Stopped: its console closed first (the engine's own exit), then by its id.
        if (-not $ServerExited) {
            $null = $ServerProcess.CloseMainWindow()
            if (-not $ServerProcess.WaitForExit(5000)) { Stop-Process -Id $ServerProcess.Id -Force -ErrorAction SilentlyContinue; $null = $ServerProcess.WaitForExit(5000) }
        }
        $ServerLines = @(Get-Content $ServerLog -Encoding utf8 -ErrorAction SilentlyContinue)
        function Find-ServerLine([string] $Pattern) { @($ServerLines | Select-String -Pattern $Pattern | ForEach-Object { $_.Line.Trim() }) | Select-Object -First 1 }
        $ServerFaults = New-Object Collections.Generic.List[string]
        if (-not $ServerLines.Count) { $ServerFaults.Add("no log written") }
        if ($ServerExited) { $ServerFaults.Add("quit by itself after $ServerSeconds s (exit code $($ServerProcess.ExitCode))") }
        if (-not (Find-ServerLine "listening on port|Listening on port")) { $ServerFaults.Add("not listening (no 'listening on port')") }
        if (-not ($ServerLines | Select-String -SimpleMatch "Using fully precompiled scripts" -Quiet)) { $ServerFaults.Add("scripts not precompiled (no 'Using fully precompiled scripts')") }
        $Awake = Find-ServerLine "AI brain \S+ awake, net mode \w+, thinking slots \[[^\]]*\]"
        if (-not $Awake) { $ServerFaults.Add("no 'AI brain ... awake' line") }
        elseif ($Awake -notmatch "net mode DedicatedServer, thinking slots \[1\]") { $ServerFaults.Add("brain not thinking for slot 1 on the dedicated server ('$($Awake -replace '^.*(AI brain )', '$1')')") }
        if (-not (Find-ServerLine "AI slot 1 step \d+:")) { $ServerFaults.Add("no 'AI slot 1 step'") }
        if (-not (Find-ServerLine "AI slot 1 step \d+: 5 idle -> gather \w+ #\d+")) { $ServerFaults.Add("no '5 idle -> gather'") }
        if (-not (Find-ServerLine "Gather order of slot 1: 5 units to source")) { $ServerFaults.Add("no 'Gather order of slot 1: 5 units to source'") }
        $ServerScriptErrors = @(Get-ScriptFileErrors $ServerLog)
        if ($ServerScriptErrors.Count) { $ServerFaults.Add("script errors: $($ServerScriptErrors -join ' ')") }
        $ServerFatal = @($ServerLines | Select-String -Pattern "Fatal error|Unhandled Exception|Assertion failed|=== Critical error ===" | ForEach-Object { $_.Line.Trim() })
        if ($ServerFatal.Count) { $ServerFaults.Add("$($ServerFatal.Count) fatal lines") }
        $ServerFaults | ForEach-Object { $Faults.Add("server: $_") }
        $ServerSaid = "server pass $(if ($ServerFaults.Count) { 'FAIL' } else { 'ok' }) in $ServerSeconds s, log $ServerLog"
    }
}
$Seconds += $ServerSeconds
$ClientSaid = if ($ServerOnly) { "client SKIPPED (-ServerOnly)" } else { "client log $GameLog" }

if (-not $Faults.Count) {
    Write-Host "packaged_smoke : PASS ($Seconds s, $(if ($ServerOnly) { $ClientSaid } else { "$($Found.Count) captures in $Shots" }); $ServerSaid)"
    exit 0
}
Write-Host "packaged_smoke : FAIL ($Seconds s, $($Faults -join '; '); $ClientSaid; $ServerSaid)"
@($Missing | Select-Object -First 10) + @($Fatal | Select-Object -First 5) + @($Errors | Select-Object -First 5) | ForEach-Object { Write-Host "  $_" }
exit 1
