# play_exit_test.ps1 : test d'effet du CODE DE SORTIE de Tools\Scenarios\play.ps1 -Hidden (SAURON-1124sf ; CELEBORN-1124se : play.ps1 rendait 0 meme quand
# le scenario echouait, journal "LogHOLTest: Error: Test sequence: timeout at step 43"). Aucun vrai editeur : meme faux moteur que play_hidden_test.ps1
# (powershell.exe renomme UnrealEditor.exe + HOL_PLAY_PREARGS) ; le faux editeur ecrit (ou non) le journal Saved\Logs\HeroesOfLiberty.log de la racine, puis sort.
#   1. journal neuf avec "Test sequence: timeout at step N" : play.ps1 rend un code NON NUL et le dit.
#   2. journal neuf sans cette ligne : play.ps1 rend 0.
#   3. journal PERIMEE (une ligne timeout d'une partie precedente, le faux editeur n'ecrit rien) : play.ps1 rend 0 (on ne condamne pas un vieux journal).
# Run : powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play_exit_test.ps1   (une vingtaine de secondes). Code 0 = PASS.
$ErrorActionPreference = 'Continue'
$Play = Join-Path $PSScriptRoot 'play.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Text) { if ($Ok) { Write-Host "  OK   $Text" } else { Write-Host "  FAIL $Text"; $script:Fails++ } }
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('playexit-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Root = Join-Path $Tmp 'root'
$Eng = Join-Path $Tmp 'engine'
$LogDir = Join-Path $Root 'Saved\Logs'
$Log = Join-Path $LogDir 'HeroesOfLiberty.log'
New-Item -ItemType Directory -Force (Join-Path $Root 'Tools\Scenarios') | Out-Null
New-Item -ItemType Directory -Force $LogDir | Out-Null
New-Item -ItemType Directory -Force $Eng | Out-Null
Set-Content (Join-Path $Root 'Tools\Scenarios\fake.txt') @('# fake', 'note fin', 'quit') -Encoding UTF8
Set-Content (Join-Path $Root 'HeroesOfLiberty.uproject') '{"EngineAssociation":"x"}' -Encoding UTF8
Copy-Item (Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe') (Join-Path $Eng 'UnrealEditor.exe')
# faux editeur : Mode = fail (ecrit la ligne timeout), ok (ecrit un journal sans erreur), stale (n'ecrit rien)
$Fake = Join-Path $Tmp 'fake_editor.ps1'
$FakeText = @(
    'param([string]$Mode = "ok", [string]$LogFile = "", [Parameter(ValueFromRemainingArguments = $true)] $Rest)',
    'if ($Mode -eq "fail") { Set-Content -LiteralPath $LogFile -Value @("LogHOLTest: Test sequence: waitlog at (-17450, -1350)", "LogHOLTest: Error: Test sequence: timeout at step 43: waitlog at (-17450, -1350)") -Encoding UTF8 }',
    'if ($Mode -eq "ok") { Set-Content -LiteralPath $LogFile -Value @("LogHOLTest: Test sequence: step 43 done", "LogHOLTest: Test sequence: finished") -Encoding UTF8 }'
)
Set-Content $Fake $FakeText -Encoding UTF8
function Run([string]$Mode) {
    $env:HOL_PLAY_PREARGS = "-NoProfile -File `"$Fake`" -Mode $Mode -LogFile `"$Log`""
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $Play fake -Root $Root -EngineDir $Eng -Hidden -TimeoutSec 30 2>&1 | Out-String
    $code = $LASTEXITCODE
    $env:HOL_PLAY_PREARGS = $null
    return [pscustomobject]@{ Out = $out; Code = $code }
}
function Flat([string]$Text) { return ($Text -replace '\s+', ' ').Trim() }

# 1. echec de sequence : code non nul
if (Test-Path $Log) { Remove-Item -LiteralPath $Log -Force }
$f = Run 'fail'
Check ($f.Code -ne 0) ("journal avec 'Test sequence: timeout at step 43' : play.ps1 rend un code non nul (code {0} ; sortie : {1})" -f $f.Code, (Flat $f.Out))
Check ($f.Out -match 'timeout at step 43') ("play.ps1 recopie la ligne d'echec (sortie : " + (Flat $f.Out) + ")")

# 2. journal sans echec : 0
$o = Run 'ok'
Check ($o.Code -eq 0) ("journal sans echec : play.ps1 rend 0 (code {0} ; sortie : {1})" -f $o.Code, (Flat $o.Out))

# 3. journal perime (une ligne timeout d'avant le lancement, rien d'ecrit par ce lancement) : 0
Set-Content -LiteralPath $Log -Value @('LogHOLTest: Error: Test sequence: timeout at step 7: waitlog old') -Encoding UTF8
(Get-Item -LiteralPath $Log).LastWriteTime = (Get-Date).AddHours(-2)
$s = Run 'stale'
Check ($s.Code -eq 0) ("journal perime (avant le lancement) : play.ps1 rend 0 (code {0} ; sortie : {1})" -f $s.Code, (Flat $s.Out))

Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($Tmp) -and $_.ProcessId -ne $PID } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
Start-Sleep -Milliseconds 500
if (Test-Path $Tmp) { Remove-Item -LiteralPath $Tmp -Recurse -Force -ErrorAction SilentlyContinue }
if ($Fails -eq 0) { Write-Host 'play_exit_test: PASS'; exit 0 } else { Write-Host "play_exit_test: FAIL ($Fails)"; exit 1 }
