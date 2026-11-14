# no_window_frames_test.ps1 (L14-8 suite, 2026-11-14) : no_window_test.ps1 controle aussi les `frames N` > 30 qui attendent le jeu dans les scenarios
# (meme fonction que check.ps1 : Tools\Scenarios\FrameWaits.ps1) ; defaut Warn (Retours\Q-frames-refus.md, option B codee), Fail par
# -FrameWaits Fail ou HOL_CHECK_FRAMES=fail. Aucun editeur : no_window_test lit des scripts et des scenarios.
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (une ligne PASS/FAIL par cas, code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'no_window_test.ps1'
$Root = (Resolve-Path (Join-Path $Here '..\..')).Path
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('nwframes-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $Tmp | Out-Null
Set-Content -Encoding ascii (Join-Path $Tmp 'zz_bad_wait.txt') @('hol.Units.Spawn 0 Spearman 1', 'frames 600', 'hol.HUD.Report 1')
Set-Content -Encoding ascii (Join-Path $Tmp 'zz_good_shot.txt') @('hol.Camera.FocusNear 0 0', 'frames 60', 'capture good_a', 'wait 2')
$env:HOL_CHECK_FRAMES = ''

# --- Defaut Warn : la ligne nomme le scenario fautif, pas celui qui encadre une prise ; code 0 (le reste de no_window_test est vert)
$o1 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Root -ScenarioDir $Tmp | Out-String
$c1 = $LASTEXITCODE
Check ($o1 -match '(?m)^WARN frames zz_bad_wait\b.*line 2') 'Warn: le scenario qui attend le jeu par frames 600 est signale (nom + ligne)' $o1
Check ($o1 -notmatch 'zz_good_shot') 'Warn: frames autour d''une prise de camera non signale' $o1
Check ($c1 -eq 0 -and $o1 -notmatch '(?m)^FAIL frames') 'Warn (defaut): code 0, pas de FAIL frames' "code=$c1"

# --- Fail : par parametre, puis par variable d'environnement
$o2 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Root -ScenarioDir $Tmp -FrameWaits Fail | Out-String
Check ($LASTEXITCODE -eq 1 -and $o2 -match '(?m)^FAIL frames.*zz_bad_wait') '-FrameWaits Fail: FAIL qui nomme le scenario, code 1' $o2
$env:HOL_CHECK_FRAMES = 'fail'
$o3 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Root -ScenarioDir $Tmp | Out-String
$c3 = $LASTEXITCODE
$env:HOL_CHECK_FRAMES = ''
Check ($c3 -eq 1 -and $o3 -match '(?m)^FAIL frames') 'HOL_CHECK_FRAMES=fail: FAIL, code 1' $o3

# --- Vraie chaine : les scenarios du depot (206 attentes dans 116 scenarios le 2026-11-14) donnent un compte, toujours code 0 par defaut
$o4 = & powershell -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Root | Out-String
Check ($LASTEXITCODE -eq 0 -and $o4 -match '(?m)^WARN frames waits: (\d+) in (\d+) scenario' -and [int]$Matches[1] -gt 0) 'scenarios du depot: ligne de compte WARN frames waits, code 0' ($o4 -split "`n" | Select-Object -Last 5 | Out-String)

Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
Write-Output ("no_window_frames_test : " + $(if ($Fails -eq 0) { 'PASS' } else { "$Fails FAIL" }))
exit $(if ($Fails -eq 0) { 0 } else { 1 })
