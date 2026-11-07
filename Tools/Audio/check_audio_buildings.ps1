# Checks the table "building -> sound" of Script/Audio/HOLAudioBuildings.as against Tools/Data/BuildingTypes.csv (AudioBuildingsInProgress.md B2):
# the table lists exactly the types of the CSV (none forgotten, none invented, none misspelled, none twice), and a type that produces something
# (Produces filled in the CSV) does not say "produces nothing" (-1) for its production moment. Designs nothing: it never reads which event a building
# rings, only that the names agree. Prints one line per problem, then "check_audio_buildings: PASS" or "check_audio_buildings: FAIL (n)".
# Exit code 0 = pass. Run: powershell -File <this file> [-Csv <BuildingTypes.csv>] [-Table <HOLAudioBuildings.as>]
param(
    [string]$Csv = (Join-Path $PSScriptRoot "..\Data\BuildingTypes.csv"),
    [string]$Table = (Join-Path $PSScriptRoot "..\..\Script\Audio\HOLAudioBuildings.as")
)

$Problems = 0
function Problem([string]$Text) { Write-Host "  FAIL : $Text"; $script:Problems++ }

if (-not (Test-Path $Csv)) { Problem "CSV not found: $Csv"; Write-Host "check_audio_buildings: FAIL ($Problems)"; exit 1 }
if (-not (Test-Path $Table)) { Problem "table not found: $Table"; Write-Host "check_audio_buildings: FAIL ($Problems)"; exit 1 }

# The types of the CSV: name -> does it produce something.
$Types = [ordered]@{}
foreach ($Row in (Import-Csv $Csv)) {
    $Name = "$($Row.Name)".Trim()
    if ($Name -eq "") { continue }
    if ($Types.Contains($Name)) { Problem "the CSV lists '$Name' twice"; continue }
    $Types[$Name] = ("$($Row.Produces)".Trim() -ne "")
}
if ($Types.Count -eq 0) { Problem "no building read from the CSV (the format changed?)" }

# The lines of the table: Add(Rows, "<Building>", <event>);
$Listed = [ordered]@{}
foreach ($Line in (Get-Content $Table)) {
    if ($Line -notmatch '^\s*Add\(Rows,\s*"(?<name>[^"]*)",\s*(?<event>[^)]*?)\s*\)\s*;') { continue }
    $Name = $Matches['name']
    if ($Listed.Contains($Name)) { Problem "the table lists '$Name' twice"; continue }
    $Listed[$Name] = $Matches['event']
}
if ($Listed.Count -eq 0) { Problem "no 'Add(Rows, ...)' line read from the table (the format changed?)" }

foreach ($Name in $Types.Keys) {
    if (-not $Listed.Contains($Name)) { Problem "'$Name' is in the CSV but missing from the table (forgotten, or spelled differently)" }
}
foreach ($Name in $Listed.Keys) {
    if (-not $Types.Contains($Name)) { Problem "'$Name' is in the table but not in the CSV (invented, or spelled differently)" }
    elseif ($Types[$Name] -and $Listed[$Name] -eq "-1") { Problem "'$Name' produces something (Produces is filled in the CSV) but its table line says it produces nothing (-1)" }
}

Write-Host "  $($Types.Count) types in the CSV, $($Listed.Count) lines in the table"
if ($Problems -eq 0) { Write-Host "check_audio_buildings: PASS"; exit 0 }
Write-Host "check_audio_buildings: FAIL ($Problems)"
exit 1
