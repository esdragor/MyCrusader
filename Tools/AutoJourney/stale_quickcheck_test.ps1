# stale_quickcheck_test.ps1 : test d'effet de stale_quickcheck.ps1 (F8, GLORFINDEL-1124d). Aucun C++, aucun editeur, aucun quick_check ;
# travaille sur un VRAI depot git jetable dans %TEMP% (branche main + branches de travail).
# Usage : powershell -ExecutionPolicy Bypass -File <ce fichier>   (une ligne PASS/FAIL par cas, code 1 s'il y a un FAIL)
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
$Tool = Join-Path $Here 'stale_quickcheck.ps1'
$Fails = 0
function Check([bool]$Ok, [string]$Name, [string]$Detail = '') {
  if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}
function Git { param([string]$Repo) & git.exe -C $Repo @args 2>&1 }   # git.exe : a bare "git" would call this function again
function Put([string]$Repo, [string]$Rel, [string]$Text) {
  $p = Join-Path $Repo $Rel
  $d = Split-Path $p -Parent
  if (-not (Test-Path $d)) { New-Item -ItemType Directory -Path $d -Force | Out-Null }
  [IO.File]::WriteAllText($p, $Text)
}
function RunTool([string]$Repo) {
  # Only the tool's own stdout lines that start with STALE: are counted.
  $out = & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $Tool -Root $Repo -Base main 2>&1 | Out-String
  return @($out -split "`r?`n" | Where-Object { $_ -like 'STALE:*' })
}
function NewBranch([string]$Repo, [string]$Name) { Git $Repo checkout -q main | Out-Null; Git $Repo checkout -q -b $Name | Out-Null }
function Commit([string]$Repo, [string]$Msg) { Git $Repo add -A | Out-Null; Git $Repo commit -q -m $Msg | Out-Null }

$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('stalequick-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
$Repo = Join-Path $Tmp 'repo'
New-Item -ItemType Directory -Path $Repo -Force | Out-Null
Git $Repo init -q | Out-Null
Git $Repo symbolic-ref HEAD refs/heads/main | Out-Null
Git $Repo config user.name 'Test' | Out-Null
Git $Repo config user.email 'test@example.com' | Out-Null
Git $Repo config core.autocrlf false | Out-Null
Put $Repo 'Script/Hero/Banner.as' "// banner`nvoid Show() { Log(`"Hero F9 label`"); }`nvoid Other() { Log(`"Kept label`"); }`n"
Put $Repo 'Script/Hero/Banner_Test.as' "void Test_OldBannerRule(FAutomationTest T) { }`nvoid Test_KeptRule(FAutomationTest T) { }`n"
Put $Repo 'Tools/Data/Abilities.csv' "Name,Cost`nOldAbility,10`nKeptAbility,20`n"
Put $Repo 'Tools/Tests/quick_check.ps1' "# quick_check`nWrite-Output 'start'`n`$want = 'Hero F9 label'`n`$t = 'Test_OldBannerRule'`n`$a = 'OldAbility'`n`$k = 'Kept label'`n`$u = 'Test_KeptRule'`n`$n = 'BrandNewLabel'`n"
Commit $Repo 'init'

# Case 1 : a removed identifier still cited -> one STALE line naming quick_check.ps1 and its line (4).
NewBranch $Repo 'work1'
Put $Repo 'Script/Hero/Banner.as' "// banner`nvoid Show() { Log(`"Hero label`"); }`nvoid Other() { Log(`"Kept label`"); }`n"
Commit $Repo 'rename label'
$r1 = RunTool $Repo
Check ($r1.Count -eq 1) 'removed label, still cited: exactly one STALE line' ($r1 -join ' | ')
Check (($r1 -join ' ') -like '*Hero F9 label*quick_check.ps1:3*') 'STALE line names the identifier and quick_check.ps1:3' ($r1 -join ' | ')

# Case 2 : an identifier ADDED only (cited or not) -> no STALE line.
NewBranch $Repo 'work2'
Put $Repo 'Script/Hero/Banner.as' "// banner`nvoid Show() { Log(`"Hero F9 label`"); }`nvoid Other() { Log(`"Kept label`"); Log(`"BrandNewLabel`"); }`n"
Commit $Repo 'add label'
$r2 = RunTool $Repo
Check ($r2.Count -eq 0) 'identifier added only: no STALE line' ($r2 -join ' | ')

# Case 3 : a removed identifier that quick_check does not cite -> no STALE line.
NewBranch $Repo 'work3'
Put $Repo 'Script/Hero/Banner.as' "// banner`nvoid Show() { Log(`"Hero F9 label`"); }`nvoid Other() { Log(`"Nobody cites this`"); }`n"
Commit $Repo 'rename uncited label'
$r3 = RunTool $Repo
Check ($r3.Count -eq 0) 'removed identifier not cited by quick_check: no STALE line' ($r3 -join ' | ')

# Case 4 : a renamed Test_ name and a removed CSV row, both cited -> two STALE lines (lines 4 and 5).
NewBranch $Repo 'work4'
Put $Repo 'Script/Hero/Banner_Test.as' "void Test_NewBannerRule(FAutomationTest T) { }`nvoid Test_KeptRule(FAutomationTest T) { }`n"
Put $Repo 'Tools/Data/Abilities.csv' "Name,Cost`nKeptAbility,20`n"
Commit $Repo 'rename test, drop csv row'
$r4 = RunTool $Repo
Check ($r4.Count -eq 2) 'renamed Test_ and removed CSV row: two STALE lines' ($r4 -join ' | ')
Check ((($r4 -join ' ') -like '*Test_OldBannerRule*quick_check.ps1:4*') -and (($r4 -join ' ') -like '*OldAbility*quick_check.ps1:5*')) 'both lines carry their quick_check.ps1 line number' ($r4 -join ' | ')

# Case 5 : a moved line (identifier removed here, present again elsewhere in HEAD) -> no STALE line.
NewBranch $Repo 'work5'
Put $Repo 'Script/Hero/Banner.as' "// banner`nvoid Other() { Log(`"Kept label`"); }`nvoid Show() { Log(`"Hero F9 label`"); }`n"
Commit $Repo 'move function'
$r5 = RunTool $Repo
Check ($r5.Count -eq 0) 'identifier moved (still in HEAD): no STALE line' ($r5 -join ' | ')

Git $Repo checkout -q main | Out-Null
Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
if ($Fails -gt 0) { Write-Output "stale_quickcheck_test : $Fails FAIL"; exit 1 }
Write-Output 'stale_quickcheck_test : PASS'
exit 0
