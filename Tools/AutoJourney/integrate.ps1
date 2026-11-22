# integrate.ps1 : script d'integration unique de la session principale (L19-9). ASCII seulement (pas de BOM a garder).
# Usage (une ligne) :
#   powershell -ExecutionPolicy Bypass -File <wt>\Tools\AutoJourney\integrate.ps1 -Root <worktree d'integration> -Branch <branche a creer depuis main> [-Reds sha1,sha2] [-Greens br1,br2] [-Parts net,build] [-DryRun]
# Deroule : branche neuve depuis main ; cherry-pick des rouges (commits "test seul") ; build (seulement si un .cpp / .h / .hpp / .inl / .cs a change
# dans les rouges) ; quick_check -Parts tests (les rouges DOIVENT y etre vus) ; remise a main (reset --hard du sha de main au depart : "verts depuis
# main, rouges retires", L20-6 ; les verts portent les memes commits, un bouchon de rouge remplace dans sa branche ne conflite plus) ; merge des verts (tips) un par un ; build (seulement si C++ dans
# un vert) ; quick_check -Parts tests (doit etre vert) ; quick_check -Parts <Parts> ; resume court (10 lignes) imprime ET ecrit dans
# <Root>\Docs\Integrate_<Branch>.txt (sortie complete : Docs\Integrate_<Branch>.log).
# -DryRun : imprime le plan (lignes "PLAN <n> <genre> : <commande>" ou "... : SKIP (<raison>)") sans rien creer ni lancer (lit seulement git).
# `powershell -File` passe une liste en UN SEUL argument ("a,b") : -Reds, -Greens et -Parts sont des chaines coupees aux virgules ici
# (piege du 2026-11-20 : "liste lue comme un seul nom"). -Parts est transmis tel quel a quick_check (qui coupe lui aussi).
# Codes : 0 PASS ; 1 FAIL (build, tests, ou compte 0/0 / aucun compte de tests lu, rouges comme verts) ; 2 conflit (etat laisse tel quel, fichiers en conflit imprimes) ; 3 usage / reference inconnue.
param(
  [string] $Root = '',
  [string] $Branch = '',
  [string] $Reds = '',
  [string] $Greens = '',
  [string] $Parts = 'net,build',
  [switch] $DryRun,
  [switch] $SkipBuildTests   # RESERVE AU TEST (integrate_test.ps1) : saute build.ps1 et quick_check (etapes build / tests / parts), garde toute la partie git
)
$ErrorActionPreference = 'Continue'
if (-not $Root) { $Root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path }
$Root = $Root.TrimEnd('\')
if (-not $Branch) { Write-Output 'integrate : ERROR -Branch est obligatoire'; exit 3 }
function Split-List([string] $Text) { return @($Text -split '[,;\s]+' | Where-Object { $_ }) }
function Git-Out { return @(& git.exe -C $Root @args 2>&1 | ForEach-Object { "$_" }) }
$RedList = Split-List $Reds
$GreenList = Split-List $Greens
$PartsText = (Split-List $Parts) -join ','
if (-not $PartsText) { $PartsText = 'net,build' }
$CppPattern = '\.(cpp|h|hpp|inl|cs)$'

# References: each red is a commit, each green a branch / commit; the integration branch must be new.
$Bad = @()
foreach ($RedItem in $RedList) { Git-Out rev-parse --verify -q "$RedItem^{commit}" | Out-Null; if ($LASTEXITCODE -ne 0) { $Bad += "rouge '$RedItem'" } }
foreach ($GreenItem in $GreenList) { Git-Out rev-parse --verify -q "$GreenItem^{commit}" | Out-Null; if ($LASTEXITCODE -ne 0) { $Bad += "vert '$GreenItem'" } }
Git-Out rev-parse --verify -q 'main^{commit}' | Out-Null; if ($LASTEXITCODE -ne 0) { $Bad += "branche 'main' (Root '$Root' n'est pas un depot git ?)" }
Git-Out rev-parse --verify -q "refs/heads/$Branch" | Out-Null; if ($LASTEXITCODE -eq 0) { $Bad += "branche d'integration '$Branch' existe deja (il faut une branche NEUVE)" }
if ($Bad.Count) { Write-Output "integrate : ERROR reference inconnue ou invalide : $($Bad -join ' ; ')"; exit 3 }

# Does the change set touch C++ ?
$RedsCpp = $false
foreach ($RedItem in $RedList) { if (@(Git-Out diff-tree --no-commit-id --name-only -r $RedItem) -match $CppPattern) { $RedsCpp = $true } }
$GreensCpp = $false
foreach ($GreenItem in $GreenList) { if (@(Git-Out diff --name-only "main...$GreenItem") -match $CppPattern) { $GreensCpp = $true } }

$PS = 'powershell -NoProfile -ExecutionPolicy Bypass -File'
$BuildCmd = "$PS $Root\Tools\build.ps1 -Root $Root -LiveCoding=0 -Wait -Priority merge"
$QcCmd = "$PS $Root\Tools\Tests\quick_check.ps1"
$SummaryPath = Join-Path $Root "Docs\Integrate_$Branch.txt"
$LogPath = Join-Path $Root "Docs\Integrate_$Branch.log"

# The plan: one record per step. Kind is what the summary and the test read; Skip is the reason, or '' when the step runs.
$Plan = New-Object System.Collections.ArrayList
function Add-Step([string] $Kind, [string] $Text, [string] $Skip = '', [scriptblock] $Action = $null) {
  if ($SkipBuildTests -and -not $Skip -and ($Kind -in 'build', 'tests', 'parts')) { $Skip = '-SkipBuildTests (reserve au test)' }
  [void] $Plan.Add([pscustomobject]@{ Kind = $Kind; Text = $Text; Skip = $Skip; Action = $Action })
}
Add-Step 'branch' "git -C $Root switch -c $Branch main"
if ($RedList.Count) { foreach ($RedItem in $RedList) { Add-Step 'red' "git -C $Root cherry-pick $RedItem" } }
else { Add-Step 'red' 'cherry-pick' 'pas de rouge' }
Add-Step 'build' $BuildCmd $(if (-not $RedList.Count) { 'pas de rouge' } elseif (-not $RedsCpp) { 'pas de C++ (.cpp/.h) dans les rouges' } else { '' })
Add-Step 'tests' "$QcCmd -Parts tests" $(if ($RedList.Count) { '' } else { 'pas de rouge' })
# L20-6: the reds only served to see the tests fail; the greens carry the same commits (a red may hold a stub that a later commit of its own
# branch replaces -> conflict if the cherry-picks stay). The integration branch holds only what this script created: back to main, then the greens.
$MainSha = (Git-Out rev-parse 'main^{commit}' | Select-Object -First 1).Trim()
Add-Step 'reset' "git -C $Root reset --hard $MainSha" $(if ($RedList.Count) { '' } else { 'pas de rouge' })
if ($GreenList.Count) { foreach ($GreenItem in $GreenList) { Add-Step 'green' "git -C $Root merge --no-ff --no-edit $GreenItem" } }
else { Add-Step 'green' 'merge' 'pas de vert' }
Add-Step 'build' $BuildCmd $(if (-not $GreenList.Count) { 'pas de vert' } elseif (-not $GreensCpp) { 'pas de C++ (.cpp/.h) dans les verts' } else { '' })
Add-Step 'tests' "$QcCmd -Parts tests"
Add-Step 'parts' "$QcCmd -Parts $PartsText"
Add-Step 'summary' "ecrit $SummaryPath"

if ($DryRun) {
  $Number = 0
  foreach ($Step in $Plan) {
    $Number++
    if ($Step.Skip) { Write-Output "PLAN $Number $($Step.Kind) : SKIP ($($Step.Skip))" } else { Write-Output "PLAN $Number $($Step.Kind) : $($Step.Text)" }
  }
  Write-Output "integrate : DryRun (rien cree, rien lance) ; rouges=$($RedList.Count) verts=$($GreenList.Count) C++ rouges=$RedsCpp verts=$GreensCpp"
  exit 0
}

# ---------------------------------------------------------------- real run
if (-not (Test-Path (Join-Path $Root 'Docs'))) { New-Item -ItemType Directory -Path (Join-Path $Root 'Docs') -Force | Out-Null }
Set-Content -Path $LogPath -Value "integrate $Branch $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -Encoding ASCII
$Started = Get-Date
$Rows = [ordered]@{}        # stage -> text of the summary line
$RedsSeen = @()
$RedsNote = ''
function Log([string] $Line) { Write-Output $Line; Add-Content -Path $LogPath -Value $Line -Encoding ASCII }
function Run-Native([string] $Command) {
  # Runs one command line, mirrors the output to the log, returns @{ Code; Out }.
  $Collected = New-Object System.Collections.ArrayList
  if ($SkipBuildTests -and ($Command -match 'build\.ps1|quick_check\.ps1')) { Add-Content -Path $LogPath -Value "SKIP (-SkipBuildTests) $Command" -Encoding ASCII; return @{ Code = 0; Out = @('SKIP (-SkipBuildTests)') } }
  & cmd.exe /c "$Command 2>&1" | ForEach-Object { [void] $Collected.Add("$_"); Add-Content -Path $LogPath -Value "$_" -Encoding ASCII }
  return @{ Code = $LASTEXITCODE; Out = @($Collected) }
}
function Write-Summary([string] $Verdict) {
  $Lines = @("INTEGRATE $Branch : $Verdict ($([int]((Get-Date) - $Started).TotalSeconds) s, $(Get-Date -Format 'yyyy-MM-dd HH:mm'))")
  foreach ($Key in 'branche+rouges', 'build rouges', 'tests rouges', 'remise a main', 'merge verts', 'build verts', 'tests verts', "parts $PartsText") {
    $Lines += "  $Key : $(if ($Rows.Contains($Key)) { $Rows[$Key] } else { 'NON LANCE' })"
  }
  $Lines += "  rouges vus : $(if ($RedsSeen.Count) { $RedsSeen -join ', ' } else { if ($RedsNote) { $RedsNote } else { 'aucun' } }) ; sortie complete : Docs\Integrate_$Branch.log"
  Set-Content -Path $SummaryPath -Value $Lines -Encoding ASCII
  $Lines | ForEach-Object { Write-Output $_ }
}
function Stop-Conflict([string] $Where) {
  $Files = @(Git-Out diff --name-only --diff-filter=U)
  Log "integrate : CONFLIT pendant $Where ; etat laisse tel quel. Fichiers en conflit :"
  $Files | ForEach-Object { Log "  $_" }
  $Rows['conflit'] = "$Where : $($Files -join ', ')"
  Write-Summary 'FAIL (conflit)'
  exit 2
}
function Stage-Time([datetime] $Since) { return "$([int]((Get-Date) - $Since).TotalSeconds) s" }
function Qc-Result($Run) {
  $Text = $Run.Out -join "`n"
  $Count = ''
  if ($Text -match 'tests (\d+)/(\d+) : (PASS|FAIL)') { $Count = "$($Matches[1])/$($Matches[2]) tests" }
  $Names = @()
  if ($Text -match 'all green \(failed: ([^)]*)\)') { $Names = @($Matches[1] -split ',\s*' | Where-Object { $_ }) }
  $Known = ($Text -match 'rouge connu') -and ($Text -notmatch 'REGRESSION')
  # 0/0 (a script did not compile: "scripts compiled" KO) or no count read at all = no test ran: never a verdict (integ-50, 2026-11-22).
  $NoTests = $true
  if ($Text -match 'tests (\d+)/(\d+) : (PASS|FAIL)') { $NoTests = ([int]$Matches[2] -eq 0) }
  $KoLines = @($Run.Out | Where-Object { ($_ -match 'scripts compiled') -or (($_ -match '\.as') -and ($_ -match '\[KO\]|error')) } | ForEach-Object { $_.Trim() } | Select-Object -First 3)
  $Ko = ($KoLines | ForEach-Object { if ($_.Length -gt 400) { $_.Substring(0, 400) } else { $_ } }) -join ' | '
  return @{ Ok = ($Run.Code -eq 0) -or $Known; Count = $Count; Names = $Names; NoTests = ($NoTests -and -not $SkipBuildTests); Ko = $Ko }
}

# 1. Branch + reds.
$T0 = Get-Date
$Out = Git-Out switch -c $Branch main
if ($LASTEXITCODE -ne 0) { Log "integrate : ERROR switch -c : $($Out -join ' ')"; Write-Summary 'FAIL (branche)'; exit 3 }
$Picked = 0
foreach ($RedItem in $RedList) {
  $Out = Git-Out cherry-pick $RedItem
  Add-Content -Path $LogPath -Value ($Out -join "`n") -Encoding ASCII
  if ($LASTEXITCODE -ne 0) { Stop-Conflict "cherry-pick $RedItem" }
  $Picked++
}
$Rows['branche+rouges'] = "PASS ($Picked cherry-pick, $(Stage-Time $T0))"
Log "integrate : branche $Branch creee, $Picked rouge(s) appliques"

function Build-Stage([string] $Key, [bool] $Needed, [string] $Why) {
  if (-not $Needed) { $Rows[$Key] = "SKIP ($Why)"; Log "integrate : $Key SKIP ($Why)"; return }
  $Since = Get-Date
  Log "integrate : $Key ..."
  $Run = Run-Native $BuildCmd
  if ($Run.Code -ne 0) { $Rows[$Key] = "FAIL (code $($Run.Code), $(Stage-Time $Since))"; Write-Summary 'FAIL (build)'; exit 1 }
  $Rows[$Key] = "PASS ($(Stage-Time $Since))"
}

# 2-3. Build + tests of the reds: the reds MUST be seen red here.
if ($RedList.Count) {
  Build-Stage 'build rouges' $RedsCpp 'pas de C++ dans les rouges'
  $Since = Get-Date
  Log 'integrate : tests des rouges ...'
  $Run = Run-Native "$QcCmd -Parts tests"
  $Result = Qc-Result $Run
  $RedsSeen = $Result.Names
  if ($Result.NoTests) {
    $RedsNote = 'aucun (0/0)'
    $Rows['tests rouges'] = "FAIL $(if ($Result.Count) { $Result.Count } else { 'aucun nombre de tests lu' }) : aucun test lance, les rouges ne sont pas vus ($(Stage-Time $Since))$(if ($Result.Ko) { ' : ' + $Result.Ko })"
    Write-Summary 'FAIL (tests rouges 0/0)'
    exit 1
  }
  $Rows['tests rouges'] = "$(if ($Result.Ok) { 'PASS : AUCUN rouge vu (les rouges sont-ils de vrais rouges ?)' } else { 'FAIL attendu' }) $($Result.Count) ($(Stage-Time $Since))"
} else { $Rows['build rouges'] = 'SKIP (pas de rouge)'; $Rows['tests rouges'] = 'SKIP (pas de rouge)' }

# 3b. Back to main WITHOUT the cherry-picked reds (they served to see the tests fail; the greens carry the same commits).
if ($RedList.Count) {
  $Out = Git-Out reset --hard $MainSha
  Add-Content -Path $LogPath -Value ($Out -join "`n") -Encoding ASCII
  if ($LASTEXITCODE -ne 0) { Log "integrate : ERROR reset --hard $MainSha : $($Out -join ' ')"; Write-Summary 'FAIL (reset)'; exit 3 }
  $Rows['remise a main'] = "PASS (verts depuis main, rouges retires : reset --hard $($MainSha.Substring(0, 7)))"
  Log "integrate : verts depuis main, rouges retires (reset --hard $($MainSha.Substring(0, 7)))"
} else { $Rows['remise a main'] = 'SKIP (pas de rouge)' }

# 4. Merge the greens, one by one (stop at the first conflict).
$Since = Get-Date
$Merged = 0
foreach ($GreenItem in $GreenList) {
  $Out = Git-Out merge --no-ff --no-edit $GreenItem
  Add-Content -Path $LogPath -Value ($Out -join "`n") -Encoding ASCII
  if ($LASTEXITCODE -ne 0) { Stop-Conflict "merge $GreenItem" }
  $Merged++
}
$Rows['merge verts'] = "PASS ($Merged merge, $(Stage-Time $Since))"
Log "integrate : $Merged vert(s) fusionne(s)"

# 5-7. Build, tests (must be green), final parts.
Build-Stage 'build verts' $GreensCpp 'pas de C++ dans les verts'
$Since = Get-Date
Log 'integrate : tests (apres les verts) ...'
$Run = Run-Native "$QcCmd -Parts tests"
$Result = Qc-Result $Run
if ($Result.NoTests) { $Rows['tests verts'] = "FAIL $(if ($Result.Count) { $Result.Count } else { 'aucun nombre de tests lu' }) : aucun test lance ($(Stage-Time $Since))$(if ($Result.Ko) { ' : ' + $Result.Ko })"; Write-Summary 'FAIL (tests verts 0/0)'; exit 1 }
if (-not $Result.Ok) { $Rows['tests verts'] = "FAIL $($Result.Count) ($(Stage-Time $Since)) : $($Result.Names -join ', ')"; Write-Summary 'FAIL (tests)'; exit 1 }
$Rows['tests verts'] = "PASS $($Result.Count) ($(Stage-Time $Since))"
$Since = Get-Date
Log "integrate : parts $PartsText ..."
$Run = Run-Native "$QcCmd -Parts $PartsText"
$Result = Qc-Result $Run
$PartsKey = "parts $PartsText"
if (-not $Result.Ok) { $Rows[$PartsKey] = "FAIL ($(Stage-Time $Since))"; Write-Summary 'FAIL (parts)'; exit 1 }
$Rows[$PartsKey] = "PASS ($(Stage-Time $Since))"
Write-Summary 'PASS'
exit 0
