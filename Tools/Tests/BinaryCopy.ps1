# Shared helper (dot-source it): run the editor / the game from a COPY of a worktree's binaries so that Tools\build.ps1 stays free.
#
# Why (2026-11-05, Auto_Journey_Checklist L6-6): a repeated measure (`neutrals`, 12 passes) kept the project DLL
# (<worktree>\Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll) open in a running editor, and the build of that worktree could only
# compile at 14:00, once the measure had finished. The engine loads the project's modules from <ProjectDir>\Binaries\Win64, so a
# SHADOW project directory is made in %TEMP%: its own copy of Binaries\Win64 (DLLs, .modules, .target; no .pdb/.lib/.exp/.exe, which
# only weigh 6 GB and are not loaded by the editor), the .uproject, and directory JUNCTIONS to everything else the game reads
# (Content, Config, Script, Tools, Build, DerivedDataCache, Saved_Tests : logs still land in the worktree's Saved_Tests).
# The editor started on the shadow .uproject locks only the copied DLLs. build.ps1 does not count it as "this copy" either (its
# test is the worktree's .uproject path), so the build compiles at once, and the next measure pass takes a new copy.
#
#   . C:\Users\baudo\Documents\MyCrusader\Tools\Tests\BinaryCopy.ps1
#   $Shadow = New-BinaryCopy -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen     # returns the shadow directory
#   ... run Tools\Scenarios\check.ps1 <name> -Root $Shadow ...
#   Remove-BinaryCopy $Shadow
# DANGER handled here: Remove-Item -Recurse can follow a junction and delete the REAL folder's content : the junctions are removed
# first with [IO.Directory]::Delete(path, $false), which never enters them.

$script:BinaryCopyLinks = @('Content', 'Config', 'Script', 'Tools', 'Build', 'DerivedDataCache', 'Saved_Tests')

function New-BinaryCopy {
    param([Parameter(Mandatory = $true)] [string] $Root, [string] $Parent = $env:TEMP)
    $Root = (Resolve-Path $Root).Path
    $Source = Join-Path $Root 'Binaries\Win64'
    if (-not (Get-ChildItem $Source -Filter '*.dll' -ErrorAction SilentlyContinue)) { throw "no DLL in $Source : build the worktree first (Tools\build.ps1 -Root $Root)" }
    $Shadow = Join-Path $Parent ('hol_bincopy_' + (Split-Path $Root -Leaf) + '_' + [guid]::NewGuid().ToString('N').Substring(0, 6))
    New-Item -ItemType Directory -Force (Join-Path $Shadow 'Binaries\Win64') | Out-Null
    # robocopy exit codes 0-7 are success
    & robocopy $Source (Join-Path $Shadow 'Binaries\Win64') /E /NFL /NDL /NJH /NJS /NP /XF *.pdb *.lib *.exp *.exe | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "robocopy failed ($LASTEXITCODE) copying $Source" }
    Copy-Item (Join-Path $Root 'HeroesOfLiberty.uproject') (Join-Path $Shadow 'HeroesOfLiberty.uproject')
    foreach ($Name in $script:BinaryCopyLinks) {
        $Target = Join-Path $Root $Name
        if ($Name -eq 'Saved_Tests') { New-Item -ItemType Directory -Force $Target | Out-Null }
        if (-not (Test-Path $Target)) { continue }
        & cmd /c mklink /J "$(Join-Path $Shadow $Name)" "$Target" | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "mklink /J failed for $Name" }
    }
    return $Shadow
}

function Remove-BinaryCopy {
    param([Parameter(Mandatory = $true)] [string] $Shadow)
    if (-not $Shadow -or -not (Test-Path $Shadow) -or (Split-Path $Shadow -Leaf) -notlike 'hol_bincopy_*') { return }   # never delete anything else
    foreach ($Item in Get-ChildItem $Shadow -Force -Directory) {
        if ($Item.Attributes -band [IO.FileAttributes]::ReparsePoint) { [IO.Directory]::Delete($Item.FullName, $false) }
    }
    # The editor killed by check.ps1 takes a few seconds to release its DLLs : retry for 60 s, then say what is left.
    for ($Try = 0; $Try -lt 30 -and (Test-Path $Shadow); $Try++) {
        Remove-Item $Shadow -Recurse -Force -ErrorAction SilentlyContinue
        if (Test-Path $Shadow) { Start-Sleep -Seconds 2 }
    }
    if (Test-Path $Shadow) { Write-Host "BinaryCopy: could not remove $Shadow (a process still holds a DLL ?); delete it by hand, it is only a copy" }
}
