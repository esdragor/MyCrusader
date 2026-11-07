# KnownReds.ps1 : L8-5. Les rouges CONNUS de quick_check (Tools\Tests\known_reds.txt) distincts des regressions.
# Format de known_reds.txt : une ligne par partie, "test | depuis (AAAA-MM-JJ) | cause supposee" ; '#' = commentaire.
# Le nom du test est celui que quick_check imprime devant ": FAIL" (ex. "camera 2p"). Une ligne retiree = le rouge compte de nouveau comme regression.
function Get-KnownReds([string] $Path) {
    if (-not (Test-Path $Path)) { return @() }
    foreach ($Line in Get-Content $Path) {
        if ($Line -match '^\s*(#|$)') { continue }
        $Cols = @($Line -split '\|', 3 | ForEach-Object { $_.Trim() })
        if ($Cols.Count -lt 3 -or -not $Cols[0]) { continue }
        [pscustomobject]@{ Test = $Cols[0]; Since = $Cols[1]; Cause = $Cols[2] }
    }
}
function Find-KnownRed([string] $Part, $Reds) {
    @($Reds | Where-Object { $_.Test -eq $Part }) | Select-Object -First 1
}
function Get-RedSummary($FailedParts, $Reds) {
    $Failed = @($FailedParts)
    $Known = @($Failed | Where-Object { Find-KnownRed $_ $Reds })
    $New = @($Failed | Where-Object { -not (Find-KnownRed $_ $Reds) })
    if ($New.Count -eq 0) { return "red summary : aucune regression ($($Known.Count) rouge(s) connu(s) : $($Known -join ', '))" }
    "red summary : $($New.Count) regression(s) : $($New -join ', ') ; $($Known.Count) rouge(s) connu(s)$(if ($Known.Count) { ' : ' + ($Known -join ', ') })"
}
