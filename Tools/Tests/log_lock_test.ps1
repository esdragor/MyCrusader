# Test of the two test tools' handling of a log held by another process (2026-11-03: stale PASS of check.ps1 three times,
# "0/0" of quick_check -Parts tests while the engine wrote quick_tests_2.log). No game and no engine are started: a fake
# "engine" (a small exe compiled here) writes the -abslog file as Unreal does: when the file is held it writes <name>_2.log.
#   powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Tests\log_lock_test.ps1
# Prints one PASS / FAIL line per case, then "log_lock_test : PASS" or "FAIL (n)"; exit 0 / 1. ~1 min (the tools wait a few
# seconds on a held log before giving up on it).
$ErrorActionPreference = "Stop"
$Project = (Resolve-Path "$PSScriptRoot\..\..").Path
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
        return 0;
    }
}
'@
$Engine = Join-Path $Work "engine"
New-Item -ItemType Directory -Force $Engine | Out-Null
Add-Type -TypeDefinition $Source -OutputType ConsoleApplication -OutputAssembly (Join-Path $Engine "UnrealEditor.exe")
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

    # ---- quick_check.ps1 -Parts tests: the log of THIS pass is read, whatever the file's name.
    $QuickLog = Join-Path $Project "Saved_Tests\Logs\quick_tests.log"
    $Fresh = Join-Path $Work "fresh.txt"
    $Lines = @()
    foreach ($N in 1..3) {
        $Lines += "LogAutomationController: Test Started. Name={T$N}"
        $Lines += "LogAutomationController: Test Completed. Result={Success} Name={T$N} Path={Angelscript.UnitTests.T$N}"
    }
    Set-Content -Encoding ascii $Fresh $Lines
    $Quick = Join-Path $Project "Tools\Tests\quick_check.ps1"
    $Stale = "LogAutomationController: Test Started. Name={Old}`r`nLogAutomationController: Test Completed. Result={Success} Name={Old} Path={Angelscript.UnitTests.Old}`r`n"

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
}
finally {
    Remove-Item Env:\FAKE_ENGINE_TEXT -ErrorAction SilentlyContinue
    Remove-Item $Work -Recurse -Force -ErrorAction SilentlyContinue
}
Write-Host "log_lock_test : $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" })"
if ($Fails -eq 0) { exit 0 } else { exit 1 }
