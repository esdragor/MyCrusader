# CELEBORN-1119m : D851 etape 5, lot R2 (le tribut n'a plus qu'une source : le script)

Impact joueur : aucun voulu (memes montants debites et credites, memes refus, memes lignes de journal) ; risque moyen (ressources), NON COMPILE.
Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche hero-j8-close (depart main 3b52f7a). Pas de push, pas de merge.

## Etapes (un commit chacune)

1. 14d2a50 (tests) :
   - Script `Script\Economy\HOLMarketApply_Test.as` : `Test_TributeSettleReasonOrderAndConservation` (ordre des raisons tel que ApplyTribute le faisait : NoMarket avant stock, Self avant NoRecipient, BadAmount avant BadResource, BadResource avant stock ; conservation : envoye = arrive + taxe, 99 a 30 % -> 70). Les 3 tests d'hier (`Test_TributeSettle*`) restent.
   - C++ `Tests\HOLMarketBridgeTests.cpp` (Market.ServerBoard) : appel REEL du pont `UHOLMarketSubsystem::SettleTribute` dans un monde de test (100 bois : 500->400 / 5->75, Received 70 ; 99 pour 100 : Reason 4, stocks inchanges). Cela fixe aussi le numero 4 (NotEnough) que le C++ lit.
   - Chemin reel de l'ordre (file d'ordres -> pas de production -> ApplyTribute -> stocks -> message) : DEJA couvert par le test C++ `HeroesOfLiberty.Market.Tribute` (FHOLTributeChainTest, HOLMarketRulesTests.cpp:224) : debit 200 bois, credit 140, or 50 -> 35, message au receveur seul (140/60), et DOUZE refus sans mouvement (plus que le stock, 0, -5, soi-meme, slot 7, INDEX_NONE, ressource 9, INDEX_NONE, Marche non fini, Marche d'un autre, pas un Marche, pas de batiment). Ce test n'a pas ete modifie : c'est lui qui prouve le branchement (rouge s'il casse). Un test script ne peut pas creer le monde (sous-systemes, GameState) : le chemin reel reste en C++ (liaison, D838).
2. 889f50f (branchement + retrait) : `ApplyTribute` (HOLProductionSubsystem.cpp:236) appelle `UHOLMarketSubsystem::SettleTribute(GetWorld(), bSenderHasMarket = type == Market, Sender, Receiver, bReceiverInMatch = Taker != nullptr, stocks, Resource, Amount, DefaultTaxPercent)` puis `SetResources(Done.Giver / Done.Taker)`. Retire du C++ : la definition `bResource` et la condition (ressource, montant, Marche, receveur), la comparaison de stock, l'appel `Received`, `GiverStock[...] -=` et `TakerStock[...] +=` (9 lignes de regle ; la fonction passe de 33 a 33 lignes car le journal et les deux branches de refus restent, git : +18/-18). `HOLMarketRules.h` : +2 lignes (`HOLTribute::NotEnoughReason = 4`, pour distinguer la ligne « cannot give » de la ligne « ignored » ; miroir du numero du script, tenu par le test du pont). Devis du Marche du HUD : non touche (D850).
3. Pont absent : `Find(World)` nul (pas de board) -> `SettleTribute` journalise une erreur (LogTemp, « no script board ... »), renvoie Reason -1 et les stocks inchanges ; board present mais .as non compile -> `TributeSettle_Implementation` journalise l'erreur et refuse (Reason -1). Dans les deux cas ApplyTribute ecrit la ligne « Tribute: slot A to slot B ignored (...) » et NE DEBITE PERSONNE (comportement sur, aucun tribut perdu ni cree). `ApplyTribute` ne tourne que cote serveur (pas de production sur client) ; le .as est dans `Script\Economy\` (pas Dev / Editor / Examples / Developers : verifie), donc il naît sur le serveur dedie. Garde en plus `|| !Taker` contre un `bOk` inattendu.

## Journaux lus (grep Tools\, Script\)

| Fichier:ligne | Texte lu | Etat |
|---|---|---|
| Tools\Tests\quick_check.ps1:7296 | `Tribute: slot $BobSlot gave 100 of resource 3 to slot $AliceSlot, 70 arrived (tax 30)` | inchange mot pour mot |
| Tools\Scenarios\diplomacy_compare.txt:136 | `waitlog Tribute: slot 0 gave 100 of resource 3 to slot 3` | inchange |
| rpc_orders_p3p4_run.ps1, net_match.ps1, autres Scenarios\*.txt | aucune ligne `Tribute:` lue | - |
Les trois autres lignes (`cannot give`, `ignored`, `Tribute message:`) ne sont lues par aucun outil ; elles sont gardees telles quelles (meme format, meme niveau Display, meme LogHOLCommands).

## Champs
- BUILDS_DEMANDES: 1 (C++ : HOLProductionSubsystem.cpp, HOLMarketRules.h, HOLMarketBridgeTests.cpp ; a grouper avec le lot C++ de la seance)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 1 (Test_TributeSettleReasonOrderAndConservation) ; +1 test C++ de pont (assertions ajoutees a Market.ServerBoard)
- LIGNES_CPP_SUPPRIMEES: 9 (regle) ; LIGNES_MIGREES_PAR_LOT: 9
- TYPES_A_EXPOSER: aucun nouveau
- TEST_ROUGE: non vu (aucune execution). Pour le voir : dans HOLTributeRules.as `Settle`, remplacer `-Sent` par `-Arrives` : `Market.Tribute` (C++ chain) et `Market.ServerBoard` ET les tests script donnent rouge ; remettre.
- TESTS_TAUTOLOGIQUES: 0
- Placeholders.txt : non touche (aucun provisoire).

## NON VERIFIE
Rien compile ni execute : syntaxe de `Test_TributeSettleReasonOrderAndConservation`, includes de HOLProductionSubsystem.cpp (FHOLTributeBridgeResult via HOLMarketSubsystem.h : deja inclus), `FHOLResources::operator==` et constructeur 4 entiers dans le test du pont (utilises tels quels par le chain test), `GetResources()[EHOLResource]`. Equivalence d'ordre des raisons : le C++ avait [Marche, receveur, ressource, montant] puis stock ; le script a [montant, ressource, soi, receveur, Marche] puis stock : seul change, sans effet joueur, le choix entre deux raisons simultanees, et les deux donnent la meme ligne « ignored » (seul NotEnough change de ligne, et il reste le dernier).

## Commandes pour la session principale
1. `. Tools\Scenarios\LaunchCheck.ps1; Get-StaleBinaryProblem C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` puis `Tools\build.ps1 -LiveCoding=0 -Wait`.
2. `quick_check -Parts tests` : `HeroesOfLiberty.Market.Tribute`, `Market.ServerBoard`, `Test_TributeSettle*` (4 en script) verts ; total +1 au moins.
3. Preuve reseau : `Tools\Tests\quick_check.ps1` partie market (ligne 7296 : serveur dedie, Bob -> Alice 100 or, `70 arrived (tax 30)`), `rpc_orders_p3p4_run.ps1`, scenario `diplomacy_compare` (waitlog ligne 136).

## Questions
Aucune (pas de design tranche). Reste du lot : `UHOLMarketSubsystem::Received` (C++) + `TributeReceived` ne servent plus qu'au test du pont (plus d'appel de jeu) : a retirer au lot suivant, avec ses 7 cas dans HOLMarketBridgeTests.cpp.
