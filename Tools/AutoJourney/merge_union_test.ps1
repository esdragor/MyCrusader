# merge_union_test.ps1 (L20-1) : le .gitattributes du worktree fait fusionner deux journaux *InProgress.md sans conflit (merge=union).
# Cree un depot git jetable dans TEMP (jamais dans le worktree), y copie le .gitattributes, fait deux branches qui ajoutent chacune une ligne
# en fin du MEME CheckInProgress.md, fusionne.
#   - cas ROUGE-de-controle : .gitattributes SANS la regle merge=union -> la fusion doit conflictuer (prouve que le test mesure la regle)
#   - cas VERT : .gitattributes TEL QUEL (vrai fichier du worktree) -> aucun conflit et les deux lignes gardees
# Code 0 = tout vert. Un journal en union peut garder deux lignes d'etat contradictoires : relire apres fusion.
param(
  [string]$Attributes = (Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) '.gitattributes')
)
$ErrorActionPreference = 'Continue'
$fails = 0
function Check([bool]$ok, [string]$name) {
  if ($ok) { Write-Output "PASS $name" } else { Write-Output "FAIL $name"; $script:fails++ }
}

function Invoke-UnionCase([string]$AttributesText) {
  $dir = Join-Path ([IO.Path]::GetTempPath()) ("merge_union_" + [guid]::NewGuid().ToString('N').Substring(0, 8))
  New-Item -ItemType Directory -Path $dir | Out-Null
  try {
    $g = @('-C', $dir, '-c', 'user.name=t', '-c', 'user.email=t@t', '-c', 'core.safecrlf=false')
    git @g init -q -b base 2>&1 | Out-Null
    [IO.File]::WriteAllText("$dir\.gitattributes", $AttributesText)
    [IO.File]::WriteAllText("$dir\CheckInProgress.md", "# Journal`n- etat initial`n")
    git @g add -A 2>&1 | Out-Null
    git @g commit -q -m base 2>&1 | Out-Null
    foreach ($b in @('a', 'b')) {
      git @g checkout -q -b $b base 2>&1 | Out-Null
      [IO.File]::AppendAllText("$dir\CheckInProgress.md", "- ligne de $b`n")
      git @g commit -q -am "line $b" 2>&1 | Out-Null
    }
    git @g checkout -q a 2>&1 | Out-Null
    git @g merge --no-edit b 2>&1 | Out-Null
    $code = $LASTEXITCODE
    $text = [IO.File]::ReadAllText("$dir\CheckInProgress.md")
    return [pscustomobject]@{ Conflict = ($code -ne 0); HasA = ($text -match 'ligne de a'); HasB = ($text -match 'ligne de b') }
  } finally {
    Remove-Item -Recurse -Force $dir -ErrorAction SilentlyContinue
  }
}

if (-not (Test-Path $Attributes)) { Write-Output "FAIL .gitattributes introuvable : $Attributes"; exit 1 }
$real = [IO.File]::ReadAllText($Attributes)
$without = (($real -split "`r?`n") | Where-Object { $_ -notmatch 'InProgress\.md\s+merge=union' }) -join "`n"

$red = Invoke-UnionCase $without
Check $red.Conflict 'sans la regle merge=union : la fusion de deux journaux conflictue (controle)'

$green = Invoke-UnionCase $real
Check (-not $green.Conflict) 'avec le .gitattributes du worktree : aucun conflit'
Check ($green.HasA -and $green.HasB) 'avec le .gitattributes du worktree : les deux lignes sont gardees'

if ($fails -eq 0) { Write-Output 'merge_union_test : VERT'; exit 0 } else { Write-Output "merge_union_test : ROUGE ($fails)"; exit 1 }
