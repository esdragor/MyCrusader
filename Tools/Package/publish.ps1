# Publishes a version of the game for the launcher (L4): packages it (package.ps1) or takes an archive of it, then
# HOLPublish (Manager\HOLPublish, C#) writes the player folder <archive>\Windows\ into the versions folder served by the
# lobby manager (manifest.json, notes.txt, files\<path>.gz, patches\<from>\<path>.patch from the -Keep newest versions,
# latest.txt last; the versions beyond -Keep removed). A launcher never sees half a version: HOLPublish writes <v>.tmp,
# renames it, then replaces latest.txt, so the manager may run meanwhile (it caches nothing).
# The dedicated server (WindowsServer\) is not touched: the manager reads its game version at startup only.
# Run from Windows PowerShell (never Git Bash: package.ps1 would get /Game/... paths rewritten):
#   powershell -ExecutionPolicy Bypass -File Tools\Package\publish.ps1 [-Root <copy>] [-Archive <copy>\Saved\Packages\<v>]
#       [-Version <name>] [-Notes <file>] [-Keep 3] [-VersionsDir <folder>]
# - no -Archive: package.ps1 -Root <copy> [-Version <name>] first (1.5 to 5 min, more the first time), then its archive;
# - the version published is the one in <archive>\Windows\HeroesOfLiberty\Version.txt (what the game reads); a -Version
#   that differs from it is refused;
# - -Notes: the "what's new" text shown by the launcher (copied as notes.txt; none without it);
# - -VersionsDir: <copy>\Saved\Manager\Versions by default (the manager built by build_manager.ps1 in Saved\Manager\
#   reads Versions\ next to its exe, VersionSettings.VersionsDir).
# HOLPublish is built (Release) into <copy>\Saved\HOLPublish\, with hdiffz.exe / hpatchz.exe beside it. The archive is
# kept (remove it by hand once published: 6 GB).
# Full log: <copy>\Saved\Logs\publish.log. Prints the lines of package.ps1 and HOLPublish, then one line PASS / FAIL;
# exit code 0 / 1 (2: package.ps1 found this copy busy).
param(
    [string] $Root = "",
    [string] $Archive = "",
    [string] $Version = "",
    [string] $Notes = "",
    [int] $Keep = 3,
    [string] $VersionsDir = ""
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$Log = Join-Path $Project "Saved\Logs\publish.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
Set-Content -Path $Log -Value "publish : $(Split-Path -Leaf $Project), started $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -Encoding utf8
function Say([string] $Line) { Write-Host $Line; Add-Content -Path $Log -Value $Line -Encoding utf8 }
function Fail([string] $Why, [int] $Code = 1) { Say "publish : FAIL ($Why; log $Log)"; exit $Code }

if (-not (Test-Path (Join-Path $Project "HeroesOfLiberty.uproject"))) { Fail "no HeroesOfLiberty.uproject in $Project" }
if ($Keep -lt 0) { Fail "-Keep wants a number >= 0, not $Keep" }
if ($Notes) {
    if (-not (Test-Path $Notes -PathType Leaf)) { Fail "notes file not found: $Notes" }
    $Notes = (Resolve-Path $Notes).Path
}
if (-not $VersionsDir) { $VersionsDir = Join-Path $Project "Saved\Manager\Versions" }
$VersionsDir = [IO.Path]::GetFullPath($VersionsDir)
if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) { Fail "dotnet not found" }
$Clock = [Diagnostics.Stopwatch]::StartNew()

# 1. The archive: packaged now (package.ps1, its lines repeated as they come), or the one given.
$PackageMinutes = $null
if (-not $Archive) {
    $PackageArgs = @("-ExecutionPolicy", "Bypass", "-File", (Join-Path $PSScriptRoot "package.ps1"), "-Root", $Project)
    if ($Version) { $PackageArgs += @("-Version", $Version) }
    $PassLine = ""
    & powershell @PackageArgs | ForEach-Object {
        Say "  $_"
        if ($_ -match '^package : PASS \(([^,]+),') { $PassLine = $Matches[1] }
    }
    $PackageCode = $LASTEXITCODE
    $PackageMinutes = [math]::Round($Clock.Elapsed.TotalMinutes, 1)
    if ($PackageCode -ne 0) { Fail "package.ps1 exit $PackageCode after $PackageMinutes min" $(if ($PackageCode -eq 2) { 2 } else { 1 }) }
    if (-not $PassLine) { Fail "package.ps1 exit 0 but no PASS line read" }
    $Archive = Join-Path $Project "Saved\Packages\$PassLine"
}
if (-not (Test-Path $Archive -PathType Container)) { Fail "archive not found: $Archive" }
$Archive = (Resolve-Path $Archive).Path
$Game = Join-Path $Archive "Windows"
if (-not (Test-Path (Join-Path $Game "HeroesOfLiberty.exe"))) { Fail "no Windows\HeroesOfLiberty.exe in $Archive" }

# 2. The version: package.ps1 writes it in <platform>\HeroesOfLiberty\Version.txt, not at the root of Windows\ where
# HOLPublish would look for it without --version: read here and passed.
$VersionFile = Join-Path $Game "HeroesOfLiberty\Version.txt"
if (-not (Test-Path $VersionFile)) { Fail "no Windows\HeroesOfLiberty\Version.txt in $Archive" }
$GameVersion = (Get-Content $VersionFile -Raw -Encoding utf8).Trim()
if (-not $GameVersion) { Fail "$VersionFile is empty" }
if ($Version -and $Version -ne $GameVersion) { Fail "-Version $Version but the archive's Version.txt says $GameVersion" }

# 3. HOLPublish, built in Release (hdiffz.exe and hpatchz.exe copied beside it by its .csproj).
$PublishDir = Join-Path $Project "Saved\HOLPublish"
$PublishExe = Join-Path $PublishDir "HOLPublish.exe"
$Csproj = Join-Path $Project "Manager\HOLPublish\HOLPublish.csproj"
if (-not (Test-Path $Csproj)) { Fail "no $Csproj" }
& dotnet build $Csproj --nologo -c Release -o $PublishDir 2>&1 | Out-File -Append -Encoding utf8 $Log
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $PublishExe)) {
    Select-String -Path $Log -Pattern ": error " | Select-Object -First 3 | ForEach-Object { Write-Host "  $($_.Line.Trim())" }
    Fail "dotnet build of HOLPublish"
}
foreach ($Tool in "hdiffz.exe", "hpatchz.exe") {
    if (-not (Test-Path (Join-Path $PublishDir $Tool))) { Fail "no $Tool beside $PublishExe (Tools\ThirdParty\HDiffPatch\)" }
}

# 4. HOLPublish on the player folder. Its lines are UTF-8 ("compresses" with an accent): the console's encoding set to
# UTF-8 while it runs, then put back (it belongs to the console window, which outlives this script).
$Arguments = @("--game", $Game, "--versions", $VersionsDir, "--version", $GameVersion, "--keep", "$Keep")
if ($Notes) { $Arguments += @("--notes", $Notes) }
Say "HOLPublish $($Arguments -join ' ')"
$PublishClock = [Diagnostics.Stopwatch]::StartNew()
$Encoding = [Console]::OutputEncoding
$Result = ""
try {
    [Console]::OutputEncoding = New-Object Text.UTF8Encoding $false
    & $PublishExe @Arguments 2>&1 | ForEach-Object {
        $Line = "$_"
        Say "  $Line"
        if ($Line -match '^HOLPublish : (PASS|FAIL)') { $Result = $Line }
    }
    $PublishCode = $LASTEXITCODE
}
finally { [Console]::OutputEncoding = $Encoding }
$PublishSeconds = [math]::Round($PublishClock.Elapsed.TotalSeconds)
if ($PublishCode -ne 0) { Fail "HOLPublish exit $PublishCode after $PublishSeconds s$(if ($Result) { ": $Result" })" }

# 5. Checked: latest.txt names the new version, its folder is there.
$Latest = (Get-Content (Join-Path $VersionsDir "latest.txt") -Raw -Encoding utf8 -ErrorAction SilentlyContinue)
if (-not $Latest -or $Latest.Trim() -ne $GameVersion) { Fail "latest.txt says '$Latest', not $GameVersion" }
if (-not (Test-Path (Join-Path $VersionsDir "$GameVersion\manifest.json"))) { Fail "no $GameVersion\manifest.json in $VersionsDir" }
$Kept = @(Get-ChildItem $VersionsDir -Directory | Where-Object { Test-Path (Join-Path $_.FullName "manifest.json") } | ForEach-Object { $_.Name })
$Bytes = (Get-ChildItem $VersionsDir -Recurse -File | Measure-Object Length -Sum).Sum
$Package = if ($null -ne $PackageMinutes) { "package $PackageMinutes min, " } else { "archive given, " }
Say ("publish : PASS ($GameVersion -> $VersionsDir ; ${Package}HOLPublish $PublishSeconds s; versions kept: " +
    "$($Kept -join ', '), $([math]::Round($Bytes / 1MB)) MB)")
exit 0
