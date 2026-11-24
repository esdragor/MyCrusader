# stale_quickcheck.ps1 : lists the identifiers a branch REMOVED or RENAMED that quick_check.ps1 still cites (F8, L22-3 / L19-4).
# Case seen: integ-58, quick_check.ps1:4273 still waited for an F9 / F10 label the greens had changed.
# Usage : powershell -ExecutionPolicy Bypass -File stale_quickcheck.ps1 -Root <worktree> [-Base main] [-QuickCheck <path>]
# Output: one line `STALE: <identifier> : quick_check.ps1:<line>` per removed identifier still cited; exit 1 if any, else 0.
# Identifiers read from `git diff <Base>...HEAD -U0 -- Script Tools/Data Source`: quoted strings (NSLOCTEXT key and text, log labels,
# automation test paths), Test_* names, and the first field of a CSV row under Tools/Data. An identifier counts as removed when a
# removed line carries it, no added line does, and `git grep` finds it nowhere in HEAD's Script / Tools/Data / Source. Reads only; no editor, no quick_check.
param(
  [Parameter(Mandatory = $true)][string]$Root,
  [string]$Base = 'main',
  [string]$QuickCheck = ''
)
$ErrorActionPreference = 'Continue'
$Root = (Resolve-Path -LiteralPath $Root).Path
if ($QuickCheck -eq '') {
  foreach ($c in @('Tools\Tests\quick_check.ps1', 'Tools\quick_check.ps1')) {
    if (Test-Path -LiteralPath (Join-Path $Root $c)) { $QuickCheck = Join-Path $Root $c; break }
  }
}
if ($QuickCheck -eq '' -or -not (Test-Path -LiteralPath $QuickCheck)) { Write-Output "ERROR: quick_check.ps1 not found under $Root"; exit 2 }
[Console]::OutputEncoding = [Text.Encoding]::UTF8
$Paths = @('Script', 'Tools/Data', 'Source')

function Get-Identifiers([string]$Text, [bool]$IsCsv) {
  $ids = New-Object System.Collections.Generic.List[string]
  foreach ($m in [regex]::Matches($Text, '"([^"\\]{4,})"')) { $ids.Add($m.Groups[1].Value.Trim()) }
  foreach ($m in [regex]::Matches($Text, '\bTest_\w+')) { $ids.Add($m.Value) }
  if ($IsCsv) {
    $first = ($Text -split ',')[0].Trim().Trim('"')
    if ($first.Length -ge 3) { $ids.Add($first) }
  }
  return $ids
}

$diff = & git.exe -C $Root -c core.quotepath=off diff "$Base...HEAD" -U0 --no-color -- @Paths 2>&1
if ($LASTEXITCODE -ne 0) { Write-Output "ERROR: git diff $Base...HEAD failed: $($diff -join ' ')"; exit 2 }

$removed = New-Object System.Collections.Generic.HashSet[string]
$added = New-Object System.Collections.Generic.HashSet[string]
$file = ''
foreach ($raw in $diff) {
  $line = ([string]$raw).TrimEnd("`r")
  if ($line.StartsWith('+++ ')) { $file = $line.Substring(4); continue }
  if ($line.StartsWith('--- ') -or $line.StartsWith('diff ') -or $line.StartsWith('@@')) { continue }
  $isCsv = $file -like '*Tools/Data/*.csv'
  if ($line.StartsWith('-')) { foreach ($i in (Get-Identifiers $line.Substring(1) $isCsv)) { [void]$removed.Add($i) } }
  elseif ($line.StartsWith('+')) { foreach ($i in (Get-Identifiers $line.Substring(1) $isCsv)) { [void]$added.Add($i) } }
}

$qcLines = [IO.File]::ReadAllLines($QuickCheck)
$stale = 0
foreach ($id in ($removed | Sort-Object)) {
  if ($added.Contains($id)) { continue }
  $hits = @()
  for ($n = 0; $n -lt $qcLines.Count; $n++) { if ($qcLines[$n].Contains($id)) { $hits += ($n + 1) } }
  if ($hits.Count -eq 0) { continue }
  & git.exe -C $Root grep -q -F -e $id HEAD -- @Paths 2>&1 | Out-Null
  if ($LASTEXITCODE -eq 0) { continue }   # still present somewhere in HEAD: moved, not retired
  foreach ($h in $hits) { Write-Output "STALE: $id : quick_check.ps1:$h"; $stale++ }
}
if ($stale -gt 0) { exit 1 }
Write-Output 'OK: no removed identifier is still cited by quick_check.ps1'
exit 0
