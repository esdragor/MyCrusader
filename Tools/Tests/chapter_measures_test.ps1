# End-to-end test of chapter_measures.ps1 running its S1-S4 / S7 editors from a binary COPY (BinaryCopy.ps1), with the REAL editor
# (starts Unreal twice: S1 for 5 s, two rounds; 3-6 min on a loaded machine).
#   powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\chapter_measures_test.ps1 -Root <built worktree> [-InPlace]
# Proves : (1) the measure editor runs from a hol_bincopy_ shadow project, (2) the shadow DLL is locked but the worktree's REAL DLL can be
# opened for writing (build.ps1 can compile during the measures), (3) the S1 report was read (a "ms per step" line, from the worktree's
# Saved_Tests\Logs), (4) the shadow directory is gone afterwards and the junction targets are untouched.
# -InPlace passes -InPlace to chapter_measures (the old behaviour): checks 1 and 3 must then be RED (that is the red-first proof).
# The measure runs with -IgnoreBusy (other sessions' editors are open): the NUMBERS are worthless, only the plumbing is tested.
# Exit code = red checks.
param([Parameter(Mandatory = $true)] [string] $Root, [switch] $InPlace, [int] $WaitSeconds = 420)
$Root = (Resolve-Path $Root).Path
$Tool = Join-Path $PSScriptRoot 'chapter_measures.ps1'
$Out = Join-Path $env:TEMP ('chapter_measures_test_' + [guid]::NewGuid().ToString('N').Substring(0, 6) + '.out')
$Fails = 0
function Check([bool]$Ok, [string]$What, [string]$Detail = '') { if ($Ok) { Write-Host "PASS $What" } else { $script:Fails++; Write-Host "FAIL $What  $Detail" } }
function CanWrite([string]$Path) { try { $S = [IO.File]::Open($Path, 'Open', 'ReadWrite', 'None'); $S.Dispose(); $true } catch { $false } }
$RealDll = Join-Path $Root 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
$ContentBefore = (Get-ChildItem (Join-Path $Root 'Content') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$ScriptBefore = (Get-ChildItem (Join-Path $Root 'Script') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$Tag = 'plumbing_' + [guid]::NewGuid().ToString('N').Substring(0, 4)
$ExtraArg = if ($InPlace) { '-InPlace' } else { '' }

$P = Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$Tool`" -Tests S1:5 -NoS5 -IgnoreBusy -Tag $Tag $ExtraArg" -PassThru -WindowStyle Hidden -RedirectStandardOutput $Out
$null = $P.Handle
$Start = Get-Date; $Shadow = $null; $ShadowLocked = $false; $RealFree = $null; $EditorCmd = ''; $SawEditor = $false; $RealFreeInPlace = $null
$RootLeaf = [regex]::Escape((Split-Path $Root -Leaf))
while (-not $P.HasExited -and ((Get-Date) - $Start).TotalSeconds -lt $WaitSeconds) {
    Start-Sleep -Seconds 2
    $Ed = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -match 'hol_bincopy_' -and $_.CommandLine -match $RootLeaf })
    if ($Ed.Count -and -not $Shadow) { $EditorCmd = $Ed[0].CommandLine; if ($EditorCmd -match '"([^"]*hol_bincopy_[^"]*)\\HeroesOfLiberty\.uproject"') { $Shadow = $Matches[1] } }
    if ($Shadow -and -not $ShadowLocked) {
        $Dll = Join-Path $Shadow 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
        if ((Test-Path $Dll) -and -not (CanWrite $Dll)) { $ShadowLocked = $true; $RealFree = CanWrite $RealDll }
    }
    if ($InPlace -and -not $SawEditor) {
        $In = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -like "*$Root\HeroesOfLiberty.uproject*" })
        if ($In.Count) { $SawEditor = $true; Start-Sleep -Seconds 20; $RealFreeInPlace = CanWrite $RealDll }   # give the engine time to load the DLL
    }
}
if (-not $P.HasExited) { Stop-Process -Id $P.Id -Force; Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" | Where-Object { $_.CommandLine -match 'hol_bincopy_' -and $_.CommandLine -match $RootLeaf } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force } }
else { $P.WaitForExit() }
$Text = if (Test-Path $Out) { Get-Content $Out -Raw } else { '' }
Write-Host $Text.TrimEnd()
Check ($null -ne $Shadow) 'the measure editor ran from a hol_bincopy_ shadow project' $EditorCmd
Check $ShadowLocked 'the shadow DLL was locked by the editor (it is the one loaded)'
$Free = if ($InPlace) { $RealFreeInPlace } else { $RealFree }
Check ($Free -eq $true) 'the worktree DLL was FREE for writing while the measure editor ran (build.ps1 can compile)' "free=$Free"
Check ($Text -match 'S1 run 1 : [\d.,]+ ms per step') 'the S1 report was read (ms per step line)' 'see the output above'
Check ($Shadow -and -not (Test-Path $Shadow)) 'the shadow directory was removed'
$ContentAfter = (Get-ChildItem (Join-Path $Root 'Content') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$ScriptAfter = (Get-ChildItem (Join-Path $Root 'Script') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
Check ($ContentAfter -eq $ContentBefore -and $ScriptAfter -eq $ScriptBefore) "junction targets untouched (Content $ContentBefore -> $ContentAfter, Script $ScriptBefore -> $ScriptAfter)"
Check (Test-Path (Join-Path $Root "Saved_Tests\Logs\chapter_${Tag}_s1_1.log")) 'the S1 log landed in the worktree Saved_Tests\Logs'
Remove-Item $Out -ErrorAction SilentlyContinue
Write-Host ("chapter_measures_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $Fails
