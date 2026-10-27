# Get-ScriptFileErrors: the compile errors of the project's .as files in a log (D484), at most three lines: the file,
# then its line and message ("Script/X.as:", " (3:20): Expected ')' or ','"), and the hot reload's failure line.
# Not the engine's own bind errors: the packaged game and server log twelve at start, on engine test delegates whose
# parameter is named after a reserved word ("Failed in call to function 'RegisterObjectMethod' with
# 'FTestDynamicDelegateFloat' and 'void Execute(float32 float) const'"), unusable from script and harmless.
# Dot-sourced by precompile_scripts.ps1 and Tools\Tests\packaged_smoke.ps1.
function Get-ScriptFileErrors([string] $Path) {
    if (-not (Test-Path $Path)) { return @() }
    $Found = New-Object Collections.Generic.List[string]
    $Take = 0
    foreach ($Line in [IO.File]::ReadLines($Path)) {
        if ($Line -notmatch "Angelscript: Error: (.*)$") { $Take = 0; continue }
        $Message = $Matches[1].Trim()
        if ($Message -match "\.as:$") { $Take = 3 }
        if ($Take -gt 0) { $Found.Add($Message); $Take-- }
        elseif ($Message -match "script compile errors") { $Found.Add($Message) }
    }
    return @($Found | Select-Object -First 3)
}
