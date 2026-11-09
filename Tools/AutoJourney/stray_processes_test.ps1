# stray_processes_test.ps1 : stray_processes.ps1 -Stop doit epargner toute la chaine d'un build en cours
# (build.ps1 / Build.bat / UnrealBuildTool et leurs fils) et tuer les faux editeurs / scripts detaches.
# Usage : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-<nom>\Tools\AutoJourney\stray_processes_test.ps1
# Aucun vrai build ni vrai editeur : des cmd.exe factices (copies renommees) dont la ligne de commande contient un Root factice.
# Code de sortie 0 = vert, 1 = rouge.
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$script = Join-Path $here 'stray_processes.ps1'
$tmp = Join-Path $env:TEMP ('stray_test_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $tmp | Out-Null
$root = Join-Path $tmp 'FakeWorktree'
$cmdExe = Join-Path $env:SystemRoot 'System32\cmd.exe'
$fakeEditor = Join-Path $tmp 'UnrealEditor.exe'
Copy-Item $cmdExe $fakeEditor

$started = @()
function Start-Fake([string]$Exe, [string]$ArgString) {
  $p = Start-Process -FilePath $Exe -ArgumentList $ArgString -WindowStyle Hidden -PassThru
  $script:started += $p
  return $p
}
function Get-ChildPids([int]$ParentPid) {
  # conhost.exe (console cachee) suit la mort de son parent : ce n'est pas un fils applicatif.
  return @(Get-CimInstance Win32_Process -Filter "ParentProcessId=$ParentPid" | Where-Object { $_.Name -ne 'conhost.exe' } | ForEach-Object { [int]$_.ProcessId })
}
function Alive([int]$ProcId) { return [bool](Get-Process -Id $ProcId -ErrorAction SilentlyContinue) }

$fail = 0
function Check([string]$Name, [bool]$Ok) {
  if ($Ok) { Write-Output "  ok    $Name" } else { Write-Output "  ROUGE $Name"; $script:fail++ }
}

try {
  $sleep = 'ping -n 120 127.0.0.1 >nul'
  # Faux builds (3 formes) : le parent porte le Root, le fils (ping) ne le porte pas.
  $buildPs1 = Start-Fake $cmdExe "/c `"set MARK=$root\Tools\build.ps1 & $sleep`""
  $buildBat = Start-Fake $cmdExe "/c `"set MARK=$root\Engine\Build\BatchFiles\Build.bat & $sleep`""
  $buildUbt = Start-Fake $cmdExe "/c `"set MARK=$root\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe & $sleep`""
  # Faux editeur detache (nom UnrealEditor.exe) et faux script de mesure detache (chain), ce dernier parent d'un faux build.
  $editor = Start-Fake $fakeEditor "/c `"set MARK=$root\Game.uproject & $sleep`""
  # Le nom build.ps1 n'apparait pas dans la ligne du parent (expansion retardee), seulement dans celle de son fils.
  $chain = Start-Fake $cmdExe "/v:on /c `"set A=bui& set B=ld.ps1& set MARK=$root\Tools\Tests\chain_measure.ps1 & cmd.exe /c set MARK=$root\Tools\!A!!B! ^& $sleep`""
  Start-Sleep -Milliseconds 1500
  $kids = @{}
  foreach ($p in @($buildPs1, $buildBat, $buildUbt, $editor, $chain)) { $kids[$p.Id] = @(Get-ChildPids $p.Id) }
  $nestedBuild = @($kids[$chain.Id])
  Check 'precondition : faux processus vivants' ((@($buildPs1, $buildBat, $buildUbt, $editor, $chain) | Where-Object { -not (Alive $_.Id) }).Count -eq 0)
  Check 'precondition : chaque faux parent a un fils' ((@($buildPs1, $buildBat, $buildUbt, $editor) | Where-Object { $kids[$_.Id].Count -eq 0 }).Count -eq 0)

  $out = & powershell -NoProfile -ExecutionPolicy Bypass -File $script -Root $root -Stop 2>&1 | Out-String
  Write-Output $out
  Start-Sleep -Milliseconds 500

  Check 'build.ps1 epargne' (Alive $buildPs1.Id)
  Check 'fils de build.ps1 epargne' ((@($kids[$buildPs1.Id]) | Where-Object { -not (Alive $_) }).Count -eq 0)
  Check 'Build.bat epargne' (Alive $buildBat.Id)
  Check 'fils de Build.bat epargne' ((@($kids[$buildBat.Id]) | Where-Object { -not (Alive $_) }).Count -eq 0)
  Check 'UnrealBuildTool epargne' (Alive $buildUbt.Id)
  Check 'fils de UnrealBuildTool epargne' ((@($kids[$buildUbt.Id]) | Where-Object { -not (Alive $_) }).Count -eq 0)
  Check 'faux editeur tue' (-not (Alive $editor.Id))
  Check 'fils du faux editeur tue' ((@($kids[$editor.Id]) | Where-Object { Alive $_ }).Count -eq 0)
  Check 'faux script de mesure detache tue' (-not (Alive $chain.Id))
  Check 'build lance par le script detache epargne' ((@($nestedBuild) | Where-Object { -not (Alive $_) }).Count -eq 0 -and $nestedBuild.Count -gt 0)
} finally {
  # Nettoyage : tout ce qui porte le Root factice, puis les copies.
  $ErrorActionPreference = 'Continue'
  foreach ($p in @(Get-CimInstance Win32_Process)) {
    $c = [string]$p.CommandLine
    if ($c -and $c.IndexOf($tmp, [StringComparison]::OrdinalIgnoreCase) -ge 0 -and [int]$p.ProcessId -ne $PID) {
      & taskkill /PID $p.ProcessId /T /F 2>&1 | Out-Null
    }
  }
  foreach ($p in $started) { & taskkill /PID $p.Id /T /F 2>&1 | Out-Null }
  Start-Sleep -Milliseconds 500
  Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}
if ($fail -eq 0) { Write-Output 'stray_processes_test : VERT'; exit 0 }
Write-Output "stray_processes_test : ROUGE ($fail)"
exit 1
