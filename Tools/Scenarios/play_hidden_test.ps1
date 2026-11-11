# play_hidden_test.ps1 : test d'effet de Tools\Scenarios\play.ps1 -Hidden (Auto_Journey_Checklist L11-6). Aucun vrai editeur : un faux moteur (powershell.exe renomme
# UnrealEditor.exe + HOL_PLAY_PREARGS = point d'entree de test) qui lance un petit-fils qui dort.
#   1. -Hidden -DryRun : la ligne de commande centre le curseur virtuel (hol.Camera.TestCursor 0.5 0.5) AVANT hol.Scenario ; sans -Hidden, rien de tel.
#   2. -Hidden : play.ps1 ne rend la main qu'apres avoir ferme l'editeur ET ses fils (aucun processus du faux moteur ne reste).
#   3. -Hidden, le faux editeur sort tout de suite en laissant un petit-fils orphelin : l'orphelin est ferme aussi.
#   4. -Hidden -NoWait : ancien comportement (rend la main tout de suite, l'editeur vit encore) ; le test le ferme ensuite lui-meme.
# Run : powershell -ExecutionPolicy Bypass -File Tools\Scenarios\play_hidden_test.ps1   (une vingtaine de secondes). Code 0 = PASS.
$ErrorActionPreference = 'Continue'
$Play = Join-Path $PSScriptRoot 'play.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Text) { if ($Ok) { Write-Host "  OK   $Text" } else { Write-Host "  FAIL $Text"; $script:Fails++ } }
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('playhidden-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Root = Join-Path $Tmp 'root'
$Eng = Join-Path $Tmp 'engine'
New-Item -ItemType Directory -Force (Join-Path $Root 'Tools\Scenarios') | Out-Null
New-Item -ItemType Directory -Force $Eng | Out-Null
Set-Content (Join-Path $Root 'Tools\Scenarios\fake.txt') '# fake' -Encoding UTF8
Set-Content (Join-Path $Root 'HeroesOfLiberty.uproject') '{"EngineAssociation":"x"}' -Encoding UTF8
Copy-Item (Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe') (Join-Path $Eng 'UnrealEditor.exe')
# faux editeur : Mode = stay (dort lui aussi) ou orphan (sort apres 3 s en laissant le petit-fils)
$Fake = Join-Path $Tmp 'fake_editor.ps1'
$FakeText = @(
    'param([string]$Mode = "stay", [Parameter(ValueFromRemainingArguments = $true)] $Rest)',
    'Start-Process powershell -ArgumentList "-NoProfile", "-Command", "Start-Sleep 120" -WindowStyle Hidden',
    'if ($Mode -eq "stay") { Start-Sleep 120 } else { Start-Sleep 3 }'
)
Set-Content $Fake $FakeText -Encoding UTF8
function Alive { @(Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($Tmp) -and $_.ProcessId -ne $PID }) }
function Kill-Leftovers { Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -and $_.CommandLine.Contains($Tmp) -and $_.ProcessId -ne $PID } | ForEach-Object { & taskkill /PID $_.ProcessId /F 2>&1 | Out-Null } }
function Run([string[]]$PlayArgs, [string]$Mode) {
    $env:HOL_PLAY_PREARGS = "-NoProfile -File `"$Fake`" -Mode $Mode"
    $sw = [Diagnostics.Stopwatch]::StartNew()
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $Play fake -Root $Root -EngineDir $Eng @PlayArgs 2>&1 | Out-String
    $env:HOL_PLAY_PREARGS = $null
    return [pscustomobject]@{ Out = $out; Sec = $sw.Elapsed.TotalSeconds }
}
function Flat([string]$Text) { return ($Text -replace '\s+', ' ').Trim() }

# 1. ligne de commande
$d = Run @('-Hidden', '-DryRun') 'stay'
$iCursor = $d.Out.IndexOf('hol.Camera.TestCursor 0.5 0.5')
$iScen = $d.Out.IndexOf('hol.Scenario fake')
Check ($iCursor -ge 0 -and $iScen -gt $iCursor) ("-Hidden -DryRun : curseur virtuel centre avant hol.Scenario (sortie : " + (Flat $d.Out) + ")")
Check ((Alive).Count -eq 0) '-DryRun ne lance rien'
$v = Run @('-DryRun') 'stay'
Check ($v.Out -match 'hol\.Scenario fake' -and $v.Out -notmatch 'TestCursor') 'sans -Hidden (fenetre de l utilisateur) : pas de curseur virtuel force'

# 2. -Hidden ferme l'editeur et ses fils
$r = Run @('-Hidden', '-TimeoutSec', '5') 'stay'
$left = (Alive).Count
Check ($left -eq 0) ("-Hidden : plus aucun processus du faux moteur apres play.ps1 ($left restent ; sortie : " + (Flat $r.Out) + ")")
Check ($r.Sec -ge 4 -and $r.Sec -lt 8) ("-Hidden attend l editeur et s arrete a -TimeoutSec 5 (+ demarrage de powershell, au plus 3 s ; avant 2026-11-11 : 15-18 s de trop, Get-CimInstance a chaque passe) : {0:N1} s" -f $r.Sec)
Kill-Leftovers

# 3. editeur sorti, petit-fils orphelin
$o = Run @('-Hidden', '-TimeoutSec', '8') 'orphan'
$left = (Alive).Count
Check ($left -eq 0) ("-Hidden : le petit-fils orphelin est ferme aussi ($left restent)")
Kill-Leftovers

# 4. -NoWait : rend la main tout de suite
$n = Run @('-Hidden', '-NoWait') 'stay'
Check ($n.Sec -lt 15) ("-NoWait rend la main tout de suite ({0:N1} s)" -f $n.Sec)
$al = @(Alive)
Check ($al.Count -gt 0) ('-NoWait : l editeur vit encore (reste a charge de stray_processes.ps1 -Stop) ; sortie : ' + (Flat $n.Out))
Kill-Leftovers

Start-Sleep -Milliseconds 500
if (Test-Path $Tmp) { Remove-Item -LiteralPath $Tmp -Recurse -Force -ErrorAction SilentlyContinue }
if ($Fails -eq 0) { Write-Host 'play_hidden_test: PASS'; exit 0 } else { Write-Host "play_hidden_test: FAIL ($Fails)"; exit 1 }
