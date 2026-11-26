# Test of the editor kit of an artist without Visual Studio (D927, AR7-d), in temporary folders only (fake engine, fake repos).
# - publish_editor_binaries.ps1 copies UnrealEditor.modules and the DLLs it names into ArtistBinaries\Win64 with the source commit;
#   it refuses DLLs older than the last C++ commit, and a tree whose C++ is not committed;
# - open_editor.ps1 finds the engine, refuses DLLs made for another engine (BuildId), copies the kit into Binaries\Win64,
#   keeps the DLLs of a development machine (Intermediate\Build) unless -Force, warns when C++ changed after the kit, launches;
# - register_engine.ps1 writes the engine name -> folder (registry path given by the test), refuses a folder without UnrealEditor.exe.
# Exit code 0 = green, 1 = red.
param([string]$Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path)
$ErrorActionPreference = 'Continue'
$Publish = Join-Path $Root 'Tools\Art\publish_editor_binaries.ps1'
$Open = Join-Path $Root 'Tools\Art\open_editor.ps1'
$Register = Join-Path $Root 'Tools\Art\register_engine.ps1'
$fails = 0
function Fail([string]$Text) { Write-Output "FAIL $Text"; $script:fails++ }
function Ok([string]$Text) { Write-Output "ok $Text" }
function Run-Script([string]$Path, [string[]]$Arguments) {
    if (-not (Test-Path $Path)) { return [pscustomobject]@{ Code = 99; Out = "missing script $Path" } }
    $out = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Path @Arguments 2>&1 | ForEach-Object { "$_" })
    return [pscustomobject]@{ Code = $LASTEXITCODE; Out = ($out -join "`n") }
}
function Check([string]$Name, [bool]$Cond, [string]$Detail = '') { if ($Cond) { Ok $Name } else { Fail "$Name $Detail" } }
function Git { & git.exe @args 2>&1 | Out-Null }
function Write-Modules([string]$Dir, [string]$BuildId) {
    New-Item -ItemType Directory -Force $Dir | Out-Null
    Set-Content -Encoding ascii (Join-Path $Dir 'UnrealEditor.modules') "{`n`t`"BuildId`": `"$BuildId`",`n`t`"Modules`":`n`t{`n`t`t`"HeroesOfLiberty`": `"UnrealEditor-HeroesOfLiberty.dll`",`n`t`t`"HeroesOfLibertyUI`": `"UnrealEditor-HeroesOfLibertyUI.dll`"`n`t}`n}"
}

$Temp = Join-Path ([IO.Path]::GetTempPath()) ('hol_artist_bin_' + [guid]::NewGuid().ToString('N'))
$RegPath = 'HKCU:\Software\HOLTest\artist_bin_' + [guid]::NewGuid().ToString('N')
try {
    # Fake engine.
    $Engine = Join-Path $Temp 'Engine58'
    $EngineBin = Join-Path $Engine 'Engine\Binaries\Win64'
    Write-Modules $EngineBin 'build-A'
    Set-Content (Join-Path $EngineBin 'UnrealEditor.exe') 'fake'

    # Developer repo: committed C++, then a build (DLLs newer than the commit).
    $Dev = Join-Path $Temp 'dev'
    New-Item -ItemType Directory -Force (Join-Path $Dev 'Source\HeroesOfLiberty'), (Join-Path $Dev 'Source\HeroesOfLibertyUI') | Out-Null
    Git -C $Dev init -q --template=
    Git -C $Dev config user.email t@t; Git -C $Dev config user.name t; Git -C $Dev config core.autocrlf false
    Set-Content (Join-Path $Dev '.gitignore') @('Binaries/', 'Intermediate/')
    Set-Content (Join-Path $Dev 'HeroesOfLiberty.uproject') '{ "EngineAssociation": "UE Test HOL" }'
    Set-Content (Join-Path $Dev 'Source\HeroesOfLiberty\A.cpp') 'int A;'
    Set-Content (Join-Path $Dev 'Source\HeroesOfLibertyUI\U.cpp') 'int U;'
    $env:GIT_AUTHOR_DATE = $env:GIT_COMMITTER_DATE = (Get-Date).AddDays(-2).ToString('yyyy-MM-ddTHH:mm:ss')
    Git -C $Dev add -A; Git -C $Dev commit -q -m code
    Remove-Item Env:GIT_AUTHOR_DATE, Env:GIT_COMMITTER_DATE
    $DevBin = Join-Path $Dev 'Binaries\Win64'
    Write-Modules $DevBin 'build-A'
    Set-Content (Join-Path $DevBin 'UnrealEditor-HeroesOfLiberty.dll') 'dll-main-v1'
    Set-Content (Join-Path $DevBin 'UnrealEditor-HeroesOfLibertyUI.dll') 'dll-ui-v1'
    Set-Content (Join-Path $DevBin 'UnrealEditor-HeroesOfLiberty.pdb') 'pdb'
    Set-Content (Join-Path $DevBin 'HeroesOfLibertyEditor.target') '{ "TargetName": "HeroesOfLibertyEditor" }'
    Set-Content (Join-Path $DevBin 'HeroesOfLiberty.target') '{ "TargetName": "HeroesOfLiberty" }'
    $future = (Get-Date).AddMinutes(5)
    Get-ChildItem $DevBin | ForEach-Object { $_.LastWriteTime = $future }
    $devSha = (& git.exe -C $Dev rev-parse HEAD).Trim()

    # 1. Publish copies the modules file, the two DLLs it names, and the source commit.
    $r = Run-Script $Publish @('-From', $Dev, '-Root', $Dev)
    $kit = Join-Path $Dev 'ArtistBinaries\Win64'
    Check 'publish exit 0' ($r.Code -eq 0) "(code $($r.Code): $($r.Out))"
    Check 'publish copies the main DLL' ((Test-Path (Join-Path $kit 'UnrealEditor-HeroesOfLiberty.dll')) -and ((Get-Content (Join-Path $kit 'UnrealEditor-HeroesOfLiberty.dll')) -eq 'dll-main-v1'))
    Check 'publish copies the UI DLL' (Test-Path (Join-Path $kit 'UnrealEditor-HeroesOfLibertyUI.dll'))
    Check 'publish copies UnrealEditor.modules' (Test-Path (Join-Path $kit 'UnrealEditor.modules'))
    Check 'publish leaves the pdb out' (-not (Test-Path (Join-Path $kit 'UnrealEditor-HeroesOfLiberty.pdb')))
    # The editor of a code project asks to build when <Project>Editor.target is missing ("Missing target file", seen 2026-11-26).
    Check 'publish copies the editor target' (Test-Path (Join-Path $kit 'HeroesOfLibertyEditor.target'))
    Check 'publish leaves the game target out' (-not (Test-Path (Join-Path $kit 'HeroesOfLiberty.target')))
    $stamp = if (Test-Path (Join-Path $kit 'source_commit.txt')) { @(Get-Content (Join-Path $kit 'source_commit.txt'))[0] } else { '' }
    Check 'publish writes the source commit' ($stamp -eq $devSha) "(got '$stamp', want '$devSha')"
    Git -C $Dev add -A; Git -C $Dev commit -q -m kit

    # 2. Refused: DLLs older than the last C++ commit.
    Set-Content (Join-Path $Dev 'Source\HeroesOfLiberty\A.cpp') 'int A2;'
    Git -C $Dev add -A; Git -C $Dev commit -q -m code2
    Get-ChildItem $DevBin | ForEach-Object { $_.LastWriteTime = (Get-Date).AddDays(-1) }
    $r = Run-Script $Publish @('-From', $Dev, '-Root', $Dev, '-DryRun')
    Check 'publish refuses DLLs older than the C++' (($r.Code -eq 1) -and ($r.Out -match 'FAIL')) "(code $($r.Code): $($r.Out))"
    # 2b. Accepted: UBT relinks only the changed module; the UI DLL stays older than the last C++ commit but newer than its own module.
    (Get-Item (Join-Path $DevBin 'UnrealEditor-HeroesOfLiberty.dll')).LastWriteTime = $future
    $r = Run-Script $Publish @('-From', $Dev, '-Root', $Dev, '-DryRun')
    Check 'publish accepts an untouched module whose DLL was not relinked' ($r.Code -eq 0) "(code $($r.Code): $($r.Out))"
    Get-ChildItem $DevBin | ForEach-Object { $_.LastWriteTime = $future }

    # 3. Refused: C++ not committed.
    Set-Content (Join-Path $Dev 'Source\HeroesOfLiberty\A.cpp') 'int A3;'
    $r = Run-Script $Publish @('-From', $Dev, '-Root', $Dev, '-DryRun')
    Check 'publish refuses uncommitted C++' (($r.Code -eq 1) -and ($r.Out -match 'FAIL')) "(code $($r.Code): $($r.Out))"
    Git -C $Dev checkout -q -- Source
    $r = Run-Script $Publish @('-From', $Dev, '-Root', $Dev)
    Git -C $Dev add -A; Git -C $Dev commit -q -m kit2

    # Artist machine: a clone, no Binaries, no Intermediate.
    $Art = Join-Path $Temp 'artist'
    & git.exe clone -q --template= $Dev $Art 2>&1 | Out-Null

    # 4. Open: copies the kit and launches (dry run).
    $r = Run-Script $Open @('-Root', $Art, '-EngineRoot', $Engine, '-DryRun')
    Check 'open exit 0' ($r.Code -eq 0) "(code $($r.Code): $($r.Out))"
    Check 'open copies the DLL into Binaries\Win64' ((Get-Content (Join-Path $Art 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll') -ErrorAction SilentlyContinue) -eq 'dll-main-v1')
    Check 'open copies UnrealEditor.modules' (Test-Path (Join-Path $Art 'Binaries\Win64\UnrealEditor.modules'))
    Check 'open copies the editor target' (Test-Path (Join-Path $Art 'Binaries\Win64\HeroesOfLibertyEditor.target'))
    Check 'open names the launch' ($r.Out -match 'LAUNCH .*UnrealEditor\.exe.*HeroesOfLiberty\.uproject') "($($r.Out))"
    Check 'open does not warn on an up-to-date kit' ($r.Out -notmatch 'WARN') "($($r.Out))"

    # 5. Open refuses DLLs made for another engine.
    Write-Modules $EngineBin 'build-B'
    $r = Run-Script $Open @('-Root', $Art, '-EngineRoot', $Engine, '-DryRun')
    Check 'open refuses another engine BuildId' (($r.Code -eq 1) -and ($r.Out -match 'FAIL') -and ($r.Out -notmatch 'LAUNCH')) "(code $($r.Code): $($r.Out))"
    Write-Modules $EngineBin 'build-A'

    # 6. Development machine (Intermediate\Build): local DLLs kept unless -Force.
    New-Item -ItemType Directory -Force (Join-Path $Art 'Intermediate\Build\Win64') | Out-Null
    Set-Content (Join-Path $Art 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll') 'dll-local'
    $r = Run-Script $Open @('-Root', $Art, '-EngineRoot', $Engine, '-DryRun')
    Check 'open keeps local DLLs on a dev machine' ((Get-Content (Join-Path $Art 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll')) -eq 'dll-local') "($($r.Out))"
    $r = Run-Script $Open @('-Root', $Art, '-EngineRoot', $Engine, '-DryRun', '-Force')
    Check 'open -Force copies anyway' ((Get-Content (Join-Path $Art 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll')) -eq 'dll-main-v1') "($($r.Out))"
    Remove-Item -Recurse -Force (Join-Path $Art 'Intermediate')
    # 6b. The editor itself runs UBT -Mode=QueryTargets, which writes Intermediate\Build\BuildRules on an artist machine too
    # (seen 2026-11-26): that is not a development machine, the kit is still copied.
    New-Item -ItemType Directory -Force (Join-Path $Art 'Intermediate\Build\BuildRules') | Out-Null
    Set-Content (Join-Path $Art 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll') 'dll-old-kit'
    $r = Run-Script $Open @('-Root', $Art, '-EngineRoot', $Engine, '-DryRun')
    Check 'open still copies after the editor wrote Intermediate\Build\BuildRules' ((Get-Content (Join-Path $Art 'Binaries\Win64\UnrealEditor-HeroesOfLiberty.dll')) -eq 'dll-main-v1') "($($r.Out))"
    Remove-Item -Recurse -Force (Join-Path $Art 'Intermediate')

    # 7. C++ changed after the kit: warning, still opens.
    Git -C $Art config user.email t@t; Git -C $Art config user.name t
    Set-Content (Join-Path $Art 'Source\HeroesOfLiberty\A.cpp') 'int A4;'
    Git -C $Art add -A; Git -C $Art commit -q -m code4
    $r = Run-Script $Open @('-Root', $Art, '-EngineRoot', $Engine, '-DryRun')
    Check 'open warns when C++ is newer than the kit' (($r.Code -eq 0) -and ($r.Out -match 'WARN') -and ($r.Out -match 'LAUNCH')) "(code $($r.Code): $($r.Out))"

    # 8. Open without engine: clear refusal.
    $r = Run-Script $Open @('-Root', $Art, '-EngineRoot', (Join-Path $Temp 'nowhere'), '-DryRun')
    Check 'open refuses a missing engine' (($r.Code -eq 1) -and ($r.Out -match 'register_engine')) "(code $($r.Code): $($r.Out))"

    # 9. register_engine.
    $r = Run-Script $Register @('-EnginePath', $Engine, '-Name', 'UE Test HOL', '-RegistryPath', $RegPath)
    $val = (Get-ItemProperty -Path $RegPath -Name 'UE Test HOL' -ErrorAction SilentlyContinue).'UE Test HOL'
    Check 'register writes the engine folder' (($r.Code -eq 0) -and $val -and ((Resolve-Path $val).Path -eq (Resolve-Path $Engine).Path)) "(code $($r.Code), value '$val': $($r.Out))"
    $r = Run-Script $Register @('-EnginePath', (Join-Path $Temp 'dev'), '-Name', 'UE Test HOL 2', '-RegistryPath', $RegPath)
    Check 'register refuses a folder without UnrealEditor.exe' ($r.Code -eq 1) "(code $($r.Code): $($r.Out))"

    # 10. open finds the engine through that registry entry.
    $r = Run-Script $Open @('-Root', $Art, '-RegistryPath', $RegPath, '-DryRun')
    Check 'open finds the engine by EngineAssociation in the registry' (($r.Code -eq 0) -and ($r.Out -match 'LAUNCH')) "(code $($r.Code): $($r.Out))"
} finally {
    Remove-Item -Recurse -Force $Temp -ErrorAction SilentlyContinue
    Remove-Item -Recurse -Force $RegPath -ErrorAction SilentlyContinue
}

if ($fails) { Write-Output "artist_binaries_test RED : $fails failure(s)"; exit 1 }
Write-Output 'artist_binaries_test GREEN'
exit 0
