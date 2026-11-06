# Checks the sound table of Script/Audio/HOLAudioTable.as against the Kenney packs on disk (AudioInProgress.md A2):
# every `Add(Cues, ..., "<pack>", "<file>,<file>", ...)` line names a pack folder that exists and files that exist in it
# (<file>.ogg, anywhere under the pack), and every pack is in Docs/assets_audio.csv. Prints one line per problem, then
# "check_audio_table: PASS" or "check_audio_table: FAIL (n)". Exit code 0 = pass. Run: powershell -File <this file> [-Root <Kenney folder>]
# With -Assets (A2b) it also checks that each file was imported: Content\Audio\Kenney\<pack>\SW_<file>.uasset exists.
param(
    [switch]$Assets,
    [string]$Content = (Join-Path $PSScriptRoot "..\..\Content\Audio\Kenney"),
    [string]$Root ="C:\Users\baudo\Documents\MyCrusader_Audio\Kenney",
    [string]$Table = (Join-Path $PSScriptRoot "..\..\Script\Audio\HOLAudioTable.as"),
    [string]$Registry = (Join-Path $PSScriptRoot "..\..\Docs\assets_audio.csv")
)

$Problems = 0
$Silent = 0
function Problem([string]$Text) { Write-Host "  FAIL : $Text"; $script:Problems++ }

if (-not (Test-Path $Table)) { Problem "table not found: $Table"; Write-Host "check_audio_table: FAIL ($Problems)"; exit 1 }
if (-not (Test-Path $Root)) { Problem "sound folder not found: $Root"; Write-Host "check_audio_table: FAIL ($Problems)"; exit 1 }

$Registered = if (Test-Path $Registry) { Get-Content $Registry -Raw } else { "" }
$Lines = Get-Content $Table | Where-Object { $_ -match '^\s*Add\(Cues,' }
$Cues = 0
$Files = 0
$Index = @{}
foreach ($Pack in (Get-ChildItem $Root -Directory)) {
    $Names = @{}
    Get-ChildItem $Pack.FullName -Recurse -Filter *.ogg | ForEach-Object { $Names[$_.BaseName] = $true }
    $Index[$Pack.Name] = $Names
}

foreach ($Line in $Lines) {
    if ($Line -notmatch '"(?<name>[A-Za-z]+\.[A-Za-z]+)",\s*"(?<pack>[a-z-]*)",\s*"(?<stems>[A-Za-z0-9_,]*)"') { Problem "cannot read the line: $Line"; continue }
    $Cues++
    $CueName = $Matches['name']
    $Pack = $Matches['pack']
    $StemList = $Matches['stems']
    if ($StemList -eq '') { $Silent++; continue }   # a silent cue (the user wants no sound): no pack, no file
    if (-not $Index.ContainsKey($Pack)) { Problem "${CueName}: pack '$Pack' is not in $Root"; continue }
    if ($Registered -notmatch [regex]::Escape("\Kenney\$Pack\License.txt")) { Problem "${CueName}: pack '$Pack' has no licence line in the registry" }
    foreach ($Stem in ($StemList -split ',')) {
        $Files++
        if (-not $Index[$Pack].ContainsKey($Stem)) { Problem "${CueName}: file '${Stem}.ogg' is not in pack '$Pack'" }
        if ($Assets -and -not (Test-Path (Join-Path $Content "$Pack\SW_$Stem.uasset"))) { Problem "${CueName}: asset SW_$Stem is not imported under Content\Audio\Kenney\$Pack" }
    }
}

if ($Cues -eq 0) { Problem "no cue line read from the table (the format changed?)" }
Write-Host "  $Cues cues ($Silent silent), $Files files read"
if ($Problems -eq 0) { Write-Host "check_audio_table: PASS"; exit 0 }
Write-Host "check_audio_table: FAIL ($Problems)"
exit 1
