# ELROND-1118c (D845, lot A): gate "no-cpp-rpc". The RPCs retired from the C++ controller must have no UFUNCTION(Server...) left in HOLPlayerController.h
# (the script RPC actor, Script\Net\HOLRpcPlayer.as, is their only entry). Returns the names still declared (empty = green). Dot-sourced by quick_check.ps1 ("values" part).
# Lot A: QueueProduction, Tribute, MarketTrade, CancelProduction, RequestPause, RequestResume. Surrender stays (not proved, P3).
$NoCppRpcLotA = @('ServerQueueProduction', 'ServerTribute', 'ServerMarketTrade', 'ServerCancelProduction', 'ServerRequestPause', 'ServerRequestResume')
# Lot B (D845): StartSpecialization, CancelSpecialization, DuelChallenge, DuelAnswer, ReturnHero (RED until the C++ lot B is committed).
$NoCppRpcLotB = @('ServerStartSpecialization', 'ServerCancelSpecialization', 'ServerDuelChallenge', 'ServerDuelAnswer', 'ServerReturnHero')

function Get-NoCppRpcLeft([string] $HeaderPath, [string[]] $Names = ($NoCppRpcLotA + $NoCppRpcLotB)) {
    if (-not (Test-Path $HeaderPath)) { return @('header missing') }
    $Text = [IO.File]::ReadAllText($HeaderPath)
    $Alt = ($Names | ForEach-Object { [regex]::Escape($_) }) -join '|'
    # UFUNCTION(Server, ...) then (comments or blank lines) then "void <Name>(" : the RPC declaration of this name.
    $Pattern = 'UFUNCTION\(\s*Server[^)]*\)\s*(?://[^\r\n]*\s*|/\*.*?\*/\s*)*void\s+(' + $Alt + ')\s*\('
    return @([regex]::Matches($Text, $Pattern, 'Singleline') | ForEach-Object { $_.Groups[1].Value })
}
