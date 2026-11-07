# known_reds_test.ps1 : test d'effet de L8-5 (rouges connus distincts des regressions dans quick_check). Aucun C++, aucun editeur.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (PASS/FAIL par cas, code 1 s'il y a un FAIL)
# Il prend la VRAIE fonction Report de quick_check.ps1 (analyse syntaxique du fichier) et le VRAI known_reds.txt.
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
$Lib = Join-Path $Here 'KnownReds.ps1'
$List = Join-Path $Here 'known_reds.txt'
$Quick = Join-Path $Here 'quick_check.ps1'
Check (Test-Path $Lib) 'KnownReds.ps1 existe'
Check (Test-Path $List) 'known_reds.txt existe'
if (-not (Test-Path $Lib) -or -not (Test-Path $List)) { Write-Output '=> FAIL (fichiers absents)'; exit 1 }
. $Lib

# Le vrai fichier : camera 2p et camera 8p, avec depuis quand et cause
$Real = @(Get-KnownReds $List)
foreach ($n in 'camera 2p', 'camera 8p') {
  $e = $Real | Where-Object { $_.Test -eq $n } | Select-Object -First 1
  Check ($e -and $e.Since -match '^\d{4}-\d{2}-\d{2}$' -and $e.Cause.Length -gt 5) "known_reds.txt : $n (depuis quand + cause)" ($e | Out-String)
}

# Un fichier factice : commentaires et lignes vides ignores, 3 colonnes separees par |
$Fake = Join-Path ([IO.Path]::GetTempPath()) ('known_reds_test-' + [guid]::NewGuid().ToString('N').Substring(0, 8) + '.txt')
Set-Content -Path $Fake -Encoding ASCII -Value @('# test | depuis | cause', '', 'fake part | 2026-01-02 | cause inventee pour le test', 'other | 2026-03-04 | autre cause')
$F = @(Get-KnownReds $Fake)
Remove-Item $Fake -ErrorAction SilentlyContinue
Check ($F.Count -eq 2 -and $F[0].Test -eq 'fake part' -and $F[0].Since -eq '2026-01-02' -and $F[0].Cause -eq 'cause inventee pour le test') 'Get-KnownReds : 2 lignes, commentaires ignores' ($F | Out-String)

# La vraie fonction Report de quick_check.ps1
$ast = [System.Management.Automation.Language.Parser]::ParseFile($Quick, [ref]$null, [ref]$null)
$fn = $ast.FindAll({ param($a) $a -is [System.Management.Automation.Language.FunctionDefinitionAst] -and $a.Name -eq 'Report' }, $true) | Select-Object -First 1
Check ($null -ne $fn) 'quick_check.ps1 definit Report'
if ($fn) {
  . ([scriptblock]::Create($fn.Extent.Text))
  $SubCases = @{}
  $KnownReds = $Real
  $FailedParts = @()
  $ko = [ordered]@{ 'right edge 1 s: +Y (got 0 m)' = $false; 'something ok' = $true }
  $ok = [ordered]@{ 'something ok' = $true }
  $out = & { $null = Report 'camera 2p' $ko; $null = Report 'camera 3p' $ko; $null = Report 'camera 8p' $ok; $null = Report 'tests' $ok } 6>&1 | Out-String
  Check ($out -match '(?m)^camera 2p : FAIL.*rouge connu.*2026-11-0\d') 'Report : camera 2p rouge -> FAIL, "rouge connu depuis <date>"' $out
  Check ($out -match '(?m)^camera 3p : FAIL.*REGRESSION') 'Report : un rouge hors liste -> FAIL, REGRESSION' $out
  Check ($out -match '(?m)^camera 8p : PASS.*known_reds') 'Report : un rouge connu qui passe -> PASS + "retirer de known_reds.txt"' $out
  Check ($out -match '(?m)^tests : PASS\s*$') 'Report : un test hors liste qui passe -> PASS inchange' $out
  Check ($out -match '\[KO\] right edge') 'Report : les lignes [KO] sont gardees' $out
  Check (($FailedParts -join ',') -eq 'camera 2p,camera 3p') 'Report : FailedParts = les parties rouges' ($FailedParts -join ',')
  $sum = Get-RedSummary $FailedParts $KnownReds
  Check ($sum -match '1 regression' -and $sum -match 'camera 3p' -and $sum -match '1 rouge connu') 'Get-RedSummary : 1 regression (camera 3p) + 1 rouge connu' $sum
  $sumKnown = Get-RedSummary @('camera 2p', 'camera 8p') $KnownReds
  Check ($sumKnown -match 'aucune regression') 'Get-RedSummary : rouges connus seulement -> "aucune regression"' $sumKnown
}
if ($Fails -gt 0) { Write-Output "=> $Fails FAIL"; exit 1 } else { Write-Output '=> tout PASS'; exit 0 }
