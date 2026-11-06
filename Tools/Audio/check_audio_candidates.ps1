# Checks the candidates of Script/Audio/HOLAudioCandidates.as (AudioInProgress.md, medieval-fantasy interface sounds):
# every `Add(List, "<Cue>", "<ten stems>")` line names a cue of the table HOLAudioTable.as, has ten distinct stems, none from the
# Kenney interface-sounds / ui-audio packs (the spaceship-console sounds), and every stem exists on disk: Kenney stems as <stem>.ogg in
# rpg-audio or impact-sounds, v_ stems as <stem>.wav in MyCrusader_Audio\Candidates\vsco (made by make_ui_candidates.py, licence file
# MyCrusader_Audio\VSCO\LICENSE in Docs/assets_audio.csv). With -Assets also that Content\Audio\Candidates\<kenney|vsco>\SW_<stem>.uasset exists.
# Prints one line per problem, then "check_audio_candidates: PASS" or "FAIL (n)". Exit 0 = pass.
param(
    [switch]$Assets,
    [string]$Audio = "C:\Users\baudo\Documents\MyCrusader_Audio",
    [string]$Content = (Join-Path $PSScriptRoot "..\..\Content\Audio\Candidates"),
    [string]$File = (Join-Path $PSScriptRoot "..\..\Script\Audio\HOLAudioCandidates.as"),
    [string]$Table = (Join-Path $PSScriptRoot "..\..\Script\Audio\HOLAudioTable.as"),
    [string]$Registry = (Join-Path $PSScriptRoot "..\..\Docs\assets_audio.csv")
)
$Problems = 0
function Problem([string]$Text) { Write-Host "  FAIL : $Text"; $script:Problems++ }
$Names = @(Get-Content $Table | Where-Object { $_ -match '^\s*Add\(Cues,' } | ForEach-Object { if ($_ -match '"([A-Za-z]+\.[A-Za-z]+)"') { $Matches[1] } })
$Lines = @(Get-Content $File | Where-Object { $_ -match '^\s*Add\(List,' })
$Kenney = @{}
foreach ($Pack in "rpg-audio", "impact-sounds") {
    Get-ChildItem (Join-Path $Audio "Kenney\$Pack") -Recurse -Filter *.ogg -ErrorAction SilentlyContinue | ForEach-Object { $Kenney[$_.BaseName] = $true }
}
$Banned = @{}
foreach ($Pack in "interface-sounds", "ui-audio") {
    Get-ChildItem (Join-Path $Audio "Kenney\$Pack") -Recurse -Filter *.ogg -ErrorAction SilentlyContinue | ForEach-Object { $Banned[$_.BaseName] = $true }
}
$Registered = if (Test-Path $Registry) { Get-Content $Registry -Raw } else { "" }
if ($Registered -notmatch [regex]::Escape("VSCO\LICENSE")) { Problem "VSCO has no licence line in the registry" }
$Seen = @()
$Stems = 0
foreach ($Line in $Lines) {
    if ($Line -notmatch '"(?<cue>[A-Za-z]+\.[A-Za-z]+)",\s*"(?<stems>[A-Za-z0-9_,]+)"') { Problem "cannot read the line: $Line"; continue }
    $Cue = $Matches['cue']; $List = @($Matches['stems'] -split ',')
    $Seen += $Cue
    if ($Names -notcontains $Cue) { Problem "${Cue}: not a cue of the table" }
    if ($List.Count -ne 10) { Problem "${Cue}: $($List.Count) variants instead of 10" }
    if (@($List | Select-Object -Unique).Count -ne $List.Count) { Problem "${Cue}: a stem twice" }
    foreach ($Stem in $List) {
        $Stems++
        if ($Stem.StartsWith("v_")) {
            if (-not (Test-Path (Join-Path $Audio "Candidates\vsco\$Stem.wav"))) { Problem "${Cue}: ${Stem}.wav is not in Candidates\vsco (run make_ui_candidates.py)" }
            $Kind = "vsco"
        } else {
            if ($Banned.ContainsKey($Stem) -and -not $Kenney.ContainsKey($Stem)) { Problem "${Cue}: $Stem is a spaceship-console sound (interface-sounds / ui-audio)" }
            elseif (-not $Kenney.ContainsKey($Stem)) { Problem "${Cue}: ${Stem}.ogg is not in rpg-audio or impact-sounds" }
            $Kind = "kenney"
        }
        if ($Assets -and -not (Test-Path (Join-Path $Content "$Kind\SW_$Stem.uasset"))) { Problem "${Cue}: asset SW_$Stem is not imported under Content\Audio\Candidates\$Kind" }
    }
}
foreach ($Name in $Names) { if ($Seen -notcontains $Name) { Problem "${Name}: a cue of the table without candidates" } }
Write-Host "  $($Seen.Count) cues, $Stems stems read"
if ($Problems -eq 0) { Write-Host "check_audio_candidates: PASS"; exit 0 }
Write-Host "check_audio_candidates: FAIL ($Problems)"
exit 1
