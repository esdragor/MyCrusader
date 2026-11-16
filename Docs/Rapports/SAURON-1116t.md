# SAURON-1116t : copie C++ de Recover et Received retirée (MIG-CPP-Market)

- AGENT: SAURON-1116t, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, branche visual, commit f1dc2dc « Market: script is the source for Recover and Received, C++ copy removed » (branche prête, rien poussé, merge main non fait : DLL non reconstruite).
- TÂCHE: le script est la seule source de Recover et Received ; le pont n'a plus de repli C++ (D836 point 4).
- C++ : OUI, motif : suppression des parties de la règle du marché que le pont a déjà passées au script.
- LAUNCHCHECK_DEBUT: non lancé (aucun build ni test dans ce lot, la session principale compile).

## Changement de comportement du pont (HOLMarketSubsystem.h/.cpp)
- `UHOLMarketSubsystem::Recover` sans board script : renvoie le livre d'entrée inchangé + UE_LOG Error.
- `UHOLMarketSubsystem::Received` sans board script : renvoie `Sent` (montant envoyé, sans taxe) + UE_LOG Error.
- `RecoverPrices_Implementation` / `TributeReceived_Implementation` (défauts du BlueprintNativeEvent, appelés si une sous-classe n'a pas surchargé) : même défaut inerte (livre inchangé / Sent) + UE_LOG Error. Commentaires du .h mis à jour.

## REGISTRE (C++ -> script)
| Symbole C++ supprimé | Où | Équivalent script |
|---|---|---|
| `HOLMarket::Recover` (déclaration + corps) | HOLMarketRules.h / .cpp | `HOLMarket::Recover`, Script\Economy\HOLMarketRules.as:242 (appelé par HOLMarketBoardScript.as:20) |
| `HOLTribute::Received` (déclaration + corps) | HOLMarketRules.h / .cpp | `HOLTribute::Received`, Script\Economy\HOLTributeRules.as:36 (appelé par HOLMarketBoardScript.as:29) |

| Test C++ | Retiré | Équivalent script |
|---|---|---|
| Market.Rules (4 assertions Recover, ex-l. 115-118) | 4 assertions (le test reste : Sell, Buy, BuyWithGold, SettingsFromRow, NewBook y sont encore testés, et encore appelés par Apply et le devis HOLGameScreen.cpp:2381) | Test_MarketRulesVectors, HOLMarketRules_Test.as l. 358-361 (mêmes 75 / 80 / 80 / MaxPrice-3) |
| Market.Tribute (tête pure, ex-l. 364-367 : Received 100->70, 1->1, tax 250 -> 0, 0->0) | 4 assertions (la ligne `DefaultTaxPercent == 30` et toute la partie monde restent) | Test_MarketTributeVectors, HOLMarketRules_Test.as l. 369-372 (+ Test_TributeReceived, HOLTributeRules_Test.as l. 7-16) |
| Market.ServerBoard (HOLMarketBridgeTests.cpp) | comparaisons à `HOLMarket::Recover` / `HOLTribute::Received` C++ remplacées par des chiffres calculés à la main (Recover : 0/5/10/40 intervalles ; Received : 1->1, 7->5, 10->7, 99->70, 100->70, 250->175, 1000->700) | le test passe par le vrai board script |

- TESTS_CPP_RETIRES: 0 test entier ; 8 assertions retirées (Market.Rules 4, Market.Tribute 4). Le total de tests quick_check ne baisse pas.
- TESTS_SCRIPT_AJOUTES: 0 (les équivalents existaient). TEST_ROUGE: sans objet pour ce lot de suppression ; le rouge du pont (Market.ServerBoard, bouchon dans HOLMarketBoardScript.as) reste à voir à la compilation (cf. SAURON-1116s).
- LIGNES_CPP_SUPPRIMEES (git show --stat f1dc2dc : +30 / -64, net -34) : HOLMarketRules.cpp -30 ; HOLMarketRules.h -7 / +1 (commentaire) ; HOLMarketSubsystem.cpp -4 / +8 (repli remplacé par log + valeur d'entrée) ; HOLMarketSubsystem.h -7 / +8 (commentaires) ; HOLMarketBridgeTests.cpp -8 / +11 ; HOLMarketRulesTests.cpp -8 / +2 (remplacées par 2 lignes de commentaire). Règle C++ supprimée proprement dite : 36 lignes (Recover + Received, .h et .cpp). LIGNES_MIGREES_PAR_LOT: 36.
- BUILDS_DEMANDES: 1, à lancer par la session principale : `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts Cpp,Script` (attendus : Market.ServerBoard, Market.Rules, Market.Tribute, Market.Apply/.Chain verts ; Test_MarketRulesVectors, Test_MarketTributeVectors verts).

## Greps (Source\ et Script\, *.cpp *.h *.as)
- `HOLMarket::Recover` / `HOLTribute::Received` : plus aucun APPEL en C++ ; il ne reste que des commentaires (HOLProductionSubsystem.h:62, HOLMarketSubsystem.h:15, 54, 59) et les appels du script (HOLMarketBoardScript.as, tests .as). `HOLTribute::DefaultTaxPercent` gardé (HOLProductionSubsystem.cpp:255, HOLDiplomacyReadLibrary.cpp:163, HOLMarketBridgeTests.cpp).
- Gardé et listé (appelants C++ restants) : Apply, NewBook, SettingsFromRow, Sell, Buy, BuyWithGold, LoadStandardSettings, FSettings/FBook/FTrade, Lot, TradableResources, BookSize (HOLGameState, HOLGameScreen.cpp:2373-2390, HOLDiplomacyReadLibrary.cpp:163, HOLPlayerController, HOLAIOrders, HOLProductionSubsystem.cpp:216).
- TYPES_A_EXPOSER: aucun.

## NON VÉRIFIÉ
- Tout : aucun build, aucun test. Risques à la compilation : le tableau local `FRecoverCase` à tableau intégré initialisé en accolades imbriquées, la boucle `for (const int32 (&Case)[2] : SentAndArrives)` du test ; paramètre `Settings` inutilisé dans le repli de `Recover` (pas une erreur UE). Le repli « sans board » n'est couvert par aucun test (un Error loggé ferait échouer un test automatisé sans AddExpectedError ; un monde sans board exige un type de monde non supporté).
- Anomalie : la file (ligne MIG-CPP-Market) disait que HOLMarketRules.cpp/.h seraient « supprimés » et que Market.Rules serait retiré en entier ; faux tant que Apply/NewBook/devis restent C++ : correction écrite dans la ligne.
- Docs mises à jour : ligne MIG-CPP-Market de Auto_Journey_File.md, AngelscriptInProgress.md (l. 254), EtapeDefini.md (l. 30), IN_USE.txt du worktree (libre, CPP: oui). Placeholders.txt : sans objet (aucun provisoire ajouté ni retiré).
- PROCESSUS_DETACHES: 0 (aucun lancé).
