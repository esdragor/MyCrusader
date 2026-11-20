# no_window_test.ps1 : no tool script may open a visible UnrealEditor window (2026-11-08: "des tests scenario pop sur mon ecran et prennent le focus").
# Usage : powershell -ExecutionPolicy Bypass -File <this file> [-Root <worktree>]   (output: PASS/FAIL per case, exit 1 on any FAIL)
# Rule (agent_rules.md, "aucun lancement fenetre"): every Start-Process of UnrealEditor(.exe / -Cmd.exe) or of the cooked server in
# Tools\**\*.ps1 carries -WindowStyle Hidden (or $WinStyle, which is Hidden unless -Visible), and a launch with -game or -server also
# has -nullrhi or -RenderOffScreen in its argument string; `& UnrealEditor-Cmd.exe` calls carry -nullrhi.
# Exempt by design (the user's own window): Tools\Scenarios\play.ps1 without -Hidden (checked separately below) and Tools\Scenarios\ai_lobby.ps1
# (a trial the user plays) and Tools\Scenarios\ecran_multijoueur.ps1 (same, modelled on ai_lobby.ps1). No editor is started here: the test reads the scripts and runs on fake scripts in %TEMP%.
# L14-8 suite (2026-11-14) : the scenarios too (Tools\Scenarios\*.txt or -ScenarioDir): a `frames N` > 30 that waits for the game (same function as
# check.ps1, Tools\Scenarios\FrameWaits.ps1). Default Warn (question Retours\Q-frames-refus.md, option B coded): "WARN frames <scenario> : ..." lines
# (15 at most) and a count line "WARN frames waits: N in M scenario(s)", exit unchanged; -FrameWaits Fail or HOL_CHECK_FRAMES=fail = FAIL.
# Test: Tools\AutoJourney\no_window_frames_test.ps1.
param([string] $Root = '', [string] $ScenarioDir = '',
      [ValidateSet('Warn', 'Fail')] [string] $FrameWaits = $(if ($env:HOL_CHECK_FRAMES -eq 'fail') { 'Fail' } else { 'Warn' }))
$ErrorActionPreference = 'Continue'
$Here = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $Root) { $Root = (Resolve-Path (Join-Path $Here '..\..')).Path }
$Tools = Join-Path $Root 'Tools'
# ecran_multijoueur.ps1 (SAURON-1120sb): a trial the user plays (D840 ECR-3, header "le jeu en fenetre ... modele ai_lobby.ps1"), window wanted.
$Exempt = @('play.ps1', 'ai_lobby.ps1', 'ecran_multijoueur.ps1', 'no_window_test.ps1')
$Fails = 0
function Check([bool] $Ok, [string] $Name, [string] $Detail = '') {
    if ($Ok) { Write-Output "PASS $Name" } else { $script:Fails++; Write-Output "FAIL $Name $Detail" }
}

# One statement = a line plus its continuations (line ending in a backtick, +, or ,).
function Get-Statement([string[]] $Lines, [int] $Index) {
    $Text = $Lines[$Index]
    while ($Index + 1 -lt $Lines.Count -and $Text -match '(`|\+|,)\s*$') { $Index++; $Text += ' ' + $Lines[$Index].Trim() }
    return $Text
}

# Returns the violations (strings) of one script.
function Get-Violations([string] $Path) {
    $Lines = @(Get-Content $Path)
    $Out = @()
    # Variables that hold the path of an engine executable (UnrealEditor*.exe) or of the cooked server.
    $Exe = @{}
    for ($i = 0; $i -lt $Lines.Count; $i++) {
        if ($Lines[$i] -match '^\s*(\$\w+)\s*=.*(UnrealEditor|HeroesOfLibertyServer)') { $Exe[$Matches[1]] = $true }
    }
    for ($i = 0; $i -lt $Lines.Count; $i++) {
        $Line = $Lines[$i]
        if ($Line -match '^\s*#') { continue }
        $Where = "$(Split-Path $Path -Leaf):$($i + 1)"
        if ($Line -match 'Start-Process\s+(-FilePath\s+)?(\(Join-Path[^)]*\)\s*\)?|\$\w+)') {
            $File = $Matches[2]
            $IsEngine = ($File -match 'UnrealEditor') -or ($File -match '^\$\w+$' -and $Exe.ContainsKey($File))
            if (-not $IsEngine -and $Line -notmatch 'UnrealEditor') { continue }
            $Statement = Get-Statement $Lines $i
            if ($Statement -notmatch '-WindowStyle\s+(Hidden|\$WinStyle)') { $Out += "$Where : Start-Process without -WindowStyle Hidden" }
            $ArgText = $Statement
            if ($Statement -match '-ArgumentList\s+\(?(\$\w+)') {
                $Var = $Matches[1]
                # The nearest assignment of the argument variable before the launch, and of the *Display variables it may use.
                $Pattern = '^\s*' + [regex]::Escape($Var) + '\s*='
                for ($j = $i; $j -ge 0; $j--) { if ($Lines[$j] -match $Pattern) { $ArgText += ' ' + (Get-Statement $Lines $j); break } }
                for ($j = $i; $j -ge 0; $j--) { if ($Lines[$j] -match '^\s*\$\w*Display\s*=') { $ArgText += ' ' + (Get-Statement $Lines $j); break } }
            }
            if ($ArgText -match '(^|\s)-(game|server)(\s|$)' -and $ArgText -notmatch '-nullrhi|-RenderOffScreen') {
                $Out += "$Where : -game/-server launch without -nullrhi or -RenderOffScreen"
            }
        }
        elseif ($Line -match '^\s*&\s*(\$\w+)\s' -and $Exe.ContainsKey($Matches[1])) {
            if ((Get-Statement $Lines $i) -notmatch '-nullrhi') { $Out += "$Where : & UnrealEditor call without -nullrhi" }
        }
    }
    return $Out
}

# --- Cases on fake scripts: the detector itself.
$Tmp = Join-Path ([IO.Path]::GetTempPath()) ('nowin-' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Path $Tmp | Out-Null
try {
    $Bad1 = Join-Path $Tmp 'bad_server.ps1'
    Set-Content -Encoding ascii $Bad1 @(
        '$EngineDir = "C:\E"',
        '$ServerArguments = "`"$UProject`" /Game/Maps/Plain -server -Port=1 -nosound"',
        '$P = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $ServerArguments -PassThru -WindowStyle Hidden')
    Check ((Get-Violations $Bad1).Count -eq 1) 'detector: -server without -nullrhi is flagged' ((Get-Violations $Bad1) -join ' | ')
    $Bad2 = Join-Path $Tmp 'bad_client.ps1'
    Set-Content -Encoding ascii $Bad2 @(
        '$EngineDir = "C:\E"',
        '$Arguments = "`"$UProject`" -game -RenderOffScreen -nosound"',
        '$P = Start-Process -FilePath (Join-Path $EngineDir "UnrealEditor.exe") -ArgumentList $Arguments -PassThru')
    Check ((Get-Violations $Bad2).Count -eq 1) 'detector: Start-Process without -WindowStyle Hidden is flagged' ((Get-Violations $Bad2) -join ' | ')
    $Bad3 = Join-Path $Tmp 'bad_cmd.ps1'
    Set-Content -Encoding ascii $Bad3 @('$Cmd = Join-Path $R "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"', '& $Cmd $UProject -run=x -unattended')
    Check ((Get-Violations $Bad3).Count -eq 1) 'detector: & UnrealEditor-Cmd without -nullrhi is flagged' ((Get-Violations $Bad3) -join ' | ')
    $Good = Join-Path $Tmp 'good.ps1'
    Set-Content -Encoding ascii $Good @(
        '$Editor = Join-Path $EngineDir "UnrealEditor.exe"',
        '$Display = if ($Visible) { "-windowed -ResX=940" } else { "-RenderOffScreen -ResX=960 -nosound" }',
        '$Arguments = "`"$UProject`" -game $Display -unattended"',
        '$WinStyle = if ($Visible) { "Normal" } else { "Hidden" }',
        '$P = Start-Process -FilePath $Editor -ArgumentList $Arguments -PassThru -WindowStyle $WinStyle',
        '$S = Start-Process -FilePath $Editor -ArgumentList "$M -server -nullrhi" -PassThru -WindowStyle Hidden')
    Check ((Get-Violations $Good).Count -eq 0) 'detector: masked launches pass' ((Get-Violations $Good) -join ' | ')
}
finally { Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue }

# --- The real tool scripts.
$All = 0; $Bad = @()
foreach ($File in Get-ChildItem $Tools -Recurse -Filter *.ps1 | Where-Object { $Exempt -notcontains $_.Name }) {
    $All++
    $Bad += @(Get-Violations $File.FullName)
}
Check ($Bad.Count -eq 0) "tool scripts: no visible UnrealEditor launch ($All scripts scanned)" ("`n  " + ($Bad -join "`n  "))

# --- play.ps1: visible for the user by default, -Hidden for an agent replaying the user's gesture.
$Play = Join-Path $Tools 'Scenarios\play.ps1'
$PlayText = if (Test-Path $Play) { Get-Content $Play -Raw } else { '' }
Check ($PlayText -match '\[switch\]\s*\$Hidden') 'play.ps1 has -Hidden'
Check ($PlayText -match '-RenderOffScreen' -and $PlayText -match '-nosound' -and $PlayText -match '-unattended' -and $PlayText -match '-WindowStyle') 'play.ps1 -Hidden adds -RenderOffScreen -nosound -unattended and -WindowStyle Hidden'
Check ($PlayText -match 'if\s*\(\$Hidden\)') 'play.ps1 stays visible unless -Hidden'

# --- quick_check: -Visible is the only way to a window; net_match too.
$QC = Join-Path $Tools 'Tests\quick_check.ps1'
Check ((Test-Path $QC) -and ((Get-Content $QC -Raw) -match '\$WinStyle\s*=\s*if\s*\(\$Visible\)')) 'quick_check.ps1: window style Hidden unless -Visible'
$NM = Join-Path $Tools 'Tests\net_match.ps1'
Check ((Test-Path $NM) -and ((Get-Content $NM -Raw) -match '\[switch\]\s*\$Visible')) 'net_match.ps1: clients hidden unless -Visible'

# --- The rule is written where agents read it.
$Rules = Join-Path $Here 'agent_rules.md'
Check ((Test-Path $Rules) -and ((Get-Content $Rules -Raw) -match 'aucun lancement f')) 'agent_rules.md: rule "aucun lancement fenetre"'

# --- Scenarios: frames > 30 waiting for the game (hidden window ~150 frames/s: the wait is far shorter than written).
if (-not $ScenarioDir) { $ScenarioDir = Join-Path $Tools 'Scenarios' }
$FrameLib = Join-Path $Tools 'Scenarios\FrameWaits.ps1'
if (Test-Path $FrameLib) {
    . $FrameLib
    $FrameHits = @()
    foreach ($Sc in Get-ChildItem $ScenarioDir -Filter *.txt -ErrorAction SilentlyContinue) {
        foreach ($P in @(Get-FrameWaitProblem $Sc.FullName | Where-Object { $_.Level -eq 'FAIL' })) {
            $FrameHits += [pscustomobject]@{ Name = $Sc.BaseName; Text = $P.Text }
        }
    }
    $Level = if ($FrameWaits -eq 'Fail') { 'FAIL' } else { 'WARN' }
    foreach ($H in ($FrameHits | Select-Object -First 15)) { Write-Output ("{0} frames {1} : {2}" -f $Level, $H.Name, $H.Text) }
    $ScCount = @($FrameHits | Select-Object -ExpandProperty Name -Unique).Count
    if ($FrameWaits -eq 'Fail') { Check ($FrameHits.Count -eq 0) "scenarios: no frames > 30 waiting for the game (-FrameWaits Fail)" ("{0} in {1} scenario(s)" -f $FrameHits.Count, $ScCount) }
    elseif ($FrameHits.Count -gt 0) { Write-Output ("WARN frames waits: {0} in {1} scenario(s) of {2} (default Warn, Retours\Q-frames-refus.md ; Fail: -FrameWaits Fail or HOL_CHECK_FRAMES=fail)" -f $FrameHits.Count, $ScCount, $ScenarioDir) }
    else { Write-Output "PASS scenarios: no frames > 30 waiting for the game" }
}
else { Write-Output "WARN frames: $FrameLib missing, scenarios not checked" }

if ($Fails -gt 0) { Write-Output "no_window_test : $Fails FAIL"; exit 1 }
Write-Output 'no_window_test : PASS'
exit 0
