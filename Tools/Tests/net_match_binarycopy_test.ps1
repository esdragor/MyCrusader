# End-to-end test of net_match.ps1 -BinaryCopy (the S5 clients run from a copy of the binaries, BinaryCopy.ps1), with the REAL staged
# server and the REAL editor as headless clients (needs the staged server: Saved\StagedBuilds\WindowsServer; ~2-4 min).
#   powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\MyCrusader\Tools\Tests\net_match_binarycopy_test.ps1 -Root <built worktree> [-NoCopy]
# Proves : (1) a client editor runs from a hol_bincopy_ shadow project, (2) its shadow DLL is locked but the worktree's REAL DLL can be
# opened for writing (build.ps1 can compile during the S5 measures), (3) the client log lands in the worktree's Saved_Tests\Logs and the
# client was welcomed by the server, (4) the shadow directory is gone afterwards and the junction targets are untouched.
# -NoCopy runs net_match without -BinaryCopy (the old behaviour): checks 1 and 2 must then be RED (the red-first proof).
# Port 7945 and tag "bc" keep it apart from other sessions' servers. Numbers of the S5 are worthless (machine shared).
param([Parameter(Mandatory = $true)] [string] $Root, [switch] $NoCopy, [int] $WaitSeconds = 360)
$Root = (Resolve-Path $Root).Path
$Tool = Join-Path $PSScriptRoot 'net_match.ps1'
$Out = Join-Path $env:TEMP ('net_match_bc_test_' + [guid]::NewGuid().ToString('N').Substring(0, 6) + '.out')
$Fails = 0
function Check([bool]$Ok, [string]$What, [string]$Detail = '') { if ($Ok) { Write-Host "PASS $What" } else { $script:Fails++; Write-Host "FAIL $What  $Detail" } }
function CanWrite([string]$Path) { try { $S = [IO.File]::Open($Path, 'Open', 'ReadWrite', 'None'); $S.Dispose(); $true } catch { $false } }
$RealDll = Join-Path $Root 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
$ContentBefore = (Get-ChildItem (Join-Path $Root 'Content') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$ScriptBefore = (Get-ChildItem (Join-Path $Root 'Script') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$ExtraArg = if ($NoCopy) { '' } else { '-BinaryCopy' }
$RootLeaf = [regex]::Escape((Split-Path $Root -Leaf))

$P = Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$Tool`" -Clients 1 -Units 200 -Seconds 10 -Headless -Port 7945 -Tag bc $ExtraArg" -PassThru -WindowStyle Hidden -RedirectStandardOutput $Out
$null = $P.Handle
$Start = Get-Date; $Shadow = $null; $ShadowLocked = $false; $RealFree = $null; $EditorCmd = ''; $SawEditor = $false; $RealFreeInPlace = $null
while (-not $P.HasExited -and ((Get-Date) - $Start).TotalSeconds -lt $WaitSeconds) {
    Start-Sleep -Seconds 2
    $Ed = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -match 'hol_bincopy_' -and $_.CommandLine -match $RootLeaf -and $_.CommandLine -match '127\.0\.0\.1' })
    if ($Ed.Count -and -not $Shadow) { $EditorCmd = $Ed[0].CommandLine; if ($EditorCmd -match '"([^"]*hol_bincopy_[^"]*)\\HeroesOfLiberty\.uproject"') { $Shadow = $Matches[1] } }
    if ($Shadow -and -not $ShadowLocked) {
        $Dll = Join-Path $Shadow 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll'
        if ((Test-Path $Dll) -and -not (CanWrite $Dll)) { $ShadowLocked = $true; $RealFree = CanWrite $RealDll }
    }
    if ($NoCopy -and -not $SawEditor) {
        $In = @(Get-CimInstance Win32_Process -Filter "Name = 'UnrealEditor.exe'" -ErrorAction SilentlyContinue | Where-Object { $_.CommandLine -like "*$Root\HeroesOfLiberty.uproject*" -and $_.CommandLine -match ':7945|7945' })
        if ($In.Count) { $SawEditor = $true; Start-Sleep -Seconds 15; $RealFreeInPlace = CanWrite $RealDll }
    }
}
if (-not $P.HasExited) { Stop-Process -Id $P.Id -Force }
else { $P.WaitForExit() }
# Whatever is left of this test's own processes (port 7945): the client editors and the server.
Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -match '7945' -and $_.CommandLine -match $RootLeaf -and $_.Name -match 'Unreal|HeroesOfLiberty' } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
$Text = if (Test-Path $Out) { Get-Content $Out -Raw } else { '' }
Write-Host $Text.TrimEnd()
Check ($null -ne $Shadow) 'a client editor ran from a hol_bincopy_ shadow project' $EditorCmd
Check $ShadowLocked 'the shadow DLL was locked by the client editor (it is the one loaded)'
$Free = if ($NoCopy) { $RealFreeInPlace } else { $RealFree }
Check ($Free -eq $true) 'the worktree DLL was FREE for writing while the client ran (build.ps1 can compile)' "free=$Free"
$ClientLog = Join-Path $Root 'Saved_Tests\Logs\net_clientbc1.log'
Check ((Test-Path $ClientLog) -and (Select-String -Path $ClientLog -Pattern 'Welcomed by server' -Quiet)) 'the client log is in the worktree Saved_Tests\Logs and the client was welcomed by the server' $ClientLog
Check ($Shadow -and -not (Test-Path $Shadow)) 'the shadow directory was removed'
$ContentAfter = (Get-ChildItem (Join-Path $Root 'Content') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
$ScriptAfter = (Get-ChildItem (Join-Path $Root 'Script') -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
Check ($ContentAfter -eq $ContentBefore -and $ScriptAfter -eq $ScriptBefore) "junction targets untouched (Content $ContentBefore -> $ContentAfter, Script $ScriptBefore -> $ScriptAfter)"
Remove-Item $Out -ErrorAction SilentlyContinue
Write-Host ("net_match_binarycopy_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $Fails
