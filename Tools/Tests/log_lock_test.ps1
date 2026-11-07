# Test of the two test tools' handling of a log held by another process (2026-11-03: stale PASS of check.ps1 three times,
# "0/0" of quick_check -Parts tests while the engine wrote quick_tests_2.log). No game and no engine are started: a fake
# "engine" (a small exe compiled here) writes the -abslog file as Unreal does: when the file is held it writes <name>_2.log.
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\log_lock_test.ps1
# Prints one PASS / FAIL line per case, then "log_lock_test : PASS" or "FAIL (n)"; exit 0 / 1. ~1 min (the tools wait a few
# seconds on a held log before giving up on it).
#   -Only check | tests | named : one family of cases (a red pass in a few seconds); -NamedOnly <part> : one part of the named ones.
param([string] $Only = "", [string] $NamedOnly = "")
$ErrorActionPreference = "Stop"
$Project =(Resolve-Path "$PSScriptRoot\..\..").Path
$Work = Join-Path ([IO.Path]::GetTempPath()) ("hol_log_lock_test_" + [Guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force $Work | Out-Null
$Fails = 0
function Case([string] $Name, [bool] $Ok, [string] $Detail = "") {
    Write-Host "  $(if ($Ok) { 'PASS' } else { 'FAIL' }) : $Name$(if (-not $Ok -and $Detail) { " ($Detail)" })"
    if (-not $Ok) { $script:Fails++ }
}

# The fake engine: -abslog="<path>" taken from its arguments; writes the file named by FAKE_ENGINE_TEXT into it (nothing
# when the variable is empty) and exits. A held log (IOException) goes to <name>_2<ext>, as the engine's own.
$Source = @'
using System;
using System.IO;
using System.Text;
public static class FakeEngine {
    public static int Main(string[] args) {
        string log = null;
        foreach (string a in args) { if (a.StartsWith("-abslog=")) log = a.Substring(8).Trim('"'); }
        if (log == null) return 2;
        string textFile = Environment.GetEnvironmentVariable("FAKE_ENGINE_TEXT");
        string text = string.IsNullOrEmpty(textFile) ? "" : File.ReadAllText(textFile);
        FileStream stream;
        try { stream = new FileStream(log, FileMode.Create, FileAccess.Write, FileShare.Read); }
        catch (IOException) {
            string other = Path.Combine(Path.GetDirectoryName(log), Path.GetFileNameWithoutExtension(log) + "_2" + Path.GetExtension(log));
            stream = new FileStream(other, FileMode.Create, FileAccess.Write, FileShare.Read);
        }
        using (stream) { byte[] bytes = Encoding.UTF8.GetBytes(text); stream.Write(bytes, 0, bytes.Length); }
        // FAKE_ENGINE_LINGER_MS: stays alive a moment after writing, as a server that listens (the parts wait for "listening on port").
        int linger;
        if (int.TryParse(Environment.GetEnvironmentVariable("FAKE_ENGINE_LINGER_MS"), out linger) && linger > 0) System.Threading.Thread.Sleep(linger);
        return 0;
    }
}
'@
$Engine = Join-Path $Work "engine"
New-Item -ItemType Directory -Force $Engine | Out-Null
# WindowsApplication, not ConsoleApplication: the tools under test start it with Start-Process, and a console exe opens a
# terminal window each time that closes at once (2026-11-05).
Add-Type -TypeDefinition $Source -OutputType WindowsApplication -OutputAssembly (Join-Path $Engine "UnrealEditor.exe")
Copy-Item (Join-Path $Engine "UnrealEditor.exe") (Join-Path $Engine "UnrealEditor-Cmd.exe")

# Holds a file open the way a process still writing it does (nobody can delete or write it, others may read it).
function Hold([string] $Path, [string] $Text) {
    New-Item -ItemType Directory -Force (Split-Path $Path) | Out-Null
    $Stream = New-Object IO.FileStream($Path, [IO.FileMode]::Create, [IO.FileAccess]::Write, [IO.FileShare]::Read)
    $Bytes = [Text.Encoding]::UTF8.GetBytes($Text)
    $Stream.Write($Bytes, 0, $Bytes.Length); $Stream.Flush()
    return $Stream
}

try {
    $Quick = Join-Path $Project "Tools\Tests\quick_check.ps1"
    if (-not $Only -or $Only -eq "lint") {
    # ---- L3-3, the source of quick_check.ps1 (no process started): every log handed to the game (-abslog=`"$Var`") comes from
    # Get-FreshLog (its last assignment before the launch), and no "Logs\quick_*.log" path is used as a read path without going through it
    # (a literal default path, as Get-NetClientLog's for a client that never launched, reads the PASS of an older pass).
    $QuickLines = @(Get-Content $Quick -Encoding UTF8)
    $Bad = @()
    for ($i = 0; $i -lt $QuickLines.Count; $i++) {
        $Line = $QuickLines[$i]
        if ($Line -match '-abslog=`"\$(\w+)`"') {
            $Var = $Matches[1]
            $Last = $null
            for ($j = $i - 1; $j -ge 0; $j--) { if ($QuickLines[$j] -match "^\s*\`$$Var\s*=") { $Last = $QuickLines[$j]; break } }
            if (-not $Last -or $Last -notmatch 'Get-FreshLog') { $Bad += "line $($i + 1): `$$Var is handed to the game without Get-FreshLog" }
        }
        if ($Line -match 'Logs\\quick_[^"]*\.log"' -and $Line -notmatch 'Get-FreshLog|Remove-Item|^\s*#') {
            # A bare path: allowed only as the assignment of a variable that Get-FreshLog re-assigns within the next 40 lines.
            $Name = if ($Line -match '^\s*\$(\w+)\s*=\s*Join-Path') { $Matches[1] } else { $null }
            $Wrapped = $Name -and [bool](@($QuickLines[($i + 1)..([Math]::Min($i + 40,$QuickLines.Count - 1))]) -match "^\s*\`$$Name\s*=\s*Get-FreshLog")
            if (-not $Wrapped) { $Bad += "line $($i + 1): a Logs\quick_*.log path outside Get-FreshLog: $($Line.Trim())" }
        }
    }
    Case "lint: every log of quick_check.ps1 given to the game or read goes through Get-FreshLog ($($Bad.Count) outside)" ($Bad.Count -eq 0) ($Bad -join " / ")
    }
    if (-not $Only -or $Only -eq "check") {
    # ---- check.ps1: a scenario whose previous log (still held) says "ready" must not pass when this pass writes nothing.
    $Root = Join-Path $Work "root"
    New-Item -ItemType Directory -Force (Join-Path $Root "Tools\Scenarios") | Out-Null
    Set-Content -Encoding ascii (Join-Path $Root "Tools\Scenarios\fake.txt") "# fake"
    Set-Content -Encoding ascii (Join-Path $Root "HeroesOfLiberty.uproject") '{ "EngineAssociation": "fake" }'
    $ScenarioLog = Join-Path $Root "Saved_Tests\Logs\scenario_fake.log"
    $Ready = Join-Path $Work "ready.txt"; Set-Content -Encoding ascii $Ready "Scenario fake: ready"
    $Check = Join-Path $Project "Tools\Scenarios\check.ps1"

    $Held = Hold $ScenarioLog "Scenario fake: ready`r`n"
    $env:FAKE_ENGINE_TEXT = ""
    $Output = @(& powershell -ExecutionPolicy Bypass -File $Check fake -Root $Root -EngineDir $Engine -TimeoutSeconds 20 2>&1 | ForEach-Object { "$_" })
    $Exit = $LASTEXITCODE
    $Held.Dispose()
    Case "check.ps1: a held stale log saying ready does not give PASS (exit $Exit)" ($Exit -ne 0 -and -not ($Output -match "check fake : PASS")) (($Output | Select-Object -Last 2) -join " / ")
    Case "check.ps1: says why (the log is held)" ([bool]($Output -match "held|locked|cannot be removed|cannot remove"))

    Remove-Item $ScenarioLog -Force -ErrorAction SilentlyContinue
    $env:FAKE_ENGINE_TEXT = $Ready
    $Output = @(& powershell -ExecutionPolicy Bypass -File $Check fake -Root $Root -EngineDir $Engine -TimeoutSeconds 20 2>&1 | ForEach-Object { "$_" })
    Case "check.ps1: a free log and a ready line written by this pass gives PASS (control)" ($LASTEXITCODE -eq 0 -and [bool]($Output -match "check fake : PASS"))

    # L1-12(c): the scenario reaches "ready" but the game's own audit (hol.Test.Sanity) logged "Sanity: FAIL ...": check.ps1 must FAIL
    # (only preflight.ps1 used to see it). The fake engine writes the lines as the game does; the audit line may come before or after "ready".
    foreach ($Order in "before", "after") {
        $SanityText = Join-Path $Work "sanity_$Order.txt"
        $SanityLine = "LogHOLTest: Sanity: FAIL unit #12 of slot 1 (neutral camp) has the colour of a player"
        Set-Content -Encoding ascii $SanityText $(if ($Order -eq "before") { @("Scenario fake: step 1", $SanityLine, "Scenario fake: ready") } else { @("Scenario fake: ready", $SanityLine) })
        Remove-Item $ScenarioLog -Force -ErrorAction SilentlyContinue
        $env:FAKE_ENGINE_TEXT = $SanityText
        $Output = @(& powershell -ExecutionPolicy Bypass -File $Check fake -Root $Root -EngineDir $Engine -TimeoutSeconds 20 2>&1 | ForEach-Object { "$_" })
        $Exit = $LASTEXITCODE
        Case "check.ps1: ready and a 'Sanity: FAIL' line ($Order the ready line) is a FAIL, not a PASS (exit $Exit)" ($Exit -ne 0 -and -not [bool]($Output -match "check fake : PASS") -and [bool]($Output -match "check fake : FAIL")) (($Output | Select-Object -Last 2) -join " / ")
        Case "check.ps1: says it is the sanity audit and shows the line ($Order)" ([bool]($Output -match "sanity audit") -and [bool]($Output -match "Sanity: FAIL unit #12"))
    }
    $SanityOk = Join-Path $Work "sanity_ok.txt"
    Set-Content -Encoding ascii $SanityOk @("Sanity: OK (600 steps, 12 units followed)", "Scenario fake: ready")
    Remove-Item $ScenarioLog -Force -ErrorAction SilentlyContinue
    $env:FAKE_ENGINE_TEXT = $SanityOk
    $Output = @(& powershell -ExecutionPolicy Bypass -File $Check fake -Root $Root -EngineDir $Engine -TimeoutSeconds 20 2>&1 | ForEach-Object { "$_" })
    Case "check.ps1: ready and 'Sanity: OK' gives PASS (control)" ($LASTEXITCODE -eq 0 -and [bool]($Output -match "check fake : PASS"))

    }
    if (-not $Only -or $Only -eq "tests") {
    # ---- quick_check.ps1 -Parts tests: the log of THIS pass is read, whatever the file's name.
    $QuickLog = Join-Path $Project "Saved_Tests\Logs\quick_tests.log"
    $Fresh = Join-Path $Work "fresh.txt"
    $Lines = @()
    foreach ($N in 1..3) {
        $Lines += "LogAutomationController: Test Started. Name={T$N}"
        $Lines += "LogAutomationController: Test Completed. Result={Success} Name={T$N} Path={Angelscript.UnitTests.T$N}"
    }
    Set-Content -Encoding ascii $Fresh $Lines
    $Stale ="LogAutomationController: Test Started. Name={Old}`r`nLogAutomationController: Test Completed. Result={Success} Name={Old} Path={Angelscript.UnitTests.Old}`r`n"

    $Held = Hold $QuickLog $Stale
    $env:FAKE_ENGINE_TEXT = $Fresh
    $Output = @(& powershell -ExecutionPolicy Bypass -File $Quick -Parts tests -EngineDir $Engine 2>&1 | ForEach-Object { "$_" })
    $Held.Dispose()
    Case "quick_check: with quick_tests.log held, the 3 results of this pass are read, not 0/0 nor the held file's 1" ([bool]($Output -match "tests 3/3 : PASS")) (($Output | Select-String "tests " | Select-Object -First 2) -join " / ")
    Case "quick_check: says the log was held" ([bool]($Output -match "held"))

    # The engine wrote no log at all: a clear failure, not a pass.
    Remove-Item (Join-Path $Project "Saved_Tests\Logs\quick_tests*.log") -Force -ErrorAction SilentlyContinue
    $env:FAKE_ENGINE_TEXT = ""
    $Output = @(& powershell -ExecutionPolicy Bypass -File $Quick -Parts tests -EngineDir $Engine 2>&1 | ForEach-Object { "$_" })
    Case "quick_check: an empty log of this pass is a FAIL (not a pass)" ([bool]($Output -match "tests 0/0 : FAIL"))

    # Control: nothing held.
    Remove-Item (Join-Path $Project "Saved_Tests\Logs\quick_tests*.log") -Force -ErrorAction SilentlyContinue
    $env:FAKE_ENGINE_TEXT = $Fresh
    $Output = @(& powershell -ExecutionPolicy Bypass -File $Quick -Parts tests -EngineDir $Engine 2>&1 | ForEach-Object { "$_" })
    Case "quick_check: nothing held gives tests 3/3 PASS (control)" ([bool]($Output -match "tests 3/3 : PASS"))
    # Coordinator's request (2026-11-07): -Parts tests must not launch the main game (menu/camera sequence): no "sequence" report line, and
    # the fake engine, which writes into whatever -abslog it is given, finds no quick_game.log written by this pass.
    Case "quick_check: -Parts tests prints no 'sequence' line (the main game is not launched)" (-not [bool]($Output -match "sequence \d+/\d+ ?:|sequence [^\r\n]*: (PASS|FAIL)")) (($Output | Select-String "sequence" | Select-Object -First 2) -join " / ")
    Case "quick_check: -Parts tests writes no quick_game.log" (-not (Test-Path (Join-Path $Project "Saved_Tests\Logs\quick_game.log")) -or ((Get-Item (Join-Path $Project "Saved_Tests\Logs\quick_game.log")).LastWriteTime -lt (Get-Date).AddMinutes(-3)))

    }
    if (-not $Only -or $Only -eq "named") {
    # ---- quick_check.ps1, the named logs of the other parts (net, aiserver, aiduel, aimarket, aisiege, aitech, aiwave, netai,
    # netfog*, netspec*): each log of the part held by an earlier process (its stale lines must never be read, nor the
    # engine's <name>_2.log), the part is played with the fake engine, which writes one text into whatever log it is given and
    # stays alive a moment (the parts wait for "listening on port" before they launch their clients). For every log: the pass
    # says it was held, writes its own <name>_<stamp>.log (the fake wrote there, not into <name>_2.log), and the checks that read
    # that text do not fail ("[KO] <check>" absent: the stale file has none of the lines). "net diplomacy" has its own PASS line.
    # The parts read a text that is not a real match's: a check that throws on it (stderr) must not stop this test.
    $ErrorActionPreference = "Continue"
    $env:HOL_LOG_HOLD_TRIES = "2"          # a held log is given up after ~1 s here (10 tries of 0.5 s by default)
    $env:FAKE_ENGINE_LINGER_MS = "1500"
    $Marks = "netfog", "netfogoff", "netfogback", "netfogtrees", "netspec", "netspecteam", "netspecteams", "netai"
    $Text = @("LogHOLNet: listening on port 7777")
    foreach ($Mark in $Marks) { $Text += "Test mark: $Mark"; $Text += "Sources (client): 5 standing of 9" }
    $Text += "Test mark: end"
    $Text += "Sources (server): 9 standing"
    $Text += "Victory: slot 1 defeated"
    $Text += "Starting Peasants of slot 1: 5 in front of its main forum"
    $Text += "Starting Peasants of slot 2: 5 in front of its main forum"
    $Text += "Production: Peasant of slot 0 done"
    $Text += "Production: Peasant of slot 1 done"
    $Text += "Pause: this machine's world paused"
    $Text += "Pause: this machine's world running"
    $Text += "Diplomacy report: mode Diplomacy"
    $Text += "Diplomacy report: mode Diplomacy"
    $Text += "Diplomacy report: slot 0 toward slot 1: stance Ally, relation Ally, vision 0, fires 0, adversary 0"
    $Text += "Diplomacy report: slot 1 toward slot 0: stance Ally, relation Ally, vision 0, fires 0, adversary 0"
    $Text += "Diplomacy report: slot 0 toward slot 1: stance Enemy, relation Enemy, vision 0, fires 1, adversary 1"
    $Text += "Diplomacy report: slot 1 toward slot 0: stance Enemy, relation Enemy, vision 0, fires 1, adversary 1"
    $Text += "AI brain X awake, net mode DedicatedServer, thinking slots [0, 1]"
    $Text += "AI slot 0 step 5: level Hard"
    $Text += "AI slot 1 step 5: level Normal"
    $Text += "AI slot 2 step 5: level Easy"
    $Text += "AI slot 3 step 5: level Evolving"
    $Text += "AI slot 0 step 5: attack launched with 12 soldiers"
    $Named = Join-Path $Work "named.txt"
    Set-Content -Encoding ascii $Named $Text
    $env:FAKE_ENGINE_TEXT = $Named
    $LogDir = Join-Path $Project "Saved_Tests\Logs"

    # Part; the logs it writes (without .log); the "[KO]" lines that must be absent (a check that reads the fresh text); the
    # line that must be present.
    $NamedCases = @(
        @{ Part = "net"; Logs = "quick_net_server", "quick_net_client1", "quick_net_client2"; NoKo = "server: one Peasant born per slot", "client 1 : its world paused", "client 2 : its world paused"; Has = "net diplomacy : PASS" },
        @{ Part = "aiserver"; Logs = , "quick_aiserver"; NoKo = , "5 starting Peasants for slot 1" },
        @{ Part = "aiduel"; Logs = "quick_aiduel_a", "quick_aiduel_b"; NoKo = , "thinking slots" },
        @{ Part = "aimarket"; Logs = , "quick_aimarket"; NoKo = , "slot 0 is Hard, slot 1 Normal" },
        @{ Part = "aisiege"; Logs = , "quick_aisiege"; NoKo = , "slot 0 is Hard, slot 1 Normal" },
        @{ Part = "aitech"; Logs = , "quick_aitech"; NoKo = , "slot 0 is Hard, 1 Normal, 2 Easy, 3 Evolving" },
        @{ Part = "aiwave"; Logs = , "quick_aiwave"; NoKo = , "Hard's wave is launched" },
        @{ Part = "netai"; Logs = "quick_netai_server", "quick_netai_client1", "quick_netai_client2"; NoKo = "server : 5 starting Peasants for slot 2", "client 1 : sequence done", "client 2 : sequence done" },
        @{ Part = "netfog"; Logs = "quick_netfog_server", "quick_netfog_client1", "quick_netfog_client2"; NoKo = "client 1 : sources", "client 1 : sequence done", "client 2 : sequence done" },
        @{ Part = "netfogback"; Logs = "quick_netfogback_server", "quick_netfogback_client1", "quick_netfogback_client2", "quick_netfogback_back"; NoKo = "client 1 : sources", "client 2 back : sequence done" },
        @{ Part = "netfogview"; Logs = "quick_netfogview_server", "quick_netfogview_client1", "quick_netfogview_client2"; NoKo = "server listening", "client 1 : sequence done", "client 2 : sequence done" },
        @{ Part = "netspec"; Logs ="quick_netspec_server", "quick_netspec_client1", "quick_netspec_client2"; NoKo = "server : slot 1 defeated", "client 1 : sequence done", "client 2 : sequence done" },
        # L3-3: the variants that share the code of netfog / netspec (their own log names).
        @{ Part = "netfogoff"; Logs = "quick_netfogoff_server", "quick_netfogoff_client1", "quick_netfogoff_client2"; NoKo = "client 1 : sequence done", "client 2 : sequence done" },
        @{ Part = "netfogtrees"; Logs = "quick_netfogtrees_server", "quick_netfogtrees_client1", "quick_netfogtrees_client2"; NoKo = "client 1 : sequence done", "client 2 : sequence done" },
        @{ Part = "netspecteam"; Logs = "quick_netspecteam_server", "quick_netspecteam_client1", "quick_netspecteam_client2"; NoKo = "client 1 : sequence done", "client 2 : sequence done" },
        @{ Part = "netspecteams"; Logs = "quick_netspecteams_server", "quick_netspecteams_client1", "quick_netspecteams_client2"; NoKo = "client 1 : sequence done", "client 2 : sequence done" }
    )
    foreach ($NamedCase in $NamedCases) {
        if ($NamedOnly -and $NamedCase.Part -ne $NamedOnly) { continue }
        $Part = $NamedCase.Part
        Get-ChildItem $LogDir -Filter "quick_$Part*.log" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
        $Holds = @($NamedCase.Logs | ForEach-Object { Hold (Join-Path $LogDir "$_.log") "STALE LINE`r`n" })
        $Output = @(& powershell -ExecutionPolicy Bypass -File $Quick -Parts $Part -EngineDir $Engine 2>&1 | ForEach-Object { "$_" })
        $Holds | ForEach-Object { $_.Dispose() }
        foreach ($Log in $NamedCase.Logs) {
            $Own = @(Get-ChildItem $LogDir -Filter "${Log}_*.log" -ErrorAction SilentlyContinue | Where-Object { $_.Name -match "^${Log}_\d{8}_\d{6}_\d{3}\.log$" })
            Case "${Part}: $Log held -> says so" ([bool]($Output -match "$([regex]::Escape("$Log.log")) is held by another process")) (($Output | Select-String "held" | Select-Object -First 1) -join "")
            Case "${Part}: $Log held -> the engine wrote the pass's own log, not ${Log}_2.log" ($Own.Count -eq 1 -and $Own[0].Length -gt 0 -and -not (Test-Path (Join-Path $LogDir "${Log}_2.log"))) "own: $($Own.Name -join ','), _2: $(Test-Path (Join-Path $LogDir "${Log}_2.log"))"
            Case "${Part}: $Log held -> the held file is left alone" ((Get-Content (Join-Path $LogDir "$Log.log") -Raw) -match "^STALE LINE")
        }
        foreach ($NoKo in $NamedCase.NoKo) {
            Case "${Part}: the check '$NoKo' reads the pass's text, not the stale one" (-not [bool]($Output -match "\[KO\] .*$([regex]::Escape($NoKo))")) (($Output | Select-String "\[KO\] .*$([regex]::Escape($NoKo))" | Select-Object -First 1) -join "")
        }
        if ($NamedCase.Has) { Case "${Part}: '$($NamedCase.Has)' is printed" ([bool]($Output -match [regex]::Escape($NamedCase.Has))) (($Output | Select-String "net " | Select-Object -First 3) -join " / ") }
        Get-ChildItem $LogDir -Filter "quick_$Part*.log" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
    }
    # Control (nothing held, a part with a server and its clients): no log said held, the checks read the pass's text.
    if (-not $NamedOnly -or $NamedOnly -eq "netspec") {
    $Output =@(& powershell -ExecutionPolicy Bypass -File $Quick -Parts netspec -EngineDir $Engine 2>&1 | ForEach-Object { "$_" })
    Case "netspec: nothing held -> no 'held' line (control)" (-not [bool]($Output -match "held by another process"))
    Case "netspec: nothing held -> the checks read the text (control)" (-not [bool]($Output -match "\[KO\] (server : slot 1 defeated|client \d : sequence done)")) (($Output | Select-String "\[KO\]" | Select-Object -First 2) -join " / ")
    Get-ChildItem $LogDir -Filter "quick_netspec*.log" -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
    }
    }
}
finally {
    Remove-Item Env:\FAKE_ENGINE_TEXT, Env:\FAKE_ENGINE_LINGER_MS, Env:\HOL_LOG_HOLD_TRIES -ErrorAction SilentlyContinue
    Remove-Item $Work -Recurse -Force -ErrorAction SilentlyContinue
}
Write-Host "log_lock_test : $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
if ($Fails -eq 0) { exit 0 } else { exit 1 }
