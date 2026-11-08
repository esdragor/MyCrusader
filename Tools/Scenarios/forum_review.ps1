# Composes the ten Forum variants K to T for the review scenario forum_review (VisualInProgress.md): one static mesh each,
# /Game/Buildings/Forum/Review/SM_ForumReview_<letter>, banners kept (VI-Q3 = B). SM_Forum (the chosen K) is not touched.
# Run once, with no other Unreal editor open on this worktree:
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\forum_review.ps1 [-Letters K,L] [-Root <worktree>]
param(
    [string[]] $Letters = @("K", "L", "M", "N", "O", "P", "Q", "R", "S", "T"),
    [string] $Root = ""
)
$Letters = @($Letters | ForEach-Object { $_ -split "," } | Where-Object { $_ })
if (-not $Root) { $Root = Resolve-Path "$PSScriptRoot\..\.." }
$Root = (Resolve-Path $Root).Path
$UProject = Join-Path $Root "HeroesOfLiberty.uproject"
$Association = (Get-Content $UProject -Raw | ConvertFrom-Json).EngineAssociation
$EngineRoot = (Get-ItemProperty "HKCU:\Software\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue).$Association
$Cmd = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$Script = Join-Path $Root "Tools\Editor\make_forum_mesh.py"
$Failed = 0
foreach ($Letter in $Letters) {
    # unreal.log() goes to the editor log file, not to the standard output: -abslog sends it to a file we read back.
    $LogFile = Join-Path $env:TEMP "forum_review_$Letter.log"
    Remove-Item $LogFile -ErrorAction SilentlyContinue
    & $Cmd $UProject -run=pythonscript "-script=$Script --compose --review --variant=$Letter --colour=banner" -unattended -nullrhi -nosplash -nosound -Multiprocess -LiveCoding=0 "-abslog=$LogFile" | Out-Null
    $Log = if (Test-Path $LogFile) { Get-Content $LogFile -Raw } else { "" }
    $Line = ($Log -split "`n" | Where-Object { $_ -match "Forum compose: .*triangles" } | Select-Object -Last 1)
    if ($Line) { Write-Host "variant ${Letter}: $($Line.Trim())" } else { Write-Host "variant ${Letter}: NOT COMPOSED"; $Failed++ }
}
$global:LASTEXITCODE = 0
Write-Host "forum_review: $(if ($Failed -eq 0) { 'PASS' } else { "FAIL ($Failed)" })"
