# CELEBORN-1124f : grille fine G5 (serveur : une piece par sous-case), integ-41

NON COMPILE : sans C++, aucun build. Le jeu charge les scripts (play.ps1 -Hidden), 0 erreur Angelscript.

APPELS_DECLARES: 20 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 11:28:33 / 11:45
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: c383070 + assertit l'effet : oui (13 sous-cases x 2 Paysans = 26 ordres de batisseur, file : la 1re piece non mise en file, les suivantes oui ; Maj : la 1re aussi), sur la chaine PlanFromRequestFine (SubDivision 4) puis OrdersForLine. Vu a 11:36 : tests 2204/2205, seul Test_WallLineFineGridOrdersOnePiecePerSubCellForEveryPeasant rouge.   TEST_ROUGE_SEPARE: oui
COMMITS: 2 (c383070 rouge + bouchon, c1cae68 regle + branchement), dernier a 11:38, merge_main_fait: non (non demande, branche integ-41 : la session principale synchronise)
TESTS: quick_check -Parts tests : rouge 2204/2205 (11:36) ; apres correctif : tests 2205/2205 PASS (quick_check 221 s, vu apres le heure d arret 11:43 : lance avant, fini a ~11:44) ; scenario wall_line_skip : play.ps1 -Hidden -Root worktree, joue en 28 s jusqu'a la fin (ready), journal `place wall line received by the server: slot 0 type Palisade builders 1 queue false`, 0 erreur Angelscript (les 20 `LogAutomationTest: Error: Condition failed` sont les autotests moteur au demarrage, avant le chargement du scenario) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (pas de E-*.md : G5 n'a pas d'effet visible a SubDivision 1)
QUESTIONS: aucune
FAIT / RESTE:
- FAIT HOLWallLineRules.as : `FHOLWallOrder {Cell, Builders, bQueue}` et `OrdersForLine(Plan, UnitIds, bQueue)` (une commande par case du plan, Paysans de BuildersForCell, file de QueueForCell : une seule regle).
- FAIT HOLRpcPlayer.as `ServerPlaceWallLine` : `HOLWallLine::SubDivision()` -> `PlanFromRequestFine(..., Tile, SubDivision, ...)`, pas `Pitch = Tile / SubDivision`, une `OrderPlaceBuilding` par commande au centre de la case (`CellToPoint(Cell, Pitch, Z)`). A SubDivision 1 (defaut) : PlanFromRequestFine delegue a PlanFromRequest, meme plan, meme pas : comportement inchange.
- RESTE G4 (C++) : a SubDivision > 1 le serveur envoie bien N pieces au pas de 100, mais `UHOLConstructionSubsystem::SnapFootprint` (HOLConstructionSubsystem.cpp:73) rabat encore chaque piece sur la case de 4 m et `GetWallLineWorld` rend l'occupation en cases de 4 m : le scenario `wall_line_long` a SubDivision 4 (Bois = 5 x pieces) ne peut pas passer avant G4. Pas de scenario a SubDivision 4 lance pour cette raison.
TYPES_A_EXPOSER: aucun nouveau (G4 : SnapFootprint par type et GetWallLineWorld avec un pas, deja prevus au plan)
BUILDS_DEMANDES: 0 (G4 les demande)   TESTS_SCRIPT_AJOUTES: 1 (Test_WallLineFineGridOrdersOnePiecePerSubCellForEveryPeasant)
DIAGNOSTICS_AS: 0 erreur (chargement du jeu par play.ps1 ; serveur de langage VS Code non consulte)
NON VERIFIE: effet Bois = 5 x pieces a SubDivision 4 (depend de G4) ; la reception des commandes par le moteur avec des positions au pas de 100 ; Auto_Journey_File.md n'a pas de ligne G5 a cocher (la ligne est dans PLAN-GRILLE-FINE-1124.md, non editee, document d'une autre session).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: le quick_check `tests` du rouge a depasse 180 s et est passe en tache de fond (276 s) ; un 2e quick_check lance en fond pour le vert, sortie dans CELEBORN-1124f_qc.txt (worktree, non commitee).
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, FortificationsInProgress.md, ce rapport) : oui
