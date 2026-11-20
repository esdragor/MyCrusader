# GLORFINDEL-1120pe : case de grille « Sortir tous » (D813, point 3 du lot garnison d'ELROND-1120pb)

NON COMPILE (C++ touché, aucun build, aucun quick_check, par consigne).

APPELS_DECLARES: ~22 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 16:03 / 16:10
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (pas de compilation par consigne)
TEST_ROUGE: 58e69f7 (stub qui compile : valeurs d'enum UngarrisonAll, champ bGarrisonOccupied, GetUngarrisonAllCell, aucune règle) ; assertit l'effet : oui (la cellule rendue par GetCommand passe de None à UngarrisonAll selon l'état de la garnison, la production A reste, un chantier ou une sélection d'unités n'a pas la case) ; rouge NON VU à l'exécution
COMMITS: 3 (58e69f7 test seul, d3fcb3e correctif, 91a6f36 journal), dernier 16:10, merge_main_fait: oui (Already up to date)
TESTS: aucun lancé ; TESTS_SCRIPT_AJOUTES: 1 (Test_GridUngarrisonAllCellOfAGarrisonedBuilding dans Script\Tests\HOLCommandGrid_Test.as) ; TESTS_TAUTOLOGIQUES: 0 (la valeur assertie est calculée par HOLCommandGridRules::GetCommand)
ESSAIS: aucun (pas de build)
QUESTIONS: Retours\Q-garrison-cell.md (case et touche, défaut A : case 7 = touche F ; `const int UngarrisonAllCell = 7;` dans HOLCommandGridRules.as)

## Fait
- Le GDD (D813) dit « case de grille Sortir tous (touche, comme les autres ordres) » sans case ni touche : la touche est celle de la position de la case (grille de clavier existante, HOLCommandGridKeys.as : bOwnBuilding = cible Grille), donc case 7 = F par défaut, derrière une constante d'une ligne.
- Règle en script (D839) : HOLCommandGridRules::GetCommandWithOptions, branche d'un bâtiment fini non Marché : si bGarrisonOccupied, pas de groupe de bâtiments, Cell == UngarrisonAllCell -> EHOLGridCommandKind::UngarrisonAll (après les cellules de production, avant spécialisation et recherche).
- C++ minimal (liaison) : EHOLGridCommand::UngarrisonAll (HOLCommandGrid.h) et EHOLGridCommandKind::UngarrisonAll (HOLCommandGridReadLibrary.h, même ordre, fin d'enum) ; champ bGarrisonOccupied dans FHOLGridContext et FHOLGridContextView, copié par les trois liaisons (HOLCommandGridBoard.cpp:~71, HOLCommandGridReadLibrary.cpp:~30, HOLScriptScreen.cpp:~90).
- HOLPlayerController::GetGridCommands : bGarrisonOccupied = bâtiment fini && GetGarrisonOccupants().Num() > 0 (bâtiment sélectionné seul, GetOwnSelection).
- HOLPlayerController (dispatch de commande de grille) : case UngarrisonAll -> UHOLControllerCommandLibrary::RequestUngarrisonAllSelected(this) (include ajouté).
- HOLGameScreen.cpp : libellé « Sortir » et infobulle « Sortir tous ».
- L18-d : grep des tests qui comptent les cases d'un bâtiment (HOLCommandGrid_Test.as 572/590/595/607/620/631, Coverage, Sweeps, Rules_Test) : aucun ne met bGarrisonOccupied, défaut false, donc leurs cellules vides ne changent pas.

## Reste
- Build + quick_check tests (parties script et Mass) par la session principale ; vérifier le rouge sur 58e69f7 (compte N-1/N).
- Essai du geste : bâtiment garni sélectionné, touche F (ou clic) fait sortir tout le monde ; non fait.
- Si Q-garrison-cell tranche autre chose : changer UngarrisonAllCell.
- Non touché : HOLCommandGridPanel.as (si le panneau script rend les cellules par type, il faudra le libellé du nouveau type), GetReportName (nom de rapport de la cellule : retombe sur TypeName vide).

## NON VÉRIFIÉ
Rien n'est compilé : noms UHT de l'enum et du champ, include HOLControllerCommandLibrary.h dans HOLPlayerController.cpp, que le switch du dispatch de HOLPlayerController soit bien celui de la grille du jeu (case Deploy voisin), l'indentation des insertions, que GetGarrisonOccupants soit const accessible depuis un `const AHOLBuilding*` (déclarée const, HOLBuilding.h:136), et que les liaisons de la grille copient bien tous les champs (grep fait sur bDeployedAny : 3 sites trouvés, tous modifiés).

DIAGNOSTICS_AS: non relus (pas d'éditeur)
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: edit_line.ps1 via `powershell -File` refuse un -Append dont la valeur commence par « - » ; `$E` et `$e` sont la même variable en PowerShell (2 appels perdus). IN_USE.txt remis à « libre ».
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md, Retours\Q-garrison-cell.md, ce rapport) : oui
