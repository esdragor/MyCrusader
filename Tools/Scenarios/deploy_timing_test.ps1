# D869b : le scenario deploy.txt doit prendre deploy_deploying / deploy_packing PENDANT le montage / demontage, avec au moins UN diese
# dans la barre (la barre de la case D a 8 crans : Filled = permille * 8 / 1000), quel que soit le debit d'images (fenetre cachee
# ~150 images/s, fenetre visible ~60, machine chargee ~30). Un `frames N` entre la touche D et la capture donne un temps qui suit le
# debit d'images : il est mesure ici entre 150 et 30 images/s ; un `wait X` est exact (temps du jeu).
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\Scenarios\deploy_timing_test.ps1 [-Root <worktree>] ; code 0 = vert.
param([string] $Root = (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent))

$Scenario = Join-Path $Root "Tools\Scenarios\deploy.txt"
$Lines = @(Get-Content $Scenario)

function Get-ProposedMs([string]$Name, [int]$Fallback) {
    $Hit = Get-ChildItem (Join-Path $Root "Source\HeroesOfLiberty\Siege") -Filter "HOLDeployRules.*" |
        Select-String -Pattern ("{0}\D{{1,12}}(\d+)" -f $Name) | Select-Object -First 1
    if ($Hit) { return [int]$Hit.Matches[0].Groups[1].Value }
    return $Fallback
}

$DeployMs = Get-ProposedMs "ProposedDeployMs" 1500
$PackMs = Get-ProposedMs "ProposedPackMs" 1000
$Fail = 0

function Test-Capture([string]$Capture, [int]$DurationMs) {
    $Index = [array]::IndexOf($Lines, ($Lines | Where-Object { $_ -match ("^capture\s+{0}\s*$" -f $Capture) } | Select-Object -First 1))
    if ($Index -lt 0) { Write-Host "FAIL $Capture : capture absente de deploy.txt"; return 1 }
    $Key = -1
    for ($i = $Index; $i -ge 0; $i--) { if ($Lines[$i] -match '^hol\.UI\.Key\s+D\b') { $Key = $i; break } }
    if ($Key -lt 0) { Write-Host "FAIL $Capture : pas de touche D avant la capture"; return 1 }
    $MinS = 0.0; $MaxS = 0.0
    for ($i = $Key + 1; $i -lt $Index; $i++) {
        if ($Lines[$i] -match '^wait\s+([\d.]+)') { $MinS += [double]$Matches[1]; $MaxS += [double]$Matches[1] }
        elseif ($Lines[$i] -match '^frames\s+(\d+)') { $MinS += [int]$Matches[1] / 150.0; $MaxS += [int]$Matches[1] / 30.0 }
    }
    $Duration = $DurationMs / 1000.0
    $HashesLow = [Math]::Floor([Math]::Min(1000, $MinS / $Duration * 1000) * 8 / 1000)
    $Ok = ($HashesLow -ge 1) -and ($MaxS -lt $Duration)
    Write-Host ("{0} {1} : entre {2:N2} s et {3:N2} s apres la touche D (duree {4:N2} s), au moins {5} diese(s) a coup sur" -f $(if ($Ok) { "PASS" } else { "FAIL" }), $Capture, $MinS, $MaxS, $Duration, $HashesLow)
    if ($Ok) { return 0 } else { return 1 }
}

$Fail += Test-Capture "deploy_deploying" $DeployMs
$Fail += Test-Capture "deploy_packing" $PackMs
if ($Fail -gt 0) { Write-Host "deploy_timing_test : FAIL ($Fail)"; exit 1 }
Write-Host "deploy_timing_test : PASS"
exit 0
