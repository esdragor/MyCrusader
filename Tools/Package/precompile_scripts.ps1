# The precompiled script cache of a package (D484, AngelscriptInProgress.md AS2): each target of the archive (the game
# in Windows\, the server in WindowsServer\) is started once with -as-generate-precompiled-data. It compiles the .as
# files staged in <platform>\HeroesOfLiberty\Script\, writes Script\PrecompiledScript.Cache there and quits. The packaged
# game and server then start on that cache ("Using fully precompiled scripts", checked by packaged_smoke.ps1) instead of
# compiling the scripts at every start. No script translated to C++ (static JIT, D484): its output is removed.
# The cache only checks the build configuration, not the exe: it is made again by every package.ps1.
# Run by package.ps1 after UAT; alone from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Package\precompile_scripts.ps1 -PackageDir <copy>\Saved\Packages\<version> [-Root <copy>]
# A target passes when its cache is written and no .as file failed (script_errors.ps1), whatever its exit code.
# Logs: <copy>\Saved_Tests\Logs\precompile_<platform>.log. Prints one line per target and one line PASS / FAIL;
# exit code 0 / 1.
param(
    [Parameter(Mandatory = $true)] [string] $PackageDir,
    [string] $Root = "",
    [int] $TimeoutSeconds = 300
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$PackageDir = (Resolve-Path $PackageDir).Path
$Logs = Join-Path $Project "Saved_Tests\Logs"
New-Item -ItemType Directory -Force $Logs | Out-Null

. (Join-Path $PSScriptRoot "script_errors.ps1")
$Targets = [ordered]@{ "Windows" = "HeroesOfLiberty.exe"; "WindowsServer" = "HeroesOfLibertyServer.exe" }
$Faults = New-Object Collections.Generic.List[string]
$Done = New-Object Collections.Generic.List[string]
foreach ($Platform in $Targets.Keys) {
    $Game = Join-Path $PackageDir "$Platform\HeroesOfLiberty"
    $Exe = Join-Path $Game "Binaries\Win64\$($Targets[$Platform])"
    $Script = Join-Path $Game "Script"
    if (-not (Test-Path $Exe)) { $Faults.Add("${Platform}: no $Exe"); continue }
    if (-not (Test-Path (Join-Path $Script "Binds.Cache"))) { $Faults.Add("${Platform}: no Script\Binds.Cache staged"); continue }
    Get-ChildItem $Script -Filter "PrecompiledScript*.Cache" -ErrorAction SilentlyContinue | Remove-Item -Force
    # What the run leaves in the package and must not be published: the game's Saved folder (logs, config) when the
    # package had none, and the static JIT's C++ (AS_JITTED_CODE beside the game folder).
    $Saved = Join-Path $Game "Saved"
    $HadSaved = Test-Path $Saved
    $Jitted = Join-Path $PackageDir "$Platform\AS_JITTED_CODE"

    $Log = Join-Path $Logs "precompile_$Platform.log"
    Remove-Item $Log -ErrorAction SilentlyContinue
    $Arguments = "-as-generate-precompiled-data -as-exit-on-error -nullrhi -nosound -unattended -nosplash -abslog=`"$Log`""
    $Start = Get-Date
    $Process = Start-Process -FilePath $Exe -ArgumentList $Arguments -PassThru -WindowStyle Hidden
    $null = $Process.Handle
    $TimedOut = -not $Process.WaitForExit($TimeoutSeconds * 1000)
    if ($TimedOut) { Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue }
    $Seconds = [math]::Round(((Get-Date) - $Start).TotalSeconds)

    if (-not $HadSaved) { Remove-Item -Recurse -Force $Saved -ErrorAction SilentlyContinue }
    Remove-Item -Recurse -Force $Jitted -ErrorAction SilentlyContinue

    $Cache = Join-Path $Script "PrecompiledScript.Cache"
    $Errors = @(Get-ScriptFileErrors $Log)
    if ($TimedOut) { $Faults.Add("${Platform}: did not quit within $TimeoutSeconds s (killed)") }
    elseif ($Errors.Count) { $Faults.Add("${Platform}: script errors: $($Errors -join ' ')") }
    elseif (-not (Test-Path $Cache) -or (Get-Item $Cache).LastWriteTime -lt $Start) { $Faults.Add("${Platform}: no Script\PrecompiledScript.Cache written (exit $($Process.ExitCode), log $Log)") }
    else {
        # Not the exit code: the run quits while the engine starts, and the PCG plugin's shutdown then fails an ensure
        # ("Attempted to unregister a data class that was not registered"), exit 1, the cache already written.
        $Line = "${Platform}: PrecompiledScript.Cache $([math]::Round((Get-Item $Cache).Length / 1KB, 1)) KB in $Seconds s (exit $($Process.ExitCode))"
        $Done.Add($Line)
        Write-Host "  $Line"
    }
}

if (-not $Faults.Count) { Write-Host "precompile_scripts : PASS ($($Done.Count) targets)"; exit 0 }
Write-Host "precompile_scripts : FAIL ($($Faults -join '; '))"
exit 1
