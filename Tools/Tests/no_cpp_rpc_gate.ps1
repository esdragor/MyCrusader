# ELROND-1118c (D845, lot A): gate "no-cpp-rpc". The RPCs retired from the C++ controller must have no UFUNCTION(Server...) or UFUNCTION(Client...) left in HOLPlayerController.h
# (the script RPC actor, Script\Net\HOLRpcPlayer.as, is their only entry). Returns the names still declared (empty = green). Dot-sourced by quick_check.ps1 ("values" part).
# Lot A: QueueProduction, Tribute, MarketTrade, CancelProduction, RequestPause, RequestResume. Surrender went in lot E.
$NoCppRpcLotA = @('ServerQueueProduction', 'ServerTribute', 'ServerMarketTrade', 'ServerCancelProduction', 'ServerRequestPause', 'ServerRequestResume')
# Lot B (D845): StartSpecialization, CancelSpecialization, DuelChallenge, DuelAnswer, ReturnHero (RED until the C++ lot B is committed).
$NoCppRpcLotB = @('ServerStartSpecialization', 'ServerCancelSpecialization', 'ServerDuelChallenge', 'ServerDuelAnswer', 'ServerReturnHero')
# Lot C (D845): PostSignal, PostChat, CheatResources (RED until the C++ lot C is committed). ClientReceiveSignal / ClientReceiveChat went in lot E (Client RPCs, the pattern sees UFUNCTION(Client ...) since lot E).
$NoCppRpcLotC = @('ServerPostSignal', 'ServerPostChat', 'ServerCheatResources')
# Lot D (lot D, 2026-11-18): HeroTalent, DiplomacyStance and the 13 cheats (RED until the C++ lot D is committed). ServerDiplomacyVision went in lot E.
$NoCppRpcLotD = @('ServerHeroTalent', 'ServerDiplomacyStance', 'ServerCheatFastBuild', 'ServerCheatPopulation', 'ServerCheatTechnology', 'ServerCheatTier', 'ServerCheatUnitSpeed', 'ServerCheatGatherSpeed', 'ServerCheatWound', 'ServerCheatDamageBuilding', 'ServerCheatSpawnBuilding', 'ServerCheatDefeat', 'ServerCheatFallForum', 'ServerCheatSkipForum', 'ServerCheatHoldPresence')
# Lot E (lot E, 2026-11-18): WatchUnit, HeroAbility, CastRage, Surrender, DiplomacyVision (Server RPCs) and ClientReceiveSignal, ClientReceiveChat (Client RPCs) (RED until the C++ lot E is committed).
$NoCppRpcLotE = @('ServerWatchUnit', 'ServerHeroAbility', 'ServerCastRage', 'ServerSurrender', 'ServerDiplomacyVision', 'ClientReceiveSignal', 'ClientReceiveChat')

function Get-NoCppRpcLeft([string] $HeaderPath, [string[]] $Names = ($NoCppRpcLotA + $NoCppRpcLotB + $NoCppRpcLotC + $NoCppRpcLotD + $NoCppRpcLotE)) {
    if (-not (Test-Path $HeaderPath)) { return @('header missing') }
    $Text = [IO.File]::ReadAllText($HeaderPath)
    $Alt = ($Names | ForEach-Object { [regex]::Escape($_) }) -join '|'
    # UFUNCTION(Server|Client, ...) then (comments or blank lines) then "void <Name>(" : the RPC declaration of this name.
    $Pattern = 'UFUNCTION\(\s*(?:Server|Client)[^)]*\)\s*(?://[^\r\n]*\s*|/\*.*?\*/\s*)*void\s+(' + $Alt + ')\s*\('
    return @([regex]::Matches($Text, $Pattern, 'Singleline') | ForEach-Object { $_.Groups[1].Value })
}
