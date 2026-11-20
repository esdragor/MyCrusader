# regen_tables_test.ps1 : test de regen_tables.ps1 (L20-9) en -DryRun. Aucun editeur lance : le script imprime les commandes, ce test les lit.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (une ligne PASS/FAIL par cas, code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'regen_tables.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
Check (Test-Path $Tool) 'regen_tables.ps1 existe' $Tool
if (Test-Path $Tool) {
  $Tmp = Join-Path ([IO.Path]::GetTempPath()) ('regen-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
  New-Item -ItemType Directory -Path $Tmp -Force | Out-Null
  [IO.File]::WriteAllText((Join-Path $Tmp 'HeroesOfLiberty.uproject'), '{}')
  $Out = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Tmp -DryRun 2>&1 | ForEach-Object { "$_" })
  $Code = $LASTEXITCODE
  Check ($Code -eq 0) 'DryRun : code 0' "code=$Code sortie=$($Out -join ' | ')"
  $Cmds = @($Out | Where-Object { $_ -match 'UnrealEditor-Cmd\.exe' })
  Check ($Cmds.Count -eq 2) 'DryRun : deux commandes imprimees' "n=$($Cmds.Count)"
  if ($Cmds.Count -eq 2) {
    Check ($Cmds[0] -match 'make_unit_types\.py' -and $Cmds[1] -match 'make_building_types\.py') 'ordre : make_unit_types puis make_building_types'
    foreach ($i in 0, 1) {
      $c = $Cmds[$i]
      Check ($c -match 'HeroesOfLiberty\.uproject') "commande $($i + 1) : uproject du -Root"
      Check ($c.Contains($Tmp)) "commande $($i + 1) : chemin du script sous -Root" $c
      Check ($c -match '-run=pythonscript') "commande $($i + 1) : -run=pythonscript"
      $miss = @('-unattended', '-nullrhi', '-nosplash', '-nosound', '-Multiprocess', '-LiveCoding=0', '-abslog=') | Where-Object { -not $c.Contains($_) }
      Check ($miss.Count -eq 0) "commande $($i + 1) : drapeaux sans fenetre" ($miss -join ',')
    }
  }
  $Bad = @(& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Root (Join-Path $Tmp 'absent') -DryRun 2>&1 | ForEach-Object { "$_" })
  Check ($LASTEXITCODE -ne 0) '-Root sans uproject : refus (code non nul)' ($Bad -join ' | ')
  Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
}
if ($Fails -gt 0) { Write-Output "regen_tables_test : $Fails FAIL"; exit 1 }
Write-Output 'regen_tables_test : tout PASS'
exit 0
