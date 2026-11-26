# essay_audit_test.ps1 : test of essay_audit.ps1 (L14-5 / L14-6, SAURON-1126m2). ASCII only, no editor launched.
# Usage : powershell -ExecutionPolicy Bypass -File <this file>   (one PASS/FAIL line per case, exit 1 on any FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'essay_audit.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
function Put([string]$Path, [string]$Text) { [IO.File]::WriteAllText($Path, $Text, (New-Object Text.UTF8Encoding $false)) }
$A = [string][char]0xC0   # capital A grave of the essay title tag
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('essay-audit-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Ret = Join-Path $Tmp 'Retours'
$Rap = Join-Path $Tmp 'Rapports'
New-Item -ItemType Directory -Path $Ret, $Rap -Force | Out-Null
Put (Join-Path $Ret 'E-aaa_internal.md') "# [${A} ESSAYER] E-aaa_internal : x`r`n`r`nSTATUT : VU`r`nGESTE: interne      (commande hol.X)`r`nSON: sans objet`r`n"
Put (Join-Path $Ret 'E-bbb_real.md') "# [${A} ESSAYER] E-bbb_real : x`r`n`r`nGESTE: r$([char]0xE9)el (vrai clic)`r`nSON: sans objet`r`n"
Put (Join-Path $Ret 'E-ccc_nogesture.md') "# [${A} ESSAYER] E-ccc_nogesture : x`r`n`r`nSTATUT : VU`r`n"
Put (Join-Path $Ret 'E-ddd_done.md') "# [ESSAY$([char]0xC9)] E-ddd_done : x`r`n`r`nGESTE: interne (old)`r`n"
Put (Join-Path $Rap 'R-seen.md') "TEST_ROUGE: vu a 15:07 : FAIL (6)   TEST_ROUGE_SEPARE: oui`r`n"
Put (Join-Path $Rap 'R-seen2.md') "TEST_ROUGE_VU: oui`r`n"
Put (Join-Path $Rap 'R-na.md') "TEST_ROUGE: sans objet (docs)`r`n"
Put (Join-Path $Rap 'R-supposed.md') "TEST_ROUGE: suppose rouge, non lance (pas de build)`r`n"
Put (Join-Path $Rap 'R-notseen.md') "TEST_ROUGE: 4ba5ec2 + assertit l'effet : oui   TEST_ROUGE_SEPARE: oui`r`n"
Put (Join-Path $Rap 'R-nofield.md') "FAIT: rien`r`n"
Check (Test-Path $Tool) 'essay_audit.ps1 exists' $Tool
if (Test-Path $Tool) {
  $Out = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Retours $Ret -Reports $Rap 2>&1 | ForEach-Object { "$_" })
  $Code = $LASTEXITCODE
  $Txt = $Out -join "`n"
  Check ($Code -eq 0) 'report mode : exit code 0' "code=$Code"
  Check (@($Out -match '^INTERNE .*E-aaa_internal').Count -gt 0) 'lists the essay with GESTE: interne'
  Check (-not ($Txt -match 'E-bbb_real')) 'does not list the essay with GESTE: reel'
  Check (@($Out -match '^SANS_GESTE .*E-ccc_nogesture').Count -gt 0) 'lists the essay without a GESTE line'
  Check (-not ($Txt -match 'E-ddd_done')) 'ignores an essay that is not [A ESSAYER]'
  Check (@($Out -match '^ROUGE_NON_VU .*R-notseen').Count -gt 0) 'lists a report whose TEST_ROUGE is a bare sha'
  Check (@($Out -match '^ROUGE_NON_VU .*R-supposed').Count -gt 0) 'lists a report whose TEST_ROUGE is only supposed'
  Check (-not ($Txt -match 'R-seen|R-na|R-nofield')) 'does not list seen, sans objet or field-less reports'
  Check (@($Out -match 'essay_audit : 1 interne, 1 sans geste, 2 rouges non vus').Count -gt 0) 'summary line counts' $Txt
  $Strict = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Retours $Ret -Reports $Rap -Fail 2>&1 | ForEach-Object { "$_" })
  Check ($LASTEXITCODE -eq 1) '-Fail : exit code 1 when something is listed' "code=$LASTEXITCODE"
  $Bad = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Retours (Join-Path $Tmp 'absent') -Reports $Rap 2>&1 | ForEach-Object { "$_" })
  Check ($LASTEXITCODE -ne 0) 'missing -Retours folder : refusal' ($Bad -join ' | ')
}
Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
if ($Fails -gt 0) { Write-Output "essay_audit_test : $Fails FAIL"; exit 1 }
Write-Output 'essay_audit_test : all PASS'
exit 0
