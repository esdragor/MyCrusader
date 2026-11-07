# Checks the listening scenarios of the sound comparisons (L8-4, AudioInProgress.md), end to end on the real generator:
# runs make_audio_compare.py into a temporary folder (scenarios AND the pre-written E-<scenario>.md files), then verifies for every
# audio_compare*.txt that
#   - each `waitkey` that follows a played variant names the CUE next to the LETTER ("Alert.TrainingDone C (stem) vient de jouer.
#     Entree : Alert.TrainingDone D"), so the panel of the scenario never shows a bare letter;
#   - the E-<scenario>.md file has one line `<Cue> = _` for EVERY cue the scenario plays (the user fills the letters in), the two
#     status lines GESTE: and SON:, and the command to launch;
# and that the scenarios in Tools\Scenarios (the ones the user launches) are the generator's output, byte for byte.
# Prints one line per problem, then "check_audio_compare: PASS" or "FAIL (n)". Exit 0 = pass.
param(
    [string]$Root = (Join-Path $PSScriptRoot "..\.."),
    [string]$Python = "python"
)
$Root = (Resolve-Path $Root).Path
$Problems = 0
function Problem([string]$Text) { Write-Host "  FAIL : $Text"; $script:Problems++ }
$Tmp = Join-Path ([System.IO.Path]::GetTempPath()) ("audio_compare_check_" + [guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Force (Join-Path $Tmp "Scenarios"), (Join-Path $Tmp "Retours") | Out-Null
& $Python (Join-Path $Root "Tools\Audio\make_audio_compare.py") --scenarios (Join-Path $Tmp "Scenarios") --retours (Join-Path $Tmp "Retours") --root $Root | Out-Host
if ($LASTEXITCODE -ne 0) { Problem "make_audio_compare.py failed (exit $LASTEXITCODE; it must accept --scenarios, --retours and --root)" }
$Files = @(Get-ChildItem (Join-Path $Tmp "Scenarios") -Filter "audio_compare*.txt" -ErrorAction SilentlyContinue)
if ($Files.Count -eq 0) { Problem "no audio_compare*.txt generated" }
$Cues = 0
foreach ($File in $Files) {
    $Name = $File.BaseName
    $Lines = Get-Content $File.FullName
    $Played = @($Lines | Where-Object { $_ -match '^hol\.Audio\.Candidate (?<cue>\S+) (?<letter>[A-J])$' } | ForEach-Object { $Matches['cue'] } | Select-Object -Unique)
    $Waits = @($Lines | Where-Object { $_ -match '^waitkey .*vient de jouer' })
    if ($Waits.Count -eq 0) { Problem "${Name}: no `waitkey ... vient de jouer` line" }
    foreach ($Wait in $Waits) {
        if ($Wait -notmatch '^waitkey [A-Za-z]+\.[A-Za-z]+ [A-J] \(') { Problem "${Name}: the panel text has no cue name next to the letter: $Wait" }
        if ($Wait -match 'Entree : (?<next>\S+) ?(?<letter>[A-J])?' -and $Wait -notmatch 'Entree : fin' -and $Wait -notmatch 'Entree : [A-Za-z]+\.[A-Za-z]+ [A-J]') { Problem "${Name}: the next variant has no cue name: $Wait" }
    }
    $E = Join-Path $Tmp "Retours\E-$Name.md"
    if (-not (Test-Path $E)) { Problem "${Name}: no pre-written Retours\E-$Name.md"; continue }
    $Text = Get-Content $E -Raw
    foreach ($Cue in $Played) {
        $Cues++
        if ($Text -notmatch "(?m)^(?:[^\r\n=]*\[)?$([regex]::Escape($Cue))\]? = _\s*$") { Problem "${Name}: E-$Name.md has no line '$Cue = _'" }
    }
    $Extra = @([regex]::Matches($Text, '(?m)^(?:[^\r\n=]*\[)?([A-Za-z]+\.[A-Za-z]+)\]? = _\s*$') | ForEach-Object { $_.Groups[1].Value } | Where-Object { $Played -notcontains $_ })
    foreach ($Cue in $Extra) { Problem "${Name}: E-$Name.md asks about $Cue, which the scenario does not play" }
    if ($Text -notmatch '(?m)^GESTE:') { Problem "${Name}: E-$Name.md has no GESTE: line" }
    if ($Text -notmatch '(?m)^SON:') { Problem "${Name}: E-$Name.md has no SON: line" }
    if ($Text -notmatch [regex]::Escape("play.ps1 $Name")) { Problem "${Name}: E-$Name.md has no play.ps1 command" }
    $Mine = Join-Path $Root "Tools\Scenarios\$Name.txt"
    if (-not (Test-Path $Mine)) { Problem "${Name}: Tools\Scenarios\$Name.txt is missing" }
    elseif ((Get-Content $Mine -Raw) -ne (Get-Content $File.FullName -Raw)) { Problem "${Name}: Tools\Scenarios\$Name.txt is not the generator's output (run make_audio_compare.py)" }
}
Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
Write-Host "  $($Files.Count) scenarios, $Cues cue lines checked"
if ($Problems -eq 0) { Write-Host "check_audio_compare: PASS"; exit 0 }
Write-Host "check_audio_compare: FAIL ($Problems)"
exit 1
