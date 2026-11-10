# Makes the files of a match map at every named size (or the sizes given), with its recipe (chantier Cartes, C0.5):
# deletes Content\Maps\<Map>_<Size>.umap, runs the editor once with Tools\Editor\create_match_map.py <Map> <Sizes>,
# and checks its log for one "Match map created" line per size. The map must be a line of Tools\Data\Maps.csv.
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\make_maps.ps1 -Map Plain [-Sizes Small,Large] [-Seed 3] [-Root <copy>]
# Without -Sizes: every size of Tools\Data\MapSizes.csv. -Seed N: the layout seed of the files made (C1c, default 1 = the shipped
# layout; a map that draws its shapes and placements from it, the Steppe, comes out as a variant; the file names do not change). -Test: the fixed 400 m test map of Plain (/Game/Maps/Plain,
# D494) instead of the sizes. Log: <copy>\Saved\Logs\make_maps_<Map>.log. Prints one line PASS / FAIL; exit code 0 / 1.
# The editor loads this copy's DLLs: never run it while this copy builds (Tools\build.ps1 would stop as BUSY).
param(
    [Parameter(Mandatory = $true)] [string] $Map,
    [string[]] $Sizes = @(),
    [switch] $Test,
    [int] $Seed = 0,
    [string] $Root = "",
    [string] $LogOnly = ""
)
# -LogOnly <journal>: no editor, nothing deleted: gives the verdict on a journal the editor already wrote (Tools\Tests\make_maps_sites_test.ps1).
# Each map's journal line must end with its sacred sites, ", 2 sites" or ", 3 sites" (2 or 3 IN ALL, common.SITE_COUNTS, Retours\Q-sites-nombre.md): a line
# without them is a map made without its sites layer (a recipe from before S4), which Map.Table refuses (bSitesLaidByMaps).

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..").Path }
$UProject = Join-Path $Project "HeroesOfLiberty.uproject"
if (-not (Test-Path $UProject)) { Write-Host "make_maps : FAIL (no HeroesOfLiberty.uproject in $Project)"; exit 1 }
if (-not $LogOnly) {
    $Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
    $EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
    if (-not $EngineRoot) { Write-Host "make_maps : FAIL (engine '$Association' of the .uproject not found in the registry)"; exit 1 }
    $Editor = Join-Path ($EngineRoot.Replace('/', '\').TrimEnd('\')) "Engine\Binaries\Win64\UnrealEditor.exe"
}

# The map: a line of Tools\Data\Maps.csv (Name,DisplayName,Recipe).
$MapsCsv = Join-Path $Project "Tools\Data\Maps.csv"
$Known = @(Import-Csv $MapsCsv | ForEach-Object { $_.Name })
if ($Known -notcontains $Map) { Write-Host "make_maps : FAIL (map '$Map' is not in $MapsCsv : $($Known -join ', '))"; exit 1 }

# The sizes: keys of Tools\Data\MapSizes.csv.
$AllSizes = @(Import-Csv (Join-Path $Project "Tools\Data\MapSizes.csv") | ForEach-Object { $_.Name })
if ($Test) {
    if ($Map -ne "Plain") { Write-Host "make_maps : FAIL (only Plain has a fixed test map, D494)"; exit 1 }
    $Sizes = @()
    $Files = @("Plain")
    $Arguments = @()
} else {
    if (-not $Sizes.Count) { $Sizes = $AllSizes }
    $Unknown = @($Sizes | Where-Object { $AllSizes -notcontains $_ })
    if ($Unknown.Count) { Write-Host "make_maps : FAIL (unknown sizes $($Unknown -join ', '); known: $($AllSizes -join ', '))"; exit 1 }
    $Files = @($Sizes | ForEach-Object { "${Map}_$_" })
    $Arguments = @($Map) + $Sizes
}
if ($Seed -ne 0) { $Arguments += "seed=$Seed" }

$Clock = [Diagnostics.Stopwatch]::StartNew()
$TimedOut = $false
if ($LogOnly) {
    $Log = $LogOnly
    if (-not (Test-Path $Log)) { Write-Host "make_maps : FAIL (no journal $Log)"; exit 1 }
} else {
    # The old files go (the recipe refuses to overwrite).
    foreach ($File in $Files) {
        $Path = Join-Path $Project "Content\Maps\$File.umap"
        if (Test-Path $Path) { Remove-Item $Path -Force }
    }

    $Log = Join-Path $Project "Saved\Logs\make_maps_$Map.log"
    New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
    Remove-Item $Log -ErrorAction SilentlyContinue
    $Script = Join-Path $Project "Tools\Editor\create_match_map.py"
    # The script's path has no space (the copies live under Documents\HeroesOfLiberty-*): no quotes inside -ExecCmds.
    if ($Script -match ' ') { Write-Host "make_maps : FAIL (a space in $Script; -ExecCmds cannot quote it)"; exit 1 }
    $EditorArguments = "`"$UProject`" -nosplash -unattended -Multiprocess -LiveCoding=0 -nosound -abslog=`"$Log`" -ExecCmds=`"py $Script $($Arguments -join ' ')`""
    $Process = Start-Process -FilePath $Editor -ArgumentList $EditorArguments -PassThru -WindowStyle Hidden
    $TimedOut = -not $Process.WaitForExit(1800000)
    if ($TimedOut) { Stop-Process -Id $Process.Id -Force -ErrorAction SilentlyContinue }
}
$Seconds = [math]::Round($Clock.Elapsed.TotalSeconds)

$Text = if (Test-Path $Log) { Get-Content $Log } else { @() }
$Created = @($Text | Select-String -Pattern "Match map created at (/Game/Maps/\S+):" | ForEach-Object { $_.Matches[0].Groups[1].Value })
# The sacred sites of each map made: the ", N sites" of its line, N = 2 or 3. A map of the run with no such part (or another N) is an error.
$SiteErrors = @()
foreach ($Line in @($Text | Select-String -Pattern "Match map created at (/Game/Maps/\S+):")) {
    $Name = $Line.Matches[0].Groups[1].Value
    if ($Files -notcontains ($Name -replace '^/Game/Maps/', '')) { continue }
    $Sites = [regex]::Match($Line.Line, ", (\d+) sites(,|$)")
    if (-not $Sites.Success) { $SiteErrors += "$Name : no ', N sites' in its journal line (a map made without its sites layer; Map.Table needs 2 or 3 sites)" }
    elseif ([int]$Sites.Groups[1].Value -notin 2, 3) { $SiteErrors += "$Name : $($Sites.Groups[1].Value) sites in its journal line (a map has 2 or 3 sites in all)" }
}
$Errors = @($Text | Select-String -Pattern "Match map: |LogPython: Error|No navmesh was built|Could not save|Already there" | ForEach-Object { $_.Line.Trim() })
$Missing = @($Files | Where-Object { $Name = $_; -not ($Created -contains "/Game/Maps/$Name") -or -not (Test-Path (Join-Path $Project "Content\Maps\$Name.umap")) })
if (-not $TimedOut -and -not $Errors.Count -and -not $Missing.Count -and -not $SiteErrors.Count) {
    Write-Host "make_maps : PASS ($Map : $($Files -join ', ') ; $Seconds s)"
    $Text | Select-String -Pattern "Match map created" | ForEach-Object { Write-Host "  $($_.Line -replace '^.*LogPython: ', '')" }
    exit 0
}
Write-Host "make_maps : FAIL ($Map ; $Seconds s$(if ($TimedOut) { ', timed out' }) ; log $Log)"
if ($Missing.Count) { Write-Host "  missing: $($Missing -join ', ')" }
$SiteErrors | ForEach-Object { Write-Host "  sites: $_" }
$Errors | Select-Object -First 10 | ForEach-Object { Write-Host "  $_" }
exit 1
