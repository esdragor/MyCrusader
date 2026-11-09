# Test of repeat_scenario.ps1's VALID passes (2026-11-07, Auto_Journey_File.md: "log of an earlier pass is held by another process"
# made 7 passes out of 8 invalid, each one a copy of the previous pass's log; -Names 'a,b' reached the script as one string).
# No game and no engine : a fake "engine" (small exe, as in log_lock_test.ps1) writes the -abslog file. The REAL tools are copied
# into a fake worktree (check.ps1, EditorSlot.ps1, BinaryCopy.ps1, repeat_scenario.ps1) and run end to end.
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\repeat_scenario_validity_test.ps1
# Cases : (1) the scenario log is held by another process for 12 s when the tool starts : the passes must wait for it and be VALID
# (a pass is never a stale copy, never counted as a failure of the scenario) ; (2) -Names 'a,b' runs both scenarios ;
# (3) a log that stays held is reported INVALID (not PASS, not FAIL of the scenario) and no stale log is kept as run<k>.log.
# Prints one PASS / FAIL line per case, then "repeat_scenario_validity_test : PASS" or "FAIL (n)"; exit = number of red cases.
$ErrorActionPreference = "Stop"
$Project = (Resolve-Path "$PSScriptRoot\..\..").Path
$Work = Join-Path ([IO.Path]::GetTempPath()) ("hol_repeat_validity_" + [Guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force $Work | Out-Null
$Fails = 0
function Case([string] $Name, [bool] $Ok, [string] $Detail = "") {
    Write-Host "  $(if ($Ok) { 'PASS' } else { 'FAIL' }) : $Name$(if (-not $Ok -and $Detail) { " ($Detail)" })"
    if (-not $Ok) { $script:Fails++ }
}

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
        string pattern = Environment.GetEnvironmentVariable("FAKE_ENGINE_PATTERN");   // e.g. "PFP" : launch k writes the good text (P) or the bad one (F)
        if (!string.IsNullOrEmpty(pattern)) {
            string counter = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "count.txt");
            int n = File.Exists(counter) ? int.Parse(File.ReadAllText(counter).Trim()) : 0;
            File.WriteAllText(counter, (n + 1).ToString());
            if (pattern[n % pattern.Length] == 'F') textFile = Environment.GetEnvironmentVariable("FAKE_ENGINE_BAD");
        }
        string text = string.IsNullOrEmpty(textFile) ? "" : File.ReadAllText(textFile);
        FileStream stream;
        try { stream = new FileStream(log, FileMode.Create, FileAccess.Write, FileShare.Read); }
        catch (IOException) {
            string other = Path.Combine(Path.GetDirectoryName(log), Path.GetFileNameWithoutExtension(log) + "_2" + Path.GetExtension(log));
            stream = new FileStream(other, FileMode.Create, FileAccess.Write, FileShare.Read);
        }
        using (stream) { byte[] bytes = Encoding.UTF8.GetBytes(text); stream.Write(bytes, 0, bytes.Length); }
        return 0;
    }
}
'@
$Engine = Join-Path $Work "engine"
New-Item -ItemType Directory -Force $Engine | Out-Null
Add-Type -TypeDefinition $Source -OutputType WindowsApplication -OutputAssembly (Join-Path $Engine "UnrealEditor.exe")

# The fake worktree : the real tools, a fake DLL (BinaryCopy needs one), two fake scenarios, a project file.
$Root = Join-Path $Work "root"
foreach ($D in "Tools\Scenarios", "Tools\Tests", "Binaries\Win64", "Saved_Tests\Logs") { New-Item -ItemType Directory -Force (Join-Path $Root $D) | Out-Null }
foreach ($F in "Tools\Scenarios\check.ps1", "Tools\Scenarios\EditorSlot.ps1", "Tools\Tests\TestSettings.ps1", "Tools\Tests\BinaryCopy.ps1", "Tools\Tests\repeat_scenario.ps1") { Copy-Item (Join-Path $Project $F) (Join-Path $Root $F) }
Set-Content -Encoding ascii (Join-Path $Root "Binaries\Win64\UnrealEditor-Fake.dll") "dll"
Set-Content -Encoding ascii (Join-Path $Root "HeroesOfLiberty.uproject") '{ "EngineAssociation": "fake" }'
foreach ($N in "a", "b") { Set-Content -Encoding ascii (Join-Path $Root "Tools\Scenarios\$N.txt") "# fake" }
$Ready = Join-Path $Work "ready.txt"
Set-Content -Encoding ascii $Ready @("Scenario a: ready", "Scenario b: ready")
$env:FAKE_ENGINE_TEXT = $Ready
$Tool = Join-Path $Root "Tools\Tests\repeat_scenario.ps1"
$Logs = Join-Path $Root "Saved_Tests\Logs"

# A process that holds <log> open (nobody can delete it) for <Seconds> s ; returns once the file is held.
function Start-Holder([string] $Path, [int] $Seconds) {
    $Command = "`$s = New-Object IO.FileStream('$Path', [IO.FileMode]::Create, [IO.FileAccess]::Write, [IO.FileShare]::Read); Start-Sleep -Seconds $Seconds; `$s.Dispose()"
    $P = Start-Process powershell -ArgumentList "-NoProfile", "-Command", $Command -PassThru -WindowStyle Hidden
    for ($I = 0; $I -lt 50 -and -not (Test-Path $Path); $I++) { Start-Sleep -Milliseconds 200 }
    Start-Sleep -Milliseconds 500
    return $P
}
function Run-Tool([string[]] $ToolArgs) {
    $ErrorActionPreference = "Continue"   # a native tool writing on stderr (a bad parameter, a red case) must not stop the test
    $Out = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $Tool @ToolArgs -Root $Root -EngineDir $Engine -TimeoutSeconds 30 2>&1 | ForEach-Object { "$_" })
    return [pscustomobject]@{ Exit = $LASTEXITCODE; Lines = $Out; Text = ($Out -join "`n") }
}

try {
    # ---- (1) the scenario log is held for 12 s when the tool starts : every pass must be valid.
    $Holder = Start-Holder (Join-Path $Logs "scenario_a.log") 12
    $R = Run-Tool @("a", "-Runs", "2")
    if (-not $Holder.HasExited) { $Holder.WaitForExit() }
    Case "held log at start : both passes PASS (exit $($R.Exit))" ($R.Exit -eq 0 -and $R.Text -match "repeat a : PASS 2/2" -and $R.Text -match "verdict OK") (($R.Lines | Select-Object -Last 4) -join " / ")
    Case "held log at start : no pass reported as held / invalid" (-not [bool]($R.Text -match "held by another process|INVALID"))
    $Run1 = Join-Path $Logs "scenario_a_run1.log"
    Case "held log at start : run1.log is the log of that pass (ready line)" ((Test-Path $Run1) -and ((Get-Content $Run1 -Raw) -match "Scenario a: ready"))

    # ---- (2) -Names 'a,b' : both scenarios, one summary line each.
    Remove-Item (Join-Path $Logs "scenario_*.log") -Force -ErrorAction SilentlyContinue
    $R = Run-Tool @("-Names", "a,b", "-Runs", "1")
    Case "-Names 'a,b' : repeat a and repeat b both PASS 1/1 (exit $($R.Exit))" ($R.Exit -eq 0 -and $R.Text -match "repeat a : PASS 1/1" -and $R.Text -match "repeat b : PASS 1/1") (($R.Lines | Select-Object -Last 4) -join " / ")

    # ---- (3) a log held for the whole run (60 s) : INVALID, not a PASS and not a FAIL of the scenario ; no stale log kept as run1.
    Remove-Item (Join-Path $Logs "scenario_*.log") -Force -ErrorAction SilentlyContinue
    Set-Content -Encoding ascii (Join-Path $Logs "scenario_a_run1.log") "STALE"
    $Holder = Start-Holder (Join-Path $Logs "scenario_a.log") 90
    $R = Run-Tool @("a", "-Runs", "1", "-MaxWaitSeconds", "10", "-Retries", "0")
    Case "log held all along : INVALID, exit non-zero, never PASS (exit $($R.Exit))" ($R.Exit -ne 0 -and $R.Text -cmatch "pass 1/1 : INVALID" -and -not [bool]($R.Text -match "verdict OK")) (($R.Lines | Select-Object -Last 3) -join " / ")
    Case "log held all along : no run1.log is left for the invalid pass (neither the old STALE one nor a copy of the held log)" (-not (Test-Path (Join-Path $Logs "scenario_a_run1.log")))
    Stop-Process -Id $Holder.Id -Force -ErrorAction SilentlyContinue

    # ---- (4) the summary always reads "PASS k/N" with k = passes that PASSED (2026-11-08, L9-7 : it used to read "FAIL k/N" with k = PASS count,
    #      which agents read as "k failures"). Launches go good / bad / good : 2 of 3. The verdict is a separate word : "verdict RED" (exit 1).
    Start-Sleep -Seconds 2
    Remove-Item (Join-Path $Logs "scenario_*.log") -Force -ErrorAction SilentlyContinue
    $Bad = Join-Path $Work "bad.txt"
    Set-Content -Encoding ascii $Bad @("LogHOLTest: Error fake failure")
    Remove-Item (Join-Path $Engine "count.txt") -Force -ErrorAction SilentlyContinue
    $env:FAKE_ENGINE_PATTERN = "PFP"; $env:FAKE_ENGINE_BAD = $Bad
    $R = Run-Tool @("a", "-Runs", "3")
    Remove-Item Env:FAKE_ENGINE_PATTERN, Env:FAKE_ENGINE_BAD -ErrorAction SilentlyContinue
    Case "one failed pass of 3 : summary reads 'PASS 2/3' (exit $($R.Exit))" ($R.Exit -eq 1 -and $R.Text -match "repeat a : PASS 2/3") (($R.Lines | Select-Object -Last 4) -join " / ")
    Case "one failed pass of 3 : 'verdict RED', never 'FAIL 2/3'" ($R.Text -match "verdict RED" -and -not [bool]($R.Text -match "FAIL 2/3"))
} finally {
    Get-CimInstance Win32_Process -Filter "Name = 'powershell.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -match "scenario_a.log" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    Remove-Item Env:FAKE_ENGINE_TEXT -ErrorAction SilentlyContinue
    Start-Sleep -Seconds 1
    Remove-Item $Work -Recurse -Force -ErrorAction SilentlyContinue
}
Write-Host ("repeat_scenario_validity_test : " + $(if ($Fails -eq 0) { "PASS" } else { "FAIL ($Fails)" }))
exit $Fails
