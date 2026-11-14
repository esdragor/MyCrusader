# build_queue_output_test.ps1 (2026-11-14, FARAMIR-1114b) : le controle par build_queue_test.ps1 rendait une sortie VIDE.
# Causes : (1) toutes ses lignes partaient par Write-Host (flux d'information) : un appelant qui capture la SORTIE (`$o = & build_queue_test.ps1`,
# `| Out-String`, un outil qui ne garde que stdout) ne recevait rien, ni PASS/FAIL ni la ligne finale ; (2) un -Only inconnu ne jouait aucun cas
# et finissait sur "build_queue_test : PASS" (vert sans rien tester).
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (une ligne PASS/FAIL par cas, code 1 s'il y a un FAIL ; ~30 s)
$ErrorActionPreference = 'Continue'
$Bqt = Join-Path $PSScriptRoot 'build_queue_test.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}

# (2) -Only inconnu : refuse (code 2, liste des parties), jamais PASS
$o1 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Bqt -Only q 2>&1 | Out-String
$c1 = $LASTEXITCODE
Check ($c1 -eq 2 -and $o1 -notmatch 'build_queue_test : PASS' -and $o1 -match '\bb\b.*\bt\b') '-Only inconnu: code 2, liste des parties, pas de PASS' "code=$c1 out=$o1"

# (1) appel dans le meme processus, sortie capturee : les lignes PASS et la ligne finale sont dans la SORTIE (vraie partie [t] jouee)
$o2 = & $Bqt -Only t 6>$null | Out-String
$c2 = $LASTEXITCODE
Check ($o2 -match '(?m)^\s*PASS ' -and $o2 -match '(?m)^build_queue_test : PASS' -and $c2 -eq 0) 'appel capture (meme processus): PASS par cas et ligne finale dans la sortie' "code=$c2 len=$($o2.Length) out=$o2"
Check ($o2 -match '\[t\]') 'appel capture: le titre de la partie [t] est dans la sortie' "len=$($o2.Length)"

Write-Output ("build_queue_output_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $(if ($Fails -eq 0) { 0 } else { 1 })
