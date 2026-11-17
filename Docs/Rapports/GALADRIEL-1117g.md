# GALADRIEL-1117g : MIG-Market-source-unique (copie C++ de Apply / Buy supprimee, D836)

Worktree HeroesOfLiberty-Galadriel, branche galadriel-tab. C++ : OUI, NON COMPILE (aucun build lance par cette session).

## Supprime (C++)
| Element | Decision |
|---|---|
| HOLMarket::Apply (HOLMarketRules.h/.cpp) | SUPPRIME : plus aucun appelant C++ (le serveur et le controleur passent par le pont UHOLMarketSubsystem::Apply -> script) |
| HOLMarket::Buy (idem) | SUPPRIME : seuls appelants = tests C++ de Market.Rules |
| HOLMarket::Sell | GARDE : le devis Slate (HOLGameScreen.cpp:2382) l'appelle (consigne : garder le devis) ; plus de copie jumelle utilisee par la partie |
| HOLMarket::BuyWithGold | GARDE : meme raison (HOLGameScreen.cpp:2381) |
| HOLMarket::NewBook, LoadStandardSettings, SettingsFromRow | GARDES (HOLGameState.cpp:1538, tests de la table) |
Note : la consigne demandait de supprimer Sell et BuyWithGold ET de garder le devis Slate ; les deux se contredisent, le devis gagne. Pour les supprimer : porter le devis par le pont avec un cache du livre (mesure d'abord, voir GALADRIEL-1117f).

## Registre des tests C++
| Test C++ | Sort | Equivalent script (vu vert, quick_tests.log 10:11, 1394/1396, seuls rouges Nomad) |
|---|---|---|
| HeroesOfLiberty.Market.Apply (1 test, supprime en entier) | RETIRE | Test_MarketApplySellIsSell, Test_MarketApplyBuyIsBuyWithGold, Test_MarketApplyRefusals (regle) ; le mouvement de stock (500/100/0/159, 64 pierre, refus = stock inchange) reste tenu par Market.Chain |
| Market.Rules, assertions "a lot of wood bought / it costs 101 / wood returns to 80" (3 lignes, appel Buy) | RETIREES | Test_MarketRulesVectors (memes noms) |
| Market.Rules, "a lot of stone bought back costs 153" (Buy) | RETIREE | Test_MarketRulesVectors |
| Market.Rules, "a lot with 10 gold" + "a refusal moves nothing" (Buy) | RETIREES | Test_MarketRulesVectors |
| Market.Rules, "the price stops at the cap" | GARDEE, appel Buy remplace par BuyWithGold (meme resultat : le prix monte du pas, plafonne a 300) | Test_MarketRulesVectors (Buy) |
| Market.Rules, reste (CSV importe, SettingsFromRow, NewBook, Sell, BuyWithGold) | GARDE, entier | pas d'equivalent script pour la chaine CSV -> SettingsFromRow ; Sell/BuyWithGold C++ restent pour le devis |
| Market.BookState | GARDE ; son appel a Sell remplace par un livre ecrit a la main (bois 80 - pas = 77) : le test verifie la replication, pas la regle | - |
| Market.Chain, Market.Tribute, Market.Table, Market.ServerBoard | GARDES, non touches (aucun appel a Apply/Buy) | - |

Tests C++ retires : 1 (Market.Apply). Total quick_check attendu : 1396 - 1 = 1395 (tests), apres build. Lignes C++ supprimees : HOLMarketRules.cpp -37, .h -8 (+1 commentaire), HOLMarketRulesTests.cpp -55 (+6).

## Commande
`C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests,net` (attendus : Market.Rules / BookState / Chain / Tribute / ServerBoard verts, Market.Apply absent, Test_MarketApply* verts, total 1395).

## Non verifie
Compilation (aucun build) ; que plus rien en C++ n'appelle Buy/Apply en dehors des fichiers lus (grep par nom fait sur Source, pas de compilation pour le prouver) ; Market.Rules garde reste vert apres le changement Buy -> BuyWithGold du plafond.
