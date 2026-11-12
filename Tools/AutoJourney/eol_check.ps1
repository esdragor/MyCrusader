# eol_check.ps1 : L8-6. Liste les fichiers suivis dont l'arbre de travail a des fins de ligne MELEES (CRLF et LF dans le meme fichier) ;
# ces fichiers font refuser les Edit multi-lignes. .gitattributes (text=auto eol=crlf) garde l'index en LF et l'arbre en CRLF : rien
# n'est reecrit dans le depot. -Fix remet l'arbre de travail en CRLF partout (octets : BOM et accents intacts, aucun diff git).
# Usage : powershell -ExecutionPolicy Bypass -File <worktree>\Tools\AutoJourney\eol_check.ps1 [-Root <worktree>] [-Fix]
# Code de sortie : 1 s'il reste des fichiers mixtes (sans -Fix), 0 sinon.
param(
  [string]$Root = '',
  [switch]$Fix
)
if (-not $Root) { $Root = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)) }   # <worktree>\Tools\AutoJourney\eol_check.ps1
# 2026-11-12 (L9-4, session_gate Close le lance sur chaque worktree) : les binaires sont exclus du releve, git lisait leur contenu pour rien
# (Haldir : 3303 fichiers en 47 s, 2 s sans Content ; aucun fichier mixte dans Content). Un binaire n'a pas de fins de ligne a corriger.
$BinarySpecs = @('.', ':(exclude)*.uasset', ':(exclude)*.umap', ':(exclude)*.png', ':(exclude)*.jpg', ':(exclude)*.wav', ':(exclude)*.ogg', ':(exclude)*.fbx', ':(exclude)*.dll', ':(exclude)*.exe', ':(exclude)*.zip')
$mixed = @(git -C $Root ls-files --eol -- $BinarySpecs 2>$null | Where-Object { $_ -match '\bw/mixed\b' } | ForEach-Object { ($_ -split "`t", 2)[1] })
if ($mixed.Count -eq 0) { Write-Output "aucun fichier mixte dans $Root"; exit 0 }
foreach ($f in $mixed) {
  $path = Join-Path $Root $f
  if (-not $Fix) { Write-Output "MIXTE $f"; continue }
  $in = [IO.File]::ReadAllBytes($path)
  $out = New-Object 'System.Collections.Generic.List[byte]' ($in.Length + 1024)
  for ($i = 0; $i -lt $in.Length; $i++) {
    $b = $in[$i]
    if ($b -eq 10 -and ($i -eq 0 -or $in[$i - 1] -ne 13)) { $out.Add(13); $out.Add(10) }   # LF seul -> CRLF
    else { $out.Add($b) }                                                                    # CRLF et le reste inchanges
  }
  [IO.File]::WriteAllBytes($path, $out.ToArray())
  # git prend un changement de taille pour une modification (status ' M' alors que diff est vide) : si le contenu converti est celui de l'index,
  # `git add` ne met en scene que le releve (aucun changement de contenu) ; un fichier vraiment modifie par ailleurs n'est jamais mis en scene.
  git -C $Root diff --quiet -- $f 2>$null
  if ($LASTEXITCODE -eq 0) { git -C $Root add -- $f 2>$null }
  Write-Output "CORRIGE $f"
}
if ($Fix) { exit 0 } else { Write-Output "$($mixed.Count) fichier(s) mixte(s) : relancer avec -Fix"; exit 1 }
