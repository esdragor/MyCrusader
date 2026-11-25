# after_commit.ps1 (TOOLS-1125a) : build + tests d'un worktree en UN appel, UNE ligne de sortie.
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\after_commit.ps1 <worktree>
# 1) <worktree>\Tools\build.ps1 -LiveCoding=0 -Wait (un seul appel bloquant, la file de build attend) ;
# 2) si PASS : <worktree>\Tools\Tests\quick_check.ps1 -Parts tests (le quick_check DU worktree, jamais celui de la copie principale) ;
# 3) sortie complete (build, quick_check, resume) dans <copie principale>\Docs\Rapports\AFTER-<nom du worktree>-<HHmm>.txt ;
# 4) UNE ligne imprimee : `PASS build <s> s, tests N/N` | `FAIL build : <premiere erreur>` | `FAIL tests N/M : <premier rouge>` ; code 0 (PASS) ou 1.
# Options de test : -OutDir <dossier>, -BuildScript <ps1>, -QuickCheck <ps1>. Test : Tools\AutoJourney\after_commit_test.ps1 (fonction pure Get-AfterSummary, aucun build).
# Aucune boucle d'attente : ce script EST l'attente (agent_rules 3d). Ne tue aucun build.
param(
  [Parameter(Position = 0)][string]$Worktree = '',
  [string]$MainRoot = 'C:\Users\baudo\Documents\MyCrusader',
  [string]$OutDir = '',
  [string]$BuildScript = '',
  [string]$QuickCheck = ''
)

function Get-AfterSummary {
  # Fonction pure : codes de sortie et lignes du build / de quick_check -> @{ Ok; Line }.
  param([int]$BuildExit, [int]$BuildSeconds, [string[]]$BuildLines = @(), $TestExit = $null, [string[]]$TestLines = @())
  $cap = { param([string]$s) $s = $s.Trim(); if ($s.Length -gt 300) { $s.Substring(0, 300) } else { $s } }
  if ($BuildExit -ne 0) {
    $b = @($BuildLines | Where-Object { $_ -and $_.Trim() })
    $pick = @($b | Where-Object { $_ -match 'error\s+[A-Za-z]+\d+|: error\b' -and $_ -notmatch '^\s*build : (FAIL|PASS)\b' } | Select-Object -First 1)
    if (-not $pick.Count) { $pick = @($b | Where-Object { $_ -match '\berror\b' -and $_ -notmatch '^\s*build : (FAIL|PASS)\b' } | Select-Object -First 1) }
    if (-not $pick.Count) { $pick = @($b | Where-Object { $_ -match '^\s*build : FAIL' } | Select-Object -First 1 | ForEach-Object { $_ -replace '^\s*build : FAIL\s*', '' }) }
    $msg = if ($pick.Count -and ([string]$pick[0]).Trim()) { & $cap ([string]$pick[0]) } else { "no output from build.ps1 (exit $BuildExit)" }
    return [pscustomobject]@{ Ok = $false; Line = "FAIL build : $msg" }
  }
  $t = @($TestLines | Where-Object { $_ -ne $null })
  $m = $null
  foreach ($l in $t) { $mm = [regex]::Match($l, '^tests (\d+)/(\d+) : (PASS|FAIL)'); if ($mm.Success) { $m = $mm; break } }
  $n = if ($m) { [int]$m.Groups[1].Value } else { 0 }
  $total = if ($m) { [int]$m.Groups[2].Value } else { 0 }
  $green = $m -and $m.Groups[3].Value -eq 'PASS' -and $n -eq $total -and $total -gt 0 -and ($null -eq $TestExit -or [int]$TestExit -eq 0)
  if ($green) { return [pscustomobject]@{ Ok = $true; Line = "PASS build $BuildSeconds s, tests $n/$total" } }
  # premier rouge : le premier test en echec ("all green (failed: a, b)"), sinon le premier [KO], sinon la ligne FAIL de quick_check
  $red = ''
  foreach ($l in $t) { $mm = [regex]::Match($l, '\[KO\] all green \(failed: ([^,)]+)'); if ($mm.Success) { $red = $mm.Groups[1].Value.Trim(); break } }
  if (-not $red) { foreach ($l in $t) { $mm = [regex]::Match($l, '^\s*\[KO\]\s*(.+)$'); if ($mm.Success) { $red = $mm.Groups[1].Value.Trim(); break } } }
  if (-not $red) { foreach ($l in $t) { if ($l -match '^quick_check : FAIL') { $red = $l.Trim(); break } } }
  if (-not $red) { $red = "quick_check exit $TestExit, no red line" }
  return [pscustomobject]@{ Ok = $false; Line = "FAIL tests $n/$total : $(& $cap $red)" }
}

if ($MyInvocation.InvocationName -eq '.') { return }   # charge par le test : seulement la fonction

$ErrorActionPreference = 'Continue'
if (-not $OutDir) { $OutDir = Join-Path $MainRoot 'Docs\Rapports' }
if (-not $Worktree -or -not (Test-Path -LiteralPath $Worktree -PathType Container)) {
  Write-Output "FAIL build : worktree introuvable : '$Worktree'"
  exit 1
}
$Root = (Resolve-Path -LiteralPath $Worktree).Path
if (-not $BuildScript) { $BuildScript = Join-Path $Root 'Tools\build.ps1' }
if (-not $QuickCheck) { $QuickCheck = Join-Path $Root 'Tools\Tests\quick_check.ps1' }
$short = (Split-Path $Root -Leaf) -replace '^HeroesOfLiberty-', ''
$outFile = Join-Path $OutDir ('AFTER-' + $short + '-' + (Get-Date).ToString('HHmm') + '.txt')
if (-not (Test-Path -LiteralPath $OutDir)) { New-Item -ItemType Directory -Path $OutDir -Force | Out-Null }
$all = New-Object System.Collections.Generic.List[string]

$clock = [Diagnostics.Stopwatch]::StartNew()
$buildLines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $BuildScript -Root $Root '-LiveCoding=0' -Wait 2>&1 | ForEach-Object { "$_" })
$buildExit = $LASTEXITCODE
$buildSeconds = [int][math]::Round($clock.Elapsed.TotalSeconds)
$all.Add("=== build.ps1 -LiveCoding=0 -Wait (exit $buildExit, $buildSeconds s) ===")
foreach ($l in $buildLines) { $all.Add($l) }

$testLines = @(); $testExit = $null
if ($buildExit -eq 0) {
  $testLines = @(& powershell -NoProfile -ExecutionPolicy Bypass -File $QuickCheck -Parts tests 2>&1 | ForEach-Object { "$_" })
  $testExit = $LASTEXITCODE
  $all.Add("=== quick_check.ps1 -Parts tests (exit $testExit) ===")
  foreach ($l in $testLines) { $all.Add($l) }
}

$r = Get-AfterSummary -BuildExit $buildExit -BuildSeconds $buildSeconds -BuildLines $buildLines -TestExit $testExit -TestLines $testLines
$all.Add('=== resume ===')
$all.Add($r.Line)
[IO.File]::WriteAllText($outFile, (($all -join "`r`n") + "`r`n"), (New-Object Text.UTF8Encoding $false))
Write-Output $r.Line
exit $(if ($r.Ok) { 0 } else { 1 })
