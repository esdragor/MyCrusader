# GALADRIEL-1117f : Market Apply / NewBook (MIG-C++-FINAL, D836 point 4)

Worktree HeroesOfLiberty-Galadriel, branche galadriel-tab. Commits : 5730795 (test script rouge), 719b49e (code). C++ : OUI (pont), NON COMPILE, aucun build lance.

## Decision par lecture
| Element | Frequence | Decision |
|---|---|---|
| HOLMarket::Apply (Sell / BuyWithGold choisis, stock deplace) | 1 par ordre Trade du serveur, 1 par clic Marche | PORTE : script HOLMarket::Apply (HOLMarketRules.as), pont UHOLMarketSubsystem::Apply -> TradeNow -> TradeOnce (override HOLMarketBoardScript.as), type FHOLMarketBridgeTrade |
| Appelants Apply : HOLProductionSubsystem::ApplyMarketTrade, HOLPlayerController (devis du clic) | par action du joueur | REDIRIGES vers le pont |
| NewBook | 1 appel par partie (HOLGameState::ResetMarketBook) | portable (deja HOLMarket::NewBook en script), PAS fait ce lot : un appel, a joindre au lot suivant avec la suppression C++ |
| Devis Slate (HOLGameScreen.cpp:2381, Sell/BuyWithGold sur stock "Huge") | a chaque rafraichissement de cellule | RESTE C++ ce lot : Slate, et un aller-retour pont par cellule demande une mesure ; si on le porte, le faire par le pont avec un cache du livre |
| LoadStandardSettings / SettingsFromRow | 1 fois | RESTENT C++ : lecture d'un UDataTable (/Game/Match/MarketRates), actif moteur ; FSettings/FBook/FTrade, Lot, BookSize aussi (types du pont) |
| HOLAIOrders | n'appelle aucune fonction HOLMarket::* (grep) | rien a porter |
| Remarque | | le script a un Preset(C) code en dur et le C++ lit la table : deux sources de reglages, a trancher par l'utilisateur (hors lot) |

## Test
Script\Economy\HOLMarketApply_Test.as : Test_MarketApplySellIsSell, Test_MarketApplyBuyIsBuyWithGold, Test_MarketApplyRefusals. Rouge : HOLMarket::Apply n'existait pas au commit 5730795 (ne compile qu'apres le build ; le rouge est "fonction absente"). Vert a voir apres le build. Le pont lui-meme (C++ -> board) n'a pas de test neuf : a ajouter a Market.ServerBoard (une assertion TradeNow) au lot suivant.

## Reste C++ (et copies a supprimer apres build)
HOLMarket::Apply, Sell, Buy, BuyWithGold, NewBook (utilises par HOLMarketRulesTests.cpp, HOLGameScreen, HOLGameState) : supprimer au lot suivant avec les tests C++ Market.Rules/.Apply/.Chain (equivalents script : Test_MarketRulesVectors, Test_MarketApply*).

## Build
`C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts Cpp,Script` (attendus : Market.ServerBoard, Market.Apply, Market.Chain verts ; Test_MarketApply* verts). Risques : UFUNCTION BlueprintNativeEvent a 6 parametres avec const FHOLMarketBridge*& (meme forme que RecoverPrices) ; FName(*FString) ; l'override script TradeOnce.
