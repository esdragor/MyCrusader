# Edits the launcher's home page served by the lobby manager (launcher 5; Manager\PROTOCOL.md, "Accueil du lanceur"):
# announcements, community links and key art in <VersionsDir>\home\, read by the manager at each request (no restart
# needed; every file is written beside then moved over the old one, so a launcher never reads half a file).
# The work is done by "HOLPublish home" (Manager\HOLPublish\HomeCommand.cs, C#, tested), built in Release into
# <copy>\Saved\HOLPublish\ as publish.ps1 does; this script only passes the options.
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Manager\home.ps1 <command> [-VersionsDir <folder>] [-Root <copy>] ...
#   list
#   add -Id <id> -From <folder> [-Link <https://...>] [-Date <2026-11-24T18:00:00Z>]
#       <folder> holds title.txt and body.txt (French), title.en.txt, body.en.txt, title.ja.txt, body.ja.txt: ALL
#       mandatory and not empty (user's rule: published in every language of the launcher or not at all; FAIL and
#       nothing written otherwise). A title is one line. Id: letters, digits, '.', '_', '-'. The same -Id again
#       replaces that announcement (its date kept unless -Date). Default date: now. The launcher shows the 20 newest.
#   remove -Id <id>
#   link -Kind discord|youtube|website|other -Url <https://...>   (one link per kind; the same kind again replaces it)
#   unlink -Kind <kind>
#   background -Image <file.jpg|.png>   (4 Mo at most; replaces the previous one)   |   background -Remove
# Example:
#   powershell -ExecutionPolicy Bypass -File Tools\Manager\home.ps1 add -Id playtest-3 -From D:\Annonces\playtest-3 `
#       -Link https://discord.gg/xxxx
# -VersionsDir: <copy>\Saved\Manager\Versions by default (the manager's, VersionSettings.VersionsDir).
# Full log of the build: <copy>\Saved\Logs\home.log. Prints HOLPublish's lines, then one line PASS / FAIL; exit code 0 / 1.
param(
    [Parameter(Position = 0)] [string] $Command = "",
    [string] $Root = "",
    [string] $VersionsDir = "",
    [string] $Id = "",
    [string] $From = "",
    [string] $Link = "",
    [string] $Date = "",
    [string] $Kind = "",
    [string] $Url = "",
    [string] $Image = "",
    [switch] $Remove
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$Log = Join-Path $Project "Saved\Logs\home.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
function Fail([string] $Why) { Write-Host "home : FAIL ($Why)"; exit 1 }

if ($Command -notin @("list", "add", "remove", "link", "unlink", "background")) {
    Fail "command list, add, remove, link, unlink or background expected, not '$Command' (usage at the top of $PSCommandPath)"
}
if (-not $VersionsDir) { $VersionsDir = Join-Path $Project "Saved\Manager\Versions" }
$VersionsDir = [IO.Path]::GetFullPath($VersionsDir)
if (-not (Test-Path $VersionsDir -PathType Container)) { Fail "versions folder not found: $VersionsDir (-VersionsDir)" }
if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) { Fail "dotnet not found" }

# HOLPublish, built in Release (the same exe as publish.ps1's).
$PublishDir = Join-Path $Project "Saved\HOLPublish"
$PublishExe = Join-Path $PublishDir "HOLPublish.exe"
$Csproj = Join-Path $Project "Manager\HOLPublish\HOLPublish.csproj"
if (-not (Test-Path $Csproj)) { Fail "no $Csproj" }
"home $Command $(Get-Date -Format s)" | Set-Content -Encoding utf8 $Log
& dotnet build $Csproj --nologo -c Release -o $PublishDir 2>&1 | Out-File -Append -Encoding utf8 $Log
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $PublishExe)) {
    Select-String -Path $Log -Pattern ": error " | Select-Object -First 3 | ForEach-Object { Write-Host "  $($_.Line.Trim())" }
    Fail "dotnet build of HOLPublish, see $Log"
}

# Only the options given: an empty string would reach HOLPublish as a value.
$Arguments = @("home", $Command, "--versions", $VersionsDir)
if ($Id) { $Arguments += @("--id", $Id) }
if ($From) { $Arguments += @("--from", [IO.Path]::GetFullPath($From)) }
if ($Link) { $Arguments += @("--link", $Link) }
if ($Date) { $Arguments += @("--date", $Date) }
if ($Kind) { $Arguments += @("--kind", $Kind) }
if ($Url) { $Arguments += @("--url", $Url) }
if ($Image) { $Arguments += @("--image", [IO.Path]::GetFullPath($Image)) }
if ($Remove) { $Arguments += "--remove" }

# HOLPublish writes UTF-8 (Japanese titles in "list"): the console's encoding set to UTF-8 while it runs, then put back.
$Encoding = [Console]::OutputEncoding
$Result = ""
try {
    [Console]::OutputEncoding = New-Object Text.UTF8Encoding $false
    & $PublishExe @Arguments 2>&1 | ForEach-Object {
        $Line = "$_"
        Add-Content -Path $Log -Value $Line -Encoding utf8
        if ($Line -match '^HOLPublish : (PASS|FAIL) ?(.*)$') { $Result = $Matches[2] } else { Write-Host "  $Line" }
    }
    $Code = $LASTEXITCODE
}
finally { [Console]::OutputEncoding = $Encoding }
if ($Code -ne 0) { Fail $(if ($Result) { $Result -replace '^\((.*)\)$', '$1' } else { "HOLPublish exit $Code" }) }
Write-Host "home : PASS ($Result; $VersionsDir\home)"
exit 0
