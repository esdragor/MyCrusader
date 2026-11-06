# stray_processes.ps1 : les processus detaches d'un worktree (chain/loop *.ps1, UnrealEditor, build en cours) - L7-4.
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-<nom>\Tools\AutoJourney\stray_processes.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-<nom> [-Stop]
#   Sans -Stop : liste (PID, nom, type, debut, ligne de commande) ; code de sortie 1 s'il reste un processus a arreter, 0 sinon.
#   -Stop      : arrete aussi ces processus (et leurs fils), puis relit la liste.
#   Un processus compte pour ce worktree si sa ligne de commande ou son executable contient le chemin du worktree.
#   Jamais comptes : ce script, son shell et leurs parents (la commande qui lance ce script contient le chemin). Les builds
#   (build.ps1, UnrealBuildTool, MSBuild) sont listes avec le type Build et ne comptent pas comme restes (une compilation attendue n'est pas un reste) ;
#   -IncludeBuild les compte.
# Le rapport d'agent a un champ PROCESSUS_DETACHES : colle ici la sortie (ou "aucun").
param(
  [Parameter(Mandatory=$true)][string]$Root,
  [switch]$Stop,
  [switch]$IncludeBuild
)
$ErrorActionPreference = 'Continue'
$rootFull = [IO.Path]::GetFullPath($Root).TrimEnd('\')

# Ce processus et toute sa chaine de parents.
$all = @(Get-CimInstance Win32_Process)
$byId = @{}
foreach ($p in $all) { $byId[[int]$p.ProcessId] = $p }
$skip = New-Object 'System.Collections.Generic.HashSet[int]'
$cur = [int]$PID
while ($cur -gt 0 -and -not $skip.Contains($cur)) {
  [void]$skip.Add($cur)
  if ($byId.ContainsKey($cur)) { $cur = [int]$byId[$cur].ParentProcessId } else { break }
}

function Find-Strays {
  $found = @()
  foreach ($p in @(Get-CimInstance Win32_Process)) {
    if ($skip.Contains([int]$p.ProcessId)) { continue }
    $cmd = [string]$p.CommandLine
    $exe = [string]$p.ExecutablePath
    $hit = ($cmd -and $cmd.IndexOf($rootFull, [StringComparison]::OrdinalIgnoreCase) -ge 0) -or ($exe -and $exe.StartsWith($rootFull + '\', [StringComparison]::OrdinalIgnoreCase))
    if (-not $hit) { continue }
    $type = 'Autre'
    if ($p.Name -match '^UnrealEditor|^UE4Editor|^UnrealEditor-Cmd') { $type = 'Editeur' }
    elseif ($cmd -match 'build\.ps1|UnrealBuildTool|MSBuild|dotnet.*build|AutomationTool') { $type = 'Build' }
    elseif ($p.Name -match '^(powershell|pwsh)' -and $cmd -match '\.ps1|chain|loop') { $type = 'Script' }
    $found += [pscustomobject]@{ Pid = [int]$p.ProcessId; Name = [string]$p.Name; Type = $type; Start = $p.CreationDate; Cmd = $cmd }
  }
  return $found
}

function Show([object[]]$List) {
  foreach ($s in $List) {
    $c = [string]$s.Cmd
    if ($c.Length -gt 140) { $c = $c.Substring(0, 140) + '...' }
    Write-Output ("  PID {0,-7} {1,-8} {2,-22} {3}  {4}" -f $s.Pid, $s.Type, $s.Name, $s.Start, $c)
  }
}

$list = @(Find-Strays)
$strays = @($list | Where-Object { $IncludeBuild -or $_.Type -ne 'Build' })
if ($list.Count -eq 0) { Write-Output "aucun processus detache pour $rootFull"; exit 0 }
Write-Output ("{0} processus pour {1} ({2} a arreter) :" -f $list.Count, $rootFull, $strays.Count)
Show $list
if ($strays.Count -eq 0) { Write-Output 'aucun reste (seulement des builds en cours)'; exit 0 }
if ($Stop) {
  foreach ($s in $strays) {
    & taskkill /PID $s.Pid /T /F 2>&1 | Out-Null
  }
  Start-Sleep -Milliseconds 700
  $left = @(Find-Strays | Where-Object { $IncludeBuild -or $_.Type -ne 'Build' })
  if ($left.Count -eq 0) { Write-Output ("arretes : {0}, aucun reste" -f (($strays | ForEach-Object { $_.Pid }) -join ', ')); exit 0 }
  Write-Output 'RESTENT :'
  Show $left
  exit 1
}
Write-Output 'relancer avec -Stop pour les arreter'
exit 1
