# Test of check_audio_buildings.ps1 (AudioBuildingsInProgress.md B2), on the real chain: the real BuildingTypes.csv and the real
# Script/Audio/HOLAudioBuildings.as are copied and spoiled one way at a time; the check must say PASS on the real pair and FAIL on each spoiled one,
# naming the guilty building. Run: powershell -File <this file>. Exit code 0 = every case behaved as expected.
$Check = Join-Path $PSScriptRoot "check_audio_buildings.ps1"
$RealCsv = Join-Path $PSScriptRoot "..\Data\BuildingTypes.csv"
$RealTable = Join-Path $PSScriptRoot "..\..\Script\Audio\HOLAudioBuildings.as"
$Temp = Join-Path ([IO.Path]::GetTempPath()) ("check_audio_buildings_test_" + [Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $Temp | Out-Null
$Failures = 0

function Run-Check([string]$Csv, [string]$Table) {
    $Output = & powershell -NoProfile -ExecutionPolicy Bypass -File $Check -Csv $Csv -Table $Table 2>&1 | Out-String
    return @{ Code = $LASTEXITCODE; Output = $Output }
}

function Expect([string]$Case, [hashtable]$Result, [int]$Code, [string]$Mention) {
    $Good = ($Result.Code -eq $Code)
    if ($Good -and $Mention -ne "") { $Good = ($Result.Output -match [regex]::Escape($Mention)) }
    if ($Good) { Write-Host "  ok   : $Case (exit $($Result.Code))" }
    else {
        Write-Host "  FAIL : $Case (exit $($Result.Code), expected $Code, expected to mention '$Mention')"
        Write-Host ($Result.Output -replace '(?m)^', '         | ')
        $script:Failures++
    }
}

$CsvLines = @(Get-Content $RealCsv)
$TableText = [IO.File]::ReadAllText($RealTable)

function Write-Csv([string]$Name, [string[]]$Lines) {
    $Path = Join-Path $Temp $Name
    [IO.File]::WriteAllLines($Path, $Lines)
    return $Path
}
function Write-Table([string]$Name, [string]$Text) {
    $Path = Join-Path $Temp $Name
    [IO.File]::WriteAllText($Path, $Text)
    return $Path
}

# 0. The real pair passes.
Expect "real CSV + real table" (Run-Check $RealCsv $RealTable) 0 "PASS"

# 1. A building too many in the CSV (a 15th type the table does not know).
$Extra = $CsvLines + ($CsvLines | Where-Object { $_ -like "House,*" } | ForEach-Object { $_ -replace '^House,', 'WatchTower,' })
Expect "CSV with a 15th building (WatchTower)" (Run-Check (Write-Csv "extra.csv" $Extra) $RealTable) 1 "WatchTower"

# 2. A building missing from the CSV (the table keeps a name the CSV no longer has).
$Missing = $CsvLines | Where-Object { $_ -notlike "Forge,*" }
Expect "CSV without Forge" (Run-Check (Write-Csv "missing.csv" $Missing) $RealTable) 1 "Forge"

# 3. A name misspelled in the CSV.
$Typo = $CsvLines | ForEach-Object { $_ -replace '^Barracks,', 'Barraks,' }
Expect "CSV with Barraks" (Run-Check (Write-Csv "typo.csv" $Typo) $RealTable) 1 "Barraks"

# 4. A name misspelled in the table.
$TableTypo = $TableText.Replace('Add(Rows, "Stable",', 'Add(Rows, "Stabel",')
Expect "table with Stabel" (Run-Check $RealCsv (Write-Table "typo.as" $TableTypo)) 1 "Stabel"

# 5. A line forgotten in the table.
$TableLines = $TableText -split "`r?`n"
$TableMissing = ($TableLines | Where-Object { $_ -notmatch 'Add\(Rows, "Mill",' }) -join "`n"
Expect "table without Mill" (Run-Check $RealCsv (Write-Table "missing.as" $TableMissing)) 1 "Mill"

# 6. A line added in the table (a type that does not exist).
$TableExtra = $TableText.Replace('Add(Rows, "Forge",', 'Add(Rows, "Tavern", -1);' + "`n`t`t" + 'Add(Rows, "Forge",')
Expect "table with a Tavern" (Run-Check $RealCsv (Write-Table "extra.as" $TableExtra)) 1 "Tavern"

# 7. A line twice in the table.
$TableTwice = $TableText.Replace('Add(Rows, "House", -1);', 'Add(Rows, "House", -1);' + "`n`t`t" + 'Add(Rows, "House", -1);')
Expect "table with House twice" (Run-Check $RealCsv (Write-Table "twice.as" $TableTwice)) 1 "House"

# 8. A building that produces something (Produces filled in the CSV) but whose table line says "produces nothing" (-1).
$TableSilentProduction = $TableText.Replace('Add(Rows, "Barracks", HOLAudioEvent::AlertTrainingDone);', 'Add(Rows, "Barracks", -1);')
Expect "Barracks produces but its table line says nothing" (Run-Check $RealCsv (Write-Table "silent.as" $TableSilentProduction)) 1 "Barracks"

Remove-Item -Recurse -Force $Temp
if ($Failures -eq 0) { Write-Host "check_audio_buildings_test: PASS"; exit 0 }
Write-Host "check_audio_buildings_test: FAIL ($Failures)"
exit 1
