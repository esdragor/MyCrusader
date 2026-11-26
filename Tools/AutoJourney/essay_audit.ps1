# essay_audit.ps1 : lists what the bilan must see (L14-5 / L14-6, SAURON-1126m2). Read-only, no editor launched.
#   INTERNE <file>        a Retours\E-*.md titled [A ESSAYER] whose line is "GESTE: interne" (not a real player gesture)
#   SANS_GESTE <file>     a Retours\E-*.md titled [A ESSAYER] with no "GESTE:" line at all
#   ROUGE_NON_VU <file>   a report of Docs\Rapports\*.md whose TEST_ROUGE field says neither "vu" nor "sans objet"
#                         (a bare sha, "suppose", "non vu"...) and has no "TEST_ROUGE_VU: oui" line
# Usage : essay_audit.ps1 -Retours <Retours folder> -Reports <Docs\Rapports folder> [-Fail]   (-Fail : exit 1 if anything is listed)
param([Parameter(Mandatory = $true)][string]$Retours, [Parameter(Mandatory = $true)][string]$Reports, [switch]$Fail)
$ErrorActionPreference = 'Stop'
foreach ($d in $Retours, $Reports) {
  if (-not (Test-Path -LiteralPath $d -PathType Container)) { Write-Output "essay_audit : folder not found: $d"; exit 2 }
}
$Utf8 = New-Object Text.UTF8Encoding $false
$Intern = 0; $NoGesture = 0; $Red = 0
foreach ($f in Get-ChildItem -LiteralPath $Retours -Filter 'E-*.md' -File | Sort-Object Name) {
  $lines = [IO.File]::ReadAllLines($f.FullName, $Utf8)
  if ($lines.Count -eq 0 -or $lines[0] -notmatch '^#\s*\[.{1,3}\s*ESSAYER\]') { continue }
  $g = @($lines | Where-Object { $_ -match '^GESTE\s*:' } | Select-Object -First 1)
  if ($g.Count -eq 0) { $NoGesture++; Write-Output "SANS_GESTE $($f.FullName)" }
  elseif ($g[0] -match '^GESTE\s*:\s*interne') { $Intern++; Write-Output "INTERNE $($f.FullName)" }
}
foreach ($f in Get-ChildItem -LiteralPath $Reports -Filter '*.md' -File | Sort-Object Name) {
  $lines = [IO.File]::ReadAllLines($f.FullName, $Utf8)
  $field = @($lines | Where-Object { $_ -match '^TEST_ROUGE\s*:' } | Select-Object -First 1)
  if ($field.Count -eq 0) { continue }
  if (@($lines | Where-Object { $_ -match '^TEST_ROUGE_VU\s*:\s*oui' }).Count -gt 0) { continue }
  $v = $field[0] -replace '^TEST_ROUGE\s*:\s*', ''
  if ($v -match '(?i)sans objet') { continue }
  if ($v -match '(?i)\b(non|jamais)\s+vu\b|suppos') { $Red++; Write-Output "ROUGE_NON_VU $($f.FullName) : $v"; continue }
  if ($v -match '(?i)\bvus?\b|\bFAIL\b|\d+\s*/\s*\d+') { continue }
  $Red++; Write-Output "ROUGE_NON_VU $($f.FullName) : $v"
}
Write-Output "essay_audit : $Intern interne, $NoGesture sans geste, $Red rouges non vus"
if ($Fail -and ($Intern + $NoGesture + $Red) -gt 0) { exit 1 }
exit 0
