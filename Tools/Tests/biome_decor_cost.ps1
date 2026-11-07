# B3 (BiomeInProgress.md), cost of the generic biome decor (HISM pieces laid at the loading of the map) on the S1 and S2 stress tests, in
# the manner of rest_measures.ps1 / chapter_measures.ps1 (stress_ab.ps1: off screen, 2000 units, same options) but on the PLAIN (the decor
# needs the map's bounds and sites) and with the biome as the only difference between the runs of a round:
#   none = no biome (the map's own look, nothing laid), grass = fresh_grass (2128 pieces on the Plain Small), forest = dark_forest (the
#   most pieces, 2340). Runs in alternation (none grass forest, none grass forest, ...) so that a drift of the machine's load spreads over
#   the three. Each run says the processor load just before it (stress_ab.ps1 "cpu before"), the editor's peak working set (MB) and,
#   for a biome, the number of pieces its own journal says it laid (a run whose journal does not say so FAILS: no decor, no measure).
# The machine is NOT asked to be at rest: every number is printed with its spread and the load of its runs; read it as noise first.
#   powershell -ExecutionPolicy Bypass -File Tools\Tests\biome_decor_cost.ps1 [-Root <worktree>] [-Rounds 3] [-Tests S1:10,S2:10] [-Map /Game/Maps/Plain]
# Writes Saved_Tests\Logs\biome_decor_cost.txt.
param(
    [string] $Root = "",
    [int] $Rounds = 3,
    [string[]] $Tests = @("S1:10", "S2:10"),
    [string] $Map = "/Game/Maps/Plain"
)
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$LogDir = Join-Path $Root "Saved_Tests\Logs"
$Stamp = Get-Date -Format "MMdd_HHmm"
$Conditions = @(
    [pscustomobject]@{ Name = "none"; Index = -1 },
    [pscustomobject]@{ Name = "grass"; Index = 0 },
    [pscustomobject]@{ Name = "forest"; Index = 3 }
)
$Runs = @()
$Fails = 0
foreach ($Round in 1..$Rounds) {
    foreach ($T in $Tests) {
        $Scenario, $Seconds = $T.Split(":")
        foreach ($C in $Conditions) {
            $Name = "cost_${Stamp}_$($Scenario.ToLower())_$($C.Name)_$Round"
            # Other sessions run editors and builds on this machine: stress_ab.ps1 refuses to start while one runs (BUSY), so wait for a free
            # moment (up to 40 min per run), and a run during which another editor / build appeared is thrown away and run again (3 tries).
            $Out = ""; $Tries = 0; $Waited = [Diagnostics.Stopwatch]::StartNew()
            while ($Tries -lt 3 -and $Waited.Elapsed.TotalMinutes -lt 40) {
                $Out = & (Join-Path $PSScriptRoot "stress_ab.ps1") -Project $Root -Map $Map -Test $Scenario -Seconds ([int]$Seconds) -LogName $Name -ExtraExec "hol.Biome.Index $($C.Index)," *>&1 | Out-String
                if ($Out -match "BUSY:") { Start-Sleep 15; continue }
                $Tries++
                $After = @(Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -match 'HeroesOfLiberty' -and $_.Name -match 'Unreal|UnrealBuildTool|dotnet' })
                if ($After.Count -eq 0) { break }
                Write-Host "  $Name : $($After.Count) other editor / build process(es) at the end of the run ($(($After | ForEach-Object { $_.ProcessId }) -join ',')) : run again"
            }
            $Text = if (Test-Path (Join-Path $LogDir "$Name.log")) { Get-Content (Join-Path $LogDir "$Name.log") -Raw } else { "" }
            $Step = [regex]::Match($Text, "Simulation: ([\d.]+) ms average per step \(max ([\d.]+) ms")
            $Fps = [regex]::Match($Text, "Rendering: ([\d.]+) fps average")
            $Decor = [regex]::Match($Text, "Biome decor expect: pieces=(\d+)")
            $Cpu = [regex]::Match($Out, "cpu before: (.*)").Groups[1].Value.Trim()
            $Mem = [regex]::Match($Out, "peak memory: (\d+) MB").Groups[1].Value
            $Pieces = if ($Decor.Success) { [int]$Decor.Groups[1].Value } else { 0 }
            $Ok = $Step.Success -and $Fps.Success -and (($C.Index -lt 0 -and -not $Decor.Success) -or ($C.Index -ge 0 -and $Pieces -gt 0))
            if (-not $Ok) { Write-Host "FAIL $Name : step=$($Step.Success) fps=$($Fps.Success) decor=$($Decor.Success) pieces=$Pieces (a biome run must say it laid pieces, the none run must not)"; $Fails++ }
            $Row = [pscustomobject]@{ Round = $Round; Test = $Scenario; Cond = $C.Name; Pieces = $Pieces; StepMs = $(if ($Step.Success) { [double]$Step.Groups[1].Value } else { $null }); MaxMs = $(if ($Step.Success) { [double]$Step.Groups[2].Value } else { $null }); Fps = $(if ($Fps.Success) { [double]$Fps.Groups[1].Value } else { $null }); PeakMB = $Mem; CpuBefore = $Cpu }
            $Runs += $Row
            Write-Host ("{0} {1} {2} : step {3} ms, {4} fps, {5} pieces, peak {6} MB, cpu before {7}" -f $Round, $Scenario, $C.Name, $Row.StepMs, $Row.Fps, $Pieces, $Mem, $Cpu)
        }
    }
}
$Lines = @("Biome decor cost $Stamp on $Map ($Rounds rounds in alternation, the machine NOT at rest)")
$Lines += ($Runs | Format-Table -AutoSize | Out-String -Width 200)
foreach ($T in $Tests) {
    $Scenario = $T.Split(":")[0]
    foreach ($C in $Conditions) {
        $Set = @($Runs | Where-Object { $_.Test -eq $Scenario -and $_.Cond -eq $C.Name -and $null -ne $_.StepMs })
        if ($Set.Count -eq 0) { continue }
        $Steps = $Set | ForEach-Object { $_.StepMs }
        $Fp = $Set | ForEach-Object { $_.Fps }
        $Mean = ($Steps | Measure-Object -Average).Average
        $Lines += ("{0} {1,-6} : step mean {2:N3} ms (min {3:N3}, max {4:N3}, n {5}), fps mean {6:N0} (min {7:N0}, max {8:N0})" -f $Scenario, $C.Name, $Mean, ($Steps | Measure-Object -Minimum).Minimum, ($Steps | Measure-Object -Maximum).Maximum, $Set.Count, ($Fp | Measure-Object -Average).Average, ($Fp | Measure-Object -Minimum).Minimum, ($Fp | Measure-Object -Maximum).Maximum)
    }
}
$Lines | ForEach-Object { Write-Host $_ }
Set-Content -Path (Join-Path $LogDir "biome_decor_cost.txt") -Value $Lines -Encoding UTF8
Write-Host "biome_decor_cost: $(if ($Fails -eq 0) { 'PASS' } else { "FAIL ($Fails)" }) - Saved_Tests\Logs\biome_decor_cost.txt"
exit $(if ($Fails -eq 0) { 0 } else { 1 })
