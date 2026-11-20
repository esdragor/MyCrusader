# HALDIR-1120pc : dessin du glisser de murs (F4 client)
Joueur : pendant le glisser d'un mur, les cases de la ligne apparaissent sur le sol (vert = le relache pose la ligne, rouge = le serveur la refuse). Rien ne s'affiche avant F5 (WallTypes vide : le glisser ne demarre pas).

APPELS_DECLARES: ~27 (arret a 25 vus ; depasse de peu pour le journal / rapport)
HEURE_DEBUT/HEURE_FIN: 15:54 / 15:59 (Get-Date)
PREMIER_BUILD: sans C++ (1 UFUNCTION C++ ajoute, NON COMPILE : a mettre dans le lot C++)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++ (aucun build)
TEST_ROUGE: 2b779a4 (4 tests + stub), assertit l'effet : oui (cases montrees comparees a la ligne de Validate, verdict, case fautive) ; rouge NON VU a l'execution
COMMITS: 3 (2b779a4 rouge, dfaf618 regle, 85184e3 dessin + C++), dernier 15:58, merge_main_fait: oui
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun (WallTypes vide : rien a jouer avant F5)
QUESTIONS: aucune
FAIT / RESTE:
- Test_WallGhostShowsTheLineTheReleaseLays, ...OfAnOccupiedLineIsRedAndNamesTheCell, ...OutOfTheMapIsRed, ...TooLongShowsOneCellPastTheCap (Script\Tests\HOLWallGhostRules_Test.as).
- Regle HOLWallGhost::Build / BuildFromPoints (Script\Buildings\HOLWallGhostRules.as) : une source avec HOLWallLine::Validate / DragCells.
- Dessin Script\Buildings\HOLWallGhostDrawer.as : UHOLWallGhostDrawer (UScriptWorldSubsystem, Tick) + AHOLWallGhostActor, ISM du cube du moteur + M_MatteColor, un cube plat par case, vert / rouge ; lit Occupied / carte une fois par glisser (HOLConstructionRead::GetWallLineWorld).
- C++ (raison : liaison a exposer au script, pas de moyen script de lire l'etat du glisser) : AHOLPlayerController::GetWallDragView (UFUNCTION) + champ WallDragEnd (HOLPlayerController.h / .cpp), a compiler dans le lot C++. HOLSelectionMarkers.cpp NON touche.
- Reste : build + quick_check -Parts tests ; verifier a l'oeil quand F5 rend un mur posable.
NON VÉRIFIÉ: rien compile ni execute ; noms de l'API script devines sur les modeles du code (HOLConstructionRead::GetWallLineWorld sans contexte monde, UInstancedStaticMeshComponent::Create / ClearInstances, Gameplay::GetPlayerController, Log) ; DIAGNOSTICS_AS: non relus pour le drawer (hook IDE n'a signale que des Hint sur l'ancien stub) ; le sous-systeme se cree-t-il sans declaration ? (modele : UHOLBiomeApplier) ; M_MatteColor visible sous le fantome.
ANOMALIES_PROCESSUS: aucune ; Content/Maps/Crown.umap non suivi dans le worktree (pas de moi).
GIT_STATUS_PRINCIPALE: hors docs inchange : oui (aucun fichier de code touche dans la copie principale)
BUILDS_DEMANDES: 1 (lot C++)   TESTS_SCRIPT_AJOUTES: 4 (Test_WallGhost*)   TYPES_A_EXPOSER: aucun de plus (GetWallDragView est la liaison)
Commande : Tools\build.ps1 -LiveCoding=0 puis quick_check -Parts tests (worktree Haldir).
