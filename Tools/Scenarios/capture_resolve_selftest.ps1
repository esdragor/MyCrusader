# Self-test of the capture path logic of preflight.ps1 (no game needed, a few seconds):
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\capture_resolve_selftest.ps1
# Builds a fake root with (a) an OLD file in Screenshots\HOL and a NEW one in Screenshots\WindowsEditor\HOL, (b) no file at all.
# Prints the OLD logic (what preflight did before) and the NEW one (CaptureResolve.ps1). PASS = new logic gives the new file for (a)
# and the WindowsEditor path for (b).
. (Join-Path $PSScriptRoot "CaptureResolve.ps1")
$Fake = Join-Path ([IO.Path]::GetTempPath()) ("capture_resolve_selftest_" + [guid]::NewGuid().ToString("N"))
$A = Join-Path $Fake "Saved_Tests\Screenshots\HOL"
$B = Join-Path $Fake "Saved_Tests\Screenshots\WindowsEditor\HOL"
New-Item -ItemType Directory -Force $A, $B | Out-Null
try {
    Set-Content (Join-Path $A "en_x.png") "old"; (Get-Item (Join-Path $A "en_x.png")).LastWriteTime = (Get-Date).AddHours(-3)
    Set-Content (Join-Path $B "en_x.png") "new"
    function Old-Resolve([string]$Root, [string]$Capture) {   # the logic before the fix
        $File = Join-Path $Root ("Saved_Tests\Screenshots\HOL\" + $Capture + ".png")
        $Alt = Join-Path $Root ("Saved_Tests\Screenshots\WindowsEditor\HOL\" + $Capture + ".png")
        if (-not (Test-Path $File) -and (Test-Path $Alt)) { $File = $Alt }
        return $File
    }
    $OldA = Old-Resolve $Fake "en_x"; $NewA = Resolve-CapturePath $Fake "en_x"
    $OldB = Old-Resolve $Fake "ja_missing"; $NewB = Resolve-CapturePath $Fake "ja_missing"
    Write-Host "(a) old+new files    : before = $OldA ; after = $NewA"
    Write-Host "(b) no file at all   : before = $OldB ; after = $NewB"
    $OkA = $NewA -eq (Join-Path $B "en_x.png")
    $OkB = $NewB -eq (Join-Path $B "ja_missing.png")
    Write-Host ("capture_resolve_selftest : " + $(if ($OkA -and $OkB) { "PASS" } else { "FAIL" }))
    if ($OkA -and $OkB) { exit 0 } else { exit 1 }
} finally { Remove-Item $Fake -Recurse -Force -ErrorAction SilentlyContinue }
