# GLORFINDEL-1119a : D851 etape 5, lot R1 (marche + tribut de la production)

Impact joueur : aucun a ce stade (le C++ `ApplyTribute` n'est pas touche, le tribut joue comme avant) ; le lot suivant le remplacera par la regle script (risque moyen : ressources).

Branche net-playtest, commit 9d715d8, NON COMPILE (aucun build, aucun quick_check). Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel.

## Etat avant

- Marche : deja en script. `HOLMarket::Apply` / `Recover` / `NewBook` (HOLMarketRules.as) + `HOLMarketBoardScript.as` (TradeOnce, RecoverPrices, FreshBook). Le C++ `ApplyMarketTrade` (HOLProductionSubsystem.cpp:200) ne garde que la validation de l'ordre (type Marche, ressource, lots <= 50), le livre `GameState`, `SetResources`, les logs. Devis Sell / BuyWithGold : reste en C++ (D850, non touche).
- Tribut : seules `HOLTribute::Received` (montant apres taxe) et `Check` (raisons) etaient en script ; `Check` n'etait appelee par aucun code de jeu. Le debit/credit (`ApplyTribute` :236, GiverStock[Resource] -= Amount ; TakerStock[Resource] += Arrives ; refus si stock < Amount) etait en C++.

## Fait

1. Test ROUGE d'abord, dans `Script\Economy\HOLMarketApply_Test.as` (etendu) : `Test_TributeSettleDebitsAndCredits` (100 bois a 30 % : giver 500 -> 400, receveur 5 -> 75, Received 70, Tax 30, autres ressources intactes, entree non modifiee), `Test_TributeSettleWholeStockOfGold` (250 or, tout le stock, 175 arrivent), `Test_TributeSettleRefusals` (99 pour 100 : NotEnough, rien ne bouge ; NoMarket, Self, NoRecipient, BadAmount, BadResource). Valeurs recopiees du comportement de `ApplyTribute` (taxe `HOLTribute::DefaultTaxPercent` = 30, `Received` arrondi a l'entier inferieur de la taxe).
2. Regle pure en script, `Script\Economy\HOLTributeRules.as` : `struct FHOLTributeSettled`, `HOLTribute::StockOf`, `HOLTribute::WithAdded`, `HOLTribute::Settle(bSenderHasMarket, Sender, Receiver, bReceiverInMatch, GiverStock, TakerStock, Resource, Sent, TaxPercent)`. Passe par `Check` puis `Received`.
3. Pont C++ minimal (raison D839 : liaison a exposer au script) :
   - `Match\HOLMarketBridgeTypes.h` : `FHOLTributeBridgeResult` (+ include HOLResources.h).
   - `Match\HOLMarketSubsystem.h/.cpp` : `static SettleTribute(World, ...)`, `SettleNow`, evenement natif `TributeSettle` (defaut inerte : refus Reason -1, stocks inchanges, erreur journalisee).
   - `Script\Economy\HOLMarketBoardScript.as` : `TributeSettle` BlueprintOverride -> `HOLTribute::Settle`.
   Le C++ `ApplyTribute` n'est PAS encore branche sur le pont ni supprime (lot suivant, session principale).

## Rouge : commande a poser une fois (apres le build)

Le fichier de test appelle `HOLTribute::Settle` qui existe deja : pour VOIR le rouge, fausser une valeur attendue dans le script, par exemple dans `HOLTributeRules.as` `Settle` remplacer `-Sent` par `-Arrives` (le giver paierait 70 : `Test_TributeSettleDebitsAndCredits` donne 430 au lieu de 400, `...WholeStockOfGold` donne 75 au lieu de 0), lancer `quick_check -Parts tests` (attendu N-2/N pour ces deux tests), remettre `-Sent`, relancer (vert). Autre rouge possible sans edition de la regle : `T.AssertEquals(400, ...)` -> 430 dans le test, rouge puis remis.

## A lancer par la session principale

1. Build : `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\build.ps1 -LiveCoding=0 -Wait` (le .h change : USTRUCT + UFUNCTION natives de `UHOLMarketSubsystem`) ; avant, `Get-StaleBinaryProblem` du worktree (binaire perime, main 20a5e2f).
2. `quick_check -Parts tests` : les 3 tests `Test_TributeSettle*` doivent etre verts ; le total ne baisse pas (+3).
3. Net : `market` (HeroesOfLiberty.Market.ServerBoard) ; scenario RPC `rpc_orders_p3p4_run.ps1` (ordres de marche et tribut).
4. Lot suivant (apres vert) : dans `ApplyTribute`, remplacer le bloc verification stock + `GiverStock[Resource] -=` / `TakerStock[Resource] +=` par `UHOLMarketSubsystem::SettleTribute(GetWorld(), true, Order.PlayerIndex, Order.Receiver, Taker != nullptr, Giver.GetResources(), Taker->GetResources(), Order.Resource, Order.Amount, HOLTribute::DefaultTaxPercent)` puis `SetResources(Result.Giver/Taker)` ; garder le journal `Tribute: slot ...` et `NoteTributeReceived`. Puis retirer le `Received` C++ devenu inutile du pont.

## Champs

- BUILDS_DEMANDES: 1 (lot C++ de liaison, par la session principale)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 3 (Test_TributeSettleDebitsAndCredits, Test_TributeSettleWholeStockOfGold, Test_TributeSettleRefusals)
- LIGNES_CPP_SUPPRIMEES: 0
- LIGNES_MIGREES_PAR_LOT: 0 (la regle est ajoutee en script ; le retrait C++ vient au lot suivant)
- TYPES_A_EXPOSER: FHOLTributeBridgeResult (USTRUCT neuf ; FHOLResources deja expose)
- TEST_ROUGE: non vu (aucune execution dans cet agent), a voir selon la section Rouge
- TESTS_TAUTOLOGIQUES: 0 (la chaine passe par Check puis Received)

## NON VERIFIE

Rien n'a ete compile ni execute : syntaxe Angelscript (struct avec membre `Received` et fonction de meme nom dans le namespace, reouverture du `namespace HOLTribute` dans le meme fichier), compatibilite de la signature `TributeSettle` avec les parametres `const FHOLResources&` natifs, nom de test sans collision avec un test existant.

## Anomalies / doutes

- Fins de ligne : les fichiers sont en CRLF ; normalises en CRLF pour les ajouts.
- Placeholders.txt non touche (aucun provisoire ajoute ou retire).
- Doute : `Check` renvoie `NoRecipient` avant `NoMarket` (ordre du script existant), alors que C++ ignore tout dans une meme condition ; sans effet joueur (un seul message de refus journalise).
