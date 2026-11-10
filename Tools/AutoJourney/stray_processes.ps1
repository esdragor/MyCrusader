# stray_processes.ps1 : les processus detaches d'un worktree (chain/loop *.ps1, UnrealEditor, build en cours) - L7-4.
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-<nom>\Tools\AutoJourney\stray_processes.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-<nom> [-Stop]
#   Sans -Stop : liste (PID, nom, type, debut, ligne de commande) ; code de sortie 1 s'il reste un processus a arreter, 0 sinon.
#   -Stop      : arrete aussi ces processus (et leurs fils), puis relit la liste.
#   Un processus compte pour ce worktree si sa ligne de commande ou son executable contient le chemin du worktree.
#   Jamais comptes : ce script, son shell et leurs parents (la commande qui lance ce script contient le chemin). Les builds
#   (build.ps1, Build.bat, UnrealBuildTool, MSBuild, dotnet, AutomationTool) sont listes avec le type Build et ne comptent pas comme restes
#   (une compilation attendue n'est pas un reste) ; TOUTE la chaine est epargnee : le processus de build porteur du chemin ET tous ses
#   descendants (cmd, dotnet, cl, link... meme sans le chemin dans leur ligne). Un reste qui a lance un build est arrete SANS /T : ses
#   descendants hors chaine de build sont arretes un par un, ceux de la chaine restent. -IncludeBuild compte la chaine comme restes.
#   Les python/pythonw/py dont la ligne de commande contient le Root sont du type Python (L11-1, 2026-11-10 : 7 glade_budget.py orphelins) et sont arretes
#   comme les autres restes ; -PythonOnly se limite a eux. Le Root doit apparaitre en chemin COMPLET (suivi de \ / " ' espace ou fin) : jamais un autre
#   projet (ProjectShiva / AS_3) ni un worktree au prefixe voisin.
#   Test : stray_processes_test.ps1 (faux build / faux editeur / faux python, code 0 = vert).
# Le rapport d'agent a un champ PROCESSUS_DETACHES : colle ici la sortie (ou "aucun").
param(
  [Parameter(Mandatory=$true)][string]$Root,
  [switch]$Stop,
  [switch]$IncludeBuild,
  [switch]$PythonOnly   # L11-1 : ne liste / n'arrete QUE les python/pythonw du Root (session_gate Launch)
)
$ErrorActionPreference = 'Continue'
$rootFull = [IO.Path]::GetFullPath($Root).TrimEnd('\')
# Le Root doit etre un chemin COMPLET dans la ligne de commande : suivi d'un separateur, d'un guillemet, d'un espace ou de la fin
# (HeroesOfLiberty-Elrond ne doit pas attraper un processus de HeroesOfLiberty-ElrondBis ni d'un autre projet au prefixe voisin).
$rootRx = [regex]::new([regex]::Escape($rootFull) + '(?=$|[\\/"''\s;,)])', 'IgnoreCase')
# python / pythonw / python3.12 / py (lanceur) : un script Python detache (ex. glade_budget.py) survit a l'agent qui l'a lance.
function Test-PythonProc($p) { return ([string]$p.Name -match '^(py|pythonw?[\d.]*)(\.exe)?$') }

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

# Une ligne de commande ou un nom qui releve d'une compilation (Tools\build.ps1 -> Build.bat -> dotnet/UnrealBuildTool -> cl/link...).
function Test-BuildProc($p) {
  $cmd = [string]$p.CommandLine
  return ($cmd -match 'build\.ps1|Build\.bat|RunUAT|UnrealBuildTool|MSBuild|dotnet.*build|AutomationTool') -or
         ($p.Name -match '^(dotnet|cl|link|lib|rc|MSBuild|UnrealBuildTool|UnrealHeaderTool|AutomationTool|ShaderCompileWorker)(\.exe)?$')
}

# Tous les descendants d'un ensemble de PID, d'apres un instantane.
function Get-Descendants([object[]]$Snapshot, [int[]]$RootPids) {
  $kids = @{}
  foreach ($p in $Snapshot) { $pp = [int]$p.ParentProcessId; if (-not $kids.ContainsKey($pp)) { $kids[$pp] = @() }; $kids[$pp] += [int]$p.ProcessId }
  $seen = New-Object 'System.Collections.Generic.HashSet[int]'
  $queue = New-Object 'System.Collections.Generic.Queue[int]'
  foreach ($r in $RootPids) { $queue.Enqueue($r) }
  while ($queue.Count -gt 0) {
    $c = $queue.Dequeue()
    if ($kids.ContainsKey($c)) { foreach ($k in $kids[$c]) { if ($seen.Add($k)) { $queue.Enqueue($k) } } }
  }
  return $seen
}

# Toute la chaine d'un build en cours (processus de build porteurs du Root + tous leurs descendants) : jamais arretee.
function Get-BuildChain([object[]]$Snapshot) {
  $roots = @()
  foreach ($p in $Snapshot) {
    if ($skip.Contains([int]$p.ProcessId)) { continue }
    $cmd = [string]$p.CommandLine
    if ($cmd -and $rootRx.IsMatch($cmd) -and (Test-BuildProc $p)) { $roots += [int]$p.ProcessId }
  }
  $chain = New-Object 'System.Collections.Generic.HashSet[int]'
  foreach ($r in $roots) { [void]$chain.Add($r) }
  foreach ($d in (Get-Descendants $Snapshot $roots)) { [void]$chain.Add([int]$d) }
  return ,$chain
}

function Find-Strays {
  $found = @()
  $snap = @(Get-CimInstance Win32_Process)
  $chain = Get-BuildChain $snap
  foreach ($p in $snap) {
    if ($skip.Contains([int]$p.ProcessId)) { continue }
    $cmd = [string]$p.CommandLine
    $exe = [string]$p.ExecutablePath
    $hit = ($cmd -and $rootRx.IsMatch($cmd)) -or ($exe -and $exe.StartsWith($rootFull + '\', [StringComparison]::OrdinalIgnoreCase))
    if (-not $hit) { continue }
    $type = 'Autre'
    if ($chain.Contains([int]$p.ProcessId)) { $type = 'Build' }
    elseif (Test-PythonProc $p) { $type = 'Python' }
    elseif ($p.Name -match '^UnrealEditor|^UE4Editor|^UnrealEditor-Cmd') { $type = 'Editeur' }
    elseif ($p.Name -match '^(powershell|pwsh)' -and $cmd -match '\.ps1|chain|loop') { $type = 'Script' }
    if ($PythonOnly -and $type -ne 'Python') { continue }
    $found += [pscustomobject]@{ Pid = [int]$p.ProcessId; Name = [string]$p.Name; Type = $type; Start = $p.CreationDate; Cmd = $cmd }
  }
  return $found
}

# Arrete un reste ET ses descendants, sauf la chaine d'un build en cours (jamais de taskkill /T aveugle : un script
# detache qui a lance un build ne doit pas l'emporter avec lui). Le plus profond d'abord.
function Stop-Stray([int]$StrayPid) {
  $snap = @(Get-CimInstance Win32_Process)
  $chain = Get-BuildChain $snap
  $subtree = @($StrayPid) + @(Get-Descendants $snap @($StrayPid) | ForEach-Object { [int]$_ })
  [array]::Reverse($subtree)
  foreach ($id in $subtree) {
    if ((-not $IncludeBuild -and $chain.Contains([int]$id)) -or $skip.Contains([int]$id)) { continue }
    & taskkill /PID $id /F 2>&1 | Out-Null
  }
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
    Stop-Stray $s.Pid
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
