# End-to-end test of repeat_scenario.ps1 / BinaryCopy.ps1 with the REAL editor (starts Unreal once: ~1-3 min, longer behind a build).
#   powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\repeat_scenario_test.ps1 -Root <built worktree> [-Scenario sanity_ok]
# Proves : (1) during the pass the editor runs from the shadow copy (its command line names hol_bincopy_), (2) the shadow DLL is locked
# but the worktree's REAL DLL can be opened for writing (so build.ps1 can replace it), (3) the pass result line comes back,
# (4) the shadow directory is gone afterwards and the junction targets (Content, Script...) are untouched. Exit code = red checks.
param([Parameter(Mandatory = $true)] [string] $Root, [string] $Scenario = 'sanity_ok', [int] $WaitSeconds = 300)
$Root = (Resolve-Path $Root).Path
$Tool = Join-Path $PSScriptRoot 'repeat_scenario.ps1'
$Mine = 'hol_bincopy_' + [regex]::Escape((Split-Path $Root -Leaf)) + '_'   # only THIS worktree's shadows (other sessions' editors also run from hol_bincopy_ directories)
$Out = Join-Path $env:TEMP ('repeat_scenario_test_' + [guid]::NewGuid().ToString('N').Substring(0, 6) + '.out')
$Fails = 0
function Check([bool]$Ok, [string]$What, [string]$Detail = '') { if ($Ok) { Write-Host "PASS $What" } else { $script:Fails++; Write-Host "FAIL $What  $Detail" } }
function CanWrite([string]$Path) { try { $S = [IO.File]::Open($Path, 'Open', 'ReadWrite', 'None'); $S.Dispose(); $true } catch { $false } }
$RealDll = Join-Path $Root 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
$ContentBefore = (Get-ChildItem (Join-Path $Root 'Content') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$ScriptBefore = (Get-ChildItem (Join-Path $Root 'Script') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count

$P = Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$Tool`" $Scenario -Runs 1 -Root `"$Root`"" -PassThru -WindowStyle Hidden -RedirectStandardOutput $Out
$null = $P.Handle
$Start = Get-Date; $Shadow = $null; $ShadowLocked = $false; $RealFree = $null; $EditorCmd = ''
while (-not $P.HasExited -and ((Get-Date) - $Start).TotalSeconds -lt $WaitSeconds) {
    Start-Sleep -Seconds 2
    $Ed = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -match $Mine })
    if ($Ed.Count -and -not $Shadow) { $EditorCmd = $Ed[0].CommandLine; if ($EditorCmd -match '"([^"]*hol_bincopy_[^"]*)\\HeroesOfLiberty\.uproject"') { $Shadow = $Matches[1] } }
    if ($Shadow -and -not $ShadowLocked) {
        $Dll = Join-Path $Shadow 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
        if ((Test-Path $Dll) -and -not (CanWrite $Dll)) { $ShadowLocked = $true; $RealFree = CanWrite $RealDll }   # the engine has loaded the SHADOW dll: is the real one free?
    }
}
if (-not $P.HasExited) { Stop-Process -Id $P.Id -Force; Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" | Where-Object { $_.CommandLine -match $Mine } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force } }
else { $P.WaitForExit() }
$Text = if (Test-Path $Out) { Get-Content $Out -Raw } else { '' }
Write-Host $Text.TrimEnd()
Check ($null -ne $Shadow) 'the editor ran from a hol_bincopy_ shadow project' $EditorCmd
Check $ShadowLocked 'the shadow DLL was locked by the editor (it is the one loaded)'
Check ($RealFree -eq $true) 'the worktree DLL was FREE for writing while the editor ran (build.ps1 can compile)' "RealFree=$RealFree"
Check ($Text -match 'repeat \S+ : PASS 1/1') 'the repeat tool reports PASS 1/1' 'see the output above'
Check ($Shadow -and -not (Test-Path $Shadow)) 'the shadow directory was removed'
$ContentAfter = (Get-ChildItem (Join-Path $Root 'Content') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$ScriptAfter = (Get-ChildItem (Join-Path $Root 'Script') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
Check ($ContentAfter -eq $ContentBefore -and $ScriptAfter -eq $ScriptBefore) "junction targets untouched (Content $ContentBefore -> $ContentAfter, Script $ScriptBefore -> $ScriptAfter)"
Check (Test-Path (Join-Path $Root "Saved_Tests\Logs\scenario_${Scenario}_run1.log")) 'the pass log was kept as scenario_<name>_run1.log'
Remove-Item $Out -ErrorAction SilentlyContinue
Write-Host ("repeat_scenario_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $Fails
