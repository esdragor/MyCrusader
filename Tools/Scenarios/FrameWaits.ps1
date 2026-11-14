# FrameWaits.ps1 (L14-8, 2026-11-14) : a dot-sourcer (check.ps1). Get-FrameWaitProblem <scenario.txt> renvoie des objets {Level, Text} :
#  - FAIL : `frames N` avec N > $MaxFrames qui n'encadre pas une prise de camera (pas camera/shot/capture juste avant ou juste apres) :
#    c'est une attente d'evenement du jeu par images ; en fenetre cachee ~150 images/s, `frames 600` dure 4 s (a633b97, legions_warrior
#    le 2026-11-13 19:30). Attendre par `wait <s>` ou mieux `waitlog <texte>`.
#  - WARN : `waitlog` juste apres un `wait` de plus de $LongWait s : l'evenement a pu passer pendant l'attente, le waitlog expire.
# Test : Tools\Tests\frame_waits_test.ps1.
function Get-FrameWaitProblem([string]$Path, [int]$MaxFrames = 30, [double]$LongWait = 10) {
    $Steps = @()
    $n = 0
    foreach ($raw in (Get-Content -LiteralPath $Path -ErrorAction SilentlyContinue)) {
        $n++
        $l = $raw.Trim()
        if (-not $l -or $l.StartsWith('#') -or $l.StartsWith('//')) { continue }
        $Steps += [pscustomobject]@{ N = $n; Line = $l }
    }
    $Camera = '(?i)camera|shot|capture'
    for ($i = 0; $i -lt $Steps.Count; $i++) {
        $s = $Steps[$i]
        if ($s.Line -match '^frames\s+(\d+)' -and [int]$Matches[1] -gt $MaxFrames) {
            $prev = if ($i -gt 0) { $Steps[$i - 1].Line } else { '' }
            $next = if ($i + 1 -lt $Steps.Count) { $Steps[$i + 1].Line } else { '' }
            if (-not ($prev -match $Camera -or $next -match $Camera)) {
                [pscustomobject]@{ Level = 'FAIL'; Text = "line $($s.N): '$($s.Line)' waits for the game by frames (> $MaxFrames ; hidden window ~150 frames/s: frames 600 = 4 s) : use 'wait <s>' or 'waitlog <text>'" }
            }
        }
        if ($s.Line -match '^waitlog\b' -and $i -gt 0 -and $Steps[$i - 1].Line -match '^wait\s+([\d\.]+)' -and [double]$Matches[1] -gt $LongWait) {
            [pscustomobject]@{ Level = 'WARN'; Text = "line $($s.N): '$($s.Line)' comes right after '$($Steps[$i - 1].Line)' (> $LongWait s) : the event may be past already, the waitlog may time out" }
        }
    }
}
