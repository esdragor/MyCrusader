# make_maps.ps1 checks the ", N sites" part of the journal's "Match map created" line (chantier Cartes, Auto_Journey_File.md line 79: NON VERIFIE de Haldir-8).
# Every map module lays 2 or 3 sacred sites IN ALL (common.SITE_COUNTS, Retours\Q-sites-nombre.md), so a map's line without ", 2 sites" / ", 3 sites" is a map made by an
# old recipe (or without its sites layer): make_maps.ps1 must say FAIL, with the sites in its message, and exit 1.
# Run on the REAL chain: make_maps.ps1 -LogOnly reads a journal the editor wrote (the Plain one of Saved\Logs, made by the real make_maps.ps1 run) and gives its verdict;
# the cases below change only the ", N sites" part of that real journal.
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\make_maps_sites_test.ps1 [-Root <copy>]   PASS / FAIL, exit code 0 / 1
param([string] $Root = "")

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$Tool = Join-Path $Project "Tools\make_maps.ps1"
$Real = Join-Path $Project "Saved\Logs\make_maps_Plain.log"
if (-not (Test-Path $Real)) { Write-Host "make_maps_sites_test : FAIL (no real journal $Real : run Tools\make_maps.ps1 -Map Plain once)"; exit 1 }

$Text = Get-Content $Real
$Names = @($Text | Select-String -Pattern "Match map created at /Game/Maps/Plain_(\w+):" | ForEach-Object { $_.Matches[0].Groups[1].Value })
if (-not $Names.Count) { Write-Host "make_maps_sites_test : FAIL (the real journal $Real has no 'Match map created' line)"; exit 1 }

$Temp = Join-Path $env:TEMP ("make_maps_sites_test_" + [guid]::NewGuid().ToString("N").Substring(0, 8))
New-Item -ItemType Directory -Force $Temp | Out-Null
$Failed = @()

function Run-Case([string] $Name, [string] $Suffix, [bool] $WantPass, [string] $WantText) {
    $Log = Join-Path $Temp "$Name.log"
    $Lines = foreach ($Line in $Text) {
        if ($Line -match "Match map created at " -and $Suffix) { $Line -replace "navmesh built", ("navmesh built" + $Suffix) } else { $Line }
    }
    Set-Content -Path $Log -Value $Lines -Encoding UTF8
    $Command = "& '$Tool' -Map Plain -Sizes $($Names -join ',') -LogOnly '$Log' -Root '$Project'; if (`$null -eq `$LASTEXITCODE) { exit 9 } else { exit `$LASTEXITCODE }"
    $Output = & powershell -NoProfile -ExecutionPolicy Bypass -Command $Command 2>&1 | Out-String
    $Code = $LASTEXITCODE
    # the verdict is the tool's own line AND its exit code (0 = PASS, 1 = FAIL); anything else (a parameter error, exit 9) is neither
    $Pass = ($Code -eq 0) -and ($Output -match "make_maps : PASS")
    if (-not $Pass -and -not (($Code -eq 1) -and ($Output -match "make_maps : FAIL"))) { $Output = "NO VERDICT: $Output"; $WantText = "^$" }
    $Ok = ($Pass -eq $WantPass) -and (-not $WantText -or $Output -match $WantText)
    Write-Host ("{0}: {1} (exit {2}{3})" -f $Name, $(if ($Ok) { "ok" } else { "FAIL" }), $Code, $(if ($Ok) { "" } else { "; wanted " + $(if ($WantPass) { "PASS" } else { "FAIL" }) + " matching '$WantText', got: " + ($Output.Trim() -replace "\s+", " ") }))
    if (-not $Ok) { $script:Failed += $Name }
}

Run-Case "real_journal_without_sites" "" $false "sites"
Run-Case "two_sites" ", 2 sites" $true ""
Run-Case "three_sites" ", 3 sites" $true ""
Run-Case "one_site" ", 1 sites" $false "sites"
Run-Case "five_sites" ", 5 sites" $false "sites"

Remove-Item $Temp -Recurse -Force -ErrorAction SilentlyContinue
if ($Failed.Count) { Write-Host "make_maps_sites_test : FAIL ($($Failed -join ', '))"; exit 1 }
Write-Host "make_maps_sites_test : PASS"
exit 0
