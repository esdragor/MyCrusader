# ELROND-1119ba (migration C++ -> Angelscript, reprise 16, 2026-11-19) - NON COMPILE, aucun build lance

APPELS_DECLARES: 16 (arret a 25 vus ; seuil 80 juge sur tool_uses)
HEURE_DEBUT/HEURE_FIN: 15:22:52 / ~15:26
PREMIER_BUILD: sans C++ (le retrait d'un test C++ est le seul changement C++)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++ (non lance)
TEST_ROUGE: sans objet pour A (retrait d'un test C++ dont la regle est couverte par un test script existant ; rouge prevu dans l'en-tete de HOLPlayerOrders_Test.as : 201 -> 200 dans le cas (1, 201)) ; B : test d'outil, pas de rouge rejoue. Assertit l'effet : oui (verdict de IsAccepted calcule par la regle script)
COMMITS: 2, dernier a 15:24 (667d3ad, 1ace000), merge_main_fait: non (consigne : jamais de merge)
TESTS: aucun lance (aucun build/quick_check demande) ; TESTS_TAUTOLOGIQUES: 0 (les cas du test script sont des valeurs litterales confrontees au verdict calcule par la regle script)
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- A (667d3ad) : issue (2). Le test C++ `HeroesOfLiberty.AI.Orders.Rule` (HOLAIOrdersTests.cpp:154-166) est retire : HOLPlayerOrders::IsAccepted C++ passe par UHOLPlayerOrdersBoard::AskIsAcceptedAnywhere (HOLPlayerOrders.cpp:64-67), qui renvoie -1 sans planche de monde de jeu = refus -> les 3 cas "accepte" etaient rouges hors monde. Couverture script : `Test_PlayerOrdersIsAccepted` de Script\Commands\HOLPlayerOrders_Test.as:17-30 : plafond 200 (`GetMaxOrderedUnits()==200` + cas (1,200) accepte), slot 1/200 accepte, slot 0/1 accepte, dernier slot (7)/0 accepte, 201 refuse, -1 (INDEX_NONE) refuse, MaxSlots (8) refuse, -1 unite refuse. Ajout : `AssertEquals(8, MaxSlots)` pour que "slot 7 / slot 8" soient bien les cas nommes (FHOLMatchConfig::MaxSlots=8, HOLMatchConfig.h:107). Pas de test C++ des autres cas touche.
- B (1ace000) : lobby_match.ps1 ~625-640 : apres "Menu: back in lobby" des deux jeux, le salon du gestionnaire est sonde toutes les 500 ms jusqu'a 20 s tant que Alice ET Bob ne sont pas nommes ; le verdict final (2 joueurs Alice et Bob exiges, ligne 732 inchangee) n'est pas affaibli. Syntaxe PowerShell verifiee par le parseur (0 erreur) ; test non rejoue.
- RESTE : la session principale lance build + quick_check (parties tests + script) et un lobby_match.ps1 complet.
NON VERIFIE: rien compile ni execute. Test script Test_PlayerOrdersIsAccepted (en-tete "NOT COMPILED") jamais vu rouge puis vert dans cette branche ; Test_PlayerOrdersBoardAgreesIsAccepted compare aussi le pont a la regle. Le total quick_check baisse de 1 (test C++ retire) alors que le total script est +0 (assertion ajoutee, pas de test neuf) : exception voulue par la tache (issue 2). Le correctif B n'a pas ete rejoue (le rouge etait intermittent).
PROCESSUS_DETACHES: aucun (aucun lancement ; stray_processes non lance)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: non lance ; aucun fichier ecrit dans la copie principale hors ce rapport

BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 1 (HeroesOfLiberty.AI.Orders.Rule) ; TESTS_SCRIPT_AJOUTES: 0 (1 assertion ajoutee a Test_PlayerOrdersIsAccepted) ; LIGNES_CPP_SUPPRIMEES: 13 (test) ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: aucun
Commande a lancer : quick_check -Parts tests (puis script) sur le worktree HeroesOfLiberty-Elrond apres Tools\build.ps1 -LiveCoding=0 -Wait ; puis Tools\Tests\lobby_match.ps1.
