# CELEBORN-1124e : grille fine, etape G3 (D898), debut et fin sous la souris
APPELS_DECLARES: ~19 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:16:46 / ~11:22
PREMIER_BUILD: sans C++   BUILD: NON COMPILE (sans C++)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 9820aad (bouchon PieceBox boite nulle + Test_WallLineFineGridStartAndEndBoxesTouchTheMouseCells) ; assertit l'effet : oui (boites calculees par PieceBox sur les cases du plan rendu par PlanFromRequestFine). Rouge NON VU a l'execution (pas de quick_check dans l'agent). TEST_ROUGE_SEPARE: oui
COMMITS: 2 (9820aad rouge 11:18, 62eedb4 correctif 11:18), merge_main_fait: non (inutile : Script seul, integ-41 deja a jour au debut)
TESTS: aucun lance. TESTS_SCRIPT_AJOUTES: 1 (Test_WallLineFineGridStartAndEndBoxesTouchTheMouseCells)   BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun   TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun liste. Chargement : play.ps1 wall_line_skip -Hidden -Root <worktree> 36 s ; DIAGNOSTICS_AS: 0 erreur (seul avertissement : HOLWallLineRules.as:58:27 division entiere dans Cells, ancien, hors G3). Le journal montre des "LogAutomationTest: Error: Condition failed" (non attribues a G3, aucune mention de mes tests ni de PieceBox).
QUESTIONS: aucune
FAIT: HOLWallLineRules.as : struct HOLWallLine::FHOLWallPieceBox (MinX/MinY/MaxX/MaxY) et PieceBox(Cell, Pitch) = [Cell x Pitch, Cell x Pitch + Pitch]. Test : ligne G1 -> boite 1 MinX 0, boite 17 MaxX 1700, contigues, somme 1700, profondeur 100 ; diagonale (50,50)->(850,850) = 9 cases, derniere (8,8), boite 800..900 ; snap 8 directions.
ANOMALIE DU PLAN : G3 dit « tir a 30 degres : colle sur l'axe ». La regle (Cells, seuil Lo x 1000 < Hi x 414 = tan 22.5 deg) envoie 30 deg sur la DIAGONALE (la plus proche des 8 directions). Test ecrit sur la regle reelle : 20 deg = axe, 30 deg = diagonale. Si l'utilisateur voulait un autre seuil, c'est un design a trancher (non fait). Anomalie d'Occupied en cases de 4 m (1124d) : deja traitee en G2, PieceBox n'y touche pas.
Reste : PieceBox n'est appelee que par le test ; G4 (C++) branchera GetWallLineWorld / SnapFootprint ; defaut hol.Wall.SubDivision = 1 inchange (la regle ne lit pas le cvar).
NON VERIFIE: rouge a l'execution ; test vert ; Pitch = Tile / SubDivision reste fourni par l'appelant.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (Auto_Journey_File, EtapeDefini, FortificationsInProgress, Docs\Rapports) : oui
